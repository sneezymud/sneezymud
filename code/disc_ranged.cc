//////////////////////////////////////////////////////////////////////////
//
// SneezyMUD - All rights reserved, SneezyMUD Coding Team
//
// $Log: disc_ranged.cc,v $
// Revision 5.2  2001/09/07 07:07:34  peel
// changed TThing->stuff to getStuff() and setStuff()
//
// Revision 5.1.1.1  1999/10/16 04:32:20  batopr
// new branch
//
// Revision 5.1  1999/10/16 04:31:17  batopr
// new branch
//
// Revision 1.1  1999/09/12 17:24:04  sneezy
// Initial revision
//
//
//////////////////////////////////////////////////////////////////////////


#include "stdsneezy.h"
#include "disc_ranged.h"

void TThing::stringMeBow(TBeing *ch, TThing *)
{
  act("$p isn't a bow.", FALSE, ch, this, 0, TO_CHAR);
  return;
}

void TThing::stringMeString(TBeing *ch, TBow *)
{
  act("$p isn't bowstring.", FALSE, ch, this, 0, TO_CHAR);
  return;
}

void TTool::stringMeString(TBeing *ch, TBow *bow)
{
  if (getToolType() != TOOL_BOWSTRING) {
    act("$p isn't bowstring.", FALSE, ch, this, 0, TO_CHAR);
    return;
  }
  if (!bow->isBowFlag(BOW_STRING_BROKE)) {
    act("$p doesn't need any restringing.", FALSE, ch, bow, 0, TO_CHAR);
    return;
  }

  bow->remBowFlags(BOW_STRING_BROKE);
  act("You restring $p with $P.", FALSE, ch, bow, this, TO_CHAR);
  act("$n restrings $s $o with $P.", FALSE, ch, bow, this, TO_ROOM);

  addToToolUses(-1);
  if (getToolUses() <= 0) {
    act("$p is all used up.", FALSE, ch, this, 0, TO_CHAR);
    delete this;
    return;
  }
}

void TBeing::doRestring(const char *argument)
{
  TThing *bow = NULL;
  TThing *bstr = NULL;
  char arg1[256], arg2[256];

  if (sscanf(argument, "%s %s", arg1, arg2) != 2) {
    sendTo("Syntax : restring <bow> <string>\n\r");
    return;
  }
#if 1
  TThing *t;
  for (t = getStuff(); t && !(bow && bstr); t = t->nextThing) {
    if (!bow) {
      bow = dynamic_cast<TBow *>(t);
      if (bow && !isname(arg1, bow->name))
        bow = NULL;
    }
    if (!bstr) {
      bstr = dynamic_cast<TTool *>(t);
      if (bstr && !isname(arg2, bstr->name))
        bstr = NULL;
    }
  }
  if (!bow) {
    sendTo("You don't seem to have '%s' in your inventory.\n\r", arg1);
    sendTo("Syntax : restring <bow> <string>\n\r");
    return;
  }
  if (!bstr) {
    sendTo("You don't seem to have '%s' in your inventory.\n\r", arg2);
    sendTo("Syntax : restring <bow> <string>\n\r");
    return;
  }
#else
  // works, but gets confused since "bow" is an abbrev for "bowstring"
  if (!(bow = searchLinkedListVis(this, arg1, getStuff()))) {
    sendTo("You don't seem to have '%s' in your inventory.\n\r", arg1);
    sendTo("Syntax : restring <bow> <string>\n\r");
    return;
  }
  if (!(bstr = searchLinkedListVis(this, arg2, getStuff()))) {
    sendTo("You don't seem to have '%s' in your inventory.\n\r", arg2);
    sendTo("Syntax : restring <bow> <string>\n\r");
    return;
  }
#endif
  bow->stringMeBow(this, bstr);
  // string may be invalid here
  return;
}
