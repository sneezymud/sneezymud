// other_obj.cc

#include "obj_other_obj.h"
#include "extern.h"

TOtherObj::TOtherObj() :
  TObj()
{
}

TOtherObj::TOtherObj(const TOtherObj &a) :
  TObj(a)
{
}

TOtherObj & TOtherObj::operator=(const TOtherObj &a)
{
  if (this == &a) return *this;
  TObj::operator=(a);
  return *this;
}

TOtherObj::~TOtherObj()
{
}

void TOtherObj::assignFourValues(int, int, int, int)
{
}

void TOtherObj::getFourValues(int *x1, int *x2, int *x3, int *x4) const
{
  *x1 = 0;
  *x2 = 0;
  *x3 = 0;
  *x4 = 0;
}

sstring TOtherObj::statObjInfo() const
{
  sstring a("");
  return a;
}

void TOtherObj::lowCheck()
{
  // Unlike other lowCheck functions, we do NOT recurse to TObj:: base
  // function.  We do however do modified versions of what that func does

  if (!getVolume() && canWear(ITEM_TAKE))
    vlogf(LOG_LOW,format("other item (%s:%d) had 0 volume.") % getName() % objVnum());

  // simulated light sources are allowed under following constraints:
  // non-takeable : can have as much light as want (lampposts, etc)
  // takeable : max_exist < 10 : weight = light
  // takeable : max_exist>= 10 : weight = 2*light
  if (canWear(ITEM_TAKE)) {
    int i;
    int lgt = 0;
    for (i=0; i<MAX_OBJ_AFFECT;i++) {
      if (affected[i].location == APPLY_LIGHT) 
        lgt += affected[i].modifier;
    }
    if ((max_exist < 10 && 
        ((int) getWeight() < lgt)) ||
        (max_exist >= 10 &&
        ((int) getWeight() < 2*lgt))) {
      vlogf(LOG_LOW,format("other item (%s:%d) had bad light-to-weight ratio.") % getName() % objVnum());
      setWeight(2*lgt);
    }
  }
}

void TOtherObj::writeAffects(int i, FILE *fp) const
{
  if (affected[i].location != APPLY_NONE) {
    fprintf(fp, "A\n%d %ld %ld\n", 
            mapApplyToFile(affected[i].location),
            applyTypeShouldBeSpellnum(affected[i].location) ? mapSpellnumToFile(spellNumT(affected[i].modifier)) : affected[i].modifier,
            affected[i].modifier2);
  }
}

int TOtherObj::addApply(TBeing *, applyTypeT)
{
  return FALSE;
}

