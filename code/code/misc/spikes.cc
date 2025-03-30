
#include <stdio.h>

#include <cmath>

#include "discipline.h"
#include "handler.h"
#include "extern.h"
#include "immunity.h"
#include "room.h"
#include "being.h"
#include "client.h"
#include "low.h"
#include "colorstring.h"
#include "monster.h"
#include "configuration.h"
#include "materials.h"
#include "range.h"
#include "combat.h"
#include "statistics.h"
#include "person.h"
#include "disease.h"
#include "mail.h"
#include "shop.h"
#include "database.h"
#include "obj_money.h"
#include "obj_trash.h"
#include "obj_arrow.h"
#include "obj_general_weapon.h"
#include "obj_base_weapon.h"
#include "obj_gun.h"
#include "obj_handgonne.h"
#include "obj_base_clothing.h"
#include "cmd_trophy.h"
#include "obj_base_cup.h"
#include "rent.h"
#include "stats.h"
/*
/// checking for the possibility of causing a bleed on victim
  bool LimbCanBleed = !victim->isLimbFlag(limb, PART_MISSING) && !victim->isUndead() && !victim->isImmune(IMMUNE_BLEED, limb);  return LimbCanBleed;
  }
    /// checking to see if weapon is sharp and if mob is tough
  bool LimbGetsHurt = !victim->isTough() && (weapon->getCurSharp()>=weapon->getMaxSharp()); return LimbGetsHurt;
  }
  /// checking limb health and bleeding and bruising
  bool LimbCanBeSevered = !victim->isLimbFlag(limb, PART_MISSING) && (victim->isLimbFlag(limb, PART_BLEEDING) || victim->isLimbFlag(limb, PART_BRUISED)) && victim ->getCurHealt(limb) >= victim->(getMaxHealth(limb)/2)); return LimbCanBeSevered;
  }

int spikesHit(TBeing* victim, TBeing* ch, TObj* obj) {
   if (!victim)
    return false;
   bool LimbCanBleed = !victim->isLimbFlag(limb, PART_MISSING) && !victim->isUndead() && !victim->isImmune(IMMUNE_BLEED, limb);  return LimbCanBleed;
  }
  if (victim->LimbCanBleed(limb) &&)(obj->isObjStat(ITEM_SPIKED)){
  victim->rawBleed(limb, 250, SILENT_YES, CHECK_IMMUNITY_NO);
  act("A <r>bloody wound<1> opens as the spikes on $o shred $N's %s!", false, ch, obj, victim, TO_NOTVICT);
  act("A <r>bloody wound<1> opens as the spikes on $o shred your $s!", false, ch, obj, victim, TO_VICT);
  }
  return false;
}

int spikesHitLimb(TBeing* victim, TBeing* ch, TObj* obj) {
  wearSlotT limb = vict->getPartHit(ch, false);
  if (limb == WEAR_NONE)
    return false;
  if (victim->LimbCanBleed(limb) &&)(obj->isObjStat(ITEM_SPIKED)) (
  victim->rawBleed(limb, 250, SILENT_YES, CHECK_IMMUNITY_NO);
  act("A <r>bloody wound<1> opens as the spikes on $o shred $N's %s!", false, ch, obj, victim, TO_NOTVICT);
  act("A <r>bloody wound<1> opens as the spikes on $o shred your $s!", false, ch, obj, victim, TO_VICT);
  )
  return false;
}
  int spikesBreak(TBeing* victim, TBeing* thief, TObj* obj) {
   TObj* obj;
    if (!obj || !percentChance(50));
    return false;
    if ((obj->isObjStat(ITEM_SPIKED)) && victim->isTough()) {
     int dam = number::(1,4);
     obj->addCurrentStructPoints(-dam);
     obj->addMaxStructPoints(-1);
      if obj->getMaxStructPoints() <= 0) {
       obj-makeBroken();
       act("$p shatters as its spikes break off!", false, ch, obj, TO_ROOM);
       return DELETE_OBJ;
      }
    auto* weapon = dynamic_cast<TGenWeapon*>(obj);
       weapon->addCurrentSharp(-dam);
       weapon->addMaxSharp(-1);
        if (weapon->getMaxSharp() <= 0) {
         weapon->makeBroken();
         act("$p shatters as its spikes break off!", false, ch, obj, TO_ROOM);
        return DELETE_OBJ;
        }
      }
    act("The impact of the blow degrades the quality of your $o.", false, thief, obj, TO_CHAR);
    if (!obj || percentChance(25)) {
      obj->removeObjStat(ITEM_SPIKED);
      act("The jagged spikes fall from $o. It no longer looks as brutal as it did before.", false, thief, obj, TO_CHAR);
    }
    }
    */