//////////////////////////////////////////////////////////////////////////
//
// SneezyMUD - All rights reserved, SneezyMUD Coding Team
//
//////////////////////////////////////////////////////////////////////////


#include "stdsneezy.h"
#include "obj_pool.h"
#include "liquids.h"

void TPool::setDrinkUnits(int n)
{
  setMaxDrinkUnits(n);
  TBaseCup::setDrinkUnits(n);
  setVolume((int)(getVolume() + (getDrinkUnits() * SIP_VOLUME)));
  weightChangeObject(0);
  updateDesc();
  obj_flags.decay_time=getDrinkUnits();
}

void TPool::addToDrinkUnits(int n)
{
  addToMaxDrinkUnits(n);
  TBaseCup::addToDrinkUnits(n);
  setVolume((int)(getVolume() + (getDrinkUnits() * SIP_VOLUME)));
  weightChangeObject(0);
  updateDesc();
  obj_flags.decay_time=getDrinkUnits();
}

void TPool::setDrinkType(liqTypeT n)
{
  TBaseCup::setDrinkType(n);
  updateDesc();
}

// the best way to do this would be to explicitly set the proper weight in the
// accessors, rather than using weightChangeObject.  It is prone to float
// rounding errors and some functions don't even bother to use it (ie, pour,
// which I fixed).  But instead of recoding everything, I'll just put this here
// so that pools don't generate errors in weightChangeObject.
void TPool::weightChangeObject(float fWeight)
{
  setWeight(obj_index[getItemIndex()].weight);
  TBaseCup::weightChangeObject(getDrinkUnits() * SIP_WEIGHT);
}

int TPool::getDrinkIndex() const
{
  int drinkunits=getDrinkUnits();

  if(drinkunits<=0){
    return 0;
  } else if(drinkunits<=5){
    return 1;
  } else if(drinkunits<=10){
    return 2;
  } else if(drinkunits<=25){
    return 3;
  } else if(drinkunits<=50){ 
    return 4;
  } else if(drinkunits<=100){
    return 5;
  } else if(drinkunits<=250){
    return 6;
  } else if(drinkunits<=1000){
    return 7;
  } else if(drinkunits<=10000){
    return 8;
  } else if(drinkunits<=25000){
    return 9;
  } else {
    return 10;
  }
}
  
void TPool::updateDesc()
{
  int drinkindex=getDrinkIndex();
  char buf[256];
  const char *liqname=liquidInfo[getDrinkType()]->name;
  sstring buf2;
  
  const char *poolname [] =
  {
    "a few drops of %s", 
    "a tiny puddle of %s", 
    "a small puddle of %s", 
    "a puddle of %s", 
    "a fair sized puddle of %s", 
    "a big pool of %s", 
    "a large pool of %s", 
    "a huge pool of %s",
    "a massive pool of %s",
    "a tremendously huge pool of %s",
    "a veritable ocean of %s"
  };
  
  const char *pooldesc [] =
  {
    "A few drops of %s sprinkle the $g here and are fading fast.",
    "A tiny puddle of %s has gathered on the $g here.",
    "A small puddle of %s is on the $g here.",
    "A puddle of %s is here on the $g.",
    "A fair sized puddle of %s is here on the $g.",
    "A big pool of %s is on the $g here.",
    "A large pool of %s is here on the $g.",
    "A huge pool of %s is on the $g here.",
    "A massive pool of %s is on the $g here.",
    "A tremendously huge pool of %s dominates the area.",
    "A veritable ocean of %s covers the area."
  };

  if (isObjStat(ITEM_STRUNG)) {
    delete [] shortDescr;
    delete [] descr;
    delete [] name;

    extraDescription *exd;
    while ((exd = ex_description)) {
      ex_description = exd->next;
      delete exd;
    }
    ex_description = NULL;
    delete [] action_description;
    action_description = NULL;
  } else {
    addObjStat(ITEM_STRUNG);
    ex_description = NULL;
    action_description = NULL;
  }

  if(isDrinkConFlag(DRINK_FROZEN)){
    buf2 = fmt("chunk frozen %s %s") %
      stripColorCodes(liquidInfo[getDrinkType()]->name) %
      stripColorCodes(liquidInfo[getDrinkType()]->color);
    name = mud_str_dup(buf2);

    sprintf(buf, "a <C>frozen<1> chunk of %s", liqname);
    shortDescr = mud_str_dup(buf);
    
    sprintf(buf, "A chunk of <C>frozen<1> %s is here.", liqname);
    setDescr(mud_str_dup(buf));
    
    SET_BIT(obj_flags.wear_flags, ITEM_TAKE);
  } else {
    buf2 = fmt("pool puddle %s %s") %
      stripColorCodes(liquidInfo[getDrinkType()]->name) %
      stripColorCodes(liquidInfo[getDrinkType()]->color);
    name = mud_str_dup(buf2);

    sprintf(buf, poolname[drinkindex], liqname);
    shortDescr = mud_str_dup(buf);
    
    sprintf(buf, pooldesc[drinkindex], liqname);
    setDescr(mud_str_dup(buf));

    REMOVE_BIT(obj_flags.wear_flags, ITEM_TAKE);
  }
}

bool TPool::isPluralItem() const
{
  // a few drops of blood
  if (getDrinkIndex() == 0)
    return TRUE;

  // otherwise, make recursive
  return TBaseCup::isPluralItem();
}

int TRoom::dropPool(int amt, liqTypeT liq)
{
  TPool *pool=NULL;
  TThing *t;
  TObj *obj;

  if(amt==0)
    return FALSE;

  /* look for preexisting pool */
  for(t=getStuff();t;t=t->nextThing){
    TPool *tp = dynamic_cast<TPool *>(t);
    if (tp && (tp->getDrinkType() == liq)){
      pool=tp;
      break;
    }
  }

  if(!pool){
    // create new pool
#if 1
// builder port uses stripped down database which was causing problems
// hence this setup instead.
    int robj = real_object(GENERIC_POOL);
    if (robj < 0 || robj >= (signed int) obj_index.size()) {
      vlogf(LOG_BUG, fmt("dropPool(): No object (%d) in database!") %  GENERIC_POOL);
      return false;
    }

    obj = read_object(robj, REAL);
#else
    obj = read_object(GENERIC_POOL, VIRTUAL);
#endif
    pool = dynamic_cast<TPool *>(obj);
    if (!pool)
      return false;
    pool->initPool(amt, liq);

    *this += *pool;
  } else {
    pool->fillMeAmount(amt, liq);
  }

  return TRUE;

}

int TBeing::dropPool(int amt, liqTypeT liq)
{
  TPool *pool=NULL;
  TThing *t;
  TObj *obj;

  if(amt==0)
    return FALSE;

  /* look for preexisting pool */
  for(t=roomp->getStuff();t;t=t->nextThing){
    TPool *tp = dynamic_cast<TPool *>(t);
    if (tp && (tp->getDrinkType() == liq)){
      pool=tp;
      break;
    }
  }

  if(!pool){
    // create new pool
#if 1
// builder port uses stripped down database which was causing problems
// hence this setup instead.
    int robj = real_object(GENERIC_POOL);
    if (robj < 0 || robj >= (signed int) obj_index.size()) {
      vlogf(LOG_BUG, fmt("dropPool(): No object (%d) in database!") %  GENERIC_POOL);
      return false;
    }

    obj = read_object(robj, REAL);
#else
    obj = read_object(GENERIC_POOL, VIRTUAL);
#endif
    pool = dynamic_cast<TPool *>(obj);
    if (!pool)
      return false;
    pool->initPool(amt, liq);

    *roomp += *pool;
  } else {
    pool->fillMeAmount(amt, liq);
  }

  return TRUE;
}

void TPool::decayMe()
{
  int drinkunits=getDrinkUnits();

  // check for non-decaying pools
  if (isDrinkConFlag(DRINK_PERM))
    return;

  if(drinkunits<=0)
    setDrinkUnits(0);
  else if(drinkunits<25)
    addToDrinkUnits(-1);
  else // large pools evaporate faster
    addToDrinkUnits(-(drinkunits/25)); 
}


void TPool::initPool(int amt, liqTypeT liq)
{
  swapToStrung();
  REMOVE_BIT(obj_flags.wear_flags, ITEM_TAKE);
  setDrinkType(liq);
  canBeSeen = 1;
  setMaterial(MAT_WATER);
  
  fillMeAmount(amt, liq);
}


TPool::TPool() :
  TBaseCup()
{
}

TPool::TPool(const TPool &a) :
  TBaseCup(a)
{
}

TPool & TPool::operator=(const TPool &a)
{
  if (this == &a) return *this;
  TBaseCup::operator=(a);
  return *this;
}

TPool::~TPool()
{
}

void TPool::fillMeAmount(int amt, liqTypeT liq)
{
  setDrinkType(liq);
  addToDrinkUnits(amt);

  weightChangeObject(amt * SIP_WEIGHT);
}

