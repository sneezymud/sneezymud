//////////////////////////////////////////////////////////////////////////
//
// SneezyMUD - All rights reserved, SneezyMUD Coding Team
//
// $Log: obj_base_container.cc,v $
// Revision 1.1  1999/09/12 17:24:04  sneezy
// Initial revision
//
//
//////////////////////////////////////////////////////////////////////////


// container.cc
//

#include "stdsneezy.h"

TContainer::TContainer() :
  TObj()
{
}

TContainer::TContainer(const TContainer &a) :
  TObj(a)
{
}

TContainer & TContainer::operator=(const TContainer &a)
{
  if (this == &a) return *this;
  TObj::operator=(a);
  return *this;
}

TContainer::~TContainer()
{
}

bool TContainer::engraveMe(TBeing *ch, TMonster *me, bool give)
{
  char buf[256];

  // engraved bags would protect too many things
  sprintf(buf, "%s The powers that be say I can't do that anymore.", ch->getName
());
  me->doTell(buf);

  if (give) {
    strcpy(buf, name);
    add_bars(buf);
    sprintf(buf + strlen(buf), " %s", fname(ch->name).c_str());
    me->doGive(buf);
  }
  return TRUE;
}

int TContainer::stealModifier()
{
  return 50;   // make bags tough to steal
}

int TContainer::getReducedVolume(const TThing *) const
{
  return getTotalVolume();
}

