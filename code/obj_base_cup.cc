//////////////////////////////////////////////////////////////////////////
//
// SneezyMUD - All rights reserved, SneezyMUD Coding Team
//
// $Log: obj_base_cup.cc,v $
// Revision 5.1  1999/10/16 04:31:17  batopr
// new branch
//
// Revision 1.1  1999/09/12 17:24:04  sneezy
// Initial revision
//
//
//////////////////////////////////////////////////////////////////////////


// base_cup.cc
// An abstract class to model drink containers, pools, vials, etc upon

#include "stdsneezy.h"

TBaseCup::TBaseCup() :
  TObj(),
  maxDrinks(0),
  curDrinks(0),
  liquidType(MIN_DRINK_TYPES), 
  drinkFlags(0)
{
}

TBaseCup::TBaseCup(const TBaseCup &a) :
  TObj(a),
  maxDrinks(a.maxDrinks),
  curDrinks(a.curDrinks),
  liquidType(a.liquidType),
  drinkFlags(a.drinkFlags)
{
}

TBaseCup & TBaseCup::operator=(const TBaseCup &a)
{
  if (this == &a) return *this;
  TObj::operator=(a);
  maxDrinks = a.maxDrinks;
  curDrinks = a.curDrinks;
  liquidType = a.liquidType;
  drinkFlags = a.drinkFlags;
  return *this;
}

TBaseCup::~TBaseCup()
{
}

unsigned int TBaseCup::getDrinkConFlags() const
{
  return drinkFlags;
}

void TBaseCup::setDrinkConFlags(unsigned int r)
{
  drinkFlags = r;
}

bool TBaseCup::isDrinkConFlag(unsigned int r) const
{
  return ((drinkFlags & r) != 0);
}

void TBaseCup::addDrinkConFlags(unsigned int r)
{
  drinkFlags |= r;
}

void TBaseCup::remDrinkConFlags(unsigned int r)
{
  drinkFlags &= ~r;
}

int TBaseCup::getMaxDrinkUnits() const
{
  return maxDrinks;
}

void TBaseCup::setMaxDrinkUnits(int n)
{
  maxDrinks = n;
}

void TBaseCup::addToMaxDrinkUnits(int n)
{
  maxDrinks += n;
}

int TBaseCup::getDrinkUnits() const
{
  return curDrinks;
}

void TBaseCup::setDrinkUnits(int n)
{
  curDrinks = n;
}

void TBaseCup::addToDrinkUnits(int n)
{
  curDrinks += n;
}

liqTypeT TBaseCup::getDrinkType() const
{
  return liquidType;
}

void TBaseCup::setDrinkType(liqTypeT n)
{
  liquidType = n;
}

int TBaseCup::getLiqDrunk() const
{
  return DrinkInfo[liquidType]->drunk;
}

int TBaseCup::getLiqHunger() const
{
  return DrinkInfo[liquidType]->hunger;
}

int TBaseCup::getLiqThirst() const
{
  return DrinkInfo[liquidType]->thirst;
}

void TBaseCup::fillMe(const TBeing *ch, liqTypeT liq)
{
 int water;

 if ((getDrinkType() != liq) && (getDrinkUnits() != 0)) {
    setDrinkType(LIQ_SLIME);
    act("$p is filled (but you won't like what it's filled with!)",
         FALSE, ch, this, 0, TO_CHAR);
  } else {
    if ((water = (getMaxDrinkUnits() - getDrinkUnits())) > 0) {
      setDrinkType(liq);
      addToDrinkUnits(water);
      weightChangeObject( water * SIP_WEIGHT);
      act("$p is filled.", FALSE, ch, this, 0, TO_CHAR);
    } else {
      ch->sendTo("That is already completely full!\n\r");
    }
  }
}

void TBaseCup::genericEmpty()
{
  weightChangeObject( -(getDrinkUnits() * SIP_WEIGHT));
  setDrinkUnits(0);
  setDrinkType(LIQ_WATER);
  remDrinkConFlags(DRINK_POISON);
}

void TBaseCup::pourMeOut(TBeing *ch)
{
  if (!getDrinkUnits()) {
    act("$p is empty.", FALSE, ch, this, 0, TO_CHAR);
    return;
  }
  act("$n empties $p.", TRUE, ch, this, 0, TO_ROOM);
  act("You empty $p.", FALSE, ch, this, 0, TO_CHAR);

  ch->dropPool(getDrinkUnits(), getDrinkType());
  genericEmpty();
}

void TBaseCup::lowCheck()
{
  if (getMaxDrinkUnits() < getDrinkUnits())
    vlogf(LOW_ERROR,"drinkcon %s  maxdrinks < current drinks.",
         getName());
  if (isDrinkConFlag(DRINK_PERM) && canWear(ITEM_TAKE))
    vlogf(LOW_ERROR,"drinkcon %s  takeable and permanent container.",
         getName());

  TObj::lowCheck();
}

bool TBaseCup::objectRepair(TBeing *ch, TMonster *repair, silentTypeT silent)
{
  if (!silent) {
    char buf[256];
    sprintf(buf, "%s you might wanna take that to the diner!", fname(ch->name).c_str());
    repair->doTell(buf);
  }
  return TRUE;
}

void TBaseCup::assignFourValues(int x1, int x2, int x3, int x4)
{
  setMaxDrinkUnits(x1);
  setDrinkUnits(x2);
  setDrinkConFlags((unsigned) x4);

  if (x3 < MIN_DRINK_TYPES || x3 >= MAX_DRINK_TYPES) {
    vlogf(5, "Bad parm (%d) for drink type on %s", x3, getName());
    x3 = MIN_DRINK_TYPES;
  }
  setDrinkType(liqTypeT(x3));
}

void TBaseCup::getFourValues(int *x1, int *x2, int *x3, int *x4) const
{
  *x1 = getMaxDrinkUnits();
  *x2 = getDrinkUnits();
  *x3 = getDrinkType();
  *x4 = getDrinkConFlags();
}

int TBaseCup::objectSell(TBeing *ch, TMonster *keeper)
{
  char buf[256];

  sprintf(buf, "%s I'm sorry, I don't purchase drink containers.", ch->getName());
  keeper->doTell(buf);
  return TRUE;
}

void TBaseCup::weightCorrection()
{
  float wgt = obj_index[getItemIndex()].weight;  // base weight
  wgt += (SIP_WEIGHT * getDrinkUnits());  // weight of liquid
  setWeight(wgt);
}

string TBaseCup::statObjInfo() const
{
  char buf[256];

  sprintf(buf, "Max-contains : %d     Contains : %d\n\rPoisoned : %s   Permanent : %s   Spillable : %s\n\rLiquid : %s",
          getMaxDrinkUnits(), getDrinkUnits(),
          (isDrinkConFlag(DRINK_POISON) ? "true" : "false"),
          (isDrinkConFlag(DRINK_PERM) ? "true" : "false"),
          (isDrinkConFlag(DRINK_SPILL) ? "true" : "false"),
          DrinkInfo[getDrinkType()]->name);

  string a(buf);
  return a;
}

void TBaseCup::peeMe(const TBeing *ch)
{
  act("$n smiles happily as $e pisses into $p.", TRUE, ch, this, NULL, TO_ROOM);
  act("You smile happily as you piss into $p.", TRUE, ch, this, NULL, TO_CHAR);

  fillMe(ch, LIQ_LEMONADE);
}

bool TBaseCup::poisonObject()
{
  addDrinkConFlags(DRINK_POISON);
  return TRUE;
}

void TBaseCup::setEmpty()
{
  int amt = getDrinkUnits();
  setDrinkUnits(0);
  weightChangeObject(-amt * SIP_WEIGHT);
}

bool TBaseCup::waterSource()
{
  if (isDrinkConFlag(DRINK_PERM))
    return TRUE;
  return FALSE;
}

void TBaseCup::nukeFood()
{
  genericEmpty();
}

void TBaseCup::evaporate(TBeing *ch)
{
  if (!isDrinkConFlag(DRINK_PERM)) {
    if (getDrinkUnits() > 0) {   // has liquid
      int amount = 1 + (getDrinkUnits()/2);
      weightChangeObject(-amount * SIP_WEIGHT);
      addToDrinkUnits(-amount);
      ch->sendTo(COLOR_OBJECTS, "The desert heat causes some of your %s to evaporate.\n\r",
          DrinkInfo[getDrinkType()]->name);
    }
  }
}

int TBaseCup::quaffMe(TBeing *ch)
{
  return drinkMe(ch);
}

void TBaseCup::tasteMe(TBeing *ch)
{
  sipMe(ch);
}

void TBaseCup::spill(const TBeing *ch)
{
  int num, cur;

  if (!isDrinkConFlag(DRINK_SPILL))
    return;

  num = cur = getDrinkUnits();
  num *= 9;
  num /= 10;

  if (cur != num) {
    act("Your $o spills.", FALSE, ch, this, 0, TO_CHAR);
    setDrinkUnits(num);
  }
}

void TBaseCup::pourMeIntoDrink1(TBeing *ch, TObj *to_obj)
{
  to_obj->pourMeIntoDrink2(ch, this);
}

int TBaseCup::getReducedVolume(const TThing *) const
{
  return getTotalVolume();
}
