#include "stdsneezy.h"
#include "obj_general_weapon.h"
#include "obj_base_weapon.h"
#include "obj_gun.h"


TGenWeapon::TGenWeapon() :
  TBaseWeapon(),
  weapon_type(WEAPON_TYPE_NONE)
{
}

TGenWeapon::TGenWeapon(const TGenWeapon &a) :
  TBaseWeapon(a),
  weapon_type(a.weapon_type)
{
}

TGenWeapon & TGenWeapon::operator=(const TGenWeapon &a)
{
  if (this == &a) return *this;
  TBaseWeapon::operator=(a);
  weapon_type = a.weapon_type;
  return *this;
}

TGenWeapon::~TGenWeapon()
{
}

void TGenWeapon::assignFourValues(int x1, int x2, int x3, int x4)
{
  TBaseWeapon::assignFourValues(x1, x2, x3, x4);

  setWeaponType((weaponT) x4);
}

void TGenWeapon::getFourValues(int *x1, int *x2, int *x3, int *x4) const
{
  TBaseWeapon::getFourValues(x1, x2, x3, x4);

  *x4 = getWeaponType();
}

weaponT TGenWeapon::getWeaponType() const
{
  return weapon_type;
}

void TGenWeapon::setWeaponType(weaponT n)
{
  weapon_type = n;
}

string TGenWeapon::statObjInfo() const
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
  string a(buf);

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
    sprintf(buf, "Type : %s (%d)",
	    attack_hit_text_twink[(getWtype() - TYPE_MIN_HIT)].singular,
	    getWeaponType());
    a += buf;
  } else {
    sprintf(buf, "Type : %s (%d)",
	    attack_hit_text[(getWtype() - TYPE_MIN_HIT)].singular,
	    getWeaponType());
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
    vlogf(LOG_LOW,"weapon %s has a bad weight set.",
             getName());

  if ((getVolume() <= 800) && (getWeight() < 3))
    if (!canWear(ITEM_THROW) && !dynamic_cast<TGun *>(this))
      vlogf(LOG_LOW,"weapon %s probably needs to be set throwable.",
              getName());

  if (getWeaponType() == WEAPON_TYPE_NONE)
    vlogf(LOG_LOW,"weapon %s needs a weapon_type defined",
             getName());

  if (!isBluntWeapon() && !isSlashWeapon() && !isPierceWeapon())
    vlogf(LOG_LOW,"weapon %s has bogus type apparently.",
             getName());

  TBaseWeapon::lowCheck();
}

bool TGenWeapon::sellMeCheck(const TBeing *ch, TMonster *keeper) const
{
  int total = 0;
  TThing *t;
  char buf[256];

  for (t = keeper->getStuff(); t; t = t->nextThing) {
    if ((t->number == number) &&
        (t->getName() && getName() &&
         !strcmp(t->getName(), getName()))) {
      total += 1;
      if (total > 9) {
        sprintf(buf, "%s I already have plenty of those.", ch->name);
        keeper->doTell(buf);
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
