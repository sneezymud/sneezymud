// magic_item.cc

#include "stdsneezy.h"
#include "obj_magic_item.h"

TMagicItem::TMagicItem() :
  TObj(),
  magic_level(0),
  magic_learnedness(0)
{
}

TMagicItem::TMagicItem(const TMagicItem &a) :
  TObj(a),
  magic_level(a.magic_level),
  magic_learnedness(a.magic_learnedness)
{
}

TMagicItem & TMagicItem::operator=(const TMagicItem &a)
{
  if (this == &a) return *this;
  TObj::operator=(a);
  magic_level = a.magic_level;
  magic_learnedness = a.magic_learnedness;
  return *this;
}

TMagicItem::~TMagicItem()
{
}

void TMagicItem::changeObjValue1(TBeing *ch)
{
  ch->specials.edit = CHANGE_MAGICITEM_VALUE1;
  change_magicitem_value1(ch, this, "", ENTER_CHECK);
  return;
}

void TMagicItem::assignFourValues(int x1, int, int, int)
{
  setMagicLevel(GET_BITS(x1, 7, 8));
  setMagicLearnedness(GET_BITS(x1, 15, 8));
}

void TMagicItem::getFourValues(int *x1, int *x2, int *x3, int *x4) const
{
  int x = 0;

  SET_BITS(x, 7, 8, getMagicLevel());
  SET_BITS(x, 15, 8, getMagicLearnedness());

  *x1 = x;
  *x2 = 0;
  *x3 = 0;
  *x4 = 0;
}

sstring TMagicItem::displayFourValues()
{
  char tString[256];
  int  x1,
       x2,
       x3,
       x4;

  getFourValues(&x1, &x2, &x3, &x4);
  sprintf(tString, "Current values : %d %d %d %d\n\r", x1, x2, x3, x4);
  sprintf(tString + strlen(tString),
          "Current values : Lvl: %d Learn: %d %d %d %d",
          getMagicLevel(), getMagicLearnedness(), x2, x3, x4);

  return tString;
}

int TMagicItem::objectSell(TBeing *ch, TMonster *keeper)
{
  if (getMagicLearnedness() < MAX_SKILL_LEARNEDNESS) {
    keeper->doTell(ch->getName(), fmt("I'm sorry, that %s is of sub-standard quality.") % fname(name));
    return TRUE;
  }

  return FALSE;
}

void TMagicItem::setMagicLevel(int num)
{
  magic_level = num;
}

int TMagicItem::getMagicLevel() const
{
  return magic_level;
}

void TMagicItem::setMagicLearnedness(int num)
{
  magic_learnedness = num;
}

int TMagicItem::getMagicLearnedness() const
{
  return magic_learnedness;
}

void TMagicItem::objMenu(const TBeing *ch) const
{
  ch->sendTo(fmt(VT_CURSPOS) % 3 % 1);
  ch->sendTo(fmt("%sSuggested price:%s %d%s") %
             ch->purple() % ch->norm() % suggestedPrice() %
             (suggestedPrice() != obj_flags.cost ? " *" : ""));
}

void TMagicItem::lowCheck()
{
  int sp = suggestedPrice();
  if (obj_flags.cost >= 0 && obj_flags.cost < sp) {
    vlogf(LOG_LOW, fmt("magicitem (%s:%d) with bad price %d should be %d.") % 
          getName() % objVnum() % obj_flags.cost % sp);
    obj_flags.cost = sp;
  }
}
