//////////////////////////////////////////////////////////////////////////
//
// SneezyMUD - All rights reserved, SneezyMUD Coding Team
//
//
//
//////////////////////////////////////////////////////////////////////////


#ifndef __COLORSTRING_H
#define __COLORSTRING_H

#include "ansi.h"

class sstring;
class TBeing;
class TThing;
class Descriptor;

sstring stripColorCodes(const sstring &s);

bool hasColorStrings(const TBeing *, const sstring &, int);

sstring addNameToBuf(const TBeing *, const Descriptor *, const TThing *, const sstring &, colorTypeT);

sstring nameColorString(TBeing *, Descriptor *, const sstring &, int *, colorTypeT, bool = false);

const sstring colorString(const TBeing *, const Descriptor *, const sstring &, int *, colorTypeT, bool, bool = false);


#endif
