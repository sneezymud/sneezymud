
//////////////////////////////////////////////////////////////////////////
//
// SneezyMUD - All rights reserved, SneezyMUD Coding Team
//
//      "disease.cc" - functions handling disease affects
//
///////////////////////////////////////////////////////////////////////////

#include "stdsneezy.h"
#include "disease.h"
#include "obj_tool.h"

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
  vlogf(LOG_BUG, fmt("WARNING:  %s has a bogus disease #%d affect.") % 
	((victim) ? victim->getName() : "A null pointer"));
  return FALSE;
}

void spread_affect(TBeing *ch, int chance_to_spread, bool race, bool not_race, affectedData * af)
{
  TThing *t;

  if (ch->inRoom() == ROOM_NOCTURNAL_STORAGE)
    return;

  // Do not spread disease in peacful zones, this can become real hairy.
  if (ch->roomp && ch->roomp->isRoomFlag(ROOM_PEACEFUL))
    return;

  for (t = ch->roomp->getStuff(); t; t = t->nextThing) {
    TBeing *v = dynamic_cast<TBeing *>(t);
    if (!v || v == ch)
      continue;
    if (v->isImmortal())
      continue;
    if (v->isImmune(IMMUNE_DISEASE))
      continue;
    if (number(1,50000) >= chance_to_spread)
      continue;
    if (race && (v->getRace() != race))
      continue;
    if (not_race && (v->getRace() == not_race))
      continue;

    // guess who shouldn't get diseases...
    if (v->spec == SPEC_HORSE_PESTILENCE ||
        v->isShopkeeper() ||
        v->mobVnum() == APOC_PESTHORSE)
      continue;

    if ((af->type != AFFECT_DISEASE && !v->affectedBySpell(af->type)) ||
        (af->type == AFFECT_DISEASE && !v->hasDisease(affToDisease(*af)))) {

#if 0
      vlogf(LOG_MISC, fmt("%s (%s:%d) spreading from %s to %s at %d") % 
             af->type == AFFECT_DISEASE ? "Disease" : "Spell" %
             af->type == AFFECT_DISEASE ? 
                 DiseaseInfo[af->modifier].name : 
                 discArray[af->type]->name %
             af->type == AFFECT_DISEASE ? af->modifier : af->type %
             ch->getName() % v->getName() % ch->inRoom());
#endif

      v->affectTo(af, TRUE);
      if (af->type == AFFECT_DISEASE)
        disease_start(v, af);
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
        sendTo("Puking in your sleep has cause you to gag!\n\r");
        if ((rc = reconcileDamage(this, ::number(1,10) +15, DAMAGE_SUFFOCATION)) == -1)
          return DELETE_THIS; 
      }
      break;
    case 3:
    case 4:
    case 5:
    case 6:
      act("You begin to sweat for no apparent reason.",TRUE,this,0,0,TO_CHAR);
      act("$n is sweating and $e looks feverish.",TRUE,this,0,0,TO_ROOM);
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
      sendTo("You suddenly feel immensely tired.  It must be this damn flu.\n\r");
      setMove(max((getMove() - 50), 0));
      if (reconcileDamage(this, 5, SPELL_DISEASE) == -1)
        return DELETE_THIS;
      break;
    case 16:
    case 17:
    case 18:
      act("The world starts to spin and the landscape seems to leap upwards.",
           TRUE,this,0,0,TO_CHAR);
      if (riding) {
        act("$n sways then crumples as $e faints.",TRUE,this,0,0,TO_ROOM);
        rc = fallOffMount(riding, POSITION_RESTING);
        if (IS_SET_DELETE(rc, DELETE_THIS)) {
          return DELETE_THIS;
        }
      } else
        act("$n stumbles then crumples as $e faints.",TRUE,this,0,0,TO_ROOM);
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
      sendTo("You breathe in short shallow gasps from the effects of this fever.\n\r");
      act("$n's fever gets worse and $e starts breathing in short, shallow gasps.",TRUE,this,0,0,TO_ROOM);
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

  if (message == DISEASE_PULSE) {
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
  } else if (message == DISEASE_BEGUN) {
    // no warning 
    return FALSE;
  } else if (message == DISEASE_DONE) {
    if (victim->getPosition() > POSITION_DEAD) {
      act("$n relaxes a bit.", FALSE, victim, NULL, NULL, TO_ROOM);
      victim->sendTo("You feel much better, as your stomach settles down.\n\r");
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

  if (message == DISEASE_PULSE) {
    if (victim->isHumanoid()) {
      rc = victim->dummyCold();
      if (IS_SET_DELETE(rc, DELETE_THIS))
        return DELETE_THIS;
    }
    vaf.type = AFFECT_DISEASE;
    vaf.level = 0;
    vaf.duration = 100;
    vaf.modifier = DISEASE_COLD;
    vaf.location = APPLY_NONE;
    vaf.bitvector = 0;
    spread_affect(victim, 50, false, false, &vaf);
  } else if (message == DISEASE_BEGUN) {
    act("$n doesn't look very well.", FALSE, victim, NULL, NULL, TO_ROOM);
    victim->sendTo("You don't feel very well at all.\n\r");
    return FALSE;
  } else if (message == DISEASE_DONE) {
    if (victim->getPosition() > POSITION_DEAD) {
      act("Some color has returned to $n's skin.", FALSE, victim, NULL, NULL, TO_ROOM);
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
    vlogf(LOG_BUG, fmt("disease_numbed called with bad slot: %i") % slot);
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
                                                                            
      if (victim->getPosition() > POSITION_DEAD) {
        victim->sendTo(fmt("You feel the life come back to your %s!\n\r") % victim->describeBodySlot(slot));
      }
      break;                                                                    
    default:
      break;
  }
  return FALSE;
}

int disease_bleeding(TBeing *victim, int message, affectedData *af)
{
  int j, dam, amt;
  char buf[256];
  // defines the limb that is bleeding
  wearSlotT i = wearSlotT(af->level);


  if(i < MIN_WEAR || i >= MAX_WEAR){
    vlogf(LOG_BUG, fmt("disease_bleeding called with bad slot: %i") % i);
    return FALSE;
  }


  if (victim->isPc() && !victim->desc)
    return FALSE;

  switch (message) {
    case DISEASE_BEGUN:
      victim->addToLimbFlags(i, PART_BLEEDING);
      break;
    case DISEASE_PULSE:
      // defines the severity of the bleeding
      j = af->modifier2;

      // check to see if somehow the bleeding bit got taken off
      if (!victim->slotChance(i) ||
          !victim->isLimbFlags(i, PART_BLEEDING) ||
          victim->isLimbFlags(i, PART_MISSING)) {
	af->duration = 0;
	break;
      }

      if (!number(0, 10)) {
        if (victim->doesKnowSkill(SKILL_SNOFALTE)) {
          // attempt to dodge it altogether
          amt = victim->getSkillValue(SKILL_SNOFALTE);
          if ((::number(0,99) < 40) &&
               victim->bSuccess(amt, SKILL_SNOFALTE)) {
            victim->sendTo("You utilize the powers of snofalte to slow your bleeding.\n\r");
            break;
          }
        }
	victim->sendTo(fmt("You feel your energy drained as your blood drips out of your %s.\n\r") %victim-> describeBodySlot(i));
	sprintf(buf, "$n looks stunned as blood drips from $s %s.", victim->describeBodySlot(i).c_str());

	act(buf, FALSE, victim, NULL, NULL, TO_ROOM);
	victim->dropBloodLimb(i);
        int rc = victim->hurtLimb(1, i);
        if (IS_SET_DELETE(rc, DELETE_THIS))
          return DELETE_THIS;

        dam = (VITAL_PART(i) ? 2 * j : j);
        dam *= victim->GetMaxLevel();
        dam /= 25;
        dam *= (100 - max(0, (int) victim->getSkillValue(SKILL_SNOFALTE)));
        dam /= 100;
        dam = max(1, dam);
        if (victim->reconcileDamage(victim, dam, SPELL_BLEED) == -1)
          return DELETE_THIS;
      }
      break;
    case DISEASE_DONE:
      if (victim->isLimbFlags(i, PART_BLEEDING))
	victim->remLimbFlags(i, PART_BLEEDING);

      if (victim->getPosition() > POSITION_DEAD) {
        victim->sendTo(fmt("Your %s stops bleeding and clots!\n\r") % victim->describeBodySlot(i));
        sprintf(buf, "$n's %s stops bleeding and clots!", victim->describeBodySlot(i).c_str());
        act(buf, FALSE, victim, NULL, NULL, TO_ROOM);
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
    vlogf(LOG_BUG, fmt("disease_infection called with bad slot: %i") % slot);
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
      if (!victim->isLimbFlags(slot, PART_INFECTED)) {
	af->duration = 0;
	break;
      }

      if (!number(0, 10)) {
	victim->sendTo(fmt("Your %s shakes and twinges as the infection in it festers.\n\r") % victim->describeBodySlot(slot));
	sprintf(buf, "$n's %s shakes and twinges as the infection in it festers.", victim->describeBodySlot(slot).c_str());
	act(buf, FALSE, victim, NULL, NULL, TO_ROOM);
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

      if (victim->getPosition() > POSITION_DEAD) {
        victim->sendTo(fmt("Your %s stops twitching and heals!\n\r") % victim->describeBodySlot(slot));
        sprintf(buf, "$n's %s stops twitching and heals!", victim->describeBodySlot(slot).c_str());
        act(buf, FALSE, victim, NULL, NULL, TO_ROOM);
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
      act("$n groans in agony.",TRUE,victim,0,0,TO_ROOM);
      dam = number(1,3);
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
      if (number(0,15))
        break;
      victim->doAction("", CMD_SHIVER);
      break;
    case DISEASE_BEGUN:
	for (i = MIN_WEAR; i < MAX_WEAR; i++) {
	  if (victim->validEquipSlot(i) && !(victim->equipment[i]) && !number(0, 3)) {
	    victim->addToLimbFlags(i, PART_USELESS);
	    sprintf(buf, "$n's %s takes on a gray-blue color as frost-bite sets in.", victim->describeBodySlot(i).c_str());
	    act(buf, TRUE, victim, NULL, NULL, TO_ROOM);
	    victim->sendTo(fmt("Your %s feels numb and takes on a gray-blue color.  Looks like frostbite.\n\r") % victim->describeBodySlot(i));
	  }
        }
	break;
    case DISEASE_DONE:
      if (victim->getPosition() > POSITION_DEAD) {
        act("$n still looks cold, but the shivering has stopped.", FALSE, victim, NULL, NULL, TO_ROOM);
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
      if ((victim->getCond(DRUNK) > 0) && number(0,1))
        victim->gainCondition(DRUNK, -1);
      if (victim->getCond(THIRST) > 0)
        victim->gainCondition(THIRST, -1);
      else if (!victim->getCond(THIRST)) {
        victim->sendTo("Your stomach wound causes an immense thirst!\n\r");
        victim->doSay("Water, please, someone give me water...");
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
      dam = number(1,3);
      if (victim->reconcileDamage(victim, dam, DAMAGE_SUFFOCATION) == -1)
        return DELETE_THIS;
      break;
    case DISEASE_BEGUN:
      // code that sets this tells how it happened
      break;
    case DISEASE_DONE:
      if (victim->getPosition() > POSITION_DEAD) {
        victim->sendTo("The hole in your lung seems to close up and your lungs thankfully fill with air.\n\r");
        act("The hole in $n's lung heals and $e takes a big gulp of oxygen.",TRUE,victim,0,0,TO_ROOM);
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
      victim->sendTo("GASP! You can't breathe!\n\r");
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
                    TRUE,victim,0,0,TO_ROOM);
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

  victim->sendTo("GASP! You can't breathe!\n\r");
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
    act("$p snaps.", TRUE, victim, this, 0, TO_ROOM);
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
	  ch->dropPool(10, LIQ_VOMIT); 
          if (ch->getPosition() <= POSITION_SLEEPING) {
            ch->sendTo("Puking in your sleep has cause you to gag!\n\r");
            if ((rc = ch->reconcileDamage(ch, ::number(1,10) +15, DAMAGE_SUFFOCATION)) == -1)
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
      victim->sendTo("GASP! You can't breathe!\n\r");
      act("$n flails about and gasps, trying to breathe.",TRUE,victim,0,0,TO_ROOM);
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
  return (!ch->slotChance(slot) ||
          slot == HOLD_RIGHT ||
          slot == HOLD_LEFT);
}

void TBeing::bodySpread(int chance_to_spread, affectedData * af)
{
  affectedData vaf;
  char buf[255];
  wearSlotT choice1 = WEAR_NOWHERE;
  wearSlotT choice2 = WEAR_NOWHERE;
  wearSlotT part = wearSlotT(af->level);

  // note, taht we allow WEAR_NOWHERE since it's the starting point
  if(part < WEAR_NOWHERE || part >= MAX_WEAR){
    vlogf(LOG_BUG, fmt("bodySpread called with bad slot: %i") % part);
    return;
  }


  if (::number(1, 50000) > chance_to_spread)
    return;

  if(isImmortal())
    return;

  if(roomp->isRoomFlag(ROOM_PEACEFUL))
    return;

  if (part == WEAR_NOWHERE) {

    do {
      choice1 = pickRandomLimb();
    } while (badSpreadSlot(this, choice1));

    do {
      choice2 = pickRandomLimb();
    } while (badSpreadSlot(this, choice2));

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
          choice1 = WEAR_WAISTE;
        else
          choice1 = WEAR_NECK;
        if (::number(0,1))
          choice2 = WEAR_ARM_R;
        else
          choice2 = WEAR_ARM_L;
        break;
      case WEAR_FOOT_R:
        choice1 = WEAR_LEGS_R;
        choice2 = WEAR_FOOT_L;
        break;
      case WEAR_FOOT_L:
        choice1 = WEAR_LEGS_L;
        choice2 = WEAR_FOOT_R;
        break;
      case WEAR_LEGS_R:
        choice1 = WEAR_FOOT_R;
        choice2 = WEAR_WAISTE;
        break;
      case WEAR_LEGS_L:
        choice1 = WEAR_FOOT_L;
        choice2 = WEAR_WAISTE;
        break;
      case WEAR_WAISTE:
        choice1 = WEAR_BODY;
        if (::number(0,1))
          choice2 = WEAR_LEGS_R;
        else
          choice2 = WEAR_LEGS_L;
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
  vaf.duration = 400;
  vaf.modifier = af->modifier;
  vaf.location = af->location;
  vaf.bitvector = af->bitvector;

  if (af->modifier == DISEASE_INFECTION) {
    if (!isLimbFlags(choice1, PART_INFECTED) && slotChance(choice1))
      vaf.level = choice1;
    else if (!isLimbFlags(choice2, PART_INFECTED) && slotChance(choice2))
      vaf.level = choice2;
    else
      return;   // already infected, do nothing
    affectTo(&vaf);
    wearSlotT slot = wearSlotT(vaf.level);
    if (!part)
      sprintf(buf, "Infection has set into your %s.\n\r",
          describeBodySlot(slot).c_str());
    else {
      // describeBody uses static buffer
      char tmpb[64];
      sprintf(tmpb, describeBodySlot(slot).c_str());
      sprintf(buf, "The infection in your %s has spread to your %s!\n\r",
          describeBodySlot(part).c_str(), tmpb);
    }
    sendTo(buf);
    disease_start(this, &vaf);
  } else {  // leprosy 
    if (!isLimbFlags(choice1, PART_LEPROSED) && slotChance(choice1))
      vaf.level = choice1;
    else if (!isLimbFlags(choice2, PART_LEPROSED) && slotChance(choice2))
      vaf.level = choice2;
    else
      return;   // already leprosed, do nothing
    affectTo(&vaf);

    wearSlotT slot = wearSlotT(vaf.level);
    if (!part)
      sprintf(buf, "Your %s shows signs of leprosy!\n\r",
          describeBodySlot(slot).c_str());
    else {
      // describeBody uses static buffer
      char tmpb[64];
      sprintf(tmpb, describeBodySlot(slot).c_str());
      sprintf(buf, "Leprosy has spread from your %s to your %s!\n\r",
          describeBodySlot(part).c_str(), tmpb);
    }
    sendTo(buf);
    disease_start(this, &vaf);
  }
  return;
}

void TBeing::dummyLeprosy(wearSlotT part)
{
  char buf[255];

  switch (::number(1,200)) {
    case 1:
    case 2:
      sprintf(buf, "The skin on your %s cracks and puss forms near where it is peeling.\n\r",describeBodySlot(part).c_str());
      sendTo(buf);
      act("$n's skin looks diseased and falls off in puss-encrusted gobs.",
          TRUE,this,0,0,TO_ROOM);
      break;
    default:
      break;
  }
}

int disease_leprosy(TBeing *victim, int message, affectedData * af)
{
  affectedData vaf;
  wearSlotT slot = wearSlotT(af->level);

  if(slot < WEAR_NOWHERE || slot >= MAX_WEAR){
    vlogf(LOG_BUG, fmt("disease_leprosy called with bad slot: %i") % slot);
    return FALSE;
  }


  switch (message) {
    case DISEASE_PULSE:
      if (slot && !victim->isLimbFlags(slot, PART_LEPROSED)) {
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
        spread_affect(victim, 20, false, false, &vaf);
      }
      victim->bodySpread(500,af);
      break;
    case DISEASE_BEGUN:
      if (slot)
        victim->addToLimbFlags(slot, PART_LEPROSED);
      // no text, whetever calls this will have to say if its spreading or 
      // just been contracted
      break;
    case DISEASE_DONE:
      if (victim->getPosition() > POSITION_DEAD) {
        act("$n skin clears up and the festering stops.", FALSE, victim, NULL, NULL, TO_ROOM);
        victim->sendTo("Your skin condition heals and the festering ceases.\n\r");
      }
      if (slot)
        victim->remLimbFlags(slot, PART_LEPROSED);
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
    vlogf(LOG_BUG, fmt("disease_plague called with bad slot: %i") % slot);
    return FALSE;
  }


  switch (message) {
    case DISEASE_PULSE:
      if (slot != WEAR_NOWHERE &&
         !victim->isLimbFlags(slot, PART_LEPROSED)) {
        af->duration = 0;
        break;
      }
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
        vaf.type = AFFECT_DISEASE;
        vaf.level = 0;
        vaf.duration = PERMANENT_DURATION;
        vaf.modifier = DISEASE_PLAGUE;
        vaf.location = APPLY_NONE;
        vaf.bitvector = 0;
        spread_affect(victim, 40, false, false, &vaf);
      }
    // spread an infection+leprosy
      vaf.level = slot;
      vaf.modifier = DISEASE_INFECTION;
      victim->bodySpread(500,&vaf);
      vaf.modifier = DISEASE_LEPROSY;
      victim->bodySpread(500,&vaf);

      break;
    case DISEASE_BEGUN:
      if (slot != WEAR_NOWHERE)
        victim->addToLimbFlags(slot, PART_LEPROSED);
      // no text, whetever calls this will have to say if its spreading or
      // just been contracted
      break;
    case DISEASE_DONE:
      if (victim->getPosition() > POSITION_DEAD) {
        act("$n's twitchings lessen as the plague leaves $s body.", FALSE, 
               victim, NULL, NULL, TO_ROOM);
        victim->sendTo("Your twitching lessens as the plague leaves your body.\n\r");
      }
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
  {disease_leprosy,"leprosy",1220},
  {disease_plague,"THE PLAGUE!", 10220},
  {disease_suffocate,"a breathing problem",10000},
  {disease_food_poison, "food poisoning", 2450},
  {disease_drowning,"drowning",10000},
  {disease_garrotte,"a breathing problem",100000},
  {disease_poison,"poison",450},
  {disease_syphilis,"syphilis",8500},
};

