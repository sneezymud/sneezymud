/*************************************************************************
  
      SneezyMUD - All rights reserved, SneezyMUD Coding Team
      physics.cc : functions to add gravity/reality to the game :)
  
*************************************************************************/

#include "stdsneezy.h"
#include "combat.h"

bool TBeing::hasBoat() const
{
  int has_boat = FALSE;
  TThing *t;

  for (t = getStuff(); t; t = t->nextThing)
    t->usingBoat(&has_boat);

  return has_boat;
}

bool TBeing::isSwimming() const
{
  if (roomp->isUnderwaterSector() || 
      (roomp->isWaterSector() && !isLevitating() && !isFlying())) {
    if (hasBoat())
      return FALSE;
    return TRUE;
  }
  return FALSE;
}

/* returns true if climb roll successful. false = fall */
bool TBeing::canClimb()
{
  int skill, num;
  TBeing *tbt;

  if (!isPc() && canFly() && !isFlying()) {
    doFly();
  } else {
    tbt = dynamic_cast<TBeing *>(riding);
    if (tbt && !tbt->isPc() && tbt->canFly() && !tbt->isFlying()) {
      tbt->doFly();
    }
  }

  if (isFlying())
    return TRUE;
  if (riding && riding->isFlying())
    return TRUE;

  if (raceHasNaturalClimb())
    return TRUE;
  tbt = dynamic_cast<TBeing *>(riding);
  if (tbt && tbt->raceHasNaturalClimb())
    return TRUE;

  if (getDiscipline(DISC_ADVENTURING))
    skill = 5 * getSkillValue(SKILL_CLIMB);
  else
    skill = 0;

  learnFromDoingUnusual(LEARN_UNUSUAL_NORM_LEARN, SKILL_CLIMB, 1);

  skill += plotStat(STAT_CURRENT, STAT_AGI, 15, 100, 65);
  skill -= 10 * drunkMinus();
  skill -= (int) (getTotalWeight(FALSE)/5.0);
  skill -= getCarriedVolume()/100;
  skill -= (fight() ? 125 : 0);
  skill -= (heldInPrimHand() ? 65 : 0);
  skill -= (heldInSecHand() ? 35 : 0);
  skill -= (bothLegsHurt() ? 120 : 0);
  skill -= (canUseLeg(LEG_PRIMARY) ? 0 : 45);
  skill -= (canUseLeg(LEG_SECONDARY) ? 0 : 45);
  skill -= (canUseArm(HAND_PRIMARY) ? 0 : 65);
  skill -= (canUseArm(HAND_SECONDARY) ? 0 : 65);
  num = 100 - GetMaxLevel();
  skill += ::number(-(3*num),(2*num));

  if (skill >= 0)
    return TRUE;
  else {
    if (!clearpath(in_room, DIR_DOWN)) {
      if (roomp->dir_option[DIR_DOWN] && 
          IS_SET(roomp->dir_option[DIR_DOWN]->condition, EX_CLOSED)) {
        return TRUE;
      } else {
        vlogf(LOG_BUG,fmt("%s falling from room %d with no down dir.") % getName() % in_room);
        return TRUE;
      }
    } else if (!riding) {
      sendTo("You lose your grip and begin to plummet earthward!\n\r");
    } else {
      sendTo("Your cursed mount slips and tumbles earthward!\n\r");
      sendTo("You, however, manage to grab ahold of something.\n\r");
    }
    return FALSE;
  }
}

/* function returns true if the char should NOT fall */
bool TBeing::sectorSafe()
{
  TRoom *rp;
  if (!(rp = roomp))
    return TRUE;
  if (!rp->isFallSector())
    return TRUE;

  if (rp->isAirSector() || rp->isFlyingSector())
    return FALSE;

  if (canClimb())
    return TRUE;

  return FALSE;    // climb failed and they fall
}

bool TBeing::canSwim(dirTypeT dir)
{
  TRoom *rp;

  if (isImmortal())
    return TRUE;

  if (eitherLegHurt()) {
    sendTo("Your damaged legs makes it impossible to swim.\n\r");
    return 0;
  }
  if (IS_SET(specials.affectedBy, AFF_SWIM))
    return 1;
  
  if (!getDiscipline(DISC_ADVENTURING)) {
    if (!::number(0,5))
      return 1;
    else
      return 0;
  }
  if (!(rp = roomp))
    return 0;

  if (!(rp->isWaterSector() || rp->isUnderwaterSector())) {
    vlogf(LOG_BUG,"can swim called in non-water room.");
    return 0;
  }

  // some basic info:
  // things sink if their density is greater than the density of the medium
  // they are in.  The density of water is roughly 1 gram/cm^3.
  // 1 pound = 4.4482 newton
  // 1 newton = 0.102 kg assuming 9.8 m/s^2 gravity
  // 1 pound = 0.454 kg assuming Earth gravity
  // 1 pound = 454 grams assuming Earth gravity
  const int GRAMS_PER_POUND = 454;
  // 1 inch = 2.54 cm
  // density = mass/volume
  int mass = (int) (getTotalWeight(TRUE) * GRAMS_PER_POUND);
  int volume = (int) (getVolume() * 2.54 * 2.54 * 2.54);

  // make some adjustments based on skill and difficulty

  // strong endurance means you can paddle upwards a lot, offset mass some
  // bad = 25 pounds more weight, excellent, can carry 50 extra pounds
  mass += plotStat(STAT_CURRENT, STAT_CON, 25 * GRAMS_PER_POUND, -50 * GRAMS_PER_POUND, 0);

  // salt water has a higher density (1.2 g/cm^3)
  // offset the volume to account for this
  if (rp->isOceanSector()) {
    volume = (int) (volume * 1.2);
  }

  // rough water hard to swim in
  // worst case (speed=1): 75 pounds more weight
  // best case (speed=100): 5 pounds more weight
  // slope = -70/99, intercept = 75
  if (rp->getRiverSpeed())
    mass += (int) ((75 + rp->getRiverSpeed() * -70.0 / 99.0) * GRAMS_PER_POUND);

  mass += (!canUseLimb(WEAR_LEGS_R)) ? 5 * GRAMS_PER_POUND : 0;
  mass += (!canUseLimb(WEAR_LEGS_L)) ? 5 * GRAMS_PER_POUND : 0;
  
  // adjust for swimming with/against current
  if (rp->getRiverSpeed()) {
    if (dir == DIR_NONE)  // floating in place
      mass += 5 * GRAMS_PER_POUND;  // tough to stay where I am
    else if (dir == rp->getRiverDir())
      mass += -30 * GRAMS_PER_POUND;  // easy to go with flow
    else if (dir == rev_dir[rp->getRiverDir()])
      mass += 30 * GRAMS_PER_POUND;  // tough to go against the flow
  }
  
  // significantly cut the mass (level based) if they have the proper spell
  // L50 should cut it in half
  // note that garmul's tail also raises swim skill, so that spell not only
  // decreases weight (guaranteed) here, it raises chance of bSuccess
  // succeeding later on too
  affectedData *aff;
  for (aff = affected; aff; aff = aff->next)
    if (aff->type == SPELL_GARMULS_TAIL)
      mass = (int) (mass * MAX_MORT / (MAX_MORT + aff->level));

  // good swimming skill allows extra 60 pounds of mass
  int swim = getSkillValue(SKILL_SWIM);
  if (bSuccess(swim, SKILL_SWIM))
    mass += (int) ((-0.6 * swim) * GRAMS_PER_POUND);

  // this is ok to do since mass is in grams, and volume is in cm^3
  // this essentially says if my density is < 1.0 g/cm^3, i swim
  if (mass < volume) {
    return 1;
  } else {
    return 0;
  }
}

bool TObj::willFloat()
{
  int x = material_nums[getMaterial()].float_weight;

  if ((x == 255) || 
      // weight <= x
      (compareWeights(getWeight(), (float) x) != -1) ||
      isObjStat(ITEM_FLOAT))
    return TRUE;

  // grimhaven sewer pipe - high pressure!
  if(inRoom()==18982 || inRoom()==27250)
    return TRUE;

  return FALSE;
}

int check_sinking_obj(TObj *obj, int room)
{
  TRoom *rp;

  if (room == ROOM_NOWHERE)
    return FALSE;
  if (!(rp = obj->roomp) || !rp->isWaterSector())
    return FALSE;

  if (obj->willFloat()) {
    if (!rp->isUnderwaterSector() || !clearpath(obj->in_room, DIR_UP))
      return FALSE;
    sendrpf(rp, "%s floats silently upward.\n\r", sstring(obj->shortDescr).cap().c_str());
    if (!(rp = real_roomp(obj->roomp->dir_option[DIR_UP]->to_room))) {
      vlogf(LOG_BUG, "Serious bug in floating objects!");
      return FALSE;
    }
    --(*obj);
    *rp += *obj;
    if (rp->isWaterSector())
      sendrpf(rp, "%s floats up from below.\n\r", sstring(obj->shortDescr).cap().c_str());
    else
      sendrpf(rp, "%s pops out of a burst of water from below.\n\r", sstring(obj->shortDescr).cap().c_str());
  } else {
    if (!clearpath(obj->in_room, DIR_DOWN))
      return FALSE;
    sendrpf(rp, "%s sinks downward into the water.\n\r", sstring(obj->shortDescr).cap().c_str());
    if (!(rp = real_roomp(obj->roomp->dir_option[DIR_DOWN]->to_room))) {
      vlogf(LOG_BUG, "Serious bug in sinking objects!");
      return FALSE;
    }
    --(*obj);
    *rp += *obj;
    if (rp->isWaterSector()){
      sendrpf(rp, "%s sinks into the water from above.\n\r", sstring(obj->shortDescr).cap().c_str());
      obj->extinguishWater();
    } else
      sendrpf(rp, "%s drops with a gush of water from above.\n\r", sstring(obj->shortDescr).cap().c_str());

    
  }
  return TRUE;
}

int TBeing::checkSinking(int)
{
  TRoom *rp;
  TBeing *tbr = dynamic_cast<TBeing *>(riding);

  if (!(rp = roomp) || !rp->isWaterSector() || 
      (desc && (desc->connected > 20)) ||
      !clearpath(in_room, DIR_DOWN) || 
      (tbr &&
          (tbr->isAffected(AFF_SWIM) || 
           tbr->isLevitating() ||
           tbr->isFlying())) ||
      isFlying() || isLevitating())
    return FALSE;

  if (fight())
    return FALSE;

  bool has_boat = hasBoat();
  if (has_boat)
    return FALSE;

  if (canSwim(DIR_NONE) > 0) {
    if (!(::number(0, 20)) || getRace() == RACE_DWARF) {
      act("$n swims to stay afloat.", FALSE, this, 0, 0, TO_ROOM);
      sendTo("You swim hard to stay afloat.\n\r");
      addToMove(-3);
      if (!getMove() || (getRace() == RACE_DWARF && !affectedBySpell(AFFECT_TRANSFORMED_LEGS))) {
        if (getRace() == RACE_DWARF)
          sendTo("The weight of your dwarven limbs gets to you!\n\r");
        else
          sendTo("Your swimming has totally exhausted you.\n\r");
        sendTo("You SINK into the waters!\n\r");
        act("$n sinks downwards, into the waters.", FALSE, this, 0, 0, TO_ROOM);
        --(*this);
        rp = real_roomp(rp->dir_option[DIR_DOWN]->to_room);
        *rp += *this;
        doLook("", CMD_LOOK);
        if (rp->isWaterSector())
          act("Flailing frantically, $n sinks into the room from above.", FALSE, this, 0, 0, TO_ROOM);
        else
          act("$n falls from a gush of water above.", FALSE, this, 0, 0, TO_ROOM);
      }
    }
    return FALSE;
  }

  if (riding)
    dismount(POSITION_STANDING);
  if (rider)
    rider->dismount(POSITION_STANDING);

  sendTo("You SINK into the waters!\n\r");
  act("$n sinks downwards, into the waters.", FALSE, this, 0, 0, TO_ROOM);
  --(*this);
  rp = real_roomp(rp->dir_option[DIR_DOWN]->to_room);
  *rp += *this;
  doLook("", CMD_LOOK);
  if (rp->isWaterSector())
    act("Flailing frantically, $n sinks into the room from above.", FALSE, this, 0, 0, TO_ROOM);
  else
    act("$n falls from a gush of water above.", FALSE, this, 0, 0, TO_ROOM);

  return TRUE;
}

// note, hokey formula is the same as in thrown objects.  This will need tweek
int obj_hit_mobs(TObj *o, TRoom *rp)
{
  int rc;
  TBeing *c = NULL;
  TThing *t, *t2;
  
  for (t = rp->getStuff(); t; t = t2) {
    t2 = t->nextThing;
    c = dynamic_cast<TBeing *>(t);
    if (!c)
      continue;
    if (!number(0, 2) && hitInnocent(NULL, o, c)) {
      if (number(5, 30) < c->plotStat(STAT_CURRENT, STAT_SPE, 3, 19, 13)) {
        act("$n dodges out of the way of $p.", FALSE, c, o, NULL, TO_ROOM);
        c->sendTo("You dodge out of its way.\n\r");
      } else {
        act("$n is smacked by $p!", FALSE, c, o, NULL, TO_ROOM);
        act("You are unable to dodge being hit by $p!", FALSE, c, o, NULL, TO_CHAR);
        rc = c->damageEm(min(max(0, ((int) o->getWeight() - 5)), 30), 
             "killed by a falling object!",DAMAGE_FALL);
        if (IS_SET_ONLY(rc, DELETE_THIS)) {
          delete c;
          c = NULL;
          return TRUE;
        }
      }
      return TRUE;
    }
  }
  return FALSE;
}


/* another hokey formula twisted from range.c .. this time doing a bit of damage */
int obj_hit_objs(TObj *o, TRoom *rp)
{
  TObj *t = NULL;
  TThing *a;
  int d;

  for (a = rp->getStuff(); a; a = a->nextThing) {
    t = dynamic_cast<TObj *>(a);
    if (!t)
      continue;
    if ((t != o) && (t->getVolume() > 10000) &&
        ((t->getVolume() + o->getVolume() ) > ((dice(1, 500) * 1000)))) {
      d = (min(max(0, ((int) o->getWeight() - 5)), 30) / 6);
      if ((t->obj_flags.struct_points -= d) <= 0) {
        sendrpf(rp, "%s DESTROYS %s, as it smacks into it!\n\r", o->shortDescr, t->shortDescr);
        delete t;
        t = NULL;
        return TRUE;
      }
      if (d)
        sendrpf(rp, "%s damages %s, as it smacks into it!\n\r", o->shortDescr, t->shortDescr);
      else
        sendrpf(rp, "%s bounces off %s, spinning wildly!\n\r", o->shortDescr, t->shortDescr);
      return TRUE;
    }
  }
  return FALSE;
}


// return DELETE_THIS
int TObj::checkFalling()
{
  int count, i, damaged, water;

  if (!roomp || !roomp->isFallSector() || 
      !clearpath(in_room, DIR_DOWN) || 
      (compareWeights(getWeight(), 0.0) != -1)) {
      // weight <= 0
    return FALSE;
  }

  if (isObjStat(ITEM_ATTACHED))
    return FALSE;

  count = 0;

  int rc = checkSpec(NULL, CMD_OBJ_START_TO_FALL, "", NULL);
  if (IS_SET_DELETE(rc, DELETE_THIS))
    return DELETE_THIS;

  while (count < 100) {
    sendrpf(roomp, "%s drops downward.\n\r", sstring(shortDescr).cap().c_str());
    if (!roomp) {
      vlogf(LOG_BUG, "Serious bug in falling objects!");
      return FALSE;
    }
    roomDirData * rdd = roomp->exitDir(DIR_DOWN);
    if (!rdd) {
      vlogf(LOG_BUG, "Serious bug in falling objects!");
      return FALSE;
    }
    int new_room = rdd->to_room;
    TRoom * rp = real_roomp(new_room);
    if (!rp) {
      vlogf(LOG_BUG, "Serious bug in falling objects!");
      return FALSE;
    }
    --(*this);
    *rp += *this;
    sendrpf(rp, "%s falls from above.\n\r", sstring(shortDescr).cap().c_str());
    count++;
    if (!obj_hit_mobs(this, roomp))
      obj_hit_objs(this, roomp);

    if (!clearpath(in_room, DIR_DOWN) || !roomp->isFallSector()) {
      if ((water = roomp->isWaterSector()))
        count >>= 1;                /* water is a bit softer for damage purposes */
      damaged = FALSE;
      if (obj_flags.struct_points != -1)
        for (i = 0; i < count; i++)
          if (dice(1, 10) <= (material_nums[getMaterial()].fall_susc % 10)) {
            damaged = TRUE;
            if ((obj_flags.struct_points -= (material_nums[getMaterial()].fall_susc / 10)) <= 0) {
	      if(water)
                act("With a loud SPLASH, $n strikes the water and is utterly destroyed.",
                  FALSE, this, 0, 0, TO_ROOM);
              else
                act("With a loud CLUNK, $n smacks into the $g and is utterly destroyed.",
                  FALSE, this, 0, 0, TO_ROOM);
              return DELETE_THIS;
            }
          }
      if (damaged) {
        if (water)
          act("With a loud SPLASH, $n is damaged as it strikes the water.",
                  FALSE, this, 0, 0, TO_ROOM);
        else
          act("With a loud CLUNK, $n is damaged as it strikes the $g.",
                  FALSE, this, 0, 0, TO_ROOM);
      } else {
               if (water)
          act("With a gentle SPLASH, $n plops into the water, unharmed by the fall.",
                  FALSE, this, 0, 0, TO_ROOM);
        else
          act("With a gentle THUD, $n falls to the $g, unharmed by the fall.",
                  FALSE, this, 0, 0, TO_ROOM);
      }
      return TRUE;
    }
  }
  if (count >= 100) {
    vlogf(LOG_BUG, fmt("Air room %d is screwed.  Tell Brutius.") %  in_room);
    return FALSE;
  }
  return TRUE;
}

// returns DELETE_THIS
int TBeing::fallKill()
{
  TRoom *rp;

  if (!(rp = roomp))
    return FALSE;

  if (rp->isWaterSector())
    act("$n drops into the room, and SMASHES into the water at high speed.", FALSE, this, 0, 0, TO_ROOM);
  else
    act("$n drops into the room, and SMASHES against the $g at high speed.", FALSE, this, 0, 0, TO_ROOM);
  act("You are drenched with blood and gore.... yuck!", FALSE, this, 0, 0, TO_ROOM);
  sendTo("You are SMASHED into tiny pieces!\n\r");

  damageEm(hitLimit() + 20, "has fallen to death!",DAMAGE_FALL);
  return DELETE_THIS;
}

bool TBeing::fallingMobHitMob(TRoom *rp, int count)
{
  TThing *t;
  int prod, d;

  prod = (int) (2.35 * height * getWeight());

  for (t = rp->getStuff(); t; t = t->nextThing) {
    TBeing *k = dynamic_cast<TBeing *>(t);
    if (!k)
      continue;
    if ((this != k) && ((prod + (2.35 * k->height * k->getWeight())) > (dice(1, 500) * 1000))) {
      act("$N slams into you!  OUCH!", FALSE, k, 0, this, TO_CHAR);
      act("$N slams into $n!  OUCH!", FALSE, k, 0, this, TO_NOTVICT);
      act("You slam into $n!  OUCH!", FALSE, k, 0, this, TO_VICT);
      d = (int) (getWeight() * count / 10);
      if (!isImmortal() && !k->isImmortal() && 
	  !k->hasQuestBit(TOG_MONK_GREEN_FALLING)) {
        // we don't want this to start fights...
        if (k->reconcileDamage(k, d, DAMAGE_FALL) == -1) {
          delete k;
          k = NULL;
        }
      }
      return TRUE;
    }
  }
  return FALSE;
}

// returns DELETE_THIS
int TBeing::checkFalling()
{
  int rc = FALSE;
  TRoom *rp = NULL;
  int count, new_room, num1, num2 = 0;

  // this check is just here to make sure flying stops appropriately
  // don't rely on this, make things that stop canFly do crashlands...
  rc = flightCheck();
  if (IS_SET_DELETE(rc, DELETE_THIS))
    return DELETE_THIS;

  if (!(rp = roomp) ||
      sectorSafe() || !clearpath(in_room, DIR_DOWN) || 
      (desc && (desc->connected > 20)))
    return FALSE;

  rc = triggerSpecialOnPerson(NULL, CMD_OBJ_START_TO_FALL, "");
  if (IS_SET_DELETE(rc, DELETE_THIS))
    return DELETE_THIS;

  // this used to be only for isPc()
  // but lets allow a certain degree of self-preservation
  if (!riding && canFly() && !isFlying()) {
    if (riding) {
      dismount(POSITION_STANDING);
    }
    doFly();
  } else {
    TBeing *tbt = dynamic_cast<TBeing *>(riding);
    if (tbt && tbt->canFly() && !tbt->isFlying()) {
      tbt->doFly();
    }
  }
  if (!riding && (getPosition() >= POSITION_RESTING) && roomp && roomp->isFlyingSector()) {
    setPosition(POSITION_FLYING);
  }
  if (isFlying() || 
      (riding && riding->isFlying()))
    return FALSE;

  // basically, separate riders and mounts, but insure that the
  // mount always drops first
  while (rider)
    rider->dismount(POSITION_SITTING);

  TThing *ttt = riding;
  if (ttt) {
    rc = ttt->checkFalling();
    if (IS_SET_DELETE(rc, DELETE_THIS)) {
      delete ttt;
    }
  }

  sendTo("The world spins, as you sky-dive OUT OF CONTROL!!!!!\n\r");
  count = 0;
  num1 = (doesKnowSkill(SKILL_CATFALL) ||
	  affectedBySpell(SPELL_FEATHERY_DESCENT) ||
          bSuccess(getSkillValue(SPELL_FEATHERY_DESCENT)/4, SPELL_FEATHERY_DESCENT)) ? 10 : 5;
  num2 = num1 - 2;

  while (count < 100) {
    act("$n plunges downwards, towards oblivion.", FALSE, this, 0, 0, TO_ROOM);
    --(*this);
  
    if (!(rp = real_roomp(new_room = rp->dir_option[DIR_DOWN]->to_room))) {
      vlogf(LOG_BUG, fmt("illegal room number for falling - %d") %  rp->dir_option[DIR_DOWN]->to_room);
      thing_to_room(this, ROOM_VOID);
      return FALSE;
    }
    *rp += *this;

    // this is needed for monkQuestLand special proc
    if(isPc())
      specials.last_direction=DIR_DOWN;

    if (count) {
      if (affectedBySpell(SPELL_FEATHERY_DESCENT))
        sendTo("You spiral downward, like a feather on the wind.\n\r");
      else if(doesKnowSkill(SKILL_CATFALL))
	sendTo("You twist around like a cat as you fall, in order to lessen the damage when you land.\n\r");
      else
        sendTo("You continue to plunge downwards, towards your doom.\n\r");
    }
    doLook("", CMD_LOOK);

    rc = genericMovedIntoRoom(rp, -1, CHECK_FALL_NO);
    if (IS_SET_DELETE(rc, DELETE_THIS))
      return DELETE_THIS;

    act("$n falls from above.", FALSE, this, 0, 0, TO_ROOM);
    count++;
    // peg innocents here, hitting them stops further fall
    if (fallingMobHitMob(rp, count))
      return FALSE;

    if (!clearpath(in_room, DIR_DOWN) || (!rp->isFallSector() ||
rp->isFlyingSector())) {
      if (rp->isFlyingSector() && (getPosition() >= POSITION_RESTING)) {
        act("Your descent is stopped by a magic force before hitting the $g.", FALSE, this, 0, 0, TO_CHAR);
        act("$n drops into the room, and stops just above the $g.", FALSE , this, 0, 0, TO_ROOM);
         if (!isFlying())
           setPosition(POSITION_FLYING);
         return TRUE;
      }
      if (isImmortal()) {
        act("You bounce like rubber upon hitting the $g.  Immortality rocks!", FALSE, this, 0, 0, TO_CHAR);
        act("$n drops into the room, and bounces off the $g like rubber.", FALSE, this, 0, 0, TO_ROOM);
        setPosition(POSITION_STANDING);
        roomp->playsound(SOUND_BOING, SOUND_TYPE_NOISE);
        return TRUE;
      }
      if (count > num1) {
        fallKill();
        return DELETE_THIS;
      }
      if (count > num2) {
        if (::number(0, getConShock()) < (count * 10)) {
          fallKill();
          return DELETE_THIS;
        } else {
          sendTo(fmt("You are CRUSHED as you impact with the %s.\n\r") % rp->describeGround());
          if (rp->isWaterSector())
            act("$n plunges into the waters.", FALSE, this, 0, 0, TO_ROOM);
          else
            act("$n *SLAMS* into the $g, looking rather pancake-like.", FALSE, this, 0, 0, TO_ROOM);

          if (!isAgile(-count*2)) {
            break_bone(this, WEAR_LEGS_L);
            break_bone(this, WEAR_LEGS_R);
            sendTo("You feel the bones in your legs splinter into a million pieces.\n\r");
            act("$n's legs twist beneath $m as $e screams in pain!.", FALSE, this, 0, 0, TO_ROOM);
          }
          int dam = count * ::number(40,80);
          if (affectedBySpell(SPELL_FEATHERY_DESCENT) || doesKnowSkill(SKILL_CATFALL))
            dam /= 2;

          rc = damageEm(dam, "has fallen to death!", DAMAGE_FALL);
          if (IS_SET_ONLY(rc, DELETE_THIS))
            return DELETE_THIS;
        }
        return TRUE;
      }
      if (count > 0) {
        if (isAgile(5)) {
          if (rp->isWaterSector()) {
            act("$n dives gracefully into the waters.", FALSE, this, 0, 0, TO_ROOM);
            sendTo("You dive gracefully into the waters.\n\r");
          } else {
            act("$n drops to the $g safely.  What a lucky bastard.", FALSE, this, 0, 0, TO_ROOM);
            sendTo("You land safely on your feet!\n\r");
          }
          return TRUE;
        }
        if (rp->isWaterSector()) {
          act("$n belly-flops into the water.  OUCH!  That _had_ to have hurt.", FALSE, this, 0, 0, TO_ROOM);
          sendTo("You scream in pain as you belly-flop into the water.\n\r");
          int dam = count * ::number(5,30);
          if (affectedBySpell(SPELL_FEATHERY_DESCENT) ||
              doesKnowSkill(SKILL_CATFALL))
            dam /= 2;
          rc = damageEm(dam, "breaks their neck as they smack into the water!",DAMAGE_FALL);
          if (IS_SET_ONLY(rc, DELETE_THIS))
            return DELETE_THIS;
        } else {
          act("$n screams as $e tumbles to the $g.", FALSE, this, 0, 0, TO_ROOM);
          sendTo(fmt("You scream in pain as you tumble to the %s.\n\r") % roomp->describeGround());
          int dam = count * ::number(15,55);
          if (affectedBySpell(SPELL_FEATHERY_DESCENT) ||
              doesKnowSkill(SKILL_CATFALL))
            dam /= 2;
          rc = damageEm(dam, "has fallen to death!", DAMAGE_FALL);
          if (IS_SET_ONLY(rc, DELETE_THIS))
            return DELETE_THIS;
        }
        return TRUE;
      }
      if (rp->isWaterSector()) {
        act("$n splashes into the water.", FALSE, this, 0, 0, TO_ROOM);
        sendTo("You fall un-scathed into the refreshing waters.\n\r");
      } else {
        act("$n drops gracefully onto the $g.", FALSE, this, 0, 0, TO_ROOM);
        sendTo(fmt("You drop gracefully to the %s.\n\r") % roomp->describeGround());
      }
      return TRUE;
    }
  }
  if (count >= 100) {
    vlogf(LOG_BUG, fmt("Air room %d is screwed.  Tell Brutius.") %  in_room);
    if (!isImmortal()) {
      fallKill();
      return DELETE_THIS;
    }
    return FALSE;
  }
  return TRUE;
}

// returns DELETE_THIS
int TBeing::checkDrowning()
{
  int rc;
  TRoom *rp;

  if (!isPc())
    return FALSE;

  rp = roomp;
  if (!rp)
    return FALSE;

  if (rp->isUnderwaterSector()) {
    if (!isImmortal()) {
      bool rc2 = isAffected(AFF_WATERBREATH);
      if (rc2 == true)
        return FALSE;

      sendTo("PANIC!  You're drowning!!!!!!\n\r");
      act("$n waves $s hands vigorously, and turns a deep purple color.", FALSE, this, 0, 0, TO_ROOM);
      addToHit(-(::number(1, 10)));
      addToMove(-::number(1, 10));
    }
    updatePos();
    if (getHit() < -10) {
      vlogf(LOG_MISC, fmt("%s killed by drowning at %s (%d)") % 
          getName() % roomp->getName() % inRoom());

      if (!desc)
        setMoney(0);
      rc = die(DAMAGE_DROWN);
      if (IS_SET_ONLY(rc, DELETE_THIS))
        return DELETE_THIS;
    }
  }
  return FALSE;
}

bool TBeing::canFly() const
{
  if (getPosition() <= POSITION_STUNNED)
    return FALSE;

  // natural fly requires arms/wings be useful
  if (race->isWinged()) {
    if (!eitherArmHurt())
      return TRUE;
  }
  // a non-natural ability to fly (air elemental, djinni, fly spell, etc)
  if (isAffected(AFF_FLYING))
    return TRUE;

  if (isImmortal())
    return true;

  return FALSE;
}

int TBeing::flightCheck()
{
  if (isFlying() && !canFly() && !roomp->isFlyingSector()) {
    int rc = crashLanding(POSITION_SITTING);
    if (IS_SET_DELETE(rc, DELETE_THIS))
      return DELETE_THIS;
  }
  return FALSE;
}
