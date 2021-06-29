#include "being.h"
#include "obj_general_weapon.h"

// quest weapon for psionicists
//
// tracks whether it's extended by varying between WEAPON_TYPE_SLICE and _BLUNT

// TODO: make crit freq just be a multiple of the attackers base frequency
const int LIGHTSABER_CRITS_INFREQUENCY = 50;
const int LIGHTSABER_NOISE_INFREQUENCY = 10;


const int LS_CRIT_LIST[] = {67,68,69,70,71,72,73,74,75,76,77,78,79,80,81,82,91,92,98,99};
const int LS_MAX_CRITS = sizeof(LS_CRIT_LIST) / sizeof(LS_CRIT_LIST[0]);


const sstring LS_COLOR_NAMES[] = {"red", "yellow", "green", "blue", "purple", "white"};
const sstring LS_COLOR_CODES[] = {"<r>", "<Y>", "<g>", "<b>", "<p>", "<W>"};
const int LS_MAX_COLORS = sizeof(LS_COLOR_NAMES) / sizeof(LS_COLOR_NAMES[0]);

int which_color(TBeing *ch)
{
  int color = 0;
  for (auto c: ch->getName())
    color += c;
  return color % LS_MAX_COLORS;
}


void lightsaber_extend(TBeing *ch, TGenWeapon *weapon)
{
  if (weapon->getWeaponType(0) == WEAPON_TYPE_SLICE)
    return;

  weapon->setWeaponType(WEAPON_TYPE_SLICE);
  weapon->setWeapDamLvl(38 * 4);
  weapon->setWeapDamDev(8);
  weapon->swapToStrung();

  int color = which_color(ch);
  const sstring cname = LS_COLOR_NAMES[color];
  const sstring ccode = LS_COLOR_CODES[color];

  act(format("A brilliant blade of %s%s<o> light springs forth from your $o.") % ccode % cname,
      false, ch, weapon, NULL, TO_CHAR, ANSI_ORANGE);

  act(format("A brilliant blade of %s%s<o> light springs forth from $n's $o.") % ccode % cname,
      false, ch, weapon, NULL, TO_ROOM, ANSI_ORANGE);

  weapon->shortDescr = format("%s with a brilliant blade of %s%s<1> light")
    % obj_index[weapon->getItemIndex()].short_desc % ccode % cname;
}


void lightsaber_retract(TBeing *ch, TGenWeapon *weapon)
{
  if (weapon->getWeaponType(0) == WEAPON_TYPE_BLUDGEON)
    return;

  weapon->setWeaponType(WEAPON_TYPE_BLUDGEON);
  weapon->setWeapDamLvl(1);
  weapon->setWeapDamDev(1);
  weapon->swapToStrung();

  if (!ch)
    return;

  int color = which_color(ch);
  const sstring cname = LS_COLOR_NAMES[color];
  const sstring ccode = LS_COLOR_CODES[color];

  act(format("A brilliant blade of %s%s<o> light retracts into your $o.") % ccode % cname,
      false, ch, weapon, NULL, TO_CHAR, ANSI_ORANGE);
  act(format("A brilliant blade of %s%s<o> light retracts into $n's $o.") % ccode % cname,
      false, ch, weapon, NULL, TO_ROOM, ANSI_ORANGE);
}


int lightsaber(TBeing *vict, cmdTypeT cmd, const char *, TObj *obj, TObj *)
{
  TGenWeapon *weapon = dynamic_cast<TGenWeapon *>(obj);
  if (!weapon)
    return FALSE;

  TBeing *ch = dynamic_cast<TBeing *>(obj->equippedBy);
  if (!ch) {
    lightsaber_retract(NULL, weapon);
    return FALSE;
  }

  if (cmd == CMD_OBJ_USED && ch && ch->hasQuestBit(TOG_PSIONICIST)) {
    if (weapon->getWeaponType(0) == WEAPON_TYPE_SLICE)
      lightsaber_retract(ch, weapon);
    else
      lightsaber_extend(ch, weapon);
    return TRUE;

  } else if ( (cmd == CMD_GENERIC_QUICK_PULSE && !ch)
           || (cmd == CMD_GENERIC_PULSE && ch && (!ch->hasQuestBit(TOG_PSIONICIST) || !ch->fight())) ) {
    lightsaber_retract(ch, weapon);
    return TRUE;

  } else if (cmd == CMD_OBJ_HITTING && ch && ch->hasQuestBit(TOG_PSIONICIST)) {
    lightsaber_extend(ch, weapon);
    return FALSE;

  } else if (cmd != CMD_OBJ_HIT || !ch || !ch->hasQuestBit(TOG_PSIONICIST)) {
    return FALSE;
  }

  if (::number(0, LIGHTSABER_CRITS_INFREQUENCY)) {
    if (!::number(0, LIGHTSABER_NOISE_INFREQUENCY))
      act("$n hums loudly as it swings.", false, obj, nullptr, nullptr, TO_ROOM);
    return FALSE;
  }

  act(format("$p %sflashes brightly<1> as it strikes!") % LS_COLOR_CODES[which_color(ch)],
      false, obj, nullptr, nullptr, TO_ROOM);

  wearSlotT part = vict->getPartHit(ch, TRUE);
  int damage = ch->getWeaponDam(vict, weapon, HAND_PRIMARY);
  int crit = LS_CRIT_LIST[::number(0, LS_MAX_CRITS - 1)];
  spellNumT type = ch->getAttackType(weapon, HAND_PRIMARY);

  int rc = ch->critSuccessChance(vict, weapon, &part, type, &damage, crit);
  if (IS_SET_DELETE(rc, DELETE_VICT))
    return DELETE_VICT;
  rc = ch->applyDamage(vict, damage, type);
  if (IS_SET_DELETE(rc, DELETE_VICT))
    return DELETE_VICT;
  return FALSE;
}
