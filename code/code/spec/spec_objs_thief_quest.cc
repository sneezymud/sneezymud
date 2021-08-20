#include <stdio.h>

#include "being.h"
#include "comm.h"
#include "extern.h"
#include "obj_base_weapon.h"

int thiefQuestWeapon(TBeing *victim, cmdTypeT command, const char *arg, TObj *object, TObj *) {
  wearSlotT limb = WEAR_NOWHERE;

  if (command != CMD_STAB && command != CMD_BACKSTAB && command != CMD_SLIT)
    return false;

  if (command == CMD_STAB)
    limb = static_cast<wearSlotT>(reinterpret_cast<uintptr_t>(arg));

  if (command != CMD_STAB && strcmp(arg, "-special-"))
    return false;

  if (!victim || !object)
    return false;

  limb = command == CMD_BACKSTAB
      ? WEAR_BACK
      : command == CMD_SLIT
      ? WEAR_NECK
      : limb;

  if (limb == WEAR_NOWHERE || limb == HOLD_RIGHT || limb == HOLD_LEFT || limb == MAX_WEAR)
    return false;

  sstring limbDescription = victim->describeBodySlot(limb);

  // 10% chance on stab, 50% on backstab/slit
  if (command == CMD_STAB) {
    if (::number(0, 9))
      return false;
  } else if (::number(0, 1))
    return false;

  auto weapon = dynamic_cast<TBaseWeapon *>(object);

  auto thief = weapon
      ? dynamic_cast<TBeing *>(weapon->equippedBy)
      : nullptr;

  if (!weapon || !thief)
    return false;

  int damage = 0;
  spellNumT damageType = SKILL_STABBING;

  if (command == CMD_STAB) {
    damage = thief->GetMaxLevel() * 1.5;
  } else if (command == CMD_BACKSTAB) {
    damage = thief->GetMaxLevel() * 2.5;
    damageType = SKILL_BACKSTAB;
    act("<W>The weapon sears down $N's spine!<z>", FALSE, thief, weapon, victim, TO_CHAR);
    act("<W>The weapon sears down $N's spine!<z>", FALSE, thief, weapon, victim, TO_ROOM);
  } else {
    damage = thief->GetMaxLevel() * 3;
    damageType = SKILL_THROATSLIT;
    act("<W>The weapon sears through $N's throat!<z>", FALSE, thief, weapon, victim, TO_CHAR);
    act("<W>The weapon sears through $N's throat!<z>", FALSE, thief, weapon, victim, TO_ROOM);
  }

  int rc = thief->reconcileDamage(victim, damage, damageType);
  if (IS_SET_DELETE(rc, DELETE_VICT))
    return DELETE_VICT;

  if (victim->slotChance(limb) && !victim->isImmune(IMMUNE_BLEED, WEAR_BACK) &&
      !victim->isLimbFlags(limb, PART_BLEEDING) && !victim->isUndead()) {
    act("<r>Blood begins to pour from the wound!<z>", false, victim, nullptr, nullptr, TO_ROOM);
    victim->rawBleed(limb, thief->GetMaxLevel() * 3 + 100, SILENT_YES, CHECK_IMMUNITY_NO);
  }

  return true;
}
