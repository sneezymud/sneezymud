//////////////////////////////////////////////////////////////////////////
//
// SneezyMUD - All rights reserved, SneezyMUD Coding Team
//
//
//
//////////////////////////////////////////////////////////////////////////


#ifndef __COLORSTRING_H
#define __COLORSTRING_H

sstring stripColorCodes(sstring s);

bool hasColorStrings(const TBeing *, const char *, int);

sstring addNameToBuf(const TBeing *, const Descriptor *, const TThing *, const char *, colorTypeT);

sstring nameColorString(TBeing *, Descriptor *, const char *, int *, colorTypeT, bool = false);

const sstring colorString(const TBeing *, const Descriptor *, const char *, int *, colorTypeT, bool, bool = false);


#endif
