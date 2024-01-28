#pragma once

#include "sstring.h"
#include <queue>
#include <thread>
#include <mutex>
#include <utility>

class Discord {
  private:
    Discord();
    static bool sendMessageAsync(sstring channel, sstring msg);
    static void messenger();
    static std::thread messenger_thread;
    static std::mutex queue_mutex;
    static bool stop_thread;
    static std::queue<std::pair<sstring, sstring>> message_queue;

  public:
    static sstring CHANNEL_DEATHS;
    static sstring CHANNEL_SYS;
    static sstring CHANNEL_ACHIEVEMENT;

    static int ACHIEVEMENT_THRESHOLD;

    static bool doConfig();
    static bool doCleanup();
    static void sendMessage(sstring channel, sstring msg);
};