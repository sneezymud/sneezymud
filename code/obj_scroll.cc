// scroll.cc

#include "stdsneezy.h"
#include "obj_magic_item.h"
#include "obj_scroll.h"

TScroll::TScroll() :
  TMagicItem()
{
  int i;
  for (i= 0; i < 3; i++) {
    spells[i] = TYPE_UNDEFINED;
  }
}

TScroll::TScroll(const TScroll &a) :
  TMagicItem(a)
{
  int i;
  for (i= 0; i < 3; i++) {
    spells[i] = a.spells[i];
  }
}

TScroll & TScroll::operator=(const TScroll &a)
{
  if (this == &a) return *this;
  TMagicItem::operator=(a);
  int i;
  for (i= 0; i < 3; i++) {
    spells[i] = a.spells[i];
  }
  return *this;
}

TScroll::~TScroll()
{
}

int TScroll::changeItemVal2Check(TBeing *ch, int the_update)
{
  if (the_update != -1 &&
      (!discArray[the_update] ||
      (!discArray[the_update]->minMana && !discArray[the_update]->minLifeforce && !discArray[the_update]->minPiety))) {
    ch->sendTo("Invalid value or value is not a spell.\n\r");
    return TRUE;
  }
  return FALSE;
}

int TScroll::changeItemVal3Check(TBeing *ch, int the_update)
{
  return changeItemVal2Check(ch, the_update);
}

int TScroll::changeItemVal4Check(TBeing *ch, int the_update)
{
  return changeItemVal2Check(ch, the_update);
}

void TScroll::divinateMe(TBeing *caster) const
{
  caster->describeMagicLevel(this, 101);
  caster->describeMagicLearnedness(this, 101);
  caster->describeMagicSpell(this, 101);
}

void TScroll::assignFourValues(int x1, int x2, int x3, int x4)
{
  TMagicItem::assignFourValues(x1,x2,x3,x4);

  setSpell(0, mapFileToSpellnum(x2));
  setSpell(1, mapFileToSpellnum(x3));
  setSpell(2, mapFileToSpellnum(x4));
}

void TScroll::getFourValues(int *x1, int *x2, int *x3, int *x4) const
{
  TMagicItem::getFourValues(x1,x2,x3,x4);

  *x2 = mapSpellnumToFile(getSpell(0));
  *x3 = mapSpellnumToFile(getSpell(1));
  *x4 = mapSpellnumToFile(getSpell(2));
}

sstring TScroll::statObjInfo() const
{
  char buf[256];
  sprintf(buf, "Level:   %d        Learnedness:   %d\n\rSpells : %s, %s, %s",
          getMagicLevel(), getMagicLearnedness(),
          (getSpell(0) == TYPE_UNDEFINED ? "No Spell" :
            (discArray[getSpell(0)] ?
                discArray[getSpell(0)]->name :
                "BOGUS SPELL.  Bug this")),
          (getSpell(1) == TYPE_UNDEFINED ? "No Spell" :
            (discArray[getSpell(1)] ?
                discArray[getSpell(1)]->name :
                "BOGUS SPELL.  Bug this")),
          (getSpell(2) == TYPE_UNDEFINED ? "No Spell" :
            (discArray[getSpell(2)] ?
                discArray[getSpell(2)]->name :
                "BOGUS SPELL.  Bug this")));

  sstring a(buf);
  return a;
}

void TScroll::lowCheck()
{
  int i;

  for (i= 0; i < 2; i++) {
    spellNumT curspell = getSpell(i);
    if ((curspell < TYPE_UNDEFINED) || (curspell >= MAX_SKILL) ||
        ((curspell > TYPE_UNDEFINED) &&
         ((!discArray[curspell] ||
          ((discArray[curspell]->typ != SPELL_RANGER) &&
           !discArray[curspell]->minMana &&
           !discArray[curspell]->minLifeforce &&
           !discArray[curspell]->minPiety)) ||
        (getDisciplineNumber(curspell, FALSE) == DISC_NONE)))) {
      vlogf(LOG_LOW, fmt("scroll (%s:%d) has messed up spell (slot %d: %d)") % 
           getName() % objVnum() % (i+1) % curspell);
      if ((curspell < TYPE_UNDEFINED) || (curspell >= MAX_SKILL))
        vlogf(LOG_LOW, "bogus range");
      else if (!discArray[curspell])
        vlogf(LOG_LOW, fmt("bogus spell, %d") %  curspell);
      else if ((!discArray[curspell]->minMana && !discArray[curspell]->minLifeforce &&
        !discArray[curspell]->minPiety))
        vlogf(LOG_LOW, "non-spell");
      continue;
    }
    if (curspell > TYPE_UNDEFINED &&
        discArray[curspell]->targets & TAR_CHAR_WORLD) {
      // spells that use this setting are not a good choice for obj spells
      vlogf(LOG_LOW, fmt("Obj (%s : %d) had spell that shouldn't be on objs (%s : %d)") %
          getName() % objVnum() % discArray[curspell]->name % curspell);
    }
  }

  TMagicItem::lowCheck();
}

bool TScroll::objectRepair(TBeing *ch, TMonster *repair, silentTypeT silent)
{
  if (!silent) {
    repair->doTell(fname(ch->name), "You might wanna take that to the magic shop!");
  }
  return TRUE;
}

spellNumT TScroll::getSpell(int num) const
{
  mud_assert(num >= 0 && num < 3, "Bad num");
  return spells[num];
}

void TScroll::setSpell(int num, spellNumT xx)
{
  mud_assert(num >= 0 && num < 3, "Bad num");
  spells[num] = xx;
}

int TScroll::suggestedPrice() const
{
  int tot = 0;
  int i;
  for (i = 0; i < 3; i++) {
    spellNumT curspell = getSpell(i);
    int value = 0;
    if (curspell >= 0) {
      value = getSpellCost(curspell, getMagicLevel(), getMagicLearnedness());
      // since it's cast instantaneously, adjust for this
      value *= getSpellCasttime(curspell);

      // since it's from an obj, arbitrarily double it
      value *= 2;
      // scrolls are underpriced too
      value *= 15;
      if (curspell == SPELL_FLY)
        value *= 4;

    }


    tot += value;
  }

  // add material value
  tot += (int)(10.0 * getWeight() * material_nums[getMaterial()].price);

  return tot;
}

