//////////////////////////////////////////////////////////////////////////
//
// SneezyMUD - All rights reserved, SneezyMUD Coding Team
//
//////////////////////////////////////////////////////////////////////////


#include "stdsneezy.h"
#include "disease.h"
#include "combat.h"
#include "spelltask.h"
#include "disc_nature.h"
#include "obj_magic_item.h"



int barkskin(TBeing * caster, TBeing * victim, int level, byte bKnown)
{
  affectedData aff;

  if (victim->isPlayerAction(PLR_SOLOQUEST) && (victim != caster) &&
      !caster->isImmortal() && caster->isPc()) {
    act("$N is on a quest, you can't invoke barkskin on $M!",
      FALSE, caster, NULL, victim, TO_CHAR); 

    return FALSE;
  }
  if (victim->isPlayerAction(PLR_GRPQUEST) && (victim != caster) &&
          !caster->isImmortal() && caster->isPc() && !caster->isPlayerAction(PLR_GRPQUEST)) {
    act("$N is on a group quest you aren't on!  No help allowed!",
      FALSE, caster, NULL, victim, TO_CHAR);

    return FALSE;
  }

  aff.type = SKILL_BARKSKIN;
  aff.location = APPLY_ARMOR;
  aff.duration = max(min(level/2, 25), 1) * UPDATES_PER_MUDHOUR;
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

    act("Your skin turns into an extremely hard, oak-like bark.", 
        FALSE, victim, NULL, NULL, TO_CHAR);
    act("$n's skin turns into an extremely hard, oak-like bark.", 
        TRUE, victim, NULL, NULL, TO_ROOM);

    caster->reconcileHelp(victim, discArray[SKILL_BARKSKIN]->alignMod);
    return SPELL_SUCCESS;
  } else {
    if (critFail(caster, SKILL_BARKSKIN)) {
      CF(SKILL_BARKSKIN);
      act("Your skin turns to hard bark, but then softens considerably!", FALSE, victim, NULL, NULL, TO_CHAR);
      act("$n's skin turns to hard bark, but then seems to soften.", TRUE, victim, NULL, NULL, TO_ROOM);
      aff.modifier = +20;
      caster->affectTo(&aff);
    } else {
      caster->sendTo("Nothing seems to happen.\n\r");
      act("Nothing seems to happen.", FALSE, caster, NULL, NULL, TO_ROOM);
    }
    return SPELL_FAIL;
  }
}

int barkskin(TBeing * caster, TBeing * victim, TMagicItem * obj)
{
  int rc = 0;

  int ret=barkskin(caster,victim,obj->getMagicLevel(),obj->getMagicLearnedness());
  if (IS_SET(ret, VICTIM_DEAD))
    ADD_DELETE(rc, DELETE_VICT);
  if (IS_SET(ret, CASTER_DEAD))
    ADD_DELETE(rc, DELETE_THIS);
  return rc;
}

int TBeing::doBarkskin(const char *argument)
{
  int rc = 0;
  TBeing *victim = NULL;
  char namebuf[256];

  if (!doesKnowSkill(SKILL_BARKSKIN)) {
    sendTo("You know nothing about barkskin.\n\r");
    return FALSE;
  }

  if (!argument) {
    victim = this;
  } else {
    strcpy(namebuf, argument);
    if (!(victim = get_char_room_vis(this, namebuf))) {
      sendTo("Apply barkskin to what?\n\r");
      return FALSE;
    }
  }
  if (!sameRoom(*victim)) {
    sendTo("That person isn't around.\n\r");
    return FALSE;
  }

  int level = getSkillLevel(SKILL_BARKSKIN);
  int bKnown = getSkillValue(SKILL_BARKSKIN);

  // not technically a spell, but needs a component anyway
  if (!useComponent(findComponent(SKILL_BARKSKIN), victim, CHECK_ONLY_NO))
    return FALSE;

  int ret=barkskin(this,victim,level,bKnown);
  if (IS_SET(ret, VICTIM_DEAD))
    ADD_DELETE(rc, DELETE_VICT);
  if (IS_SET(ret, CASTER_DEAD))
    ADD_DELETE(rc, DELETE_THIS);
  return rc;
}

// this is the old entry point for barskin (as a spell)
// it is still needed for mob-casting
int barkskin(TBeing * caster, TBeing * victim)
{
  if (!bPassMageChecks(caster, SKILL_BARKSKIN, victim))
    return FALSE;

  lag_t rounds = discArray[SKILL_BARKSKIN]->lag;
  taskDiffT diff = discArray[SKILL_BARKSKIN]->task;

  start_cast(caster, victim, NULL, caster->roomp, SKILL_BARKSKIN, diff, 1, "", rounds, caster->in_room, 0, 0,TRUE, 0);
  return FALSE;
}

int castBarkskin(TBeing * caster, TBeing * victim)
{
  int rc = 0;

  int level = caster->getSkillLevel(SKILL_BARKSKIN);
  int bKnown = caster->getSkillValue(SKILL_BARKSKIN);

  int ret=barkskin(caster,victim,level,bKnown);
  if (IS_SET(ret, VICTIM_DEAD))
    ADD_DELETE(rc, DELETE_VICT);
  if (IS_SET(ret, CASTER_DEAD))
    ADD_DELETE(rc, DELETE_THIS);
  return rc;
}

// TREE WALK
 
int TObj::treeMe(TBeing *, const char *, int, int *)
{
  return FALSE;
}

int treeWalk(TBeing * caster, const char * arg, int, byte bKnown)
{
  TBeing *ch = NULL;
  TObj *o;
  TRoom *rp = NULL;
  TThing *t, *t2, *t3;
  int rc;
  int numx, j = 1;
  char tmpname[MAX_INPUT_LENGTH], *tmp;

  act("You reach into the Sydarthae, in search of the life force of a powerful tree.", FALSE, caster, 0, 0, TO_CHAR);
  act("$n enters a trance.", FALSE, caster, 0, 0, TO_ROOM);


  for (;arg && *arg && isspace(*arg); arg++);

  if (caster->bSuccess(bKnown,SPELL_TREE_WALK)) {
    strcpy(tmpname, arg);
    tmp = tmpname;

    if (!(numx = get_number(&tmp)))
      numx = 1;

    o=NULL;
    for(TObjIter iter=object_list.begin();iter!=object_list.end();++iter){
      o=*iter;
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
              act("You locate $N, and form a magical anchor between $M and you.", 
                    FALSE, caster, 0, ch, TO_CHAR);
              break;
            }
          }
          j++;
        }
      }
    }
    if (!o && !ch) {
      act("You fail to find any lifeforce by that name.", 
             FALSE, caster, 0, 0, TO_CHAR);
      act("$n snaps out of $s trance.", FALSE, caster, 0, 0, TO_ROOM);
      return SPELL_SUCCESS;
    }

    for (t = caster->roomp->getStuff(); t; t = t2) {
      t2 = t->nextThing;
      TBeing *tbt = dynamic_cast<TBeing *>(t);
      if (!tbt)
        continue;
      if (tbt->inGroup(*caster)) {
        act("A mystic force thrusts you into the Sydarthae, and out the otherside.",
           FALSE, tbt, 0, 0, TO_CHAR);
        act("A mystic force yanks $n into somewhere unknown.",
           FALSE, caster, 0, 0, TO_ROOM);

        while ((t3 = tbt->rider)) {
          TBeing *tb = dynamic_cast<TBeing *>(t3);
          if (tb) {
            rc = tb->fallOffMount(t, POSITION_STANDING);
            if (IS_SET_DELETE(rc, DELETE_THIS)) {
              delete tb;
              tb = NULL;
            }
          } else {
            t3->dismount(POSITION_DEAD);
          }
        }

        if (tbt->riding) {
          
          rc = tbt->fallOffMount(tbt->riding, POSITION_STANDING);
          if (IS_SET_DELETE(rc, DELETE_THIS)) {
            delete tbt;
            tbt = NULL;
          }
        }

        --(*tbt);
        *rp += *tbt;

        act("$n shimmers into existence.", FALSE, tbt, NULL, NULL, TO_ROOM);
        act("You shimmer into existence.", FALSE, tbt, NULL, NULL, TO_CHAR);

        tbt->doLook("", CMD_LOOK);

        rc = tbt->genericMovedIntoRoom(rp, -1);
        if (IS_SET_DELETE(rc, DELETE_THIS)) {
          if (tbt != caster) {
            delete tbt;
            tbt = NULL;
          } else {
            return SPELL_SUCCESS + CASTER_DEAD;
          }
        }
      }
    }
    return SPELL_SUCCESS;
  } else {
    act("You fail to detect a life force strong enough to anchor yourself with.", FALSE, caster, 0, 0, TO_CHAR);
    act("$n snaps out of $s trance.", FALSE, caster, 0, 0, TO_ROOM);
    return SPELL_FAIL;
  }
}

int treeWalk(TBeing * caster, const char * arg)
{
  int ret,level;
 
  if (!bPassMageChecks(caster, SPELL_TREE_WALK, NULL))
    return FALSE;

  if (caster->roomp->isFlyingSector()) {
    caster->sendTo("You are unable to break through the magic.");
    return FALSE;
  }

  if (caster->fight()) {
    caster->sendTo("You are unable to commune with nature while fighting.");
    return FALSE;
  }
 
  level = caster->getSkillLevel(SPELL_TREE_WALK);
  int bKnown = caster->getSkillValue(SPELL_TREE_WALK);
 
  ret=treeWalk(caster,arg,level,bKnown);
  if (IS_SET(ret, SPELL_SUCCESS)) {
  } else {
  }

  if (IS_SET(ret, CASTER_DEAD))
    return DELETE_THIS;
  return FALSE;
}

// END TREE WALK

