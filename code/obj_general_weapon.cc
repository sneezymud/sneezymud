#include "stdsneezy.h"
#include "obj_general_weapon.h"
#include "obj_base_weapon.h"
#include "obj_gun.h"
#include "shop.h"
#include "shopowned.h"
#include "liquids.h"


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
  int x;

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
  char buf[256];

  sprintf(buf, "Maximum %s: %d  Current %s: %d\n\r",
              ((isBluntWeapon() ? "bluntness" :
               (isPierceWeapon() ? "pointiness" :
               "sharpness"))),
              getMaxSharp(),
              ((isBluntWeapon() ? "bluntness" :
               (isPierceWeapon() ? "pointiness" :
               "sharpness"))),
               getCurSharp());
  sstring a(buf);

  sprintf(buf, "Damage Level: %.2f, Damage Deviation: %d\n\r",
               getWeapDamLvl() / 4.0,
               getWeapDamDev());
  a += buf;

  double base = baseDamage();
  double flux = base * getWeapDamDev() / 10;
  sprintf(buf, "Damage When Swung: %d-%d, Average Damage: %d\n\r",
               (int) (base - (int) flux),
               (int) (base + (int) flux),
               (int) baseDamage());
  a += buf;

  sprintf(buf, "Damage When Thrown: %d\n\r",
           (int) damageLevel());
  a += buf;

  if (Twink == 1) {
    sprintf(buf, "Type : %s (%d)\n\r",
	    attack_hit_text_twink[(getWtype() - TYPE_MIN_HIT)].singular,
	    getWeaponType());
    a += buf;
  } else {
    sprintf(buf, "Type : %s (%d)\n\r",
	    attack_hit_text[(getWtype() - TYPE_MIN_HIT)].singular,
	    getWeaponType());
    a += buf;
  }

  if(isPoisoned()){
    sprintf(buf, "Poisoned : %s (%i)",
	    liquidInfo[getPoison()]->name, (int)getPoison());
    a += buf;
  }

  //  sprintf(buf, "New weapons system stats:\n\r Weapon type: %d\n\r Weapon quality: %d\n\r",
  //	  getWeapType(),
  //	  getWeapQual());

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
    vlogf(LOG_LOW,fmt("weapon %s has a bad weight set.") % 
             getName());

  if ((getVolume() <= 800) && (getWeight() < 3))
    if (!canWear(ITEM_THROW) && !dynamic_cast<TGun *>(this))
      vlogf(LOG_LOW,fmt("weapon %s probably needs to be set throwable.") % 
              getName());

  if (getWeaponType() == WEAPON_TYPE_NONE)
    vlogf(LOG_LOW,fmt("weapon %s needs a weapon_type defined") % 
             getName());

  if (!isBluntWeapon() && !isSlashWeapon() && !isPierceWeapon())
    vlogf(LOG_LOW,fmt("weapon %s has bogus type apparently.") % 
             getName());

  TBaseWeapon::lowCheck();
}

bool TGenWeapon::sellMeCheck(TBeing *ch, TMonster *keeper, int) const
{
  int total = 0;
  TThing *t;
  unsigned int shop_nr;

  for (shop_nr = 0; (shop_nr < shop_index.size()) && (shop_index[shop_nr].keeper != (keeper)->number); shop_nr++);

  if (shop_nr >= shop_index.size()) {
    vlogf(LOG_BUG, fmt("Warning... shop # for mobile %d (real nr) not found.") %  (keeper)->number);
    return FALSE;
  }
  
  TShopOwned tso(shop_nr, keeper, ch);
  int max_num=10;

  if(tso.isOwned())
    max_num=tso.getMaxNum(this);

  if(max_num == 0){
    keeper->doTell(ch->name, "I don't wish to buy any of those right now.");
    return TRUE;
  }


  for (t = keeper->getStuff(); t; t = t->nextThing) {
    if ((t->number == number) &&
        (t->getName() && getName() &&
         !strcmp(t->getName(), getName()))) {
      total += 1;
      if (total >= max_num) {
        keeper->doTell(ch->name, "I already have plenty of those.");
        return TRUE;
      }
    }
  }
  return FALSE;
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
