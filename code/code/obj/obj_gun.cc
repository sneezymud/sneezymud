//////////////////////////////////////////////////////////////////////////
//
// SneezyMUD - All rights reserved, SneezyMUD Coding Team
//
//////////////////////////////////////////////////////////////////////////


#include "handler.h"
#include "extern.h"
#include "room.h"
#include "being.h"
#include "materials.h"
#include "obj_general_weapon.h"
#include "obj_base_weapon.h"
#include "colorstring.h"
#include "obj_gun.h"
#include "range.h"
#include "obj_arrow.h"
#include "obj_handgonne.h"
#include "obj_tool.h"

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


void TGun::dropSpentCasing(TRoom *roomp){
  TObj *obj;
  char buf[256];
  int ammo=getAmmoType();

  int robj = real_object(13874);
  if (robj < 0 || robj >= (signed int) obj_index.size()) {
    vlogf(LOG_BUG, format("dropSpentCasing(): No object (%d) in database!") %  13874);
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

void TGun::loadMe(TBeing *ch, TAmmo *ammo)
{
  --(*ammo);
  setAmmo(ammo);
  
  act("You load $p into $N.", TRUE, ch, ammo, this, TO_CHAR);
  act("$n loads $p into $N.", TRUE, ch, ammo, this, TO_ROOM);
  ch->addToWait(combatRound(1));
}

void TGun::unloadMe(TBeing *ch, TAmmo *ammo)
{
  if(ammo->getRounds() == 0){
    --(*ammo);
    *ch->roomp += *ammo;
    
    act("You unload $N and drop $p.", TRUE, ch, ammo, this, TO_CHAR);
    act("$n unloads $N and drops $p.", TRUE, ch, ammo, this, TO_ROOM);
  } else {
    --(*ammo);
    *ch += *ammo;
    
    act("You unload $N.", TRUE, ch, ammo, this, TO_CHAR);
    act("$n unloads $N.", TRUE, ch, ammo, this, TO_ROOM);
  }
  
  ch->addToWait(combatRound(1));    
}

void TBeing::doGload(sstring arg)
{
  sstring arg1, arg2;
  sstring buf;
  TObj  *bow;
  TThing  *arrow;
  TGun *gun;
  TAmmo *ammo=NULL;
  TBeing *tb;

  arg1=arg.word(0);
  arg2=arg.word(1);

  if(arg1.empty()){
    gload_usage(this);
    return;
  }
  
  if(arg1 != "unload"){

    for(int i=1;i<=100;++i){
      generic_find(((sstring)(format("%i.%s") % i % arg1)).c_str(), FIND_OBJ_INV | FIND_OBJ_EQUIP | FIND_OBJ_ROOM, this, &tb, &bow);
      if(bow && dynamic_cast<TGun *>(bow))
	break;
    }

    if(!bow || !(gun=dynamic_cast<TGun *>(bow))){
      gload_usage(this);
      return;
    }

    if(arg2.empty()){
      arg2=getAmmoKeyword(gun->getAmmoType());
    } 

    if(!(arrow = searchLinkedListVis(this, arg2, stuff)) ||
       !(ammo=dynamic_cast<TAmmo *>(arrow))){
      gload_usage(this);
      return;
    }
    
    if(gun->getAmmo()){
      buf = format("unload %s") % arg1;
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

    gun->loadMe(this, ammo);
  } else {
    generic_find(arg2.c_str(), FIND_OBJ_INV | FIND_OBJ_EQUIP | FIND_OBJ_ROOM, this, &tb, &bow);

    if (!bow || !(gun=dynamic_cast<TGun *>(bow))){
      gload_usage(this);
      return;
    }

    if(!(ammo=gun->getAmmo())){
      sendTo("That gun isn't loaded!\n\r");
      return;
    }

    gun->unloadMe(this, ammo);
  }
}


int TGun::suggestedPrice() const
{
  int pricetmp=TBaseWeapon::suggestedPrice();

  pricetmp *= getROF();
  pricetmp /= 10;

  // add material value
  pricetmp += (int)(10.0 * getWeight() * material_nums[getMaterial()].price);

  return pricetmp;
}



sstring TGun::statObjInfo() const
{
  sstring buf, sbuf;

  TGenWeapon::statObjInfo();

  TAmmo *ammo=getAmmo();
  buf = format("Rate of Fire: %i, Ammo Type: %s, Ammo: %i, Ammo Loaded: %s") %
    getROF() % getAmmoDescr(getAmmoType()) % (ammo?ammo->getRounds():0) %
    (ammo?ammo->getName():"None");
  sbuf += buf;

  buf = format("\n\rSilenced: %s  Caseless: %s  Clipless: %s  Fouled: %s") %
	   (isSilenced()?"yes":"no") % (isCaseless()?"yes":"no") %
	   (isClipless()?"yes":"no") % (isFouled()?"yes":"no");
  sbuf += buf;

  return sbuf;
}


void TGun::describeObjectSpecifics(const TBeing *ch) const
{
  if(getAmmo()){
    ch->sendTo(format("It has %i rounds of %s ammunition left.\n\r") %
	       getAmmo()->getRounds() % getAmmoDescr(getAmmoType()));
  } else {
    // yeah yeah bad grammar, may as well be consistant though
    ch->sendTo(format("It has 0 rounds of %s ammunition left.\n\r") %
	       getAmmoDescr(getAmmoType()));
  }

}


void TAmmo::describeObjectSpecifics(const TBeing *ch) const
{
  ch->sendTo(format("It has %i rounds of %s ammunition left.\n\r") %
	     getRounds() % getAmmoDescr(getAmmoType()));

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


sstring TAmmo::statObjInfo() const
{
  sstring buf, sbuf;

  buf = format("Ammo Type: %s, Rounds Remaining: %i") %
	  getAmmoDescr(getAmmoType()) % getRounds();
  sbuf += buf;

  return sbuf;
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


sstring TAmmo::showModifier(showModeT tMode, const TBeing *tBeing) const
{
  // recurse if necessary
  sstring tString = TObj::showModifier(tMode, tBeing);

  if (getRounds()<=0) {
    tString += " (empty)";                                          
  }                                                           
                                                              
  return tString;                                             
}

sstring TGun::showModifier(showModeT tMode, const TBeing *tBeing) const
{
  // recurse if necessary
  sstring tString = TObj::showModifier(tMode, tBeing);

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

  sstring capbuf, capbuf2;
  
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
      dropSpentCasing(ch->roomp);
    setRounds(getRounds()-1);
    
    // send messages
    capbuf = colorString(ch, ch->desc, bullet->getName(), NULL, COLOR_OBJECTS, TRUE);
    capbuf2 = colorString(ch, ch->desc, getName(), NULL, COLOR_OBJECTS, TRUE);
    
    if (targ)
      ch->sendTo(COLOR_MOBS, format("You shoot %s out of %s at %s.\n\r") %
		 capbuf.uncap() % capbuf2.uncap() %
		 targ->getName());
    else
      ch->sendTo(format("You shoot %s out of %s.\n\r") %
		 capbuf.uncap() % 
		 capbuf2.uncap());
    
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


TThing *findFlint(StuffList list){
  TThing *tt;
  TTool *flint;
  TThing *ret;

  for(StuffIter it=list.begin();it!=list.end();++it){
    tt=*it;
    if(tt && (flint=dynamic_cast<TTool *>(tt)) &&
       (flint->getToolType() == TOOL_FLINTSTEEL))
      return tt;

    if(tt && (ret=findFlint(tt->stuff)))
      return ret;
  }

  return NULL;
}


TThing *findPowder(StuffList list, int uses){
  TThing *tt;
  TTool *powder;
  TThing *ret;
  
  for(StuffIter it=list.begin();it!=list.end();++it){
    tt=*it;
    if(tt && (powder=dynamic_cast<TTool *>(tt)) &&
       (powder->getToolType() == TOOL_BLACK_POWDER) &&
       powder->getToolUses() >= uses)
      return tt;

    if(tt && (ret=findPowder(tt->stuff, uses)))
      return ret;
  }

  return NULL;
}


TThing *findShot(StuffList list, ammoTypeT ammotype){
  TThing *tt;
  TAmmo *Shot;
  TThing *ret;

  for(StuffIter it=list.begin();it!=list.end();++it){
    tt=*it;

    if(tt && (Shot=dynamic_cast<TAmmo *>(tt)) &&
       Shot->getAmmoType()==ammotype)
      return tt;

    if(tt && (ret=findShot(tt->stuff, ammotype)))
      return ret;
  }

  return NULL;
}


