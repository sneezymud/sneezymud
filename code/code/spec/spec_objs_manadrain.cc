//////////////////////////////////////////////////////////////////////////
//
// SneezyMUD - All rights reserved, SneezyMUD Coding Team
//
//////////////////////////////////////////////////////////////////////////


#include "stdsneezy.h"
#include "obj_base_weapon.h"

/*
  Name: Mana Drainer
  Type: Weapon
  Information:
    Base chance of 1 in 51 for call.
    Success based on level variation of user, weapon and victim.

    !::number(0, max(10, (weaponLevel + (victimLevel - playerLevel))));
    Weapon 25, Player 25, Victim 25: 0, .(1 in 26)., 25
    Weapon 25, Player 20, Victim 30: 0, .(1 in 36)., 35
    Weapon 25, Player 30, Victim 20: 0, .(1 in 16)., 15
    Weapon 50, Player 50, Victim  1: 0, .(1 in 11)., 10
    Weapon 50, Player  1, Victim 50: 0, .(1 in 75)., 74
 */

int weaponManaDrainer(TBeing *tVictim, cmdTypeT tCmd, const char *, TObj *tObj, TObj *)
{
  TBeing      *ch;
  TBaseWeapon *tWeap;
  bool         forceSuccess = false;

  if (!(tWeap = dynamic_cast<TBaseWeapon *>(tObj)) || !tVictim)
    return FALSE;

  if (!(ch = dynamic_cast<TBeing *>(tObj->equippedBy)))
    return FALSE;

  if (!strcmp(ch->getName(), "Lapsos") && ch->isImmortal())
    forceSuccess = true;

  if (::number(0, 50) && !forceSuccess)
    return FALSE;

  if (tCmd != CMD_OBJ_HITTING)
    return FALSE;

  if ((!tVictim->hasClass(CLASS_MAGE) &&
       !tVictim->hasClass(CLASS_RANGER) &&
       !tVictim->hasClass(CLASS_MONK)) ||
      (!ch->hasClass(CLASS_MAGE) &&
       !ch->hasClass(CLASS_RANGER) &&
       !ch->hasClass(CLASS_MONK)))
    return FALSE;

  if (!::number(0, max(10, (int)(tWeap->weaponLevel() +
                                 (tVictim->GetMaxLevel() -
                                  ch->GetMaxLevel())))) || forceSuccess) {
    act("A field of darkness seeps from $p.",
        FALSE, ch, tObj, tVictim, TO_CHAR);
    act("$n braces $mself as a field of darkness seeps from $p.",
        FALSE, ch, tObj, tVictim, TO_NOTVICT);
    act("You feel your energy sucked out of you as a field of darkness seeps from $n's $p.",
        FALSE, ch, tObj, tVictim, TO_VICT);

    int manaDrawn = ::number(1, max(2, min(10, (ch->GetMaxLevel() - tVictim->GetMaxLevel()))));

    tVictim->addToMana(-manaDrawn);
    ch->addToMana(manaDrawn);

    return TRUE;
  }

  return FALSE;
}
