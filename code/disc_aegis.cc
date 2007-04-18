#include "stdsneezy.h"
#include "disease.h"
#include "combat.h"
#include "disc_aegis.h"
#include "obj_magic_item.h"
#include "obj_player_corpse.h"

void relive(TBeing *ch, TBeing *vict)
{
  affectedData aff;
  TThing *t;
  TPCorpse *corpse=NULL;
  sstring s;
  TObj *o;

  if (!bPassClericChecks(ch,SPELL_RELIVE))
    return;

  // need to be holding angel heart
  o=dynamic_cast<TObj *>(ch->equipment[HOLD_LEFT]);
  if(!o || o->objVnum() != OBJ_ANGEL_HEART){
    ch->sendTo("You lack an appropriate object to focus this prayer with.\n\r");
    return;
  }
  

  // locate corpse
  for(t=ch->roomp->getStuff();t;t=t->nextThing){
    if((corpse=dynamic_cast<TPCorpse *>(t)) &&
       corpse->getOwner().lower() == sstring(vict->getName()).lower())
	break;
  }

  if(!corpse){
    act("You could not find an appropriate corpse.",
	FALSE, ch, NULL, vict, TO_CHAR);
    return;
  }
  
  if (ch->bSuccess(ch->getSkillValue(SPELL_RELIVE), ch->getPerc(), SPELL_RELIVE)) {
    
    act("Images rapidly flash through your mind as you relive the experiences of your corpse.",
	FALSE, ch, NULL, vict, TO_VICT);
    act("$N looks dazed as $n completes $s prayer.",
	FALSE, ch, NULL, vict, TO_NOTVICT);
    act("$N looks dazed as you complete your prayer.",
	FALSE, ch, NULL, vict, TO_CHAR);
    
    
    int exp_perc=::number(1,25);  // 1-25% random chance  
    exp_perc += ch->getSkillValue(SPELL_RELIVE)/4;  // 1-25% based on skill
    vict->addToExp((corpse->getExpLost() * exp_perc)/100);
    vict->doSave(SILENT_YES);


    aff.type = SPELL_RELIVE;
    aff.duration = 24 * UPDATES_PER_MUDHOUR;
    aff.modifier = -50;
    aff.location = APPLY_STR;
    vict->affectJoin(ch, &aff, AVG_DUR_NO, AVG_EFF_YES);
    aff.location = APPLY_CON;
    vict->affectJoin(ch, &aff, AVG_DUR_NO, AVG_EFF_YES);
    aff.location = APPLY_AGI;
    vict->affectJoin(ch, &aff, AVG_DUR_NO, AVG_EFF_YES);
    aff.location = APPLY_DEX;
    vict->affectJoin(ch, &aff, AVG_DUR_NO, AVG_EFF_YES);
    aff.location = APPLY_SPE;
    vict->affectJoin(ch, &aff, AVG_DUR_NO, AVG_EFF_YES);

    act("The ordeal has left you feeling aged.",
	FALSE, ch, NULL, vict, TO_VICT);

    corpse->objectDecay();
    delete corpse;
  } else {
    act("You are unable to convince $d to let $P relive $s experiences.", 
	FALSE, ch, NULL, vict, TO_CHAR);
    ch->deityIgnore(SILENT_YES);
    return;
  }
}

int cureBlindness(TBeing *c, TBeing * victim, int level, byte learn)
{
  affectedData * aff;  // pointer declaration is an exception to this rule

  if (!victim->isAffected(AFF_BLIND)) {
    act("Nothing seems to happen.  Try someone who is blind.", FALSE, c, NULL, victim, TO_CHAR);
    c->deityIgnore(SILENT_YES);
    return FALSE;
  }

  if (c->bSuccess(learn, c->getPerc(), SPELL_CURE_BLINDNESS)) {
    if (victim->affected) {
      for (aff = victim->affected; aff; aff = aff->next) {
        if ((aff->type == AFFECT_DISEASE) && (aff->modifier == DISEASE_EYEBALL)) {
            victim->sendTo(
   "You are blind because you have no eyeballs.  Fix that problem first.\n\r");
          return SPELL_FAIL;
        }
      }
    }
    victim->affectFrom(SPELL_BLINDNESS);
    c->reconcileHelp(victim,discArray[SPELL_CURE_BLINDNESS]->alignMod);
    checkFactionHelp(c,victim);
    return SPELL_SUCCESS;
  } else {
    int duration = (level/10 + 1) * UPDATES_PER_MUDHOUR;
    duration = (int) (c->percModifier() * duration);

    switch (critFail(c, SPELL_CURE_BLINDNESS)) {
      case CRIT_F_HITOTHER:
      case CRIT_F_HITSELF:
        CF(SPELL_CURE_BLINDNESS);
        c->rawBlind(level, duration, SAVE_YES);
        return SPELL_CRIT_FAIL;
      default:
        return SPELL_FAIL;
    }
    return SPELL_FAIL;
  }
}

void cureBlindness(TBeing *c, TBeing * victim, TMagicItem * obj)
{
  int ret = 0;
  ret=cureBlindness(c,victim,obj->getMagicLevel(), obj->getMagicLearnedness());
  if (ret==SPELL_SUCCESS) {
    act("$n's vision has been thankfully restored by $p!",
          FALSE, victim, obj, NULL, TO_ROOM);
    act("Your vision has been thankfully restored by $p!",
          FALSE, victim, obj, NULL, TO_CHAR);
  } else if (ret==SPELL_CRIT_FAIL) {
    act("Ack, $p has blinded you instead of curing $N!",
          FALSE, c, obj, victim, TO_CHAR);
    act("$p has blinded $n instead of curing $N!",
          FALSE, c, NULL, victim, TO_ROOM);
    act("$p has blinded $mself instead of curing you!",
          FALSE, c, NULL, victim, TO_VICT);
  } else if (ret==SPELL_FAIL) {
    act("$d ignores you.", FALSE, c, NULL, 0, TO_CHAR);
     if (c != victim) {
       act("$p fails to restore your eyesight.", FALSE, c, obj, victim, TO_VICT);
       act("$p fails to restore $N's eyesight.", FALSE, c, obj, victim, TO_CHAR);
       act("$p fails to restore $N's eyesight.", FALSE, c, obj, victim, TO_ROOM);
     } else {
       act("$p fails to restore your eyesight.", FALSE, c, obj, victim, TO_CHAR);
       act("$p fails to restore $n's eyesight.", FALSE, c, obj, 0, TO_ROOM);
     } 
  } else {
  }
}

void cureBlindness(TBeing *c, TBeing *victim)
{
  int ret,level;

  if (!bPassClericChecks(c,SPELL_CURE_BLINDNESS))
    return;

  level = c->getSkillLevel(SPELL_CURE_BLINDNESS);

  ret=cureBlindness(c,victim,level, c->getSkillValue(SPELL_CURE_BLINDNESS));
  if (ret==SPELL_SUCCESS) {
    act("$n's vision has been thankfully restored!",
          FALSE, victim, NULL, NULL, TO_ROOM);
    act("Your vision has been thankfully restored!",
          FALSE, victim, NULL, NULL, TO_CHAR);
  } else if (ret == SPELL_CRIT_FAIL) { 
    act("Ack, you have blinded yourself instead of curing $N!",
          FALSE, c, NULL, victim, TO_CHAR);
    act("$n has blinded $mself instead of curing $N!",
          FALSE, c, NULL, victim, TO_ROOM);
  } else  if (ret == SPELL_FAIL) {
    c->deityIgnore();
  } else {
  }
}

int cureDisease(TBeing *caster, TBeing * victim, int, byte learn, spellNumT spell)
{
  char buf[256];
  bool found = FALSE;
  diseaseTypeT disease = DISEASE_NULL;
  int s1 = 0, u1 = 0, mod;
  int i;
  
  if (spell == SKILL_WOHLIN || 
      caster->bSuccess(learn, caster->getPerc(), spell)) {
  
    for (i=1; i <= 10; i++) {
      switch (i) {
        case 1:
          disease = DISEASE_COLD;
          s1 = 1;
          u1 = 5;
          break;
        case 2:
          disease = DISEASE_DYSENTERY;
          s1 = 10;
          u1 = 5;
          break;
        case 3:
          disease = DISEASE_FLU;
          s1 = 15;
          u1 = 4;
          break;
        case 4:
          disease = DISEASE_PNEUMONIA;
          s1 = 40;
          u1 = 3;
          break;
        case 5:
          disease = DISEASE_SCURVY;
          s1 = 30;
          u1 = 4;
          break;
        case 6:
          disease = DISEASE_FROSTBITE;
          s1 = 30;
          u1 = 4;
          break;
        case 7:
          disease = DISEASE_LEPROSY;
          s1 = 50;
          u1 = 3;
          break;
        case 8:
          disease = DISEASE_GANGRENE;
          s1 = 75;
          u1 = 2;
          break;
        case 9:
          disease = DISEASE_SYPHILIS;
          s1 = 75;
          u1 = 2;
          break;
        case 10:
          disease = DISEASE_PLAGUE;
          s1 = 75;
          u1 = 2;
          break;
      }
      if (victim->hasDisease(disease)) {
        mod = min(((learn - s1 + 1) * u1), (int) MAX_SKILL_LEARNEDNESS);
        found = TRUE;
  
        if (::number(1,100) > mod && spell != SKILL_WOHLIN) {
          sprintf(buf, "$d fails to hear your plea to cure %s.", DiseaseInfo[disease].name);
          act(buf, FALSE, caster, 0, 0, TO_CHAR);
          sprintf(buf, "$d fails to hear $s pleas.");
          act(buf, FALSE, caster, 0, 0, TO_ROOM);
        } else {
          if (caster == victim) {
            sprintf(buf, "You remove %s inflicting you.", DiseaseInfo[disease].name);
            act(buf, FALSE, caster, 0, 0, TO_CHAR);
            sprintf(buf, "$n removes %s inflicting $m.", DiseaseInfo[disease].name);
            act(buf, FALSE, caster, 0, 0, TO_ROOM);
          } else {
            sprintf(buf, "You remove %s inflicting $N.", DiseaseInfo[disease].name);
            act(buf, FALSE, caster, 0, victim, TO_CHAR);
            sprintf(buf, "$n removes %s inflicting you.", DiseaseInfo[disease].name);
            act(buf, FALSE, caster, 0, victim, TO_VICT);
            sprintf(buf, "$n removes %s inflicting $N.", DiseaseInfo[disease].name);
            act(buf, FALSE, caster, 0, victim, TO_NOTVICT);
          }
          victim->diseaseFrom(disease);

          // only fix one disease per entry for monks
          if (spell == SKILL_WOHLIN)
            break;
        }
      }
    }

    if ((spell != SKILL_WOHLIN) || !found)
      for (wearSlotT iLimb = MIN_WEAR; iLimb < MAX_WEAR; iLimb++) {
        if (iLimb == HOLD_RIGHT || iLimb == HOLD_LEFT)
          continue;

        if (!victim->slotChance(iLimb))
          continue;
	  
        if (victim->isLimbFlags(iLimb, PART_GANGRENOUS)) {
          // 10% chance max
          mod = min(((learn - 50 + 1) * 3), (int) MAX_SKILL_LEARNEDNESS);
          found = TRUE;
          if (::number(1, 1000) > mod) {
            if (spell != SKILL_WOHLIN) {
              act("$d fails to hear your plea to cure the gangrene!", FALSE, caster, 0, 0, TO_CHAR);
              act("$d fails to hear $n's plea.", FALSE, caster, 0, 0, TO_ROOM);
            }
          } else {
            if (caster == victim) {
              act("You remove the gangrene inflicting you.", FALSE, caster, 0, 0, TO_CHAR);
              act("$m removes the gangrene inflicting $m.", FALSE, caster, 0, 0, TO_ROOM);
            } else {
              act("You remove the gangrene inflicting $N.", FALSE, caster, 0, victim, TO_CHAR);
              act("$n removes the gangrene inflicting you.", FALSE, caster, 0, victim, TO_VICT);
              act("$n removes the gangrene inflicting $N.", FALSE, caster, 0, victim, TO_NOTVICT);
            }
            victim->remLimbFlags(iLimb, PART_GANGRENOUS);
            if (spell == SKILL_WOHLIN)
              break;
          }
        }
		
        if (victim->isLimbFlags(iLimb, PART_LEPROSED)) {
          mod = min(((learn - 50 + 1) * 3), (int)MAX_SKILL_LEARNEDNESS);
          found = TRUE;

          // While we now allow this we do not want it easy to do.  10% chance at most, sounds good to me. -Lapsos
          if (::number(1, 1000) > mod) {
            if (spell != SKILL_WOHLIN) {
              act("$d fails to hear your plea to cure the leprosy!", FALSE, caster, 0, 0, TO_CHAR);
              act("$d fails to hear $n's plea.", FALSE, caster, 0, 0, TO_ROOM);
            }
          } else {
            if (caster == victim) {
              act("You remove the leprosy inflicting you.", FALSE, caster, 0, 0, TO_CHAR);
              act("$m removes the leprosy inflicting $m.", FALSE, caster, 0, 0, TO_ROOM);
            } else {
              act("You remove the leprosy inflicting $N.", FALSE, caster, 0, victim, TO_CHAR);
              act("$n removes the leprosy inflicting you.", FALSE, caster, 0, victim, TO_VICT);
              act("$n removes the leprosy inflicting $N.", FALSE, caster, 0, victim, TO_NOTVICT);
            }

            victim->remLimbFlags(iLimb, PART_LEPROSED);

            if (spell == SKILL_WOHLIN)
              break;
          }
        }
      }

    if (!found && spell!=SKILL_WOHLIN) {
      act("$d wisely ignores you.", FALSE, caster, NULL, 0, TO_CHAR);
      act("Do you normally go around curing diseases on unafflicted people?",
            FALSE, caster, 0, 0, TO_CHAR);
      caster->deityIgnore(SILENT_YES);
      return SPELL_FALSE;
    }
    return SPELL_SUCCESS;
  } else {

    caster->deityIgnore();
    return SPELL_FAIL;
  }
}

void cureDisease(TBeing *c, TBeing *victim)
{
  spellNumT spell = c->getSkillNum(SPELL_CURE_DISEASE);

  if (!bPassClericChecks(c,spell))
    return;

  int level = c->getSkillLevel(spell);

  int ret=cureDisease(c,victim,level, c->getSkillValue(spell), spell);
  if (ret==SPELL_SUCCESS) {
 } else if (ret==SPELL_FAIL) {
 }
}

void cureDisease(TBeing *c, TBeing * victim, TMagicItem * obj, spellNumT spell)
{
  int ret=cureDisease(c,victim, obj->getMagicLevel(),obj->getMagicLearnedness(), spell);
  if (ret==SPELL_SUCCESS) {
  } else if (ret==SPELL_FAIL) {
  } else {
  }
}

int curePoison(TBeing *c, TBeing * victim, int level, byte learn, spellNumT spell)
{
  char buf[256];
  affectedData aff;

  if (spell==SKILL_WOHLIN || c->bSuccess(learn, c->getPerc(), spell)) {
    if (victim->isAffected(AFF_POISON) || 
        victim->affectedBySpell(SPELL_POISON) ||
        victim->affectedBySpell(SPELL_POISON_DEIKHAN) ||
        victim->hasDisease(DISEASE_FOODPOISON)) {
      sprintf(buf, "You succeed in extracting the poison from %s body.", (c == victim) ? "your" : "$N's");
      act(buf, FALSE, c, NULL, victim, TO_CHAR);
      if (c != victim)
        act("You suddenly start sweating profusely as the poison is extracted from your body.", FALSE, c, NULL, victim, TO_VICT);
      act("$N suddenly starts sweating profusely as the poison is extracted from $S body.", FALSE, c, NULL, victim, TO_NOTVICT);
      act("Your system has been relieved of the poison that once wracked your veins.", FALSE, victim, NULL, NULL, TO_CHAR);
      victim->affectFrom(SPELL_POISON_DEIKHAN);
      victim->affectFrom(SPELL_POISON);
      victim->diseaseFrom(DISEASE_FOODPOISON);
      victim->diseaseFrom(DISEASE_POISON);
      c->reconcileHelp(victim,discArray[spell]->alignMod);
      checkFactionHelp(c,victim);
    } else {
      if(spell==SKILL_WOHLIN)
	return FALSE;
      act("Do you often go around removing poison from people that aren't poisoned?", FALSE, c, NULL, NULL, TO_CHAR);
      c->deityIgnore(SILENT_YES);
      return FALSE;
    }
    return SPELL_SUCCESS;
  } else {
    switch (critFail(c, spell)) {
      case CRIT_F_HITOTHER:
      case CRIT_F_HITSELF:
        CF(spell);
        aff.type = SPELL_POISON;
        aff.level = level;
        aff.duration = (aff.level << 1) * UPDATES_PER_MUDHOUR;
        aff.modifier = -20;
        aff.location = APPLY_STR;
        aff.bitvector = AFF_POISON;
        c->affectJoin(c, &aff, AVG_DUR_NO, AVG_EFF_YES);
        return SPELL_CRIT_FAIL;
      default:
        break;
    }
    return SPELL_FAIL;
  }
}

void curePoison(TBeing *c, TBeing * victim, TMagicItem * obj, spellNumT spell)
{
  int ret=curePoison(c,victim, obj->getMagicLevel(),obj->getMagicLearnedness(),spell);
  if (ret==SPELL_SUCCESS) {
  } else if (ret==SPELL_CRIT_FAIL) {
      act("Ack, $p has backfired!",
            FALSE, c, obj, NULL, TO_CHAR);
      act("$n seems to have poisoned $mself!",
            FALSE, c, obj, c, TO_NOTVICT);
      act("Oh no! You seem to have poisoned yourself!",
            FALSE, c, obj, NULL, TO_CHAR);
  } else if (ret==SPELL_FAIL) {
    c->deityIgnore();
  } else {
  }
}

void curePoison(TBeing *c, TBeing *victim)
{
  spellNumT spell = c->getSkillNum(SPELL_CURE_POISON);

  if (!bPassClericChecks(c,spell))
    return;

  int level = c->getSkillLevel(spell);

  int ret=curePoison(c,victim,level, c->getSkillValue(spell), spell);
  if (ret==SPELL_SUCCESS) {
  } else if (ret==SPELL_CRIT_FAIL) {
      c->spellMessUp(spell);
      act("$n seems to have poisoned $mself!", 
            FALSE, c, NULL, c, TO_NOTVICT);
      act("Oh no! You seem to have poisoned yourself!", 
            FALSE, c, NULL, NULL, TO_CHAR);
   } else if (ret==SPELL_FAIL) {
    c->deityIgnore();
   } else {
   }
}

int refresh(TBeing *c, TBeing * victim, int level, byte learn, spellNumT spell)
{
  int retCode;

  int dam = ::number(10, level); 
  dam = max(dam, 15);
  dam = min(dam, 35);

  if (c->bSuccess(learn, c->getPerc(), spell)) {
    LogDam(c,spell, dam);

    switch (critSuccess(c, spell)) {
      case CRIT_S_DOUBLE:
        CS(spell);
        dam *= 2;
        retCode = SPELL_CRIT_SUCCESS;
        break;
      default:
        retCode = SPELL_SUCCESS;
        break;
    }
    dam = min(dam, victim->moveLimit() - victim->getMove());
    LogDam(c, spell, dam);
    victim->addToMove(dam);

    victim->updatePos();
    c->reconcileHelp(victim,discArray[spell]->alignMod);
    checkFactionHelp(c,victim);
    return retCode;
  } else {
    switch(critFail(c, spell)) {
      case CRIT_F_HITOTHER:
      case CRIT_F_HITSELF:
        CF(spell);
        dam >>= 1;
        c->addToMove(-dam);
        return SPELL_CRIT_FAIL;
        break;
      default:
        return SPELL_FAIL;
        break;
    }
  }
}

void refresh(TBeing *c, TBeing * victim, TMagicItem * obj, spellNumT spell)
{
  int ret=refresh(c,victim,obj->getMagicLevel(),obj->getMagicLearnedness(), spell);
  if (ret==SPELL_SUCCESS) {

    if (c == victim) {
      act("You use $p to restore some of your spent energy!",
          FALSE, c, obj, 0, TO_CHAR);
      act("$n has used $p and seems revitalized.", 
          TRUE, c, obj, NULL, TO_ROOM);
    } else {
      act("You use $p to in restore some of $N's spent energy!",
          FALSE, c, obj, victim, TO_CHAR);
      act("$n uses $p to restore some of your spent energy!",
          FALSE, c, obj, victim, TO_VICT);
      act("$n uses $p and $N seems revitalized.", 
          TRUE, c, obj, victim, TO_NOTVICT);
    }
  } else if (ret==SPELL_CRIT_SUCCESS) {
    act("$p restores a lot of $n's spent energy!",
         FALSE, victim, obj, 0, TO_ROOM);
    act("$p restores a lot of your spent energy!",
         FALSE, victim, obj, 0, TO_CHAR);
  } else if (ret==SPELL_CRIT_FAIL) {
      act("Something went wrong with $p.",
          FALSE, c, obj, NULL, TO_CHAR);
      act("You feel your own energy draining away!",
          FALSE, c, obj, NULL, TO_CHAR);
      act("$p seems to have weakened $n!",
          FALSE, c, obj, NULL, TO_ROOM);
  } else if (ret==SPELL_FAIL) { 
    c->deityIgnore();
  }
  return;
}

void refresh(TBeing *c, TBeing * victim)
{
  spellNumT spell = c->getSkillNum(SPELL_REFRESH);
 
  if (!bPassClericChecks(c,spell))
    return;

  int level = c->getSkillLevel(spell);

  int ret=refresh(c,victim,level, c->getSkillValue(spell), spell);
  if (ret==SPELL_SUCCESS) {
    if (c == victim) {
      act("You succeed in restoring some of your spent energy!", 
         FALSE, c, NULL, 0, TO_CHAR);
      act("$n seems revitalized.", FALSE, c, NULL, NULL, TO_ROOM);
    } else {
      act("You succeed in restoring some of $N's spent energy!", 
         FALSE, c, NULL, victim, TO_CHAR);
      act("$n restores some of your spent energy!", 
         FALSE, c, NULL, victim, TO_VICT);
      act("$N seems revitalized.", FALSE, c, NULL, victim, TO_NOTVICT);
    }
  } else if (ret==SPELL_CRIT_SUCCESS) {
    if (c == victim) {
      act("You succeed in restoring a lot of your spent energy!", 
         FALSE, c, NULL, 0, TO_CHAR);
      act("$n seems very revitalized.", FALSE, c, NULL, NULL, TO_ROOM);
    } else {
      act("You succeed in restoring a lot of $N's spent energy!",
         FALSE, c, NULL, victim, TO_CHAR);
      act("$n restores a lot of your spent energy!",
         FALSE, c, NULL, victim, TO_VICT);
      act("$N seems very revitalized.", FALSE, c, NULL, victim, TO_NOTVICT);
    }
  } else if (ret==SPELL_CRIT_FAIL) {
      c->spellMessUp(spell);
      act("You feel your own energy draining away!", 
          FALSE, c, NULL, NULL, TO_CHAR);
      act("$n seems to have accidentally weakened $mself!", 
          FALSE, c, NULL, NULL, TO_ROOM);
  } else if (ret == SPELL_FAIL) { 
    c->deityIgnore();
  }
  return;
}

int secondWind(TBeing *c, TBeing *victim, int level, byte learn)
{
  int dam = 0;

  dam = dice(level, 3)+ 20;
  dam = max(dam, 40);
  dam = min(dam, 110);

  if (c->bSuccess(learn, c->getPerc(), SPELL_SECOND_WIND)) {
    switch (critSuccess(c, SPELL_SECOND_WIND)) {
      case CRIT_S_DOUBLE:
        CS(SPELL_SECOND_WIND);
        dam *= 2;
        dam = min(dam, victim->moveLimit() - victim->getMove());

        LogDam(c, SPELL_SECOND_WIND, dam);
        victim->addToMove(dam);

        victim->updatePos();
        c->reconcileHelp(victim,discArray[SPELL_SECOND_WIND]->alignMod);
        checkFactionHelp(c,victim);
        return SPELL_CRIT_SUCCESS;
      default:
        break;
    }

    dam = min(dam, victim->moveLimit() - victim->getMove());
    LogDam(c, SPELL_SECOND_WIND, dam);
    victim->addToMove(dam);

    victim->updatePos();
    c->reconcileHelp(victim,discArray[SPELL_SECOND_WIND]->alignMod);
    checkFactionHelp(c,victim);
    return SPELL_SUCCESS;
  } else {
    switch(critFail(c, SPELL_SECOND_WIND)) {
      case CRIT_F_HITOTHER:
      case CRIT_F_HITSELF:
        CF(SPELL_SECOND_WIND);
        dam >>= 1;
        c->addToMove(-dam);
        return SPELL_CRIT_FAIL;
      default:
        break;
    }
    return SPELL_FAIL;
  }
}

void secondWind(TBeing *c, TBeing * victim, TMagicItem * obj)
{
  int ret = 0;
  ret=secondWind(c,victim,obj->getMagicLevel(),obj->getMagicLearnedness());
  if (ret==SPELL_SUCCESS) {
    act("$n's face begins to glow!",
       FALSE, victim, obj, 0, TO_ROOM);
    act("Your skin begins to tingle!",
      FALSE, victim, obj, 0, TO_CHAR);
    act("$p succeeds in restoring some of $n's spent energy!",
      FALSE, victim, obj, 0, TO_ROOM);
    act("$p restores some of your spent energy!",
      FALSE, victim, obj, 0, TO_CHAR);
  } else if (ret==SPELL_CRIT_SUCCESS) {
     act("$n's face begins to glow brightly!",
      FALSE, victim, obj, 0, TO_ROOM);
     act("Your skin begins to tingle strongly!",
      FALSE, victim, obj, 0, TO_CHAR);
     act("$p succeeds in restoring a lot of $n's spent energy!",
          FALSE, victim, obj, 0, TO_ROOM);
     act("$p restores a lot of your spent energy!",
      FALSE, victim, obj, 0, TO_CHAR);
  } else if (ret==SPELL_CRIT_FAIL) {
     act("Something went wrong with $p.",
          FALSE, c, obj, NULL, TO_CHAR);
     act("You feel your own energy draining away!",
          FALSE, c, obj, NULL, TO_CHAR);
     act("$p seems to have weakened $n!",
          FALSE, c, obj, NULL, TO_ROOM);
  } else if (ret==SPELL_FAIL) {
    c->deityIgnore();
  } else {
  }
}

void secondWind(TBeing *c, TBeing * victim)
{
  int ret,level;

  if (!bPassClericChecks(c,SPELL_SECOND_WIND))
    return;

  level = c->getSkillLevel(SPELL_SECOND_WIND);

  ret=secondWind(c,victim,level, c->getSkillValue(SPELL_SECOND_WIND));
  if (ret==SPELL_SUCCESS) {
    act("$n's face begins to glow!",
          FALSE, victim, 0, 0, TO_ROOM);
    act("Your skin begins to tingle!",
          FALSE, victim, 0, 0, TO_CHAR);
    act("You succeed in restoring some of $N's spent energy!", 
          FALSE, c, NULL, victim, TO_CHAR);
    act("$n restores some of your spent energy!", 
          FALSE, c, NULL, victim, TO_VICT);
    act("$N seems to have caught a second wind.", 
          FALSE, c, NULL, victim, TO_NOTVICT);
  } else if (ret==SPELL_CRIT_SUCCESS) {
    act("$n's face begins to glow brightly!",
          FALSE, victim, 0, 0, TO_ROOM);
    act("Your skin begins to tingle strongly!",
          FALSE, victim, 0, 0, TO_CHAR);
    act("You succeed in restoring a lot of $N's spent energy!",
          FALSE, c, NULL, victim, TO_CHAR);
    act("$n restores a lot of your spent energy!",
          FALSE, c, NULL, victim, TO_VICT);
    act("$N seems to have caught a second and third wind.",
          FALSE, c, NULL, victim, TO_NOTVICT);
  } else if (ret==SPELL_CRIT_FAIL) {
      c->spellMessUp(SPELL_SECOND_WIND);
      act("You feel your own energy draining away!", 
          FALSE, c, NULL, NULL, TO_CHAR);
      act("$n seems to have accidentally weakened $mself!", 
          FALSE, c, NULL, NULL, TO_ROOM);
  } else if (ret==SPELL_FAIL) {
    c->deityIgnore();
  } else {
  }
}

int cureParalysis(TBeing *c, TBeing * victim, int level, byte learn)
{
  affectedData aff;

  if (c->bSuccess(learn, c->getPerc(), SPELL_CURE_PARALYSIS)) {
    if (victim->isAffected(AFF_PARALYSIS)) {
      act("You succeed in restoring $N's body to mobility!", FALSE, c, NULL, victim, TO_CHAR);
      act("$N's body constricts violently and then relaxes -- $E can move again!", FALSE, c, NULL, victim, TO_NOTVICT);
      act("You feel a great wrenching followed by great relief! Once again, you are free to move.", FALSE, victim, NULL, NULL, TO_CHAR);
      victim->affectFrom(SPELL_PARALYZE);
      c->reconcileHelp(victim,discArray[SPELL_CURE_PARALYSIS]->alignMod);
      checkFactionHelp(c,victim);
    } else {
      act("Do you often go around removing the paralysis from people that aren't paralyzed?", FALSE, c, NULL, NULL, TO_CHAR);
      c->deityIgnore(SILENT_YES);
    }
    return SPELL_SUCCESS;
  } else {
    switch(critFail(c, SPELL_CURE_PARALYSIS)) {
      case CRIT_F_HITOTHER:
      case CRIT_F_HITSELF:
        CF(SPELL_CURE_PARALYSIS);
        aff.type = SPELL_PARALYZE;
        aff.level = level;
        aff.location = APPLY_NONE;
        aff.bitvector = AFF_PARALYSIS;
        aff.duration = number(1, level);
        aff.modifier = 0;
        aff.duration = (int) (c->percModifier() * aff.duration);
        c->affectJoin(c, &aff, AVG_DUR_NO, AVG_EFF_YES);
        c->setPosition(POSITION_STUNNED);
        return SPELL_CRIT_FAIL;
        break;
      default:
        break;
    }
    return SPELL_FAIL;
  }
}


void cureParalysis(TBeing *c, TBeing * victim, TMagicItem * obj)
{
  int ret=cureParalysis(c,victim, obj->getMagicLevel(),obj->getMagicLearnedness());
  if (ret==SPELL_SUCCESS) {
  } else if ((ret=SPELL_CRIT_FAIL)) {
    act("Ack, $p has backfired!",
                 FALSE, c, obj, NULL, TO_CHAR);
    act("$n seems to have paralyzed $mself!", 
                 FALSE, c, NULL, c, TO_NOTVICT);
    act("Oh no! You seem to have paralyzed yourself!", 
                 FALSE, c, NULL, NULL, TO_CHAR);
  } else if (ret==SPELL_FAIL) {
    c->deityIgnore();
  } else {
  }
}


void cureParalysis(TBeing *c, TBeing * victim)
{
  int ret,level;

  if (!bPassClericChecks(c,SPELL_CURE_PARALYSIS))
    return;

  level = c->getSkillLevel(SPELL_CURE_PARALYSIS);

  ret=cureParalysis(c,victim,level, c->getSkillValue(SPELL_CURE_PARALYSIS));
  if (ret==SPELL_SUCCESS) {
  } else if (ret==SPELL_CRIT_FAIL) {
    c->spellMessUp(SPELL_CURE_PARALYSIS);
    act("$n seems to have paralyzed $mself!", 
          FALSE, c, NULL, c, TO_NOTVICT);
    act("Oh no! You seem to have paralyzed yourself!", 
          FALSE, c, NULL, NULL, TO_CHAR);
  } else if (ret==SPELL_FAIL) { 
    c->deityIgnore();
  } else {
  }
}

int TBeing::removeCurseObj(TObj * obj, int, byte learn, spellNumT spell)
{
  if (!obj->isObjStat(ITEM_NODROP)) {
    act("Do you often go around removing curses from items that aren't cursed?", FALSE, this, NULL, NULL, TO_CHAR);
    deityIgnore(SILENT_YES);
    return SPELL_FALSE;
  }

  if (bSuccess(learn, getPerc(), spell)) {
    act("$p glows with a flash of bright white light!",
         FALSE, this, obj, NULL, TO_CHAR);
    act("$p glows with a flash of bright white light!",
         FALSE, this, obj, NULL, TO_ROOM);

    act("You succeed in chasing the demon spirit from $p.", FALSE, this, obj, NULL, TO_CHAR);
    act("$n chases the demon spirit from $p.", FALSE, this, obj, NULL, TO_ROOM);
    obj->remObjStat(ITEM_NODROP);
    act("The presence of that evil spirit may have caused some structural damage.", FALSE, this, obj, NULL, TO_CHAR);
    return SPELL_SUCCESS;
  } else {
    return SPELL_FAIL;
  }
}

void TBeing::removeCurseObj(TObj * target, TMagicItem * obj, spellNumT spell)
{
  int ret;

  ret=removeCurseObj(target,obj->getMagicLevel(),obj->getMagicLearnedness(), spell);
  if (ret == SPELL_SUCCESS) {
  } else if (ret == SPELL_FAIL) {
    deityIgnore();
  }
}

void TBeing::removeCurseObj(TObj * obj)
{
  spellNumT spell = getSkillNum(SPELL_REMOVE_CURSE);

  if (!bPassClericChecks(this,spell))
    return;

  int level = getSkillLevel(spell);

  int ret=removeCurseObj(obj,level, getSkillValue(spell), spell);
  if (ret== SPELL_SUCCESS) {
  } else if (ret == SPELL_FAIL) {
    deityIgnore();
  } else {
  }
}

int TBeing::removeCurseBeing(TBeing * victim, int level, byte learn, spellNumT spell)
{
  affectedData aff;
  char buf[256];

  if (!victim->affectedBySpell(SPELL_CURSE) &&
      !victim->affectedBySpell(SPELL_CURSE_DEIKHAN)) {
    // victim wasn't cursed directly, see if using cursed item
    int i;
    for (i=MIN_WEAR;i < MAX_WEAR; i++) {
      if (!victim->equipment[i])
        continue;
      TObj * obj = dynamic_cast<TObj *>(victim->equipment[i]);
      if (!obj || !obj->isObjStat(ITEM_NODROP))
        continue;
      return removeCurseObj(obj, level, learn, spell);
    }
    act("Do you often go around removing curses from people that aren't cursed?", FALSE, this, NULL, NULL, TO_CHAR);
    deityIgnore(SILENT_YES);
    return SPELL_FALSE;
  }

  if (bSuccess(learn, getPerc(), spell)) {
    act("$n glows with a flash of bright white light!",
             FALSE, victim, NULL, NULL, TO_ROOM);
    act("You glow with a flash of bright white light!",
             FALSE, victim, NULL, NULL, TO_CHAR);

    sprintf(buf, "You succeed in exorcising the demon spirit from %s body.", (this == victim) ? "your" : "$N's");
    act(buf, FALSE, this, NULL, victim, TO_CHAR);
    if (this != victim)
      act("$n forces the demon spirit from your body.", FALSE, this, NULL, victim, TO_VICT);

    sprintf(buf, "$n forces the demon spirits from %s body.", (this == victim) ? "$s" : "$N's");
    act(buf, FALSE, this, NULL, victim, TO_NOTVICT);
    act("You feel much better, not to mention much relieved.", 
            FALSE, victim, NULL, NULL, TO_CHAR);
    if (victim->affectedBySpell(SPELL_CURSE))
      victim->affectFrom(SPELL_CURSE);
    if (victim->affectedBySpell(SPELL_CURSE_DEIKHAN))  
      victim->affectFrom(SPELL_CURSE_DEIKHAN);

    return SPELL_SUCCESS;
  } else {
    if (critFail(this, spell)) {
      CF(spell);

      aff.type = SPELL_CURSE;
      aff.level = level;
      aff.duration = 24 * 3 * UPDATES_PER_MUDHOUR; // 3 Days 
      aff.modifier = -10;
      aff.location = APPLY_SPELL_HITROLL;
      aff.bitvector = AFF_CURSE;
      affectJoin(this, &aff, AVG_DUR_NO, AVG_EFF_YES);

      aff.location = APPLY_IMMUNITY;
      aff.modifier = IMMUNE_CHARM;              // Make worse 
      aff.modifier2 = -15;
      affectJoin(this, &aff, AVG_DUR_NO, AVG_EFF_YES);
      return SPELL_CRIT_FAIL;
    }
    return SPELL_FAIL;
  }
}

void TBeing::removeCurseBeing(TBeing * victim, TMagicItem * obj, spellNumT spell)
{
  int ret;

  ret = removeCurseBeing(victim,obj->getMagicLevel(),obj->getMagicLearnedness(), spell);

  if (ret==SPELL_CRIT_FAIL) {
    act("Ack! $p has backfired!", FALSE, this, obj, NULL, TO_CHAR);
    act("$n seems to have cursed $mself!", FALSE, this, NULL, NULL, TO_ROOM);
    act("You seem to have cursed yourself!", FALSE, this, NULL, NULL, TO_CHAR);
  } else if (ret==SPELL_FAIL) 
    deityIgnore();
}

void TBeing::removeCurseBeing(TBeing * victim)
{
  int ret,level;

  spellNumT spell = getSkillNum(SPELL_REMOVE_CURSE);

  if (!bPassClericChecks(this,spell))
    return;

  level = getSkillLevel(spell);

  ret = removeCurseBeing(victim,level, getSkillValue(spell), spell);

  if (ret==SPELL_CRIT_FAIL) {
    spellMessUp(spell);
    act("$n seems to have cursed $mself!", FALSE, this, NULL, NULL, TO_ROOM);
    act("Oh no! You seem to have cursed yourself!", FALSE, this, NULL, NULL, TO_CHAR);
  } else if (ret==SPELL_FAIL) 
    deityIgnore();
}

int armor(TBeing *c, TBeing * victim, int level, byte learn, spellNumT spell)
{
  affectedData aff;

  aff.type = SPELL_ARMOR;
  aff.level = level;
  aff.duration = (3 + (aff.level / 2)) * UPDATES_PER_MUDHOUR;
  aff.location = APPLY_ARMOR;
  aff.bitvector = 0;

  // deikhan armor does less (c.f. balance notes)
  if (spell == SPELL_ARMOR)
    aff.modifier = -100;
  else if (spell == SPELL_ARMOR_DEIKHAN)
    aff.modifier = -75;
  else {
    vlogf(LOG_BUG, fmt("Unknown spell %d in armor()") %  spell);
    aff.modifier = 0;
  }
  
  if (c->bSuccess(learn, c->getPerc(), spell)) {
    c->reconcileHelp(victim, discArray[spell]->alignMod);

    if (c != victim)
      aff.modifier /= 8;

    victim->roomp->playsound(SOUND_SPELL_ARMOR, SOUND_TYPE_MAGIC);

    switch (critSuccess(c, spell)) {
      case CRIT_S_KILL:
      case CRIT_S_DOUBLE:
        CS(spell);
        aff.duration *= 2;
        if (c != victim)
          aff.modifier *= 2;

        if (!victim->affectJoin(c, &aff, AVG_DUR_NO, AVG_EFF_YES))
          return SPELL_FAIL;
        return SPELL_CRIT_SUCCESS;
      default:
        if (!victim->affectJoin(c, &aff, AVG_DUR_NO, AVG_EFF_YES))
          return SPELL_FAIL;
        break;
    }    
    return SPELL_SUCCESS;
  } else {
    return SPELL_FAIL;
  }
}

void armor(TBeing *c, TBeing * victim, TMagicItem * obj, spellNumT spell)
{
  int ret=armor(c,victim,obj->getMagicLevel(),obj->getMagicLearnedness(), spell);
  if (ret == SPELL_SUCCESS) {
    if (c != victim) {
      act("$N is now defended by $d!",
          FALSE, c, NULL, victim, TO_CHAR);
      act("$N is now defended by $d!",
          FALSE, c, NULL, victim, TO_NOTVICT);
      act("You are now defended by $d!",
          FALSE, c, NULL, victim, TO_VICT);
    } else {
      act("$n is now defended by $d!",
          FALSE, c, NULL, 0, TO_ROOM);
      act("You are now defended by $d!",
          FALSE, c, NULL, 0, TO_CHAR);
    }
  } else if (ret == SPELL_CRIT_SUCCESS) {
    if (c != victim) {
      act("$N is now strongly defended by $d!",
          FALSE, c, NULL, victim, TO_CHAR);
      act("$N is now strongly defended by $d!",
          FALSE, c, NULL, victim, TO_NOTVICT);
      act("You are now strongly defended by $d!",
          FALSE, c, NULL, victim, TO_VICT);
    } else {
      act("$n is now strongly defended by $d!",
          FALSE, c, NULL, 0, TO_ROOM);
      act("You are now strongly defended by $d!",
          FALSE, c, NULL, 0, TO_CHAR);
    }
  } else if (ret == SPELL_FAIL) {
    act("$p fails to bring forth any protection.", 
           FALSE, c, obj, NULL, TO_CHAR);
    c->deityIgnore(SILENT_YES);
  }
  return;
}

void armor(TBeing *c, TBeing * victim)
{
  int ret,level;

  spellNumT spell = c->getSkillNum(SPELL_ARMOR);

  if (!bPassClericChecks(c,spell))
    return;

  level = c->getSkillLevel(spell);

  if ((ret=armor(c,victim,level, c->getSkillValue(spell), spell))==SPELL_SUCCESS) {
    if (c != victim) {
      act("$N is now defended by $d!",
          FALSE, c, NULL, victim, TO_CHAR);
      act("$N is now defended by $d!",
          FALSE, c, NULL, victim, TO_NOTVICT);
      act("You are now defended by $d!",
          FALSE, c, NULL, victim, TO_VICT);
    } else {
      act("$n is now defended by $d!",
          FALSE, c, NULL, 0, TO_ROOM);
      act("You are now defended by $d!",
          FALSE, c, NULL, 0, TO_CHAR);
    }
  } else if (ret == SPELL_CRIT_SUCCESS) {
    if (c != victim) {
      act("$N is now strongly defended by $d!",
          FALSE, c, NULL, victim, TO_CHAR);
      act("$N is now strongly defended by $d!",
          FALSE, c, NULL, victim, TO_NOTVICT);
      act("You are now strongly defended by $d!",
          FALSE, c, NULL, victim, TO_VICT);
    } else {
      act("$n is now strongly defended by $d!",
          FALSE, c, NULL, 0, TO_ROOM);
      act("You are now strongly defended by $d!",
          FALSE, c, NULL, 0, TO_CHAR);
    }
  } else if (ret == SPELL_FAIL) {
    if (c != victim)
      act("You are unable to convince $d to armor $N.", 
           FALSE, c, NULL, victim, TO_CHAR);
    else
      act("You are unable to convince $d to armor you.", 
           FALSE, c, NULL, NULL, TO_CHAR);
    c->deityIgnore(SILENT_YES);
  }
}

int sanctuary(TBeing *c, TBeing *victim, int level, byte learn)
{
  affectedData aff;

  aff.type = SPELL_SANCTUARY;
  aff.level = level;
  aff.duration = ((level <= MAX_MORT) ? 3 : level) * UPDATES_PER_MUDHOUR;
  aff.location = APPLY_PROTECTION;
  aff.modifier = min(level, 50);
  aff.bitvector = AFF_SANCTUARY;

  if (c != victim)
    aff.modifier /= 3;

  if (c->bSuccess(learn, c->getPerc(), SPELL_SANCTUARY)) {
    c->reconcileHelp(victim, discArray[SPELL_SANCTUARY]->alignMod);

    switch  (critSuccess(c, SPELL_SANCTUARY)) {
      case CRIT_S_KILL:
      case CRIT_S_TRIPLE:
      case CRIT_S_DOUBLE:
        CS(SPELL_SANCTUARY);
        aff.duration = ((level <= MAX_MORT) ? 5 : level) * UPDATES_PER_MUDHOUR;
        if (!victim->affectJoin(c, &aff, AVG_DUR_NO, AVG_EFF_YES))
          return SPELL_FAIL;
        return SPELL_CRIT_SUCCESS;
      default:
        if (!victim->affectJoin(c, &aff, AVG_DUR_NO, AVG_EFF_YES))
          return SPELL_FAIL;
    }
    victim->roomp->playsound(SOUND_SPELL_SANCTUARY, SOUND_TYPE_MAGIC);
    return SPELL_SUCCESS;
  } else 
    return SPELL_FAIL;
}

void sanctuary(TBeing *c, TBeing * victim, TMagicItem * obj)
{
  int ret = sanctuary(c,victim,obj->getMagicLevel(),obj->getMagicLearnedness());

  if (ret==SPELL_SUCCESS) {
    act("$n glows with a powerful white aura!", FALSE, victim, NULL, NULL, TO_ROOM);
    act("You glow with a powerful white aura!", FALSE, victim, NULL, NULL, TO_CHAR);
  } else if (ret==SPELL_CRIT_SUCCESS) {
    act("$n glows with a powerful and especially bright white aura!", FALSE, victim, NULL, NULL, TO_ROOM);
    act("You glow with a powerful and especially bright white aura!", FALSE, victim, NULL, NULL, TO_CHAR);
  } else if (ret==SPELL_FAIL) {
    act("You are unable to convince $d to aid you.", FALSE, c, NULL, NULL, TO_CHAR);
    c->deityIgnore(SILENT_YES);
  }
}

void sanctuary(TBeing *c, TBeing * victim)
{
  int ret,level;

  if (!bPassClericChecks(c,SPELL_SANCTUARY))
    return;

  level = c->getSkillLevel(SPELL_SANCTUARY);

  ret = sanctuary(c,victim,level, c->getSkillValue(SPELL_SANCTUARY));

  if (ret==SPELL_SUCCESS) {
    act("$n glows with a powerful white aura!", FALSE, victim, NULL, NULL, TO_ROOM);
    act("You glow with a powerful white aura!", FALSE, victim, NULL, NULL, TO_CHAR);
  } else if (ret == SPELL_CRIT_SUCCESS) {
    act("$n glows with a powerful and especially bright white aura!", FALSE, victim, NULL, NULL, TO_ROOM);
    act("You glow with a powerful and especially bright white aura!", FALSE, victim, NULL, NULL, TO_CHAR);
  } else if (ret == SPELL_FAIL) {
    act("You are unable to convince $d to aid you.", FALSE, c, NULL, NULL, TO_CHAR);
    c->deityIgnore(SILENT_YES);
  }
}

int bless(TBeing *c, TObj * obj, int level, byte learn, spellNumT spell)
{
  if (c->bSuccess(learn, c->getPerc(), spell)) {
    if ((5 * level > (int) obj->getWeight())) { 
      obj->addObjStat(ITEM_BLESS);
      act("A soft yellow light shines down upon $p for a moment and then fades away...", FALSE, c, obj, NULL, TO_ROOM);
      act("A soft yellow light shines down upon $p for a moment and then fades away...", FALSE, c, obj, NULL, TO_CHAR);
      act("$d blesses $p.",
          FALSE, c, obj, NULL, TO_ROOM);
      act("$d blesses $p.",
          FALSE, c, obj, NULL, TO_CHAR);

      c->roomp->playsound(SOUND_SPELL_BLESS, SOUND_TYPE_MAGIC);

    } else {
      SV(spell);
      act("You are unable to convice $d to bless $p.", 
               FALSE, c, obj, NULL, TO_CHAR);
      c->deityIgnore(SILENT_YES);
    }
    return SPELL_SUCCESS;
  } else {
    act("You are unable to convice $d to bless $p.", 
               FALSE, c, obj, NULL, TO_CHAR);
    c->deityIgnore(SILENT_YES);
    return SPELL_FAIL;
  }
}

void bless(TBeing *c, TObj * obj)
{
  spellNumT spell = c->getSkillNum(SPELL_BLESS);

  if (!bPassClericChecks(c,spell))
    return;

  int level = c->getSkillLevel(spell);

  bless(c,obj,level, level, spell);
}

int bless(TBeing *c, TBeing * victim, int level, byte learn, spellNumT spell)
{
  if (c->bSuccess(learn, c->getPerc(), spell)) {
    c->reconcileHelp(victim, discArray[spell]->alignMod);

    bool isCrit = FALSE;
    if (critSuccess(c, spell)) {
      CS(spell);
      isCrit = TRUE;
    }
    bool success = genericBless(c, victim, level, isCrit);
  
    if (!success) {
      c->deityIgnore();
      return SPELL_FAIL;
    }
    if (isCrit) {
      act("A blinding light of divine purity sears down upon $n!", 
          FALSE, victim, NULL, NULL, TO_ROOM);
      act("A blinding light of divine purity sears down upon you!", 
          FALSE, victim, NULL, NULL, TO_CHAR);
      if (c == victim) {
        act("You have been greatly blessed by $d.", FALSE, c, NULL, NULL, TO_CHAR);
        act("$d greatly blesses $n.",
                   FALSE, c, NULL, NULL, TO_ROOM);
      } else {
        act("$d greatly blesses you.",
                   FALSE, c, NULL, victim, TO_VICT);
        act("$d greatly blesses $N.",
                   FALSE, c, NULL, victim, TO_CHAR);
        act("$d greatly blesses $N.",
                   FALSE, c, NULL, victim, TO_NOTVICT);
      }
    } else {
      act("A soft yellow light shines down upon $n for a moment and then fades away...", FALSE, victim, NULL, NULL, TO_ROOM);
      act("A soft yellow light shines down upon you for a moment and then fades away...", FALSE, victim, NULL, NULL, TO_CHAR);
      if (c == victim) {
        act("You have been blessed by $d.", FALSE, c, NULL, NULL, TO_CHAR);
        act("$n has been blessed by $d.", FALSE, c, NULL, NULL, TO_ROOM);
      } else {
        act("$d blesses you.",
                   FALSE, c, NULL, victim, TO_VICT);
        act("$d blesses $N.",
                   FALSE, c, NULL, victim, TO_CHAR);
        act("$d blesses $N.",
                   FALSE, c, NULL, victim, TO_NOTVICT);
      }
    }
    victim->roomp->playsound(SOUND_SPELL_BLESS, SOUND_TYPE_MAGIC);
    return SPELL_SUCCESS;
  } else {
    c->deityIgnore();
    return SPELL_FAIL;
  }
}

void bless(TBeing *c, TObj * target, TMagicItem * obj, spellNumT spell)
{
  bless(c,target,obj->getMagicLevel(),obj->getMagicLearnedness(), spell);
}

void bless(TBeing *c, TBeing * victim, TMagicItem * obj, spellNumT spell)
{
  bless(c,victim,obj->getMagicLevel(),obj->getMagicLearnedness(), spell);
}

void bless(TBeing *c, TBeing * victim)
{
  spellNumT spell = c->getSkillNum(SPELL_BLESS);

  if (!bPassClericChecks(c,spell))
    return;

  int level = c->getSkillLevel(spell);

  bless(c,victim,level, c->getSkillValue(spell), spell);
}
