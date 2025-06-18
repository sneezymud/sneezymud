#pragma once

#include "sstring.h"

class Discord {
  public:
    // Discord Channels available to receive notifications. New channels must
    // be added in the `discord.cfg` file and in the `Channels` struct below,
    // then added as an option to the config parser in `doConfig()`. Default
    // values should just be an empty string, which is handled automatically
    // by member default initialization for sstrings.
    struct Channels {
        sstring deaths;
        sstring system;
        sstring achievements;
    };

    // Settings related to any Discord integration. New settings must be added
    // in the `discord.cfg` file and in the `Settings` struct below, then added
    // as an option to the config parser in `doConfig()`. Default values can be
    // handled in the struct definition via member initialization.
    struct Settings {
        // Level threshold for mob kill notifications
        int achievement_threshold{80};
    };

    Discord() = delete;

    static Discord::Channels channels;
    static Discord::Settings settings;

    static bool doConfig();
    static bool doCleanup();
    static void sendMessageAsync(const sstring& channel, const sstring& msg);
};
