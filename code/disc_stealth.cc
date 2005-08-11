#include "stdsneezy.h"
#include "games.h"
#include "disease.h"
#include "combat.h"
#include "disc_thief.h"
#include "obj_tool.h"
#include "disc_stealth.h"

void TBeing::doConceal(sstring argument)
{
  sstring name_buf;
  int rc;
  TBeing *vict;
 
  if (getSkillValue(SKILL_CONCEALMENT) <= 0) {
    sendTo("You do not have the ability to conceal paths.\n\r");
    return;
  }
  argument = one_argument(argument, name_buf);
 
  if (name_buf.empty()) {
    vict = this;
  } else {
    if (is_abbrev(name_buf, "off") || 
	is_abbrev(name_buf, "stop")) {
      one_argument(argument, name_buf);
      if (!name_buf.empty()) {
        vict = get_char_room_vis(this, name_buf);
        if (!vict) {
           sendTo("No such person present.\n\r");
           sendTo("Syntax: conceal off <person>\n\r");
           return;
        } else {
          affectedData *AffL;

          for(AffL = vict->affected; AffL; AffL = AffL->next)
            if (AffL->type == SKILL_CONCEALMENT)
              break;
          if (!AffL) {
            sendTo("How can you stop concealing their trail?  You never started.\n\r");
            return;
          } else if (AffL->be != this) {
            sendTo("Yes, their trail is concealed...But not by you.\n\r");
            return;
          }

          sendTo(COLOR_MOBS, fmt("You stop concealing %s's trail.\n\r") % vict->getName());
          vict->affectFrom(SKILL_CONCEALMENT);
          return;
        }
      }
      if (affectedBySpell(SKILL_CONCEALMENT)) {
        sendTo("You stop trying to conceal your trail.\n\r");
        affectFrom(SKILL_CONCEALMENT);
        return;
      } else {
        sendTo("You were not trying to conceal your trail.\n\r");
        return;
      }
    } else {
      vict = get_char_room_vis(this, name_buf);
      if (!vict) {
        sendTo("No such person present.\n\r");
        sendTo("Syntax: conceal <person>\n\r");
        return;
      }
    }
  }
  rc = conceal(this, vict);
  if (rc)
    addSkillLag(SKILL_CONCEALMENT, rc);

  return;
}
 
// return FALSE to cease tracking
int conceal(TBeing *caster, TBeing *vict)
{
  affectedData aff;
  int level = caster->getSkillLevel(SKILL_CONCEALMENT);
  int lnd = caster->getSkillValue(SKILL_CONCEALMENT);

  if (caster->getPosition() != POSITION_STANDING) {
    caster->sendTo("You need to be standing in order to conceal trails.\n\r");
    return FALSE;
  }
  if (caster->fight()) {
    caster->sendTo("The ensuing battle makes it difficult to conceal a trail.\n\r");
    return FALSE;
  }
  if (vict->fight()) {
    act("You can't conceal $N's path while $E is fighting.", 
         FALSE, caster, 0, vict, TO_CHAR);
    return FALSE;
  }
  if (vict->affectedBySpell(SKILL_CONCEALMENT)) {
    if (vict == caster)
      act("Your path is already being concealed.",
              FALSE, caster, 0, 0, TO_CHAR);
    else
      act("$N's path is already being concealed.",
              FALSE, caster, 0, vict, TO_CHAR);
    return FALSE;
  }
  if (vict != caster) {
    if (lnd <= 50) {
      act("You lack the training to conceal other people's path with any degree of success.",
         FALSE, caster, 0, 0, TO_CHAR);
      return FALSE;
    }
    // even if they can, reduce the chance of success
    lnd /= 2;
  }
  
  if (caster == vict) {  
    act("You attempt to conceal your path.",
        FALSE, caster, 0, 0, TO_CHAR);
    act("$n attempts to conceal $s path.",
        FALSE, caster, 0, 0, TO_ROOM);
  } else {
    act("You attempt to conceal $N's path.",
        FALSE, caster, 0, vict, TO_CHAR);
    act("$n attempts to conceal your path.",
        FALSE, caster, 0, vict, TO_VICT);
    act("$n attempts to conceal $N's path.",
        FALSE, caster, 0, vict, TO_NOTVICT);
  }

  aff.type = SKILL_CONCEALMENT;
  aff.duration = (level/2 + 1) * UPDATES_PER_MUDHOUR;
  aff.be = caster;

  if (caster->bSuccess(lnd, SKILL_CONCEALMENT)) {
    aff.level = level;
    aff.modifier = lnd;

    vict->affectTo(&aff);
  } else {
    aff.level = 0;
    aff.modifier = 0;

    vict->affectTo(&aff);
  }
  
  return TRUE;
}



int TBeing::doDisguise(const char *arg)
{
  sendTo("This skill has been temporarily disabled.\n\r");
  return FALSE;
  
  char name_buf[MAX_INPUT_LENGTH];
  int rc;

  if (!doesKnowSkill(SKILL_DISGUISE)) {
    sendTo("You know nothing about disguising yourself.\n\r");
    return FALSE;
  }
  if (isCombatMode(ATTACK_BERSERK)){
    sendTo("You can't disguise yourself while going berserk.\n\r");
    return FALSE;
  }
  one_argument(arg, name_buf);

  rc = disguise(this, name_buf);
  if (rc)
    addSkillLag(SKILL_DISGUISE, rc);

  return rc;
}

static const int LAST_DISGUISE_MOB = 28;
struct PolyType DisguiseList[LAST_DISGUISE_MOB] =
{
  /*
  Use RACE_NORACE when anything can use it.  Race will be subbed
  later, not name/desc tho, for the thiefs race.

  All grabbed, so far, from zones: 0-4

  If you add any between the first one and up to the Racial Specific,
  or remove one either, then please address the stuff in gaining.cc
  also.  Thanks  --Lapsos
   */

  {"citizen male"       ,  1 , 1, 108, DISC_STEALTH, RACE_NORACE},
  {"citizen female"     ,  1 , 1, 109, DISC_STEALTH, RACE_NORACE},
  {"bar-hopper male"    ,  5, 10,  52, DISC_STEALTH, RACE_NORACE},
  {"bar-hopper female"  ,  5, 10,  53, DISC_STEALTH, RACE_NORACE},
  {"church male"        , 10, 20,  50, DISC_STEALTH, RACE_NORACE},
  {"church female"      , 10, 20,  51, DISC_STEALTH, RACE_NORACE},
  {"old-man male"       , 15, 30, 174, DISC_STEALTH, RACE_NORACE},
  {"old-woman female"   , 15, 30, 175, DISC_STEALTH, RACE_NORACE},
  {"patron male"        , 20, 40, 300, DISC_STEALTH, RACE_NORACE},
  {"patron female"      , 20, 40, 301, DISC_STEALTH, RACE_NORACE},

  {"peasant"            , 10, 20, 110, DISC_STEALTH, RACE_NORACE},
  {"bard"               , 25, 50, 188, DISC_STEALTH, RACE_NORACE},
  {"pilgrim"            , 30, 60, 186, DISC_STEALTH, RACE_NORACE},

  /** Gender Specific (Meaning not a set) **/
  {"drunk male"         , 15, 30, 106, DISC_STEALTH, RACE_NORACE},
  {"evening-lady female", 15, 30, 130, DISC_STEALTH, RACE_NORACE},
  {"deputy male"        , 20, 40, 121, DISC_STEALTH, RACE_NORACE},
  {"gypsy male"         , 20, 40, 191, DISC_STEALTH, RACE_NORACE},
  {"cityguard male"     , 25, 50, 101, DISC_STEALTH, RACE_NORACE},

  /** Race Specfic **/
  {"merchant dwarf"     , 20, 40, 197, DISC_STEALTH, RACE_DWARF},
  {"ambassador male"    , 40, 80, 125, DISC_STEALTH, RACE_DWARF},
  {"young-male gnome"   , 20, 40, 124, DISC_STEALTH, RACE_GNOME},
  {"missionary male"    , 40, 80, 126, DISC_STEALTH, RACE_GNOME},
  {"outcast ogre"       , 20, 40, 196, DISC_STEALTH, RACE_OGRE}, 
  {"vigilante male"     , 40, 80, 127, DISC_STEALTH, RACE_OGRE},
  {"trader hobbit"      , 20, 40, 198, DISC_STEALTH, RACE_HOBBIT},
  {"emissary male"      , 40, 80, 128, DISC_STEALTH, RACE_HOBBIT}, //***
  {"tradeswoman female" , 20, 40, 138, DISC_STEALTH, RACE_ELVEN},
  {"traveler elf"       , 40, 80, 195, DISC_STEALTH, RACE_ELVEN},
};

int disguise(TBeing *caster, char * buffer)
{
  int i, duration, column=0;
  TMonster *mob;
  affectedData aff;
  affectedData aff2;
  affectedData aff3;

  if (!caster->isImmortal() && caster->checkForSkillAttempt(SKILL_DISGUISE)) {
    act("You are not prepared to try to disguise yourself again so soon.",
         FALSE, caster, NULL, NULL, TO_CHAR);
    return SPELL_FAIL;
  }

  if(!*buffer)
    caster->sendTo("You know how to apply the following disguises:\n\r");

  // Find not only the first match but the first match that
  // also works out in comparision to the thief.
  for (i = 0; (i < LAST_DISGUISE_MOB); i++) {
    if (((isname("male"  , DisguiseList[i].name) &&
          caster->getSex() != SEX_MALE  ) ||
         (isname("female", DisguiseList[i].name) &&
          caster->getSex() != SEX_FEMALE)) && !caster->isImmortal())
      continue;

    if ((signed) DisguiseList[i].tRace != RACE_NORACE &&
        !caster->isImmortal() &&
        caster->getRace() != (signed) DisguiseList[i].tRace)
      continue;

    if (DisguiseList[i].level > caster->GetMaxLevel())
      continue;

    if (DisguiseList[i].learning > caster->getSkillValue(SKILL_DISGUISE))
      continue;

    if(!*buffer){
      caster->sendTo(fmt("%-25s") % DisguiseList[i].name);
      if((column++)==2){
	caster->sendTo("\n\r");
	column=0;
      }
      continue;
    } else if (!isname(buffer, DisguiseList[i].name))
      continue;

    break;
  }

  if(!*buffer){
    caster->sendTo("\n\r");
    return FALSE;
  }

  if (i >= LAST_DISGUISE_MOB) {
    caster->sendTo("You haven't a clue where to start on that one.\n\r");
    return FALSE;
  }

  int level = caster->getSkillLevel(SKILL_DISGUISE);
  int bKnown = caster->getSkillValue(SKILL_DISGUISE);

  discNumT das = getDisciplineNumber(SKILL_DISGUISE, FALSE);
  if (das == DISC_NONE) {
    vlogf(LOG_BUG, "bad disc for SKILL_DISGUISE");
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
  mob->swapToStrung();

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
  aff.duration = (2 + (level/5)) * UPDATES_PER_MUDHOUR;
  aff.bitvector = 0;
  aff.modifier = SKILL_DISGUISE;

  if (!caster->bSuccess(bKnown, SKILL_DISGUISE)) {
    caster->sendTo("You seem to have screwed something up.\n\r");
    delete mob;
    mob = NULL;
    caster->affectTo(&aff, -1);
    return TRUE;
  }

  int awesom = TRUE;

  if ((critFail(caster, SKILL_DISGUISE) != CRIT_F_NONE))
    awesom = FALSE;

  switch (critSuccess(caster, SKILL_DISGUISE)) {
    case CRIT_S_KILL:
      CS(SKILL_DISGUISE);
      duration = 20 * UPDATES_PER_MUDHOUR;
      break;
    case CRIT_S_TRIPLE:
      if (caster->getSkillLevel(SKILL_DISGUISE) > 20)
        CS(SKILL_DISGUISE);

      duration = 15 * UPDATES_PER_MUDHOUR;
      break;
    case CRIT_S_DOUBLE:
      if (caster->getSkillLevel(SKILL_DISGUISE) > 40)
        CS(SKILL_DISGUISE);

      duration = 15 * UPDATES_PER_MUDHOUR;
      break;
    default:
      duration = 10 * UPDATES_PER_MUDHOUR;
      break;
  }
  act("You apply your skills and make yourself look like $N.", 
       TRUE, caster, NULL, mob, TO_CHAR);
  act("$n applies $s skills and makes $mself look like $N.", 
       TRUE, caster, NULL, mob, TO_ROOM);

// first add the attempt -- used to regulate attempts
  aff.duration = duration + (2 * UPDATES_PER_MUDHOUR);
  caster->affectTo(&aff, -1);

  aff3.type = AFFECT_SKILL_ATTEMPT;
  aff3.location = APPLY_NONE;
  aff3.duration = duration + (2 * UPDATES_PER_MUDHOUR);
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

  appendPlayerName(caster, mob);
  
 /*
  It is critical to remember that some diguises are race/sex independent
  so they Must be set here else it'll not always fit.
   */
  mob->setRace(caster->getRace());
  mob->setHeight(caster->getHeight());
  mob->setWeight(caster->getWeight());

  return TRUE;
}
