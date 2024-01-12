#include "being.h"
#include "comm.h"
#include "defs.h"
#include "enum.h"
#include "limbs.h"
#include "obj.h"
#include "parse.h"
#include "sstring.h"
#include "thing.h"

int graffitiObject(TBeing* ch, cmdTypeT cmd, const char* arg, TObj* o, TObj*) {
  if (cmd != CMD_PEE)
    return false;

  if (!o || !ch)
    return false;

  sstring buf = sstring(arg).word(0);

  if (!isname(buf, o->name))
    return false;

  if (ch->getSex() != SEX_MALE &&
      (ch->equipment[WEAR_WAIST] || ch->equipment[WEAR_LEG_R] ||
        ch->equipment[WEAR_LEG_L])) {
    ch->sendTo(
      "You are not skilled enough to do this without removing the equipment "
      "that lies between you and your target.\n\r");
    return true;
  }

  act("$n directs a stream of urine at $p, dissolving it completely.", true, ch,
    o, nullptr, TO_ROOM, nullptr);
  act("You direct a stream of urine at $p, dissolving it completely.", true, ch,
    o, nullptr, TO_CHAR, nullptr);

  //  delete o;
  //  o = nullptr;
  return DELETE_THIS;
}
