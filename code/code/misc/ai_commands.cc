//////////////////////////////////////////////////////////////////////////
//
//      SneezyMUD - All rights reserved, SneezyMUD Coding Team
//      "ai_commands.cc" - handles the actual responses to all commands
//        a user can do for use with the mob AI routines.
//
//      The SneezyMUD mob AI was coded by Jeff Bennett, August 1994.
//      Last revision, December 24th, 1994.
//
//////////////////////////////////////////////////////////////////////////

//  return DELETE_THIS if this dies, or DELETE_VICT if doer dies

#include "room.h"
#include "extern.h"
#include "being.h"
#include "low.h"
#include "colorstring.h"
#include "monster.h"
#include "combat.h"
#include "materials.h"
#include "spec_mobs.h"

int TMonster::aiBounce(TBeing* doer, TBeing*, aiTarg cond) {
  switch (cond) {
    case TARGET_NONE:
      if (!isPerceptive()) {
        if (!::number(0, 2)) {
          act("$n's head bobs up and down following your antics.", true, this,
            0, doer, TO_VICT);
          act("$n's head bobs up and down as $e watches $N.", true, this, 0,
            doer, TO_NOTVICT);
        }
      } else {
        US(1);
        if (!::number(0, 2)) {
          act("$n thinks you are strange.", true, this, 0, doer, TO_VICT);
          act("$n looks at $N strangely.", true, this, 0, doer, TO_NOTVICT);
        }
      }
      break;
    case TARGET_SELF:
      if (!isPerceptive()) {
        if (!::number(0, 2)) {
          act("$n's head bobs up and down following your antics.", true, this,
            0, doer, TO_VICT);
          act("$n's head bobs up and down as $e watches $N.", true, this, 0,
            doer, TO_NOTVICT);
        }
      } else {
        US(1);
        if (!::number(0, 2)) {
          act("$n thinks you are strange.", true, this, 0, doer, TO_VICT);
          act("$n looks at $N strangely.", true, this, 0, doer, TO_NOTVICT);
        }
      }
      break;
    case TARGET_MOB:
      if (isAngry()) {
        act("$n looks a bit uncomfortable doing this.", true, this, 0, doer,
          TO_VICT);
        act("$n doesn't like all this bouncing.", true, this, 0, doer,
          TO_NOTVICT);
        UA(2);
        UM(1);
      } else {
        act("$n enjoys $mself.", true, this, 0, doer, TO_VICT);
        act("$n likes to bounce like this.", true, this, 0, doer, TO_NOTVICT);
        DA(2);
        DMal(1);
      }
      break;
    case TARGET_OTHER:
      if (!::number(0, 2))
        act("$n head bobs about watching people bounce around.", true, this, 0,
          doer, TO_ROOM);
      break;
  }

  return false;
}

int TMonster::aiDance(TBeing* doer, TBeing* other, aiTarg cond) {
  if (!isHumanoid()) {
    if (cond == TARGET_MOB) {
      UA(3);
      US(4);
      UM(1);
      act("$n doesn't like dancing as $e is an animal.", true, this, 0, doer,
        TO_VICT);
    }
    return false;
  }
  switch (cond) {
    case TARGET_NONE:
    case TARGET_SELF:
      if (!doer->isUgly() && doer->isAgile(0)) {
        DA(2);
        DMal(1);
        if (!::number(0, 2)) {
          act("$n checks out your moves.", true, this, 0, doer, TO_VICT);
          act("$n watches $N's moves.", true, this, 0, doer, TO_NOTVICT);
        }
      } else if (!doer->isUgly()) {
        DA(1);
        if (!::number(0, 2)) {
          act("$n thinks you're cute, but clumsy.", true, this, 0, doer,
            TO_VICT);
          act("$n watches $N dance.", true, this, 0, doer, TO_NOTVICT);
        }
      } else if (doer->isAgile(0)) {
        DA(1);
        if (!::number(0, 2)) {
          act("$n watches your fancy footwork.", true, this, 0, doer, TO_VICT);
          act("$n watches $N dance.", true, this, 0, doer, TO_NOTVICT);
        }
      } else {
        US(1);
        if (!::number(0, 2)) {
          act("$n looks at you strangely.", true, this, 0, doer, TO_VICT);
          act("$n looks at $N's move with disgust.", true, this, 0, doer,
            TO_NOTVICT);
        }
      }
      break;
    case TARGET_MOB:
      if ((doer->getSex() == SEX_NEUTER) || (getSex() == SEX_NEUTER)) {
        act("$n dances with you without emotion.", true, this, 0, doer,
          TO_VICT);
        act("$n dances emotionlessly with $N.", true, this, 0, doer,
          TO_NOTVICT);
      } else if (doer->getSex() == getSex()) {
        act("$n prefers dancing with the opposite sex!", true, this, 0, doer,
          TO_ROOM);
        if (pissed() && !::number(0, 2)) {
          return takeFirstHit(*doer);
        } else if (pissed())
          aiFag(doer, 1);
      } else if (!doer->isUgly() && doer->isAgile(0)) {
        DA(6);
        DMal(3);
        act("$n enjoys dancing with you.", true, this, 0, doer, TO_VICT);
        act("$n enjoys dancing with $N.", true, this, 0, doer, TO_NOTVICT);
      } else if (!doer->isUgly()) {
        act("$n gazes longingly into your eyes as your dance.", true, this, 0,
          doer, TO_VICT);
        DMal(2);
        DA(4);
      } else if (doer->isUgly()) {
        UA(6);
        UM(3);
        act("$n is totally repulsed by your looks!", true, this, 0, doer,
          TO_VICT);
        act("$n is repulsed by $N's ugliness as they dance.", true, this, 0,
          doer, TO_NOTVICT);
        if (pissed()) {
          return takeFirstHit(*doer);
        }
      } else if (doer->isAgile(0)) {
        DMal(2);
        DA(4);
        act("$n is enthralled by your superb dancing.", true, this, 0, doer,
          TO_VICT);
      } else {
        US(3);
        UA(5);
        UM(1);
        act("$n winces in pain as you step on $s foot.", true, this, 0, doer,
          TO_VICT);
      }
      break;
    case TARGET_OTHER:
      if ((doer->getSex() == SEX_NEUTER) || (getSex() == SEX_NEUTER) ||
          (other->getSex() == SEX_NEUTER)) {
        // could care less about the dance
      } else if (doer->getSex() == other->getSex()) {
        if (!::number(0, 3)) {
          if (doer->getSex() == SEX_MALE)
            act("$n would prefer not to watch two guys dance together.", true,
              this, 0, doer, TO_ROOM);
          else
            act("$n would prefer not to watch two girls dance together.", true,
              this, 0, doer, TO_ROOM);
        }
        if (pissed() && !::number(0, 3)) {
          return takeFirstHit(*doer);
        } else if (pissed())
          aiFag(doer, 1);
      } else if ((getSex() != other->getSex()) && !other->isUgly()) {
        UA(2);
        UM(1);
        if (::number(0, 1)) {
          act("$n seems upset to see $N dancing with someone else.", true, this,
            0, other, TO_NOTVICT);
          act("$n watches you dance with someone else.", true, this, 0, other,
            TO_VICT);
        }
      } else if (!doer->isUgly() && (doer->getSex() != getSex())) {
        UA(2);
        UM(1);
        if (!::number(0, 1)) {
          act("$n seems upset to see $N dancing with someone else.", true, this,
            0, doer, TO_NOTVICT);
          act("$n watches you dance with someone else.", true, this, 0, doer,
            TO_VICT);
        }
      } else {
        US(1);
        if (!::number(0, 2)) {
          act("$n looks at you strangely.", true, this, 0, doer, TO_VICT);
          act("$n looks at $N's strangely.", true, this, 0, doer, TO_NOTVICT);
        }
      }
      break;
  }

  return false;
}

int TMonster::aiSmile(TBeing* doer, TBeing* other, aiTarg cond) {
  if (!isHumanoid())
    return false;

  switch (cond) {
    case TARGET_NONE:
    case TARGET_SELF:
      if (!doer->isUgly() && !isSusp()) {
        DA(2);
        DMal(1);
        DS(2);
        if (!::number(0, 2)) {
          act("$n's face brightens up as you smile.", true, this, 0, doer,
            TO_VICT);
          act("$n's face brightens as $N smiles.", true, this, 0, doer,
            TO_NOTVICT);
        }
      } else if (!doer->isUgly()) {
        US(1);
        DA(2);
        if (!::number(0, 2)) {
          act("$n smiles back nervously.", true, this, 0, doer, TO_VICT);
          act("$n smiles nervously back at $N.", true, this, 0, doer,
            TO_NOTVICT);
        }
      } else if (isSusp()) {
        UA(1);
        US(2);
        if (!::number(0, 2)) {
          act("$n smiles back, but begins to watch you.", true, this, 0, doer,
            TO_VICT);
          act("$n smiles as $e begins to watch $N more closely.", true, this, 0,
            doer, TO_NOTVICT);
        }
      } else {
        US(4);
        UA(1);
        if (!::number(0, 2)) {
          act("$n looks at you suspiciously.", true, this, 0, doer, TO_VICT);
          act("$n looks at $N suspiciously.", true, this, 0, doer, TO_NOTVICT);
        }
      }
      break;
    case TARGET_MOB:
      if ((doer->getSex() == SEX_NEUTER) || (getSex() == SEX_NEUTER)) {
        act("$n emotionlessly smiles back.", true, this, 0, doer, TO_VICT);
        act("$n emotionlessly smiles back at $N.", true, this, 0, doer,
          TO_NOTVICT);
      } else if (doer->getSex() == getSex()) {
        if (doer->getSex() == SEX_MALE)
          act("$n wonders about men who smile at him.", true, this, 0, doer,
            TO_ROOM);
        else {
          act("$n gives you that fake little smile girls use.", true, this, 0,
            doer, TO_VICT);
          act("$n gives $N a fake-smile back.", true, this, 0, doer,
            TO_NOTVICT);
        }
        UM(1);
        UA(3);
        US(6);
        if (pissed())
          aiFag(doer, 1);
      } else if ((doer->isUgly() && doer->isUgly()) && isSusp()) {
        UA(5);
        UM(2);
        US(6);
        act("$n watches you cautiously.", true, this, 0, doer, TO_VICT);
        act("$n watches $N alertly.", true, this, 0, doer, TO_NOTVICT);
      } else if (doer->isUgly()) {
        doAction(fname(doer->name), CMD_SMILE);
        DA(2);
      } else {
        US(1);
        DA(3);
        DMal(1);
        act("$n likes it when you smile at $m.", true, this, 0, doer, TO_VICT);
      }
      break;
    case TARGET_OTHER:
      if ((doer->getSex() == SEX_NEUTER) || (getSex() == SEX_NEUTER) ||
          (other->getSex() == SEX_NEUTER)) {
        // could care less about it
      } else if ((getSex() != other->getSex()) && !other->isUgly()) {
        UA(2);
        UM(1);
        if (::number(0, 1)) {
          act("$n smiles at $N too.", true, this, 0, other, TO_NOTVICT);
          act("$n smiles at you as well.", true, this, 0, other, TO_VICT);
        }
      } else if (!doer->isUgly() && (doer->getSex() != getSex())) {
        UA(2);
        UM(1);
        if (!::number(0, 1))
          act("$n seems upset that you didn't smile at $m.", true, this, 0,
            doer, TO_VICT);
      } else {
        // who cares?  they're ugly   :)
      }
      break;
  }

  return false;
}

int TMonster::aiCackle(TBeing* doer, TBeing*, aiTarg cond) {
  if (!isHumanoid()) {
    aiRudeNoise(doer);
    return false;
  }

  switch (cond) {
    case TARGET_NONE:
    case TARGET_SELF:
    case TARGET_MOB:
    case TARGET_OTHER:
      if (isSusp())
        US(2);
      aiStrangeThings(doer);
      break;
  }

  return false;
}

int TMonster::aiLaugh(TBeing* doer, TBeing* other, aiTarg cond) {
  if (!isHumanoid()) {
    aiRudeNoise(doer);
    return false;
  }

  switch (cond) {
    case TARGET_NONE:
      DA(1);
      if (!::number(0, 2))
        DMal(1);
      if (!::number(0, 3) && !isAngry())
        doAction("", CMD_CHUCKLE);
      break;
    case TARGET_SELF:
      aiStrangeThings(doer);
      DA(1);
      break;
    case TARGET_MOB:
      if (isAngry()) {
        if (!::number(0, 2))
          doAction(fname(doer->name), CMD_SNARL);
        UA(1);
      } else {
        aiShutUp(doer);
      }
      break;
    case TARGET_OTHER:
      if (!::number(0, 2)) {
        if (::number(0, 1))
          doAction("", CMD_SNICKER);
        else
          doAction("", CMD_SMIRK);
      }
      DA(1);
      if (::number(0, 1))
        DS(1);
      if (::number(0, 1))
        DMal(1);
      aiOtherInsulted(doer, other);
      break;
  }
  return false;
}

int TMonster::aiGiggle(TBeing* doer, TBeing*, aiTarg cond) {
  if (!isHumanoid()) {
    // doubt they would understand a giggle
    return false;
  }

  switch (cond) {
    case TARGET_NONE:
    case TARGET_SELF:
    case TARGET_MOB:
    case TARGET_OTHER:
      DA(1);
      if (!::number(0, 2))
        DMal(1);
      if (!::number(0, 3) && !isAngry() && (getSex() == SEX_FEMALE))
        doAction("", CMD_GIGGLE);
      else if (!::number(0, 4)) {
        act("$n thinks you're weird.", true, this, 0, doer, TO_VICT);
        act("$n looks at $N oddly.", true, this, 0, doer, TO_NOTVICT);
        US(1);
      }
      break;
  }

  return false;
}

int TMonster::aiShake(TBeing* doer, TBeing*, aiTarg cond) {
  if (!isHumanoid()) {
    US(1);
    if (cond == TARGET_SELF) {
      act("$n looks at you questiongly.", true, this, 0, doer, TO_VICT);
      if (doer->getSex() == SEX_MALE)
        act("$n watches the jello-man wobble around.", true, this, 0, doer,
          TO_NOTVICT);
      else if (doer->getSex() == SEX_FEMALE)
        act("$n watches the jello-girl wobble around.", true, this, 0, doer,
          TO_NOTVICT);
      else
        act("$n watches the jello-thing wobble around.", true, this, 0, doer,
          TO_NOTVICT);
    } else if ((cond == TARGET_MOB) && (getRace() == RACE_CANINE)) {
      if (isPerceptive())
        act("$n learned this trick in obedience school.", true, this, 0, doer,
          TO_ROOM);
      else {
        act(
          "$n doesn't know that trick and gets mad when you force $m to do it.",
          true, this, 0, doer, TO_VICT);
        doAction(fname(doer->name), CMD_GROWL);
        UA(1);
      }
    } else if ((cond == TARGET_MOB) && isStrong()) {
      act("$n has been known to rip the arms off of folks who do that...", true,
        this, 0, doer, TO_VICT);
      UA(2);
      UM(1);
      if (aggro()) {
        doAction(fname(doer->name), CMD_GROWL);
        return takeFirstHit(*doer);
      }
    } else
      US(1);
    return false;
  }
  switch (cond) {
    case TARGET_NONE:
      break;  // shakes their head
    case TARGET_SELF:
      act("$n looks at you questiongly.", true, this, 0, doer, TO_VICT);
      if (doer->getSex() == SEX_MALE)
        act("$n watches the jello-man wobble around.", true, this, 0, doer,
          TO_NOTVICT);
      else if (doer->getSex() == SEX_FEMALE)
        act("$n watches the jello-girl wobble around.", true, this, 0, doer,
          TO_NOTVICT);
      else
        act("$n watches the jello-thing wobble around.", true, this, 0, doer,
          TO_NOTVICT);
      if (!::number(0, 2)) {
        doAction("", CMD_SNICKER);
      }
      break;
    case TARGET_MOB:
      if (isAngry()) {
        if (pissed()) {
          return takeFirstHit(*doer);
        } else
          aiUpset(doer);
      } else if (isSusp()) {
        if (::number(0, 2))
          UM(1);
        UA(1);
        US(1);
        act("$n smiles nervously.", 0, this, 0, 0, TO_ROOM);
      } else {
        DA(1);
        doAction(fname(doer->name), CMD_SMILE);
      }
      break;
    case TARGET_OTHER:
      break;
  }

  return false;
}

int TMonster::aiPuke(TBeing* doer, TBeing*, aiTarg cond) {
  if (!isHumanoid()) {
    UA(1);
    if (cond == TARGET_MOB) {
      UM(4);
      UA(6);
      doAction(fname(doer->name), CMD_SNARL);
      if (pissed()) {
        return takeFirstHit(*doer);
      }
    }
    return false;
  }
  switch (cond) {
    case TARGET_MOB:
      UM(4);
      UA(6);
      doAction(fname(doer->name), CMD_SNARL);
      aiInsultDoer(doer);
      if (pissed()) {
        return takeFirstHit(*doer);
      }
    case TARGET_NONE:
    case TARGET_SELF:
    case TARGET_OTHER:
      UA(2);
      UM(2);
      US(1);
      if (!::number(0, 3))
        doSay("Oh yuck!");
      if (!::number(0, 4)) {
        doSay("I'm not cleaning THAT up.");
      }
      break;
  }

  return false;
}

int TMonster::aiGrowl(TBeing* doer, TBeing*, aiTarg cond) {
  if (!isHumanoid()) {
    UA(1);
    US(2);
    if (cond == TARGET_MOB) {
      UA(3);
      UM(2);
      US(4);
      if (getMaterial(WEAR_BACK) == MAT_FUR_DOG)
        act("The hair on $n's back bristles.", true, this, 0, doer, TO_ROOM);
      if (pissed()) {
        doAction("", CMD_SNARL);
        return takeFirstHit(*doer);
      }
    }
    return false;
  }
  switch (cond) {
    case TARGET_SELF:
      US(1);
    case TARGET_NONE:
      UA(1);
      US(2);
      if (!::number(0, 2))
        doSay("What's the problem?");
      break;
    case TARGET_MOB:
      aiUpset(doer);
      if (getMaterial(WEAR_BACK) == MAT_FUR_DOG)
        act("The hair on $n's back bristles.", true, this, 0, doer, TO_ROOM);
      if (pissed()) {
        return takeFirstHit(*doer);
      }
      break;
    case TARGET_OTHER:
      break;
  }
  return false;
}

int TMonster::aiScream(TBeing* doer, TBeing*, aiTarg cond) {
  if (!isHumanoid()) {
    aiRudeNoise(doer);
    return false;
  }
  switch (cond) {
    case TARGET_NONE:
      US(3);
      if (!isSusp()) {
        if (!::number(0, 3)) {
          act("$N gets startled and jumps slightly.", true, doer, 0, this,
            TO_CHAR);
          act("You jump as $N startles you.", true, doer, 0, this, TO_VICT);
          act("$N jumps slightly as $n startles $M.", true, doer, 0, this,
            TO_ROOM);
          UM(1);
          US(1);
          UA(1);
          if (!::number(0, 2)) {
            if (!isAngry()) {
              doSay("Ooooh, you scared me.");
            } else {
              UA(2);
            }
          }
        }
      }
      break;
    case TARGET_SELF:
    case TARGET_MOB:
    case TARGET_OTHER:
      vlogf(LOG_MOB_AI, "TMonster::aiScream is whacky.");
      break;
  }
  return false;
}

int TMonster::aiComfort(TBeing* doer, TBeing*, aiTarg cond) {
  if (!isHumanoid()) {
    // lets just assume they do nothing
    return false;
  }
  switch (cond) {
    case TARGET_NONE:
      break;
    case TARGET_SELF:
      aiStrangeThings(doer);
      break;
    case TARGET_MOB:
      if (isSusp()) {
        US(2);
        UA(1);
        if (isAngry() && !::number(0, 2))
          doSay("Stop being so damn condescending.");
      } else {
        DA(2);
        DMal(1);
        if (!::number(0, 3))
          doSay("Thanks for understanding.");
      }
      break;
    case TARGET_OTHER:
      // comfort reaction towards other is in cry, whine, etc
      break;
  }
  return false;
}

int TMonster::aiNod(TBeing* doer, TBeing*, aiTarg cond) {
  if (!isHumanoid()) {
    // sheyah, like they know what that would mean
    return false;
  }
  switch (cond) {
    case TARGET_NONE:
    case TARGET_SELF:
      break;
    case TARGET_MOB:
      if (!::number(0, 2))
        doAction(fname(doer->name), CMD_SMILE);
      break;
    case TARGET_OTHER:
      break;
  }
  return false;
}

int TMonster::aiSigh(TBeing*, TBeing*, aiTarg cond) {
  if (!isHumanoid()) {
    return false;
  }
  switch (cond) {
    case TARGET_NONE:
      if (isAngry()) {
        US(1);
        UM(1);
        if (!::number(0, 3)) {
          doSay("Stop being such a wussy.");
        }
      } else {
        US(1);
        if (!::number(0, 3))
          doSay("what's wrong?");
      }
      break;
    case TARGET_SELF:
    case TARGET_MOB:
    case TARGET_OTHER:
      vlogf(LOG_MOB_AI, "Wierd call into TMonster::aiSigh.");
      break;
  }
  return false;
}

int TMonster::aiSulk(TBeing*, TBeing*, aiTarg cond) {
  if (!isHumanoid()) {
    return false;
  }
  switch (cond) {
    case TARGET_NONE:
    case TARGET_SELF:
    case TARGET_MOB:
    case TARGET_OTHER:
      US(1);
      if (::number(0, 1))
        break;
      if (!pissed())
        doSay("What's the matter?");
      else
        doSay("Stop being such a crybaby.");
      break;
  }
  return false;
}

int TMonster::aiHug(TBeing* doer, TBeing* other, aiTarg cond) {
  if (aiLoveNonHumanoid(doer, cond))
    return false;

  switch (cond) {
    case TARGET_NONE:
      break;
    case TARGET_SELF:
      aiLoveSelf(doer);
      break;
    case TARGET_MOB:
      if ((doer->getSex() == SEX_NEUTER) || (getSex() == SEX_NEUTER)) {
        act("$n holds you without emotion.", true, this, 0, doer, TO_VICT);
        act("$n emotionlessly hugs $N back.", true, this, 0, doer, TO_NOTVICT);
      } else if (doer->getSex() == getSex()) {
        if (doer->getSex() == SEX_FEMALE) {
          aiFag(doer, 1);
          US(2);
          if (isSusp()) {
            UM(1);
            UA(1);
          } else {
            DMal(1);
            DA(1);
          }
        } else {
          aiFag(doer, 1);
          US(3);
          UM(2);
          UA(3);
        }
      } else {
        aiMudSex(doer);
      }
      break;
    case TARGET_OTHER:
      if ((doer->getSex() == SEX_NEUTER) || (other->getSex() == SEX_NEUTER)) {
      } else if (doer->getSex() == other->getSex()) {
        aiFag(doer, 0);
      } else {
        aiMudSexOther(doer, other);
      }
      US(2);
      break;
  }
  return false;
}

int TMonster::aiSnuggle(TBeing* doer, TBeing* other, aiTarg cond) {
  if (aiLoveNonHumanoid(doer, cond))
    return false;

  switch (cond) {
    case TARGET_NONE:
      break;
    case TARGET_SELF:
      aiLoveSelf(doer);
      break;
    case TARGET_MOB:
      if ((doer->getSex() == SEX_NEUTER) || (getSex() == SEX_NEUTER)) {
        act("$n holds you without emotion.", true, this, 0, doer, TO_VICT);
        act("$n emotionlessly holds $N.", true, this, 0, doer, TO_NOTVICT);
      } else if (doer->getSex() == getSex()) {
        if (doer->getSex() == SEX_FEMALE) {
          aiFag(doer, 1);
          US(2);
          if (isSusp()) {
            UM(1);
            UA(1);
          } else {
            DMal(1);
            DA(1);
          }
        } else {
          aiFag(doer, 1);
          US(3);
          UM(2);
          UA(3);
        }
      } else {
        aiMudSex(doer);
      }
      break;
    case TARGET_OTHER:
      if ((doer->getSex() == SEX_NEUTER) || (other->getSex() == SEX_NEUTER)) {
      } else if (doer->getSex() == other->getSex())
        aiFag(doer, 0);
      else
        aiMudSexOther(doer, other);
      US(2);
      break;
  }
  return false;
}

int TMonster::aiCuddle(TBeing* doer, TBeing* other, aiTarg cond) {
  if (aiLoveNonHumanoid(doer, cond))
    return false;

  switch (cond) {
    case TARGET_NONE:
      break;
    case TARGET_SELF:
      aiLoveSelf(doer);
      break;
    case TARGET_MOB:
      if ((doer->getSex() == SEX_NEUTER) || (getSex() == SEX_NEUTER)) {
        act("$n holds you without emotion.", true, this, 0, doer, TO_VICT);
        act("$n emotionlessly cuddles $N.", true, this, 0, doer, TO_NOTVICT);
      } else if (doer->getSex() == getSex()) {
        if (doer->getSex() == SEX_FEMALE) {
          aiFag(doer, 1);
          US(4);
          UM(1);
          UA(1);
        } else {
          aiFag(doer, 1);
          US(3);
          UM(2);
          UA(3);
        }
      } else {
        aiMudSex(doer);
      }
      break;
    case TARGET_OTHER:
      if ((doer->getSex() == SEX_NEUTER) || (other->getSex() == SEX_NEUTER)) {
      } else if (doer->getSex() == other->getSex())
        aiFag(doer, 0);
      else
        aiMudSexOther(doer, other);
      US(2);
      break;
  }
  return false;
}

int TMonster::aiNuzzle(TBeing* doer, TBeing* other, aiTarg cond) {
  if (aiLoveNonHumanoid(doer, cond))
    return false;

  switch (cond) {
    case TARGET_NONE:
      break;
    case TARGET_SELF:
      aiLoveSelf(doer);
      break;
    case TARGET_MOB:
      if ((doer->getSex() == SEX_NEUTER) || (getSex() == SEX_NEUTER)) {
        act("$n holds you without emotion.", true, this, 0, doer, TO_VICT);
        act("$n emotionlessly hugs $N back.", true, this, 0, doer, TO_NOTVICT);
      } else if (doer->getSex() == getSex()) {
        if (doer->getSex() == SEX_FEMALE) {
          aiFag(doer, 1);
          US(2);
          if (isSusp()) {
            UM(1);
            UA(1);
          } else {
            DMal(1);
            DA(1);
          }
        } else {
          aiFag(doer, 1);
          US(3);
          UM(2);
          UA(3);
        }
      } else {
        aiMudSex(doer);
      }
      break;
    case TARGET_OTHER:
      if ((doer->getSex() == SEX_NEUTER) || (other->getSex() == SEX_NEUTER)) {
      } else if (doer->getSex() == other->getSex())
        aiFag(doer, 0);
      else
        aiMudSexOther(doer, other);
      US(2);
      break;
  }
  return false;
}

int TMonster::aiCry(TBeing* doer, TBeing*, aiTarg cond) {
  if (!isHumanoid()) {
    // do nothing
    return false;
  }
  switch (cond) {
    case TARGET_NONE:
    case TARGET_SELF:
    case TARGET_OTHER:
      DS(1);
      DA(2);
      if (!isSusp() && !isAngry()) {
        doAction(fname(doer->name), CMD_COMFORT);
        doSay("Cheer up kid.");
      } else if (!pissed())
        doSay("Dry your eyes, life will get better.");
      else
        doSay("Stop your blubbering, cry-baby.");
      break;
    case TARGET_MOB:
      if (pissed()) {
        doSay("Don't come crying to me.");
        US(3);
        UA(1);
      } else {
        doAction(fname(doer->name), CMD_COMFORT);
        doSay("There, there, it can't be all bad.");
        DS(2);
        DA(1);
      }
      break;
  }
  return false;
}

int TMonster::aiPoke(TBeing* doer, TBeing*, aiTarg cond) {
  if (!isHumanoid()) {
    if (cond != TARGET_MOB)
      return false;
    US(2);
    UA(2);
    UM(1);
    if (getRace() == RACE_CANINE)
      act("$n yips at you and seems displeased.", 0, this, 0, 0, TO_ROOM);
    else
      act("$n makes a growling noise.", 0, this, 0, 0, TO_ROOM);
    return false;
  }
  switch (cond) {
    case TARGET_NONE:
      break;
    case TARGET_SELF:
      aiStrangeThings(doer);
      break;
    case TARGET_MOB:
      US(3);
      if (::number(0, 2))
        break;
      if (!pissed()) {
        doSay("ouch");
      } else {
        doSay("Hey, stop poking me.");
        doAction(fname(doer->name), CMD_POKE);
        doSay("See?  How do you like it when someone does that?");
        UM(1);
        UA(2);
      }
    case TARGET_OTHER:
      // ignore poking others
      break;
  }
  return false;
}

int TMonster::aiAccuse(TBeing* doer, TBeing* other, aiTarg cond) {
  if (!isHumanoid()) {
    if ((cond != TARGET_MOB) || ::number(0, 2))
      return false;
    US(1);
    act("$n looks sheepish and wears a guilty look.", 0, this, 0, 0, TO_ROOM);
    return false;
  }
  switch (cond) {
    case TARGET_NONE:
      break;
    case TARGET_SELF:
      aiStrangeThings(doer);
      break;
    case TARGET_MOB:
      aiAccuseAvoid(doer);
      break;
    case TARGET_OTHER:
      if (::number(0, 1) && canSpeak()) {
        act("You point at $N and say, \"Yeah, it was $M, I saw $M do it!\"",
          false, this, 0, other, TO_CHAR);
        act("$n points at you and says, \"Yeah, it was $M, I saw $M do it!\"",
          false, this, 0, other, TO_VICT);
        act("$n points at $N and says, \"Yeah, it was $M, I saw $M do it!\"",
          false, this, 0, other, TO_NOTVICT);
      }
      break;
  }
  return false;
}

int TMonster::aiGrin(TBeing* doer, TBeing*, aiTarg cond) {
  if (!isHumanoid()) {
    US(1);
    return false;
  }
  switch (cond) {
    case TARGET_NONE:
      US(1);
      UM(1);
      if (!::number(0, 3))
        doSay("What are you grinning about?");
      break;
    case TARGET_SELF:
      UM(1);
      aiStrangeThings(doer);
      break;
    case TARGET_MOB:
      aiGrinnedAt(doer);
      break;
    case TARGET_OTHER:
      US(4);
      if (!::number(0, 2))
        doSay("Uh oh.");
      break;
  }
  return false;
}

int TMonster::aiBow(TBeing* doer, TBeing*, aiTarg cond) {
  if (!isHumanoid())
    return false;

  switch (cond) {
    case TARGET_NONE:
    case TARGET_OTHER:
      DMal(1);
      DA(3);
      DS(2);
      doAction("", CMD_SMILE);
      break;
    case TARGET_SELF:
      break;
    case TARGET_MOB:
      DMal(1);
      DA(3);
      DS(2);
      if (getSex() == SEX_FEMALE)
        doAction(fname(doer->name), CMD_CURTSEY);
      else
        doAction(fname(doer->name), CMD_BOW);
      break;
  }
  return false;
}

int TMonster::aiApplaud(TBeing* doer, TBeing* other, aiTarg cond) {
  if (!isHumanoid()) {
    aiRudeNoise(doer);
    return false;
  }
  switch (cond) {
    case TARGET_NONE:
    case TARGET_SELF:
    case TARGET_OTHER:
      DMal(2);
      DA(3);
      US(2);
      aiClapReact(doer, other);
      break;
    case TARGET_MOB:
      if (isAngry() || isSusp()) {
        doSay("Don't get sarcastic with me.");
        doAction(fname(doer->name), CMD_POKE);
      } else {
        doSay("Why thank you!");
        doAction(fname(doer->name), CMD_THANK);
        doAction(fname(doer->name), CMD_BEAM);
        DMal(3);
        DA(5);
        US(2);
      }
      break;
  }
  return false;
}

int TMonster::aiBlush(TBeing*, TBeing*, aiTarg cond) {
  if (!isHumanoid())
    return false;  // do nothing

  switch (cond) {
    case TARGET_NONE:
      if (!isSusp() && !::number(0, 2))
        doSay("There's no need to be embarrassed.");
      break;
    case TARGET_SELF:
    case TARGET_MOB:
    case TARGET_OTHER:
      break;
  }
  return false;
}

int TMonster::aiBurp(TBeing* doer, TBeing*, aiTarg cond) {
  if (!isHumanoid()) {
    return false;
  }
  switch (cond) {
    case TARGET_NONE:
    case TARGET_SELF:
    case TARGET_MOB:
    case TARGET_OTHER:
      if (::number(0, 1))
        doSay("Yummm, tasty!");
      else
        aiBadManners(doer);
      break;
  }
  return false;
}

int TMonster::aiChuckle(TBeing*, TBeing*, aiTarg cond) {
  if (!isHumanoid()) {
    return false;
  }
  switch (cond) {
    case TARGET_NONE:
    case TARGET_SELF:
    case TARGET_MOB:
    case TARGET_OTHER:
      DA(1);
      if (!::number(0, 2))
        DMal(1);
      if (!::number(0, 3) && !isAngry() && (getSex() == SEX_FEMALE))
        doAction("", CMD_GIGGLE);
      else if (!::number(0, 4))
        doAction("", CMD_SMILE);
      break;
  }
  return false;
}

int TMonster::aiClap(TBeing* doer, TBeing* other, aiTarg cond) {
  if (!isHumanoid()) {
    aiRudeNoise(doer);
    return false;
  }
  switch (cond) {
    case TARGET_NONE:
    case TARGET_SELF:
    case TARGET_OTHER:
      DMal(1);
      DA(2);
      US(1);
      aiClapReact(doer, other);
      break;
    case TARGET_MOB:
      if (isAngry() || isSusp()) {
        doSay("Hey, I don't need any more attitude from you.");
        doAction(fname(doer->name), CMD_SLAP);
      } else {
        doSay("Why, thank you!");
        if (getSex() == SEX_FEMALE)
          doAction(fname(doer->name), CMD_CURTSEY);
        else
          doAction(fname(doer->name), CMD_BOW);
        DMal(1);
        DA(2);
        US(1);
      }
      break;
  }
  return false;
}

int TMonster::aiCough(TBeing* doer, TBeing*, aiTarg cond) {
  if (!isHumanoid()) {
    aiRudeNoise(doer);
    return false;
  }
  switch (cond) {
    case TARGET_NONE:
    case TARGET_SELF:
    case TARGET_MOB:
    case TARGET_OTHER:
      if (!::number(0, 4))
        doSay("You trying to make me sick?");
      else if (!::number(0, 2))
        doSay("Cover your mouth when you do that!");
      else if (::number(0, 1))
        doSay("Don't cough near me!");
      break;
  }
  return false;
}

int TMonster::aiCurtsey(TBeing* doer, TBeing*, aiTarg cond) {
  if (!isHumanoid())
    return false;

  if (doer->getSex() != SEX_FEMALE)
    doWhisper(format("%s Pssst, usually it's the WOMEN that curtsey...") %
              fname(doer->name));

  switch (cond) {
    case TARGET_NONE:
    case TARGET_OTHER:
      DMal(1);
      DA(3);
      DS(2);
      doAction("", CMD_SMILE);
      break;
    case TARGET_SELF:
      break;
    case TARGET_MOB:
      DMal(1);
      DA(3);
      DS(2);
      if (getSex() == SEX_FEMALE)
        doAction(fname(doer->name), CMD_CURTSEY);
      else
        doAction(fname(doer->name), CMD_BOW);
      break;
  }
  return false;
}

int TMonster::aiFart(TBeing* doer, TBeing*, aiTarg cond) {
  if (!isHumanoid()) {
    return false;
  }
  switch (cond) {
    case TARGET_NONE:
    case TARGET_SELF:
    case TARGET_OTHER:
      if (::number(0, 1))
        doSay("What a sweet aroma!");
      else
        aiBadManners(doer);
      break;
    case TARGET_MOB:
      if (pissed())
        return takeFirstHit(*doer);
      else
        aiUpset(doer);
      break;
  }
  return false;
}

int TMonster::aiFlip(TBeing* doer, TBeing*, aiTarg cond) {
  if (!isHumanoid()) {
    US(2);
    aiMobShock(doer);
    return false;
  }
  switch (cond) {
    case TARGET_NONE:
      US(3);
      aiStrangeThings(doer);
    case TARGET_SELF:
    case TARGET_MOB:
    case TARGET_OTHER:
      // these aren't possible, right?
      break;
  }
  return false;
}

int TMonster::aiFondle(TBeing* doer, TBeing* other, aiTarg cond) {
  if (aiLoveNonHumanoid(doer, cond))
    return false;

  switch (cond) {
    case TARGET_NONE:
      break;  // fondle who?
    case TARGET_SELF:
      aiLoveSelf(doer);
      break;
    case TARGET_MOB:
      if (getSex() == doer->getSex())
        aiFag(doer, true);
      else {
        aiMudSex(doer);
      }
      break;
    case TARGET_OTHER:
      if (getSex() == SEX_NEUTER) {
        // who cares, i'm a eunich
      } else if (getSex() == doer->getSex()) {
        aiFag(doer, false);
      } else {
        aiMudSexOther(doer, other);
      }
      break;
  }
  return false;
}

int TMonster::aiFrown(TBeing*, TBeing*, aiTarg cond) {
  if (!isHumanoid()) {
    return false;
  }
  switch (cond) {
    case TARGET_NONE:
    case TARGET_SELF:
    case TARGET_MOB:
    case TARGET_OTHER:
      break;
  }
  return false;
}

int TMonster::aiGasp(TBeing*, TBeing*, aiTarg cond) {
  if (!isHumanoid()) {
    return false;
  }
  switch (cond) {
    case TARGET_NONE:
    case TARGET_SELF:
    case TARGET_MOB:
    case TARGET_OTHER:
      break;
  }
  return false;
}

int TMonster::aiGlare(TBeing*, TBeing*, aiTarg cond) {
  if (!isHumanoid()) {
    return false;
  }
  switch (cond) {
    case TARGET_NONE:
    case TARGET_SELF:
    case TARGET_MOB:
    case TARGET_OTHER:
      break;
  }
  return false;
}

int TMonster::aiGroan(TBeing*, TBeing*, aiTarg cond) {
  if (!isHumanoid()) {
    return false;
  }
  switch (cond) {
    case TARGET_NONE:
    case TARGET_SELF:
    case TARGET_MOB:
    case TARGET_OTHER:
      break;
  }
  return false;
}

int TMonster::aiGrope(TBeing* doer, TBeing* other, aiTarg cond) {
  if (aiLoveNonHumanoid(doer, cond))
    return false;

  switch (cond) {
    case TARGET_NONE:
      break;
    case TARGET_SELF:
      aiLoveSelf(doer);
      break;
    case TARGET_MOB:
      if (doer->getSex() == getSex()) {
        if (doer->getSex() == SEX_FEMALE) {
          aiFag(doer, 1);
          US(2);
          if (isSusp()) {
            UM(1);
            UA(1);
          } else {
            DMal(1);
            DA(1);
          }
        } else {
          aiFag(doer, 1);
          US(3);
          UM(2);
          UA(3);
        }
      } else {
        aiMudSex(doer);
      }
      break;
    case TARGET_OTHER:
      if ((doer->getSex() == SEX_NEUTER) || (getSex() == SEX_NEUTER) ||
          (other->getSex() == SEX_NEUTER)) {
        // skip
      } else if (doer->getSex() == other->getSex())
        aiFag(doer, 0);
      else
        aiMudSexOther(doer, other);
      US(2);
      break;
  }
  return false;
}

int TMonster::aiHiccup(TBeing*, TBeing*, aiTarg cond) {
  if (!isHumanoid()) {
    return false;
  }
  switch (cond) {
    case TARGET_NONE:
    case TARGET_SELF:
    case TARGET_MOB:
    case TARGET_OTHER:
      break;
  }
  return false;
}

int TMonster::aiLick(TBeing*, TBeing*, aiTarg cond) {
  if (!isHumanoid()) {
    return false;
  }
  switch (cond) {
    case TARGET_NONE:
    case TARGET_SELF:
    case TARGET_MOB:
    case TARGET_OTHER:
      break;
  }
  return false;
}

int TMonster::aiLove(TBeing* doer, TBeing* other, aiTarg cond) {
  if (aiLoveNonHumanoid(doer, cond))
    return false;

  switch (cond) {
    case TARGET_NONE:
      if (!::number(0, 3))
        doAction(fname(doer->name), CMD_SMILE);
      break;
    case TARGET_SELF:
      aiLoveSelf(doer);
      break;
    case TARGET_MOB:
      if ((doer->getSex() == SEX_NEUTER) || (getSex() == SEX_NEUTER)) {
        doAction("", CMD_BLINK);
        act("$n lacks the emotional context to understand you.", true, this, 0,
          doer, TO_VICT);
      } else if (doer->getSex() == getSex()) {
        if (doer->getSex() == SEX_FEMALE) {
          aiFag(doer, 1);
          US(2);
          if (isSusp()) {
            UM(1);
            UA(1);
          } else {
            DMal(1);
            DA(1);
          }
        } else {
          aiFag(doer, 1);
          US(3);
          UM(2);
          UA(3);
        }
      } else {
        aiMudSex(doer);
      }
      break;
    case TARGET_OTHER:
      if (getSex() == SEX_NEUTER) {
      } else if ((other->getSex() == SEX_NEUTER) ||
                 (doer->getSex() == SEX_NEUTER)) {
      } else if (doer->getSex() == other->getSex())
        aiFag(doer, 0);
      else
        aiMudSexOther(doer, other);
      US(1);
      break;
  }
  return false;
}

int TMonster::aiMoan(TBeing*, TBeing*, aiTarg cond) {
  if (!isHumanoid()) {
    return false;
  }
  switch (cond) {
    case TARGET_NONE:
    case TARGET_SELF:
    case TARGET_MOB:
    case TARGET_OTHER:
      break;
  }
  return false;
}

int TMonster::aiNibble(TBeing* doer, TBeing* other, aiTarg cond) {
  if (aiLoveNonHumanoid(doer, cond))
    return false;

  switch (cond) {
    case TARGET_NONE:
      break;  // nibble who?
    case TARGET_SELF:
      break;  // not possible
    case TARGET_MOB:
      if ((getSex() == SEX_NEUTER)) {
        if (!::number(0, 1))
          doSay("Ow!  Why are you biting me?");
        UM(1);
        US(2);
        UA(3);
      } else if (doer->getSex() == getSex()) {
        if (doer->getSex() == SEX_FEMALE) {
          aiFag(doer, 1);
          US(2);
          if (isSusp()) {
            UM(1);
            UA(1);
          } else {
            DMal(1);
            DA(1);
          }
        } else {
          aiFag(doer, 1);
          US(3);
          UM(2);
          UA(3);
        }
      } else {
        aiMudSex(doer);
      }
      break;
    case TARGET_OTHER:
      if (getSex() == SEX_NEUTER) {
      } else if ((other->getSex() == SEX_NEUTER) ||
                 (doer->getSex() == SEX_NEUTER)) {
      } else if (doer->getSex() == other->getSex())
        aiFag(doer, 0);
      else
        aiMudSexOther(doer, other);
      US(2);
      break;
  }
  return false;
}

int TMonster::aiPout(TBeing*, TBeing*, aiTarg cond) {
  if (!isHumanoid()) {
    return false;
  }
  switch (cond) {
    case TARGET_NONE:
    case TARGET_SELF:
    case TARGET_MOB:
    case TARGET_OTHER:
      break;
  }
  return false;
}

int TMonster::aiPurr(TBeing*, TBeing*, aiTarg cond) {
  if (!isHumanoid()) {
    return false;
  }
  switch (cond) {
    case TARGET_NONE:
    case TARGET_SELF:
    case TARGET_MOB:
    case TARGET_OTHER:
      break;
  }
  return false;
}

int TMonster::aiRuffle(TBeing*, TBeing*, aiTarg cond) {
  if (!isHumanoid()) {
    return false;
  }
  switch (cond) {
    case TARGET_NONE:
    case TARGET_SELF:
    case TARGET_MOB:
    case TARGET_OTHER:
      break;
  }
  return false;
}

int TMonster::aiShiver(TBeing*, TBeing*, aiTarg cond) {
  if (!isHumanoid()) {
    return false;
  }
  switch (cond) {
    case TARGET_NONE:
    case TARGET_SELF:
    case TARGET_MOB:
    case TARGET_OTHER:
      break;
  }
  return false;
}

int TMonster::aiShrug(TBeing*, TBeing*, aiTarg cond) {
  if (!isHumanoid()) {
    return false;
  }
  switch (cond) {
    case TARGET_NONE:
    case TARGET_SELF:
    case TARGET_MOB:
    case TARGET_OTHER:
      break;
  }
  return false;
}

int TMonster::aiSing(TBeing*, TBeing*, aiTarg cond) {
  if (!isHumanoid()) {
    return false;
  }
  switch (cond) {
    case TARGET_NONE:
    case TARGET_SELF:
    case TARGET_MOB:
    case TARGET_OTHER:
      break;
  }
  return false;
}

int TMonster::aiSlap(TBeing*, TBeing*, aiTarg cond) {
  if (!isHumanoid()) {
    return false;
  }
  switch (cond) {
    case TARGET_NONE:
    case TARGET_SELF:
    case TARGET_MOB:
    case TARGET_OTHER:
      break;
  }
  return false;
}

int TMonster::aiSmirk(TBeing*, TBeing*, aiTarg cond) {
  if (!isHumanoid()) {
    return false;
  }
  switch (cond) {
    case TARGET_NONE:
    case TARGET_SELF:
    case TARGET_MOB:
    case TARGET_OTHER:
      break;
  }
  return false;
}

int TMonster::aiSnap(TBeing*, TBeing*, aiTarg cond) {
  if (!isHumanoid()) {
    return false;
  }
  switch (cond) {
    case TARGET_NONE:
    case TARGET_SELF:
    case TARGET_MOB:
    case TARGET_OTHER:
      break;
  }
  return false;
}

int TMonster::aiSneeze(TBeing*, TBeing*, aiTarg cond) {
  if (!isHumanoid()) {
    return false;
  }
  switch (cond) {
    case TARGET_NONE:
    case TARGET_SELF:
    case TARGET_MOB:
    case TARGET_OTHER:
      break;
  }
  return false;
}

int TMonster::aiSnicker(TBeing*, TBeing*, aiTarg cond) {
  if (!isHumanoid()) {
    return false;
  }
  switch (cond) {
    case TARGET_NONE:
    case TARGET_SELF:
    case TARGET_MOB:
    case TARGET_OTHER:
      break;
  }
  return false;
}

int TMonster::aiSniff(TBeing*, TBeing*, aiTarg cond) {
  if (!isHumanoid()) {
    return false;
  }
  switch (cond) {
    case TARGET_NONE:
    case TARGET_SELF:
    case TARGET_MOB:
    case TARGET_OTHER:
      break;
  }
  return false;
}

int TMonster::aiSnore(TBeing*, TBeing*, aiTarg cond) {
  if (!isHumanoid()) {
    return false;
  }
  switch (cond) {
    case TARGET_NONE:
    case TARGET_SELF:
    case TARGET_MOB:
    case TARGET_OTHER:
      break;
  }
  return false;
}

int TMonster::aiSpit(TBeing*, TBeing*, aiTarg cond) {
  if (!isHumanoid()) {
    return false;
  }
  switch (cond) {
    case TARGET_NONE:
    case TARGET_SELF:
    case TARGET_MOB:
    case TARGET_OTHER:
      break;
  }
  return false;
}

int TMonster::aiSqueeze(TBeing* doer, TBeing* other, aiTarg cond) {
  if (aiLoveNonHumanoid(doer, cond))
    return false;

  switch (cond) {
    case TARGET_NONE:
      break;
    case TARGET_SELF:
      aiLoveSelf(doer);
      break;
    case TARGET_MOB:
      if ((doer->getSex() == SEX_NEUTER) || (getSex() == SEX_NEUTER)) {
      } else if (doer->getSex() == getSex()) {
        if (doer->getSex() == SEX_FEMALE) {
          aiFag(doer, 1);
          US(2);
          if (isSusp()) {
            UM(1);
            UA(1);
          } else {
            DMal(1);
            DA(1);
          }
        } else {
          aiFag(doer, 1);
        }
      } else {
        aiMudSex(doer);
      }
      break;
    case TARGET_OTHER:
      if (getSex() == SEX_NEUTER) {
      } else if ((other->getSex() == SEX_NEUTER) ||
                 (doer->getSex() == SEX_NEUTER)) {
      } else if (doer->getSex() == other->getSex())
        aiFag(doer, 0);
      else
        aiMudSexOther(doer, other);
      US(2);
      break;
  }
  return false;
}

int TMonster::aiStare(TBeing*, TBeing*, aiTarg cond) {
  if (!isHumanoid()) {
    return false;
  }
  switch (cond) {
    case TARGET_NONE:
    case TARGET_SELF:
    case TARGET_MOB:
    case TARGET_OTHER:
      break;
  }
  return false;
}

int TMonster::aiStrut(TBeing*, TBeing*, aiTarg cond) {
  if (!isHumanoid()) {
    return false;
  }
  switch (cond) {
    case TARGET_NONE:
    case TARGET_SELF:
    case TARGET_MOB:
    case TARGET_OTHER:
      break;
  }
  return false;
}

int TMonster::aiThank(TBeing*, TBeing*, aiTarg cond) {
  if (!isHumanoid()) {
    return false;
  }
  switch (cond) {
    case TARGET_NONE:
    case TARGET_SELF:
    case TARGET_MOB:
    case TARGET_OTHER:
      break;
  }
  return false;
}

int TMonster::aiTwiddle(TBeing*, TBeing*, aiTarg cond) {
  if (!isHumanoid()) {
    return false;
  }
  switch (cond) {
    case TARGET_NONE:
    case TARGET_SELF:
    case TARGET_MOB:
    case TARGET_OTHER:
      break;
  }
  return false;
}

int TMonster::aiWave(TBeing*, TBeing*, aiTarg cond) {
  if (!isHumanoid()) {
    return false;
  }
  switch (cond) {
    case TARGET_NONE:
    case TARGET_SELF:
    case TARGET_MOB:
    case TARGET_OTHER:
      break;
  }
  return false;
}

int TMonster::aiWhistle(TBeing*, TBeing*, aiTarg cond) {
  if (!isHumanoid()) {
    return false;
  }
  switch (cond) {
    case TARGET_NONE:
    case TARGET_SELF:
    case TARGET_MOB:
    case TARGET_OTHER:
      break;
  }
  return false;
}

int TMonster::aiWiggle(TBeing*, TBeing*, aiTarg cond) {
  if (!isHumanoid()) {
    return false;
  }
  switch (cond) {
    case TARGET_NONE:
    case TARGET_SELF:
    case TARGET_MOB:
    case TARGET_OTHER:
      break;
  }
  return false;
}

int TMonster::aiWink(TBeing*, TBeing*, aiTarg cond) {
  if (!isHumanoid()) {
    return false;
  }
  switch (cond) {
    case TARGET_NONE:
    case TARGET_SELF:
    case TARGET_MOB:
    case TARGET_OTHER:
      break;
  }
  return false;
}

int TMonster::aiYawn(TBeing*, TBeing*, aiTarg cond) {
  if (!isHumanoid()) {
    return false;
  }
  switch (cond) {
    case TARGET_NONE:
    case TARGET_SELF:
    case TARGET_MOB:
    case TARGET_OTHER:
      break;
  }
  return false;
}

int TMonster::aiSnowball(TBeing*, TBeing*, aiTarg cond) {
  if (!isHumanoid()) {
    return false;
  }
  switch (cond) {
    case TARGET_NONE:
    case TARGET_SELF:
    case TARGET_MOB:
    case TARGET_OTHER:
      break;
  }
  return false;
}

int TMonster::aiFrench(TBeing* doer, TBeing* other, aiTarg cond) {
  return aiKiss(doer, other, cond);
}

int TMonster::aiComb(TBeing*, TBeing*, aiTarg cond) {
  if (!isHumanoid()) {
    return false;
  }
  switch (cond) {
    case TARGET_NONE:
    case TARGET_SELF:
    case TARGET_MOB:
    case TARGET_OTHER:
      break;
  }
  return false;
}

int TMonster::aiMassage(TBeing* doer, TBeing* other, aiTarg cond) {
  if (aiLoveNonHumanoid(doer, cond))
    return false;

  switch (cond) {
    case TARGET_NONE:
      break;
    case TARGET_SELF:
      break;
    case TARGET_MOB:
      if ((doer->getSex() == SEX_NEUTER) || (getSex() == SEX_NEUTER)) {
      } else if (doer->getSex() == getSex()) {
        if (doer->getSex() == SEX_FEMALE) {
          aiFag(doer, 1);
          US(2);
          if (isSusp()) {
            UM(1);
            UA(1);
          } else {
            DMal(1);
            DA(1);
          }
        } else {
          aiFag(doer, 1);
          US(3);
          UM(2);
          UA(3);
        }
      } else {
        aiMudSex(doer);
      }
      break;
    case TARGET_OTHER:
      if (getSex() == SEX_NEUTER) {
      } else if ((other->getSex() == SEX_NEUTER) ||
                 (doer->getSex() == SEX_NEUTER)) {
      } else if (doer->getSex() == other->getSex())
        aiFag(doer, 0);
      else
        aiMudSexOther(doer, other);
      US(2);
      break;
  }
  return false;
}

int TMonster::aiTickle(TBeing*, TBeing*, aiTarg cond) {
  if (!isHumanoid()) {
    return false;
  }
  switch (cond) {
    case TARGET_NONE:
    case TARGET_SELF:
    case TARGET_MOB:
    case TARGET_OTHER:
      break;
  }
  return false;
}

int TMonster::aiPat(TBeing*, TBeing*, aiTarg cond) {
  if (!isHumanoid()) {
    return false;
  }
  switch (cond) {
    case TARGET_NONE:
    case TARGET_SELF:
    case TARGET_MOB:
    case TARGET_OTHER:
      break;
  }
  return false;
}

int TMonster::aiCurse(TBeing*, TBeing*, aiTarg cond) {
  if (!isHumanoid()) {
    return false;
  }
  switch (cond) {
    case TARGET_NONE:
    case TARGET_SELF:
    case TARGET_MOB:
    case TARGET_OTHER:
      break;
  }
  return false;
}

int TMonster::aiBeg(TBeing*, TBeing*, aiTarg cond) {
  if (!isHumanoid()) {
    return false;
  }
  switch (cond) {
    case TARGET_NONE:
    case TARGET_SELF:
    case TARGET_MOB:
    case TARGET_OTHER:
      break;
  }
  return false;
}

int TMonster::aiBleed(TBeing*, TBeing*, aiTarg cond) {
  if (!isHumanoid()) {
    return false;
  }
  switch (cond) {
    case TARGET_NONE:
    case TARGET_SELF:
    case TARGET_MOB:
    case TARGET_OTHER:
      break;
  }
  return false;
}

int TMonster::aiCringe(TBeing*, TBeing*, aiTarg cond) {
  if (!isHumanoid()) {
    return false;
  }
  switch (cond) {
    case TARGET_NONE:
    case TARGET_SELF:
    case TARGET_MOB:
    case TARGET_OTHER:
      break;
  }
  return false;
}

int TMonster::aiDaydream(TBeing*, TBeing*, aiTarg cond) {
  if (!isHumanoid()) {
    return false;
  }
  switch (cond) {
    case TARGET_NONE:
    case TARGET_SELF:
    case TARGET_MOB:
    case TARGET_OTHER:
      break;
  }
  return false;
}

int TMonster::aiFume(TBeing*, TBeing*, aiTarg cond) {
  if (!isHumanoid()) {
    return false;
  }
  switch (cond) {
    case TARGET_NONE:
    case TARGET_SELF:
    case TARGET_MOB:
    case TARGET_OTHER:
      break;
  }
  return false;
}

int TMonster::aiGrovel(TBeing*, TBeing*, aiTarg cond) {
  if (!isHumanoid()) {
    return false;
  }
  switch (cond) {
    case TARGET_NONE:
    case TARGET_SELF:
    case TARGET_MOB:
    case TARGET_OTHER:
      break;
  }
  return false;
}

int TMonster::aiHop(TBeing*, TBeing*, aiTarg cond) {
  if (!isHumanoid()) {
    return false;
  }
  switch (cond) {
    case TARGET_NONE:
    case TARGET_SELF:
    case TARGET_MOB:
    case TARGET_OTHER:
      break;
  }
  return false;
}

int TMonster::aiNudge(TBeing*, TBeing*, aiTarg cond) {
  if (!isHumanoid()) {
    return false;
  }
  switch (cond) {
    case TARGET_NONE:
    case TARGET_SELF:
    case TARGET_MOB:
    case TARGET_OTHER:
      break;
  }
  return false;
}
int TMonster::aiPeer(TBeing*, TBeing*, aiTarg cond) {
  if (!isHumanoid()) {
    return false;
  }
  switch (cond) {
    case TARGET_NONE:
    case TARGET_SELF:
    case TARGET_MOB:
    case TARGET_OTHER:
      break;
  }
  return false;
}

int TMonster::aiPoint(TBeing*, TBeing*, aiTarg cond) {
  if (!isHumanoid()) {
    return false;
  }
  switch (cond) {
    case TARGET_NONE:
    case TARGET_SELF:
    case TARGET_MOB:
    case TARGET_OTHER:
      break;
  }
  return false;
}

int TMonster::aiPonder(TBeing*, TBeing*, aiTarg cond) {
  if (!isHumanoid()) {
    return false;
  }
  switch (cond) {
    case TARGET_NONE:
    case TARGET_SELF:
    case TARGET_MOB:
    case TARGET_OTHER:
      break;
  }
  return false;
}

int TMonster::aiPunch(TBeing*, TBeing*, aiTarg cond) {
  if (!isHumanoid()) {
    return false;
  }
  switch (cond) {
    case TARGET_NONE:
    case TARGET_SELF:
    case TARGET_MOB:
    case TARGET_OTHER:
      break;
  }
  return false;
}

int TMonster::aiSnarl(TBeing*, TBeing*, aiTarg cond) {
  if (!isHumanoid()) {
    return false;
  }
  switch (cond) {
    case TARGET_NONE:
    case TARGET_SELF:
    case TARGET_MOB:
    case TARGET_OTHER:
      break;
  }
  return false;
}

int TMonster::aiSpank(TBeing*, TBeing*, aiTarg cond) {
  if (!isHumanoid()) {
    return false;
  }
  switch (cond) {
    case TARGET_NONE:
    case TARGET_SELF:
    case TARGET_MOB:
    case TARGET_OTHER:
      break;
  }
  return false;
}

int TMonster::aiSteam(TBeing*, TBeing*, aiTarg cond) {
  if (!isHumanoid()) {
    return false;
  }
  switch (cond) {
    case TARGET_NONE:
    case TARGET_SELF:
    case TARGET_MOB:
    case TARGET_OTHER:
      break;
  }
  return false;
}

int TMonster::aiTackle(TBeing*, TBeing*, aiTarg cond) {
  if (!isHumanoid()) {
    return false;
  }
  switch (cond) {
    case TARGET_NONE:
    case TARGET_SELF:
    case TARGET_MOB:
    case TARGET_OTHER:
      break;
  }
  return false;
}

int TMonster::aiTaunt(TBeing*, TBeing*, aiTarg cond) {
  if (!isHumanoid()) {
    return false;
  }
  switch (cond) {
    case TARGET_NONE:
    case TARGET_SELF:
    case TARGET_MOB:
    case TARGET_OTHER:
      break;
  }
  return false;
}

int TMonster::aiWhine(TBeing*, TBeing*, aiTarg cond) {
  if (!isHumanoid()) {
    return false;
  }
  switch (cond) {
    case TARGET_NONE:
    case TARGET_SELF:
    case TARGET_MOB:
    case TARGET_OTHER:
      break;
  }
  return false;
}

int TMonster::aiWorship(TBeing*, TBeing*, aiTarg cond) {
  if (!isHumanoid()) {
    return false;
  }
  switch (cond) {
    case TARGET_NONE:
    case TARGET_SELF:
    case TARGET_MOB:
    case TARGET_OTHER:
      break;
  }
  return false;
}

int TMonster::aiYodel(TBeing*, TBeing*, aiTarg cond) {
  if (!isHumanoid()) {
    return false;
  }
  switch (cond) {
    case TARGET_NONE:
    case TARGET_SELF:
    case TARGET_MOB:
    case TARGET_OTHER:
      break;
  }
  return false;
}

int TMonster::aiThink(TBeing*, TBeing*, aiTarg cond) {
  if (!isHumanoid()) {
    return false;
  }
  switch (cond) {
    case TARGET_NONE:
    case TARGET_SELF:
    case TARGET_MOB:
    case TARGET_OTHER:
      break;
  }
  return false;
}

int TMonster::aiWhap(TBeing*, TBeing*, aiTarg cond) {
  if (!isHumanoid()) {
    return false;
  }
  switch (cond) {
    case TARGET_NONE:
    case TARGET_SELF:
    case TARGET_MOB:
    case TARGET_OTHER:
      break;
  }
  return false;
}

int TMonster::aiBeam(TBeing*, TBeing*, aiTarg cond) {
  if (!isHumanoid()) {
    return false;
  }
  switch (cond) {
    case TARGET_NONE:
    case TARGET_SELF:
    case TARGET_MOB:
    case TARGET_OTHER:
      break;
  }
  return false;
}

int TMonster::aiChortle(TBeing*, TBeing*, aiTarg cond) {
  if (!isHumanoid()) {
    return false;
  }
  switch (cond) {
    case TARGET_NONE:
    case TARGET_SELF:
    case TARGET_MOB:
    case TARGET_OTHER:
      break;
  }
  return false;
}

int TMonster::aiBonk(TBeing*, TBeing*, aiTarg cond) {
  if (!isHumanoid()) {
    return false;
  }
  switch (cond) {
    case TARGET_NONE:
    case TARGET_SELF:
    case TARGET_MOB:
    case TARGET_OTHER:
      break;
  }
  return false;
}

int TMonster::aiScold(TBeing*, TBeing*, aiTarg cond) {
  if (!isHumanoid()) {
    return false;
  }
  switch (cond) {
    case TARGET_NONE:
    case TARGET_SELF:
    case TARGET_MOB:
    case TARGET_OTHER:
      break;
  }
  return false;
}

int TMonster::aiDrool(TBeing*, TBeing*, aiTarg cond) {
  if (!isHumanoid()) {
    return false;
  }
  switch (cond) {
    case TARGET_NONE:
    case TARGET_SELF:
    case TARGET_MOB:
    case TARGET_OTHER:
      break;
  }
  return false;
}

int TMonster::aiRip(TBeing*, TBeing*, aiTarg cond) {
  if (!isHumanoid()) {
    return false;
  }
  switch (cond) {
    case TARGET_NONE:
    case TARGET_SELF:
    case TARGET_MOB:
    case TARGET_OTHER:
      break;
  }
  return false;
}

int TMonster::aiStretch(TBeing*, TBeing*, aiTarg cond) {
  if (!isHumanoid()) {
    return false;
  }
  switch (cond) {
    case TARGET_NONE:
    case TARGET_SELF:
    case TARGET_MOB:
    case TARGET_OTHER:
      break;
  }
  return false;
}

int TMonster::aiPimp(TBeing*, TBeing*, aiTarg cond) {
  if (!isHumanoid()) {
    return false;
  }
  switch (cond) {
    case TARGET_NONE:
    case TARGET_SELF:
    case TARGET_MOB:
    case TARGET_OTHER:
      break;
  }
  return false;
}

int TMonster::aiBelittle(TBeing*, TBeing*, aiTarg cond) {
  if (!isHumanoid()) {
    return false;
  }
  switch (cond) {
    case TARGET_NONE:
    case TARGET_SELF:
    case TARGET_MOB:
    case TARGET_OTHER:
      break;
  }
  return false;
}

int TMonster::aiPiledrive(TBeing*, TBeing*, aiTarg cond) {
  if (!isHumanoid()) {
    return false;
  }
  switch (cond) {
    case TARGET_NONE:
    case TARGET_SELF:
    case TARGET_MOB:
    case TARGET_OTHER:
      break;
  }
  return false;
}

int TMonster::aiTap(TBeing*, TBeing*, aiTarg cond) {
  if (!isHumanoid()) {
    return false;
  }
  switch (cond) {
    case TARGET_NONE:
    case TARGET_SELF:
    case TARGET_MOB:
    case TARGET_OTHER:
      break;
  }
  return false;
}

int TMonster::aiFlipoff(TBeing*, TBeing*, aiTarg cond) {
  if (!isHumanoid()) {
    return false;
  }
  switch (cond) {
    case TARGET_NONE:
    case TARGET_SELF:
    case TARGET_MOB:
    case TARGET_OTHER:
      break;
  }
  return false;
}
int TMonster::aiMoon(TBeing*, TBeing*, aiTarg cond) {
  if (!isHumanoid()) {
    return false;
  }
  switch (cond) {
    case TARGET_NONE:
    case TARGET_SELF:
    case TARGET_MOB:
    case TARGET_OTHER:
      break;
  }
  return false;
}

int TMonster::aiPinch(TBeing*, TBeing*, aiTarg cond) {
  if (!isHumanoid()) {
    return false;
  }
  switch (cond) {
    case TARGET_NONE:
    case TARGET_SELF:
    case TARGET_MOB:
    case TARGET_OTHER:
      break;
  }
  return false;
}

int TMonster::aiBite(TBeing*, TBeing*, aiTarg cond) {
  if (!isHumanoid()) {
    return false;
  }
  switch (cond) {
    case TARGET_NONE:
    case TARGET_SELF:
    case TARGET_MOB:
    case TARGET_OTHER:
      break;
  }
  return false;
}

int TMonster::aiKiss(TBeing* doer, TBeing* other, aiTarg cond) {
  if (aiLoveNonHumanoid(doer, cond)) {
    if (cond != TARGET_MOB)
      return false;

    switch (getMaterial(WEAR_HEAD)) {
      case MAT_FUR_DOG:
      case MAT_FUR_CAT:
      case MAT_FUR_RABBIT:
      case MAT_FUR:
      case MAT_WOOL:
        act("You spit up some fur and hair.  YUCK!", true, doer, 0, 0, TO_CHAR);
        act("$n spits out some fur and hair.  Gross!", true, doer, 0, 0,
          TO_ROOM);
        break;
      default:
        break;
    }
    return false;
  }
  switch (cond) {
    case TARGET_NONE:
      break;
    case TARGET_SELF:
      break;
    case TARGET_MOB:
      if ((doer->getSex() == SEX_NEUTER) || (getSex() == SEX_NEUTER)) {
      } else if (doer->getSex() == getSex()) {
        if (doer->getSex() == SEX_FEMALE) {
          aiFag(doer, 1);
          US(2);
          if (isSusp()) {
            UM(1);
            UA(1);
          } else {
            DMal(1);
            DA(1);
          }
        } else {
          aiFag(doer, 1);
          US(3);
          UM(2);
          UA(3);
        }
      } else {
        aiMudSex(doer);
      }
      break;
    case TARGET_OTHER:
      if (getSex() == SEX_NEUTER) {
      } else if ((other->getSex() == SEX_NEUTER) ||
                 (doer->getSex() == SEX_NEUTER)) {
      } else if (doer->getSex() == other->getSex())
        aiFag(doer, 0);
      else
        aiMudSexOther(doer, other);
      US(2);
      break;
  }
  return false;
}

int TMonster::aiCheer(TBeing*, TBeing*, aiTarg cond) {
  if (!isHumanoid()) {
    return false;
  }
  switch (cond) {
    case TARGET_NONE:
    case TARGET_SELF:
    case TARGET_MOB:
    case TARGET_OTHER:
      break;
  }
  return false;
}

int TMonster::aiWoo(TBeing*, TBeing*, aiTarg cond) {
  if (!isHumanoid()) {
    return false;
  }
  switch (cond) {
    case TARGET_NONE:
    case TARGET_SELF:
    case TARGET_MOB:
    case TARGET_OTHER:
      break;
  }
  return false;
}

int TMonster::aiGrumble(TBeing*, TBeing*, aiTarg cond) {
  if (!isHumanoid()) {
    return false;
  }
  switch (cond) {
    case TARGET_NONE:
    case TARGET_SELF:
    case TARGET_MOB:
    case TARGET_OTHER:
      break;
  }
  return false;
}

int TMonster::aiApologize(TBeing*, TBeing*, aiTarg cond) {
  if (!isHumanoid()) {
    return false;
  }
  switch (cond) {
    case TARGET_NONE:
    case TARGET_SELF:
    case TARGET_MOB:
    case TARGET_OTHER:
      break;
  }
  return false;
}

int TMonster::aiAgree(TBeing*, TBeing*, aiTarg cond) {
  if (!isHumanoid()) {
    return false;
  }
  switch (cond) {
    case TARGET_NONE:
    case TARGET_SELF:
    case TARGET_MOB:
    case TARGET_OTHER:
      break;
  }
  return false;
}

int TMonster::aiDisagree(TBeing*, TBeing*, aiTarg cond) {
  if (!isHumanoid()) {
    return false;
  }
  switch (cond) {
    case TARGET_NONE:
    case TARGET_SELF:
    case TARGET_MOB:
    case TARGET_OTHER:
      break;
  }
  return false;
}

int TMonster::aiSpam(TBeing* doer, TBeing*, aiTarg cond) {
  if (!isHumanoid()) {
    doAction("", CMD_SNARL);
    UM(1);
    UA(2);
    return false;
  }
  switch (cond) {
    case TARGET_NONE:
    case TARGET_SELF:
    case TARGET_OTHER:
      UM(1);
      UA(2);
      if (!::number(0, 1))
        doSay("Quit it man, that stuff is annoying.");
      break;
    case TARGET_MOB:
      UM(2);
      UA(3);
      if (!pissed())
        doSay("Don't piss me off, you won't like me when I'm mad.");
      else
        aiInsultDoer(doer);
      break;
  }
  return false;
}

int TMonster::aiRaise(TBeing*, TBeing*, aiTarg cond) {
  // do_nothing
  if (!isHumanoid()) {
    return false;
  }
  switch (cond) {
    case TARGET_NONE:
    case TARGET_SELF:
    case TARGET_MOB:
    case TARGET_OTHER:
      break;
  }
  return false;
}

int TMonster::aiRoll(TBeing*, TBeing*, aiTarg cond) {
  if (!isHumanoid()) {
    return false;
  }
  switch (cond) {
    case TARGET_NONE:
    case TARGET_SELF:
    case TARGET_OTHER:
      break;
    case TARGET_MOB:
      if (!::number(0, 2)) {
        doSay("Don't roll your eyes at me!");
        UA(1);
        UM(1);
      }
      break;
  }
  return false;
}

int TMonster::aiBlink(TBeing*, TBeing*, aiTarg cond) {
  // gotta assume this is a do_nothing social
  if (!isHumanoid()) {
    return false;
  }
  switch (cond) {
    case TARGET_NONE:
    case TARGET_SELF:
    case TARGET_MOB:
    case TARGET_OTHER:
      break;
  }
  return false;
}

void TBeing::aiWear(TObj* obj) {
  int num;
  TThing* tmp = nullptr;

  if (!obj) {
    vlogf(LOG_MOB_AI, "AI_wear called with bad declarations.");
    return;
  }
  if (inRoom() == Room::NOWHERE)
    return;

  for (StuffIter it = roomp->stuff.begin();
       it != roomp->stuff.end() && (tmp = *it); ++it) {
    TMonster* tbc = dynamic_cast<TMonster*>(tmp);
    if (!tbc)
      continue;
    if (!tbc->isPc() && tbc->canSee(this) && (tbc != this) &&
        tbc->isHumanoid() && tbc->awake() && isPc() && !UtilMobProc(tbc)) {
      num = (1 + obj->obj_flags.cost / 10000);
      tbc->UG(num);
      tbc->aiTarget(this);
    }
  }
}

void TBeing::aiGet(TThing* obj) {
  int num;
  TThing* tmp = nullptr;

  if (!obj) {
    vlogf(LOG_MOB_AI, "AI_wear called with bad declarations.");
    return;
  }
  if (!roomp) {
    vlogf(LOG_MOB, format("%s without a roomp in aiGet") % getName());
    return;
  }

  for (StuffIter it = roomp->stuff.begin();
       it != roomp->stuff.end() && (tmp = *it); ++it) {
    TMonster* tmons = dynamic_cast<TMonster*>(tmp);
    if (!tmons)
      continue;
    if (!tmons->isPc() && tmons->canSee(this) && (tmons != this) &&
        tmons->isHumanoid() && tmons->awake() && isPc() &&
        !UtilMobProc(tmons)) {
      num = 1;

      TObj* tob = dynamic_cast<TObj*>(obj);
      if (tob)
        num += tob->obj_flags.cost / 10000;
      tmons->UG(num);
      tmons->aiTarget(this);
    }
  }
}

void TMonster::aiLook(TBeing* doer) {
  TThing* t;
  sstring buf;

  if (UtilMobProc(this))
    return;

  aiTarget(doer);
  for (StuffIter it = doer->roomp->stuff.begin();
       it != doer->roomp->stuff.end();) {
    t = *(it++);
    TMonster* tmons = dynamic_cast<TMonster*>(t);
    if (!tmons || !tmons->canSee(doer) || !tmons->awake())
      continue;

    tmons->aiTarget(doer);
    tmons->US(2);
    if (tmons == this) {
      US(3);
      if (doer->isRealUgly()) {
        if (getRace() == doer->getRace())
          buf = "Stop looking at me ugly!";
        else
          buf = format("Stop looking at me you ugly %s.") %
                doer->getMyRace()->getSingularName();
        if (!::number(0, 3))
          doSay(buf);
        UA(4);
        continue;
      }
      if (!isHumanoid()) {
        continue;
      }
      if (isAngry() && !::number(0, 2)) {
        UM(1);
        UA(2);
        US(1);
        switch (::number(1, 2)) {
          case 1:
            doAction(fname(doer->name), CMD_GROWL);
            doSay("What are you looking at?");
            break;
          case 2:
            doSay("Bring it on!");
            doAction(fname(doer->name), CMD_GRIN);
            break;
        }
      } else if (isSusp() && !::number(0, 2)) {
        UM(1);
        UA(3);
        US(3);
        switch (::number(1, 2)) {
          case 1:
            doAction(fname(doer->name), CMD_POKE);
            doSay("Don't even think about it.");
            break;
          case 2:
            doAction(fname(doer->name), CMD_GLARE);
            break;
        }
      } else if (getSex() == SEX_NEUTER || doer->getSex() == SEX_NEUTER) {
        continue;
      } else if (doer->isUgly()) {
        if (!::number(0, 2))
          doAction(fname(doer->name), CMD_SMILE);
        DA(1);
      } else if (getSex() == doer->getSex()) {
        if (getSex() == SEX_MALE) {
          if (!::number(0, 15)) {
            act("$n notices you checking $m out.", true, this, 0, doer,
              TO_VICT);
#if 0
            act("You turn away before $e thinks you're queer.",true, this, 0, doer,TO_VICT);
#endif
          } else if (!::number(0, 2))
            doSay("Hi there.");
        } else {
          if (!::number(0, 15)) {
#if 0
            act("$n looks back at you.  I think $e's worried you might be queer.",true,this,0,doer,TO_VICT);
#endif
          } else if (::number(0, 1))
            doSay("Hi there.");
        }
        US(1);
        UA(1);
        UM(1);
      } else {
        if (doer->getSex() == SEX_MALE) {
          if (!::number(0, 5)) {
            act("$n notices you checking $m out and $e smiles back.", true,
              this, 0, doer, TO_VICT);
          }
        } else {
          if (!::number(0, 5)) {
            act("$n sees you looking at $m and gives you a little wink.", true,
              this, 0, doer, TO_VICT);
          }
        }
        if (!::number(0, 1))
          DMal(1);
        DA(1);
      }
    }
  }
}

int TMonster::aiSay(TBeing* doer, char*) {
  if (!doer->isPc())
    return false;
  if (isPc())
    return false;
  if (UtilMobProc(this))
    return false;
  if (!awake() || !canSee(doer))
    return false;
  aiTarget(doer);

  return true;
}

// returns DELETE_THIS, or DELETE_VICT (thief)
int TMonster::aiSteal(TBeing* thief) {
  sstring buf;

  if (isPc())
    return false;

  US(25);
  UM(38);
  UA(38);
  if (!pissed() && !isPet(PETTYPE_PET | PETTYPE_CHARM | PETTYPE_THRALL) &&
      canSee(thief, INFRA_YES) && thief->isPc()) {
    if (!hasClass(CLASS_THIEF)) {
      buf = format("%s is a bloody thief.") % thief->getName();
      doShout(buf);
      CallForGuard(this, thief, 2);
    } else {
      buf = format("Alright %s, you little punk, nice try but no dice.") %
            thief->getName();
      doSay(buf);
    }
  } else
    return (takeFirstHit(*thief));

  return false;
}

int TMonster::aiTug(TBeing*, TBeing*, aiTarg cond) {
  if (!isHumanoid()) {
    return false;
  }
  switch (cond) {
    case TARGET_NONE:
    case TARGET_SELF:
    case TARGET_MOB:
    case TARGET_OTHER:
      break;
  }
  return false;
}
int TMonster::aiCross(TBeing*, TBeing*, aiTarg cond) {
  if (!isHumanoid()) {
    return false;
  }
  switch (cond) {
    case TARGET_NONE:
    case TARGET_SELF:
    case TARGET_MOB:
    case TARGET_OTHER:
      break;
  }
  return false;
}

int TMonster::aiSneer(TBeing*, TBeing*, aiTarg cond) {
  if (!isHumanoid()) {
    return false;
  }
  switch (cond) {
    case TARGET_NONE:
    case TARGET_SELF:
    case TARGET_MOB:
    case TARGET_OTHER:
      break;
  }
  return false;
}
int TMonster::aiPet(TBeing*, TBeing*, aiTarg cond) {
  if (!isHumanoid()) {
    return false;
  }
  switch (cond) {
    case TARGET_NONE:
    case TARGET_SELF:
    case TARGET_MOB:
    case TARGET_OTHER:
      break;
  }
  return false;
}

int TMonster::aiDuck(TBeing*, TBeing*, aiTarg cond) {
  if (!isHumanoid()) {
    return false;
  }
  switch (cond) {
    case TARGET_NONE:
    case TARGET_SELF:
    case TARGET_MOB:
    case TARGET_OTHER:
      break;
  }
  return false;
}

int TMonster::aiBeckon(TBeing*, TBeing*, aiTarg cond) {
  if (!isHumanoid()) {
    return false;
  }
  switch (cond) {
    case TARGET_NONE:
    case TARGET_SELF:
    case TARGET_MOB:
    case TARGET_OTHER:
      break;
  }
  return false;
}

int TMonster::aiSalute(TBeing*, TBeing*, aiTarg cond) {
  if (!isHumanoid()) {
    return false;
  }
  switch (cond) {
    case TARGET_NONE:
    case TARGET_SELF:
    case TARGET_MOB:
    case TARGET_OTHER:
      break;
  }
  return false;
}

int TMonster::aiMoo(TBeing*, TBeing*, aiTarg cond) {
  if (!isHumanoid()) {
    return false;
  }
  switch (cond) {
    case TARGET_NONE:
    case TARGET_SELF:
    case TARGET_MOB:
    case TARGET_OTHER:
      break;
  }
  return false;
}

int TMonster::aiFaint(TBeing*, TBeing*, aiTarg cond) {
  if (!isHumanoid()) {
    return false;
  }
  switch (cond) {
    case TARGET_NONE:
    case TARGET_SELF:
    case TARGET_MOB:
    case TARGET_OTHER:
      break;
  }
  return false;
}

int TMonster::aiSnort(TBeing*, TBeing*, aiTarg cond) {
  if (!isHumanoid()) {
    return false;
  }
  switch (cond) {
    case TARGET_NONE:
    case TARGET_SELF:
    case TARGET_MOB:
    case TARGET_OTHER:
      break;
  }
  return false;
}

int TMonster::aiGag(TBeing*, TBeing*, aiTarg cond) {
  if (!isHumanoid()) {
    return false;
  }
  switch (cond) {
    case TARGET_NONE:
    case TARGET_SELF:
    case TARGET_MOB:
    case TARGET_OTHER:
      break;
  }
  return false;
}

int TMonster::aiTrip(TBeing*, TBeing*, aiTarg cond) {
  if (!isHumanoid()) {
    return false;
  }
  switch (cond) {
    case TARGET_NONE:
    case TARGET_SELF:
    case TARGET_MOB:
    case TARGET_OTHER:
      break;
  }
  return false;
}

int TMonster::aiBoggle(TBeing*, TBeing*, aiTarg cond) {
  if (!isHumanoid()) {
    return false;
  }
  switch (cond) {
    case TARGET_NONE:
    case TARGET_SELF:
    case TARGET_MOB:
    case TARGET_OTHER:
      break;
  }
  return false;
}

int TMonster::aiJump(TBeing*, TBeing*, aiTarg cond) {
  if (!isHumanoid()) {
    return false;
  }
  switch (cond) {
    case TARGET_NONE:
    case TARGET_SELF:
    case TARGET_MOB:
    case TARGET_OTHER:
      break;
  }
  return false;
}

int TMonster::aiTango(TBeing*, TBeing*, aiTarg cond) {
  if (!isHumanoid()) {
    return false;
  }
  switch (cond) {
    case TARGET_NONE:
    case TARGET_SELF:
    case TARGET_MOB:
    case TARGET_OTHER:
      break;
  }
  return false;
}

int TMonster::aiWince(TBeing*, TBeing*, aiTarg cond) {
  if (!isHumanoid()) {
    return false;
  }
  switch (cond) {
    case TARGET_NONE:
    case TARGET_SELF:
    case TARGET_MOB:
    case TARGET_OTHER:
      break;
  }
  return false;
}

int TMonster::aiBrandish(TBeing*, TBeing*, aiTarg cond) {
  if (!isHumanoid()) {
    return false;
  }
  switch (cond) {
    case TARGET_NONE:
    case TARGET_SELF:
    case TARGET_MOB:
    case TARGET_OTHER:
      break;
  }
  return false;
}

int TMonster::aiNoogie(TBeing*, TBeing*, aiTarg cond) {
  if (!isHumanoid()) {
    return false;
  }
  switch (cond) {
    case TARGET_NONE:
    case TARGET_SELF:
    case TARGET_MOB:
    case TARGET_OTHER:
      break;
  }
  return false;
}

int TMonster::aiGrunt(TBeing*, TBeing*, aiTarg cond) {
  if (!isHumanoid()) {
    return false;
  }
  switch (cond) {
    case TARGET_NONE:
    case TARGET_SELF:
    case TARGET_MOB:
    case TARGET_OTHER:
      break;
  }
  return false;
}

int TMonster::aiHowl(TBeing*, TBeing*, aiTarg cond) {
  if (!isHumanoid()) {
    return false;
  }
  switch (cond) {
    case TARGET_NONE:
    case TARGET_SELF:
    case TARGET_MOB:
    case TARGET_OTHER:
      break;
  }
  return false;
}

int TMonster::aiGrimace(TBeing*, TBeing*, aiTarg cond) {
  if (!isHumanoid()) {
    return false;
  }
  switch (cond) {
    case TARGET_NONE:
    case TARGET_SELF:
    case TARGET_MOB:
    case TARGET_OTHER:
      break;
  }
  return false;
}

int TMonster::aiTip(TBeing*, TBeing*, aiTarg cond) {
  if (!isHumanoid()) {
    return false;
  }
  switch (cond) {
    case TARGET_NONE:
    case TARGET_SELF:
    case TARGET_MOB:
    case TARGET_OTHER:
      break;
  }
  return false;
}

int TMonster::aiAvert(TBeing*, TBeing*, aiTarg cond) {
  if (!isHumanoid()) {
    return false;
  }
  switch (cond) {
    case TARGET_NONE:
    case TARGET_SELF:
    case TARGET_MOB:
    case TARGET_OTHER:
      break;
  }
  return false;
}

int TMonster::aiWedgie(TBeing*, TBeing*, aiTarg cond) {
  if (!isHumanoid()) {
    return false;
  }
  switch (cond) {
    case TARGET_NONE:
    case TARGET_SELF:
    case TARGET_MOB:
    case TARGET_OTHER:
      break;
  }
  return false;
}

int TMonster::aiHum(TBeing*, TBeing*, aiTarg cond) {
  if (!isHumanoid()) {
    return false;
  }
  switch (cond) {
    case TARGET_NONE:
    case TARGET_SELF:
    case TARGET_MOB:
    case TARGET_OTHER:
      break;
  }
  return false;
}

int TMonster::aiBop(TBeing*, TBeing*, aiTarg cond) {
  if (!isHumanoid()) {
    return false;
  }
  switch (cond) {
    case TARGET_NONE:
    case TARGET_SELF:
    case TARGET_MOB:
    case TARGET_OTHER:
      break;
  }
  return false;
}

int TMonster::aiWhimper(TBeing*, TBeing*, aiTarg cond) {
  if (!isHumanoid()) {
    return false;
  }
  switch (cond) {
    case TARGET_NONE:
    case TARGET_SELF:
    case TARGET_MOB:
    case TARGET_OTHER:
      break;
  }
  return false;
}

int TMonster::aiScuff(TBeing*, TBeing*, aiTarg cond) {
  if (!isHumanoid()) {
    return false;
  }
  switch (cond) {
    case TARGET_NONE:
    case TARGET_SELF:
    case TARGET_MOB:
    case TARGET_OTHER:
      break;
  }
  return false;
}

int TMonster::aiRoar(TBeing*, TBeing*, aiTarg cond) {
  if (!isHumanoid()) {
    return false;
  }
  switch (cond) {
    case TARGET_NONE:
    case TARGET_SELF:
    case TARGET_MOB:
    case TARGET_OTHER:
      break;
  }
  return false;
}

int TMonster::aiGreet(TBeing*, TBeing*, aiTarg cond) {
  if (!isHumanoid()) {
    return false;
  }
  switch (cond) {
    case TARGET_NONE:
    case TARGET_SELF:
    case TARGET_MOB:
    case TARGET_OTHER:
      break;
  }
  return false;
}

int TMonster::aiFlex(TBeing*, TBeing*, aiTarg cond) {
  if (!isHumanoid()) {
    return false;
  }
  switch (cond) {
    case TARGET_NONE:
    case TARGET_SELF:
    case TARGET_MOB:
    case TARGET_OTHER:
      break;
  }
  return false;
}

int TMonster::aiRazz(TBeing*, TBeing*, aiTarg cond) {
  if (!isHumanoid()) {
    return false;
  }
  switch (cond) {
    case TARGET_NONE:
    case TARGET_SELF:
    case TARGET_MOB:
    case TARGET_OTHER:
      break;
  }
  return false;
}

int TMonster::aiToast(TBeing* doer, TBeing* other, aiTarg cond) {
  // this get's triggered by the doToast routine, not aiSocialSwitch
  if (!isHumanoid()) {
    switch (::number(0, 3)) {
      case 0:
        doAction(add_bars(doer->name), CMD_LICK);
        break;
      default:
        break;
    }
    return false;
  }
  switch (cond) {
    case TARGET_NONE:
      if (!::number(0, 3))
        doSay("Jolly good times!");
      break;
    case TARGET_SELF:
      if (!::number(0, 3))
        doAction(add_bars(doer->name), CMD_ROLL);
      break;
    case TARGET_OTHER:
      if (!::number(0, 5))
        doSay("Jolly good times!");
      else if (!::number(0, 3) && other)
        doSay(format("%s is the %s!") %
              stripColorCodes(sstring(other->getName()).cap()) % RandomWord());
      break;
    case TARGET_MOB:
      aiToastedAt(doer);
      break;
    default:
      break;
  }
  return false;
}
