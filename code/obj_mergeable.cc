#include "stdsneezy.h"
#include "obj_mergeable.h"


bool TMergeable::willMerge(TMergeable *t)
{
  vlogf(LOG_PEEL, fmt("TMergeable::willMerge: %s %s") %
	getName() % t->getName());
  return false;
}

void TMergeable::doMerge(TMergeable *t)
{
  vlogf(LOG_PEEL, fmt("TMergeable::doMerge: %s %s") %
	getName() % t->getName());
  return;
}


TMergeable::TMergeable()
{
}

TMergeable::TMergeable(const TMergeable &a)
{
}

TMergeable & TMergeable::operator=(const TMergeable &a)
{
  if (this == &a) return *this;
  TObj::operator=(a);
  return *this;
}

TMergeable::~TMergeable()
{
}
