// staff.cc

#include "stdsneezy.h"
#include "obj_magic_item.h"
#include "obj_staff.h"

TStaff::TStaff() :
  TMagicItem(),
  maxCharges(0),
  curCharges(0),
  spell(TYPE_UNDEFINED)
{
}

TStaff::TStaff(const TStaff &a) :
  TMagicItem(a),
  maxCharges(a.maxCharges),
  curCharges(a.curCharges),
  spell(a.spell)
{
}

TStaff & TStaff::operator=(const TStaff &a)
{
  if (this == &a) return *this;
  TMagicItem::operator=(a);
  maxCharges = a.maxCharges;
  curCharges = a.curCharges;
  spell = a.spell;
  return *this;
}

TStaff::~TStaff() {
}

void TStaff::setMaxCharges(int n)
{
  maxCharges = n;
}

int TStaff::getMaxCharges() const
{
  return maxCharges;
}

void TStaff::addToMaxCharges(int n)
{
  maxCharges += n;
}

void TStaff::setCurCharges(int n)
{
  curCharges = n;
}

int TStaff::getCurCharges() const
{
  return curCharges;
}

void TStaff::addToCurCharges(int n)
{
  curCharges += n;
}

void TStaff::setSpell(spellNumT n)
{
  spell = n;
}

spellNumT TStaff::getSpell() const
{
  return spell;
}

int TStaff::changeItemVal4Check(TBeing *ch, int the_update)
{
  if (the_update != -1 &&
      (!discArray[the_update] ||
       (!discArray[the_update]->minMana && !discArray[the_update]->minLifeforce && !discArray[the_update]->minPiety))) {
    ch->sendTo("Invalid value or value is not a spell.\n\r");
    return TRUE;
  }
  return FALSE;
}

void TStaff::divinateMe(TBeing *caster) const
{
  caster->sendTo(fmt("It has %d out of %d charges left.\n\r") %
           getCurCharges() % getMaxCharges());
  caster->describeMagicLevel(this, 101);
  caster->describeMagicLearnedness(this, 101);
  caster->describeMagicSpell(this, 101);
}

void TStaff::assignFourValues(int x1, int x2, int x3, int x4)
{
  TMagicItem::assignFourValues(x1,x2,x3,x4);

  setMaxCharges(x2);
  setCurCharges(x3);
  setSpell(mapFileToSpellnum(x4));
}

void TStaff::getFourValues(int *x1, int *x2, int *x3, int *x4) const
{
  TMagicItem::getFourValues(x1,x2,x3,x4);

  *x2 = getMaxCharges();
  *x3 = getCurCharges();
  *x4 = mapSpellnumToFile(getSpell());
}

sstring TStaff::statObjInfo() const
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

int TStaff::objectSell(TBeing *ch, TMonster *keeper)
{
  if (getCurCharges() != getMaxCharges()) {
    keeper->doTell(ch->getName(), "I'm sorry, I don't buy back expended staves.");
    return TRUE;
  }

  return TMagicItem::objectSell(ch, keeper);
}

void TStaff::lowCheck()
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
    vlogf(LOG_LOW, fmt("staff (%s:%d) has messed up spell(%d)") % 
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
  if (curspell > TYPE_UNDEFINED &&
      discArray[curspell]->targets & TAR_IGNORE) {
    // spells that use this setting are not a good choice for staffs
    // TAR_IGNORE is generally an area effect, while a staff makes the spell
    // go area effect to begin with
// Fixed the code so it doesnt loop if its an area..used TAR_AREA for areas

//    vlogf(LOG_LOW, "Staff (%s : %d) had possible-area-spell that shouldn't be on objs (%s : %d)" , getName(), objVnum(), discArray[curspell]->name, curspell);
  }

  TMagicItem::lowCheck();
}

bool TStaff::objectRepair(TBeing *ch, TMonster *repair, silentTypeT silent)
{
  if (!silent) {
    repair->doTell(fname(ch->name), "You might wanna take that to the magic shop!");
  }
  return TRUE;
}


int TStaff::rentCost() const
{
  int num = TMagicItem::rentCost();

  num *= getCurCharges();
  num /= max(1, getMaxCharges());
  return num;
}

int TStaff::suggestedPrice() const
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
    value *= 15;
    if (curspell == SPELL_FLY)
      value *= 4;
  }
  return value;
}

int TStaff::useMe(TBeing *ch, const char * argument)
{
  TBeing *tmp_char;
  TThing *t2, *t;
  int rc = 0;
  bool isViolent = FALSE;
  spellNumT the_spell = getSpell();
  bool sleepTag = FALSE;
  if (objVnum() == OBJ_SLEEPTAG_STAFF) {
    if (!ch->hasWizPower(POWER_GOD) && !ch->inLethargica()) {
      ch->sendTo("You can only use that staff in Lethargica.\n\r");
      return FALSE;
    }
    sleepTag = TRUE;
  }
  if (!discArray[the_spell]) {
    vlogf(LOG_BUG,fmt("doUse (%s) called spell (%d) that does not exist! - Don't do that!") %  getName() % the_spell);
    return FALSE;
  }
  act("$n taps $p three times on the $g.", TRUE,  ch, this, 0, TO_ROOM);
  act("You tap $p three times on the $g.", FALSE, ch, this, 0, TO_CHAR);

  if (getCurCharges() > 0) {        // Is there any charges left?
    addToCurCharges(-1);
    if (!ch->isLucky(200 + ch->spellLuckModifier(the_spell)) &&
        (objVnum() != OBJ_SLEEPTAG_STAFF)) {
      act("The $o blows sparks out one end, something went wrong!",TRUE,ch, this,0,TO_CHAR);
      act("$n's $o blows sparks out one end, something went wrong!",TRUE,ch, this,0,TO_ROOM);
      act("You should gain more experience before using such powerful items of magic.",TRUE, ch, this,0,TO_CHAR);
    } else {
      ch->addToWait(combatRound(1));
      if (IS_SET(discArray[the_spell]->targets, TAR_AREA)) {
          rc = doObjSpell(ch,NULL,this, NULL,argument,the_spell);
          if (IS_SET_DELETE(rc, DELETE_THIS))
            return DELETE_VICT;
      } else {
        if ((discArray[the_spell]->targets & TAR_VIOLENT))
          isViolent = TRUE;
        for (t = ch->roomp->getStuff(); t; t = t2) {
          t2 = t->nextThing;
          tmp_char = dynamic_cast<TBeing *>(t);
          if (!tmp_char)
            continue;
          if (tmp_char == ch)
            continue;
          if (!sleepTag && isViolent && tmp_char->inGroup(*ch))
            continue;
          rc = doObjSpell(ch,tmp_char,this, NULL,argument,the_spell);
          if (IS_SET_DELETE(rc, DELETE_VICT) && ch != tmp_char) {
            delete tmp_char;
            tmp_char = NULL;
          }
          if ((IS_SET_DELETE(rc, DELETE_VICT) && ch == tmp_char) ||
              IS_SET_DELETE(rc, DELETE_THIS))
            return DELETE_VICT;
        }
      }
    }
  } else
    act("$p seems powerless.", FALSE, ch, this, 0, TO_CHAR);

  return FALSE;
}

