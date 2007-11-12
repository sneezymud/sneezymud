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

class TPulseList {
public:  
  int pulse;
  bool teleport, combat, drowning, special_procs, update_stuff;
  bool pulse_mudhour, mobstuff, pulse_tick, wayslowpulse;

  sstring showPulses(){
    sstring buf;

    buf += (teleport?"teleport ":"");
    buf += (combat?"combat ":"");
    buf += (drowning?"drowning ":"");
    buf += (special_procs?"special_procs ":"");
    buf += (update_stuff?"update_stuff ":"");
    buf += (pulse_mudhour?"pulse_mudhour ":"");
    buf += (mobstuff?"mobstuff ":"");
    buf += (pulse_tick?"pulse_tick ":"");
    buf += (wayslowpulse?"wayslowpulse ":"");

    return buf;
  }

  void init(int pulse){
    this->pulse=pulse;
    teleport = !(pulse % PULSE_TELEPORT);
    combat = !(pulse % PULSE_COMBAT);
    drowning = !(pulse % PULSE_DROWNING);
    special_procs = !(pulse % PULSE_SPEC_PROCS);
    update_stuff = !(pulse % PULSE_NOISES);
    pulse_mudhour = !(pulse % PULSE_MUDHOUR);
    mobstuff = !(pulse % PULSE_MOBACT);
    pulse_tick = !(pulse % PULSE_UPDATE);
    wayslowpulse = !(pulse % 2400);
  }

  TPulseList & operator=(const TPulseList &a){
    if (this == &a) return *this;    
    pulse=a.pulse;
    teleport=a.teleport;
    combat=a.combat;
    drowning=a.drowning;
    special_procs=a.special_procs;
    update_stuff=a.update_stuff;
    pulse_mudhour=a.pulse_mudhour;
    mobstuff=a.mobstuff;
    pulse_tick=a.pulse_tick;
    wayslowpulse=a.wayslowpulse;
    return *this;
  }
};


class TMainSocket {
 private:
  vector <int> m_sock;
  TBeing *tmp_ch;
  TObj *placeholder;
  TObjIter iter;
  int vehiclepulse;

  struct timeval handleTimeAndSockets();
  bool handleShutdown();
  TSocket *newConnection(int);
  int newDescriptor(int);

 public:
  void addNewDescriptorsDuringBoot(sstring);
  void closeAllSockets();
  void initSocket(int);
  int gameLoop();
  int objectPulse(TPulseList &, int);
  int characterPulse(TPulseList &, int);
  void dequeueBeing(TBeing* being);

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
