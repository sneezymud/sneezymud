//////////////////////////////////////////////////////////////////////////
//
// SneezyMUD - All rights reserved, SneezyMUD Coding Team
//
// expandable_container.cc
//
//////////////////////////////////////////////////////////////////////////


#include "stdsneezy.h"
#include "obj_expandable_container.h"
#include "obj_open_container.h"

TExpandableContainer::TExpandableContainer() :
  TOpenContainer()
{
}

TExpandableContainer::TExpandableContainer(const TExpandableContainer &a) :
  TOpenContainer(a)
{
}

TExpandableContainer & TExpandableContainer::operator=(const TExpandableContainer &a)
{
  if (this == &a) return *this;
  TOpenContainer::operator=(a);
  return *this;
}

TThing & TExpandableContainer::operator+= (TThing &t)
{
  TOpenContainer::operator+=(t);


  return *this;
}




TExpandableContainer::~TExpandableContainer()
{
}

void TExpandableContainer::assignFourValues(int x1, int x2, int x3, int x4)
{
  TOpenContainer::assignFourValues(x1, x2, x3, x4);
}

void TExpandableContainer::getFourValues(int *x1, int *x2, int *x3, int *x4) const
{
  TOpenContainer::getFourValues(x1, x2, x3, x4);
}

sstring TExpandableContainer::statObjInfo() const
{
  return TOpenContainer::statObjInfo();
}

int TExpandableContainer::getTotalVolume() const
{
  int num = getVolume();

  if(!isContainerFlag(CONT_WEIGHTLESS))
    num += getCarriedVolume();
  return num;
}

void TExpandableContainer::addToCarriedVolume(int num)
{
  // make this recursive...
  TOpenContainer::addToCarriedVolume(num);

  // increase my parent's volume if *I* am able to expand
  if (parent)
    parent->addToCarriedVolume(num);

  // not sure why this here, tables???
  // it was here before the switch to TExpand, shrug
  if (riding)
    riding->addToCarriedVolume(num);
}


