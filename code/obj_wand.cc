// wand.cc

#include "stdsneezy.h"
#include "obj_magic_item.h"
#include "obj_wand.h"
TWand::TWand() :
  TMagicItem(),
  maxCharges(0),
  curCharges(0),
  spell(TYPE_UNDEFINED)
{
}

TWand::TWand(const TWand &a) :
  TMagicItem(a),
  maxCharges(a.maxCharges),
  curCharges(a.curCharges),
  spell(a.spell)
{
}

TWand & TWand::operator=(const TWand &a)
{
  if (this == &a) return *this;
  TMagicItem::operator=(a);
  maxCharges = a.maxCharges;
  curCharges = a.curCharges;
  spell = a.spell;
  return *this;
}

TWand::~TWand()
{
}

void TWand::setMaxCharges(int n)
{
  maxCharges = n;
}

int TWand::getMaxCharges() const
{
  return maxCharges;
}

void TWand::addToMaxCharges(int n)
{
  maxCharges += n;
}

void TWand::setCurCharges(int n)
{
  curCharges = n;
}

int TWand::getCurCharges() const
{
  return curCharges;
}

void TWand::addToCurCharges(int n)
{
  curCharges += n;
}

void TWand::setSpell(spellNumT n)
{
  spell = n;
}

spellNumT TWand::getSpell() const
{
  return spell;
}

int TWand::changeItemVal4Check(TBeing *ch, int the_update)
{
  if (the_update != -1 &&
      (!discArray[the_update] ||
       (!discArray[the_update]->minMana && !discArray[the_update]->minLifeforce && !discArray[the_update]->minPiety))) {
    ch->sendTo("Invalid value or value is not a spell.\n\r");
    return TRUE;
  }
  return FALSE;
}

void TWand::divinateMe(TBeing *caster) const
{
  caster->sendTo(fmt("It has %d out of %d charges left.\n\r") %
           getCurCharges() % getMaxCharges());
  caster->describeMagicLevel(this, 101);
  caster->describeMagicLearnedness(this, 101);
  caster->describeMagicSpell(this, 101);
}

void TWand::assignFourValues(int x1, int x2, int x3, int x4)
{
  TMagicItem::assignFourValues(x1,x2,x3,x4);

  setMaxCharges(x2);
  setCurCharges(x3);
  setSpell(mapFileToSpellnum(x4));
}

void TWand::getFourValues(int *x1, int *x2, int *x3, int *x4) const
{
  TMagicItem::getFourValues(x1,x2,x3,x4);

  *x2 = getMaxCharges();
  *x3 = getCurCharges();
  *x4 = mapSpellnumToFile(getSpell());
}

sstring TWand::statObjInfo() const
{
  char buf[256];

  sprintf(buf, "Level:   %d        Learnedness:   %d\n\rSpell: %s    Charges: %d/%d",
              getMagicLevel(), getMagicLearnedness(),
              (getSpell() == TYPE_UNDEFINED ? "No Spell" :
                (discArray[getSpell()] ?
                    discArray[getSpell()]->name :
                    "BOGUS SPELL.  Bug this")),
              getCurCharges(),
              getMaxCharges());

  sstring a(buf);
  return a;
}

int TWand::objectSell(TBeing *ch, TMonster *keeper)
{
  if (getCurCharges() != getMaxCharges()) {
    keeper->doTell(ch->getName(), "I'm sorry, I don't buy back expended wands.");
    return TRUE;
  }

  return TMagicItem::objectSell(ch, keeper);
}

void TWand::lowCheck()
{
  spellNumT curspell = getSpell();
  if ((curspell < TYPE_UNDEFINED) || (curspell >= MAX_SKILL) ||
      ((curspell > TYPE_UNDEFINED) &&
       ((!discArray[curspell] ||
        ((discArray[curspell]->typ != SPELL_RANGER) && 
         !discArray[curspell]->minMana && 
         !discArray[curspell]->minLifeforce && 
         !discArray[curspell]->minPiety)) ||
      (getDisciplineNumber(curspell, FALSE) == DISC_NONE)))) {
    vlogf(LOG_LOW, fmt("wand (%s:%d) has messed up spell(%d)") % 
         getName() % objVnum() % curspell);
    if ((curspell < TYPE_UNDEFINED) || (curspell >= MAX_SKILL))
      vlogf(LOG_LOW, "bogus range");
    else if (!discArray[curspell])
      vlogf(LOG_LOW, fmt("bogus spell, %d") %  curspell);
    else if ((!discArray[curspell]->minMana && !discArray[curspell]->minLifeforce && 
      !discArray[curspell]->minPiety))
      vlogf(LOG_LOW, "non-spell");
  }
  if (curspell > TYPE_UNDEFINED &&
      discArray[curspell]->targets & TAR_CHAR_WORLD) {
    // spells that use this setting are not a good choice for obj spells
    vlogf(LOG_LOW, fmt("Obj (%s : %d) had spell that shouldn't be on objs (%s : %d)") %
        getName() % objVnum() % discArray[curspell]->name % curspell);
  }

  TMagicItem::lowCheck();
}

bool TWand::objectRepair(TBeing *ch, TMonster *repair, silentTypeT silent)
{
  if (!silent)
    repair->doTell(fname(ch->name), "You might wanna take that to the magic shop!");

  return TRUE;
}

int TWand::suggestedPrice() const
{
  spellNumT curspell = getSpell();
  int value = 0;
  if (curspell > TYPE_UNDEFINED) {
    value = getSpellCost(curspell, getMagicLevel(), getMagicLearnedness());
    value *= getMaxCharges();

    // since it's cast instantaneously, adjust for this
    value *= getSpellCasttime(curspell);

    // since it's from an obj, arbitrarily double it
    value *= 2;
    // staves are pretty cheap too
    value *= 15;
    if (curspell == SPELL_FLY)
      value *= 4;

  }
  return value;
}

int TWand::useMe(TBeing *ch, const char * argument)
{
  TBeing *tmp_char;
  TObj *o;
  int rc;
  unsigned int bits;
  spellNumT the_spell;

  the_spell = getSpell();
  if (!discArray[the_spell]) {
    vlogf(LOG_BUG,fmt("doUse (%s) called spell (%d) that does not exist! - Don't do that!") %  getName() % getSpell());
    return FALSE;
  }

  int bv = 0;
  if (IS_SET(discArray[the_spell]->targets, TAR_CHAR_ROOM))
    bv |= FIND_CHAR_ROOM;
  if (IS_SET(discArray[the_spell]->targets, TAR_OBJ_INV))
    bv |= FIND_OBJ_INV;
  if (IS_SET(discArray[the_spell]->targets, TAR_OBJ_ROOM))
    bv |= FIND_OBJ_ROOM;
  if (IS_SET(discArray[the_spell]->targets, TAR_OBJ_EQUIP))
    bv |= FIND_OBJ_EQUIP;

  if (!bv) {
    // wands are for targetable spells
    // if we ain't a targetted spell (i.e. get here)
    // then we ought to use some other magic-item (probably scroll)
    act("Sparks and smoke come forth from $p.", FALSE, ch, this, 0, TO_CHAR);
    act("Sparks and smoke come forth from $p.", FALSE, ch, this, 0, TO_ROOM);
    return FALSE; 
  }

  bits = generic_find(argument, bv, ch, &tmp_char, &o);
  if (bits) {
    generalUseMessage(ch, bits, tmp_char, o);

    if (IS_SET(discArray[the_spell]->targets, TAR_VIOLENT) &&
        ch->checkPeaceful("Impolite magic is banned here.\n\r"))
      return FALSE;

    if (getCurCharges() > 0) {      // Is there any charges left?
      addToCurCharges(-1);
      ch->addToWait(combatRound(1));
      rc = doObjSpell(ch,tmp_char,this,o,argument,the_spell);
      if (IS_SET_DELETE(rc, DELETE_VICT) && ch != tmp_char) {
        delete tmp_char;
        tmp_char = NULL;
      }
      if (IS_SET_DELETE(rc, DELETE_ITEM)) {
        delete o;
        o = NULL;
      }
      if ((IS_SET_DELETE(rc, DELETE_VICT) && ch == tmp_char) ||
           IS_SET_DELETE(rc, DELETE_THIS))
        return DELETE_VICT;
    } else
      act("$p seems powerless.", FALSE, ch, this, 0, TO_CHAR);
  } else
    act("What should $p be pointed at?", FALSE, ch, this, 0, TO_CHAR);

  return FALSE;
}

void TWand::generalUseMessage(const TBeing *ch, unsigned int bits, const TBeing *tmp_char, const TObj *o) const
{
  if (bits == FIND_CHAR_ROOM) {
    act("$n points $p at you.", TRUE, ch, this, tmp_char, TO_VICT);
    act("$n points $p at $N.", TRUE, ch, this, tmp_char, TO_NOTVICT);
    act("You point $p at $N.", FALSE, ch, this, tmp_char, TO_CHAR);
  } else {
    act("$n points $p at $P.", TRUE, ch, this, o, TO_ROOM);
    act("You point $p at $P.", FALSE, ch, this, o, TO_CHAR);
  }
}

sstring TWand::getNameForShow(bool useColor, bool useName, const TBeing *ch) const
{
  char buf2[256];
  sprintf(buf2, "%s [%s]",
       useName ? name : (useColor ? getName() : getNameNOC(ch).c_str()),
       (getSpell() > TYPE_UNDEFINED ? (discArray[getSpell()] ? discArray[getSpell()]->name : "Unknown") : "None"));
  return buf2;
}

