//////////////////////////////////////////////////////////////////////////
//
// SneezyMUD - All rights reserved, SneezyMUD Coding Team
//
//////////////////////////////////////////////////////////////////////////

#include <boost/format.hpp>
#include <string.h>
#include <memory>

#include "ansi.h"
#include "being.h"
#include "comm.h"
#include "log.h"
#include "obj.h"
#include "spell2.h"
#include "spells.h"
#include "sstring.h"
#include "thing.h"

#if 0

doCast -> doDiscipline -> flamingSword -> start_cast

--
doStore:
go through normal cast, stop at spelltask.cc:1439 and check if we are storing
a spell, if so, store it and return

doTrigger:
go through cast checks etc.  once number is parsed, search for that spell in
the stored list, if its there, move to spelltask and call start_cast
--

need to make it a list so we can store more than one somehow

spellTaskData

class spellStoreData {
public:
  spellTaskData *spelltask;
  bool storing;
};

#endif

int TBeing::doTrigger(const char* argument) {
  char arg[256];
  TBeing* ch = nullptr;
  TObj* o = nullptr;
  TThing* t = nullptr;
  int rc;

  if (!preCastCheck())
    return false;

  strcpy(arg, argument);

  auto [which, target] = parseSpellNum(argument);
  if (which == TYPE_UNDEFINED)
    return false;

  if (!discArray[which]) {
    vlogf(LOG_BUG, format("doTrigger called with null discArray[] (%d) (%s)") %
                     which % getName());
    return false;
  }

  if (which <= TYPE_UNDEFINED || !preDiscCheck(which) ||
      !parseTarget(which, arg, &t))
    return false;

  ch = dynamic_cast<TBeing*>(t);
  o = dynamic_cast<TObj*>(t);

  spelltask = spellstore.spelltask;

  rc =
    doSpellCast(this, ch, o, roomp, which, getSpellType(discArray[which]->typ));
  spellstore.spelltask = nullptr;
  spellstore.storing = false;

  return rc;
}

int TBeing::doStore(const char* argument) {
  int rc;

  if (spellstore.storing || spelltask) {
    sendTo("You are already casting a spell.\n\r");
    return false;
  }
  if (spellstore.spelltask) {
    sendTo("You already have a spell stored.\n\r");
    return false;
  }

  spellstore.storing = true;
  rc = doCast(argument);

  if (rc == false) {
    act("Your spell has not been stored.", true, this, nullptr, nullptr, TO_CHAR,
      ANSI_RED);
    spellstore.storing = false;
    delete spellstore.spelltask;
  }

  return rc;
}
