//////////////////////////////////////////////////////////////////////////
//
// SneezyMUD - All rights reserved, SneezyMUD Coding Team
//
// cmd_rescue.cc
//
//////////////////////////////////////////////////////////////////////////

#include <boost/format.hpp>
#include <string.h>
#include <list>
#include <memory>

#include "ansi.h"
#include "being.h"
#include "comm.h"
#include "defs.h"
#include "disc_deikhan.h"
#include "enum.h"
#include "extern.h"
#include "handler.h"
#include "room.h"
#include "spell2.h"
#include "spells.h"
#include "sstring.h"
#include "structs.h"
#include "thing.h"

static int rescue(TBeing* caster, TBeing* victim, spellNumT skill) {
  TBeing* tmp_ch = nullptr;
  int percent = 0;

  if (caster->getCombatMode() == ATTACK_BERSERK) {
    caster->sendTo(
      "You are berserking! You can't focus enough to rescue anyone!\n\r");
    return false;
  }

  if (caster->checkPeaceful("No one should need rescuing here.\n\r"))
    return false;

  if (victim == caster) {
    caster->sendTo("What about fleeing instead?\n\r");
    return false;
  }

  if (victim->riding) {
    caster->sendTo(COLOR_MOBS, format("You can't rescue %s off of %s!\n\r") %
                                 victim->getName() % victim->riding->getName());
    return false;
  }

  if (caster->fight() == victim) {
    caster->sendTo("How can you rescue someone you are trying to kill?\n\r");
    return false;
  }

  for (StuffIter it = victim->roomp->stuff.begin();
       it != victim->roomp->stuff.end(); ++it, tmp_ch = nullptr) {
    tmp_ch = dynamic_cast<TBeing*>(*it);
    if (!tmp_ch)
      continue;
    if (tmp_ch->fight() == victim)
      break;
  }

  if (!tmp_ch) {
    act("But nobody is fighting $M?", false, caster, 0, victim, TO_CHAR);
    return false;
  }

  if (caster->noHarmCheck(tmp_ch)) {
    act("You cannot rescue someone who is fighting that target?", false, caster,
      0, victim, TO_CHAR);
    return false;
  }

  // int level = caster->getSkillLevel(skill);
  int bKnown = caster->getSkillValue(skill);

  if (caster->bSuccess(bKnown + percent, skill)) {
    caster->sendTo("Banzai! To the rescue...\n\r");
    act("You are rescued by $N, you are confused!", false, victim, 0, caster,
      TO_CHAR);
    act("$n heroically rescues $N.", false, caster, 0, victim, TO_NOTVICT);

    if (victim->fight() == tmp_ch)
      victim->stopFighting();
    if (tmp_ch->fight())
      tmp_ch->stopFighting();
    if (caster->fight())
      caster->stopFighting();

    caster->setCharFighting(tmp_ch);
    caster->setVictFighting(tmp_ch);

    victim->addToWait(combatRound(1));

    // We've succeeded the rescue and done all the important shit
    // Now check for divine rescue
    if (caster->doesKnowSkill(SKILL_DIVINE_RESCUE)) {
      divineRescue(caster, victim);
    }

    if (tmp_ch->GetMaxLevel() >= victim->GetMaxLevel()) {
      if (caster->getFactionTarget() != victim) {
        caster->reconcileHelp(victim, discArray[skill]->alignMod);
        caster->setFactionTarget(victim);
      } else {
        // rescuing over and over, or rescuing back and forth
        // let it help some but not as much
        caster->reconcileHelp(victim, discArray[skill]->alignMod / 4.0);
      }
    }
    return true;
  } else {
    caster->sendTo("You fail the rescue.\n\r");
    act("$n attempts to rescue you, but fails miserably.", false, caster, 0,
      victim, TO_VICT);
    act("$n attempts to rescue $N, but fails miserably.", false, caster, 0,
      victim, TO_NOTVICT);

    // start them fighting at least
    if (!caster->fight())
      caster->setCharFighting(tmp_ch);
  }
  return true;
}

int TBeing::doRescue(const char* argument) {
  int rc;
  TBeing* victim = nullptr;
  char name_buf[240];

  spellNumT skill = getSkillNum(SKILL_RESCUE);

  if (checkBusy())
    return false;

  if (!doesKnowSkill(skill)) {
    sendTo("You know nothing about rescuing.\n\r");
    return false;
  }

  // Default to the first PC group member who is currently tanking, or the first
  // mobile group member if no PC is.
  if (isAffected(AFF_GROUP) && (!argument || !*argument) &&
      (followers || (master && master->followers))) {
    followData* tFData = (master ? master->followers : followers);
    TBeing *tRescueMe = nullptr, *tMaybeMe = nullptr;

    if (master && master->isAffected(AFF_GROUP) && sameRoom(*master) &&
        canSee(master) && master->fight() &&
        (master->fight()->fight() == master))
      tRescueMe = master;
    else
      for (; tFData; tFData = tFData->next)
        if (tFData->follower && tFData->follower->isAffected(AFF_GROUP) &&
            canSee(tFData->follower) && tFData->follower->fight() &&
            (tFData->follower->fight()->fight() == tFData->follower) &&
            (tFData->follower != this) && sameRoom(*tFData->follower)) {
          if (tFData->follower->isPc()) {
            tRescueMe = tFData->follower;
            break;
          } else if (!tMaybeMe)
            tMaybeMe = tFData->follower;
        }

    if (!tRescueMe && tMaybeMe)
      tRescueMe = tMaybeMe;

    victim = tRescueMe;  // Since this is nullptr'ed above we are safe doing this
                         // without a check.
  } else
    strcpy(name_buf, argument);

  if (!victim)
    if (!(victim = get_char_room_vis(this, name_buf))) {
      sendTo("Whom do you want to rescue?\n\r");
      return false;
    }

  if (!sameRoom(*victim)) {
    sendTo("That person isn't around.\n\r");
    return false;
  }

  rc = rescue(this, victim, skill);

  if (rc)
    addSkillLag(skill, rc);

  if (IS_SET_DELETE(rc, DELETE_THIS))
    return DELETE_THIS;

  return rc;
}
