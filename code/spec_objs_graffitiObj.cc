#include "stdsneezy.h"

int graffitiObject(TBeing *ch, cmdTypeT cmd, const char *arg, TObj *o, TObj *)
{
  if (cmd != CMD_PEE)
    return FALSE;

  if (!o || !ch)
    return FALSE;
  
  sstring buf = sstring(arg).word(0);

  if (!isname(buf, o->name))
    return FALSE;

  if (ch->getSex() != SEX_MALE && (ch->equipment[WEAR_WAISTE]
        || ch->equipment[WEAR_LEGS_R] || ch->equipment[WEAR_LEGS_L])) {
      ch->sendTo("You are not skilled enough to do this without removing the equipment that lies between you and your target.\n\r");
    return TRUE;
  }
  
  act("$n directs a stream of urine at $p, dissolving it completely.", TRUE,
    ch, o, NULL, TO_ROOM, NULL);
  act("You direct a stream of urine at $p, dissolving it completely.", TRUE,
    ch, o, NULL, TO_CHAR, NULL);

  delete o;
  o = NULL;
  return TRUE;
}
