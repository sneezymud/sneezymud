
#include "sstring.h"

class Discord {
    private: 
        Discord();

    public:
        static sstring CHANNEL_DEATHS;
        static sstring CHANNEL_SYS;
        static sstring CHANNEL_LEVELUP;

        static bool doConfig();
        static bool sendMessage(sstring channel, sstring msg);
};