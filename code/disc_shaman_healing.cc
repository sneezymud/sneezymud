#include "stdsneezy.h"
#include "disease.h"
#include "combat.h"
#include "disc_shaman_healing.h"
#include "spelltask.h"
#include "statistics.h"
#include "obj_magic_item.h"


////////////////////////////////////////////////////////////////
//   The following are copies of repHealing and adjustHealHp
//   from disc_cures.cc I used them here for convienence and
//   to make my life a little easier.
////////////////////////////////////////////////////////////////

static void repHealing2(TBeing *caster, TBeing *victim)
{
  char nameBuf[256];

  strcpy(nameBuf, good_cap(victim->getName()).c_str());
  sprintf(nameBuf,"%s",colorString(caster,caster->desc,nameBuf,NULL,COLOR_MOBS, TRUE).c_str());

  if (caster != victim) {
    caster->sendTo(COLOR_SPELLS, "%s looks to be at <r>%.1f%%<z> HP.\n\r",
       nameBuf, victim->getPercHit());
  }
}

static void adjustHealHp2(const TBeing *caster, int &hp, int durat)
{
  hp = (int)((double)(hp)*(stats.damage_modifier * 1.15));

  if (caster->spelltask && 
      IS_SET(caster->spelltask->flags, CASTFLAG_CAST_INDEFINITE)) {
    hp /= durat;
    hp = max(1,hp);
  } else if (caster->isImmortal() || (caster->spelltask && caster->spelltask->rounds == 0)) {
    hp /= (durat <= 1 ? 1 : 2);
    hp = max(3,hp);
  } else if (caster->spelltask) {
    if (durat <= 1) {
      vlogf(LOG_BUG, "Problem with hitpoint/rounds formula in shaman heals, caster is %s", caster->getName());
     durat = 2;
    }
    hp /= (durat-1)*2;
    hp = max(1,hp);
  } else {
    hp = max(3,hp);
  }
}

int healingGrasp(TBeing *caster, TBeing * victim, int level, byte bKnown, spellNumT spell, int adv_learn)
{
  int hp = caster->getSkillDam(victim, SPELL_HEALING_GRASP, level, adv_learn);
  adjustHealHp2(caster, hp, discArray[spell]->lag);

  if (bSuccess(caster, bKnown,caster->getPerc(), SPELL_HEALING_GRASP)) {
    LogDam(caster, spell, hp);
    switch (critSuccess(caster, SPELL_HEALING_GRASP)) {
      case CRIT_S_KILL:
      case CRIT_S_TRIPLE:
      case CRIT_S_DOUBLE:
        CS(SPELL_HEALING_GRASP);
        hp *= 2;
        colorAct(COLOR_SPELLS, "$n's eyes glow <G>bright green<z>." , FALSE, victim, NULL,0, TO_ROOM);
        colorAct(COLOR_SPELLS, "Your eyes glow <G>bright green<z>." , FALSE, victim, NULL, 0, TO_CHAR);
        break;
      case CRIT_S_NONE:
        colorAct(COLOR_SPELLS, "$n's eyes glow briefly with a <g>green tint<z>.", FALSE, victim, NULL, 0, TO_ROOM);
        colorAct(COLOR_SPELLS, "Your eyes glow briefly with a <c>green tint<z>.", FALSE, victim, NULL, 0, TO_CHAR);
        break;
    }
    if (victim->getHit() < victim->hitLimit()) {
      caster->reconcileHelp(victim,discArray[SPELL_HEALING_GRASP]->alignMod);
    }
    victim->addToHit(hp);
    victim->updatePos();
    return SPELL_SUCCESS;
  } else {
    caster->nothingHappens();
    return SPELL_FAIL;
  }
}

void healingGrasp(TBeing * caster, TBeing * victim, TMagicItem * obj, spellNumT spell)
{
  int ret;
  ret=healingGrasp(caster,victim,obj->getMagicLevel(),obj->getMagicLearnedness(), SPELL_HEALING_GRASP, 0);
  if (ret==SPELL_SUCCESS) {
    repHealing2(caster, victim);
  } else {
  }
}

void healingGrasp(TBeing *caster, TBeing *victim)
{
  spellNumT spell = caster->getSkillNum(SPELL_HEALING_GRASP);

  if (!bPassShamanChecks(caster, spell, victim))
    return;

  lag_t rounds = caster->isImmortal() ? LAG_0 : discArray[SPELL_HEALING_GRASP]->lag;
  taskDiffT diff = discArray[spell]->task;

  start_cast(caster, victim, NULL, caster->roomp, SPELL_HEALING_GRASP, diff, 1, "", rounds, caster->in_room, 0, 0,TRUE, 0);
}

int castHealingGrasp(TBeing *caster, TBeing *victim)
{
  char nameBuf[256];

  strcpy(nameBuf, good_cap(victim->getName()).c_str());
  sprintf(nameBuf,"%s",colorString(caster,caster->desc,nameBuf,NULL,COLOR_MOBS, TRUE).c_str());

  spellNumT spell = caster->getSkillNum(caster->spelltask->spell); 
  int ret=healingGrasp(caster,victim, caster->getSkillLevel(spell), caster->getSkillValue(spell), spell, caster->getAdvLearning(spell));
  if (ret == SPELL_SUCCESS) {
    if (victim->getHit() >= victim->hitLimit()) {
      if (caster == victim)
        caster->sendTo(COLOR_SPELLS, "<p>You are fully healed so you cease the ritual.<z>\n\r");
      else
        caster->sendTo(COLOR_SPELLS, "<p>%s <z><p>is fully healed so you cease the ritual.<z>\n\r", nameBuf);

      act("$n stops $s healing ritual.", TRUE, caster, NULL, NULL, TO_ROOM);
      caster->stopCast(STOP_CAST_NONE);
    } else 
      repHealing2(caster, victim);
  } else {
    if (victim->getHit() >= victim->hitLimit()) {
      if (caster == victim)
        caster->sendTo(COLOR_SPELLS, "<p>You are fully healed so you cease the ritual.<z>\n\r");
      else 
        caster->sendTo(COLOR_SPELLS, "<p>%s <z><p>is fully healed so you cease the ritual.<z>\n\r", nameBuf);
      act("$n stops $s healing ritual.", TRUE, caster, NULL, NULL, TO_ROOM);
      caster->stopCast(STOP_CAST_NONE);
    }
  }
  return ret;
}
