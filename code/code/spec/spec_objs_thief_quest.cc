#include <stdio.h>

#include "being.h"
#include "comm.h"
#include "extern.h"
#include "obj_base_weapon.h"

int thiefQuestWeapon(TBeing *victim, cmdTypeT command, const char *arg, TObj *object, TObj *) {
  if (command != CMD_STAB && command != CMD_BACKSTAB && command != CMD_SLIT) return false;
  if (strcmp(arg, "-special-") && command != CMD_STAB) return false;
  if (!victim || !object) return false;

  auto limb = command == CMD_BACKSTAB ? WEAR_BACK
              : command == CMD_SLIT   ? WEAR_NECK
                                      : limbStringToEnum(arg);

  if (limb == WEAR_NOWHERE || limb == HOLD_RIGHT || limb == HOLD_LEFT || limb == MAX_WEAR)
    return false;

  auto limbDescription = victim->describeBodySlot(limb);

  // 50% chance to proc on successful special attack
  if (!::number(0, 1)) return false;

  auto weapon = dynamic_cast<TBaseWeapon *>(object);
  auto thief = weapon ? dynamic_cast<TBeing *>(weapon->equippedBy) : nullptr;
  if (!weapon || !thief) return false;

  auto damage = 0;
  auto damageType = SKILL_STABBING;

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
  if (IS_SET_DELETE(rc, DELETE_VICT)) return DELETE_VICT;

  if (victim->slotChance(limb) && !victim->isImmune(IMMUNE_BLEED, WEAR_BACK) &&
      !victim->isLimbFlags(limb, PART_BLEEDING) && !victim->isUndead()) {
    act("<r>Blood begins to pour from the wound!<z>", false, victim, nullptr, nullptr, TO_ROOM);
    victim->rawBleed(limb, thief->GetMaxLevel() * 3 + 100, SILENT_YES, CHECK_IMMUNITY_NO);
  }

  return true;
}
