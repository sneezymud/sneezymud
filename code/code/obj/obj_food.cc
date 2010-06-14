//////////////////////////////////////////////////////////////////////////
//
// SneezyMUD - All rights reserved, SneezyMUD Coding Team
//
//      "food.cc" - Procedures for eating/drinking. And general liquid 
//      containers like vials
//
//////////////////////////////////////////////////////////////////////////

#include <cmath>

#include "handler.h"
#include "room.h"
#include "being.h"
#include "low.h"
#include "monster.h"
#include "materials.h"
#include "disease.h"
#include "shop.h"
#include "obj_food.h"
#include "obj_base_cup.h"
#include "obj_pool.h"
#include "disc_aegis.h"
#include "disc_shaman_armadillo.h"
#include "shopowned.h"
#include "corporation.h"
#include "liquids.h"
#include "obj_player_corpse.h"

#define DRINK_DEBUG 0

void TBaseCup::weightChangeObject(float wgt_amt)
{
  TThing *tmp;
  wearSlotT pos;
#if DRINK_DEBUG
  float sweight = getWeight();
#endif
  float max_amt;
  float weightDiff = 0;

  // OED items ignore liquid weights
  if (number < 0)
    return;

  // obj-weight < -weight + indexed weight
  if (compareWeights(getWeight(), 
               (-wgt_amt + obj_index[getItemIndex()].weight)) == 1) {
#if DRINK_DEBUG
    // this happens sometimes because of that stupid roundoff error in floats
    vlogf(LOG_BUG, format("Attempt to reduce %s below its empty weight") %  getName());
    vlogf(LOG_BUG, format("weight: %f, adjust: %f, base: %f") %  getWeight() % 
         -wgt_amt % obj_index[getItemIndex()].weight);
#endif
    wgt_amt = -(getWeight() - obj_index[getItemIndex()].weight);
  }
  
  if (in_room != Room::NOWHERE) 
    addToWeight(wgt_amt);
  else if ((tmp = parent)) {
    if (dynamic_cast<TBeing *> (tmp)) {
      weightDiff = getWeight();
      addToWeight(wgt_amt);
      weightDiff -= getWeight();
      //      tmp->addToCarriedWeight(-weightDiff);
    } else {
      --(*this);
      addToWeight(wgt_amt);
      *tmp += *this;
    }
  } else if ((tmp = equippedBy)) {
    TBeing *tbt = dynamic_cast<TBeing *>(tmp);
    pos = eq_pos;
    tbt->unequip(pos);
    addToWeight(wgt_amt);
    tbt->equipChar(this, pos, SILENT_YES);
  } else if ((tmp = riding)) {
    dismount(POSITION_STANDING);
    addToWeight(wgt_amt);
    mount(tmp);
  } else {
    // if object (pool) is changing in weight at load time, this is ok
    //vlogf(LOG_BUG, format("Unknown attempt to subtract weight from an object. (weightChangeObject) obj=%s") %  getName());

    addToWeight(wgt_amt);
  }

  // weight debug: check if greater then max units
  max_amt = obj_index[getItemIndex()].weight + (getMaxDrinkUnits() * SIP_WEIGHT);
  // obj weight > max
  if (compareWeights(getWeight(), max_amt) == -1) {
#if DRINK_DEBUG
    // this happens, silly float round off
    vlogf(LOG_BUG, format("DRINK: Bad weight change on %s.  Location %s") %  getName() %
           (in_room != Room::NOWHERE ? roomp->getName() :
           (equippedBy ? equippedBy->getName() :
           parent->getName())));
    vlogf(LOG_BUG, format("DRINK: Orig: %.1f, change %.1f, now %.1f, max %.1f, sips: %d") %  
        sweight % wgt_amt % getWeight() % max_amt % getMaxDrinkUnits());
    vlogf(LOG_BUG, "DRINK: resetting");
#endif
    if ((tmp = parent)) {
      if (dynamic_cast<TBeing *> (tmp)) {
        weightDiff = getWeight();
        setWeight(obj_index[getItemIndex()].weight +
                     getDrinkUnits() * SIP_WEIGHT);
        weightDiff -= getWeight();
	//        tmp->addToCarriedWeight(-weightDiff);
      } else {
        (*this)--;
        setWeight(obj_index[getItemIndex()].weight +
                     getDrinkUnits() * SIP_WEIGHT);
        *tmp += *this;
      }
    } else if ((tmp = equippedBy)) {
      TBeing *tbt = dynamic_cast<TBeing *>(tmp);
      pos = eq_pos;
      tbt->unequip(pos);
      setWeight(obj_index[getItemIndex()].weight +
                     getDrinkUnits() * SIP_WEIGHT);
      tbt->equipChar(this, pos, SILENT_YES);
    } else {
      // in room
      setWeight(obj_index[getItemIndex()].weight +
                     getDrinkUnits() * SIP_WEIGHT);
    }
  }
}

int TObj::drinkMe(TBeing *ch)
{ 
  ch->sendTo("Drink is generally used for liquids.\n\r");
  return FALSE;
}

int TBaseCup::drinkMe(TBeing *ch)
{ 
  int amount;
  affectedData af, aff2, aff3;
  sstring drinkName;

  if (ch->hasDisease(DISEASE_FOODPOISON)) {
    ch->sendTo("Uggh, your stomach is queasy and the thought of doing that is unappetizing.\n\r");
    ch->sendTo("You decide to skip this drink until you feel better.\n\r");
    return FALSE;
  }
  if(isDrinkConFlag(DRINK_FROZEN)){
    act("It's frozen solid, you can't drink from it.",
	FALSE, ch, 0, 0, TO_CHAR);
    return FALSE;
  }
  if (getDrinkUnits() <= 0) {
    act("It's empty already.", FALSE, ch, 0, 0, TO_CHAR);
    return FALSE;
  }
  if ((ch->getCond(DRUNK) > 15) && (ch->getCond(THIRST) > 0)) {
    act("You're just sloshed.", FALSE, ch, 0, 0, TO_CHAR);
    act("$n looks really drunk.", TRUE, ch, 0, 0, TO_ROOM);
    return FALSE;
  }
  if (ch->getCond(THIRST) > 20) {
    act("Your stomach can't contain anymore!", FALSE, ch, 0, 0, TO_CHAR);
    return FALSE;
  }

  act(format("$n drinks %s from $p.") % liquidInfo[getDrinkType()]->name, TRUE, ch, this, 0, TO_ROOM);
  ch->sendTo(COLOR_OBJECTS, format("You drink the %s.\n\r") % liquidInfo[getDrinkType()]->name);

  // a single drink "should" average about 8 oz.
  // this means have to drink 3-4 to get unthirsty so go with unreal value.
  if (getLiqDrunk() > 0) {
    amount = 10 * (25 - ch->getCond(THIRST)) / getLiqDrunk();
    amount = min(15, amount);
  } else if(liquidInfo[liquidType]->potion){
    amount = 5;
  } else
    amount = ::number(6, 20);

  amount = max(1, min(amount, getDrinkUnits()));

  // Subtract amount, if not a never-emptying container 
  if (!isDrinkConFlag(DRINK_PERM))
    weightChangeObject(-(amount * SIP_WEIGHT));

  if (getLiqDrunk()) {
    ch->gainCondition(DRUNK, (getLiqDrunk() * amount) / 10);

    // use leftover as chance to go 1 more unit up/down
    if (::number(0,9) < ((abs(getLiqDrunk()) * amount) % 10))
      ch->gainCondition(DRUNK, (getLiqDrunk() > 0 ? 1 : -1));

    if(ch->hasQuestBit(TOG_IS_ALCOHOLIC)){
      ch->gainCondition(THIRST, (getLiqDrunk() * amount) / 10);
      ch->sendTo("The deliciously satisfying alcohol quenches your thirst.\n\r");
    }

    if(getLiqDrunk()>0)
      ch->bSuccess(SKILL_ALCOHOLISM);
  }

  if(ch->isVampire()){
    ch->sendTo("You drink the mortal food, but it has no affect on you.\n\r");
  } else {
    if (ch->getCond(FULL) >= 0) {
      ch->gainCondition(FULL, (getLiqHunger() * amount) / 10);
      
      // use leftover as chance to go 1 more unit up/down
      if (::number(0,9) < ((abs(getLiqHunger()) * amount) % 10))
        ch->gainCondition(FULL, (getLiqHunger() > 0 ? 1 : -1));
    }

    
    if(ch->hasQuestBit(TOG_IS_ALCOHOLIC) && !getLiqDrunk() && 
       ch->getCond(THIRST) > 3){
        ch->sendTo("Only sweet, sweet alcohol can quench your thirst any further.\n\r");
    } else {
      if (ch->getCond(THIRST) >= 0) {
        ch->gainCondition(THIRST, (getLiqThirst() * amount) / 10);
	
	// use leftover as chance to go 1 more unit up/down
	if (::number(0,9) < ((abs(getLiqThirst()) * amount) % 10))
	  ch->gainCondition(THIRST, (getLiqThirst() > 0 ? 1 : -1));
      }
    }
  }

  if (ch->getCond(DRUNK) > 10)
    act("You feel drunk.", FALSE, ch, 0, 0, TO_CHAR);

  if (ch->getCond(THIRST) > 20)
    act("You do not feel thirsty.", FALSE, ch, 0, 0, TO_CHAR);

  if (ch->getCond(FULL) > 20)
    act("You are full.", FALSE, ch, 0, 0, TO_CHAR);

  if (isDrinkConFlag(DRINK_POISON)) {
    if (ch->isImmune(IMMUNE_POISON, WEAR_BODY)) {
      act("Oops, it tasted rather strange, but you don't think it had any ill-effect.",
            FALSE, ch, 0, 0, TO_CHAR);
    } else {
      act("Oops, it tasted rather strange?!!?", FALSE, ch, 0, 0, TO_CHAR);
      act("$n chokes and utters some strange sounds.", TRUE, ch, 0, 0, TO_ROOM);
      af.type = SPELL_POISON;
      af.duration = amount * 3 * UPDATES_PER_MUDHOUR;
      af.modifier = -20;
      af.location = APPLY_STR;
      af.bitvector = AFF_POISON;

      aff2.type = AFFECT_DISEASE;
      aff2.level = 0;
      aff2.duration = af.duration;
      aff2.modifier = DISEASE_POISON;
      aff2.location = APPLY_NONE;
      aff2.bitvector = AFF_POISON;

      ch->affectJoin(NULL, &af, AVG_DUR_NO, AVG_EFF_YES);
      ch->affectJoin(NULL, &aff2, AVG_DUR_NO, AVG_EFF_YES);
      disease_start(ch, &aff2);
    }
  }
  if (getDrinkType() == LIQ_BLOOD) {
    int numchance = ::number(0,5);
    if (numchance < 4) {
      act("Oops, it tasted rather strange, but you don't think it had any ill-effect.",
            FALSE, ch, 0, 0, TO_CHAR);
    } else {
      act("Oops, it tasted rather strange?!!?", FALSE, ch, 0, 0, TO_CHAR);
      act("$n chokes and utters some strange sounds.", TRUE, ch, 0, 0, TO_ROOM);
      aff2.type = AFFECT_DISEASE;
      aff2.level = 0;
      aff2.duration = af.duration;
      aff2.modifier = DISEASE_SYPHILIS;
      aff2.location = APPLY_NONE;
      aff2.bitvector = AFF_SYPHILIS;
      ch->affectJoin(NULL, &aff2, AVG_DUR_NO, AVG_EFF_YES);
      disease_start(ch, &aff2);

      act("Oops, it tasted rather strange?!!?", FALSE, ch, 0, 0, TO_CHAR);
      act("$n chokes and utters some strange sounds.", TRUE, ch, 0, 0, TO_ROOM);
    }
  }

  
  if(liquidInfo[liquidType]->potion)
    doLiqSpell(ch, ch, getDrinkType(), amount);

  // save this before the call to updatedesc from addToDrinkUnits
  drinkName = sstring(ch->objs(this)).cap();

  if (!isDrinkConFlag(DRINK_PERM))
    addToDrinkUnits(-amount);
  
  TPool * tPool = dynamic_cast<TPool *>(this);
  if (tPool && getDrinkType() == LIQ_WATER && !ch->isImmune(IMMUNE_DISEASE, WEAR_BODY) && !ch->hasDisease(DISEASE_DYSENTERY)) {
    // drinking from standing water: chance for dysentery
    if (!::number(0, 100 / max(1, amount))) {
      aff3.type = AFFECT_DISEASE;
      aff3.level = 0;
      aff3.location = APPLY_NONE;
      aff3.bitvector = 0;
      aff3.duration = amount * 3 * UPDATES_PER_MUDHOUR;
      aff3.duration *= (100 - ch->getImmunity(IMMUNE_DISEASE));
      aff3.duration /= 100;
      aff3.modifier = DISEASE_DYSENTERY;
      aff3.modifier2 = ch->GetMaxLevel();
      if (aff3.duration > 0) {
        act("Uh oh! Standing water isn't always the safest to drink...", FALSE, ch, 0, 0, TO_CHAR);
        ch->affectTo(&aff3);
        disease_start(ch, &aff3);
      }
    }
  }
  if (getDrinkUnits() <= 0) {
    if (!tPool) {
      act(format("%s is completely empty.") % drinkName, FALSE, ch, this, 0, TO_CHAR);
      remDrinkConFlags(DRINK_POISON);
    } else {
      act("You finish licking up $p from the $g.", FALSE, ch, this, 0, TO_CHAR);
      return DELETE_THIS;
    }
  }

  return FALSE;
}

// DELETE_THIS means this must die
int TBeing::doDrink(const char *argument)
{
  TObj *temp;
  int rc;

  if (fight()) {
    sendTo("You are too busy fending off your foes!\n\r");
    return FALSE;
  }
  if (!argument || !*argument) {
    sendTo("Drink from what?\n\r");
    return FALSE;
  }
  if (!(temp = get_obj_vis_accessible(this, argument))) {
    if (roomp->isWaterSector()) {
      sendTo("You drink from the surroundings.\n\r");
      if (roomp->isRiverSector()) {
        setCond(THIRST, 24);
      } else {
        sendTo("ACK!!  Salt water!!!\n\r");
        setCond(THIRST, 0);
        setCond(FULL, 0);
      }
      return TRUE;
    } else if (roomp->isUnderwaterSector()) {
      act("There is so much water around, you'd wind up drowning rather than drinking.", FALSE, this, 0, 0, TO_CHAR);
      return FALSE;
    } else {
      act("You can't find it!", FALSE, this, 0, 0, TO_CHAR);
      return FALSE;
    }
  }
  rc = temp->drinkMe(this);
  if (IS_SET_DELETE(rc, DELETE_THIS)) {
    delete temp;
    temp = NULL;
  }
  if (IS_SET_DELETE(rc, DELETE_VICT)) {
    return DELETE_THIS;
  }
  return FALSE;
}


void TThing::eatMe(TBeing *ch)
{
  if (!ch->isImmortal()) {
    ch->sendTo("That item is impossible to eat!\n\r");
    return;
  }
  act("$n eats $p.", TRUE, ch, this, 0, TO_ROOM);
  act("You eat the $o.", FALSE, ch, this, 0, TO_CHAR);

  delete this;
  return;
}


void TObj::eatMe(TBeing *ch)
{
  if (ch->getMyRace()->hasTalent(TALENT_GARBAGEEATER)) {
    if (isMonogrammed()) {
      ch->sendTo("You can't eat that, its been monogrammed!\n\r");
      return;
    } else if (!isOrganic()) {
      ch->sendTo("Even you wouldn't consider eathing that!\n\r");
      return;
    } else if (!stuff.empty()) {
      ch->sendTo("You can't eat that - its full of something!\n\r");
      return;
    } else if (isObjStat(ITEM_NOJUNK_PLAYER)) {
      ch->sendTo("You can't eat an object that's been marked no-junk.\n\r");
      return;
    } else if (isObjStat(ITEM_BURNING) || isObjStat(ITEM_PROTOTYPE) ||
              isObjStat(ITEM_NOPURGE) || isObjStat(ITEM_NORENT) || isObjStat(ITEM_ATTACHED) ||
              getLocked() || stuckIn || equippedBy || tied_to || riding) {
      ch->sendTo("You can't eat that, it just wouldn't be right!\n\r");
      return;
    } else if (isObjStat(ITEM_NODROP)) {
      ch->sendTo("You can't eat that - its been cursed!\n\r");
      return;
    } else if (getStructPoints() > 20 || getVolume() > 1728) {
      ch->sendTo("There is simply too much of that to eat in one bite.\n\r");
      return;
    } else if (!canWear(ITEM_TAKE)) {
      ch->sendTo("How can you eat that?  You can't even pick it up!\n\r");
      return;
    } else if (dynamic_cast<const TPCorpse*>(this)) {
      ch->sendTo("You probably shouldn't be eating player corpses.\n\r");
      return;
    }

  } else if (!ch->isImmortal()) {
    ch->sendTo("That item is impossible to eat!\n\r");
    return;
  }
  act("$n eats $p.", TRUE, ch, this, 0, TO_ROOM);
  act("You eat the $o.", FALSE, ch, this, 0, TO_CHAR);

  if (ch->getMyRace()->hasTalent(TALENT_GARBAGEEATER))
  {
    int foodValue = max(0, max(int(getStructPoints()), getVolume()/(1728/20)));
    ch->gainCondition(FULL, foodValue);
    if (ch->getCond(FULL) > 20)
      act("You are full.", FALSE, ch, 0, 0, TO_CHAR);
  }

  delete this;
  return;
}


void TFood::Poisoned(TBeing *ch, int dur)
{
  affectedData af;

  if (isFoodFlag(FOOD_POISON) && 
      !ch->isAffected(AFF_POISON)) {
    if (ch->getMyRace()->hasTalent(TALENT_GARBAGEEATER)) {
      act("Mmm, that had a bit of a kick to it!", FALSE, ch, 0, 0, TO_CHAR);
    } else if (ch->isImmune(IMMUNE_POISON, WEAR_BODY)) {
      act("That tasted rather strange, but you don't think it had any ill-effect!!", FALSE, ch, 0, 0, TO_CHAR);
    } else {
      act("That tasted rather strange !!", FALSE, ch, 0, 0, TO_CHAR);
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

void TFood::Spoiled(TBeing *ch, int dur)
{
  affectedData af;

  if (isFoodFlag(FOOD_SPOILED)) {
    if (ch->getMyRace()->hasTalent(TALENT_GARBAGEEATER)) {
      ch->sendTo("Mmm, that was tangy!\n\r");
    } else if (ch->isImmune(IMMUNE_POISON, WEAR_BODY)) {
      ch->sendTo("BLAH!  That was some rotten food.  Hopefully you won't have any ill-effect.\n\r");
    } else {
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

void TFood::eatMe(TBeing *ch)
{
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
  delete this;
}

void TBeing::doEat(const char *argument)
{
  char buf[100];

  one_argument(argument, buf, cElements(buf));

  if (fight() && !isImmortal()) {
    sendTo("You are too busy fending off your foes!\n\r");
    return;
  }
  
  TThing *temp = generic_find_obj(buf, FIND_OBJ_INV | FIND_OBJ_HELD, this);

  if(!temp)
    temp = generic_find_obj(buf, FIND_OBJ_ROOM, this);

  //  TThing *temp = get_thing_char_using(this, buf, -1, false, false);
  if (!temp) {
    sendTo("You can't find it!\n\r");
    return;
  }
  if (hasDisease(DISEASE_FOODPOISON)) {
    sendTo("Uggh, your stomach feels just horrible and the thought of food nauseates you.\n\r");
    sendTo("You decide to skip this meal until you feel better.\n\r");
    return;
  }
  temp->eatMe(this);
  doSave(SILENT_YES);
  // temp may not be valid anymore
}

void TObj::pourMeOut(TBeing *ch)
{
  act("You can't pour from that!", FALSE, ch, 0, 0, TO_CHAR);
  return;
}

void TObj::pourMeIntoDrink1(TBeing *ch, TObj *)
{
  act("You can't pour from $p!", FALSE, ch, this, 0, TO_CHAR);
  return;
}

void TObj::pourMeIntoDrink2(TBeing *ch, TBaseCup *)
{
  act("You can't pour anything into $p!", FALSE, ch, this, 0, TO_CHAR);
  return;
}

void TBaseCup::pourMeIntoDrink2(TBeing *ch, TBaseCup *from_obj)
{
  int temp;

  if(from_obj->getDrinkUnits() == 0){
    act("$p is empty.", FALSE, ch, from_obj, 0, TO_CHAR);
    return;
  }

  if(from_obj->isDrinkConFlag(DRINK_FROZEN)){
    act("$p is frozen solid.", FALSE, ch, from_obj, 0, TO_CHAR);
    return;
  }

  if ((getDrinkUnits() != 0) &&
      (getDrinkType() != from_obj->getDrinkType())) {
    act("There is already another liquid in it!", FALSE, ch, 0, 0, TO_CHAR);
    return;
  }
  if (getDrinkUnits() >= getMaxDrinkUnits()) {
    act("There is no room for more.", FALSE, ch, 0, 0, TO_CHAR);
    return;
  }
  if (from_obj == this) {
    ch->sendTo("How are you going to pour from something into itself?\n\r");
    return;
  }
  ch->sendTo(COLOR_OBJECTS, format("You pour %s into %s.\n\r") % 
          liquidInfo[from_obj->getDrinkType()]->name % ch->objs(this));

  // set liquid type
  setDrinkType(from_obj->getDrinkType());

  temp = min(from_obj->getDrinkUnits(),
       getMaxDrinkUnits() - getDrinkUnits());

  if(!from_obj->isDrinkConFlag(DRINK_PERM))
    from_obj->addToDrinkUnits(-temp);
  addToDrinkUnits(temp);

  from_obj->weightChangeObject(-temp * SIP_WEIGHT);
  weightChangeObject(temp * SIP_WEIGHT);

  //  addDrinkConFlags(from_obj->getDrinkConFlags());
  if(from_obj->isDrinkConFlag(DRINK_POISON))
    addDrinkConFlags(DRINK_POISON);
}

int TBeing::doPour(const char *argument)
{
  char arg1[256], arg2[256];
  TObj *from_obj;
  TObj *to_obj;
  int  rc;

  argument_interpreter(argument, arg1, cElements(arg1), arg2, cElements(arg2));

  if (!*arg1) {
    act("What do you want to pour from?", FALSE, this, 0, 0, TO_CHAR);
    return FALSE;
  }
  TThing *t_from_obj = searchLinkedListVis(this, arg1, stuff);
  from_obj = dynamic_cast<TObj *>(t_from_obj);
  if (!from_obj) {
    act("You can't find it!", FALSE, this, 0, 0, TO_CHAR);
    return FALSE;
  }
  if (!*arg2) {
    act("Where do you want it? Out or in what?", FALSE, this, 0, 0, TO_CHAR);
    return FALSE;
  }
  if (!strcasecmp(arg2, "out")) {
    from_obj->pourMeOut(this);
    return FALSE;
  }

  TThing *t_to_obj = searchLinkedListVis(this, arg2, stuff);
  if (!t_to_obj)

    t_to_obj = searchLinkedListVis(this, arg2, roomp->stuff);
  if (t_to_obj && (rc = t_to_obj->pourWaterOnMe(this, from_obj)) != 0) {
    if (rc == -1 && dynamic_cast<TBeing *>(t_to_obj)) {
      dynamic_cast<TBeing *>(t_to_obj)->reformGroup();
      delete t_to_obj;
      t_to_obj = NULL;
    }
    return FALSE;
  }
  to_obj = dynamic_cast<TObj *>(t_to_obj);
  if (!to_obj) {
    t_to_obj = searchLinkedListVis(this, arg2, roomp->stuff);
    to_obj = dynamic_cast<TObj *>(t_to_obj);
  }
  if (!to_obj) {
    act("You can't find it!", FALSE, this, 0, 0, TO_CHAR);
    return FALSE;
  }
  from_obj->pourMeIntoDrink1(this, to_obj);

  return FALSE;
}

void TObj::sipMe(TBeing *ch)
{
  act("You can't sip from that!", FALSE, ch, 0, 0, TO_CHAR);
}

void TBaseCup::sipMe(TBeing *ch)
{
  sstring drinkName;
  affectedData af;

  if (ch->getCond(DRUNK) > 10) {    /* The pig is drunk ! */
    act("You simply fail to reach your mouth!", FALSE, ch, 0, 0, TO_CHAR);
    act("$n tries to sip, but fails!", TRUE, ch, 0, 0, TO_ROOM);
    return;
  }
  if(isDrinkConFlag(DRINK_FROZEN)){
    act("It's frozen solid, you can't sip it.", FALSE, ch, 0, 0, TO_CHAR);
    return;
  }
  if (!getDrinkUnits()) {
    act("But there is nothing in it?", FALSE, ch, 0, 0, TO_CHAR);
    return;
  }
  if (ch->hasDisease(DISEASE_FOODPOISON)) {
    ch->sendTo("Uggh, your stomach is queasy and the thought of doing that is unappetizing.\n\r");
    ch->sendTo("You decide to skip this drink until you feel better.\n\r");
    return;
  }
  act("You sip from the $o.", FALSE, ch, this, NULL, TO_CHAR);
  act("$n sips from the $o.", TRUE, ch, this, 0, TO_ROOM);
  ch->sendTo(COLOR_OBJECTS, format("It tastes like %s.\n\r") % liquidInfo[getDrinkType()]->name);

  bool noFood = ch->getMyRace()->hasTalent(TALENT_FISHEATER) || ch->getMyRace()->hasTalent(TALENT_MEATEATER) ||
                ch->getMyRace()->hasTalent(TALENT_INSECT_EATER);

  if(ch->isVampire()){
    ch->sendTo("You drink the mortal food, but it has no affect on you.\n\r");
  } else if (noFood) {
    act("You drink the $o but it doesn't seem very nourishing.", FALSE, ch, this, NULL, TO_CHAR);
  } else {
    if (getLiqDrunk()) {
      ch->gainCondition(DRUNK, (getLiqDrunk() / 10));
      // use leftover as chance to go 1 more unit up/down
      if (::number(0,9) < (abs(getLiqDrunk()) % 10))
        ch->gainCondition(DRUNK, (getLiqDrunk() > 0 ? 1 : -1));
    }
    
    ch->gainCondition(FULL, (getLiqHunger() / 10));
    // use leftover as chance to go 1 more unit up/down
    if (::number(0,9) < (abs(getLiqHunger()) % 10))
      ch->gainCondition(FULL, (getLiqHunger() > 0 ? 1 : -1));
    
    ch->gainCondition(THIRST, (getLiqThirst() / 10));
    // use leftover as chance to go 1 more unit up/down
    if (::number(0,9) < (abs(getLiqThirst()) % 10))
      ch->gainCondition(THIRST, (getLiqThirst() > 0 ? 1 : -1));
  }

  if (!isDrinkConFlag(DRINK_PERM) || (getMaxDrinkUnits() > 19))
    weightChangeObject( -SIP_WEIGHT);

  if (ch->getCond(DRUNK) > 10)
    act("You feel drunk.", FALSE, ch, 0, 0, TO_CHAR);

  if (ch->getCond(THIRST) > 20)
    act("You do not feel thirsty.", FALSE, ch, 0, 0, TO_CHAR);

  if (ch->getCond(FULL) > 20)
    act("You are full.", FALSE, ch, 0, 0, TO_CHAR);

  if (isDrinkConFlag(DRINK_POISON) && !ch->isAffected(AFF_POISON)) {
    if (ch->isImmune(IMMUNE_POISON, WEAR_BODY)) 
      act("But it also had a strange aftertaste!", FALSE, ch, 0, 0, TO_CHAR);
    else {
      act("But it also had a strange aftertaste!", FALSE, ch, 0, 0, TO_CHAR);
      af.type = SPELL_POISON;
      af.duration = 3 * UPDATES_PER_MUDHOUR;
      af.modifier = 0;
      af.location = APPLY_NONE;
      af.bitvector = AFF_POISON;
      ch->affectTo(&af);
    }
  }

  if(liquidInfo[liquidType]->potion)
    doLiqSpell(ch, ch, getDrinkType(), 1);

  // save this before the call to updatedesc from addToDrinkUnits
  drinkName = sstring(ch->objs(this)).cap();

  if (!isDrinkConFlag(DRINK_PERM))
    addToDrinkUnits(-1);

  if (getDrinkUnits() <= 0) {
    TPool * tPool = dynamic_cast<TPool *>(this);

    if (!tPool) {
      act(format("%s is completely empty.") % drinkName, FALSE, ch, this, 0, TO_CHAR);
      remDrinkConFlags(DRINK_POISON);
    } else {
      act("You finish licking up $p from the $g.", FALSE, ch, this, 0, TO_CHAR);

      delete this;
      return;
    }
  }
}

void TBeing::doSip(const char *argument)
{
  char arg[256];
  TObj *temp;

  one_argument(argument, arg, cElements(arg));

  if (fight()) {
    sendTo("You are too busy fending off your foes!\n\r");
    return;
  }
  if (!(temp = get_obj_vis_accessible(this, arg))) {
    act("You can't find it!", FALSE, this, 0, 0, TO_CHAR);
    return;
  }
  temp->sipMe(this);
  return;
}

void TObj::tasteMe(TBeing *ch)
{
  if (ch->getMyRace()->hasTalent(TALENT_GARBAGEEATER) && isOrganic()) {
    act("That looks edible, but you'd rather eat it in one big bite.", FALSE, ch, 0, 0, TO_CHAR);
    return;
  }
  act("Taste that?!? Your stomach refuses!", FALSE, ch, 0, 0, TO_CHAR);
}

void TFood::tasteMe(TBeing *ch)
{
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
    delete this;
    return;
  }
  return;
}

void TBeing::doTaste(const char *argument)
{
  char arg[80];
  TObj *temp;

  one_argument(argument, arg, cElements(arg));

  if (fight()) {
    sendTo("You are too busy fending off your foes!\n\r");
    return;
  }
  if (!(temp = get_obj_vis_accessible(this, arg))) {
    act("You can't find it!", FALSE, this, 0, 0, TO_CHAR);
    return;
  }
  temp->tasteMe(this);
  // temp may no longer be valid
}

void TBeing::foodNDrink(sectorTypeT sector, int modifier)
{
  int food   = 11 - modifier * TerrainInfo[sector]->hunger;
  int thirst = 9 - modifier * TerrainInfo[sector]->thirst;
  int drunk  = 9 - modifier * TerrainInfo[sector]->drunk;

  food = max(0,food);
  thirst = max(0,thirst);
  drunk = max(0,drunk);
  if ((::number(0,9) < 6) && !::number(0,food)){
    gainCondition(FULL, -1);
    gainCondition(POOP, 1);
  }
  
  if ((::number(0,9) < 5) && !::number(0,thirst)){
    gainCondition(THIRST, -1);
    gainCondition(PEE, 1);
  }
  
  if ((::number(0,9) < 6) && !::number(0,drunk)) 
    gainCondition(DRUNK, -1);
  
  return;
}

void TBeing::doFill(const char *arg)
{
  char arg1[256], arg2[256];
  int bits;
  TObj *obj1;
  TObj *obj2;
  TBeing *tmp;

  argument_interpreter(arg, arg1, cElements(arg1), arg2, cElements(arg2));
  // we use to check for FIND_OBJ_EQUIP too
  // since doDrink doesn't check for stuff in equip, neither should this
  bits = generic_find(arg1, FIND_OBJ_INV | FIND_OBJ_ROOM, 
		      this, &tmp, &obj1);
  if (!bits) {
    sendTo("You must fill SOMETHING!!\n\r");
    return;
  }
  
  if(*arg2){
    bits = generic_find(arg2, FIND_OBJ_INV | FIND_OBJ_ROOM, 
			this, &tmp, &obj2);
    if (!bits) {
      sendTo("You must fill from SOMETHING!!\n\r");
      return;
    }

    obj2->pourMeIntoDrink1(this, obj1);
    return;
  } else {
    TBaseCup *tbc=NULL;
    for(StuffIter it=roomp->stuff.begin();it!=roomp->stuff.end();++it){
      if((tbc=dynamic_cast<TBaseCup *>(*it)) && tbc->getDrinkUnits()>0){
	tbc->pourMeIntoDrink1(this, obj1);
	return;
      }
    }
  }
  
  if(roomp->isWaterSector() || roomp->isUnderwaterSector()) {
    obj1->fillMe(this, roomp->isRiverSector() ? LIQ_WATER : LIQ_SALTWATER);
  } else 
    doNotHere();
  
  return;
}

void TBeing::checkForSpills() const
{
  TThing *t=NULL, *t2=NULL;
  int i;

  for (i = MIN_WEAR; i < MAX_WEAR; i++) {
    if ((t = equipment[i])) {
      t->spill(this);
      for(StuffIter it=t->stuff.begin();it!=t->stuff.end() && (t2=*it);++it) {
        t2->spill(this);
      }
    }
  }
  for(StuffIter it=stuff.begin();it!=stuff.end() && (t=*it);++it) {
    t->spill(this);
    for(StuffIter it=t->stuff.begin();it!=t->stuff.end() && (t2=*it);++it) {
      t2->spill(this);
    }
  }
}

void TBeing::setCond(condTypeT i, short val)
{
  // Don't set thirst/hunter/drunk for immortals, even if in mortal form at current.
  if (GetMaxLevel() > MAX_MORT) {
    specials.conditions[i] = -1;
    return;
  }

  specials.conditions[i] = val;
}

short TBeing::getCond(condTypeT i) const
{
  return (specials.conditions[i]);
}

int TBeing::drunkMinus() const
{
   return ((getCond(DRUNK) <= 4) ? 0 : ((getCond(DRUNK) <= 10) ? 1 :
          ((getCond(DRUNK) <= 15) ? 2 : 3)));
}

TFood::TFood() :
  TObj(),
  foodFlags(0),
  foodFill(0)
{
}

TFood::TFood(const TFood &a) :
  TObj(a),
  foodFlags(a.foodFlags),
  foodFill(a.foodFill)
{
}

TFood & TFood::operator=(const TFood &a)
{
  if (this == &a) return *this;
  TObj::operator=(a);
  foodFlags = a.foodFlags;
  foodFill = a.foodFill;
  return *this;
}

TFood::~TFood()
{
}

int TFood::getFoodFill() const
{
  return foodFill;
}

void TFood::setFoodFill(int r)
{
  foodFill = r;
}

unsigned int TFood::getFoodFlags() const
{
  return foodFlags;
}

void TFood::setFoodFlags(unsigned int r)
{
  foodFlags = r;
}

bool TFood::isFoodFlag(unsigned int r) const
{
  return ((foodFlags & r) != 0);
}

void TFood::addFoodFlags(unsigned int r)
{
  foodFlags |= r;
}

void TFood::remFoodFlags(unsigned int r)
{
  foodFlags &= ~r;
}

int TFood::suggestedPrice() const
{
  int decay = obj_flags.decay_time < 1 ? 5000 : obj_flags.decay_time;
  return (int) ((pow(getFoodFill(), .75) * pow(decay, .35)) +
		(int)(10.0 * getWeight() * material_nums[getMaterial()].price));
}

int TFood::productionPrice() const
{
  // who comes up with these formulas, christ
  int decay = obj_flags.decay_time < 1 ? 5000 : obj_flags.decay_time;
  int p=suggestedPrice();
  float x=pow(getFoodFill(), .75);
  float y=pow(decay, .35);
  float m=(int)(10.0 * getWeight() * material_nums[getMaterial()].price);

  // basically just subtraction the portion allocated to food fill
  return (int)(p - ((p-m) * (x / (x+y))));
}

void TFood::lowCheck()
{
  if (getFoodFill() <= 0)
        vlogf(LOG_LOW,format("food (%s) with bad fills value.") %  getName());

  int vModified = suggestedPrice();
  if (vModified != obj_flags.cost) {
    vlogf(LOG_LOW, format("food (%s:%d) with bad price.  Should be: %d.") % 
          getName() % objVnum() % vModified);
    obj_flags.cost = vModified;
  }
  if (obj_flags.decay_time < 0 && !isObjStat(ITEM_MAGIC)) {
    vlogf(LOG_LOW, format("food (%s:%d) with bad decay.  Should be magic or decay.") % 
          getName() % objVnum());
  }

  if ((vModified = (getFoodFill() * 10)) != getVolume()) {
    vlogf(LOG_LOW, format("food (%s) with bad volume.  Should be: %d.") % 
          getName() % vModified);
    setVolume(vModified);
  }
#if 0
  if ((vModified = (int) (getFoodFill() / 12)) != getWeight()) {
    vlogf(LOG_LOW, format("food (%s) with bad weight.  Should be: %d") % 
          getName() % vModified);
    setWeight(vModified);
  }
#endif
  TObj::lowCheck();
}

sstring TFood::statObjInfo() const
{
  char buf[256];

  sprintf(buf, "Makes full : %d      Poisoned : %d",
          getFoodFill(), getFoodFlags());

  sstring a(buf);
  return a;
}

int TFood::objectSell(TBeing *ch, TMonster *keeper)
{
  keeper->doTell(ch->getName(), "I'm sorry, I don't purchase food.");
  return TRUE;
}

bool TFood::objectRepair(TBeing *ch, TMonster *repair, silentTypeT silent)
{
  if (!silent) {
    repair->doTell(fname(ch->name), "you might wanna take that to the diner!");
  }
  return TRUE;
}

int TFood::objectDecay()
{
  if(isFoodFlag(FOOD_SPOILED)){
    return FALSE;
  } else {
    addFoodFlags(FOOD_SPOILED);
    obj_flags.decay_time = getVolume()*10;
  }
  return TRUE;
}

void TFood::assignFourValues(int x1, int, int, int x4)
{
  setFoodFill(x1);
  setFoodFlags((unsigned) x4);
}

void TFood::getFourValues(int *x1, int *x2, int *x3, int *x4) const
{
  *x1 = getFoodFill();
  *x2 = 0;
  *x3 = 0;
  *x4 = getFoodFlags();
}

bool TFood::poisonObject()
{
  addFoodFlags(FOOD_POISON);
  return TRUE;
}

void TFood::nukeFood()
{
  delete this;
}

void TFood::purchaseMe(TBeing *ch, TMonster *keeper, int cost, int shop_nr)
{
  TShopOwned tso(shop_nr, keeper, ch);
  tso.doBuyTransaction(cost, getName(), TX_BUYING, this);
}

void TFood::sellMeMoney(TBeing *ch, TMonster *keeper, int cost, int shop_nr)
{
  TShopOwned tso(shop_nr, keeper, ch);
  tso.doSellTransaction(cost, getName(), TX_SELLING);
}

int TFood::chiMe(TBeing *tLunatic)
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

  act("You focus your chi, causing $p to become a little fresher!",
      FALSE, tLunatic, this, NULL, TO_CHAR);
  act("$n stares at $p, causing it to become a little fresher!",
      TRUE, tLunatic, this, NULL, TO_ROOM);

  obj_flags.decay_time += ::number(1, 3);

  return true;
}
