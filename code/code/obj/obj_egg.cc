//////////////////////////////////////////////////////////////////////////
//
// SneezyMUD - All rights reserved, SneezyMUD Coding Team
//
//      "obj_egg.cc" - Procedures for eggs.  Derived from obj_food.cc
//
//////////////////////////////////////////////////////////////////////////

#include <cmath>

#include "monster.h"
#include "obj_food.h"
#include "obj_egg.h"
#include "room.h"

TEgg::TEgg() :
  TFood(),
  touched(0),
  incubationTimer(0),
  mobVNum(0)
{
}

TEgg::TEgg(const TEgg &a) :
  TFood(a),
  touched(a.touched),
  incubationTimer(a.incubationTimer),
  mobVNum(a.mobVNum)
{
}

TEgg & TEgg::operator=(const TEgg &a)
{
  if (this == &a) return *this;
  TFood::operator=(a);
  touched = a.touched;
  incubationTimer = a.incubationTimer;
  mobVNum = a.mobVNum;
  return *this;
}

TEgg::~TEgg()
{
}

// why is this different from foodPoisoned?!
void eggPoisoned(TEgg *egg, TBeing *ch, int dur)
{
  affectedData af;

  if (egg->isFoodFlag(FOOD_POISON) && 
      !ch->isAffected(AFF_POISON)) {
    if (ch->getMyRace()->hasTalent(TALENT_GARBAGEEATER)) {
      act("Mmm, that had a bit of a kick to it!", FALSE, ch, 0, 0, TO_CHAR);
    } else if (ch->isImmune(IMMUNE_POISON, WEAR_BODY)) {
      act("That tasted rather strange, but you don't think it had any ill-effect!", FALSE, ch, 0, 0, TO_CHAR);
    } else {
      act("That tasted rather strange!", FALSE, ch, 0, 0, TO_CHAR);
      act("$n coughs and utters some strange sounds.", FALSE, ch, 0, 0, TO_ROOM);
      af.type = SPELL_POISON;
      af.duration = dur * UPDATES_PER_MUDHOUR;
      af.modifier = 0;
      af.location = APPLY_NONE;
      af.bitvector = AFF_POISON;
      ch->affectJoin(NULL, &af, AVG_DUR_NO, AVG_EFF_NO);
    }
  }
}

// why is this different from foodSpoiled?!
void eggSpoiled(TEgg *egg, TBeing *ch, int dur)
{
  affectedData af;

  if (egg->isFoodFlag(FOOD_SPOILED)) {
    if (ch->getMyRace()->hasTalent(TALENT_GARBAGEEATER)) {
      ch->sendTo("Mmm, that was tangy!\n\r");
    } else if (ch->isImmune(IMMUNE_POISON, WEAR_BODY)) {
      act("Blarg!  That $o was rotten!  Hopefully it won't have any ill-effects.", FALSE, ch, egg, 0, TO_CHAR);
    } else {
      act("Blarg!  That $o was rotten!  Your stomach begins to churn.", FALSE, ch, egg, 0, TO_CHAR);
      act("$n begins to look glassy eyed and pale.", FALSE, ch, 0, 0, TO_ROOM);
      af.type = AFFECT_DISEASE;
      af.level = 0;
     // Added /4 because of player complaints of food poisoning - Russ 04/28/96
      af.duration = dur * UPDATES_PER_MUDHOUR;
      af.modifier = DISEASE_FOODPOISON;
      af.location = APPLY_NONE;
      af.bitvector = 0;
      ch->affectTo(&af);
      disease_start(ch, &af);
    }
  }
}

void TEgg::eatMe(TBeing *ch)
{
  if ((ch->getCond(FULL) > 20) && !ch->isImmortal()) {
  	act("You try to stuff another $o into your mouth, but alas, you are full!", FALSE, ch, this, 0, TO_CHAR);
    return;
  }
  if (isFoodFlag(FOOD_SPOILED) && ch->isPerceptive()) {
    act("You gag at the smell of $p and discard it instead.", TRUE, ch, this, 0, TO_CHAR);
    act("$n gags at the smell of $p and throws it out.", TRUE, ch, this, 0, TO_ROOM);

    ch->playsound(SOUND_FOODPOISON, SOUND_TYPE_NOISE);

    delete this;
    return;
  }

  act("$n eats $p.", TRUE, ch, this, 0, TO_ROOM);
  act("You eat the $o.", FALSE, ch, this, 0, TO_CHAR);

  if(ch->isVampire()){
    ch->sendTo("You eat the mortal food, but it has no affect on you.\n\r");
  } else {
    if (ch->getCond(FULL) > -1)
      ch->gainCondition(FULL, TFood::getFoodFill());
  }    

  if (ch->getCond(FULL) > 20)
    act("You are full.", FALSE, ch, 0, 0, TO_CHAR);

  if (!ch->isImmortal()) {
    eggPoisoned(this, ch, getFoodFill());
    eggSpoiled(this, ch, getFoodFill());
  }
  delete this;
}

void TEgg::tasteMe(TBeing *ch)
{
  if (ch->hasDisease(DISEASE_FOODPOISON)) {
    ch->sendTo("Uggh, your stomach feels just horrible and the thought of eating nauseates you.\n\r");
    ch->sendTo("You decide to skip this meal until you feel better.\n\r");
    return;
  }
  act("$n tastes the $o.", FALSE, ch, this, 0, TO_ROOM);
  act("You taste the $o.", FALSE, ch, this, 0, TO_CHAR);

  sstring msg;
  int amt = 1;

  // race-based food preferences
  if(ch->isVampire()){
    msg = "You eat the mortal food, but it has no affect on you.\n\r";
    amt = 0;
  } else if (ch->getMyRace()->hasTalent(TALENT_FISHEATER) && isFoodFlag(FOOD_FISHED)) {
    msg = "You savor this delicious fishy egg bite!\n\r";
    amt = 2;
  } else if (ch->getMyRace()->hasTalent(TALENT_FISHEATER) && !isFoodFlag(FOOD_FISHED)) {
    msg = "This food tastes bland and unappetizing.  You miss the raw and wriggly texture of fish.\n\r";
    amt = 0;
  } else if (ch->getMyRace()->hasTalent(TALENT_MEATEATER) && isFoodFlag(FOOD_BUTCHERED)) {
    msg = "Mmmmhhh!  Finally, some raw eggy meat!\n\r";
    amt = 2;
  } else if (ch->getMyRace()->hasTalent(TALENT_MEATEATER) && !isFoodFlag(FOOD_BUTCHERED)) {
    msg = "Pfwah!  This food tastes horrible!\n\r";
    amt = 0;
  }

  if (!msg.empty())
    ch->sendTo(msg);
  if (amt)
    ch->gainCondition(FULL, amt);

  if (ch->getCond(FULL) > 20)
    act("You are full.", FALSE, ch, 0, 0, TO_CHAR);

  if (!ch->isImmortal()) {
    eggPoisoned(this, ch, 1);
  }
  if (isFoodFlag(FOOD_SPOILED)) {
    act("Blarg!  That $o is spoiled!", FALSE, ch, this, 0, TO_CHAR);
    ch->sendTo("No point in keeping it around now...\n\r");
    delete this;
    return;
  }

  // we filled them "1 unit"
  // let's actually remove "1+ unit" so that we insure that taste isn't
  // better at filling than outright eating
  setFoodFill(getFoodFill() - 2);

  if (getFoodFill() <= 0) {    /* Nothing left */
    act("There is nothing left now.", FALSE, ch, 0, 0, TO_CHAR);
    delete this;
    return;
  }
  return;
}

bool TEgg::getEggTouched() const
{
  return touched;
}

void TEgg::setEggTouched(bool r)
{
  touched = r;
}

int TEgg::getEggTimer() const
{
  return incubationTimer;
}

void TEgg::setEggTimer(int r)
{
  incubationTimer = r;
}

int TEgg::getEggMobVNum() const
{
  return mobVNum;
}

void TEgg::setEggMobVNum(int r)
{
  mobVNum = r;
}

void TEgg::lowCheck()
{
  TFood::lowCheck();
}

sstring TEgg::statObjInfo() const
{
  char buf[256];

  sprintf(buf, "Makes full : %d\n\rPoisoned : %d\n\rIncubation timer : %d\n\rMob vnum : %d\n\rTouched : %d",
    getFoodFill(), getFoodFlags(), getEggTimer(), getEggMobVNum(),
    getEggTouched());

  sstring a(buf);
  return a;
}

void TEgg::assignFourValues(int x1, int x2, int x3, int x4)
{
  TFood::assignFourValues(x1, x2, x3, x4);

  setEggTouched(GET_BITS(x1, 31, 1));
  setEggTimer(x2);
  setEggMobVNum(x3);
}

void TEgg::getFourValues(int *x1, int *x2, int *x3, int *x4) const
{
  TFood::getFourValues(x1, x2, x3, x4);

  int r = *x1;
  SET_BITS(r, 31, 1, getEggTouched());
  *x1 = r;

  *x2 = getEggTimer();
  *x3 = getEggMobVNum();
}

sstring TEgg::displayFourValues()
{
  char tString[256];
  int x1, x2, x3, x4;

  getFourValues(&x1, &x2, &x3, &x4);
  sprintf(tString, "Current values : %d %d %d %d\n\r", x1, x2, x3, x4);

  sprintf(tString + strlen(tString),
          "Current values : Touched[%d] Food Fill[%d]\n\r\
Incubation Timer[%d] Mob VNum[%d] Food Flags[%d]",
          getEggTouched(), getFoodFill(), getEggTimer(), getEggMobVNum(),
          getFoodFlags());

  return tString;
}

void TEgg::changeObjValue1(TBeing *ch)
{
  ch->specials.edit = CHANGE_EGG_VALUE1;
  change_egg_value1(ch, this, "", ENTER_CHECK);
  return;
}

int TEgg::chiMe(TBeing *tLunatic)
{
  int tMana  = ::number(10, 30),
      bKnown = tLunatic->getSkillLevel(SKILL_CHI);

  if (tLunatic->getMana() < tMana) {
    tLunatic->sendTo("You lack the chi to do this.\n\r");
    return RET_STOP_PARSING;
  } else
    tLunatic->reconcileMana(TYPE_UNDEFINED, 0, tMana);

  if (!tLunatic->bSuccess(bKnown, SKILL_CHI) || isFoodFlag(FOOD_SPOILED)) {
    act("You fail to affect $p in any way.",
        FALSE, tLunatic, this, NULL, TO_CHAR);
    return true;
  }

  act("You focus your chi, causing $p to get warmer!",
      FALSE, tLunatic, this, NULL, TO_CHAR);
  act("$n stares at $p, causing it to get warmer!",
      TRUE, tLunatic, this, NULL, TO_ROOM);
  act("$p begins to wiggle a bit.",
      TRUE, tLunatic, this, NULL, TO_CHAR);
  act("$p begins to wiggle a bit.",
      TRUE, tLunatic, this, NULL, TO_ROOM);

  obj_flags.decay_time += ::number(1, 3);
  incubationTimer = 1;

  return true;
}

int TEgg::getMe(TBeing *ch, TThing *sub)
{
  // do baseclass stuff
  int rc = TObj::getMe(ch, sub);
  if (rc)
    return rc;

  touched=TRUE;

  return TRUE;
}

void TEgg::hatch(TRoom *rp)
{
  TMonster *mob;
  TBeing *ch;
  affectedData aff;

  if (!(mob = read_mobile(mobVNum, VIRTUAL))) {
    vlogf(LOG_BUG, "Problem loading monster in TEgg::hatch");
    return;
  }

  *rp += *mob;
  mob->oldRoom = inRoom();
  act("Suddenly, $p begins to move violently from within!",
    TRUE, mob, this, NULL, TO_ROOM);
  act("With a final push, $n emerges from $p!",
    TRUE, mob, this, NULL, TO_ROOM);

  if (
  	  ((parent && (ch=dynamic_cast<TBeing *>(parent)))
      || (equippedBy && (ch = dynamic_cast<TBeing *>(equippedBy))))
      && ch->isPc() 
      && mob->GetMaxLevel() < ch->GetMaxLevel() 
      && !ch->tooManyFollowers(mob, FOL_PET)) {
    // this code was cut and pasted from the pet buying code, sorry :(
    mob->doAction(ch->name, CMD_STARE);

    SET_BIT(mob->specials.affectedBy, AFF_CHARM);
    ch->addFollower(mob);
    mob->balanceMakeNPCLikePC();
    
    aff.type = AFFECT_PET;
    aff.level = 0;
    aff.duration  = PERMANENT_DURATION;
    aff.location = APPLY_NONE;
    aff.modifier = 0;   // to be used for elemental skill level
    aff.bitvector = 0;

	aff.be = static_cast<TThing *>((void *) mud_str_dup(ch->getName()));

    mob->affectTo(&aff, -1);
  } 
}

int TEgg::eggIncubate()
{
  return --incubationTimer;
}
