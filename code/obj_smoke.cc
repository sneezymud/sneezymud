//////////////////////////////////////////////////////////////////////////
//
// SneezyMUD - All rights reserved, SneezyMUD Coding Team
//
// $Log: obj_smoke.cc,v $
// Revision 1.1  1999/09/12 17:24:04  peel
// Initial revision
//
//
//////////////////////////////////////////////////////////////////////////


#include "stdsneezy.h"

TSmoke::TSmoke() :
  TObj()
{
}

TSmoke::TSmoke(const TSmoke &a) :
  TObj(a)
{
}

TSmoke & TSmoke::operator=(const TSmoke &a)
{
  if (this == &a) return *this;
  TObj::operator=(a);
  return *this;
}

TSmoke::~TSmoke()
{
}

int TSmoke::getSizeIndex() const
{
  int volume=getVolume();

  if(volume<=0){
    return 0;
  } else if(volume<=5){
    return 1;
  } else if(volume<=10){
    return 2;
  } else if(volume<=25){
    return 3;
  } else if(volume<=50){ 
    return 4;
  } else if(volume<=100){
    return 5;
  } else if(volume<=250){
    return 6;
  } else if(volume<=1000){
    return 7;
  } else if(volume<=10000){
    return 8;
  } else if(volume<=25000){
    return 9;
  } else {
    return 10;
  }
}
  
void TSmoke::updateDesc()
{
  int sizeindex=getSizeIndex();
  char buf[256];
  
  const char *smokename [] =
  {
    "a few drops of smoke", 
    "a tiny puddle of smoke", 
    "a small puddle of smoke", 
    "a puddle of smoke", 
    "a fair sized puddle of smoke", 
    "a big pool of smoke", 
    "a large pool of smoke", 
    "a huge pool of smoke",
    "a massive pool of smoke",
    "a tremendously huge pool of smoke",
    "a veritable ocean of smoke"
  };
  
  const char *smokedesc [] =
  {
    "A few drops of smoke sprinkle the ground here and are fading fast.",
    "A tiny puddle of smoke has gathered here.",
    "A small puddle of smoke is here.",
    "A puddle of smoke is here.",
    "A fair sized puddle of smoke is here.",
    "A big pool of smoke is here.",
    "A large pool of smoke is here.",
    "A huge pool of smoke is here.",
    "A massive pool of smoke is here.",
    "A tremendously huge pool of smoke dominates the area.",
    "A veritable ocean of smoke covers the area."
  };

  if (isObjStat(ITEM_STRUNG)) {
    delete [] shortDescr;
    delete [] descr;

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
    name = mud_str_dup(obj_index[getItemIndex()].name);
    ex_description = NULL;
    action_description = NULL;
  }

  sprintf(buf, smokename[sizeindex]);
  shortDescr = mud_str_dup(buf);

  sprintf(buf, smokedesc[sizeindex]);
  setDescr(mud_str_dup(buf));
}

bool TSmoke::isPluralItem() const
{
  // a few drops of blood
  if (getSizeIndex() == 0)
    return TRUE;

  // otherwise, make recursive
  return TObj::isPluralItem();
}

int TThing::dropSmoke(int amt)
{
  TSmoke *smoke=NULL;
  TThing *t;
  char buf[256];
  TObj *obj;

  if(amt==0 || !roomp)
    return FALSE;

  // look for preexisting smoke
  for(t=roomp->stuff;t;t=t->nextThing){
    if((smoke = dynamic_cast<TSmoke *>(t)))
      break;
  }

  if (!smoke) {
    // create new smoke
    if (!(obj = read_object(GENERIC_SMOKE, VIRTUAL))) {
      vlogf(LOW_ERROR, "Error, No GENERIC_SMOKE  (%d)", GENERIC_SMOKE);
      return FALSE;
    }
    if (!(smoke = dynamic_cast<TSmoke *>(obj))) {
      vlogf(5, "Error, unable to cast object to smoke: smoke.cc:TThing::dropSmoke");
      return FALSE;
    }
    smoke->swapToStrung();
    smoke->remObjStat(ITEM_TAKE);
    smoke->canBeSeen = 1;

    sprintf(buf, "smoke cloud");
    delete [] smoke->name;
    smoke->name = mud_str_dup(buf);

    *roomp += *smoke;
  }

  smoke->addToVolume(amt);

  return TRUE;
}

void TSmoke::decayMe()
{
  int volume=getVolume();

  if(volume<=0)
    setVolume(0);
  else if(volume<25)
    addToVolume(-1);
  else // large smokes evaporate faster
    addToVolume(-(volume/25)); 
}

string TSmoke::statObjInfo() const
{
  return "";
}

void TSmoke::getFourValues(int *x1, int *x2, int *x3, int *x4) const
{
  *x1 = 0;
  *x2 = 0;
  *x3 = 0;
  *x4 = 0;
}

void TSmoke::assignFourValues(int, int, int, int)
{
}
