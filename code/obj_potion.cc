// potion.cc

#include "stdsneezy.h"

TPotion::TPotion() :
  TMagicItem()
{
  int i;
  for (i= 0; i < 3; i++) {
    spells[i] = TYPE_UNDEFINED;
  }
}

TPotion::TPotion(const TPotion &a) :
  TMagicItem(a)
{
  int i;
  for (i= 0; i < 3; i++) {
    spells[i] = a.spells[i];
  }
}

TPotion & TPotion::operator=(const TPotion &a)
{
  if (this == &a) return *this;
  TMagicItem::operator=(a);
  int i;
  for (i= 0; i < 3; i++) {
    spells[i] = a.spells[i];
  }
  return *this;
}

TPotion::~TPotion()
{
}

int TPotion::changeItemVal2Check(TBeing *ch, int the_update)
{
  if (the_update != -1 &&
      (!discArray[the_update] ||
      (!discArray[the_update]->minMana && !discArray[the_update]->minLifeforce && !discArray[the_update]->minPiety))) {
    ch->sendTo("Invalid value or value is not a spell.\n\r");
    return TRUE;
  }
  return FALSE;
}

int TPotion::changeItemVal3Check(TBeing *ch, int the_update)
{
  return changeItemVal2Check(ch, the_update);
}

int TPotion::changeItemVal4Check(TBeing *ch, int the_update)
{
  return changeItemVal2Check(ch, the_update);
}

void TPotion::divinateMe(TBeing *caster) const
{
  caster->describeMagicLevel(this, 101);
  caster->describeMagicLearnedness(this, 101);
  caster->describeMagicSpell(this, 101);
}

void TPotion::assignFourValues(int x1, int x2, int x3, int x4)
{
  TMagicItem::assignFourValues(x1,x2,x3,x4);

  setSpell(0, mapFileToSpellnum(x2));
  setSpell(1, mapFileToSpellnum(x3));
  setSpell(2, mapFileToSpellnum(x4));
}

void TPotion::getFourValues(int *x1, int *x2, int *x3, int *x4) const
{
  TMagicItem::getFourValues(x1,x2,x3,x4);

  *x2 = mapSpellnumToFile(getSpell(0));
  *x3 = mapSpellnumToFile(getSpell(1));
  *x4 = mapSpellnumToFile(getSpell(2));
}

spellNumT TPotion::getSpell(int num) const
{
  mud_assert(num >= 0 && num < 3, "Bad num");
  return spells[num];
}

void TPotion::setSpell(int num, spellNumT xx)
{
  mud_assert(num >= 0 && num < 3, "Bad num");
  spells[num] = xx;
}

string TPotion::statObjInfo() const
{
  char buf[256];

  sprintf(buf, "Level:   %d        Learnedness:   %d\n\rSpells : %s (%d), %s (%d), %s (%d)",
          getMagicLevel(), getMagicLearnedness(),
          (getSpell(0) == TYPE_UNDEFINED ? "No Spell" :
            (discArray[getSpell(0)] ?
                discArray[getSpell(0)]->name :
                "BOGUS SPELL.  Bug this")),
           getSpell(0),
          (getSpell(1) == TYPE_UNDEFINED ? "No Spell" :
            (discArray[getSpell(1)] ?
                discArray[getSpell(1)]->name :
                "BOGUS SPELL.  Bug this")),
           getSpell(1),
          (getSpell(2) == TYPE_UNDEFINED ? "No Spell" :
            (discArray[getSpell(2)] ?
                discArray[getSpell(2)]->name :
                "BOGUS SPELL.  Bug this")),
           getSpell(2));

  string a(buf);
  return a;
}

void TPotion::lowCheck()
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
      vlogf(LOG_LOW, "potion (%s:%d) has messed up spell (slot %d: %d)",
           getName(), objVnum(), i+1, curspell);
      if ((curspell < TYPE_UNDEFINED) || (curspell >= MAX_SKILL))
        vlogf(LOG_LOW, "bogus range");
      else if (!discArray[curspell])
        vlogf(LOG_LOW, "bogus spell, %d", curspell);
      else if ((!discArray[curspell]->minMana && !discArray[curspell]->minLifeforce && 
        !discArray[curspell]->minPiety))
        vlogf(LOG_LOW, "non-spell");
      continue;
    }
    if (curspell > TYPE_UNDEFINED &&
        discArray[curspell]->targets & TAR_CHAR_WORLD) {
      // spells that use this setting are not a good choice for obj spells
      vlogf(LOG_LOW, "Obj (%s : %d) had spell that shouldn't be on objs (%s : %d)" ,
          getName(), objVnum(), discArray[curspell]->name, curspell);
    }
  }

  TMagicItem::lowCheck();
}

bool TPotion::objectRepair(TBeing *ch, TMonster *repair, silentTypeT silent)
{
  if (!silent) {
    char buf[256];

    sprintf(buf, "%s You might wanna take that to the magic shop!", fname(ch->name).c_str());

    repair->doTell(buf);
  }
  return TRUE;
}

int TPotion::drinkMe(TBeing *ch)
{
  return quaffMe(ch);
}


bool TPotion::sellMeCheck(const TBeing *ch, TMonster *keeper) const
{
  int total = 0;
  TThing *t;
  char buf[256];

  for (t = keeper->getStuff(); t; t = t->nextThing) {
    if ((t->number == number) &&
        (t->getName() && getName() &&
         !strcmp(t->getName(), getName()))) {
      total += 1;
      if (total > 9) {
        sprintf(buf, "%s I already have plenty of those.", ch->name);
        keeper->doTell(buf);
        return TRUE;
      }
    }
  }
  return FALSE;
}

int TPotion::suggestedPrice() const
{
  int tot = 0;
  int i;
  for (i = 0; i < 3; i++) {
    spellNumT curspell = getSpell(i);
    int value = 0;
    if (curspell != TYPE_UNDEFINED) {
      value = getSpellCost(curspell, getMagicLevel(), getMagicLearnedness());

      // since it's cast instantaneously, adjust for this
      value *= getSpellCasttime(curspell);

      // since it's from an obj, arbitrarily double it
      value *= 2;
      // potions are really cheap for some reason
      value *= 15;
      if (curspell == SPELL_FLY)
	value *= 4;
    }

    tot += value;
  }
  return tot;
}
