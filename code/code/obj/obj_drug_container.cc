///////////////////////////////////////////////////////////////////////////
//
//      SneezyMUD++ 4.5 - All rights reserved, SneezyMUD Coding Team
//      "drugs_container.cc" - Methods for TDrugContainer class
//
//      Last revision December 18, 1997.
//
///////////////////////////////////////////////////////////////////////////

#include "comm.h"
#include "room.h"
#include "obj_drug.h"
#include "extern.h"
#include "being.h"
#include "obj_drug_container.h"

TDrugContainer::TDrugContainer() :
  TObj(),
  drugType(DRUG_NONE),
  maxBurn(0),
  curBurn(0),
  lit(0)
{
}

TDrugContainer::TDrugContainer(const TDrugContainer &a) :
  TObj(a),
  drugType(a.drugType),
  maxBurn(a.maxBurn),
  curBurn(a.curBurn),
  lit(a.lit)
{
}

TDrugContainer & TDrugContainer::operator=(const TDrugContainer &a)
{
  if (this == &a) return *this;
  TObj::operator=(a);
  drugType = a.drugType;
  maxBurn = a.maxBurn;
  curBurn = a.curBurn;
  lit = a.lit;
  return *this;
}

TDrugContainer::~TDrugContainer()
{
}


void TDrugContainer::addToMaxBurn(int n)
{
  maxBurn += n;
}

void TDrugContainer::setMaxBurn(int n)
{
  maxBurn = n;
}

int TDrugContainer::getMaxBurn() const
{
  return maxBurn;
}

void TDrugContainer::addToCurBurn(int n)
{
  curBurn += n;
}

void TDrugContainer::setCurBurn(int n)
{
  curBurn = n;
}

int TDrugContainer::getCurBurn() const
{
  return curBurn;
}

void TDrugContainer::setLit(bool n)
{
  lit = n;
}

bool TDrugContainer::getLit() const
{
  return lit;
}

void TDrugContainer::setDrugType(drugTypeT n)
{
  drugType = n;
}

drugTypeT TDrugContainer::getDrugType() const
{
  return drugType;
}

bool TDrugContainer::monkRestrictedItem(const TBeing *) const
{
  return FALSE;
}

bool TDrugContainer::shamanRestrictedItem(const TBeing *) const
{
  return FALSE;
}

bool TDrugContainer::rangerRestrictedItem(const TBeing *) const
{
  return FALSE;
}

void TDrugContainer::putLightOut()
{
  setLit(FALSE);
}

void TDrugContainer::extinguishWater(TBeing *ch)
{
  if (getLit()) {
    act("$p is put out by the room's water.", TRUE, ch, this, 0, TO_CHAR);
    act("$p is put out by the room's water.", TRUE, ch, this, 0, TO_ROOM);

    putLightOut();
  }
}

void TDrugContainer::extinguishWater()
{
  if (getLit()) {
    act("$p is put out by the room's water.", TRUE, 0, this, 0, TO_ROOM);
    putLightOut();
  }
}

void TDrugContainer::lightDecay()
{
  if (getLit()) {
    addToCurBurn(-1);
    if (getCurBurn() <= 0) {
      setCurBurn(0);
      putLightOut();
      setDrugType(DRUG_NONE);


      if (roomp && !roomp->stuff.empty()) {
        act("With a puff of smoke, $p burns out.",
                 FALSE, roomp->stuff.front(), this, 0, TO_CHAR);
      } else if (parent) {
        act("With a puff of smoke, $p burns out.",
                FALSE, parent, this, 0, TO_CHAR);
      } else if (equippedBy) {
        act("With a puff of smoke, $p burns out.",
                FALSE, equippedBy, this, 0, TO_CHAR);
      }
    }
  }
}

void TDrugContainer::extinguishMe(TBeing *ch)
{
  if (!getLit()) {
    ch->sendTo("That is already extinguished!\n\r");
    return;
  }
  putLightOut();

  act("You extinguish $p, and it smolders slightly before going out.", FALSE, ch, this, 0, TO_CHAR);
  act("$n extinguishes $p, and it smolders slightly before going out.", FALSE, ch, this, 0, TO_ROOM);
  return;
}

void TDrugContainer::refuelMeLight(TBeing *ch, TThing *fuel)
{
    fuel->refuelMeDrug(ch, this);
}

void TDrugContainer::assignFourValues(int x1, int x2, int x3, int x4)
{
  drugTypeT dtt = mapFileToDrug(x1);
  setDrugType(dtt);

  setMaxBurn(x2);
  setCurBurn(x3);
  setLit(x4);
}

void TDrugContainer::getFourValues(int *x1, int *x2, int *x3, int *x4) const
{
  *x1 = mapDrugToFile(getDrugType());
  *x2 = getMaxBurn();
  *x3 = getCurBurn();
  *x4 = getLit();
}

void TDrugContainer::lowCheck()
{
  int i;

  for (i=0; i<MAX_OBJ_AFFECT;i++) {
    if (affected[i].location == APPLY_LIGHT) {
      if (!getLit())
        vlogf(LOG_LOW,format("item %s was defined apply-light.") % getName());
    }
  }
}

sstring TDrugContainer::statObjInfo() const
{
  char buf[256];

  sprintf(buf, "DrugContainer: drug: %s, Max drug: %s%d, drug left: %d, Lit? : %s",
	  drugTypes[getDrugType()].name,
          (getMaxBurn() <= 0 ? "non-refuelable :" : ""),
          getMaxBurn(),
          getCurBurn(),
          (getLit() ? "Yes" : "No"));

  sstring a(buf);
  return a;
}

int TDrugContainer::objectDecay()
{
  if (roomp) {
    act("$p flickers then fades into insignificance.",
         TRUE, roomp->stuff.front(), this, 0, TO_CHAR);
    act("$p flickers then fades into insignificance.",
         TRUE, roomp->stuff.front(), this, 0, TO_ROOM);
  } else {
    TThing *t = NULL;
    if (parent)
      t = parent;
    else if (equippedBy)
      t = equippedBy;
    else if (stuckIn)
      t = stuckIn;
  
    if (t) {
      act("Your $o flickers then fades into insignificance.",
         TRUE, t, this, 0, TO_CHAR);
      act("$n's $o flickers then fades into insignificance.",
         TRUE, t, this, 0, TO_ROOM);
    }
  }

  return DELETE_THIS;
}

sstring TDrugContainer::showModifier(showModeT, const TBeing *) const
{
  sstring a;

  if (getLit())
    a = " (lit)";
  else
    a = "";

  return a;
}

void TDrugContainer::describeObjectSpecifics(const TBeing *ch) const
{
  double diff;

  if (!getLit())
    act("$p is not lit.", FALSE, ch, this, 0, TO_CHAR);
  else
    act("$p is lit.", FALSE, ch, this, 0, TO_CHAR);

  if (getMaxBurn() < 0)
    act("$p doesn't seem to be reusable.", FALSE, ch, this, 0, TO_CHAR);
  else 
    ch->sendTo(COLOR_OBJECTS,format("%s is reusable.\n\r") % sstring(getName()).cap());
  
  diff = (double) ((double) getCurBurn() / max(1.0, (double) getMaxBurn()));
  if(diff==0 || getDrugType()==DRUG_NONE)
    ch->sendTo(COLOR_OBJECTS, format("%s is completely empty.\n\r") %
	       sstring(getName()).uncap());
  else      
    ch->sendTo(COLOR_OBJECTS, format("You can tell that %s has %s %s left.\n\r") % sstring(getName()).uncap() %
	       (((diff == 0) ? "no" :
		((diff < .20) ? "very little" :
		 ((diff < .50) ? "some" :
		  ((diff < .75) ? "a good bit of" : 
                    ((diff==1.00) ? "all of" : "almost all of its")))))) %
	       drugTypes[getDrugType()].name);
}

bool TDrugContainer::isSimilar(const TThing *t) const
{
  const TDrugContainer * tl = dynamic_cast<const TDrugContainer *>(t);
  if (!tl)
    return false;

  // lit status identical is required
  if (getLit() && !tl->getLit())
    return false;
  else if (!getLit() && tl->getLit())
    return false;
  else if (getDrugType() != tl->getDrugType())
    return false;

  return TObj::isSimilar(t);
}

void TDrugContainer::peeOnMe(const TBeing *ch)
{
  if(getLit()){
    act("$p sputters, sparks then finally relents to $n's downpour of <y>pee<1> and goes out.", TRUE, ch, this, NULL, TO_ROOM);
    act("$p sputters, sparks then finally relents to your downpour of <y>pee<1> and goes out.", TRUE, ch, this, NULL, TO_CHAR);

    putLightOut();
  } else
    act("You try to light $p by peeing on it, but sadly it does not appear to be working.", TRUE, ch, this, NULL, TO_CHAR);
}


void TDrugContainer::lightMe(TBeing *ch, silentTypeT silent)
{
  char buf[256];

  if (getLit()) {
    if(!silent)
      act("$p is already lit!", FALSE, ch, this, 0, TO_CHAR);
    return;
  }
  if (ch->roomp->isUnderwaterSector()) {
    if(!silent)
      ch->sendTo("Impossible! You are underwater!\n\r");
    return;
  }
  if (getCurBurn() <= 0) {
    if(!silent)
      act("$p is empty!", FALSE, ch, this, 0, TO_CHAR);
    return;
  }
  setLit(TRUE);
  if(!silent){
    sprintf(buf, "You light $p, and the %s begins to burn.", 
	    drugTypes[getDrugType()].name);
    act(buf, FALSE, ch, this, 0, TO_CHAR);
    sprintf(buf, "$n light $p, and the %s begins to burn.", 
	    drugTypes[getDrugType()].name);
    act(buf, TRUE, ch, this, 0, TO_ROOM);
  }
  return;
}

