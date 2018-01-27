#include "being.h"
#include "obj_general_weapon.h"

// quest weapon for psionicists

const sstring color_names[] = {"red", "yellow", "green", "blue", "purple", "white"};
const sstring color_codes[] = {"<r>", "<Y>", "<g>", "<b>", "<p>", "<W>"};

#define LIGHTSABER_MAX_COLORS   6


int which_color(TBeing *ch)
{
  int color = 0;
  for (auto &c: ch->getName())
    color += c;
  return color % LIGHTSABER_MAX_COLORS;
}


void lightsaber_extend(TBeing *ch, TGenWeapon *weapon)
{
  if(weapon->getWeaponType(0)==WEAPON_TYPE_SLICE)
    return;

  weapon->setWeaponType(WEAPON_TYPE_SLICE);
  weapon->setWeapDamLvl(38 * 4);
  weapon->setWeapDamDev(8);
  weapon->swapToStrung();

  int color = which_color(ch);
  sstring ccode = color_codes[color];
  sstring cname = color_names[color];

  act(format("A brilliant blade of %s%s<o> light springs forth from $p.") % ccode % cname,
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
  sstring ccode = color_codes[color];
  sstring cname = color_names[color];

  act(format("A brilliant blade of %s%s<o> light retracts into $p.") % ccode % cname,
      false, ch, weapon, NULL, TO_CHAR, ANSI_ORANGE);

  act(format("A brilliant blade of %s%s<o> light retracts into $n's $o.") % ccode % cname,
      false, ch, weapon, NULL, TO_ROOM, ANSI_ORANGE);
}


int lightsaber(TBeing *vict, cmdTypeT cmd, const char *, TObj *o, TObj *)
{
  TGenWeapon *weapon;
  TBeing *ch;
  int crits[20]={67,68,69,70,71,72,73,74,75,76,77,78,79,80,81,82,91,92,98,99};
  wearSlotT part;
  int dam, rc=0;
  spellNumT wtype;

  if(!o)
    return FALSE;
  
  if(!(weapon = dynamic_cast<TGenWeapon *>(o)))
    return FALSE;

  if(!(ch = dynamic_cast<TBeing *>(o->equippedBy))){
    lightsaber_retract(NULL, weapon);
    return FALSE;
  }


  if(cmd==CMD_OBJ_USED && ch && ch->hasQuestBit(TOG_PSIONICIST)){
    if(weapon->getWeaponType(0)==WEAPON_TYPE_SLICE){
      lightsaber_retract(ch, weapon);
    } else {
      lightsaber_extend(ch, weapon);
    }

    return TRUE;
  } else if(cmd==CMD_GENERIC_QUICK_PULSE && !ch){
    lightsaber_retract(ch, weapon);
    return TRUE;
  } else if(cmd==CMD_GENERIC_PULSE && ch && 
	    (!ch->hasQuestBit(TOG_PSIONICIST) || !ch->fight())){
    lightsaber_retract(ch, weapon);
    return TRUE;
  } else if(cmd==CMD_OBJ_HITTING && ch && ch->hasQuestBit(TOG_PSIONICIST)){
    lightsaber_extend(ch, weapon);
    return FALSE;
  } else if(cmd==CMD_OBJ_HIT && ch && ch->hasQuestBit(TOG_PSIONICIST)){
    if(::number(0,1000))
      return FALSE;

    act("$p <W>flashes brightly!<1>", 0, vict, o, 0, TO_ROOM);

    part = vict->getPartHit(ch, TRUE);
    dam = ch->getWeaponDam(vict, weapon, HAND_PRIMARY);

    if(!vict)
      return FALSE;
    
    if (weapon)
      wtype = ch->getAttackType(weapon, HAND_PRIMARY);
    else
      wtype = TYPE_HIT;
    
    rc = ch->critSuccessChance(vict, weapon, &part, wtype, &dam, crits[::number(0,20)]);
    if (IS_SET_DELETE(rc, DELETE_VICT)) {
      return DELETE_VICT;
    }
    rc = ch->applyDamage(vict, dam, wtype);
    if (IS_SET_DELETE(rc, DELETE_VICT)) {
      return DELETE_VICT;
    }
  }

  return FALSE;
}
