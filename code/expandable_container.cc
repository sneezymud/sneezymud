//////////////////////////////////////////////////////////////////////////
//
// SneezyMUD - All rights reserved, SneezyMUD Coding Team
//
// $Log: expandable_container.cc,v $
// Revision 5.1  1999/10/16 04:31:17  batopr
// new branch
//
// Revision 1.1  1999/09/12 17:24:04  sneezy
// Initial revision
//
//
//////////////////////////////////////////////////////////////////////////


// expandable_container.cc
//

#include "stdsneezy.h"

TExpandableContainer::TExpandableContainer() :
  TRealContainer()
{
}

TExpandableContainer::TExpandableContainer(const TExpandableContainer &a) :
  TRealContainer(a)
{
}

TExpandableContainer & TExpandableContainer::operator=(const TExpandableContainer &a)
{
  if (this == &a) return *this;
  TRealContainer::operator=(a);
  return *this;
}

TExpandableContainer::~TExpandableContainer()
{
}

void TExpandableContainer::assignFourValues(int x1, int x2, int x3, int x4)
{
  TRealContainer::assignFourValues(x1, x2, x3, x4);
}

void TExpandableContainer::getFourValues(int *x1, int *x2, int *x3, int *x4) const
{
  TRealContainer::getFourValues(x1, x2, x3, x4);
}

string TExpandableContainer::statObjInfo() const
{
  return TRealContainer::statObjInfo();
}

int TExpandableContainer::getTotalVolume() const
{
  int num = getVolume();
  num += getCarriedVolume();
  return num;
}

void TExpandableContainer::addToCarriedVolume(int num)
{
  // make this recursive...
  TRealContainer::addToCarriedVolume(num);

  // increase my parent's volume if *I* am able to expand
  if (parent)
    parent->addToCarriedVolume(num);

  // not sure why this here, tables???
  // it was here before the switch to TExpand, shrug
  if (riding)
    riding->addToCarriedVolume(num);
}


