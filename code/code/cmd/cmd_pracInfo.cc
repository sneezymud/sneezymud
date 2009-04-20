#include "handler.h"
#include "being.h"


void TBeing::doPracInfo(sstring arg) {
  TBeing *ch;
  sstring buf;
  int count = 1;

  if (!(ch = get_char_room(arg, in_room))) {
    if (!(ch = get_pc_world(this, arg, EXACT_NO))) {
      if (!(ch = get_char_vis_world(this, arg, &count, EXACT_YES))) {
        if (!(ch = get_char_vis_world(this, arg, &count, EXACT_NO))) {
          sendTo("Syntax: pracinfo <char name>\n\r");
          sendTo("(The character must be online.)\n\r");
          return;
        }
      }
    }
  }
 
  if (!isImmortal() || !desc)
    return;
  if (powerCheck(POWER_ACCESS) || powerCheck(POWER_SET))
    return;
  short expected = ch->expectedPracs();
  short actual = ch->meanPracsSoFar();
  buf = format("Practice info for %s:\n\r") % ch->getName();
  buf += format("\tExpected pracs for level: %d \n\r")
    % expected;
  buf += format("\tActual pracs for level:   %d (approx)\n\r")
    % actual;
  buf += format("\tDiscrepancy:              %d\n\r")
    % (actual - expected);
  sendTo(buf);
  
}
