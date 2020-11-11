// discipline.cc

#include <stdio.h>
#include <vector>
#include <cmath>

#include "extern.h"
#include "room.h"
#include "being.h"
#include "monster.h"
#include "disease.h"
#include "obj_component.h"
#include "combat.h"
#include "disc_mage.h"
#include "person.h"
#include "disc_cleric.h"
#include "disc_soldiering.h"
#include "disc_blacksmithing.h"
#include "disc_deikhan_martial.h"
#include "disc_deikhan_guardian.h"
#include "disc_deikhan_absolution.h"
#include "disc_deikhan_vengeance.h"
#include "disc_defense.h"
#include "disc_offense.h"
#include "disc_mounted.h"
#include "disc_monk.h"
#include "disc_iron_body.h"
#include "disc_meditation.h"
#include "disc_leverage.h"
#include "disc_mindbody.h"
#include "disc_fattacks.h"
#include "disc_plants.h"
#include "disc_shaman_alchemy.h"
#include "disc_shaman_frog.h"
#include "disc_shaman_control.h"
#include "disc_shaman_spider.h"
#include "disc_shaman_skunk.h"
#include "disc_shaman_armadillo.h"
#include "disc_shaman_healing.h"
#include "disc_ritualism.h"
#include "disc_thief.h"
#include "disc_thief_fight.h"
#include "disc_poisons.h"
#include "disc_traps.h"
#include "disc_stealth.h"
#include "disc_air.h"
#include "disc_alchemy.h"
#include "disc_earth.h"
#include "disc_fire.h"
#include "disc_sorcery.h"
#include "disc_spirit.h"
#include "disc_water.h"
#include "disc_wrath.h"
#include "disc_shaman.h"
#include "disc_aegis.h"
#include "disc_afflictions.h"
#include "disc_cures.h"
#include "disc_hand_of_god.h"
#include "disc_ranger.h"
#include "disc_deikhan.h"
#include "disc_looting.h"
#include "disc_murder.h"
#include "disc_dueling.h"
#include "disc_warrior.h"
#include "disc_brawling.h"
#include "disc_adventuring.h"
#include "disc_combat.h"
#include "disc_wizardry.h"
#include "disc_lore.h"
#include "disc_theology.h"
#include "disc_faith.h"
#include "disc_slash.h"
#include "disc_blunt.h"
#include "disc_pierce.h"
#include "disc_ranged.h"
#include "disc_barehand.h"
#include "disc_animal.h"
#include "disc_nature.h"
#include "disc_psionics.h"
#include "disc_adv_adventuring.h"
#include "spelltask.h"
#include "obj_symbol.h"
#include "disc_commoner.h"
#include "stats.h"

#define DISC_DEBUG  0

static bool enforceVerbal(TBeing *ch, spellNumT spell)
{
  if (!IS_SET(discArray[spell]->comp_types, COMP_VERBAL))
    return TRUE;

  if (ch->isPc()) {
    if (!canDoVerbal(ch) ) {
      if (ch->hasClass(CLASS_MAGE)) {
	if (ch->getWizardryLevel() >= WIZ_LEV_NO_MANTRA) {
	  act("Your skill at wizardry allows you to merely think the incantation.",TRUE,ch,0,0,TO_CHAR);
	  return TRUE;
	}
      } else {
	if (ch->getRitualismLevel() >= RIT_LEV_NO_MANTRA) {
	  act("Your superior channel to the ancestors enables you to conduct the ritual in silence.",TRUE,ch,0,0,TO_CHAR);
	  return TRUE;
	}
      }
      act("$n opens $s mouth as if to say something.", TRUE, ch, 0, 0, TO_ROOM);
      ch->sendTo("You are unable to chant the incantation!\n\r");
      return FALSE;
    }
    if (ch->hasClass(CLASS_MAGE)) {
      if (ch->getWizardryLevel() >= WIZ_LEV_NO_MANTRA) {
	act("$n begins to chant a mysterious and melodic incantation.", TRUE, ch, 0, 0, TO_ROOM, ANSI_CYAN);
	act("Although you no longer need to, you begin an incantation to facilitate your spell.", TRUE, ch, 0, 0, TO_CHAR, ANSI_CYAN);
	return TRUE;
      } else {
	act("$n begins to chant a mysterious and melodic incantation.", TRUE, ch, 0, 0, TO_ROOM, ANSI_CYAN);
	act("You begin to chant a mysterious and melodic incantation.", TRUE, ch, 0, 0, TO_CHAR, ANSI_CYAN);
	return TRUE;
      } 
    } else {
      if (ch->getRitualismLevel() >= RIT_LEV_NO_MANTRA) {
	act("$n begins to dance and sing in an unfamiliar tongue.", TRUE, ch, 0, 0, TO_ROOM, ANSI_RED);
	act("You begin the rada song in the ancient tongue.", TRUE, ch, 0, 0, TO_CHAR, ANSI_RED);
	return TRUE;
      } else {
	act("$n begins to dance and sing in an unfamiliar tongue.", TRUE, ch, 0, 0, TO_ROOM, ANSI_RED);
	act("You begin to dance and sing your rada in the ancient tongue.", TRUE, ch, 0, 0, TO_CHAR, ANSI_RED);
	return TRUE;
      }
    }
  } else 
    return TRUE;
}

// returns TRUE if they can use their hands or don't need to
static bool enforceGestural(TBeing *ch, spellNumT spell)
{
  TThing *sec_obj, *prim_obj;
  int sec_okay, prim_okay, sec_usable, prim_usable;
  char buf[256], buf2[40], msg[512];
  int num = ::number(1,100);

  if (ch->isImmortal())
    return TRUE;

  if (!IS_SET(discArray[spell]->comp_types, COMP_GESTURAL))
    return TRUE;

  if (ch->hasClass(CLASS_MAGE)) {
    if (ch->getWizardryLevel() < WIZ_LEV_NO_GESTURES) {
      //////////////////////////////////////////////////////////////
      // wizardry level still requries gestures, make generic checks
      //////////////////////////////////////////////////////////////
      if (ch->isPc() && (!ch->hasHands() || ch->eitherArmHurt())) {
	act("You cannot perform the ritual's gestures without arms and hands!", FALSE, ch, NULL, NULL, TO_CHAR);
	act("Nothing seems to happen.", FALSE, ch, NULL, NULL, TO_ROOM);
	return FALSE;
      }
      ///////////////////////////////////////////////////////////////
      // min position in discArray defines absolute minimum level
      // however, check some positions to see if restricted movement 
      // affects ability to make gestures
      // higher nums are more likely to cause restriction
      ///////////////////////////////////////////////////////////////
      ///////////////////////////////////////////////////////////
      // sit, crawl use normal num.  raise it further for resting
      ///////////////////////////////////////////////////////////
      if (ch->getPosition() == POSITION_RESTING)
	num += 4 * num / 10;
      
      if ((ch->getPosition() == POSITION_RESTING || ch->getPosition() == POSITION_SITTING || ch->getPosition() == POSITION_CRAWLING) && (num > ch->getSkillValue(SKILL_WIZARDRY))) {
	////////////////////////////////////////////////////////////
	// we know that wizradry is < 60 from getWizardryLevel check
	////////////////////////////////////////////////////////////
	ch->sendTo(format("Restricted movement while %s causes you to mess up the ritual's gestures.\n\r") % sstring(position_types[ch->getPosition()]).uncap());
	act("Nothing seems to happen.", FALSE, ch, NULL, NULL, TO_ROOM);
	return FALSE;
      }
    }
  } else {
    if (ch->getRitualismLevel() < RIT_LEV_NO_GESTURES) {
      //////////////////////////////////////////////////////////////
      // wizardry level still requries gestures, make generic checks
      //////////////////////////////////////////////////////////////
      if (ch->isPc() && (!ch->hasHands() || ch->eitherArmHurt())) {
	act("You cannot invoke the ritual without arms and hands!",  FALSE, ch, NULL, NULL, TO_CHAR);
	act("Nothing seems to happen.", FALSE, ch, NULL, NULL, TO_ROOM);
	return FALSE;
      }
      ///////////////////////////////////////////////////////////////
      // min position in discArray defines absolute minimum level
      // however, check some positions to see if restricted movement 
      // affects ability to make gestures
      // higher nums are more likely to cause restriction
      ///////////////////////////////////////////////////////////////
      ///////////////////////////////////////////////////////////
      // sit, crawl use normal num.  raise it further for resting
      ///////////////////////////////////////////////////////////
      if (ch->getPosition() == POSITION_RESTING)
	num += 4 * num / 10;
      
      if ((ch->getPosition() == POSITION_RESTING || ch->getPosition() == POSITION_SITTING || ch->getPosition() == POSITION_CRAWLING) && (num > ch->getSkillValue(SKILL_RITUALISM))) {
	////////////////////////////////////////////////////////////////
	// we know that ritualism is < 60 from getRitualismLevel check
	///////////////////////////////////////////////////////////////
	ch->sendTo(format("Restricted movement while %s causes you to mess up the ritual's gestures.\n\r") % sstring(position_types[ch->getPosition()]).uncap());
	act("Nothing seems to happen.", FALSE, ch, NULL, NULL, TO_ROOM);
	return FALSE;
      }
    }
  }
  if (!ch->isPc()) {
    if (ch->hasClass(CLASS_MAGE)) {
      if (ch->getWizardryLevel() >= WIZ_LEV_NO_GESTURES) {
	act("You concentrate intently upon the magical task at hand...", FALSE, ch, NULL, NULL, TO_CHAR, ANSI_CYAN);
	act("$n stares off into space, concentrating on something...", FALSE, ch, NULL, NULL, TO_ROOM, ANSI_CYAN);
	return TRUE;
      }
    } else {
      if (ch->getRitualismLevel() >= RIT_LEV_NO_GESTURES) {
	act("You focus your thoughts upon the ancestors and their swift movements...", FALSE, ch, NULL, NULL, TO_CHAR, ANSI_RED);
	act("$n concentrates deeply upon $s task...", FALSE, ch, NULL, NULL, TO_ROOM, ANSI_RED);
	return TRUE;
      }
    }
    if (ch->hasClass(CLASS_MAGE)) {
      if (ch->hasHands()) {
	sprintf(msg, "$n performs magical gestures with both of $s hands.");
	act(msg, FALSE, ch, NULL, NULL, TO_ROOM, ANSI_PURPLE);
	sprintf(msg, "You perform magical gestures with both of your hands.");
	act(msg, FALSE, ch, NULL, NULL, TO_CHAR, ANSI_PURPLE);
	return TRUE;
      } else {
	act("You hop and wiggle about while creating the magical runes in the air...", FALSE, ch, NULL, NULL, TO_CHAR, ANSI_CYAN);
	act("$n hops and wiggles about while creating the magical runes in the air...", FALSE, ch, NULL, NULL, TO_ROOM, ANSI_CYAN);
	return TRUE;
      }
    } else {
      if (ch->hasHands()) {
	sprintf(msg, "$n performs ritualistic gestures with $s hands.");
	act(msg, FALSE, ch, NULL, NULL, TO_ROOM, ANSI_RED);
	sprintf(msg, "You perform ritualistic gestures with both hands.");
	act(msg, FALSE, ch, NULL, NULL, TO_CHAR, ANSI_RED);
	return TRUE;
      } else {
	act("You hop and wiggle about while creating the magical runes in the air...", FALSE, ch, NULL, NULL, TO_CHAR, ANSI_CYAN);
	act("$n hops and wiggles about while creating the magical runes in the air...", FALSE, ch, NULL, NULL, TO_ROOM, ANSI_CYAN);
	return TRUE;
      }
    }
    return TRUE;
  } else {
    /////////
    // PCs
    /////////
    sec_obj = ch->heldInSecHand();
    sec_usable = ch->canUseArm(HAND_SECONDARY);
    sec_okay = ((!sec_obj || sec_obj->allowsCast()) && sec_usable);
    prim_obj = ch->heldInPrimHand();
    prim_usable = ch->canUseArm(HAND_PRIMARY);
    prim_okay = ((!prim_obj || prim_obj->allowsCast()) && prim_usable);

    if (ch->hasClass(CLASS_MAGE)) {
      if (ch->getWizardryLevel() >= WIZ_LEV_COMP_EITHER) {
	if (sec_okay || prim_okay) {
	  if (sec_okay)
	    sprintf(buf, "%s", (ch->isRightHanded() ? "left" : "right"));
	  else
	    sprintf(buf, "%s", (ch->isRightHanded() ? "right" : "left"));
	  sprintf(msg, "$n traces a magical rune in the air with $s %s hand.", buf);
	  act(msg, FALSE, ch, NULL, NULL, TO_ROOM, ANSI_PURPLE);
	  if (ch->getWizardryLevel() >= WIZ_LEV_NO_GESTURES) {
	    sprintf(msg, "While not absolutely necessary, you trace a rune with your %s hand to facilitate your spell in forming.", buf);
	    act(msg, FALSE, ch, NULL, NULL, TO_CHAR, ANSI_PURPLE);
	  } else {
	    sprintf(msg, "You trace a magical rune in the air with your %s hand.", buf);
	    act(msg, FALSE, ch, NULL, NULL, TO_CHAR, ANSI_PURPLE);
	  }
	  return TRUE;
	} else {
	  if (ch->getWizardryLevel() >= WIZ_LEV_NO_GESTURES) {
	    act("You concentrate intently upon the magical task at hand...", FALSE, ch, NULL, NULL, TO_CHAR, ANSI_CYAN);
	    act("$n stares off into space, concentrating on something...", FALSE, ch, NULL, NULL, TO_ROOM, ANSI_CYAN);
	    return TRUE;
	  }
	  act("Nothing seems to happen.", FALSE, ch, NULL, NULL, TO_ROOM);
	  act("You must have one hand free and usable to perform the ritual's gestures!", FALSE, ch, NULL, NULL, TO_CHAR);
	  return FALSE;
	}
      }
    } else {
      if (ch->getRitualismLevel() >= RIT_LEV_COMP_EITHER) {
	if (sec_okay || prim_okay) {
	  if (sec_okay)
	    sprintf(buf, "%s", (ch->isRightHanded() ? "left" : "right"));
	  else
	    sprintf(buf, "%s", (ch->isRightHanded() ? "right" : "left"));
	  sprintf(msg, "$n stretches $s %s arm skyward as $e asks for ritual blessing.", buf);
	  act(msg, FALSE, ch, NULL, NULL, TO_ROOM, ANSI_RED);
	  if (ch->getRitualismLevel() >= RIT_LEV_NO_GESTURES) {
	    sprintf(msg, "While not absolutely necessary, you stretch your %s arm to the sky for the blessings of the loa.", buf);
	    act(msg, FALSE, ch, NULL, NULL, TO_CHAR, ANSI_RED);
	  } else {
	    sprintf(msg, "You thrust your %s arm skyward as a symbol of ritualistic power.", buf);
	    act(msg, FALSE, ch, NULL, NULL, TO_CHAR, ANSI_RED);
	  }
	  return TRUE;
	} else {
	  if (ch->getRitualismLevel() >= RIT_LEV_NO_GESTURES) {
	    act("You chant the ancient rada song offering precious lifeforce to the spirits.", FALSE, ch, NULL, NULL, TO_CHAR, ANSI_YELLOW);
	    act("$n groans as $s eyes roll back into $s head in deep concentration.", FALSE, ch, NULL, NULL, TO_ROOM, ANSI_YELLOW);
	    return TRUE;
	  }
	  act("Nothing seems to happen.", FALSE, ch, NULL, NULL, TO_ROOM);
	  act("You must have one hand free and usable to perform the ritual's gestures!", FALSE, ch, NULL, NULL, TO_CHAR);
	  return FALSE;
	}
      }
    }
    if (ch->hasClass(CLASS_MAGE)) {
      if (prim_okay && sec_okay) {
	sprintf(msg, "$n traces a magical rune in the air with $s hands.");
	act(msg, FALSE, ch, NULL, NULL, TO_ROOM, ANSI_PURPLE);
	sprintf(msg, "You trace a magical rune in the air with your hands.");
	act(msg, FALSE, ch, NULL, NULL, TO_CHAR, ANSI_PURPLE);
	return TRUE;
      } else {
	sprintf(buf, "%s",  (ch->isRightHanded() ? "right" : "left"));
	sprintf(buf2, "%s", (ch->isRightHanded() ? "left" : "right"));
	
	act("Nothing seems to happen.", FALSE, ch, NULL, NULL, TO_ROOM);
	
	if (ch->getWizardryLevel() < WIZ_LEV_COMP_EITHER_OTHER_FREE) {
	  if (!(IS_SET(discArray[spell]->comp_types, COMP_MATERIAL))) {
	    sprintf(msg, "Both of your hands must be free and usable to perform the ritual's gestures!");
	  } else {
	    if (!prim_okay && !sec_okay) {
	      sprintf(msg, "Your component must be in your %s hand and your %s must be free and usable to perform the ritual's gestures!", buf, buf2);
	    } else if (!prim_okay) {
	      sprintf(msg, "Your component must be in your %s hand for you to be able to perform the ritual's gestures!", buf);
	    } else {
	      sprintf(msg, "Your %s hand must be free and usable to perform the ritual's gestures!", buf2);
	    }
	  }
	} else {
	  if (!prim_okay && !sec_okay) {
	    sprintf(msg, "Your %s hand must hold the component and your %s must be free to perform the ritual!", buf2, buf);
	  } else if (!prim_okay) {
	    sprintf(msg, "Your %s hand must be holding the component for you to properly perform this spell!", buf);
	  } else {
	    sprintf(msg, "Your %s hand must be free and usable to perform the ritual's gestures!", buf2);
	  }
	}
	act(msg, FALSE, ch, NULL, NULL, TO_CHAR);
	return FALSE;
      }
    } else {
      if (prim_okay && sec_okay) {
	sprintf(msg, "$n stretches $s arms skyward as $e begs $s ancestors for a blessing.");
	act(msg, FALSE, ch, NULL, NULL, TO_ROOM, ANSI_RED);
	sprintf(msg, "You stretch your arms to the skies and beg the ancestors for a mighty blessing.");
	act(msg, FALSE, ch, NULL, NULL, TO_CHAR, ANSI_RED);
	return TRUE;
      } else {
	sprintf(buf, "%s",  (ch->isRightHanded() ? "right" : "left"));
	sprintf(buf2, "%s", (ch->isRightHanded() ? "left" : "right"));
	act("Nothing seems to happen.", FALSE, ch, NULL, NULL, TO_ROOM);
	
	if (ch->getRitualismLevel() < RIT_LEV_COMP_EITHER_OTHER_FREE) {
	  if (!(IS_SET(discArray[spell]->comp_types, COMP_MATERIAL))) {
	    sprintf(msg, "Both of your hands must be free and usable to perform the ritual's gestures!");
	  } else {
	    if (!prim_okay && !sec_okay) {
	      sprintf(msg, "Your component must be in your %s hand and your %s must be free and usable to perform the ritual's gestures!", buf, buf2);
	    } else if (!prim_okay) {
	      sprintf(msg, "Your component must be in your %s hand for you to be able to perform the ritual's gestures!", buf);
	    } else {
	      sprintf(msg, "Your %s hand must be free and usable to perform the ritual's gestures!", buf2);
	    }
	  }
	} else {
	  if (!prim_okay && !sec_okay) {
	    sprintf(msg, "Your %s hand must hold the component and your %s must be free to perform the ritual!", buf2, buf);
	  } else if (!prim_okay) {
	    sprintf(msg, "Your %s hand must be holding the component for you to properly perform this spell!", buf);
	  } else {
	    sprintf(msg, "Your %s hand must be free and usable to perform the ritual's gestures!", buf2);
	  }
	}
	act(msg, FALSE, ch, NULL, NULL, TO_CHAR);
	return FALSE;
      }
    }
    return TRUE;
  }
}

bool bPassMageChecks(TBeing * caster, spellNumT spell, TThing *target)
{
  if (!caster->getSkillLevel(spell)) {
    // probably an immort with improper class
    caster->sendTo("You need to have the appropriate level AND class for this to work.\n\r");
    return FALSE;
  }

  if (caster->isImmortal() && caster->isPlayerAction(PLR_NOHASSLE))
    return TRUE;

  if (!enforceVerbal(caster, spell))
    return FALSE;

  if (!enforceGestural(caster, spell))
    return FALSE;

  // if spell uses component, check for it
  if (IS_SET(discArray[spell]->comp_types, COMP_MATERIAL)) {
    TBeing * vict = dynamic_cast<TBeing *>(target);
    TObj * obj = dynamic_cast<TObj *>(target);
    if (vict && !caster->useComponent(caster->findComponent(spell), vict, CHECK_ONLY_YES))
      return FALSE;
    if (obj && !caster->useComponentObj(caster->findComponent(spell), obj, CHECK_ONLY_YES))
      return FALSE;
  }

  return TRUE;
}

bool bPassShamanChecks(TBeing * caster, spellNumT spell, TThing *target)
{
  if (!caster->getSkillLevel(spell)) {
    // probably an immort with improper class
    caster->sendTo("You need to have the appropriate level AND class for this to work.\n\r");
    return FALSE;
  }

  if (caster->isImmortal() && caster->isPlayerAction(PLR_NOHASSLE))
    return TRUE;

  if (!enforceVerbal(caster, spell))
    return FALSE;

  if (!enforceGestural(caster, spell))
    return FALSE;

  // if spell uses component, check for it
  if (IS_SET(discArray[spell]->comp_types, COMP_MATERIAL)) {
    TBeing * vict = dynamic_cast<TBeing *>(target);
    TObj * obj = dynamic_cast<TObj *>(target);
    if (vict && !caster->useComponent(caster->findComponent(spell), vict, CHECK_ONLY_YES))
      return FALSE;
    if (obj && !caster->useComponentObj(caster->findComponent(spell), obj, CHECK_ONLY_YES))
      return FALSE;
  }

  return TRUE;
}

static bool enforcePrayer(TBeing *ch, spellNumT spell)
{
  if (!ch)
    return FALSE;

  if (ch->isImmortal())
    return TRUE;

  if (!IS_SET(discArray[spell]->comp_types, COMP_VERBAL))
    return TRUE;

  if (ch->isPc()) {

    if (!canDoVerbal(ch) ) {
      if (ch->getDevotionLevel() >= DEV_LEV_NO_MANTRA) {
        ch->sendTo("Your devotion allows you to merely think the prayer.\n\r");
        act("$n's face turns solemn as $e concentrates on something.",TRUE,ch,0,0,TO_ROOM);
        return TRUE;
      }
      act("$n opens $s mouth as if to say something.", TRUE, ch, 0, 0, TO_ROOM);
      ch->sendTo("You are unable to recite the sacred words!\n\r");
      return FALSE;
    }
    act("$n chants mysterious and sacred words.", TRUE, ch, 0, NULL, TO_ROOM, ANSI_CYAN);
    act("You beseech $d for aid.", TRUE, ch, NULL, NULL, TO_CHAR, ANSI_CYAN);
    return TRUE;
  } else 
    return TRUE;
}

static bool requireHolySym(const TBeing *ch)
{
  if (ch->isImmortal() || dynamic_cast<const TMonster *>(ch))
    return FALSE;

  if (ch->hasClass(CLASS_CLERIC) || ch->hasClass(CLASS_DEIKHAN))
    return TRUE;

  return FALSE;
}

//static
bool enforceHolySym(const TBeing *ch, spellNumT spell, bool checkDamage)
{
  // get the rough level of the spell being cast
  int level = ch->GetMaxLevel();
  int minl, maxl;
  getSkillLevelRange(spell, minl, maxl, 0);
  level = min(max(level, minl), maxl);

  int sym_stress = level * level;

  char buf[128];

  if (requireHolySym(ch)) {
    TSymbol * holy = ch->findHolySym(SILENT_NO);
    if (holy) {
      int curr_strength = holy->getSymbolCurStrength();
      int orig_strength = holy->getSymbolMaxStrength();

      // adjust the damage if overpowering the symbol
      float sym_level = holy->getSymbolLevel();
      sstring sb;  // damage sstring
      if (level >= (sym_level+15)) {
        sym_stress *= 8;
        sb = "$p suffers TREMENDOUS structural damage from the stress!";
      } else if (level >= (sym_level+10)) {
        sym_stress *= 4;
        sb = "$p suffers massive structural damage from the stress!";
      } else if (level >= (sym_level+5)) {
        sym_stress *= 2;
        sb = "$p suffers intense structural damage from the stress!";
      } else if (level < (sym_level-10)) {
        sb = "$p suffers some light damage from the stress!";
      } else {
        sb = "$p suffers structural damage from the stress!";
      }
    
      if (sym_stress > orig_strength) {
        act("$n's $o starts to glow with power but the glow fades away!", 
            FALSE, ch, holy, NULL, TO_ROOM);
        act("Your $o starts to glow with power but the glow fades away!", 
            FALSE, ch, holy, NULL, TO_CHAR);
        act("You sense that $p is not strong enough for that prayer.",
            FALSE, ch, holy, NULL, TO_CHAR);
        return FALSE;
      }

      if (curr_strength == orig_strength) {
          holy->setSymbolCurStrength((orig_strength - 1));
          act("The deities favor $p and accept it as a symbol of prayer.",
              FALSE, ch, holy, NULL, TO_CHAR);
      }
      if (!checkDamage) {
        act("$n's $o glows with power!", FALSE, ch, holy, NULL, TO_ROOM);
        act("Your $o glows with power!", FALSE, ch, holy, NULL, TO_CHAR);
        return TRUE;   // has holy and was a strong enough symbol
      } else {
        if (sym_stress >= curr_strength) {
          sprintf(buf, "$p %sshatters%s from the stress of the prayer.", ch->red(), ch->norm());
          act(buf, FALSE, ch, holy, NULL, TO_ROOM);
          act("$p shatters from the stress of the prayer!", 0, ch, holy, 0, TO_CHAR, ANSI_RED);
          delete holy;
          holy = NULL;
          return FALSE;
        }
      
        holy->addToSymbolCurStrength(-sym_stress);
        act(sb, false, ch, holy, NULL, TO_CHAR);
        return TRUE;
      }
    } else {
      return FALSE;    // requires symbol, user didn't have one
    }
  } 
  return TRUE;         // doesn't require holy symbol
}

bool bPassClericChecks(TBeing * caster, spellNumT spell)
{
  if (!caster->getSkillLevel(spell)) {
    // probably an immort with improper class
    caster->sendTo("You need to have the appropriate level AND class for this to work.\n\r");
    return FALSE;
  }

  if (caster->isImmortal() && caster->isPlayerAction(PLR_NOHASSLE))
    return TRUE;

  if (!enforcePrayer(caster, spell))
    return FALSE;

  if (!enforceHolySym(caster,spell, false))
    return FALSE;

  return TRUE;
}

int CDiscipline::getDoLearnedness() const
{
    return(uDoLearnedness);
}

int CDiscipline::getNatLearnedness() const
{
    return(uNatLearnedness);
}

int CDiscipline::getLearnedness() const
{
    return(uLearnedness);
}


struct cplist_pair {
    const unsigned short class_num;
    const spellNumT skill_num;
};
typedef std::vector<cplist_pair> cplist;

spellNumT pick_best_skill(const TBeing *caster, spellNumT skill_num, const cplist list) {
    int best_value = 0;
    for (auto pair : list) {
        if (!caster->hasClass(pair.class_num))
            continue;
        int this_value = caster->getSkillValue(pair.skill_num);
        if (this_value > best_value) {
            best_value = this_value;
            skill_num = pair.skill_num;
        }
    }
    return skill_num;
}

spellNumT TBeing::getSkillNum(spellNumT skill_num) const
{
    if ((skill_num < MIN_SPELL) || (skill_num >= MAX_SKILL)) {
        vlogf(LOG_BUG, format("Something is passing a bad skill number (%d) to getSkillNum for %s") %  skill_num % getName());
        return TYPE_UNDEFINED;
    }

    switch (skill_num) {
        case SKILL_KICK:
          if (hasClass(CLASS_MONK))
            return SKILL_KICK_MONK;
          else
            return pick_best_skill(this, skill_num, {
                {CLASS_WARRIOR, SKILL_KICK},
                {CLASS_THIEF, SKILL_KICK_THIEF}});
      
        case SKILL_RETREAT:
            return pick_best_skill(this, skill_num, {
                {CLASS_WARRIOR, SKILL_RETREAT},
                {CLASS_DEIKHAN, SKILL_RETREAT_DEIKHAN},
                {CLASS_THIEF, SKILL_RETREAT_THIEF},
                {CLASS_MONK, SKILL_RETREAT_MONK}});
        
        case SKILL_SWITCH_OPP:
            return pick_best_skill(this, skill_num, {
                {CLASS_WARRIOR, SKILL_SWITCH_OPP},
                {CLASS_DEIKHAN, SKILL_SWITCH_DEIKHAN},
                {CLASS_THIEF, SKILL_SWITCH_THIEF},
                {CLASS_MONK, SKILL_SWITCH_MONK}});
        
        case SKILL_DISARM:
            return pick_best_skill(this, skill_num, {
                {CLASS_WARRIOR, SKILL_DISARM},
                {CLASS_DEIKHAN, SKILL_DISARM_DEIKHAN},
                {CLASS_THIEF, SKILL_DISARM_THIEF},
                {CLASS_MONK, SKILL_DISARM_MONK}});
        
        case SKILL_BASH:
            return pick_best_skill(this, skill_num, {
                {CLASS_WARRIOR, SKILL_BASH},
                {CLASS_DEIKHAN, SKILL_BASH_DEIKHAN}});
        
        case SKILL_RESCUE:
            return pick_best_skill(this, skill_num, {
                {CLASS_WARRIOR, SKILL_RESCUE},
                {CLASS_DEIKHAN, SKILL_RESCUE_DEIKHAN}});
        
        case SKILL_DUAL_WIELD:
            return pick_best_skill(this, skill_num, {
                {CLASS_WARRIOR, SKILL_DUAL_WIELD},
                {CLASS_THIEF, SKILL_DUAL_WIELD_THIEF}});
        
        case SKILL_SHOVE:
            return pick_best_skill(this, skill_num, {
                {CLASS_WARRIOR, SKILL_SHOVE},
                {CLASS_DEIKHAN, SKILL_SHOVE_DEIKHAN}});
        
        case SPELL_SALVE:
            return pick_best_skill(this, skill_num, {
                {CLASS_CLERIC, SPELL_SALVE},
                {CLASS_DEIKHAN, SPELL_SALVE_DEIKHAN}});
        
        case SPELL_HEAL_LIGHT:
            return pick_best_skill(this, skill_num, {
                {CLASS_CLERIC, SPELL_HEAL_LIGHT},
                {CLASS_DEIKHAN, SPELL_HEAL_LIGHT_DEIKHAN}});
        
        case SPELL_HEAL_SERIOUS:
            return pick_best_skill(this, skill_num, {
                {CLASS_CLERIC, SPELL_HEAL_SERIOUS},
                {CLASS_DEIKHAN, SPELL_HEAL_SERIOUS_DEIKHAN}});
        
        case SPELL_HEAL_CRITICAL:
            return pick_best_skill(this, skill_num, {
                {CLASS_CLERIC, SPELL_HEAL_CRITICAL},
                {CLASS_DEIKHAN, SPELL_HEAL_CRITICAL_DEIKHAN}});
        
        case SPELL_NUMB:
            return pick_best_skill(this, skill_num, {
                {CLASS_CLERIC, SPELL_NUMB},
                {CLASS_DEIKHAN, SPELL_NUMB_DEIKHAN}});
        
        case SPELL_HARM_LIGHT:
            return pick_best_skill(this, skill_num, {
                {CLASS_CLERIC, SPELL_HARM_LIGHT},
                {CLASS_DEIKHAN, SPELL_HARM_LIGHT_DEIKHAN}});
        
        case SPELL_HARM_SERIOUS:
            return pick_best_skill(this, skill_num, {
                {CLASS_CLERIC, SPELL_HARM_SERIOUS},
                {CLASS_DEIKHAN, SPELL_HARM_SERIOUS_DEIKHAN}});
        
        case SPELL_HARM_CRITICAL:
            return pick_best_skill(this, skill_num, {
                {CLASS_CLERIC, SPELL_HARM_CRITICAL},
                {CLASS_DEIKHAN, SPELL_HARM_CRITICAL_DEIKHAN}});
        
        case SPELL_HARM:
            return pick_best_skill(this, skill_num, {
                {CLASS_CLERIC, SPELL_HARM},
                {CLASS_DEIKHAN, SPELL_HARM_DEIKHAN}});
        
        case SPELL_CURE_POISON:
            return pick_best_skill(this, skill_num, {
                {CLASS_CLERIC, SPELL_CURE_POISON},
                {CLASS_DEIKHAN, SPELL_CURE_POISON_DEIKHAN}});
        
        case SPELL_POISON:
            return pick_best_skill(this, skill_num, {
                {CLASS_CLERIC, SPELL_POISON},
                {CLASS_DEIKHAN, SPELL_POISON_DEIKHAN}});
        
        case SPELL_REFRESH:
            return pick_best_skill(this, skill_num, {
                {CLASS_CLERIC, SPELL_REFRESH},
                {CLASS_DEIKHAN, SPELL_REFRESH_DEIKHAN}});
        
        case SPELL_HEROES_FEAST:
            return pick_best_skill(this, skill_num, {
                {CLASS_CLERIC, SPELL_HEROES_FEAST},
                {CLASS_DEIKHAN, SPELL_HEROES_FEAST_DEIKHAN}});
        
        case SPELL_CREATE_FOOD:
            return pick_best_skill(this, skill_num, {
                {CLASS_CLERIC, SPELL_CREATE_FOOD},
                {CLASS_DEIKHAN, SPELL_CREATE_FOOD_DEIKHAN}});
        
        case SPELL_CREATE_WATER:
            return pick_best_skill(this, skill_num, {
                {CLASS_CLERIC, SPELL_CREATE_WATER},
                {CLASS_DEIKHAN, SPELL_CREATE_WATER_DEIKHAN}});
        
        case SPELL_ARMOR:
            return pick_best_skill(this, skill_num, {
                {CLASS_CLERIC, SPELL_ARMOR},
                {CLASS_DEIKHAN, SPELL_ARMOR_DEIKHAN}});
        
        case SPELL_BLESS:
            return pick_best_skill(this, skill_num, {
                {CLASS_CLERIC, SPELL_BLESS},
                {CLASS_DEIKHAN, SPELL_BLESS_DEIKHAN}});
        
        case SPELL_EXPEL:
            return pick_best_skill(this, skill_num, {
                {CLASS_CLERIC, SPELL_EXPEL},
                {CLASS_DEIKHAN, SPELL_EXPEL_DEIKHAN}});
        
        case SPELL_CLOT:
            return pick_best_skill(this, skill_num, {
                {CLASS_CLERIC, SPELL_CLOT},
                {CLASS_DEIKHAN, SPELL_CLOT_DEIKHAN}});
        
        case SPELL_STERILIZE:
            return pick_best_skill(this, skill_num, {
                {CLASS_CLERIC, SPELL_STERILIZE},
                {CLASS_DEIKHAN, SPELL_STERILIZE_DEIKHAN}});
        
        case SPELL_REMOVE_CURSE:
            return pick_best_skill(this, skill_num, {
                {CLASS_CLERIC, SPELL_REMOVE_CURSE},
                {CLASS_DEIKHAN, SPELL_REMOVE_CURSE_DEIKHAN}});
        
        case SPELL_CURSE:
            return pick_best_skill(this, skill_num, {
                {CLASS_CLERIC, SPELL_CURSE},
                {CLASS_DEIKHAN, SPELL_CURSE_DEIKHAN}});
        
        case SPELL_INFECT:
            return pick_best_skill(this, skill_num, {
                {CLASS_CLERIC, SPELL_INFECT},
                {CLASS_DEIKHAN, SPELL_INFECT_DEIKHAN}});
        
        case SPELL_CURE_DISEASE:
            return pick_best_skill(this, skill_num, {
                {CLASS_CLERIC, SPELL_CURE_DISEASE},
                {CLASS_DEIKHAN, SPELL_CURE_DISEASE_DEIKHAN}});
        
#if 0
        // unimplemented
        case SPELL_DETECT_POISON:
            return pick_best_skill(this, skill_num, {
                {CLASS_CLERIC, SPELL_DETECT_POISON},
                {CLASS_DEIKHAN, SPELL_DETECT_POISON_DEIKHAN}});
#endif
        default:
            return skill_num;
    };
}

bool TBeing::isValidDiscClass(discNumT discNum, int classNum, int indNum) 
{
  if (discNum >= MAX_DISCS)
    return FALSE;
  if (indNum) {
  }

  if ((discNames[discNum].class_num == 0)) {
    return TRUE;
  }
  if ((discNames[discNum].class_num == classNum)) {
    return TRUE;
  }

  return FALSE;
}

discNumT getDisciplineNumber(spellNumT spell_num, int class_num)
{
  mud_assert(spell_num >= MIN_SPELL && spell_num < MAX_SKILL, 
          "Bad skill in getDisciplineNumber: %d", spell_num);

  if (!class_num) {
    if(discArray[spell_num]) {
      return discArray[spell_num]->disc;
    } else {
      return DISC_NONE;
    }
  } else {
    return DISC_NONE;
  }
  return DISC_NONE;
}

void CDiscipline::setDoLearnedness(int uNewValue)
{
    uDoLearnedness = uNewValue;
}

void CDiscipline::setNatLearnedness(int uNewValue)
{
    if (uNewValue > MAX_DISC_LEARNEDNESS)
      uNatLearnedness = MAX_DISC_LEARNEDNESS;
    else
      uNatLearnedness = uNewValue;
}

void CDiscipline::setLearnedness(int uNewValue)
{
    if (uNewValue > MAX_DISC_LEARNEDNESS)
      uLearnedness = MAX_DISC_LEARNEDNESS;
    else 
      uLearnedness = uNewValue;
}

enum logSkillAttemptT {
  ATTEMPT_ADD_NORM,
  ATTEMPT_ADD_ENGAGE,
  ATTEMPT_REM_NORM,
  ATTEMPT_REM_ENGAGE
};

static void logSkillAttempts(const TBeing *caster, spellNumT spell, logSkillAttemptT type)
{
  int value;

  if (caster->desc) {
    if (discArray[spell]->minMana) {
      if (type == ATTEMPT_ADD_NORM) {
        caster->desc->session.spell_success_attempts++;
        caster->desc->career.spell_success_attempts++;
      } else if (type == ATTEMPT_REM_NORM) {
        caster->desc->session.spell_success_attempts--;
        caster->desc->career.spell_success_attempts--;
      }
    } else if (discArray[spell]->minLifeforce) {
      if (type == ATTEMPT_ADD_NORM) {
        caster->desc->session.spell_success_attempts++;
        caster->desc->career.spell_success_attempts++;
      } else if (type == ATTEMPT_REM_NORM) {
        caster->desc->session.spell_success_attempts--;
        caster->desc->career.spell_success_attempts--;
      }
    } else if (discArray[spell]->minPiety) {
      if (type == ATTEMPT_ADD_NORM) {
        caster->desc->session.prayer_success_attempts++;
        caster->desc->career.prayer_success_attempts++;
      } else if (type == ATTEMPT_REM_NORM) {
        caster->desc->session.prayer_success_attempts--;
        caster->desc->career.prayer_success_attempts--;
      }
    } else {
      if (type == ATTEMPT_ADD_NORM) {
        caster->desc->session.skill_success_attempts++;
        caster->desc->career.skill_success_attempts++;
      } else if (type == ATTEMPT_REM_NORM) {
        caster->desc->session.skill_success_attempts--;
        caster->desc->career.skill_success_attempts--;
      }
    }
  }

  value = caster->getSkillValue(spell);

  if (value < 0)
    return;

#if 0
// don't log if maxed?
  if (value >= MAX_SKILL_LEARNEDNESS)
    return; 
#endif

  // count the number of immortal uses
  if (caster->desc && caster->GetMaxLevel() > MAX_MORT) {
    switch (type) {
      case ATTEMPT_ADD_NORM:
        discArray[spell]->immUses++;
        discArray[spell]->immLevels += caster->getSkillLevel(spell);
        discArray[spell]->immLearned += (long) max(0, (int) caster->getSkillValue(spell));
        break;
      case ATTEMPT_REM_NORM:
        discArray[spell]->immUses--;
        discArray[spell]->immLevels -= caster->getSkillLevel(spell);
        discArray[spell]->immLearned -= (long) max(0, (int) caster->getSkillValue(spell));
        break;
      default:
        break;
    }
    return;
  } 

  // count mob uses
  if (!caster->desc || !caster->isPc()) {
    switch (type) {
      case ATTEMPT_ADD_NORM:
        discArray[spell]->mobUses++;
        discArray[spell]->mobLevels += caster->getSkillLevel(spell);
        discArray[spell]->mobLearned += (long) max(0, (int) caster->getSkillValue(spell));
        break;
      case ATTEMPT_REM_NORM:
        discArray[spell]->mobUses--;
        discArray[spell]->mobLevels -= caster->getSkillLevel(spell);
        discArray[spell]->mobLearned -= (long) max(0, (int) caster->getSkillValue(spell));
        break;
      default:
        break;
    }
    return;
  }

  // mortal PCs only get here

  switch (type) {
    case ATTEMPT_ADD_NORM:
      discArray[spell]->uses++;
      discArray[spell]->levels += caster->GetMaxLevel();
      discArray[spell]->learned += value;
      discArray[spell]->focusValue += caster->getStat(STAT_CURRENT, STAT_FOC);

      // log quality
      if (value <= 20) {
        discArray[spell]->newAttempts++;
      } else if (value <= 40) {
        discArray[spell]->lowAttempts++;
      } else if (value <= 60) {
        discArray[spell]->midAttempts++;
      } else if (value <= 80) {
        discArray[spell]->goodAttempts++;
      } else {
        discArray[spell]->highAttempts++;
      }

      break;
    case ATTEMPT_ADD_ENGAGE:
      discArray[spell]->engAttempts++;
      break;
    case ATTEMPT_REM_NORM:
      discArray[spell]->uses--;
      discArray[spell]->levels -= caster->GetMaxLevel();
      discArray[spell]->learned -= value;
      discArray[spell]->focusValue -= caster->getStat(STAT_CURRENT, STAT_FOC);

      if (value <= 20) {
        discArray[spell]->newAttempts--;
      } else if (value <= 40) {
        discArray[spell]->lowAttempts--;
      } else if (value <= 60) {
        discArray[spell]->midAttempts--;
      } else if (value <= 80) {
        discArray[spell]->goodAttempts--;
      } else {
        discArray[spell]->highAttempts--;
      }
      break;
    case ATTEMPT_REM_ENGAGE:
      discArray[spell]->engAttempts--;
      break;
  }
}

enum skillSuccessT {
  SKILL_SUCCESS_NORMAL,
  SKILL_SUCCESS_POTION
};

static void logSkillSuccess(const TBeing *caster, spellNumT spell, skillSuccessT type)
{
  // this is used to log skill success 
  // there is usually no need to call this directly as it sits inside bSuccess
  // however, it can be called directly if appropriate

  if (!caster) {
    vlogf(LOG_BUG, "Something went into logSkillSuccess with no caster");
    return;
  }

  if (type == SKILL_SUCCESS_POTION) {
    if (caster->GetMaxLevel() > MAX_MORT && caster->desc)
      discArray[spell]->potSuccessImm++;
    else if (!caster->desc || !caster->isPc())
      discArray[spell]->potSuccessMob++;
    else
      discArray[spell]->potSuccess++;
    return;
  }
  
  if ((caster->GetMaxLevel() > MAX_MORT) && caster->desc) {
    discArray[spell]->immSuccess++;
    return;
  } else if (!caster->desc || !caster->isPc()) {
    discArray[spell]->mobSuccess++;
    return;
  }

  // mortal PCs only get here
  discArray[spell]->success++;
  
  if (discArray[spell]->minMana) {
    caster->desc->session.spell_success_pass++;
    caster->desc->career.spell_success_pass++;
  } else if (discArray[spell]->minLifeforce) {
    caster->desc->session.spell_success_pass++;
    caster->desc->career.spell_success_pass++;
  } else if (discArray[spell]->minPiety) {
    caster->desc->session.prayer_success_pass++;
    caster->desc->career.prayer_success_pass++;
  } else {
    caster->desc->session.skill_success_pass++;
    caster->desc->career.skill_success_pass++;
  }
}

enum logSkillFailT {
  FAIL_GENERAL,
  FAIL_FOCUS,
  FAIL_ENGAGE
};

static void logSkillFail(const TBeing *caster, spellNumT spell, logSkillFailT type)
{
  // this is used to log skill fails 
  // there is usually no need to call this directly as it sits inside bSuccess
  // however, it can be called directly if appropriate

  if (!caster) {
    vlogf(LOG_BUG,"Something went into logSkillFail with no caster");
    return;
  }

#if DISC_DEBUG
  vlogf(LOG_BUG, format("%s Fail Spell %s (%d) ubComp < 0") %  caster->getName() % discArray[spell]->name % spell);
#endif

  if ((caster->GetMaxLevel() > MAX_MORT) && caster->desc) {
    discArray[spell]->immFail++;
    return;
  } else if (!caster->desc || !caster->isPc()) {
    discArray[spell]->mobFail++;
    return;
  }

  // mortal PCs only get here
  switch (type) {
    case FAIL_GENERAL:    // genFail
      discArray[spell]->genFail++;
      break;
    case FAIL_FOCUS:   // Focus Fail
      discArray[spell]->focFail++;
      break;
    case FAIL_ENGAGE:   // EngageFail
      discArray[spell]->engFail++;
      break;
  }
  discArray[spell]->fail++;
}

static bool bSucCounter(TBeing *caster, skillUseClassT skillType, spellNumT spell, int roll, int ubCompetence)
{
  // success counter
  if (caster->desc) {
    if (skillType == SPELL_MAGE ||
        skillType == SPELL_CLERIC || 
        skillType == SPELL_DEIKHAN ||
        skillType == SPELL_SHAMAN || 
        skillType == SPELL_RANGER) {
      // Is a spell and not a skill
      if (((caster->fight() && !caster->isAffected(AFF_ENGAGER)) ||
         (!caster->fight() && IS_SET(discArray[spell]->targets, TAR_VIOLENT) && !IS_SET(caster->desc->autobits, AUTO_ENGAGE | AUTO_ENGAGE_ALWAYS)))) {
        // caster is hitting not engaging at the time
        logSkillAttempts(caster, spell, ATTEMPT_ADD_ENGAGE);
        int boost = caster->plotStat(STAT_CURRENT, STAT_FOC, 60, 10, 40, 1);
        int num = ::number(1,100);

        // if they are not fighting, don't let fighting distract...
        // if my victim is uncon, skip distract too
        if (!caster->fight() ||
            (caster->attackers == 1 && !caster->fight()->awake()))
          boost = 0;

        if (num <= boost) {
          //fail them some % of the time if fighting-adjust if nessessarry
          logSkillFail(caster, spell, FAIL_ENGAGE);
#if DISC_DEBUG
          if (caster->desc && caster->isPc()) {
            vlogf(LOG_BUG, format("%s Fail Spell %s (%d) EngFail: boost (%d) num (%d) , roll (%d) ubComp (%d)") %  caster->getName() % discArray[spell]->name % spell % boost % num % roll % ubCompetence);
          }
#endif
          switch (getSpellType(discArray[spell]->typ)) {
            case SPELL_CASTER:
              caster->sendTo(COLOR_SPELLS, "<c>Your fighting distracts you and you feel your casting skills failing you.<1>\n\r");
              break;
            case SPELL_DANCER:
              caster->sendTo(COLOR_SPELLS, "<c>Your fighting distracts your ritual.<1>\n\r");
              break;
            case SPELL_PRAYER:
              caster->sendTo(COLOR_SPELLS, "<c>Your fighting distracts you from your prayer.<1>\n\r");
              break;
            default:
              vlogf(LOG_BUG, "bad spot in distract (1).");
              break;
          }
          return FALSE;
        } else {
          // success since made the fightin roll for spells
          if ((caster->inPraying || caster->spelltask) && (skillType == SPELL_CLERIC || skillType == SPELL_DEIKHAN)) {
            if (!enforceHolySym(caster, spell, TRUE)) {
              logSkillAttempts(caster, spell, ATTEMPT_REM_NORM);
              logSkillAttempts(caster, spell, ATTEMPT_REM_ENGAGE);
              return FALSE;
            }
          }
          logSkillSuccess(caster, spell, SKILL_SUCCESS_NORMAL);
          return TRUE;
        }
      } else {
        // Is gonna be an engager--no penalty
        if ((caster->inPraying || caster->spelltask) && (skillType == SPELL_CLERIC || skillType == SPELL_DEIKHAN)) {
          if (!enforceHolySym(caster, spell, TRUE)) {
            logSkillAttempts(caster, spell, ATTEMPT_REM_NORM);
            return FALSE;
          }
        }
        logSkillSuccess(caster, spell, SKILL_SUCCESS_NORMAL);
        return TRUE;
      }
    } else {
      if ((caster->inPraying || caster->spelltask) && (skillType == SPELL_CLERIC || skillType == SPELL_DEIKHAN)) {
        if (!enforceHolySym(caster, spell, TRUE)) {
          logSkillAttempts(caster, spell, ATTEMPT_REM_NORM);
          return FALSE;
        }
      }
      logSkillSuccess(caster, spell, SKILL_SUCCESS_NORMAL);
      return TRUE;
    }
  } else {
    if ((caster->inPraying || caster->spelltask) && (skillType == SPELL_CLERIC || skillType == SPELL_DEIKHAN)) {
      if (!enforceHolySym(caster, spell, TRUE)) {
        logSkillAttempts(caster, spell, ATTEMPT_REM_NORM);
        return FALSE;
      }
    }
    logSkillSuccess(caster, spell, SKILL_SUCCESS_NORMAL);
    return TRUE;
  }
  
  return FALSE;
}

bool TBeing::bSuccess(int ubCompetence, double dPiety, spellNumT spell)
{
// Is same as other formulas, with this correction being made
// since factions' aren't in use, I'm simplifying and just making it
// call the other function
  return bSuccess(ubCompetence, spell);

#if FACTIONS_IN_USE
  // slight penalty based on low getPerc()
  if (desc) {
    int pietyNum;
    if ((skillType == SPELL_CLERIC) || (skillType == SPELL_DEIKHAN) ||
        (skillType == SKILL_CLERIC) || (skillType == SKILL_DEIKHAN)) {
      pietyNum = min(95, (3 * GetMaxLevel())); 
    } else { 
      pietyNum = min(70, (2 * GetMaxLevel()));
    }
    pietyNum = min(0, (((int) dPiety) - pietyNum));
    pietyNum = max(-64, pietyNum);
    pietyNum /= -4;
    roll += pietyNum;
  }
#endif
}

#if 0
static void logLearnFail(TBeing *caster, spellNumT spell, int type)
{
  // this is used to log learn fail
  // there is usually no need to call this directly as it sits inside i
  // learnFromDoing and learnFromDoingUnusual

  if (!caster) {
    vlogf(LOG_BUG,format("Something went into logLearnFail with no caster (%d)") %  spell);
    return;
  }

  if (caster->GetMaxLevel() > MAX_MORT) {
    return;
  } 

  if (!caster->desc) {
    vlogf(LOG_BUG,format("Something went into logLearnFail with no desc (%d)") %  spell);
    return;
  }

#if DISC_DEBUG
  vlogf(LOG_BUG, format("%s Fail Spell %s (%d) ubComp < 0") %  caster->getName() % discArray[spell]->name % spell); 
#endif

  if (type) {
    ;
  }
  discArray[spell]->learnFail++;
  return;
}
#endif

bool TBeing::bSuccess(spellNumT spell)
{
  return bSuccess(getSkillValue(spell), spell);
}

bool TBeing::bSuccess(int ubCompetence, spellNumT spell)
{
  // number of uses
  logSkillAttempts(this, spell, ATTEMPT_ADD_NORM);

  if (getQuaffUse()) {
    logSkillSuccess(this, spell, SKILL_SUCCESS_POTION);
    return true;
  }

  if (isImmortal() && desc &&
      IS_SET(desc->autobits, AUTO_SUCCESS)) {
    if (isPlayerAction(PLR_NOHASSLE))
      return TRUE;
    else
      return FALSE;
  }

  if (desc) {
    // Do learning
    if (getRawSkillValue(spell) >= 0) {
      if (learnFromDoing(spell, SILENT_NO, 0)) {
        ubCompetence++;
      }
    }
  }

   // not learned at all
  if (ubCompetence <= 0) {
    logSkillFail(this, spell, FAIL_GENERAL);
#if DISC_DEBUG
    if (desc && isPc()) {
      vlogf(LOG_BUG, format("%s Fail Spell %s (%d) ubComp < 0") %  
	    getName() % discArray[spell]->name % spell);
    }
#endif
    return FALSE;
  }

// force into range
  ubCompetence = min(max(ubCompetence, 0), (int) MAX_SKILL_LEARNEDNESS);

  // Here's the basis of this stuff:
  // At max learning, we desire the following results:
  // trivial    = 100%
  // easy       = 90%
  // normal     = 80%
  // difficult  = 70%
  // dangerous  = 60%
  // for less than maxed learning, scale it up linearly
  // Have focus factor in: high focus = 1.25 * above rates
  // low focus = 0.80 * above rates

  float limit = getSkillDiffModifier(spell);

  // scale linearly based on learning
  limit *= ubCompetence;
  limit /= MAX_SKILL_LEARNEDNESS;

  // factor in focus
  //limit *= plotStat(STAT_CURRENT, STAT_FOC, 0.80, 1.25, 1.00);
  limit *= getFocMod(); // does the same thing, just uses standard formula

  // Adding in Karma (luck) as a smaller component than focus
  limit *= plotStat(STAT_CURRENT, STAT_KAR, 0.9, 1.125, 1.0);

  // make other adjustments here
  // possibly have some for things like position, etc

  int iLimit = (int) limit;
  int roll = ::number(0,99);
  skillUseClassT skillType = discArray[spell]->typ;

  if (roll < iLimit) {
    // success
    return bSucCounter(this, skillType, spell, roll, ubCompetence);
  } else {
    // fail
    logSkillFail(this, spell, FAIL_GENERAL);
    return false;
  }

}

byte defaultProficiency(byte uLearned, byte uStart, byte uLearn)
{
  return((uLearned-uStart)*uLearn);
}

// CritSuccess for spells only
// The goal here is a crit rate between
// min - 0.1% -   10 out of 10000
// max - 10%  - 1000 out of 10000
// avg - 1%   -  100 out of 10000
//
// Factors should be
// Wisdom:      Primary
// Karma:       Secondary
// Learnedness: Modifier
// Position:    Modifier
//
critSuccT spellCritSuccess(TBeing *caster, spellNumT spell)
{
  // Base chance is 100
  // If we are totally average then this will hold up
  double chance = 100.00;

  // WIS is the primary so we'll plot to 10x down to 1/10th
  // We'll adjust all the way to the min/max
  chance *= caster->plotStat(STAT_CURRENT, STAT_WIS, 0.1, 10.0, 1.0);

  // Next we'll have KAR modify the chance to a lesser degree
  // And we will min/max after this so it doesn't affect the range
  chance *= caster->plotStat(STAT_CURRENT, STAT_KAR, 0.80, 1.25, 1.00);

  // Adjust downward for non-maxed spells
  chance *= caster->getSkillValue(spell) / 100;

  // Decrease chance if limited position
  if (caster->getPosition() == POSITION_RESTING || caster->getPosition() == POSITION_SITTING)
    chance *= 0.75;
  else if (caster->getPosition() == POSITION_CRAWLING)
    chance *= 0.50;

  // Min/Max here to keep the chance within bounds based on stats, skillValue and position
  chance = max(10.00, min(1000.00, chance));

  // If there were spellcrit eq or other skills/buff to modify crit
  // -- it would go here --

  int roll = ::number(1, 10000);

  if (roll <= chance) {
    // roll determined Crit success 
    // but we need to roll again to figure out what kind of crit
    int crit_roll = ::number(1, 10);

    // 10% crit kill
    // 20% triple crit
    // 70% double crit
    switch (crit_roll) {
      case 1:
        CS(caster, spell);
        return(CRIT_S_KILL);
        break;
      case 2:
      case 3:
        CS(caster, spell);
        return(CRIT_S_TRIPLE);
        break;
      default:
        CS(caster, spell);
        return(CRIT_S_DOUBLE);
    }
  }

  return(CRIT_S_NONE);
}


// Original critSuccess
//
// Min crit chance is:
// 8 out of 400 * 0.1 (for arbitrary return) = 0.002 or 0.2%
//
// Max is:
// 8 out of 40 * 0.1 (for arbitrary return) = 0.02 or 2%
critSuccT critSuccess(TBeing *caster, spellNumT spell)
{
  if (!caster->isPc())
    return CRIT_S_NONE;
  
  // use the new spell function for spells
  if (spell >= MIN_SPELL && spell < MAX_SPELL)
    return spellCritSuccess(caster, spell);

  // arbitrary to control overall rate of these
  if (::number(0,9))
    return CRIT_S_NONE;

  int roll, adjusted, task = 100; 
  discNumT das = getDisciplineNumber(spell, FALSE);
  CDiscipline *cd;

  if (das == DISC_NONE) {
    vlogf(LOG_BUG, format("bad disc for skill %d") %  spell);
    return CRIT_S_NONE;
  }
  if (!(cd = caster->getDiscipline(das)))
    return CRIT_S_NONE;

  if (discArray[spell]->task >= 0) {
    task += (discArray[spell]->task* 5);

    // increase difficulty if limited position
    if (caster->getPosition() == POSITION_RESTING)
      task += 200;
    else if (caster->getPosition() == POSITION_SITTING)
      task += 100;
    else if (caster->getPosition() == POSITION_CRAWLING)
      task += 300;
   }

  adjusted = cd->getLearnedness() - discArray[spell]->start + 1;

  adjusted *= 2;
  adjusted += 100;
  adjusted=max(100, adjusted);

  task *= 100;
  task /= adjusted;
  task = max(40, task);
  task = min(400,task);
  roll = ::number(1, task);

// pick it up below in the cases

  switch (roll) {
    case 1:
      CS(caster, spell);
      return(CRIT_S_KILL);
      break;
    case 2:
    case 3:
      CS(caster, spell);
      return(CRIT_S_TRIPLE);
      break;
    case 4:
    case 5:
    case 6:
    case 7:
    case 8:
      CS(caster, spell);
      return(CRIT_S_DOUBLE);
      break;
    default:
      return(CRIT_S_NONE);
  }
}

critFailT critFail(TBeing *caster, spellNumT spell)
{
  // arbitrary to control overall rate of these
  if (::number(0,9))
    return CRIT_F_NONE;

  int roll, adjusted, task = 100;

  if (discArray[spell]->task >= 0) {
    task -= (discArray[spell]->task* 10);

    // adjust for position
    if (caster->getPosition() == POSITION_RESTING)
      task -= 15;
    else if (caster->getPosition() == POSITION_SITTING)
      task -= 10;
    else if (caster->getPosition() == POSITION_CRAWLING)
      task -= 20;
  }
  discNumT das = getDisciplineNumber(spell, FALSE);
  if (das == DISC_NONE) {
    vlogf(LOG_BUG, format("bad disc for spell %d") %  spell);
    return CRIT_F_NONE; 
  } 
// adjust for learnedness of caster
  adjusted = caster->getDiscipline(das)->getLearnedness() - discArray[spell]->start + 1;

  adjusted *= 2;  
  adjusted += 100;
  adjusted=max(100, adjusted);

  task *= adjusted;  
  task /= 100;

  // Higher Focus gets better results
  task += ::number(0, caster->plotStat(STAT_CHOSEN, STAT_FOC, -5, 5, 0, 1));
  task = max(30, task);
  task = min(220,task);
  roll = ::number(1, task);

// Pick it up again below

  switch(roll) {
    case 1:
      CF(caster, spell);
      return(CRIT_F_HITOTHER);
      break;
    case 2:
    case 3:
    case 4:
    case 5:
      CF(caster, spell);
      return(CRIT_F_HITSELF);
      break;
    default:
      return(CRIT_F_NONE);
  }
}

int CDiscipline::useMana(byte ubCompetence, byte ubDifficulty)
{
  return(max((int) ubDifficulty,(100-((int) ubCompetence))/2));
}

// LIFEFORCE
int CDiscipline::useLifeforce(int ubCompetence, int ubDifficulty)
{
  return(max((int) ubDifficulty,(100-((int) ubCompetence))/2));
}
// END LIFEFORCE

double CDiscipline::usePerc(byte ubCompetence, double fDifficulty)
{
  return(fDifficulty+((fDifficulty*((double)(100-ubCompetence)))/100));
}

int checkMana(TBeing * caster, int mana)
{
  if (caster->noMana(mana)) {
    act("You can't seem to summon the magic energies...", FALSE, caster, 0, 0, TO_CHAR, ANSI_CYAN);
    act("$n begins to glow for a brief second, but it quickly fades.", FALSE, caster, NULL, NULL, TO_ROOM, ANSI_CYAN);
    return TRUE;
  } else   
    return FALSE;
}

// LIFEFORCE
int checkLifeforce(TBeing * caster, int lifeforce)
{
  if (caster->noLifeforce(lifeforce)) {
    act("You don't seem to have enough lifeforce...", FALSE, caster, 0, 0, TO_CHAR, ANSI_ORANGE);
    act("$n's eyes glow fire red and then quickly fade back to normal.", FALSE, caster, NULL, NULL, TO_ROOM, ANSI_RED);
    return TRUE;
  } else   
    return FALSE;
}
// END LIFEFORCE

#if FACTIONS_IN_USE
bool checkPerc(const TBeing * caster, double align)
{
  if (caster->percLess(align)) {
    act("$d scoffs in your general direction!", FALSE, caster, NULL, NULL, TO_CHAR);
    act("$n blushes as $d falls down laughing at $s request!", FALSE, caster, NULL, NULL, TO_ROOM);
    return TRUE;
  } else  
    return FALSE;
}
#endif

void TSymbol::findSym(TSymbol **best)
{
  *best = this;
}

TSymbol * TBeing::findHolySym(silentTypeT silent) const
{
  TThing *primary, *secondary, *neck;
  TSymbol *best = NULL;
  devotionLevelT dev_lev = getDevotionLevel();
  int bad_faction = 0, attuned = 0;
  int attuneCode = 0;
  factionTypeT sym_faction = FACT_UNDEFINED;

  primary = heldInPrimHand();
  secondary = heldInSecHand();
  neck = equipment[WEAR_NECK];

  if (isPc()) {
    if (primary) {
      best = NULL;
      primary->findSym(&best);
      if (best) {
        sym_faction = best->getSymbolFaction();
        attuned = 1;
        if ((sym_faction != FACT_UNDEFINED) || attuneCode) {
          if ((getFaction() == sym_faction) || attuneCode) { 
            // symbol in primary
            if (secondary && dev_lev <= DEV_LEV_SYMB_EITHER_OTHER_FREE) {
              // 0 = prim + sec free
              // 1 = sec + other free
              // must have sec free
              if (!silent) {
                act("$n looks confused as $e tentatively raises $p to the sky!",
                    FALSE, this, primary, NULL, TO_ROOM);
                act("$p feels inert as $N disrupts the conduit to $d!",
                    FALSE, this, primary, secondary, TO_CHAR);
              }
              return NULL;
            }
            return best;
          } else {
            bad_faction++;
          }
        }
      }
    }
    if (secondary) {
      best = NULL;
      secondary->findSym(&best);
      if (best) {
        sym_faction = best->getSymbolFaction();
        if ((sym_faction != FACT_UNDEFINED) || attuneCode) {
          attuned++;
          if ((getFaction() == sym_faction) || attuneCode) {
            // symbol in secondary
            if (dev_lev <= DEV_LEV_SYMB_PRIM_OTHER_FREE) {
             // 0 = prim + sec free
             // can only have it in primary
              if (!silent) {
                act("$n looks confused as $e tentatively raises $p to the sky!",
                    FALSE, this, secondary, NULL, TO_ROOM);
                act("$p feels inert and unbalanced as you raise it to the sky with the wrong hand!",
                    FALSE, this, secondary, NULL, TO_CHAR);
              }
              return NULL;
            } else if (primary && dev_lev < DEV_LEV_SYMB_EITHER_OTHER_EQUIP) {
              // 1 = sec + other free
              // 2 = prim + sec no restrict
              // must have primary free
              if (!silent) {
                act("$n looks confused as $e tentatively raises $p to the sky!",
                    FALSE, this, secondary, NULL, TO_ROOM);
                act("$p feels inert as $N disrupts the conduit to $d!",
                    FALSE, this, secondary, primary, TO_CHAR);
              }
              return NULL;
            }
            return best;
          } else {
            bad_faction++;
          }
        }
      }
    }
    if (neck) {
      best = NULL;
      neck->findSym(&best);
      if (best) {
        sym_faction = best->getSymbolFaction();
        if ((sym_faction != FACT_UNDEFINED) || attuneCode) {
          attuned++;
          if ((getFaction() == sym_faction) || attuneCode) {
            // symbol on neck
            if (dev_lev < DEV_LEV_SYMB_NECK) {
              // can't use neck
              if (!silent) {
                act("$n looks confused.",
                    FALSE, this, NULL, NULL, TO_ROOM);
                act("$d can't contact you without your symbol in your hands.",
                    FALSE, this, NULL, NULL, TO_CHAR);
              }
              return NULL;
            }
            return best;
          } else {
            bad_faction++;
          }
        }
      }
    }
  }
  if (!silent) {
    act("$n frantically gropes for something that $e can't seem to find!", 
             FALSE, this, NULL, NULL, TO_ROOM);
    if (bad_faction > 1) {
      act("Your deities do not recognize your choice of holy symbols!", 
             FALSE, this, NULL, NULL, TO_CHAR);
    } else if (bad_faction == 1) {
      act("Your deities do not recognize your choice of holy symbols!", FALSE, this, NULL, NULL, TO_CHAR);
    } else if (!bad_faction && (attuned > 1)) {
      act("Your holy symbols have not yet been attuned!", FALSE, this, NULL, NULL, TO_CHAR);
    } else if (!bad_faction && (attuned == 1)) {
     act("Your holy symbol has not yet been attuned!", FALSE, this, NULL, NULL, TO_CHAR);

    } else {
      act("You frantically grope for your holy symbol but can't find it! DAMN!", FALSE, this, NULL, NULL, TO_CHAR);
    }
  }
  return NULL;
}

bool canDoVerbal(TBeing *ch)
{
  // glub glub glub

  if (ch->roomp && ch->roomp->isUnderwaterSector())
    return FALSE;

  return (ch->canSpeak());
}

bool checkRoom(const TBeing * ch)
{
  if (ch->roomp->isRoomFlag(ROOM_NO_MAGIC)) {
    act("$n is surrounded by a dim glow that quickly fades.", TRUE, ch, 0, NULL, TO_ROOM);
    ch->sendTo("Some mysterious force prevents you from doing that here.\n\r");
    return FALSE;
  }
  return TRUE;
}

void checkFactionHurt(TBeing * caster, TBeing * victim) 
{
  int dec_amt;

  if (caster->isSameFaction(victim)) {
    dec_amt = (int) (caster->getMove() / 4);
    caster->addToMove(-dec_amt);
    caster->sendTo(format("%s frown upon the harming of a creature of the same faction.\n\r") % sstring(caster->yourDeity(your_deity_val, FIRST_PERSON).cap()));
    caster->sendTo("You are exhausted from the effort of doing so.\n\r");
    act("$n's chest heaves from exhaustion.", FALSE, caster, 0, 0, TO_ROOM);
    caster->updatePos();
  }
}

void checkFactionHelp(TBeing *caster, TBeing *victim) 
{
  int dec_amt;

  return;

  if (caster->isOppositeFaction(victim)) {
    dec_amt = (int) (caster->getMove() / 4);
    caster->addToMove(-dec_amt);
    caster->sendTo(format("%s frown upon the minions of the enemy.\n\r") % sstring(caster->yourDeity(your_deity_val, FIRST_PERSON)).cap());
    caster->sendTo("You are exhausted from the effort of doing so.\n\r");
    act("$n's chest heaves from exhaustion.", FALSE, caster, 0, 0, TO_ROOM);
    caster->updatePos();
  }
}

void TBeing::assignDisciplinesClass()
{
  // verify that disciplines haven't already been initted
  mud_assert(discs == NULL, "assignDisc(): assignment when already initted (1)");
  try {
    discs = new CMasterDiscipline();
  } catch (...) {
    vlogf(LOG_BUG, "assignDisc(): assertion trapped");
  }
  mud_assert(discs != NULL, "assignDisc(): discs was null after new");

  if (isPc()) {
    discs->disc[DISC_MAGE] = new CDMage();
    discs->disc[DISC_AIR] = new CDAir();
    discs->disc[DISC_ALCHEMY] = new CDAlchemy();
    discs->disc[DISC_EARTH] = new CDEarth();
    discs->disc[DISC_FIRE] = new CDFire();
    discs->disc[DISC_SORCERY] = new CDSorcery();
    discs->disc[DISC_SPIRIT] = new CDSpirit();
    discs->disc[DISC_WATER] = new CDWater();

    discs->disc[DISC_CLERIC] = new CDCleric();
    discs->disc[DISC_WRATH] = new CDWrath();
    discs->disc[DISC_AEGIS] = new CDAegis();
    discs->disc[DISC_AFFLICTIONS] = new CDAfflict();
    discs->disc[DISC_CURES] = new CDCures();
    discs->disc[DISC_HAND_OF_GOD] = new CDHand();

    discs->disc[DISC_WARRIOR] = new CDWarrior();
    discs->disc[DISC_DUELING] = new CDDueling();
    discs->disc[DISC_BRAWLING] = new CDBrawling();
    discs->disc[DISC_SOLDIERING] = new CDSoldiering();
    discs->disc[DISC_BLACKSMITHING] = new CDBlacksmithing();

    discs->disc[DISC_RANGER] = new CDRanger();
    discs->disc[DISC_ANIMAL] = new CDAnimal();
    discs->disc[DISC_PLANTS] = new CDPlants();
    discs->disc[DISC_NATURE] = new CDNature();

    discs->disc[DISC_DEIKHAN] = new CDDeikhan();
    discs->disc[DISC_DEIKHAN_MARTIAL] = new CDDeikhanMartial();
    discs->disc[DISC_MOUNTED] = new CDMounted();
    discs->disc[DISC_DEIKHAN_GUARDIAN] = new CDDeikhanGuardian();
    discs->disc[DISC_DEIKHAN_ABSOLUTION] = new CDDeikhanAbsolution();
    discs->disc[DISC_DEIKHAN_VENGEANCE] = new CDDeikhanVengeance();

    discs->disc[DISC_MONK] = new CDMonk();
    discs->disc[DISC_MEDITATION_MONK] = new CDMeditationMonk();
    discs->disc[DISC_LEVERAGE] = new CDLeverage();
    discs->disc[DISC_MINDBODY] = new CDMindBody();
    discs->disc[DISC_FOCUSED_ATTACKS] = new CDFAttacks();
    discs->disc[DISC_IRON_BODY] = new CDIronBody();

    discs->disc[DISC_THIEF] = new CDThief();
    discs->disc[DISC_THIEF_FIGHT] = new CDThiefFight();
    discs->disc[DISC_MURDER] = new CDMurder();
    discs->disc[DISC_LOOTING] = new CDLooting();
    discs->disc[DISC_POISONS] = new CDPoisons();
    discs->disc[DISC_STEALTH] = new CDStealth();
    discs->disc[DISC_TRAPS] = new CDTraps();

    discs->disc[DISC_SHAMAN_ALCHEMY] = new CDShamanAlchemy();
    discs->disc[DISC_SHAMAN_ARMADILLO] = new CDShamanArmadillo();
    discs->disc[DISC_SHAMAN_CONTROL] = new CDShamanControl();
    discs->disc[DISC_SHAMAN_FROG] = new CDShamanFrog();
    discs->disc[DISC_SHAMAN_SKUNK] = new CDShamanSkunk();
    discs->disc[DISC_SHAMAN_SPIDER] = new CDShamanSpider();
    discs->disc[DISC_SHAMAN_HEALING] = new CDShamanHealing();
    discs->disc[DISC_SHAMAN] = new CDShaman();
    discs->disc[DISC_RITUALISM] = new CDRitualism();

    discs->disc[DISC_WIZARDRY] = new CDWizardry();
    discs->disc[DISC_FAITH] = new CDFaith();
    discs->disc[DISC_THEOLOGY] = new CDTheology();
    discs->disc[DISC_LORE] = new CDLore();

    discs->disc[DISC_SLASH] = new CDSlash();
    discs->disc[DISC_BLUNT] = new CDBash();
    discs->disc[DISC_PIERCE] = new CDPierce();
    discs->disc[DISC_RANGED] = new CDRanged();
    discs->disc[DISC_BAREHAND] = new CDBarehand();

    discs->disc[DISC_COMBAT] = new CDCombat();
    discs->disc[DISC_ADVENTURING] = new CDAdventuring();
    discs->disc[DISC_ADVANCED_ADVENTURING] = new CDAdvAdventuring();
    discs->disc[DISC_DEFENSE] = new CDDefense();
    discs->disc[DISC_OFFENSE] = new CDOffense();
 
    discs->disc[DISC_PSIONICS] = new CDPsionics();

    discs->disc[DISC_COMMONER] = new CDCommoner();

    // only players get psionics
    if(hasQuestBit(TOG_PSIONICIST) || isImmortal())
      getDiscipline(DISC_PSIONICS)->ok_for_class |= getClass();
  }
  // assign these to every class
  if (!isPc()) {
    discs->disc[DISC_ADVENTURING] = new CDAdventuring();
    discs->disc[DISC_ADVANCED_ADVENTURING] = new CDAdvAdventuring();
    discs->disc[DISC_COMBAT] = new CDCombat();
    discs->disc[DISC_BLUNT] = new CDBash();
    discs->disc[DISC_SLASH] = new CDSlash();
    discs->disc[DISC_PIERCE] = new CDPierce();
    discs->disc[DISC_RANGED] = new CDRanged();
  }


    
  if (!player.Class) {
    vlogf(LOG_BUG,format("call to assignDisciplinesClass without a valid Class (%s)") %  getName());
    return;
  }

  if (hasClass(CLASS_MAGE)) {
    if (!isPc()) {
      discs->disc[DISC_MAGE] = new CDMage();
      discs->disc[DISC_AIR] = new CDAir();
      discs->disc[DISC_ALCHEMY] = new CDAlchemy();
      discs->disc[DISC_EARTH] = new CDEarth();
      discs->disc[DISC_FIRE] = new CDFire();
      discs->disc[DISC_SORCERY] = new CDSorcery();
      discs->disc[DISC_SPIRIT] = new CDSpirit();
      discs->disc[DISC_WATER] = new CDWater();
      discs->disc[DISC_WIZARDRY] = new CDWizardry();
      discs->disc[DISC_LORE] = new CDLore();
    }
  }

  if (hasClass(CLASS_CLERIC)) {
    if (!isPc()) {
      discs->disc[DISC_CLERIC] = new CDCleric();
      discs->disc[DISC_WRATH] = new CDWrath();
      discs->disc[DISC_AEGIS] = new CDAegis();
      discs->disc[DISC_AFFLICTIONS] = new CDAfflict();
      discs->disc[DISC_CURES] = new CDCures();
      discs->disc[DISC_HAND_OF_GOD] = new CDHand();
      discs->disc[DISC_FAITH] = new CDFaith();
      discs->disc[DISC_THEOLOGY] = new CDTheology();
    }
  }

  if (hasClass(CLASS_WARRIOR)) {
    if (!isPc()) {
      discs->disc[DISC_WARRIOR] = new CDWarrior();
      discs->disc[DISC_DUELING] = new CDDueling();
      discs->disc[DISC_BRAWLING] = new CDBrawling();
      discs->disc[DISC_SOLDIERING] = new CDSoldiering();
      discs->disc[DISC_BLACKSMITHING] = new CDBlacksmithing();
      discs->disc[DISC_DEFENSE] = new CDDefense();
    }
  }

  if (hasClass(CLASS_RANGER)) {
    if (!isPc()) {
      discs->disc[DISC_RANGER] = new CDRanger();
      discs->disc[DISC_ANIMAL] = new CDAnimal();
      discs->disc[DISC_PLANTS] = new CDPlants();
      discs->disc[DISC_NATURE] = new CDNature();
      discs->disc[DISC_DEFENSE] = new CDDefense();
    }
  }

  if (hasClass(CLASS_DEIKHAN)) {
    if (!isPc()) {
      discs->disc[DISC_DEIKHAN] = new CDDeikhan();
      discs->disc[DISC_DEIKHAN_MARTIAL] = new CDDeikhanMartial();
      discs->disc[DISC_MOUNTED] = new CDMounted();
      discs->disc[DISC_DEIKHAN_GUARDIAN] = new CDDeikhanGuardian();
      discs->disc[DISC_DEIKHAN_ABSOLUTION] = new CDDeikhanAbsolution();
      discs->disc[DISC_DEIKHAN_VENGEANCE] = new CDDeikhanVengeance();
      discs->disc[DISC_FAITH] = new CDFaith();
      discs->disc[DISC_THEOLOGY] = new CDTheology();
      discs->disc[DISC_DEFENSE] = new CDDefense();
    }
  }

  if (hasClass(CLASS_MONK)) {
    if (!isPc()) {
      discs->disc[DISC_MONK] = new CDMonk();
      discs->disc[DISC_MEDITATION_MONK] = new CDMeditationMonk();
      discs->disc[DISC_LEVERAGE] = new CDLeverage();
      discs->disc[DISC_MINDBODY] = new CDMindBody();
      discs->disc[DISC_FOCUSED_ATTACKS] = new CDFAttacks();
      discs->disc[DISC_BAREHAND] = new CDBarehand();
      discs->disc[DISC_IRON_BODY] = new CDIronBody();
      discs->disc[DISC_OFFENSE] = new CDOffense();
    }
  }

  if (hasClass(CLASS_THIEF)) {
    if (!isPc()) {
      discs->disc[DISC_THIEF] = new CDThief();
      discs->disc[DISC_THIEF_FIGHT] = new CDThiefFight();
      discs->disc[DISC_MURDER] = new CDMurder();
      discs->disc[DISC_LOOTING] = new CDLooting();
      discs->disc[DISC_POISONS] = new CDPoisons();
      discs->disc[DISC_STEALTH] = new CDStealth();
      discs->disc[DISC_TRAPS] = new CDTraps();
      discs->disc[DISC_OFFENSE] = new CDOffense();
    }
  }

  if (hasClass(CLASS_SHAMAN)) {
    if (!isPc()) {
      discs->disc[DISC_SHAMAN] = new CDShaman();
      discs->disc[DISC_RITUALISM] = new CDRitualism();
      discs->disc[DISC_SHAMAN_HEALING] = new CDShamanHealing();
      discs->disc[DISC_SHAMAN_ALCHEMY] = new CDShamanAlchemy();
      discs->disc[DISC_SHAMAN_ARMADILLO] = new CDShamanArmadillo();
      discs->disc[DISC_SHAMAN_CONTROL] = new CDShamanControl();
      discs->disc[DISC_SHAMAN_FROG] = new CDShamanFrog();
      discs->disc[DISC_SHAMAN_SKUNK] = new CDShamanSkunk();
      discs->disc[DISC_SHAMAN_SPIDER] = new CDShamanSpider();
    }
  }

  if(hasClass(CLASS_COMMONER)){
    if(!isPc()){
      discs->disc[DISC_COMMONER] = new CDCommoner();
    }
  }

  for(int i=0;i<MAX_DISCS;++i){
    if(hasClass(discNames[i].class_num) && i!=DISC_PSIONICS){
      getDiscipline((discNumT)i)->ok_for_class |= discNames[i].class_num;
    }
  }



  // fix the mobs learnednesses
  if (isPc())
    return;

  // assign mobs skills
  assignSkillsClass();

  mud_assert(discs != NULL, "assignDisc(): discs was null after new (2)");
}


void TBeing::initSkillsBasedOnDiscLearning(discNumT disc_num) 
{
  int disc_learn = 0, boost = 0, max_amt = 0, value = 0;
  CDiscipline *cd;

  mud_assert(discs != NULL, "Somehow there was a call to initSkillsBasedOnDiscLearning without a discs %s", getName().c_str());

  // mob skills are always maxed for their disc-training
  if (!(cd = getDiscipline(disc_num))) {
  }
  disc_learn = cd->getLearnedness();

  spellNumT i;
  for (i = MIN_SPELL; i < MAX_SKILL; i++) {
    if (hideThisSpell(i))
      continue;
    if (discArray[i]->disc == disc_num) {
      if (!getSkill(i))
        continue;

      if (disc_learn < discArray[i]->start) 
        continue;

      if (isPc()) {
        setNatSkillValue(i, getMaxSkillValue(i));
        setSkillValue(i, getMaxSkillValue(i));
      } else {
        max_amt = (disc_learn - discArray[i]->start + 1) *  discArray[i]->learn;
        max_amt = max(max_amt, 1);
        max_amt = min(max_amt, (int) MAX_SKILL_LEARNEDNESS);
        if (discArray[i]->startLearnDo >= 0) {
          boost = min(max_amt, (int) discArray[i]->startLearnDo);
          if ((5 * discArray[i]->learn) >= MAX_SKILL_LEARNEDNESS) {
             value = 50 + ((disc_learn - discArray[i]->start) * 2);
             min(value, 100);
             value = value * max_amt;
             value /= 100;
             value = min((int) MAX_SKILL_LEARNEDNESS, value);
             value = max(value, 10);
             value = max(boost, value);
          } else {
            if (disc_learn <= MAX_DISC_LEARNEDNESS) {
              value = 75 + (disc_learn - discArray[i]->start);
              min(value, 100);
              value = value * max_amt;
              value /= 100;
              value=min((int) MAX_SKILL_LEARNEDNESS, value);
              value = max(value, 10);
              value = max(boost, value);
            } else {
              value = max_amt;
            }
          }
        } else {
          value = max_amt;
        }
        setNatSkillValue(i, value);
        setSkillValue(i,value);
        if (i ==  SKILL_TACTICS){
          setNatSkillValue(SKILL_TACTICS,min(100, (GetMaxLevel() * 12)));
          setSkillValue(SKILL_TACTICS,min(100, (GetMaxLevel() * 12)));
	}
        if (i == SKILL_RIDE){
          setNatSkillValue(SKILL_RIDE,min(100,5 + GetMaxLevel() * 2));
          setSkillValue(SKILL_RIDE,min(100,5 + GetMaxLevel() * 2)); 
	}
      }
    }
  }
}

const char *skill_diff(byte num)
{
  if (num <= 1)
    return "next practice";
  else if (num <= 3)
    return "very soon";
  else if (num < 8)
    return "soon";
  else if (num < 12)
    return "fairly soon";
  else if (num < 16)
    return "little more training";
  else if (num < 20)
    return "not too long now";
  else if (num < 30)
    return "lot more training";
  else if (num < 40)
    return "in a while";
  else if (num < 50)
    return "some day";
  else if (num < 60)
    return "in the future";
  else
    return "way in the future";
}

// this should be more exotic taking into effect things like
// wizardry skill, etc

// returns TRUE if this is lower lev then vict
// returns 2 if vict is immortal
// else is FALSE
// COME BACK TO THIS - ADD SKILL FOR LEARNING IN ADVANCED DISCIPLINES - MAROR
int TBeing::isNotPowerful(TBeing *vict, int lev, spellNumT skill, silentTypeT silent)
{
  // force success for immorts
  if (isImmortal() && !vict->isImmortal())
    return FALSE;

  if (vict->isImmortal() && !isImmortal()) {
    if (!silent) {
      act("You can't do that to $N.", FALSE, this, 0, vict, TO_CHAR);
      act("$E's a god!", FALSE, this, 0, vict, TO_CHAR);
      act("Nothing seems to happen.", TRUE, this, 0, 0, TO_ROOM);
    }
    return 2;
  }

  int advLearning = 0;
  // adjust lev for stuff here
  // this allows for casting spells over level
// COSMO DISC MARKER - change to add in deikhan and shaman, ranger for casting
  skill = getSkillNum(skill);
  int level = GetMaxLevel();
  CDiscipline *cd;
  lev = level;
  switch (getDisciplineNumber(skill, FALSE)) {
    case DISC_MAGE:
    case DISC_PLANTS:
    case DISC_AIR:
    case DISC_EARTH:
    case DISC_FIRE:
    case DISC_WATER:
    case DISC_SPIRIT:
    case DISC_SORCERY:
    case DISC_ALCHEMY:
    case DISC_ANIMAL:
    case DISC_NATURE:
      cd = getDiscipline(DISC_WIZARDRY);
      if (cd && cd->getLearnedness() > 0)
        lev += 2 + (cd->getLearnedness() / 34);
      break;
    case DISC_SHAMAN_HEALING:
    case DISC_SHAMAN_ARMADILLO:
    case DISC_SHAMAN:
    case DISC_RITUALISM:
    case DISC_SHAMAN_FROG:
    case DISC_SHAMAN_CONTROL:
    case DISC_SHAMAN_SPIDER:
    case DISC_SHAMAN_SKUNK:
      cd = getDiscipline(DISC_RITUALISM);
      if (cd && cd->getLearnedness() > 0)
        lev += 2 + (cd->getLearnedness() / 34);
      break;
    case DISC_CURES:
    case DISC_AEGIS:
    case DISC_WRATH:
    case DISC_HAND_OF_GOD:
    case DISC_CLERIC:
    case DISC_DEIKHAN:
    case DISC_DEIKHAN_VENGEANCE:
    case DISC_DEIKHAN_ABSOLUTION:
    case DISC_DEIKHAN_GUARDIAN:
      cd = getDiscipline(DISC_FAITH);
      if (cd && cd->getLearnedness() > 0)
        lev += 2 + (cd->getLearnedness() / 34);
      break;
    case DISC_AFFLICTIONS:
      // a very special case so big mobs are especially vulnerble
      // basically means clerics rock vs mobs <= 100th level
      cd = getDiscipline(DISC_FAITH);
      if (cd && cd->getLearnedness() > 0)
        lev += 2 + (cd->getLearnedness() / 34);
      break;
    default:
      break;
  }
/*
  // Calculate aggregate learning of skills
  // the problem with doing it this way is that it depends on people
  // putting in enough skills to have a representative sample - Maror
  discNumT assDiscNum = discArray[skill]->assDisc;
  spellNumT i;
  double basSum = 0, advSum = 0, basCount = 0, advCount = 0;
  double basMean = 0, advMean = 0;
  for (i = MIN_SPELL; i < MAX_SKILL; i++) {
    if (hideThisSpell(i)) continue;
    if (discArray[i]->assDisc != assDiscNum) continue;
    if (!getDiscipline(discArray[i]->assDisc)) continue;
    if (discArray[i]->assDisc == assDiscNum 
         && getDiscipline(discArray[i]->disc)->isBasic()) {
      basSum += getSkillValue(i);
      basCount++;
    } else if (discArray[i]->assDisc == assDiscNum) {
      advSum += getSkillValue(i);
      advCount++;
    }
  }
  if (basCount > 0) { basMean = basSum / basCount; } else basMean = 0;
  if (advCount > 0) { advMean = advSum / advCount; } else advMean = 0;
*/
  double bonus = 0;

  if (discArray[skill]->disc != discArray[skill]->assDisc) {
    CDiscipline * assDisc = getDiscipline(discArray[skill]->assDisc);
    if (assDisc) {
      advLearning = getAdvLearning(skill);
      bonus += advLearning;
//      advLearning = getAdvDoLearning(skill);
//      bonus += (level * advLearning) / 200;
    }                      
  } 
  
  // make the chance at equal level 90%
  // still always works on something more than 3 levels lower than you
  double halfLev = 6;
  double startLev = 1;  // number of levels below current where this takes effect
  // same level = 85%, level + 10 = 16%
  
  // add a bonus for training in skills

/*  halfLev += basMean / 50 + advMean / 20; // add up to 6 levels to double time
  startLev -= basMean / 50 + advMean / 50; // boost success guarantee up to 4
  */

  halfLev += bonus / 16.7; // up to 6
  startLev -= bonus / 25; // up to 4

  
  double levelDiff = (double) vict->GetMaxLevel() - (double) lev + startLev;
  int chance = (int) (10000 * exp(-levelDiff / halfLev ));
  if (chance < 0) chance = 10001; //roll over of integer
  int roll = ::number(1,10000);
  if (levelDiff >= 0 && chance < roll) {
    if (!silent) {
        act("You are unable to get past $N's defenses.", FALSE, this, 0, vict, TO_CHAR);

      act("$n is unable to get past your defenses.", TRUE, this, 0, vict, TO_VICT);
      act("$n is unable to get past $N's defenses.", TRUE, this, 0, vict, TO_NOTVICT);
    }
    return TRUE;
  }
  return FALSE;
}

int TBeing::getSkillLevel(spellNumT skill) const
{
  discNumT disc_num = getDisciplineNumber(skill, FALSE);
  int lev = 0;

  if (isImmortal()) {
    return GetMaxLevel();
  }

  switch(disc_num) {
    case DISC_COMMONER:
      lev = getClassLevel(CLASS_COMMONER);
      break;
    case DISC_CLERIC:
    case DISC_AEGIS:
    case DISC_WRATH:
    case DISC_AFFLICTIONS:
    case DISC_CURES:
    case DISC_HAND_OF_GOD:
      lev = getClassLevel(CLASS_CLERIC);
      break;
    case DISC_MAGE:
    case DISC_SPIRIT:
    case DISC_ALCHEMY:
    case DISC_AIR:
    case DISC_WATER:
    case DISC_FIRE:
    case DISC_EARTH:
    case DISC_SORCERY:
      lev = getClassLevel(CLASS_MAGE);
      break;
    case DISC_RANGER: 
    case DISC_PLANTS:
    case DISC_ANIMAL:
    case DISC_NATURE:
      lev = getClassLevel(CLASS_RANGER);
      break;
    case DISC_WARRIOR:
    case DISC_DUELING:
    case DISC_BRAWLING:
    case DISC_SOLDIERING:
    case DISC_BLACKSMITHING:
      lev = getClassLevel(CLASS_WARRIOR);
      break;
    case DISC_DEIKHAN:
    case DISC_DEIKHAN_MARTIAL:
    case DISC_MOUNTED:
    case DISC_DEIKHAN_GUARDIAN:
    case DISC_DEIKHAN_ABSOLUTION:
    case DISC_DEIKHAN_VENGEANCE:
      lev = getClassLevel(CLASS_DEIKHAN);
      break;
    case DISC_THIEF:
    case DISC_THIEF_FIGHT:
    case DISC_LOOTING:
    case DISC_MURDER:
    case DISC_POISONS:
    case DISC_STEALTH:
    case DISC_TRAPS:
      lev = getClassLevel(CLASS_THIEF);
      break;
    case DISC_MONK:
    case DISC_MEDITATION_MONK:
    case DISC_LEVERAGE:
    case DISC_MINDBODY:
    case DISC_FOCUSED_ATTACKS:
    case DISC_IRON_BODY:
      lev = getClassLevel(CLASS_MONK);
      break;
    case DISC_SHAMAN:
    case DISC_SHAMAN_FROG:
    case DISC_SHAMAN_ALCHEMY:
    case DISC_SHAMAN_ARMADILLO:
    case DISC_SHAMAN_SKUNK:
    case DISC_SHAMAN_SPIDER:
    case DISC_SHAMAN_CONTROL:
    case DISC_SHAMAN_HEALING:
    case DISC_RITUALISM:
      lev = getClassLevel(CLASS_SHAMAN);
      break;
    case DISC_WIZARDRY:
      lev = getClassLevel(CLASS_MAGE);
      break;
    case DISC_LORE:
      lev = getClassLevel(CLASS_MAGE);
      break;
    case DISC_THEOLOGY:
    case DISC_FAITH:
      if (hasClass(CLASS_CLERIC)) {
        lev = getClassLevel(CLASS_CLERIC);
        break;
      } else {
        lev = getClassLevel(CLASS_DEIKHAN);
      }
      break;
    case DISC_ADVENTURING:
    case DISC_ADVANCED_ADVENTURING:
    case DISC_COMBAT:
    case DISC_SLASH:
    case DISC_BLUNT:
    case DISC_PIERCE:
    case DISC_RANGED:
    case DISC_BAREHAND:
    case DISC_DEFENSE:
    case DISC_OFFENSE:
    case DISC_PSIONICS:
    case DISC_BOGUS1:
    case DISC_BOGUS2:
      lev = GetMaxLevel();
      break;
    case MAX_DISCS:
    case DISC_NONE:
    case MAX_SAVED_DISCS:
      vlogf(LOG_BUG, format("bad disc (%d, %d) in getSkillLevel (%s).") % 
               disc_num % skill % getName());
      lev = 0;
      break;
  }
  return lev;
}

short TBeing::getMaxSkillValue(spellNumT skill) const
{
  int tmp2;
  discNumT dn = getDisciplineNumber(skill, FALSE);
  if (dn == DISC_NONE) {
    vlogf(LOG_BUG, format("bad disc for skill %d") %  skill);
    return SKILL_MIN;
  }
  CDiscipline * cdisc = getDiscipline(dn);

  if (cdisc && discArray[skill] && *discArray[skill]->name) {
    tmp2 = max(0, cdisc->getLearnedness() - discArray[skill]->start + 1);

    if (((!desc || isImmortal()) || (!discArray[skill]->toggle || hasQuestBit(discArray[skill]->toggle))) && tmp2 > 0) {
      if (!desc && discArray[skill]->toggle && (master && master->desc && !master->isImmortal())) {
        return SKILL_MIN;
      } else {
        return min((discArray[skill]->learn * tmp2), (int) MAX_SKILL_LEARNEDNESS);
      }
    }
  }
  return SKILL_MIN;
}

CDiscipline * TBeing::getDiscipline(discNumT n) const
{
  if(n < 0 || n > MAX_DISCS){
    vlogf(LOG_BUG, format("getDiscipline called out of range: n=%i") %  n);
    return NULL;
  }

  if (discs){
    return discs->disc[n];
  } else {
    mud_assert(0,
    "TBeing had no CMasterDiscipline. '%s'", !getName().empty() ? getName().c_str() : "NoName");

    return NULL;
  }
}

void CS(const TBeing *caster, spellNumT spell)
{
  if ((caster->GetMaxLevel() > MAX_MORT) && caster->desc) {
    discArray[spell]->immCrits++;
  } else if (caster->desc && caster->isPc()) {
    discArray[spell]->crits++;
  } else {
    discArray[spell]->mobCrits++;
  }

}

static void learnAttemptLog(const TBeing *caster, spellNumT spell)
{
  if (caster->desc && caster->isPc()) {
    discArray[spell]->learnAttempts++;
  } 
}

static void learnLearnednessLog(const TBeing *caster, spellNumT spell, int amt)
{
  if (caster->desc && caster->isPc()) {
      discArray[spell]->learnLearn += amt;
  }
}

static void learnSuccessLog(const TBeing *caster, spellNumT spell, int boost)
{
  if (caster->desc && caster->isPc()) {
    discArray[spell]->learnSuccess++;
    discArray[spell]->learnBoost += boost;
    caster->desc->session.hones++;
  }
}

void CS(spellNumT spell_num)
{
  return;
  // crit success counter
  discArray[spell_num]->crits++;
}

void CF(const TBeing *caster, spellNumT spell)
{
  if ((caster->GetMaxLevel() > MAX_MORT) && caster->desc) {
    discArray[spell]->immCritf++;
  } else if (caster->desc && caster->isPc()) {
    discArray[spell]->critf++;
  } else {
    discArray[spell]->mobCritf++;
  }
}

void CF(spellNumT spell_num)
{
  return;
  // crit fail counter
  discArray[spell_num]->critf++;
}

void SV(const TBeing *caster, spellNumT spell)
{
  if ((caster->GetMaxLevel() > MAX_MORT) && caster->desc) {
    discArray[spell]->immSaves++;
  } else if (caster->desc && caster->isPc()) {
    discArray[spell]->saves++;
  } else {
    discArray[spell]->mobSaves++;
  }
}

void SV(spellNumT spell_num)
{
  // saves counter
  discArray[spell_num]->saves++;
}

void LogDam(const TBeing *caster, spellNumT spell_num, int dam)
{
  // this is used to log damage and number of victims
  // theire is no need to call this directly as it sits inside reconcileDamage()
  // however, non damagin spells that want to log "damage" (eg, heal) do call it

  // sometimes we call reconcileDamage with dam = 0 just to start a fight
  // ignore such events
  // i'm not sure why we should ignore dam < 0, but it seems right to do.

  if (dam <= 0)
    return;
  if ((caster->GetMaxLevel() > MAX_MORT) && caster->desc) {
    discArray[spell_num]->immDamage += dam;
    discArray[spell_num]->immVictims++;
  } else if (caster->desc && caster->isPc()) {
    discArray[spell_num]->damage += dam;
    discArray[spell_num]->victims++;
  } else {
    discArray[spell_num]->mobDamage += dam;
    discArray[spell_num]->mobVictims++;
  }
}

#if 0
enum logLearnAttemptT {
  LEARN_ATT_ADD,
  LEARN_ATT_REM
};

static void logLearnAttempts(TBeing *caster, spellNumT spell, logLearnAttemptT type, int)
{
  if ((caster->GetMaxLevel() > MAX_MORT) || !caster->isPc() || !caster->desc)
    return;

  switch (type) {
    case LEARN_ATT_ADD:
      discArray[spell]->learnAttempts++;
      discArray[spell]->learnLearn += caster->getSkillValue(spell);
      discArray[spell]->learnLevel += caster->GetMaxLevel(); 
      break;
    case LEARN_ATT_REM:
      discArray[spell]->learnAttempts -= 1;
      discArray[spell]->learnLearn -= caster->getSkillValue(spell);
      discArray[spell]->learnLevel += caster->GetMaxLevel();
      break;
  }

 return;
}

enum logLearnSuccessT {
  LEARN_SUC_NORM,
  LEARN_SUC_DISC,
  LEARN_SUC_ADV
};

static void logLearnSuccess(TBeing *caster, spellNumT spell, logLearnSuccessT type, int boost)
{
  // this is used to log learn success 
  // there is usually no need to call this directly as it sits inside i
  // learnFromDoing and learnFromDoingUnusual

  if (!caster) {
    vlogf(LOG_BUG,format("Something went into logLearnSuccess with no caster (%d)") %  spell);
    return;
  }

  if (caster->GetMaxLevel() > MAX_MORT) {
    return;
  }

  if (!caster->desc) {
    vlogf(LOG_BUG,format("Something went into logLearnSuccess with no desc (%d)") %  spell);
    return;
  }

  switch (type) {
    case LEARN_SUC_NORM:
      discArray[spell]->learnSuccess++;
      discArray[spell]->learnBoost += boost;
      break;
    case LEARN_SUC_DISC:
      discArray[spell]->learnDiscSuccess++;
      break;
    case LEARN_SUC_ADV:
      discArray[spell]->learnAdvDiscSuccess++;
      break;
  }
  return;
}
#endif

int TMonster::learnFromDoingUnusual(learnUnusualTypeT, spellNumT, int)
{
  return FALSE;
}

int TPerson::learnFromDoingUnusual(learnUnusualTypeT type, spellNumT spell, int amt)
{
  spellNumT w_type = spell;
  spellNumT spell2 = TYPE_UNDEFINED;
  int value = 0;

  if (isImmortal() || !desc || roomp->isRoomFlag(ROOM_ARENA))
    return FALSE;

// for combat skills/armor wearing adventuring stuff that doesnt use bsuccess
  switch (type) {
    case LEARN_UNUSUAL_PROFICIENCY:   // proficiencies
      if (w_type == TYPE_HIT) {
          spell = SKILL_BAREHAND_PROF; 
	  spell2 = SKILL_BAREHAND_SPEC;
          amt *= 2;
      } else if (slashType(w_type)) {
        spell = SKILL_SLASH_PROF;
        spell2 = SKILL_SLASH_SPEC;
      } else if (bluntType(w_type)) {
        spell = SKILL_BLUNT_PROF;
        spell2 = SKILL_BLUNT_SPEC;
      } else if (pierceType(w_type)) {
        spell = SKILL_PIERCE_PROF;
        spell2 = SKILL_PIERCE_SPEC;
      } else {
        vlogf(LOG_BUG, format("Wierd case in learnFromDoingUnusual %s, %d") %  getName() % w_type);
        return FALSE;
      }
      if (amt && ::number(0,amt)) {
        // arbitrary dont let the skill increase
        if (doesKnowSkill(spell) && 
            (value = getRawNatSkillValue(spell)) < MAX_SKILL_LEARNEDNESS) {
          learnAttemptLog(this, spell);
          learnLearnednessLog(this, spell, value);
          return FALSE;
        }
        if ((spell2 >= 0) && ((value = getRawNatSkillValue(spell2)) < MAX_SKILL_LEARNEDNESS)) {
          learnAttemptLog(this, spell2);
          learnLearnednessLog(this, spell2, value);
        }
        return FALSE;
      } else if (0 && spell == SKILL_BAREHAND_PROF) {
        if (doesKnowSkill(spell))
          return learnFromDoing(spell, SILENT_NO, 0);
        else
          return FALSE;
      } else {
        if (doesKnowSkill(spell) && !(value = learnFromDoing(spell, SILENT_NO, 0))) {
          if ((spell2 >= 0) &&
              (getDiscipline(DISC_COMBAT)->getLearnedness() >= MAX_DISC_LEARNEDNESS) &&
              doesKnowSkill(spell2)) {
            return learnFromDoing(spell2, SILENT_NO, 0);
          } else 
            return FALSE;
        } 
      } 
      break;
    case LEARN_UNUSUAL_NORM_LEARN:
      if (amt && ::number(0, amt)) {
        // arbitrary dont let the skill increase
        if (doesKnowSkill(spell) && (value = getRawNatSkillValue(spell)) < MAX_SKILL_LEARNEDNESS) {
          learnAttemptLog(this, spell);
          learnLearnednessLog(this, spell, value);
          return FALSE;
        }
        return FALSE;
      } else {
        if (doesKnowSkill(spell))
          return learnFromDoing(spell, SILENT_NO, 0);
        else
          return FALSE;
      }
      break;
    case LEARN_UNUSUAL_FORCED_LEARN:
      if (amt && ::number(0, amt)) {
      // arbitrary dont let the skill increase
        if (doesKnowSkill(spell) && (value = getRawNatSkillValue(spell)) < MAX_SKILL_LEARNEDNESS) {
          learnAttemptLog(this, spell);
          learnLearnednessLog(this, spell, value);
           return FALSE;
        }
        return FALSE;
      } else {
        if (doesKnowSkill(spell))
          return learnFromDoing(spell, SILENT_NO, 1);
        else
           return FALSE;
      }
      break;
    case LEARN_UNUSUAL_NONE:
      vlogf(LOG_BUG, format("Wierd case in learnFromDoingUnusual %s, type %d spell %d") %  getName() % type % spell);
      return FALSE;
  }
  return FALSE;
}

// flags 1 = linear/no discipline
int TMonster::learnFromDoing(spellNumT sknum, silentTypeT silent, unsigned int)
{
  return FALSE;
}

// flags |= 1 == forced learn
int TPerson::learnFromDoing(spellNumT sknum, silentTypeT silent, unsigned int flags)
{
  CSkill *sk;
  CDiscipline *assDiscipline, *discipline;
  int chanceDisc = 0, chanceAss = 0, discLearn = 0;
  char tString[256];

  if (isImmortal() || !desc || roomp->isRoomFlag(ROOM_ARENA)) {
    return FALSE;
  }


  if (!discArray[sknum] || 
      !*discArray[sknum]->name || 
      discArray[sknum]->startLearnDo == -1) {
    return FALSE;
  }
  if (!doesKnowSkill(sknum)) {
    return FALSE;
  }

  sk = getSkill(sknum);
  if (!sk) {
    return FALSE;
  }

  int actual = getRawNatSkillValue(sknum);
  if (actual < getMaxSkillValue(sknum)) {
    learnAttemptLog(this, sknum);
    learnLearnednessLog(this, sknum, actual);
  }
  int boost = 1;

  // this prevents them from gaining further without a minimum wait between
  // increases.
  // since the chance (below) drops off as they approach max, we don't want
  // them to wait too long or they will never gain some skills
  //
  // allow them to boost the skill up to a "usable" amount without problems.
  //

  if (actual <= 15) {
    if ((time(0) - sk->lastUsed) < (1 * SECS_PER_REAL_MIN))  {
      return FALSE;
    }
  } else if ((time(0) - sk->lastUsed) < (5 * SECS_PER_REAL_MIN))  {
    return FALSE;
  }

// DISCIPLINE LEARN BY DOING FIRST- PLAYERS DO *NOT* SEE THIS
// first set learning rates
// discipline and assDiscipline are used later to hold discipline number
// here they are just used to hold if the learning should take place

  if (discArray[sknum]->disc == DISC_COMBAT ||
        discArray[sknum]->disc == DISC_SLASH ||
        discArray[sknum]->disc == DISC_BLUNT ||
        discArray[sknum]->disc == DISC_PIERCE ||
        discArray[sknum]->disc == DISC_RANGED ||
        discArray[sknum]->disc == DISC_BAREHAND) {
    chanceDisc = ::number(0,200);
    chanceAss = ::number(0, 400);  
  } else if (discArray[sknum]->assDisc == discArray[sknum]->disc) {
    // advanced discipline here
    chanceDisc = ::number(0,150);
    chanceAss = 1;  // no chance of learn 
  } else {
    chanceDisc = ::number(0, 200);
    chanceAss = ::number(0,400);
  }

  if ((actual >= MAX_SKILL_LEARNEDNESS) || 
      (actual >= getMaxSkillValue(sknum))) {
    chanceDisc = (max(0, chanceDisc - 25));
    chanceAss = (max(0, chanceAss - 25));
  }


  if (!chanceDisc) {
  //   do skill's disc learning here COSMO MARKER
    if (!(discipline = getDiscipline(discArray[sknum]->disc))) {
#if DISC_DEBUG
      vlogf(LOG_SILENT, format("(%s) has a skill (%d) but doesnt have the discipline") %  getName() % sknum);
#endif
      return FALSE;
    } 
    discLearn = discipline->getDoLearnedness();
    discLearn = max(1, discLearn);
    discLearn = min(100, discLearn + 1);
    if (discLearn < 100) {
      discipline->setDoLearnedness(discLearn);
#if DISC_DEBUG
      vlogf(LOG_SILENT, format("%s just learned something in %s, Learn = %d.") %  getName() % discNames[(discArray[sknum]->assDisc)].properName % discLearn); 
#endif
    }
  }
  if (!chanceAss) {
    if (!(assDiscipline = getDiscipline(discArray[sknum]->assDisc))) {
#if DISC_DEBUG
      vlogf(LOG_SILENT, format("(%s) has a skill (%d) but doesnt have the assDisc") %  getName() % sknum);
#endif
      return FALSE;
    }
    discLearn = assDiscipline->getDoLearnedness();
    discLearn = max(1, discLearn);
    discLearn = min(100, discLearn + 1);
    if (discLearn < 100) {
      assDiscipline->setDoLearnedness(discLearn);
#if DISC_DEBUG
      vlogf(LOG_SILENT, format("%s just learned something in %s, Learn = %d.") %  getName() % discNames[(discArray[sknum]->assDisc)].properName % discLearn); 
#endif
    }
  }

// SKILL LEARNING NOW

  if (actual >= getMaxSkillValue(sknum)) {
    return FALSE;
  }

  if (IS_SET(flags, 1U)) {
// learn
  } else {
    const int max_amt = MAX_SKILL_LEARNEDNESS;
    float amount = ((float) max_amt - (float) actual) / ((float) max_amt);
#if DISC_DEBUG
    vlogf(LOG_SILENT, format("learnFromDoing (%s) amt(%f) max(%d) actual(%d)") %  discArray[sknum]->name % amount % max_amt % actual);
#endif

  // some basic background on how this was formulated.
  // let y = f(amount) = the percentage chance we raise the skill 1%
  // given that amount is calculated as above, it goes from 0.0 to 1.0
  // ie. amount = 0 (skill at current max), amount = 1.0 (skill at 0).
  // we desire f(1.0) = 100% and f(0.0) = 0%
  // additionally, to have f(amount) to have the form amount ^ power
  // seems to make sense.
  // thus
  // y = A x ^ B + C     : amount = x
  // C = 0 from f(0.0) = 0
  // A = 100% from f(1.0)
  // y = 100% * amount ^ (B)
  // we desire B to be in range 1.0 (high wis) to 3.5 (low wis) 
  // solving for a linear formula, gave slope of (-1/60) and intersect of 4
    float power;
    power = 4.0 - ( plotStat(STAT_NATURAL, STAT_WIS, 0.5, 3.0, 1.75, 1.0));
    int chance = (int) (1000.0 * (pow(amount, power)));

    // make a minimum chance of increase.
    if (amount > 0.0)
      chance = max(10, chance);


    if (::number(0, 999) >= chance)
      return FALSE;
  }

#if 1
  if (!silent) {
    if ((discArray[sknum]->comp_types & COMP_MATERIAL))
      strcpy(tString, "feel you have more control over the powers of");
    else if (discArray[sknum]->holyStrength) {
      sstring tStDeity("");

      tStDeity = yourDeity(sknum, FIRST_PERSON);
      sprintf(tString, "feel %s favoring you more in respects to",
              tStDeity.c_str());
    } else
      strcpy(tString, "feel your skills honing in regards to");

    sendTo(COLOR_BASIC, format("<c>You %s %s.<z>\n\r") % tString % discArray[sknum]->name);
  }
#else
  if (!silent)
    sendTo(COLOR_BASIC, format("<c>You increase your mastery of %s.<z>\n\r") % discArray[sknum]->name);
#endif

  // boost at this point is 1, now make it more it if appropriate
  if (discArray[sknum]->amtLearnDo > 1) {
    boost = discArray[sknum]->amtLearnDo;
    if (actual >= 90) 
      boost = 1;
    else if ((actual + boost) >= 90) 
      boost = 90 - actual;
  }
#if DISC_DEBUG
  vlogf(LOG_SILENT, format("learnFromDoing (%s)(%d): actual (%d), boost (%d)") %  discArray[sknum]->name % sknum % actual % boost);
#endif
  setSkillValue(sknum, getSkillValue(sknum) + boost);
  setNatSkillValue(sknum, actual + boost);

  if(hasQuestBit(TOG_STARTED_MONK_RED) && !hasQuestBit(TOG_FINISHED_MONK_RED)){
    if(getNatSkillValue(SKILL_SLASH_PROF) >= 20 &&
       getNatSkillValue(SKILL_BLUNT_PROF) >= 20 &&
       getNatSkillValue(SKILL_PIERCE_PROF) >= 20 &&
       getNatSkillValue(SKILL_RANGED_PROF) >= 20){
      sendTo(COLOR_BASIC, "<c>You are now proficient enough with weapons to earn your red sash.<z>");
      setQuestBit(TOG_FINISHED_MONK_RED);
    } else if(getNatSkillValue(sknum) >= 20 &&
	      (sknum == SKILL_SLASH_PROF || sknum == SKILL_BLUNT_PROF ||
	       sknum == SKILL_PIERCE_PROF || sknum == SKILL_RANGED_PROF)){
      sendTo(COLOR_BASIC, format("<c>You feel that you have enough knowledge of %s to please your guildmaster.<z>") % discArray[sknum]->name);
    }
  }
  
  sk->lastUsed = time(0);
  learnSuccessLog(this, sknum, boost);

  if (getNatSkillValue(sknum) == 100) {
    if ((discArray[sknum]->comp_types & COMP_MATERIAL))
      strcpy(tString, "feel you have total control over the powers of");
    else if (discArray[sknum]->holyStrength) {
      sstring tStDeity("");

      tStDeity = yourDeity(sknum, FIRST_PERSON);
      sprintf(tString, "feel %s has blessed you fully with the powers of", tStDeity.c_str());
    } else
      strcpy(tString, "feel you have total mastery over");

    if (!silent)
      sendTo(COLOR_BASIC, format("<c>You %s %s.<z>\n\r") % tString % discArray[sknum]->name);

    if (doesKnowSkill(SKILL_KICK_MONK) && sknum == SKILL_KICK_MONK) {
      setQuestBit(TOG_ELIGIBLE_ADVANCED_KICKING);

      sendTo(COLOR_BASIC, "<c>Perhaps your guildmaster could help you with <p>advanced kicking<c> now.<1>\n\r");
    }
  } else if (getNatSkillValue(sknum) == getMaxSkillValue(sknum)) {
    if ((discArray[sknum]->comp_types & COMP_MATERIAL))
      strcpy(tString, "feel you have all the control you can currently obtain of");
    else if (discArray[sknum]->holyStrength) {
      sstring tStDeity("");

      tStDeity = yourDeity(sknum, FIRST_PERSON);
      sprintf(tString, "feel %s refuses to bless you more, for now, in respects to", tStDeity.c_str());
    } else
      strcpy(tString, "feel you have all the control you can currently have over");

    if (!silent)
      sendTo(COLOR_BASIC, format("<c>You %s %s.<z>\n\r") % tString % discArray[sknum]->name);
  }

  return TRUE;
}

void TBeing::addSkillLag(spellNumT skill, int rc)
{
  // rc is just a playerfavorable way to end lag when fight is over
  // We do this already in places. This makes it general.
  // If there is any issue just use 0 for rc and it will put in regular lag
  lag_t lag_num = discArray[skill]->lag;
  float f_lag  = lagAdjust(lag_num),
        f_base = 1.0;
#if 1 
  if (IS_SET_DELETE(rc, DELETE_VICT))
    f_lag = min(f_base, f_lag);
#else
    f_base = 2;
#endif
  f_lag *= combatRound(1);
  int i_lag = static_cast<int>(f_lag);

  // we wind up truncating, might want to randomize this based on remainder
  addToWait(i_lag);
}

CMasterDiscipline::CMasterDiscipline()
{
  for (discNumT i = MIN_DISC; i < MAX_DISCS; i++)
    disc[i] = NULL;
}

CMasterDiscipline::CMasterDiscipline(const CMasterDiscipline &a)
{
  for (discNumT i = MIN_DISC; i < MAX_DISCS; i++) {
    if (a.disc[i])
      disc[i] = a.disc[i]->cloneMe();
    else
      disc[i] = NULL;
  }
}

CMasterDiscipline & CMasterDiscipline::operator=(const CMasterDiscipline &a)
{
  if (this == &a) return *this;
  for (discNumT i = MIN_DISC; i < MAX_DISCS; i++) {
    delete disc[i];
    if (a.disc[i])
      disc[i] = a.disc[i]->cloneMe();
    else
      disc[i] = NULL;
  }
  return *this;
}

CMasterDiscipline::~CMasterDiscipline()
{
  for (discNumT i = MIN_DISC; i < MAX_DISCS; i++) {
    delete disc[i];
    disc[i] = NULL;
  }
}

CDiscipline::CDiscipline() :
  uNatLearnedness(0),
  uLearnedness(0),
  uDoLearnedness(0),
  ok_for_class(0)
{
}

CDiscipline::CDiscipline(const CDiscipline &a) :
  uNatLearnedness(a.uNatLearnedness), 
  uLearnedness(a.uLearnedness),
  uDoLearnedness(a.uDoLearnedness),
  ok_for_class(a.ok_for_class)
{
}

CDiscipline & CDiscipline::operator= (const CDiscipline &a)
{
  if (this == &a) return *this;
  uNatLearnedness = a.uNatLearnedness;
  uLearnedness = a.uLearnedness;
  uDoLearnedness = a.uDoLearnedness;
  ok_for_class = a.ok_for_class;
  return *this;
}

CDiscipline::~CDiscipline()
{
}
