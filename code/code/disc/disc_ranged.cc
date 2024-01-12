//////////////////////////////////////////////////////////////////////////
//
// SneezyMUD - All rights reserved, SneezyMUD Coding Team
//
//////////////////////////////////////////////////////////////////////////

#include <boost/format.hpp>
#include <cstdio>
#include <list>
#include <memory>

#include "being.h"
#include "comm.h"
#include "disc_ranged.h"
#include "obj.h"
#include "obj_bow.h"
#include "obj_tool.h"
#include "sstring.h"
#include "thing.h"

void TThing::sstringMeBow(TBeing* ch, TThing*) {
  act("$p isn't a bow.", false, ch, this, 0, TO_CHAR);
}

void TThing::sstringMeString(TBeing* ch, TBow*) {
  act("$p isn't bowsstring.", false, ch, this, 0, TO_CHAR);
}

void TTool::sstringMeString(TBeing* ch, TBow* bow) {
  if (getToolType() != TOOL_BOWSTRING) {
    act("$p isn't bowsstring.", false, ch, this, 0, TO_CHAR);
    return;
  }
  if (!bow->isBowFlag(BOW_STRING_BROKE)) {
    act("$p doesn't need any restringing.", false, ch, bow, 0, TO_CHAR);
    return;
  }

  bow->remBowFlags(BOW_STRING_BROKE);
  act("You restring $p with $P.", false, ch, bow, this, TO_CHAR);
  act("$n restrings $s $o with $P.", false, ch, bow, this, TO_ROOM);

  addToToolUses(-1);
  if (getToolUses() <= 0) {
    act("$p is all used up.", false, ch, this, 0, TO_CHAR);
    delete this;
    return;
  }
}

void TBeing::doRestring(const sstring& argument) {
  TThing* bow = nullptr;
  TThing* bstr = nullptr;
  char arg1[256], arg2[256];

  if (sscanf(argument.c_str(), "%s %s", arg1, arg2) != 2) {
    sendTo("Syntax : restring <bow> <string>\n\r");
    return;
  }
#if 1
  TThing* t = nullptr;
  for (StuffIter it = stuff.begin(); it != stuff.end() && (t = *it); ++it) {
    if (!bow) {
      bow = dynamic_cast<TBow*>(t);
      if (bow && !isname(arg1, bow->name))
        bow = nullptr;
    }
    if (!bstr) {
      bstr = dynamic_cast<TTool*>(t);
      if (bstr && !isname(arg2, bstr->name))
        bstr = nullptr;
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
}
