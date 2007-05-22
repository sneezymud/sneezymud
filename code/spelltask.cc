//////////////////////////////////////////////////////////////////////////
//
//      SneezyMUD - All rights reserved, SneezyMUD Coding Team
//      "spelltask.cc" - All functions related to casting
//
//////////////////////////////////////////////////////////////////////////

/*
    Cosmo 6-10-97:
  to start a cast, do:
  int start_cast(TBeing *ch, TBeing *victim, TThing *obj, TRoom *rp, spellNumT spell, int difficulty, int target, char *arg, int rounds, ushort wasInRoom, ubyte status, int flags, int text, int nextUpdate, int offensive)
  ch          = caster
  victim      = target if a mob
  obj         = target if an object (leave room and victim null)
  rp          = victim's room when casting if relevant or room for an area spell
                most offensive victim spells will have a victim and a room
                ranged spells should use either victims room or no room
  spell       = spell number
  difficulty  = spell difficulty/used for some distract and success formulas
  target      = 1 if person or area spell, 2 if cast on an object/used for 
                components
  arg         = argument stored for spells like telepath (usually "") 
  rounds      = casting rounds till effect/set to discArray[spell]->lag number 
                to start then decreases.
  wasInRoom   = What room caster started in so he doesnt get moved around
                cast will fail if caster isnt in room he started cast 
  status      = number of rounds into the spell/ initiates to 0
                used to initiate a cast (is this the first round) and keep track
  text        = if True Show casting text
  flags       = can be used to add specific elements to spells
  next_update = sets when next call to casting switch (inititiated first pass)
  offensive   = Is the spell offensive/ TRUE is offensive


  nextUpdate:
    - NOT WHAT IT SEEMS:  
      start_cast() says ch->spelltask->nextUpdate = nextUpdate.
    - calls to CMD_TASK_CONTINUE occur if pulse > ch->spelltask->nextUpdate
    - notice that because pulse is typically big, calls to TASK CONTINUE are
      virtually instantaneous.
    - it is not until the first call to CMD_TASK_CONTINUE that we typically set
      ch->spelltask->nextUpdate = pulse + xxxx
    - this leads to using the init for the first time through the function.
    - SO BASICALLY, THIS VALUE IS WORTHLESS
*/

#include "stdsneezy.h"
#include "discipline.h"
#include "spelltask.h"
#include "disc_air.h"
#include "disc_alchemy.h"
#include "disc_shaman_armadillo.h"
#include "disc_shaman_frog.h"
#include "disc_shaman_healing.h"
#include "disc_shaman_spider.h"
#include "disc_shaman_skunk.h"
#include "disc_shaman_control.h"
#include "disc_nature.h"
#include "disc_cures.h"
#include "disc_earth.h"
#include "disc_fire.h"
#include "disc_sorcery.h"
#include "disc_spirit.h"
#include "disc_water.h"
#include "disc_aegis.h"
#include "disc_wrath.h"
#include "disc_shaman.h"
#include "disc_afflictions.h"
#include "disc_cures.h"
#include "disc_hand_of_god.h"
#include "disc_shaman_control.h"
#include "disc_deikhan.h"
#include "disc_ranger.h"
#include "disc_animal.h"
#include "disc_shaman_armadillo.h"
#include "disc_leverage.h"
#include "disc_ranged.h"
#include "combat.h"
#include "disease.h"
#include "being.h"
#include "obj_symbol.h"

// FYI: CMD_TASK_CONTINUE is checked once per PULSE_COMBAT
void spellTaskData::getNextUpdate(int pulse, int interval) 
{

  nextUpdate = pulse + interval;
  if (nextUpdate > 2400)
    nextUpdate -= 2400;
}

void TBeing::stopCast(stopCastT messages)
{
  TThing *target = NULL;
  TBeing * ch = NULL;
  TBeing *tmp_ch = NULL;
  int which = 0;

  if (!spelltask)
    return;

  skillUseTypeT caster_type = getSpellType(discArray[spelltask->spell]->typ);

  if (spelltask->victim) {
    target = spelltask->victim;
    which = 1;
  } else if (spelltask->object) {
    target = spelltask->object;     
    which = 2;
  }


  if (target) {
    if (!(ch = target->getCaster())) {
      vlogf(LOG_BUG, fmt("%s doesnt have a casterList in stopCast(%s)") % target->getName() %getName());
    } else {
      if (target->getCaster() == this) {
        // first in list
        target->setCaster(next_caster);
        
        if (which == 1) 
          spelltask->victim = NULL;
        else if (which == 2) 
          spelltask->object = NULL;
      } else {
        for (tmp_ch = ch, ch = ch->next_caster;ch;tmp_ch = ch, ch = ch->next_caster) {
          if (ch == this) {
            tmp_ch->next_caster = next_caster;
            
            if (which == 1) 
              spelltask->victim = NULL;
            else if (which == 2) 
              spelltask->object = NULL;
            
            break;
          }  
        }
        if (!ch)
          vlogf(LOG_BUG,fmt("%s not found in spelltasking list on %s") %  target->getName() %getName());
      }
    }  
  }
  switch (messages) {
    case STOP_CAST_LOCATION: 
      if (caster_type == SPELL_CASTER) {
	  colorAct(COLOR_SPELLS, "<R>Your change of location distracts you and you stop casting.<z>",FALSE, this, NULL, NULL, TO_CHAR);
	  colorAct(COLOR_SPELLS, "<R>$n stops chanting and the magic dissipates.<z>", TRUE, this, NULL, NULL, TO_ROOM);
      } else if (caster_type == SPELL_DANCER) {
	  colorAct(COLOR_SPELLS, "<o>Your change of location distracts you and you cease the ritual.<z>",FALSE, this, NULL, NULL, TO_CHAR);
	  colorAct(COLOR_SPELLS, "<o>$n stops chanting and stops the ritual.<z>", TRUE, this, NULL, NULL, TO_ROOM);
      } else if (caster_type == SPELL_PRAYER) {
        colorAct(COLOR_SPELLS, "<R>Your change of location distracts you and you stop praying.<z>", FALSE, this, NULL, NULL, TO_CHAR);
        colorAct(COLOR_SPELLS, "<R>$n stops praying as $s holy symbol becomes inert.<z>", TRUE, this, NULL, NULL, TO_ROOM);
      }
      break;
    case STOP_CAST_GENERIC:
      if (caster_type == SPELL_CASTER) {
        colorAct(COLOR_SPELLS, "<R>You stop casting your spell.<z>",FALSE, this, NULL, NULL, TO_CHAR);
        colorAct(COLOR_SPELLS, "<R>$n stops chanting and the magic dissipates.<z>", TRUE, this, NULL, NULL, TO_ROOM);
      } else if (caster_type == SPELL_PRAYER) {
        colorAct(COLOR_SPELLS, "<R>You stop praying to your deities for help.<z>", FALSE, this, NULL, NULL, TO_CHAR);
        colorAct(COLOR_SPELLS, "<R>$n stops praying as $s holy symbol becomes inert.<z>", TRUE, this, NULL, NULL, TO_ROOM);
      } else if (caster_type == SPELL_DANCER) {
        colorAct(COLOR_SPELLS, "<o>You stop your ritual to your ancestors for help.<z>", FALSE, this, NULL, NULL, TO_CHAR);
        colorAct(COLOR_SPELLS, "<o>$n interrups $s ritual dance to change location.<z>", TRUE, this, NULL, NULL, TO_ROOM);
      }
      break;
    case STOP_CAST_DEATH:
      if (caster_type == SPELL_CASTER) {
        colorAct(COLOR_SPELLS, "<R>You hear the last echo of $n's spell hanging in the air left there by $s death.<z>",TRUE, this, NULL, NULL, TO_ROOM);
      } else if (caster_type == SPELL_PRAYER) {
        colorAct(COLOR_SPELLS, "<R>A last word $n's prayer hangs in the air left there by $s death.<z>",TRUE, this, NULL, NULL, TO_ROOM);
      } else if (caster_type == SPELL_DANCER) {
        colorAct(COLOR_SPELLS, "<r>Phantom spirits appear suddenly and remove $n's soul.<z>",TRUE, this, NULL, NULL, TO_ROOM);
      }
      break;
    case STOP_CAST_NOT_AROUND:
      if (caster_type == SPELL_CASTER) {
        if (which == 1) {
          if (IS_SET(discArray[spelltask->spell]->targets, TAR_VIOLENT))
            colorAct(COLOR_SPELLS, "<y>You stop casting as your victim is no longer there.<z>",FALSE, this, NULL, NULL, TO_CHAR);
          else
            colorAct(COLOR_SPELLS, "<y>You stop casting as your target is no longer there.<z>",FALSE, this, NULL, NULL, TO_CHAR);
        } else if (which == 2) {
          colorAct(COLOR_SPELLS, "<y>You stop casting as the object no longer seems to be there.<z>",FALSE, this,NULL, NULL, TO_CHAR);
        } else {
          colorAct(COLOR_SPELLS, "<y>You stop casting as your target is no longer there.<z>",FALSE, this,NULL, NULL, TO_CHAR);
        }
        colorAct(COLOR_SPELLS, "<y>$n stops chanting and the magic dissipates.<z>", TRUE, this, NULL, NULL, TO_ROOM);
      } else if (caster_type == SPELL_PRAYER) {
        if (which == 1) {
          if (IS_SET(discArray[spelltask->spell]->targets, TAR_VIOLENT))
            colorAct(COLOR_SPELLS, "<y>You stop praying as your victim is no longer there.<z>", FALSE, this, NULL, NULL, TO_CHAR);
          else
            colorAct(COLOR_SPELLS, "<y>You stop praying as your target is no longer there.<z>", FALSE, this, NULL, NULL, TO_CHAR);
        } else if (which == 2) {
          colorAct(COLOR_SPELLS, "<y>You stop praying as the object no longer seems to be there.<z>.",FALSE, this,NULL, NULL, TO_CHAR);
        } else {
          colorAct(COLOR_SPELLS, "<y>You stop praying as your target no longer seems to be there.<z>",FALSE, this,NULL, NULL, TO_CHAR);
        }
        colorAct(COLOR_SPELLS, "<y>$n stops praying as $s holy symbol becomes inert.<z>", TRUE, this, NULL, NULL, TO_ROOM);
      } else if (caster_type == SPELL_DANCER) {
        if (which == 1) {
          if (IS_SET(discArray[spelltask->spell]->targets, TAR_VIOLENT))
            colorAct(COLOR_SPELLS, "<y>You cease the ritual as your victim is no longer there.<z>", FALSE, this, NULL, NULL, TO_CHAR);
          else
            colorAct(COLOR_SPELLS, "<y>You cease the ritual as your target is no longer there.<z>", FALSE, this, NULL, NULL, TO_CHAR);
        } else if (which == 2) {
          colorAct(COLOR_SPELLS, "<y>You cease the ritual as the object no longer seems to be there.<z>.",FALSE, this,NULL, NULL, TO_CHAR);
        } else {
          colorAct(COLOR_SPELLS, "<y>You cease the ritual as your target no longer seems to be there.<z>",FALSE, this,NULL, NULL, TO_CHAR);
        }
        colorAct(COLOR_SPELLS, "<y>$n<z><y> stops $s ritual dance and the spirits ignore $m.<z>", TRUE, this, NULL, NULL, TO_ROOM);
      }
      break;
    case STOP_CAST_NONE:
      break;
  }
  delete spelltask;
  spelltask = NULL;
  next_caster = NULL;
  spellstore.storing=FALSE;
  delete spellstore.spelltask;

}

skillUseTypeT getSpellType(skillUseClassT typ) 
{
  switch (typ) {
    case SKILL_SHAMAN:
      return SKILL_DANCER;
    case SPELL_SHAMAN:
      return SPELL_DANCER;
    case SPELL_MAGE:
    case SPELL_RANGER:
      return SPELL_CASTER;
    case SKILL_MAGE:
    case SKILL_RANGER:
    case SKILL_MAGE_TYPES:
      return SKILL_CASTER;
    case SPELL_CLERIC:
    case SPELL_DEIKHAN:
      return SPELL_PRAYER;
    case SKILL_CLERIC:
    case SKILL_DEIKHAN:
    case SKILL_CLERIC_TYPES:
      return SKILL_PRAYER;
    case SKILL_GENERAL:
    case SKILL_WARRIOR:
    case SKILL_THIEF:
    case SKILL_MONK:
    case SPELL_ANTI:
    case SPELL_NOCLASS:
      return SPELL_UNDEFINED;
  }
  return SPELL_UNDEFINED;
}

static bool doComponentUse(spellNumT spell, TBeing *ch)
{
  // spell on person
  if (ch->spelltask->target == 1) {
    if (!ch->useComponent(ch->findComponent(spell), ch->spelltask->victim)) 
      return FALSE;
// spell on object
  } else if (ch->spelltask->target == 2) {
    if (!ch->useComponentObj(ch->findComponent(spell), ch->spelltask->object)) 
      return FALSE;
  } else {
    vlogf(LOG_BUG, fmt("Bad target in doComponentUse(%s)(%d).") %  ch->getName() % spell);
    return FALSE;
  }
  return TRUE;
}

int start_cast(TBeing *ch, TBeing *victim, TThing *obj, TRoom *rp, spellNumT spell, taskDiffT difficulty, int target, const char *arg, lag_t rounds, ushort wasInRoom, ubyte status, int flags, int text, int nextUpdate)
{
// COSMO MARKER, change target to the discArray target

  // this used to be passed in as a formal parm, but this seems a better method
  bool offensive = discArray[spell]->targets & TAR_VIOLENT;

  if (!ch || (ch->spelltask)) {
    if (ch && ch->isImmortal()) {
      act("Just cause you are immortal, you still can't cast two spells.",
        TRUE, ch, NULL, NULL, TO_CHAR);
      act("$n stops casting.",
          TRUE, ch, NULL, NULL, TO_ROOM);
      return FALSE;
    }
    // mob casting 2 spells.  2nd spell is "spell", look at spelltask for first...
    vlogf(LOG_BUG, fmt("%s got to bad place in start_cast (%d).  Tell a coder.") % 
       (ch ? ch->getName() : "Unknown") % spell);

    if (ch)
      ch->sendTo("Problem in spelltask.  Bug this please or tell a god.\n\r");

    return FALSE;
  }
  if (!(ch->spelltask = new spellTaskData())) {
    vlogf(LOG_BUG, fmt("Couldn't allocate memory in start_cast for %s") %  ch->getName());
    return FALSE;
  }
  ch->spelltask->orig_arg = mud_str_dup(arg);
  ch->spelltask->victim = victim;
  ch->spelltask->object = dynamic_cast<TObj *>(obj);
  ch->spelltask->room = rp;
  ch->spelltask->spell = spell;
  ch->spelltask->difficulty = difficulty;

  if (ch->isImmortal()) {
#if 0
    ch->spelltask->rounds = min(LAG_1, rounds);
#else
    // enum into a min() might be bad
    ch->spelltask->rounds = rounds < LAG_1 ? rounds : LAG_1;
#endif
  } else 
    ch->spelltask->rounds = rounds;

  ch->spelltask->target = target;
  ch->spelltask->wasInRoom = wasInRoom;
  ch->spelltask->text = text;
// SEMI-Kludge to approximate the one round lag between a pc
// typing a cast and the cast actually hitting
// If rounds == 0, the spell will go right off
// May have to do followup work on pc's to get this to work right
  if (!ch->desc && !(discArray[spell]->lag == 0))
    ch->spelltask->status = -99;
  else 
    ch->spelltask->status = status;

  if (flags) 
    ch->spelltask->flags = flags;
  else {
    if (victim && rp) 
      ch->spelltask->flags += CASTFLAG_SEE_VICT;
    else 
      ch->spelltask->flags = flags;
  }
  ch->spelltask->nextUpdate = nextUpdate;

  if (victim) {
    ch->next_caster = victim->getCaster();
    victim->setCaster(ch);
  }
  if (obj) {
    ch->next_caster = obj->getCaster();
    obj->setCaster(ch);
  }
  if (victim && victim == ch) {
    act("You concentrate intensely on yourself.", 
        FALSE, ch, NULL, NULL, TO_CHAR);
    act("$n concentrates hard.", 
        TRUE, ch, NULL, NULL, TO_ROOM);
  } else if (victim && victim->isImmortal() && (!ch->isImmortal() || offensive)) {
    act("You can not cast that on an immortal.",
        FALSE, ch, NULL, NULL, TO_CHAR);
    act("$n concentrates hard but to no avail.",
        TRUE, ch, NULL, NULL, TO_ROOM);

    return FALSE;
  } else if (victim) {
    act("You concentrate intensely on $N.", 
        FALSE, ch, NULL, victim, TO_CHAR);
    if (ch->sameRoom(*victim)) {
      act("$n gazes intensely at $N.", 
          TRUE, ch, NULL, victim, TO_NOTVICT);
      act("$n gazes intensely at you.",
          TRUE, ch, NULL, victim, TO_VICT);
    } else { 
      // casting on victim in another room
      act("$n concentrates intensely on something.",
          TRUE, ch, NULL, NULL, TO_ROOM);
      if ((::number(0,400)) < (victim->plotStat(STAT_CURRENT, STAT_PER, 25, 60, 40, 1.0))) { 
        act("You sense $N's presence near you.",
            TRUE, victim, NULL, ch, TO_CHAR);
        act("You sense $N's presence near you.",
            TRUE, victim, NULL, ch, TO_ROOM);
      }
    }
    if (offensive && (victim->getPosition() >= POSITION_RESTING)) {
      act("$N senses your offensive magic is directed at $M.",
          FALSE, ch, NULL, victim, TO_CHAR);
      act("You sense offensive magic directed at you.",
          FALSE, ch, NULL, victim, TO_VICT);
      act("$N senses offensive magic directed at $M.",
           TRUE, ch, NULL, victim, TO_NOTVICT);
    }
  } else if (obj) {
    act("You concentrate intensely on $p.",      
        FALSE, ch, obj, NULL, TO_CHAR);
    act("$n gazes intensely at $p.",
        TRUE, ch, obj, NULL, TO_ROOM);
  } else {
    act("You concentrate intensely on your task.",
        FALSE, ch, NULL, NULL, TO_CHAR);
    act("$n concentrates hard on something.",
        FALSE, ch, NULL, NULL, TO_ROOM);
  }
  if (IS_SET(discArray[spell]->comp_types, COMP_MATERIAL) &&
      IS_SET(discArray[spell]->comp_types, COMP_MATERIAL_INIT)) {
    if (!ch->spelltask->component) { 
      if (!doComponentUse(spell, ch)) {
        // couldn't use comp, so lets delete spelltask
        ch->stopCast(STOP_CAST_NONE);
      }
    }
  }
  return TRUE;
}


void cast_warn_busy(const TBeing *ch, spellNumT which)
{
  skillUseTypeT styp;

  if (!ch || !(ch->spelltask)) {
    vlogf(LOG_BUG, fmt("%s got to bad place in cast_warn_busy.  Tell a coder") % 
       (ch ? ch->getName() : "Unknown"));
    return;
  }
   styp = getSpellType(discArray[which]->typ);
 
  if (styp == SPELL_PRAYER) {
    ch->sendTo("You are busy praying.\n\r");
    ch->sendTo("You may type 'abort' or 'stop' to quit praying.\n\r");
  } else if (styp == SPELL_CASTER) {
    ch->sendTo("You are busy casting a spell.\n\r");
    ch->sendTo("You may type 'abort' or 'stop' to quit casting.\n\r");
  } else if (styp == SPELL_DANCER) {
    ch->sendTo("You are busy invoking a ritual.\n\r");
    ch->sendTo("You may type 'abort' or 'stop' to cease the ritual.\n\r");
  }
}

int TBeing::cast_spell(TBeing *ch, cmdTypeT cmd, int pulse)
{
  int counter = 6;
  int distract = 0, roll = 0, rounds;
  skillUseTypeT typ;
  TRoom *room = NULL;
  int rc, ret;
  bool limbs, silence;
  int status;

  // since in the process of casting, we might want to both kill the target
  // and stop the cast (in arbitrary order), we should save a local copy
  // of the target, so we can refer to it always, even after cast has stopped
  TBeing *vict = ch->spelltask->victim;
  TObj *obj = ch->spelltask->object;

  pulse = rc = ret = 0;
  limbs = silence = false;
  if (!ch->spelltask) {
    vlogf(LOG_BUG,fmt("Somehow %s got to cast_spell with no spelltask structure,") %  ch->getName());
    act("Something went wrong here in spellcasting. Could you please place a bug or tell a coder the details" , FALSE, ch, NULL, NULL,TO_CHAR);
   return FALSE;
  } 
  if (ch->isLinkdead() || (ch->getPosition() < POSITION_RESTING)) {
    ch->stopCast(STOP_CAST_NONE);
    return FALSE;
  }

  if (ch->castAllowCommand(cmd))
    return FALSE;

  spellNumT spell = ch->spelltask->spell;

// COSMO KLUDGE to make sure that mobs get a slight delay between the
// start of cast and the continuation.  Otherwise the first cast
// and first round are the same.

  if (!ch->desc && (ch->spelltask->status == -99)) {
    ch->spelltask->status = 0;
    return FALSE;
  }
// COSMO MARKER
//  NEED a small success check

  if (checkBadSpellCondition(ch, spell)) {
    ch->stopCast(STOP_CAST_GENERIC);
    return FALSE;
  }

// Make sure rounds don't get too small 0 is last round anyhow
  if (ch->spelltask->rounds < 0) 
    ch->spelltask->rounds = 0;

// determine if a prayer or a spell
  rounds = ch->spelltask->rounds;
  typ = getSpellType(discArray[spell]->typ);
  status = ch->spelltask->status;
  spell = ch->spelltask->spell;

// check for target
  if (ch->spelltask->room)
    room = ch->spelltask->room;
 
// check to make sure that victim is in the same room as the spell
  if (vict && room && !(discArray[spell]->targets & TAR_CHAR_WORLD)) {
    if (!(room == vict->roomp) || ((ch != vict) && !(ch->canSee(vict) || ch->canSee(vict, INFRA_YES)))) {
      stopCast(STOP_CAST_NOT_AROUND);
      return FALSE;
    }
  }

// check to make sure that caster is in the same room as started spell 
  if (ch && ch->spelltask->wasInRoom && (ch->in_room != ch->spelltask->wasInRoom)) {
    stopCast(STOP_CAST_LOCATION);
    return FALSE;
  }

// need to add a check here to make sure that the an object
// spell target is still in inventory and visible
// COSMO MARKER need a discArray flag for if visible is important, or put 
// in spelltask->flags

   if (obj && discArray[spell] && *discArray[spell]->name) {
     if (discArray[spell]->targets & TAR_OBJ_INV) {
       if (!ch->canSee(obj) || !(obj->thingHolding() == ch)) {
         stopCast(STOP_CAST_NOT_AROUND);
         return FALSE;
       }
     }
     if (discArray[spell]->targets & TAR_OBJ_ROOM) {
       if (!ch->canSee(obj) || (!(obj->roomp == ch->roomp) && 
                               !(obj->thingHolding() == ch))) {
         stopCast(STOP_CAST_NOT_AROUND);
         return FALSE;
       }
     }
     if (discArray[spell]->targets & TAR_OBJ_WORLD) {
       if (!ch->canSee(obj)) {
         stopCast(STOP_CAST_NOT_AROUND);
         return FALSE;
       }
     }
   }

// COSMO MARKER
// MAKE MORE COMPLEX

  if (IS_SET(ch->spelltask->flags, CASTFLAG_SEE_VICT)) {
    if (vict && (ch != vict) && room && !ch->canSee(vict) && !ch->canSee(vict, INFRA_YES)) {
      stopCast(STOP_CAST_NOT_AROUND);
      return FALSE;
    }
  }
// check for caster using his wizardry to cast or doing the full form
// if has limbs, limbs = TRUE, if can talk, silence = TRUE

  limbs = TRUE;
  silence = TRUE;

  switch (cmd) {
    case CMD_TASK_CONTINUE:
      ch->spelltask->getNextUpdate(pulse, PULSE_COMBAT);
      roll = ::number(0, min(435, (35 + ((status +1) * 50))));

//      roll = min(300,(::number(1,((status + 1) *50))));
// roll is used each round to determine continued success or fail
// the more rounds you cast, the less likely of having a random fail
// used in conjunction with difficulty which is stored in spelltask->
// and is a function of spell difficulty, engaged status, position

// first go through an extended if else for some failure or delaying
// conditions using distraction and spell difficulty
//          general roll for failure
//          check for change in position -- use status in bash or combat
//          where if a successful spell distraction status goes to > 0 
//          thus in each spell continue, it checks for distraction set
//          to anything.  The code does a roll for 1. total fail, 2. time added
//          or nothing then clears distraction bit.
//          distractions will be bash, or skills or big w lloping hits
      if ((distract = ch->spelltask->distracted)) {
        if (typ == SPELL_CASTER) 
          colorAct(COLOR_SPELLS, "You try to center yourself and continue your cast.", FALSE, ch, NULL, NULL, TO_CHAR);
        else if (typ == SPELL_PRAYER) 
          colorAct(COLOR_SPELLS, "You try to center yourself and continue your prayer.", FALSE, ch, NULL, NULL, TO_CHAR);
        else if (typ == SPELL_DANCER) 
          colorAct(COLOR_SPELLS, "You try to ignore the surroundings and continue the ritual.", FALSE, ch, NULL, NULL, TO_CHAR);
      }
#if SPELLTASK_DEBUG
      vlogf(LOG_BUG, fmt("%s has a distract of %d in round %d on spell %s") % ch->getName() % distract % rounds % discArray[spell]->name);
#endif
      
      if (distract && (((2 * distract) >= ::number(1,20)) || (distract == -1))) {
        if (typ == SPELL_CASTER) {
          colorAct(COLOR_SPELLS, "<y>The distraction was too much for you and you lose your spell.<z>",
              FALSE, ch, NULL, NULL, TO_CHAR);
          colorAct(COLOR_SPELLS, "<y>$n looks totally dazed and stops casting altogether.<z>",
              TRUE, ch, NULL, NULL, TO_ROOM);
        } else if (typ == SPELL_PRAYER) {
          colorAct(COLOR_SPELLS, "<y>The distraction was too much for you and you stop praying.<z>",
              FALSE, ch, NULL, NULL, TO_CHAR);
          colorAct(COLOR_SPELLS, "<y>$n looks totally dazed and stops praying altogether.<z>",
              TRUE, ch, NULL, NULL, TO_ROOM);
        } else if (typ == SPELL_DANCER) {
          colorAct(COLOR_SPELLS, "<y>The distraction was too great and your ancestors ignore your needs.<z>",
              FALSE, ch, NULL, NULL, TO_CHAR);
          colorAct(COLOR_SPELLS, "<y>$n looks pale and stops $s ritual dance.<z>",
              TRUE, ch, NULL, NULL, TO_ROOM);
        }
        ch->spelltask->distracted = 0;
//        vlogf(LOG_BUG,fmt("1. before distracted(%d) distract= %d rounds = %d") %  ch->spelltask->distracted % distract % ch->spelltask->rounds);
        ch->stopCast(STOP_CAST_NONE);
        return FALSE;
      } else if (distract && (distract >= ::number(0,(distract + 1)))) {
          if (typ == SPELL_CASTER) {
            act("Although distracted, you recover your focus and continue to cast.",
                FALSE, ch, NULL, NULL, TO_CHAR, ANSI_GREEN);
            act("$n shakes $s distraction off and continues to cast.",
                TRUE, ch, NULL, NULL, TO_ROOM, ANSI_GREEN);
          } else if (typ == SPELL_PRAYER) {
            act("Although distracted, you recover your focus and continue to pray.",
                FALSE, ch, NULL, NULL, TO_CHAR, ANSI_GREEN);
            act("$n shakes $s distraction off and continues to pray.",
                TRUE, ch, NULL, NULL, TO_ROOM, ANSI_GREEN);
          } else if (typ == SPELL_DANCER) {
            act("Although distracted, you continue to call the ancestors.",
                FALSE, ch, NULL, NULL, TO_CHAR, ANSI_PURPLE_BOLD);
            act("$n shakes $s distraction and continues to call upon $s ancestors.",
                TRUE, ch, NULL, NULL, TO_ROOM, ANSI_PURPLE_BOLD);
          }
          ch->spelltask->distracted = 0;
          ch->spelltask->rounds = ch->spelltask->rounds + max(1, distract);
          rounds += max(1, distract);
      } else if (distract) {
          if (typ == SPELL_CASTER) {
            colorAct(COLOR_SPELLS, "<g>You are able to continue casting without feeling distracted.<z>",
                FALSE, ch, NULL, NULL, TO_CHAR);
          colorAct(COLOR_SPELLS, "<g>$n shakes $s distraction off and continues to cast.<z>",
              TRUE, ch, NULL, NULL, TO_ROOM);
          } else if (typ == SPELL_PRAYER) {
            colorAct(COLOR_SPELLS, "<g>You are able to continue casting without feeling distracted.<z>",
                FALSE, ch, NULL, NULL, TO_CHAR);
          colorAct(COLOR_SPELLS, "<g>$n shakes $s distraction off and continues to pray.<z>",
              TRUE, ch, NULL, NULL, TO_ROOM);
          } else if (typ == SPELL_DANCER) {
            colorAct(COLOR_SPELLS, "<P>You are able to continue your ritual without feeling distracted.<z>",
                FALSE, ch, NULL, NULL, TO_CHAR);
          colorAct(COLOR_SPELLS, "<P>$n shakes $s distraction off and continues to dance $s ritual.<z>",
              TRUE, ch, NULL, NULL, TO_ROOM);
          }
          ch->spelltask->distracted = 0;
      } else if (roll < (3 * discArray[spell]->task)) {
        int foc = ch->plotStat(STAT_CURRENT, STAT_FOC, 20, 40, 30);
        if (::number(0, foc) <= 20) {
          if (!ch->checkEngagementStatus()) {
            if (typ == SPELL_CASTER) {
              colorAct(COLOR_SPELLS, "<y>You lose your concentration and stop casting.<z>",
                  FALSE, ch, NULL, NULL, TO_CHAR);
              colorAct(COLOR_SPELLS, "$n loses $s concentration and stops casting.",
                FALSE, ch, NULL, NULL, TO_ROOM, ANSI_YELLOW);

              ch->stopCast(STOP_CAST_NONE);
              return FALSE;
            } else if (typ == SPELL_PRAYER) {
              colorAct(COLOR_SPELLS, "<y>You lose your concentration and stop praying.<z>",
                  FALSE, ch, NULL, NULL, TO_CHAR);
              colorAct(COLOR_SPELLS, "$n loses $s concentration and stops praying.",
                FALSE, ch, NULL, NULL, TO_ROOM, ANSI_YELLOW);

              ch->stopCast(STOP_CAST_NONE);
              return FALSE;
            } else if (typ == SPELL_DANCER) {
              colorAct(COLOR_SPELLS, "<R>You lose your concentration and stop your ritual dance.<z>",
                  FALSE, ch, NULL, NULL, TO_CHAR);
              colorAct(COLOR_SPELLS, "$n loses $s concentration and stops $s ritual dance.",
                FALSE, ch, NULL, NULL, TO_ROOM, ANSI_RED_BOLD);

              ch->stopCast(STOP_CAST_NONE);
              return FALSE;
            }
          }
        }
      } else if ((3* discArray[spell]->task) > (::number(0,100))) {
        if (typ == SPELL_CASTER) {
          colorAct(COLOR_SPELLS, "<c>You almost lose your focus but slowly you manage to continue your spell.<z>",
               FALSE, ch, NULL, NULL, TO_CHAR);
#if SPELLTASK_DEBUG
          vlogf(LOG_BUG,fmt("Distracted(%d) distract= %d add 1 to rounds = %d") %  ch->spelltask->distracted % distract % ch->spelltask->rounds);
#endif
          ch->spelltask->rounds++;
          rounds++;
        } else if (typ == SPELL_PRAYER) {
          colorAct(COLOR_SPELLS, "<c>You almost lose your focus but slowly you manage to continue your prayer.<z>",
              FALSE, ch, NULL, NULL, TO_CHAR);
#if SPELLTASK_DEBUG
          vlogf(LOG_BUG,fmt("Distracted(%d) distract= %d add 1 to rounds = %d") %  ch->spelltask->distracted % distract % ch->spelltask->rounds);
#endif
          ch->spelltask->rounds++;
          rounds++;
        } else if (typ == SPELL_DANCER) {
          colorAct(COLOR_SPELLS, "<B>You almost lose your focus but slowly you manage to continue the ritual.<z>",
              FALSE, ch, NULL, NULL, TO_CHAR);
#if SPELLTASK_DEBUG
          vlogf(LOG_BUG,fmt("Distracted(%d) distract= %d add 1 to rounds = %d") %  ch->spelltask->distracted % distract % ch->spelltask->rounds);
#endif
          ch->spelltask->rounds++;
          rounds++;
        }
        ch->spelltask->distracted = 0;
      } else if (rounds && ((ch->plotStat(STAT_CURRENT, STAT_WIS, 1, 10, 6, 1.0)) > (::number(1, 100)))) {
        if (typ == SPELL_CASTER) {
          colorAct(COLOR_SPELLS, "<c>Your concentration is good and your spell forms faster than usual.<z>", FALSE, ch, NULL, NULL, TO_CHAR);
#if SPELLTASK_DEBUG
          vlogf(LOG_BUG,fmt("%s subtract 1 from rounds = %d") %  ch->getName() % ch->spelltask->rounds);
#endif
          ch->spelltask->rounds--;
          rounds--;
        } else if (typ == SPELL_PRAYER) {
          colorAct(COLOR_SPELLS, "<c>Your concentration is good and you feel your prayer will be answered faster.<z>", FALSE, ch, NULL, NULL, TO_CHAR);
#if SPELLTASK_DEBUG
          ch->spelltask->rounds--;
#endif
          rounds--;
        } else if (typ == SPELL_DANCER) {
          colorAct(COLOR_SPELLS, "<B>Your concentration is good and you feel your ancestors pleasure.<z>", FALSE, ch, NULL, NULL, TO_CHAR);
#if SPELLTASK_DEBUG
          ch->spelltask->rounds--;
#endif
          rounds--;
        }
        ch->spelltask->distracted = 0;
      }

      // set counter
      if (IS_SET(ch->spelltask->flags, CASTFLAG_CAST_INDEFINITE)) {
        counter = 1;
      } else if (ch->spelltask->rounds == 1) {
        counter = 2;
      // case for second to last casting round
      } else if (ch->spelltask->rounds == 0) {
        counter = 0;
      // case for last round and spell success
      } else {
        counter = 1;
      // if not last or next to last round counter switch will use default case
      }
      if (isPc()) {
        if (discArray[spell]->minMana) {
          if (!reconcileMana(spell, FALSE)) {
            act("You have totally run out of mana and are forced to abort your spell.", FALSE, ch, NULL, NULL, TO_CHAR);
            ch->stopCast(STOP_CAST_NONE);
            return FALSE;
          }
        } else if (discArray[spell]->minLifeforce) {
          if (!reconcileLifeforce(spell, FALSE)) {
            act("You have totally run out of lifeforce!.", FALSE, ch, NULL, NULL, TO_CHAR);
            ch->stopCast(STOP_CAST_NONE);
            return FALSE;
          }
        } else {
          if (!reconcilePiety(spell, FALSE)) {
            act("You have totally run out of piety and are forced to abort your prayer.", FALSE, ch, NULL, NULL, TO_CHAR);
            ch->stopCast(STOP_CAST_NONE);
            return FALSE;
          }
        }
      }

#if 0
// XXXXXXXX
//maybe take out
      // use Mana/piety
          if (typ == SPELL_CASTER) {
            temp = useMana(spell);
          } else if (typ == SPELL_PRAYER) {
            usePiety(spell);
          } else if (typ == SPELL_DANCER) {
            temp = useLifeforce(spell);
          }
#endif
      switch (counter) {
        case 2:    // almost last round 
          if (typ == SPELL_CASTER) {
            act("You begin to feel your spell taking form.",
            FALSE, ch, NULL, NULL, TO_CHAR);
          } else if (typ == SPELL_PRAYER) {
            act("You begin to feel your prayer being answered.",
            FALSE, ch, NULL, NULL, TO_CHAR);
          } else if (typ == SPELL_DANCER) {
            act("<Y>You feel your rada song is pleasing to the loa.<1>",
            FALSE, ch, NULL, NULL, TO_CHAR);
          }
          ch->sendCastingMessages(limbs, silence, rounds, typ, counter); 
          if (typ == SPELL_CASTER) {
            if (ch->isPc() && !(ch->applyCompCheck(spell, rounds, status))) {
              ch->stopCast(STOP_CAST_NONE);
              return FALSE;
            }
          } else if (typ == SPELL_DANCER) {
            if (ch->isPc() && !(ch->applyCompCheck(spell, rounds, status))) {
              ch->stopCast(STOP_CAST_NONE);
              return FALSE;
            }
          } else if (typ == SPELL_PRAYER) {
            if (ch->isPc() && !ch->checkHolySymbol(spell)) {
              ch->stopCast(STOP_CAST_NONE);
              return FALSE;
            }
            if (spelltask && IS_SET(discArray[spell]->comp_types, SPELL_TASKED_EVERY)) {
              ret = ch->doSpellCast(ch, vict, obj, room, spell, typ);
            }
          }
#if 0
// COSMO MARKER
// NEED a FUNCTION
          useCastMana(typ); 
#endif
          if (ch->spelltask) {
            ch->spelltask->rounds--;
            ch->spelltask->status++;
          }
          break;
        case 1:
          ch->sendCastingMessages(limbs, silence, rounds, typ, counter);
          if (typ == SPELL_CASTER) {
            if (ch->isPc() && !ch->applyCompCheck(spell, rounds, status)) {
              ch->stopCast(STOP_CAST_NONE);
              return FALSE;
            }
          } else if (typ == SPELL_DANCER) {
            if (ch->isPc() && !ch->applyCompCheck(spell, rounds, status)) {
              ch->stopCast(STOP_CAST_NONE);
              return FALSE;
            }
          } else if (typ == SPELL_PRAYER) {
            if (ch->isPc() && !ch->checkHolySymbol(spell)) {
              ch->stopCast(STOP_CAST_NONE);
              return FALSE;
            }
            if (spelltask && IS_SET(discArray[spell]->comp_types, SPELL_TASKED_EVERY)) {
              ret = ch->doSpellCast(ch, vict, obj, room, spell, typ);
            }
          }
#if 0
// COSMO MARKER
// NEED a FUNCTION
          useCastMana(typ);
#endif
          if (ch->spelltask) {
            ch->spelltask->rounds--;
            ch->spelltask->status++;
          }
          break;
        case 0:
          ch->sendFinalCastingMessages(limbs, silence, typ);
          if (typ == SPELL_CASTER) {
            if (ch->isPc() && !(ch->applyCompCheck(spell, rounds, status))) {
              ch->stopCast(STOP_CAST_NONE);
              return FALSE;
            }
          } else if (typ == SPELL_DANCER) {
            if (ch->isPc() && !(ch->applyCompCheck(spell, rounds, status))) {
              ch->stopCast(STOP_CAST_NONE);
              return FALSE;
            }
          } else if (typ == SPELL_PRAYER) {
            if (ch->isPc() && !ch->checkHolySymbol(spell)) {
              ch->stopCast(STOP_CAST_NONE);
              return FALSE;
            }
          }
#if 0
// COSMO MARKER
// NEED a FUNCTION
          useCastMana(typ);
#endif

          if ((ch->GetMaxLevel() < GOD_LEVEL1) && ch->desc && (ch->spelltask && !ch->spelltask->component) && IS_SET(discArray[spell]->comp_types, COMP_MATERIAL)) {
            sendTo(COLOR_SPELLS, "<r>You seem to have lost your component.<z>\n\r");
            vlogf(LOG_BUG, fmt("%s got to end of casting without using component, spell:%s (%d)") %  ch->getName() % discArray[spell]->name % spell);
           ch->stopCast(STOP_CAST_NONE);
            return FALSE;
          }

          ret = ch->doSpellCast(ch, vict, obj, room, spell, typ);
          if (vict && (IS_SET_DELETE(ret, DELETE_VICT))) {
            delete vict;
            vict = NULL;
          }
          if (obj && IS_SET_DELETE(ret, DELETE_ITEM)) {
            delete obj;
            obj = NULL;
          }
          if (IS_SET_DELETE(ret, DELETE_THIS))
            ADD_DELETE(rc, DELETE_THIS);
          break;
        default:
          vlogf(LOG_BUG,fmt("Somehow %s got sent to bad counter case in cast_spell") % ch->getName());
          break;
      }
      break;
    case CMD_STAND:
// COSMO MARKER
// PUT A SPEED CHECK IN HERE
      if (ch->getPosition() >= POSITION_STANDING) {
        act("You are already standing.",
            FALSE, ch, NULL, NULL, TO_CHAR);
        break;
      }
      if (typ == SPELL_CASTER) {
        act("You interrupt your spell to stand.",
            FALSE, ch, NULL, NULL, TO_CHAR);
      } else if (typ == SPELL_DANCER) {
        act("<B>You interrupt your ritual to stand.<1>",
            FALSE, ch, NULL, NULL, TO_CHAR);
      } else if (typ == SPELL_PRAYER) {
        act("You interupt your prayer to stand.",
            FALSE, ch, NULL, NULL, TO_CHAR);
      }
      if (ch->spelltask) {
        ch->spelltask->rounds++;
      }
      return FALSE;
      break;
    case CMD_SAY:
    case CMD_TELL:
    case CMD_NOD:
    case CMD_SHOUT:
// COSMO MARKER
// PUT A SPEED CHECK
      if (ch->spelltask) {
        if (typ == SPELL_CASTER) {
          act("You interrupt your spell to communicate.",
              FALSE, ch, NULL, NULL, TO_CHAR);
        } else if (typ == SPELL_DANCER) {
          act("<B>You interrupt your ritual dance to communicate.<1>",
              FALSE, ch, NULL, NULL, TO_CHAR);
        } else if (typ == SPELL_PRAYER) {
          act("You interupt your prayer to communicate.",
              FALSE, ch, NULL, NULL, TO_CHAR);
        }
        ch->spelltask->rounds++;
      }
      return FALSE;
      break;
    case CMD_FLEE:
      if (typ == SPELL_CASTER) {
        colorAct(COLOR_SPELLS, "<R>You stop casting.<z>", FALSE, ch, 0, 0, TO_CHAR);
        colorAct(COLOR_SPELLS, "<R>$n stops casting.<z>", FALSE, ch, 0, 0, TO_ROOM);
      } else if (typ == SPELL_DANCER) {
        colorAct(COLOR_SPELLS, "<R>You stop your ritual.<z>", FALSE, ch, 0, 0, TO_CHAR);
        colorAct(COLOR_SPELLS, "<R>$n stops $s ritual.<z>", FALSE, ch, 0, 0, TO_ROOM);
      } else if (typ == SPELL_PRAYER) {
        colorAct(COLOR_SPELLS, "<R>You stop praying.<z>", FALSE, ch, 0, 0, TO_CHAR);
        colorAct(COLOR_SPELLS, "<R>$n stops praying.<z>", FALSE, ch, 0, 0, TO_ROOM);
      }
      ch->stopCast(STOP_CAST_NONE);
      return FALSE;
    case CMD_ABORT:
    case CMD_STOP:
      if (typ == SPELL_CASTER) {
        colorAct(COLOR_SPELLS, "<R>You stop casting.<z>", FALSE, ch, 0, 0, TO_CHAR);
        colorAct(COLOR_SPELLS, "<R>$n stops casting.<z>", FALSE, ch, 0, 0, TO_ROOM);
      } else if (typ == SPELL_DANCER) {
        colorAct(COLOR_SPELLS, "<R>You stop your ritual.<z>", FALSE, ch, 0, 0, TO_CHAR);
        colorAct(COLOR_SPELLS, "<R>$n stops dancing.<z>", FALSE, ch, 0, 0, TO_ROOM);
      } else if (typ == SPELL_PRAYER) {
        colorAct(COLOR_SPELLS, "<R>You stop praying.<z>", FALSE, ch, 0, 0, TO_CHAR);
        colorAct(COLOR_SPELLS, "<R>$n stops praying.<z>", FALSE, ch, 0, 0, TO_ROOM);
      }
      ch->stopCast(STOP_CAST_NONE);
      break;
    default:
      if (ch->isImmortal())
        return FALSE;
      if (cmd < MAX_CMD_LIST)
        cast_warn_busy(ch, spell);
      break;                    // eat the command 
  }
  if (ch->spelltask) {
    if (ch->spelltask->rounds < 0)
      ch->spelltask->rounds = 0;
  }
  rc = rc + TRUE;
  return rc;
}

// COSMO MARKER
// Function for checking specific donditions and aborting spells. 
// Mirrors the conditions in the spells themselves for initializing cast. 
int TBeing::checkBadSpellCondition(TBeing *caster, int which)
{
  TBeing *victim = spelltask->victim;
  //TObj *obj = spelltask->object;
  char buf[256];
  int again;

  switch (which) {
    // disc_cures
    case SPELL_HEAL_LIGHT:
    case SPELL_HEAL_LIGHT_DEIKHAN:
    case SPELL_HEAL_SERIOUS:
    case SPELL_HEAL_SERIOUS_DEIKHAN:
    case SPELL_HEAL_CRITICAL:
    case SPELL_HEAL_CRITICAL_DEIKHAN:
    case SPELL_HEAL:
    case SPELL_HEAL_FULL:
      return FALSE;
// disc_air
    case SPELL_GUST:
      return FALSE;
    case SPELL_LEVITATE:
      if (victim->getPosition() != POSITION_STANDING) {
       caster->sendTo("You can't induce levitation on someone that is not standing.\n\r");
       return TRUE;
      }
      return FALSE;
      if (victim->affectedBySpell(SPELL_FLY)) {
        act("$N is already affected by a spell of flight.", FALSE, this, NULL, victim, TO_CHAR);
        return TRUE;
      }
      return FALSE;
    case SPELL_IMMOBILIZE:
      if (victim->affectedBySpell(SPELL_IMMOBILIZE)) {
        act("$N is already immobilized!", FALSE, caster, NULL, victim, TO_CHAR);
        return TRUE;
      }
      return FALSE;
    case SPELL_SUFFOCATE:
      if (victim->affectedBySpell(SPELL_SUFFOCATE)) {
        act("$N is already choking!", FALSE, caster, NULL, victim, TO_CHAR);
        return TRUE;
      }
      return FALSE;
    case SPELL_DUST_STORM:
      if (caster->roomp->isUnderwaterSector()) {
         caster->sendTo("Not much air down here under all this water.\n\r");
         return TRUE;
      }
      return FALSE;
    case SPELL_FLY:
      if (victim->affectedBySpell(SPELL_LEVITATE)) {
         act("$n is already affected by a spell of levitation .", FALSE, caster, NULL, NULL, TO_ROOM);
         return TRUE;
      }
      return FALSE;
    case SPELL_ANTIGRAVITY:

      // A) this is causing a crash
      // B) i think we check for this on a per person in group basis at the end of a success.
      // dash 05/26/01
      // blah
#if 0
      if ((caster == victim) || (caster->inGroup(*victim))) {
        if (victim->isAffected(AFF_FLYING) || 
            victim->isAffected(AFF_LEVITATING)) {
           victim->sendTo("Nothing seems to happen.\n\r");
           caster->sendTo("$n is already effected by an anti gravity spell!\n\r"
);
        return TRUE;
        }
      }
#endif
      return FALSE;
    case SPELL_FALCON_WINGS:
      if (victim->affectedBySpell(SPELL_LEVITATE) || victim->affectedBySpell(SPELL_FLY)) {
       act("$n is already affected by a some type of flight spell.", FALSE, caster, NULL, NULL, TO_ROOM);
       return TRUE;
     }
     return FALSE;
    case SPELL_PROTECTION_FROM_AIR:
    case SPELL_CONJURE_AIR:
    case SPELL_ENTHRALL_SPECTRE: // shaman 
    case SPELL_ENTHRALL_GHOUL: // shaman 
    case SPELL_ENTHRALL_GHAST: // shaman 
    case SPELL_ENTHRALL_DEMON: // shaman 
    case SPELL_HEALING_GRASP: // shaman
    case SPELL_CREATE_WOOD_GOLEM:
    case SPELL_CREATE_ROCK_GOLEM:
    case SPELL_CREATE_IRON_GOLEM:
    case SPELL_CREATE_DIAMOND_GOLEM:
    case SPELL_FEATHERY_DESCENT:
    case SPELL_TORNADO:
      return FALSE;

// disc_earth
    case SPELL_METEOR_SWARM:
      if (!caster->outside()) {
        caster->sendTo("You can only cast this outside!\n\r");
        act("Nothing seems to happen.", TRUE, caster, 0, 0, TO_ROOM);
	   return TRUE;
      }
      return FALSE;
    case SPELL_STONE_SKIN:
      if (victim->affectedBySpell(SPELL_FLAMING_FLESH)) {
        act("$N's skin is already defended by elementals of fire.", FALSE, caster, NULL, victim, TO_CHAR);
        act("Nothing seems to happen.", FALSE, caster, NULL, NULL, TO_ROOM);
        return TRUE;
      }
      return FALSE;
    case SPELL_SLING_SHOT:
    case SPELL_GRANITE_FISTS:
    case SPELL_PEBBLE_SPRAY:
    case SPELL_SAND_BLAST:
    case SPELL_FLATULENCE:
    case SPELL_LAVA_STREAM:
    case SPELL_SQUISH: // shaman
    case SPELL_DEATH_MIST: // shaman
    case SPELL_TRAIL_SEEK:
    case SPELL_CONJURE_EARTH:
    case SPELL_PROTECTION_FROM_EARTH:
      return FALSE;

// disc_water

   case SPELL_ICY_GRIP:
     if (victim->affectedBySpell(SPELL_ICY_GRIP)) {
        act("Nothing seems to happen.", FALSE, caster, NULL, victim, TO_NOTVICT);
        return TRUE;
     }
     return FALSE;
   case SPELL_WATERY_GRAVE:
     if (victim->hasDisease(DISEASE_DROWNING)) {
        act("Nothing seems to happen.", TRUE,caster,0,0,TO_ROOM);
        act("$N is already drowning.", TRUE,caster,0,victim,TO_CHAR);
          return TRUE;
     }
     return FALSE;
   case SPELL_BREATH_OF_SARAHAGE:
     if (victim && victim->isAffected(AFF_WATERBREATH)) {
       act("$N already has the ability to breathe underwater.",
           FALSE, caster, NULL, victim, TO_CHAR);
       act("Nothing seems to happen.",
           FALSE, caster, NULL, NULL, TO_ROOM);
       return TRUE;
     }
     return FALSE;
   case SPELL_GILLS_OF_FLESH:
   case SPELL_PLASMA_MIRROR:
   case SPELL_THORNFLESH:
   case SPELL_FAERIE_FOG:
    case SPELL_DJALLA: // shaman
    case SPELL_LEGBA: // shaman
   case SPELL_PROTECTION_FROM_WATER:
   case SPELL_ARCTIC_BLAST:
   case SPELL_ICE_STORM:
   case SPELL_TSUNAMI:
   case SPELL_CONJURE_WATER:
   case SPELL_GUSHER:
      return FALSE;

// disc_spirit
   case SPELL_POLYMORPH:
     return FALSE;
   case SPELL_CLOUD_OF_CONCEALMENT:
     return FALSE;
   case SPELL_SHAPESHIFT:
     return FALSE;
   case SPELL_DISPEL_INVISIBLE:
     if (!victim->isAffected(AFF_INVISIBLE)) {
       act("Do you need glasses or something?  $N is already visible!", FALSE, caster, NULL, victim, TO_CHAR,ANSI_WHITE);
       act("Humor the pitiful mage and pretend that $N just magically appeared. Clap or something.", FALSE, caster, NULL, victim, TO_ROOM, ANSI_WHITE);
      return TRUE;
     }
     return FALSE;
   case SPELL_ENSORCER:
      if (victim == caster) {
        sprintf(buf, "You tell yourself, \"Gosh darnit! I'm a pretty okay %s!\"", (!caster->getSex() ? "eunuch" : (caster->getSex() == 1 ? "guy" : "gal")));
        act(buf, FALSE, caster, NULL, NULL, TO_CHAR);
	act("Nothing seems to happen.", FALSE, caster, NULL, NULL, TO_ROOM);
       return TRUE;
      }
      if (caster->isAffected(AFF_CHARM)) {
        sprintf(buf, "You can't charm $N -- you're busy taking orders yourself!");
        act("Nothing seems to happen.", FALSE, caster, NULL, NULL, TO_ROOM);
        act(buf, FALSE, caster, NULL, victim, TO_CHAR);
       return TRUE;
      }
      if (victim->isAffected(AFF_CHARM)) {
        again = (victim->master == caster);
        sprintf(buf, "You can't charm $N%s -- $E's busy following %s!", (again ? " again" : ""), (again ? "you already" : "somebody else"));
        act("Nothing seems to happen.", FALSE, caster, NULL, NULL, TO_ROOM);
        act(buf, FALSE, caster, NULL, victim, TO_CHAR);
       return TRUE;
      }
      if (caster->tooManyFollowers(victim, FOL_CHARM)) {
        act("$N refuses to enter a group the size of yours!", TRUE, caster, NULL, victim, TO_CHAR, ANSI_WHITE_BOLD);
        act("$N refuses to enter $ group the size of $n's!", TRUE, caster, NULL, victim, TO_ROOM, ANSI_WHITE_BOLD);
       return TRUE;
      }
      if (victim->circleFollow(caster)) {
        caster->sendTo("Umm, you probably don't want to follow each other around in circles.\n\r");
        act("Nothing seems to happen.", FALSE, caster, NULL, NULL, TO_ROOM);
       return TRUE;
      }
      return FALSE;
    case SPELL_ENLIVEN:
    case SPELL_CLARITY: // shaman
    case SPELL_SILENCE:
    case SPELL_SLUMBER:
    case SPELL_STEALTH:
    case SPELL_CONTROL_UNDEAD:
    case SPELL_ACCELERATE:
    case SPELL_CHEVAL: // shaman
    case SPELL_CELERITE:
    case SPELL_HASTE:
    case SPELL_CALM:
    case SPELL_SENSE_LIFE:
    case SPELL_SENSE_LIFE_SHAMAN: // shaman
    case SPELL_DETECT_INVISIBLE:
    case SPELL_DETECT_SHADOW: // shaman
    case SPELL_TRUE_SIGHT:
    case SPELL_TELEPATHY:
    case SPELL_ROMBLER: // shaman
    case SPELL_INTIMIDATE: // shaman
    case SPELL_FEAR:
    case SPELL_FUMBLE:
      return FALSE;
// disc_alchemy
    case SPELL_EYES_OF_FERTUMAN:
      if (caster->affectedBySpell(SPELL_BLINDNESS)) {
        act("How do you expect to see while you are blind?",
            FALSE, caster, NULL, NULL, TO_CHAR);
        return TRUE;
      } 
      return FALSE;
    case SPELL_FARLOOK:
      if (caster->affectedBySpell(SPELL_BLINDNESS)) {
        act("How do you expect to see while you are blind.", TRUE,caster,0,0,TO_CHAR);
        return TRUE;
      }
      return FALSE;
   case SPELL_HYPNOSIS:
      if (victim == caster) {
        sprintf(buf, "You refuse...and for obvious reasons...");
        act(buf, FALSE, caster, NULL, NULL, TO_CHAR);
	act("Nothing seems to happen.", FALSE, caster, NULL, NULL, TO_ROOM);
       return TRUE;
      }
      if (caster->isAffected(AFF_CHARM)) {
        sprintf(buf, "You can't hypnotize $N while you're under the same affects!");
        act("Nothing seems to happen.", FALSE, caster, NULL, NULL, TO_ROOM);
        act(buf, FALSE, caster, NULL, victim, TO_CHAR);
       return TRUE;
      }
      if (victim->isAffected(AFF_CHARM)) {
        again = (victim->master == caster);
        sprintf(buf, "You can't hypnotize $N%s while $E's busy following %s!", (again ? " again" : ""), (again ? "you already" : "somebody else"));
        act("Nothing seems to happen.", FALSE, caster, NULL, NULL, TO_ROOM);
        act(buf, FALSE, caster, NULL, victim, TO_CHAR);
       return TRUE;
      }
      if (caster->tooManyFollowers(victim, FOL_CHARM)) {
        act("$N refuses to enter a group the size of yours!", TRUE, caster, NULL, victim, TO_CHAR, ANSI_RED_BOLD);
        act("$N refuses to enter $ group the size of $n's!", TRUE, caster, NULL, victim, TO_ROOM, ANSI_RED_BOLD);
       return TRUE;
      }
      if (victim->circleFollow(caster)) {
        caster->sendTo("Umm, you probably don't want to follow each other around in circles.\n\r");
        act("Nothing seems to happen.", FALSE, caster, NULL, NULL, TO_ROOM);
       return TRUE;
      }
      return FALSE;
    case SPELL_SHADOW_WALK: // shaman
    case SPELL_CHRISM: // shaman
    case SPELL_INVISIBILITY:
    case SPELL_POWERSTONE:
    case SPELL_SHATTER:
    case SPELL_DETECT_MAGIC:
    case SPELL_DISPEL_MAGIC:
    case SPELL_CHASE_SPIRIT: // shaman
    case SPELL_ENHANCE_WEAPON:
    case SPELL_MATERIALIZE:
    case SPELL_SPONTANEOUS_GENERATION:
    case SPELL_ETHER_GATE:
    case SPELL_COPY:
    case SPELL_GALVANIZE:
    case SPELL_ILLUMINATE:
    case SPELL_IDENTIFY:
    case SPELL_DIVINATION:
      return FALSE;
// disc_sorcery
    case SPELL_MYSTIC_DARTS:
    case SPELL_BLAST_OF_FURY:
    case SPELL_COLOR_SPRAY:
    case SPELL_ENERGY_DRAIN:
    case SPELL_ACID_BLAST: 
    case SPELL_ATOMIZE:
    case SPELL_SORCERERS_GLOBE:
    case SPELL_AQUATIC_BLAST:
    case SPELL_STORMY_SKIES:
    case SPELL_BLOOD_BOIL:
    case SPELL_AQUALUNG:
    case SPELL_CARDIAC_STRESS:
    case SPELL_DEATHWAVE:
    case SPELL_STICKS_TO_SNAKES:
    case SPELL_DISTORT: // shaman
    case SPELL_LICH_TOUCH: // shaman
    case SPELL_VAMPIRIC_TOUCH: // shaman
    case SPELL_SHIELD_OF_MISTS: // shaman
    case SPELL_LIFE_LEECH: // shaman
    case SPELL_VOODOO: // shaman
    case SPELL_RESURRECTION: // shaman
    case SPELL_DANCING_BONES: // shaman
    case SPELL_RAZE: // shaman
    case SPELL_ANIMATE:
    case SPELL_BIND:
    case SPELL_CLEANSE:
    case SPELL_TELEPORT:
    case SPELL_KNOT:
    case SPELL_PROTECTION_FROM_ELEMENTS:
    case SPELL_STUNNING_ARROW:
    case SPELL_SOUL_TWIST: // shaman
      return FALSE; 

// disc_nature
    case SKILL_BARKSKIN:
    case SKILL_TRANSFORM_LIMB:
      return FALSE;

// disc_fire
    case SPELL_HANDS_OF_FLAME:
      if (!caster->hasHands() || caster->eitherHandHurt()) {
        act("You cannot continue to cast this spell without hands.",
            TRUE, caster, 0, 0, TO_CHAR);
        return TRUE;
      }
      return FALSE;
    case SPELL_FAERIE_FIRE:
      if (victim->affectedBySpell(SPELL_FAERIE_FIRE)) {
        act("You sense that $N is already affected by the spell!",
            FALSE, this, NULL, victim, TO_CHAR);
        return TRUE;
      }
      return FALSE;
    case SPELL_STUPIDITY:
      if (victim->affectedBySpell(SPELL_STUPIDITY)) {
        act("You sense that $N is already stupid!",
            FALSE, this, NULL, victim, TO_CHAR);
        return TRUE;
      }
      return FALSE;
    case SPELL_FLAMING_SWORD:
    case SPELL_INFERNO:
    case SPELL_HELLFIRE:
    case SPELL_FIREBALL:
    case SPELL_CONJURE_FIRE:
    case SPELL_FLARE:
    case SPELL_PROTECTION_FROM_FIRE:
      return FALSE;
    case SPELL_FLAMING_FLESH:
      if (victim->affectedBySpell(SPELL_STONE_SKIN)) { 
        act("$N's skin is already defended by elementals of earth.",
            FALSE, caster, NULL, victim, TO_CHAR);
        return TRUE;
      }
      return FALSE;
    case SPELL_INFRAVISION:
     if (victim->isAffected(AFF_BLIND)) {
       act("Infravision can't work on the blind.", FALSE, caster, 0, victim, TO_CHAR);
       return SPELL_FALSE;
     }
    default:
      return FALSE;
  }
  return FALSE;

}

// returns DELETE_THIS
// returns DELETE_VICT
// returns DELETE_ITEM
int TBeing::doSpellCast(TBeing *caster, TBeing*victim, TObj *o, TRoom *room, spellNumT which, skillUseTypeT styp) 
{
  const char *orgArg;
  TThing *t;
  int ok;
  int rc = 0;
  int retCode = FALSE;
  if (!caster->spelltask)
    return FALSE;


  orgArg = caster->spelltask->orig_arg;

  if (!discArray[which]) {
    vlogf(LOG_BUG, fmt("doSpellCast called with null discArray[] (%d) (%s)") %  which % getName());
    return FALSE;
  }

  if (getPosition() < discArray[which]->minPosition) {
    switch (getPosition()) {
      case POSITION_SLEEPING:
        sendTo("You can't do that while sleeping.\n\r");
        stopCast(STOP_CAST_GENERIC);
        return FALSE;
      case POSITION_CRAWLING:
        sendTo("You can't do that while crawling.\n\r");
        stopCast(STOP_CAST_GENERIC);
        return FALSE;
      default:
        break;
    }
  }

  ok = FALSE;
//   o = NULL;


  if(spellstore.storing && spelltask){
    spellstore.spelltask=spelltask;
    spelltask=NULL;
    spellstore.storing=false;
    act("Your spell has been successfully stored.",
	TRUE,this, NULL, NULL, TO_CHAR, ANSI_BLUE);    
    return TRUE;
  }



  if ((discArray[which]->targets & TAR_VIOLENT) &&
      checkPeaceful("Violent disciplines are not allowed here!\n\r")) {
    stopCast(STOP_CAST_GENERIC);
    return FALSE;
  }

  if (IS_SET(discArray[which]->targets, TAR_CHAR_ROOM) && spelltask->target == 1) {
    if (!victim) {
      vlogf(LOG_BUG,"No victim where there should be in doSpellCast");
      stopCast(STOP_CAST_GENERIC);
      return FALSE;
    }
    if (victim->isPlayerAction(PLR_SOLOQUEST) && (victim != this) &&
        !isImmortal() && isPc()) {
      if (styp == SPELL_PRAYER) {
        act("$N is on a quest, you can't invoke prayers on $M!",
            FALSE, this, NULL, victim, TO_CHAR);
      } else if (styp == SPELL_CASTER) {
        act("$N is on a quest, you can't cast spells on $M!",
            FALSE, this, NULL, victim, TO_CHAR);
      } else if (styp == SPELL_DANCER) {
        act("$N is on a quest, you can't invoke on $M!",
            FALSE, this, NULL, victim, TO_CHAR);
      }
      stopCast(STOP_CAST_GENERIC);
      return FALSE;
    }
    if (victim->isPlayerAction(PLR_GRPQUEST) && (victim != this) &&
            !isImmortal() && isPc() && !isPlayerAction(PLR_GRPQUEST)) {
      if (styp == SPELL_PRAYER) {
        act("$N is on a group quest you aren't on!  No prayers allowed!",
            FALSE, this, NULL, victim, TO_CHAR);
      } else if (styp == SPELL_CASTER) {
        act("$N is on a group quest you aren't on! No spells allowed!",
            FALSE, this, NULL, victim, TO_CHAR);
      } else if (styp == SPELL_DANCER) {
        act("$N is on a group quest you aren't on! No invokation allowed!",
            FALSE, this, NULL, victim, TO_CHAR);
      }
      stopCast(STOP_CAST_GENERIC);
      return FALSE;
    }
    if ((discArray[which]->targets & TAR_VIOLENT) && noHarmCheck(victim)) {
      stopCast(STOP_CAST_GENERIC);
      return FALSE;
    }

    ok = TRUE;
  }
  if (!ok && (spelltask->target == 1) && (discArray[which]->targets & TAR_CHAR_WORLD)) {
    if (!victim) {
      vlogf(LOG_BUG,"No victim where there should be in doSpellCast");
      stopCast(STOP_CAST_GENERIC);
      return FALSE;
    }
    if (canSee(victim) || canSee(victim, INFRA_YES) || (this == victim) ||
          (GetMaxLevel() > victim->getInvisLevel())) {
// make sure is visible COSMO MARKER 

      if (victim && victim->isPlayerAction(PLR_SOLOQUEST) && (victim != this) &&
          !isImmortal() && isPc()) {
        if (styp == SPELL_PRAYER) 
          act("$N is on a quest, you can't invoke prayers on $M!", FALSE, this, NULL, victim, TO_CHAR);
        else if (styp == SPELL_CASTER) 
          act("$N is on a quest, you can't cast spells on $M!", FALSE, this, NULL, victim, TO_CHAR);
        else if (styp == SPELL_DANCER) 
          act("$N is on a quest, you can't invoke upon $M!", FALSE, this, NULL, victim, TO_CHAR);
        stopCast(STOP_CAST_GENERIC);
        return FALSE;
      }
      if ((discArray[which]->targets & TAR_VIOLENT) && victim && noHarmCheck(victim)) {
        stopCast(STOP_CAST_GENERIC);
        return FALSE;
      }
      
      if (victim && victim->isPlayerAction(PLR_GRPQUEST) && (victim != this) &&
            !isImmortal() && isPc() && !isPlayerAction(PLR_GRPQUEST)) {
        if (styp == SPELL_PRAYER) {
          act("$N is on a group quest you aren't on!  No prayers allowed!", 
              FALSE, this, NULL, victim, TO_CHAR);
        } else if (styp == SPELL_CASTER) {
          act("$N is on a group quest you aren't on! No spells allowed!", 
              FALSE, this, NULL, victim, TO_CHAR);
        } else if (styp == SPELL_DANCER) {
          act("$N is on a group quest you aren't on! No invokations allowed!", 
              FALSE, this, NULL, victim, TO_CHAR);
        }
        stopCast(STOP_CAST_GENERIC);
        return FALSE;
      }
      ok = TRUE;
    }
  }
  if (!ok && victim && !(spelltask->flags & CASTFLAG_SEE_VICT)) 
    ok = TRUE;

  if (o) {
    if (!ok && (discArray[which]->targets & TAR_OBJ_INV)) {
      if (canSee(o) && o->thingHolding() == this) {
        ok = TRUE;
      }
    }
  
    if (!ok && (discArray[which]->targets & TAR_OBJ_ROOM)) {
      if (canSee(o) && ((o->roomp == roomp) || (o->thingHolding() == this))) {
        ok = TRUE;
      }
    }
    if (!ok && (discArray[which]->targets & TAR_OBJ_WORLD)) {
      if (canSee(o))
        ok = TRUE;
    }

    if (!ok && (discArray[which]->targets & TAR_OBJ_EQUIP)) {
      for (int i = MIN_WEAR; i < MAX_WEAR && !ok; i++) {
        if ((t = equipment[i]) && t == o) { 
          if (canSee(o))
            ok = TRUE;
        }
      }
    }
  }

  if (!ok && (discArray[which]->targets & TAR_SELF_ONLY)) {
      victim = this;
      ok = TRUE;
  }
  if (!ok && (discArray[which]->targets & TAR_NAME))
    ok = TRUE;

  if (victim) {
    if (dynamic_cast<TMonster *>(victim)) {
      if ((victim->specials.act & ACT_IMMORTAL)) {
        if (styp == SPELL_PRAYER) {
          sendTo("Invoke a prayer on an immortal being?!?! Never!!\n\r");
        } else if (styp == SPELL_CASTER) {
          sendTo("Cast magic on an immortal being?!?! Never!!\n\r");
        } else if (styp == SPELL_DANCER) {
          sendTo("Invoke on an immortal being?!?! Don't be a moron!!\n\r");
        }
        stopCast(STOP_CAST_GENERIC);
        return FALSE;
      }
    }
    if (!ok && (discArray[which]->targets & TAR_IGNORE))  
      ok = TRUE;
  } else {
    if ((discArray[which]->targets & TAR_FIGHT_SELF)) {
      if (fight() && sameRoom(*fight())) {
        victim = this;
        ok = TRUE;
      }
    }
    if (!ok && (discArray[which]->targets & TAR_FIGHT_VICT)) {
      if (fight() && sameRoom(*fight())) {
        victim = fight();
        ok = TRUE;
      }
    }
    if (!ok && (discArray[which]->targets & TAR_SELF_ONLY)) {
      victim = this;
      ok = TRUE;
    }
    if (!ok && (discArray[which]->targets & TAR_IGNORE)) {
      victim = this;
      ok = TRUE;
    }
  }
  if (!ok) {
      if ((discArray[which]->targets & TAR_CHAR_WORLD) || (discArray[which]->targets & TAR_CHAR_WORLD))
        sendTo("There is no such person in this realm!\n\r");
      else if (discArray[which]->targets & TAR_CHAR_ROOM)
        sendTo("There is no such person in this room!\n\r");
      else if (discArray[which]->targets & TAR_OBJ_INV)
        sendTo("There is no such object in your possession.\n\r");
      else if (discArray[which]->targets & TAR_OBJ_ROOM)
        sendTo("There is no such object in this room!\n\r");
      else if (discArray[which]->targets & TAR_OBJ_WORLD)
        sendTo("There is no such object in this realm!\n\r");
      else if (discArray[which]->targets & TAR_OBJ_EQUIP)
        sendTo("You are not wearing anything like that.\n\r");
      else
         sendTo("Invalid target flag for that spell.  Bug this!\n\r");

    if ((victim == this) && (discArray[which]->targets & TAR_SELF_NONO)) {
      if (styp == SPELL_PRAYER) 
        sendTo("You cannot invoke this prayer upon yourself.\n\r");
      else if (styp == SPELL_CASTER) 
        sendTo("You cannot cast this spell upon yourself.\n\r");
      else if (styp == SPELL_DANCER) 
        sendTo("You cannot invoke this upon yourself.\n\r");
      stopCast(STOP_CAST_GENERIC);
      return FALSE;
    } else if ((victim != this) && (discArray[which]->targets & TAR_SELF_ONLY)) {
      if (styp == SPELL_PRAYER) 
        sendTo("You can only invoke this prayer upon yourself.\n\r");
      else if (styp == SPELL_CASTER) 
        sendTo("You can only cast this spell upon yourself.\n\r");
      else if (styp == SPELL_DANCER) 
        sendTo("You can only invoke this upon yourself.\n\r");
      stopCast(STOP_CAST_GENERIC);
      return FALSE;
    } else if (isAffected(AFF_CHARM) && (master == victim) && !isPc()) {
      sendTo("You are afraid that it could harm your master.\n\r");
      stopCast(STOP_CAST_GENERIC);
      return FALSE;
    }
    stopCast(STOP_CAST_GENERIC);
    return FALSE;
  }
  if (isPc() && canSpeak()) {
    if ((discArray[which]->minMana) && (getWizardryLevel() < WIZ_LEV_NO_MANTRA))
      saySpell(which);
    if ((discArray[which]->minLifeforce) && (getRitualismLevel() < RIT_LEV_NO_MANTRA))
      saySpell(which);

    if (spelltask && IS_SET(discArray[which]->comp_types, SPELL_TASKED_EVERY)) {
      if (!IS_SET(spelltask->flags, CASTFLAG_CAST_INDEFINITE) && !(spelltask->rounds)) {
        if (discArray[which]->holyStrength && 
            (getDevotionLevel() < DEV_LEV_NO_MANTRA)) 
          saySpell(which);
      }
    } else {
      if (discArray[which]->holyStrength && 
          (getDevotionLevel() < DEV_LEV_NO_MANTRA)) 
        saySpell(which);
    }
  }

// COSMO MARKER: Spell lag added here
#if 0
  // one combat round is 2 seconds
  addSkillLag(which, rc);
  // We can call the spell now, switch to see which to call!
  sendTo("Ok.\n\r");
// COSMO MARKER: Mana..Piety taken through here ...have to change useMana too
  if (isPc()) {
    if (discArray[which]->minMana)
      reconcileMana(which, FALSE);
    else if (discArray[which]->minLifeforce)
      reconcileLifeforce(which, FALSE);
    else
      reconcilePiety(which, FALSE);
  }
#endif
  your_deity_val = which;

  switch(which) {
// disc_hand_of_god
      case SPELL_CREATE_WATER_DEIKHAN:
      case SPELL_CREATE_WATER:
        if (o)
          rc = castCreateWater(this, o);
 
        break;
// disc_air
      case SPELL_HEAL_LIGHT:
      case SPELL_HEAL_LIGHT_DEIKHAN:
        if (!o)
          rc = castHealLight(this,victim);
        else
          vlogf(LOG_BUG,"SPELL_HEAL_LIGHT called with null obj");
        break;
      case SPELL_HEAL_SERIOUS:
      case SPELL_HEAL_SERIOUS_DEIKHAN:
        if (!o)
          rc = castHealSerious(this,victim);
        else
          vlogf(LOG_BUG,"SPELL_HEAL_SERIOUS called with null obj");
        break;
      case SPELL_HEAL_CRITICAL:
      case SPELL_HEAL_CRITICAL_DEIKHAN:
        if (!o)
          rc = castHealCritical(this,victim);
        else
          vlogf(LOG_BUG,"SPELL_HEAL_CRITICAL called with null obj");
        break;
      case SPELL_HEAL:
        if (!o)
          rc = castHeal(this,victim);
        else
          vlogf(LOG_BUG,"SPELL_HEAL called with null obj");
        break;
      case SPELL_HEAL_FULL:
        if (!o)
          rc = castHealFull(this,victim);
        else
          vlogf(LOG_BUG,"SPELL_HEAL_FULL called with null obj");
        break;
      case SPELL_GUST:
        if (!o) 
          rc = castGust(this,victim);
        else
          vlogf(LOG_BUG,"SPELL_GUST called with null obj");
        break;
      case SPELL_IMMOBILIZE: 
        if (!o) 
          rc = castImmobilize(this,victim);
        else
          vlogf(LOG_BUG,"SPELL_IMMOBILIZE called with null obj");
        break;
      case SPELL_SUFFOCATE:
        if (!o) 
          rc = castSuffocate(this,victim);
        else
          vlogf(LOG_BUG,"SPELL_SUFFOCATE called with null obj");
        break;
      case SPELL_DUST_STORM:
        if (!o) 
          rc = castDustStorm(this);
        else
          vlogf(LOG_BUG, "SPELL_DUST_STORM called with null obj");
        break;
      case SPELL_FEATHERY_DESCENT:
        if (!o) {
          rc = castFeatheryDescent(this, victim);
        } else
          vlogf(LOG_BUG, "SPELL_FEATHERY_DESCENT called with null obj");
        break;
      case SPELL_FLY:
        if (!o) {
          rc = castFly(this, victim);
        } else
          vlogf(LOG_BUG, "SPELL_FLY called with null obj");
        break;
      case SPELL_LEVITATE:
        if (!o) {
          rc = castLevitate(this,victim);
        } else
          vlogf(LOG_BUG, "SPELL_LEVITATE called with null obj");
        break;
      case SPELL_TORNADO:
        if (!o) {
          rc = castTornado(this);
        } else
          vlogf(LOG_BUG, "SPELL_TORNADO called with null obj");
        break;
      case SPELL_ANTIGRAVITY:
        if (!o) {
          rc = castAntigravity(this);
        } else
          vlogf(LOG_BUG, "SPELL_ANTIGRAVITY called with null obj");
        break;
      case SPELL_FALCON_WINGS:
        if (!o) {
          rc = castFalconWings(this, victim);
        } else
          vlogf(LOG_BUG, "SPELL_FALCON_WINGS called with null obj");
        break;
      case SPELL_CONJURE_AIR:
        if (!o) {
          rc = castConjureElemAir(this);
        } else
          vlogf(LOG_BUG, "SPELL_CONJURE_AIR called with null obj");
        break;
      case SPELL_PROTECTION_FROM_AIR:
        if (!o) {
          rc = castProtectionFromAir(this, victim);
        } else
          vlogf(LOG_BUG, "SPELL_PROTECTION_FROM_AIR called with null obj");
        break;

// disc_air
      case SPELL_SLING_SHOT:
        if (!o) {
          rc = castSlingShot(this, victim);
        } else
          vlogf(LOG_BUG, "SPELL_SLING_SHOT called with null obj");
        break;
      case SPELL_GRANITE_FISTS:
        if (!o) {
          rc = castGraniteFists(this, victim);
        } else
          vlogf(LOG_BUG, "SPELL_GRANITE_FISTS called with null obj");
        break;
      case SPELL_PEBBLE_SPRAY:
        if (!o) {
          rc = castPebbleSpray(this);
        } else
          vlogf(LOG_BUG, "SPELL_PEBBLE_SPRAY called with null obj");
        break;
      case SPELL_SAND_BLAST:
        if (!o) {
          rc = castSandBlast(this);
        } else
          vlogf(LOG_BUG, "SPELL_SAND_BLAST called with null obj");
        break;
      case SPELL_LAVA_STREAM:
        if (!o) {
          rc = castLavaStream(this);
        } else
          vlogf(LOG_BUG, "SPELL_LAVA_STREAM called with null obj");
        break;
      case SPELL_METEOR_SWARM:
        if (!o) {
          rc = castMeteorSwarm(this, victim);
        } else
          vlogf(LOG_BUG, "SPELL_LAVA_STREAM called with null obj");
        break;
      case SPELL_STONE_SKIN:
        if (!o) {
          castStoneSkin(this, victim);
        } else
          vlogf(LOG_BUG, "SPELL_STONE_SKIN called with null obj");
        break;
      case SPELL_TRAIL_SEEK:
        if (!o) {
          castTrailSeek(this, victim);
        } else
          vlogf(LOG_BUG, "SPELL_TRAIL_SEEK called with null obj");
        break;
      case SPELL_CONJURE_EARTH:
        if (!o) {
          rc = castConjureElemEarth(this);
        } else
          vlogf(LOG_BUG, "SPELL_CONJURE_EARTH called with null obj");
        break;
      case SPELL_AQUATIC_BLAST:
	if (!o) {
	  rc = castAquaticBlast(this, victim);
	} else
	  vlogf(LOG_BUG, "SPELL_AQUATIC_BLAST called with NULL obj");
        break;
      case SPELL_STORMY_SKIES:
	if (!o) {
	  rc = castStormySkies(this, victim);
	} else
	  vlogf(LOG_BUG, "SPELL_STORMY_SKIES called with NULL obj");
        break;
      case SPELL_CARDIAC_STRESS:
	if (!o) {
	  rc = castCardiacStress(this, victim);
	} else
	  vlogf(LOG_BUG, "SPELL_CARDIAC_STRESS [coronary] called with NULL obj");
        break;
    case SPELL_PROTECTION_FROM_EARTH:
        if (!o) {
          rc = castProtectionFromEarth(this, victim);
        } else
          vlogf(LOG_BUG, "SPELL_PROTECTION_FROM_EARTH called with null obj");
        break;
    case SPELL_HEALING_GRASP:
      if (!o)
	rc = castHealingGrasp(this,victim);
      else
	vlogf(LOG_BUG,"SPELL_HEALING_GRASP called with null obj");
      break;
    case SPELL_FLATULENCE:
      if (!o) {
	rc = castFlatulence(this);
      } else
	vlogf(LOG_BUG, "SPELL_FLATULENCE called with null obj");
      break;
    case SPELL_SQUISH:
        if (!o) {
          rc = castSquish(this, victim);
        } else
          vlogf(LOG_BUG, "SPELL_SQUISH called with null obj");
        break;
      case SPELL_ENTHRALL_SPECTRE:
        if (!o) {
          rc = castEnthrallSpectre(this);
        } else
          vlogf(LOG_BUG, "SPELL_ENTHRALL_SPECTRE called with null obj");
        break;
      case SPELL_ENTHRALL_GHAST:
        if (!o) {
          rc = castEnthrallGhast(this);
        } else
          vlogf(LOG_BUG, "SPELL_ENTHRALL_GHAST called with null obj");
        break;
      case SPELL_ENTHRALL_GHOUL:
        if (!o) {
          rc = castEnthrallGhoul(this);
        } else
          vlogf(LOG_BUG, "SPELL_ENTHRALL_GHOUL called with null obj");
        break;
      case SPELL_ENTHRALL_DEMON:
        if (!o) {
          rc = castEnthrallDemon(this);
        } else
          vlogf(LOG_BUG, "SPELL_ENTHRALL_DEMON called with null obj");
        break;
      case SPELL_CREATE_WOOD_GOLEM:
        if (!o) {
          rc = castCreateWoodGolem(this);
        } else
          vlogf(LOG_BUG, "SPELL_CREATE_WOOD_GOLEM called with null obj");
        break;
      case SPELL_CREATE_ROCK_GOLEM:
        if (!o) {
          rc = castCreateRockGolem(this);
        } else
          vlogf(LOG_BUG, "SPELL_CREATE_ROCK_GOLEM called with null obj");
        break;
      case SPELL_CREATE_IRON_GOLEM:
        if (!o) {
          rc = castCreateIronGolem(this);
        } else
          vlogf(LOG_BUG, "SPELL_CREATE_IRON_GOLEM called with null obj");
        break;
      case SPELL_CREATE_DIAMOND_GOLEM:
        if (!o) {
          rc = castCreateDiamondGolem(this);
        } else
          vlogf(LOG_BUG, "SPELL_CREATE_DIAMOND_GOLEM called with null obj");
        break;
// disc_water
      case SPELL_FAERIE_FOG:
        rc = castFaerieFog(this);
        break;
      case SPELL_ICY_GRIP:
        rc = castIcyGrip(this, victim);
        break;
      case SPELL_WATERY_GRAVE:
        rc = castWateryGrave(this, victim);
        break;
      case SPELL_ICE_STORM:
        rc = castIceStorm(this);
        break;
      case SPELL_TSUNAMI:
        rc = castTsunami(this);
        break;
    case SPELL_CONJURE_WATER:
        rc = castConjureElemWater(this);
        break;
    case SPELL_GILLS_OF_FLESH:
        rc = castGillsOfFlesh(this, victim);
        break;
    case SPELL_AQUALUNG:
        rc = castAqualung(this, victim);
        break;
   case SPELL_GARMULS_TAIL:
        rc = castGarmulsTail(this, victim);
        break;
   case SPELL_PLASMA_MIRROR:
        rc = castPlasmaMirror(this);
        break;
   case SPELL_THORNFLESH:
        rc = castThornflesh(this);
        break;
   case SPELL_BREATH_OF_SARAHAGE:
        rc = castBreathOfSarahage(this);
        break;
   case SPELL_PROTECTION_FROM_WATER:
        rc = castProtectionFromWater(this, victim);
        break;
   case SPELL_DJALLA:
        rc = castDjallasProtection(this, victim);
        break;
   case SPELL_LEGBA:
        rc = castLegbasGuidance(this, victim);
        break;
   case SPELL_GUSHER:
        rc = castGusher(this, victim);
        break;
   case SPELL_ARCTIC_BLAST:
        rc = castArcticBlast(this);
        break;

// disc_spirit
//#if 0
     case SPELL_POLYMORPH:
        rc = castPolymorph(this);
        break;
//#endif
     case SPELL_SHADOW_WALK:
        castShadowWalk(this, victim);
        break;
     case SPELL_CLARITY:
        castClarity(this, victim);
        break;
     case SPELL_CONTROL_UNDEAD:
        castControlUndead(this, victim);
        break;
     case SPELL_SHAPESHIFT:
        rc = castShapeShift(this);
        break;
     case SPELL_SILENCE:
        rc = castSilence(this, victim);
        break;
     case SPELL_SLUMBER:
        rc = castSlumber(this, victim);
        break;
     case SPELL_ENSORCER:
        castEnsorcer(this, victim);
        break;
     case SPELL_CLOUD_OF_CONCEALMENT:
        rc = castCloudOfConcealment(this);
        break;
     case SPELL_DISPEL_INVISIBLE:
        castDispelInvisible(this, victim);
        break;
     case SPELL_STEALTH:
        castStealth(this, victim);
        break;
     case SPELL_ACCELERATE:
        castAccelerate(this, victim);
        break;
     case SPELL_CHEVAL:
        castCheval(this, victim);
        break;
     case SPELL_CELERITE:
        castCelerite(this, victim);
        break;
     case SPELL_HASTE:
        castHaste(this, victim);
        break;
     case SPELL_CALM:
        castCalm(this, victim);
        break;
     case SPELL_INVISIBILITY:
        if (!o) {
          castInvisibility(this, victim);
        } else
          castInvisibility(this, o);
        break;
     case SPELL_SENSE_LIFE:
        castSenseLife(this, victim);
        break;
     case SPELL_SENSE_LIFE_SHAMAN:
        castSenseLifeShaman(this, victim);
        break;
     case SPELL_DETECT_INVISIBLE:
        castDetectInvisibility(this, victim);
        break;
     case SPELL_DETECT_SHADOW:
        castDetectShadow(this, victim);
        break;
     case SPELL_TRUE_SIGHT:
        castTrueSight(this, victim);
        break;
     case SPELL_TELEPATHY:
        castTelepathy(this);
        break;
     case SPELL_ROMBLER:
        castRombler(this);
        break;
     case SPELL_FEAR:
        castFear(this, victim);
        break;
     case SPELL_INTIMIDATE:
        castIntimidate(this, victim);
        break;
     case SPELL_FUMBLE:
        rc = castFumble(this, victim);
        break;
// disc_alchemy
      case SPELL_ILLUMINATE:
        rc = castIlluminate(this, o);
        break;
      case SPELL_IDENTIFY:
        if (o) {
          rc = castIdentify(this, o);
        } else
          rc = castIdentify(this, victim);
        break;
      case SPELL_DIVINATION:
        if (o) {
          rc = castDivinationObj(this, o);
        } else
          rc = castDivinationBeing(this, victim);
        break;
      case SPELL_EYES_OF_FERTUMAN:
        rc = castEyesOfFertuman(this, orgArg);
	break;
      case SPELL_POWERSTONE:
        rc = castPowerstone(this, o);
        break;
      case SPELL_SHATTER:
        rc = castShatter(this, victim);
        break;
      case SPELL_FARLOOK:
        rc = castFarlook(this, victim);
        break;
      case SPELL_DETECT_MAGIC:
        rc = castDetectMagic(this, victim);
        break;
      case SPELL_DISPEL_MAGIC:
        if (o) {
          rc = castDispelMagic(this, o);
        } else
          rc = castDispelMagic(this, victim);
        break;
      case SPELL_ENHANCE_WEAPON:
        rc = castEnhanceWeapon(this, o);
        break;
     case SPELL_MATERIALIZE:
        rc = castMaterialize(this, orgArg);
        break;
     case SPELL_SPONTANEOUS_GENERATION:
        rc = castSpontaneousGeneration(this, orgArg);
        break;
     case SPELL_ETHER_GATE:
        castEthrealGate(this, o);
        break;
      case SPELL_COPY:
        rc = castCopy(this, o);
        break;
      case SPELL_GALVANIZE:
        rc = castGalvanize(this, o);
        break;
// disc_sorcery
      case SPELL_MYSTIC_DARTS:
        if (!o) 
          rc = castMysticDarts(this, victim);
        else
          vlogf(LOG_BUG, "SPELL_MYSTIC_DARTS called with null obj");
        break;
     case SPELL_STICKS_TO_SNAKES:
        if (!o) 
          rc = castSticksToSnakes(this, victim);
        else
          vlogf(LOG_BUG, "SPELL_STICKS_TO_SNAKES called with null obj");
        break;
      case SPELL_DISTORT:
        if (!o) 
          rc = castDistort(this, victim);
        else
          vlogf(LOG_BUG, "SPELL_DISTORT called with null obj");
        break;
      case SPELL_CHASE_SPIRIT:
        if (o) {
          rc = castChaseSpirits(this, o);
        } else
          rc = castChaseSpirits(this, victim);
        break;
      case SPELL_DEATHWAVE:
        if (!o) 
          rc = castDeathWave(this, victim);
        else
          vlogf(LOG_BUG, "SPELL_DEATHWAVE called with null obj");
        break;
      case SPELL_BLOOD_BOIL:
        if (!o) {
          rc = castBloodBoil(this, victim);
        } else
          vlogf(LOG_BUG, "SPELL_BLOOD_BOIL called with null obj");
        break;
      case SPELL_BLAST_OF_FURY:
        if (!o) {
          rc = castBlastOfFury(this, victim);
        } else
          vlogf(LOG_BUG, "SPELL_BLAST_OF_FURY called with null obj");
        break;
      case SPELL_COLOR_SPRAY:
        if (!o) {
          rc = castColorSpray(this);
        } else
          vlogf(LOG_BUG, "SPELL_COLOR_SPRAY called with null obj");
        break;
      case SPELL_ENERGY_DRAIN:
        if (!o) {
          rc = castEnergyDrain(this, victim);
        } else
          vlogf(LOG_BUG, "SPELL_ENERGY_DRAIN called with null obj");
        break;
      case SPELL_ACID_BLAST:
        if (!o) {
          rc = castAcidBlast(this);
        } else
          vlogf(LOG_BUG, "SPELL_ACID_BLAST called with null obj");
        break;
      case SPELL_ATOMIZE:
        if (!o) {
          rc = castAtomize(this, victim);
        } else
          vlogf(LOG_BUG, "SPELL_ATOMIZE called with null obj");
        break;
      case SPELL_SORCERERS_GLOBE:
        if (!o) {
          rc = castSorcerersGlobe(this, victim);
        } else
          vlogf(LOG_BUG, "SPELL_SORCERERS_GLOBE called with null obj");
        break;
      case SPELL_LICH_TOUCH:
        if (!o) {
          rc = castLichTouch(this, victim);
        } else
          vlogf(LOG_BUG, "SPELL_LICH_TOUCH called with null obj");
        break;
      case SPELL_HYPNOSIS:
        castHypnosis(this, victim);
        break;
      case SPELL_CLEANSE:
        castCleanse(this, victim);
        break;
     case SPELL_CHRISM:
        rc = castChrism(this, orgArg);
        break;
      case SPELL_VAMPIRIC_TOUCH:
        if (!o) {
          rc = castVampiricTouch(this, victim);
        } else
          vlogf(LOG_BUG, "SPELL_VAMPIRIC_TOUCH called with null obj");
        break;
      case SPELL_VOODOO:
	if (o) { // !o
          castVoodoo(this, o);
        } else
          vlogf(LOG_BUG, "SPELL_VOODOO called with null obj");
        break;
      case SPELL_ENLIVEN:
        castEnliven(this, victim);
        break;
      case SPELL_DANCING_BONES:
	if (o) { // !o
          castDancingBones(this, o);
        } else
          vlogf(LOG_BUG, "SPELL_DANCING_BONES called with null obj");
        break;
      case SPELL_RAZE:
        if (!o) {
          rc = castRaze(this, victim);
        } else
          vlogf(LOG_BUG, "SPELL_RAZE called with null obj");
        break;
      case SPELL_RESURRECTION:
	if (o) { // !o
          castResurrection(this, o);
        } else
          vlogf(LOG_BUG, "SPELL_RESURRECTION called with null obj");
        break;
      case SPELL_LIFE_LEECH:
        if (!o) {
          rc = castLifeLeech(this, victim);
        } else
          vlogf(LOG_BUG, "SPELL_LIFE_LEECH called with null obj");
        break;
      case SPELL_SHIELD_OF_MISTS:
        if (!o) {
          rc = castShieldOfMists(this, victim);
        } else
          vlogf(LOG_BUG, "SPELL_SHIELD_OF_MISTS called with null obj");
        break;
      case SPELL_ANIMATE:
       if (!o) {
          castAnimate(this);
        } else
          vlogf(LOG_BUG, "SPELL_ANIMATE called with null obj");
        break;
      case SPELL_BIND:
        if (!o) {
          rc = castBind(this, victim);
        } else
          vlogf(LOG_BUG, "SPELL_BIND called with null obj");
        break;
      case SPELL_KNOT:
	if (!o) {
          rc = castKnot(this, victim);
        } else
          vlogf(LOG_BUG, "SPELL_KNOT called with null obj");
	break;
      case SPELL_TELEPORT:
        if (!o) {
          rc = castTeleport(this, victim);
        } else
          vlogf(LOG_BUG, "SPELL_TELEPORT called with null obj");
        break;
      case SPELL_DEATH_MIST:
        if (!o) {
          rc = castDeathMist(this);
        } else
          vlogf(LOG_BUG, "SPELL_DEATH_MIST called with null obj");
        break;
      case SPELL_PROTECTION_FROM_ELEMENTS:
        if (!o) 
          rc = castProtectionFromElements(this, victim);
        else
          vlogf(LOG_BUG, "SPELL_PROTECTION_FROM_ELEMENTS called with null obj");
        break;
      case SPELL_STUNNING_ARROW:
        if (!o) {
          rc = castStunningArrow(this, victim);
        } else
          vlogf(LOG_BUG, "SPELL_STUNNING_ARROW called with null obj");
        break;
      case SPELL_SOUL_TWIST:
        if (!o) {
          rc = castSoulTwist(this, victim);
        } else
          vlogf(LOG_BUG, "SPELL_SOUL_TWIST called with null obj");
        break;

// disc_nature
      case SKILL_BARKSKIN:
        if (!o) 
          rc = castBarkskin(this, victim);
        else
          vlogf(LOG_BUG ,"SKILL_BARKSKIN called with null obj");
        break;
      case SKILL_TRANSFORM_LIMB:
        if (!o) {
          rc = castTransformLimb(this);
        } else
          vlogf(LOG_BUG, "SKILL_TRANSFORM_LIMB called with null obj");
        break;

// disc_fire
      case SPELL_HANDS_OF_FLAME:
        if (!o) {
          rc = castHandsOfFlame(this, victim);
        } else
          vlogf(LOG_BUG, "SPELL_HANDS_OF_FLAME called with null obj");
        break;
      case SPELL_FAERIE_FIRE:
        if (!o) {
          rc = castFaerieFire(this, victim);
        } else
          vlogf(LOG_BUG, "SPELL_FAERIE_FIRE called with null obj");
        break;
      case SPELL_FLAMING_SWORD:
        if (!o) {
          rc = castFlamingSword(this, victim);
        } else
          vlogf(LOG_BUG, "SPELL_FLAMING_SWORD called with null obj");
        break;
      case SPELL_INFERNO:
	if (!o) {
	  rc = castInferno(this, victim);
	} else
          vlogf(LOG_BUG, "SPELL_INFERNO called with null obj");
        break;
    case SPELL_HELLFIRE:  
        if (!o) {
          rc = castHellfire(this);
        } else
          vlogf(LOG_BUG, "SPELL_HELLFIRE called with null obj");
        break;
    case SPELL_FIREBALL:
	if (!o) {
	  rc = castFireball(this);
	} else
	  vlogf(LOG_BUG, "SPELL_FIREBALL called with null obj");
	break;
    case SPELL_FLAMING_FLESH:
        if (!o) {
          rc = castFlamingFlesh(this, victim);
        } else
          vlogf(LOG_BUG, "SPELL_FLAMING_FLESH called with null obj");
        break;
    case SPELL_CONJURE_FIRE:
	if (!o) {
	  rc = castConjureElemFire(this);
	} else
	  vlogf(LOG_BUG, "SPELL_CONJURE_FIRE called with null obj");
	break;
    case SPELL_FLARE:
        if (!o) {
          rc = castFlare(this);
        } else
          vlogf(LOG_BUG, "SPELL_FLARE called with null obj");
        break;
    case SPELL_INFRAVISION:
        if (!o) {
          rc = castInfravision(this, victim);
        } else
          vlogf(LOG_BUG, "SPELL_INFRAVISION called with null obj");
        break; 
    case SPELL_STUPIDITY:
      if (!o) {
	rc = castStupidity(this, victim);
      } else
          vlogf(LOG_BUG, "SPELL_STUPIDITY called with null obj");
        break;
    case SPELL_PROTECTION_FROM_FIRE:
        if (!o) {
          rc = castProtectionFromFire(this, victim);
        } else
          vlogf(LOG_BUG, "SPELL_PROTECTION_FROM_FIRE called with null obj");
        break;
    case SPELL_EMBALM:
      if (o) { // !o
	castEmbalm(this, o);
      } else
	vlogf(LOG_BUG, "SPELL_EMBALM called with null obj");
      break;
      break;
    default:
      sendTo("Spell or discipline not yet implemented (doSpellCast)!\n\r");
      break;
  }
  if (spelltask && IS_SET(discArray[which]->comp_types, SPELL_TASKED_EVERY)) {
    if (!IS_SET(spelltask->flags, CASTFLAG_CAST_INDEFINITE) &&  
          spelltask->rounds == 0) {
      stopCast(STOP_CAST_NONE);
    } else if (IS_SET(spelltask->flags, CASTFLAG_CAST_INDEFINITE)) {
      if ((styp == SPELL_PRAYER) && (getPiety() <= 0)) {
        REMOVE_BIT(spelltask->flags, CASTFLAG_CAST_INDEFINITE);
        spelltask->rounds = min((int) discArray[which]->lag, spelltask->rounds);
        sendTo("You are forced to start finishing your prayer!\n\r");
      }
    }
  } else if (spelltask) {
    stopCast(STOP_CAST_NONE);
  }

  if (IS_SET_DELETE(rc, DELETE_VICT) && victim != this) {
    ADD_DELETE(retCode, DELETE_VICT);
  }
  if (o && IS_SET_DELETE(rc, DELETE_ITEM)) {
    ADD_DELETE(retCode, DELETE_ITEM);
  }
  if ((IS_SET_DELETE(rc, DELETE_VICT) && victim == this) ||
      IS_SET_DELETE(rc, DELETE_THIS))
    ADD_DELETE(retCode, DELETE_THIS);

  return retCode;
}

int TBeing::applyCompCheck(spellNumT spell, int round, int status)
{
  int use = 0;
  int roll = 0;

  if (spelltask->component) 
    return TRUE;
  
// status is not used as of now
  if (status) {
  } 
//  use = discArray[spell]->comp_types;

  roll = max(0,((::number(0,round)) - 1));

  if (!IS_SET(discArray[spell]->comp_types, COMP_MATERIAL))   
    use = 0;
  else if (IS_SET(discArray[spell]->comp_types, COMP_MATERIAL_INIT)) {
    return TRUE;
  } else if (IS_SET(discArray[spell]->comp_types, COMP_MATERIAL_END))
    use = 2;
  else if (IS_SET(discArray[spell]->comp_types, COMP_MATERIAL_ALWAYS))
    use = 3;
  else if (IS_SET(discArray[spell]->comp_types, COMP_MATERIAL_RANDOM))
    use = 4;
  else if (IS_SET(discArray[spell]->comp_types, COMP_MATERIAL_ALMOST_END))
    use = 5;
  else 
    vlogf(LOG_BUG,fmt("Bad case in spell_parser.comp_type(%d)") % spell);
  
// No component needed
  if (!use)
    return TRUE;

  if (!useComponent(findComponent(spell), spelltask->victim, CHECK_ONLY_YES))
    return FALSE;
   
// see if the spell array wants the component or symbol used each round
// or this round
// use is 0 if no component, 1 if at inititation, 2 if at end, 3 if every 
// round, 4 if random internal round, 5 is next to last
  switch (use) {
    case 0:
      return TRUE;
    case 1:
//      if (status)
//        return TRUE;
//      break;
    case 2:
      if (round)
        return TRUE;
      break;
    case 3:
      break;
    case 4:
      if ((round = 0))
        break;
      if (roll) 
        return TRUE;
      break;
    case 5:
      if (!(round == 1))
        return TRUE;
      break;
    default:
      vlogf(LOG_BUG,fmt("Bad case in applyCompCheck (%s)") %  getName());
      return FALSE;
  }

  if (!doComponentUse(spell, this))
    return FALSE;

  return TRUE;
}

int TBeing::checkHolySymbol(spellNumT spell)
{
  TSymbol * holy;
  int orig_strength, sym_stress;

  if (isImmortal() || !desc)
    return TRUE;

  if (desc && dynamic_cast<TMonster *>(this)) {
    if (desc->original && (desc->original->GetMaxLevel() > MAX_MORT))
      return FALSE;
  }

  // TThing *primary = heldInPrimHand();
  // TThing *secondary = heldInSecHand();
  // TThing *neck = equipment[WEAR_NECK];

  // byte ubDevotion = getSkillValue(SKILL_DEVOTION);
  sym_stress = discArray[spell]->holyStrength;


#if 0
  if (!desc) {
    return TRUE;
  }
#endif

  if (isPc()) {
    holy = findHolySym(SILENT_YES);
    if (!holy)
      return FALSE;

    // int curr_strength = holy->getSymbolCurStrength();
    orig_strength = holy->getSymbolMaxStrength();
    if (sym_stress >= orig_strength) 
      return FALSE;
    else 
      return TRUE;
  }
  // doesn't require holy symbol
  return TRUE;
}



void TBeing::sendFinalCastingMessages(bool limbs, bool silence, skillUseTypeT typ)
{
  int roll;

//  First, do gestures
  if (limbs) {
    roll = ::number(0,4);
    switch (roll) {
      default:
        if (typ == SPELL_CASTER) {
          act("With a flourish, you draw the final line of the magic pattern.",
              TRUE,this, NULL, NULL, TO_CHAR, ANSI_CYAN);
          act("$n makes a final gesture and completes the magic pattern.",
              TRUE,this, NULL, NULL, TO_ROOM, ANSI_CYAN);
        } else if (typ == SPELL_DANCER) {
          act("You hold your hands up high and call upon your ancestors for power.",
              FALSE, this, NULL, NULL, TO_CHAR, ANSI_YELLOW);
          act("$n holds $s hands up high and cries out to $s ancestors for power.",
              TRUE, this, NULL, NULL, TO_ROOM, ANSI_YELLOW);
        } else if (typ == SPELL_PRAYER) {
          if (isPc()) {
            act("You raise your glowing symbol strongly to the heavens.",
                FALSE,this, NULL, NULL, TO_CHAR, ANSI_GREEN);
            act("$n raises $s glowing symbol strongly towards the heavens.",
                TRUE,this, NULL, NULL, TO_ROOM, ANSI_GREEN);
          } else {
            act("You look to the heavens for assistance.",
                FALSE,this, NULL, NULL, TO_CHAR, ANSI_GREEN);
            act("$n looks to the heavens for assistance.",
                TRUE,this, NULL, NULL, TO_ROOM, ANSI_GREEN);
          }
        }
        break;
    }
  } else {
    if (typ == SPELL_CASTER) {
      act("You focus one last time on casting your spell.",
          FALSE, this, NULL, NULL, TO_CHAR, ANSI_CYAN);
    } else if (typ == SPELL_DANCER) {
      act("You sing out the last lyric of your rada song.",
          FALSE, this, NULL, NULL, TO_CHAR, ANSI_YELLOW);
      act("$n sings out the last lyric of $s rada song.",
          FALSE, this, NULL, NULL, TO_ROOM, ANSI_YELLOW);
    } else if (typ == SPELL_PRAYER) {
      act("Your symbol glows brightly one last time.",
          FALSE, this, NULL, NULL, TO_CHAR, ANSI_GREEN);
      act("$n's symbol glows brightly.",
          TRUE,this, NULL, NULL, TO_ROOM, ANSI_GREEN);
    }
  }
  //  Next, do verbal
  // determine if the caster will be using verbal or will be relying on wiz
  if (silence) {
    roll = ::number(0,4);
    switch (roll) {
      default:
        if (typ == SPELL_CASTER) {
          act("You utter the final words of the incantation.",
              FALSE, this, NULL, NULL, TO_CHAR, ANSI_CYAN);
          act("$n's voice cries out as $e utters a last word of power.",
              TRUE, this, NULL, NULL, TO_ROOM, ANSI_CYAN);
        } else if (typ == SPELL_DANCER) {
          act("$n cries out and is enveloped in a blood red mist.",
              TRUE,this, NULL, NULL, TO_ROOM, ANSI_YELLOW);
          act("Crying out, you can feel the power of your ancestors flow.",
              TRUE,this, NULL, NULL, TO_CHAR, ANSI_YELLOW);
        } else if (typ == SPELL_PRAYER) {
          act("You give thanks as you utter the final word of your prayer.",
              FALSE,this, NULL, NULL, TO_CHAR, ANSI_GREEN);
          act("$n utters the final word of $s prayer and then is silent.",
              TRUE,this, NULL, NULL, TO_ROOM, ANSI_GREEN);
        }
        break;
    }
  } else {
    if (typ == SPELL_CASTER) {
      act("You silently say the final word of power.",
          TRUE,this, NULL, NULL, TO_CHAR, ANSI_CYAN);
    } else if (typ == SPELL_DANCER) {
      act("You silently chant your rada song.",
          TRUE,this, NULL, NULL, TO_CHAR, ANSI_RED);
      act("$n's eyes roll back into $s head.",
          TRUE,this, NULL, NULL, TO_ROOM, ANSI_RED);
    } else if (typ == SPELL_PRAYER) {
      act("You dwell silently on the final words of the prayer.",
          TRUE,this, NULL, NULL, TO_CHAR, ANSI_GREEN);
    }
  }
}

void TBeing::sendCastingMessages(bool limbs, bool silence, int round, skillUseTypeT typ, int counter)
{
  int roll, sendToRoom = TRUE;
  char buf[255];
  char buf2[255];
  TThing *temp = NULL;

  roll = ::number(0,max(2, 5 - round));
//  First, do gestures
//  if counter = 2 show all messages, or if roll do messages this time
  if (counter == 2 || roll) {

// determine if the caster will be using gestures or will be relying on wiz 
    if (limbs) {
      roll = ::number(0,4);
      switch (roll) {
        case 4:
          if (typ == SPELL_CASTER) {
            sprintf(buf, "You continue to trace a complex pattern in the air with your hand.");
            sprintf(buf2, "$n continues to trace a complex magic pattern in the air.");
          } else if (typ == SPELL_DANCER) {
            sprintf(buf, "You continue to sing your rada song to the ancestors.");
            sprintf(buf2, "$n continues to sing and dance for $s ancestors.");
          } else if (typ == SPELL_PRAYER) {
            sprintf(buf, "You continue to raise your symbol towards the sky.");
            sprintf(buf2, "$n continues to raise $s symbol towards the sky.");
          }
          break;
        case 3:
          if (typ == SPELL_CASTER) {
            sprintf(buf,"You trace another line of the magic rune in the air.");
            sprintf(buf2,"$n concentrates hard as $e makes a gesture in the air.");
          } else if (typ == SPELL_DANCER) {
            sprintf(buf,"You continue to dance and can feel the power of your ancestors.");
            sprintf(buf2,"$n continues to dance. You can feel a stange power eminating from $m.");
          } else if (typ == SPELL_PRAYER) {
            sprintf(buf, "Your symbol flares with heat as you raise it towards the heavens.");
            sprintf(buf2, "$n's symbol flares as it is raised towards the heavens.");
          }
          break;
        case 2:
          if (typ == SPELL_CASTER) {
            sprintf(buf, "You add another line to the magic rune you have been tracing.");
            sprintf(buf2, "$n adds another line to the magic rune $e has been tracing.");
          } else if (typ == SPELL_DANCER) {
            sprintf(buf, "You continue to sing your rada song to the ancestors.");
            sprintf(buf2, "$n sings a mystical lyric to $s ancestors.");
          } else if (typ == SPELL_PRAYER) {
            sprintf(buf, "Your symbol glows with power as you call on its strength.");
            sprintf(buf2, "$n's symbol glows with power as $e calls on its strength.");
          }
          break;
        case 1:
          if (typ == SPELL_CASTER) {
            sprintf(buf, "The air shimmers around your hand as it follows a magic line in the air.");
            sprintf(buf2, "The air shimmers around $n's hand as it follows a magic line in the air.");
          } else if (typ == SPELL_DANCER) {
            sprintf(buf, "The ground starts to move with the power of the loa.");
            sprintf(buf2, "A blood red mist starts to form around $n as $e continues to dance.");
          } else if (typ == SPELL_PRAYER) {
            sprintf(buf, "You raise your symbol towards the heavens looking for favor.");
            sprintf(buf2, "$n holds $s symbol towards the heaven looking for favor.");
          }
          break;
        default:
          if (typ == SPELL_CASTER) {
            sprintf(buf, "Your gesture continues to fill the air with part of a magic rune.");
            sprintf(buf2, "$n's gesture fills the air with part of a magic rune.");
          } else if (typ == SPELL_DANCER) {
            sprintf(buf, "Your ritualistic dance and rada fill you with power.");
            sprintf(buf2, "$n's ritualistic gestures fill the air with power.");
          } else if (typ == SPELL_PRAYER) {
            sprintf(buf, "Your symbol shines with a faint light as you raise it toward the heavenly sky.");
            sprintf(buf2, "$n's symbol shines with a faint light as it is raised toward the heavenly sky.");
          }
          break;
      }
    } else {
      if (typ == SPELL_CASTER) {
        sendToRoom = FALSE;
        sprintf(buf, "Your hands are still as you focus on casting your spell.")
;
      } else if (typ == SPELL_DANCER) {
        sendToRoom = FALSE;
        sprintf(buf, "Your skin crawls with the power of your rada.")
;
      } else if (typ == SPELL_PRAYER) {
        sprintf(buf, "Your symbol glows as you focus on finishing your prayer."
);
        sprintf(buf2, "$n's symbol glows as $e focuses on finishing $s prayer.");
      }
    }
// First do the character
    if (desc) {
      if (IS_SET(desc->autobits, AUTO_NOSPELL)) {
      } else if ((!(::number(0,1))) && 
                 IS_SET(desc->autobits, AUTO_HALFSPELL)) {
      } else {
        act(buf, TRUE,this, NULL, NULL, TO_CHAR, ANSI_GREEN);
      }
    }
// Then to room
    if (sendToRoom && roomp) {
      for (temp = getStuff(); temp; temp = temp->nextThing) {
        if (!dynamic_cast<TBeing *>(temp))
          continue;
        if (!temp->desc)
          continue;
        if (temp == this)
          continue;
        if (IS_SET(temp->desc->autobits, AUTO_NOSPELL)) {
// no text sent
        } else if ((!(::number(0,1))) && 
                   IS_SET(temp->desc->autobits, AUTO_HALFSPELL)) {
// no text sent
        } else {
          act(buf2, TRUE,this, NULL, temp, TO_VICT, ANSI_GREEN);
        }
      }
    }
//      act(buf2, TRUE,this, NULL, NULL, TO_ROOM, ANSI_GREEN);

  }

  roll = ::number(0,max(2, 5 - round));
//  Next, do verbal
//  if counter = 2 show all messages, or if roll do verbal time
  if (counter == 2 || roll) {
    sendToRoom = TRUE; // reset this to true to make sure to room
// determine if the caster will be using verbal or will be relying on wiz
    if (silence) {
      roll = ::number(0,4);
      switch (roll) {
        case 4:
          if (typ == SPELL_CASTER) {
           sprintf(buf, "You lyrically continue to chant the words of a gentle incantation.");
           sprintf(buf2, "$n lyrically continues to chant the words of a gentle incantation.");
          } else if (typ == SPELL_DANCER) {
           sprintf(buf, "You lyrically continue to call upon the ancestors for power through your rada song.");
           sprintf(buf2, "$n lyrically continues to call upon $s ancestors.");
          } else if (typ == SPELL_PRAYER) {
            sprintf(buf, "You softly say the familar words of your prayer.");
            sprintf(buf2, "$n softly says the familar word of $s prayer.");
          }
          break;
        case 3:
          if (typ == SPELL_CASTER) {
            sprintf(buf, "You continue to utter the words needed to complete your spell.");
            sprintf(buf2, "$n continues to utter the words needed to complete $s spell.");
          } else if (typ == SPELL_DANCER) {
            sprintf(buf, "You continue to sing the song needed to complete the ritual.");
            sprintf(buf2, "$n continues to sing to complete $s ritual.");
          } else if (typ == SPELL_PRAYER) {
            sprintf(buf, "You continue to utter the words needed to complete your prayer.");
            sprintf(buf2, "$n continues to utter the words needed to complete $s prayer.");
          }
          break;
        case 2:
          if (typ == SPELL_CASTER) {
            sprintf(buf, "Your words form the pattern calling on the magic to be unleashed.");
            sprintf(buf2, "$n words form a pattern calling on the magic to be released.");
          } else if (typ == SPELL_DANCER) {
            sprintf(buf, "Your song calls upon the ancestors to release their powers.");
            sprintf(buf2, "$n song calls upon $s ancestors to release their power.");
          } else if (typ == SPELL_PRAYER) {
            sprintf(buf, "Your voice forms the historic words of supplication and entreaty.");
            sprintf(buf2, "$n gives voice to the historic words of supplication and entreaty.");
          }
          break;
        case 1:
          if (typ == SPELL_CASTER) {
            sprintf(buf, "Your voice calls on the primordal forms and draws forth the primal magic.");
            sprintf(buf2, "$n's voice calls on the primordal forms and draws forth the primal magic.");
          } else if (typ == SPELL_DANCER) {
            sprintf(buf, "Your voice calls the ancestors and draws their power forth.");
            sprintf(buf2, "$n's voice calls to $s ancestors to do $s bidding.");
          } else if (typ == SPELL_PRAYER) {
            sprintf(buf, "Your voice continues towards the culmination of your prayer.");
            sprintf(buf2, "$n's voice is heard clearly echoing the words of prayer.");
          }
          break;
        default:
          if (typ == SPELL_CASTER) {
            sprintf(buf, "Your voice forms the words continuing the incantation.");
            sprintf(buf2, "$n's voice can be heard chanting the words of the incantation.");
          } else if (typ == SPELL_DANCER) {
            sprintf(buf, "Your voice forms the words continuing the invokation.");
            sprintf(buf2, "$n's voice can be heard singing the words of ancestoral power.");
          } else if (typ == SPELL_PRAYER) {
            sprintf(buf, "Your faith is funnelled into the soft words of prayer you chant.");
            sprintf(buf2, "$n chants more of the simple words of prayer and supplication.");
          }
          break;
      }
    } else {
      sendToRoom = FALSE; // neither of these have a to room
      if (typ == SPELL_CASTER) {
        sprintf(buf, "Your voice is silent as you focus on casting your spell.");
      } else if (typ == SPELL_DANCER) {
        sprintf(buf, "Your voice is silent as you focus on your invokation.");
      } else if (typ == SPELL_PRAYER) {
        sprintf(buf, "Your voice is silent as you focus on finishing your prayer.");
      }
    }
// First do the character
    if (desc) {
      if (IS_SET(desc->autobits, AUTO_NOSPELL)) {
      } else if ((!(::number(0,1))) &&
                 IS_SET(desc->autobits, AUTO_HALFSPELL)) {
      } else {
        act(buf, TRUE,this, NULL, NULL, TO_CHAR, ANSI_GREEN);
      }
    }
// Then to room
    if (sendToRoom && roomp) {
      for (temp = getStuff(); temp; temp = temp->nextThing) {
        if (!dynamic_cast<TBeing *>(temp))
          continue;
        if (!temp->desc)
          continue;
        if (temp == this)
          continue;
        if (IS_SET(temp->desc->autobits, AUTO_NOSPELL)) {
// dont send text
        } else if ((!(::number(0,1))) && 
                   IS_SET(temp->desc->autobits, AUTO_HALFSPELL)) {
// 50% dont send text
        } else {
          act(buf2, TRUE,this, NULL, temp, TO_VICT, ANSI_GREEN);
        }
      }
    }
//      act(buf2, TRUE,this, NULL, NULL, TO_ROOM, ANSI_GREEN);
  }
}

bool TBeing::castDenyCommand(cmdTypeT cmd)
{
  switch (cmd) {
    case CMD_SAY:
    case CMD_SAY2:
    case CMD_GLANCE:
    case CMD_TELL:
    case CMD_SHOUT:
    case CMD_WEATHER:
    case CMD_INVENTORY:
    case CMD_EQUIPMENT:
    case CMD_SMILE:
    case CMD_SHAKE:
    case CMD_NOD:
      return TRUE;
    default:
      return FALSE;
  }
}

bool TBeing::castAllowCommand(cmdTypeT cmd)
{
  switch (cmd) {
    case CMD_CONTINUE:
    case CMD_WIZLIST:
    case CMD_LOOK:
    case CMD_GLANCE:
    case CMD_TIME:
    case CMD_SCORE:
    case CMD_TROPHY:
    case CMD_HELP:
    case CMD_WHO:
    case CMD_NEWS:
    case CMD_SAVE:
    case CMD_IDEA:
    case CMD_TYPO:
    case CMD_BUG:
    case CMD_LEVELS:
    case CMD_ATTRIBUTE:
    case CMD_WORLD:
    case CMD_CLS:
    case CMD_PROMPT:
    case CMD_ALIAS:
    case CMD_CLEAR:
    case CMD_HISTORY:
    case CMD_COLOR:
    case CMD_MOTD:
    case CMD_TITLE:
    case CMD_PRACTICE:
    case CMD_NOSHOUT:
    case CMD_DESCRIPTION:
    case CMD_LIST:  // for list faction
    case CMD_ATTACK:
    case CMD_GROUP:
      return TRUE;
    default:
      return FALSE;
  }
}










