//////////////////////////////////////////////////////////////////////////
//
// SneezyMUD - All rights reserved, SneezyMUD Coding Team
//
//////////////////////////////////////////////////////////////////////////


#ifndef __SOCKET_H
#define __SOCKET_H

extern int maxdesc, avail_descs;

const int OPT_USEC = 100000; 

extern char hostlist[MAX_BAN_HOSTS][40];

class TMainSocket {
 private:
  vector <int> m_sock;
  
  struct timeval handleTimeAndSockets();
  bool handleShutdown();
  TSocket *newConnection(int);
  int newDescriptor(int);

 public:
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
  int writeToSocket(const char *);

  TSocket();
  ~TSocket();
};

extern TMainSocket *gSocket;

#endif
