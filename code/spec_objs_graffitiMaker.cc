#include "stdsneezy.h"
#include "obj_tool.h"

const unsigned int GRAFFITI_MAX = 50;
const int GRAFFITI_OBJ = 33315;

// this proc is meant for tools only, designed with chalk in mind
int graffitiMaker(TBeing *ch, cmdTypeT cmd, const char *arg, TObj *o, TObj *)
{
  if (cmd != CMD_WRITE)
    return FALSE;

  sstring buf;
//  buf = sstring(arg).word(0);
  buf = sstring(stripColorCodes(arg));
    
  if (!o || !ch)
    return FALSE;

  TTool *tool = dynamic_cast<TTool *>(o);

  if (!tool)
    return FALSE;

  TBeing *cho = dynamic_cast<TBeing *>(o->equippedBy);
  
  if (!cho) {
    return FALSE;
  }

//  buf = buf.upper();

//  if (!buf.isWord() || buf.length() > GRAFFITI_MAX) {
  if (buf.length() > GRAFFITI_MAX) {
    ch->sendTo("You can't write that - try something shorter.\n\r");
    return TRUE;
  } 
    
  TObj * gfti = read_object(GRAFFITI_OBJ, VIRTUAL);

  if (!gfti) {
    ch->sendTo("Problem making graffiti object, bug a coder.\n\r");
    vlogf(LOG_BUG, fmt("Couldn't load object (%d) in graffitiMaker.") 
        % GRAFFITI_OBJ);
    return TRUE;
  }

  sstring newName = fmt("%s message [graffiti] [%s]") % buf % ch->name;
  sstring newShort = fmt("the message '<W>%s<z>'") % buf;
  sstring newLong = fmt("Some vandal has left a message: '<W>%s<z>'.") % buf;
  gfti->swapToStrung();
  gfti->name = mud_str_dup(newName);
  gfti->shortDescr = mud_str_dup(newShort);
  gfti->setDescr(mud_str_dup(newLong));
  
  act("$n scrawls some graffiti with $s $p.", TRUE, ch, o, NULL, TO_ROOM);
  act("You make your mark.", TRUE, ch, o, NULL, TO_CHAR);
  *ch->roomp += *gfti;
  tool->addToToolUses(-1);
  if (tool->getToolUses() <= 0) {
    act("Your $p is all used up.", FALSE, ch, o, NULL, TO_CHAR);
    act("$n uses up the last of $s $p.", FALSE, ch, 0, NULL, TO_ROOM);
    tool->makeScraps();
    delete tool;
  }
  return TRUE;
}
