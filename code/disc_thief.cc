//////////////////////////////////////////////////////////////////////////
//
// SneezyMUD - All rights reserved, SneezyMUD Coding Team
//
// $Log: disc_thief.cc,v $
// Revision 5.1  1999/10/16 04:31:17  batopr
// new branch
//
// Revision 1.1  1999/09/12 17:24:04  sneezy
// Initial revision
//
//
//////////////////////////////////////////////////////////////////////////



#include "stdsneezy.h"
#include "games.h"
#include "disease.h"
#include "combat.h"
#include "disc_thief.h"

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
    addSkillLag(skill);

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

  if (bSuccess(thief, bKnown, skill)) {
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
    addSkillLag(skill);

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

  if (bSuccess(thief, bKnown, skill)) {
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
    addSkillLag(SKILL_SUBTERFUGE);

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

  if (bSuccess(thief, bKnown, SKILL_SUBTERFUGE)) {
    if (victim->isLucky(thief->spellLuckModifier(SKILL_SUBTERFUGE))) {
      thief->sendTo("Uhoh! You simply fail to confuse your target!\n\r");
      thief->setCharFighting(victim);
      thief->setVictFighting(victim);
      return TRUE;
    }
    thief->sendTo("You have totally confused the monster!\n\r");
    if (IS_SET(victim->specials.act, ACT_HUNTING))
      REMOVE_BIT(victim->specials.act, ACT_HUNTING);

    if (IS_SET(victim->specials.act, ACT_HATEFUL))
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

  if (!thief->hasClass(CLASS_THIEF) && !thief->hasClass(CLASS_MONK) && !thief->isImmortal()) {
    thief->sendTo("You're no thief!\n\r");
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
  if (bSuccess(this, getSkillValue(SKILL_SPY), SKILL_SPY))
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
  aff.duration = (((int) bKnown/ 10) + 1) * UPDATES_PER_TICK;
  aff.modifier = 0;
  aff.location = APPLY_NONE;

  if (bSuccess(thief, bKnown, SKILL_SPY)) {
    aff.bitvector = AFF_SCRYING;
    thief->affectTo(&aff, -1);
    return TRUE;
  } 
  aff.bitvector = 0;
  thief->affectTo(&aff, -1);
  return TRUE;
}

int TBeing::doDisguise(const char *arg)
{
  char name_buf[MAX_INPUT_LENGTH];
  int rc;

  if (!doesKnowSkill(SKILL_DISGUISE)) {
    sendTo("You know nothing about disguising yourself.\n\r");
    return FALSE;
  }
  one_argument(arg, name_buf);

  if (!*name_buf){
    sendTo("If you want to disguise yourself as nothing at all, try hide!??\n\r");
    return FALSE;
  }
  rc = disguise(this, name_buf);
  if (rc)
    addSkillLag(SKILL_DISGUISE);

  return rc;
}

static const int LAST_DISGUISE_MOB = 4;
struct PolyType DisguiseList[LAST_DISGUISE_MOB] =
{
  {"citizen male", 1, 1, 108, DISC_STEALTH},
  {"citizen female", 1, 1, 109, DISC_STEALTH},
  {"peasant", 1, 10, 110, DISC_STEALTH},
  {"cityguard", 25, 50, 101, DISC_STEALTH},
};

int disguise(TBeing *caster, char * buffer)
{
  int i, duration;
  TMonster *mob;
  char buf[256];
  affectedData aff;
  affectedData aff2;
  affectedData aff3;

  if (!caster->isImmortal() && caster->checkForSkillAttempt(SKILL_DISGUISE)) {
    act("You are not prepared to try to disguise yourself again so soon.",
         FALSE, caster, NULL, NULL, TO_CHAR);
    return SPELL_FAIL;
  }

  for (i = 0; 
        ((i < LAST_DISGUISE_MOB) && (!is_abbrev(buffer,DisguiseList[i].name)));
        i++);

  if (i >= LAST_DISGUISE_MOB) {
    caster->sendTo("You lack the training to don that disguise.\n\r");
    return FALSE;
  } 
  int level = caster->getSkillLevel(SKILL_DISGUISE);
  int bKnown = caster->getSkillValue(SKILL_DISGUISE);

  discNumT das = getDisciplineNumber(SKILL_DISGUISE, FALSE);
  if (das == DISC_NONE) {
    vlogf(5, "bad disc for SKILL_DISGUISE");
    return FALSE;
  }
  if ((caster->getDiscipline(das)->getLearnedness() < DisguiseList[i].learning) && (level < DisguiseList[i].level)) {
    caster->sendTo("You don't seem to have the ability to disguise yourself as that (yet).\n\r");
    return FALSE;
  }
  if (!(mob = read_mobile(DisguiseList[i].number, VIRTUAL))) {
    caster->sendTo("You couldn't envision an image of that creature.\n\r");
    return FALSE;
  }
  thing_to_room(mob,ROOM_VOID);

  // Check to make sure that there is no snooping going on. 
  if (!caster->desc || caster->desc->snoop.snooping) {
    caster->sendTo("Nothing seems to happen.\n\r");
    delete mob;
    mob = NULL;
    return FALSE;
  }
  if (caster->desc->original) {
    // implies they are switched, while already switched (as x disguise)
    caster->sendTo("You already seem to be switched.\n\r");
    return FALSE;
  }

  aff.type = AFFECT_SKILL_ATTEMPT;
  aff.location = APPLY_NONE;
  aff.duration = (2 + (level/5)) * UPDATES_PER_TICK;
  aff.bitvector = 0;
  aff.modifier = SKILL_DISGUISE;

  if (!bSuccess(caster, bKnown, SKILL_DISGUISE)) {
    caster->sendTo("You seem to have screwed something up.\n\r");
    delete mob;
    mob = NULL;
    caster->affectTo(&aff, -1);
    return TRUE;
  }


  int awesom= FALSE;
  switch (critSuccess(caster, SKILL_DISGUISE)) {
    case CRIT_S_KILL:
      CS(SKILL_DISGUISE);
      awesom= TRUE;
      duration = 20 * UPDATES_PER_TICK;
    case CRIT_S_TRIPLE:
      if (caster->getSkillLevel(SKILL_DISGUISE) > 20) {
        awesom= TRUE;
        CS(SKILL_DISGUISE);
      }
      duration = 15 * UPDATES_PER_TICK;
    case CRIT_S_DOUBLE:
      if (caster->getSkillLevel(SKILL_DISGUISE) > 40) {
        awesom= TRUE;
        CS(SKILL_DISGUISE);
      }
      duration = 15 * UPDATES_PER_TICK;
    default:
      duration = 10 * UPDATES_PER_TICK;
      break;
  }
  act("You apply your skills and make yourself look like $N.", 
       TRUE, caster, NULL, mob, TO_CHAR);
  act("$n apply $s skills and makes $mself look like $N.", 
       TRUE, caster, NULL, mob, TO_ROOM);

// first add the attempt -- used to regulate attempts
  aff.duration = duration + ((2 + (level/5)) * UPDATES_PER_TICK);
  caster->affectTo(&aff, -1);

  aff3.type = AFFECT_SKILL_ATTEMPT;
  aff3.location = APPLY_NONE;
  aff3.duration = duration + ((2 + (level/5)) * UPDATES_PER_TICK);
  aff3.bitvector = 0;
  aff3.modifier = SKILL_DISGUISE;
  mob->affectTo(&aff3, -1);

// then the skill -- used to wear off
  aff2.type = SKILL_DISGUISE;
  aff2.location = APPLY_NONE;
  aff2.duration = duration;
  aff2.bitvector = 0;
  aff2.modifier = 0;
  mob->affectTo(&aff2, -1);

  DisguiseStuff(caster, mob);

  --(*mob);
  *caster->roomp += *mob;
  --(*caster);
  thing_to_room(caster, ROOM_POLY_STORAGE);

#if 0
  if (caster->isPlayerAction(PLR_ANSI | PLR_VT100))
    caster->doTerminal("none");
#endif

  // stop following whoever you are following.
  if (caster->master)
    caster->stopFollower(TRUE);
    
  // switch caster into mobile 
  caster->desc->character = mob;
  caster->desc->original = dynamic_cast<TPerson *>(caster);

  mob->desc = caster->desc;
  caster->desc = NULL;
  caster->polyed = POLY_TYPE_DISGUISE;

  SET_BIT(mob->specials.act, ACT_DISGUISED);
  SET_BIT(mob->specials.act, ACT_POLYSELF);
  SET_BIT(mob->specials.act, ACT_NICE_THIEF);
  SET_BIT(mob->specials.act, ACT_SENTINEL);
  REMOVE_BIT(mob->specials.act, ACT_AGGRESSIVE);
  REMOVE_BIT(mob->specials.act, ACT_SCAVENGER);
  REMOVE_BIT(mob->specials.act, ACT_DIURNAL);
  REMOVE_BIT(mob->specials.act, ACT_NOCTURNAL);

  if (!awesom) {
#if 1
    // remake some of the strings on the player 
    // disassociate the mob from global memory 
    mob->swapToStrung();
    
    if (caster->name) {
      delete [] mob->name;
      mob->name = mud_str_dup(caster->name);
    }

    delete [] mob->shortDescr;
    if (caster->shortDescr) {
      mob->shortDescr = mud_str_dup(caster->shortDescr);
    } else {
      // always true for caster = PC
      mob->shortDescr = mud_str_dup(caster->name);
    }

    delete [] mob->player.longDescr;
    if (caster->player.longDescr) {
      mob->player.longDescr = mud_str_dup(caster->player.longDescr);
    } else {
      // always true for caster = PC
      sprintf(buf, "%s is here.", caster->name);
      mob->player.longDescr = mud_str_dup(cap(buf));
    }

#endif
  }
  return TRUE;
}

void TObj::pickMe(TBeing *thief)
{
  act("$p: That's not a container.", false, thief, this, 0, TO_CHAR);
}

void TRealContainer::pickMe(TBeing *thief)
{
  if (!isContainerFlag( CONT_CLOSED)) {
    act("$p: Silly - it ain't even closed!", false, thief, this, 0, TO_CHAR);
    return;
  }
  if (getKeyNum() < 0) {
    thief->sendTo("Odd - you can't seem to find a keyhole.\n\r");
    return;
  }
  if (!isContainerFlag( CONT_LOCKED)) {
    thief->sendTo("Oho! This thing is NOT locked!\n\r");
    return;
  }
  if (isContainerFlag( CONT_PICKPROOF)) {
    thief->sendTo("It resists your attempts at picking it.\n\r");
    return;
  }

  int bKnown = thief->getSkillValue(SKILL_PICK_LOCK);

  if (bSuccess(thief, bKnown, SKILL_PICK_LOCK)) {
    remContainerFlag( CONT_LOCKED);
    thief->sendTo("*Click*\n\r");
    act("$n fiddles with $p.", FALSE, thief, this, 0, TO_ROOM);
  } else {
    if (critFail(thief, SKILL_PICK_LOCK)) {
      act("Uhoh.  $n seems to have jammed the lock!", TRUE, thief, 0, 0, TO_ROOM);
      thief->sendTo("Uhoh.  You seemed to have jammed the lock!\n\r");
      addContainerFlag(CONT_PICKPROOF);
    } else {
      thief->sendTo("You fail to pick the lock.\n\r");
    }
  }
}
