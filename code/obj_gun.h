//////////////////////////////////////////////////////////////////////////
//
// SneezyMUD - All rights reserved, SneezyMUD Coding Team
//
//
//////////////////////////////////////////////////////////////////////////

#ifndef __OBJ_GUN_H
#define __OBJ_GUN_H

#include "obj_general_weapon.h"

const unsigned int GUN_FLAG_SILENCED         = (1<<0);
const unsigned int GUN_FLAG_CASELESS         = (1<<1);
const unsigned int GUN_FLAG_CLIPLESS         = (1<<2);
const unsigned int GUN_FLAG_FOULED           = (1<<3);

enum ammoTypeT {
  AMMO_NONE = 0,                // 0
  AMMO_10MM_PISTOL,             // 1
  AMMO_9MM_PARABELLEM_PISTOL,   // 2
  AMMO_45CAL_ACP_PISTOL,        // 3
  AMMO_50CAL_AE_PISTOL,         // 4
  AMMO_44CAL_MAGNUM_PISTOL,     // 5
  AMMO_32CAL_ACP_PISTOL,        // 6
  AMMO_50CAL_BMG_PISTOL,        // 7
  AMMO_556MM_NATO_PISTOL,       // 8
  AMMO_SS190,                   // 9
  AMMO_9MM_PARABELLEM_RIFLE,    // 10
  AMMO_45CAL_ACP_RIFLE,         // 11
  AMMO_556MM_RIFLE,             // 12
  AMMO_762MM_RIFLE,             // 13
  AMMO_30CAL_RIFLE,             // 14
  AMMO_FLECHETTE,               // 15
  AMMO_LAW,                     // 16
  AMMO_LEAD_SHOT,               // 17
  AMMO_CANNON_BALL,             // 18
  AMMO_MAX
};


extern const char *shelldesc [];
extern const char *shellkeyword [];


TThing *findFlint(TThing *stuff);
TThing *findPowder(TThing *stuff, int uses);
TThing *findShot(TThing *stuff, ammoTypeT ammotype);



class TAmmo : public TObj {
  private:
    int ammotype;
    int rounds;

  public:
    void setAmmoType(int a) { ammotype=a; }
    int getAmmoType() const { return ammotype; }
    void setRounds(int);
    int getRounds() const { return rounds; }

    virtual itemTypeT itemType() const { return ITEM_AMMO; }
    virtual void assignFourValues(int, int, int, int);
    virtual void getFourValues(int *, int *, int *, int *) const;
    virtual sstring statObjInfo() const;
    virtual void describeObjectSpecifics(const TBeing *) const;
    virtual sstring showModifier(showModeT , const TBeing *) const;

    TAmmo();
    TAmmo(const TAmmo &a);
    TAmmo & operator=(const TAmmo &a);
    virtual ~TAmmo();
};


class TGun : public TGenWeapon {
  private:
    int rof;
    int ammotype;
    int flags;

  public:    
    virtual void loadMe(TBeing *ch, TAmmo *ammo);
    virtual void unloadMe(TBeing *ch, TAmmo *ammo);

    void setROF(int r) { rof=r; }
    int getROF() const { return rof; }
    void setAmmoType(int a) { ammotype=a; }
    int getAmmoType() const { return ammotype; }
    void setAmmo(TAmmo *a) { 
      *this += *a;
    }
    TAmmo *getAmmo() const { return dynamic_cast<TAmmo *>(getStuff()); }
    void setRounds(int);
    int getRounds() const;
    void setFlags(int f) { flags=f; }
    int getFlags() const { return flags; }
    void addToFlags(int f) { flags=flags ^ f; }
    void remFromFlags(int f) { flags &= ~f; }

    bool isSilenced() const { return flags & GUN_FLAG_SILENCED; }
    bool isCaseless() const { return flags & GUN_FLAG_CASELESS; }
    bool isClipless() const { return flags & GUN_FLAG_CLIPLESS; }
    bool isFouled() const { return flags & GUN_FLAG_FOULED; }
    void dropSpentCasing(TRoom *roomp);


    bool canStab() const { return false; }
    bool canBackstab() const { return false; }
    int shootMeBow(TBeing *, TBeing *, unsigned int, dirTypeT, int);
    virtual int suggestedPrice() const;
    virtual weaponT TGun::getWeaponType(int) const { return WEAPON_TYPE_SHOOT; }
    virtual itemTypeT itemType() const { return ITEM_GUN; }
    virtual int getCurSharp() const { return 100; }
    virtual int getMaxSharp() const { return 100; }
    virtual void assignFourValues(int, int, int, int);
    virtual void getFourValues(int *, int *, int *, int *) const;
    virtual void TGun::changeBaseWeaponValue1(TBeing *, const char *, editorEnterTypeT);
    virtual void describeObjectSpecifics(const TBeing *) const;
    virtual void describeContains(const TBeing *) const;

    virtual sstring showModifier(showModeT , const TBeing *) const;
    virtual sstring statObjInfo() const;
    TGun();
    TGun(const TGun &a);
    TGun & operator=(const TGun &a);
    virtual ~TGun();
};



#endif
