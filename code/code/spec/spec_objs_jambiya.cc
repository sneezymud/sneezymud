#include "comm.h"
#include "obj_base_weapon.h"
#include "being.h"
#include "extern.h"

int weaponJambiyaSpecial(TBeing *tVictim, cmdTypeT tCmd, const char *tArg, TObj *tObj, TObj *)
{
  if (tCmd != CMD_STAB && tCmd != CMD_BACKSTAB)
    return FALSE;

  if (strcmp(tArg, "-special-") ||
      !tVictim || !tObj)
    return FALSE;

  TBaseWeapon *tWeapon;
  int          tDamage = 0;
  char         tToChar[256],
               tToRoom[256];
  TBeing      *tThief;
  spellNumT    tDamageType;

  if (!(tWeapon = dynamic_cast<TBaseWeapon *>(tObj)) ||
      !(tThief  = dynamic_cast<TBeing  *>(tObj->equippedBy)))
    return FALSE;

  bool forceSuccess = !strcmp(tThief->getName(), "Lapsos");

  if (::number(0, 100) && !forceSuccess)
    return FALSE;

  if (tCmd == CMD_STAB) {
    tDamage = max(1, min(50, (int) (tWeapon->getWeapDamLvl() / 5)));
    tDamageType = SKILL_STABBING;

    sprintf(tToChar, "Your $o slides deep into $N.");
    sprintf(tToRoom, "$n's $o slides deep into $N.");
  } else {
    tDamage = max(1, min(50, (int) (tWeapon->getWeapDamLvl() / 2)));
    tDamageType = SKILL_BACKSTAB;

    sprintf(tToChar, "Your $o sears $N down the spine.");
    sprintf(tToRoom, "$n's $o sears $N down the spine.");
  }

  tDamage = ::number(1, tDamage);

  act(tToChar, FALSE, tThief, tWeapon, tVictim, TO_CHAR);
  act(tToRoom, FALSE, tThief, tWeapon, tVictim, TO_ROOM);

  if (tThief->reconcileDamage(tVictim, tDamage, tDamageType) == -1)
    return DELETE_VICT;

  if (tCmd == CMD_BACKSTAB && !notBleedSlot(WEAR_BACK) &&
      !tVictim->isUndead() && tVictim->slotChance(WEAR_BACK) &&
      !tVictim->isImmune(IMMUNE_BLEED, WEAR_BACK) &&
      !tVictim->isLimbFlags(WEAR_BACK, PART_BLEEDING)) {
    sprintf(tToRoom, "Blood begins to pour from $n's %s!",
            tVictim->describeBodySlot(WEAR_BACK).c_str());
    act(tToRoom, FALSE, tVictim, NULL, NULL, TO_ROOM);
    tVictim->rawBleed(WEAR_BACK, (tWeapon->getWeapDamLvl() * 3) + 100,
                      SILENT_YES, CHECK_IMMUNITY_NO);
  }

  return TRUE;
}
