#include "obj_general_weapon.h"
#include "obj_base_weapon.h"
#include "obj_gun.h"
#include "shop.h"
#include "shopowned.h"
#include "liquids.h"
#include "toggle.h"
#include "extern.h"
#include "being.h"

TGenWeapon::TGenWeapon() :
  TBaseWeapon()
{
  for(int i=0;i<3;++i){
    weapon_type[i]=WEAPON_TYPE_NONE;
    wtype_frequency[i]=0;
  }
}

TGenWeapon::TGenWeapon(const TGenWeapon &a) :
  TBaseWeapon(a)
{
  for(int i=0;i<3;++i){
    weapon_type[i]=a.weapon_type[i];
    wtype_frequency[i]=a.wtype_frequency[i];
  }
}

TGenWeapon & TGenWeapon::operator=(const TGenWeapon &a)
{
  if (this == &a) return *this;
  TBaseWeapon::operator=(a);
  for(int i=0;i<3;++i){
    weapon_type[i]=a.weapon_type[i];
    wtype_frequency[i]=a.wtype_frequency[i];
  }
  return *this;
}

TGenWeapon::~TGenWeapon()
{
}

void TGenWeapon::assignFourValues(int x1, int x2, int x3, int x4)
{
  TBaseWeapon::assignFourValues(x1, x2, x3, x4);

  setWeaponType((weaponT)GET_BITS(x3, 7, 8), 0);
  setWeaponFreq(GET_BITS(x3, 15, 8), 0);

  setWeaponType((weaponT)GET_BITS(x3, 23, 8), 1);
  setWeaponFreq(GET_BITS(x3, 31, 8), 1);

  setWeaponType((weaponT)GET_BITS(x4, 7, 8), 2);
  setWeaponFreq(GET_BITS(x4, 15, 8), 2);
}

void TGenWeapon::getFourValues(int *x1, int *x2, int *x3, int *x4) const
{
  int x=0;

  TBaseWeapon::getFourValues(x1, x2, x3, x4);
  
  SET_BITS(x, 7, 8, getWeaponType(0));
  SET_BITS(x, 15, 8, getWeaponFreq(0));
  SET_BITS(x, 23, 8, getWeaponType(1));
  SET_BITS(x, 31, 8, getWeaponFreq(1));
  *x3=x;
  SET_BITS(x, 7, 8, getWeaponType(2));
  SET_BITS(x, 15, 8, getWeaponFreq(2));
  *x4=x;
}

weaponT TGenWeapon::getWeaponType(int which) const
{
  if(which>=0 && which<=3){
    return weapon_type[which];
  } else {  // return a random type
    int c=::number(0,getWeaponFreq(0)+getWeaponFreq(1)+getWeaponFreq(2));
    
    for(int i=0;i<3;++i){
      c-=getWeaponFreq(i);
      if(c<=0)
	return weapon_type[i];
    }
  }

  return WEAPON_TYPE_NONE;
}

int TGenWeapon::getWeaponFreq(int which) const
{
  return wtype_frequency[which];
}

void TGenWeapon::setWeaponType(weaponT n, int which)
{
  weapon_type[which] = n;
}

void TGenWeapon::setWeaponFreq(int n, int which)
{
  wtype_frequency[which] = n;
}

sstring TGenWeapon::statObjInfo() const
{
  sstring a = "";
  
  a += format("Current %-11s %-7d  Damage Level:     %d\n\r") % ((isBluntWeapon() ? "bluntness:" : (isPierceWeapon() ? "pointiness:" : "sharpness:"))) % getCurSharp() % (int) (getWeapDamLvl() / 4.0);
  a += format("Maximum %-11s %-7d  Damage Deviation: %d\n\r") % ((isBluntWeapon() ? "bluntness:" : (isPierceWeapon() ? "pointiness:" : "sharpness:"))) % getCurSharp() % getWeapDamDev();
  
  double base = baseDamage();
  double flux = base * getWeapDamDev() / 10;
  sstring buf = format("%d-%d") % (int) (base - (int) flux) % (int) (base + (int) flux);
  a += format("Damage When Swung:  %-7s  Average Damage:   %d\n\r") % buf % (int) baseDamage();
  
  a += format("Damage When Thrown: %d\n\r") % (int) damageLevel();
  
  for (int wt = 0; wt < 3; ++wt) {
    if (!getWeaponType(wt))
      continue;
    if (toggleInfo[TOG_TWINK]->toggle) {
      a += format("Attack Type:        %-8s") % attack_hit_text_twink[getWtype(wt) - TYPE_MIN_HIT].singular;
    } else {
      a += format("Attack Type:        %-8s") % attack_hit_text[getWtype(wt) - TYPE_MIN_HIT].singular;
    }
    if (getWeaponFreq(wt))
      a += format(" Attack Frequeny:  %d%%") % getWeaponFreq(wt);
    a += "\n\r";
  }
  if(isPoisoned()){
    a += format("Poisoned with:      %s\n\r") % liquidInfo[getPoison()]->name;
  }
  return a;
}

float TGenWeapon::blowCountSplitter(const TBeing *, bool) const
{
  return 1.0;
}

float TThing::blowCountSplitter(const TBeing *, bool) const
{
  return 0.0;
}

void TGenWeapon::lowCheck()
{
  if ((int) getWeight() < 1)
    vlogf(LOG_LOW,format("weapon %s has a bad weight set.") % 
             getName());

  if ((getVolume() <= 800) && (getWeight() < 3))
    if (!canWear(ITEM_THROW) && !dynamic_cast<TGun *>(this))
      vlogf(LOG_LOW,format("weapon %s probably needs to be set throwable.") % 
              getName());

  if (getWeaponType() == WEAPON_TYPE_NONE)
    vlogf(LOG_LOW,format("weapon %s needs a weapon_type defined") % 
             getName());

  if (!isBluntWeapon() && !isSlashWeapon() && !isPierceWeapon())
    vlogf(LOG_LOW,format("weapon %s has bogus type apparently.") % 
             getName());

  TBaseWeapon::lowCheck();
}

bool TGenWeapon::sellMeCheck(TBeing *ch, TMonster *keeper, int, int) const
{
  return TBaseWeapon::sellMeCheck(ch, keeper, 1, 10);
}

bool TGenWeapon::canCudgel() const
{
  return isBluntWeapon() && getVolume() <= 1500;
}

bool TGenWeapon::canBackstab() const
{
  return isPierceWeapon() && getVolume() <= 1500;
}

bool TGenWeapon::canStab() const
{
  return isPierceWeapon() && getVolume() <= 2000;
}
