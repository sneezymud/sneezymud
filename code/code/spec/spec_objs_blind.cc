//////////////////////////////////////////////////////////////////////////
//
// SneezyMUD - All rights reserved, SneezyMUD Coding Team
//
//
//////////////////////////////////////////////////////////////////////////


#include "comm.h"
#include "obj_base_weapon.h"
#include "being.h"

/*
  Name: Blinder
  Type: Weapon
  Information:
    Base chance of 1 in 101 for call.
    Success based on level variation of user, weapon and victim.

    !::number(0, max(10, (weaponLevel + (victimLevel - playerLevel))));
    Weapon 25, Player 25, Victim 25: 0, .(1 in 26)., 25
    Weapon 25, Player 20, Victim 30: 0, .(1 in 36)., 35
    Weapon 25, Player 30, Victim 20: 0, .(1 in 16)., 15
    Weapon 50, Player 50, Victim  1: 0, .(1 in 11)., 10
    Weapon 50, Player  1, Victim 50: 0, .(1 in 75)., 74
 */

int weaponBlinder(TBeing *tVictim, cmdTypeT tCmd, const char *, TObj *tObj, TObj *)
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

  if (::number(0, 100) && !forceSuccess)
    return FALSE;

  if (tCmd != CMD_OBJ_HITTING)
    return FALSE;

  if (tVictim->affectedBySpell(SPELL_BLINDNESS) ||
      tVictim->isAffected(AFF_TRUE_SIGHT) ||
      tVictim->isAffected(AFF_CLARITY) ||
      ch->isNotPowerful(tVictim, (int)tWeap->weaponLevel(), SPELL_BLINDNESS, SILENT_YES))
    return FALSE;

  if (!::number(0, std::max(10, (int)(tWeap->weaponLevel() +
				      (tVictim->GetMaxLevel() -
				       ch->GetMaxLevel())))) || forceSuccess) {
    act("A Seering light shines from $p, blinding $N.",
        FALSE, ch, tObj, tVictim, TO_CHAR);
    act("$n shields $s eyes as a seering light shines from $p, blinding $N.",
        FALSE, ch, tObj, tVictim, TO_NOTVICT);
    act("The world goes white then black as a seering light shines from $n's $p.",
        FALSE, ch, tObj, tVictim, TO_VICT);

    int       tDuration = (int)(tWeap->weaponLevel() * UPDATES_PER_MUDHOUR);
    saveTypeT tSave     = SAVE_NO;

    tVictim->rawBlind((int)tWeap->weaponLevel(), tDuration, tSave);

    return TRUE;
  }

  return FALSE;
}
