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

class TSocket {
  public:
    int m_sock;
    int m_port;
    
  int gameLoop();
  int writeToSocket(const char *);
  void closeAllSockets();
  TSocket *newConnection();
  int newDescriptor();
  void nonBlock();
  void initSocket();
  void addNewDescriptorsDuringBoot(sstring);
  bool handleShutdown();


  TSocket(int p);
  ~TSocket();

  private:
    TSocket() {}  // prevent default ctor being used
};

extern TSocket *gSocket;

#endif
