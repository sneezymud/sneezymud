//////////////////////////////////////////////////////////////////////////
//
// SneezyMUD - All rights reserved, SneezyMUD Coding Team
//
//////////////////////////////////////////////////////////////////////////

#include <algorithm>
#include <climits>
#include <map>

#include "comm.h"
#include "handler.h"
#include "being.h"
#include "low.h"
#include "trap.h"
#include "obj_trapcomp_bag.h"
#include "obj_trap_component.h"

TTrapCompBag::TTrapCompBag() : TExpandableContainer() {}

TTrapCompBag::TTrapCompBag(const TTrapCompBag& a) : TExpandableContainer(a) {}

TTrapCompBag& TTrapCompBag::operator=(const TTrapCompBag& a) {
  if (this == &a)
    return *this;
  TExpandableContainer::operator=(a);
  return *this;
}

TTrapCompBag::~TTrapCompBag() {}

void TTrapCompBag::assignFourValues(int x1, int x2, int x3, int x4) {
  TExpandableContainer::assignFourValues(x1, x2, x3, x4);
}

void TTrapCompBag::getFourValues(int* x1, int* x2, int* x3, int* x4) const {
  TExpandableContainer::getFourValues(x1, x2, x3, x4);
}

sstring TTrapCompBag::statObjInfo() const {
  sstring a = TExpandableContainer::statObjInfo();
  a += "This is a trap component bag.\n\r";
  return a;
}

int TTrapCompBag::putSomethingInto(TBeing* ch, TThing* obj) {
  if (!dynamic_cast<TTrapComponent*>(obj)) {
    act("$p can only hold trap components.", false, ch, this, obj, TO_CHAR);
    return 2;  // stop trying to put (halts "put all")
  }
  return TExpandableContainer::putSomethingInto(ch, obj);
}

bool TTrapCompBag::lowCheckSlots(silentTypeT silent) {
  // Trap bags may be (take), and worn at hold/waist/body/legs; permit throw.
  unsigned int value = obj_flags.wear_flags;
  REMOVE_BIT(value, ITEM_WEAR_THROW);
  REMOVE_BIT(value, ITEM_WEAR_TAKE);
  REMOVE_BIT(value, ITEM_WEAR_HOLD);
  REMOVE_BIT(value, ITEM_WEAR_WAIST);
  REMOVE_BIT(value, ITEM_WEAR_BODY);
  REMOVE_BIT(value, ITEM_WEAR_LEGS);

  if (value != 0) {
    if (!silent)
      vlogf(LOG_LOW, format("trap component bag (%s) with bad wear slots: %d") %
                       getName() % value);
    return true;
  }
  return false;
}

TThing* TTrapCompBag::findComponent(const TBeing* ch, const sstring& name) {
  return searchLinkedListVis(ch, name, stuff);
}

void TTrapCompBag::evaluateMe(TBeing* ch) const {
  if (ch->getSkillValue(SKILL_EVALUATE) <= 0) {
    ch->sendTo("You don't know enough about evaluation to analyze this.\n\r");
    return;
  }
  ch->learnFromDoingUnusual(LEARN_UNUSUAL_NORM_LEARN, SKILL_EVALUATE, 7);

  if (isContainerFlag(CONT_CLOSED)) {
    ch->sendTo("You need to open the bag to analyze its contents.\n\r");
    return;
  }

  // Tally charges available per component vnum.
  std::map<int, int> counts;
  for (StuffIter it = stuff.begin(); it != stuff.end(); ++it) {
    if (auto* comp = dynamic_cast<TTrapComponent*>(*it))
      if (comp->getTrapComponentCharges() > 0)
        counts[comp->objVnum()] += comp->getTrapComponentCharges();
  }
  if (counts.empty()) {
    ch->sendTo("The bag holds no usable trap components.\n\r");
    return;
  }

  // How many traps of `kw` you could build for `targ`, gated by the scarcest
  // reagent. Recipes come from the authoritative trapComponents() table; the
  // parseTrapType() check keeps us to legal type/target combos and avoids
  // trapComponents()'s set-path warning logging. `casing` is an extra reagent
  // required for portable (mine/grenade) traps, 0 when none is needed.
  auto buildable = [&](const char* kw, trap_targ_t targ, int casing) -> int {
    if (parseTrapType(kw, targ) == MAX_TRAP_TYPES)
      return 0;
    int i1 = 0, i2 = 0, i3 = 0;
    if (!trapComponents(kw, targ, i1, i2, i3))
      return 0;
    int fewest = INT_MAX;
    for (int vnum : {i1, i2, i3, casing}) {
      if (!vnum)
        continue;
      auto found = counts.find(vnum);
      int have = (found != counts.end()) ? found->second : 0;
      if (have == 0)
        return 0;
      fewest = std::min(fewest, have);
    }
    return (fewest == INT_MAX) ? 0 : fewest;
  };

  static const char* const TYPES[] = {"fire", "explosive", "poison", "sleep",
    "acid", "spore", "frost", "teleport", "power", "spike", "blade", "hammer",
    "pebble", "bolt", "disk"};

  // Organize by trap source: each heading lists only the damage types that
  // source can actually take (parseTrapType inside buildable() gates this, so
  // e.g. hammer shows only under Door, pebble only under Container/Arrow).
  struct Source {
    trap_targ_t targ;
    const char* label;
    int casing;  // extra reagent for portable traps; 0 when none is needed
  };
  static const Source SOURCES[] = {
    {TRAP_TARG_DOOR, "Door", 0},
    {TRAP_TARG_CONT, "Container", 0},
    {TRAP_TARG_ARROW, "Arrow", 0},
    {TRAP_TARG_MINE, "Mine", Obj::ST_CASE_MINE},
    {TRAP_TARG_GRENADE, "Grenade", Obj::ST_CASE_GRENADE},
  };

  sstring out;
  for (const Source& src : SOURCES) {
    sstring block;
    for (const char* kw : TYPES) {
      if (int n = buildable(kw, src.targ, src.casing))
        block += format("  %s: <Y>%d<1>\n\r") % sstring(kw).cap() % n;
    }
    if (!block.empty()) {
      out += format("<c>%s:<1>\n\r") % src.label;
      out += block;
    }
  }

  if (out.empty()) {
    ch->sendTo("You can't assemble a complete trap from these components.\n\r");
    return;
  }

  ch->sendTo("Trap-making analysis:\n\r");
  ch->sendTo(out);
}
