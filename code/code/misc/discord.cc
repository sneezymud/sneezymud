#include "discord.h"

#include <curl/curl.h>
#include <boost/program_options/options_description.hpp>
#include <boost/program_options/parsers.hpp>
#include <boost/program_options/value_semantic.hpp>
#include <boost/program_options/variables_map.hpp>
#include <chrono>
#include <filesystem>
#include <fstream>
#include <iosfwd>
#include <ostream>
#include <string>
#include <string_view>
#include <thread>

#include "colorstring.h"
#include "log.h"
#include "sstring.h"

namespace po = boost::program_options;

// available discord channels
// channels are configured in lib/discord.cfg
// new channels must be added in that file, here, in discord.h, and in
// po::options_descriptions below
sstring Discord::CHANNEL_DEATHS;
sstring Discord::CHANNEL_SYS;
sstring Discord::CHANNEL_ACHIEVEMENT;
sstring Discord::CHANNEL_CRASH_LOGS;

// threshold level for discord mob kill notifications
int Discord::ACHIEVEMENT_THRESHOLD;

// read the configuration
bool Discord::doConfig() {
  static constexpr const char* configFile = "discord.cfg";

  po::options_description config_options("Configuration File Only");

  // clang-format off
  config_options.add_options()
    ("deaths_webhook", po::value<std::string>(&CHANNEL_DEATHS)->default_value(""), "see discord.h")
    ("sys_webhook", po::value<std::string>(&CHANNEL_SYS)->default_value(""), "see discord.h")
    ("achieve_webhook", po::value<std::string>(&CHANNEL_ACHIEVEMENT)->default_value(""), "see discord.h")
    ("achieve_threshold", po::value<int>(&ACHIEVEMENT_THRESHOLD)->default_value(80), "see discord.h")
    ("crash_logs_webhook", po::value<std::string>(&CHANNEL_CRASH_LOGS)->default_value(""), "see discord.h");
  // clang-format on

  po::variables_map vm;
  po::notify(vm);
  std::ifstream ifs(configFile);

  const std::string configPath = std::filesystem::current_path() / configFile;

  if (!ifs.is_open()) {
    vlogf(LOG_FILE,
      format("Discord webhooks: Failed to open config file \"%s\". "
             "Discord integration disabled.") %
        configPath);
    return false;
  }

  vlogf(LOG_FILE,
    format(
      "Discord webhooks: Using config file %s. Discord integration enabled.") %
      configPath);

  po::store(parse_config_file(ifs, config_options), vm);
  po::notify(vm);

  return true;
}

// Synchronously send a message to a discord webhook
// we use the curl library for this and keep it simple
bool Discord::sendMessage(const sstring& channel, const sstring& msg) {
  if (channel.empty())
    return false;

  CURL* curl = curl_easy_init();
  if (!curl) {
    vlogf(LOG_MISC, "Discord webhooks: curl_easy_init() failed");
    return false;
  }

  const sstring formatted_msg =
    format(R"({"content": "%s"})") % stripColorCodes(msg).escapeJson();

  const char* webhookURL = channel.c_str();
  const char* content = formatted_msg.c_str();

  curl_slist* headers = nullptr;
  headers = curl_slist_append(headers, "Content-Type: application/json");

  curl_easy_setopt(curl, CURLOPT_URL, webhookURL);
  curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
  curl_easy_setopt(curl, CURLOPT_POSTFIELDS, content);

  CURLcode res = curl_easy_perform(curl);
  if (res != CURLE_OK) {
    vlogf(LOG_MISC,
      format("Discord webhooks: curl_easy_perform() failed: '%s'") %
        curl_easy_strerror(res));
    curl_slist_free_all(headers);
    curl_easy_cleanup(curl);
    return false;
  }

  curl_slist_free_all(headers);
  curl_easy_cleanup(curl);
  return true;
}

// Send Discord messages asynchronously by default so the mud doesn't hang until
// the POST request resolves. If the caller wants to block the main thread
// until resolution for some reason they can do so by calling sendMessage
// directly.
void Discord::sendMessageAsync(const sstring& channel, const sstring& msg) {
  if (channel.empty())
    return;

  vlogf(LOG_MISC, format("Discord webhooks: '%s'") % msg);
  std::thread(sendMessage, channel, msg).detach();
}

// Post a file to a discord channel via webhook, to allow messages longer than
// the character limit to be posted all at once (such as crash logs)
bool Discord::sendFile(const sstring& channel, const sstring& filePath) {
  if (channel.empty())
    return false;

  CURL* curl = curl_easy_init();

  if (!curl) {
    vlogf(LOG_MISC, "Discord webhooks: curl_easy_init() failed");
    return false;
  }

  const char* const webhookURL = channel.c_str();
  curl_easy_setopt(curl, CURLOPT_URL, webhookURL);

  struct curl_httppost* formpost = nullptr;
  struct curl_httppost* lastptr = nullptr;
  curl_formadd(&formpost, &lastptr, CURLFORM_COPYNAME, "file", CURLFORM_FILE,
    filePath.c_str(), CURLFORM_END);

  curl_easy_setopt(curl, CURLOPT_HTTPPOST, formpost);

  CURLcode res = curl_easy_perform(curl);

  if (res != CURLE_OK) {
    vlogf(LOG_MISC,
      format("curl_easy_perform() failed: %s") % curl_easy_strerror(res));
    curl_easy_cleanup(curl);
    curl_formfree(formpost);
    return false;
  }

  curl_easy_cleanup(curl);
  curl_formfree(formpost);
  return true;
}

void Discord::sendFileAsync(const sstring& channel, const sstring& filePath) {
  if (channel.empty())
    return;

  std::thread(sendFile, channel, filePath).detach();
}

namespace {
  constexpr std::string_view crashLogsDir = "./mutable/crash_logs";
  constexpr std::string_view lastLogPostedPath =
    "./mutable/crash_logs/last_asan_log_posted";

  std::string readFileContents(const std::filesystem::path& path) {
    std::ifstream logFile;

    try {
      logFile.open(path);
    } catch (...) {
      return {};
    }

    std::string logContents((std::istreambuf_iterator<char>(logFile)), {});
    return logContents;
  }

  std::filesystem::path findNewestCrashLog() {
    std::filesystem::path newestLogPath;
    auto latestTime = std::filesystem::file_time_type::min();

    std::filesystem::create_directories(crashLogsDir);

    for (const auto& file : std::filesystem::directory_iterator(crashLogsDir)) {
      if (file.path().filename().string().find("asan_log.") == 0) {
        std::string logContents = readFileContents(file);

        // Delete LeakSanitizer logs, as they're created on every graceful
        // shutdown and we don't want to spam Discord with them. However we
        // don't want to totally disable them via ASAN_OPTIONS=detect_leaks=0
        // so we're aware of the leaks.
        if (logContents.empty() ||
            logContents.find("LeakSanitizer") != std::string::npos) {
          std::filesystem::remove(file.path());
          continue;
        }

        auto time = std::filesystem::last_write_time(file);
        if (time > latestTime) {
          latestTime = time;
          newestLogPath = file;
        }
      }
    }

    return newestLogPath;
  }

  std::string getLastLogPostedName() {
    std::ifstream lastLogPosted(lastLogPostedPath.data());
    std::string logName;
    std::getline(lastLogPosted, logName);
    return logName;
  }

  void writeLastCrashLogPosted(const std::filesystem::path& newestLogPath) {
    std::ofstream lastLogPosted(lastLogPostedPath.data());
    lastLogPosted.seekp(0);
    lastLogPosted.clear();
    lastLogPosted << newestLogPath.filename().string();
  }
}  // namespace

void Discord::maybePostNewestCrashLog() {
  if (CHANNEL_CRASH_LOGS.empty())
    return;

  const std::filesystem::path newestLogPath = findNewestCrashLog();
  const std::string lastLogPostedName = getLastLogPostedName();

  if (newestLogPath.empty() ||
      newestLogPath.filename().string() == lastLogPostedName) {
    return;
  }

  // Discord won't embed a file and display its contents unless it has a
  // supported extension.
  std::filesystem::path renamedLogPath = newestLogPath.string() + ".txt";
  std::filesystem::rename(newestLogPath, renamedLogPath);

  vlogf(LOG_MISC, format("New crash log '%s' found - posting to Discord.") %
                    renamedLogPath.filename().string());

  Discord::sendFileAsync(Discord::CHANNEL_CRASH_LOGS, renamedLogPath.string());

  writeLastCrashLogPosted(renamedLogPath);
}
