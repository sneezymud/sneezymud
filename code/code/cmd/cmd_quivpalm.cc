//////////////////////////////////////////////////////////////////////////
//
// SneezyMUD - All rights reserved, SneezyMUD Coding Team
//
//////////////////////////////////////////////////////////////////////////


#include "handler.h"
#include "being.h"
#include "combat.h"

static int quiveringPalm(TBeing *c, TBeing *v)
{
  affectedData aff;
  int percent;
  int i;

  if (c->checkPeaceful("You feel too peaceful to contemplate violence.\n\r"))
    return FALSE;

  if (c->noHarmCheck(v))
    return FALSE;
  if (!c->hasHands()) {
    c->sendTo("You need hands to successfully quiver someone!\n\r");
    return FALSE;
  }
  if (c->bothArmsHurt()) {
    c->sendTo("At least one of your arms needs to work to try to quiver!\n\r");
    return FALSE;
  }
  //  if (IS_SET(v->specials.act, ACT_IMMORTAL)) {
  if(v->isImmortal()){
    c->sendTo("You decide not to waste your concentration on an immortal.\n\r");
    return FALSE;
  }
  if (!v->isHumanoid()) {
    c->sendTo("You can only do this to humanoid opponents.\n\r");
    return FALSE;
  }
  if (c->affectedBySpell(SKILL_QUIV_PALM) || c->checkForSkillAttempt(SKILL_QUIV_PALM)) {
    c->sendTo("You are not yet centered enough to attempt this maneuver again.\n\r");
    return FALSE;
  }
  percent = 0;

  if(c->getMana()<100){
    c->sendTo("You lack the chi.\n\r");
  }
  c->reconcileMana(TYPE_UNDEFINED, 0, 100);
  
  c->sendTo("You begin to work on the vibrations.\n\r");
  c->reconcileHurt(v, 0.1);

  int bKnown = c->getSkillValue(SKILL_QUIV_PALM);

  c->reconcileDamage(v, 0,SKILL_QUIV_PALM);

  int dmg=bKnown*10;

  if (v->getHit() > dmg){
    SV(SKILL_QUIV_PALM);
    act("$N seems unaffected by the vibrations.", 
         FALSE, c, NULL, v, TO_CHAR);
    act("$n touches you, but you ignore the puny vibrations.", 
         FALSE, c, NULL, v, TO_VICT);
    act("$n touches $N, but $E ignores it.", 
         FALSE, c, NULL, v, TO_NOTVICT);
    aff.type = AFFECT_SKILL_ATTEMPT;
    aff.duration = 10 * UPDATES_PER_MUDHOUR;
    aff.modifier = SKILL_QUIV_PALM;
    aff.location = APPLY_NONE;
    aff.bitvector = 0;
    c->affectTo(&aff, -1);

    return TRUE;
  }

  if (c->bSuccess(bKnown + percent, SKILL_QUIV_PALM) &&
      ((i = c->specialAttack(v, SKILL_QUIV_PALM)) || (i == GUARANTEED_SUCCESS))) {
    int dam = v->getHit()+100;
    if (c->willKill(v, dam, SKILL_QUIV_PALM, false)) {
      act("$N is killed instantly by the dreaded quivering palm.", 
            FALSE, c, NULL, v, TO_CHAR);
      act("As $n touches you, you feel your bones and organs shatter inside.", 
              FALSE, c, NULL, v, TO_VICT);
      act("$N dies as $n touches $M.", FALSE, c, NULL, v, TO_NOTVICT);
    } else {
      act("$N is heinously wounded by the dreaded quivering palm.", 
            FALSE, c, NULL, v, TO_CHAR);
      act("As $n touches you, you feel your bones and organs shatter inside.", 
              FALSE, c, NULL, v, TO_VICT);
      act("$N is grievously wounded as $n touches $M.", FALSE, c, NULL, v, TO_NOTVICT);
    }

    aff.type = SKILL_QUIV_PALM;
    aff.duration = 4 * UPDATES_PER_MUDHOUR;
    aff.modifier = 0;
    aff.location = APPLY_NONE;
    aff.bitvector = 0;
    c->affectTo(&aff, -1);
    if (c->reconcileDamage(v, dam,SKILL_QUIV_PALM) == -1)
      return DELETE_VICT;
    return TRUE;
  } else {
    c->sendTo("The vibrations fade ineffectively.\n\r");

    aff.type = AFFECT_SKILL_ATTEMPT;
    aff.duration = 4 * UPDATES_PER_MUDHOUR;
    aff.modifier = SKILL_QUIV_PALM;
    aff.location = APPLY_NONE;
    aff.bitvector = 0;
    c->affectTo(&aff, -1);

    act("$n touches you, but nothing seems to happen.", 0, c, 0, v, TO_VICT);
    act("$n touches $N, but nothing seems to happen.", 0, c, 0, v, TO_NOTVICT);
  }
  return TRUE;
}

int TBeing::doQuiveringPalm(const char *arg, TBeing *vict)
{
  TBeing *victim;
  char v_name[MAX_INPUT_LENGTH];
  int rc;

  if (!doesKnowSkill(SKILL_QUIV_PALM)) {
    sendTo("You don't know the secret of quivering palm.\n\r");
    return FALSE;
  }

  strcpy(v_name, arg);

  if (!(victim = vict)) {
    if (!(victim = get_char_room_vis(this, v_name))) {
      if (!(victim = fight())) {
        sendTo("Use the fabled quivering palm on whom?\n\r");
        return FALSE;
      }
    }
  }
  if (!sameRoom(*victim)) {
    sendTo("That person isn't around.\n\r");
    return FALSE;
  }
  if (victim == this) {
    sendTo("This is not an approved method for committing ritual suicide.\n\r");
    return FALSE;
  }
  rc = quiveringPalm(this, victim);
  if (rc)
    addSkillLag(SKILL_QUIV_PALM, rc);
  if (IS_SET_DELETE(rc, DELETE_VICT)) {
    if (vict)
      return rc;
    delete victim;
    victim = NULL;
    REM_DELETE(rc, DELETE_VICT);
  }
  return rc;
}


