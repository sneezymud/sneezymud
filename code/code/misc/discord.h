#pragma once

#include "sstring.h"
#include <queue>
#include <thread>
#include <mutex>
#include <utility>
#include <condition_variable>

class Discord {
  private:
    static bool sendMessage(sstring channel, sstring msg);
    static void messenger();
    static std::thread messenger_thread;
    static std::condition_variable cv;
    static std::mutex queue_mutex;
    static bool stop_thread;
    static std::queue<std::pair<sstring, sstring>> message_queue;

  public:
    Discord() = delete;

    static sstring CHANNEL_DEATHS;
    static sstring CHANNEL_SYS;
    static sstring CHANNEL_ACHIEVEMENT;

    static int ACHIEVEMENT_THRESHOLD;

    static bool doConfig();
    static bool doCleanup();
    static void sendMessageAsync(sstring channel, sstring msg);
};