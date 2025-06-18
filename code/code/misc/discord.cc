#include "discord.h"

#include <atomic>
#include <boost/program_options.hpp>
#include <boost/program_options/options_description.hpp>
#include <boost/program_options/parsers.hpp>
#include <boost/program_options/value_semantic.hpp>
#include <boost/program_options/variables_map.hpp>
#include <condition_variable>
#include <curl/curl.h>
#include <curl/easy.h>
#include <exception>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <mutex>
#include <queue>
#include <stdexcept>
#include <string>
#include <thread>
#include <unistd.h>
#include <utility>

#include "colorstring.h"
#include "log.h"
#include "sstring.h"

// Static member variables for Discord configuration
Discord::Channels Discord::channels;
Discord::Settings Discord::settings;

// Anonymous namespace for Discord implementation details
namespace {
  // Variables for thread management
  std::atomic<bool> stop_thread = false;
  std::thread messenger_thread;
  std::condition_variable cv;
  std::queue<std::pair<sstring, sstring>> message_queue;
  std::mutex queue_mutex;

  // Helper functions for easy logging directly to console, as `vlogf` isn't
  // thread-safe and shouldn't be called within the messenger thread.
  void error(const sstring& msg) {
    std::cerr << "Discord Webhooks: " << msg << std::endl;
  }

  void info(const sstring& msg) {
    std::cout << "Discord Webhooks: " << msg << std::endl;
  }

  // Sends a message to a specific Discord channel via webhook using the curl
  // library
  void sendMessage(const sstring& channel, const sstring& msg) {
    CURL* curl = curl_easy_init();

    if (!curl) {
      throw std::runtime_error("curl_easy_init() failed");
    }

    curl_easy_setopt(curl, CURLOPT_URL, channel.c_str());

    struct curl_slist* headers =
      curl_slist_append(nullptr, "Content-Type: application/json");
    curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);

    // sanitize and format the message
    const sstring content =
      format(R"({"content": "%s"})") % stripColorCodes(msg).escapeJson();
    curl_easy_setopt(curl, CURLOPT_POSTFIELDS, content.c_str());

    // TIMEOUT SETTINGS
    curl_easy_setopt(curl, CURLOPT_TIMEOUT, 10);
    curl_easy_setopt(curl, CURLOPT_CONNECTTIMEOUT, 5);
    curl_easy_setopt(curl, CURLOPT_LOW_SPEED_LIMIT, 1);
    curl_easy_setopt(curl, CURLOPT_LOW_SPEED_TIME, 5);
    curl_easy_setopt(curl, CURLOPT_NOSIGNAL, 1L);

    CURLcode res = curl_easy_perform(curl);
    curl_slist_free_all(headers);
    curl_easy_cleanup(curl);

    if (res != CURLE_OK) {
      if (res == CURLE_OPERATION_TIMEDOUT) {
        throw std::runtime_error("Request timed out");
      }

      throw std::runtime_error(sstring(
        format("curl_easy_perform() failed: '%s'") % curl_easy_strerror(res)));
    }
  }

  // Thread function for processing messages in the queue. Runs until the
  // stop_thread flag is set, processing messages from the queue as they
  // become available.
  void messenger() {
    // An exception thrown outside of the sendMessage call indicates something
    // unexpected occurring inside the messenger thread, so we should log it and
    // exit the background thread without terminating the main thread.
    try {
      info("Messenger thread started.");

      std::unique_lock<std::mutex> lock(queue_mutex);
      info("Messenger acquired lock. Waiting for messages.");

      // Thread should run as long as the game is running, only exiting after
      // all messages have been processed and stop_thread == true.
      while (true) {
        cv.wait(lock, [] { return !message_queue.empty() || stop_thread; });

        // If stop_thread is true, process any remaining messages before exiting
        while (!message_queue.empty()) {
          info("Processing message from queue.");
          std::pair<sstring, sstring> message = message_queue.front();
          message_queue.pop();

          // Unlock the mutex while each message is being processed, as the
          // queue won't be accessed again by this thread until the current
          // message is finished processing and the next iteration of the loop
          // starts.
          lock.unlock();

          // An exception thrown by sendMessage() should be caught and logged
          // but not rethrown, as individual messages failing to send shouldn't
          // terminate the messenger thread.
          try {
            sendMessage(message.first, message.second);
          } catch (const std::exception& e) {
            error(format("Exception in sendMessage: %s.") % e.what());
          } catch (...) {
            error("Unknown exception in sendMessage");
          }

          lock.lock();
        }

        if (stop_thread) {
          info("Stop signal received, messenger thread exiting.");
          return;
        }
      }
    } catch (const std::exception& e) {
      error(format("CRITICAL - Thread exception: %s. Discord "
                   "functionality disabled due to error") %
            e.what());

    } catch (...) {
      error(
        "CRITICAL - Unknown thread exception. Discord functionality disabled "
        "due to error.");
    }
  }
}  // namespace

// Reads the configuration from the `discord.cfg` file and starts the messenger
// thread
bool Discord::doConfig() {
  namespace po = boost::program_options;
  using po::value;
  static constexpr const char* helper_desc = "See discord.h";

  po::options_description config("Discord Configuration");

  // clang-format off
  config.add_options()
    ("deaths_webhook", value<sstring>(&channels.deaths), helper_desc)
    ("sys_webhook", value<sstring>(&channels.system), helper_desc)
    ("achieve_webhook", value<sstring>(&channels.achievements), helper_desc)
    ("achieve_threshold", value<int>(&settings.achievement_threshold), helper_desc);
  // clang-format on

  po::variables_map vm;
  std::ifstream ifs("discord.cfg");

  if (!ifs.is_open()) {
    vlogf(LOG_FILE,
      format("Discord Webhooks: Failed to open 'discord.cfg' file. "
             "Ensure config is located in '%s'.") %
        std::filesystem::current_path());
    return false;
  }

  try {
    po::store(parse_config_file(ifs, config), vm);
    po::notify(vm);
  } catch (const std::exception& e) {
    vlogf(LOG_FILE, format("Error parsing config: %s") % e.what());
    return false;
  }

  messenger_thread = std::thread(messenger);
  return true;
}

bool Discord::doCleanup() {
  vlogf(LOG_MISC, "Discord Webhooks: Cleaning up Discord messenger thread...");

  // With stop_thread being a std::atomic<bool>, no need to lock the mutex -
  // just set the flag and notify
  stop_thread = true;
  cv.notify_one();

  if (messenger_thread.joinable()) {
    vlogf(LOG_MISC,
      "Discord Webhooks: Waiting for messenger thread to finish processing "
      "queue...");
    messenger_thread.join();
  }

  vlogf(LOG_MISC, "Discord Webhooks: Cleanup finished.");
  return true;
}

void Discord::sendMessageAsync(const sstring& channel, const sstring& msg) {
  if (channel.empty() || msg.empty()) {
    // Missing required parameters, no reason to continue
    return;
  }

  std::lock_guard<std::mutex> lock(queue_mutex);
  vlogf(LOG_MISC,
    format("Discord Webhooks: Adding message to queue: '%s'") % msg);
  message_queue.emplace(channel, msg);
  cv.notify_one();
}
