//////////////////////////////////////////////////////////////////////////
//
// SneezyMUD - All rights reserved, SneezyMUD Coding Team
//
// $Log: utility.cc,v $
// Revision 1.3  1999/10/15 21:37:06  batopr
// Added crash protection sanity check to TObj::canGetMe
//
// Revision 1.2  1999/10/07 15:27:27  batopr
// Added case for GOLD_DUMP to addMoney
//
// Revision 1.1  1999/09/12 17:24:04  sneezy
// Initial revision
//
//
//////////////////////////////////////////////////////////////////////////


//
//      SneezyMUD - All rights reserved, SneezyMUD Coding Team
//      "utility.cc" - Various utility functions.
//
//      Last major revision : August 1996
//
//////////////////////////////////////////////////////////////////////////

#include "stdsneezy.h"

#include <csignal>
#include <cstdarg>

extern "C" {
#include <unistd.h>
#include <sys/socket.h>
#include <sys/wait.h>
#include <netinet/in.h>
#include <netdb.h>
#include <fcntl.h>
#include <arpa/telnet.h>
#include <sys/resource.h>
#include <sys/syscall.h>
#include <sys/param.h>
#include <cmath>
#include <dirent.h>
#include <sys/stat.h>                                                           
#include <sys/wait.h>                                                           

pid_t vfork(void);

#ifdef SOLARIS
extern long random(void);
#endif
}
#include "disease.h"
#include "shop.h"
#include "combat.h"
#include "account.h"
#include "statistics.h"
#include "socket.h"
#include "games.h"
#include "mail.h"

bool TBeing::canSeeWho(const TBeing *o) const
{
  if (inRoom() < 0 || o->inRoom() < 0)
    return FALSE;

  if (isImmortal()) {
    if (GetMaxLevel() < o->getInvisLevel())
      return FALSE;
    else
      return TRUE;
  }
  if ((GetMaxLevel() < o->getInvisLevel()) && (o->isImmortal()))
    return FALSE;   // invis gods 

  if (!isImmortal() && (o->getInvisLevel() >= GOD_LEVEL1))
    return FALSE;   // link deads 

  if (o->isAffected(AFF_INVISIBLE)) {
    if (o->isImmortal())
      return FALSE;
    if (!isAffected(AFF_DETECT_INVISIBLE))
      return FALSE;
  }
  if (isAffected(AFF_BLIND) && !isAffected(AFF_TRUE_SIGHT))
    return FALSE;

  return TRUE;
}

bool exit_ok(roomDirData *exit, TRoom **rpp)
{
  TRoom *rp;

  if (!rpp)
    rpp = &rp;

  if (!exit) {
    *rpp = NULL;
    return FALSE;
  }
  return ((*rpp = real_roomp(exit->to_room)) != NULL);
}

int TMonster::mobVnum() const
{
  if (number < 0)
    return number;

  return (mob_index[getMobIndex()].virt);
}

// creates a random number in interval [from;to] 
int number(int from, int to)
{
  if (to - from + 1) 
    return ((random() % (to - from + 1)) + from);
  else
    return (from);
}

// simulates dice roll 
int dice(int number, int size)
{
  int r;
  int sum = 0;

  if (size < 0) {
    vlogf(10, "Dice called with negative size!");
    return 0;
  }
  if (!size)
    return (0);

  for (r = 1; r <= number; r++) 
    sum += ((random() % size) + 1);
  
  return (sum);
}

bool scan_number(const char *text, int *rval)
{
  if (1 != sscanf(text, " %i ", rval))
    return 0;

  return 1;
}

// Calculate the REAL time passed over the last t2-t1 centuries (secs) 
void realTimePassed(time_t t2, time_t t1, struct time_info_data *now)
{
  long secs;

  secs = (long) (t2 - t1);

  now->minutes = (secs / SECS_PER_REAL_MIN) % 60;
  secs -= SECS_PER_REAL_MIN * now->minutes;

  now->hours = (secs / SECS_PER_REAL_HOUR) % 24;
  secs -= SECS_PER_REAL_HOUR * now->hours;

  now->day = (secs / SECS_PER_REAL_DAY);
  secs -= SECS_PER_REAL_DAY * now->day;

  now->month = -1;
  now->year = -1;
  now->seconds = secs;

  return;
}

// Calculate the MUD time passed over the last t2-t1 centuries (secs) 
void mudTimePassed(time_t t2, time_t t1, struct time_info_data *now)
{
  long secs;

  secs = (long) (t2 - t1);

  now->hours = (secs / SECS_PER_MUD_HOUR) % 48;	
  secs -= SECS_PER_MUD_HOUR * now->hours;

  now->day = (secs / SECS_PER_MUD_DAY) % 28;	
  secs -= SECS_PER_MUD_DAY * now->day;

  now->month = (secs / SECS_PER_MUD_MONTH) % 12;		
  secs -= SECS_PER_MUD_MONTH * now->month;

  now->year = (secs / SECS_PER_MUD_YEAR);	

  return;
}

time_info_data *TBeing::age() const
{
  static time_info_data player_age;

  mudTimePassed(time(0), player.time.birth, &player_age);

  player_age.year += getBaseAge();
  player_age.year += age_mod;

  return (&player_age);
}

bool TBeing::inGroup(const TBeing *tbt) const
{
  if (!tbt)
    return FALSE;

  if ((this == tbt) || 
      (tbt == dynamic_cast<const TBeing *>(riding)) || 
      (tbt == dynamic_cast<const TBeing *>(rider)))
    return TRUE;

  if (!isAffected(AFF_GROUP))
    return FALSE;

  TBeing *tbt2 = dynamic_cast<TBeing *>(tbt->rider);
  if (tbt2 && inGroup(tbt2))
    return TRUE;

  if (!tbt->isAffected(AFF_GROUP))
    return FALSE;

  if (!master && !tbt->master)
    return FALSE;

  if (this == tbt->master)
    return TRUE;

  if (master == tbt)
    return TRUE;

  if (master == tbt->master) {
    return TRUE;
  }

  return FALSE;
}

unsigned int TBeing::numberInGroupInRoom() const
{
  TThing *t;
  unsigned int count = 0;

  for (t = roomp->stuff; t; t = t->nextThing) {
    TBeing *tbt = dynamic_cast<TBeing *>(t);
    if (tbt && inGroup(tbt))
      count++;
  }
  return count;
}

bool getall(const char *name, char *newname)
{
  char arg[40], tmpname[80], otname[80];
  char prd;

  *arg = *tmpname = *otname = '\0';

  sscanf(name, "%s ", otname);	
  if (strlen(otname) < 5)
    return FALSE;

  sscanf(otname, "%3s%c%s", arg, &prd, tmpname);

  if (prd != '.')
    return FALSE;
  if (!tmpname)
    return FALSE;
  if (strcmp(arg, "all"))
    return FALSE;

  while (*name != '.')
    name++;

  name++;

  for (; (*newname = *name); name++, newname++);

  return TRUE;
}

int getabunch(const char *name, char *newname)
{
  int num = 0;
  char tmpname[80] = "\0";

  sscanf(name, "%d*%s", &num, tmpname);
  if (tmpname[0] == '\0')
    return FALSE;
  if (num < 1)
    return FALSE;
  if (num > 99)
    num = 99;

  while (*name != '*')
    name++;

  name++;

  for (; (*newname = *name); name++, newname++);

  return (num);
}

int TMonster::standUp()
{
  if (((getPosition() < POSITION_STANDING) && 
         (getPosition() > POSITION_STUNNED)) || 
          ((getPosition() > POSITION_STUNNED) && riding && 
                !dynamic_cast<TBeing *>(riding))) {
    if (isFourLegged()) {
      if (getHit() > (hitLimit() / 2))
        act("$n quickly rolls over and leaps to $s feet.", 
               TRUE, this, 0, 0, TO_ROOM);
      else if (getHit() > (hitLimit() / 6))
        act("$n slowly rolls over and leaps to $s feet.", 
               TRUE, this, 0, 0, TO_ROOM);
      else
        act("$n rolls over and gets to $s feet very slowly.", 
               TRUE, this, 0, 0, TO_ROOM);
    } else if (riding && !dynamic_cast<TBeing *> (riding)) {
      if (getHit() > (hitLimit() / 2))
        act("$n quickly rises off $p onto $s feet.", 1, this, riding, 0, TO_ROOM);
      else if (getHit() > (hitLimit() / 6))
        act("$n rolls off $p and gets on $s feet.", 1, this, riding, 0, TO_ROOM);
      else
        act("$n slowly rolls off $p and gets on $s feet.", 1, this, riding, 0, TO_ROOM);
    } else {
      if (getHit() > (hitLimit() / 2))
        act("$n quickly stands up.", 1, this, 0, 0, TO_ROOM);
      else if (getHit() > (hitLimit() / 6))
        act("$n slowly stands up.", 1, this, 0, 0, TO_ROOM);
      else
        act("$n gets to $s feet very slowly.", 1, this, 0, 0, TO_ROOM);
    }

    if (checkBlackjack())
      gBj.exitGame(this);
    if (gGin.check(this))
      gGin.exitGame(this);
    if (checkHearts())
      gHearts.exitGame(this);
    if (checkCrazyEights())
      gEights.exitGame(this);
    if (checkDrawPoker())
      gPoker.exitGame(this);

    if (riding) {
      dismount(POSITION_STANDING);
    }
    setPosition(POSITION_STANDING);
    addToWait(combatRound(1));
    return TRUE;
  }
  return FALSE;
}


#if 0
bool TThing::hasObject(int ob_num)
{
  int j, found;
  TThing *t;

  found = 0;

  for (j = MIN_WEAR; j < MAX_WEAR; j++)
    if (equipment[j])
      found += RecCompObjNum(equipment[j], ob_num);

  if (found > 0)
    return TRUE;

  for (t = stuff; t; t = t->nextThing)
    found += RecCompObjNum(t, ob_num);

  if (found > 0)
    return TRUE;
  else
    return FALSE;
}
#endif

int roomOfObject(const TThing *t)
{
  if (t->in_room != ROOM_NOWHERE)
    return t->in_room;
  else if (t->equippedBy)
    return t->equippedBy->in_room;
  else if (t->parent)
    return roomOfObject(t->parent);
  else if (t->riding)
    return roomOfObject(t->riding);
  else if (t->stuckIn)
    return t->stuckIn->in_room;
  else
    return ROOM_NOWHERE;
}

// searches recursively and finds character holding the thing
TThing *TThing::thingHolding() const
{
  if (in_room != ROOM_NOWHERE)
    return NULL;
  else if (equippedBy)
    return equippedBy;
  else if (stuckIn)
    return stuckIn;
  else if (parent && dynamic_cast<TBeing *>(parent))
    return parent;
  else if (parent)
    return parent->thingHolding();
  else if (riding)
    return riding->thingHolding();
  else
    return 0;
}

int RecCompObjNum(const TObj *o, int obj_num)
{
  int total = 0;
  TThing *i;

  if (obj_index[o->getItemIndex()].virt == obj_num)
    total = 1;

  for (i = o->stuff; i; i = i->nextThing) {
    TObj *to = dynamic_cast<TObj *>(i);
    if (to)
      total += RecCompObjNum(to, obj_num);
  }

  return (total);

}

// this is a silly noop function
// occasionally we need to apply flags or "fix" things, so I usually do them
// all here, for easy centralizing.
void BatoprsResetCharFlags(TBeing *ch)
{
  // strip off free_deaths from 5.0 for 5.1
  affectedData *aff, *an;
  for (aff = ch->affected; aff; aff = an) {
    an = aff->next;
    if (aff->type != AFFECT_FREE_DEATHS)
      continue;
    if (aff->level <= 3) {
      ch->affectRemove(aff);
    }
  }

  // insure multiclassing flags set up ok, this can go away eventually
  if (ch->GetMaxLevel() >= MAX_MORT && ch->desc) {
    SET_BIT(ch->desc->account->flags, ACCOUNT_ALLOW_DOUBLECLASS);
    ch->desc->saveAccount();
  }
  // allow access to factions for L50, others can get it on gain
  if (ch->GetMaxLevel() == 50 && ch->isUnaff() &&
      !ch->hasQuestBit(TOG_FACTIONS_ELIGIBLE)) {
    ch->setQuestBit(TOG_FACTIONS_ELIGIBLE);
  }
    
  extern void giveGodsTheirPowers(TBeing *ch);
  giveGodsTheirPowers(ch);

  return;
}

bool TBeing::nomagic(const char *msg_ch, const char *msg_rm) const
{
  if (isImmortal())
    return FALSE;

  if (roomp && roomp->isRoomFlag(ROOM_NO_MAGIC)) {
    if (msg_ch)
      act(msg_ch, FALSE, this, 0, 0, TO_CHAR);

    if (msg_rm)
      act(msg_rm, FALSE, this, 0, 0, TO_ROOM);

    return 1;
  }
  return 0;
}

// please note, this ignores riders
int MobCountInRoom(const TThing *list)
{
  int i;
  const TThing *t;

  for (i = 0, t = list; t; t = t->nextThing) {
    if (dynamic_cast<const TBeing *>(t))
      i++;
  }
  return i;
}

int GetApprox(int num, int perc)
{
  return (int) (GetApprox((double) num, perc) + 0.5);
}

double GetApprox(double num, int perc)
{
// perc is 0-100 represents how well elarned 
// num is actual number 

  float adj = 100.0 - perc;
  adj = max((float) 5.0, adj);
  // the better learned they are, the closer adj will be to 0 

  // we should be +- 50% at most
  // adj is in range [5-100]
  adj /= 2.0;
  // adj is now [2.5 - 50], treat as a +- percentage

  // calculate the biggest deviation to allow
  double max_adj = adj * num / 100.0;

  // number can't do double, so scale by 100
  max_adj *= 100.0;
  double act_adj = ::number((int) -max_adj, (int) max_adj);
  act_adj /= 100.0; 

  double fnum = num + act_adj;

  return fnum;
}

int TBeing::numClasses() const
{
  int x = 0;
  classIndT i;

  for (i = MIN_CLASS_IND; i < MAX_CLASSES; i++)
    if (getLevel(i))
      x++;
  return x;
}

void vlogf(int severity, string errorMsg,...)
{
  vlogf(severity, errorMsg.c_str());
}

void vlogf(int severity, const char *errorMsg,...)
{
  char message[MAX_STRING_LENGTH + MAX_STRING_LENGTH];
  char buf[MAX_STRING_LENGTH + MAX_STRING_LENGTH];
  Descriptor *i;
  time_t lt;
  struct tm *this_time;
  va_list ap;

  va_start(ap, errorMsg);
  vsprintf(message, errorMsg, ap);
  va_end(ap);

  lt = time(0);
  this_time = localtime(&lt);
  if (severity == LOW_ERROR) {
    sprintf(buf, "// L.O.W. Error:   %s \n\r", message); 
    severity = 5;

    fprintf(stderr,  "%2.2d%2.2d%2.2d|%2.2d:%2.2d:%2.2d :: L.O.W. Error: %s\n",
         this_time->tm_year, this_time->tm_mon + 1, this_time->tm_mday,
         this_time->tm_hour, this_time->tm_min, this_time->tm_sec, message);
  } else {
    sprintf(buf, "// %s", message);

    fprintf(stderr,  "%2.2d%2.2d%2.2d|%2.2d:%2.2d:%2.2d :: %s\n",
         this_time->tm_year, this_time->tm_mon + 1, this_time->tm_mday,
         this_time->tm_hour, this_time->tm_min, this_time->tm_sec, message);
  }
  for (i = descriptor_list; i; i = i->next) {
    if (!i->connected && i->character &&
        ((i->character->hasWizPower(POWER_WIZNET_ALWAYS)) ||
            ((i->character->GetMaxLevel() >= GOD_LEVEL1) && 
            i->character->hasQuestBit(TOG_IMMORTAL_LOGS))) &&
	(i->severity <= severity) && 
        !(i->character->isPlayerAction(PLR_MAILING | PLR_BUGGING))) {

      if (i->client) 
        i->clientf("%d|%d|%s", CLIENT_LOG, severity, buf);
      else 
        i->character->sendTo(COLOR_LOGS, "%s\n\r", buf);
    }
  }
}

void dirwalk(const char *dir, void (*fcn) (const char *))
{
  struct dirent *dp;
  DIR *dfd;

  if (!dir || !(dfd = opendir(dir))) {
    vlogf(10, "Unable to dirwalk directory %s", dir);
    return;
  }
  while ((dp = readdir(dfd))) {
    if (!strcmp(dp->d_name, ".") || !strcmp(dp->d_name, ".."))
      continue;
    (*fcn) (dp->d_name);
  }
  closedir(dfd);
}

void dirwalk_fullname(const char *dir, void (*fcn) (const char *))
{
  char filepath[1024];
  struct dirent *dp;
  DIR *dfd;

  if (!dir || !(dfd = opendir(dir))) {
    vlogf(10, "Unable to dirwalk directory %s", dir);
    return;
  }
  while ((dp = readdir(dfd))) {
    if (!strcmp(dp->d_name, ".") || !strcmp(dp->d_name, ".."))
      continue;
    sprintf(filepath, "%s/%s", dir, dp->d_name);
    (*fcn) (filepath);
  }
  closedir(dfd);
}

void dirwalk_subs_fullname(const char *dir, void (*fcn) (const char *))
{
  char name[1024];
  struct dirent *dp;
  struct stat stbuf;
  DIR *dfd;

  if (!dir || !(dfd = opendir(dir))) {
    vlogf(10, "Unable to dirwalk_subs directory %s", dir);
    return;
  }
  while ((dp = readdir(dfd))) {
    if (!strcmp(dp->d_name, ".") || !strcmp(dp->d_name, ".."))
      continue;
    sprintf(name, "%s/%s", dir, dp->d_name);
    if ((stat(name, &stbuf) == -1) || ((stbuf.st_mode & S_IFMT) != S_IFDIR))
      continue;
    dirwalk_fullname(name, fcn);
  }
  closedir(dfd);
}

bool TBeing::canSee(const TThing *t, infraTypeT infra) const
{
  mud_assert(t != NULL, "canSee with NULL t");

  return t->canSeeMe(this, infra);
}

bool TBeing::canSeeMe(const TBeing *ch, infraTypeT infra) const
{
  TRoom *r;

  if (!(r = roomp)) {
    if (parent) 
      r = parent->roomp;
    else {
      forceCrash("Thing (%s) has no rp pointer in TThing::canSee", name);
      return FALSE;
    }
  }

  // hide imms with invis level set, and linkdeads (L51)
  if (ch->GetMaxLevel() < getInvisLevel()) 
    return FALSE;

  if (ch->isImmortal())
    return TRUE;

  // I can see myself, even if invisible
  // otherwise, how do I target dispel invisibility  :)
  if (this == ch)
    return TRUE;

  if (isAffected(AFF_INVISIBLE)) {
    if (!ch->isAffected(AFF_DETECT_INVISIBLE))
      return FALSE;
  }

  if (ch->isAffected(AFF_TRUE_SIGHT))
    return TRUE;

  if (ch->isAffected(AFF_BLIND))
    return FALSE;

  if (isAffected(AFF_SANCTUARY))
    return TRUE;

  if (getLight() > 0)
    return TRUE;

  // invis gods have been checked above, so this is safe
  if (isImmortal())
    return TRUE;

  if (r->isRoomFlag(ROOM_ALWAYS_LIT))
    return TRUE;

  int vision = ch->eyeSight(r);
  int vision_adj = 0;

  if (infra && ch->isAffected(AFF_INFRAVISION)) {
    if (!isColdBlooded()) {

      vision_adj += 2;   // basic assistance

      // easier at night, harder at day
      if (outside()) {
        if (is_nighttime())
          vision_adj += 1;
        else if (is_daytime())
          vision_adj -= 1;
      } else   // indoors or underwater
        vision_adj += 1;

      if (roomp->isArcticSector())
        vision_adj += 1;
      else if (roomp->isTropicalSector())
        vision_adj -= 1;
    } else {
      // don't reveal cold blooded with infravision
      return FALSE;
    }
  }
  vision += max(vision_adj, 0);
  if (vision > visibility())
    return TRUE;

  return FALSE;
}

bool TObj::canSeeMe(const TBeing *ch, infraTypeT) const
{
  TRoom *rp;

  if (ch->isImmortal())
    return TRUE;

  if (isObjStat(ITEM_INVISIBLE))
    if (!ch->isAffected(AFF_DETECT_INVISIBLE))
      return FALSE;

  int room = roomOfObject(this);

  if (room == ROOM_NOWHERE)
    room = ch->in_room;

  if (!(rp = real_roomp(room))) 
    return FALSE;
  
  if (ch->isAffected(AFF_TRUE_SIGHT))
    return TRUE;

  if (ch->isAffected(AFF_BLIND))
    return FALSE;

  if (rp->isRoomFlag(ROOM_ALWAYS_LIT))
    return TRUE;

  if (isObjStat(ITEM_GLOW) || isObjStat(ITEM_BURNING))
    return TRUE;

  int s = ch->eyeSight(rp);
  int mod = (((dynamic_cast<const TBeing *>(equippedBy) == ch) ? -7 : 
         ((dynamic_cast<const TBeing *>(parent) == ch) ? -4 : 
         ((dynamic_cast<const TBeing *>(stuckIn) == ch) ? -7 : 0))));
  if (s >= (canBeSeen + mod))
    return TRUE;

  if (getLight() > 0)
    return TRUE;

  return false;
}

bool TSeeThru::canSeeMe(const TBeing *ch, infraTypeT infra) const
{
  if (givesOutsideLight())
    return TRUE;

  return TObj::canSeeMe(ch, infra);
}

bool TThing::canSeeMe(const TBeing *, infraTypeT) const
{
  return FALSE;
}

bool can_see_char_other_room(const TBeing *ch, TBeing *victim, TRoom *)
{
  if (!victim || ch->in_room < 0 || victim->in_room < 0)
    return FALSE;

  if (ch->isImmortal()) {
    if (ch->GetMaxLevel() < (victim->getInvisLevel()))
      return FALSE;
    else
      return TRUE;
  } else {
    // this is here to keep imm's gone mort from seeing link deads
    if (victim->getInvisLevel() >= GOD_LEVEL1)
      return FALSE;
  }
  if (victim->isAffected(AFF_INVISIBLE)) {
    if (!ch->isAffected(AFF_DETECT_INVISIBLE))
      return FALSE;
  }
  if (ch->GetMaxLevel() < victim->getInvisLevel())
    return FALSE;

  if (ch->isAffected(AFF_TRUE_SIGHT))
    return TRUE;

  if (ch->isAffected(AFF_BLIND))
    return FALSE;

  sh_int sight = ch->eyeSight(victim->roomp);

  if ((sight >= victim->visibility()) || victim->roomp->isRoomFlag(ROOM_ALWAYS_LIT))
    return TRUE;
  if (victim->getLight() > 0)
    return TRUE;

  // all the invis gods have been checked for in above checks
  if (victim->isImmortal())
    return TRUE;

  return FALSE;
}

// disallow any bogus characters in automated system requests.
//   this is intented to prevent 'hacking' 
bool safe_to_be_in_system(const char *cp)
{
  return (strpbrk(cp, "\"';`") == NULL);
}

bool safe_to_be_in_system_filepath(const char *cp)
{
  const char *c;

  if (!cp)
    return FALSE;

  for (c = cp; *c; c++)
    if (!(isalnum(*c) || (*c == '*')))
      return FALSE;

  return TRUE;
}

bool TBeing::makesNoise() const
{
  int n;

  if (isImmortal())
    return FALSE;

  n = noise(this);

  // The higher the noise, the better chance <= 80 fails. - Russ 
  if (n <= 1)
    return FALSE;

  if (::number(1, n) <= 100)
    return FALSE;
  else
    return TRUE;
}

// vsystem - do a system command without duplicating the core image. 
//   This should save a lot of CPU time considering the mud can be
//  over 10 megs in size.  - SG                                    
int vsystem(char *buf)
{
  extern char **environ;
  int pid;
  char sh[64];
  char *prog;
  char tmp[64];
  char * argv[4];
#ifndef SUN
  int *status = NULL;
#else
  union wait status;
#endif

  if (!buf) {
    vlogf(10, "vsystem called with NULL parameter.");
    return FALSE;
  }
  strcpy(sh, "/bin/sh");
  if (!(prog = strrchr(sh, '/')))
    prog = sh;

  argv[0] = prog;
  strcpy(tmp, "-c");
  argv[1] = tmp;
  argv[2] = buf;
  argv[3] = NULL;
  if (!(pid = vfork())) {
    execve(sh, argv, environ);
    _exit(-1);
  }
  if (pid < 0) {
    vlogf(10, "Error in vsystem.  Fork failed.");
    return FALSE;
  }
#ifndef SUN
  while (wait(status) != pid);
#else
  while (wait(&status) != pid);
#endif

  return TRUE;
}

bool TThing::shouldntBeShown(wearSlotT pos) const
{
  return FALSE;
}

bool TObj::shouldntBeShown(wearSlotT pos) const
{
  if (!isPaired())
    return FALSE;

  if ((pos == WEAR_LEGS_L) || (pos == HOLD_LEFT) || (pos == WEAR_EX_LEG_L))
    return TRUE;

  return FALSE;
}

void TBeing::fixLevels(int lev)
{
  if (hasClass(CLASS_MAGIC_USER))
    setLevel(MAGE_LEVEL_IND, lev);
  if (hasClass(CLASS_CLERIC))
    setLevel(CLERIC_LEVEL_IND, lev);
  if (hasClass(CLASS_WARRIOR))
    setLevel(WARRIOR_LEVEL_IND, lev);
  if (hasClass(CLASS_THIEF))
    setLevel(THIEF_LEVEL_IND, lev);
  if (hasClass(CLASS_RANGER))
    setLevel(RANGER_LEVEL_IND, lev);
  if (hasClass(CLASS_MONK))
    setLevel(MONK_LEVEL_IND, lev);
  if (hasClass(CLASS_DEIKHAN))
    setLevel(DEIKHAN_LEVEL_IND, lev);
  if (hasClass(CLASS_SHAMAN))
    setLevel(SHAMAN_LEVEL_IND, lev);

  calcMaxLevel();
}

bool should_be_logged(const TBeing *ch)
{
  if (!strcmp(ch->getName(), "Batopr"))
    return FALSE;
  else
    return TRUE;
}

// I hate typing > x && < y  -  Russ 
bool in_range(int num, int low, int high)
{
  if ((num < low) || (num > high))
    return FALSE;

  return TRUE;
}

bool thingsInRoomVis(TThing *ch, TRoom *rp)
{
  TThing *o;

  if (!ch || !rp) {
    vlogf(10, "thingsInRoomVis() called with NULL ch, or room!");
    return FALSE;
  }
  for (o = rp->stuff; o; o = o->nextThing) {
    if (ch->canSee(o))
      return TRUE;
  }
  return FALSE;
}

// can_get - Russ Russell c June 1994. last editted June 96
// can_get returns TRUE if the ch can get the thing
// passed to it. If the ch can't get it, it sends to  
// the ch why it can't get it, and then returns FALSE.

bool TBeing::canGet(const TThing *t, silentTypeT silent) const
{
  return t->canGetMe(this, silent);
}

bool TObj::canGetMe(const TBeing *ch, silentTypeT silent) const
{
  // sanity check
  if (!shortDescr) {
    forceCrash("!shortDescr for obj in canGetMe");
    return false;
  }

  char capbuf[256];
  strcpy(capbuf, shortDescr);

  if (!ch->canSee(this)) 
    return FALSE;

  if (ch->isImmortal() || 
      (canWear(ITEM_TAKE) && !isObjStat(ITEM_PROTOTYPE))) {
    // flat out deny
    if (canGetMeDeny(ch, silent))
      return FALSE;

    if (rider) {
      if (!silent)
        ch->sendTo(COLOR_OBJECTS, "%s : Occupied.\n\r", cap(capbuf));
      return FALSE;
    }

    // attached items 
    if (isObjStat(ITEM_ATTACHED)) {
      if (!ch->isImmortal()) {
        if (canWear(ITEM_TAKE)) {
          if (riding) {
            ch->sendTo(COLOR_OBJECTS, "%s is attached to %s and is not currently getable.\n\r", getName(), riding->getName());
          } else {
            ch->sendTo(COLOR_OBJECTS, "%s is attached and is not currently getable.\n\r", getName());
          
          }
        } else {
          ch->sendTo(COLOR_OBJECTS, "%s : You can't take that.\n\r", getName());
        }
        return FALSE;
      }
    }

    // weight/vol check
    // items in bags already have been accounted for
    if (!ch->canCarry(this, silent))
      return FALSE;

    return TRUE;
  } else {
    // attached items
    if (isObjStat(ITEM_ATTACHED)) {
      if (canWear(ITEM_TAKE))
        ch->sendTo(COLOR_OBJECTS, "%s is attached and is not currently getable.\n\r", getName());
      else
        ch->sendTo(COLOR_OBJECTS, "%s : You can't take that.\n\r", getName());
      return FALSE;
    }

    if (!silent)
      ch->sendTo(COLOR_OBJECTS, "%s : You can't take that.\n\r", cap(capbuf));

    return FALSE;
  }
  return TRUE;
}

bool TBeing::canGetMe(const TBeing *ch, silentTypeT) const
{
  return ((ch->GetMaxLevel() == GOD_LEVEL10) && (ch != this) && !desc);
  // return ((ch->GetMaxLevel() == GOD_LEVEL10) && (ch != this));
}

bool TThing::canGetMe(const TBeing *, silentTypeT) const
{
  return FALSE;
}

const char *TBeing::movementType(bool enter) const
{
  if (!roomp) {
    forceCrash("NULL roomp in MovementType()!");
    return "";
  }
  if (getPosition() == POSITION_CRAWLING)
    return(enter ? "crawls in" : "crawls");

  if (isFlying())
    return(enter ? "flies in" : "flies");


  if (roomp->isUnderwaterSector() ||
      (roomp->isWaterSector() && !isLevitating() &&
      !isFlying())) {
    if (hasBoat()) {
      return(enter ? "boats in" : "boats");
    }
    return(enter ? "swims in" : "swims");
  }

  if (IS_SET(specials.act, ACT_GHOST))
    return (enter ? "glides in" : "glides");
  if (IS_SET(specials.act, ACT_ZOMBIE))
    return(enter ? "shuffles in" : "shuffles");

  if (getCond(DRUNK) > 6)
    return(enter ? "staggers in" : "staggers");

  return (enter ? race->moveIn() : race->moveOut());
}

// limit followers based on total power of all pets and sheer number
// of pets owned
bool TBeing::tooManyFollowers(const TBeing *pet, newFolTypeT type) const
{
  followData *k;
  int max_followers = 0;  // total power of pets allowed
  int count = 0;  // current power of pets
  unsigned int tot_num = 0;  // actual number of pets ALREADY IN GROUP

  max_followers = GetMaxLevel() / 5;
  max_followers += plotStat(STAT_CURRENT, STAT_CHA, 1, 19, 9);

  for(k = followers, count = 0; k; k = k->next) {
    if (k->follower->isZombie()) {
      count += 1 + (k->follower->GetMaxLevel() / 10);
      tot_num++;
    } else if (k->follower->isCharm()) {
      count += 2 + (k->follower->GetMaxLevel() / 10);
      tot_num++;
    } else if (k->follower->isPet()) {
      count += 1 + (k->follower->GetMaxLevel() / 7);
      tot_num++;
    }
  }
  // count now represents the weight of all my current followers
  // now see if adding pet will push me over the top
  if (type == FOL_ZOMBIE)
    count += 1 + (pet->GetMaxLevel() / 10);
  else if (type == FOL_CHARM)
    count += 2 + (pet->GetMaxLevel() / 10);
  else if (type == FOL_PET)
    count += 1 + (pet->GetMaxLevel() / 7);

  if (count > max_followers)
    return TRUE;
  if (tot_num >= 3)  // allow 3 pets max
    return TRUE;

  return FALSE;
}

int TBeing::followTime() const
{
  if (!strcmp("cosmo", name)) 
    return UPDATES_PER_TICK;

  return (plotStat(STAT_CURRENT, STAT_CHA, 6, 36, 22) * UPDATES_PER_TICK);
}

// a higher value = harder to see
int TThing::visibility() const
{
  // sneak sets canBeSeen
  int cbs = canBeSeen;   // natural amount

  if (!roomp)
    return cbs;

  const TBeing *tbt = dynamic_cast<const TBeing *>(this);
  if (tbt) {
    if (tbt->isAffected(AFF_HIDE) && !tbt->fight())
      cbs += 5 + tbt->GetMaxLevel()/2;

    // adjust for other stuff here
    if (tbt->getRace() == RACE_ELVEN) {
      if (tbt->roomp->isForestSector())
        cbs += 5;
    }
  }

  if (roomp->isForestSector())
    cbs += 2;
  if (roomp->getWeather() == WEATHER_RAINY)
    cbs += 1;
  else if (roomp->getWeather() == WEATHER_SNOWY)
    cbs -= 2;
  else if (roomp->getWeather() == WEATHER_LIGHTNING)
    cbs -= 1;

  return cbs;
}

int TBeing::eyeSight(TRoom *rp) const
{
  int vision = 0;

  vision = visionBonus;
  vision += getMyRace()->visionBonus;
  
  // this is here so that items carried (!rp) are treated as being in room i 
  // am looking from
  if (!rp)
    rp = roomp;

  vision += rp->getLight();

  if (isAffected(AFF_TRUE_SIGHT)) 
    vision += 25;
  
  if (rp->getWeather() == WEATHER_RAINY)
    vision -= 1;
  else if (rp->getWeather() == WEATHER_SNOWY)
    vision -= 2;
  else if (rp->getWeather() == WEATHER_LIGHTNING)
    vision -= 1;

  // if they are indoors, the lighting of the room should be "subdued"
  if (rp->isRoomFlag(ROOM_INDOORS))
    vision -= rp->getLight() / 2;

  return vision;
}

bool TBeing::willBumpHeadDoor(roomDirData *exitp, int *height) const
{
  if (!roomp)
    return FALSE;

  int height1 = (real_roomp(exitp->to_room))->getRoomHeight();
  int height2 = roomp->getRoomHeight();

  if (isImmortal())
    return FALSE;

  *height = min(height1, height2);
  if (height1 <= 0)
    *height = height2;
  if (height2 <= 0)
    *height = height1;

  if (*height <= 0)
    return FALSE;

  if (exitp->door_type != DOOR_NONE)
    *height = 9 * *height / 10;   //  doors are 90% size of room 

  return (willBump(*height));
}

// returns DELETE_THIS
int TBeing::bumpHeadDoor(roomDirData *exitp, int *height)
{
  char buf[160], doorbuf[80];
  TThing *helm;
  int hardness, check;

  if (isImmortal())
    return FALSE;

  if (!willBumpHeadDoor(exitp, height))
    return FALSE;

  sprintf(doorbuf, exitp->getName().c_str());
  if (::number(1, 300) > plotStat(STAT_CURRENT, STAT_AGI, 30, 180, 110)) {
    sendTo("You bump your head as you go through the %s.  OUCH!\n\r",
            uncap(doorbuf));
    sprintf(buf, "$n bumps $s head on the %s.  That had to hurt.",
            uncap(doorbuf));
    act(buf,TRUE, this, 0,0,TO_ROOM);
    // Lets do some head-gear checks to see if gear can absorb or negate damage
    // Very simple to start - Brutius - 12-31-95
    if ((helm = equipment[WEAR_HEAD])) {
      hardness = material_nums[helm->getMaterial()].hardness;
      // The harder the material the less chance of damage to head or item
      check = ::number(0, hardness);
      if (!check) {
        if (reconcileDamage(this,::number(1,3),DAMAGE_NORMAL) == -1)
          return DELETE_THIS;
      }
    } else {
      // no helm, just take damage
      if (reconcileDamage(this,::number(1,3),DAMAGE_NORMAL) == -1)
        return DELETE_THIS;
    } 
  } else 
    sendTo("You duck down as you go through the %s.\n\r", uncap(doorbuf));
  
  return FALSE;
}

bool TBeing::willBumpHead(TRoom *rp) const
{
  int height = rp->getRoomHeight();

  if (isImmortal())
    return FALSE;

  if (height <= 0)
    return FALSE;

  return (willBump(height));
}

bool TBeing::willBump(int height) const
{
  if (height <= 0)
    return FALSE;

  if (isImmortal())
    return FALSE;

  if (getPosHeight() <= height)
    return FALSE;

  return TRUE;
}

bool hasDigit(char *s)
{
  char *c;

  for (c = s; *c; c++) {
    if (isdigit(*c))
      return true;
  }
  return false;
}

bool TBeing::isNaked() const
{
  int i;
  for (i = MIN_WEAR; i < MAX_WEAR; i++) {
    if (i == HOLD_RIGHT || i == HOLD_LEFT)
      continue;
    if (equipment[i])
      return FALSE;
  }
  return TRUE;
}

// posistive mod makes them smarter
bool TMonster::isSmartMob(int mod) const
{
  int i = plotStat(STAT_CURRENT, STAT_INT, 8, 50, 23);

  // a negative mod makes them dumber
  if (::number(0,100) < (i + mod))
    return TRUE;
  else
    return FALSE;
}

void TPerson::addToWait(int orig_amt)
{
  int mod = 100;
  if (affectedBySpell(SPELL_ACCELERATE))
    mod = 80;
  if (affectedBySpell(SPELL_HASTE))
    mod = 60;

  int amt = orig_amt * mod / 100;

  if (desc && !isImmortal()) {
    if (orig_amt >= 1)
      desc->wait += max(1,amt);
    else 
      desc->wait += max(0,amt);
  }
}

void TMonster::addToWait(int amt)
{
  // pulse_mobact == pulse_combat
  // combat gets done before mobacts in the main socket.cc loop
  // so if combat adds 1 round of lag, mobact wills strip it off immediately
  // eg. pulse 1: mob gets bashed (1 round lag), mobact removes lag
  //     pulse 2: combat + mob stands up
  // thus looking like no lag at all
  // lets get around this by just adding 1 extra round of lag
  // yeah, this messes things up for non combat generated lag, but oh well
  // obviously, this is mob only due to way we handle wait differently
  // - Batopr 2-6-97

// Cos- 7/98-- needed for polymorph type spells
  if (desc) {
    if(!(desc->original && desc->original->GetMaxLevel()>MAX_MORT &&
       IS_SET(desc->autobits, AUTO_SUCCESS))){
      desc->wait += amt;
    }
  } else {
    wait += amt;
    wait += 1; // 1 extra lag
  }
}

float TBeing::getTotalWeight(bool pweight) const
{
  float calc = getCarriedWeight();;
  int i;
  TThing *t;

  for (i = MIN_WEAR; i < MAX_WEAR; i++) {
    if ((t = equipment[i])) {
      calc += t->getTotalWeight(TRUE);
    }
  }
  // add in char's weight if appropriate
  if (pweight)
    calc += getWeight();

  return calc;
}

void TThing::addToCarriedWeight(float num)
{
  carried_weight += num;

  if (parent)
    parent->addToCarriedWeight(num);
  if (riding)
    riding->addToCarriedWeight(num);
}

void TThing::addToCarriedVolume(int num)
{
  carried_volume += num;
}

bool TBeing::isCharm() const
{
  return (!isPc() && master && isAffected(AFF_CHARM) &&
          affectedBySpell(SPELL_ENSORCER) &&
          !isUndead());
}

bool TBeing::isZombie() const
{
  return (!isPc() &&  master && isAffected(AFF_CHARM) &&
          isUndead());
}

bool TBeing::isElemental() const
{
  int  mVn         = mobVnum();
  bool isElemental = (mVn == FIRE_ELEMENTAL  || mVn == WATER_ELEMENTAL ||
                        mVn == EARTH_ELEMENTAL || mVn == AIR_ELEMENTAL);
  return isElemental;
}

bool TBeing::isPet() const
{
  return (!isPc() && master && isAffected(AFF_CHARM) &&
          !affectedBySpell(SPELL_ENSORCER));
}

// checks a room looking for pcs present
// returns TRUE if no pc's are around
// ignore_imms allows imms to be considered or not
// eg ignore_imms==TRUE, room with just an immortal will be considered empty
bool TRoom::roomIsEmpty(bool ignore_imms) const
{
  TThing *t;
  TBeing *vict;

  for (t = stuff; t; t = t->nextThing) {
    vict = dynamic_cast<TBeing *>(t);
    if (!vict)
      continue;
    if (vict->isPc() && !vict->isImmortal())  // mortals trigger always
      return FALSE;
    if (vict->isPc() && vict->isImmortal() && !ignore_imms)
      return FALSE;
  }
  return TRUE;
}

void TBeing::addToMoney(int money, moneyTypeT type)
{
  int lev = 0;
  int amount;

  points.money += money;
 
  // due to the way the stats are set up, don't try to change this 60 to
  // a #define value
  if (isPc() && ((lev = GetMaxLevel()) <= 60)) {
    switch (type) {
      case GOLD_XFER:
        // do not track, essentially an internal transfer (to/from bank)
        break;
      case GOLD_INCOME:
        gold_statistics[GOLD_INCOME][lev-1] += money;
        gold_positive[GOLD_INCOME][lev-1] += max(money, 0);

        // figure out tithe
        if (money > 0) {
          amount = (int) (money * FactionInfo[getFaction()].faction_tithe / 100.0);
          // subtract the tithe amount from me
          points.money -= amount;
          gold_statistics[GOLD_INCOME][(lev-1)] -= amount;
          gold_positive[GOLD_INCOME][(lev-1)] -= max(amount, 0);

          FactionInfo[getFaction()].faction_wealth += amount;
          gold_statistics[GOLD_TITHE][(lev-1)] += amount;
          gold_positive[GOLD_TITHE][(lev-1)] += max(amount, 0);
          reconcileHelp(NULL, amount * TITHE_FACTOR);
        }
        break;
      case GOLD_TITHE:
        gold_statistics[GOLD_TITHE][(lev-1)] += money;
        gold_positive[GOLD_TITHE][(lev-1)] += max(money, 0);
        // note, this will cause leaders percent to drop if they withdraw.
        // no other good way to do it without letting leaders get huge boost
        // it's helpful, if they GIVE money by tithing (money would be < 0)
        reconcileHelp(NULL, -money * TITHE_FACTOR);
        break;
      case GOLD_REPAIR:
      case GOLD_SHOP:
      case GOLD_COMM:
      case GOLD_HOSPITAL:
      case GOLD_GAMBLE:
      case GOLD_RENT:
      case GOLD_DUMP:
      case GOLD_SHOP_SYMBOL:
      case GOLD_SHOP_WEAPON:
      case GOLD_SHOP_ARMOR:
      case GOLD_SHOP_PET:
      case GOLD_SHOP_FOOD:
      case GOLD_SHOP_RESPONSES:
      case GOLD_SHOP_COMPONENTS:
        gold_statistics[type][(lev-1)] += money;
        gold_positive[type][(lev-1)] += max(money, 0);
        break;
      case MAX_MONEY_TYPE:
        break;
    }
    save_game_stats();
  }
}

bool TThing::outside() const
{
  return (!roomp->isRoomFlag(ROOM_INDOORS) &&
          !roomp->isUnderwaterSector() &&
          roomp->getSectorType() != SECT_ASTRAL_ETHREAL);
}

bool TBeing::isSimilar(const TThing *t) const
{
  const TBeing *tb = dynamic_cast<const TBeing *>(t);
  if (!tb)
    return FALSE;

  if (tb->number == number && (tb->getPosition() == getPosition()) &&
      (tb->specials.affectedBy == specials.affectedBy) &&
      // if they have spells on them, prevent being similar
      !affected && !tb->affected &&
      ((tb->fight() == fight()) &&
       (tb->getName() && getName() &&
       !strcmp(tb->getName(), getName()))))
     return TRUE;

  return FALSE;
}

bool TObj::isSimilar(const TThing *t) const
{
  const TObj *obj = dynamic_cast<const TObj *>(t);
  if (!obj)
    return FALSE;

  if (number != obj->number)
    return false;
  if (!getDescr() || !obj->getDescr() ||
      strcmp(getDescr(), obj->getDescr()))
    return false;
  if (!getName() || !obj->getName() ||
      strcmp(getName(), obj->getName()))
    return false;
  if (getStructPoints() != obj->getStructPoints())
    return false;

  // requires the effects be in same order (as well as identical)
  // the ordering  really isn't important, but it makes logic here simpler
  // we got away without the affect loop for quite some time
  // added due to enhanced weapon being clustered with non-magic ones
  int i;
  for (i = 0; i < MAX_OBJ_AFFECT; i++) {
    if (affected[i].location != obj->affected[i].location)
      return false;
    if (affected[i].modifier != obj->affected[i].modifier)
      return false;
    if (affected[i].modifier2 != obj->affected[i].modifier2)
      return false;
    if (affected[i].bitvector != obj->affected[i].bitvector)
      return false;
  }

  return true;
}

// the more NEGATIVE this is, the better the item is
int TObj::itemAC() const
{
  int j, armor = 0;

  for (j = 0; j < MAX_OBJ_AFFECT; j++) {
    if (affected[j].location == APPLY_ARMOR)
      armor += affected[j].modifier;
  }
  return armor;
}

int TObj::itemNoise() const
{
  int j, noise = 0;

  noise = material_nums[getMaterial()].noise;
  for (j = 0; j < MAX_OBJ_AFFECT; j++) {
    if (affected[j].location == APPLY_NOISE)
      noise += affected[j].modifier;
  }

  return noise;
}

int TObj::itemDamroll() const
{
  int j, total = 0;

  for (j = 0; j < MAX_OBJ_AFFECT; j++) {
    if ((affected[j].location == APPLY_DAMROLL) ||
        (affected[j].location == APPLY_HITNDAM)) {
      total += affected[j].modifier;
    }
  }
  return total;
}

int TObj::itemHitroll() const
{
  int j, total = 0;

  for (j = 0; j < MAX_OBJ_AFFECT; j++) {
    if ((affected[j].location == APPLY_HITROLL) ||
        (affected[j].location == APPLY_HITNDAM)) {
      total += affected[j].modifier;
    }
  }
  return total;
}

double TMonster::baseDamage() const
{
  // follows concepts developed in balance theorems
  double amt = (getDamLevel() / 1.1);
  // adjust for numAtt
  amt /= getMult();

  return amt;
}

int TBeing::getVolume() const
{
  // we are going to fudge a value for volume
  // model a being as a cylinder of height h and radius r.
  // For the typical human, height is 175 cm (70 in), and r is 13 cm (5 in)
  // obviously, if we are going to scale, r must be a function of h
  // thus we can sort of say V = h * pi * r^2 = k^2 * pi * h^3
  // where k is an arbitrary constant used to scale k=r/h
  // since we want V for a 70 inch human to = 42500 (sum of limb volumes)
  // this becomes a k of 0.2
  // we have set corpse_const on all races equivalent to k
 

  int vol;

  mud_assert(race != NULL, "No race in getVolume()");

  vol = (int) ((race->corpse_const) * (race->corpse_const) * M_PI * getHeight() * getHeight() * getHeight());

  return vol;
}

char LOWER(char c)
{
  return ((c >= 'A' && c <= 'Z') ? (c+('a'-'A')) : c);
}

char UPPER(char c)
{
  return ((c >= 'a' && c <= 'z') ? (c+('A'-'a')) : c );
}

char ISNEWL(char ch) 
{
  return (ch == '\n' || ch == '\r');
}

int combatRound(double n)
{
  return (int) (n * PULSE_COMBAT / (TurboMode ? 2 : 1));
}

bool TBeing::checkBusy(const char *buf)
{
  // I added the max to avoid the 10Mill report
  // which happens if they have like 0.9 attacks a round.
  float fx, fy;
  blowCount(true, fx, fy);
  double hitsPerRound = fx + fy;

  if (cantHit <= 0)
    return FALSE;

  if (buf && *buf)
    sendTo(COLOR_BASIC, buf);
  else {
    // this intentionally has no "/n/r"
    sendTo("You are still busy orienting yourself.");
  }
#if 0
  int tmpnum = (hitsPerRound ? (int) (cantHit/hitsPerRound + 1) : 1000000); 
  sendTo(" (Roughly %d round%s to go)\n\r", tmpnum, (tmpnum > 1) ? "s" : "");
#else
  float tmpnum = (hitsPerRound ? (cantHit/hitsPerRound) : 1000000); 
  tmpnum *= PULSE_COMBAT / (TurboMode ? 2 : 1);
  tmpnum /= ONE_SECOND;

  sendTo(" (Roughly %.1f seconds to go)\n\r", tmpnum);
#endif
  return TRUE;
}

float TBeing::lagAdjust(lag_t orig_lag)
{
  // basically, take a fixed amount of "orig_lag", and adjust it for speed
  float min_lag = 0.80 * orig_lag;
  float max_lag = 1.25 * orig_lag;

  // remember that the min_lag should be held by max-stat, and vice-versa
  float act_lag = plotStat(STAT_CURRENT, STAT_SPE, max_lag, min_lag, (float) orig_lag);

  act_lag = max(act_lag, (float) 0.0);
  return act_lag;
}

string secsToString(time_t num)
{
  unsigned int days = num / SECS_PER_REAL_DAY;
  unsigned int hours = (num / SECS_PER_REAL_HOUR) % 24;
  unsigned int mins = (num / SECS_PER_REAL_MIN) % 60;
  unsigned int secs = num % 60;

  string timestring = "";
  char buf[256];
#if 0
  if (weeks) {
    sprintf(buf, "%d week%s", weeks, weeks == 1 ? "" : "s");
    timestring += buf;
  }
#endif
  if (days) {
    sprintf(buf, "%d day%s", days, days == 1 ? "" : "s");
    if (!timestring.empty())
      timestring += ", ";
    timestring += buf;
  }
  if (hours) {
    sprintf(buf, "%d hour%s", hours, hours == 1 ? "" : "s");
    if (!timestring.empty())
      timestring += ", ";
    timestring += buf;
  }
  if (mins) {
    sprintf(buf, "%d minute%s", mins, mins == 1 ? "" : "s");
    if (!timestring.empty())
      timestring += ", ";
    timestring += buf;
  }
  if (secs) {
    sprintf(buf, "%d second%s", secs, secs == 1 ? "" : "s");
    if (!timestring.empty())
      timestring += ", ";
    timestring += buf;
  }
  return timestring;
}

bool isanumber(const char *c)
{
  if (!c)
    return FALSE;
  for (; *c; c++)
    if (!isdigit(*c))
      return FALSE;
  return TRUE;
}

