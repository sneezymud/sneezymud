
//////////////////////////////////////////////////////////////////////////
//
// SneezyMUD - All rights reserved, SneezyMUD Coding Team
//
//      "disease.cc" - functions handling disease affects
//
///////////////////////////////////////////////////////////////////////////

#include "room.h"
#include "being.h"
#include "monster.h"
#include "low.h"
#include "disease.h"
#include "obj_tool.h"
#include "obj_corpse.h"
#include "spec_mobs.h"
#include "materials.h"

//  use reconcileDamage to apply damage to victims.
//  if victim dies, leave victim valid (do not delete) annd return a -1

diseaseTypeT affToDisease(affectedData &af)
{
  diseaseTypeT dtt = diseaseTypeT(af.modifier);
  if (dtt < DISEASE_NULL || dtt >= MAX_DISEASE)
    dtt = DISEASE_NULL;

  return dtt;
}

int disease_start(TBeing *victim, affectedData *af)
{
  return (*(DiseaseInfo[affToDisease(*af)].code)) (victim, DISEASE_BEGUN, af);
}

bool TBeing::hasDisease(diseaseTypeT disease) const
{
  affectedData *aff;

  for (aff = affected; aff; aff = aff->next) {
    if (aff->type == AFFECT_DISEASE) {
      if (aff->modifier == disease) 
        return TRUE;
    }
  }
  return FALSE;
}

void TBeing::diseaseFrom(diseaseTypeT disease)
{
  affectedData *aff, *anext;

  for (aff = affected; aff; aff = anext) {
    anext = aff->next;
    if (aff->type == AFFECT_DISEASE) {
      if (aff->modifier == disease) {
        diseaseStop(aff);
        affectRemove(aff);
      }
    }
  }
}

int TBeing::diseaseStop(affectedData *af)
{
  return (*(DiseaseInfo[affToDisease(*af)].code)) (this, DISEASE_DONE, af);
}

int disease_null(TBeing *victim, int, affectedData *)
{
  vlogf(LOG_BUG, format("WARNING:  %s has a bogus disease #%d affect.") % 
	((victim) ? victim->getName() : "A null pointer"));
  return FALSE;
}

void spread_affect(TBeing *ch, int chance_to_spread, bool race, bool not_race, affectedData * af)
{
  TThing *t=NULL;
  affectedData vaf;
  if (ch->inRoom() == Room::NOCTURNAL_STORAGE)
    return;
  int spread_controller = 1; // to reduce rate of spreading in crowds. it gets out of hand.
  int effective_chance = chance_to_spread; // to store the original chance
  
  // Do not spread disease in peacful zones, this can become real hairy.
  if (ch->roomp && ch->roomp->isRoomFlag(ROOM_PEACEFUL))
    return;


  for(StuffIter it=ch->roomp->stuff.begin();it!=ch->roomp->stuff.end() && (t=*it);++it) {
    TBeing *v = dynamic_cast<TBeing *>(t);
    if (!v || v == ch)
      continue;
    if (v->isImmortal())
      continue;
    if (v->isImmune(IMMUNE_DISEASE, WEAR_BODY))
      continue;
    
    if (effective_chance > 50)
      effective_chance = chance_to_spread / spread_controller++;

    if (number(1,50000) >= effective_chance)
      continue;
    if (race && (v->getRace() != race))
      continue;
    if (not_race && (v->getRace() == not_race))
      continue;

    // guess who shouldn't get diseases...
    if (v->spec == SPEC_HORSE_PESTILENCE ||
        v->isShopkeeper() ||
        v->mobVnum() == Mob::APOC_PESTHORSE)
      continue;

    if ((af->type != AFFECT_DISEASE && !v->affectedBySpell(af->type)) ||
        (af->type == AFFECT_DISEASE && !v->hasDisease(affToDisease(*af)))) {

#if 0
      vlogf(LOG_MISC, format("%s (%s:%d) spreading from %s to %s at %d") % 
             af->type == AFFECT_DISEASE ? "Disease" : "Spell" %
             af->type == AFFECT_DISEASE ? 
                 DiseaseInfo[af->modifier].name : 
                 discArray[af->type]->name %
             af->type == AFFECT_DISEASE ? af->modifier : af->type %
             ch->getName() % v->getName() % ch->inRoom());
#endif
      vaf.type = af->type;
      vaf.level = af->level;
      vaf.duration = af->duration;
      vaf.modifier = af->modifier;
      vaf.modifier2 = af->modifier2;
      vaf.location = af->location;
      vaf.bitvector = af->bitvector;
      if (vaf.duration != PERMANENT_DURATION) {
        vaf.duration *= (100 - v->getImmunity(IMMUNE_DISEASE));
        vaf.duration /= 100;
      }
			if (vaf.duration > 0 || vaf.duration == PERMANENT_DURATION) {
        v->affectTo(&vaf, TRUE);
        if (vaf.type == AFFECT_DISEASE)
          disease_start(v, &vaf);
      }
    }
  }
}

// since the effects of flu get called by multiple diseases i've set it up
// as this dummy function
// returns DELETE_THIS
int TBeing::dummyFlu()
{
  int rc;

  if (isPc() && !desc)
    return FALSE;

  switch (::number(1,150)) {
    case 1:
    case 2:
      sendTo("Your stomach churns uncomfortably.\n\r");
      doAction("",CMD_PUKE);
      this->dropPool(10, LIQ_VOMIT);
      if (getPosition() <= POSITION_SLEEPING) {
        sendTo("Puking in your sleep causes you to gag!\n\r");
        if ((rc = reconcileDamage(this, ::number(1,10) +15, DAMAGE_SUFFOCATION)) == -1)
          return DELETE_THIS; 
      }
      break;
    case 3:
    case 4:
    case 5:
    case 6:
      act("You sweat for no apparent reason.",FALSE,this,0,0,TO_CHAR);
			if (isHumanoid()) {
				// i suppose sweating is for humanoids
      	act("$n is sweating and $e looks feverish.",TRUE,this,0,0,TO_ROOM);
			} else {
      	act("$n looks feverish.",TRUE,this,0,0,TO_ROOM);
			}
      break;
    case 7:
    case 8:
    case 9:
    case 10:
      sendTo("A sudden chill runs through you.\n\r");
      doAction("",CMD_SHIVER);
      break;
    case 11:
    case 12:
    case 13:
    case 14:
    case 15:
      sendTo("You suddenly feel immensely tired.\n\r");
      setMove(max((getMove() - 30), 0));
      if (reconcileDamage(this, 5, SPELL_DISEASE) == -1)
        return DELETE_THIS;
      break;
    case 16:
    case 17:
    case 18:
      act("The world starts to spin and the landscape seems to leap upwards.",
           TRUE,this,0,0,TO_CHAR);
      if (riding) {
        act("$n sways then crumples as $e faints.",FALSE,this,0,0,TO_ROOM);
        rc = fallOffMount(riding, POSITION_RESTING);
        if (IS_SET_DELETE(rc, DELETE_THIS)) {
          return DELETE_THIS;
        }
      } else
        act("$n stumbles then crumples as $e faints.",FALSE,this,0,0,TO_ROOM);
      setPosition(POSITION_SLEEPING);
      break;
    case 19:
    case 20:
    case 21:
    case 22:
    case 23:
      sendTo("Your stomach aches and you feel nauseous.\n\r");
      break;
    case 24:
    case 25:
      sendTo("You breath feverishly in short shallow gasps.\n\r");
      act("$n's is breathing in short, shallow gasps.",FALSE,this,0,0,TO_ROOM);
      break;
    default:
      break;
  }
  return FALSE;
}

int disease_flu(TBeing *victim, int message, affectedData *)
{
  int chance;
  affectedData vaf;
  int rc; 
  if (message == DISEASE_BEGUN) {
    act("You begin to feel hot and nauseous.", FALSE, victim, 0, 0, TO_CHAR);
    if (victim->isHumanoid()) {
      act("$n starts to sweat and $s face looks pale.", TRUE, victim, NULL, NULL, TO_ROOM);
    } else {
      act("$n looks lethargic and glassy eyed.", TRUE, victim, NULL, NULL, TO_ROOM);
    }
  } else if (message == DISEASE_PULSE) {
    if (victim->isHumanoid()) {
      rc = victim->dummyCold();
      if (IS_SET_DELETE(rc, DELETE_THIS))
        return DELETE_THIS;
      rc = victim->dummyFlu();
      if (IS_SET_DELETE(rc, DELETE_THIS))
        return DELETE_THIS;
    }
    chance = 50;
    vaf.type = AFFECT_DISEASE;
    vaf.level = 0;
    vaf.duration = 200;
    vaf.modifier = DISEASE_FLU;
    vaf.location = APPLY_NONE;
    vaf.bitvector = 0;
    spread_affect(victim, chance, false, false, &vaf);
    vaf.duration = 130;
    vaf.modifier = DISEASE_COLD;
    chance = 85;
    spread_affect(victim, chance, false, false, &vaf);
  } else if (message == DISEASE_DONE) {
    if (victim->getPosition() > POSITION_DEAD) {
      act("$n relaxes a bit.", TRUE, victim, NULL, NULL, TO_ROOM);
      victim->sendTo("Your stomach settles down and you feel much better.\n\r");
    }
    return FALSE;
  }
  return FALSE;
}

// since the effects of cold get called by multiple diseases i've set it up
// as this dummy function
// may return DELETE_THIS
int TBeing::dummyCold()
{
  if (isPc() && !desc)
    return FALSE;

  switch (::number(1,100)) {
    case 1:
    case 2:
    case 3:
      sendTo("You start to cough loudly.  The back of your throat is raw.\n\r");
      doAction("",CMD_COUGH);
      break;
    case 4:
    case 5:
    case 6:
      sendTo("A sneezing fit overcomes you and tears come to your eyes.\n\r");
      doAction("",CMD_SNEEZE);
      break;
    case 7:
    case 8:
    case 9:
    case 10:
      act("Your nose starts to drip and run.",TRUE,this,0,0,TO_CHAR);
      act("$n's nose starts to drip and $e sniffs loudly.",TRUE,this,0,0,TO_ROOM);
      break;
    case 11:
    case 12:
    case 13:
    case 14:
      sendTo("Your muscles ache from this damn cold you have.\n\r");
      if (reconcileDamage(this, 2, SPELL_DISEASE) == -1)
        return DELETE_THIS;
      break;
    case 15:
    case 16:
    case 17:
    case 18:
      sendTo("You have a massive, splitting headache.  Damn cold!\n\r");
      setMana(max((getMana() - 35), 0));
      addToLifeforce(-2);
      break;
    default:
      break;
  }
  return FALSE;
}

int disease_cold(TBeing *victim, int message, affectedData *)
{
  affectedData vaf;
  int rc = 0;
  int spread_chance = 250;
  if (message == DISEASE_PULSE) {
    if (victim->isHumanoid()) {
      rc = victim->dummyCold();
      if (IS_SET_DELETE(rc, DELETE_THIS))
        return DELETE_THIS;
    }
    vaf.type = AFFECT_DISEASE;
    vaf.level = 0;
    vaf.duration = 150;
    vaf.modifier = DISEASE_COLD;
    vaf.location = APPLY_NONE;
    vaf.bitvector = 0;
    spread_affect(victim, spread_chance, FALSE, FALSE, &vaf);
  } else if (message == DISEASE_BEGUN) {
    act("$n doesn't look very well.", TRUE, victim, NULL, NULL, TO_ROOM);
    victim->sendTo("You don't feel very well at all.\n\r");
    return FALSE;
  } else if (message == DISEASE_DONE) {
    if (victim->getPosition() > POSITION_DEAD) {
      act("Some color has returned to $n.", TRUE, victim, NULL, NULL, TO_ROOM);
      victim->sendTo("You're feeling much better.\n\r");
    }
    return FALSE;
  }
  return FALSE;
}

int disease_numbed(TBeing *victim, int message, affectedData *af)
{
  wearSlotT slot = wearSlotT(af->level);

  if(slot < MIN_WEAR || slot >= MAX_WEAR){
    vlogf(LOG_BUG, format("disease_numbed called with bad slot: %i") % slot);
    return FALSE;
  }

  switch(message) {
    case DISEASE_BEGUN:
      victim->addToLimbFlags(slot, PART_PARALYZED);
      break;
    case DISEASE_PULSE:
      // Do nothing, all this disease does is keep up with numb time 
      break;
    case DISEASE_DONE:
      if (victim->isLimbFlags(slot, PART_PARALYZED))
        victim->remLimbFlags(slot, PART_PARALYZED); 
                                                                            
      if (victim->getPosition() > POSITION_DEAD && victim->hasPart(slot)) {
        victim->sendTo(format("You feel the life come back to your %s!\n\r") % victim->describeBodySlot(slot));
      }
      break;                                                                    
    default:
      break;
  }
  return FALSE;
}

int disease_bruised(TBeing *victim, int message, affectedData *af)
{
  int j;
  char buf[256];
  // defines the limb that is bruised
  wearSlotT i = wearSlotT(af->level);


  if(i < MIN_WEAR || i >= MAX_WEAR){
    vlogf(LOG_BUG, format("disease_bruised called with bad slot: %i") % i);
    return FALSE;
  }


  if (victim->isPc() && !victim->desc)
    return FALSE;

  switch (message) {
    case DISEASE_BEGUN:
      victim->addToLimbFlags(i, PART_BRUISED);
      break;
    case DISEASE_PULSE:
      // defines the severity of the bruise
      j = af->modifier2;

      // check to see if somehow the bruised bit got taken off
      if (!victim->hasPart(i) || !victim->isLimbFlags(i, PART_BRUISED)) {
        af->duration = 0;
        break;
      }

      if (!number(0, 50)) {
        // just for fun, no damage or anything
        victim->sendTo(format("You feel your %s throb and the bruise turns a deeper shade of purple.\n\r") % victim->describeBodySlot(i));
      }
      break;
    case DISEASE_DONE:
      if (victim->isLimbFlags(i, PART_BRUISED))
        victim->remLimbFlags(i, PART_BRUISED);
      if (victim->getPosition() > POSITION_DEAD && victim->hasPart(i)) {
        victim->sendTo(format("The bruise on your %s fades away!\n\r") % victim->describeBodySlot(i));
        sprintf(buf, "The bruise on $n's %s fades away!", victim->describeBodySlot(i).c_str());
        act(buf, TRUE, victim, NULL, NULL, TO_ROOM);
      }
      break;
    default:
      break;
  }
  return FALSE;
}

int disease_bleeding(TBeing *victim, int message, affectedData *af)
{
  char buf[256];
  // defines the limb that is bleeding
  wearSlotT i = wearSlotT(af->level);
  if(i < MIN_WEAR || i >= MAX_WEAR){
    vlogf(LOG_BUG, format("disease_bleeding called with bad slot: %i") % i);
    return FALSE;
  }
  if (victim->isPc() && !victim->desc)
    return FALSE;

  switch (message) {
    case DISEASE_BEGUN:
      victim->addToLimbFlags(i, PART_BLEEDING);
      break;
    case DISEASE_PULSE:
      // check to see if somehow the bleeding bit got taken off
      if (!victim->hasPart(i) || !victim->isLimbFlags(i, PART_BLEEDING)) {
        af->duration = 0;
        break;
      }

      if (!victim->isLimbFlags(i, PART_BANDAGED) && !::number(0, 750)) {
        // start an infection
        victim->rawInfect(i, ::number(50, 150), SILENT_NO, CHECK_IMMUNITY_YES, victim->GetMaxLevel());
      }
      
      if (!number(0, 10)) {
        if (victim->doesKnowSkill(SKILL_SNOFALTE)) {
          // attempt to dodge it altogether
          if ((::number(0,99) < 40) &&
               victim->bSuccess((int) victim->getSkillValue(SKILL_SNOFALTE), SKILL_SNOFALTE)) {
            victim->sendTo("You utilize the powers of snofalte to slow your bleeding.\n\r");
            break;
          }
        }
        victim->sendTo(format("You feel your energy drained as your blood drips out of your %s.\n\r") %victim-> describeBodySlot(i));
        sprintf(buf, "$n looks stunned as blood drips from $s %s.", victim->describeBodySlot(i).c_str());

        act(buf, TRUE, victim, NULL, NULL, TO_ROOM);
        victim->dropBloodLimb(i);
        int rc = victim->hurtLimb(1, i);
        if (IS_SET_DELETE(rc, DELETE_THIS))
          return DELETE_THIS;
        
        // took out the damage computations since it always ended up 1 no matter what slot anyhow
        // which i think is fine since i upped chances for bleeding from combat and from the disease prayer
        // and added a chance for infections
        if (victim->reconcileDamage(victim, 1, SPELL_BLEED) == -1)
          return DELETE_THIS;
      }
      break;
    case DISEASE_DONE:
      if (victim->isLimbFlags(i, PART_BLEEDING))
        victim->remLimbFlags(i, PART_BLEEDING);

      if (victim->getPosition() > POSITION_DEAD && victim->hasPart(i)) {
        victim->sendTo(format("Your %s clots and stops bleeding!\n\r") % victim->describeBodySlot(i));
        sprintf(buf, "$n's %s clots and stops bleeding!", victim->describeBodySlot(i).c_str());
        act(buf, TRUE, victim, NULL, NULL, TO_ROOM);
      }

      break;
    default:
      break;
  }
  return FALSE;
}

int disease_infection(TBeing *victim, int message, affectedData * af)
{
  char buf[256];
  wearSlotT slot = wearSlotT(af->level);

  if(slot < MIN_WEAR || slot >= MAX_WEAR){
    vlogf(LOG_BUG, format("disease_infection called with bad slot: %i") % slot);
    return FALSE;
  }
  if (victim->isPc() && !victim->desc)
    return FALSE;

  switch (message) {
    case DISEASE_BEGUN:
      victim->addToLimbFlags(slot, PART_INFECTED);
      break;
    case DISEASE_PULSE:
      // check to see if somehow the infected bit got taken off (via spell)
      if (!victim->hasPart(slot) || !victim->isLimbFlags(slot, PART_INFECTED)) {
        af->duration = 0;
        break;
      }
      // if duration is permanent, boldy assume it is from a gangrenous limb
      // so is gangrene was removed, shorten duration
      if (af->duration == PERMANENT_DURATION && !victim->isLimbFlags(slot, PART_GANGRENOUS)) {
        af->duration = 100;
      }
      // level of infection dictates damage rate
			int level = af->modifier2;
      if (!number(0, max(14, 35 - (int) ((double) level / 3.2)))) {
        victim->sendTo(format("Your %s shakes and twinges as the infection in it festers.\n\r") % victim->describeBodySlot(slot));
        sprintf(buf, "$n's %s shakes and twinges as the infection in it festers.", victim->describeBodySlot(slot).c_str());
        act(buf, TRUE, victim, NULL, NULL, TO_ROOM);
        int rc = victim->hurtLimb(1, slot);
        if (IS_SET_DELETE(rc, DELETE_THIS))
          return DELETE_THIS;

        int dam;
        dam = (VITAL_PART(slot) ? 3 : 1);
        if (victim->reconcileDamage(victim, dam, SPELL_INFECT) == -1)
          return DELETE_THIS;

        if (dynamic_cast<TMonster *>(victim) && !victim->isPc())
          (dynamic_cast<TMonster *>(victim))->UA((dam * 4));
      }
      victim->bodySpread(500, af);
      break;
    case DISEASE_DONE:
      victim->remLimbFlags(slot, PART_INFECTED);
      if (victim->getPosition() > POSITION_DEAD && victim->hasPart(slot)) {
        victim->sendTo(format("Your %s stops twitching and heals!\n\r") % victim->describeBodySlot(slot));
        sprintf(buf, "$n's %s stops twitching and heals!", victim->describeBodySlot(slot).c_str());
        act(buf, TRUE, victim, NULL, NULL, TO_ROOM);
      }

      break;
    default:
      break;
  }
  return FALSE;
}

int disease_syphilis(TBeing *ch, int message, affectedData *)
{
  switch (message) {
    case DISEASE_PULSE:
      if (::number(0,10))
        return FALSE;
      switch (::number(1,5)) {
        case 1:
        case 2:
          ch->sendTo("The burning of your syphilis causes you pain.\n\r");
          break;
        case 3:
        case 4:
        case 5:
          ch->sendTo("You feel a stinging in your waist area.\n\r");
          break;
        default:
          break;
      }
      act("$n breaks out in open sores.",
           TRUE, ch, 0,0,TO_ROOM);
      if (ch->reconcileDamage(ch, ::number(3,6), DAMAGE_NORMAL) == -1) {
        return DELETE_THIS;
      }
      break;
    case DISEASE_BEGUN:
      // code that sets this tells how it happened
      break;
    case DISEASE_DONE:
      if (ch->getPosition() > POSITION_DEAD) {
        ch->sendTo("The syphilis in your body becomes dormant.\n\r");
      }
      break;
    default:
      break;
  }
  return FALSE;
}

int disease_hemorraging(TBeing *victim, int message, affectedData *)
{
  int dam;

    if (victim->isPc() && !victim->desc)
    return FALSE;

  switch (message) {
    case DISEASE_PULSE:
      if (number(0,4))
        return FALSE;

      victim->sendTo("An immense pain in your chest causes you to grunt in agony!\n\r");
      act("$n groans in agony.",FALSE,victim,0,0,TO_ROOM);
      dam = ::number(1,3);
      if (victim->reconcileDamage(victim, dam, DAMAGE_HEMORRAGE) == -1)
        return DELETE_THIS;
      break;
    case DISEASE_BEGUN:
      // code that sets this tells how it happened
      break;
    case DISEASE_DONE:
      if (victim->getPosition() > POSITION_DEAD) {
        victim->sendTo("Your internal hemorrhaging has stopped.\n\r");
        act("$n seems to get better.",TRUE,victim,0,0,TO_ROOM);
      }
      break;
    default:
      break;
  }
  return FALSE;
}

int disease_frostbite(TBeing *victim, int message, affectedData *)
{
  wearSlotT i;
  char buf[256];

  switch (message) {
    case DISEASE_PULSE:
      if (number(0,50))
        return FALSE;
      
      for (i = MIN_WEAR; i < MAX_WEAR; i++) {
	if (victim->validEquipSlot(i) && !(victim->equipment[i]) && !number(0, 3)) {
	  victim->hurtLimb(victim->getMaxLimbHealth((wearSlotT)i) /
			   ::number(1,5), 
			   (wearSlotT)i);
	  sprintf(buf, "$n's %s takes on a gray-blue color as frost-bite sets in.", victim->describeBodySlot(i).c_str());
	  act(buf, TRUE, victim, NULL, NULL, TO_ROOM);
	  victim->sendTo(format("Your %s feels numb and takes on a gray-blue color.  Looks like frostbite.\n\r") % victim->describeBodySlot(i));
	}
      }
      
      if (number(0,15))
        break;
      victim->doAction("", CMD_SHIVER);
      break;
    case DISEASE_BEGUN:
      act("$n looks very cold and has begun shivering involuntarily.",
	  TRUE, victim, NULL, NULL, TO_ROOM);
      act("Frostbite will likely set in soon.", 
	  FALSE, victim, NULL, NULL, TO_ROOM);
      victim->sendTo("You are very cold and have begun shivering involuntarily.\n\r");
      victim->sendTo("Frostbite will likely set in soon.\n\r");
      break;
    case DISEASE_DONE:
      if (victim->getPosition() > POSITION_DEAD) {
        act("$n still looks cold, but the shivering has stopped.", TRUE, victim, NULL, NULL, TO_ROOM);
        victim->sendTo("You almost feel warm again.\n\r");
      }
      break;
    default:
      break;
  }
  return FALSE;
}

int disease_voicebox(TBeing *victim, int message, affectedData *)
{
  switch (message) {
    case DISEASE_PULSE:
      break;
    case DISEASE_BEGUN:
      // code that sets this tells how it happened
      break;
    case DISEASE_DONE:
      if (victim->getPosition() > POSITION_DEAD) {
        act("$n's voicebox heals and $s voice returns to $m.",TRUE,victim,0,0,TO_ROOM);
        victim->sendTo("You feel your larynx mend and your voice has returned.\n\r");
      }
      break;
    default:
      break;
  }
  return FALSE;
}

int disease_eyeball(TBeing *victim, int message, affectedData *)
{
  switch (message) {
    case DISEASE_PULSE:
      break;
    case DISEASE_BEGUN:
      // code that sets this tells how it happened
      break;
    case DISEASE_DONE:
      if (victim->getPosition() > POSITION_DEAD) {
        act("$n's missing eyeballs slowly regenerate.",TRUE,victim,0,0,TO_ROOM);
        victim->sendTo("You feel something growing in your empty eye sockets.\n\r");
        victim->sendTo("These new eyeballs don't seem to work fully.\n\r");
        victim->sendTo("Perhaps a cleric can cure your blindness now though...\n\r");
      }
      break;
    default:
      break;
  }
  return FALSE;
}

int disease_stomach(TBeing *victim, int message, affectedData *)
{
  if (victim->isPc() && !victim->desc)
    return FALSE;

  switch (message) {
    case DISEASE_PULSE:
      if (number(0,2))
        return FALSE;
      if ((victim->getCond(FULL) > 0) && !number(0,4))
        victim->gainCondition(FULL, -1);
      else if (!victim->getCond(FULL)) {
        victim->sendTo("Your stomach wound pains you!\n\r");
        if (victim->reconcileDamage(victim, 2, DAMAGE_STOMACH_WOUND) == -1)
          return DELETE_THIS;
      } 
      if ((victim->getCond(DRUNK) > 0) && ::number(0,1))
        victim->gainCondition(DRUNK, -1);
      if (victim->getCond(THIRST) > 0)
        victim->gainCondition(THIRST, -1);
      else if (!victim->getCond(THIRST)) {
        victim->sendTo("Your stomach wound causes an immense thirst!\n\r");
        if (victim->reconcileDamage(victim, 2, DAMAGE_STOMACH_WOUND) == -1)
          return DELETE_THIS;
      }
      break;
    case DISEASE_BEGUN:
      // code that sets this tells how it happened
      break;
    case DISEASE_DONE:
      if (victim->getPosition() > POSITION_DEAD) {
        act("$n's stomach wound slowly closes up and mends.",TRUE,victim,0,0,TO_ROOM);
        victim->sendTo("Your stomach wound slowly closes over and mends itself.\n\r");
      }
      break;
    default:
      break;
  }
  return FALSE;
}

int disease_lung(TBeing *victim, int message, affectedData *)
{
  int dam;

  if (victim->isPc() && !victim->desc)
    return FALSE;

  switch (message) {
    case DISEASE_PULSE:
      if (number(0,2))
        return FALSE;
      victim->sendTo("You gasp and wheeze, you can't seem to breathe properly!\n\r");
      act("$n makes gasping and wheezing sounds as the air rushes from $s punctured lung.",TRUE,victim,0,0,TO_ROOM);
      dam = ::number(1,3);
      if (victim->reconcileDamage(victim, dam, DAMAGE_SUFFOCATION) == -1)
        return DELETE_THIS;
      break;
    case DISEASE_BEGUN:
      // code that sets this tells how it happened
      break;
    case DISEASE_DONE:
      if (victim->getPosition() > POSITION_DEAD) {
        victim->sendTo("The hole in your lung seems to close up and your lungs thankfully fill with air.\n\r");
        act("The hole in $n's lung heals and $e takes a big gulp of air.",TRUE,victim,0,0,TO_ROOM);
      }
      break;
    default:
      break;
  }
  return FALSE;
}

int disease_suffocate(TBeing *victim, int message, affectedData *af)
{
  int dam;

  if (victim->isPc() && !victim->desc)
    return FALSE;

  switch (message) {
    case DISEASE_PULSE:
      if (::number(0,1))
        return FALSE;
      dam = ::number(min((int) af->level,50)/2 + 1,min((int) af->level,50));
      // narrowed this from 1-50 (at 50th) to 25-50 or so... the damage was very lame
      victim->sendTo("GASP!  You can't breathe!\n\r");
      act("$n grabs $s throat and gasps, trying to breathe.",
                 TRUE,victim,0,0,TO_ROOM);
      victim->addToMove(-::number(1, 10));

      if (victim->reconcileDamage(victim, dam, SPELL_SUFFOCATE) == -1)
        return DELETE_THIS;
      break;
    case DISEASE_BEGUN:
      // code that sets this tells how it happened
      break;
    case DISEASE_DONE:
      if (victim->getPosition() > POSITION_DEAD) {
        victim->sendTo("The vaccum around you disappates.\n\r");
        victim->sendTo("You can breathe again!\n\r");
        act("Air returns to $n as $e takes a deep breath of air.",
                    FALSE,victim,0,0,TO_ROOM);
      }
      break;
    default:
      break;
  }
  return FALSE;
}

int TThing::garottePulse(TBeing *, affectedData *af)
{
  // bad news, garotting without it being a garotte
  af->duration = 0;
  return FALSE;
}

int TTool::garottePulse(TBeing *victim, affectedData *af)
{
  int dam;

  if (getToolType() != TOOL_GARROTTE) {
    // garrotte was removed somehow?  possibly due to scrapping
    af->duration = 0;
    return FALSE;
  }
  if (::number(0,2))
    return FALSE;

  victim->sendTo("GASP!  You can't breathe!\n\r");
  act("$n grabs $s throat and gasps, trying to breathe.",
                TRUE,victim,0,0,TO_ROOM);
  dam = ::number(1,min((int) af->level,50));

  victim->addToMove(-::number(1, 10));
  if (victim->reconcileDamage(victim, dam, DAMAGE_SUFFOCATION) == -1)
    return DELETE_VICT;

  // damage garrotte
  addToToolUses(-1);
  if (getToolUses() <= 0) {
    act("$p snaps.", FALSE, victim, this, 0, TO_CHAR);
    act("$p snaps.", FALSE, victim, this, 0, TO_ROOM);
    victim->unequip(WEAR_NECK);
    // af is now bogus
    delete this;
  }
  return FALSE;
}

// this is essentially suffocate but also finds and damages the garrotte
int disease_garrotte(TBeing *victim, int message, affectedData *af)
{
  TThing *obj;
  int rc;

  if (victim->isPc() && !victim->desc)
    return FALSE;

  switch (message) {
    case DISEASE_PULSE:
      if (!(obj = victim->equipment[WEAR_NECK])) {
        // garrotte was removed somehow?  possibly due to scrapping
        af->duration = 0;
        return FALSE;
      }
      rc = obj->garottePulse(victim, af);
      if (IS_SET_DELETE(rc, DELETE_VICT))
        return DELETE_THIS;
      break;
    case DISEASE_BEGUN:
      // code that sets this tells how it happened
      break;
    case DISEASE_DONE:
      if (victim->getPosition() > POSITION_DEAD) {
        victim->sendTo("The vaccum around you disappates.\n\r");
        victim->sendTo("You can breathe again!\n\r");
        act("Air returns to $n as $e takes a deep breath of air.",
                   TRUE,victim,0,0,TO_ROOM);
      }
      break;
    default:
      break;
  }
  return FALSE;
}

int disease_food_poison(TBeing *ch, int message, affectedData *)
{
  int rc;

  switch (message) {
    case DISEASE_PULSE:
      if (::number(0,10))
        return FALSE;
      switch (::number(1,5)) {
        case 1:
        case 2:
          ch->sendTo("Your stomach churns, and the taste of bile comes to your mouth.\n\r");
          break;
        case 3:
        case 4:
          ch->sendTo("You don't feel well at all.  It must have been something you ate.\n\r");
          break;
        case 5:
          ch->doAction("",CMD_PUKE);
          ch->dropPool(4, LIQ_VOMIT); 
          if (ch->getPosition() <= POSITION_SLEEPING) {
            ch->sendTo("Puking in your sleep causes you to gag!\n\r");
            if ((rc = ch->reconcileDamage(ch, ::number(1,10) + 15, DAMAGE_SUFFOCATION)) == -1)
              return DELETE_THIS; 
          }
          break;
        default:
          break;
      }
      act("$n's face is pale and $e doesn't look well at all.",
           TRUE, ch, 0,0,TO_ROOM);
      if (ch->getCond(FULL) > 0)
        ch->gainCondition(FULL, -ch->getCond(FULL));

      if (ch->getCond(THIRST) > 0)
        ch->gainCondition(THIRST, -ch->getCond(THIRST));

      break;
    case DISEASE_BEGUN:
      // code that sets this tells how it happened
      break;
    case DISEASE_DONE:
      if (ch->getPosition() > POSITION_DEAD) {
        ch->sendTo("The discomfort in your stomach and bowels seems to have cleared up, thankfully.\n\r");
      }
      break;
    default:
      break;
  }
  return FALSE;
}

int disease_poison(TBeing *ch, int message, affectedData *)
{
  switch (message) {
    case DISEASE_PULSE:
      if (::number(0,10))
        return FALSE;
      switch (::number(1,5)) {
        case 1:
        case 2:
          ch->sendTo("Toxins course through your body and you are overcome by cramps.\n\r");
          break;
        case 3:
        case 4:
        case 5:
          ch->sendTo("You feel very sick.\n\r");
          break;
        default:
          break;
      }
      act("$n's face is pale and $e doesn't look well at all.",
           TRUE, ch, 0,0,TO_ROOM);
      if (ch->reconcileDamage(ch, ::number(3,6), SPELL_POISON) == -1) {
        return DELETE_THIS;
      }
      break;
    case DISEASE_BEGUN:
      // code that sets this tells how it happened
      break;
    case DISEASE_DONE:
      if (ch->getPosition() > POSITION_DEAD) {
        ch->sendTo("The toxins in your system dissipate.\n\r");
      }
      break;
    default:
      break;
  }
  return FALSE;
}

int disease_drowning(TBeing *victim, int message, affectedData *af)
{
  int dam;
  if (victim->isPc() && !victim->desc)
    return FALSE;

  switch (message) {
    case DISEASE_PULSE:
      if (::number(0,2))
        return FALSE;
      if (victim->isAffected(AFF_WATERBREATH))
        return FALSE;

      dam = ::number(1,min((int) af->level,50));
      victim->sendTo("GASP!  You can't breathe!\n\r");
      act("$n flails about and gasps, trying to breathe.",FALSE,victim,0,0,TO_ROOM);
      victim->addToMove(-::number(1, 10));
      if (victim->reconcileDamage(victim, dam, SPELL_WATERY_GRAVE) == -1)
        return DELETE_THIS;
      break;
    case DISEASE_BEGUN:
      // code that sets this tells how it happened
      break;
    case DISEASE_DONE:
      if (victim->getPosition() > POSITION_DEAD) {
        victim->sendTo("The water around you evaporates.  You can breath again!\n\r");
        act("Oxygen returns to $n as $e takes a deep breath of air.",TRUE,victim,0,0,TO_ROOM);
      }
      break;
    default:
      break;
  }
  return FALSE;
}
int disease_herpes(TBeing *, int, affectedData *)
{
  return FALSE;
}

int disease_broken_bone(TBeing *, int, affectedData *)
{
  return FALSE;
}

static bool badSpreadSlot(const TBeing *ch, wearSlotT slot)
{
  return (!ch->hasPart(slot) ||
          slot == HOLD_RIGHT ||
          slot == HOLD_LEFT);
}

void TBeing::bodySpread(int chance_to_spread, affectedData * af)
{
  affectedData vaf;
  wearSlotT choice1 = WEAR_NOWHERE;
  wearSlotT choice2 = WEAR_NOWHERE;
  wearSlotT part = wearSlotT(af->level);
  int brakes;
  // note, taht we allow WEAR_NOWHERE since it's the starting point
  if(part < WEAR_NOWHERE || part >= MAX_WEAR){
    vlogf(LOG_BUG, format("bodySpread called with bad slot: %i") % part);
    return;
  }


  if (::number(1, 50000) > chance_to_spread)
    return;

  if(isImmortal())
    return;

  if(roomp->isRoomFlag(ROOM_PEACEFUL))
    return;

  if (part == WEAR_NOWHERE) {
    // is there a chance this could loop infinitely?
    // like if some race has no limbs or something?
    // adding brakes...
    brakes = 20;
    do {
      choice1 = pickRandomLimb();
      --brakes;
    } while (brakes && badSpreadSlot(this, choice1));
    if (!brakes)
      choice1 = WEAR_NOWHERE;
    brakes = 20;
    do {
      choice2 = pickRandomLimb();
      --brakes;
    } while (brakes && badSpreadSlot(this, choice2));
    if (!brakes)
      choice2 = WEAR_NOWHERE;
    
  } else if (isLimbFlags(part, PART_INFECTED | PART_LEPROSED)) {
    switch (part) {
      case WEAR_FINGER_R:
        choice1 = WEAR_HAND_R;
        choice2 = WEAR_FINGER_L;
        break;
      case WEAR_FINGER_L:
        choice1 = WEAR_HAND_L;
        choice2 = WEAR_FINGER_R;
        break;
      case WEAR_HAND_R:
        choice1 = WEAR_FINGER_R;
        choice2 = WEAR_WRIST_R;
        break;
      case WEAR_HAND_L:
        choice1 = WEAR_FINGER_L;
        choice2 = WEAR_WRIST_L;
        break;
      case WEAR_WRIST_R:
        choice1 = WEAR_HAND_R;
        choice2 = WEAR_ARM_R;
        break;
      case WEAR_WRIST_L:
        choice1 = WEAR_HAND_L;
        choice2 = WEAR_ARM_L;
        break;
      case WEAR_ARM_R:
        choice1 = WEAR_WRIST_R;
        choice2 = WEAR_BODY;
        break;
      case WEAR_ARM_L:
        choice1 = WEAR_WRIST_L;
        choice2 = WEAR_BODY;
        break;
      case WEAR_NECK:
        choice1 = WEAR_HEAD;
        choice2 = WEAR_BODY;
        break;
      case WEAR_HEAD:
        choice1 = choice2 = WEAR_NECK;
        break;
      case WEAR_BODY:
        if (::number(0,2))
          choice1 = WEAR_WAIST;
        else
          choice1 = WEAR_NECK;
        if (::number(0,1))
          choice2 = WEAR_ARM_R;
        else
          choice2 = WEAR_ARM_L;
        break;
      case WEAR_FOOT_R:
        choice1 = WEAR_LEG_R;
        choice2 = WEAR_FOOT_L;
        break;
      case WEAR_FOOT_L:
        choice1 = WEAR_LEG_L;
        choice2 = WEAR_FOOT_R;
        break;
      case WEAR_LEG_R:
        choice1 = WEAR_FOOT_R;
        choice2 = WEAR_WAIST;
        break;
      case WEAR_LEG_L:
        choice1 = WEAR_FOOT_L;
        choice2 = WEAR_WAIST;
        break;
      case WEAR_WAIST:
        choice1 = WEAR_BODY;
        if (::number(0,1))
          choice2 = WEAR_LEG_R;
        else
          choice2 = WEAR_LEG_L;
        break;
      case WEAR_BACK:
        choice1 = choice2 = WEAR_BODY;
        break;
      case WEAR_NOWHERE:
      case HOLD_RIGHT:
      case HOLD_LEFT:
      case WEAR_EX_LEG_R:
      case WEAR_EX_LEG_L:
      case WEAR_EX_FOOT_R:
      case WEAR_EX_FOOT_L:
      case MAX_WEAR:
//      default:
        return;  // nowhere on body      
    }
  } else
    return;  // bogus?

  vaf.type = AFFECT_DISEASE;

  vaf.modifier = af->modifier;
  vaf.location = af->location;
  vaf.bitvector = af->bitvector;

  if (af->modifier == DISEASE_INFECTION) {
		// the goal here is to have the spread rate keep step with the dissipation rate
		// so that infections generally just slow down healing
		// the older method guaranteed death over time
		// this would probably be neater if I understood probability...
		int spread_em = (50000 / chance_to_spread) + ::number(-(chance_to_spread / 10), (chance_to_spread / 10));
    vaf.duration = spread_em;
    if (af->duration == PERMANENT_DURATION) {
      // slightly weaken if spreading from a permanent infection (like from gangrene)... too deadly...
      vaf.duration = max(10, vaf.duration - 15);
    }
    if (choice1 && !isLimbFlags(choice1, PART_INFECTED) && hasPart(choice1))
      vaf.level = choice1;
    else if (choice2 && !isLimbFlags(choice2, PART_INFECTED) && hasPart(choice2))
      vaf.level = choice2;
    else
      return;   // already infected, do nothing
    affectTo(&vaf);
    wearSlotT slot = wearSlotT(vaf.level);
    if (!part)
			sendTo(format("Infection has set into your %s!\n\r") % describeBodySlot(slot));
    else
			sendTo(format("Infection spreads to your %s!\n\r") % describeBodySlot(slot));
			
    disease_start(this, &vaf);
  } else {  
	  // leprosy- slow uptake, but will persist well over time
    if (af->duration == PERMANENT_DURATION) {
      vaf.duration = (50000 / chance_to_spread) * 2;
    } else {
      vaf.duration = min(af->duration + ((50000 / chance_to_spread) * 2), 1200);
		}
    if (choice1 && !isLimbFlags(choice1, PART_LEPROSED) && hasPart(choice1))
      vaf.level = choice1;
    else if (choice2 && !isLimbFlags(choice2, PART_LEPROSED) && hasPart(choice2))
      vaf.level = choice2;
    else
      return;   // already leprosed, do nothing
    affectTo(&vaf);

    wearSlotT slot = wearSlotT(vaf.level);
    sendTo(format("Lesions begin to form on your %s and it begins to feel numb!\n\r") % describeBodySlot(slot));
    disease_start(this, &vaf);
  }
  return;
}

void TBeing::dummyLeprosy(wearSlotT part)
{
  if (!::number(0, 199)) {
      sendTo(format("The lesions covering your %s thicken.\n\r") % describeBodySlot(part));
      act(format("The lesions covering $n's %s appear thicker.") % describeBodySlot(part), TRUE, this, 0, 0, TO_ROOM);
  }
}

int disease_leprosy(TBeing *victim, int message, affectedData * af)
{
  affectedData vaf;
  wearSlotT slot = wearSlotT(af->level);

  if(slot < WEAR_NOWHERE || slot >= MAX_WEAR){
    vlogf(LOG_BUG, format("disease_leprosy called with bad slot: %i") % slot);
    return FALSE;
  }
  switch (message) {
    case DISEASE_PULSE:
      if (slot && (!victim->hasPart(slot) || !victim->isLimbFlags(slot, PART_LEPROSED))) {
        af->duration = 0;
        break;
      }
      if (slot)
        victim->dummyLeprosy(slot);
      else {
        vaf.type = AFFECT_DISEASE;
        vaf.level = 0;
        vaf.duration = PERMANENT_DURATION;
        vaf.modifier = DISEASE_LEPROSY;
        vaf.location = APPLY_NONE;
        vaf.bitvector = 0;
        spread_affect(victim, 20, FALSE, FALSE, &vaf);
      }
      victim->bodySpread(375, af);
      break;
    case DISEASE_BEGUN:
      if (slot) {
        victim->addToLimbFlags(slot, PART_LEPROSED);
			} else {
		    act("Your skin becomes scaly and insensitive to feeling!", FALSE, victim, 0, 0, TO_CHAR);
	      act("$n's skin begins to look scaly!", TRUE, victim, NULL, NULL, TO_ROOM);
			}
      break;
    case DISEASE_DONE:
      if (slot) {
        victim->remLimbFlags(slot, PART_LEPROSED);
        if (victim->hasPart(slot)) {
          act(format("The skin of your %s softens a bit and the feeling returns.") % victim->describeBodySlot(slot), FALSE, victim, 0, 0, TO_CHAR);
        }
			} else if (victim->getPosition() > POSITION_DEAD) {
		    act("The hardened lesions on your skin disappear.", FALSE, victim, 0, 0, TO_CHAR);
	      act("The lesions covering $n disappear.", TRUE, victim, NULL, NULL, TO_ROOM);
			}
      break;
  }
  return FALSE;
}

int disease_plague(TBeing *victim, int message, affectedData * af)
{
  int rc;
  affectedData vaf;
  wearSlotT slot = wearSlotT(af->level);

  if(slot < WEAR_NOWHERE || slot >= MAX_WEAR){
    vlogf(LOG_BUG, format("disease_plague called with bad slot: %i") % slot);
    return FALSE;
  }


  switch (message) {
    case DISEASE_PULSE:
      if (slot != WEAR_NOWHERE &&
         !victim->isLimbFlags(slot, PART_LEPROSED)) {
        af->duration = 0;
        break;
      }
      vaf.type = AFFECT_DISEASE;
      vaf.location = APPLY_NONE;
      vaf.bitvector = 0;
      vaf.modifier2 = af->modifier2;
      if (slot != WEAR_NOWHERE)
        victim->dummyLeprosy(slot);
      else {
        if (victim->isHumanoid()) {
          rc = victim->dummyFlu();
          if (IS_SET_DELETE(rc, DELETE_THIS))
            return DELETE_THIS;
          rc = victim->dummyCold();
          if (IS_SET_DELETE(rc, DELETE_THIS))
            return DELETE_THIS;
        }
        vaf.level = 0;
        vaf.duration = 500;
        vaf.modifier = DISEASE_PLAGUE;
        spread_affect(victim, 40, false, false, &vaf);
      }
      // spread an infection+leprosy
      vaf.duration = 250;
      vaf.level = slot;
      vaf.modifier = DISEASE_INFECTION;
      victim->bodySpread(500,&vaf);
      vaf.modifier = DISEASE_LEPROSY;
      victim->bodySpread(500,&vaf);
      break;
    case DISEASE_BEGUN:
	    act("Painful lumps form over your body and <o>dark, weeping sores<1> appear!", FALSE, victim, 0, 0, TO_CHAR);
      act("Lumps and <o>dark splotches<1> begin to cover $n's body!", TRUE, victim, NULL, NULL, TO_ROOM);
      if (slot != WEAR_NOWHERE)
        victim->addToLimbFlags(slot, PART_LEPROSED);
      break;
    case DISEASE_DONE:
      if (victim->getPosition() > POSITION_DEAD) {
		    act("The festering sores and lumps covering your body disappear!", FALSE, victim, 0, 0, TO_CHAR);
	      act("The festering sores and lumps covering $n's body disappear!!", TRUE, victim, NULL, NULL, TO_ROOM);
      }
      break;
  }
  return FALSE;
}

int disease_gangrene(TBeing *victim, int message, affectedData *af)
{
  // produces limb rot (non-spreading) + infection (spreads) + fever & damage when on critical slots
    
  if (victim->isPc() && !victim->desc)
    return FALSE;

  wearSlotT slot = wearSlotT(af->level); // defines the limb that has gangrene

  if (slot < MIN_WEAR || slot >= MAX_WEAR){
    vlogf(LOG_BUG, format("disease_gangrene called with bad slot: %i on %s") % slot % victim->getName());
    return FALSE;
  }
  
  switch (message) {
    case DISEASE_BEGUN:
      victim->addToLimbFlags(slot, PART_GANGRENOUS);
	    act(format("Your %s has become <k>gangrenous<1>!") % victim->describeBodySlot(slot), FALSE, victim, 0, 0, TO_CHAR);
      act(format("$n's %s has become <k>gangrenous<1>!") % victim->describeBodySlot(slot), TRUE, victim, NULL, NULL, TO_ROOM);
      break;
    case DISEASE_PULSE:
      if (isCritPart(slot) && victim->getCurLimbHealth(slot) <= 0) {
        // do something nasty? crit body parts just stay at 0...
        // vlogf(LOG_MISC, format("%s at 0 on %s") % victim->describeBodySlot(slot) % victim->getName());
      }

      // check to see if somehow the gangrene bit got taken off
      if (!victim->hasPart(slot) || !victim->isLimbFlags(slot, PART_GANGRENOUS)) {
        af->duration = 0;
        break;
      }
      
      if (!number(0, 10)) {
        switch (number(0, 1)) {
          case 0:
            victim->sendTo(format("You are in a world of pain as your %s darkens and swells.\n\r") % victim->describeBodySlot(slot));
            break;
          case 1:
					  act(format("Your %s discharges a <o>foul<1> <k>black<1> ichor.  The pain!") % victim->describeBodySlot(slot), FALSE, victim, 0, 0, TO_CHAR);
            break;
          default:
            // should never fall through here
            break;
        }
        act("The foul stench of dying flesh surrounds $n.", FALSE, victim, NULL, NULL, TO_ROOM);

        // do some damage if it's a critical body part
        if (isCritPart(slot))
          if (victim->reconcileDamage(victim, ::number(1, 4), SPELL_INFECT) == -1)
            return DELETE_THIS;

        // start a permanent infection in the gangrenous slot
        // infection will spread but not as ravenously as other infections
        if (!number(0, 1)) {
          if (!victim->isLimbFlags(slot, PART_INFECTED)) {
            victim->rawInfect(slot, PERMANENT_DURATION, SILENT_NO, CHECK_IMMUNITY_YES, af->modifier2);
        }
      }
			break;
    }

    // cause flu effect when a critical body part is gangrenous
    if (isCritPart(slot) && !number(0, 15))
      if (victim->isHumanoid())
        if (IS_SET_DELETE(victim->dummyFlu(), DELETE_THIS))
          return DELETE_THIS;
      break;
    case DISEASE_DONE:
      // infection duration will be shortened from permanent in the disease_infect pulse
      if (victim->isLimbFlags(slot, PART_GANGRENOUS))
        victim->remLimbFlags(slot, PART_GANGRENOUS);
      if (victim->getPosition() > POSITION_DEAD && victim->hasPart(slot)) {
        act(format("You experience profound relief as the <k>gangrene<1> in your %s disappears!") % victim->describeBodySlot(slot), FALSE, victim, 0, 0, TO_CHAR);
        act(format("$n's necrotic %s regains some of its health!") % victim->describeBodySlot(slot).c_str(), TRUE, victim, NULL, NULL, TO_ROOM);
      }
      break;
    default:
      // do nothing
      break;
  }
  return FALSE;
}

int disease_scurvy(TBeing *victim, int message, affectedData *af)
{
  // spontaneous bleeding and tooth loss, increased chance of bruising in combat
  sstring buf;
  bool found = FALSE;
  switch (message) {
    case DISEASE_BEGUN:
	    act("Your skin <w>pales<1> and your eyes begin to hurt.", FALSE, victim, 0, 0, TO_CHAR);
      act("$n pales and $s eyes become hollow and bl<r>oo<1>dsh<r>o<1>t.", TRUE, victim, NULL, NULL, TO_ROOM);
      break;
    case DISEASE_PULSE:
      wearSlotT slot;
      TObj *corpse;
      TCorpse *tooth;
      switch (number(0, 500)) {
        case 0:
          // tooth falls out
          if (victim->getMyRace()->hasNoBones())
            return FALSE;
          corpse = read_object(Obj::GENERIC_TOOTH, VIRTUAL);
          corpse->swapToStrung();
          if ((tooth = dynamic_cast<TCorpse *>(corpse))) {
            tooth->setCorpseRace(victim->getRace());
            tooth->setCorpseLevel(victim->GetMaxLevel());
            tooth->setCorpseVnum(victim->mobVnum());
          }
          delete corpse->name;
          buf = format("tooth lost limb rotten %s") % victim->name;
          corpse->name = mud_str_dup(buf);

          delete corpse->shortDescr;
          buf = format("a <k>rotten<1> tooth of %s") % victim->getName();
          corpse->shortDescr = mud_str_dup(buf);

          delete corpse->descr;
          buf = format("A <k>rotten<1> tooth lies here, having fallen from %s's mouth.") % victim->getName();
          corpse->setDescr(mud_str_dup(buf));

          corpse->obj_flags.wear_flags = ITEM_TAKE | ITEM_HOLD | ITEM_THROW;
          corpse->obj_flags.decay_time=-1;
          corpse->setWeight(0.1);
          corpse->canBeSeen = victim->canBeSeen;
          corpse->setVolume(1);
          corpse->setMaterial(MAT_BONE);
          corpse->obj_flags.cost=50;
          act("Your gums ache and a <w>tooth<1> comes loose, falling to the $g!", FALSE, victim, 0, 0, TO_CHAR);
          act("A <k>rotten<1> tooth falls from $n's mouth.", FALSE, victim, corpse, NULL, TO_ROOM);
          *victim->roomp += *corpse;
          break;
        case 1:
        case 2:
        case 3:
        case 4:
          if (victim->isUndead())
            return FALSE;
          // bruise- 1st, find a limb
          found = FALSE;
          for (int i = 0; i < 20; ++i) {
            slot = pickRandomLimb();
            if (notBleedSlot(slot))
              continue;
            if (!victim->hasPart(slot))
              continue;
            if (victim->isLimbFlags(slot, PART_BRUISED))
              continue;
            if (victim->isImmune(IMMUNE_SKIN_COND, slot))
             continue;
            found = TRUE;
            break;
          }
          if (!found)
            return FALSE;
          if (victim->rawBruise(slot, 225, SILENT_YES, CHECK_IMMUNITY_NO)) {
            // note: bruise duration is doubled in rawBruise for all you scurvy coves
            act(format("Your %s feels painfully tender and a <p>mottled<1> <P>bruise<1> blooms.") % victim->describeBodySlot(slot), FALSE, victim, 0, 0, TO_CHAR);
            act(format("A mottled bruise forms on $n's %s.") % victim->describeBodySlot(slot), TRUE, victim, NULL, NULL, TO_ROOM);
          }
          break;
        case 5:
        case 6:
        case 7:
          if (!victim->isUndead()) {
            // start bleeding
            // find a suitable slot to bleed 
            found = FALSE;
            for (int i = 0; i < 20; ++i) {
              slot = pickRandomLimb();
              if (notBleedSlot(slot))
                continue;
              if (!victim->hasPart(slot))
                continue;
              if (victim->isLimbFlags(slot, PART_BLEEDING))
                continue;
              if (victim->isImmune(IMMUNE_BLEED, slot))
               continue;
              found = TRUE;
              break;
            }
            if (found) {
              int duration = ::number(100,  150);
              victim->rawBleed(slot, duration, SILENT_YES, CHECK_IMMUNITY_NO);
              act(format("Your %s feels painfully tender and begins to <r>bleed<1>.") % victim->describeBodySlot(slot), FALSE, victim, 0, 0, TO_CHAR);
              act(format("$n's %s starts to bleed.") % victim->describeBodySlot(slot), TRUE, victim, NULL, NULL, TO_ROOM);
            }
          }
          break;
        default:
          break;
      }
      break;
    case DISEASE_DONE:
      if (victim->getPosition() > POSITION_DEAD) {
        act("Your are <c>relieved<1> of some of your aches and pains!", FALSE, victim, 0, 0, TO_CHAR);
        act("Some <c>life<1> returns to $n's eyes.", TRUE, victim, NULL, NULL, TO_ROOM);
      }
      break;
    default:
    // do nothing
      break;
  }
  return FALSE;
}

int disease_dysentery(TBeing *victim, int message, affectedData *af)
{
  // diarrhea, thirst & some mana drain
  switch (message) {
    case DISEASE_BEGUN:
      act("Something rotten in your guts causes you to double over momentarily.", FALSE, victim, 0, 0, TO_CHAR);
      if (victim->isHumanoid()) {
        act("$n pales and doubles over momentarily.", TRUE, victim, NULL, NULL, TO_ROOM);
			} else {
        act("$n winces and doubles over momentarily.", TRUE, victim, NULL, NULL, TO_ROOM);
			}
      break;
    case DISEASE_PULSE:
      switch (number(0, 50)) {
			  case 0:
        case 1:
			  case 2:
          // weaken
          victim->sendTo("There is a painful knot in your stomach and you feel clammy.\n\r");
          victim->setMana(max((victim->getMana() - 35), 0));
          victim->addToLifeforce(-2);
          break;
        case 3:
			  case 4:
          // poop
          if (victim->isPc()) {
            if (victim->getCond(POOP) <= 0) {
              victim->setCond(POOP, ::number(2, 6));
              victim->sendTo("You experience a hot flash and your bowels clench.\n\r");
              if (victim->isHumanoid()) {
                act("$n looks pale and sweaty.", TRUE, victim, NULL, NULL, TO_ROOM);
				      } else {
                act("$n looks uncomfortable.", TRUE, victim, NULL, NULL, TO_ROOM);
              }
            } else {
               victim->sendTo("Your stomach is gripped by a dreadful spasm and you cannot contain yourself.\n\r");
               act("You have <o>besmirched<1> yourself!", FALSE, victim, NULL, NULL, TO_CHAR);
               act("The air around $n is woefully befouled as $e <o>besmirches<1> $mself.", FALSE, victim, NULL, NULL, TO_ROOM);
               victim->dropPool(min((int) victim->getWeight() / 10, (int) victim->getCond(POOP)), LIQ_POT_FILTH);
               victim->setCond(THIRST, max(0, ((int) (victim->getCond(THIRST) - (2 * (int) victim->getCond(POOP))))));
               victim->setCond(POOP, 0);
            }
          } else {
            switch (::number(0, 1)) {
              // doesn't actually do anything to thirstless & pooless mobs, so this is just for color
              case 0:
                victim->sendTo("You experience a hot flash and your bowels clench.\n\r");
	              if (victim->isHumanoid()) {
	                act("$n looks pale and sweaty.", TRUE, victim, NULL, NULL, TO_ROOM);
					      } else {
	                act("$n looks uncomfortable.", TRUE, victim, NULL, NULL, TO_ROOM);
	              }
								break;
              case 1:
                victim->sendTo("Your stomach is gripped by a dreadful spasm and you cannot contain yourself.\n\r");
                act("You have <o>besmirched<1> yourself!", FALSE, victim, NULL, NULL, TO_CHAR);
                act("The air around $n is woefully befouled as $e <o>besmirches<1> $mself.", FALSE, victim, NULL, NULL, TO_ROOM);
                victim->dropPool(::number(2, 6), LIQ_POT_FILTH);
								break;
              default:
                // nothing should fall through here
                break;
            }
          }
          victim->addToDistracted(1, FALSE);
          break;
        default:
          break;
      }
      break;
    case DISEASE_DONE:
      if (victim->getPosition() > POSITION_DEAD) {
        victim->sendTo("The gnawing in your stomach ceases and you can walk normally again!\n\r");
        act("$n appears as if a weight were lifted from $s back.", TRUE, victim, NULL, NULL, TO_ROOM);
      }
      break;
    default:
    // do nothing
      break;
  }
  return FALSE;
}

int disease_pneumonia(TBeing *victim, int message, affectedData *af)
{
  affectedData vaf;
  // -moves, coughing & suffocation damage
  switch (message) {
    case DISEASE_BEGUN:
      act("Your chest begins to hurt and breathing becomes difficult.", FALSE, victim, 0, 0, TO_CHAR);
	    act("$n looks <w>pale<1> and $s breathing becomes labored.", TRUE, victim, NULL, NULL, TO_ROOM);
      break;
    case DISEASE_PULSE:
      // spread cold, flu, pneumonia
      vaf.type = AFFECT_DISEASE;
      vaf.level = 0;
      vaf.location = APPLY_NONE;
      vaf.bitvector = 0;
      vaf.modifier2 = af->modifier2;
      vaf.duration = min(150, af->duration + ::number(-150, 150));
      vaf.modifier = DISEASE_PNEUMONIA;
      spread_affect(victim, 25, false, false, &vaf);
      vaf.duration = min(150, af->duration + ::number(-150, 150));
      vaf.modifier = DISEASE_FLU;
      spread_affect(victim, 50, false, false, &vaf);
      vaf.duration = min(150, af->duration + ::number(-150, 150));
      vaf.modifier = DISEASE_COLD;
      spread_affect(victim, 75, false, false, &vaf);

      switch (number(0, 49)) {
        case 0:
        case 1:
          // heart and breath: -moves
           if (victim->isUndead())
             return FALSE;
           victim->sendTo("Your heart pounds and you can barely breath.\n\r");
           act("$n begins to pant in short, sharp breaths.", TRUE, victim, 0, 0, TO_ROOM);
          victim->setMove(max((victim->getMove() - 30), 0));
          break;
        case 2:
        case 3:
          // cough blood: damage
          act("You cough violently, bringing up <G>phelgm<1> and <r>blood<1>.", FALSE, victim, 0, 0, TO_CHAR);
          act("$n erupts into a hoarse, hacking cough.  It does not sound good.", TRUE, victim, 0, 0, TO_ROOM);
          if (victim->reconcileDamage(victim, ::number(2, 4), SPELL_DISEASE) == -1)
            return DELETE_THIS;
          break;
        case 4:
          // suffocation: suffocation damage
          int dam = ::number(5, 15);
          dam *= (100 - victim->getImmunity(IMMUNE_SUFFOCATION));
          dam /= 100;
          if (dam <= 0)
            return FALSE;
          victim->sendTo("Your lungs burn with pain and you cannot breathe!\n\r");
          act("$n's face <b>darkens<1> as $e struggles to draw a breath.", TRUE, victim, 0, 0, TO_ROOM);
          victim->addToMove(-::number(0, (10 * ((100 - victim->getImmunity(IMMUNE_SUFFOCATION)) / 100))));
          if (victim->reconcileDamage(victim, dam, DAMAGE_SUFFOCATION) == -1)
            return DELETE_THIS;
          break;
      default:
        break;
      }
      break;
    case DISEASE_DONE:
      if (victim->getPosition() > POSITION_DEAD) {
        victim->sendTo("Your lungs clear and you regain the strength to draw breath!\n\r");
        act("$n loses some of $s pallor.", TRUE, victim, NULL, NULL, TO_ROOM);
      }
      break;
    default:
      // do nothing
      break;
  }
  return FALSE;
}

DISEASEINFO DiseaseInfo[MAX_DISEASE] =
{
  {disease_null,"bogus disease!",0},
  {disease_cold,"the common cold",250},
  {disease_flu,"the flu",500},
  {disease_frostbite,"frostbite",2250},
  {disease_bleeding,"bleeding",250},
  {disease_infection,"infection",450},
  {disease_herpes,"herpes",775},
  {disease_broken_bone,"broken bone",500},
  {disease_numbed,"numbed limb",700},
  {disease_voicebox,"a punctured voicebox",4000},
  {disease_eyeball,"a missing eyeball", 4000},
  {disease_lung,"a punctured lung",2750},
  {disease_stomach,"a stomach wound",3300},
  {disease_hemorraging,"internal bleeding",3500},
  {disease_leprosy,"leprosy",800},
  {disease_plague,"THE PLAGUE!", 10220},
  {disease_suffocate,"a breathing problem",10000},
  {disease_food_poison, "food poisoning", 2450},
  {disease_drowning,"drowning",10000},
  {disease_garrotte,"a breathing problem",100000},
  {disease_poison,"poison",450},
  {disease_syphilis,"syphilis",8500},
  {disease_bruised,"bruised",200},
  {disease_scurvy,"scurvy",600},
  {disease_dysentery,"dysentery",375},
  {disease_pneumonia,"pneumonia",650},
  {disease_gangrene,"gangrene",1250},
};

