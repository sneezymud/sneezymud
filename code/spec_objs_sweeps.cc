#include "stdsneezy.h"
#include "spec_objs_sweeps.h"


int sweepsScratch(TBeing *ch, cmdTypeT cmd, const char *arg, TObj *o, TObj *) 
{
  
  if (cmd != CMD_SHAKE)
    return FALSE;
  
  sstring buf;
  buf=sstring(arg).word(0);

  if (!isname(buf, o->name) || !is_abbrev(buf, "tile"))
    return FALSE;

  if (!o || !ch)
    return FALSE;

  TBeing *cho;
  cho = ch;
  if (!(ch = dynamic_cast<TBeing *>(o->equippedBy)))
  {
    cho->sendTo("Maybe you should hold the tile in your hand.\n\r");
    return TRUE;
  }

  TObj *tile = NULL;

  if (!(tile = dynamic_cast<TObj *>(ch->equipment[HOLD_RIGHT])) &&
    !(tile = dynamic_cast<TObj *>(ch->equipment[HOLD_LEFT]))) 
  {
    act("You must hold the tile before trying to shake it.",TRUE,ch,NULL,NULL,TO_CHAR,NULL);
    return TRUE;
  }
  
  act("$n shakes a tile vigorously.",TRUE,ch,NULL,NULL,TO_ROOM,NULL);
  act("You shake the tile vigorously.",TRUE,ch,NULL,NULL,TO_CHAR,NULL);

  int roll = ::number(1,100);
  sstring buf3, buf4;

  if (roll == 1)
    buf3 = "C";
  else if (roll == 2)
    buf3 = "L";
  else if (roll == 3)
    buf3 = "Y";
  else if (roll > 3 && roll <= 7)
    buf3 = "M";
  else if (roll > 7 && roll <= 36)
    buf3 = "O";
  else if (roll > 36)
    buf3 = "P";
  
  ch->sendTo("As you watch, the blank face of the tile blurs momentarily.\n\r");
  buf4 = fmt("The letter <Y>%s<z> appears.") % buf3;
  sstring buf5 = fmt("tile %s") % buf3;
  sstring buf6 = fmt("an <b>obsidian tile<z> inscribed with <Y>%s<z>") % buf3;
  o->swapToStrung();
  o->name = mud_str_dup(buf5);
  o->shortDescr = mud_str_dup(buf6);
  o->spec = SPEC_SPLIT_JOIN;
  act(buf4,TRUE,ch,NULL,NULL,TO_CHAR,NULL);

  return TRUE;
}
  
int sweepsSplitJoin(TBeing *ch, cmdTypeT cmd, const char *arg, TObj *o, TObj *) {

  if (cmd != CMD_COMBINE && cmd != CMD_SPLIT)
    return FALSE;
  
  sstring buf, buf4, buf5, buf6;
  buf=sstring(arg).word(0);

  if (!is_abbrev(buf, "tiles"))
    return FALSE;

  if (!o || !ch)
    return FALSE;
  TBeing *cho;
  cho = ch;
  if (!(ch = dynamic_cast<TBeing *>(o->equippedBy)))
  {
    cho->sendTo("Maybe you should hold the tile in your hand.\n\r");
    return TRUE;
  }

  TObj *tile1 = NULL;
  TObj *tile2 = NULL;

  if (cmd == CMD_COMBINE) 
  {
    if (!(tile1 = dynamic_cast<TObj *>(ch->equipment[HOLD_RIGHT])) ||
      !(tile2 = dynamic_cast<TObj *>(ch->equipment[HOLD_LEFT])) ||
      !(tile1->spec == SPEC_SPLIT_JOIN) || 
      !(tile2->spec == SPEC_SPLIT_JOIN)) 
    {
      act("You must hold two tiles, on in each hand, in order to combine them.",TRUE,ch,NULL,NULL,TO_CHAR,NULL);
     return TRUE;
    }
 
    //return quietly if the proc is firing from both hands
    if (o == ch->equipment[HOLD_LEFT])
      return TRUE;

    act("$n fiddles with some tiles.",TRUE,ch,NULL,NULL,TO_ROOM,NULL);
    act("You fiddle with some tiles.",TRUE,ch,NULL,NULL,TO_CHAR,NULL);

    sstring name1 = sstring(tile1->name).word(1);
    sstring name2 = sstring(tile2->name).word(1);

    if (name1 == "" || name2 == "") {
      act("Nothing happens.",TRUE,ch,NULL,NULL,TO_ROOM,NULL);
      act("One of both of your tiles is faulty.  Nothing happens.",TRUE,ch,NULL,NULL,TO_CHAR,NULL);
      return TRUE;
    }

    act("<R>*SNAP*<z>\n\r$n lays one tile on top of another and they merge into one.",TRUE,ch,NULL,NULL,TO_ROOM,NULL);
    act("<R>*SNAP*<z>\n\rYou lay one tile on top of the other and they merge into one.",TRUE,ch,NULL,NULL,TO_CHAR,NULL);

    delete tile2;
  
    sstring newname = fmt("%s%s") % name1 % name2;
    ch->sendTo("As you watch, the face of the tile blurs momentarily.\n\r");
    buf4 = fmt("The letters <Y>%s<z> appear.") % newname;
    buf5 = fmt("tile %s") % newname;
    buf6 = fmt("an <b>obsidian tile<z> inscribed with <Y>%s<z>") % newname;
    tile1->swapToStrung();
    tile1->name = mud_str_dup(buf5);
    tile1->shortDescr = mud_str_dup(buf6);
    act(buf4, TRUE,ch,NULL,NULL,TO_CHAR,NULL);
  } else if (cmd == CMD_SPLIT) {
    TThing *left=NULL, *right=NULL;
    tile1 = dynamic_cast<TObj *>(ch->equipment[HOLD_RIGHT]);
    if (!tile1)
      tile1 = dynamic_cast<TObj *>(ch->equipment[HOLD_LEFT]);
    left = ch->equipment[HOLD_LEFT];
    right = ch->equipment[HOLD_RIGHT];
    if ( ( left && right ) || !tile1 )
    {
      act("You must be holding a tile, with the other hand empty, to split the tile.",TRUE,ch,NULL,NULL,TO_CHAR,NULL);
     return TRUE;
    }
    if (!(tile2 = read_object(TILEVNUM, VIRTUAL) ))
    {
      vlogf(LOG_BUG, fmt("spec_objs_sweeps.cc, problem loading object %d") % TILEVNUM);
      act("Nothing happens.",TRUE,ch,NULL,NULL,TO_ROOM,NULL);
      act("Nothing happens.",TRUE,ch,NULL,NULL,TO_CHAR,NULL);
      return TRUE;
    }
    
    act("$n fiddles with a tile.",TRUE,ch,NULL,NULL,TO_ROOM,NULL);
    act("You fiddle with a tile.",TRUE,ch,NULL,NULL,TO_CHAR,NULL);

    sstring name1 = sstring(tile1->name).word(1);
    sstring name2 = name1.substr(name1.length()-1, name1.length());
    name1 = name1.substr(0, (name1.length())-1);

    if (name1 == "" || name2 == "") {
      act("Nothing happens.",TRUE,ch,NULL,NULL,TO_ROOM,NULL);
      act("You can't split a tile with only one letter on it.",TRUE,ch,NULL,NULL,TO_CHAR,NULL);
      return TRUE;
    }

    act("<R>*SNAP*<z>\n\r$n tugs at a tile and it separates into two.",TRUE,ch,NULL,NULL,TO_ROOM,NULL);
    act("<R>*SNAP*<z>\n\rYou tug at a tile and it separates into two.",TRUE,ch,NULL,NULL,TO_CHAR,NULL);

    // deal with tile1
    buf5 = fmt("tile %s") % name1;
    buf6 = fmt("an <b>obsidian tile<z> inscribed with <Y>%s<z>") % name1;
    tile1->swapToStrung();
    tile1->name = mud_str_dup(buf5);
    tile1->shortDescr = mud_str_dup(buf6);

    // deal with tile2 - new object
    sstring buf5 = fmt("tile %s") % name2;
    sstring buf6 = fmt("an <b>obsidian tile<z> inscribed with <Y>%s<z>") % name2;
    tile2->swapToStrung();
    tile2->name = mud_str_dup(buf5);
    tile2->shortDescr = mud_str_dup(buf6);
    tile2->spec = SPEC_SPLIT_JOIN;
    
    if (!left)
    {
      ch->equipChar(tile2, HOLD_LEFT, SILENT_YES);
    } else if (!right) {
      ch->equipChar(tile2, HOLD_RIGHT, SILENT_YES);
    } else {
      vlogf(LOG_BUG, "Somehow got to end of splitting tiles in spec_objs_sweeps.cc with both hands full.");
    }
    
  }

  return TRUE;
}

