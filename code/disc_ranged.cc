//////////////////////////////////////////////////////////////////////////
//
// SneezyMUD - All rights reserved, SneezyMUD Coding Team
//
// $Log: disc_ranged.cc,v $
// Revision 5.7  2003/03/19 19:01:51  peel
// fixed restring
// I broke it with the string->sstrng conversion
//
// Revision 5.6  2003/03/13 22:40:53  peel
// added sstring class, same as string but takes NULL as an empty string
// replaced all uses of string to sstring
//
// Revision 5.5  2003/01/28 19:30:15  peel
// converted a few more things from char * to sstring
//
// Revision 5.4  2002/01/10 00:45:47  peel
// more splitting up of obj2.h
//
// Revision 5.3  2002/01/09 23:27:03  peel
// More splitting up of obj2.h
// renamed food.cc to obj_food.cc
// renamed organic.cc to obj_organic.cc
//
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
#include "obj_bow.h"
#include "obj_tool.h"

void TThing::sstringMeBow(TBeing *ch, TThing *)
{
  act("$p isn't a bow.", FALSE, ch, this, 0, TO_CHAR);
  return;
}

void TThing::sstringMeString(TBeing *ch, TBow *)
{
  act("$p isn't bowsstring.", FALSE, ch, this, 0, TO_CHAR);
  return;
}

void TTool::sstringMeString(TBeing *ch, TBow *bow)
{
  if (getToolType() != TOOL_BOWSTRING) {
    act("$p isn't bowsstring.", FALSE, ch, this, 0, TO_CHAR);
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

void TBeing::doRestring(sstring argument)
{
  TThing *bow = NULL;
  TThing *bstr = NULL;
  char arg1[256], arg2[256];

  if (sscanf(argument.c_str(), "%s %s", arg1, arg2) != 2) {
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
  // works, but gets confused since "bow" is an abbrev for "bowsstring"
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
  bow->sstringMeBow(this, bstr);
  // sstring may be invalid here
  return;
}
