#include "comm.h"
#include "colorstring.h"
#include "obj_tool.h"
#include "being.h"
#include "room.h"

const unsigned int GRAFFITI_MAX = 50;
const int GRAFFITI_OBJ = 33315;

const int ncolors = 6;

// this proc is meant for tools only, designed with chalk in mind
int graffitiMaker(TBeing* ch, cmdTypeT cmd, const char* arg, TObj* o, TObj*) {
  sstring colors[ncolors] = {"white", "green", "blue", "red", "yellow",
    "purple"};
  sstring ccodes[ncolors] = {"<W>", "<g>", "<b>", "<r>", "<Y>", "<p>"};
  int color = 0;

  // we really should do this on created, but for some reason that isn't
  // working.  it seems as though the item hasn't been strung yet at that
  // point. using quick pulse should work fine.
  if (cmd == CMD_GENERIC_QUICK_PULSE && !o->isObjStat(ITEM_STRUNG)) {
    sstring buf;
    int c = ::number(0, ncolors - 1);
    o->swapToStrung();

    buf = o->name;
    o->name = format("%s %s") % buf % colors[c];

    buf = o->shortDescr;
    o->shortDescr = format("%s%s<1>") % ccodes[c] % buf;

    buf = o->descr;
    o->descr = format("%s%s<1>") % ccodes[c] % buf;
    return FALSE;
  }

  if (cmd != CMD_WRITE)
    return FALSE;

  sstring buf;
  //  buf = sstring(arg).word(0);
  buf = sstring(stripColorCodes(arg));

  if (!o || !ch)
    return FALSE;

  TTool* tool = dynamic_cast<TTool*>(o);

  if (!tool)
    return FALSE;

  TBeing* cho = dynamic_cast<TBeing*>(o->equippedBy);

  if (!cho) {
    return FALSE;
  }

  //  buf = buf.upper();

  //  if (!buf.isWord() || buf.length() > GRAFFITI_MAX) {
  if (buf.length() > GRAFFITI_MAX) {
    ch->sendTo("You can't write that - try something shorter.\n\r");
    return TRUE;
  }

  TObj* gfti = read_object(GRAFFITI_OBJ, VIRTUAL);

  if (!gfti) {
    ch->sendTo("Problem making graffiti object, bug a coder.\n\r");
    vlogf(LOG_BUG,
      format("Couldn't load object (%d) in graffitiMaker.") % GRAFFITI_OBJ);
    return TRUE;
  }

  for (int i = 0; i < ncolors; ++i) {
    if (sstring(o->name).find(colors[i], 0) != sstring::npos) {
      color = i;
      break;
    }
  }

  sstring newName = format("%s message [graffiti] [%s]") % buf % ch->name;
  sstring newShort = format("the message '%s%s<z>'") % ccodes[color] % buf;
  sstring newLong =
    format("Some vandal has left a message: '%s%s<z>'.") % ccodes[color] % buf;
  gfti->swapToStrung();
  gfti->name = newName;
  gfti->shortDescr = newShort;
  gfti->setDescr(newLong);

  act("$n scrawls some graffiti with $s $p.", TRUE, ch, o, NULL, TO_ROOM);
  act("You make your mark.", TRUE, ch, o, NULL, TO_CHAR);

  // this clunky block of code moves the message directly beneath
  // any existing message
  /*
  TThing *last=NULL, *first=NULL;
  TObj *obj;
  for(StuffIter it=ch->roomp->stuff.begin();it!=ch->roomp->stuff.end();++it){
    if((obj=dynamic_cast<TObj *>(*it)) && obj->objVnum()==GRAFFITI_OBJ) {
      last=*it;
      first=ch->roomp->stuff.front();
    }
  }

  *ch->roomp += *gfti;
  */ // To me it looks like this clunky block of code does nothing at all.
  //////

  tool->addToToolUses(-1);
  if (tool->getToolUses() <= 0) {
    act("Your $p is all used up.", FALSE, ch, o, NULL, TO_CHAR);
    act("$n uses up the last of $s $p.", FALSE, ch, o, NULL, TO_ROOM);
    if (!tool->makeScraps()) {
      delete tool;
      tool = NULL;
    }
  }
  return TRUE;
}
