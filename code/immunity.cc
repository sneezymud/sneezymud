//////////////////////////////////////////////////////////////////////////
//
// SneezyMUD - All rights reserved, SneezyMUD Coding Team
//
// $Log: immunity.cc,v $
// Revision 1.1  1999/09/12 17:24:04  sneezy
// Initial revision
//
//
//////////////////////////////////////////////////////////////////////////


// Immunities.cc
//
// Class for Immunity Data.

#include "stdsneezy.h"
#include "immunity.h"

// Constructor.  Zero out everything.
Immunities::Immunities()
{
  for (immuneTypeT i=MIN_IMMUNE; i < MAX_IMMUNES; i++)
    ImmunityArray[i] = 0;
}

// convert() is a utility function to switch from const char *
// to immune_t so other functions can access the ImmunityArray.
immuneTypeT Immunities::convert(const string & immunity) const
{
  if (!immunity.compare("IMMUNE_HEAT"))
    return IMMUNE_HEAT;
  if (!immunity.compare("IMMUNE_COLD"))
    return IMMUNE_COLD;
  if (!immunity.compare("IMMUNE_ACID"))
    return IMMUNE_ACID;
  if (!immunity.compare("IMMUNE_POISON"))
    return IMMUNE_POISON;
  if (!immunity.compare("IMMUNE_SLEEP"))
    return IMMUNE_SLEEP;
  if (!immunity.compare("IMMUNE_PARALYSIS"))
    return IMMUNE_PARALYSIS;
  if (!immunity.compare("IMMUNE_CHARM"))
    return IMMUNE_CHARM;
  if (!immunity.compare("IMMUNE_PIERCE"))
    return IMMUNE_PIERCE;
  if (!immunity.compare("IMMUNE_SLASH"))
    return IMMUNE_SLASH;
  if (!immunity.compare("IMMUNE_BLUNT"))
    return IMMUNE_BLUNT;
  if (!immunity.compare("IMMUNE_NONMAGIC"))
    return IMMUNE_NONMAGIC;
  if (!immunity.compare("IMMUNE_PLUS1"))
    return IMMUNE_PLUS1;
  if (!immunity.compare("IMMUNE_PLUS2"))
    return IMMUNE_PLUS2;
  if (!immunity.compare("IMMUNE_PLUS3"))
    return IMMUNE_PLUS3;
  if (!immunity.compare("IMMUNE_AIR"))
    return IMMUNE_AIR;
  if (!immunity.compare("IMMUNE_ENERGY"))
    return IMMUNE_ENERGY;
  if (!immunity.compare("IMMUNE_ELECTRICITY"))
    return IMMUNE_ELECTRICITY;
  if (!immunity.compare("IMMUNE_DISEASE"))
    return IMMUNE_DISEASE;
  if (!immunity.compare("IMMUNE_SUFFOCATION"))
    return IMMUNE_SUFFOCATION;
  if (!immunity.compare("IMMUNE_SKIN_COND"))
    return IMMUNE_SKIN_COND;
  if (!immunity.compare("IMMUNE_BONE_COND"))
    return IMMUNE_BONE_COND;
  if (!immunity.compare("IMMUNE_BLEED"))
    return IMMUNE_BLEED;
  if (!immunity.compare("IMMUNE_WATER"))
    return IMMUNE_WATER;
  if (!immunity.compare("IMMUNE_DRAIN"))
    return IMMUNE_DRAIN;
  if (!immunity.compare("IMMUNE_FEAR"))
    return IMMUNE_FEAR;
  if (!immunity.compare("IMMUNE_EARTH"))
    return IMMUNE_EARTH;
  if (!immunity.compare("IMMUNE_SUMMON"))
    return IMMUNE_SUMMON;
  if (!immunity.compare("IMMUNE_UNUSED2"))
    return IMMUNE_UNUSED2;

  vlogf(5, "Unknown immunity '%s', in convert()", immunity.c_str());
  return IMMUNE_NONE;
}

// setImmunity() assigns a percentage to a particular immunity.
void Immunities::setImmunity(const string whichImmunity, byte percent)
{
  immuneTypeT itt = convert(whichImmunity);
  ImmunityArray[itt] = percent;
}

// getImmunity() returns the value of the particular immunity.
byte Immunities::getImmunity(immuneTypeT whichImmunity) const
{
  return ImmunityArray[whichImmunity];
}

byte TBeing::getImmunity(immuneTypeT type) const
{
  int amount, imm;

  if (hasClass(CLASS_MONK) && doesKnowSkill(SKILL_DUFALI)) {
    amount = getSkillValue(SKILL_DUFALI);
    amount = max(amount, 0);
    switch(type){
      case IMMUNE_PARALYSIS: 
	imm=immunities.immune_arr[type]+(amount/3);
	break;
      case IMMUNE_CHARM:
	imm=immunities.immune_arr[type]+amount;
	break;
      case IMMUNE_POISON:
	imm=immunities.immune_arr[type]+(amount/2);
	break;
      default:
	imm=immunities.immune_arr[type];
    }
    if (imm < -100)
      imm=-100;
    if (imm > 100)
      imm=100;
    
    return imm;
  } else {  // non-monks
    return immunities.immune_arr[type];
  }
}

void TBeing::setImmunity(immuneTypeT type, byte amt)
{
  immunities.immune_arr[type] = amt;
}

void TBeing::addToImmunity(immuneTypeT type, byte amt)
{
  immunities.immune_arr[type] = min(max(immunities.immune_arr[type] + amt, -100), 100);
}

bool TBeing::isImmune(immuneTypeT bit, int modifier) const
{
#if 1
  // this is used to flat out deny some things, so lets not have
  // if too easy to pass true
  // the higher modifier is, the HARDER it ought to be to succeed
  int gi = getImmunity(bit);
  if (gi >= 100)
   return TRUE;
  if (gi <= -100)
    return FALSE;

  return gi > (modifier+50);
#else
  if (getImmunity(bit) >= 100)
   return TRUE;
  if (getImmunity(bit) <= -100)
    return FALSE;

  int level = GetMaxLevel() - modifier;
  if (level < 0)
    level *= 3;

  int num = ::number(-100,100);

  return ((num - level) < getImmunity(bit));
#endif
}
