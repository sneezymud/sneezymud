//////////////////////////////////////////////////////////////////////////
//
// SneezyMUD - All rights reserved, SneezyMUD Coding Team
//
//
//////////////////////////////////////////////////////////////////////////
#include "stdsneezy.h"
#include "games.h"
#include "disease.h"
#include "combat.h"
#include "disc_thief.h"
#include "obj_tool.h"
#include "pathfinder.h"
#include "obj_portal.h"

int TBeing::doSneak(const char *argument)
{
  int rc = 0;
  char arg[80];

  spellNumT skill = getSkillNum(SKILL_SNEAK);
  argument = one_argument(argument, arg);

  if (!doesKnowSkill(skill)) {
    sendTo("You know nothing about sneaking.\n\r");
    return FALSE;
  }
  if (!*arg && desc) {
    if (affectedBySpell(skill) || checkForSkillAttempt(skill)) {
      sendTo("You are already trying to be sneaky.\n\r");
      return FALSE;
    }
  }

  if (*arg) {
    if (is_abbrev(arg, "off") || is_abbrev(arg, "stop")) {
      if (affectedBySpell(skill) || checkForSkillAttempt(skill)) {
        sendTo("You will no longer try to be sneaky.\n\r");
        removeSkillAttempt(skill);
        if (affectedBySpell(skill))
          affectFrom(skill);
        return FALSE;
      } else {
        sendTo("You were not trying to be sneaky.\n\r");
        return FALSE;
      }
    } else if (is_abbrev(arg, "retry")) {
      if (affectedBySpell(skill) || checkForSkillAttempt(skill)) {
	removeSkillAttempt(skill);
	if (affectedBySpell(skill))
	  affectFrom(skill);
      }
    }
  }
  rc = sneak(this, skill);
  if (rc)
    addSkillLag(skill, rc);

  return rc;
}

int sneak(TBeing *thief, spellNumT skill)
{
  affectedData af;
  const int SNEAK_COST = 3;

  if (thief->fight()) {
    thief->sendTo("No way!! You simply can NOT sneak while fighting!\n\r");
    return FALSE;
  }
  if (thief->affectedBySpell(skill)) {
    thief->affectFrom(skill);
    thief->sendTo("You are no longer sneaky.\n\r");

    // this should technically be a return TRUE, but by sending back false
    // we don't lag them for ending their sneak
    return FALSE;
  }
  if (thief->riding) {
    thief->sendTo("It is impossible to sneak while mounted.\n\r");
    return FALSE;
  }
  if (thief->isFlying()) {
    thief->sendTo("It is impossible to sneak while flying.\n\r");
    return FALSE;
  }
  if (thief->isSwimming()) {
    thief->sendTo("It is impossible to sneak while swimming.\n\r");
    return FALSE;
  }
  if (thief->getMove() < SNEAK_COST) {
    thief->sendTo("You don't have the vitality to do that.\n\r");
    return FALSE;
  }
  thief->addToMove(-SNEAK_COST);
  thief->sendTo("Ok, you'll try to move silently for a while.\n\r");

  int level = thief->getSkillLevel(skill);
  int bKnown = thief->getSkillValue(skill);
  bKnown += thief->plotStat(STAT_CURRENT, STAT_DEX, -70, 15, 0);

  if (thief->bSuccess(bKnown, skill)) {
    af.modifier = 1 + level/2;
    af.type = skill;
    af.duration = 5 * level;
    af.modifier = 1 + level/2;
    af.location = APPLY_CAN_BE_SEEN;
    af.bitvector = AFF_SNEAK;
    thief->affectTo(&af, -1);
    return TRUE;
  } else {
    af.modifier = 0;
    af.type = AFFECT_SKILL_ATTEMPT;
    af.duration = 5 * level;
    af.modifier = skill;
    af.location = APPLY_NONE;
    af.bitvector = 0;
    thief->affectTo(&af, -1);
    return TRUE;
  }
  return TRUE;
}

int TBeing::doHide()
{
  spellNumT skill = getSkillNum(SKILL_HIDE);

  if (!doesKnowSkill(skill)) {
    sendTo("You know nothing about hiding.\n\r");
    return FALSE;
  }
  int rc = hide(this, skill);
  if (rc)
    addSkillLag(skill, rc);

  return rc;
}

int hide(TBeing *thief, spellNumT skill)
{
  if (thief->fight()) {
    thief->sendTo("No way!! You simply can NOT hide while fighting!\n\r");
    return FALSE;
  }
  thief->sendTo("You attempt to hide yourself.\n\r");

  if (thief->riding) {
    thief->sendTo("Yeah... right... while mounted\n\r");
    return FALSE;
  }
  int bKnown = thief->getSkillValue(skill);
  bKnown += thief->plotStat(STAT_CURRENT, STAT_DEX, -40, 15, 0);

  if (thief->bSuccess(bKnown, skill)) {
    SET_BIT(thief->specials.affectedBy, AFF_HIDE);
  } else {
  }
  return TRUE;
}

int TBeing::doSubterfuge(const char *arg)
{
  TBeing *victim;
  char name_buf[MAX_INPUT_LENGTH];
  int rc;

  if (!doesKnowSkill(SKILL_SUBTERFUGE)) {
    sendTo("You know nothing about the art of subterfuge.\n\r");
    return FALSE;
  }
  one_argument(arg, name_buf);
  if (!(victim = get_char_room_vis(this, name_buf))) {
    sendTo("No one here by that name.\n\r");
    return FALSE;
  }
  if (victim->isPc()) {
    sendTo("You can't subterfuge a player, sorry.\n\r");
    return FALSE;
  }
  rc = subterfuge(this, victim);
  if (rc)
    addSkillLag(SKILL_SUBTERFUGE, rc);

  return rc;
}

int subterfuge(TBeing *thief, TBeing *victim)
{
  if (thief->fight()) {
    thief->sendTo("No way!! You simply can NOT concentrate!\n\r");
    return FALSE;
  }
  if (thief->getMove() < 50) {
    thief->sendTo("You don't have enough mp to make the move.\n\r");
    return FALSE;
  }

  // failure sets fighting
  if (thief->checkPeaceful("You can't subterfuge in a place of refuge.\n\r"))
    return FALSE;

  int level = thief->getSkillLevel(SKILL_SUBTERFUGE);
  int bKnown = thief->getSkillValue(SKILL_SUBTERFUGE);
  int tmpNum = ::number(0, bKnown);

  thief->addToMove(-tmpNum);
  thief->setMove(max(0, thief->getMove()));

  if (thief->isNotPowerful(victim, level, SKILL_SUBTERFUGE, SILENT_YES)) {
    act("$s mind is too powerful to be confused.", FALSE, thief, NULL, victim, TO_CHAR);
    thief->sendTo("You simply fail to confuse your target.\n\r");
    return TRUE;
  }
  if ((victim->plotStat(STAT_CURRENT, STAT_PER, 3, 18, 12) + 
       victim->plotStat(STAT_CURRENT, STAT_FOC, 3, 18, 12)) >
      (thief->plotStat(STAT_CURRENT, STAT_KAR, 3, 18, 12) +
       thief->plotStat(STAT_CURRENT, STAT_FOC, 3, 18, 12))) {
    act("$N is too smart to fall for this ploy.",
          FALSE, thief, NULL, victim, TO_CHAR);
    thief->sendTo("You simply fail to confuse your target.\n\r");
    return TRUE;
  }

  if (thief->bSuccess(bKnown, SKILL_SUBTERFUGE)) {
    if (victim->isLucky(thief->spellLuckModifier(SKILL_SUBTERFUGE))) {
      thief->sendTo("Uhoh! You simply fail to confuse your target!\n\r");
      thief->setCharFighting(victim);
      thief->setVictFighting(victim);
      return TRUE;
    }
    thief->sendTo("You have totally confused the monster!\n\r");

    REMOVE_BIT(victim->specials.act, ACT_HUNTING);
    REMOVE_BIT(victim->specials.act, ACT_HATEFUL);

    return TRUE;
  } else {
    thief->sendTo("Uhoh! Something went wrong!\n\r");
    thief->setCharFighting(victim);
    thief->setVictFighting(victim);
    return TRUE;
  }
}

int TBeing::doPick(const char *argument)
{
  char type[MAX_INPUT_LENGTH], dir[MAX_INPUT_LENGTH];
  int rc;

  // Gin Game Command, not Thief skill
  if (gGin.check(this)) {
    gGin.draw(this, argument);
    return TRUE;
  }
  if (!doesKnowSkill(SKILL_PICK_LOCK)) {
    sendTo("You know nothing about picking locks.\n\r");
    return FALSE;
  }
  // Thief skill
  argument_interpreter(argument, type, dir);

  if (!*type) {
    sendTo("Pick what?\n\r");
    return FALSE;
  }
  rc = pickLocks(this, argument, type, dir);
  return rc;
}

int TThing::pickWithMe(TBeing *thief, const char *, const char *, const char *)
{
  thief->sendTo("You need to hold a lock pick in your primary hand in order to pick locks.\n\r");
  return FALSE;
}

int TTool::pickWithMe(TBeing *thief, const char *argument, const char *type, const char *dir)
{
  dirTypeT door;
  roomDirData *exitp = NULL;
  TObj *obj;
  TBeing *victim;

  if ((getToolType() != TOOL_LOCKPICK) ||
      (getToolUses() <= 0)) {
    thief->sendTo("You need to hold a lock pick in your primary hand in order to pick locks.\n\r");
    return FALSE;
  }
  int bKnown = thief->getSkillValue(SKILL_PICK_LOCK);

  // moved door check before obj check as "pick gate s" seemed to
  // pick up objs with "s" in the name, not sure why gate was ignored though
  if ((door = thief->findDoor(type, dir, DOOR_INTENT_UNLOCK, SILENT_YES)) >= MIN_DIR) {
    exitp = thief->exitDir(door);
    if (exitp->door_type == DOOR_NONE)
      thief->sendTo("That's absurd.\n\r");

    if (!IS_SET(exitp->condition, EX_CLOSED))
      thief->sendTo("You realize that the door is already open.\n\r");
    else if (exitp->key < 0)
      thief->sendTo("You can't seem to spot any lock to pick.\n\r");
    else if (!IS_SET(exitp->condition , EX_LOCKED))
      thief->sendTo("Oh.. it wasn't locked at all.\n\r");
    else {
      act("$n begins fiddling with a lock.", FALSE, thief, 0, 0, TO_ROOM);
      act("You begin fiddling with a lock.", FALSE, thief, 0, 0, TO_CHAR);
      thief->learnFromDoingUnusual(LEARN_UNUSUAL_NORM_LEARN, SKILL_PICK_LOCK, 10);

      // silly, but what if they sit down and pick the lock...
      if (thief->task)
        thief->stopTask();

      start_task(thief, NULL, NULL, TASK_PICKLOCKS,"",0,thief->in_room,door,0,120-bKnown);
    }
  } else if (generic_find(argument, FIND_OBJ_INV | FIND_OBJ_ROOM, thief, &victim, &obj)) {
    obj->pickMe(thief);
  } else
    thief->sendTo("You don't see that here.\n\r");

  return TRUE;
}

int pickLocks(TBeing *thief, const char * argument, const char * type, const char * dir)
{
  TThing *pick;

  if (!thief->doesKnowSkill(SKILL_PICK_LOCK)){
    thief->sendTo("You don't know to pick locks!\n\r");
    return FALSE;
  }
  if (!(pick = thief->heldInPrimHand())) {
    thief->sendTo("You need to hold a lock pick in your primary hand in order to pick locks.\n\r");
    return FALSE;
  }
  pick->pickWithMe(thief, argument, type, dir);
  return TRUE;
}

int TBeing::SpyCheck()
{
  if (bSuccess(SKILL_SPY))
    return TRUE;

  return FALSE;
}

int TBeing::doSpy()
{
  if (!doesKnowSkill(SKILL_SPY)) {
    sendTo("You know nothing about spying.\n\r");
    return FALSE;
  }
  return spy(this);
}

int spy(TBeing *thief)
{
  affectedData aff;

  if (thief->affectedBySpell(SKILL_SPY)) {
    thief->sendTo("You cease spying.\n\r");
    thief->affectFrom(SKILL_SPY);
    return FALSE;
  }

  if (thief->isAffected(AFF_SCRYING)) {
    thief->sendTo("You are already doing your best spy imitation.\n\r");
    return FALSE;
  }
  thief->sendTo("Ok, try your best to spy.\n\r");

  int bKnown = thief->getSkillValue(SKILL_SPY);

  // We will set an affect regardless of success, failure will just
  // not set the AFF_SCRYING bit so check for isAff(scry) in code to
  // see if spying
  aff.type = SKILL_SPY;
  aff.duration = (((int) bKnown/ 10) + 1) * UPDATES_PER_MUDHOUR;
  aff.modifier = 0;
  aff.location = APPLY_NONE;

  if (thief->bSuccess(bKnown, SKILL_SPY)) {
    aff.bitvector = AFF_SCRYING;
    thief->affectTo(&aff, -1);
    return TRUE;
  } 
  aff.bitvector = 0;
  thief->affectTo(&aff, -1);
  return TRUE;
}


void TObj::pickMe(TBeing *thief)
{
  act("$p: That's not a container.", false, thief, this, 0, TO_CHAR);
}



int TBeing::thiefDodge(TBeing *v, TThing *weapon, int *dam, int w_type, wearSlotT part_hit)
{
  char buf[256], type[16];

  // presumes thief is in appropriate position for dodging already

  if (!v->doesKnowSkill(SKILL_DODGE_THIEF))
    return FALSE;

  w_type -= TYPE_HIT;

  // base amount, modified for difficulty
  // the higher amt is, the more things get blocked
  int amt = (int) (45 * 100 / getSkillDiffModifier(SKILL_DODGE_THIEF));

  if (::number(0, 999) >= amt)
    return FALSE;

  // check bSuccess after above check, so that we limit how often we
  // call the learnFrom stuff
  if (v->bSuccess(SKILL_DODGE_THIEF)) {
    *dam = 0;

    strcpy(type, "dodge");
    if (Twink == 1) {
      sprintf(buf, "You %s $n's %s at your %s.", type,
	      attack_hit_text_twink[w_type].singular,
	      v->describeBodySlot(part_hit).c_str());
    } else {
      sprintf(buf, "You %s $n's %s at your %s.", type,
	      attack_hit_text[w_type].singular,
	      v->describeBodySlot(part_hit).c_str());
    }
    act(buf, FALSE, this, 0, v, TO_VICT, ANSI_CYAN);
    if (Twink == 1) {    
      sprintf(buf, "$N %ss your %s at $S %s.", type,
	      attack_hit_text_twink[w_type].singular,
	      v->describeBodySlot(part_hit).c_str());
    } else {
      sprintf(buf, "$N %ss your %s at $S %s.", type,
	      attack_hit_text[w_type].singular,
	      v->describeBodySlot(part_hit).c_str());
    }
    act(buf, FALSE, this, 0, v, TO_CHAR, ANSI_CYAN);
    if (Twink == 1) {
      sprintf(buf, "$N %ss $n's %s at $S %s.", type,
	      attack_hit_text_twink[w_type].singular,
	      v->describeBodySlot(part_hit).c_str());
    } else {
      sprintf(buf, "$N %ss $n's %s at $S %s.", type,
	      attack_hit_text[w_type].singular,
	      v->describeBodySlot(part_hit).c_str());
    }
    act(buf, TRUE, this, 0, v, TO_NOTVICT);

    return TRUE;
  }
  return FALSE;
}


void TBeing::doTrack(const char *argument)
{
  char namebuf[256], found = FALSE;
  int dist = 0, targrm, worked, skill;
  int code;
  TBeing *scan;
  affectedData aff, *Vaff;
  TThing *t = NULL;
  char buf[256]="\0\0\0", buf2[256]="\0\0\0";

  strcpy(namebuf, argument);
 
  if (!*namebuf && !specials.hunting) {
    sendTo("You need to search for SOMEONE.\n\r");
    return;
  }
  if (affectedBySpell(SKILL_SEEKWATER)) {
    sendTo("You can't track something while seeking water.\n\r");
    return;
  }
  int learning = getSkillValue(SKILL_TRACK);
  if (learning <= 0 && !affectedBySpell(SPELL_TRAIL_SEEK)) {
    sendTo("You do not have the ability to track.\n\r");
    return;
  }
  if (getPosition() < POSITION_STANDING) {
    sendTo("You really need to be standing in order to start tracking.\n\r");
    return;
  }
  if (fight()) {
    sendTo("The ensuing battle makes it difficult to track.\n\r");
    return;
  }
  if (roomp && !isImmortal() &&
      (roomp->getLight() + visionBonus <= 0) &&
      !roomp->isRoomFlag(ROOM_ALWAYS_LIT) &&
      !isAffected(AFF_TRUE_SIGHT)) {
    sendTo("You can't see well enough to track.\n\r");
    return;
  }
  found = FALSE;
  for (scan = character_list; scan; scan = scan->next) {
    if (isname(namebuf, scan->name)) {
      found = TRUE;
      break;
    }
  }
  if (!found) {
    sendTo("You are unable to find any signs of that.\n\r");
    return;
  }
 
  // Lets determine the distance: [roomCount == 11865 as of 12-18-98]
  //  Ranger  TRAIL_SEEK :  25 - 176
  //  Ranger !TRAIL_SEEK :  20 - 161
  // !Ranger  TRAIL_SEEK :  15 - 153
  int level = 0;
  if (learning > 0) {
    level = getSkillLevel(SKILL_TRACK); 

    if (affectedBySpell(SPELL_TRAIL_SEEK))
      dist = max(25, (int) (((((roomCount*((level+1)/2))/1000)+
                           min(50, learning)))/2));
    else
      dist = max(20, (int) (((((roomCount*((level+1)/2))/1000)+
                           min(20, learning)))/2));
  } else if (affectedBySpell(SPELL_TRAIL_SEEK)) {
    learning = getSkillValue(SPELL_TRAIL_SEEK);
    level = getSkillLevel(SPELL_TRAIL_SEEK);
    dist = max(15, (int) (((((roomCount*((level+1)/2))/1000)+
                         min(5, learning)))/2));
  }

  if (hasClass(CLASS_THIEF) || hasClass(CLASS_RANGER))
    dist *= 2;
  else if (hasClass(CLASS_WARRIOR))
    dist /= 2;
  else if (hasClass(CLASS_MAGE))
    dist += getLevel(MAGE_LEVEL_IND);
  else
    dist = dist;
 
  switch (getRace()) {
    case RACE_GIANT:
    case RACE_ELVEN:
      dist *= 2;                // even better 
      break;
    case RACE_DEVIL:
    case RACE_DEMON:
      dist = MAX_ROOMS;         //  4 as good as can be 
      break;
    default:
      break;
  }
 
  if (isImmortal())
    dist = MAX_ROOMS;
 
  hunt_dist = dist;
  specials.hunting = 0;
  TPathFinder path(dist);
 
  // note: -dist will look THRU doors.
  // all subsequent calls use track() which does not go thru doors
  // this is intentional so they lose track after 1 step
  if ((level < MIN_GLOB_TRACK_LEV) ||
      (affectedBySpell(SPELL_TRAIL_SEEK))){
    path.setStayZone(true);
    path.setThruDoors(true);

    code=path.findPath(in_room, findBeing(namebuf));
    targrm=path.getDest();
  } else {
    path.setStayZone(false);
    path.setThruDoors(true);
    
    code=path.findPath(in_room, findBeing(namebuf));
    targrm=path.getDest();
  }

  if (code == -1) {
    addToWait(combatRound(1));
    if (targrm == inRoom()) {
      if (get_char_room(namebuf, targrm) == this)
        sendTo("Do you often lose yourself?\n\r");
      else
        sendTo("I believe what your tracking is right here with you.\n\r");
    } else
      sendTo("You are unable to find any signs of that.\n\r");
    return;
  } else {
    addPlayerAction(PLR_HUNTING);
    specials.hunting = get_char_room(namebuf, targrm);
    skill = 0;
    for (Vaff = specials.hunting->affected; Vaff; Vaff = Vaff->next)
      if (Vaff->type == SKILL_CONCEALMENT)
        skill = -Vaff->modifier;
    if (affectedBySpell(SPELL_TRAIL_SEEK)) {
      skill += 50;
      worked = (::number(0, 110) < skill);
    } else {
      skill += getSkillValue(SKILL_TRACK);
      worked = bSuccess(skill, SKILL_TRACK);
    }
    if (worked) {
      if (code <= 9)
        sendTo(fmt("%sYou see traces of your quarry %s.%s\n\r") % purple() %
               dirs_to_blank[code] % norm());
      else {
        int count = code - 9, seen = 0;
        for (t = roomp->getStuff(); t; t = t->nextThing) {
          TPortal *tp = dynamic_cast<TPortal *>(t);
          if (tp) {
            seen++;
            if (count == seen) {
              sendTo(COLOR_OBJECTS, fmt("%sYou see traces of your quarry through %s.%s\n\r") % 
                     purple() % tp->getName() % norm());
              break;
            }
          }
        }
        if (!t) {
          sendTo("Error finding path target!  Tell a god.\n\r");
          vlogf(LOG_BUG, "Error finding path (doTrack)");
          return;
        }
      }
    } else {
      code = -2;
      sendTo(COLOR_MOBS, fmt("You begin tracking %s.\n\r") % specials.hunting->getName());
    }
  }

  aff.type = SKILL_TRACK;
  aff.level = getSkillLevel(SKILL_TRACK);
  aff.duration = PERMANENT_DURATION;
  affectTo(&aff);

  if (desc && desc->m_bIsClient)
    desc->clientf(fmt("%d|%d") % CLIENT_TRACKING % (1 << code));

  if (code <= 9) {
    if (code >= 0 && desc && (desc->autobits & AUTO_HUNT)) {
      strcpy(buf, dirs[code]);
      addCommandToQue(buf);
    }
  } else if (desc && (desc->autobits & AUTO_HUNT) && t) {
      strcpy(buf, t->name);
      strcpy(buf, add_bars(buf).c_str());
      addToWait(combatRound(1));
      sprintf(buf2, "enter %s", buf);
      addCommandToQue(buf2);
  }

  bool isTR = affectedBySpell(SPELL_TRAIL_SEEK);

  addToWait(combatRound(1));
  addToMove((int) min(10, (-2-((110-getSkillValue((isTR ? SKILL_TRACK : SPELL_TRAIL_SEEK))))/6)));

  start_task(this, NULL, NULL, TASK_TRACKING, "", 1, in_room, 1, code+1, 40);
}
 
// used by doLook() to display next direction to go.
// return FALSE to cease tracking
int TBeing::track(TBeing *vict)
{
  int code;
  TThing *t;
  int targetRm = -1;
  int isSW = affectedBySpell(SKILL_SEEKWATER);
  char buf[256];
  char buf2[256];
  TPathFinder path(hunt_dist);
  path.setUsePortals(false);

  if (!vict && !isSW) {
    vlogf(LOG_BUG, fmt("Problem in track() %s") %  getName());
    return TRUE;
  }
  if (roomp && !isImmortal() && 
      (roomp->getLight() + visionBonus <= 0) &&
      !roomp->isRoomFlag(ROOM_ALWAYS_LIT) &&
      !isAffected(AFF_TRUE_SIGHT)) {
    return TRUE;
  }
  if (!vict) {
    if (isSW){
      code=path.findPath(in_room, findWater());
      targetRm=path.getDest();
    }
    else {
      vlogf(LOG_BUG, "problem in track()");
      stopTask();
      return FALSE;
    }
  } else {
    if (isImmortal())  // look through doors
      code = choose_exit_global(in_room, vict->in_room, hunt_dist);
    else if ((GetMaxLevel() < MIN_GLOB_TRACK_LEV) ||
           affectedBySpell(SPELL_TRAIL_SEEK))
      code = choose_exit_in_zone(in_room, vict->in_room, hunt_dist);
    else
      code = choose_exit_global(in_room, vict->in_room, hunt_dist);
  }
  if ((vict && sameRoom(*vict)) ||
      (targetRm != -1 && targetRm == inRoom())) {
    sendTo(fmt("%s###You have found %s!%s\n\r") % orange() % (isSW ? "some water" :
           "your quarry") % norm());
    addToWait(combatRound(1));
    if (desc && desc->m_bIsClient)
      desc->clientf(fmt("%d") % CLIENT_TRACKOFF);
    stopTask();
    addToWait(combatRound(1));
    return FALSE;
  }
  if (task && task->flags > 0 && task->flags != 100) {
    if (task->flags != (code + 1)) {
      sendTo(fmt("%s###For some reason the path you found is gone.%s\n\r") %
             purple() % norm());
      task->flags = 0;
      return TRUE;
    } else if (code <= 9) {
      sendTo(fmt("%s###You track %s %s.%s\n\r") % purple() %
             (isSW ? "some water" : "your target") % dirs_to_blank[code] % norm());
      if (desc && (desc->autobits & AUTO_HUNT)) {
        strcpy(buf, dirs[code]);
        addCommandToQue(buf);
      }
    } else {
      int count = code - 9, seen = 0;
      for (t = roomp->getStuff(); t; t = t->nextThing) {
        TPortal *tp = dynamic_cast<TPortal *>(t);
        if (tp) {
          seen++;
          if (count == seen) {
            sendTo(COLOR_OBJECTS, fmt("%sYou track %s through %s.%s\n\r") % purple() %
                   (isSW ? "some water" : "your quarry") % tp->getName() % norm());
            if (desc && (desc->autobits & AUTO_HUNT)) {
              strcpy(buf, tp->name);
              strcpy(buf, add_bars(buf).c_str());
              sprintf(buf2, "enter %s", buf);
              addCommandToQue(buf2);
            }
            break;
          }
        }
      }
    }
    return TRUE;
  }
  return TRUE;
}
 
// this is called exclusively by TMonster::hunt()
// returns 0-9 for dir to travel, or 10+ for a portal (indexed)
// return -1 to stop the tracking
dirTypeT TBeing::dirTrack(TBeing *vict)
{
  dirTypeT code;
  affectedData *aff;
  TThing *t;

  if (roomp && !isImmortal() && 
      (roomp->getLight() + visionBonus <= 0) &&
      !roomp->isRoomFlag(ROOM_ALWAYS_LIT) &&
      !isAffected(AFF_TRUE_SIGHT)) {
    sendTo("You can't see well enough to find a trail.\n\r");
    return DIR_NONE;
  }

  for (aff = vict->affected; aff; aff = aff->next) {
    if (aff->type == SKILL_CONCEALMENT) {
      if (::number(1,150) < aff->modifier) {
        sendTo(fmt("%s##You have lost the trail.%s\n\r") % orange() % norm());

        if (aff->be == vict) {
          act("You have successfully concealed your path from $N.",
                    FALSE, vict, 0, this, TO_CHAR, ANSI_GREEN);
          return DIR_NONE;
        } else if (aff->be && vict->sameRoom(*aff->be)) {
          act("$N has successfully concealed your path from $P.",
                    FALSE, vict, this, aff->be, TO_CHAR, ANSI_GREEN);
          act("You have successfully concealed $n's path from $P.",
                    FALSE, vict, this, aff->be, TO_VICT, ANSI_GREEN);
          return DIR_NONE;
        }
      }
    }
  }

  if ((GetMaxLevel() >= MIN_GLOB_TRACK_LEV) || affectedBySpell(SPELL_TRAIL_SEEK)
      || IS_SET(specials.act, ACT_HUNTING)) {
    code = choose_exit_global(in_room, vict->in_room, hunt_dist);
  } else
    code = choose_exit_in_zone(in_room, vict->in_room, hunt_dist);

  if (code == DIR_NONE) {
    if (sameRoom(*vict))
      sendTo(fmt("%s##You have found your target!%s\n\r") % orange() % norm());
    else
      sendTo(fmt("%s##You have lost the trail.%s\n\r") % orange() % norm());
 
    return DIR_NONE;                // false to continue the hunt 
  } else if (code < MAX_DIR) {
    sendTo(fmt("%s##You see a faint trail %s.%s\n\r") % 
         purple() % dirs_to_leading[code] % norm());
    return code;
  } else {
    int count = code - 9, seen = 0;
    for (t = roomp->getStuff(); t; t = t->nextThing) {
      TPortal *tp = dynamic_cast<TPortal*>(t);
      if (tp) {
         seen++;
        if (count == seen) {
          sendTo(COLOR_OBJECTS, fmt("%sYou see a faint trail through %s.%s\n\r") %
             purple() % tp->getName() % norm());
          break;
        }
      }
    }
    if (!t) {
      sendTo("Error finding path target!  Tell a god.\n\r");
      vlogf(LOG_BUG, "Error finding path (dirTrack)");
    }
    return code;
  }
}
 
