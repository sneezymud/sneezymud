#include "extern.h"
#include "room.h"
#include "low.h"
#include "obj_money.h"
#include "person.h"
#include "obj_table.h"
#include "obj_portal.h"

int objCastFaerieFire(TObj *o, TBeing *targ)
{
  affectedData aff;
  if (targ->affectedBySpell(SPELL_FAERIE_FIRE))
  {
    act("$p just tried to cast something on you!", 0, NULL, o, targ, TO_VICT);
    return FALSE;
  }
  aff.type = SPELL_FAERIE_FIRE;
  aff.level = 50;
  aff.location = APPLY_ARMOR;
  aff.bitvector = 0;
  aff.duration = 10 * UPDATES_PER_MUDHOUR; // 4x longer than normal
  aff.modifier = 100 + (aff.level*4);
  targ->affectJoin(NULL, &aff, AVG_DUR_NO, AVG_EFF_YES);
  act("A faint pink outline puffs out around $N!",
      TRUE, targ, NULL, NULL, TO_ROOM);
  act("A faint pink outline puffs out around you!",
      FALSE, targ, NULL, NULL, TO_CHAR);  
  return TRUE;  
}

int objCastSorcGlobe(TObj *o, TBeing *targ)
{
  affectedData aff;
  if (targ->affectedBySpell(SPELL_SORCERERS_GLOBE))
  {
    act("$n just tried to cast something on you!", 0, o, NULL, targ, TO_VICT);
    return FALSE;
  }
  aff.type = SPELL_SORCERERS_GLOBE;
  aff.level = 50;
  aff.location = APPLY_ARMOR;
  aff.bitvector = 0;
  aff.duration = (3 + (aff.level / 2)) * UPDATES_PER_MUDHOUR;
  aff.modifier = -100;
  targ->affectJoin(NULL, &aff, AVG_DUR_NO, AVG_EFF_YES);


  targ->roomp->playsound(SOUND_SPELL_SORCERERS_GLOBE, SOUND_TYPE_MAGIC);
  act("$n is instantly surrounded by a hardened wall of air!",\
      TRUE, targ, NULL, NULL, TO_ROOM);
  act("You are instantly surrounded by a hardened wall of air!", \
      FALSE, targ, NULL, NULL, TO_CHAR);

  return TRUE;  
}

int marukalia(TBeing *targ, cmdTypeT cmd, const char *arg, TObj *o, TObj *)
{
  if (!cmd || !targ || !arg || !o)
    return FALSE;
  if (cmd != CMD_SAY && cmd != CMD_SAY2)
    return FALSE;
  if (!(dynamic_cast<TPerson *>(targ)))
    return FALSE;
  TTable *table = dynamic_cast<TTable *>(o);
  if (!table)
    return FALSE;

  sstring sarg = arg;
  TThing *t;
  TMoney *mon;
  std::vector <sstring> expected_contents;
  std::vector <sstring> contents;
  int talens = 0;
  int dmg;
  int rc = 0;
  
  // check what's on the item
  // 
  for (t = table->rider; t; t = t->nextRider)
  {
    if ((mon = dynamic_cast<TMoney *>(t)))
      talens += mon->getMoney();
    else
      contents.push_back(t->getName()); // list of names of items (shortDescr)
  }
  
  if (sarg.lower() == "sunday gravy")
  {
    act("<b>A faint blue aura flickers around $p.<z>", \
      TRUE, targ, o, NULL, TO_ROOM);
    act("<b>A faint blue aura flickers around $p.<z>", \
      TRUE, targ, o, NULL, TO_CHAR);
    if (talens > 1000)
      objCastSorcGlobe(o, targ);
    else
      objCastFaerieFire(o, targ);
  } else {
    // do random -10 hp target / +10 moves target
    dmg = ::number(5,15);
    if (::number(0,1)) {
      act("<B>A tongue of blue flame strikes out from $p and lashes $n.<z>", \
        TRUE, targ, o, NULL, TO_ROOM);
      act("<B>OUCH!  A tongue of blue flame strikes out from $p, burning you!<z>", \
        TRUE, targ, o, NULL, TO_CHAR);
      rc = targ->reconcileDamage(targ, dmg, DAMAGE_FIRE);
    } else {
      act("<b>A blue aura flickers around $p and reaches out to touch $n.<z>", \
        TRUE, targ, o, NULL, TO_ROOM);
      act("<b>A blue aura flickers around $p and reaches out to\n\rtouch you.\n\rYou feel refreshed.<z>", \
        TRUE, targ, o, NULL, TO_CHAR);
      targ->addToMove(dmg);
    }
  }
  act("<B>Blue flames erupt around $p.<z>", \
    TRUE, targ, o, NULL, TO_ROOM);
  act("<B>Blue flames erupt around $p.<z>", \
    TRUE, targ, o, NULL, TO_CHAR);
  // delete all items in container
  TThing *next = table->rider->nextRider;
  for (t = table->rider; t; t = next) {
    next = t->nextRider;
    act("<b>The flames incinerate $p.<z>", TRUE, targ, t, NULL, TO_CHAR);
    act("<b>The flames incinerate $p.<z>", TRUE, targ, t, NULL, TO_ROOM);
    delete t;
  }
  if (rc == -1)
    return DELETE_VICT;
  else
    return FALSE;
  
}

/* The following proc is meant for a worn object that is personalized,
 * lest things get out of control
 * NOTE: the proc uses the the 4th value of 4 values, which works for 
 * armor and worn object... check first for other types
 */
int objWornAstralWalk(TBeing *targ, cmdTypeT cmd, const char *arg, TObj *o, TObj *)
{
  TBeing *ch;
  int rc;
  affectedData aff;
  int location;
  TRoom *room = NULL;
  sstring new_name, name_end, old_name;
  int i = 1;
  
  if (!o or !(ch = dynamic_cast<TBeing *>(o->equippedBy)))
    return FALSE;
  if (!arg)
    return FALSE;

  sstring buf2, buf=sstring(arg).word(0);

  if (cmd == CMD_USE)
  {
    if (!isname(buf, o->getName())) {
      return FALSE;
    }
 
    act("$n wraps $s fingers around $p.", TRUE, ch,o,NULL,TO_ROOM,NULL);
    act("You wrap your fingers around $p.", TRUE, ch,o,NULL,TO_CHAR,NULL);
    

    if (ch->checkForSkillAttempt(SPELL_ASTRAL_WALK)) {
      act("The $o's powers can only be used once per day.",
          TRUE,ch,o,NULL,TO_CHAR,NULL);
      return TRUE;
    }
    
    old_name = sstring(o->name);
    while(old_name.word(i+1) != "") {
      i++;
    }
    name_end = old_name.word(i);
    location = atoi(name_end.substr(1,name_end.length()-2).c_str());
    if (location == 0) {
      location = Room::TOP_OF_TREE;
    }
    room = real_roomp(location);
    
    if (!room) {
      room = real_roomp(Room::TOP_OF_TREE);
      if (!room) {
        vlogf(LOG_BUG, "Attempt to astral to NULL room in objWornAstralWalk.");
        act("Something went wrong with the $o and you don't go anywhere. [BUG]",
            TRUE,ch,o,NULL,TO_CHAR,NULL);
        return TRUE;
      }
    }

    if (room->isFlyingSector() || ch->roomp->isFlyingSector()) {
      act("The room's magic prevents the $o from functioning.",
          TRUE,ch,o,NULL,TO_CHAR,NULL);
      act("Nothing seems to happen.", FALSE, ch, NULL, NULL, TO_ROOM);
      return TRUE;
    }

    if (ch->roomp->isRoomFlag(ROOM_NO_ESCAPE) &&
        !ch->isImmortal()) {
      act("The room's magic prevents the $o from functioning.",
          TRUE,ch,o,NULL,TO_CHAR,NULL);
      act("Nothing seems to happen.", FALSE, ch, NULL, NULL, TO_ROOM);
      return TRUE;
    }

    if ( room->isRoomFlag(ROOM_PRIVATE) ||
        room->isRoomFlag(ROOM_HAVE_TO_WALK) ||
        (zone_table[room->getZoneNum()].enabled == FALSE) ||
        (toggleInfo[TOG_QUESTCODE2]->toggle &&
         room->number >= 5700 && room->number < 5900) ||
        room->isRoomFlag(ROOM_NO_MAGIC)) {
      act("The room's magic prevents the $o from functioning.",
          TRUE,ch,o,NULL,TO_CHAR,NULL);
      act("Nothing seems to happen.", FALSE, ch, NULL, NULL, TO_ROOM);
      return TRUE;
    }
    
    // SKILL ATTEMPT (PREVENT IMMEDIATE RE-USE)
    aff.type = AFFECT_SKILL_ATTEMPT;
    aff.level = 0;
    aff.duration = 24*UPDATES_PER_MUDHOUR;
    aff.location = APPLY_NONE;
    aff.modifier = SPELL_ASTRAL_WALK;
    
    act("$n opens a door to another dimension and steps through.",
      TRUE, ch,o,NULL,TO_ROOM,NULL);
    act("You open a door to another dimension and step through.",
      TRUE, ch,o,NULL,TO_CHAR,NULL);
    ch->roomp->playsound(SOUND_SPELL_ASTRAL_WALK, SOUND_TYPE_MAGIC);

    --(*ch);
    *room += *ch;
    ch->doLook("", CMD_LOOK);

    act("You are blinded for a moment as $n appears in a flash of light!",
            FALSE, ch, NULL, NULL, TO_ROOM);

    if (ch->riding) {
      rc = ch->riding->genericMovedIntoRoom(room, -1);
      if (IS_SET_DELETE(rc, DELETE_THIS)) {
        delete ch->riding;
        ch->riding = NULL;
      }
    } else {
      rc = ch->genericMovedIntoRoom(room, -1);
      if (IS_SET_DELETE(rc, DELETE_THIS)) 
      {
        ch->reformGroup();
        delete ch;
        ch = NULL;
        return TRUE;
      }
    }

    if (!(ch->isImmortal())) ch->affectTo(&aff);
    
    ch->addToWait(combatRound(3));
    
    return TRUE;
    
  } else if (cmd == CMD_WHISPER) {
    
    if (buf == "whence")
    {

      act("$n wraps $s fingers around $p.", TRUE, ch,o,NULL,TO_ROOM,NULL);
      act("You wrap your fingers around $p.", TRUE, ch,o,NULL,TO_CHAR,NULL);

      old_name = sstring(o->name);
      new_name = old_name.word(0);
      while(old_name.word(i+1) != "") {
        new_name += format(" %s") % old_name.word(i);
        i++;
      }
      name_end = old_name.word(i);
      
        
      if (!(atoi(name_end.substr(1,name_end.length()-2).c_str())))
      {
        new_name += format(" %s") % name_end;
      }
    
      location = ch->in_room;
      
      room = real_roomp(location);
      
      if (!room) {
        vlogf(LOG_BUG, "Attempt to astral to NULL room in objWornAstralWalk.");
        act("Something went wrong with the $o and you don't go anywhere. [BUG]",
            TRUE,ch,o,NULL,TO_CHAR,NULL);
        return TRUE;
      }

      if (room->isFlyingSector() || ch->roomp->isFlyingSector()) {
        act("The room's magic prevents the $o from functioning.",
            TRUE,ch,o,NULL,TO_CHAR,NULL);
        act("Nothing seems to happen.", FALSE, ch, NULL, NULL, TO_ROOM);
        return TRUE;
      }

      if (ch->roomp->isRoomFlag(ROOM_NO_ESCAPE) &&
          !ch->isImmortal()) {
        act("The room's magic prevents the $o from functioning.",
            TRUE,ch,o,NULL,TO_CHAR,NULL);
        act("Nothing seems to happen.", FALSE, ch, NULL, NULL, TO_ROOM);
        return TRUE;
      }

      if ( room->isRoomFlag(ROOM_PRIVATE) ||
          room->isRoomFlag(ROOM_HAVE_TO_WALK) ||
          (zone_table[room->getZoneNum()].enabled == FALSE) ||
          (toggleInfo[TOG_QUESTCODE2]->toggle &&
           room->number >= 5700 && room->number < 5900) ||
          room->isRoomFlag(ROOM_NO_MAGIC)) {
        act("The room's magic prevents the $o from functioning.",
            TRUE,ch,o,NULL,TO_CHAR,NULL);
        act("Nothing seems to happen.", FALSE, ch, NULL, NULL, TO_ROOM);
        return TRUE;
      }
   
      o->swapToStrung();
      delete [] o->name;
      new_name += format(" [%d]") % location;
      o->name = mud_str_dup(new_name);
      act("Your $o throbs.", 
          TRUE, ch,o,NULL,TO_CHAR,NULL);
      return TRUE;
    } else if (buf == "whither") {

      act("$n wraps $s fingers around $p.", TRUE, ch,o,NULL,TO_ROOM,NULL);
      act("You wrap your fingers around $p.", TRUE, ch,o,NULL,TO_CHAR,NULL);
      
      old_name = sstring(o->name);
      while(old_name.word(i+1) != "") {
        i++;
      }
      name_end = old_name.word(i);
      location = atoi(name_end.substr(1,name_end.length()-2).c_str());
      
      act("Another place drifts across your perception...", 
          TRUE, ch,o,NULL,TO_CHAR,NULL);
      if (location == 0)
        location = Room::TOP_OF_TREE;
      room = real_roomp(location);
      ch->sendTo(format("%s\n\r") % room->getName());
      return TRUE;
    } else
      return FALSE;
  }

  return FALSE;

}

/* The following proc is meant for a worn object that is personalized,
 * lest things get out of control
 * NOTE: the proc uses the the 4th value of 4 values, which works for 
 * armor and worn object... check first for other types
 */
int objWornMinorAstralWalk(TBeing *targ, cmdTypeT cmd, const char *arg, TObj *o, TObj *)
{
  TBeing *ch;
  int rc;
  affectedData aff;
  int location;
  TRoom *room = NULL;
  sstring new_name, name_end, old_name;
  
  if (!o or !(ch = dynamic_cast<TBeing *>(o->equippedBy)))
    return FALSE;
  if (!arg)
    return FALSE;

  sstring buf=sstring(arg).word(0);

  if (cmd == CMD_USE)
  {
    if (!isname(buf, o->getName())) {
      return FALSE;
    }
 
    act("$n wraps $s fingers around $p.", TRUE, ch,o,NULL,TO_ROOM,NULL);
    act("You wrap your fingers around $p.", TRUE, ch,o,NULL,TO_CHAR,NULL);
    

    if (ch->checkForSkillAttempt(SPELL_ASTRAL_WALK)) {
      act("The $o's powers can only be used once per day.",
          TRUE,ch,o,NULL,TO_CHAR,NULL);
      return TRUE;
    }
    
    location = Room::TOP_OF_TREE;
    room = real_roomp(location);
    
    if (!room) {
      vlogf(LOG_BUG, "Attempt to astral to NULL room in objWornMinorAstralWalk.");
      act("Something went wrong with the $o and you don't go anywhere. [BUG]",
          TRUE,ch,o,NULL,TO_CHAR,NULL);
      return TRUE;
    }

    if (room->isFlyingSector() || ch->roomp->isFlyingSector()) {
      act("The room's magic prevents the $o from functioning.",
          TRUE,ch,o,NULL,TO_CHAR,NULL);
      act("Nothing seems to happen.", FALSE, ch, NULL, NULL, TO_ROOM);
      return TRUE;
    }

    if (ch->roomp->isRoomFlag(ROOM_NO_ESCAPE) &&
        !ch->isImmortal()) {
      act("The room's magic prevents the $o from functioning.",
          TRUE,ch,o,NULL,TO_CHAR,NULL);
      act("Nothing seems to happen.", FALSE, ch, NULL, NULL, TO_ROOM);
      return TRUE;
    }

    if ( room->isRoomFlag(ROOM_PRIVATE) ||
        room->isRoomFlag(ROOM_HAVE_TO_WALK) ||
        (zone_table[room->getZoneNum()].enabled == FALSE) ||
        (toggleInfo[TOG_QUESTCODE2]->toggle &&
         room->number >= 5700 && room->number < 5900) ||
        room->isRoomFlag(ROOM_NO_MAGIC)) {
      act("The room's magic prevents the $o from functioning.",
          TRUE,ch,o,NULL,TO_CHAR,NULL);
      act("Nothing seems to happen.", FALSE, ch, NULL, NULL, TO_ROOM);
      return TRUE;
    }
    
    // SKILL ATTEMPT (PREVENT IMMEDIATE RE-USE)
    aff.type = AFFECT_SKILL_ATTEMPT;
    aff.level = 0;
    aff.duration = 24*UPDATES_PER_MUDHOUR;
    aff.location = APPLY_NONE;
    aff.modifier = SPELL_ASTRAL_WALK;
    
    act("$n opens a door to another dimension and steps through.",
      TRUE, ch,o,NULL,TO_ROOM,NULL);
    act("You open a door to another dimension and step through.",
      TRUE, ch,o,NULL,TO_CHAR,NULL);
    ch->roomp->playsound(SOUND_SPELL_ASTRAL_WALK, SOUND_TYPE_MAGIC);

    --(*ch);
    *room += *ch;
    ch->doLook("", CMD_LOOK);

    act("You are blinded for a moment as $n appears in a flash of light!",
            FALSE, ch, NULL, NULL, TO_ROOM);

    if (ch->riding) {
      rc = ch->riding->genericMovedIntoRoom(room, -1);
      if (IS_SET_DELETE(rc, DELETE_THIS)) {
        delete ch->riding;
        ch->riding = NULL;
      }
    } else {
      rc = ch->genericMovedIntoRoom(room, -1);
      if (IS_SET_DELETE(rc, DELETE_THIS)) 
      {
        ch->reformGroup();
        delete ch;
        ch = NULL;
        return TRUE;
      }
    }

    if (!(ch->isImmortal())) ch->affectTo(&aff);
    
    ch->addToWait(combatRound(3));
    
    return TRUE;
  } 
  return FALSE;

}

/* The following proc is meant for a worn object that is personalized,
 * lest things get out of control
 * NOTE: the proc uses the the 4th value of 4 values, which works for 
 * armor and worn object... check first for other types
 */
int objWornPortal(TBeing *targ, cmdTypeT cmd, const char *arg, TObj *o, TObj *)
{
  TBeing *ch;
  affectedData aff;
  int location;
  TRoom *room = NULL;
  sstring new_name, name_end, old_name;
  int i = 1;
  
  if (!o or !(ch = dynamic_cast<TBeing *>(o->equippedBy)))
    return FALSE;

  if (!arg)
    return FALSE;

  sstring buf2, buf=sstring(arg).word(0);

  if (cmd == CMD_USE)
  {
    if (!isname(buf, o->getName())) {
      return FALSE;
    }

    act("$n wraps $s fingers around $p.", TRUE, ch,o,NULL,TO_ROOM,NULL);
    act("You wrap your fingers around $p.", TRUE, ch,o,NULL,TO_CHAR,NULL);
    
    if (ch->checkForSkillAttempt(SPELL_PORTAL)) {
      act("The $o's powers can only be used once per day.",
          TRUE,ch,o,NULL,TO_CHAR,NULL);
      return TRUE;
    }
    
    old_name = sstring(o->name);
    while(old_name.word(i+1) != "") {
      i++;
    }
    name_end = old_name.word(i);
    location = atoi(name_end.substr(1,name_end.length()-2).c_str());
    if (location == 0) {
      location = Room::TOP_OF_TREE;
    }
    room = real_roomp(location);
    
    if (!room) {
      room = real_roomp(Room::TOP_OF_TREE);
      if (!room) {
        vlogf(LOG_BUG, "Attempt to portal to NULL room in objWornPortal.");
        act("Something went wrong with the $o and you don't go anywhere. [BUG]",
            TRUE,ch,o,NULL,TO_CHAR,NULL);
        return TRUE;
      }
    }

    if (room->isFlyingSector() || ch->roomp->isFlyingSector()) {
      act("The room's magic prevents the $o from functioning.",
          TRUE,ch,o,NULL,TO_CHAR,NULL);
      act("Nothing seems to happen.", FALSE, ch, NULL, NULL, TO_ROOM);
      return TRUE;
    }

    if (ch->roomp->isRoomFlag(ROOM_NO_ESCAPE) &&
        !ch->isImmortal()) {
      act("The room's magic prevents the $o from functioning.",
          TRUE,ch,o,NULL,TO_CHAR,NULL);
      act("Nothing seems to happen.", FALSE, ch, NULL, NULL, TO_ROOM);
      return TRUE;
    }

    if ( room->isRoomFlag(ROOM_PRIVATE) ||
        room->isRoomFlag(ROOM_HAVE_TO_WALK) ||
        (zone_table[room->getZoneNum()].enabled == FALSE) ||
        (toggleInfo[TOG_QUESTCODE2]->toggle &&
         room->number >= 5700 && room->number < 5900) ||
        room->isRoomFlag(ROOM_NO_MAGIC)) {
      act("The room's magic prevents the $o from functioning.",
          TRUE,ch,o,NULL,TO_CHAR,NULL);
      act("Nothing seems to happen.", FALSE, ch, NULL, NULL, TO_ROOM);
      return TRUE;
    }
    
    // SKILL ATTEMPT (PREVENT IMMEDIATE RE-USE)
    aff.type = AFFECT_SKILL_ATTEMPT;
    aff.level = 0;
    aff.duration = 24*UPDATES_PER_MUDHOUR;
    aff.location = APPLY_NONE;
    aff.modifier = SPELL_PORTAL;

    TPerson *tPerson = dynamic_cast<TPerson *>(ch);
    TPortal * tmp_obj = new TPortal(room);
    tmp_obj->setPortalNumCharges(11);
    *ch->roomp += *tmp_obj;

    if (tPerson)
      tmp_obj->checkOwnersList(tPerson);

    ch->roomp->playsound(SOUND_SPELL_PORTAL, SOUND_TYPE_MAGIC);

    TPortal * next_tmp_obj = new TPortal(ch->roomp);
    *room += *next_tmp_obj;

    if (tPerson)
      next_tmp_obj->checkOwnersList(tPerson);

    room->playsound(SOUND_SPELL_PORTAL, SOUND_TYPE_MAGIC);

    act("$p suddenly appears out of a swirling mist.", TRUE, ch, tmp_obj, NULL, TO_ROOM);
    act("$p suddenly appears out of a swirling mist.", TRUE, ch, tmp_obj, NULL, TO_CHAR);

    buf2 = format("%s suddenly appears out of a swirling mist.\n\r") % 
      sstring(next_tmp_obj->shortDescr).cap();
    sendToRoom(buf2.c_str(), location);

 
    if (!(ch->isImmortal())) ch->affectTo(&aff);
    
    ch->addToWait(combatRound(3));
    
    return TRUE;
    
  } else if (cmd == CMD_WHISPER) {
    
    if (buf == "whence")
    {

      act("$n wraps $s fingers around $p.", TRUE, ch,o,NULL,TO_ROOM,NULL);
      act("You wrap your fingers around $p.", TRUE, ch,o,NULL,TO_CHAR,NULL);

      old_name = sstring(o->name);
      new_name = old_name.word(0);
      while(old_name.word(i+1) != "") {
        new_name += format(" %s") % old_name.word(i);
        i++;
      }
      name_end = old_name.word(i);
      
        
      if (!(atoi(name_end.substr(1,name_end.length()-2).c_str())))
      {
        new_name += format(" %s") % name_end;
      }
    
      location = ch->in_room;
      
      room = real_roomp(location);
      
      if (!room) {
        vlogf(LOG_BUG, "Attempt to astral to NULL room in objPortalWalk.");
        act("Something went wrong with the $o and you don't go anywhere. [BUG]",
            TRUE,ch,o,NULL,TO_CHAR,NULL);
        return TRUE;
      }

      if (room->isFlyingSector() || ch->roomp->isFlyingSector()) {
        act("The room's magic prevents the $o from functioning.",
            TRUE,ch,o,NULL,TO_CHAR,NULL);
        act("Nothing seems to happen.", FALSE, ch, NULL, NULL, TO_ROOM);
        return TRUE;
      }

      if (ch->roomp->isRoomFlag(ROOM_NO_ESCAPE) &&
          !ch->isImmortal()) {
        act("The room's magic prevents the $o from functioning.",
            TRUE,ch,o,NULL,TO_CHAR,NULL);
        act("Nothing seems to happen.", FALSE, ch, NULL, NULL, TO_ROOM);
        return TRUE;
      }

      if ( room->isRoomFlag(ROOM_PRIVATE) ||
          room->isRoomFlag(ROOM_HAVE_TO_WALK) ||
          (zone_table[room->getZoneNum()].enabled == FALSE) ||
          (toggleInfo[TOG_QUESTCODE2]->toggle &&
           room->number >= 5700 && room->number < 5900) ||
          room->isRoomFlag(ROOM_NO_MAGIC)) {
        act("The room's magic prevents the $o from functioning.",
            TRUE,ch,o,NULL,TO_CHAR,NULL);
        act("Nothing seems to happen.", FALSE, ch, NULL, NULL, TO_ROOM);
        return TRUE;
      }
   
      o->swapToStrung();
      delete [] o->name;
      new_name += format(" [%d]") % location;
      o->name = mud_str_dup(new_name);
      act("Your $o throbs.", 
          TRUE, ch,o,NULL,TO_CHAR,NULL);
      return TRUE;
    } else if (buf == "whither") {

      act("$n wraps $s fingers around $p.", TRUE, ch,o,NULL,TO_ROOM,NULL);
      act("You wrap your fingers around $p.", TRUE, ch,o,NULL,TO_CHAR,NULL);
      
      old_name = sstring(o->name);
      while(old_name.word(i+1) != "") {
        i++;
      }
      name_end = old_name.word(i);
      location = atoi(name_end.substr(1,name_end.length()-2).c_str());
      
      act("Another place drifts across your perception...", 
          TRUE, ch,o,NULL,TO_CHAR,NULL);
      if (location == 0)
        location = Room::TOP_OF_TREE;
      room = real_roomp(location);
      ch->sendTo(format("%s\n\r") % room->getName());
      return TRUE;
    } else
      return FALSE;
  }

  return FALSE;

}


