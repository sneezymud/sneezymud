#include <stdio.h>

#include "being.h"
#include "comm.h"
#include "extern.h"
#include "obj_base_weapon.h"

int thiefQuestWeapon(TBeing* victim, cmdTypeT command, const char* arg,
  TObj* object, TObj* thief) {
  // Thief will only == nullptr when proc is called on the generic command proc
  // check. Don't want proc to execute in that instance - should only happen in
  // response to skill use. Otherwise it'll execute twice as often as it should.
  if ((command != CMD_STAB && command != CMD_BACKSTAB && command != CMD_SLIT) ||
      !thief || !victim || !object)
    return false;

  auto* weapon = dynamic_cast<TBaseWeapon*>(object);

  // Double cast required because thief comes in having already been cast
  // from TBeing to TThing to TObj, and casting directly from TObj to TBeing
  // is not allowed (outside of reinterpret_cast).
  auto* thiefAsTBeing = dynamic_cast<TBeing*>(dynamic_cast<TThing*>(thief));

  // Verify stabber is wielding the object in their primary hand
  if (!weapon || !thiefAsTBeing || !(thiefAsTBeing->heldInPrimHand() == weapon))
    return false;

  wearSlotT limb = WEAR_NOWHERE;

  // This reinterpret_cast is necessary because there's no clean way to pass the
  // wearSlotT value of the stab location into the function, short of completely
  // changing the function signature of every object spec proc function.
  if (command == CMD_STAB)
    limb = static_cast<wearSlotT>(reinterpret_cast<uintptr_t>(arg));

  limb = command == CMD_BACKSTAB ? WEAR_BACK
         : command == CMD_SLIT   ? WEAR_NECK
                                 : limb;

  if (limb == WEAR_NOWHERE || limb == HOLD_RIGHT || limb == HOLD_LEFT ||
      limb == MAX_WEAR)
    return false;

  // 12.5% chance on stab, 50% on backstab/slit
  if (command == CMD_STAB && ::number(0, 7))
    return false;

  if (::number(0, 1))
    return false;

  int damage = 0;
  spellNumT damageType = SKILL_STABBING;
  int level = thiefAsTBeing->GetMaxLevel();

  if (command == CMD_STAB) {
    damage = level * 1.5;
  } else if (command == CMD_BACKSTAB) {
    damage = level * 2.5;
    damageType = SKILL_BACKSTAB;
    act("<W>The weapon sears down $N's spine!<z>", false, thiefAsTBeing, weapon,
      victim, TO_CHAR);
    act("<W>The weapon sears down $N's spine!<z>", false, thiefAsTBeing, weapon,
      victim, TO_ROOM);
  } else {
    damage = level * 3;
    damageType = SKILL_THROATSLIT;
    act("<W>The weapon sears through $N's throat!<z>", false, thiefAsTBeing,
      weapon, victim, TO_CHAR);
    act("<W>The weapon sears through $N's throat!<z>", false, thiefAsTBeing,
      weapon, victim, TO_ROOM);
  }

  int rc = thiefAsTBeing->reconcileDamage(victim, damage, damageType);
  if (IS_SET_DELETE(rc, DELETE_VICT))
    return DELETE_VICT;

  if (victim->slotChance(limb) && !victim->isImmune(IMMUNE_BLEED, WEAR_BACK) &&
      !victim->isLimbFlags(limb, PART_BLEEDING) && !victim->isUndead()) {
    act("<r>Blood begins to pour from the wound!<z>", false, victim, nullptr,
      nullptr, TO_ROOM);
    victim->rawBleed(limb, level * 3 + 100, SILENT_YES, CHECK_IMMUNITY_NO);
  }

  return true;
}
