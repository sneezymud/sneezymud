/*****************************************************************************

  SneezyMUD++ - All rights reserved, SneezyMUD Coding Team.

  "spec_objs_lightning_rod.cc"
  All functions and routines related to the Lightning Rod Prock

  Created 10/30/99 - Lapsos(William A. Perrotto III)

******************************************************************************/

#include "stdsneezy.h"
#include "obj_base_weapon.h"
#include "obj_base_clothing.h"


int lightningRodFryPerson  (TBaseWeapon   *, TBeing *, TBeing *);
int lightningRodGotHit     (TBaseWeapon   *, TBeing *, TBeing *);
int lightningRodGotHit     (TBaseClothing *, TBeing *, TBeing *);
int lightningRodFryRoom    (TBaseWeapon   *, TRoom *);
int lightningRodInternalFry(TBaseWeapon   *, TBeing *);

int weaponLightningRod(TBeing *tVictim, cmdTypeT tCmd, const char *, TObj *tObj, TObj *)
{
  TBaseWeapon   *tWeapon = NULL;
  TBaseClothing *tArmor  = NULL;

  if (!(tWeapon = dynamic_cast<TBaseWeapon   *>(tObj)) &&
      !(tArmor  = dynamic_cast<TBaseClothing *>(tObj)))
    return FALSE;

  switch (tCmd) {
    case CMD_OBJ_HIT:
      if (!tObj->equippedBy || !dynamic_cast<TBeing *>(tObj->equippedBy) || !tVictim || !tWeapon)
        return FALSE;

      return lightningRodFryPerson(tWeapon, dynamic_cast<TBeing *>(tObj->equippedBy), tVictim);

    case CMD_OBJ_BEEN_HIT:
      if (!tObj->equippedBy || !dynamic_cast<TBeing *>(tObj->equippedBy) || !tVictim)
        return FALSE;

      if (tWeapon)
        return lightningRodGotHit(tWeapon, dynamic_cast<TBeing *>(tObj->equippedBy), tVictim);
      else
        return lightningRodGotHit(tArmor, dynamic_cast<TBeing *>(tObj->equippedBy), tVictim);

    case CMD_GENERIC_PULSE:
      if (!tObj || tObj->parent || tObj->stuckIn || tObj->equippedBy || !tObj->roomp || !tWeapon)
        return FALSE;

      return lightningRodFryRoom(tWeapon, tObj->roomp);

    case CMD_OBJ_STUCK_IN:
      if (!tObj || !tVictim || !tWeapon)
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

  sendrpf(COLOR_OBJECTS, tMaster->roomp,
          "%s<W> glows violently as sparks begin to leap from it.<z>\n\r",
          (tObj->getName() ? sstring(tObj->getName()).cap().c_str() : "Bogus Object"));
  tMaster->sendTo(COLOR_OBJECTS, fmt("%s<W> is lit up like a lightbulb!<z>\n\r") %
                  (tSucker->getName() ? sstring(tSucker->getName()).cap() : "Bogus Mobile"));
  tSucker->sendTo(COLOR_OBJECTS, "<W>You feel volts of electricty rush through your body!<z>\n\r");

  int tDamage = max(1, (int) (tObj->getWeapDamLvl() / 8.0));

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

  sendrpf(COLOR_OBJECTS, tMaster->roomp,
          "%s<W> glows violently in reaction to being struck!<z>\n\r",
          (tObj->getName() ? sstring(tObj->getName()).cap().c_str() : "Bogus Object"));
  tSucker->sendTo(COLOR_OBJECTS, "<W>Volts of electricty course through your body!<z>\n\r");

  int tDamage = max(1, (int) (tObj->getWeapDamLvl() / 8.0));

  tDamage = ::number(1, tDamage);

  if (IS_SET_DELETE(tMaster->reconcileDamage(tSucker, tDamage, DAMAGE_ELECTRIC), DELETE_VICT))
    return DELETE_VICT;

  return TRUE;
}

// Object has been hit by something else.
int lightningRodGotHit(TBaseClothing *tObj, TBeing *tMaster, TBeing *tSucker)
{
  if (::number(0, 10))
    return FALSE;

  sendrpf(COLOR_OBJECTS, tMaster->roomp,
          "%s<W> glows violently in reaction to being struck!<z>\n\r",
          (tObj->getName() ? sstring(tObj->getName()).cap().c_str() : "Bogus Object"));
  tSucker->sendTo(COLOR_OBJECTS, "<W>Volts of electricty course through your body!<z>\n\r");

  int tDamage = max(1, (int) (tObj->armorLevel(ARMOR_LEV_REAL) / 8.0));

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

  sendrpf(COLOR_OBJECTS, tRoom,
          "<W>A bolt of lightning streaks down and strikes %s<W>!<z>\n\r",
          (tObj->getName() ? sstring(tObj->getName()).uncap().c_str() : "Bogus Object"));

  TThing *tThing,
         *tThingNext;
  TBeing *tBeing;

  for (tThing = tRoom->getStuff(); tThing; tThing = tThingNext) {
    tThingNext = tThing->nextThing;

    if (!(tBeing = dynamic_cast<TBeing *>(tThing)) || !::number(0, 3))
      continue;


    tBeing->sendTo(COLOR_OBJECTS, fmt("<W>A stream of energy launches from %s<W>, frying you!<z>\n\r") %
                   (tObj->getName() ? sstring(tObj->getName()).uncap() : "Bogus Object"));

    int tDamage = max(1, (int) (tObj->getWeapDamLvl() / 4.0));

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
  if (::number(0, 3))
    return FALSE;

  sendrpf(COLOR_OBJECTS, tSucker->roomp,
          "%s<W> suddenly flares up violently!<z>\n\r",
          (tObj->getName() ? sstring(tObj->getName()).cap().c_str() : "Bogus Object"));
  tSucker->sendTo(COLOR_OBJECTS, "<W>Volts of energy course through your body!<z>\n\r");

  int tDamage = max(1, (int) (tObj->getWeapDamLvl() / 8.0));

  tDamage = ::number(1, tDamage);

  if (tSucker->reconcileDamage(tSucker, tDamage, DAMAGE_ELECTRIC) == -1)
    return DELETE_VICT;

  return TRUE;
}
