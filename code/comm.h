//////////////////////////////////////////////////////////////////////////
//
// SneezyMUD - All rights reserved, SneezyMUD Coding Team
//
// $Log: comm.h,v $
// Revision 1.1  1999/09/12 17:24:04  sneezy
// Initial revision
//
//
//////////////////////////////////////////////////////////////////////////


#ifndef __COMM_H
#define __COMM_H

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
void perform_complex(TBeing *, TBeing *, TObj *, TObj *, char *, byte, int);
void sendrpf(int, colorTypeT, TRoom *, const char *,...);
void sendrpf(int, TRoom *, const char *,...);
void sendrpf(colorTypeT, TRoom *, const char *,...);
void sendrpf(TRoom *, const char *,...);
void sendToOutdoor(colorTypeT, const char *, const char *);
void colorAct(colorTypeT, const char *, bool, const TThing *, const TThing *, const TThing *, actToParmT, const char * color = NULL, int = 0);
void act(const char *, bool, const TThing *, const TThing *, const TThing *, actToParmT, const char * color = NULL, int = 0);
void nukeMobsInZone(int);
bool isEmpty(int);

const int PULSE_COMMAND     =0;
const int PULSE_TICK        =1;

#if defined(SLOW)
// this is for slow running machines
const int PULSE_MOBACT      =6;
const int PULSE_TELEPORT    =6;
const int PULSE_COMBAT      =6;
const int PULSE_DROWNING    =12;
const int PULSE_SPEC_PROCS  =18;
const int PULSE_NOISES      =24;
const int PULSE_UPDATES     =75;
const int PULSE_TICKS       =150;
const int ONE_SECOND        =2;

#elif defined(FAST)
// These things should be changed if machine changes to make ticks
// as even as possible. They have been tweaked to make the current Linux
// machine work correctly. They were 2* normal mode before tweaking
// desired goal is combat round <= 3 secs
// I cut these by 25% for v5.1 - Bat
// - To test real time to pulse conversion, look at bottom of gameLoop
// - 9600 pulses take 1237 secs : Bat 5/5/99  (low load)
const int PULSE_MOBACT      =18;  // ONE_SEC * 2.5
const int PULSE_TELEPORT    =18;  // ONE_SEC * 2.5
const int PULSE_COMBAT      =18;  // ONE_SEC * 2.5
const int PULSE_DROWNING    =35;  // ONE_SEC * 5
const int PULSE_SPEC_PROCS  =53;  // ONE_SEC * 7.5
const int PULSE_NOISES      =105;  // ONE_SEC * 15
const int PULSE_UPDATES     =280;  // ONE_SEC * 40
const int PULSE_TICKS       =560;  // PULSE_UPDATE * 2
const int ONE_SECOND        =7;

#else
// this is normal mode
const int PULSE_MOBACT      =12;
const int PULSE_TELEPORT    =12;
const int PULSE_COMBAT      =12;
const int PULSE_DROWNING    =24;
const int PULSE_SPEC_PROCS  =36;
const int PULSE_NOISES      =48;
const int PULSE_UPDATES     =150;
const int PULSE_TICKS       =300;
const int ONE_SECOND        =4;

#endif

// updateAffects() called on combat counter (socket.cc)
// = 25 for all speeds
const int UPDATES_PER_TICK    = (PULSE_TICKS/PULSE_COMBAT);

extern const char * const prompt_mesg[];
extern void signalSetup(void);
extern int noSpecials;

#endif
