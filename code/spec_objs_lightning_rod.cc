/*****************************************************************************

  SneezyMUD++ - All rights reserved, SneezyMUD Coding Team.

  "spec_objs_lightning_rod.cc"
  All functions and routines related to the Lightning Rod Prock

  Created 10/30/99 - Lapsos(William A. Perrotto III)

******************************************************************************/

#include "stdsneezy.h"

int lightningRodFryPerson  (TBaseWeapon *, TBeing *, TBeing *);
int lightningRodGotHit     (TBaseWeapon *, TBeing *, TBeing *);
int lightningRodFryRoom    (TBaseWeapon *, TRoom *);
int lightningRodInternalFry(TBaseWeapon *, TBeing *);

int weaponLightningRod(TBeing *tVictim, cmdTypeT tCmd, const char *, TObj *tObj, TObj *)
{
  TBaseWeapon *tWeapon;

  if (!(tWeapon = dynamic_cast<TBaseWeapon *>(tObj)))
    return FALSE;

  switch (tCmd) {
    case CMD_OBJ_HIT:
      if (!tObj->equippedBy || !dynamic_cast<TBeing *>(tObj->equippedBy) || !tVictim)
        return FALSE;

      return lightningRodFryPerson(tWeapon, dynamic_cast<TBeing *>(tObj->equippedBy), tVictim);

    case CMD_OBJ_BEEN_HIT:
      if (!tObj->equippedBy || !dynamic_cast<TBeing *>(tObj->equippedBy) || !tVictim)
        return FALSE;

      return lightningRodGotHit(tWeapon, dynamic_cast<TBeing *>(tObj->equippedBy), tVictim);

    case CMD_GENERIC_PULSE:
      if (!tObj || tObj->parent || tObj->stuckIn || tObj->equippedBy || !tObj->roomp)
        return FALSE;

      return lightningRodFryRoom(tWeapon, tObj->roomp);

    case CMD_OBJ_STUCK_IN:
      if (!tObj || !tVictim)
        return FALSE;

      return lightningRodInternalFry(tWeapon, tVictim);

    default:
      break;
  }

  return FALSE;
}

// In-Battle swing hit.
int lightningRodFryPerson(TBaseWeapon *tObj, TBeing *tMaster, TBeing *tSucker)
{
  if (::number(0, 10))
    return FALSE;

  /*
  vlogf(10, "Lightning Rod: Fry Person");
  sendrpf(tMaster->roomp, "Something is getting cooked!!!\n\r");
  */

  act("$p glows violently as sparks begin to leap from it.",
      TRUE, tMaster, tObj, tSucker, TO_ROOM);
  act("You feel volts of elecriticy eminating from $p flood your body!",
      TRUE, tMaster, tObj, tSucker, TO_VICT);
  act("$N is lit up like a lightbulb as $p strikes them!",
      TRUE, tMaster, tObj, tSucker, TO_NOTVICT);

  int tDamage = (int) (tObj->getWeapDamLvl() / 8.0);

  tDamage = ::number(1, tDamage);

  if (tMaster->reconcileDamage(tSucker, tDamage, DAMAGE_ELECTRIC) == -1)
    return DELETE_VICT;

  return TRUE;
}

// Object has been hit by something else.
int lightningRodGotHit(TBaseWeapon *tObj, TBeing *tMaster, TBeing *tSucker)
{
  if (::number(0, 10))
    return FALSE;

  /*
  vlogf(10, "Lightning Rod: Fry Hitter");
  sendrpf(tMaster->roomp, "Something is getting cooked!!!\n\r");
  */

  act("$p reacts violenty to being struck!",
      FALSE, tMaster, tObj, tSucker, TO_ROOM);
  act("Volts of electricty rise from $p and strike you in self defense!",
      FALSE, tMaster, tObj, tSucker, TO_VICT);
  act("$N is lit up like a lightbulb as $p counter attacks!",
      FALSE, tMaster, tObj, tSucker, TO_NOTVICT);

  int tDamage = (int) (tObj->getWeapDamLvl() / 8.0);

  tDamage = ::number(1, tDamage);

  if (IS_SET_DELETE(tMaster->reconcileDamage(tSucker, tDamage, DAMAGE_ELECTRIC), DELETE_VICT))
    return DELETE_VICT;

  return TRUE;
}

// Generic in-room without master fry anything.
int lightningRodFryRoom(TBaseWeapon *tObj, TRoom *tRoom)
{
  if (tRoom->getWeather() != WEATHER_LIGHTNING || tRoom->isRoomFlag(ROOM_INDOORS) || ::number(0, 100))
    return FALSE;

  /*
  vlogf(10, "Lightning Rod: Fry Anyone");
  sendrpf(tRoom, "Something is getting cooked!!!\n\r");
  */

  act("A bolt of lightning streaks down and strikes $p!",
      FALSE, NULL, tObj, NULL, TO_ROOM);

  TThing *tThing,
         *tThingNext;
  TBeing *tBeing;

  for (tThing = tRoom->stuff; tThing; tThing = tThingNext) {
    tThingNext = tThing;

    if (!(tBeing = dynamic_cast<TBeing *>(tThing)) || !::number(0, 3))
      continue;

    act("A stream of energy launches from $p, frying you!",
        FALSE, tBeing, tObj, NULL, TO_CHAR);
    act("A stream of energy launches from $p, frying $n!",
        FALSE, tBeing, tObj, NULL, TO_ROOM);

    int tDamage = (int) (tObj->getWeapDamLvl() / 4.0);

    tDamage = ::number(1, tDamage);

    if (tBeing->reconcileDamage(tBeing, tDamage, DAMAGE_ELECTRIC) == -1) {
      --(*tBeing);
      delete tBeing;
      tBeing = NULL;
    }
  }

  return TRUE;
}

// Stuck in fry our current host.
int lightningRodInternalFry(TBaseWeapon *tObj, TBeing *tSucker)
{
  if (::number(0, 10))
    return FALSE;

  /*
  vlogf(10, "Lightning Rod: Fry Sucker");
  sendrpf(tSucker->roomp, "Something is getting cooked!!!\n\r");
  */

  act("$p suddenly flares up violently!",
      FALSE, tSucker, tObj, tSucker, TO_ROOM);
  act("$p sinks a little deeper and releases volts of energy!",
      FALSE, tSucker, tObj, tSucker, TO_CHAR);

  int tDamage = (int) (tObj->getWeapDamLvl() / 8.0);

  tDamage = ::number(1, tDamage);

  if (tSucker->reconcileDamage(tSucker, tDamage, DAMAGE_ELECTRIC) == -1)
    return DELETE_VICT;

  return TRUE;
}
