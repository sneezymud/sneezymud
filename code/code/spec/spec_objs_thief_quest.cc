#include <stdio.h>

#include "being.h"
#include "comm.h"
#include "extern.h"
#include "obj_base_weapon.h"

int thiefQuestWeapon(TBeing *victim, cmdTypeT command, const char *arg, TObj *object, TObj *thief) {
  // Thief will only == nullptr when proc is called on the generic command proc
  // check. Don't want proc to execute in that instance - should only happen in
  // response to skill use. Otherwise it'll execute twice as often as it should.
  if ((command != CMD_STAB && command != CMD_BACKSTAB && command != CMD_SLIT) || !thief || !victim || !object)
    return false;

  auto *weapon = dynamic_cast<TBaseWeapon *>(object);
  auto *wielder = weapon ? dynamic_cast<TBeing *>(weapon->equippedBy) : nullptr;

  // Cast both thief and wielder to base class and check if both pointing to same
  // memory location
  if (!weapon || !wielder || !(dynamic_cast<TThing *>(thief) == dynamic_cast<TThing *>(wielder)))
    return false;

  wearSlotT limb = WEAR_NOWHERE;

  if (command == CMD_STAB)
    limb = static_cast<wearSlotT>(reinterpret_cast<uintptr_t>(arg));

  limb = command == CMD_BACKSTAB ? WEAR_BACK : command == CMD_SLIT ? WEAR_NECK : limb;

  if (limb == WEAR_NOWHERE || limb == HOLD_RIGHT || limb == HOLD_LEFT || limb == MAX_WEAR)
    return false;

  // 10% chance on stab, 50% on backstab/slit
  if (command == CMD_STAB) {
    if (::number(0, 9))
      return false;
  } else if (::number(0, 1))
    return false;

  int damage = 0;
  spellNumT damageType = SKILL_STABBING;

  if (command == CMD_STAB) {
    damage = wielder->GetMaxLevel() * 1.5;
  } else if (command == CMD_BACKSTAB) {
    damage = wielder->GetMaxLevel() * 2.5;
    damageType = SKILL_BACKSTAB;
    act("<W>The weapon sears down $N's spine!<z>", false, wielder, weapon, victim, TO_CHAR);
    act("<W>The weapon sears down $N's spine!<z>", false, wielder, weapon, victim, TO_ROOM);
  } else {
    damage = wielder->GetMaxLevel() * 3;
    damageType = SKILL_THROATSLIT;
    act("<W>The weapon sears through $N's throat!<z>", false, wielder, weapon, victim, TO_CHAR);
    act("<W>The weapon sears through $N's throat!<z>", false, wielder, weapon, victim, TO_ROOM);
  }

  int rc = wielder->reconcileDamage(victim, damage, damageType);
  if (IS_SET_DELETE(rc, DELETE_VICT))
    return DELETE_VICT;

  if (victim->slotChance(limb) && !victim->isImmune(IMMUNE_BLEED, WEAR_BACK) &&
      !victim->isLimbFlags(limb, PART_BLEEDING) && !victim->isUndead()) {
    act("<r>Blood begins to pour from the wound!<z>", false, victim, nullptr, nullptr, TO_ROOM);
    victim->rawBleed(limb, wielder->GetMaxLevel() * 3 + 100, SILENT_YES, CHECK_IMMUNITY_NO);
  }

  return true;
}
