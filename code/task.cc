//////////////////////////////////////////////////////////////////////////
//
//      SneezyMUD - All rights reserved, SneezyMUD Coding Team
//      "task.cc" - All functions related to tasks that keep mobs/PCs busy
//
//////////////////////////////////////////////////////////////////////////

#if 0
    Batopr 8-10-96:
  to start a task, do:
  start_task(this, obj, rp, TASK_MEDITATE, arg, 0, in_room, 1, 0, 40);

  obj and rp can be NULL
  arg is probably ""
   
  timeLeft: (the first 0)
    - generally used as a counter of the number of interations of
      CMD_TASK_CONTINUE that have been encountered.
    - can be any (int) value desired 
 
  in_room: (sets ch->task->was_in_room)
    - task should check current room vs this.
      reason: start the task and let my group leader move me around mud.

  status (the 1)
    - a ubyte.
    - some tasks set this to one value in start_task and check in
      CMD_TASK_CONTINUE status.  If its not what it was initted to, do the
      continue code.  at the bottom of CONTINUE, set it to some new value.
      The reason for doing this is that the first call to CMD_TASK_CONTINUE is
      almost instantaneous with when it was started.
    - probably a better way to do this would be by initing timeLeft to 0
      in start_task and incrementing it 1 for each call to CONTINUE.  CONTINUE
      code would only execute if timeLeft > 0.  That would free the status byte
      for use any way we want.

  flags:
    - an int, use depends on the task.

  nextUpdate:
    - NOT WHAT IT SEEMS:  
      start_task() says ch->task->nextUpdate = nextUpdate.
    - calls to CMD_TASK_CONTINUE occur if pulse > ch->task->nextUpdate
    - notice that because pulse is typically big, calls to TASK CONTINUE are
      virtually instantaneous.
    - it is not until the first call to CMD_TASK_CONTINUE that we typically set
      ch->task->nextUpdate = pulse + xxxx
    - this leads to the status trick defined above.
    - SO BASICALLY, THIS VALUE IS WORTHLESS
#endif

#include "stdsneezy.h"

//FYI: CMD_TASK_CONTINUE is checked once per PULSE_MOBACT

taskData::taskData() :
  task(TASK_BOGUS),
  nextUpdate(0),
  timeLeft(0),
  orig_arg(NULL),
  wasInRoom(0),
  status(0),
  flags(0),
  obj(NULL),
  room(NULL)
{
}

taskData::taskData(const taskData &a) :
  task(a.task),
  nextUpdate(a.nextUpdate),
  timeLeft(a.timeLeft),
  wasInRoom(a.wasInRoom),
  status(a.status),
  flags(a.flags),
  obj(a.obj),
  room(a.room)
{
  orig_arg = mud_str_dup(a.orig_arg);
}

taskData & taskData::operator=(const taskData &a)
{
  if (this == &a) return *this;
  task = a.task;
  status = a.status;
  nextUpdate = a.nextUpdate;
  timeLeft = a.timeLeft;
  flags = a.flags;
  wasInRoom = a.wasInRoom;
  obj = a.obj;
  room = a.room;
  delete [] orig_arg;
  orig_arg = mud_str_dup(a.orig_arg);
  return *this;
}

taskData::~taskData()
{
  delete [] orig_arg;
  orig_arg = NULL;
}

void taskData::calcNextUpdate(int pulse, int interval) 
{
  nextUpdate = pulse + interval;
  nextUpdate %= 2400;
}

void TBeing::stopTask()
{
  if (!task)
    return;

  delete [] task->orig_arg;
  task->orig_arg = NULL;

  delete task;
  task = NULL;
}

int start_task(TBeing *ch, TThing *t, TRoom *rp, taskTypeT task, const char *arg, int timeLeft, ushort wasInRoom, ubyte status, int flags, int nextUpdate)
{
  if (!ch || (ch->task)) {
    vlogf(LOG_BUG, fmt("%s got to bad place in start_task (%d).  Tell Brutius or Batopr") % 
       (ch ? ch->getName() : "Unknown") % task);
    if (ch)
      ch->sendTo("Problem in task.  Bug Brutius.\n\r");
    return FALSE;
  }
  if (!(ch->task = new taskData)) {
    vlogf(LOG_BUG, fmt("Couldn't allocate memory in start_task for %s") %  ch->getName());
    return FALSE;
  }

  ch->task->orig_arg = mud_str_dup(arg);
  ch->task->obj = dynamic_cast<TObj *>(t);
  ch->task->room = rp;
  ch->task->task = task;
  ch->task->timeLeft = timeLeft;
  ch->task->wasInRoom = wasInRoom;
  ch->task->status = status;
  ch->task->flags = flags;
  ch->task->nextUpdate = nextUpdate;
  return TRUE;
}

void warn_busy(TBeing *ch)
{
  if (!ch || !(ch->task)) {
    vlogf(LOG_BUG, fmt("%s got to bad place in warn_busy.  Tell Brutius or Batopr") % 
       (ch ? ch->getName() : "Unknown"));
    return;
  }
  ch->sendTo(tasks[ch->task->task].you_are_busy_msg);
  ch->sendTo("Type 'abort' or 'stop' to quit what you are doing.\n\r");
}

int task_bogus(TBeing *ch, cmdTypeT, const char *, int , TRoom *, TObj *)
{
  ch->sendTo("Um... you hit a buggy spot in the code.  Tell an immort or something.\n\r");
  vlogf(LOG_BUG, fmt("%s was busy doing a buggy task!  Yikes!") %  ch->getName());
  ch->stopTask();

  return FALSE;
}

// first argument, the task name, should be a verb for the "look" commands 
// display and the stat command 
TaskEntry tasks[NUM_TASKS] =
{
  {"performing a bogus task", "You are busy doing nothing.\n\r", task_bogus},
  {"sharpening a weapon", "You are too busy sharpening.\n\r", task_sharpening},
  {"blacksmithing", "You are too busy blacksmithing.\n\r", task_blacksmithing},
  {"fixing something", "You are too busy fixing something.\n\r", task_repair_dead},
  {"regrowing something", "You are too busy regrowing something.\n\r", task_repair_organic},
  {"regrowing something", "You are too busy regrowing something.\n\r", task_repair_wood},
  {"fixing something", "You are too busy fixing something.\n\r", task_repair_magical},
  {"fixing something", "You are too busy fixing something.\n\r", task_repair_rock},
  {"tinkering with something", "You are too busy tinkering with something.\n\r", task_blacksmithing_advanced},
  {"mending", "You are too busy mending.\n\r", task_mend_hide},
  {"mending", "You are too busy mending.\n\r", task_mend},
  {"fixing something", "You are too busy fixing something.\n\r", task_repair_spiritual},
  {"setting a trap", "You are too busy setting your trap.\n\r", task_trap_door},
  {"taking items", "You are too busy taking items from the room.\n\r", task_get},
  {"casting a spell", "You are too busy casting your spell.\n\r", task_spell_friends},
  {"meditating", "You are too busy meditating.\n\r", task_meditate},
  {"sitting","You rather like sitting.\n\r",task_sit},
  {"resting","You're far too laid back at the moment.\n\r",task_rest},
  {"sleeping","You're a wee bit too unconscious to try that.\n\r",task_sleep},
  {"picking a lock", "You don't feel like giving up on this lock just yet.\n\r",task_picklock},
  {"repenting", "You are too busy repenting.\n\r", task_penance},
  {"brewing", "You are brewing and must concentrate!\n\r", task_brew},
  {"smoothing", "You are too busy smoothing.\n\r", task_dulling},
  {"skinning", "You are too busy skinning.\n\r", task_skinning},
  {"scribing", "You are scribing and must concentrate!\n\r", task_scribe},
  {"setting a trap", "You are too busy setting your trap.\n\r", task_trap_container},
  {"setting a trap", "You are too busy setting your trap.\n\r", task_trap_mine},
  {"setting a trap", "You are too busy setting your trap.\n\r", task_trap_grenade},
  {"meditating", "You are too busy meditating.\n\r", task_yoginsa},
  {"attuning a symbol", "You are too busy attuning.\n\r", task_attuning},
  {"tracking", "You are too busy tracking.\n\r", task_tracking},
  // Until the seekwater task is stripped from the tracking task, need to make
  // the message comply.
  //  {"tracking down someone", "You are too busy tracking.\n\r", task_tracking},
  {"searching for water", "You are too busy searching for water.\n\r", task_seekwater},
  {"searching for secret exits", "You are too busy searching for secret exits.\n\r", task_search},
  {"starting a fire", "You are too busy trying to start a fire.\n\r", task_lightfire},
  {"planting seeds", "You are too busy planting some seeds.\n\r", task_plant},
  {"creating something", "You are too busy trying to create something.\n\r", task_createEngine},
  {"charging", "You are too busy barreling down on someone.\n\r", task_charge},
  {"whittling", "You are too busy using your whittle skills.\n\r", task_whittle},
  {"stave charging", "You are too busy charging a stave.\n\r", task_stavecharging},
  {"in a defensive trance", "Not while you're in a defensive trance!\n\r", task_trance_of_blades},
  {"sacrificing", "Not while you are performing the sacrificial ritual of life!\n\r", task_sacrifice},
  {"fishing", "You are too busy fishing.\n\r", task_fishing},
  {"logging", "You are too busy logging.\n\r", task_logging},
  {"extinguishing", "You are too busy putting a fire out.\n\r", task_extinguish_my_ass},
  {"butchering", "You are too busy butchering a corpse.\n\r", task_butchering},
  {"cooking", "You are too busy cooking.\n\r", task_cook},
  {"loading a handgonne", "You are too busy loading your handgonne.\n\r", task_handgonne_load},
  {"loading a cannon", "You are too busy loading your cannon.\n\r", task_handgonne_load},
  {"trapping an arrow", "You are too busy trapping your arrow.\n\r", task_trap_arrow},
};

bool TBeing::nobrainerTaskCommand(cmdTypeT cmd)
{
  switch (cmd) {
    case CMD_SAY:
    case CMD_SAY2:
    case CMD_GLANCE:
    case CMD_TELL:
    case CMD_SHOUT:
    case CMD_WEATHER:
    case CMD_INVENTORY:
    case CMD_EQUIPMENT:
    case CMD_SMILE:
    case CMD_SHAKE:
    case CMD_NOD:
    case CMD_GT:
    case CMD_WIZNET:
    case CMD_REPLY:
      return TRUE;
    default:
      return FALSE;
  }
}

bool TBeing::utilityTaskCommand(cmdTypeT cmd)
{
  switch (cmd) {
    case CMD_EQUIPMENT:
    case CMD_WIZLIST:
    case CMD_LOOK:
    case CMD_LIMBS:  // not realistic, but let's just be nice
    case CMD_SPELLS:
    case CMD_RITUALS:
    case CMD_GLANCE:
    case CMD_TIME:
    case CMD_SCORE:
    case CMD_TROPHY:
    case CMD_HELP:
    case CMD_ZONES:
    case CMD_WHO:
    case CMD_NEWS:
    case CMD_CREDITS:
    case CMD_WIZNEWS:
    case CMD_SAVE:
    case CMD_IDEA:
    case CMD_TYPO:
    case CMD_BUG:
    case CMD_LEVELS:
    case CMD_ATTRIBUTE:
    case CMD_WORLD:
    case CMD_CLS:
    case CMD_PROMPT:
    case CMD_ALIAS:
    case CMD_CLEAR:
    case CMD_HISTORY:
    case CMD_COLOR:
    case CMD_MOTD:
    case CMD_TITLE:
    case CMD_PRACTICE:
    case CMD_NOSHOUT:
    case CMD_DESCRIPTION:
    case CMD_LIST:  // for list faction
    case CMD_ATTACK:
    case CMD_GROUP:
    case CMD_AFK:
    case CMD_WEATHER:
    case CMD_TOGGLE:
      return TRUE;
    default:
      return FALSE;
  }
}





