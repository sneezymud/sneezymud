//////////////////////////////////////////////////////////////////////////
//
// SneezyMUD - All rights reserved, SneezyMUD Coding Team
//
//////////////////////////////////////////////////////////////////////////


// base_cup.cc
// An abstract class to model drink containers, pools, vials, etc upon

#include "colorstring.h"
#include "monster.h"
#include "obj_base_cup.h"
#include "liquids.h"

TBaseCup::TBaseCup() :
  TObj(),
  maxDrinks(0),
  curDrinks(0),
  liquidType(MIN_DRINK_TYPES), 
  drinkFlags(0)
{
  updateDesc();
}

TBaseCup::TBaseCup(const TBaseCup &a) :
  TObj(a),
  maxDrinks(a.maxDrinks),
  curDrinks(a.curDrinks),
  liquidType(a.liquidType),
  drinkFlags(a.drinkFlags)
{
  updateDesc();
}

TBaseCup & TBaseCup::operator=(const TBaseCup &a)
{
  if (this == &a) return *this;
  TObj::operator=(a);
  maxDrinks = a.maxDrinks;
  curDrinks = a.curDrinks;
  liquidType = a.liquidType;
  drinkFlags = a.drinkFlags;
  updateDesc();
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
  if(isDrinkConFlag(DRINK_PERM) && curDrinks<=0)
    remDrinkConFlags(DRINK_PERM);
  updateDesc();
}

void TBaseCup::addToDrinkUnits(int n)
{
  curDrinks += n;
  if(isDrinkConFlag(DRINK_PERM) && curDrinks<=0)
    remDrinkConFlags(DRINK_PERM);
  updateDesc();
}

liqTypeT TBaseCup::getDrinkType() const
{
  return liquidType;
}

void TBaseCup::setDrinkType(liqTypeT n)
{
  liquidType = n;
  updateDesc();
}

int TBaseCup::getLiqDrunk() const
{
  return liquidInfo[liquidType]->drunk;
}

int TBaseCup::getLiqHunger() const
{
  return liquidInfo[liquidType]->hunger;
}

int TBaseCup::getLiqThirst() const
{
  return liquidInfo[liquidType]->thirst;
}


sstring TBaseCup::showModifier(showModeT tMode, const TBeing *tBeing) const
{
  // recurse if necessary
  sstring tString = TObj::showModifier(tMode, tBeing);

  if (!getDrinkUnits()){
    tString += " (empty)";                                          
  }                                                           
                                                              
  return tString;                                             
}


void TBaseCup::fillMe(const TBeing *ch, liqTypeT liq)
{
  int water;

  if ((getDrinkType() != liq) && isDrinkConFlag(DRINK_PERM)) {
    sstring mesg;

    mesg = format("$p is filled with %s, but quickly dilutes.") %
      liquidInfo[liq]->name;

    act(mesg, FALSE, ch, this, 0, TO_ROOM);
    act(mesg, FALSE, ch, this, 0, TO_CHAR);
  } else if ((getDrinkType() != liq) && (getDrinkUnits() != 0)) {
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
  if(!isDrinkConFlag(DRINK_PERM)){
    weightChangeObject( -(getDrinkUnits() * SIP_WEIGHT));
    setDrinkUnits(0);
    setDrinkType(LIQ_WATER);
    remDrinkConFlags(DRINK_POISON);
  }
}

void TBaseCup::pourMeOut(TBeing *ch)
{
  if (!getDrinkUnits()) {
    act("$p is empty.", FALSE, ch, this, 0, TO_CHAR);
    return;
  }
  if(isDrinkConFlag(DRINK_FROZEN)){
    act("$p is frozen solid, you can't get it out.",
	FALSE, ch, this, 0, TO_CHAR);
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
    vlogf(LOG_LOW, format("drinkcon %s  maxdrinks < current drinks.") % 
         getName());
  if (isDrinkConFlag(DRINK_PERM) && canWear(ITEM_TAKE))
    vlogf(LOG_LOW, format("drinkcon %s  takeable and permanent container.") % 
         getName());

  TObj::lowCheck();
}

bool TBaseCup::objectRepair(TBeing *ch, TMonster *repair, silentTypeT silent)
{
  if (!silent) {
    repair->doTell(fname(ch->name), "you might wanna take that to the diner!");
  }
  return TRUE;
}

void TBaseCup::assignFourValues(int x1, int x2, int x3, int x4)
{
  setMaxDrinkUnits(x1);
  setDrinkUnits(x2);
  setDrinkConFlags((unsigned) x4);

  if (x3 < MIN_DRINK_TYPES || x3 >= MAX_DRINK_TYPES) {
    vlogf(LOG_OBJ, format("Bad parm (%d) for drink type on %s") %  x3 % getName());
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
  keeper->doTell(ch->getName(), "I'm sorry, I don't purchase drink containers.");
  return TRUE;
}

void TBaseCup::weightCorrection()
{
  float wgt = obj_index[getItemIndex()].weight;  // base weight
  wgt += (SIP_WEIGHT * getDrinkUnits());  // weight of liquid
  setWeight(wgt);
}

sstring TBaseCup::statObjInfo() const
{
  char buf[256];

  sprintf(buf, "Max-contains : %d     Contains : %d\n\rPoisoned : %s   Permanent : %s   Spillable : %s   Frozen : %s\n\rLiquid : %s (%i)",
          getMaxDrinkUnits(), getDrinkUnits(),
          (isDrinkConFlag(DRINK_POISON) ? "true" : "false"),
          (isDrinkConFlag(DRINK_PERM) ? "true" : "false"),
          (isDrinkConFlag(DRINK_SPILL) ? "true" : "false"),
          (isDrinkConFlag(DRINK_FROZEN) ? "true" : "false"),
          liquidInfo[getDrinkType()]->name, getDrinkType());

  sstring a(buf);
  return a;
}

void TBaseCup::peeMe(const TBeing *ch, liqTypeT liq)
{
  act("$n smiles happily as $e pisses into $p.", TRUE, ch, this, NULL, TO_ROOM);
  act("You smile happily as you piss into $p.", TRUE, ch, this, NULL, TO_CHAR);

  fillMe(ch, liq);
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
  if(liquidInfo[getDrinkType()]->potion)
    return;

  genericEmpty();
}

void TBaseCup::evaporate(TBeing *ch, silentTypeT tSilent)
{
  if (!isDrinkConFlag(DRINK_PERM)) {
    if (getDrinkUnits() > 0) {   // has liquid
      int amount = 1 + (getDrinkUnits()/2);
      weightChangeObject(-amount * SIP_WEIGHT);
      addToDrinkUnits(-amount);

      if (!tSilent)
        ch->sendTo(COLOR_OBJECTS, format("The desert heat causes some of your %s to evaporate.\n\r") %
            liquidInfo[getDrinkType()]->name);
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

  if (!isDrinkConFlag(DRINK_SPILL) || isDrinkConFlag(DRINK_FROZEN))
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

int TBaseCup::chiMe(TBeing *tLunatic)
{
  int tMana  = ::number(10, 30),
      bKnown = tLunatic->getSkillLevel(SKILL_CHI);

  if (tLunatic->getMana() < tMana) {
    tLunatic->sendTo("You lack the chi to do this.\n\r");
    return RET_STOP_PARSING;
  } else
    tLunatic->reconcileMana(TYPE_UNDEFINED, 0, tMana);

  if (!tLunatic->bSuccess(bKnown, SKILL_CHI) ||
      getDrinkUnits() <= 0 || isDrinkConFlag(DRINK_PERM)) {
    act("You fail to affect $p in any way.",
        FALSE, tLunatic, this, NULL, TO_CHAR);
    return true;
  }

  if(isDrinkConFlag(DRINK_FROZEN)){
    act("You focus your chi, causing $p to melt!",
	FALSE, tLunatic, this, NULL, TO_CHAR);
    act("$n stares at $p, causing it to melt.",
	TRUE, tLunatic, this, NULL, TO_ROOM);
    remDrinkConFlags(DRINK_FROZEN);
    return true;
  }

  act("You focus your chi, causing $p to become lighter!",
      FALSE, tLunatic, this, NULL, TO_CHAR);
  act("$n stares at $p, causing it to become lighter.",
      TRUE, tLunatic, this, NULL, TO_ROOM);

  evaporate(tLunatic, SILENT_YES);

  return true;

}


void TBaseCup::updateDesc()
{
  sstring newname, short_desc, long_desc, liquid;
  bool found=false;

  // get og desc
  // replace all $l
  // if $l found, set strung and move desc
  
  newname=obj_index[getItemIndex()].name;
  short_desc=obj_index[getItemIndex()].short_desc;
  long_desc=obj_index[getItemIndex()].long_desc;

  if(getDrinkUnits()<=0){
    liquid="empty";
  } else if(isDrinkConFlag(DRINK_FROZEN)){
    liquid = format("<C>frozen<1> %s") % liquidInfo[getDrinkType()]->color;
  } else {
    liquid=liquidInfo[getDrinkType()]->color;
  }

  while (newname.find("$$l") != sstring::npos){
    newname.replace(newname.find("$$l"), 3, liquid);
    found=true;
  }
  while (newname.find("$l") != sstring::npos){
    newname.replace(newname.find("$l"), 2, liquid);
    found=true;
  }
  while (short_desc.find("$$l") != sstring::npos){
    short_desc.replace(short_desc.find("$$l"), 3, liquid);
    found=true;
  }
  while (short_desc.find("$l") != sstring::npos){
    short_desc.replace(short_desc.find("$l"), 2, liquid);
    found=true;
  }
  while (long_desc.find("$$l") != sstring::npos){
    long_desc.replace(long_desc.find("$$l"), 3, liquid);
    found=true;
  }
  while (long_desc.find("$l") != sstring::npos){
    long_desc.replace(long_desc.find("$l"), 2, liquid);
    found=true;
  }

  if(getDrinkUnits()<=0){
    liquid="an empty";
  } else if(isDrinkConFlag(DRINK_FROZEN)){
    liquid = format("a <C>frozen<1> %s") % liquidInfo[getDrinkType()]->color;
  } else {
    liquid = format("a %s") % liquidInfo[getDrinkType()]->color;
  }

  while (newname.find("$$al") != sstring::npos){
    newname.replace(newname.find("$$al"), 4, liquid);
    found=true;
  }
  while (newname.find("$al") != sstring::npos){
    newname.replace(newname.find("$al"), 3, liquid);
    found=true;
  }
  while (short_desc.find("$$al") != sstring::npos){
    short_desc.replace(short_desc.find("$$al"), 4, liquid);
    found=true;
  }
  while (short_desc.find("$al") != sstring::npos){
    short_desc.replace(short_desc.find("$al"), 3, liquid);
    found=true;
  }
  while (long_desc.find("$$al") != sstring::npos){
    long_desc.replace(long_desc.find("$$al"), 4, liquid);
    found=true;
  }
  while (long_desc.find("$al") != sstring::npos){
    long_desc.replace(long_desc.find("$al"), 3, liquid);
    found=true;
  }


  if(getDrinkUnits()<=0){
    liquid="nothing";
  } else if(isDrinkConFlag(DRINK_FROZEN)){
    liquid = format("<C>frozen<1> %s") % liquidInfo[getDrinkType()]->name;
  } else {
    liquid = format("%s") % liquidInfo[getDrinkType()]->name;
  }

  while (newname.find("$$nl") != sstring::npos){
    newname.replace(newname.find("$$nl"), 4, liquid);
    found=true;
  }
  while (newname.find("$nl") != sstring::npos){
    newname.replace(newname.find("$nl"), 3, liquid);
    found=true;
  }
  while (short_desc.find("$$nl") != sstring::npos){
    short_desc.replace(short_desc.find("$$nl"), 4, liquid);
    found=true;
  }
  while (short_desc.find("$nl") != sstring::npos){
    short_desc.replace(short_desc.find("$nl"), 3, liquid);
    found=true;
  }
  while (long_desc.find("$$nl") != sstring::npos){
    long_desc.replace(long_desc.find("$$nl"), 4, liquid);
    found=true;
  }
  while (long_desc.find("$nl") != sstring::npos){
    long_desc.replace(long_desc.find("$nl"), 3, liquid);
    found=true;
  }


  if(found){
    if (isObjStat(ITEM_STRUNG)) {
      delete [] name;
      delete [] shortDescr;
      delete [] descr;
      extraDescription *exd;
      while ((exd = ex_description)) {
	ex_description = exd->next;
	delete exd;
      }
      ex_description = NULL;
      delete [] action_description;
      action_description = NULL;
    } else {
      addObjStat(ITEM_STRUNG);
      ex_description = NULL;
      action_description = NULL;
    }

    name=mud_str_dup(stripColorCodes(newname));
    shortDescr=mud_str_dup(short_desc);
    setDescr(mud_str_dup(long_desc));
  }
}
