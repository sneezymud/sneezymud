//////////////////////////////////////////////////////////////////////////
//
// SneezyMUD - All rights reserved, SneezyMUD Coding Team
//
//////////////////////////////////////////////////////////////////////////


#include "stdsneezy.h"


enum ammoTypeT {
  AMMO_NONE,
  AMMO_10MM,
  AMMO_9MM,
  AMMO_22CAL,
  AMMO_45CAL,
  AMMO_50CAL,
  AMMO_357CAL,
  AMMO_38CAL,
  AMMO_556MM,
  AMMO_768MM,
  AMMO_MAX
};

const char *shelldesc [] =
{
  "None",
  "10mm",
  "9mm",
  ".22 caliber",
  ".45 caliber",
  ".50 caliber",
  ".357 caliber",
  ".38 caliber",
  "5.56mm",
  "7.68mm"
};

const char *shellkeyword [] = 
{
  "None",
  "10mm",
  "9mm",
  "22cal",
  "45cal",
  "50cal",
  "357cal",
  "38cal",
  "556mm",
  "768mm"
};


const char *getAmmoKeyword(int ammo){
  if(ammo < AMMO_NONE ||
     ammo >= AMMO_MAX){
    return shellkeyword[0];
  }
  
  return shellkeyword[ammo];
}



const char *getAmmoDescr(int ammo){
  if(ammo < AMMO_NONE ||
     ammo >= AMMO_MAX){
    return shelldesc[0];
  }
  
  return shelldesc[ammo];
}


void dropSpentCasing(TRoom *roomp, int ammo){
  TObj *obj;
  char buf[256];

  int robj = real_object(13874);
  if (robj < 0 || robj >= (signed int) obj_index.size()) {
    vlogf(LOG_BUG, "dropSpentCasing(): No object (%d) in database!", 13874);
    return;
  }
  
  obj = read_object(robj, REAL);

  obj->swapToStrung();
  
  sprintf(buf, "casing spent %s", getAmmoDescr(ammo));
  delete [] obj->name;
  obj->name = mud_str_dup(buf);
  
  sprintf(buf, "<o>a spent %s casing<1>", getAmmoDescr(ammo));
  delete [] obj->shortDescr;
  obj->shortDescr = mud_str_dup(buf);

  sprintf(buf, "A spent <o>%s casing<1> lies here.", getAmmoDescr(ammo));
  delete [] obj->descr;
  obj->setDescr(mud_str_dup(buf));

  *roomp += *obj;

  return;
}



// ammo types
// 0 - none
// 1 - 10mm
// 2 - 9mm
// 3 - .22
// 4 - .45
// 5 - .50
// 6 - .357


void gload_usage(TBeing *tb){
  tb->sendTo("Syntax : (loading)   gload <gun> <ammo>\n\r");
  tb->sendTo("Syntax : (unloading) gload unload <gun>\n\r");
  return;
}


void TBeing::doGload(const char *arg)
{
  char    arg1[128],
          arg2[128], buf[256];
  TObj  *bow;
  TThing  *arrow;
  TGun *gun;
  TAmmo *ammo;
  int nargs;
  TBeing *tb;

  nargs=sscanf(arg, "%s %s", arg1, arg2);

  if(nargs<1 || nargs>2){
    gload_usage(this);
    return;
  }
  
  if(strcmp(arg1, "unload")){
    generic_find(arg1, FIND_OBJ_INV | FIND_OBJ_EQUIP, this, &tb, &bow);
    
    if(!bow || !(gun=dynamic_cast<TGun *>(bow))){
      gload_usage(this);
      return;
    }

    if(nargs==1){
      strcpy(arg2, getAmmoKeyword(gun->getAmmoType()));
    } 

    if(!(arrow = searchLinkedListVis(this, arg2, stuff)) ||
       !(ammo=dynamic_cast<TAmmo *>(arrow))){
      gload_usage(this);
      return;
    }
    
    if(gun->getAmmo()){
      sprintf(buf, "unload %s", arg1);
      doGload(buf);
      if(gun->getAmmo()){
	sendTo("That gun is already loaded!\n\r");
	return;
      }
    }
    
    if(ammo->getAmmoType() != gun->getAmmoType()){
      sendTo("That isn't the right kind of ammunition.\n\r");
      return;
    }
    
    --(*ammo);
    gun->setAmmo(ammo);
    
    act("You load $p into $N.", TRUE, this, ammo, gun, TO_CHAR);
    act("$n loads $p into $N.", TRUE, this, ammo, gun, TO_ROOM);
    addToWait(combatRound(1));
  } else {
    generic_find(arg2, FIND_OBJ_INV | FIND_OBJ_EQUIP, this, &tb, &bow);

    if (!bow || !(gun=dynamic_cast<TGun *>(bow))){
      gload_usage(this);
      return;
    }

    if(!(ammo=gun->getAmmo())){
      sendTo("That gun isn't loaded!\n\r");
      return;
    }
    
    arrow=dynamic_cast<TThing *>(ammo);

    if(ammo->getRounds() == 0){
      *roomp += *ammo;

      act("You unload $N and drop $p.", TRUE, this, ammo, gun, TO_CHAR);
      act("$n unloads $N and drops $p.", TRUE, this, ammo, gun, TO_ROOM);
    } else {
      arrow->nextThing=stuff;
      stuff=arrow;
      arrow->parent=this;
      
      act("You unload $N.", TRUE, this, ammo, gun, TO_CHAR);
      act("$n unloads $N.", TRUE, this, ammo, gun, TO_ROOM);
    }

    gun->setAmmo(NULL);
    addToWait(combatRound(1));    
  }
}



string TGun::statObjInfo() const
{
  char buf[256];

  sprintf(buf, "Rate of Fire: %i, Ammo Type: %s, Ammo: %i, Ammo Loaded: %s",
	  getROF(), getAmmoDescr(getAmmoType()), (ammo?ammo->getRounds():0),
	  (ammo?ammo->getName():"None"));

  string a(buf);
  return a;
}


void TGun::describeObjectSpecifics(const TBeing *ch) const
{
  if(getAmmo()){
    ch->sendTo("It has %i rounds of %s ammunition left.\n\r",
	       getAmmo()->getRounds(), getAmmoDescr(getAmmoType()));
  } else {
    // yeah yeah bad grammar, may as well be consistant though
    ch->sendTo("Is has 0 rounds of %s ammunition left.\n\r",
	       getAmmoDescr(getAmmoType()));
  }

}


void TAmmo::describeObjectSpecifics(const TBeing *ch) const
{
  ch->sendTo("It has %i rounds of %s ammunition left.\n\r",
	     getRounds(), getAmmoDescr(getAmmoType()));

}


void TGun::assignFourValues(int x1, int x2, int x3, int x4)
{
  setROF(x1);  
  setWeapDamLvl(x2);
  setWeapDamDev(x3);
  setAmmoType(x4);  
}

void TGun::getFourValues(int *x1, int *x2, int *x3, int *x4) const {
  *x1=getROF();
  *x2=getWeapDamLvl();
  *x3=getWeapDamDev();
  *x4=getAmmoType();
}


TGun::TGun() :
  TGenWeapon(),
  rof(1),
  ammotype(1),
  ammo(NULL)
{
  setMaxSharp(100);
  setCurSharp(100);

}

TGun::TGun(const TGun &a) :
  TGenWeapon(a),
  rof(a.rof),
  ammotype(a.ammotype),
  ammo(NULL)
{
}

TGun & TGun::operator=(const TGun &a)
{
  if (this == &a) return *this;
  TGenWeapon::operator=(a);
  return *this;
}

TGun::~TGun()
{
  if(ammo)
    delete ammo;
}


string TAmmo::statObjInfo() const
{
  char buf[256];
  
  sprintf(buf, "Ammo Type: %s, Rounds Remaining: %i",
	  getAmmoDescr(getAmmoType()), getRounds());

  string a(buf);
  return a;
}

TAmmo::TAmmo() :
  TObj()
{
}

TAmmo::TAmmo(const TAmmo &a) :
  TObj(a),
  ammotype(a.ammotype),
  rounds(a.rounds)
{
}

TAmmo & TAmmo::operator=(const TAmmo &a)
{
  if (this == &a) return *this;
  TObj::operator=(a);
  ammotype=a.ammotype;
  rounds=a.rounds;
  return *this;
}

TAmmo::~TAmmo()
{
}

void TAmmo::assignFourValues(int x1, int x2, int x3, int x4){
  setAmmoType(x1);
  setRounds(x2);

}

void TAmmo::getFourValues(int *x1, int *x2, int *x3, int *x4) const
{
  *x1 = getAmmoType();
  *x2 = getRounds();
  *x3 = 0;
  *x4 = 0;
}
 

