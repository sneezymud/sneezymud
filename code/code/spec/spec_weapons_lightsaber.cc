#include "being.h"
#include "obj_general_weapon.h"

void lightSaberExtend(TBeing *ch, TGenWeapon *weapon)
{
  sstring buf;
  sstring colornames[]={"red", "yellow", "green", "blue", "purple", "white"};
  sstring colorcodes[]={"<r>", "<Y>", "<g>", "<b>", "<p>", "<W>"};
  int which_color=0;

  // this is just a way to get a random but consistent color
  buf=ch->getName();
  for(unsigned int i=0;i<buf.length();++i){
    which_color += (int) buf[i];
  }
  which_color=which_color % 6;

  if(weapon->getWeaponType(0)==WEAPON_TYPE_SLICE)
    return;

  weapon->setWeaponType(WEAPON_TYPE_SLICE);
  weapon->setWeapDamLvl(38 * 4);
  weapon->setWeapDamDev(8);
  weapon->swapToStrung();

  buf=format("A brilliant blade of %s%s<o> light springs forth from $p.") %
    colorcodes[which_color] % colornames[which_color];
  act(buf, false, ch, weapon, NULL, TO_CHAR, ANSI_ORANGE);

  buf=format("A brilliant blade of %s%s<o> light springs forth from $n's $o.") %
    colorcodes[which_color] % colornames[which_color];
  act(buf, false, ch, weapon, NULL, TO_ROOM, ANSI_ORANGE);


  buf = format("%s with a brilliant %s%s<1> blade of light") %
    obj_index[weapon->getItemIndex()].short_desc %
    colorcodes[which_color] % colornames[which_color];
  weapon->shortDescr = buf;
}

void lightSaberRetract(TBeing *ch, TGenWeapon *weapon)
{
  sstring buf;
  sstring colornames[]={"red", "yellow", "green", "blue", "purple", "white"};
  sstring colorcodes[]={"<r>", "<Y>", "<g>", "<b>", "<p>", "<W>"};
  int which_color=0;

  if(weapon->getWeaponType(0)==WEAPON_TYPE_BLUDGEON)
    return;

  weapon->setWeaponType(WEAPON_TYPE_BLUDGEON);
  weapon->setWeapDamLvl(1);
  weapon->setWeapDamDev(1);
  weapon->swapToStrung();

  // restore to original description
  buf=obj_index[weapon->getItemIndex()].short_desc;
  weapon->shortDescr = buf;

  if(ch){
    // this is just a way to get a random but consistent color
    buf=ch->getName();
    for(unsigned int i=0;i<buf.length();++i){
      which_color += (int) buf[i];
    }
    which_color=which_color % 6;
    
    buf=format("A brilliant blade of %s%s<o> light retracts into $p.") %
      colorcodes[which_color] % colornames[which_color];
    act(buf, false, ch, weapon, NULL, TO_CHAR, ANSI_ORANGE);
    
    buf=format("A brilliant blade of %s%s<o> light retracts into $n's $o.") %
      colorcodes[which_color] % colornames[which_color];
    act(buf, false, ch, weapon, NULL, TO_ROOM, ANSI_ORANGE);
  }

}  


int lightSaber(TBeing *vict, cmdTypeT cmd, const char *, TObj *o, TObj *)
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
    lightSaberRetract(NULL, weapon);
    return FALSE;
  }


  if(cmd==CMD_OBJ_USED && ch && ch->hasQuestBit(TOG_PSIONICIST)){
    if(weapon->getWeaponType(0)==WEAPON_TYPE_SLICE){
      lightSaberRetract(ch, weapon);
    } else {
      lightSaberExtend(ch, weapon);
    }

    return TRUE;
  } else if(cmd==CMD_GENERIC_QUICK_PULSE && !ch){
    lightSaberRetract(ch, weapon);
    return TRUE;
  } else if(cmd==CMD_GENERIC_PULSE && ch && 
	    (!ch->hasQuestBit(TOG_PSIONICIST) || !ch->fight())){
    lightSaberRetract(ch, weapon);
    return TRUE;
  } else if(cmd==CMD_OBJ_HITTING && ch && ch->hasQuestBit(TOG_PSIONICIST)){
    lightSaberExtend(ch, weapon);
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


