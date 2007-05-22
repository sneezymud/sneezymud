// bed.cc

#include "stdsneezy.h"
#include "games.h"
#include "obj_bed.h"
#include "obj_base_corpse.h"

TBed::TBed() :
  TObj(),
  min_pos_use(0),
  max_users(0),
  max_size(0),
  seat_height(0),
  regen(0)
{
}

TBed::TBed(const TBed &a) :
  TObj(a),
  min_pos_use(a.min_pos_use),
  max_users(a.max_users),
  max_size(a.max_size),
  seat_height(a.seat_height),
  regen(a.regen)
{
}

TBed & TBed::operator=(const TBed &a)
{
  if (this == &a) return *this;
  TObj::operator=(a);
  min_pos_use = a.min_pos_use;
  max_users = a.max_users;
  max_size = a.max_size;
  seat_height = a.seat_height;
  regen = a.regen;
  return *this;
}

TBed::~TBed()
{
}

int TBed::getMinPosUse() const
{
  return min_pos_use;
}

void TBed::setMinPosUse(int n)
{
  min_pos_use = n;
}

int TBed::getMaxUsers() const
{
  return max_users;
}

void TBed::setMaxUsers(int n)
{
  max_users = n;
}

int TBed::getMaxSize() const
{
  return max_size;
}

void TBed::setMaxSize(int n)
{
  max_size = n;
}

int TBed::getSeatHeight() const
{
  return seat_height;
}

void TBed::setSeatHeight(int n)
{
  seat_height = n;
}

int TBed::getRegen() const
{
  return regen;
}

void TBed::setRegen(int n)
{
  regen = n;
}

void TBed::changeObjValue1(TBeing *ch)
{
  ch->specials.edit = CHANGE_BED_VALUE1;
  change_bed_value1(ch, this, "", ENTER_CHECK);
  return;
}

int TBed::getAllFrom(TBeing *ch, const char *argument)
{
  int rc;

  act("You start getting items off $p.", TRUE, ch, this, NULL, TO_CHAR);
  act("$n starts getting items off $p.", TRUE, ch, this, NULL, TO_ROOM);
  start_task(ch, ch->roomp->getStuff(), ch->roomp, TASK_GET_ALL, argument, 350, ch->in_room, 0,0,0);
  // this is a kludge, task_get still has a tiny delay on it
  // this dumps around it and goes right to the guts
  rc = (*(tasks[TASK_GET_ALL].taskf))
      (ch, CMD_TASK_CONTINUE, "", 0, ch->roomp, 0);
  if (IS_SET_DELETE(rc, DELETE_THIS)) {
    return DELETE_VICT;
  }
  return FALSE;
}

bool TBed::isSimilar(const TThing *t) const
{
  // things on them should make them dissimilar
  if (rider)
    return FALSE;

  const TBed *ttab = dynamic_cast<const TBed *>(t);
  if (ttab && ttab->rider) {
    return FALSE;
  }
  return TObj::isSimilar(t);
}


int TBed::putSomethingInto(TBeing *ch, TThing *tThing)
{
  TBaseCorpse *corpse;

  if(!(corpse=dynamic_cast<TBaseCorpse *>(tThing)) ||
     this->parent){
    ch->sendTo("You cannot do that.\n\r");
    return 2;
  }

  if(getStuff() || rider){
    ch->sendTo("It is already being used by somebody.\n\r");
    return 2;
  }
  
  --(*tThing);
  *this->roomp += *tThing;
  tThing->mount(this);

  act("You prop up $p on $N.", TRUE, ch, tThing, this, TO_CHAR);
  act("$N props up $p on $N.", TRUE, ch, tThing, this, TO_ROOM);


  return 0;
}

void TBed::changeBedValue1(TBeing *ch, const char *arg, editorEnterTypeT type)
{
  int update_num;

  if (type != ENTER_CHECK) {
    if (!*arg || (*arg == '\n')) {
      ch->specials.edit = CHANGE_OBJ_VALUES;
      change_obj_values(ch, this, "", ENTER_CHECK);
      return;
    }
  }
  update_num = convertTo<int>(arg);

  switch (ch->specials.edit) {
    case CHANGE_BED_VALUE1:
      switch (update_num) {
        case 1:
          ch->sendTo(VT_HOMECLR);
          ch->sendTo("Enter new max users\n\r--> ");
          ch->specials.edit = CHANGE_BED_MAXUSERS;
          return;
        case 2:
          ch->sendTo(VT_HOMECLR);
          ch->sendTo("Enter new min. position (0 = sleeping, 1 = resting, 2 = sitting).\n\r--> ");
          ch->specials.edit = CHANGE_BED_MINPOS;
          return;
      }
      break;
    case CHANGE_BED_MAXUSERS:
      if (type != ENTER_CHECK) {
        if ((update_num > 15) || (update_num < 0)) {
          ch->sendTo("Please enter a number from 0 to 15.\n\r");
          return;
        }
        setMaxUsers(update_num);
        ch->specials.edit = CHANGE_BED_VALUE1;
        change_bed_value1(ch, this, "", ENTER_CHECK);
        return;
      }
      break;
    case CHANGE_BED_MINPOS:
      if (type != ENTER_CHECK) {
        if ((update_num > 3) || (update_num < 0)) {
          ch->sendTo("Please enter a number from 0-2.\n\r");
          ch->sendTo("0 = sleeping, 1 = resting, 2 = sitting, 3 = can't be used.\n\r");
          return;
        }
        setMinPosUse(update_num);
        ch->specials.edit = CHANGE_BED_MINPOS;
        change_bed_value1(ch, this, "", ENTER_CHECK);
        return;
      }
    default:
      return;
  }
  ch->sendTo(VT_HOMECLR);
  ch->sendTo("1) Max users (number that can sit/sleep/rest on)\n\r");
  ch->sendTo(fmt("        current: %d\n\r") % getMaxUsers());
  ch->sendTo("2) Min position (0 = sleeping, 1 = resting, 2 = sitting)\n\r");
  ch->sendTo(fmt("        current: %d\n\r") % getMinPosUse());
  ch->sendTo(fmt(VT_CURSPOS) % 10 % 1);
  ch->sendTo("Enter your choice to modify.\n\r--> ");
}

void TBed::bedRegen(TBeing *ch, int *gain, silentTypeT silent) const
{
  char buf[256];

  if ((ch->getPosition() == POSITION_SLEEPING) ||
          (ch->getPosition() == POSITION_SITTING) ||
          (ch->getPosition() == POSITION_RESTING)) {
    *gain += max(1,getRegen() * *gain / 100);
  }
  if ((ch->getHeight() - 6)  > getMaxSize()) {
    // reduce gain based on how uncomfy bed is
    *gain -= max(1,(ch->getHeight() - getMaxSize())/3);
    // but don't let it kill you
    *gain = max(*gain, 0);

    sprintf(buf, "The $o $q too small and uncomfortable for you.");
    if (!silent)
      act(buf,FALSE,ch,this,0,TO_CHAR);

    // the above act won't be sent (masked) if player !awake()
    // so kludge around this
    if (!ch->awake()) {
      if (ch->getPosition() == POSITION_SLEEPING) {
        if (!silent)
          ch->sendTo("You are much too uncomfortable to continue to sleep well.\n\r");
        // we are inside a task_xxx think, so do NOTHING that changes task
        // state.  i.e. don't set them resting or something
        *gain = 0;
      }
      return;
    }

    if (!ch->isPc()) {
      ch->doStand();
      sprintf(buf, "$n says, \"%s $o sucks!\"",
            isPluralItem() ? "These" : "This");
      if (!silent)
        act(buf,FALSE,ch, this,0,TO_ROOM);
    }

    return;
  }

  // the bed is comfy
  if (ch->getHit() < ch->hitLimit()) {
    // act() is suppressed if person is !awake(), use a sendTo
    if (!silent)
      ch->sendTo(COLOR_OBJECTS, fmt("Your rest on %s rejuvenates you.\n\r") % getName());
  }
}

void TBed::sitMe(TBeing *ch)
{
  if (getMinPosUse() > 2) {
    ch->sendTo("This item can't be sat on.\n\r");
    return;
  }
  if ((getNumRiders(ch) + 1) > getMaxRiders()) {
    act("There isn't enough room on $p to sit right now.",
          FALSE, ch, this, 0, TO_CHAR);
    return;
  }
  if (ch->rider) {
    ch->sendTo("You can't sit in that right now.\n\r");
    return;
  }
  switch (ch->getPosition()) {
    case POSITION_CRAWLING:
      act("You crawl into the $o and sit upon it.",
          FALSE,ch, this,0,TO_CHAR);
      act("$n crawls into the $o and sits upon it.",
          TRUE,ch,this,0,TO_ROOM);
      ch->setPosition(POSITION_SITTING);
      if (ch->isPc())
        start_task(ch, 0, 0, TASK_SIT, "", 350, 0, 1, 0, 4 * ch->regenTime());
      ch->mount(this);
      break;
    case POSITION_STANDING:
      if (objVnum() == OBJ_BATS_JACUZZI) {
        act("You climb into $p and sit down.", FALSE, ch, this, 0, TO_CHAR);
        act("$n climbs into $p and sits down.", TRUE, ch, this, 0, TO_ROOM);
      } else {
        act("You sit down on the $o.", FALSE, ch, this, 0, TO_CHAR);
        act("$n sits down on the $o.", TRUE, ch, this, 0, TO_ROOM);
      }
      ch->setPosition(POSITION_SITTING);
      if (ch->isPc())
        start_task(ch, 0, 0, TASK_SIT, "", 350, 0, 1, 0, 4 * ch->regenTime());
      if (ch->riding != this)
        ch->mount(this);
      break;
    case POSITION_SITTING:
      ch->sendTo("You're sitting already.\n\r");
      break;
    case POSITION_RESTING:
      if (objVnum() == OBJ_BATS_JACUZZI) {
        act("You sit up, but still find $p comfortable.", 
              FALSE, ch, this, 0, TO_CHAR);
        act("$n sits up in $p.", TRUE, ch, this, 0, TO_ROOM);
      } else {
        act("You stop resting, and sit up on the $o.",
              FALSE, ch, this, 0, TO_CHAR);
        act("$n stops resting, and sits up on the $o.",
              TRUE, ch, this, 0, TO_ROOM);
      }
      ch->setPosition(POSITION_SITTING);
      if (ch->isPc())
        start_task(ch, 0, 0, TASK_SIT, "", 350, 0, 1, 0, 4 * ch->regenTime());
      if (ch->riding != this)
        ch->mount(this);
      break;
    case POSITION_SLEEPING:
      act("You have to wake up first.", FALSE, ch, 0, 0, TO_CHAR);
      break;
    case POSITION_MOUNTED:
      ch->sendTo("Not while riding you don't!\n\r");
      break;
    default:
      act("You stop floating around, and sit on the $o.", 
             FALSE, ch, this, 0, TO_CHAR);
      act("$n stops floating around, and sits down on the $o.",
             TRUE, ch, this, 0, TO_ROOM);
      ch->setPosition(POSITION_SITTING);
      if (ch->isPc())
        start_task(ch, 0, 0, TASK_SIT, "", 350, 0, 1, 0, 4 * ch->regenTime());
      break;
  }
}

void TBed::restMe(TBeing *ch)
{
  if (getMinPosUse() > 1) {
    ch->sendTo("That item can't be rested on.\n\r");
    return;
  }
  if (ch->rider) {
    ch->sendTo("You can't rest in that right now.\n\r");
    return;
  }
  if ((getNumRiders(ch) + 2) > getMaxRiders()) {
    act("There isn't enough room on $p to rest right now.",
          FALSE, ch, this, 0, TO_CHAR);
    return;
  }
  switch (ch->getPosition()) {
    case POSITION_CRAWLING:
      act("You crawl into the $o and rest your tired bones.",
          TRUE,ch,this,0,TO_CHAR);
      act("$n crawls into the $o and rests $s tired bones.",
          TRUE,ch,this,0,TO_ROOM);
      ch->setPosition(POSITION_RESTING);
      if (ch->isPc())
        start_task(ch, 0, 0, TASK_REST, "", 350, 0, 1, 0, 2 * ch->regenTime());
      if (ch->riding != this)
        ch->mount(this);
      break;
    case POSITION_STANDING:
      if (objVnum() == OBJ_BATS_JACUZZI) {
        act("You climb into $p and allow yourself to relax.", 
                FALSE, ch, this, 0, TO_CHAR);
        act("$n climbs into $p and relaxes.", 
               TRUE, ch, this, 0, TO_ROOM);
      } else {
        act("You sit down in the $o, lean back and rest your tired bones.", 
               FALSE, ch, this, 0, TO_CHAR);
        act("$n sits down in the $o, leans back and rests.",
               TRUE, ch, this, 0, TO_ROOM);
      }
      ch->setPosition(POSITION_RESTING);
      if (ch->isPc())
        start_task(ch, 0, 0, TASK_REST, "", 350, 0, 1, 0, 2 * ch->regenTime());
      if (ch->riding != this)
        ch->mount(this);
      break;
    case POSITION_SITTING:
      if (ch->checkBlackjack())
        gBj.exitGame(ch);
      if (objVnum() == OBJ_BATS_JACUZZI) {
        act("You sit back and allow yourself to relax.", 
                FALSE, ch, this, 0, TO_CHAR);
        act("$n leans back in $p and relaxes.", TRUE, ch, this, 0, TO_ROOM);
      } else {
        act("You lean back and rest your tired bones on the $o.", 
                 FALSE, ch, this, 0, TO_CHAR);
        act("$n leans back and rests on the $o.", TRUE, ch, this, 0, TO_ROOM);
      }

      ch->setPosition(POSITION_RESTING);
      if (ch->isPc())
        start_task(ch, 0, 0, TASK_REST, "", 350, 0, 1, 0, 2 * ch->regenTime());
      if (ch->riding != this)
        ch->mount(this);
      break;
    case POSITION_RESTING:
      act("You are already resting.", FALSE, ch, this, 0, TO_CHAR);
      break;
    case POSITION_SLEEPING:
      act("You have to wake up first.", FALSE, ch, this, 0, TO_CHAR);
      break;
    case POSITION_MOUNTED:
      ch->sendTo("Not while riding you don't!\n\r");
      break;
    default:
      act("You stop floating around, and stop to rest your tired bones on the $o.",
               FALSE, ch, this, 0, TO_CHAR);
      act("$n stops floating around, and rests on the $o.", 
               FALSE, ch, this, 0, TO_ROOM);
      ch->setPosition(POSITION_RESTING);
      if (ch->isPc())
        start_task(ch, 0, 0, TASK_REST, "", 350, 0, 1, 0, 2 * ch->regenTime());
      if (ch->riding != this)
        ch->mount(this);
      break;
  }
}

void TBed::sleepMe(TBeing *ch)
{
  if (getMinPosUse() > 0) {
    ch->sendTo("That object isn't designed to be slept in.\n\r");
    return;
  }
  if ((getNumRiders(ch) + 3) > getMaxRiders()) {
    act("There isn't enough room on $p to sleep right now.",
          FALSE, ch, this, 0, TO_CHAR);
    return;
  }
  if (ch->rider) {
    ch->sendTo("You can't sleep on that right now.\n\r");
    return;
  }
  switch (ch->getPosition()) {
    case POSITION_CRAWLING:
      act("You crawl into the $o and go to sleep.",
          TRUE,ch,this,0,TO_CHAR);
      act("$n crawls into the $o and goes to sleep.",
          TRUE,ch,this,0,TO_ROOM);
      ch->setPosition(POSITION_SLEEPING);
      if (ch->isPc())
        start_task(ch, 0, 0, TASK_SLEEP, "", 350, 0, 1, 0, ch->regenTime());
      if (ch->riding != this)
        ch->mount(this);
      break;
    case POSITION_STANDING:
    case POSITION_RESTING:
      act("You lie down on the $o and go to sleep.",TRUE,ch,this,0,TO_CHAR);
      act("$n lies down and falls asleep in the $o.", TRUE, ch,this, 0, TO_ROOM)
;
      ch->setPosition(POSITION_SLEEPING);
      if (ch->isPc())
        start_task(ch, 0, 0, TASK_SLEEP, "", 350, 0, 1, 0, ch->regenTime());
      if (ch->riding != this)
        ch->mount(this);
      break;
    case POSITION_SITTING:
      act("You lean back on the $o and go to sleep.",TRUE,ch,this,0,TO_CHAR);
      act("$n leans back and falls asleep in the $o.",
                TRUE, ch,this, 0, TO_ROOM);
      ch->setPosition(POSITION_SLEEPING);
      if (ch->isPc())
        start_task(ch, 0, 0, TASK_SLEEP, "", 350, 0, 1, 0, ch->regenTime());
      if (ch->riding != this)
        ch->mount(this);
      if (ch->checkBlackjack())
        gBj.exitGame(ch);
      break;
    case POSITION_SLEEPING:
      ch->sendTo("You are already sound asleep.\n\r");
      break;
    case POSITION_MOUNTED:
      ch->sendTo("Not while riding you don't!\n\r");
      break;
    default:
      act("You stop floating around, and lie down in the $o.", 
              FALSE, ch,this, 0, TO_CHAR);
      act("$n stops floating around, and lies down in the $o.",
              TRUE, ch,this, 0, TO_CHAR);
      ch->setPosition(POSITION_SLEEPING);
      if (ch->isPc())
        start_task(ch, 0, 0, TASK_SLEEP, "", 350, 0, 1, 0, ch->regenTime());
      if (ch->riding != this)
        ch->mount(this);
      break;
  }
}

int TBed::mobPulseBed(TMonster *mob, short int occurrence)
{
  int n;
  sstring fullName;
// concatenate name of object with occurrence in room, e.g. 3.chair
  fullName = fmt("%hi%s%s") % occurrence % "." % fname(name);

  if (mob->default_pos <= POSITION_SLEEPING)
    n = 3;
  else if (mob->default_pos <= POSITION_RESTING)
    n = 2;
  else
    n = 1;
  if ((getNumRiders(mob) + n) > getMaxRiders()) {
    return FALSE;
}
  mob->doWake("");
  mob->doStand();
  if (mob->default_pos == POSITION_SITTING ||
       (getMinPosUse() == 2)) {
    mob->doSit(fullName);
  } else if (mob->default_pos == POSITION_RESTING ||
      (getMinPosUse() == 1)) {
    mob->doRest(fullName);
  } else if (mob->default_pos == POSITION_SLEEPING ||
      (getMinPosUse() == 0)) {
    mob->doSleep(fullName);
  }
  return TRUE;
}

bool TBed::canGetMeDeny(const TBeing *ch, silentTypeT silent) const
{
#if 0
  if (!ch->isImmortal()) {
    if (!silent)
      ch->sendTo("Sorry, you aren't meant to be a furniture mover.\n\r");
    return TRUE;
  }
#endif
  return FALSE;
}

int TBed::getMaxRiders() const
{
  return getMaxUsers();
}

int TBed::getRiderHeight() const
{
  return getSeatHeight();
}

void TBed::assignFourValues(int x1, int x2, int x3, int x4)
{
  setMinPosUse(GET_BITS(x1, 7, 4));
  setMaxUsers(GET_BITS(x1, 3, 4));
  setMaxSize(x2);
  setSeatHeight(x3);
  setRegen(x4);
}

void TBed::getFourValues(int *x1, int *x2, int *x3, int *x4) const
{
  int x = 0;

  SET_BITS(x, 3, 4, getMaxUsers());
  SET_BITS(x, 7, 4, getMinPosUse());

  *x1 = x;
  *x2 = getMaxSize();
  *x3 = getSeatHeight();
  *x4 = getRegen();
}

sstring TBed::statObjInfo() const
{
  char buf[256];

  sprintf(buf, "Max Users: %d, Min. Position: %s\n\r",
      getMaxUsers(),
      ((getMinPosUse() == 0 ? "sleeping" :
       (getMinPosUse() == 1 ? "resting" :
       (getMinPosUse() == 2 ? "sitting" : "unusable")))));
  sprintf(buf + strlen(buf), "Max Designed Size: %d inches, Seat height: %d inches\n\rExtra Regen: %d%%",
      getMaxSize(), getSeatHeight(), getRegen());

  sstring a(buf);
  return a;
}

void TBed::lowCheck()
{
  if (canWear(ITEM_TAKE) && getVolume() <= 3000 && getWeight() <= 10 &&
      getRegen() >= 3)
    vlogf(LOG_LOW, fmt("Portable bed (%s) with excessive regen rates!") % 
             getName());

#if 0
  if (canWear(ITEM_TAKE)) {
    vlogf(LOG_LOW, fmt("Bed (%s) set to be portable.") % 
                getName());
  }
#endif

  TObj::lowCheck();
}

