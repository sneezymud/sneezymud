#pragma once

#include "sstring.h"

class Discord {
    private: 
        Discord();
        static bool sendMessageAsync(sstring channel, sstring msg);

    public:
        static sstring CHANNEL_DEATHS;
        static sstring CHANNEL_SYS;
        static sstring CHANNEL_ACHIEVEMENT;

        static int ACHIEVEMENT_THRESHOLD;

        static bool doConfig();
        static void sendMessage(sstring channel, sstring msg, bool detach = true);
};