#include <iostream>
#include <fstream>
#include <filesystem>
#include <queue>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <utility>
#include <chrono>

#include "discord.h"

#include "extern.h"
#include "colorstring.h"
#include "sstring.h"
#include <curl/curl.h>
#include <curl/easy.h>
#include <unistd.h> 

#include <boost/program_options.hpp>
namespace po = boost::program_options;

// available discord channels
// channels are configured in lib/discord.cfg
// new channels must be added in that file, here, in discord.h, and in
// po::options_descriptions below
sstring Discord::CHANNEL_DEATHS;
sstring Discord::CHANNEL_SYS;
sstring Discord::CHANNEL_ACHIEVEMENT;

// threshold level for discord mob kill notifications
int Discord::ACHIEVEMENT_THRESHOLD;

// for thread management
bool Discord::stop_thread = false;
std::thread Discord::messenger_thread;
std::condition_variable Discord::cv;
std::queue<std::pair<sstring, sstring>> Discord::message_queue;
std::mutex Discord::queue_mutex;

// read the configuration
bool Discord::doConfig() {
  using std::string;

  string configFile = "discord.cfg";

  std::string empty_string = "";

  po::options_description configOnly("Configuration File Only");
  // clang-format off
  configOnly.add_options()
    ("deaths_webhook",po::value<string>(&CHANNEL_DEATHS)->default_value(empty_string),"see discord.h")
    ("sys_webhook",po::value<string>(&CHANNEL_SYS)->default_value(empty_string),"see discord.h")
    ("achieve_webhook",po::value<string>(&CHANNEL_ACHIEVEMENT)->default_value(empty_string),"see discord.h")
    ("achieve_threshold",po::value<int>(&ACHIEVEMENT_THRESHOLD)->default_value(80), "see discord.h");
  // clang-format on
  po::options_description config_options;
  config_options.add(configOnly);

  po::variables_map vm;
  po::notify(vm);
  std::ifstream ifs(configFile.c_str());

  if (!ifs.is_open()) {
    vlogf(LOG_MISC,
      format("Discord webhooks: Failed to open config file '%s'") % configFile);
    vlogf(LOG_MISC,
      format("Discord webhooks: ensure config is located in: '%s'") %
        std::filesystem::current_path());
    return false;
  }

  po::store(parse_config_file(ifs, config_options), vm);
  po::notify(vm);

  // worker thread setup
  messenger_thread = std::thread(Discord::messenger);

  vlogf(LOG_MISC, "Discord webhooks: messenger thread started");
  
  return true;
}

// send a message to a discord webhook
// we use the curl library for this and keep it simple
bool Discord::sendMessage(sstring channel, sstring msg) {
  CURL* curl = curl_easy_init();
  if (!curl) {
    std::cerr << "curl_easy_init() failed" << std::endl;
    return false;
  }

  // sanitize and format the message
  msg = format("{\"content\": \"%s\"}") % stripColorCodes(msg).escapeJson();

  const char* webhookURL = channel.c_str();
  const char* content = msg.c_str();

  struct curl_slist* headers = NULL;
  headers = curl_slist_append(headers, "Content-Type: application/json");

  curl_easy_setopt(curl, CURLOPT_URL, webhookURL);
  curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
  curl_easy_setopt(curl, CURLOPT_POSTFIELDS, content);
  
  // TIMEOUT SETTINGS
  curl_easy_setopt(curl, CURLOPT_TIMEOUT, 10);        
  curl_easy_setopt(curl, CURLOPT_CONNECTTIMEOUT, 5);  
  curl_easy_setopt(curl, CURLOPT_LOW_SPEED_LIMIT, 1); 
  curl_easy_setopt(curl, CURLOPT_LOW_SPEED_TIME, 5);  
  curl_easy_setopt(curl, CURLOPT_NOSIGNAL, 1L); 

  CURLcode res = curl_easy_perform(curl);
  bool ok = (res == CURLE_OK);
  #ifdef DEBUG
  if (!ok) {
    // thread safety: use cerr instead of vlogf
    std::cerr << "curl_easy_perform() failed: " << curl_easy_strerror(res) << std::endl;
  } 
  #endif

  // this here is for simulating really bad latency
  // std::this_thread::sleep_for(std::chrono::seconds(15));

  curl_slist_free_all(headers);
  curl_easy_cleanup(curl);
  return ok;
}

void Discord::sendMessageAsync(sstring channel, sstring msg) {
  if (channel.empty()) {
    // no channel configuration, so bail
    return;
  }

  vlogf(LOG_MISC, format("Discord webhooks: add to queue: '%s'") % msg);

  std::lock_guard<std::mutex> lock(queue_mutex);
  message_queue.push({channel, msg});
  // vlogf(LOG_MISC, "Discord webhooks: added to queue");
  cv.notify_one();
}

void Discord::messenger() {
  
  std::unique_lock<std::mutex> lock(queue_mutex);
  
  pid_t parent_pid = getppid();
  
  do {
    
    auto timeout = std::chrono::seconds(5);
    
    bool has_messages = cv.wait_for(lock, timeout, [] { 
      return !message_queue.empty() || stop_thread; 
    });
    

    // Check if parent process still exists
    if (!has_messages && !stop_thread) {
      pid_t current_ppid = getppid();

      
      if (current_ppid != parent_pid) {
        break;
      }
    }

    while (!message_queue.empty()) {
      std::pair<sstring, sstring> message = message_queue.front();
      message_queue.pop();

      lock.unlock();
      sendMessage(message.first, message.second);
      lock.lock();
    }
    
  } while (!stop_thread);
}

bool Discord::doCleanup() {
  vlogf(LOG_MISC, "Discord webhooks: cleanup starting");
  
  // Signal the worker thread to stop
  {
    std::lock_guard<std::mutex> lock(queue_mutex);
    cv.notify_one();
    stop_thread = true;
  }
  
  if (messenger_thread.joinable()) {
    // Give the thread a reasonable time to finish naturally
    // (the shutdown message will never get a chance to send if we don't do this)
    std::this_thread::sleep_for(std::chrono::seconds(2));
  }

  // Don't call detach() or join() - just leave it to die
  vlogf(LOG_MISC, "Discord webhooks: cleanup finished.");
  
  return true;
}
