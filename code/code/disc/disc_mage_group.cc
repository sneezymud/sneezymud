#include "disc_mage_group.h"

#include "extern.h"
#include "room.h"
#include "low.h"
#include "monster.h"

bool forEachGroupBuffTarget(TBeing* caster,
  std::function<bool(TBeing*)> applyFn,
  std::function<bool(TBeing*)> skipFn) {
  bool anyBuffed = false;
  for (StuffIter it = caster->roomp->stuff.begin();
    it != caster->roomp->stuff.end();) {
    TThing* t = *(it++);
    auto* target = dynamic_cast<TBeing*>(t);
    if (!target)
      continue;
    if (!caster->inGroup(*target))
      continue;
    if (skipFn(target))
      continue;
    if (applyFn(target))
      anyBuffed = true;
  }
  return anyBuffed;
}
