//////////////////////////////////////////////////////////////////////////
//
// SneezyMUD - All rights reserved, SneezyMUD Coding Team
//
//////////////////////////////////////////////////////////////////////////


#ifndef __COMM_H
#define __COMM_H

#include <boost/shared_ptr.hpp>

#include "ansi.h"
#include "sstring.h"
#include "enum.h"
#include "log.h"

class TThing;
class TBeing;
class TRoom;
class TObj;
class sstring;

enum actToParmT {
     TO_ROOM,
     TO_VICT,
     TO_NOTVICT,
     TO_CHAR
};

void sendToAll(char *messg);
void sendToExcept(char *messg, TBeing *ch);
void sendToRoom(colorTypeT, const char *messg, int room);
void sendToRoom(const char *messg, int room);
void sendToRoomExcept(char *messg, int room, TBeing *ch);
void sendToRoomExceptTwo(char *, int, TBeing *, TBeing *);
void perform_to_all(char *messg, TBeing *ch);
void perform_complex(TBeing *, TBeing *, TObj *, TObj *, char *, char, int);
void sendrpf(int, colorTypeT, TRoom *, const char *,...);
void sendrpf(int, TRoom *, const char *,...);
void sendrpf(colorTypeT, TRoom *, const char *,...);
void sendrpf(TRoom *, const char *,...);
void sendToOutdoor(colorTypeT, const sstring &, const sstring &);
void colorAct(colorTypeT, const sstring &, bool, const TThing *, const TThing *, const TThing *, actToParmT, const char * color = NULL, int = 0);
void act(const sstring &, bool, const TThing *, const TThing *, const TThing *, actToParmT, const char * color = NULL, int = 0);
void nukeMobs(int);
bool isEmpty(int);


// see comm.cc
class Pulse {
 public:
  static const int COMMAND;
  static const int TICK;
  static const int ONE_SECOND;
  static const int EVERY;
  static const int MOBACT;
  static const int TELEPORT;
  static const int COMBAT;
  static const int DROWNING;
  static const int SPEC_PROCS;
  static const int NOISES;
  static const int WAYSLOW;
  static const int REALHOUR;
  static const int REALDAY;
  static const int UPDATE;
  static const int MUDHOUR;
  static const int MUDDAY;
  static const int SECS_PER_UPDATE;
  static const int SECS_PER_MUDHOUR;
  static const int SECS_PER_MUD_DAY;
  static const int SECS_PER_MUD_MONTH;
  static const int SECS_PER_MUD_YEAR;
  static const int UPDATES_PER_MUDHOUR;
};

extern const char * const prompt_mesg[];
extern void signalSetup();

////


// this is the (abstract) base class for output messages
class Comm
{
 public:
  sstring getComm();

  virtual ~Comm(){}

  sstring text;

 protected:

 private:
  virtual sstring getText() = 0;
};

typedef boost::shared_ptr<Comm> CommPtr;


class UncategorizedComm : public Comm {
 public:
  UncategorizedComm(const sstring &);

 private:
  virtual sstring getText();
};

class SoundComm : public Comm {
 public:
  SoundComm(const sstring &, const sstring &, const sstring &, const sstring &, int, int, int, int);

 private:
  sstring soundtype;
  int volume;
  int priority;
  int cont;
  int repeats;
  sstring type;
  sstring url;

  virtual sstring getText();
};


class WhoListComm : public Comm {
 public:
  WhoListComm(const sstring &, bool, int, int, bool, const sstring &, const sstring &);
  sstring who;
  bool online;
  int level;
  int idle;
  bool linkdead;
  sstring prof;
  sstring title;

 private:
  virtual sstring getText();
};

class TellFromComm : public Comm {
 public:
  TellFromComm(const sstring &, const sstring &, const sstring &, bool, bool);

 private:
  sstring to;
  sstring from;
  bool drunk;
  bool mob;

  virtual sstring getText();
};

class TellToComm : public Comm {
 public:
  TellToComm(const sstring &, const sstring &, const sstring &);

 private:
  sstring to;
  sstring from;

  virtual sstring getText();
};

class CmdMsgComm : public Comm {
 public:
  CmdMsgComm(const sstring &, const sstring &);

 private:
  sstring cmd;

  virtual sstring getText();
};


// for snoop output
class SnoopComm : public Comm {
 public:
  SnoopComm(const sstring &, const sstring &);

 private:
  sstring vict;

  virtual sstring getText();
};

// for vlogf output
class SystemLogComm : public Comm {
 public:
  SystemLogComm(time_t, logTypeT, const sstring &);

 private:
  time_t logtime;
  logTypeT logtype;

  virtual sstring getText();
};

class LoginComm : public Comm {
 public:
  LoginComm(const sstring &, const sstring &);

 private:
  sstring prompt;

  virtual sstring getText();
};

struct RoomExitData {
  bool exit;
  bool door;
  bool open;
};

class RoomExitComm : public Comm {
 public:
  RoomExitComm();
  RoomExitData exits[MAX_DIR];
  
 private:
  virtual sstring getText();
};

class PromptComm : public Comm {
 public:
  PromptComm(time_t, int, int, float, int, int, int, int, const sstring &);
  time_t time;
  int hp;
  int mana;
  float piety;
  int lifeforce;
  int moves;
  int money;
  int room;

 private:
  virtual sstring getText();
};



#endif
