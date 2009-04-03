//////////////////////////////////////////////////////////////////////////
//
// SneezyMUD - All rights reserved, SneezyMUD Coding Team
//
//
//////////////////////////////////////////////////////////////////////////


// Immunity.h
//
// Class for Immunity Data.

#ifndef __IMMUNITY_H
#define __IMMUNITY_H

enum immuneTypeT {
     IMMUNE_NONE = -1,
     IMMUNE_HEAT = 0,
     IMMUNE_COLD,
     IMMUNE_ACID,
     IMMUNE_POISON,
     IMMUNE_SLEEP,
     IMMUNE_PARALYSIS,
     IMMUNE_CHARM,
     IMMUNE_PIERCE,
     IMMUNE_SLASH,
     IMMUNE_BLUNT,
     IMMUNE_NONMAGIC,
     IMMUNE_PLUS1,
     IMMUNE_PLUS2,
     IMMUNE_PLUS3,
     IMMUNE_AIR,
     IMMUNE_ENERGY,
     IMMUNE_ELECTRICITY,
     IMMUNE_DISEASE,
     IMMUNE_SUFFOCATION,
     IMMUNE_SKIN_COND,
     IMMUNE_BONE_COND,
     IMMUNE_BLEED,
     IMMUNE_WATER,
     IMMUNE_DRAIN,
     IMMUNE_FEAR,
     IMMUNE_EARTH,
     IMMUNE_SUMMON,
     IMMUNE_UNUSED2,
     MAX_IMMUNES
};
// more then 28 will be real bad
// affects the mob file
extern immuneTypeT & operator++(immuneTypeT &, int);
const immuneTypeT MIN_IMMUNE = immuneTypeT(0);

class Immunities {
  private:
    byte ImmunityArray[MAX_IMMUNES];

  public:

    Immunities();
    void setImmunity(const sstring &whichImmunity, byte percent);
    byte getImmunity(immuneTypeT whichImmunity) const;

    // convert() is a utility function to switch from const char *
    // to immune_t so other functions can access the ImmunityArray.
    immuneTypeT convert(const sstring & immunity) const;
};

#endif
