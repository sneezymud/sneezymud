//////////////////////////////////////////////////////////////////////////
//
// SneezyMUD - All rights reserved, SneezyMUD Coding Team
//
//////////////////////////////////////////////////////////////////////////


#ifndef __SOCKET_H
#define __SOCKET_H

extern int maxdesc, avail_descs;

const int OPT_USEC = 100000; 

extern void close_sockets(int);
extern int init_socket(int);
extern char hostlist[MAX_BAN_HOSTS][40];

class TMainSocket {
 public:
  int m_sock;
  int m_port;

  int gameLoop();
  struct timeval handleTimeAndSockets();
  void addNewDescriptorsDuringBoot(sstring);
  void initSocket();
  bool handleShutdown();
  void closeAllSockets();
  TSocket *newConnection();
  int newDescriptor();

  TMainSocket(int p);
  ~TMainSocket();

  private:
    TMainSocket() {}  // prevent default ctor being used

};

class TSocket {
 public:
  int m_sock;
    
  int writeToSocket(const char *);
  void nonBlock();

  TSocket();
  ~TSocket();
};

extern TMainSocket *gSocket;

#endif
