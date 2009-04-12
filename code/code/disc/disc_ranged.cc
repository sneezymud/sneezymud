//////////////////////////////////////////////////////////////////////////
//
// SneezyMUD - All rights reserved, SneezyMUD Coding Team
//
//////////////////////////////////////////////////////////////////////////


#include "being.h"
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

void TBeing::doRestring(const sstring &argument)
{
  TThing *bow = NULL;
  TThing *bstr = NULL;
  char arg1[256], arg2[256];

  if (sscanf(argument.c_str(), "%s %s", arg1, arg2) != 2) {
    sendTo("Syntax : restring <bow> <string>\n\r");
    return;
  }
#if 1
  TThing *t=NULL;
  for(StuffIter it=stuff.begin();it!=stuff.end() && (t=*it);++it) {
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
    sendTo(format("You don't seem to have '%s' in your inventory.\n\r") % arg1);
    sendTo("Syntax : restring <bow> <string>\n\r");
    return;
  }
  if (!bstr) {
    sendTo(format("You don't seem to have '%s' in your inventory.\n\r") % arg2);
    sendTo("Syntax : restring <bow> <string>\n\r");
    return;
  }
#else
  // works, but gets confused since "bow" is an abbrev for "bowsstring"
  if (!(bow = searchLinkedListVis(this, arg1, stuff))) {
    sendTo(format("You don't seem to have '%s' in your inventory.\n\r") % arg1);
    sendTo("Syntax : restring <bow> <string>\n\r");
    return;
  }
  if (!(bstr = searchLinkedListVis(this, arg2, stuff))) {
    sendTo(format("You don't seem to have '%s' in your inventory.\n\r") % arg2);
    sendTo("Syntax : restring <bow> <string>\n\r");
    return;
  }
#endif
  bow->sstringMeBow(this, bstr);
  // sstring may be invalid here
  return;
}
