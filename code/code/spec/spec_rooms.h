#ifndef __SPEC_ROOMS_H
#define __SPEC_ROOMS_H

extern int sleepTagControl(TBeing *, cmdTypeT, const char *, TRoom *);
extern int sleepTagRoom(TBeing *, cmdTypeT, const char *, TRoom *);
extern int bankRoom(TBeing *, cmdTypeT, const char *, TRoom *);

struct TRoomSpecs {
  bool assignable;
  const char *name;
  int (*proc) (TBeing *, cmdTypeT, const char *, TRoom *);
};

const int SPEC_ROOM_GH_DUMP=18;
const int NUM_ROOM_SPECIALS = 33;

extern const int GET_ROOM_SPE_INDEX(int d);

extern TRoomSpecs roomSpecials[NUM_ROOM_SPECIALS + 1];



/*************************************************************************

      SneezyMUD 3.0 - All rights reserved, SneezyMUD Coding Team
      spec_rooms.h : interface for calling room special procedures

  ----------------------------------------------------------------------

  Special procedures for rooms may be called under five different
  conditions:  by a player command, when the room is first allocated in
  memory, during the room's zone reset, when the room is deallocated 
  from memory, and periodically (every PULSE_MOBILE heartbeats -- at
  the time of writing, a little over 7 seconds).  A room's special
  procedure must return "int".  A return value of zero (FALSE) indicates
  that nothing exceptional happened.  A non-zero return value (TRUE) 
  indicates that some action needs to be taken (such as ignoring the
  players command).  When in doubt, return FALSE.  

  A rooms special procedure must be defined using the following
  parameter list: 
                   (Mob **ch, cmdTypeT cmd, char *arg, Room *rp)

  These parameters are defined as follows:
     ch - used when a mob/player triggers the room's special procedure.  
          Will contain a pointer to the triggering mob/player.
    cmd - what triggered the procedure to be called.  A number greater
          than zero indicates a player command.  A number less than
          zero indicates something like ROOM_RESET.
    arg - will be non-NULL only when catching a player command.
     rp - a pointer to the room for which the spec_proc was called.

*************************************************************************/

#endif
