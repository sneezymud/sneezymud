#include "comm.h"
#include "obj_base_weapon.h"
#include "extern.h"
#include "being.h"
#include "obj_base_weapon.h"

int starMetal(TBeing* vict, cmdTypeT cmd, const char*, TObj* o, TObj*) {
  TBeing* ch;
  int rc, dam;

  // Handle the generic pulse functionality
  if (cmd == CMD_GENERIC_PULSE) {
    if (!(ch = dynamic_cast<TBeing*>(o->equippedBy)))
      return FALSE;  // Not equipped
    
    // Case 1: Fighting a reptilian race and weapon not glowing
    if (ch->fight() && !o->isObjStat(ITEM_GLOW)) {
      TBeing* opponent = ch->fight();
      if (opponent && (opponent->getRace() == RACE_DRAGON || 
                      opponent->getRace() == RACE_PANTATH ||
                      opponent->getRace() == RACE_TROG || 
                      opponent->getRace() == RACE_SNAKE)) {
        // Add enhancement similar to enhanceWeapon
        int level = ch->GetMaxLevel();
        int bonus = level / 20 + 1;  // Similar to enhanceWeapon scaling
        
        // Find an empty affect slot
        for (int i = 0; i < MAX_OBJ_AFFECT; i++) {
          if (o->affected[i].location == APPLY_NONE) {
            // Add to-hit bonus
            o->affected[i].location = APPLY_HITROLL;
            o->affected[i].modifier = bonus;
            
            // Try to find another slot for damage bonus
            if (i + 1 < MAX_OBJ_AFFECT && o->affected[i + 1].location == APPLY_NONE) {
              o->affected[i + 1].location = APPLY_DAMROLL;
              o->affected[i + 1].modifier = bonus;
            }
            
            // Make the weapon glow
            o->addObjStat(ITEM_GLOW);
            
            act("$p begins to <C>GLOW<1> with power as it senses reptilian flesh nearby!", 
                FALSE, ch, o, NULL, TO_CHAR);
            act("$n's $p begins to <C>GLOW<1> with power!", 
                FALSE, ch, o, NULL, TO_ROOM);
            
            return TRUE;
          }
        }
      }
    }
    // Case 2: Not fighting, remove glow and applies
    else if (!ch->fight() && o->isObjStat(ITEM_GLOW)) {
      // Remove all applies except armor
      bool found = false;
      for (int i = 0; i < MAX_OBJ_AFFECT; i++) {
        if ((o->affected[i].location != APPLY_NONE) &&
            (o->affected[i].location != APPLY_ARMOR)) {
          o->affected[i].location = APPLY_NONE;
          o->affected[i].modifier = 0;
          o->affected[i].modifier2 = 0;
          o->affected[i].bitvector = 0;
          found = true;
        }
      }
      
      if (found) {
        // Remove the glow
        o->remObjStat(ITEM_GLOW);
        
        act("The <C>glow<1> fades from $p as the battle ends.", 
            FALSE, ch, o, NULL, TO_CHAR);
        act("The <C>glow<1> fades from $n's $p.", 
            FALSE, ch, o, NULL, TO_ROOM);
        
        return TRUE;
      }
    }
    
    return FALSE;
  }
  
  // Handle the combat proc (when weapon hits)
  ch = genericWeaponProcCheck(vict, cmd, o, 3);
  if (!ch)
    return FALSE;
  if (vict->getRace() != RACE_DRAGON && vict->getRace() != RACE_PANTATH &&
      vict->getRace() != RACE_TROG && vict->getRace() != RACE_SNAKE)
    return FALSE;
  affectedData aff;

  dam = ::number(1, ch->GetMaxLevel());
  act("$p <c>glows with power<1> and crashes into $n, parting <g>scales<1> and <r>flesh!<1>", 0,
    vict, o, 0, TO_ROOM);
  act("$p <c>glows with power<1> and crashes into you, parting <g>scales<1> and <r>flesh!<1>", 0,
    vict, o, 0, TO_VICT);
  act("$p <c>glows with power<1> and crashes into $n, parting <g>scales<1> and <r>flesh!<1>", 0,
    vict, o, 0, TO_CHAR);
   
  if (dam > ch->GetMaxLevel() / 2) {
    // Only apply Faerie Fire if the victim isn't already affected
    if (!vict->affectedBySpell(SPELL_FAERIE_FIRE)) {
      aff.type = SPELL_FAERIE_FIRE;
      aff.level = ch->GetMaxLevel();
      aff.location = APPLY_ARMOR;
      aff.bitvector = 0;

      // we'd like it to last about 5 minutes
      aff.duration = ch->durationModify(SPELL_FAERIE_FIRE,
        5 * Pulse::UPDATES_PER_MUDHOUR / 2);

      // let the affect be level dependant
      aff.modifier = ::number(50, 100 + (aff.level * 3));
      vict->affectJoin(ch, &aff, AVG_DUR_NO, AVG_EFF_NO);

      // Use the exact same pattern as in the working faerieFire function
      act("$N is surrounded in an ominous <c>glow<1>!", TRUE, ch, NULL, vict, 
        TO_NOTVICT);  // Everyone except ch and vict
      act("$N is surrounded in an ominous <c>glow<1>!", TRUE, ch, NULL, vict,
        TO_CHAR);     // Only ch sees this
      act("You are surrounded in an ominous <c>glow<1>!", FALSE, ch, NULL, vict,
        TO_VICT);     // Only vict sees this
    } else {
      // If already affected, just mention that the glow intensifies briefly
      affectedData* existingFaerie = vict->affected->find_if([](affectedData* aff) {
  return aff->type == SPELL_FAERIE_FIRE && aff->location == APPLY_ARMOR;
  });
  int origAmt = vict->specials.affectedBy;
  vict->affectModify(existingFaerie->location, existingFaerie->modifier, 
    existingFaerie->modifier2, existingFaerie->bitvector, FALSE, SILENT_YES);
    existingFaerie->modifier += ::number(25, 50) + (ch->GetMaxLevel() * 2);
    vict->affectModify(existingFaerie->location, existingFaerie->modifier, 
      existingFaerie->modifier2, existingFaerie->bitvector, TRUE, SILENT_YES);
    vict->affectTotal();
    vict->affectChange(origAmt, SILENT_YES);
      act("The <c>glow<1> around $N <y>flares<1> briefly!", TRUE, ch, NULL, vict, 
        TO_NOTVICT);
      act("The <c>glow<1> around $N <y>flares<1> briefly!", TRUE, ch, NULL, vict, 
        TO_CHAR);
      act("The <c>glow<1> around you <y>flares<1> briefly!", FALSE, ch, NULL, vict, 
        TO_VICT);
    }
  }
    
  rc = ch->reconcileDamage(vict, dam, DAMAGE_NORMAL);
  if (IS_SET_DELETE(rc, DELETE_VICT))
    return DELETE_VICT;
  return TRUE;
}
