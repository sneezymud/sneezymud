//////////////////////////////////////////////////////////////////////////
//
// SneezyMUD - All rights reserved, SneezyMUD Coding Team
//
//
//
//////////////////////////////////////////////////////////////////////////


#ifndef __COLORSTRING_H
#define __COLORSTRING_H

string stripColorCodes(string s);

bool hasColorStrings(const TBeing *, const char *, int);

string addNameToBuf(const TBeing *, const Descriptor *, const TThing *, const char *, colorTypeT);

string nameColorString(TBeing *, Descriptor *, const char *, int *, colorTypeT, bool = false);

const string colorString(const TBeing *, const Descriptor *, const char *, int *, colorTypeT, bool, bool = false);


#endif
