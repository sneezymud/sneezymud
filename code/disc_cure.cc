#include "stdsneezy.h"
#include "disease.h"
#include "combat.h"
#include "disc_cures.h"
#include "spelltask.h"
#include "statistics.h"
#include "obj_magic_item.h"

static void repHealing(TBeing *caster, TBeing *victim)
{
  char nameBuf[256];

  strcpy(nameBuf, sstring(victim->getName()).cap().c_str());
  sprintf(nameBuf,"%s",colorString(caster,caster->desc,nameBuf,NULL,COLOR_MOBS, TRUE).c_str());

  if (caster != victim) {
    caster->sendTo(COLOR_SPELLS, fmt("%s looks to be at <r>%.1f%c<z> HP.\n\r") %
       nameBuf % victim->getPercHit() % '%');
  }
}

static void adjustHealHp(const TBeing *caster, int &hp, int durat)
{
  // hp, as we come in here is amount it should do for "full cast"
  // since it does a "per round" amount, we should split hp up
  // basically, we want to award about a lot at the end, and divy up the
  // rest over life of cast
  // divide damage over durat round so that last round gets 50% and
  // other rounds get equal shares of remainder

  // ok we have a nice damage modifer, but if we don't modify hp from heal as well,
  // then clerics can heal tanks too fast.
  // the mud is semi balanced as far as heal to 5.1, where our damage constant was .75
  // as of 5.2 the damage constant is .65 so i'm gonna make the heal constant .75
  hp = (int)((double)(hp)*(stats.damage_modifier * 1.15));
 
  // imms have had an adjustment to rounds made in start_cast
  // so adjust for that
  if (caster->isImmortal())
    durat = min(1, durat);

  if (caster->spelltask && 
      IS_SET(caster->spelltask->flags, CASTFLAG_CAST_INDEFINITE)) {
    // not sure about this, but lets just spread the hp out equally
    hp /= durat;
    hp = max(1,hp);
  } else if (caster->isImmortal() || (caster->spelltask && caster->spelltask->rounds == 0)) {
 // give out half on last round
    // if only takes 1 round, give it all out
    hp /= (durat <= 1 ? 1 : 2);
    hp = max(3,hp);
  } else if (caster->spelltask) {
    // on other rounds, divy up the other half equally
    if (durat <= 1) {
      vlogf(LOG_BUG, fmt("Problem with hitpoint/rounds formula in heal spells, caster is %s") %  caster->getName());
     durat = 2;
    }
    hp /= (durat-1)*2;
    hp = max(1,hp);
  } else {
    // potion or something
    hp = max(3,hp);
  }
}

int healLight(TBeing *caster, TBeing * victim, int level, byte bKnown, spellNumT spell, int adv_learn)
{
  int hp = caster->getSkillDam(victim, spell, level, adv_learn);
  adjustHealHp(caster, hp, discArray[spell]->lag);

  if (caster->bSuccess(bKnown,caster->getPerc(), spell)) {
    LogDam(caster, spell, hp);
    switch (critSuccess(caster, spell)) {
      case CRIT_S_KILL:
      case CRIT_S_TRIPLE:
      case CRIT_S_DOUBLE:
        CS(spell);
        hp *= 2;
        colorAct(COLOR_SPELLS, "$n is engulfed by an aura of the <C>deepest cyan.<z>" , FALSE, victim, NULL,0, TO_ROOM);
        colorAct(COLOR_SPELLS, "You are engulfed by an aura of the <C>deepest cyan<z>." , FALSE, victim, NULL, 0, TO_CHAR);
        break;
      case CRIT_S_NONE:
        colorAct(COLOR_SPELLS, "$n glows briefly with a <c>cyan hue<z>.", FALSE, victim, NULL, 0, TO_ROOM);
        colorAct(COLOR_SPELLS, "You glow briefly with a <c>cyan hue<z>.", FALSE, victim, NULL, 0, TO_CHAR);
        break;
    }
    if (victim->getHit() < victim->hitLimit()) {
      caster->reconcileHelp(victim,discArray[spell]->alignMod);
      checkFactionHelp(caster,victim);
    }
    victim->addToHit(hp);
    victim->updatePos();
    return SPELL_SUCCESS;
  } else {
    caster->deityIgnore();
    return SPELL_FAIL;
  }
}

void healLight(TBeing * caster, TBeing * victim, TMagicItem * obj, spellNumT spell)
{
  int ret;
  ret=healLight(caster,victim,obj->getMagicLevel(),obj->getMagicLearnedness(), spell, 0);
  if (ret==SPELL_SUCCESS) {
    repHealing(caster, victim);
  } else {
  }
}

void healLight(TBeing *caster, TBeing *victim)
{
  spellNumT spell = caster->getSkillNum(SPELL_HEAL_LIGHT);

  if (!bPassClericChecks(caster, spell))
    return;

  act("$n beseeches $d for divine healing.", FALSE, caster, NULL, victim, TO_ROOM);
  act("You beseech $d for divine healing.", FALSE, caster, NULL, victim, TO_CHAR);

  lag_t rounds = caster->isImmortal() ? LAG_0 : discArray[spell]->lag;
  taskDiffT diff = discArray[spell]->task;

  start_cast(caster, victim, NULL, caster->roomp, spell, diff, 1, "", rounds, caster->in_room, 0, 0,TRUE, 0);

}

int castHealLight(TBeing *caster, TBeing *victim)
{
  char nameBuf[256];

  strcpy(nameBuf, sstring(victim->getName()).cap().c_str());
  sprintf(nameBuf,"%s",colorString(caster,caster->desc,nameBuf,NULL,COLOR_MOBS, TRUE).c_str());

  spellNumT spell = caster->getSkillNum(caster->spelltask->spell); 
  int ret=healLight(caster,victim, caster->getSkillLevel(spell), caster->getSkillValue(spell), spell, caster->getAdvLearning(spell));
  if (ret == SPELL_SUCCESS) {
    if (victim->getHit() >= victim->hitLimit()) {
      if (caster == victim)
        caster->sendTo(COLOR_SPELLS, "<p>You are fully healed so you stop your prayer.<z>\n\r");
      else
        caster->sendTo(COLOR_SPELLS, fmt("<p>%s <z><p>is fully healed so you stop your prayer.<z>\n\r") % nameBuf);

      act("$n stops praying.", TRUE, caster, NULL, NULL, TO_ROOM);
      caster->stopCast(STOP_CAST_NONE);
    } else 
      repHealing(caster, victim);
  } else {
    if (victim->getHit() >= victim->hitLimit()) {
      if (caster == victim)
        caster->sendTo(COLOR_SPELLS, "<p>You are fully healed so you stop your prayer.<z>\n\r");
      else 
        caster->sendTo(COLOR_SPELLS, fmt("<p>%s <z><p>is fully healed so you stop your prayer.<z>\n\r") % nameBuf);
      act("$n stops praying.", TRUE, caster, NULL, NULL, TO_ROOM);
      caster->stopCast(STOP_CAST_NONE);
    }
  }
  return ret;
}

int healSerious(TBeing *caster, TBeing * victim, int level, byte bKnown, spellNumT spell, int adv_learn)
{
  int hp = caster->getSkillDam(victim, spell, level, adv_learn);
  adjustHealHp(caster, hp, discArray[spell]->lag);

  if (caster->bSuccess(bKnown,caster->getPerc(), spell)) {
    LogDam(caster, spell, hp);

    switch (critSuccess(caster, spell)) {
      case CRIT_S_KILL:
      case CRIT_S_TRIPLE:
      case CRIT_S_DOUBLE:
       CS(spell);
        hp *= 2;
        colorAct(COLOR_SPELLS, "$n is engulfed by an aura of the <B>deepest blue<z>.", FALSE, victim, NULL, 0, TO_ROOM);
        colorAct(COLOR_SPELLS, "You are engulfed by an aura of the <B>deepest blue<z>.", FALSE, victim, NULL, 0, TO_CHAR);
        break;
      case CRIT_S_NONE:
        colorAct(COLOR_SPELLS, "$n glows briefly with a <b>blue hue<z>.", 
                 FALSE, victim, NULL, 0, TO_ROOM);
        colorAct(COLOR_SPELLS, "You glow briefly with a <b>blue hue<z>.", 
                 FALSE, victim, NULL, 0, TO_CHAR);
        break;
    }
    if (victim->getHit() < victim->hitLimit()) {
      caster->reconcileHelp(victim,discArray[spell]->alignMod);
      checkFactionHelp(caster,victim);
    }
    victim->addToHit(hp);
    victim->updatePos();
    return SPELL_SUCCESS;
  } else {
    caster->deityIgnore();
    return SPELL_FAIL;
  }
}

void healSerious(TBeing * caster, TBeing * victim, TMagicItem * obj, spellNumT spell)
{
  int ret=healSerious(caster,victim,obj->getMagicLevel(),obj->getMagicLearnedness(), spell, 0);
  if (ret==SPELL_SUCCESS) {
    repHealing(caster, victim);
  } else {
  }
}


void healSerious(TBeing *caster, TBeing *victim)
{
  spellNumT spell = caster->getSkillNum(SPELL_HEAL_SERIOUS);

  if (!bPassClericChecks(caster, spell))
    return;

  act("$n beseeches $d for divine healing.", FALSE, caster, NULL, victim, TO_ROOM);
  act("You beseech $d for divine healing.", FALSE, caster, NULL, victim, TO_CHAR);
  lag_t rounds = caster->isImmortal() ? LAG_0 : discArray[spell]->lag;
  taskDiffT diff = discArray[spell]->task;

  start_cast(caster, victim, NULL, caster->roomp, spell, diff, 1, "",
 rounds, caster->in_room, 0, 0,TRUE, 0);

}
int castHealSerious(TBeing *caster, TBeing *victim)
{
  char nameBuf[256];

  strcpy(nameBuf, sstring(victim->getName()).cap().c_str());
  sprintf(nameBuf,"%s",colorString(caster,caster->desc,nameBuf,NULL,COLOR_MOBS, TRUE).c_str());

  spellNumT spell = caster->getSkillNum(caster->spelltask->spell);
  int ret=healSerious(caster,victim, caster->getSkillLevel(spell), caster->getSkillValue(spell), spell, caster->getAdvLearning(spell));
  if (ret == SPELL_SUCCESS) {
    if (victim->getHit() >= victim->hitLimit()) {
      if (caster == victim)
        caster->sendTo(COLOR_SPELLS, "<p>You are fully healed so you stop your prayer.<z>\n\r");
      else
        caster->sendTo(COLOR_SPELLS, fmt("<p>%s <z><p>is fully healed so you stop your prayer.<z>\n\r") % nameBuf);
    act("$n stops praying.", TRUE, caster, NULL, NULL, TO_ROOM);
    caster->stopCast(STOP_CAST_NONE);
    } else {
      repHealing(caster, victim);
    }
  } else {
    if (victim->getHit() >= victim->hitLimit()) {
      if (caster == victim)
        caster->sendTo(COLOR_SPELLS, "<p>You are fully healed so you stop your prayer.<z>\n\r");
      else
        caster->sendTo(COLOR_SPELLS, fmt("<p>%s <z><p>is fully healed so you stop your prayer.<z>\n\r") % nameBuf);
      act("$n stops praying.", TRUE, caster, NULL, NULL, TO_ROOM);
      caster->stopCast(STOP_CAST_NONE);
    }
  }
  return ret;
}

// Simple Prototype functions so it will compile. Delete if converted

int castHealCritSpray(TBeing *caster, TBeing *victim)
{
  if (caster && victim)
    return FALSE;
  return FALSE;
}

int castHealSpray(TBeing *caster, TBeing *victim)
{
  if (caster && victim)
    return FALSE;
  return FALSE;
}

int castHealFullSpray(TBeing *caster, TBeing *victim)
{
  if (caster && victim)
    return FALSE;
  return FALSE;
}

int healCritical(TBeing *caster, TBeing *victim, int level, byte bKnown, spellNumT spell, int adv_learn)
{
  int hp = caster->getSkillDam(victim, spell, level, adv_learn);
  adjustHealHp(caster, hp, discArray[spell]->lag);

  if (caster->bSuccess(bKnown,caster->getPerc(), spell)) {
    LogDam(caster, spell, hp);

    switch (critSuccess(caster, spell)) {
      case CRIT_S_KILL:
      case CRIT_S_TRIPLE:
      case CRIT_S_DOUBLE:
        CS(spell);
        hp *= 2;
        colorAct(COLOR_SPELLS, "$n is engulfed by an aura of the <P>deepest indigo<1>.", FALSE, victim, NULL, 0, TO_ROOM);
        colorAct(COLOR_SPELLS, "You are engulfed by an aura of the <P>deepest indigo<1>.", FALSE, victim, NULL, 0, TO_CHAR);
        break;
      case CRIT_S_NONE:
        colorAct(COLOR_SPELLS, "$n glows briefly with an <p>indigo hue<1>.",FALSE, victim, NULL, 0, TO_ROOM);
        colorAct(COLOR_SPELLS, "You glow briefly with an <p>indigo hue<1>.",FALSE, victim, NULL, 0, TO_CHAR);
        break;
    }
    if (victim->getHit() < victim->hitLimit()) {
      caster->reconcileHelp(victim,discArray[spell]->alignMod);
      checkFactionHelp(caster,victim);
    }
    victim->addToHit(hp);
    victim->updatePos();
    return SPELL_SUCCESS;
  } else {
    caster->deityIgnore();
    return SPELL_FAIL;
  }
}

void healCritical(TBeing * caster, TBeing * victim, TMagicItem * obj, spellNumT spell)
{
  int ret;
  ret=healCritical(caster,victim,obj->getMagicLevel(),obj->getMagicLearnedness(), spell, 0);
  if (ret==SPELL_SUCCESS) {
    repHealing(caster, victim);
  } else {
  }
}

void healCritical(TBeing *caster, TBeing *victim)
{
  spellNumT spell = caster->getSkillNum(SPELL_HEAL_CRITICAL);

  if (!bPassClericChecks(caster, spell))
    return;

  act("$n beseeches $d for divine healing.", FALSE, caster, NULL, victim, TO_ROOM);
  act("You beseech $d for divine healing.", FALSE, caster, NULL, victim, TO_CHAR);
  lag_t rounds = caster->isImmortal() ? LAG_0 : discArray[spell]->lag;
  taskDiffT diff = discArray[spell]->task;

  start_cast(caster, victim, NULL, caster->roomp, spell, diff, 1, "", rounds, caster->in_room, 0, 0,TRUE, 0);
}

int castHealCritical(TBeing *caster, TBeing *victim)
{
  char nameBuf[256];

  strcpy(nameBuf, sstring(victim->getName()).cap().c_str());
  sprintf(nameBuf,"%s",colorString(caster,caster->desc,nameBuf,NULL,COLOR_MOBS, TRUE).c_str());

  spellNumT spell = caster->getSkillNum(caster->spelltask->spell); 
  int ret=healCritical(caster,victim, caster->getSkillLevel(spell), caster->getSkillValue(spell), spell, caster->getAdvLearning(spell));
 if (ret == SPELL_SUCCESS) {
    if (victim->getHit() >= victim->hitLimit()) {
      if (caster == victim)
        caster->sendTo(COLOR_SPELLS, "<p>You are fully healed so you stop your prayer.<z>\n\r");
      else
        caster->sendTo(COLOR_SPELLS, fmt("<p>%s <z><p>is fully healed so you stop your prayer.<z>\n\r") % nameBuf);
    act("$n stops praying.", TRUE, caster, NULL, NULL, TO_ROOM);
    caster->stopCast(STOP_CAST_NONE);
    } else {
      repHealing(caster, victim);
    }
  } else {
    if (victim->getHit() >= victim->hitLimit()) {
      if (caster == victim)
        caster->sendTo(COLOR_SPELLS, "<p>You are fully healed so you stop your prayer.<z>\n\r");
      else
        caster->sendTo(COLOR_SPELLS, fmt("<p>%s <z><p>is fully healed so you stop your prayer.<z>\n\r") % nameBuf);
      act("$n stops praying.", TRUE, caster, NULL, NULL, TO_ROOM);
      caster->stopCast(STOP_CAST_NONE);
    }
  }
  return ret;
}

int heal(TBeing * caster, TBeing * victim, int level, byte bKnown, spellNumT spell, int adv_learn)
{
  int hp = caster->getSkillDam(victim, spell, level, adv_learn);
  adjustHealHp(caster, hp, discArray[spell]->lag);

  if (caster->bSuccess(bKnown,caster->getPerc(), spell)) {
    LogDam(caster, spell, hp);

    switch (critSuccess(caster, spell)) {
      case CRIT_S_KILL:
      case CRIT_S_TRIPLE:
      case CRIT_S_DOUBLE:
        CS(spell);
        hp *= 2;
        act("$n is engulfed by an aura of the <b>deepest ultramarine<1>.", FALSE, victim, NULL, 0, TO_ROOM);
        act("You are engulfed by an aura of the <b>deepest ultramarine<1>.", FALSE, victim, NULL, 0, TO_CHAR);
        break;
      case CRIT_S_NONE:
        act("$n glows briefly with an <b>ultramarine hue<1>.", FALSE, victim, NULL, 0, TO_ROOM);
        act("You glow briefly with an <b>ultramarine hue<1>.", FALSE, victim, NULL, 0, TO_CHAR);
        break;
    }
    if (victim->getHit() < victim->hitLimit()) {
      caster->reconcileHelp(victim,discArray[spell]->alignMod);
      checkFactionHelp(caster,victim);
    }
    victim->addToHit(hp);
    victim->updatePos();
    return SPELL_SUCCESS;
  } else {
    caster->deityIgnore();
    return SPELL_FAIL;
  }
}

void heal(TBeing * caster, TBeing * victim, TMagicItem * obj, spellNumT spell)
{
  int ret=heal(caster,victim,obj->getMagicLevel(),obj->getMagicLearnedness(), spell, 0);
  if (ret==SPELL_SUCCESS) {
    repHealing(caster, victim);
  } else {
  }
}

void heal(TBeing *caster, TBeing *victim)
{
  spellNumT spell = caster->getSkillNum(SPELL_HEAL);

  if (!bPassClericChecks(caster, spell))
    return;

  act("$n beseeches $d for divine healing.", FALSE, caster, NULL, victim, TO_ROOM);
  act("You beseech $d for divine healing.", FALSE, caster, NULL, victim, TO_CHAR);
  lag_t rounds = caster->isImmortal() ? LAG_0 : discArray[spell]->lag;
  taskDiffT diff = discArray[spell]->task;

  start_cast(caster, victim, NULL, caster->roomp, spell, diff, 1, "", rounds, caster->in_room, 0, 0,TRUE, 0);
}

int castHeal(TBeing *caster, TBeing *victim)
{
  char nameBuf[256];

  strcpy(nameBuf, sstring(victim->getName()).cap().c_str());
  sprintf(nameBuf,"%s",colorString(caster,caster->desc,nameBuf,NULL,COLOR_MOBS, TRUE).c_str());

  spellNumT spell = caster->getSkillNum(caster->spelltask->spell);
  int ret=heal(caster,victim, caster->getSkillLevel(spell), caster->getSkillValue(spell), spell, caster->getAdvLearning(spell));

  if (ret == SPELL_SUCCESS) {
    if (victim->getHit() >= victim->hitLimit()) {
      if (caster == victim)
        caster->sendTo(COLOR_SPELLS, "<p>You are fully healed so you stop your prayer.<z>\n\r");
      else
        caster->sendTo(COLOR_SPELLS, fmt("<p>%s <z><p>is fully healed so you stop your prayer.<z>\n\r") % nameBuf);
    act("$n stops praying.", TRUE, caster, NULL, NULL, TO_ROOM);
    caster->stopCast(STOP_CAST_NONE);
    } else {
      repHealing(caster, victim);
    }
  } else {
    if (victim->getHit() >= victim->hitLimit()) {
      if (caster == victim)
        caster->sendTo(COLOR_SPELLS, "<p>You are fully healed so you stop your prayer.<z>\n\r");
      else
        caster->sendTo(COLOR_SPELLS, fmt("<p>%s <z><p>is fully healed so you stop your prayer<z>.\n\r") % nameBuf);
      act("$n stops praying.", TRUE, caster, NULL, NULL, TO_ROOM);
      caster->stopCast(STOP_CAST_NONE);
    }
  }
  return ret;
}

int healFull(TBeing * caster, TBeing * victim, int level, byte bKnown, int adv_learn)
{                                                                               
  int hp = caster->getSkillDam(victim, SPELL_HEAL_FULL, level, adv_learn);
  adjustHealHp(caster, hp, discArray[SPELL_HEAL_FULL]->lag);

  if (caster->bSuccess(bKnown,caster->getPerc(), SPELL_HEAL_FULL)) {            
    LogDam(caster, SPELL_HEAL_FULL, hp);                                        
                                                                                
    switch (critSuccess(caster, SPELL_HEAL_FULL)) {                             
      case CRIT_S_KILL:
      case CRIT_S_TRIPLE:
      case CRIT_S_DOUBLE:                                                       
        CS(SPELL_HEAL_FULL);                                                    
        hp *= 2;                                                                
        act("$n is engulfed by an <B>aura of the deepest cobalt blue<1>.",
            FALSE, victim, NULL, 0, TO_ROOM);                                                    
        act("You are engulfed by an <B>aura of the deepest cobalt blue<1>.",
            FALSE, victim, NULL, 0, TO_CHAR);                                                  
        break;
      case CRIT_S_NONE:
        act("$n glows briefly with <b>a cobalt blue hue<1>.", 
            FALSE, victim, NULL, 0, TO_ROOM);
        act("You glow briefly with <b>a cobalt blue hue<1>.", 
            FALSE, victim, NULL, 0, TO_CHAR);
        break;
    }                                                                           
    if (victim->getHit() < victim->hitLimit()) {                                
      caster->reconcileHelp(victim,discArray[SPELL_HEAL_FULL]->alignMod);       
      checkFactionHelp(caster,victim);                                          
    }                                                                           
    victim->addToHit(hp);                                                       
    victim->updatePos();                                                        
    return SPELL_SUCCESS;                                                       
  } else {
    caster->deityIgnore();
    return SPELL_FAIL;
  }
}
void healFull(TBeing * caster, TBeing * victim, TMagicItem * obj)
{
int ret;
  ret = healFull(caster,victim,obj->getMagicLevel(),obj->getMagicLearnedness(), 0);
  if (ret == SPELL_SUCCESS) {
  } else {
  }
}

void healFull(TBeing * caster, TBeing * victim)
{
  spellNumT spell = caster->getSkillNum(SPELL_HEAL_FULL); 
                                                                               
  if (!bPassClericChecks(caster, spell))                                       
    return;                                                                    
                                                                               
  act("$n beseeches $d for divine healing.", 
      FALSE, caster, NULL, victim, TO_ROOM);
  act("You beseech $d for divine healing.", 
      FALSE, caster, NULL, victim, TO_CHAR);     

  lag_t rounds = caster->isImmortal() ? LAG_0 : discArray[spell]->lag;
  taskDiffT diff = discArray[spell]->task;

  start_cast(caster, victim, NULL, caster->roomp, spell, diff, 1, "", rounds, caster->in_room, 0, 0,TRUE, 0);
}

int castHealFull(TBeing *caster, TBeing *victim)                               
{                                                                               
  char nameBuf[256];                                                            
                                                                                
  strcpy(nameBuf, sstring(victim->getName()).cap().c_str());
  sprintf(nameBuf,"%s",colorString(caster,caster->desc,nameBuf,NULL,COLOR_MOBS, TRUE).c_str());
                                                                                
  spellNumT spell = caster->getSkillNum(caster->spelltask->spell);                        
  int ret = healFull(caster,victim, caster->getSkillLevel(spell), caster->getSkillValue(spell), caster->getAdvLearning(spell));
  if (ret == SPELL_SUCCESS) {                                                   
    if (victim->getHit() >= victim->hitLimit()) {                               
      if (caster == victim)                                                     
        caster->sendTo(COLOR_SPELLS, "<p>You are fully healed so you stop your prayer.<z>\n\r");                                                                
      else                                                                      
        caster->sendTo(COLOR_SPELLS, fmt("<p>%s <z><p>is fully healed so you stop your prayer.<z>\n\r") % nameBuf);                                                   
    act("$n stops praying.", TRUE, caster, NULL, NULL, TO_ROOM);                
    caster->stopCast(STOP_CAST_NONE);                                                        
    } else {                                                                    
      repHealing(caster, victim);                                               
    }                                                                           
  } else {                                                                      
    if (victim->getHit() >= victim->hitLimit()) {                               
      if (caster == victim)                                                     
        caster->sendTo(COLOR_SPELLS, "<p>You are fully healed so you stop your prayer.<z>\n\r");                                                                
      else                                                                      
        caster->sendTo(COLOR_SPELLS, fmt("<p>%s <z><p>is fully healed so you stop your prayer.<z>\n\r") % nameBuf);                                                   
      act("$n stops praying.", TRUE, caster, NULL, NULL, TO_ROOM);              
      caster->stopCast(STOP_CAST_NONE);                                                      
    }                                                                           
  }                                                                             
  return ret;                                                                   
}  

int healCritSpray(TBeing * caster, int level, byte bKnown, int adv_learn)
{
  int hp_tmp;
  //  bool healed_evil = FALSE;
  //  // int decrem;
  int hp = caster->getSkillDam(NULL, SPELL_HEAL_CRITICAL_SPRAY, level, adv_learn);
  TThing *t;
  TBeing *targ;

  // spray is not tasked, so no need to adjust hp like in other heals

  if (caster->bSuccess(bKnown,caster->getPerc(), SPELL_HEAL_CRITICAL_SPRAY)) {
    LogDam(caster, SPELL_HEAL_CRITICAL_SPRAY, hp);

    switch (critSuccess(caster, SPELL_HEAL_CRITICAL_SPRAY)) {
      case CRIT_S_KILL:
      case CRIT_S_TRIPLE:
      case CRIT_S_DOUBLE:
        CS(SPELL_HEAL_CRITICAL_SPRAY);
        hp *= 2;
        act("A healing fountain of beautiful colors pours forth from $n's hands!",
             FALSE, caster, NULL, NULL, TO_ROOM);
        act("A healing fountain of beautiful colors pours forth from your hands!",
             FALSE, caster, NULL, NULL, TO_CHAR);
        break;
      case CRIT_S_NONE:
        act("A beautiful healing spray of many colors issues from $n's hands!", FALSE, caster, NULL, NULL, TO_ROOM);
        act("A beautiful healing spray of many colors issues from your hands!", FALSE, caster, NULL, NULL, TO_CHAR);
        break;
    } 

    for (t = caster->roomp->getStuff(); t; t = t->nextThing) {
      targ = dynamic_cast<TBeing *>(t);
      if (!targ)
        continue;
      if ((targ->isImmortal()) && !caster->canSee(targ))
        continue;
      if (!targ->inGroup(*caster))
        continue;
      if (targ->getHit() < targ->hitLimit()) {
        caster->reconcileHelp(targ, discArray[SPELL_HEAL_CRITICAL_SPRAY]->alignMod);
      }
      act("$n revels in the healing spray!", 
                FALSE, targ, NULL, NULL, TO_ROOM);
      act("You revel in a healing spray!\n\rYou feel revived!", 
                FALSE, targ, NULL, NULL, TO_CHAR);

      hp_tmp = hp;
      //      hp_tmp /= (caster->isSameFaction(targ) ? 1 : 2);
      targ->addToHit(hp_tmp);
      targ->updatePos();
#if 0
      if (caster->isOppositeFaction(targ)) {
        decrem = (int) (caster->getMove() / 4);
        caster->addToMove(-decrem);
        healed_evil = TRUE;
      }
#endif
    }
#if 0
    if (healed_evil) {
      caster->sendTo(fmt("%s frowns upon the healing of minions of the enemy.\n\r") %
            caster->yourDeity(SPELL_HEAL_CRITICAL_SPRAY, FIRST_PERSON.cap()));
      caster->sendTo("You are exhausted from the effort of doing so.\n\r");
      act("$n's chest heaves from exhaustion.", FALSE, caster, NULL, NULL, TO_ROOM);
      caster->updatePos();
    }
#endif
    return SPELL_SUCCESS;
  } else {
    caster->deityIgnore();
    return SPELL_FAIL;
  }
}

void healCritSpray(TBeing * caster)
{
  if (!bPassClericChecks(caster,SPELL_HEAL_CRITICAL_SPRAY))
    return;

  int level = caster->getSkillLevel(SPELL_HEAL_CRITICAL_SPRAY);

  act("$n beseeches $d for divine healing.", FALSE, caster, 0, 0, TO_ROOM);
  act("You beseech $d for divine healing.", FALSE, caster, 0, 0, TO_CHAR);

  int ret=healCritSpray(caster,level,caster->getSkillValue(SPELL_HEAL_CRITICAL_SPRAY), caster->getAdvLearning(SPELL_HEAL_CRITICAL_SPRAY));
  if (ret==SPELL_SUCCESS) {
  } else {
  }
}

void healCritSpray(TBeing * caster, TMagicItem * obj)
{
int ret;
  ret=healCritSpray(caster, obj->getMagicLevel(),obj->getMagicLearnedness(), 0);
  if (ret==SPELL_SUCCESS) {
  } else {
  }
}

int healSpray(TBeing * caster, int level, byte bKnown, int adv_learn)
{
  int hp = caster->getSkillDam(NULL, SPELL_HEAL_SPRAY, level, adv_learn);
  int hp_tmp;
  // bool healed_evil= FALSE;
  // int decrem;
  TThing *t;
  TBeing *targ = NULL;

  // spray is not tasked, so no need to adjust hp like in other heals

  if (caster->bSuccess(bKnown,caster->getPerc(), SPELL_HEAL_SPRAY)) {
    LogDam(caster, SPELL_HEAL_SPRAY, hp);

    switch (critSuccess(caster, SPELL_HEAL_SPRAY)) {
      case CRIT_S_KILL:
      case CRIT_S_TRIPLE:
      case CRIT_S_DOUBLE:
        CS(SPELL_HEAL_SPRAY);
        hp *= 2;
        act("A magnificent fountain of twinkling colors gushs out in front of you!", FALSE, caster, NULL, 0, TO_CHAR);
        act("A magnificent fountain of twinkling colors gushs out in front of $n!\n\rYou feel revived!", FALSE, caster, NULL, 0, TO_ROOM);
        break;
      case CRIT_S_NONE:
        act("A magnificent spray of twinkling colors fans out in front of you!", FALSE, caster, NULL, 0, TO_CHAR);
        act("A magnificent spray of twinkling colors fans out in front of $n!", FALSE, caster, NULL, 0, TO_ROOM);
        break;
    } 

    for (t = caster->roomp->getStuff(); t; t = t->nextThing) {
      targ = dynamic_cast<TBeing *>(t);
      if (!targ)
        continue;

      if ((targ->isImmortal()) && !caster->canSee(targ))
        continue;
      if (!targ->inGroup(*caster))
        continue;

      if (targ->getHit() < targ->hitLimit()) 
        caster->reconcileHelp(targ, discArray[SPELL_HEAL_SPRAY]->alignMod);
      
      act("$n revels in the healing bath!", FALSE, targ, NULL, NULL, TO_ROOM);
      act("You revel in the healing bath!\n\rYou feel revived!", FALSE, targ, NULL, NULL, TO_CHAR);

      hp_tmp = hp;
      //      hp_tmp /= (caster->isSameFaction(targ) ? 1 : 2);
      targ->addToHit(hp_tmp);
      targ->updatePos();
#if 0
      if (caster->isOppositeFaction(targ)) {
        decrem = (int) (caster->getMove() / 4);
        caster->addToMove(-decrem);
        healed_evil = TRUE;
      }
#endif
    }
#if 0
    if (healed_evil) {
      caster->sendTo(fmt("%s frowns upon the healing of minions of the enemy.\n\r") %
          caster->yourDeity(SPELL_HEAL_SPRAY, FIRST_PERSON.cap()));
      caster->sendTo("You are exhausted from the effort of doing so.\n\r");
      act("$n's chest heaves from exhaustion.", FALSE, caster, NULL, NULL, TO_ROOM);
      caster->updatePos();
    }
#endif
    return SPELL_SUCCESS;
  } else {
    caster->deityIgnore();
    return SPELL_FAIL;
  }
}

void healSpray(TBeing * caster)
{
  if (!bPassClericChecks(caster,SPELL_HEAL_SPRAY))
    return;

  int level = caster->getSkillLevel(SPELL_HEAL_SPRAY);

  act("$n beseeches $d for divine healing.", FALSE, caster, 0, 0, TO_ROOM);
  act("You beseech $d for divine healing.", FALSE, caster, 0, 0, TO_CHAR);

  int ret=healSpray(caster,level,caster->getSkillValue(SPELL_HEAL_SPRAY), caster->getAdvLearning(SPELL_HEAL_SPRAY));
  if (ret==SPELL_SUCCESS) {
  } else {
  }
}

void healSpray(TBeing * caster, TMagicItem * obj)
{
  int ret;
  
  ret=healSpray(caster, obj->getMagicLevel(),obj->getMagicLearnedness(), 0);
  if (ret==SPELL_SUCCESS) {
  } else {
  }
}


int healFullSpray(TBeing * caster, int level, byte bKnown, int adv_learn)
{
  int hp = caster->getSkillDam(NULL, SPELL_HEAL_FULL_SPRAY, level, adv_learn);
  int hp_tmp;
  // bool healed_evil = FALSE;
  // int decrem;
  TBeing *targ = NULL;
  TThing *t;

  // spray is not tasked, so no need to adjust hp like in other heals

  if (caster->bSuccess(bKnown,caster->getPerc(), SPELL_HEAL_FULL_SPRAY)) {
    LogDam(caster, SPELL_HEAL_FULL_SPRAY, hp);

    switch (critSuccess(caster, SPELL_HEAL_FULL_SPRAY)) {
      case CRIT_S_KILL:
      case CRIT_S_TRIPLE:
      case CRIT_S_DOUBLE:
        CS(SPELL_HEAL_FULL_SPRAY);
        hp *= 2;
        act("A magnificent rainbow of bright colors streams out in front of you!", 
           FALSE, caster, NULL, 0, TO_CHAR);
        act("A magnificent rainbow of bright colors streams out in front of $n!",
           FALSE, caster, NULL, 0, TO_ROOM);
        break;
      case CRIT_S_NONE:
        act("A beautiful prism of brilliant colors fans out in front of $n!",
              FALSE, caster, NULL, 0, TO_ROOM);
        act("A beautiful prism of brilliant colors fans out in front of you!", 
              FALSE, caster, NULL, 0, TO_CHAR);
          break;
    } 

    for (t = caster->roomp->getStuff(); t; t = t->nextThing) {
      targ = dynamic_cast<TBeing *>(t);
      if (!targ)
        continue;
      if ((targ->isImmortal()) && !caster->canSee(targ))
        continue;
      if (!targ->inGroup(*caster))
        continue;
      if (targ->getHit() < targ->hitLimit()) {
        caster->reconcileHelp(targ, discArray[SPELL_HEAL_FULL_SPRAY]->alignMod);
      }
      act("$n revels in the healing colors!",
             FALSE, targ, NULL, NULL, TO_ROOM);
      act("You revel in the healing colors!\n\rYou feel revived!",
             FALSE, targ, NULL, NULL, TO_CHAR);
      hp_tmp = hp;
      //      hp_tmp /= (caster->isSameFaction(targ) ? 1 : 2);
      targ->addToHit(hp_tmp);
      targ->updatePos();
#if 0
      if (caster->isOppositeFaction(targ)) {
        decrem = (int) (caster->getMove() / 4);
        caster->addToMove(-decrem);
        healed_evil = TRUE;
      }
#endif
    }
#if 0
    if (healed_evil) {
      caster->sendTo(fmt("%s frowns upon the healing of minions of the enemy.\n\r") %
            caster->yourDeity(SPELL_HEAL_FULL_SPRAY, FIRST_PERSON.cap()));
      caster->sendTo("You are exhausted from the effort of doing so.\n\r");
      act("$n's chest heaves from exhaustion.", FALSE, caster, NULL, NULL, TO_ROOM);
      caster->updatePos();
    }
#endif
    return SPELL_SUCCESS;
  } else {
    caster->deityIgnore();
    return SPELL_FAIL;
  }
}

void healFullSpray(TBeing * caster)
{
  if (!bPassClericChecks(caster,SPELL_HEAL_FULL_SPRAY))
    return;

  int level = caster->getSkillLevel(SPELL_HEAL_FULL_SPRAY);

  act("$n beseeches $d for divine healing.", FALSE, caster, 0, 0, TO_ROOM);
  act("You beseech $d for divine healing.", FALSE, caster, 0, 0, TO_CHAR);

  int ret=healFullSpray(caster,level,caster->getSkillValue(SPELL_HEAL_FULL_SPRAY), caster->getAdvLearning(SPELL_HEAL_FULL_SPRAY));
  if (ret==SPELL_SUCCESS) {
  } else {
  }
}

void healFullSpray(TBeing * caster, TMagicItem * obj)
{
  int ret;
  
  ret=healFullSpray(caster, obj->getMagicLevel(),obj->getMagicLearnedness(), 0);
  if (ret==SPELL_SUCCESS) {
  } else {
  }
}


int knitBone(TBeing * caster, TBeing * victim, int, byte bKnown)
{
  char buf[256], limb[256];
  wearSlotT slot;

  if (!victim) {
    act("Nothing seems to happen.", TRUE, caster, 0, 0, TO_ROOM);
    return SPELL_FAIL;
  }

// changed order 1/19/98 it should check for broken bones before entering the for 
// loop. - Rix

  // What Rix did on 1/19 was buggy, needed to go through list first to see if 
  // something was broken before checking slot - Russ 1/19/98 (next 4 lines)
  for (slot = MIN_WEAR; slot < MAX_WEAR; slot++) {
    if (victim->slotChance(slot) && victim->isLimbFlags(slot, PART_BROKEN))
      break;
  }

  if (slot >= MAX_WEAR) {
    if (caster == victim)
      caster->sendTo("You have no broken bones to knit!\n\r");
    else
      act("$N has no broken bones!", FALSE, caster, NULL, victim, TO_CHAR);

    act("Nothing seems to happen.", FALSE, caster, NULL, NULL, TO_ROOM);
    return SPELL_FAIL;
  }

  // If we got here, we know that there is a broken slot, and the next
  // for loop will not go infinite or crash - Russ 01/19/98
  if (caster->bSuccess(bKnown, caster->getPerc(), SPELL_KNIT_BONE)) {
    // find a suitable slot to knit 
    for (slot = pickRandomLimb();
        ((!victim->slotChance(slot)) ||
         !victim->isLimbFlags(slot, PART_BROKEN));
        slot = pickRandomLimb());

    sprintf(limb, "%s", victim->describeBodySlot(slot).c_str());
    victim->remLimbFlags(slot, PART_BROKEN);
    sprintf(buf, "The broken bones in your %s miraculously knit together!", limb);
    act(buf, FALSE, victim, NULL, NULL, TO_CHAR);
    sprintf(buf, "The broken bones in $n's %s miraculously knit together!", limb);
    act(buf, FALSE, victim, NULL, NULL, TO_ROOM);
    caster->reconcileHelp(victim,discArray[SPELL_KNIT_BONE]->alignMod);
    checkFactionHelp(caster,victim);
    return SPELL_SUCCESS;
  } else {
    caster->deityIgnore();
    return SPELL_FAIL;
  }
}

void knitBone(TBeing * caster, TBeing * victim, TMagicItem * obj)
{
  knitBone(caster,victim,obj->getMagicLevel(),obj->getMagicLearnedness());
}

void knitBone(TBeing * caster, TBeing *victim)
{
  if (!bPassClericChecks(caster,SPELL_KNIT_BONE))
    return;

  int level = caster->getSkillLevel(SPELL_KNIT_BONE);

  int ret=knitBone(caster,victim,level,caster->getSkillValue(SPELL_KNIT_BONE));
  if (ret==SPELL_SUCCESS) {
  } else {
  }
}

int clot(TBeing * caster, TBeing * victim, int, byte bKnown, spellNumT spell)
{
  char buf[256], limb[256];
  wearSlotT slot;

  // find a bleeding slot 
  for (slot = MIN_WEAR; slot < MAX_WEAR; slot++) {
    if (!victim->slotChance(slot))
      continue;
    if (victim->isLimbFlags(slot, PART_BLEEDING))
      break;
  }

  if(slot >= MAX_WEAR && spell==SKILL_WOHLIN)
    return SPELL_FAIL;

  if (slot >= MAX_WEAR) {
    caster->sendTo("Uhm, are they bleeding???\n\r");
    act("Nothing seems to happen.", FALSE, caster, NULL, NULL, TO_ROOM);
    return SPELL_FAIL;
  }

  if (spell==SKILL_WOHLIN || 
      caster->bSuccess(bKnown, caster->getPerc(), spell)) {
    /* find a suitable slot to clot */
    for (slot = pickRandomLimb();
     ((!victim->slotChance(slot)) || (!victim->isLimbFlags(slot, PART_BLEEDING)));
       slot = pickRandomLimb());

    sprintf(limb, "%s", victim->describeBodySlot(slot).c_str());
    victim->remLimbFlags(slot, PART_BLEEDING);
    sprintf(buf, "The gash on your %s slowly stops bleeding and the flesh closes up!", limb);
    act(buf, FALSE, victim, NULL, NULL, TO_CHAR);
    sprintf(buf, "The gash on $n's %s slowly stops bleeding and the flesh closes up!", limb);
    act(buf, FALSE, victim, NULL, NULL, TO_ROOM);
    caster->reconcileHelp(victim,discArray[spell]->alignMod);
    checkFactionHelp(caster,victim);

    victim->doSave(SILENT_YES);
    return SPELL_SUCCESS;
  } else {
    caster->deityIgnore();
    return SPELL_FAIL;
  }
}

void clot(TBeing *caster, TBeing *victim)
{
  spellNumT spell = caster->getSkillNum(SPELL_CLOT);

  if (!bPassClericChecks(caster,spell))
    return;

  int level = caster->getSkillLevel(spell);
 
  int ret=clot(caster,victim,level,caster->getSkillValue(spell), spell);
  if (ret==SPELL_SUCCESS) {
  } else {
  }
}

void clot(TBeing * caster, TBeing * victim, TMagicItem * obj, spellNumT spell)
{
  clot(caster,victim,obj->getMagicLevel(),obj->getMagicLearnedness(), spell);
}

int restoreLimb(TBeing *caster, TBeing *victim, int, byte bKnown)
{
  char buf[256], limb[256];
  wearSlotT slot, num;
  wearSlotT j;

  for (slot = MIN_WEAR; slot < MAX_WEAR; slot++) {
    if (!victim->slotChance(slot))
      continue;
    if (!victim->isLimbFlags(slot, PART_USELESS | PART_PARALYZED))
      continue;

    break;
  }

  if (slot >= MAX_WEAR) {
    if (caster == victim)
      caster->sendTo("You don't have any limbs that need restoring!\n\r");
    else
      act("$N doesn't have any limbs that need restoring.", FALSE, caster, 0, victim, TO_CHAR);

    act("Nothing seems to happen.", FALSE, caster, NULL, NULL, TO_ROOM);
    return SPELL_FAIL;
  }

  if (caster->bSuccess(bKnown, caster->getPerc(), SPELL_RESTORE_LIMB)) {
    sprintf(limb, "%s", victim->describeBodySlot(slot).c_str());
    if (victim->isLimbFlags(slot, PART_PARALYZED))
      victim->remLimbFlags(slot, PART_PARALYZED);
    else
      victim->remLimbFlags(slot, PART_USELESS);
    victim->setCurLimbHealth(slot, victim->getMaxLimbHealth(slot));

    sprintf(buf, "You regain use of your %s!", limb);
    act(buf, FALSE, victim, NULL, NULL, TO_CHAR);
    sprintf(buf, "$n regains use of $s %s!", limb);
    act(buf, FALSE, victim, NULL, NULL, TO_ROOM);

    caster->reconcileHelp(victim,discArray[SPELL_RESTORE_LIMB]->alignMod);
    checkFactionHelp(caster,victim);

    victim->doSave(SILENT_YES);

    return SPELL_SUCCESS;
  } else {
    switch (critFail(caster, SPELL_RESTORE_LIMB)) {
      case CRIT_F_HITOTHER:
      case CRIT_F_HITSELF:
        CF(SPELL_RESTORE_LIMB);
        act("Oops! You have a terrible feeling something went wrong...",
             FALSE, caster, NULL, NULL, TO_CHAR);
        act("Something goes terribly wrong!", FALSE, caster, 0, 0, TO_CHAR);
        act("Something goes terribly wrong!", FALSE, caster, 0, 0, TO_ROOM);

        if (victim->isPc()) {
          do {
            num = wearSlotT(::number(WEAR_HEAD,WEAR_FOOT_L));
          } while (num == WEAR_BODY ||
                   num == WEAR_HEAD ||
                   num == WEAR_NECK ||
                   num == WEAR_BACK);
          victim->addToLimbFlags(num, PART_MISSING);
          for (j=MIN_WEAR; j < MAX_WEAR; j++) {
            if (j == HOLD_RIGHT || j == HOLD_LEFT)
              continue;
            if (!victim->slotChance(j))
              continue;
            if (!victim->limbConnections(j))
              victim->setLimbFlags(j, PART_MISSING);
          }
        }
        return SPELL_CRIT_FAIL;
      case CRIT_F_NONE:
        break;
    }
    caster->deityIgnore();
    return SPELL_FAIL;
  }
}

void restoreLimb(TBeing * caster, TBeing * victim, TMagicItem * obj)
{
  restoreLimb(caster,victim,obj->getMagicLevel(),obj->getMagicLearnedness());
}

void restoreLimb(TBeing *caster, TBeing *victim)
{
  if (!bPassClericChecks(caster,SPELL_RESTORE_LIMB))
    return;

  int level = caster->getSkillLevel(SPELL_RESTORE_LIMB);

  int ret=restoreLimb(caster,victim,level,caster->getSkillValue(SPELL_RESTORE_LIMB));
  if (ret==SPELL_SUCCESS) {
  } else {
    if (ret==SPELL_CRIT_FAIL) {
    } else {
    }
  }
}

int sterilize(TBeing * caster, TBeing * victim, int, byte bKnown, spellNumT spell)
{
  char buf[256], limb[256];
  wearSlotT slot;

  // find an infected slot 
  for (slot = MIN_WEAR; slot < MAX_WEAR; slot++) {
    if (!victim->slotChance(slot))
      continue;
    if (!victim->isLimbFlags(slot, PART_INFECTED)) 
      continue;
    break;
  }

  if(slot>=MAX_WEAR && spell==SKILL_WOHLIN)
    return SPELL_FAIL;

  if (slot >= MAX_WEAR) {
    if (caster == victim)
      act("You need no sterilization.", FALSE, caster, NULL, NULL, TO_CHAR);
    else
      act("$N needs no sterilization.", FALSE, caster, NULL, victim, TO_CHAR);

    act("Nothing seems to happen.", FALSE, caster, NULL, NULL, TO_ROOM);
    return SPELL_FAIL;
  }

  if (spell==SKILL_WOHLIN || 
      caster->bSuccess(bKnown, caster->getPerc(), spell)) {
    /* find a suitable slot to disinfect */
    for (slot = pickRandomLimb();
       (!victim->slotChance(slot) ||
        !victim->isLimbFlags(slot, PART_INFECTED));
       slot = pickRandomLimb());

    sprintf(limb, "%s", victim->describeBodySlot(slot).c_str());
    victim->remLimbFlags(slot, PART_INFECTED);
    sprintf(buf, "The infection in your %s has been killed!", limb);
    act(buf, FALSE, victim, NULL, NULL, TO_CHAR);
    sprintf(buf, "The infection in $n's %s has been killed!", limb);
    act(buf, FALSE, victim, NULL, NULL, TO_ROOM);
    caster->reconcileHelp(victim,discArray[spell]->alignMod);
    checkFactionHelp(caster,victim);
    return SPELL_SUCCESS;
  } else {
    caster->deityIgnore();
    return SPELL_FAIL;
  }
}

void sterilize(TBeing * caster, TBeing *victim)
{
  spellNumT spell = caster->getSkillNum(SPELL_STERILIZE);

  if (!bPassClericChecks(caster,spell))
    return;

  int level = caster->getSkillLevel(spell);

  int ret=sterilize(caster,victim,level,caster->getSkillValue(spell), spell);
  if (ret==SPELL_SUCCESS) {
  } else {
  }
}

void sterilize(TBeing * caster, TBeing * victim, TMagicItem * obj, spellNumT spell)
{
  sterilize(caster,victim,obj->getMagicLevel(),obj->getMagicLearnedness(), spell);
}

int salve(TBeing * caster, TBeing * victim, int level, byte bKnown, spellNumT spell)
{
  int fixed = 0, max_am;
  wearSlotT slot;
  char buf[256];

  max_am = 1 + (level * number(1,6)/2);

  if (spell==SKILL_WOHLIN ||
      caster->bSuccess(bKnown, caster->getPerc(), spell)) {
    if(spell!=SKILL_WOHLIN)
      LogDam(caster, spell, max_am);

    for (slot=MIN_WEAR; slot < MAX_WEAR; slot++) {
      if (!victim->slotChance(slot))
        continue;
      if (victim->getCurLimbHealth(slot) >= victim->getMaxLimbHealth(slot))
        continue;
      while (victim->getCurLimbHealth(slot) < victim->getMaxLimbHealth(slot)) {
        if (fixed > max_am)
          break;
        fixed++;
        victim->addCurLimbHealth(slot, 1);
      }
      if (victim->getCurLimbHealth(slot) >= victim->getMaxLimbHealth(slot))
        victim->sendTo(fmt("Your %s has been completely healed.\n\r") %victim->describeBodySlot(slot));
      else
        victim->sendTo(fmt("Your %s has been partially healed.\n\r") %victim->describeBodySlot(slot));
    
      if (fixed > max_am)
        break;
    }
    if (caster != victim) {
      sprintf(buf, "You salve %s of $N's wounds.", (fixed > max_am) ? "some" : "all");
      act(buf, FALSE, caster, NULL, victim, TO_CHAR);
      sprintf(buf, "$n salves %s of $N's wounds.", (fixed > max_am) ? "some" : "all");
      act(buf, TRUE, caster, NULL, victim, TO_NOTVICT);
    } else {
      if (spell != SKILL_WOHLIN) {
        if (!fixed)
          caster->sendTo("You didn't need any salving, dummy.\n\r");
	act("$n salves $s wounds.", TRUE, caster, NULL, victim, TO_ROOM);
      }
    }
    
    if (fixed) {
      caster->reconcileHelp(victim,discArray[spell]->alignMod);
      checkFactionHelp(caster,victim);
    }
    return SPELL_SUCCESS;
  } else {
    if (critFail(caster, spell) == CRIT_F_HITSELF) {
      do {
        slot = wearSlotT(::number(WEAR_HEAD,WEAR_FOOT_L));
      } while (!victim->slotChance(slot));
      if (IS_SET(victim->getLimbFlags(slot), PART_BLEEDING) || 
              (!caster->isLucky(levelLuckModifier(victim->GetMaxLevel())) && 
              !::number(0,2) && 
              (::number(0, victim->plotStat(STAT_CURRENT, STAT_BRA, 50, 100, 75)) >  
              caster->plotStat(STAT_CURRENT, STAT_BRA, 0, 50, 25)))) {
        caster->setLimbFlags(slot, PART_USELESS);
      } else {
        victim->rawBleed(slot, (level * 3) + 100, SILENT_YES, CHECK_IMMUNITY_YES);
      }
      act("$n screams.  Something went terribly wrong.", FALSE, caster, NULL, 0, TO_ROOM);
      act("You scream as something goes terribly wrong.", FALSE, caster, NULL, 0, TO_CHAR);
      return SPELL_CRIT_FAIL;
    }
    caster->deityIgnore();
    return SPELL_FAIL;
  }
}

void salve(TBeing * caster, TBeing * victim)
{
  spellNumT spell = caster->getSkillNum(SPELL_SALVE);

  if (!bPassClericChecks(caster,spell))
    return;

  int level = caster->getSkillLevel(spell);

  int ret=salve(caster,victim,level,caster->getSkillValue(spell), spell);
  if (ret==SPELL_SUCCESS) {
  } else {
  }
}

void salve(TBeing * caster, TBeing * victim, TMagicItem * obj, spellNumT spell)
{
  salve(caster,victim,obj->getMagicLevel(),obj->getMagicLearnedness(), spell);
}

int expel(TBeing * caster, TBeing * victim, int, byte bKnown, spellNumT spell)
{
  char buf[256], limb[256];
  wearSlotT slot;
  TThing *o = NULL;
  int res;

  /* find a slot with something stuck in it */
  for (slot = MIN_WEAR; slot < MAX_WEAR;slot++) {
    if (!victim->slotChance(slot))
      continue;
    if (victim->getStuckIn(slot))
      break;
  }

  if (slot >= MAX_WEAR) {
    act("$N's has no objects in $M to expel!",
          FALSE, caster, NULL, victim, TO_CHAR);
    act("Nothing seems to happen.", FALSE, caster, NULL, NULL, TO_ROOM);
    return SPELL_FAIL;
  }

  if (caster->bSuccess(bKnown, caster->getPerc(), spell)) {
    for (slot = pickRandomLimb();
         ((!victim->slotChance(slot)) ||
          !(o = victim->getStuckIn(slot)));
         slot = pickRandomLimb());

    sprintf(limb, "%s", victim->describeBodySlot(slot).c_str());
    sprintf(buf, "$p is expelled from your %s and falls on the $g!", limb);
    act(buf, FALSE, victim, o, NULL, TO_CHAR);
    sprintf(buf, "$p is expelled from $n's %s and falls on the $g!", limb);
    act(buf, FALSE, victim, o, NULL, TO_ROOM);
    caster->reconcileHelp(victim,discArray[spell]->alignMod);
    checkFactionHelp(caster,victim);
    *(caster->roomp) += *(victim->pulloutObj(slot, TRUE, &res));
    int rc = o->checkSpec(caster, CMD_OBJ_EXPELLED, "", NULL);
    if (IS_SET_DELETE(rc, DELETE_THIS)) {
      delete o;
      o = NULL;
    }
    return SPELL_SUCCESS;
  } else {
    act("Nothing seems to happen.", TRUE, caster, 0, 0, TO_ROOM);
    return SPELL_FAIL;
  }
}

void expel(TBeing * caster, TBeing * victim)
{
  spellNumT spell = caster->getSkillNum(SPELL_EXPEL);

  if (!bPassClericChecks(caster,spell))
    return;

  int level = caster->getSkillLevel(spell);

  int ret=expel(caster,victim,level,caster->getSkillValue(spell), spell);
  if (ret==SPELL_SUCCESS) {
  } else {
  }
}

void expel(TBeing * caster, TBeing * victim, TMagicItem * obj, spellNumT spell)
{
  expel(caster,victim,obj->getMagicLevel(),obj->getMagicLearnedness(), spell);
}

