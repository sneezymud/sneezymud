//////////////////////////////////////////////////////////////////////////
//
// SneezyMUD - All rights reserved, SneezyMUD Coding Team
//
//    "cmd_look.cc" - The look command
//
//////////////////////////////////////////////////////////////////////////

#include "extern.h"
#include "handler.h"
#include "room.h"
#include "being.h"
#include "games.h"
#include "obj_base_corpse.h"
#include "obj_table.h"
#include "obj_book.h"
#include "obj_base_cup.h"
#include "obj_drinkcon.h"
#include "liquids.h"
#include "colorstring.h"
#include "person.h"
#include "monster.h"
#include "game_crazyeights.h"
#include "game_drawpoker.h"

void TBaseCup::lookObj(TBeing *ch, int) const
{
  int temp;

  if (getMaxDrinkUnits()/128) {
    ch->sendTo(COLOR_OBJECTS, format("%s has a capacity of %d gallon%s, %d fluid ounce%s.\n\r") %
          sstring(ch->pers(this)).cap() %
          (getMaxDrinkUnits()/128) %
          (getMaxDrinkUnits()/128 == 1 ? "" : "s") %
          (getMaxDrinkUnits()%128) %
          (getMaxDrinkUnits()%128 == 1 ? "" : "s"));
  } else {
    ch->sendTo(COLOR_OBJECTS, format("%s has a capacity of %d fluid ounce%s.\n\r") %
          sstring(ch->pers(this)).cap() %
	       (getMaxDrinkUnits()%128) %
          (getMaxDrinkUnits()%128 == 1 ? "" : "s"));
  }
  if (getDrinkUnits() <= 0 || !getMaxDrinkUnits())
    act("It is empty.", FALSE, ch, 0, 0, TO_CHAR);
  else {
    temp = ((getDrinkUnits() * 3) / getMaxDrinkUnits());
    ch->sendTo(COLOR_OBJECTS, format("It's %sfull of a %s%s liquid.\n\r") %
	       fullness[temp] %
	       (isDrinkConFlag(DRINK_FROZEN)?"<C>frozen<1> ":"") %
	       liquidInfo[getDrinkType()]->color);
  }
}

void TObj::lookObj(TBeing *ch, int bits) const
{
  ch->sendTo("That is not a container.\n\r");
}

void TThing::lookAtObj(TBeing *ch, const char *, showModeT x) const
{
  ch->showTo(this, x);        // Show no-description 
  ch->describeObject(this);
}

void TBeing::lookDark()
{
  sendTo("It is very dark in here...\n\r");
  
  // this already handles stuff like infravision, and glowing mobs

  list_char_in_room(roomp->stuff, this);
  
  for(StuffIter it=roomp->stuff.begin();it!=roomp->stuff.end();++it) {
    if (dynamic_cast<TObj *>(*it) && canSee(*it))   // glowing objects
      showTo(*it, SHOW_MODE_DESC_PLUS);
  }
}

void TBeing::lookDir(int keyword_no)
{
  roomDirData *exitp;
  const char *tmp_desc=NULL;
  TRoom *rp;

  if (keyword_no >= 10)  // adjust cause of our whacky array
    keyword_no -= 4;
  
  sendTo(format("You look %swards.\n\r") % dirs[keyword_no]);
  act(format("$n looks %swards.") % dirs[keyword_no], TRUE, this, 0, 0, TO_ROOM);

  if (!(exitp = exitDir(dirTypeT(keyword_no)))) {
    if (roomp && roomp->ex_description &&
	(tmp_desc = roomp->ex_description->findExtraDesc(dirs[keyword_no])))
      sendTo(tmp_desc);
    else
      sendTo("You see nothing special.\n\r");

    return;
  } else {
    if (canSeeThruDoor(exitp)) {
      if (exitp->description)
	sendTo(COLOR_ROOMS, exitp->description);
      else {
	if (exitp->to_room && (rp = real_roomp(exitp->to_room))) {
	  if (IS_SET(desc->plr_color, PLR_COLOR_ROOM_NAME)) {
	    if (hasColorStrings(NULL, rp->getName(), 2)) {
	      sendTo(COLOR_ROOM_NAME, format("You see %s<1>.\n\r") %
		     dynColorRoom(rp, 1, TRUE));
	    } else {
	      sendTo(COLOR_ROOM_NAME, format("You see %s%s%s.\n\r") %                           addColorRoom(rp, 1) % rp->name  %norm());
	    }
	  } else {
	    sendTo(COLOR_BASIC, format("You see %s%s%s.\n\r") % purple() % 
		   rp->getNameNOC(this) % norm());
	  }
	} else {
	  sendTo("You see nothing special.\n\r");
	  vlogf(LOG_BUG, format("Bad room exit in room %d") %  in_room);
	}
      }

      if (keyword_no != DIR_UP && keyword_no != DIR_DOWN)
	if ((exitp->condition & EX_SLOPED_UP))
	  sendTo("The way seems sloped up in that direction.\n\r");
	else if ((exitp->condition & EX_SLOPED_DOWN))
	  sendTo("The way seems sloped down in that direction.\n\r");

      if (!(rp = real_roomp(exitp->to_room)))
        sendTo("You see swirling chaos.\n\r");
      else {
        if (!isPlayerAction(PLR_BRIEF))
          sendRoomDesc(rp);

        listExits(rp);
        list_thing_in_room(rp->stuff, this);
      }
    } else if (!(exitp->condition & EX_SECRET))
      sendTo(format("The %s is closed.\n\r") % exitp->getName());
    else
      sendTo("You see nothing special.\n\r");
  }
}

void TBeing::lookInObj(sstring arg2, TThing *specific, unsigned int bits, const sstring &argument, cmdTypeT cmd)
{
  TBeing *tmp_char = NULL;
  TObj *o = NULL;

  if (!(arg2.empty()) || specific) {
    if (specific) {
      if (!dynamic_cast<TObj *> (specific)) {
	sendTo("Look in what?!\n\r");
      } else {
	TObj *tmpO = dynamic_cast<TObj *> (specific);
	if (tmpO->parent && (this == parent)) {
	  bits = FIND_OBJ_INV;
	} else if (tmpO->equippedBy && (this == tmpO->equippedBy)) {
	  bits = FIND_OBJ_EQUIP;
	} else if (tmpO->parent && (roomp == tmpO->parent)) {
	  bits = FIND_OBJ_ROOM;
	}
	if ((bits == FIND_OBJ_ROOM) && riding && tmpO->parent) {
	  sendTo(format("You can't look into items on the %s while mounted!\n\r") %roomp->describeGround());
	  return;
	} else {
	  tmpO->lookObj(this, bits);
	}
      }
      return;
    }

    // handle the look in all.corpse special case
    if (is_abbrev(arg2, "all.corpse") && arg2.length() > 6) {
      TThing *t=NULL;
      for(StuffIter it=roomp->stuff.begin();it!=roomp->stuff.end() && (t=*it);++it) {
	TBaseCorpse * tbc = dynamic_cast<TBaseCorpse *>(t);
	if (tbc) {
	  // because some corpses are dust piles, we make no name check
	  doLook(argument, cmd, tbc);
	}
      }
      return;
    }

    bits = generic_find(arg2.c_str(), FIND_OBJ_INV | FIND_OBJ_ROOM | FIND_OBJ_EQUIP, this, &tmp_char, &o);
    if (bits) {
      if ((bits == FIND_OBJ_ROOM) && riding && o->parent) {
	// we want to allow them to look at item's on a table, but not
	// in a bag while on a horse.
	sendTo(format("You can't look into items on the %s while mounted!\n\r") % roomp->describeGround());
	return;
      }

      o->lookObj(this, bits);
            
    } else        // wrong argument 
      sendTo("You do not see that item here.\n\r");
  } else                // no argument 
    sendTo("Look in what?!\n\r");
}


// this is just "look" with no arguments
void TBeing::lookRoom()
{
  int res;

  // purple if color basic, nothing if no color, 
  // varied color if color room name
  sendRoomName(roomp);
  if (!isPlayerAction(PLR_BRIEF)) 
    sendRoomDesc(roomp);

  describeWeather(in_room);	
  describeGround();
  describeRoomLight();
  //  doEvaluate("room");
  listExits(roomp);

  if (dynamic_cast<TPerson *>(this)) {
    if (isPlayerAction(PLR_HUNTING)) {
      if (affectedBySpell(SKILL_TRACK)) {
	if (!(res = track(specials.hunting))) {
	  specials.hunting = 0;
	  hunt_dist = 0;
	  remPlayerAction(PLR_HUNTING);
	  affectFrom(SKILL_TRACK);
	}
      } else if (affectedBySpell(SKILL_SEEKWATER)) {
	if (!(res = track(NULL))) {
	  hunt_dist = 0;
	  remPlayerAction(PLR_HUNTING);
	  affectFrom(SKILL_SEEKWATER);
	}
      } else {
	hunt_dist = 0;
	remPlayerAction(PLR_HUNTING);
      }
    }
  } else {
    if (specials.act & ACT_HUNTING) {
      if (affectedBySpell(SKILL_TRACK)) {
	if (!(res = track(specials.hunting))) {
	  specials.hunting = 0;
	  hunt_dist = 0;
	  REMOVE_BIT(specials.act, ACT_HUNTING);
	  affectFrom(SKILL_TRACK);
	}
      } else if (affectedBySpell(SKILL_SEEKWATER)) {
	if (!(res = track(NULL))) {
	  hunt_dist = 0;
	  REMOVE_BIT(specials.act, ACT_HUNTING);
	  affectFrom(SKILL_SEEKWATER);
	}
      } else if (specials.hunting) {
	if (!(res = track(specials.hunting))) {
	  specials.hunting = 0;
	  hunt_dist = 0;
	  REMOVE_BIT(specials.act, ACT_HUNTING);
	}
      } else {
	hunt_dist = 0;
	REMOVE_BIT(specials.act, ACT_HUNTING);
      }
    }
  }
  list_thing_in_room(roomp->stuff, this);
}

void TBeing::lookAtRoom()
{
  int res;

  // purple if color basic, nothing if no color, 
  // varied color if color room name
  sendRoomName(roomp);
  sendRoomDesc(roomp);
  describeWeather(in_room);
  listExits(roomp);

  if (dynamic_cast<TPerson *>(this)) {
    if (isPlayerAction(PLR_HUNTING)) {
      if (affectedBySpell(SKILL_TRACK)) {
	if (!(res = track(specials.hunting))) {
	  specials.hunting = 0;
	  hunt_dist = 0;
	  remPlayerAction(PLR_HUNTING);
	  affectFrom(SKILL_TRACK);
	}
      } else if (affectedBySpell(SKILL_SEEKWATER)) {
	if (!(res = track(NULL))) {
	  hunt_dist = 0;
	  remPlayerAction(PLR_HUNTING);
	  affectFrom(SKILL_SEEKWATER);
	}
      } else {
	hunt_dist = 0;
	remPlayerAction(PLR_HUNTING);
      }
    }
  } else {
    if (IS_SET(specials.act, ACT_HUNTING)) {
      if (affectedBySpell(SKILL_TRACK)) {
	if (!(res = track(specials.hunting))) {
	  specials.hunting = 0;
	  hunt_dist = 0;
	  REMOVE_BIT(specials.act, ACT_HUNTING);
	  affectFrom(SKILL_TRACK);
	}
      } else if (affectedBySpell(SKILL_SEEKWATER)) {
	if (!(res = track(NULL))) {
	  hunt_dist = 0;
	  REMOVE_BIT(specials.act, ACT_HUNTING);
	  affectFrom(SKILL_SEEKWATER);
	}
      } else if (specials.hunting) {
	if (!(res = track(specials.hunting))) {
	  specials.hunting = 0;
	  hunt_dist = 0;
	  REMOVE_BIT(specials.act, ACT_HUNTING);
	}
      } else {
	hunt_dist = 0;
	REMOVE_BIT(specials.act, ACT_HUNTING);
      }
    }
  }
  list_thing_in_room(roomp->stuff, this);
}

void TBeing::lookAtBeing(TThing *specific)
{
  TBeing *tmpBeing = dynamic_cast<TBeing *> (specific);
  bool   bIsSpying = (isAffected(AFF_SCRYING) ? !::number(0, (getSkillValue(SKILL_SPY) * 10)) : false);
  TThing *t=NULL;
  sstring arg1;

  showTo(tmpBeing, SHOW_MODE_SHORT_PLUS);

  if (bIsSpying) // Let thieves have a chance of learning spy when looking at someone.
    bSuccess(SKILL_SPY);

  if (this != tmpBeing && !affectedBySpell(SKILL_SPY) && !tmpBeing->isImmortal()) {
    act("$n looks at you.", TRUE, this, 0, tmpBeing, TO_VICT);
    act("$n looks at $N.", TRUE, this, 0, tmpBeing, TO_NOTVICT);
    if (!tmpBeing->isPc() && !isname("[clone]", tmpBeing->name))
      dynamic_cast<TMonster *>(tmpBeing)->aiLook(this);
  } else if (tmpBeing != this && !tmpBeing->isImmortal()) {
    // Thieves in the room will be able to detect spying looks.
    TBeing *bOther;
    for(StuffIter it=roomp->stuff.begin();it!=roomp->stuff.end() && (t=*it);++it)
      if ((bOther = dynamic_cast<TBeing *>(t)) &&
	  (bOther->affectedBySpell(SKILL_SPY) ||
	   bOther->isAffected(AFF_SCRYING)) &&
	  bOther->GetMaxLevel() >= GetMaxLevel() &&
	  bOther != this) {
	arg1 = format("You detect $n looking at %s with spying eyes.") % (bOther == tmpBeing ? "you" : tmpBeing->getName());
	act(arg1, TRUE, this, 0, bOther, TO_VICT);
	if (bOther == tmpBeing && !tmpBeing->isPc()
	    && !isname("[clone]", tmpBeing->name))
	  dynamic_cast<TMonster *>(tmpBeing)->aiLook(this);
      }
  }
}

void TBeing::lookingAtObj(TThing *specific)
{
  char *tmp=NULL;
  const char *tmp_desc;
  int found;
  TObj *tmpObj = dynamic_cast<TObj *> (specific);

  if (!canSee(tmpObj)) {
    sendTo("Look at what?\n\r");
    return;
  }
 
  if (tmpObj->ex_description) {
    if ((tmp_desc = tmpObj->ex_description->findExtraDesc(tmp))) {
      sstring tmp_desc_str = tmp_desc;
      desc->page_string(tmp_desc_str.toCRLF());
      found = TRUE;
      describeObject(tmpObj);
      if (tmpObj->riding)
	sendTo(COLOR_OBJECTS, format("%s is on %s.") % tmpObj->getName() % tmpObj->riding->getName());
      showTo(tmpObj, SHOW_MODE_PLUS);
      return;
    } else {
      tmpObj->lookAtObj(this, tmp, SHOW_MODE_TYPE);
      if (tmpObj->riding)
	sendTo(COLOR_OBJECTS, format("%s is on %s.") % tmpObj->getName() % tmpObj->riding->getName());
      return;
    }
  } else {
    tmpObj->lookAtObj(this, tmp, SHOW_MODE_TYPE);
    return;
  }
}

void TBeing::doLook(const sstring &argument, cmdTypeT cmd, TThing *specific)
{
  char *tmp;
  const char *tmp_desc;
  sstring arg1, arg2;
  int keyword_no, j, found, totalFound = 0, iNum = 0;
  unsigned int bits = 0;
  TThing *t = NULL, *t2 = NULL;
  TObj *o = NULL;
  TObj *o2 = NULL;
  TBeing *tmp_char = NULL;

  static const char *keywords[] = {
    "north",    // 0
    "east",
    "south",
    "west",
    "up",
    "down",     // 5
    "in",
    "at",
    "",                                // Look at '' case 
    "room",
    "ne",      // 10
    "nw",
    "se",
    "sw",
    "\n"
  };

  // sendrpf(COLOR_NONE, roomp, "argument=[%s]\n\r", argument.c_str());
  if (!desc || !roomp)
    return;

  // check casino games
  if (gGin.check(this)) {
    if (gGin.look(this, argument.c_str()))
      return;
  }
  if (checkHearts()) {
    if (gHearts.look(this, argument.c_str()))
      return;
  }
  if (checkCrazyEights()) {
    if (gEights.look(this, argument.c_str()))
      return;
  }
  if (checkDrawPoker()) {
    if (gDrawPoker.look(this, argument.c_str()))
      return;
  }


  // check position and blindness
  if (getPosition() < POSITION_SLEEPING){
    sendTo("You can't see anything but stars!\n\r");
    return;
  } else if (getPosition() == POSITION_SLEEPING){
    sendTo("You can't see anything -- you're sleeping!\n\r");
    return;
  } else if (isAffected(AFF_BLIND) && !isImmortal() && 
	   !isAffected(AFF_TRUE_SIGHT) && !isAffected(AFF_CLARITY)) {
    sendTo("You can't see a damn thing -- you're blinded!\n\r");
    return;
  } else if (roomp->pitchBlackDark() && !isImmortal() &&
           (visionBonus <= 0) &&
           !(roomp->getRoomFlags() & ROOM_ALWAYS_LIT) &&
           !isAffected(AFF_TRUE_SIGHT) && !isAffected(AFF_CLARITY)) {
    lookDark();
    return;
  }




  // we use only_arg so we pick up "at" from "look at the window"
  // then we start using one_argument so we will munch "the" in above example
  if (argument.find("at ") != sstring::npos) {
    arg2 = argument.substr(3);
    keyword_no = 7;
  } else if (argument.find("in ") != sstring::npos) {
    arg2 = argument.substr(3);
    keyword_no = 6;
  } else if (argument.find("on ") != sstring::npos) {
    arg2 = argument.substr(3);
    keyword_no = 6;
  } else {
    // sendrpf(COLOR_NONE, roomp, "argument=[%s]\n\r", argument.c_str());
    keyword_no = search_block(stripColorCodes(argument), keywords, FALSE);
    
    // sendrpf(COLOR_NONE, roomp, "keyword_no=[%d]\n\r", keyword_no);
    if (keyword_no == -1) {
      if (argument.empty()) {
	keyword_no = 8;
	// sendrpf(COLOR_NONE, roomp, "keyword_no=[%d]\n\r", keyword_no);
	arg2 = stripColorCodes(argument);
      } else {
	keyword_no = 7;
	// sendrpf(COLOR_NONE, roomp, "keyword_no=[%d]\n\r", keyword_no);
	arg2 = stripColorCodes(argument);
      }
    }
  }



  // sendrpf(COLOR_NONE, roomp, "arg2=[%s]\n\r", arg2.c_str());
  found = FALSE;
  o = NULL;
  tmp_desc = NULL;

  switch (keyword_no) {
    case 0:
    case 1:
    case 2:
    case 3:
    case 4:
    case 5:
    case 10:   // diagonals
    case 11:
    case 12:
    case 13:
      lookDir(keyword_no);
      break;
    case 6:
      lookInObj(arg2, specific, bits, argument, cmd);
      break;
    case 7:{
      if (!(arg2.empty()) || specific) {
	if (cmd == CMD_READ) {
	  sstring tempArg, tempArg2;
	  tempArg = one_argument(arg2, tempArg2);
	  if (is_abbrev(tempArg2, "chapter") ||
	      is_abbrev(tempArg2, "section")) {
	    sstring tempArg3;
	    if (!(tempArg.empty()))
	      tempArg = one_argument(tempArg, tempArg3);
	    if (!(tempArg.empty()) || !convertTo<int>(tempArg3)) {
	      bits = generic_find(arg2.c_str(), FIND_OBJ_INV | FIND_OBJ_EQUIP, this, &tmp_char, &o2);
	    } else if (convertTo<int>(tempArg3)) {
	      TObj * tempObj = NULL;
	      if ((tempObj = dynamic_cast<TBook *> (heldInPrimHand()))) {
		o2 = tempObj;
		bits = FIND_OBJ_EQUIP;
	      } else if ((tempObj = dynamic_cast<TBook *> (heldInSecHand()))) {
		o2 = tempObj;
		bits = FIND_OBJ_EQUIP;
	      }
	      if (!bits) {
		TThing * tempThing = NULL;
		for(StuffIter it=stuff.begin();it!=stuff.end() && (tempThing=*it);++it) {
		  if (!dynamic_cast<TBook *> (tempThing)) {
		    continue;
		  } else {
		    o2 = dynamic_cast<TBook *> (tempThing);
		    if (o2) {
		      bits = FIND_OBJ_INV;
		      break;
		    }
		  }
		}
	      }
	      if (!bits)
		bits = generic_find(arg2.c_str(), FIND_OBJ_INV | FIND_OBJ_EQUIP, this , &tmp_char, &o2);
	    } else {
	      bits = generic_find(arg2.c_str(), FIND_OBJ_INV | FIND_OBJ_EQUIP, this , &tmp_char, &o2);
	    }
	  } else {
	    bits = generic_find(arg2.c_str(), FIND_OBJ_INV | FIND_OBJ_EQUIP,
				this, &tmp_char, &o2);
	  }
	} else {
	  bits = generic_find(arg2.c_str(), FIND_OBJ_INV | FIND_OBJ_ROOM |
			      FIND_OBJ_EQUIP | FIND_CHAR_ROOM, this, &tmp_char, &o2);
	}


	if (dynamic_cast<TBeing *> (specific)) {
	  lookAtBeing(specific);
	  return;
	} 

	if (dynamic_cast<TObj *> (specific)) {
	  lookingAtObj(specific);
	  return;
	}

	if (tmp_char) {
	  lookAtBeing(tmp_char);
	  return;
	}



	char tmpname[MAX_INPUT_LENGTH];
	strcpy(tmpname, arg2.c_str());
	tmp = tmpname;
	iNum = get_number(&tmp);

	if (!strcmp(tmp, "_tele_") && !isImmortal()) {
	  sendTo("Look at what?\n\r");
	  return;
	}
	if (!found) {
	  for(StuffIter it=roomp->stuff.begin();it!=roomp->stuff.end() && (t=*it);++it) {
	    if (!dynamic_cast<TBeing *>(t))
	      continue;
	    if (!canSee(t))
	      continue;
	    if (isname(tmp, t->name)) {
	      totalFound++;
	    }
	  }
	}
	// In inventory
	if (!found) {
	  for(StuffIter it=stuff.begin();it!=stuff.end() && (t=*it);++it) {
	    if (!canSee(t))
	      continue;
	    if (t->ex_description) {
	      if ((tmp_desc = t->ex_description->findExtraDesc(tmp))) {
		sstring tmp_desc_str = tmp_desc;
		totalFound++;
		if (iNum != totalFound)
		  continue;
		if (o2 == t) {
		  // look at XX where XX is the item's name and extradesc
		  desc->page_string(tmp_desc_str.toCRLF());
		  found = TRUE;
		  describeObject(t);
		  o2 = dynamic_cast<TObj *>(t);  // for showTo(o2,6) later on
		} else {
		  // look at XX where XX is some random desc on the obj
		  desc->page_string(tmp_desc_str.toCRLF());
		  found = TRUE;
		  return;
		}
	      }
	    }
	    if (isname(tmp, t->name)) {
	      totalFound++;
	      if (iNum == totalFound) {
		t->lookAtObj(this, tmp, SHOW_MODE_TYPE);
		return;
	      } else {
		continue;
	      }
	    }
	  }
	}
	// Worn equipment second
	if (!found) {
	  for (j = MIN_WEAR; j < MAX_WEAR && !found; j++) {
	    t = equipment[j];
	    if (t) {
	      TObj *tobj = dynamic_cast<TObj *>(t);
	      if (tobj->isPaired()) {
		if (isRightHanded()) {
		  if ((j == WEAR_LEG_L) || (j == HOLD_LEFT))
		    continue;
		} else {
		  if ((j == WEAR_LEG_L) || (j == HOLD_RIGHT))
		    continue;
		}
	      }
	      if (!canSee(t))
		continue;
	      if (t->ex_description) {
		if ((tmp_desc = t->ex_description->findExtraDesc(tmp))) {
		  sstring tmp_desc_str = tmp_desc;
		  totalFound++;
		  if (iNum != totalFound)
		    continue;
		  if (o2 == t) {
		    // look at XX where XX is the item's name and extradesc
		    desc->page_string(tmp_desc_str.toCRLF());
		    found = TRUE;
		    describeObject(t);
		    o2 = dynamic_cast<TObj *>(t);  // for showTo(o2,6) later on
		  } else {
		    // look at XX where XX is some random desc on the obj
		    desc->page_string(tmp_desc_str.toCRLF());
		    found = TRUE;
		    return;
		  }
		}
	      }
	      if (isname(tmp, t->name)) {
		totalFound++;
		if (iNum == totalFound) {
		  t->lookAtObj(this, tmp, SHOW_MODE_TYPE);
		  return;
		} else {
		  continue;
		}
	      }
	    }
	  }
	}
	// room objects
	if (!found) {
	  for(StuffIter it=roomp->stuff.begin();it!=roomp->stuff.end() && (t=*it);++it) {
	    if (dynamic_cast<TBeing *>(t))
	      continue;
	    if (!canSee(t))
	      continue;
	    if (t->ex_description) {
	      if ((tmp_desc = t->ex_description->findExtraDesc(tmp))) {
		sstring tmp_desc_str = tmp_desc;
		totalFound++;
		if (iNum != totalFound)
		  continue;
		if (o2 == (TObj *) t) {
		  if (roomp->isRoomFlag(ROOM_NO_AUTOFORMAT)) {
		    desc->page_string(tmp_desc_str.toCRLF());
		  } else {
		    desc->page_string(autoFormatDesc(tmp_desc_str, false));
		  }
		  found = TRUE;
		  describeObject(t);
		  return;
		} else {
		  // look at XX where XX is some random desc on the obj
		  if (roomp->isRoomFlag(ROOM_NO_AUTOFORMAT)) {
		    desc->page_string(tmp_desc_str.toCRLF());
		  } else {
		    desc->page_string(autoFormatDesc(tmp_desc_str, false));
		  }
		  found = TRUE;
		  return;
		}
	      }
	    }
	    if (isname(tmp, t->name)) {
	      totalFound++;
	      if (iNum == totalFound) {
		t->lookAtObj(this, tmp, SHOW_MODE_TYPE);
		return;
	      } else {
		continue;
	      }
	    }
	    if (dynamic_cast<TTable *>(t)) {
	      for (t2 = t->rider; t2; t2 = t2->nextRider) {
		if (dynamic_cast<TBeing *>(t2))
		  continue;
		if (!canSee(t2))
		  continue;
		if (t2->ex_description &&
		    (tmp_desc = t2->ex_description->findExtraDesc(tmp))) {
		  totalFound++;
		  if (iNum != totalFound)
		    continue;
		  desc->page_string(tmp_desc);
		  found = TRUE;
		  describeObject(t2);
		  sendTo(COLOR_OBJECTS, format("%s is on %s.") % t2->getName() % t->getName());
		  return;
		}
		if (isname(tmp, t2->name)) {
		  totalFound++;
		  if (iNum == totalFound) {
		    t2->lookAtObj(this, tmp, SHOW_MODE_TYPE);
		    sendTo(COLOR_OBJECTS, format("%s is on %s.") % t2->getName() % t->getName());
		    return;
		  } else {
		    continue;
		  }
		}
	      }
	    }  // table
	  }
	}
	// room extras
	if (!found) {
	  if ((tmp_desc = roomp->ex_description->findExtraDesc(tmp))) {
	    totalFound++;
	    if (totalFound == iNum) {
	      sstring tmp_desc_str = tmp_desc;

	      if (roomp->isRoomFlag(ROOM_NO_AUTOFORMAT)) {
		desc->page_string(tmp_desc_str.toCRLF());
	      } else {
		desc->page_string(autoFormatDesc(tmp_desc_str, false));
	      }
	      return;
	    }
	  }
	}
	if (!found) {
	  if (bits)                 // If an object was found 
	    o2->lookAtObj(this, tmp, SHOW_MODE_TYPE);
	  else
	    sendTo("You do not see that here.\n\r");
	  return;
	}
	showTo(o2, SHOW_MODE_PLUS);
	return;
      }
      sendTo("Look at what?\n\r");
      return;
    }
      break;

      // look '' 
    case 8:
      lookRoom();
      break;
    case -1:
      // wrong arg     
      sendTo("Sorry, I didn't understand that!\n\r");
      break;
    case 9:
      lookAtRoom();
      break;
  }
}
