//////////////////////////////////////////////////////////////////////////
//
// SneezyMUD - All rights reserved, SneezyMUD Coding Team
//    "obj_trap.cc" - All functions and routines related to traps 
//
//////////////////////////////////////////////////////////////////////////

#include "stdsneezy.h"
#include "disc_looting.h"
#include "disease.h"
#include "obj_trap.h"

int TTrap::anyTrapCheck(TBeing *ch)
{
  int rc;

  if ((getTrapCharges() > 0)) {
    if (ch->springTrap(this)) {
      act("You hear a strange noise...", TRUE, ch, 0, 0, TO_ROOM);
      act("You hear a strange noise...", TRUE, ch, 0, 0, TO_CHAR);

      rc = ch->triggerTrap(this);
      if (IS_SET_DELETE(rc, DELETE_THIS))
        return DELETE_VICT;
      if (rc)
        return TRUE;
      return FALSE;
    }
  }
  return FALSE;
}

int TTrap::getTrapCheck(TBeing *ch)
{
  int rc;

  if ((isTrapEffectType(TRAP_EFF_OBJECT)) &&
      (getTrapCharges() > 0)) {
    if (ch->springTrap(this)) {
      act("As you start to get $p, you hear a strange noise...",
               FALSE, ch, this, 0, TO_CHAR);
      act("As $n starts to get $p, you hear a strange noise...",
               FALSE, ch, this, 0, TO_ROOM);

      rc = ch->triggerTrap(this);
      if (IS_SET_DELETE(rc, DELETE_THIS))
        return DELETE_VICT;
      if (rc)
        return TRUE;
      return FALSE;
    }
  }
  return FALSE;
}

int TTrap::getTrapDamAmount() const
{
  return dice(getTrapLevel(), 8);
}

// returns DELETE_THIS or false
int TTrap::detonateGrenade()
{
  TThing *t, *t2;
  TRoom *rp;
  TRoom *newR;
  int rc;
  int new_room;

  if (isTrapEffectType(TRAP_EFF_ARMED3)) {
    remTrapEffectType(TRAP_EFF_ARMED3);
    addTrapEffectType(TRAP_EFF_ARMED2);
    return FALSE;
  } else if (isTrapEffectType(TRAP_EFF_ARMED2)) {
    remTrapEffectType(TRAP_EFF_ARMED2);
    addTrapEffectType(TRAP_EFF_ARMED1);
    return FALSE;
  } else if (!isTrapEffectType(TRAP_EFF_ARMED1)) {
    return FALSE;
  }

  // grenade detonates
  // just for simplicity, move grenade out of anything carrying it and force
  // it to be in room
  TThing *old_parent = parent;   // save this value

  if (!roomp) {
    rp = real_roomp(roomOfObject(this));
    if (!rp) {
      // I think the only way this would happen is if you OEdit the grenade
      return FALSE;
    }

    if (equippedBy) {
      dynamic_cast<TBeing *>(equippedBy)->unequip(eq_pos);
    } else if (stuckIn) {
      int res;
      stuckIn->pulloutObj(eq_stuck, TRUE, &res);
    } else if (riding) {
      positionTypeT new_pos = POSITION_STANDING;
      TBeing *tbt = dynamic_cast<TBeing *>(this);
      if (tbt)
        new_pos = tbt->getPosition();
      dismount(new_pos);
    } else if (parent) {
      --(*this);
    }
    *rp += *this;
  } 
  // don't explode in noheal
  if (roomp->isRoomFlag(ROOM_NO_HEAL)){
    act("A muffled explosion is heard as $n begins to detonate, but is suppressed by a magical field.",
	FALSE, this, 0, 0, TO_ROOM);      
    return DELETE_THIS;
  }

  dirTypeT door;
  for (door = MIN_DIR; door < MAX_DIR; door++) {
    if (canGo(door)) {
      new_room = roomp->dir_option[door]->to_room;
      newR = real_roomp(new_room);
      if (in_room != new_room) {
        sendrpf(newR,"KA-BOOM!!!!  Something has exploded nearby.\n\r");
      }
    }
  }

  switch (getTrapDamType()) {
    case DOOR_TRAP_POISON:
      act("A small canister pops out of $n and detonates.",
              FALSE, this, 0, 0, TO_ROOM);
      break;
    case DOOR_TRAP_SLEEP:
      act("A vaporous fog steams from $n.",
              FALSE, this, 0, 0, TO_ROOM);
      break;
    case DOOR_TRAP_FIRE:
      act("A tiny spark comes out of $n, just before it erupts in flame.",
              FALSE, this, 0, 0, TO_ROOM);
      break;
    case DOOR_TRAP_TELEPORT:
      act("A whirling vortex suddenly surrounds $n.",
              FALSE, this, 0, 0, TO_ROOM);
      break;
    case DOOR_TRAP_DISEASE:
      act("A cloud of spores puffs from $n.",
              FALSE, this, 0, 0, TO_ROOM);
      break;
    case DOOR_TRAP_BOLT:
      act("A canister pops out of $n and detonates, scattering hundreds of sharp, tiny bolts.",
              FALSE, this, 0, 0, TO_ROOM);
      break;
    case DOOR_TRAP_PEBBLE:
      act("A canister pops out of $n and detonates, spraying pebbles everywhere.",
              FALSE, this, 0, 0, TO_ROOM);
      break;
    case DOOR_TRAP_DISK:
      act("A canister pops out of $n and detonates, throwing razor-disks in all directions.",
              FALSE, this, 0, 0, TO_ROOM);
      break;
    case DOOR_TRAP_TNT:
      act("A canister pops out of $n and detonates spraying white hot shrapnel and bomb fragments everywhere.",
              FALSE, this, 0, 0, TO_ROOM);
      break;
    case DOOR_TRAP_FROST:
      act("An icy cloud pours out of $n.",
              FALSE, this, 0, 0, TO_ROOM);
      break;
    case DOOR_TRAP_ENERGY:
      act("$n glows with magic, before streams of plasma streak out of it.",
              FALSE, this, 0, 0, TO_ROOM);
      break;
    case DOOR_TRAP_ACID:
      act("A yellow-green cloud billows out of $n.",
              FALSE, this, 0, 0, TO_ROOM);
      break;
    default:
      act("$n explodes!", FALSE, this, 0, 0, TO_ROOM);
      break;
  }

  TObj *tobj = dynamic_cast<TObj *>(old_parent);
  if (tobj) {
    // grenade was in a bag or something
    TThing *ttt = NULL;
    if ((ttt = tobj->parent) ||
        (ttt = tobj->equippedBy)) {
      act("Your $o is utterly torn apart by the concussion of $N.",
           FALSE, ttt, tobj, this, TO_CHAR, ANSI_RED);
      act("$n's $o is utterly torn apart by the concussion of $N.",
           FALSE, ttt, tobj, this, TO_ROOM, ANSI_RED);
    } else if ((ttt = tobj->roomp)) {
      act("$n is utterly torn apart by the concussion of $p.",
           FALSE, tobj, this, 0, TO_ROOM, ANSI_RED);
    }
    tobj->makeScraps();
    delete tobj;
    tobj = NULL;
  }

  for (t = roomp->getStuff(); t; t = t2) {
    t2 = t->nextThing;
    if (t == this)
      continue;
    rc = t->grenadeHit(this);
    if (IS_SET_DELETE(rc, DELETE_THIS)) {
      delete t;
      t = NULL;
    }
  }
  return DELETE_THIS;
}

void TTrap::changeObjValue2(TBeing *ch)
{
  ch->specials.edit = CHANGE_TRAP_VALUE2;
  change_trap_value2(ch, this, "", ENTER_CHECK);
  return;
}

void TTrap::changeObjValue3(TBeing *ch)
{
  ch->specials.edit = CHANGE_TRAP_VALUE3;
  change_trap_value3(ch, this, "", ENTER_CHECK);
  return;
}

int TTrap::getTrapLevel() const
{
  return trap_level;
}

void TTrap::setTrapLevel(int r)
{
  trap_level = r;
}

int TTrap::getTrapEffectType() const
{
  return trap_effect;
}

void TTrap::setTrapEffectType(int r)
{
  trap_effect = r;
}

bool TTrap::isTrapEffectType(unsigned int r)
{
  return ((trap_effect & r) != 0);
}

void TTrap::remTrapEffectType(unsigned int r)
{
  trap_effect &= ~r;
}

void TTrap::addTrapEffectType(unsigned int r)
{
  trap_effect |= r;
}

doorTrapT TTrap::getTrapDamType() const
{
  return trap_dam_type;
}

void TTrap::setTrapDamType(doorTrapT r)
{
  trap_dam_type = r;
}

int TTrap::getTrapCharges() const
{
  return trap_charges;
}

void TTrap::setTrapCharges(int r)
{
  trap_charges = r;
}

TTrap::TTrap() :
  TObj(),
  trap_level(0),
  trap_effect(0),
  trap_dam_type(DOOR_TRAP_NONE),
  trap_charges(0)
{
}

TTrap::TTrap(const TTrap &a) :
  TObj(a),
  trap_level(a.trap_level),
  trap_effect(a.trap_effect),
  trap_dam_type(a.trap_dam_type),
  trap_charges(a.trap_charges)
{
}

TTrap & TTrap::operator=(const TTrap &a)
{
  if (this == &a) return *this;
  TObj::operator=(a);
  trap_level = a.trap_level;
  trap_effect = a.trap_effect;
  trap_dam_type = a.trap_dam_type;
  trap_charges = a.trap_charges;
  return *this;
}

TTrap::~TTrap()
{
}

void TTrap::assignFourValues(int x1, int x2, int x3, int x4)
{
  setTrapLevel(x1);
  setTrapEffectType(x2);
  setTrapDamType(mapFileToDoorTrap(x3));
  setTrapCharges(x4);
}

void TTrap::getFourValues(int *x1, int *x2, int *x3, int *x4) const
{
  *x1 = getTrapLevel();
  *x2 = getTrapEffectType();
  *x3 = mapDoorTrapToFile(getTrapDamType());
  *x4 = getTrapCharges();
}

sstring TTrap::statObjInfo() const
{
  sstring sbuf, buf;

  buf = fmt("Trap level: %d, damage type: %s (%d), charges: %d\n\r") %
           getTrapLevel() %
           trap_types[getTrapDamType()] %
           getTrapDamType() %
           getTrapCharges();
  sbuf += buf;

  sbuf+="Trap effect type: ";
  
  sbuf += sprintbit(getTrapEffectType(), trap_effects);

  sstring a(sbuf);
  return a;
}

void TTrap::evaluateMe(TBeing *ch) const
{
  int learn;

  learn = ch->getSkillValue(SKILL_EVALUATE);
  ch->learnFromDoingUnusual(LEARN_UNUSUAL_NORM_LEARN, SKILL_EVALUATE, 10);


  if (learn > 10)
    ch->describeTrapEffect(this, learn);

  if (learn > 15) {
    ch->describeTrapLevel(this, learn);
  }

  if (learn > 35) {
    ch->describeTrapCharges(this, learn);
  }

  if (learn > 50) {
    ch->describeTrapDamType(this, learn);
  }
}

void TTrap::armGrenade(TBeing *ch)
{
  addTrapEffectType(TRAP_EFF_ARMED3);

  swapToStrung();
  extraDescription *ed = new extraDescription();
  ed->next = ex_description;
  ex_description = ed;
  ed->keyword = mud_str_dup(GRENADE_EX_DESC);
  ed->description = mud_str_dup(ch->getName());

  // somewhat silly, but here to prevent throwing lots of grenades at
  // a single time.
  // keep the wait-time < 3 or person can't escape a dropped grenade
  ch->addToWait(combatRound(1));

  // run for it!
  TThing *t;
  TMonster *tm;

  if(::number(0,1)){
    for(t=ch->roomp->getStuff(); t; t=t->nextThing){
      if((tm=dynamic_cast<TMonster *>(t))){
	if(tm->canSee(this))
	  tm->doFlee("");
      }
    }
  }
}

int TTrap::throwMe(TBeing *ch, dirTypeT dir, const char *)
{
  // grenade handler
  if (!isTrapEffectType(TRAP_EFF_THROW)) {
    act("You can't throw $p.  It isn't a grenade.",
        FALSE, ch, this, 0, TO_CHAR);
  } else {
    ch->throwGrenade(this, dir);
  }
  return FALSE;
}

