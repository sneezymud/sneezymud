//////////////////////////////////////////////////////////////////////////
//
// SneezyMUD - All rights reserved, SneezyMUD Coding Team
//
//      "light.cc" - Methods for TLight class
//
///////////////////////////////////////////////////////////////////////////

#include "stdsneezy.h"
#include "obj_light.h"

TLight::TLight() :
  TBaseLight(),
  lit(0)
{
}

TLight::TLight(const TLight &a) :
  TBaseLight(a),
  lit(a.lit)
{
}

TLight & TLight::operator=(const TLight &a)
{
  if (this == &a) return *this;
  TBaseLight::operator=(a);
  lit = a.lit;
  return *this;
}

TLight::~TLight()
{
}

void TLight::setLit(bool n)
{
  lit = n;
}

bool TLight::isLit() const
{
  return lit;
}

int TLight::illuminateMe(TBeing *caster, int, byte)
{
  caster->sendTo("That object is a light; why don't you just light it?!?\n\r");
  act("Nothing seems to happen.", TRUE, caster, 0, 0, TO_ROOM);
  return SPELL_FAIL;
}

void TLight::putLightOut()
{
  setLit(FALSE);

  TBaseLight::putLightOut();
}

void TLight::unequipMe(TBeing *ch)
{
  genericExtinguish(ch);
}

void TLight::genericExtinguish(const TBeing *ch)
{
  if (isLit()) {
    act("You extinguish $p.", FALSE, ch, this, 0, TO_CHAR);
    putLightOut();
  }
}

void TLight::extinguishWater(TBeing *ch)
{
  if (isLit()) {
    ch->roomp->addToLight(-getLightAmt());
    ch->addToLight(-getLightAmt());
    act("$p is put out by the room's water.", TRUE, ch, this, 0, TO_CHAR);
    act("$p is put out by the room's water.", TRUE, ch, this, 0, TO_ROOM);

    putLightOut();
  }
}

void TLight::extinguishWater()
{
  if (isLit()) {
    roomp->addToLight(-getLightAmt());
    act("$p is put out by the room's water.", TRUE, 0, this, 0, TO_ROOM);
    putLightOut();
  }
}

void TLight::lampLightStuff(TMonster *ch)
{
  // if lamp is on during day, turn off
  if (is_daytime() && isLit()) {
    ch->doExtinguish(fname(name));
  }
  // if off at night, turn on
  // we use !isday rather than isnight so that dawn/twilight will cause
  // lights to be turned on
  if (!is_daytime() && !isLit())  {
    ch->doLight(fname(name));
  }
  // refill if < 90% full.  val1 = max, val2 = current
  if (10*getCurBurn() < 9*getMaxBurn() ) {
    act("$n opens $p and looks inside.", TRUE, ch, this, 0, TO_ROOM);
    ch->doSay("Hmm, looks a bit low.");
    act("$n takes out a supply of fuel.", TRUE, ch, 0, 0, TO_ROOM);
    act("$n reaches high up on the lamppost, and refuels it.",
             TRUE, ch, 0, 0, TO_ROOM);
    setCurBurn(getMaxBurn());
  }
}

void TLight::lightDecay()
{
  if (isLit()) {
    addToCurBurn(-1);
    if (getCurBurn() <= 0) {
      setCurBurn(0);
      putLightOut();

      if (roomp && roomp->getStuff()) {
        act("$n flickers a bit, and then burns out.",
                 FALSE, this, 0, 0, TO_ROOM);
        roomp->addToLight(-getLightAmt());
      } else if (parent) {
        act("$p flickers a bit, and then burns out.",
                FALSE, parent, this, 0, TO_CHAR);
        parent->addToLight(-getLightAmt());
        parent->roomp->addToLight(-getLightAmt());
      } else if (equippedBy) {
        act("$p flickers a bit, and then burns out.",
                FALSE, equippedBy, this, 0, TO_CHAR);
        equippedBy->addToLight(-getLightAmt());
        equippedBy->roomp->addToLight(-getLightAmt());
      }
    } else if (getCurBurn() < 4) {
      if (roomp && roomp->getStuff()) 
        act("$n flickers a bit.", FALSE, this, 0, 0, TO_ROOM);
      else if (parent) 
        act("$p flickers a bit.", FALSE, parent, this, 0, TO_CHAR);
      else if (equippedBy) 
        act("$p flickers a bit.", FALSE, equippedBy, this, 0, TO_CHAR);
    }
  }
}

void TLight::extinguishMe(TBeing *ch)
{
  if (!isLit()) {
    ch->sendTo("That light is already extinguished!\n\r");
    return;
  }
  if (objVnum() == GENERIC_FLARE) {
    sendTo("You jump real high, but your still too far away to do that.\n\r");
    return;
  }
  putLightOut();

  ch->roomp->addToLight(-getLightAmt());
  if (equippedBy == ch)
    ch->addToLight(-getLightAmt());

  act("You extinguish $p, and it smolders slightly before going out.", FALSE, ch, this, 0, TO_CHAR);
  act("$n extinguishes $p, and it smolders slightly before going out.", FALSE, ch, this, 0, TO_ROOM);
  return;
}

void TLight::assignFourValues(int x1, int x2, int x3, int x4)
{
  TBaseLight::assignFourValues(x1,x2,x3,x4);

  setLit(x4);

  // if we are setting the object lit, then add to the light...
  adjustLight();
}

void TLight::getFourValues(int *x1, int *x2, int *x3, int *x4) const
{
  TBaseLight::getFourValues(x1, x2, x3, x4);

  *x4 = isLit();
}

void TLight::lowCheck()
{
  int i;

  for (i=0; i<MAX_OBJ_AFFECT;i++) {
    if (affected[i].location == APPLY_LIGHT) {
      if (!isLit())
        vlogf(LOG_LOW,fmt("item %s was defined apply-light.") % getName());
    }
  }
}

sstring TLight::statObjInfo() const
{
  char buf[256];

  sprintf(buf, "Light: %s (%d), Max fuel: %s%d, Fuel left: %d, Lit? : %s",
          describe_light(getLightAmt()),
          getLightAmt(),
          (getMaxBurn() <= 0 ? "non-refuelable :" : ""),
          getMaxBurn(),
          getCurBurn(),
          (isLit() ? "Yes" : "No"));

  sstring a(buf);
  return a;
}

sstring TLight::showModifier(showModeT, const TBeing *) const
{
  sstring a;

  if (isLit())
    a = " (lit)";
  else
    a = "";

  return a;
}

void TLight::adjustLight()
{
  if (isLit())
    addToLight(getLightAmt());
}

int TLight::getMe(TBeing *ch, TThing *)
{
  // lit objs in inventory do not contribute to getLight()
  // we should not leave it lit in inventory.
  // move to a hand or extinguish
  if (isLit()) {

    // there's a nasty way to abuse this though
    // curse a lantern, put it on ground (how?)
    // wait for mob to scavenge it (does this get called if so?)
    // mob now wielding a cursed lantern = can't hit
    if (!ch->isPc() && isObjStat(ITEM_NODROP)) {
      genericExtinguish(ch);
      return FALSE;
    }

    if (!ch->heldInSecHand() &&
          ch->canUseLimb(ch->getSecondaryHold()) &&
          !ch->isLimbFlags(ch->getSecondaryHold(), PART_TRANSFORMED) &&
          ch->canUseLimb(ch->getSecondaryHand()) &&
          !ch->isLimbFlags(ch->getSecondaryHand(), PART_TRANSFORMED) &&
          // obj-weight <= ch wield weight
          (compareWeights(getWeight(), ch->maxWieldWeight(this, HAND_TYPE_SEC)) != -1)) {
      --(*this);
      ch->equipChar(this, ch->getSecondaryHold());
      act("You hold $p in your secondary hand.", FALSE, ch, this, 0, TO_CHAR);
    } else if (!ch->heldInPrimHand() &&
           ch->canUseLimb(ch->getPrimaryHold()) &&
           !ch->isLimbFlags(ch->getPrimaryHold(), PART_TRANSFORMED) &&
         ch->canUseLimb(ch->getPrimaryHand()) &&
           !ch->isLimbFlags(ch->getPrimaryHand(), PART_TRANSFORMED) &&
           // obj-weight <= ch wield weight
           (compareWeights(getWeight(), ch->maxWieldWeight(this, HAND_TYPE_PRIM)) != -1)) {
      --(*this);
      ch->equipChar(this, ch->getPrimaryHold());
      act("You hold $p in your primary hand.", FALSE, ch, this, 0, TO_CHAR);
    } else {
      genericExtinguish(ch);
    }
  }
  return FALSE;
}

void TLight::describeObjectSpecifics(const TBeing *ch) const
{
  double diff;

  if (!isLit())
    act("$p is not lit.", FALSE, ch, this, 0, TO_CHAR);
  else
    act("$p is lit.", FALSE, ch, this, 0, TO_CHAR);

  if (getMaxBurn() < 0)
    act("You see no way to refuel $p.", FALSE, ch, this, 0, TO_CHAR);
  else {
    diff = (double) ((double) getCurBurn() /
            max(1.0, (double) getMaxBurn()));
    ch->sendTo(COLOR_OBJECTS,fmt("%s looks refuelable.\n\r") % sstring(getName()).cap());
    ch->sendTo(COLOR_OBJECTS, fmt("You can tell that %s has %s fuel left.\n\r") %
	       sstring(getName()).uncap() %
          ((diff == 0) ? "no" :
           ((diff < .20) ? "very little" :
           ((diff < .50) ? "some" :
           ((diff < .75) ? "a good bit of" : "almost all of its")))));
  }
}

bool TLight::isSimilar(const TThing *t) const
{
  const TLight * tl = dynamic_cast<const TLight *>(t);
  if (!tl)
    return false;

  // lit status identical is required
  if (isLit() && !tl->isLit())
    return false;
  else if (!isLit() && tl->isLit())
    return false;

  return TObj::isSimilar(t);
}

void TLight::peeOnMe(const TBeing *ch)
{
  if(isLit()){
    act("$p sputters, sparks then finally relents to $n's downpour of <y>pee<1> and goes out.", TRUE, ch, this, NULL, TO_ROOM);
    act("$p sputters, sparks then finally relents to your downpour of <y>pee<1> and goes out.", TRUE, ch, this, NULL, TO_CHAR);

    putLightOut();
  } else
    act("You try to light $p by peeing on it, but sadly it does not appear to be working.", TRUE, ch, this, NULL, TO_CHAR);
}

void TLight::refuelMeLight(TBeing *ch, TThing *fuel)
{
  fuel->refuelMeFuel(ch, this);
}

int TLight::objectDecay()
{
  if (roomp) {
    act("$p flickers then fades into insignificance.",
         TRUE, roomp->getStuff(), this, 0, TO_CHAR);
    act("$p flickers then fades into insignificance.",
         TRUE, roomp->getStuff(), this, 0, TO_ROOM);
  } else {
    TThing *t = NULL;
    if (parent)
      t = parent;
    else if (equippedBy)
      t = equippedBy;
    else if (stuckIn)
      t = stuckIn;

    if (t) {
      act("Your $o flickers then fades into insignificance.",
         TRUE, t, this, 0, TO_CHAR);
      act("$n's $o flickers then fades into insignificance.",
         TRUE, t, this, 0, TO_ROOM);
    }
  }
  return DELETE_THIS;
}

void TLight::lightMe(TBeing *ch, silentTypeT silent)
{
  int i;
  bool iLit = FALSE;

  if (isLit()) {
    if(!silent)
      act("$p is already lit!", FALSE, ch, this, 0, TO_CHAR);
    return;
  }
  if (ch->roomp->isUnderwaterSector()) {
    if(!silent)
      ch->sendTo("Impossible! You are underwater!\n\r");
    return;
  }
  if (getCurBurn() <= 0) {
    if(!silent)
      act("$p is totally burned out!", FALSE, ch, this, 0, TO_CHAR);
    return;
  }
  for (i = 0; (!iLit && i < MAX_OBJ_AFFECT); i++) {
    if (affected[i].location == APPLY_NONE) {
      affected[i].location = APPLY_LIGHT;
      affected[i].modifier = getLightAmt();
      addToLight(affected[i].modifier);
      setLit(TRUE);
      iLit = TRUE;
    }
  }
  if (iLit == TRUE) {
    ch->roomp->addToLight(getLightAmt());
    if (equippedBy == ch)
      ch->addToLight(getLightAmt());
    if(!silent){
      act("You light $p, and it begins to burn brightly.", FALSE, ch, this, 0, TO_CHAR);
      act("$n lights $p, and it begins to burn brightly.", TRUE, ch, this, 0, TO_ROOM);
    }
    return;
  } else {
    ch->sendTo("Problems lighting object. Tell a god!\n\r");
    vlogf(LOG_BUG, fmt("%s had problems lighting an object.") %  ch->getName());
    return;
  }
}

int TLight::chiMe(TBeing *tLunatic)
{
  int tMana  = ::number(10, 30),
      bKnown = tLunatic->getSkillLevel(SKILL_CHI);

  if (tLunatic->getMana() < tMana) {
    tLunatic->sendTo("You lack the chi to do this!\n\r");
    return RET_STOP_PARSING;
  } else
    tLunatic->reconcileMana(TYPE_UNDEFINED, 0, tMana);

  if (!tLunatic->bSuccess(bKnown, SKILL_CHI)) {
    act("You fail to affect $p in any way.",
        FALSE, tLunatic, this, NULL, TO_CHAR);
    return true;
  }

  if (isLit()) {
      act("You concentrate hard on $p, then clap twice.  It goes out.",
          TRUE, tLunatic, this, NULL, TO_CHAR);
      act("$n knits $s brow in concentration then claps twice causing $p to go out.",
          TRUE, tLunatic, this, NULL, TO_ROOM);

      putLightOut();
  } else {
    if (tLunatic->roomp->isUnderwaterSector() || getCurBurn() <= 0) {
      tLunatic->sendTo("You seem unable to do anything to that.\n\r");
      return FALSE;
    }

    act("You furrow your brow in concentration then clap twice.",
        TRUE, tLunatic, this, NULL, TO_CHAR);
    act("$n furrows $s brow in concentration then claps twice.",
        TRUE, tLunatic, this, NULL, TO_ROOM);
    act("$p springs into light.",
        FALSE, tLunatic, this, NULL, TO_ROOM);

    lightMe(tLunatic, SILENT_YES);
  }

  return true;
}
