//////////////////////////////////////////////////////////////////////////
//
// SneezyMUD - All rights reserved, SneezyMUD Coding Team
//
// obj_open_container.cc
//
//////////////////////////////////////////////////////////////////////////


#include "stdsneezy.h"
#include "disc_looting.h"
#include "obj_open_container.h"
#include "obj_money.h"

TOpenContainer::TOpenContainer() :
  TBaseContainer(),
  max_weight(0.0),
  container_flags(0),
  trap_type(DOOR_TRAP_NONE),
  trap_dam(0),
  key_num(-1),
  max_volume(0)
{
}

TOpenContainer::TOpenContainer(const TOpenContainer &a) :
  TBaseContainer(a),
  max_weight(a.max_weight),
  container_flags(a.container_flags),
  trap_type(a.trap_type),
  trap_dam(a.trap_dam),
  key_num(a.key_num),
  max_volume(a.max_volume)
{
}

TOpenContainer & TOpenContainer::operator=(const TOpenContainer &a)
{
  if (this == &a) return *this;
  TBaseContainer::operator=(a);
  max_weight = a.max_weight;
  container_flags = a.container_flags;
  trap_type = a.trap_type;
  trap_dam = a.trap_dam;
  key_num = a.key_num;
  max_volume = a.max_volume;
  return *this;
}

TOpenContainer::~TOpenContainer()
{
}

void TOpenContainer::assignFourValues(int x1, int x2, int x3, int x4)
{
  setCarryWeightLimit((float) x1);
  setContainerFlags(GET_BITS(x2, 15, 16));
  setContainerTrapType(mapFileToDoorTrap(GET_BITS(x2, 23, 8)));
  setContainerTrapDam(GET_BITS(x2, 31, 8));
  setKeyNum(x3);
  setCarryVolumeLimit(x4);
}

void TOpenContainer::getFourValues(int *x1, int *x2, int *x3, int *x4) const
{
  *x1 = (int) carryWeightLimit();

  int r = 0;
  SET_BITS(r, 15, 16, getContainerFlags());
  SET_BITS(r, 23, 8, mapDoorTrapToFile(getContainerTrapType()));
  SET_BITS(r, 31, 8, getContainerTrapDam());
  *x2 = r;
  *x3 = getKeyNum();
  *x4 = carryVolumeLimit();
}

sstring TOpenContainer::statObjInfo() const
{
  char buf[256];

  sprintf(buf, "Max Weight :%.3f, Max Volume : %d\n\rTrap type :%s (%d), Trap damage :%d\n\r",
          carryWeightLimit(),
          carryVolumeLimit(),
          trap_types[getContainerTrapType()].c_str(),
          getContainerTrapType(),
          getContainerTrapDam());
  sprintf(buf + strlen(buf), "Vnum of key that opens: %d",
           getKeyNum());

  sstring a(buf);
  return a;
}

bool TOpenContainer::isCloseable() const
{
  return (isContainerFlag(CONT_CLOSEABLE));
}

bool TOpenContainer::isClosed() const
{
  return (isContainerFlag(CONT_CLOSED));
}

void TOpenContainer::setCarryWeightLimit(float r)
{
  max_weight = r;
}

float TOpenContainer::carryWeightLimit() const
{
  return max_weight;
}

int TOpenContainer::getContainerFlags() const
{
  return container_flags;
}

void TOpenContainer::setContainerFlags(int r)
{
  container_flags = r;
}

void TOpenContainer::addContainerFlag(int r)
{
  container_flags |= r;
}

void TOpenContainer::remContainerFlag(int r)
{
  container_flags &= ~r;
}

bool TOpenContainer::isContainerFlag(int r) const
{
  return ((container_flags & r) != 0);
}

doorTrapT TOpenContainer::getContainerTrapType() const
{
  return ((trap_type>=MAX_TRAP_TYPES)?DOOR_TRAP_NONE:trap_type);
}

void TOpenContainer::setContainerTrapType(doorTrapT r)
{
  trap_type = r;
}

char TOpenContainer::getContainerTrapDam() const
{
  return trap_dam;
}

void TOpenContainer::setContainerTrapDam(char r)
{
  trap_dam = r;
}

void TOpenContainer::setKeyNum(int r)
{
  key_num = r;
}

int TOpenContainer::getKeyNum() const
{
  return key_num;
}

int TOpenContainer::carryVolumeLimit() const
{
  return max_volume;
}

void TOpenContainer::setCarryVolumeLimit(int r)
{
  max_volume = r;
}

void TOpenContainer::changeObjValue2(TBeing *ch)
{
  ch->specials.edit = CHANGE_CHEST_VALUE2;
  change_chest_value2(ch, this, "", ENTER_CHECK);
  return;
}

void TOpenContainer::describeContains(const TBeing *ch) const
{
  if (getStuff() && !isClosed())
    ch->sendTo(COLOR_OBJECTS, fmt("%s seems to have something in it...\n\r") %
	       sstring(getName()).cap());
}

void TOpenContainer::lowCheck()
{
  if (carryWeightLimit() <= 0.0) {
    vlogf(LOG_LOW, fmt("Container (%s) with bad weight limit (%5.2f).") % 
            getName() % carryWeightLimit());
  }
  if (carryVolumeLimit() <= 0) {
    vlogf(LOG_LOW, fmt("Container (%s) with bad volume limit (%d).") % 
            getName() % carryVolumeLimit());
  }

  if (isContainerFlag(CONT_TRAPPED)) {
    if (getContainerTrapType() == DOOR_TRAP_NONE) {
      vlogf(LOG_LOW, fmt("Container (%s:%d) trapped with no trap type.  Removing.") % 
           getName() % objVnum());
      remContainerFlag(CONT_TRAPPED);
    }
  }
  TBaseContainer::lowCheck();
}

void TOpenContainer::purchaseMe(TBeing *ch, TMonster *tKeeper, int tCost, int tShop)
{
  TObj::purchaseMe(ch, tKeeper, tCost, tShop);

  // This prevents 'just bought from store' bags from having
  // Ghost Traps unless they are already ghost trapped or have
  // a real trap on them.
  if (!isContainerFlag(CONT_TRAPPED) && !isContainerFlag(CONT_GHOSTTRAP))
    addContainerFlag(CONT_EMPTYTRAP);
}

sstring TOpenContainer::showModifier(showModeT tMode, const TBeing *tBeing) const
{
  // recurse if necessary
  sstring tString = TBaseContainer::showModifier(tMode, tBeing);

  if (isCloseable()) {
    tString += " (";                                          
    tString += isClosed() ? "closed" : "opened";
    tString += ")";                                           

    if(!isClosed() && !getStuff()){
      tString += " (empty)";
    }

  }                                                           
                                                              
  return tString;                                             
}                                                             

void TOpenContainer::putMoneyInto(TBeing *ch, int amount)
{
  TMoney * money;

  if (isClosed()) {
    act("$p is closed.", FALSE, ch, this, 0, TO_CHAR);
    return;
  }
  ch->sendTo("OK.\n\r");

  act("$n puts some money into $p.", FALSE, ch, this, 0, TO_ROOM);
  money  = create_money(amount);
  *this += *money;

  ch->addToMoney(-amount, GOLD_INCOME);
  if (ch->fight())
    ch->addToWait(combatRound(1 + amount/5000));
  ch->doSave(SILENT_YES);
}

int TOpenContainer::openMe(TBeing *ch)
{
  char buf[256];

  if (!isClosed()) {
    ch->sendTo("But it's already open!\n\r");
    return FALSE;
  } else if (!isCloseable() && !isClosed()) {
    ch->sendTo("You can't do that.\n\r");
    return FALSE;
  } else if (isContainerFlag(CONT_LOCKED)) {
    ch->sendTo("It seems to be locked.\n\r");
    return FALSE;                                             
  } else if (isContainerFlag(CONT_TRAPPED) ||                 
             !isContainerFlag(CONT_EMPTYTRAP) ||              
             isContainerFlag(CONT_GHOSTTRAP)) {               
    if (ch->doesKnowSkill(SKILL_DETECT_TRAP)) {               
      if (detectTrapObj(ch, this) || isContainerFlag(CONT_GHOSTTRAP)) {      
        sprintf(buf, "You start to open $p, but then notice an insidious %s trap...",
              sstring(trap_types[getContainerTrapType()]).uncap().c_str());
        act(buf, TRUE, ch, this, NULL, TO_CHAR);

        return FALSE;
      } else if (!isContainerFlag(CONT_TRAPPED) &&
                 !ch->bSuccess(SKILL_DETECT_TRAP)) {
        setContainerTrapType(doorTrapT(::number((DOOR_TRAP_NONE + 1), (MAX_TRAP_TYPES - 1))));
        setContainerTrapDam(0);
        addContainerFlag(CONT_GHOSTTRAP);

        sprintf(buf, "You start to open $p, but then notice an insidious %s trap...",
                sstring(trap_types[getContainerTrapType()]).uncap().c_str());
        act(buf, TRUE, ch, this, NULL, TO_CHAR);

        return FALSE;
      }
    }

    remContainerFlag(CONT_CLOSED);
    remContainerFlag(CONT_GHOSTTRAP);
    addContainerFlag(CONT_EMPTYTRAP);


    if (spec) {
      int res = 0;
      int rc = checkSpec(ch, CMD_OBJ_OPENED, NULL, NULL);
      if (IS_SET_ONLY(rc, DELETE_THIS))
	res |= DELETE_ITEM;
      if (IS_SET_ONLY(rc, DELETE_VICT)) {
	res |= DELETE_VICT;
	return res;
      }
      if (rc)
	return res;
    }

    act("You open $p.", TRUE, ch, this, NULL, TO_CHAR);
    act("$n opens $p.", TRUE, ch, this, 0, TO_ROOM);



    if (isContainerFlag(CONT_TRAPPED)) {
      int rc = ch->triggerContTrap(this);
      int res = 0;
      if (IS_SET_DELETE(rc, DELETE_ITEM))
        ADD_DELETE(res, DELETE_THIS);

      if (IS_SET_DELETE(rc, DELETE_THIS))
        ADD_DELETE(res, DELETE_VICT);

      return res;
    }

    return TRUE;
  } else {
    remContainerFlag(CONT_CLOSED);
    remContainerFlag(CONT_GHOSTTRAP);
    addContainerFlag(CONT_EMPTYTRAP);

    if (spec) {
      int res = 0;
      int rc = checkSpec(ch, CMD_OBJ_OPENED, NULL, NULL);
      if (IS_SET_ONLY(rc, DELETE_THIS))
        res |= DELETE_ITEM;
      if (IS_SET_ONLY(rc, DELETE_VICT)) {
        res |= DELETE_VICT;
        return res;
      }
      if (rc)
        return res;
    }

    act("You open $p.", TRUE, ch, this, NULL, TO_CHAR);
    act("$n opens $p.", TRUE, ch, this, 0, TO_ROOM);


    return TRUE;
  }
}

int TOpenContainer::putSomethingInto(TBeing *ch, TThing *obj)
{
  int rc = obj->putSomethingIntoContainer(ch, this);

  int ret = 0;
  if (IS_SET_DELETE(rc, DELETE_THIS))
    ret |= DELETE_ITEM;
  if (IS_SET_DELETE(rc, DELETE_ITEM))
    ret |= DELETE_THIS;
  if (IS_SET_DELETE(rc, DELETE_VICT))
    ret |= DELETE_VICT;

  return ret;
}

bool TOpenContainer::getObjFromMeCheck(TBeing *ch)
{
  if (isClosed()) {
    act("$P must be opened first.", 1, ch, 0, this, TO_CHAR);
    return TRUE;
  }
  return FALSE;
}

sstring TOpenContainer::compareMeAgainst(TBeing *ch, TObj *tObj)
{
  const char * sizeLevels[] =
  {
    " has a great amount of space more than ",
    " has a lot more space than ",
    " has a little bit more space than ",
    " has the same amount of space as ",
    " has a little less space than ",
    " has a lot less space than ",
    " has a great amount less space compared to "
  };

  const char *weightLevels[] =
  {
    " can hold a great amount of weight over ",
    " can hold a lot more weight than ",
    " can hold a little more weight than ",
    " can hold the same amount of weight as ",
    " can hold less weight than ",
    " can hold a lot less weight than ",
    " can hold a great amount less weight compared to ",
  };

  TOpenContainer *tOpenContainer = NULL;

  if (!tObj)
    return "Could not find other item to compare.\n\r";

  if ((itemType() != tObj->itemType()) ||
      !(tOpenContainer = dynamic_cast<TOpenContainer *>(tObj)))
    return "These two items cannot be compared against one another.\n\r";

  int    tSize1   = carryVolumeLimit(),
         tSize2   = tOpenContainer->carryVolumeLimit(),
         tSizeDiff,
         tMessage1,
         tWeight1 = (int)carryWeightLimit(),
         tWeight2 = (int)tOpenContainer->carryWeightLimit(),
         tWeightDiff,
         tMessage2;
  sstring StString("");

  tSizeDiff   = (tSize1 - tSize2);
  tMessage1   = compareDetermineMessage(15, tSizeDiff);
  tWeightDiff = (tWeight1 - tWeight2);
  tMessage2   = compareDetermineMessage(15, tWeightDiff);

  StString += sstring(getName()).cap();
  StString += sizeLevels[tMessage1];
  StString += tOpenContainer->getName();
  StString += ".\n\r";

  StString += sstring(getName()).cap();
  StString += weightLevels[tMessage2];
  StString += tOpenContainer->getName();
  StString += ".\n\r";

  return StString;
}

void TOpenContainer::lookObj(TBeing *ch, int bits) const
{
  if (isClosed()) {
    ch->sendTo("It is closed.\n\r");
    return;
  }

  ch->sendTo(fname(name));
  switch (bits) {
    case FIND_OBJ_INV:
      ch->sendTo(" (carried) : ");
      break;
    case FIND_OBJ_ROOM:
      ch->sendTo(" (here) : ");
      break;
    case FIND_OBJ_EQUIP:
      ch->sendTo(" (used) : ");
      break;
  }
  if (carryVolumeLimit() && carryWeightLimit()) {
    // moneypouches are occasionally overfilled, so we will just force the
    // info to look right...
    ch->sendTo(fmt("%d%c full, %d%c loaded.\n\r") %
     min(100, getCarriedVolume() * 100 / carryVolumeLimit()) % '%' %
     min(100, (int) (getCarriedWeight() * 100.0 / carryWeightLimit())) % '%');
  } else {
    vlogf(LOG_BUG, fmt("Problem in look in for object: (%s:%d), check vol/weight limit") %  getName() % objVnum());
  }
  list_in_heap(getStuff(), ch, 0, 100);

  // list_in_heap uses sequential sendTo's, so lets sstring it to them for
  // easier browsing
  ch->makeOutputPaged();
}

int TOpenContainer::trapMe(TBeing *ch, const char *trap_type)
{
  char buf[256];

  if (!isCloseable()) {
    act("$p must be closeable to be trapped.", FALSE, ch, this, 0, TO_CHAR);
    return FALSE;
  }
  if (!isClosed()) {
    act("$p must be closed before you may trap it.", FALSE, ch, this, 0, TO_CHAR);
    return FALSE;
  }
  if (isContainerFlag(CONT_TRAPPED)) {
    if (ch->doesKnowSkill(SKILL_DETECT_TRAP)) {
      if (detectTrapObj(ch, this)) {
        sprintf(buf, "You start to trap $p, but then notice an insidious %s trap already present.",
           sstring(trap_types[getContainerTrapType()]).uncap().c_str());
        act(buf, TRUE, ch, this, NULL, TO_CHAR);
        return FALSE;
      }
    }
    int rc = ch->triggerContTrap(this);
    if (IS_SET_DELETE(rc, DELETE_ITEM | DELETE_THIS)) {
      return DELETE_THIS | DELETE_VICT;
    }
    if (IS_SET_DELETE(rc, DELETE_ITEM)) {
      return DELETE_THIS;
    }
    if (IS_SET_DELETE(rc, DELETE_THIS))
      return DELETE_VICT;
  }

  doorTrapT type;
  if (is_abbrev(trap_type, "fire")) {
    type = DOOR_TRAP_FIRE;
  } else if (is_abbrev(trap_type, "explosive")) {
    type = DOOR_TRAP_TNT;
  } else if (is_abbrev(trap_type, "poison")) {
    type = DOOR_TRAP_POISON;
  } else if (is_abbrev(trap_type, "sleep")) {
    type = DOOR_TRAP_SLEEP;
  } else if (is_abbrev(trap_type, "acid")) {
    type = DOOR_TRAP_ACID;
  } else if (is_abbrev(trap_type, "spore")) {
    type = DOOR_TRAP_DISEASE;
  } else if (is_abbrev(trap_type, "spike")) {
    type = DOOR_TRAP_SPIKE;
  } else if (is_abbrev(trap_type, "blade")) {
    type = DOOR_TRAP_BLADE;
  } else if (is_abbrev(trap_type, "pebble")) {
    type = DOOR_TRAP_PEBBLE;
  } else if (is_abbrev(trap_type, "frost")) {
    type = DOOR_TRAP_FROST;
  } else if (is_abbrev(trap_type, "teleport")) {
    type = DOOR_TRAP_TELEPORT;
  } else if (is_abbrev(trap_type, "power")) {
    type = DOOR_TRAP_ENERGY;
  } else {
    ch->sendTo("No such container trap-type.\n\r");
    ch->sendTo("Syntax: trap container <item> <trap-type>\n\r");
    return FALSE;
  }
  if (!ch->hasTrapComps(trap_type, TRAP_TARG_CONT, 0)) {
    ch->sendTo("You need more items to make that trap.\n\r");
    return FALSE;
  }
  if (ch->getContainerTrapLearn(type) <= 0) {
    ch->sendTo("You need more training before setting a container trap.\n\r");
    return FALSE;
  }


  ch->sendTo("You start working on your trap.\n\r");
  act("$n starts fiddling with $p.", TRUE, ch, this, 0, TO_ROOM);
  start_task(ch, this, NULL, TASK_TRAP_CONT, trap_type, 3, ch->inRoom(), type, 0, 5);
  return FALSE;
}

int TOpenContainer::disarmMe(TBeing *thief)
{
  int learnedness;
  int rc;
  char buf[256], trap_type_buf[80];
  int bKnown = thief->getSkillValue(SKILL_DISARM_TRAP);

  if (isContainerFlag(CONT_GHOSTTRAP)) {
    act("$p isn't trapped after all, must have made a mistake...",
        FALSE, thief, this, 0, TO_CHAR);
    remContainerFlag(CONT_GHOSTTRAP);
    addContainerFlag(CONT_EMPTYTRAP);

    return TRUE;
  }

  if (!isContainerFlag(CONT_TRAPPED)) {
    act("$p is not trapped.", FALSE, thief, this, 0, TO_CHAR);
    return TRUE;
  }

  strcpy(trap_type_buf, trap_types[getContainerTrapType()].c_str());
  learnedness = min((int) MAX_SKILL_LEARNEDNESS, 3*bKnown/2);
  addContainerFlag(CONT_EMPTYTRAP);

  if (thief->bSuccess(learnedness, SKILL_DISARM_TRAP)) {
    sprintf(buf, "Click.  You disarm the %s trap in the $o.", trap_type_buf);
    act(buf, FALSE, thief, this, 0, TO_CHAR);
    act("$n disarms $p.", FALSE, thief, this, 0, TO_ROOM);
    remContainerFlag( CONT_TRAPPED);

    return TRUE;
  } else {
    thief->sendTo("Click. (whoops)\n\r");
    act("$n tries to disarm $p.", FALSE, thief, this, 0, TO_ROOM);
    rc = thief->triggerContTrap(this);
    if (IS_SET_DELETE(rc, DELETE_THIS)) {
      return DELETE_VICT;
    }
    return TRUE;
  }
}

int TOpenContainer::detectMe(TBeing *thief) const
{
  int bKnown =  thief->getSkillValue(SKILL_DETECT_TRAP);

  if (!isContainerFlag( CONT_TRAPPED))
    return FALSE;

  // opening a trapped item
  if (thief->bSuccess(bKnown, SKILL_DETECT_TRAP)) {
    CS(SKILL_DETECT_TRAP);
    return TRUE;
  } else {
    CF(SKILL_DETECT_TRAP);
    return FALSE;
  }
}

void TOpenContainer::pickMe(TBeing *thief)
{
  if (!isContainerFlag( CONT_CLOSED)) {
    act("$p: Silly - it ain't even closed!", false, thief, this, 0, TO_CHAR);
    return;
  }
  if (getKeyNum() < 0) {
    thief->sendTo("Odd - you can't seem to find a keyhole.\n\r");
    return;
  }
  if (!isContainerFlag( CONT_LOCKED)) {
    thief->sendTo("Oho! This thing is NOT locked!\n\r");
    return;
  }
  if (isContainerFlag( CONT_PICKPROOF)) {
    thief->sendTo("It resists your attempts at picking it.\n\r");
    return;
  }

  int bKnown = thief->getSkillValue(SKILL_PICK_LOCK);

  if (thief->bSuccess(bKnown, SKILL_PICK_LOCK)) {
    remContainerFlag( CONT_LOCKED);
    thief->sendTo("*Click*\n\r");
    act("$n fiddles with $p.", FALSE, thief, this, 0, TO_ROOM);
  } else {
    if (critFail(thief, SKILL_PICK_LOCK)) {
      act("Uhoh.  $n seems to have jammed the lock!", 
	  TRUE, thief, 0, 0, TO_ROOM);
      thief->sendTo("Uhoh.  You seemed to have jammed the lock!\n\r");
      addContainerFlag(CONT_PICKPROOF);
    } else {
      thief->sendTo("You fail to pick the lock.\n\r");
    }
  }
}

// THIS, VICT(ch), ITEM(bag)
int TOpenContainer::sellCommod(TBeing *ch, TMonster *keeper, int shop_nr, TThing *)
{
  TThing *t, *t2;
  int rc;
  bool wasClosed = false;

  if (isClosed()) {
    wasClosed = true;
    rc = ch->rawOpen(this);
    if (IS_SET_DELETE(rc, DELETE_ITEM))
      return DELETE_THIS;
    if (IS_SET_DELETE(rc, DELETE_THIS))
      return DELETE_VICT;
  }
  if (isClosed()) {
    // if its still closed, we errored, or it was locked or something
    return FALSE;
  }
  for (t = getStuff(); t; t = t2) {
    t2 = t->nextThing;
    rc = t->sellCommod(ch, keeper, shop_nr, this);
    if (IS_SET_DELETE(rc, DELETE_THIS)) {
      delete t;
      t = NULL;
      continue;
    }
    if (IS_SET_DELETE(rc, DELETE_ITEM)) {
      return DELETE_THIS;
    }
    if (IS_SET_DELETE(rc, DELETE_VICT)) {
      return DELETE_VICT;
    }
  }

  if (wasClosed)
    closeMe(ch);

  return FALSE;
}

void TOpenContainer::closeMe(TBeing *ch)
{
  if (isClosed())
    ch->sendTo("But it's already closed!\n\r");
  else if (!isCloseable())
    ch->sendTo("That's impossible.\n\r");
  else {
    addContainerFlag(CONT_CLOSED);
    act("You close $p.", TRUE, ch, this, 0, TO_CHAR);
    act("$n closes $p.", TRUE, ch, this, 0, TO_ROOM);
  }
}

void TOpenContainer::lockMe(TBeing *ch)
{
  if (!isClosed())
    ch->sendTo("Maybe you should close it first...\n\r");
  else if (getKeyNum() < 0)
    ch->sendTo("That thing can't be locked.\n\r");
  else if (!has_key(ch, getKeyNum()))
    ch->sendTo("You don't seem to have the proper key.\n\r");
  else if (isContainerFlag(CONT_LOCKED))
    ch->sendTo("It is locked already.\n\r");
  else {
    addContainerFlag(CONT_LOCKED);
    ch->sendTo("*Click*\n\r");
    act("$n locks $p - 'cluck', it says.", TRUE, ch, this, 0, TO_ROOM);
  }
}

void TOpenContainer::unlockMe(TBeing *ch)                     
{                                                             
  if (getKeyNum() < 0)                                        
    ch->sendTo("Odd - you can't seem to find a keyhole.\n\r");
  else if (!has_key(ch, getKeyNum()))                         
    ch->sendTo("You don't seem to have the proper key.\n\r"); 
  else if (!isContainerFlag(CONT_LOCKED))                     
    ch->sendTo("Oh.. it wasn't locked, after all.\n\r");      
  else {                                                      
    remContainerFlag(CONT_LOCKED);                            
    ch->sendTo("*Click*\n\r");                                
    act("$n unlocks $p.",TRUE, ch, this, 0, TO_ROOM);         
  }                                                           
}                                                             

