//////////////////////////////////////////////////////////////////////////
//
// SneezyMUD - All rights reserved, SneezyMUD Coding Team
//
//////////////////////////////////////////////////////////////////////////


// limbs.cc
//
//  Time for some more magic.

#include "stdsneezy.h"
#include <cmath>

const char *limbNames[MAX_LIMB_TYPES] = {
  "HEAD", "NECK", "BACK", "ARM", "WAIST", "LEG", "WINGS", "TAIL",
  "", "", "", "", "", "", "", "", "",
  "BODY", "FOREARM", "HAND", "FOOT"};

// Constructor.  Should initialize a limb as LIMB_NONE and everything pretty
// much zeroed out.  Set up limbType, description and connectsTo based on
// params.

Limb::Limb(sstring typeOfLimb, sstring connector, sstring desc)
  : limbType(toInt(typeOfLimb)), 
  name(desc),
  connectsTo(toInt(connector))
{
  initLimb();
}

// Default constructor.

Limb::Limb() {
  initLimb();
}

// Destructor.  Make sure all sub-limbs are destroyed first.  If the limb
// is equipped, holding or wearing anything, delete them too.

Limb::~Limb() {

  if(subLimb)
    delete subLimb;
  if(equip)
    delete equip;
  if(holding)
    delete holding;
  if(jewelry)
    delete jewelry;

}

// initLimb() just initializes the limb with mostly zeros and NULL links.
void Limb::initLimb() {

  limbType = LIMB_NONE;
  name = "HELP! Limb initialized but not used!";

  limbHitPoints = 0;
  limbStatus = LIMB_NOSUBLIMB;

  equip = NULL;
  holding = NULL;
  jewelry = NULL;
  wornWeight = 0;

  flags = 0;

  connectsTo = LIMB_NONE;
  numSlots = 0;
  slotsFilled = 0;

  subLimb = NULL;
  next = NULL;
}

// search() looks for a limb type "target" with "status".  It first checks
// to see if it matches the description.  If it does, it returns itself.
// otherwise, it checks to see if it has a sublimb and if so, it searches
// that limb.  It should return found (which should default to NULL);

Limb *Limb::search(int target,int status) {
  Limb *found = NULL;

  if ((limbType == target) && (limbStatus & status))
    return this;
  else if(limbStatus ^ LIMB_NOSUBLIMB) {
    found = subLimb->search(target,status);
    return found;
  }
  return found;
}

// join() attaches a new limb to this one.

int Limb::join(Limb *newLimb) {
  if(newLimb->limbType == LIMB_NONE)
    return 0;

  limbStatus ^= LIMB_NOSUBLIMB;
  subLimb = newLimb;
  return 1;
}

int Limb::toInt(sstring limb_name) {

  for(int i=LIMB_NONE; i<MAX_LIMB_TYPES; i++)
    if(limb_name == limbNames[i])
      return i;
  return LIMB_NONE;
}

void Limb::showLimb(TBeing *caller){
  caller->sendTo(fmt("Limb: %s\n\r") %name);
}

bool VITAL_PART(wearSlotT pos)
{
  return ((pos == WEAR_HEAD) || (pos == WEAR_BODY) ||
          (pos == WEAR_BACK) || (pos == WEAR_NECK));
}

wearSlotT pickRandomLimb(bool)
{
  int num = ::number(MIN_WEAR, MAX_WEAR-1);

  return wearSlotT(num);
}

int TBeing::hurtLimb(unsigned int dam, wearSlotT part_hit)
{
  unsigned int limHlt = getCurLimbHealth(part_hit);
  sstring buf;

  if (limHlt > 0) {
    addCurLimbHealth(part_hit, -min(dam, limHlt));
    if (getCurLimbHealth(part_hit) <= 0) {
      sendTo(COLOR_BASIC, fmt("%sYour %s has become totally useless!%s\n\r") %
	     red() % describeBodySlot(part_hit) % norm());
      buf=fmt("$n's %s has become completely useless!") %
	describeBodySlot(part_hit);
      act(buf, TRUE, this, NULL, NULL, TO_ROOM, ANSI_ORANGE);
      addToLimbFlags(part_hit, PART_USELESS);

      int rc = flightCheck();
      if (IS_SET_DELETE(rc, DELETE_THIS))
        return DELETE_THIS;
    }
    return TRUE;
  }
  return FALSE;
}
