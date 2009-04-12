///////////////////////////////////////////////////////////////////////////
//
//      SneezyMUD++ 4.5 - All rights reserved, SneezyMUD Coding Team
//      "fuel.cc" - Methods for TFuel class
//
//      Last revision December 18, 1997.
//
///////////////////////////////////////////////////////////////////////////

#include "monster.h"
#include "obj_fuel.h"
#include "room.h"
#include "obj_light.h"

TFuel::TFuel() :
  TObj(),
  curFuel(0),
  maxFuel(0)
{
}

TFuel::TFuel(const TFuel &a) :
  TObj(a),
  curFuel(a.curFuel),
  maxFuel(a.maxFuel)
{
}

TFuel & TFuel::operator=(const TFuel &a)
{
  if (this == &a) return *this;

  TObj::operator=(a);
  curFuel = a.curFuel;
  maxFuel = a.maxFuel;
  return *this;
}

TFuel::~TFuel()
{
}

void TFuel::addToMaxFuel(int n)
{
  maxFuel += n;
}

void TFuel::setMaxFuel(int n)
{
  maxFuel = n;
}

int TFuel::getMaxFuel() const
{
  return maxFuel;
}

void TFuel::addToCurFuel(int n)
{
  curFuel += n;
}

void TFuel::setCurFuel(int n)
{
  curFuel = n;
}

int TFuel::getCurFuel() const
{
  return curFuel;
}

void TFuel::describeObjectSpecifics(const TBeing *ch) const
{
  double diff;

  if (getMaxFuel()) {
    diff = (double) ((double) getCurFuel() / (double) getMaxFuel());
    ch->sendTo(COLOR_OBJECTS,
	       format("You can tell that %s has %s of its fuel left.\n\r") %
	       sstring(getName()).uncap() %
          ((diff < .20) ? "very little" : ((diff < .50) ? "some" :
          ((diff < .75) ? "a good bit of" : "almost all of its"))));
  }
}

void TFuel::refuelMeFuel(TBeing *ch, TLight *lamp)
{
  int use;

  if (lamp->getMaxBurn() < 0) {
    act("$p can't be refueled.", FALSE, ch, lamp, 0, TO_CHAR);
    return;
  }
  if (lamp->getCurBurn() == lamp->getMaxBurn()) {
    act("$p is already full of fuel.", FALSE, ch, lamp, 0, TO_CHAR);
    return;
  }
  if (lamp->isLit()) {
    ch->sendTo("You better not fuel that while lit, it might explode.\n\r");
    return;
  }
  use = lamp->getMaxBurn() - lamp->getCurBurn();
  use = min(use, getCurFuel());

  act("$n refuels $s $o.", TRUE, ch, lamp, 0, TO_ROOM);
  ch->sendTo(format("You refuel your %s.\n\r") % fname(lamp->name));

  addToCurFuel(-use);
  lamp->addToCurBurn(use);

  if (getCurFuel() <= 0) {
    ch->sendTo("Your fuel is all used up, and you discard it.\n\r");
    if (equippedBy) {
      dynamic_cast<TBeing *>(equippedBy)->unequip(eq_pos);
    } else
      --(*this);

    delete this;
  }
}

void TFuel::assignFourValues(int x1, int x2, int, int)
{
  setCurFuel(x1);
  setMaxFuel(x2);
}

void TFuel::getFourValues(int *x1, int *x2, int *x3, int *x4) const
{
  *x1 = getCurFuel();
  *x2 = getMaxFuel();
  *x3 = 0;
  *x4 = 0;
}

void TFuel::lowCheck()
{
  if (getCurFuel() > getMaxFuel())
    vlogf(LOG_LOW,format("fuel %s had more current fuel than max.") %  getName());

  TObj::lowCheck();
}

int TFuel::objectSell(TBeing *ch, TMonster *keeper)
{
  keeper->doTell(ch->getName(), "I'm sorry, I don't buy back fuel.");
  return TRUE;
}

sstring TFuel::statObjInfo() const
{
  char buf[256];

  sprintf(buf, "Refuel capability : current : %d, max : %d",
         getCurFuel(), getMaxFuel());

  sstring a(buf);
  return a;
}

int TFuel::getVolume() const
{
  int amt = TObj::getVolume();

  amt *= getCurFuel();
  if(getMaxFuel())
    amt /= getMaxFuel();

  return amt;
}

float TFuel::getTotalWeight(bool pweight) const
{
  float amt = TObj::getTotalWeight(pweight);

  amt *= getCurFuel();

  if(getMaxFuel())
    amt /= getMaxFuel();

  return amt;
}

int TFuel::chiMe(TBeing *tLunatic)
{
  int     tMana  = ::number(10, 30),
          bKnown = tLunatic->getSkillLevel(SKILL_CHI),
          tDamage,
          tRc = DELETE_VICT;
  TThing *tThing;
  TBeing *tBeing;

  if (tLunatic->getMana() < tMana) {
    tLunatic->sendTo("You lack the chi to do this!\n\r");
    return RET_STOP_PARSING;
  } else
    tLunatic->reconcileMana(TYPE_UNDEFINED, 0, tMana);

  if (tLunatic->checkPeaceful("Violent things can not be done here and something tells you that would be violent!"))
    return FALSE;

  if (!roomp) {
    // added to prevent crashes when item is held, etc.
    act("You must be more cautious to chi something so volatile.", FALSE, tLunatic, this, NULL, TO_CHAR);
    return FALSE;
  }
  
  if (!tLunatic->bSuccess(bKnown, SKILL_CHI)) {
    act("You fail to affect $p in any way.",
        FALSE, tLunatic, this, NULL, TO_CHAR);
    return true;
  }

  act("You focus upon $p causing it to blow up violently!",
      FALSE, tLunatic, this, NULL, TO_CHAR);
  act("$n concentrates upon $p, causing it to blow up violently",
      TRUE, tLunatic, this, NULL, TO_ROOM);

  tDamage = max(1, (::number((getCurFuel() / 2), getCurFuel()) / 10));

  for(StuffIter it=roomp->stuff.begin();it!=roomp->stuff.end();){
    tThing=*(it++);

    if (!(tBeing = dynamic_cast<TBeing *>(tThing)) || tBeing->isImmortal())
      continue;

    act("You are struck by the flames, caused by $n!",
        FALSE, tLunatic, NULL, tBeing, TO_VICT);

    if (tBeing->isPc()) {
      if (tBeing->reconcileDamage(tBeing, ::number(1, tDamage), DAMAGE_FIRE) == -1) {
        if (tBeing == tLunatic)
          ADD_DELETE(tRc, (DELETE_THIS | RET_STOP_PARSING));
        else {
          delete tThing;
          tThing = NULL;
        }
      }
    } else {
      if (tLunatic->reconcileDamage(tBeing, ::number(1, tDamage), DAMAGE_FIRE) == -1) {
        delete tThing;
        tThing = NULL;
      }
    }
  }

  return tRc;
}
