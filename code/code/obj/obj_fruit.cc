//////////////////////////////////////////////////////////////////////////
//
// SneezyMUD - All rights reserved, SneezyMUD Coding Team
//
//      "obj_fruit.cc" - Procedures for fruits.  Derived from obj_food.cc
//
//////////////////////////////////////////////////////////////////////////

#include <cmath>

#include "monster.h"
#include "obj_food.h"
#include "obj_fruit.h"
#include "obj_tool.h"
#include "room.h"

TFruit::TFruit() :
  TFood(),
  seedVNum(0)
{
}

TFruit::TFruit(const TFruit &a) :
  TFood(a),
  seedVNum(a.seedVNum)
{
}

TFruit & TFruit::operator=(const TFruit &a)
{
  if (this == &a) return *this;
  TFood::operator=(a);
  seedVNum = a.seedVNum;
  return *this;
}

TFruit::~TFruit()
{
}

int TFruit::getSeedVNum() const
{
  return seedVNum;
}

void TFruit::setSeedVNum(int r)
{
  seedVNum = r;
}


sstring TFruit::statObjInfo() const
{
  char buf[256];

  sprintf(buf, "Makes full : %d\n\rPoisoned : %d\n\rSeed vnum : %d",
	  getFoodFill(), getFoodFlags(), getSeedVNum());

  sstring a(buf);
  return a;
}

void TFruit::assignFourValues(int x1, int x2, int x3, int x4)
{
  TFood::assignFourValues(x1, x2, x3, x4);
  setSeedVNum(x2);
}

void TFruit::getFourValues(int *x1, int *x2, int *x3, int *x4) const
{
  TFood::getFourValues(x1, x2, x3, x4);

  *x2 = getSeedVNum();
}

void TFruit::createSeeds(){
  TObj *obj;
  TTool *seed;
  int nseeds=::number(0,4);

  if(getSeedVNum() > 0 && nseeds > 0){
    if(!(obj=read_object(getSeedVNum(), VIRTUAL))){
      vlogf(LOG_BUG, format("failed to load seed %i") % getSeedVNum());
      return;
    }
    
    if(!(seed=dynamic_cast<TTool *>(obj))){
      vlogf(LOG_BUG, format("seed %i is not a tool") % getSeedVNum());
      delete obj;
      return;
    }
    
    seed->setToolMaxUses(nseeds);
    seed->setToolUses(nseeds);
    seed->obj_flags.decay_time = 50;
    
    seed->addObjStat(ITEM_STRUNG);
    seed->ex_description = NULL;
    seed->action_description = NULL;
    
    // apple -> seeds handful apple
    seed->name = mud_str_dup(format("seeds handful %s") % name);
    // An apple lies here. -> A handful of apple seeds lie here.
    seed->setDescr(mud_str_dup(format("A handful of %s seeds lie here.") % sstring(shortDescr).word(1)));
    // an apple -> a handful of apple seeds
    seed->shortDescr = mud_str_dup(format("a handful of %s seeds") % sstring(shortDescr).word(1));
    
    if(equippedBy){
      *equippedBy += *seed;
    } else if(parent){
      *parent += *seed;
    } else if(roomp){  // in room
      *roomp += *seed;
    }
  }
}


void TFruit::eatMe(TBeing *ch){
  if (ch->getMyRace()->hasTalent(TALENT_INSECT_EATER) &&
      objVnum() != 34732) {
    ch->sendTo("You can't eat that!  It's not an insect!\n\r");
    return;
  }
  if ((ch->getCond(FULL) > 20) && !ch->isImmortal()) {
    ch->sendTo("You try to stuff more food into your mouth, but alas, you are full!\n\r");
    return;
  }
  if (isFoodFlag(FOOD_SPOILED) && !ch->getMyRace()->hasTalent(TALENT_GARBAGEEATER) &&
    ch->isPerceptive()) {
    act("You notice some spoilage on $p and discard it instead.", TRUE, ch, this, 0, TO_CHAR);
    act("$n disposes of some spoiled $o.", TRUE, ch, this, 0, TO_ROOM);

    ch->playsound(SOUND_FOODPOISON, SOUND_TYPE_NOISE);

    createSeeds();
    delete this;
    return;
  }

  act("$n eats $p.", TRUE, ch, this, 0, TO_ROOM);
  act("You eat the $o.", FALSE, ch, this, 0, TO_CHAR);

  sstring msg;
  float adjust = 1.0;

  // race-based food preferences
  if(ch->isVampire()){
    msg = "You eat the mortal food, but it has no affect on you.\n\r";
    adjust = 0;
  } else if (ch->getMyRace()->hasTalent(TALENT_FISHEATER) && isFoodFlag(FOOD_FISHED)) {
    msg = "You savor this delicious fishy bite!\n\r";
    adjust = 2;
  } else if (ch->getMyRace()->hasTalent(TALENT_FISHEATER) && !isFoodFlag(FOOD_FISHED)) {
    msg = "This food tastes bland and unappetizing.  You miss the raw and wriggly texture of fish.\n\r";
    adjust = 0.05;
  } else if (ch->getMyRace()->hasTalent(TALENT_MEATEATER) && isFoodFlag(FOOD_BUTCHERED)) {
    msg = "Mmmmhhh!  Finally, some raw meat!\n\r";
    adjust = 2;
  } else if (ch->getMyRace()->hasTalent(TALENT_MEATEATER) && !isFoodFlag(FOOD_BUTCHERED)) {
    msg = "Pfwah!  This food tastes horrible!\n\r";
    adjust = 0.05;
  }

  if (!msg.empty())
    ch->sendTo(msg);
  if (ch->getCond(FULL) > -1 && adjust > 0)
    ch->gainCondition(FULL, (int)(getFoodFill() * adjust));

  if (ch->getCond(FULL) > 20)
    act("You are full.", FALSE, ch, 0, 0, TO_CHAR);

  if (!ch->isImmortal()) {
    Poisoned(ch, getFoodFill());
    Spoiled(ch, getFoodFill());
  }
  createSeeds();
  delete this;
}

void TFruit::tasteMe(TBeing *ch){
  if (ch->hasDisease(DISEASE_FOODPOISON)) {
    ch->sendTo("Uggh, your stomach feels just horrible and the thought of food nauseates you.\n\r");
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
    msg = "You savor this delicious fishy bite!\n\r";
    amt = 2;
  } else if (ch->getMyRace()->hasTalent(TALENT_FISHEATER) && !isFoodFlag(FOOD_FISHED)) {
    msg = "This food tastes bland and unappetizing.  You miss the raw and wriggly texture of fish.\n\r";
    amt = 0;
  } else if (ch->getMyRace()->hasTalent(TALENT_MEATEATER) && isFoodFlag(FOOD_BUTCHERED)) {
    msg = "Mmmmhhh!  Finally, some raw meat!\n\r";
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
    Poisoned(ch, 1);
  }
  if (isFoodFlag(FOOD_SPOILED)) {
    ch->sendTo("Blah, that food is spoiled!\n\r");
    ch->sendTo("No point in keeping it around now...\n\r");
    createSeeds();
    delete this;
    return;
  }

  // we filled them "1 unit"
  // let's actually remove "1+ unit" so that we insure that taste isn't
  // better at filling than outright eating
  if (amt)
    setFoodFill(getFoodFill() - 2);

  if (getFoodFill() <= 0) {    /* Nothing left */
    act("There is nothing left now.", FALSE, ch, 0, 0, TO_CHAR);
    createSeeds();
    delete this;
    return;
  }
  return;
}

int TFruit::objectDecay(){
  if(isFoodFlag(FOOD_SPOILED)){
    createSeeds();
    return FALSE;
  } else {
    addFoodFlags(FOOD_SPOILED);
    obj_flags.decay_time = getVolume()*10;
  }
  return TRUE;
}
