//////////////////////////////////////////////////////////////////////////
//
// SneezyMUD - All rights reserved, SneezyMUD Coding Team
//
//////////////////////////////////////////////////////////////////////////


#include "stdsneezy.h"
#include "obj_general_weapon.h"
#include "obj_base_weapon.h"
#include "obj_gun.h"
#include "range.h"
#include "obj_arrow.h"

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
  AMMO_MAX
};

const char *shelldesc [] =
{
  "None",                       // 0
  "10mm pistol",                // 1
  "9mm Parabellem pistol",      // 2
  ".45cal ACP pistol",          // 3
  ".50cal Action Express",      // 4
  ".44cal Magnum",              // 5
  ".32cal ACP",                 // 6
  ".50cal BMG",                 // 7
  "5.56mm NATO pistol",         // 8
  "SS190",                      // 9
  "9mm Parabellem rifle",       // 10
  ".45cal ACP rifle",           // 11
  "5.56mm rifle",               // 12
  "7.62mm rifle",               // 13
  "30cal rifle",                // 14
  "flechette",                  // 15
  "LAW rocket",                 // 16
};

const char *shellkeyword [] = 
{
  "None",                       // 0
  "10mmPistol",                 // 1
  "9mmPistol",                  // 2
  "45calPistol",                // 3
  "50calAE",                    // 4
  "44calMag",                   // 5
  "32calACP",                   // 6
  "50calBMG",                   // 7
  "556mmPistol",                 // 8
  "SS190",                      // 9
  "9mmRifle",                   // 10
  "45calRifle",                 // 11
  "556mmRifle",                 // 12
  "762mmRifle",                 // 13
  "30calRifle",                  // 14
  "flechette",                   // 15
  "lawrocket",
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



void gload_usage(TBeing *tb){
  tb->sendTo("Syntax : (loading)   gload <gun> <ammo>\n\r");
  tb->sendTo("Syntax : (unloading) gload unload <gun>\n\r");
  return;
}


void TBeing::doGload(string arg)
{
  string arg1, arg2;
  string buf;
  TObj  *bow;
  TThing  *arrow;
  TGun *gun;
  TAmmo *ammo=NULL;
  int nargs;
  TBeing *tb;

  nargs=argument_parser(arg, arg1, arg2);

  if(nargs<1 || nargs>2){
    gload_usage(this);
    return;
  }
  
  if(arg1 != "unload"){
    generic_find(arg1.c_str(), FIND_OBJ_INV | FIND_OBJ_EQUIP, this, &tb, &bow);
    
    if(!bow || !(gun=dynamic_cast<TGun *>(bow))){
      gload_usage(this);
      return;
    }

    if(nargs==1){
      arg2=getAmmoKeyword(gun->getAmmoType());
    } 

    if(!(arrow = searchLinkedListVis(this, arg2, getStuff())) ||
       !(ammo=dynamic_cast<TAmmo *>(arrow))){
      gload_usage(this);
      return;
    }
    
    if(gun->getAmmo()){
      ssprintf(buf, "unload %s", arg1.c_str());
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
    generic_find(arg2.c_str(), FIND_OBJ_INV | FIND_OBJ_EQUIP, this, &tb, &bow);

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
      --(*ammo);
      *roomp += *ammo;

      act("You unload $N and drop $p.", TRUE, this, ammo, gun, TO_CHAR);
      act("$n unloads $N and drops $p.", TRUE, this, ammo, gun, TO_ROOM);
    } else {
      --(*arrow);
      *this += *arrow;
      
      act("You unload $N.", TRUE, this, ammo, gun, TO_CHAR);
      act("$n unloads $N.", TRUE, this, ammo, gun, TO_ROOM);
    }

    addToWait(combatRound(1));    
  }
}


int TGun::suggestedPrice() const
{
  int pricetmp=TBaseWeapon::suggestedPrice();

  pricetmp *= getROF();
  pricetmp /= 10;

  return pricetmp;
}



string TGun::statObjInfo() const
{
  char buf[256];

  TGenWeapon::statObjInfo();

  TAmmo *ammo=getAmmo();
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

  setWeapDamLvl(GET_BITS(x2, 7, 8));
  setWeapDamDev(GET_BITS(x2, 15, 8));

  setFlags(x3);

  setAmmoType(x4);  
}

void TGun::getFourValues(int *x1, int *x2, int *x3, int *x4) const {
  int x = 0;

  *x1=getROF();

  SET_BITS(x, 7, 8, getWeapDamLvl());
  SET_BITS(x, 15, 8, getWeapDamDev());
  *x2 = x;

  *x3=getFlags();
  *x4=getAmmoType();
}


TGun::TGun() :
  TGenWeapon(),
  rof(1),
  ammotype(1)
{
  setMaxSharp(100);
  setCurSharp(100);

}

TGun::TGun(const TGun &a) :
  TGenWeapon(a),
  rof(a.rof),
  ammotype(a.ammotype)
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

void TAmmo::setRounds(int r) { 
  if(r<=0){
    char buf[256];
    sprintf(buf, "%s empty", name);

    if(isObjStat(ITEM_STRUNG)){
      delete [] name;
    } else {
      swapToStrung();
    }

    name=mud_str_dup(buf);
  }

  rounds=r; 
}


string TAmmo::showModifier(showModeT tMode, const TBeing *tBeing) const
{
  // recurse if necessary
  string tString = TObj::showModifier(tMode, tBeing);

  if (getRounds()<=0) {
    tString += " (empty)";                                          
  }                                                           
                                                              
  return tString;                                             
}

string TGun::showModifier(showModeT tMode, const TBeing *tBeing) const
{
  // recurse if necessary
  string tString = TObj::showModifier(tMode, tBeing);

  if (!getAmmo() || getAmmo()->getRounds()<=0) {
    tString += " (empty)";                                          
  }                                                           
                                                              
  return tString;                                             
}

int TGun::shootMeBow(TBeing *ch, TBeing *targ, unsigned int count, dirTypeT dir, int shoot_dist)
{
  TAmmo *ammo;
  TObj *bullet;
  char  buf[256];
  
  if (targ &&
      ch->checkPeacefulVictim("They are in a peaceful room. You can't seem to fire the gun.\n\r", targ))
    return FALSE;

  if (targ && ch->noHarmCheck(targ))
    return FALSE;

  string capbuf, capbuf2;
  
  ch->addToWait(combatRound(2));
  int rof=getROF();

  while(rof--){
    if(!(ammo=dynamic_cast<TAmmo *>(getAmmo())) || ammo->getRounds()<=0){
      act("Click.  $N is out of ammunition.", TRUE, ch, NULL, this, TO_CHAR);
      // keep looping to simulate trigger pulls - looks cooler
      continue;
    }

    // grab a bullet object and adjust for damage
    bullet=read_object(31864, VIRTUAL);
    TArrow *tmp=dynamic_cast<TArrow *>(bullet);
    if(tmp){
      tmp->setWeapDamLvl(getWeapDamLvl());
      tmp->setWeapDamDev(getWeapDamDev());
    }

    // decrement ammo and drop a casing
    if(!isCaseless())
      dropSpentCasing(ch->roomp, ammo->getAmmoType());
    setRounds(getRounds()-1);
    
    // send messages
    capbuf = colorString(ch, ch->desc, bullet->getName(), NULL, COLOR_OBJECTS, TRUE);
    capbuf2 = colorString(ch, ch->desc, getName(), NULL, COLOR_OBJECTS, TRUE);
    
    if (targ)
      ch->sendTo(COLOR_MOBS, "You shoot %s out of %s at %s.\n\r",
		 good_uncap(capbuf).c_str(), good_uncap(capbuf2).c_str(),
		 targ->getName());
    else
      ch->sendTo("You shoot %s out of %s.\n\r",
		 good_uncap(capbuf).c_str(), 
		 good_uncap(capbuf2).c_str());
    
    sprintf(buf, "$n points $p %swards, and shoots $N out of it.",
	    dirs[dir]);
    act(buf, FALSE, ch, this, bullet, TO_ROOM);
    
    // put the bullet in the room and then "throw" it
    *ch->roomp += *bullet;    
    
    int rc = throwThing(bullet, dir, ch->in_room, &targ, shoot_dist, 10, ch);

    if(!isSilenced())
      ch->roomp->getZone()->sendTo("A gunshot echoes in the distance.\n\r",
				   ch->in_room);

    // delete the bullet afterwards, arbitrary decision
    // since they are arrow type and you usually don't find spent lead anyway
    delete bullet;
    bullet = NULL;

    if (IS_SET_DELETE(rc, DELETE_VICT)) {
      delete targ;
      targ = NULL;
      return FALSE;
    }
  }

  return FALSE;
}


void TGun::setRounds(int r){
  if(getAmmo()){
    if(r<=0 && isClipless()){
      TAmmo *ammo=getAmmo();
      delete ammo;
    } else
      getAmmo()->setRounds(r);
  }
}

int TGun::getRounds() const {
  if(getAmmo())
    return getAmmo()->getRounds();
  else 
    return 0;
}

void TGun::describeContains(const TBeing *ch) const
{
  // just to avoid the "something in it" message
}
