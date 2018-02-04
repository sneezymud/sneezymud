//////////////////////////////////////////////////////////////////////////
//
// SneezyMUD - All rights reserved, SneezyMUD Coding Team
//
//////////////////////////////////////////////////////////////////////////


// symbol.cc
#include <stdio.h>

#include <cmath>


#include "extern.h"
#include "monster.h"
#include "materials.h"
#include "shop.h"
#include "obj_symbol.h"
#include "shopowned.h"
#include "corporation.h"

TSymbol::TSymbol() :
  TObj(),
  strength(0),
  max_strength(0),
  faction(FACT_UNDEFINED)
{
}

TSymbol::TSymbol(const TSymbol &a) :
  TObj(a),
  strength(a.strength),
  max_strength(a.max_strength),
  faction(a.faction)
{
}

TSymbol & TSymbol::operator=(const TSymbol &a)
{
  if (this == &a) return *this;
  TObj::operator=(a);
  strength = a.strength;
  max_strength = a.max_strength;
  faction = a.faction;
  return *this;
}

TSymbol::~TSymbol()
{
}

void TSymbol::assignFourValues(int x1, int x2, int x3, int x4)
{
  setSymbolCurStrength(x1);
  setSymbolMaxStrength(x2);

  // allow undefined, rather than MIN_FACTION since undefined = unattnued
  if (x3 < FACT_UNDEFINED || x3 >= MAX_FACTIONS) {
     vlogf(LOG_LOW,format("symbol with bad faction (%s).") %  getName());
    x3 = FACT_UNDEFINED;
  }
  mud_assert(x3 >= FACT_UNDEFINED && x3 < MAX_FACTIONS, "bad val");
  setSymbolFaction(factionTypeT(x3));
}

void TSymbol::getFourValues(int *x1, int *x2, int *x3, int *x4) const
{
  *x1 = getSymbolCurStrength();
  *x2 = getSymbolMaxStrength();
  *x3 = getSymbolFaction();
  *x4 = 0;
}

sstring TSymbol::statObjInfo() const
{
  char buf[256];
  sstring a;

  sprintf(buf, "Symbol Strength: Current: %d, Maximum: %d\n\r",
    getSymbolCurStrength(), 
    getSymbolMaxStrength());
  a += buf;
  sprintf(buf, "Symbol Faction: %d (%s)\n\r",
    getSymbolFaction(), 
    getSymbolFaction() == -1 ? "not attuned" : FactionInfo[getSymbolFaction()].faction_name);
  a += buf;

  return a;
}

int TSymbol::objectSell(TBeing *ch, TMonster *keeper)
{
  int attuneCode = 1;

  if ((getSymbolCurStrength() != getSymbolMaxStrength())) {
    keeper->doTell(ch->getName(), "I'm sorry, I don't buy back used symbols.");
    return TRUE;
  }

  if ((getSymbolFaction() != FACT_UNDEFINED) && attuneCode) {
    keeper->doTell(ch->getName(), "I'm sorry, I don't buy back attuned symbols.");
    return TRUE;
  }

  return FALSE;
}

int TSymbol::getSymbolCurStrength() const
{
  return strength;
}

void TSymbol::setSymbolCurStrength(int r)
{
  strength = r;
}

void TSymbol::addToSymbolCurStrength(int r)
{
  strength += r;
}

int TSymbol::getSymbolMaxStrength() const
{
  return max_strength;
}

void TSymbol::setSymbolMaxStrength(int r)
{
  max_strength = r;
}

void TSymbol::addToSymbolMaxStrength(int r)
{
  max_strength += r;
}

factionTypeT TSymbol::getSymbolFaction() const
{
  return faction;
}

void TSymbol::setSymbolFaction(factionTypeT r)
{
  faction = r;
}


bool TSymbol::sellMeCheck(TBeing *ch, TMonster *keeper, int, int) const
{
  return TObj::sellMeCheck(ch, keeper, 1, 50);
}

void TSymbol::lowCheck()
{
  int i;

  if (getSymbolMaxStrength() < getSymbolCurStrength())
    vlogf(LOG_LOW, format("symbol (%s) has lower max strength then current.") % 
             getName());
  for (i=0; i<MAX_OBJ_AFFECT;i++) {
    if (affected[i].location == APPLY_ARMOR) {
      vlogf(LOG_LOW, format("symbol (%s) had armor, bad!") % 
         getName());
    }
  }
  int ap = suggestedPrice();
  if (ap != obj_flags.cost && obj_flags.cost >= 0 && ap) {
    // ignore newbie symbol with cost = -1
    vlogf(LOG_LOW, format("symbol (%s:%d) has a bad price (%d).  should be (%d)") % 
         getName() % objVnum() % obj_flags.cost % ap);
    obj_flags.cost = ap;
  }

  TObj::lowCheck();
}

bool TSymbol::objectRepair(TBeing *ch, TMonster *repair, silentTypeT silent)
{
  if (!silent) {
    repair->doTell(fname(ch->name), "Hey, I don't get involved with religion!");
  }
  return TRUE;
}

bool TSymbol::lowCheckSlots(silentTypeT silent)
{
  // symbols should be (take hold) or (take neck) or (take neck hold)
  // no other combos allowed, although permit the throw flag too

  unsigned int value = obj_flags.wear_flags;
  REMOVE_BIT(value, ITEM_WEAR_THROW);
  REMOVE_BIT(value, ITEM_WEAR_TAKE);
  REMOVE_BIT(value, ITEM_WEAR_HOLD);
  REMOVE_BIT(value, ITEM_WEAR_NECK);

  if (value != 0) {
    if (!silent)
      vlogf(LOG_LOW, format("symbol (%s) with bad wear slots: %d") % 
                 getName() % value);
    return true;
  }
  return false;
}

int TSymbol::suggestedPrice() const
{
  // c.f. balance notes for this
  int num = (int) (15 * (float) getSymbolMaxStrength() / 1000 + 0.5);
  if (canWear(ITEM_WEAR_NECK))
    num = (int) (num * 1.1);

  // add material value
  num += (int)(10.0 * getWeight() * material_nums[getMaterial()].price);

  return num;
}

void TSymbol::objMenu(const TBeing *ch) const
{
  ch->sendTo(format(VT_CURSPOS) % 3 % 1);
  ch->sendTo(format("%sSuggested price:%s %d%s") %
             ch->purple() % ch->norm() % suggestedPrice() %
             (suggestedPrice() != obj_flags.cost ? " *" : ""));
  ch->sendTo(format(VT_CURSPOS) % 4 % 1);
  ch->sendTo(format("%sSymbol Level:%s %.1f") %
       ch->purple() % ch->norm() %
       getSymbolLevel());
}

double TSymbol::getSymbolLevel() const
{
  return sqrt((double) getSymbolMaxStrength() / 500.0);
}

sstring TSymbol::showModifier(showModeT mode, const TBeing *ch) const
{
  sstring a;
  if (mode == SHOW_MODE_SHORT_PLUS ||
       mode == SHOW_MODE_SHORT_PLUS_INV ||
       mode == SHOW_MODE_SHORT) {
    if ((ch->isImmortal() && ch->hasWizPower(POWER_IMM_EVAL)) ||
        toggleInfo[TOG_TESTCODE2]->toggle) {
      char buf[256];
      sprintf(buf, " (L%d)", (int) (getSymbolLevel() + 0.5));
      a += buf;
    }
  }
  return a;
}

void TSymbol::evaluateMe(TBeing *ch) const
{
  int learn;

  learn = ch->getSkillValue(SKILL_EVALUATE);

  ch->learnFromDoingUnusual(LEARN_UNUSUAL_NORM_LEARN, SKILL_EVALUATE, 10);

  if (learn > 10)
    ch->describeSymbolOunces(this, learn);
}

double TSymbol::objLevel() const
{
  return getSymbolLevel();
}

void TSymbol::purchaseMe(TBeing *ch, TMonster *keeper, int cost, int shop_nr)
{
  TShopOwned tso(shop_nr, keeper, ch);
  tso.doBuyTransaction(cost, getName(), TX_BUYING, this);
}

void TSymbol::sellMeMoney(TBeing *ch, TMonster *keeper, int cost, int shop_nr)
{
  TShopOwned tso(shop_nr, keeper, ch);
  tso.doSellTransaction(cost, getName(), TX_SELLING);
}

sstring TSymbol::getNameForShow(bool useColor, bool useName, const TBeing *ch) const
{
  char buf2[256];
  sprintf(buf2, "%s (L%d)",
       useName ? name.c_str() : (useColor ? getName().c_str() : getNameNOC(ch).c_str()),
       (int) (getSymbolLevel() + 0.5));
  return buf2;
}

int TSymbol::chiMe(TBeing *tLunatic)
{
  int tMana  = ::number(10, 30),
      bKnown = tLunatic->getSkillLevel(SKILL_CHI);

  if (tLunatic->getMana() < tMana) {
    tLunatic->sendTo("You lack the chi to do this!\n\r");
    return RET_STOP_PARSING;
  } else
    tLunatic->reconcileMana(TYPE_UNDEFINED, 0, tMana);

  if (!tLunatic->bSuccess(bKnown, SKILL_CHI) ||
      (getSymbolCurStrength() >= getSymbolMaxStrength())) {
    act("You focus upon $p, but faulter and gently harm it!",
        FALSE, tLunatic, this, NULL, TO_CHAR);
    act("$n focuses on $p, but it cracks gently in response!",
        TRUE, tLunatic, this, NULL, TO_ROOM);

    addToSymbolCurStrength(-::number(1, 4));

    if (getSymbolCurStrength() <= 0) {
      act("$p reacts violently and shatters!",
          FALSE, tLunatic, this, NULL, TO_ROOM);
      return DELETE_VICT;
    }
  } else {
    act("You focus upon $p causing it to mend ever so slightly!",
        FALSE, tLunatic, this, NULL, TO_CHAR);
    act("$n concentrates upon $p, causing it to mend ever so slightly!",
        TRUE, tLunatic, this, NULL, TO_ROOM);

    setSymbolCurStrength(min(getSymbolMaxStrength(), (getSymbolCurStrength() + ::number(1, 4))));
  }

  return true;
}
