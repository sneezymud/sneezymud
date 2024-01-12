#include <iostream>
#include <fstream>
#include <filesystem>
#include <thread>

#include "discord.h"

#include "extern.h"
#include "colorstring.h"
#include "sstring.h"
#include <curl/curl.h>
#include <curl/easy.h>

#include <boost/program_options.hpp>
namespace po = boost::program_options;

// available discord channels
// channels are configured in lib/discord.cfg
// new channels must be added in that file, here, in discord.h, and in po::options_descriptions below
sstring Discord::CHANNEL_DEATHS;
sstring Discord::CHANNEL_SYS;
sstring Discord::CHANNEL_ACHIEVEMENT;

// threshold level for discord mob kill notifications
int Discord::ACHIEVEMENT_THRESHOLD;

// read the configuration
bool Discord::doConfig() {
    using std::string;
    string configFile = "discord.cfg";

    std::string empty_string = "";

    po::options_description configOnly("Configuration File Only");
    configOnly.add_options()
        ("deaths_webhook",po::value<string>(&CHANNEL_DEATHS)->default_value(empty_string),"see discord.h")
        ("sys_webhook",po::value<string>(&CHANNEL_SYS)->default_value(empty_string),"see discord.h")
        ("achieve_webhook",po::value<string>(&CHANNEL_ACHIEVEMENT)->default_value(empty_string),"see discord.h")
        ("achieve_threshold",po::value<int>(&ACHIEVEMENT_THRESHOLD)->default_value(80),"see discord.h");

    po::options_description config_options;
    config_options.add(configOnly);

    po::variables_map vm;
    po::notify(vm);
    std::ifstream ifs(configFile.c_str());

    if (!ifs.is_open()) {
        vlogf(LOG_MISC, format("Discord webhooks: Failed to open config file '%s'") % configFile);
        vlogf(LOG_MISC, format("Discord webhooks: ensure config is located in: '%s'") % std::filesystem::current_path());
        return false;
    }

    po::store(parse_config_file(ifs, config_options), vm);
    po::notify(vm);

    return true;
}


// send a message to a discord webhook
// we use the curl library for this and keep it simple
bool Discord::sendMessageAsync(sstring channel, sstring msg)
{

    CURL* curl = curl_easy_init();
    if (!curl) {
        vlogf(LOG_MISC, "Discord webhooks: curl_easy_init() failed");
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

    CURLcode res = curl_easy_perform(curl);
    if (res != CURLE_OK) {
        vlogf(LOG_MISC, format("Discord webhooks: curl_easy_perform() failed: '%s'") %  curl_easy_strerror(res));
    }

    curl_slist_free_all(headers);
    curl_easy_cleanup(curl);
    return true;
}

void Discord::sendMessage(sstring channel, sstring msg, bool detach) 
{
    if (channel == "") {
        // no channel configuration, so bail
        return;
    }

    vlogf(LOG_MISC, format("Discord webhooks: '%s'")  % msg);

    // we want to do this asynchronously so POST lag doesn't hang the mud
    std::thread thread(
        [](sstring channel, sstring msg) {
            sendMessageAsync(channel, msg);
        },
        channel, msg
    );
    if (!detach) {
        thread.join();
    } else {
        thread.detach();
    }
}