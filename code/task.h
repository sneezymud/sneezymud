//////////////////////////////////////////////////////////////////////////
//
// SneezyMUD - All rights reserved, SneezyMUD Coding Team
//
// $Log: task.h,v $
// Revision 5.5  2002/01/16 05:40:29  peel
// added plants
//
// Revision 5.4  2001/12/15 04:26:52  jesus
// added in butcher skill under adventuring disc
// will add more descs for races as time goes on 30 of about 125 races supported
//
// Revision 5.3  2001/07/05 21:25:54  peel
// Trying to fix cvs
// what a headache
//
// Revision 5.2  2001/04/26 22:23:57  peel
// *** empty log message ***
//
// Revision 5.1.1.4  2001/04/17 03:32:55  peel
// added TASK_FISHING
//
// Revision 5.1.1.3  2001/04/14 07:15:24  jesus
// *** empty log message ***
//
// Revision 5.1.1.2  2000/12/27 08:27:35  dash
// added code for 'defensive trance' warrior skill
//
// Revision 5.1.1.1  1999/10/16 04:32:20  batopr
// new branch
//
// Revision 5.1  1999/10/16 04:31:17  batopr
// new branch
//
// Revision 1.1  1999/09/12 17:24:04  sneezy
// Initial revision
//
//
//////////////////////////////////////////////////////////////////////////


/*************************************************************************

      SneezyMUD - All rights reserved, SneezyMUD Coding Team
      task.h : interface for mob/player tasks

  ----------------------------------------------------------------------

  Tasks provide a mechanism for delayed/sequenced/periodic mob actions.
  Basically, they function a lot like spec_procs.  They tie up the 
  player/mob for a set ammount of time, allowing things to happen in the
  meantime.

*************************************************************************/

#ifndef __TASK
#define __TASK

extern int task_bogus         (TBeing *, cmdTypeT, const char *, int, TRoom *, TObj *);
extern int task_sharpening    (TBeing *, cmdTypeT, const char *, int, TRoom *, TObj *);
extern int task_smythe        (TBeing *, cmdTypeT, const char *, int, TRoom *, TObj *);
extern int task_trap_door     (TBeing *, cmdTypeT, const char *, int, TRoom *, TObj *);
extern int task_get           (TBeing *, cmdTypeT, const char *, int, TRoom *, TObj *);
extern int task_spell_friends (TBeing *, cmdTypeT, const char *, int, TRoom *, TObj *);
extern int task_meditate      (TBeing *, cmdTypeT, const char *, int, TRoom *, TObj *);
extern int task_sit           (TBeing *, cmdTypeT, const char *, int, TRoom *, TObj *);
extern int task_rest          (TBeing *, cmdTypeT, const char *, int, TRoom *, TObj *);
extern int task_sleep         (TBeing *, cmdTypeT, const char *, int, TRoom *, TObj *);
extern int task_picklock      (TBeing *, cmdTypeT, const char *, int, TRoom *, TObj *);
extern int task_penance       (TBeing *, cmdTypeT, const char *, int, TRoom *, TObj *);
extern int task_brew          (TBeing *, cmdTypeT, const char *, int, TRoom *, TObj *);
extern int task_dulling       (TBeing *, cmdTypeT, const char *, int, TRoom *, TObj *);
extern int task_skinning      (TBeing *, cmdTypeT, const char *, int, TRoom *, TObj *);
extern int task_scribe        (TBeing *, cmdTypeT, const char *, int, TRoom *, TObj *);
extern int task_trap_container(TBeing *, cmdTypeT, const char *, int, TRoom *, TObj *);
extern int task_trap_mine     (TBeing *, cmdTypeT, const char *, int, TRoom *, TObj *);
extern int task_trap_grenade  (TBeing *, cmdTypeT, const char *, int, TRoom *, TObj *);
extern int task_yoginsa       (TBeing *, cmdTypeT, const char *, int, TRoom *, TObj *);
extern int task_attuning      (TBeing *, cmdTypeT, const char *, int, TRoom *, TObj *);
extern int task_tracking      (TBeing *, cmdTypeT, const char *, int, TRoom *, TObj *);
extern int task_seekwater     (TBeing *, cmdTypeT, const char *, int, TRoom *, TObj *);
extern int task_search        (TBeing *, cmdTypeT, const char *, int, TRoom *, TObj *);
extern int task_lightfire     (TBeing *, cmdTypeT, const char *, int, TRoom *, TObj *);
extern int task_plant     (TBeing *, cmdTypeT, const char *, int, TRoom *, TObj *);
extern int task_createEngine  (TBeing *, cmdTypeT, const char *, int, TRoom *, TObj *);
extern int task_charge        (TBeing *, cmdTypeT, const char *, int, TRoom *, TObj *);
extern int task_whittle       (TBeing *, cmdTypeT, const char *, int, TRoom *, TObj *);
extern int task_stavecharging (TBeing *, cmdTypeT, const char *, int, TRoom *, TObj *);
extern int task_trance_of_blades(TBeing *, cmdTypeT, const char *, int, TRoom *, TObj *);
extern int task_sacrifice      (TBeing *, cmdTypeT, const char *, int, TRoom *, TObj *);
extern int task_fishing     (TBeing *, cmdTypeT, const char *, int, TRoom *, TObj *);
extern int task_extinguish_my_ass (TBeing *, cmdTypeT, const char *, int, TRoom *, TObj *);
extern int task_butchering      (TBeing *, cmdTypeT, const char *, int, TRoom *, TObj *);

typedef struct _tasks_entry {
  const char * const name;
  const char * const you_are_busy_msg;
  int (*taskf)(TBeing *, cmdTypeT, const char *,int, TRoom *, TObj *);
} TaskEntry;

enum taskTypeT {
     TASK_BOGUS,
     TASK_SHARPEN,
     TASK_SMYTHE,
     TASK_TRAP_DOOR,
     TASK_GET_ALL,
     TASK_SPELL_FRIENDS,
     TASK_MEDITATE,
     TASK_SIT,
     TASK_REST,
     TASK_SLEEP,
     TASK_PICKLOCKS,
     TASK_PENANCE,
     TASK_BREWING,
     TASK_DULL,
     TASK_SKINNING,
     TASK_SCRIBING,
     TASK_TRAP_CONT,
     TASK_TRAP_MINE,
     TASK_TRAP_GRENADE,
     TASK_YOGINSA,
     TASK_ATTUNE,
     TASK_TRACKING,
     TASK_SEEKWATER,
     TASK_SEARCH,
     TASK_LIGHTFIRE,
     TASK_PLANT,
     TASK_CREATENGINE,
     TASK_MOUNTCHARGING,
     TASK_WHITTLE,
     TASK_STAVECHARGE,
     TASK_TRANCE_OF_BLADES,
     TASK_SACRIFICE,
     TASK_FISHING,
     TASK_EXTINGUISH_MY_ASS,
     TASK_BUTCHER,
     NUM_TASKS  // keep this as max
};

extern TaskEntry tasks[NUM_TASKS];

extern int  start_task(TBeing *, TThing *, TRoom *, taskTypeT,
                       const char *, int, unsigned short, ubyte, int, int);
extern void warn_busy(TBeing *ch);

#endif
