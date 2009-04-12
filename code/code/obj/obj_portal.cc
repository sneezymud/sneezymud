//////////////////////////////////////////////////////////////////////////
//
// SneezyMUD - All rights reserved, SneezyMUD Coding Team
//
//////////////////////////////////////////////////////////////////////////


// portal.cc
//

#include "room.h"
#include "handler.h"
#include "obj_portal.h"
#include "extern.h"
#include "person.h"



TPortal::TPortal(const TRoom *rp) :
  TSeeThru(),
  charges(0),
  portal_type(0),
  trap_type(0),
  trap_damage(0),
  portal_state(0),
  portal_key(-1)
{
  sstring buf;

  swapToStrung();

  name = mud_str_dup("portal");
  shortDescr = mud_str_dup("a magic portal");
  buf = format("A portal going to %s is in the room.") % rp->name;
  setDescr(mud_str_dup(buf));
  obj_flags.wear_flags = 0;
  obj_flags.decay_time = 5;
  setWeight(0);
  obj_flags.cost = 1;
  setPortalNumCharges(10);
  setTarget(rp->number);
}



TPortal::TPortal() :
  TSeeThru(),
  charges(0),
  portal_type(0),
  trap_type(0),
  trap_damage(0),
  portal_state(0),
  portal_key(-1)
{
}

TPortal::TPortal(const TPortal &a) :
  TSeeThru(a),
  charges(a.charges),
  portal_type(a.portal_type),
  trap_type(a.trap_type),
  trap_damage(a.trap_damage),
  portal_state(a.portal_state),
  portal_key(a.portal_key)
{
}

TPortal & TPortal::operator=(const TPortal &a)
{
  if (this == &a) return *this;
  TSeeThru::operator=(a);
  charges = a.charges;
  portal_type = a.portal_type;
  trap_type = a.trap_type;
  trap_damage = a.trap_damage;
  portal_state = a.portal_state;
  portal_key = a.portal_key;
  return *this;
}

TPortal::~TPortal()
{
}


void TPortal::assignFourValues(int x1, int x2, int x3, int x4)
{
  TSeeThru::assignFourValues(x1,x2,x3,x4);

  // the bits 0-24 are used by TSeeThru for setting target room
  setPortalNumCharges(GET_BITS(x1, 31, 8));

  setPortalType(GET_BITS(x2, 7, 8));

  setPortalTrapType(GET_BITS(x3, 7, 8));
  setPortalTrapDam(GET_BITS(x3, 23, 16));

  setPortalKey(GET_BITS(x4, 23, 24));
  setPortalFlags(GET_BITS(x4, 31, 8));
}

void TPortal::getFourValues(int *x1, int *x2, int *x3, int *x4) const
{
  // we get some values from the TSeeThru base class
  TSeeThru::getFourValues(x1,x2,x3,x4);

  int r = *x1;
  SET_BITS(r, 31, 8, getPortalNumCharges());
  *x1 = r;

  r = *x2;
  SET_BITS(r, 7, 8, getPortalType());
  *x2 = r;

  r = *x3;
  SET_BITS(r, 7, 8, getPortalTrapType());
  SET_BITS(r, 23, 16, getPortalTrapDam());
  *x3 = r;

  r = *x4;
  SET_BITS(r, 23, 24, getPortalKey());
  SET_BITS(r, 31, 8, getPortalFlags());
  *x4 = r;
}

sstring TPortal::displayFourValues()
{
  char tString[256];
  int  x1,
       x2,
       x3,
       x4;

  getFourValues(&x1, &x2, &x3, &x4);
  sprintf(tString, "Current values : %d %d %d %d\n\r", x1, x2, x3, x4);
  if (x1 & (1 << 23)) {
    x1 &= ~(1 << 23);
    x1 = -(x1);
  }

  sprintf(tString + strlen(tString),
          "Current values : Trg%s[%d] Chrgs[%d] Type[%d] Trap[%d/%d] Key[%d] Flgs[%d]",
          (x1 < 0 ? "Obj" : "Rm"), (x1 < 0 ? -(x1) : x1),
          getPortalNumCharges(), getPortalType(), getPortalTrapType(),
          getPortalTrapDam(), getPortalKey(), getPortalFlags());

  return tString;
}

sstring TPortal::statObjInfo() const
{
  char   tString[256];
  sstring tStString("");

  sprintf(tString, "Portal Destination: %d, Charges: %d, Type: %d\n\r",
          getTarget(), getPortalNumCharges(), getPortalType());
  tStString += tString;
  tStString += "Portal Flags: ";
  tStString += sprintbit(getPortalFlags(), exit_bits);
  sprintf(tString, "\n\rKey: %d, Trap Type: %s, Trap Dam: %d",
          getPortalKey(), trap_types[getPortalTrapType()].c_str(), 
	  getPortalTrapDam());
  tStString += tString;

  return tStString;
}

char TPortal::getPortalNumCharges() const
{
  return charges;
}

void TPortal::setPortalNumCharges(char r)
{
  charges = r;
}

unsigned char TPortal::getPortalType() const
{
  return portal_type;
}

void TPortal::setPortalType(unsigned char r)
{
  portal_type = r;
}

unsigned short TPortal::getPortalFlags() const
{
  return portal_state;
}

void TPortal::setPortalFlags(unsigned short s)
{
  portal_state = s;
}

bool TPortal::isPortalFlag(unsigned short s) const
{
  return ((portal_state & s) != 0); 
}

void TPortal::addPortalFlag(unsigned short s)
{
  portal_state |= s;
}

void TPortal::remPortalFlag(unsigned short s)
{
  portal_state &= ~s;
}

int TPortal::getPortalKey() const
{
  return portal_key;
}

void TPortal::setPortalKey(int r)
{
  portal_key = r;
}

unsigned char TPortal::getPortalTrapType() const
{
  return trap_type;
}

void TPortal::setPortalTrapType(unsigned char r)
{
  trap_type = r;
}

unsigned short TPortal::getPortalTrapDam() const
{
  return trap_damage;
}

void TPortal::setPortalTrapDam(unsigned short r)
{
  trap_damage = r;
}

void TPortal::showMe(TBeing *ch) const
{
  if (isPortalFlag(EX_CLOSED))
    ch->sendTo("It is closed.\n\r");
  else
    ch->sendTo("It seems to lead somewhere...\n\r");
}

void TPortal::closeMe(TBeing *ch)
{
  if (!ch->canSee(this))
    ch->sendTo("Close what?\n\r");
  else {
    if (isPortalFlag(EX_CLOSED))
      ch->sendTo("It's already closed!\n\r");
    else {
      act("$n closes $p.", TRUE, ch, this, NULL, TO_ROOM);
      act("You close $p.", TRUE, ch, this, NULL, TO_CHAR);
      portal_flag_change(this, EX_CLOSED, "%s is closed from the other side.\n\r", SET_TYPE);
    }
  }
}

void TPortal::lockMe(TBeing *ch)
{
  if (!ch->canSee(this))
    ch->sendTo("Lock what?\n\r");
  else {
    if (!isPortalFlag(EX_CLOSED))
      ch->sendTo("You have to close it first, I'm afraid.\n\r");
    else if (!has_key(ch, getPortalKey()))
      ch->sendTo("You don't have the proper key.\n\r");
    else if (isPortalFlag(EX_LOCKED))
      ch->sendTo("It's already locked!\n\r");
    else {
      act("$n locks $p.", TRUE, ch, this, NULL, TO_ROOM);
      portal_flag_change(this, EX_LOCKED, "%s is locked from the other side.\n\r", SET_TYPE);
      ch->sendTo("*Click*\n\r");
    }
  }
}

void TPortal::unlockMe(TBeing *ch)
{
  if (!ch->canSee(this))
    ch->sendTo("Unlock what?\n\r");
  else {
    if (!isPortalFlag(EX_CLOSED))
      ch->sendTo("Heck!  It ain't even closed!\n\r");
    else if (!has_key(ch, getPortalKey()))
      ch->sendTo("You don't have the proper key.\n\r");
    else if (!isPortalFlag(EX_LOCKED))
      ch->sendTo("That's funny... it wasn't even locked!\n\r");
    else {
      act("$n unlocks $p.", TRUE, ch, this, NULL, TO_ROOM);
      portal_flag_change(this, EX_LOCKED, "%s is unlocked from the other side.\n\r", REMOVE_TYPE);
      ch->sendTo("*Click*\n\r");
    }
  }
}

int TPortal::objectDecay()
{

  if (roomp && !roomp->stuff.empty()) {
#if 0
    sendrpf(COLOR_OBJECTS, roomp, "%s flickers out of view.\n\r", getName(.cap()).c_str());
#else
    act("$n flickers out of view.", TRUE, this, 0, 0, TO_ROOM);
#endif
  }
  return DELETE_THIS;
}

void TPortal::changeObjValue1(TBeing *ch)
{
  ch->specials.edit = CHANGE_PORTAL_VALUE1;
  change_portal_value1(ch, this, "", ENTER_CHECK);
}

void TPortal::changeObjValue3(TBeing *ch)
{
  ch->specials.edit = CHANGE_PORTAL_VALUE3;
  change_portal_value3(ch, this, "", ENTER_CHECK);
}

void TPortal::changeObjValue4(TBeing *ch)
{
  ch->specials.edit = CHANGE_PORTAL_VALUE4;
  change_portal_value4(ch, this, "", ENTER_CHECK);
}

// returns DELETE_THIS, DELETE_VICT(ch)
int TPortal::enterMe(TBeing *ch)
{
  TRoom    *rp;
  int       rc,
            isRandom = -1;
  TPerson *tPerson = dynamic_cast<TPerson *>(ch);

  if (isPortalFlag(EX_CLOSED)) {
    ch->sendTo("You can't enter that!  It's closed!\n\r");
    return FALSE;
  }
  if (isPortalFlag(EX_NOENTER)) {
    ch->sendTo("You can't seem to find a way to enter that.\n\r");
    return FALSE;
  }
  if (!(rp = real_roomp(getTarget(&isRandom)))) {
    ch->sendTo("As you start to enter, you glimpse a swirling vortex just beyond.\n\r");
    ch->sendTo("The sheer terror of that chaos prevents you from actually going through.\n\r");
    return FALSE;
  }
  if (rp->getMoblim() &&
      (MobCountInRoom(rp->stuff) >= rp->getMoblim()) && !ch->isImmortal()) {
    act("You attempt to enter $p, but it's like an invisible wall bars your entry.", FALSE, ch, this, NULL, TO_CHAR);
    return FALSE;
  }

  if (isRandom == -1)
    ch->sendTo("You feel strangly pulled in many directions.\n\r");

  if (tPerson && checkOwnersList(tPerson, true)) {
    // This is done by clerics who create portals then logon lower
    // level characters of theirs and use them.  So we are going
    // to do something VERY crual to them for this.

    ch->sendTo("Something goes wrong as you enter the portal and you feel torn through the astral plane!\n\r");
    vlogf(LOG_CHEAT, format("Player using Portal created by other player in same account! (%s)") % 
          ch->getName());
    rc = ch->genericTeleport(SILENT_NO, true);

    if (IS_SET_DELETE(rc, DELETE_THIS))
      return DELETE_VICT;

    return FALSE;
  }

  ch->goThroughPortalMsg(this);
  if (isPortalFlag(EX_TRAPPED)) {
    rc = ch->triggerPortalTrap(this);
    if (IS_SET_DELETE(rc, DELETE_ITEM | DELETE_THIS)) {
      return DELETE_THIS | DELETE_VICT;
    }
    if (IS_SET_DELETE(rc, DELETE_THIS))
      return DELETE_VICT;

    // if it blew up, go no further
    if (IS_SET_DELETE(rc, DELETE_ITEM))
      return FALSE;

    // if we got teleported, go no further
    if (!sameRoom(*ch))
      return FALSE;
  }
  int orig_room = ch->inRoom();
  if (ch->roomp->isFlyingSector() && !rp->isFlyingSector()) {
    if (!ch->affectedBySpell(SPELL_FLY) && !ch->isAffected(AFF_FLYING)) {
      if (ch->isFlying()) {
        ch->sendTo("You stop flying around.\n\r");
        ch->setPosition(POSITION_STANDING);
      }
    }
    TBeing *tbt = dynamic_cast<TBeing *>(ch->riding);
    if (tbt) {
      if (!tbt->affectedBySpell(SPELL_FLY) && !tbt->isAffected(AFF_FLYING)) {
        if (tbt->isFlying()) {
          ch->sendTo("Your mount stops flying around.\n\r");
          tbt->setPosition(POSITION_STANDING);
        }
      }
    }
  }
  --(*ch);
  thing_to_room(ch, getTarget());
  ch->exitFromPortalMsg(this);
  ch->doLook("", CMD_LOOK);
  if (rp->isFlyingSector() && !ch->isFlying()) {
    TBeing *tbr = dynamic_cast<TBeing *>(ch->riding);
    if (tbr && !tbr->isFlying()) {
      tbr->setPosition(POSITION_FLYING);
      ch->sendTo("Without effort, your mount starts to fly around.\n\r");
    } else if (ch->riding && !tbr) {
      dismount(POSITION_FLYING);
      ch->sendTo("Without effort, you start to fly around.\n\r");
    } else if (!isFlying()) {
      ch->sendTo("Without effort, you start to fly around.\n\r");
      ch->setPosition(POSITION_FLYING);
    }
  }
  int iCharges = getPortalNumCharges();
  if (iCharges>=1) {
    if (iCharges == 1) {        /* last use */
      act("Silently, $n fades from view.", TRUE, this, 0, 0, TO_ROOM);
      return DELETE_THIS;
    } else
      setPortalNumCharges(iCharges - 1);
  }

  // and use a charge up on the farside too
  // seems silly, but otherwise one side would close before the other
  TPortal *otherport = findMatchingPortal();
  if (otherport) {
    int iCharges = otherport->getPortalNumCharges();
    if (iCharges>=1) {
      if (iCharges == 1) {        /* last use */
        act("Silently, $n fades from view.", TRUE, otherport, 0, 0, TO_ROOM);
        delete otherport;
      } else
        otherport->setPortalNumCharges(iCharges - 1);
    }
  }

  followData *k, *n;
  for (k = ch->followers; k; k = n) {
    n = k->next;
    if (k->follower->inRoom() == orig_room &&
        k->follower->getPosition() >= POSITION_CRAWLING) {
      act("You follow $N.", FALSE, k->follower, 0, ch, TO_CHAR);
      rc = k->follower->doEnter(NULL, this);
      if (IS_SET_DELETE(rc, DELETE_THIS)) {
        delete k->follower;
        k->follower = NULL;
      }
      if (IS_SET_DELETE(rc, DELETE_ITEM)) {
        return DELETE_THIS;
      }
    }
  }
  return FALSE;
}

TPortal * TPortal::findMatchingPortal() const
{
  TRoom *rp;

  if (!(rp = real_roomp(getTarget()))) {
    vlogf(LOG_BUG, format("Bad portal (%s) with destination to NULL room! %d") % 
          getName() % getTarget());
    return NULL;
  }
  if (inRoom() < 0)
    return NULL;

  TThing *t=NULL;
  for(StuffIter it=rp->stuff.begin();it!=rp->stuff.end() && (t=*it);++it) {
    TPortal *tp = dynamic_cast<TPortal *>(t);
    if (!tp)
      continue;
    if ((tp->getTarget() == inRoom()) && tp != this)
      return tp;
  }
  return NULL;
}

int TPortal::chiMe(TBeing *tLunatic)
{
  int tMana  = ::number(10, 30),
      bKnown = tLunatic->getSkillLevel(SKILL_CHI);

  if (tLunatic->getMana() < tMana) {
    tLunatic->sendTo("You lack the chi to do this!\n\r");
    return RET_STOP_PARSING;
  } else
    tLunatic->reconcileMana(TYPE_UNDEFINED, 0, tMana);

  if (!tLunatic->bSuccess(bKnown, SKILL_CHI) | obj_flags.decay_time <= 0) {
    act("You fail to affect $p in any way.",
        FALSE, tLunatic, this, NULL, TO_CHAR);
    return true;
  }

  act("You focus upon $p causing it to shimmer out of existance!",
      FALSE, tLunatic, this, NULL, TO_CHAR);
  act("$n concentrates upon $p, causing it to vanish!",
      TRUE, tLunatic, this, NULL, TO_ROOM);

  return DELETE_VICT;
}
