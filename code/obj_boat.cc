//////////////////////////////////////////////////////////////////////////
//
// SneezyMUD - All rights reserved, SneezyMUD Coding Team
//
//////////////////////////////////////////////////////////////////////////


// boat.cc

#include "stdsneezy.h"
#include "obj_boat.h"
#include "obj_light.h"


TBoat::TBoat() :
  TObj()
{
}

TBoat::TBoat(const TBoat &a) :
  TObj(a)
{
}

TBoat & TBoat::operator=(const TBoat &a)
{
  if (this == &a) return *this;
  TObj::operator=(a);
  return *this;
}

TBoat::~TBoat()
{
}

void TBoat::usingBoat(int *n)
{
  *n = TRUE;
}

void TBoat::assignFourValues(int, int, int, int)
{
}

void TBoat::getFourValues(int *x1, int *x2, int *x3, int *x4) const
{
  *x1 = 0;
  *x2 = 0;
  *x3 = 0;
  *x4 = 0;
}

sstring TBoat::statObjInfo() const
{
  sstring a("");
  return a;
}

int TBoat::putSomethingInto(TBeing *ch, TThing *tThing)
{
  TLight * tlig = dynamic_cast<TLight *>(tThing);
  if (!tlig) {
    ch->sendTo("You cannot do that.\n\r");
    return 2;
  }

  if (getStuff()) {
    ch->sendTo("There is already something attached to this.\n\r");
    return 2;
  }

  --(*tThing);
  *this += *tThing;

  act("You attach $p to $N.", TRUE, ch, tThing, this, TO_CHAR);
  act("$N attaches $p to $N.", TRUE, ch, tThing, this, TO_ROOM);

  if (!tlig->isLit()) {
    int lightAmt = 0;

    if (ch->roomp)
      lightAmt = ch->roomp->getLight();

    tlig->lightMe(ch, SILENT_NO);

    if (ch->roomp)
      ch->roomp->setLight(lightAmt);
  }

  return 0;
}

int TBoat::getObjFrom(TBeing *ch, const char *tString, const char *)
{
  if (!getStuff()) {
    ch->sendTo("There is nothing in there.\n\r");
    return TRUE;
  }

  if (!tString || !*tString) {
    ch->sendTo("Fine.  But WHAT do you want to get?\n\r");
    return TRUE;
  }

  if (isname(tString, getStuff()->getName())) {
    TThing *tThing = getStuff();
    --(*tThing);
    *ch += *tThing;

    act("You unattach and take $p from $N.", TRUE, ch, tThing, this, TO_CHAR);
    act("$n unattaches and takes $p from $N.", TRUE, ch, tThing, this, TO_ROOM);

     TLight *tlig = dynamic_cast<TLight *>(tThing);
    if (tlig && tlig->isLit()) {
      int lightAmt = 0;

      if (ch->roomp)
        lightAmt = ch->roomp->getLight();

      tlig->extinguishMe(ch);

      if (ch->roomp)
        ch->roomp->setLight(lightAmt);
    }
    return TRUE;
  }

  ch->sendTo("I'm afraid it isn't in there.\n\r");
  return TRUE;
}

int TBoat::getLight() const
{
  return (TThing::getLight() + (getStuff() ? getStuff()->getLight() : 0));
}
