//////////////////////////////////////////////////////////////////////////
//
// SneezyMUD - All rights reserved, SneezyMUD Coding Team
//
//////////////////////////////////////////////////////////////////////////

#include <ctype.h>
#include <string.h>
#include <algorithm>
#include <list>

#include "being.h"
#include "comm.h"
#include "defs.h"
#include "disc_nature.h"
#include "enum.h"
#include "extern.h"
#include "handler.h"
#include "obj.h"
#include "obj_magic_item.h"
#include "parse.h"
#include "race.h"
#include "room.h"
#include "spell2.h"
#include "spells.h"
#include "spelltask.h"
#include "structs.h"
#include "thing.h"
#include "toggle.h"

int barkskin(TBeing* caster, TBeing* victim, int level, short bKnown) {
  affectedData aff;

  if (victim->isPlayerAction(PLR_SOLOQUEST) && (victim != caster) &&
      !caster->isImmortal() && caster->isPc()) {
    act("$N is on a quest, you can't invoke barkskin on $M!", false, caster,
      nullptr, victim, TO_CHAR);

    return false;
  }
  if (victim->isPlayerAction(PLR_GRPQUEST) && (victim != caster) &&
      !caster->isImmortal() && caster->isPc() &&
      !caster->isPlayerAction(PLR_GRPQUEST)) {
    act("$N is on a group quest you aren't on!  No help allowed!", false,
      caster, nullptr, victim, TO_CHAR);

    return false;
  }

  aff.type = SKILL_BARKSKIN;
  aff.location = APPLY_ARMOR;
  aff.duration = max(min(level / 2, 25), 1) * Pulse::UPDATES_PER_MUDHOUR;
  aff.bitvector = 0;
  aff.modifier = -90;

  if (caster->bSuccess(bKnown, caster->getPerc(), SKILL_BARKSKIN)) {
    if (critSuccess(caster, SKILL_BARKSKIN)) {
      CS(SKILL_BARKSKIN);
      aff.modifier *= 2;
      aff.duration *= 2;
    }

    if (caster != victim)
      aff.modifier = -10;

    if (!victim->affectJoin(caster, &aff, AVG_DUR_NO, AVG_EFF_YES)) {
      return SPELL_FALSE;
    }

    act("Your skin turns into an extremely hard, oak-like bark.", false, victim,
      nullptr, nullptr, TO_CHAR);
    act("$n's skin turns into an extremely hard, oak-like bark.", true, victim,
      nullptr, nullptr, TO_ROOM);

    caster->reconcileHelp(victim, discArray[SKILL_BARKSKIN]->alignMod);
    return SPELL_SUCCESS;
  } else {
    if (critFail(caster, SKILL_BARKSKIN)) {
      CF(SKILL_BARKSKIN);
      act("Your skin turns to hard bark, but then softens considerably!", false,
        victim, nullptr, nullptr, TO_CHAR);
      act("$n's skin turns to hard bark, but then seems to soften.", true,
        victim, nullptr, nullptr, TO_ROOM);
      aff.modifier = +20;
      caster->affectTo(&aff);
    } else {
      caster->sendTo("Nothing seems to happen.\n\r");
      act("Nothing seems to happen.", false, caster, nullptr, nullptr, TO_ROOM);
    }
    return SPELL_FAIL;
  }
}

int barkskin(TBeing* caster, TBeing* victim, TMagicItem* obj) {
  int rc = 0;

  int ret =
    barkskin(caster, victim, obj->getMagicLevel(), obj->getMagicLearnedness());
  if (IS_SET(ret, VICTIM_DEAD))
    ADD_DELETE(rc, DELETE_VICT);
  if (IS_SET(ret, CASTER_DEAD))
    ADD_DELETE(rc, DELETE_THIS);
  return rc;
}

int TBeing::doBarkskin(const char* argument) {
  int rc = 0;
  TBeing* victim = nullptr;
  char namebuf[256];

  if (!doesKnowSkill(SKILL_BARKSKIN)) {
    sendTo("You know nothing about barkskin.\n\r");
    return false;
  }

  if (!argument) {
    victim = this;
  } else {
    strcpy(namebuf, argument);
    if (!(victim = get_char_room_vis(this, namebuf))) {
      sendTo("Apply barkskin to what?\n\r");
      return false;
    }
  }
  if (!sameRoom(*victim)) {
    sendTo("That person isn't around.\n\r");
    return false;
  }

  int level = getSkillLevel(SKILL_BARKSKIN);
  int bKnown = getSkillValue(SKILL_BARKSKIN);

  // not technically a spell, but needs a component anyway
  if (!useComponent(findComponent(SKILL_BARKSKIN), victim, CHECK_ONLY_NO))
    return false;

  int ret = barkskin(this, victim, level, bKnown);
  if (IS_SET(ret, VICTIM_DEAD))
    ADD_DELETE(rc, DELETE_VICT);
  if (IS_SET(ret, CASTER_DEAD))
    ADD_DELETE(rc, DELETE_THIS);
  return rc;
}

// this is the old entry point for barskin (as a spell)
// it is still needed for mob-casting
int barkskin(TBeing* caster, TBeing* victim) {
  if (!bPassMageChecks(caster, SKILL_BARKSKIN, victim))
    return false;

  lag_t rounds = discArray[SKILL_BARKSKIN]->lag;
  taskDiffT diff = discArray[SKILL_BARKSKIN]->task;

  start_cast(caster, victim, nullptr, caster->roomp, SKILL_BARKSKIN, diff, 1, "",
    rounds, caster->in_room, 0, 0, true, 0);
  return false;
}

int castBarkskin(TBeing* caster, TBeing* victim) {
  int rc = 0;

  int level = caster->getSkillLevel(SKILL_BARKSKIN);
  int bKnown = caster->getSkillValue(SKILL_BARKSKIN);

  int ret = barkskin(caster, victim, level, bKnown);
  if (IS_SET(ret, VICTIM_DEAD))
    ADD_DELETE(rc, DELETE_VICT);
  if (IS_SET(ret, CASTER_DEAD))
    ADD_DELETE(rc, DELETE_THIS);
  return rc;
}

// TREE WALK

int TObj::treeMe(TBeing*, const char*, int, int*) { return false; }

int treeWalk(TBeing* caster, const char* arg, int, short bKnown) {
  TBeing* ch = nullptr;
  TObj* o;
  TRoom* rp = nullptr;
  TThing *t, *t3;
  int rc;
  int numx, j = 1;
  char tmpname[MAX_INPUT_LENGTH], *tmp;

  act(
    "You reach into the Sydarthae, in search of the life force of a powerful "
    "tree.",
    false, caster, 0, 0, TO_CHAR);
  act("$n enters a trance.", false, caster, 0, 0, TO_ROOM);

  for (; arg && *arg && isspace(*arg); arg++)
    ;

  if (caster->bSuccess(bKnown, SPELL_TREE_WALK)) {
    strcpy(tmpname, arg);
    tmp = tmpname;

    if (!(numx = get_number(&tmp)))
      numx = 1;

    o = nullptr;
    for (TObjIter iter = object_list.begin(); iter != object_list.end();
         ++iter) {
      o = *iter;
      if (o->treeMe(caster, tmp, numx, &j)) {
        rp = o->roomp;
        if (rp)
          break;
      }
    }
    if (!o) {
      for (ch = character_list; ch; ch = ch->next) {
        if (ch->getRace() != RACE_TREE)
          continue;
        if (isname(tmp, ch->name)) {
          if (j >= numx) {
            rp = ch->roomp;
            if (rp) {
              act(
                "You locate $N, and form a magical anchor between $M and you.",
                false, caster, 0, ch, TO_CHAR);
              break;
            }
          }
          j++;
        }
      }
    }
    if (!o && !ch) {
      act("You fail to find any lifeforce by that name.", false, caster, 0, 0,
        TO_CHAR);
      act("$n snaps out of $s trance.", false, caster, 0, 0, TO_ROOM);
      return SPELL_SUCCESS;
    }

    for (StuffIter it = caster->roomp->stuff.begin();
         it != caster->roomp->stuff.end();) {
      t = *(it++);
      TBeing* tbt = dynamic_cast<TBeing*>(t);
      if (!tbt)
        continue;
      if (tbt->inGroup(*caster)) {
        act(
          "A mystic force thrusts you into the Sydarthae, and out the "
          "otherside.",
          false, tbt, 0, 0, TO_CHAR);
        act("A mystic force yanks $n into somewhere unknown.", false, caster, 0,
          0, TO_ROOM);

        while ((t3 = tbt->rider)) {
          TBeing* tb = dynamic_cast<TBeing*>(t3);
          if (tb) {
            rc = tb->fallOffMount(t, POSITION_STANDING);
            if (IS_SET_DELETE(rc, DELETE_THIS)) {
              delete tb;
              tb = nullptr;
            }
          } else {
            t3->dismount(POSITION_DEAD);
          }
        }

        if (tbt->riding) {
          rc = tbt->fallOffMount(tbt->riding, POSITION_STANDING);
          if (IS_SET_DELETE(rc, DELETE_THIS)) {
            delete tbt;
            tbt = nullptr;
          }
        }

        --(*tbt);
        *rp += *tbt;

        act("$n shimmers into existence.", false, tbt, nullptr, nullptr, TO_ROOM);
        act("You shimmer into existence.", false, tbt, nullptr, nullptr, TO_CHAR);

        tbt->doLook("", CMD_LOOK);

        rc = tbt->genericMovedIntoRoom(rp, -1);
        if (IS_SET_DELETE(rc, DELETE_THIS)) {
          if (tbt != caster) {
            delete tbt;
            tbt = nullptr;
          } else {
            return SPELL_SUCCESS + CASTER_DEAD;
          }
        }
      }
    }
    return SPELL_SUCCESS;
  } else {
    act(
      "You fail to detect a life force strong enough to anchor yourself with.",
      false, caster, 0, 0, TO_CHAR);
    act("$n snaps out of $s trance.", false, caster, 0, 0, TO_ROOM);
    return SPELL_FAIL;
  }
}

int treeWalk(TBeing* caster, const char* arg) {
  int ret, level;

  if (!bPassMageChecks(caster, SPELL_TREE_WALK, nullptr))
    return false;

  if (caster->roomp->isFlyingSector()) {
    caster->sendTo("You are unable to break through the magic.");
    return false;
  }

  if (caster->fight()) {
    caster->sendTo("You are unable to commune with nature while fighting.");
    return false;
  }

  level = caster->getSkillLevel(SPELL_TREE_WALK);
  int bKnown = caster->getSkillValue(SPELL_TREE_WALK);

  ret = treeWalk(caster, arg, level, bKnown);
  if (IS_SET(ret, SPELL_SUCCESS)) {
  } else {
  }

  if (IS_SET(ret, CASTER_DEAD))
    return DELETE_THIS;
  return false;
}

// END TREE WALK
