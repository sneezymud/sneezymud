//////////////////////////////////////////////////////////////////////////
//
// SneezyMUD - All rights reserved, SneezyMUD Coding Team
//
//////////////////////////////////////////////////////////////////////////


#ifndef __SOCKET_H
#define __SOCKET_H

#include "comm.h"
#include "structs.h"
#include "obj.h"
#include "process.h"

class TSocket;

extern int maxdesc, avail_descs;

const int OPT_USEC = 100000; 

extern char hostlist[MAX_BAN_HOSTS][40];

class TMainSocket {
 private:
  std::vector <int> m_sock;
  std::vector <int> m_port;

  struct timeval handleTimeAndSockets();
  bool handleShutdown();
  TSocket *newConnection(int, int);
  int newDescriptor(int, int);
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

 public:
  int m_sock;
  int port;
  int writeToSocket(const char *);
  int writeNull();

  TSocket();
  ~TSocket();
};

extern TMainSocket *gSocket;

#endif
