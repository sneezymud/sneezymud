#pragma once

class sstring;

class Discord {
  public:
    static sstring CHANNEL_DEATHS;
    static sstring CHANNEL_SYS;
    static sstring CHANNEL_ACHIEVEMENT;
    static sstring CHANNEL_CRASH_LOGS;
    static int ACHIEVEMENT_THRESHOLD;

    Discord() = delete;
    static bool doConfig();
    static void maybePostNewestCrashLog();
    static bool sendFile(const sstring& channel, const sstring& filePath);
    static void sendFileAsync(const sstring& channel, const sstring& filePath);
    static bool sendMessage(const sstring& channel, const sstring& msg);
    static void sendMessageAsync(const sstring& channel, const sstring& msg);
};
