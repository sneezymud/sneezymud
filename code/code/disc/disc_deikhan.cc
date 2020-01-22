//////////////////////////////////////////////////////////////////////////
//
// SneezyMUD - All rights reserved, SneezyMUD Coding Team
//
//////////////////////////////////////////////////////////////////////////


#include "handler.h"
#include "being.h"
#include "disease.h"
#include "combat.h"
#include "disc_deikhan.h"

extern void startChargeTask(TBeing *, const char *);

int synostodweomer(TBeing *caster, TBeing *v, int level, short bKnown)
{
  int hitp = 0;
  affectedData aff;

  if (!caster->isImmortal() && caster->checkForSkillAttempt(SPELL_SYNOSTODWEOMER)) {
    act("You are not prepared to try to Snyostodweomer again so soon.",
         FALSE, caster, NULL, NULL, TO_CHAR);
    return FALSE;
  }

  if (v->affectedBySpell(SPELL_SYNOSTODWEOMER)) {
    caster->sendTo(COLOR_MOBS, format("%s is already affected by Snyostodweomer.\n\r") % v->getName());
    return FALSE;
  }

  if (caster->bSuccess(bKnown, caster->getPerc(), SPELL_SYNOSTODWEOMER)) {
    act("Power rushes through your fingers as you bless $N.", FALSE, caster, NULL, v, TO_CHAR);
    act("Power rushes through $n's fingers as $e blesses $N.", TRUE, caster, NULL, v, TO_NOTVICT);
    act("Power surges through $n's fingers as $e blesses you.", TRUE, caster, NULL, v, TO_VICT);

    if (!caster->isImmortal()) {
      aff.type = AFFECT_SKILL_ATTEMPT;
      aff.location = APPLY_NONE;
      aff.duration = max(min(level/12, 5), 1) * Pulse::UPDATES_PER_MUDHOUR;
      aff.bitvector = 0;
      aff.modifier = SPELL_SYNOSTODWEOMER;
      caster->affectTo(&aff, -1);
    }

    // hps to add to the vict
    int learnedness = caster->getDiscipline(DISC_DEIKHAN_AEGIS)->getLearnedness();
    // max of 60
    hitp = 10 + (caster->GetMaxLevel() * learnedness / 100);

    if (critSuccess(caster, SPELL_SYNOSTODWEOMER)) {
      CS(SPELL_SYNOSTODWEOMER);
      hitp += 10;
    }

    aff.type = SPELL_SYNOSTODWEOMER;
    aff.level = level;
    aff.duration  = max(min(level/12, 5), 1) * Pulse::UPDATES_PER_MUDHOUR;
    aff.modifier = hitp;
    aff.location = APPLY_HIT;
    aff.bitvector = 0;
    v->affectTo(&aff, -1);
    aff.location = APPLY_CURRENT_HIT;
    v->affectTo(&aff, -1);

    v->updatePos();
    caster->updatePos();
    caster->reconcileHelp(v, 0.11);
    return SPELL_SUCCESS;
  } else {
    caster->sendTo("Nothing seems to happen.\n\r");
    act("Nothing seems to happen.", TRUE, caster, 0, 0, TO_ROOM);
    return SPELL_FAIL;
  }
}

int synostodweomer(TBeing *caster, TBeing *v)
{
  int level, ret;
  int rc = 0;

  if (!bPassClericChecks(caster,SPELL_SYNOSTODWEOMER))
     return FALSE;

  if (!caster->isImmortal() && 
          caster->checkForSkillAttempt(SPELL_SYNOSTODWEOMER)) {
    act("You are not prepared to try to Snyostodweomer again so soon.",
        FALSE, caster, NULL, NULL, TO_CHAR);
    return FALSE;
  }

  if (v->affectedBySpell(SPELL_SYNOSTODWEOMER)) {
    caster->sendTo(COLOR_MOBS, format("%s is already affected by Snyostodweomer.\n\r") % v->getName());
    return FALSE;
  }

  level = caster->getSkillLevel(SPELL_SYNOSTODWEOMER);

  ret = synostodweomer(caster,v,level,caster->getSkillValue(SPELL_SYNOSTODWEOMER));

  if (IS_SET(ret, VICTIM_DEAD))
    ADD_DELETE(rc, DELETE_VICT);
  if (IS_SET(ret, CASTER_DEAD))
    ADD_DELETE(rc, DELETE_THIS);
  return rc;
}

int TBeing::doLayHands(const char *arg)
{
  int amt;
  affectedData aff;
  char name_buf[MAX_INPUT_LENGTH + 1];
  TBeing *vict;

  if (!doesKnowSkill(SKILL_LAY_HANDS)) {
    sendTo("You know nothing about laying on of hands.\n\r");
    return FALSE;
  }
  if (checkForSkillAttempt(SKILL_LAY_HANDS)) {
    sendTo("You are unprepared to attempt to lay hands again.\n\r");
    return FALSE;
  }
  if (affectedBySpell(SKILL_LAY_HANDS)) {
    sendTo("You are unable to lay hands again at this time.\n\r");
    return FALSE;
  }
  one_argument(arg, name_buf, cElements(name_buf));

  if (!(vict = get_char_room_vis(this, name_buf))) {
    sendTo("No one here by that name.\n\r");
    return FALSE;
  }

  // Prevent back-to-back attempts
  aff.type = AFFECT_SKILL_ATTEMPT;
  aff.duration = 1.5 * Pulse::UPDATES_PER_MUDHOUR;
  aff.modifier = SKILL_LAY_HANDS;
  aff.location = APPLY_NONE;
  aff.bitvector = 0;
  affectTo(&aff, -1);

  if (vict == this) {
    sendTo("You attempt to lay hands on yourself.\n\r");
    act("$n tries to lay hands on $mself.", FALSE, this, NULL, NULL, TO_ROOM);
  } else {
    act("You attempt to lay hands on $N.", FALSE, this, NULL, vict, TO_CHAR);
    act("$n attempts to lay hands on you.", FALSE, this, NULL, vict, TO_VICT);
    act("$n attempts to lay hands on $N.", FALSE, this, NULL, vict, TO_NOTVICT);
  }
  amt = ::number(1,100) + (4 * getClassLevel(CLASS_DEIKHAN));

  if (bSuccess(getSkillValue(SKILL_LAY_HANDS), getPerc(), SKILL_LAY_HANDS)) {
    LogDam(this, SKILL_LAY_HANDS, amt);

    if (this != vict) {
      act("A soft yellow light surrounds your hands as you touch $N.",
          FALSE, this, 0, vict, TO_CHAR);
      act("A soft yellow light surrounds $n's hands as $e touches you.",
          FALSE, this, 0, vict, TO_VICT);
      act("A soft yellow light surrounds $n's hands as $e touches $N.",
          FALSE, this, 0, vict, TO_NOTVICT);
    } else {
      act("A soft yellow light surrounds your hands as you touch yourself.",
          FALSE, this, 0, vict, TO_CHAR);
      act("A soft yellow light surrounds $n's hands as $e touches $mself.",
          FALSE, this, 0, vict, TO_ROOM);
    }
    vict->addToHit(amt);
    vict->setHit(std::min(vict->getHit(), (int) vict->hitLimit()));

    // success prevents from working for longer
    aff.type = SKILL_LAY_HANDS;
    aff.duration = 10 * Pulse::UPDATES_PER_MUDHOUR;
    aff.location = APPLY_NONE;
    aff.modifier = 0;
    aff.bitvector = 0;
    affectTo(&aff, -1);
  }
  reconcileHelp(vict, discArray[SKILL_LAY_HANDS]->alignMod);
  return TRUE;
}

int TBeing::doSmite(const char *arg, TBeing *victim)
{
  TBeing *vict;
  char tmp[80];
  int rc;

  if (checkBusy()) {
    return FALSE;
  }
  if (!doesKnowSkill(SKILL_SMITE)) {
    sendTo("You know nothing about smiting.\n\r");
    return FALSE;
  }
  strcpy(tmp, arg);
  if (!(vict = victim)) {
    if (!(vict = get_char_room_vis(this,tmp))) {
      if (!(vict = fight())) {
        sendTo("Smite whom?\n\r");
        return FALSE;
      }
    }
  }

  if (!sameRoom(*vict)) {
    sendTo("That person isn't around.\n\r");
    return FALSE;
  }
  if (noHarmCheck(vict))
    return FALSE;
  rc = smite(this, vict);
  if (rc)
    addSkillLag(SKILL_SMITE, rc);
  if (IS_SET_DELETE(rc, DELETE_VICT)) {
    if (victim)
      return rc;
    delete vict;
    vict = NULL;
    REM_DELETE(rc, DELETE_VICT);
  }
  return rc;
}

int smite(TBeing *ch, TBeing *v)
{
  TThing *weap;

  if (ch->getPosition() < POSITION_STANDING) {
    ch->sendTo("It is undignified to smite someone while not on your own two feet.\n\r");
    return FALSE;
  }
  if (ch == v) {
    ch->sendTo("You contemplate smiting yourself.\n\rYou realize there are better ways to punish yourself.\n\r");
    return FALSE;
  }
  if (ch->checkPeaceful("This room is too peaceful to contemplate violence in.\n\r"))
    return FALSE;

  if (!(weap = ch->heldInPrimHand())) {
    ch->sendTo("Perhaps you'd like to smite with something next time...?\n\r");
    return FALSE;
  }
  return weap->smiteWithMe(ch, v);
}

int TThing::smiteWithMe(TBeing *ch, TBeing *)
{
  ch->sendTo("Perhaps you'd like to smite with a weapon next time...?\n\r");
  return FALSE;
}
