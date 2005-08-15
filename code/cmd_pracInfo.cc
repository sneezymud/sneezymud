#include "stdsneezy.h"


void TBeing::doPracInfo(sstring arg) {
  TBeing *ch;
  sstring buf;
  int i, count = 1;

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
  sh_int expected = ch->expectedPracs();
  int actual = 0;
  for (i=0;i<10;i++)
    actual += ch->pracsSoFar();
  actual /= 10; // trying to get a better single number here
  buf = fmt("Practice info for %s:\n\r") % ch->getName();
  buf += fmt("\tExpected pracs for level: %d \n\r")
    % expected;
  buf += fmt("\tActual pracs for level:   %d (approx)\n\r")
    % actual;
  buf += fmt("\tDiscrepancy:              %d\n\r")
    % (actual - expected);
  sendTo(buf);
  
}
