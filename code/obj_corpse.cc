//////////////////////////////////////////////////////////////////////////
//
// SneezyMUD - All rights reserved, SneezyMUD Coding Team
//
// corpse.cc
//
//////////////////////////////////////////////////////////////////////////


#include "stdsneezy.h"
#include "obj_corpse.h"
#include "obj_base_corpse.h"

TCorpse::TCorpse() :
  TBaseCorpse()
{
}

TCorpse::TCorpse(const TCorpse &a) :
  TBaseCorpse(a)
{
}

TCorpse & TCorpse::operator=(const TCorpse &a)
{
  if (this == &a) return *this;
  TBaseCorpse::operator=(a);
  return *this;
}

TCorpse::~TCorpse()
{
}

sstring TCorpse::statObjInfo() const
{
  char buf[256];

  sprintf(buf, "Corpse Flags: %d, Corpse race: %d:%s\n\r", 
       getCorpseFlags(), getCorpseRace(), RaceNames[getCorpseRace()]);
  sprintf(buf + strlen(buf), "Corpse Level: %d, Corpse Vnum: %d\n\r", 
       getCorpseLevel(), getCorpseVnum());

  sstring a(buf);
  return a;
}
