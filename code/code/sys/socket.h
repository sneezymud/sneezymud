//////////////////////////////////////////////////////////////////////////
//
// SneezyMUD - All rights reserved, SneezyMUD Coding Team
//
//////////////////////////////////////////////////////////////////////////

#pragma once

#include <sys/time.h>

#include "sstring.h"
#include "structs.h"

class TSocket;

extern int maxdesc, avail_descs;

const int OPT_USEC = 100000;

extern char hostlist[MAX_BAN_HOSTS][40];

class TMainSocket {
  private:
    int m_mainSockFD;

    struct timeval handleTimeAndSockets();
    bool handleShutdown();
    TSocket* newConnection(int);
    int newDescriptor(int);
    timeval sleeptime;

  public:
    friend class procHandleTimeAndSockets;
    friend class procIdle;

    void addNewDescriptorsDuringBoot(sstring);
    void closeAllSockets();
    void initSocket(int);
    int gameLoop();

    TMainSocket();
    ~TMainSocket();
};

class TSocket {
    friend class TMainSocket;

  private:
    void nonBlock();
    void setKeepalive(bool enabled);

  public:
    int m_sock;
    int port;
    int writeToSocket(const char*);
    int writeNull();

    TSocket();
    ~TSocket();
};

extern TMainSocket* gSocket;
