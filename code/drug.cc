//////////////////////////////////////////////////////////////////////////
//
// SneezyMUD - All rights reserved, SneezyMUD Coding Team
//
// $Log: drug.cc,v $
// Revision 1.1  1999/09/12 17:24:04  sneezy
// Initial revision
//
//
//////////////////////////////////////////////////////////////////////////


///////////////////////////////////////////////////////////////////////////
//
//      SneezyMUD++ - All rights reserved, SneezyMUD Coding Team
//      "drug.cc" - Methods for TDrug class
//
///////////////////////////////////////////////////////////////////////////

#include "stdsneezy.h"

TDrug::TDrug() :
  TObj(),
  curFuel(0),
  maxFuel(0),
  drugType(DRUG_NONE)
{
}

TDrug::TDrug(const TDrug &a) :
  TObj(a),
  curFuel(a.curFuel),
  maxFuel(a.maxFuel),
  drugType(a.drugType)
{
}

TDrug & TDrug::operator=(const TDrug &a)
{
  if (this == &a) return *this;

  TObj::operator=(a);
  curFuel = a.curFuel;
  maxFuel = a.maxFuel;
  drugType = a.drugType;
  return *this;
}

TDrug::~TDrug()
{
}

void TDrug::addToMaxFuel(int n)
{
  maxFuel += n;
}

void TDrug::setMaxFuel(int n)
{
  maxFuel = n;
}

int TDrug::getMaxFuel() const
{
  return maxFuel;
}

void TDrug::addToCurFuel(int n)
{
  curFuel += n;
}

void TDrug::setCurFuel(int n)
{
  curFuel = n;
}

int TDrug::getCurFuel() const
{
  return curFuel;
}

void TDrug::setDrugType(drugTypeT n)
{
  drugType = n;
}

drugTypeT TDrug::getDrugType() const
{
  return drugType;
}

void TDrug::describeObjectSpecifics(const TBeing *ch) const
{
  double diff;

  if (getMaxFuel()) {
    diff = (double) ((double) getCurFuel() / (double) getMaxFuel());
    ch->sendTo(COLOR_OBJECTS,
          "You can tell that %s has %s of its %s left.\n\r",
          good_uncap(getName()).c_str(),
          ((diff < .20) ? "very little" : ((diff < .50) ? "some" :
          ((diff < .75) ? "a good bit of" : "almost all of its"))),
	       drugTypes[getDrugType()].name);
  }
}

void TDrug::refuelMeDrug(TBeing *ch, TDrugContainer *lamp)
{
  int use;
  char buf[256];

  if (lamp->getMaxBurn() < 0) {
    act("$p can't be refueled.", FALSE, ch, lamp, 0, TO_CHAR);
    return;
  }
  if (lamp->getCurBurn() == lamp->getMaxBurn()) {
    act("$p is already full of fuel.", FALSE, ch, lamp, 0, TO_CHAR);
    return;
  }
  if (((lamp->getDrugType() != getDrugType()) && lamp->getCurBurn()>0) ||
      getDrugType()==DRUG_NONE) {
    ch->sendTo("Mixing drugs would not be wise.\n\r");
    return;
  }

  use = lamp->getMaxBurn() - lamp->getCurBurn();
  use = min(use, getCurFuel());

  sprintf(buf, "$n packs some %s into $s $o", drugTypes[getDrugType()].name);
  act(buf, TRUE, ch, lamp, 0, TO_ROOM);
  ch->sendTo("You pack some %s into the %s.\n\r", drugTypes[getDrugType()].name,
	     fname(lamp->name).c_str());

  addToCurFuel(-use);
  lamp->addToCurBurn(use);
  lamp->setDrugType(getDrugType());

  if (getCurFuel() <= 0) {
    ch->sendTo("Your %s is all used up, and you discard it.\n\r",
	       drugTypes[getDrugType()].name);
    if (equippedBy) {
      dynamic_cast<TBeing *>(equippedBy)->unequip(eq_pos);
    } else
      --(*this);

    delete this;
  }
}

void TDrug::assignFourValues(int x1, int x2, int x3, int)
{
  setCurFuel(x1);
  setMaxFuel(x2);

  drugTypeT dtt = mapFileToDrug(x3);
  setDrugType(dtt);
}

void TDrug::getFourValues(int *x1, int *x2, int *x3, int *x4) const
{
  *x1 = getCurFuel();
  *x2 = getMaxFuel();
  *x3 = mapDrugToFile(getDrugType());
  *x4 = 0;
}

void TDrug::lowCheck()
{
  if (getCurFuel() > getMaxFuel())
    vlogf(LOW_ERROR,"fuel %s had more current fuel than max.", getName());

  TObj::lowCheck();
}

int TDrug::objectSell(TBeing *ch, TMonster *keeper)
{
  char buf[256];

  sprintf(buf, "%s I'm sorry, I don't buy back drugs.", ch->getName());
  keeper->doTell(buf);
  return TRUE;
}

string TDrug::statObjInfo() const
{
  char buf[256];

  sprintf(buf, "Refuel capability : current : %d, max : %d",
         getCurFuel(), getMaxFuel());

  string a(buf);
  return a;
}

int TDrug::getVolume() const
{
  int amt = TObj::getVolume();

  amt *= getCurFuel();
  if(getMaxFuel())
    amt /= getMaxFuel();

  return amt;
}

float TDrug::getTotalWeight(bool pweight) const
{
  float amt = TObj::getTotalWeight(pweight);

  amt *= getCurFuel();
  if(getMaxFuel())
    amt /= getMaxFuel();

  return amt;
}

drugData::drugData() :
  first_use(),
  last_use(),
  total_consumed(0),
  current_consumed(0)
{
}

drugData::~drugData()
{
}

drugData::drugData(const drugData &t) :
  first_use(t.first_use),
  last_use(t.last_use),
  total_consumed(t.total_consumed),
  current_consumed(t.current_consumed)
{
}

drugData & drugData::operator =(const drugData &t)
{
  if (this == &t) return *this;

  first_use=t.first_use;
  last_use=t.last_use;
  total_consumed=t.total_consumed;
  current_consumed=t.current_consumed;

  return *this;
}

int TBeing::doSmoke(const char *argument)
{
  TDrugContainer *tdc;
  TThing *t;
  char arg[256];
  int hit_size=1, consumed, potency;
  affectedData aff;

  only_argument(argument, arg);

  if(!*arg || !(t=get_thing_char_using(this, arg, 0, FALSE, FALSE))){
    sendTo("Smoke what?\n\r");
    return FALSE;
  }
  if(!(tdc=dynamic_cast<TDrugContainer *>(t))){
    sendTo("You can't smoke that.\n\r");
    return FALSE;
  }
  if(!tdc->getLit()){
    sendTo("How could you smoke it, when it isn't even lit!\n\r");
    return FALSE;
  }
  if(tdc->getCurBurn()<1){
    sendTo("You can't smoke that, it's empty.\n\r");
    return FALSE;
  }

  sendTo("Ok, you smoke the %s.\n\r", drugTypes[tdc->getDrugType()].name);

  // Update drug stats
  tdc->addToCurBurn(-hit_size);
  if(!desc->drugs[tdc->getDrugType()].total_consumed)
    desc->drugs[tdc->getDrugType()].first_use=time_info;
  desc->drugs[tdc->getDrugType()].last_use=time_info;
  desc->drugs[tdc->getDrugType()].total_consumed+=hit_size;
  desc->drugs[tdc->getDrugType()].current_consumed+=hit_size;

  saveDrugStats();


  // Create/increase affect(s)
  aff.type = AFFECT_DRUG;
  //  aff.level = level;
  aff.bitvector = 0;
  aff.modifier2 = tdc->getDrugType();

  consumed=desc->drugs[tdc->getDrugType()].current_consumed;
  potency=drugTypes[tdc->getDrugType()].potency;
  if(consumed>potency) consumed=potency;

  aff.duration = drugTypes[tdc->getDrugType()].duration * UPDATES_PER_TICK;
  aff.renew=aff.duration;


  switch(tdc->getDrugType()){
    case DRUG_PIPEWEED:
      aff.modifier = (consumed * 5) / potency;
      aff.location = APPLY_SPE;
      //      affectTo(&aff, aff.renew);

      if(!affectJoin(this, &aff, AVG_DUR_NO, AVG_EFF_YES))
	vlogf(0, "affectJoin failed in doSmoke");

      
#if 0


      aff.modifier = (consumed * 5) / potency;
      aff.location = APPLY_MOVE;
      if(!affectJoin(this, &aff, AVG_DUR_NO, AVG_EFF_YES))
	vlogf(0, "affectJoin failed in doSmoke");

      aff.modifier = -((consumed * 5) / potency);
      aff.location = APPLY_CHA;
      if(!affectJoin(this, &aff, AVG_DUR_NO, AVG_EFF_YES))
	vlogf(0, "affectJoin failed in doSmoke");

      aff.modifier = -((consumed * 5) / potency);
      aff.location = APPLY_FOC;
      if(!affectJoin(this, &aff, AVG_DUR_NO, AVG_EFF_YES))
	vlogf(0, "affectJoin failed in doSmoke");
#endif     
 
      break;
    case DRUG_OPIUM:
    case DRUG_NONE: default:
      vlogf(0, "reached default case for drug type in doSmoke");
  }


  return TRUE;
}

TDrugInfo::TDrugInfo(const char *n, int p, int d) :
  name(n),
  potency(p),
  duration(d)
{
}

TDrugInfo::TDrugInfo() :
  name(NULL),
  potency(0),
  duration(0)
{
}

TDrugInfo::TDrugInfo(const TDrugInfo &a) :
  name(a.name),
  potency(a.potency),
  duration(a.duration)
{
}

TDrugInfo & TDrugInfo::operator=(const TDrugInfo &a)
{
  if (this == &a) return *this;

  name = a.name;
  potency = a.potency;
  duration = a.duration;

  return *this;
}

TDrugInfo::~TDrugInfo()
{
}

vector<TDrugInfo>drugTypes(0);

void assign_drug_info(void)
{
  drugTypes.push_back(TDrugInfo("none", 0, 0));
  drugTypes.push_back(TDrugInfo("pipeweed", 10, 10));
  drugTypes.push_back(TDrugInfo("opium", 0, 0));
}
