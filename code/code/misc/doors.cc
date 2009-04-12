//////////////////////////////////////////////////////////////////////////
//
//      SneezyMUD - All rights reserved, SneezyMUD Coding Team
//      "doors.cc" - All functions specific to doors
//
///////////////////////////////////////////////////////////////////////////

#include "extern.h"
#include "room.h"
#include "being.h"

void TBeing::rawUnlockDoor(roomDirData * exitp, dirTypeT door)
{
  TRoom *rp;
  roomDirData *back;

  REMOVE_BIT(exitp->condition, EX_LOCKED);
 /* now for unlocking the other side, too */
  rp = real_roomp(exitp->to_room);
  if (rp &&
      (back = rp->dir_option[rev_dir[door]]) &&
      back->to_room == in_room) {
    REMOVE_BIT(back->condition, EX_LOCKED);
  } else
    vlogf(LOG_LOW, format("Inconsistent door locks in rooms %d->%d") %  in_room % exitp->to_room);
}

// returns direction of door
// -1 no such door
// -2 door, but inappropriate given mode
dirTypeT TBeing::findDoor(const char *type, const char *direct, doorIntentT mode, silentTypeT silent)
{
  dirTypeT door;
  roomDirData *exitp;
  char action[20];
  char action2[20];
  char dir[64];
  strncpy(dir, direct, cElements(dir));

  if (!strcasecmp(dir, "ne"))
    strcpy(dir, "northeast");
  else if (!strcasecmp(dir, "nw"))
    strcpy(dir, "northwest");
  else if (!strcasecmp(dir, "sw"))
    strcpy(dir, "southwest");
  else if (!strcasecmp(dir, "se"))
    strcpy(dir, "southeast");

  switch (mode) {
    case DOOR_INTENT_OPEN:
      strcpy(action, "open");
      strcpy(action2, "opened");
      break;
    case DOOR_INTENT_CLOSE:
      strcpy(action, "close");
      strcpy(action2, "closed");
      break;
    case DOOR_INTENT_LOCK:
      strcpy(action, "lock");
      strcpy(action2, "locked");
      break;
    case DOOR_INTENT_UNLOCK:
      strcpy(action, "unlock");
      strcpy(action2, "unlocked");
      break;
    case DOOR_INTENT_LOWER:
      strcpy(action, "lower");
      strcpy(action2, "lowered");
      break;
    case DOOR_INTENT_RAISE:
      strcpy(action, "raise");
      strcpy(action2, "raised");
  }

  if (*dir) {                        /* a direction was specified */
    door = getDirFromChar(dir);
    if (door == DIR_NONE) {
      if (!silent)
        sendTo("That's not a direction.\n\r");
      return DIR_NONE;
    }
    exitp = exitDir(door);
    if (exitp) {
      if (!exitp->keyword)
        return (door);
      if (isname(type, exitp->keyword) && strcmp(type, "_unique_door_"))
        return (door);
      else {
        if (!silent)
          sendTo(format("I see no %s there.\n\r") % type);
        return DIR_NONE;
      }
    } else {
      if (!silent)
        sendTo(format("I see no %s there.\n\r") % type);
      return DIR_NONE;
    }
  } else {                        /* try to locate the keyword */
    bool found = FALSE;
    for (door = MIN_DIR; door < MAX_DIR; door++) {
      if ((exitp = exitDir(door)) && exitp->keyword &&
           strcmp(exitp->keyword, "_unique_door_")) {

        // secret doors can't be abbreviated
        if ((!IS_SET(exitp->condition, EX_SECRET) &&
            isname(type, exitp->keyword)) ||
           (IS_SET(exitp->condition, EX_SECRET) &&
            is_exact_name(type, exitp->keyword))) {
          found = TRUE;
          if ((mode == DOOR_INTENT_OPEN && IS_SET(exitp->condition, EX_CLOSED)) ||
              (mode == DOOR_INTENT_CLOSE && !IS_SET(exitp->condition, EX_CLOSED)) ||
              (mode == DOOR_INTENT_LOCK && !IS_SET(exitp->condition, EX_LOCKED)) ||
              (mode == DOOR_INTENT_UNLOCK && IS_SET(exitp->condition, EX_LOCKED)) ||
              (mode == DOOR_INTENT_LOWER && IS_SET(exitp->condition, EX_CLOSED) && 
                    (exitp->door_type == DOOR_DRAWBRIDGE)) ||
              (mode == DOOR_INTENT_LOWER && !IS_SET(exitp->condition, EX_CLOSED) && 
                    (exitp->door_type == DOOR_PORTCULLIS)) ||
              (mode == DOOR_INTENT_RAISE && !IS_SET(exitp->condition, EX_CLOSED) && 
                    (exitp->door_type == DOOR_DRAWBRIDGE)) ||
              (mode == DOOR_INTENT_RAISE && IS_SET(exitp->condition, EX_CLOSED) && 
                    (exitp->door_type == DOOR_PORTCULLIS))) {
            return (door);
          }
        }
      }
    }
    if (!silent) {
      if (!found)
        sendTo(format("I see no %s here.\n\r") % type);
      else
        sendTo(format("%s: All of them in this room are %s.\n\r") % type % action2);
    }
    if (found)
      return DIR_BOGUS;
    return DIR_NONE;
  }
}

// make sure traps have been looked for prior to calling
void TBeing::rawOpenDoor(dirTypeT dir)
{
  roomDirData *exitp, *back = NULL;
  TRoom *rp, *rp2;
  char buf[256];
  soundNumT snd = SOUND_OFF;

  if (!(rp = roomp))
    vlogf(LOG_BUG, format("NULL rp in rawOpenDoor() for %s.") %  getName());


  exitp = rp->dir_option[dir];
  if (exitp->condition & EX_DESTROYED) {
    sendTo(format("The %s has been destroyed, of course it's open.\n\r") %
       exitp->getName());
    return;
  }
  if (exitp->door_type == DOOR_PORTCULLIS && riding && getSkillValue(SKILL_RIDE) < 30) {
    sendTo("You are unable to raise that while mounted.\n\r");
    return;
  }
  if (rider) {
    sendTo("You can't seem to do that while being ridden.\n\r");
    return;
  }
  if (dir == DIR_DOWN && riding && getSkillValue(SKILL_RIDE) < 30) {
    sendTo("Seeing that you are mounted, you can't quite reach that.\n\r");
    return;
  }
  REMOVE_BIT(exitp->condition, EX_CLOSED);
  // traps have already been checked for before getting here.
  if (IS_SET(exitp->condition, EX_TRAPPED))
    REMOVE_BIT(exitp->condition, EX_TRAPPED);

  if (IS_SET(exitp->condition, EX_SECRET)) {
    act("$n reveals a hidden passage!", TRUE, this, 0, 0, TO_ROOM);
    act("You reveal a hidden passage!", TRUE, this, 0, 0, TO_CHAR);
    addToWait(combatRound(1));
  }
  switch (exitp->door_type) {
    case DOOR_DRAWBRIDGE:
      sprintf(buf, "$n causes the %s %s to lower.",
            exitp->getName().uncap().c_str(), dirs_to_blank[dir]);
      act(buf, TRUE, this, 0, 0, TO_ROOM);
      sendTo(format("You cause the %s %s to lower.\n\r") %
            exitp->getName().uncap() % dirs_to_blank[dir]);
      break;
    case DOOR_PANEL:
    case DOOR_SCREEN:
      sprintf(buf, "$n slides the %s %s to one side and opens the way.",
            exitp->getName().uncap().c_str(), dirs_to_blank[dir]);
      act(buf, TRUE, this, 0, 0, TO_ROOM);
      sendTo(format("You slide the %s %s to one side and open the way.\n\r") %
            exitp->getName().uncap() % dirs_to_blank[dir]);
      break;
    case DOOR_RUBBLE:
      sprintf(buf, "$n pushes the %s %s aside and clears the way.",
            exitp->getName().uncap().c_str(), dirs_to_blank[dir]);
      act(buf, TRUE, this, 0, 0, TO_ROOM);
      sendTo(format("You push the %s %s aside and clear the way.\n\r") %
            exitp->getName().uncap() % dirs_to_blank[dir]);
      break;
    case DOOR_PORTCULLIS:
      if (riding) {
        sprintf(buf, "$n leans over and lifts the %s %s open.",
                exitp->getName().uncap().c_str(), dirs_to_blank[dir]);
        act(buf, TRUE, this, 0, 0, TO_ROOM);
        sendTo(format("You lean over and lift the %s %s open.\n\r") %
               exitp->getName().uncap() % dirs_to_blank[dir]);
      } else {
        sprintf(buf, "$n squats down and lifts the %s %s open.",
              exitp->getName().uncap().c_str(), dirs_to_blank[dir]);
        act(buf, TRUE, this, 0, 0, TO_ROOM);
        sendTo(format("You squat down and lift the %s %s open.\n\r") %
              exitp->getName().uncap() % dirs_to_blank[dir]);
      }
      break;
    case DOOR_GRATE:
      if (dir == DIR_UP) {
        sprintf(buf, "$n reaches up and pushes the %s open.",
              exitp->getName().uncap().c_str());
        act(buf, TRUE, this, 0, 0, TO_ROOM);
        sendTo(format("You reach up and push the %s open.\n\r") %
              exitp->getName().uncap());
      } else if (dir == DIR_DOWN) {
        sprintf(buf, "$n reaches down and lifts the %s open.",
              exitp->getName().uncap().c_str());
        act(buf, TRUE, this, 0, 0, TO_ROOM);
        sendTo(format("You reach down and lift the %s open.\n\r") %
              exitp->getName().uncap());
      } else {
        sprintf(buf, "$n opens the %s %s.",
              exitp->getName().uncap().c_str(), dirs_to_blank[dir]);
        act(buf, TRUE, this, 0, 0, TO_ROOM);
        sendTo(format("You open the %s %s.\n\r") %
              exitp->getName().uncap() % dirs_to_blank[dir]);
      }
      break;
    case DOOR_GATE:
      sprintf(buf, "$n unlatches the %s %s and swings it open.",
              exitp->getName().uncap().c_str(), dirs_to_blank[dir]);
      act(buf, TRUE, this, 0, 0, TO_ROOM);
      sendTo(format("You unlatch the %s %s and swing it open.\n\r") %
              exitp->getName().uncap() % dirs_to_blank[dir]);
      break;
    case DOOR_TRAPDOOR:
      if (dir == DIR_UP) {
        sprintf(buf, "$n unlatches the %s in the ceiling and pulls it open.",
                exitp->getName().uncap().c_str());
        act(buf, TRUE, this, 0, 0, TO_ROOM);
        sendTo(format("You unlatch the %s in the ceiling and pull it open.\n\r") %
                exitp->getName().uncap());
      } else if (dir == DIR_DOWN) {
        sprintf(buf, "$n unlatches the %s in the $g and pushes it open.",
                exitp->getName().uncap().c_str());
        act(buf, TRUE, this, 0, 0, TO_ROOM);
        sendTo(format("You unlatch the %s in the %s and push it open.\n\r") %
                exitp->getName().uncap() % roomp->describeGround());
      } else {
        sprintf(buf, "$n opens the %s %s.",
              exitp->getName().uncap().c_str(), dirs_to_blank[dir]);
        act(buf, TRUE, this, 0, 0, TO_ROOM);
        sendTo(format("You open the %s %s.\n\r") %
              exitp->getName().uncap() % dirs_to_blank[dir]);
      }
      break;
    case DOOR_HATCH:
      if (dir == DIR_UP) {
        sprintf(buf, "$n reachs up and opens the %s in the ceiling.",
                exitp->getName().uncap().c_str());
        act(buf, TRUE, this, 0, 0, TO_ROOM);
        sendTo(format("You reach up and open the %s in the ceiling.\n\r") %
                exitp->getName().uncap());
      } else if (dir == DIR_DOWN) {
        sprintf(buf, "$n reaches down and opens the %s in the $g.",
                exitp->getName().uncap().c_str());
        act(buf, TRUE, this, 0, 0, TO_ROOM);
        sendTo(format("You reach down and open the %s in the %s.\n\r") %
                exitp->getName().uncap() % roomp->describeGround());
      } else {
        sprintf(buf, "$n opens the %s in the %s wall.",
              exitp->getName().uncap().c_str(), dirs[dir]);
        act(buf, TRUE, this, 0, 0, TO_ROOM);
        sendTo(format("You open the %s in the %s wall.\n\r") %
              exitp->getName().uncap() % dirs[dir]);
      }
      break;
    case DOOR_DOOR:
    default:
      snd = pickRandSound(SOUND_DOOROPEN_01, SOUND_DOOROPEN_02);
      roomp->playsound(snd, SOUND_TYPE_NOISE);

      sprintf(buf, "$n opens the %s %s.",
              exitp->getName().uncap().c_str(), dirs_to_blank[dir]);
      act(buf, TRUE, this, 0, 0, TO_ROOM);
      sendTo(format("You open the %s %s.\n\r") %
              exitp->getName().uncap() % dirs_to_blank[dir]);
  }

  // now for opening the OTHER side of the door! 
  if (exit_ok(exitp, &rp) &&
      (back = rp->dir_option[rev_dir[dir]]) &&
      (back->to_room == in_room)) {
    REMOVE_BIT(back->condition, EX_CLOSED);
    if (IS_SET(back->condition, EX_TRAPPED))
      REMOVE_BIT(back->condition, EX_TRAPPED);
    strcpy(buf, getName());
    rp2 = real_roomp(exitp->to_room);
    switch (back->door_type) {
      case DOOR_DRAWBRIDGE:
        sendrpf(rp2, "The %s %s lowers.\n\r", back->getName().uncap().c_str(), dirs_to_blank[rev_dir[dir]]);
        break;
      case DOOR_PANEL:
        sendrpf(rp2, "The %s %s slides to one side.\n\r", back->getName().uncap().c_str(), dirs_to_blank[rev_dir[dir]]);
        break;
      case DOOR_SCREEN:  // can see thru
        sendrpf(rp2, "Nearby, %s slides the %s %s to one side.\n\r", getName(), back->getName().uncap().c_str(), dirs_to_blank[rev_dir[dir]]);
        break;
      case DOOR_RUBBLE:
        sendrpf(rp2,
          "The %s %s is pushed aside from somewhere nearby.\n\r",
              back->getName().uncap().c_str(), dirs_to_blank[rev_dir[dir]]);
        break;
      case DOOR_PORTCULLIS:
        sendrpf(rp2,
          "%s struggles a bit, but manages to lift the %s %s open.\n\r",
              sstring(buf).cap().c_str(), back->getName().uncap().c_str(), dirs_to_blank[rev_dir[dir]]);
        break;
      case DOOR_GRATE:   // see thru these
        if (dir == DIR_UP) {
          sendrpf(rp2,
          "Below you, %s reaches up and opens the %s.\n\r",
              getName(), back->getName().uncap().c_str());
        } else if (dir == DIR_DOWN) {
          sendrpf(rp2,
          "Above you, %s reaches down and lifts the %s open.\n\r",
              getName(), back->getName().uncap().c_str());
        } else {
          sendrpf(rp2,
          "%s opens the %s %s from the other side.\n\r",
              sstring(buf).cap().c_str(), back->getName().uncap().c_str(), dirs_to_blank[rev_dir[dir]]);
        }
        break;
      case DOOR_GATE:
        sendrpf(rp2,
          "The %s %s is swung open from the other side.\n\r",
              back->getName().uncap().c_str(), dirs_to_blank[rev_dir[dir]]);
        break;
      case DOOR_TRAPDOOR:
        if (dir == DIR_UP) {
          sendrpf(rp2,
          "The %s in the %s is unlatched and opened from the other side.\n\r",
              back->getName().uncap().c_str(), rp->describeGround().c_str());          
        } else if (dir == DIR_DOWN) {
          sendrpf(rp2,
        "The %s in the ceiling is unlatched and opened from the other side.\n\r",
              back->getName().uncap().c_str());
        } else {
          sendrpf(rp2,
          "The %s %s is opened from the other side.\n\r",
              back->getName().uncap().c_str(), dirs_to_blank[rev_dir[dir]]);
        }
        break;
      case DOOR_HATCH:
        sendrpf(rp2,
          "The %s %s is swung open from the other side.\n\r",
              back->getName().uncap().c_str(), dirs_to_blank[rev_dir[dir]]);
        break;
      case DOOR_DOOR:
      default:
        sendrpf(rp2,
              "The %s %s is opened from the other side.\n\r",
              back->getName().uncap().c_str(), dirs_to_blank[rev_dir[dir]]);
        rp2->playsound(snd, SOUND_TYPE_NOISE);

    }
    TThing *t=NULL;
    for(StuffIter it=roomp->stuff.begin();it!=roomp->stuff.end() && (t=*it);++it) {
      if (t->desc)
        t->desc->send_client_exits();
    }
    for(StuffIter it=rp2->stuff.begin();it!=rp2->stuff.end() && (t=*it);++it) {
      if (t->desc)
        t->desc->send_client_exits();
    }
  }
}

void TBeing::rawCloseDoor(dirTypeT dir)
{
  roomDirData *exitp, *back = NULL;
  TRoom *rp, *rp2;
  char buf[256];
  soundNumT snd = SOUND_OFF;
 
  if (!(rp = roomp))
    vlogf(LOG_BUG, format("NULL rp in rawCloseDoor() for %s.") %  getName());
 
  exitp = rp->dir_option[dir];
  if (IS_SET(exitp->condition, EX_DESTROYED)) {
    sendTo(format("The %s has been destroyed, it can't be closed.\n\r") %
       exitp->getName());
    return;
  }
  if (dir == DIR_DOWN && riding) {
    sendTo("Seeing that you are mounted, you can't quite reach that.\n\r");
    return;
  }
  SET_BIT(exitp->condition, EX_CLOSED);

  switch (exitp->door_type) {
    case DOOR_DRAWBRIDGE:
      sprintf(buf, "$n raises the %s %s.",
            exitp->getName().uncap().c_str(), dirs_to_blank[dir]);
      act(buf, TRUE, this, 0, 0, TO_ROOM);
      sendTo(format("You raise the %s %s.\n\r") %
            exitp->getName().uncap() % dirs_to_blank[dir]);
      break;
    case DOOR_PANEL:
    case DOOR_SCREEN:
      sprintf(buf, "$n slides the %s %s closed.", 
            exitp->getName().uncap().c_str(), dirs_to_blank[dir]);
      act(buf, TRUE, this, 0, 0, TO_ROOM);
      sendTo(format("You slide the %s %s closed.\n\r") %
            exitp->getName().uncap() % dirs_to_blank[dir]);
      break;
    case DOOR_RUBBLE:
      sprintf(buf, "$n pushes the %s %s into the path and blocks the way.", 
            exitp->getName().uncap().c_str(), dirs_to_blank[dir]);
      act(buf, TRUE, this, 0, 0, TO_ROOM);
      sendTo(format("You push the %s %s into the path and block the way.\n\r") %
            exitp->getName().uncap() % dirs_to_blank[dir]);
      break;
    case DOOR_PORTCULLIS:
      sprintf(buf, "$n reaches up and lowers the %s %s.",
            exitp->getName().uncap().c_str(), dirs_to_blank[dir]);
      act(buf, TRUE, this, 0, 0, TO_ROOM);
      sendTo(format("You reach up and lower the %s %s.\n\r") %
            exitp->getName().uncap() % dirs_to_blank[dir]);
      break;
    case DOOR_GRATE:
      if (dir == DIR_UP) {
        sprintf(buf, "$n reaches up and pushes the %s closed.",
              exitp->getName().uncap().c_str());
        act(buf, TRUE, this, 0, 0, TO_ROOM);
        sendTo(format("You reach up and push the %s close.\n\r") %
              exitp->getName().uncap());
      } else if (dir == DIR_DOWN) {
        sprintf(buf, "$n reaches down and closes the %s.",
              exitp->getName().uncap().c_str());
        act(buf, TRUE, this, 0, 0, TO_ROOM);
        sendTo(format("You reach down and close the %s.\n\r") %
              exitp->getName().uncap());
      } else {
        sprintf(buf, "$n closes the %s %s.",
              exitp->getName().uncap().c_str(), dirs_to_blank[dir]);
        act(buf, TRUE, this, 0, 0, TO_ROOM);
        sendTo(format("You close the %s %s.\n\r") %
              exitp->getName().uncap() % dirs_to_blank[dir]);
      }
      break;
    case DOOR_GATE:
      sprintf(buf, "$n swings the %s %s closed and latches it.",
              exitp->getName().uncap().c_str(), dirs_to_blank[dir]);
      act(buf, TRUE, this, 0, 0, TO_ROOM);
      sendTo(format("You swing the %s %s closed and latch it.\n\r") %
              exitp->getName().uncap() % dirs_to_blank[dir]);
      break;
    case DOOR_TRAPDOOR:
      if (dir == DIR_UP) {
        sprintf(buf, "$n pushes the %s in the ceiling closed and latch it.",
                exitp->getName().uncap().c_str());
        act(buf, TRUE, this, 0, 0, TO_ROOM);
        sendTo(format("You push the %s in the ceiling closed and latch it.\n\r") %
                exitp->getName().uncap());
      } else if (dir == DIR_DOWN) {
        sprintf(buf, "$n pull the %s in the $g closed and latch it.",
                  exitp->getName().uncap().c_str());
        act(buf, TRUE, this, 0, 0, TO_ROOM);
        sendTo(format("You close the %s in the %s and latch it.\n\r") %
                exitp->getName().uncap() % roomp->describeGround());
      } else {
        sprintf(buf, "$n closes the %s %s.",
              exitp->getName().uncap().c_str(), dirs_to_blank[dir]);
        act(buf, TRUE, this, 0, 0, TO_ROOM);
        sendTo(format("You close the %s %s.\n\r") %
              exitp->getName().uncap() % dirs_to_blank[dir]);
      }
      break;
    case DOOR_HATCH:
      if (dir == DIR_UP) {
        sprintf(buf, "$n reaches up and pushes the %s closed.",
              exitp->getName().uncap().c_str());
        act(buf, TRUE, this, 0, 0, TO_ROOM);
        sendTo(format("You reach up and push the %s closed.\n\r") %
              exitp->getName().uncap());
      } else if (dir == DIR_DOWN) {
        sprintf(buf, "$n reaches down and closes the %s.",
              exitp->getName().uncap().c_str());
        act(buf, TRUE, this, 0, 0, TO_ROOM);
        sendTo(format("You reach down and close the %s.\n\r") %
              exitp->getName().uncap());
      } else {
        sprintf(buf, "$n closes the %s %s.",
              exitp->getName().uncap().c_str(), dirs_to_blank[dir]);
        act(buf, TRUE, this, 0, 0, TO_ROOM);
        sendTo(format("You close the %s %s.\n\r") %
              exitp->getName().uncap() % dirs_to_blank[dir]);
      }
      break;
    case DOOR_DOOR:
    default:
      sprintf(buf, "$n closes the %s %s.", exitp->getName().uncap().c_str(), dirs_to_blank[dir]);
      act(buf, TRUE, this, 0, 0, TO_ROOM);
      sendTo(format("You close the %s %s.\n\r") %exitp->getName().uncap() % dirs_to_blank[dir]);

      snd = pickRandSound(SOUND_DOORCLOSE_01, SOUND_DOORCLOSE_04);
      roomp->playsound(snd, SOUND_TYPE_NOISE);
  }
  if (IS_SET(exitp->condition, EX_SECRET)) {
    act("$n conceals a hidden passage!", TRUE, this, 0, 0, TO_ROOM);
    act("You conceal a hidden passage!", TRUE, this, 0, 0, TO_CHAR);
    addToWait(combatRound(1));
  }
 
  /* now for closing the OTHER side of the door! */
  if (exit_ok(exitp, &rp) &&
      (back = rp->dir_option[rev_dir[dir]]) &&
      (back->to_room == in_room)) {
    SET_BIT(back->condition, EX_CLOSED);
    strcpy(buf, getName());
    rp2 = real_roomp(exitp->to_room);
    switch (back->door_type) {
      case DOOR_DRAWBRIDGE:
        sendrpf(rp2,
          "The %s %s raises.\n\r",
              back->getName().uncap().c_str(), dirs_to_blank[rev_dir[dir]]);
        break;
      case DOOR_SCREEN:
        sendrpf(rp2,
          "Nearby, %s slides the %s %s closed.\n\r",
              getName(), back->getName().uncap().c_str(), dirs_to_blank[rev_dir[dir]]);
        break;
      case DOOR_PANEL:
        sendrpf(rp2,
          "The %s %s slides closed.\n\r",
              back->getName().uncap().c_str(), dirs_to_blank[rev_dir[dir]]);
        break;
      case DOOR_RUBBLE:
        sendrpf(rp2,
          "The %s %s is pushed into the path from somewhere nearby.\n\r",
              back->getName().uncap().c_str(), dirs_to_blank[rev_dir[dir]]);
        break;
      case DOOR_PORTCULLIS:
        sendrpf(rp2,
          "%s lowers the %s %s, closing it.\n\r",
              sstring(buf).cap().c_str(), back->getName().uncap().c_str(), dirs_to_blank[rev_dir[dir]]);
        break;
      case DOOR_GRATE:   // see thru these
        if (dir == DIR_UP) {
          sendrpf(rp2,
          "Below you, %s reaches up and closes the %s.\n\r",
              getName(), back->getName().uncap().c_str());
        } else if (dir == DIR_DOWN) {
          sendrpf(rp2,
          "Above you, %s reaches down and lowers the %s closed.\n\r",
              getName(), back->getName().uncap().c_str());
        } else {
          sendrpf(rp2,
          "%s closes the %s %s from the other side.\n\r",
              sstring(buf).cap().c_str(), back->getName().uncap().c_str(), dirs_to_blank[rev_dir[dir]]);
        }
        break;
      case DOOR_GATE:
        sendrpf(rp2,
         "The %s %s is swung closed from the other side.\n\r",
              back->getName().uncap().c_str(), dirs_to_blank[rev_dir[dir]]);
        break;
      case DOOR_TRAPDOOR:
        if (dir == DIR_UP) {
          sendrpf(rp2,
          "The %s in the %s is closed and latched from the other side.\n\r",
              back->getName().uncap().c_str(), rp2->describeGround().c_str());
        } else if (dir == DIR_DOWN) {
          sendrpf(rp2,
        "The %s in the ceiling is closed and latched from the other side.\n\r",
              back->getName().uncap().c_str());
        } else {
          sendrpf(rp2,
          "The %s %s is closed from the other side.\n\r",
              back->getName().uncap().c_str(), dirs_to_blank[rev_dir[dir]]);
        }
        break;
      case DOOR_HATCH:   
        if (dir == DIR_UP) {
          sendrpf(rp2,
          "Below you, %s reaches up and closes the %s.\n\r",
              getName(), back->getName().uncap().c_str());
        } else if (dir == DIR_DOWN) {
          sendrpf(rp2,
          "%s is closed from above by %s.\n\r",
              back->getName().uncap().c_str(), getName());
        } else {
          sendrpf(rp2,
          "%s closes the %s %s from the other side.\n\r",
              sstring(buf).cap().c_str(), back->getName().uncap().c_str(), dirs_to_blank[rev_dir[dir]]);
        }
        break;
      case DOOR_DOOR:
      default:
        sendrpf(rp2,
              "The %s %s is closed from the other side.\n\r",
              back->getName().uncap().c_str(), dirs_to_blank[rev_dir[dir]]);
        rp2->playsound(snd, SOUND_TYPE_NOISE);
    }
    TThing *t=NULL;
    for(StuffIter it=roomp->stuff.begin();it!=roomp->stuff.end() && (t=*it);++it) {
      if (t->desc)
        t->desc->send_client_exits();
    }
    for(StuffIter it=rp2->stuff.begin();it!=rp2->stuff.end() && (t=*it);++it) {
      if (t->desc)
        t->desc->send_client_exits();
    }
  }
}

/*
  Commentary:

  dir is the exit direction from the room that the command was operated
  intent=DOOR_UNIQUE_DEF : the same command both opens and closes the exit
      was_closed and was_open should be ""
  intent=DOOR_UNIQUE_OPEN_ONLY : command is only for opening
      if door is open when command done, they get was_open message
      was_closed, and all close_* messages should be blank
  intent=DOOR_UNIQUE_OPEN_ONLY_FORCE : command is only for opening
      if door open when done, open message sent anyway
      was_closed, and all close_* messages should be blank
  intent=DOOR_UNIQUE_CLOSE_ONLY : command is only for closeing
      if door is close when command done, they get was_close message
      was_open, and all open_* messages should be blank
  intent=DOOR_UNIQUE_CLOSE_ONLY_FORCE : command is only for closeing
      if door close when done, close message sent anyway
      was_open, and all open_* messages should be blank

  the was_* messages are only sent to the doer (think of as error messages)
  the *_far messages are sent on successes to the OTHER side of the door
  Also, the *_far messages need trailing "\n\r" because of sendrp
*/
void TBeing::openUniqueDoor(dirTypeT dir, doorUniqueT intent, 
      const char * was_closed,
      const char * was_open,
      const char * open_char,
      const char * open_room,
      const char * open_far,
      const char * close_char,
      const char * close_room,
      const char * close_far
     )
{
  roomDirData *exitp, *back = NULL;
  TRoom *rp;

  enum successResT {
    SUCCESS_OPEN,
    SUCCESS_CLOSE,
    SUCCESS_WAS_OPEN,
    SUCCESS_WAS_CLOSE
  };
  successResT open = SUCCESS_OPEN;
 
  if (!(rp = roomp)) {
    vlogf(LOG_BUG, format("NULL rp in openUniqueDoor() for %s.") %  getName());
    return;
  }

  if (!(exitp = rp->dir_option[dir])) {
    vlogf(LOG_BUG, format("Bogus exit in openUniqueDoor() for %s (%d).") %  getName() % dir);
    return;
  }

// Find out if it matters if the pc is trying to open or close a unique
// Only important if push opens and pulls closes.  Unique door function in 
// spec_rooms will set the intention or use 0 if it doesnt matter - Cos 5/97

  switch (intent) {
    case DOOR_UNIQUE_OPEN_ONLY:
      if (IS_SET(exitp->condition, EX_CLOSED)) {
        REMOVE_BIT(exitp->condition, EX_CLOSED);
        open = SUCCESS_OPEN;
      } else {
        open = SUCCESS_WAS_OPEN;
      }
      break;
    case DOOR_UNIQUE_CLOSE_ONLY:
      if (!IS_SET(exitp->condition, EX_CLOSED)) {
        SET_BIT(exitp->condition, EX_CLOSED);
        open = SUCCESS_CLOSE;
      } else {
        open = SUCCESS_WAS_CLOSE;
      }
      break;
    case DOOR_UNIQUE_DEF:
      if (IS_SET(exitp->condition, EX_CLOSED)) {
        REMOVE_BIT(exitp->condition, EX_CLOSED);
        open = SUCCESS_OPEN;
      } else {
        SET_BIT(exitp->condition, EX_CLOSED);
        open = SUCCESS_CLOSE;
      }
      break;
    case DOOR_UNIQUE_CLOSE_ONLY_FORCE:
      // works like close-only, but if door is already closed, still returns
      // the "successful close" message.
      if (!IS_SET(exitp->condition, EX_CLOSED)) {
        SET_BIT(exitp->condition, EX_CLOSED);
        open = SUCCESS_CLOSE;
      } else {
        open = SUCCESS_CLOSE;
      }
      break;
    case DOOR_UNIQUE_OPEN_ONLY_FORCE:
      // works like open only, but if door already open, still returns
      // the successfully opened message
      if (IS_SET(exitp->condition, EX_CLOSED)) {
        REMOVE_BIT(exitp->condition, EX_CLOSED);
        open = SUCCESS_OPEN;
      } else {
        open = SUCCESS_OPEN;
      }
      break;
  }

  if (open == SUCCESS_WAS_CLOSE) {
    // Already closed
    act(was_closed, TRUE, this, 0, 0, TO_CHAR);
    return;
  } else if (open == SUCCESS_WAS_OPEN) {
    // Already open
    act(was_open, TRUE, this, 0, 0, TO_CHAR);
    return;
  } else if (open == SUCCESS_OPEN) {
    act(open_char, TRUE, this, 0, 0, TO_CHAR);
    act(open_room, TRUE, this, 0, 0, TO_ROOM);
  } else if (open == SUCCESS_CLOSE) {
    act(close_char, TRUE, this, 0, 0, TO_CHAR);
    act(close_room, TRUE, this, 0, 0, TO_ROOM);
  }
 /* now for opening the OTHER side of the door! */
  if (exit_ok(exitp, &rp) &&
      (back = rp->dir_option[rev_dir[dir]]) &&
      (back->to_room == in_room)) {
    if (open == SUCCESS_OPEN)
      REMOVE_BIT(back->condition, EX_CLOSED);
    else if (open == SUCCESS_CLOSE)
      SET_BIT(back->condition, EX_CLOSED);

    // rp is now the OTHER roomp
    if (open == SUCCESS_OPEN) {
      // base these numbers on where the action was ORIGINATED
      sendrpf(rp, open_far);
      sendrpf(rp, "\n\r");
    } else if (open == SUCCESS_CLOSE) {
      sendrpf(rp, close_far);
      sendrpf(rp, "\n\r");
    }
  }
}

bool canSeeThruDoor(const roomDirData *exitp)
{
  if (IS_SET(exitp->condition, EX_CAVED_IN))
    return FALSE;

  if (IS_SET(exitp->condition, EX_DESTROYED))
    return TRUE;
  else if (!IS_SET(exitp->condition, EX_CLOSED))
   return TRUE;

  switch (exitp->door_type) {
    case DOOR_NONE:
    case DOOR_PORTCULLIS:
    case DOOR_GRATE:
    case DOOR_SCREEN:
      return TRUE;
    default:
      return FALSE;
  }
}

void roomDirData::destroyDoor(dirTypeT dir, int room)
{
  roomDirData *back = NULL;
  TRoom *rp;

  if (door_type == DOOR_NONE)
    return;
  
  condition = EX_DESTROYED;   // removes closed/locked too

  rp = real_roomp(to_room);
  if (exit_ok(this, &rp) &&
      (back = rp->dir_option[rev_dir[dir]]) &&
      (back->to_room == room)) {
    back->condition = EX_DESTROYED;
    sendrpf(rp, "The %s is destroyed from the other side.\n\r", getName().c_str());
  }
  return;
}

void roomDirData::caveinDoor(dirTypeT dir, int room)
{
  roomDirData *back = NULL;
  TRoom *rp;
 
  // this should destroy any door, closing the way

  SET_BIT(condition, EX_CAVED_IN);
  SET_BIT(condition, EX_CLOSED);
  door_type = DOOR_NONE;
 
  rp = real_roomp(to_room);
  if (exit_ok(this, &rp) &&
      (back = rp->dir_option[rev_dir[dir]]) &&
      (back->to_room == room)) {
    SET_BIT(back->condition, EX_CAVED_IN);
    SET_BIT(back->condition, EX_CLOSED);
    back->door_type = DOOR_NONE;
    sendrpf(rp, "A massive cave in blocks the way %s.\n\r", dirs[rev_dir[dir]]);
  }
  return;
}

void roomDirData::wardDoor(dirTypeT dir, int room)
{
  roomDirData *back = NULL;
  TRoom *rp;
 
  // this should pop a ward onto the exit
 
  SET_BIT(condition, EX_WARDED);
 
  rp = real_roomp(to_room);
  if (exit_ok(this, &rp) &&
      (back = rp->dir_option[rev_dir[dir]]) &&
      (back->to_room == room)) {
    SET_BIT(back->condition, EX_WARDED);
    sendrpf(rp, "You hear a soft _woompf_ as a magical ward is placed across the %s exit.\n\r", dirs[rev_dir[dir]]);
  }
  return;
}

// this is a room-special proc
int SecretDoors(TBeing *ch, cmdTypeT cmd, const char *arg, TRoom *rp)
{

  if (!rp) {
    mud_assert(rp != NULL, "No room in SecretDoors");
    return FALSE;
  }

  if (!ch) {
    // some triggers (pulse) will pass in !ch, just ignore them
    return FALSE;
  }

  if (cmd >= MAX_CMD_LIST)
    return FALSE;

  if (rp->number != ch->in_room) {
    vlogf(LOG_BUG,format("char %s not in proper room (SecretDoors)") % ch->getName());
    return FALSE;
  }

  char buf[255];

  one_argument(arg,buf,cElements(buf));
  switch (rp->number) {
    case 450:
      if ((cmd != CMD_CLOSE) && (cmd != CMD_PUSH))
        return FALSE;
      if (cmd == CMD_PUSH) {
        if (!strcasecmp(buf, "nail")) {
          ch->openUniqueDoor(DIR_SOUTH,DOOR_UNIQUE_OPEN_ONLY,
            "",
            "Nothing seems to happen as you push the nail.",
            "As you push the nail a panel of the south wall pops open with a *click*.",
            "As $n pushes a nail a panel of the south wall pops open with a *click*.",
            "The painting on the north wall suddenly opens with a *click*.",
            "",
            "",
            ""
          );
          return TRUE;
        }
      } else {
        if (isname(buf, "panel door")) {
          ch->openUniqueDoor(DIR_SOUTH,DOOR_UNIQUE_CLOSE_ONLY,
            "The panel is already closed.",
            "",
            "",
            "",
            "",
            "You close the panel which latches with a *click*.",
            "$n closes the panel in the south wall which latches with a *click*.",
            "The painting to the north closes with a *click*."
          );
          return TRUE;
        }
      }
      break;
    case 451:
      if ((cmd != CMD_CLOSE) && (cmd != CMD_PUSH))
        return FALSE;
      if (cmd == CMD_PUSH) {
        if (isname(buf, "gem ruby")) {
          ch->openUniqueDoor(DIR_NORTH,DOOR_UNIQUE_OPEN_ONLY,
            "",
            "Nothing seems to happen as you push the gem.",
            "As you push the gem the painting pops open with a *click*.",
            "As $n fiddles with the frame of a painting it pops open with a *click*.",
            "The panel to the south suddenly pops open with a *click*.",
            "",
            "",
            ""
          );
          return TRUE;
        }
      } else {
        if (!strcasecmp(buf, "painting")) {
          ch->openUniqueDoor(DIR_NORTH,DOOR_UNIQUE_CLOSE_ONLY,
            "The painting seems securely \"closed\".",
            "",
            "",
            "",
            "",
            "You close the painting which latches with a *click*.",
            "$n closes the painting on the north wall which latches with a *click*.",
            "The panel to the south closes with a *click*."
          );
          return TRUE;
        }
      }
      break;
  case 774:
      if (cmd != CMD_TWIST)
        return FALSE;
      if (!strcasecmp(buf, "lid")) {
        ch->openUniqueDoor(DIR_UP, DOOR_UNIQUE_DEF,
          "",
          "",
        "As you twist the lid, a panel of the ceiling slowly opens.",
        "$n twists the barrel lid causing a panel of the ceiling to slowly open.",
          "A panel in the floor slowly slides open.",
        "As you twist the lid, the trapdoor overhead slowly slides closed.",
        "$n twists the barrel lid causing the trapdoor overhead to slowly close.",
          "The trapdoor below you slowly slides closed."
        );
        return TRUE;
      }
      break;
    case 6156:
      if ((cmd != CMD_RAISE) && (cmd != CMD_LOWER) && (cmd != CMD_LIFT))
        return FALSE;
      if ((cmd == CMD_RAISE) || (cmd == CMD_LIFT)) {
        if (!strcasecmp(buf, "lever")) {
          ch->openUniqueDoor(DIR_NORTH, DOOR_UNIQUE_OPEN_ONLY, 
              "",
              "The lever is already raised.",
              "You squat down and raise the small lever concealed in the $g.  With a grinding of gears, a large section of the north wall breaks free and slides to one side.",
              "$n squats down near the north wall and fiddles with something on the $g.  With a grinding of gears, a large section of the north wall breaks free and slides to one side.",
              "You hear a grinding sound as a section of the north wall slides open revealing a passage.",
              "",
              "",
              ""
          );
          return TRUE;
        }
      } else {
        if (!strcasecmp(buf, "lever")) {
          ch->openUniqueDoor(DIR_NORTH, DOOR_UNIQUE_CLOSE_ONLY,
              "The lever is already lowered.",
              "",
              "",
              "",
              "",
              "You squat down and lower the small lever until it is concealed by the $g.  With a grinding of gears, the displaced section of wall to the north slides smoothly back in place leaving no trace of the way through.",
              "$n squats down near the north wall and fiddles with something on the $g.  With a grinding of gears, the displaced section of wall to the north slides smoothly back in place leaving no trace of the way through.",
              "You hear a grinding sound as a section of the south wall slides closed concealing the exit."
          );
          return TRUE;
        }
      } 
      break;
    case 6158:
      if ((cmd != CMD_RAISE) && (cmd != CMD_LOWER) && (cmd != CMD_LIFT))
        return FALSE;
      if ((cmd == CMD_RAISE) || (cmd == CMD_LIFT)) {
        if (!strcasecmp(buf, "lever")) {
          ch->openUniqueDoor(DIR_SOUTH, DOOR_UNIQUE_OPEN_ONLY,
              "",
              "The lever is already raised.",
              "You squat down and raise the small lever concealed in the $g.  A loud grinding sound errupts from the mechanism of gears, chains, and weights to the south as a section of the south wall breaks free and slides to one side.",
              "$n squats down near the north wall and fiddles with something on the $g.  A loud grinding sound errupts from the mechanism of gears, chains, and weights to the south as a section of the south wall breaks free and slides to one side.",
              "You hear a grinding sound from the mechanism to your north as a section of the south wall slides open revealing a passage.",
              "",
              "",
              ""
          );
          return TRUE;
        }
      } else {
        if (!strcasecmp(buf, "lever")) {
          ch->openUniqueDoor(DIR_SOUTH, DOOR_UNIQUE_CLOSE_ONLY,
             "The lever is already lowered.",
              "",
              "",
              "",
              "",
              "You squat down and lower the small lever until it is concealed by the $g.  With a loud grinding, the mechanism of gears and weights come to life as the displaced section of the south wall slides back into place concealing all traces of the exit.",
              "$n squats down near the south wall and fiddles with something on the $g.  With a loud grinding, the mechanism of gears and weights come to life as the displaced section of the south wall slides back into place concealing all traces of the exit.",
              "You hear a grinding sound from the mechanism to your north as a section of the south wall slides back into place."
          );
          return TRUE;
        }
      }
      break;
    case 7005:
      if (cmd != CMD_PUSH)
        return FALSE;
      if (!strcasecmp(buf, "trigger")) {
        ch->openUniqueDoor(DIR_DOWN, DOOR_UNIQUE_DEF,
              "",
              "",
	  "As you push the trigger, the ladder lowers into the pit.",
	  "As $n pushes the trigger, the ladder lowers into the pit.",
              "A ladder lowers into the pit.",
	  "As you push the trigger, the ladder rises out of the pit.",
	  "As $n pushes the trigger, the ladder rises out of the pit.",
              "The ladder raises out of the pit."
        );
        return TRUE;
      }
      break;
    case 7015:
      if (cmd != CMD_TWIST)
        return FALSE;
      if (!strcasecmp(buf, "lamp")) {
        ch->openUniqueDoor(DIR_NORTHEAST, DOOR_UNIQUE_DEF,
          "",
          "",
        "You twist the lamp, the wall slides back revealing a passage.",
        "$n twists the lamp, the wall slides back revealing a passage.",
          "A passage is revealed.",
        "You twist the lamp, the wall slides forward concealing a passage.",
        "$n twists the lamp, the wall slides forward concealing a passage.",
          "A passage is concealed."
        );
        return TRUE;
      }
      break;
    case 7016:
      if (cmd != CMD_PUSH)
        return FALSE;
      if (!strcasecmp(buf, "trigger")) {
        ch->openUniqueDoor(DIR_UP, DOOR_UNIQUE_DEF,
             "",
             "",
	  "As you push the trigger, the ladder lowers into the pit.",
	  "As $n pushes the trigger, the ladder lowers into the pit.",
              "A ladder lowers into the pit.",
	  "As you push the trigger, the ladder rises out of the pit.",
	  "As $n pushes the trigger, the ladder rises out of the pit.",
              "The ladder raises out of the pit."
        );
        return TRUE;
      } 
      break;
    case 7023:
      if ((cmd != CMD_PULL) && (cmd != CMD_SHOVE))
        return FALSE;
      if (!strcasecmp(buf, "lever")) {
        ch->openUniqueDoor(DIR_SOUTHWEST, DOOR_UNIQUE_DEF,
            "",
            "",
        "You pull the lever and a wall opens up, revealing a passage.",
        "$n pulls the lever and a wall opens up, revealing a passage.",
            "The wall slides up, revealing a passage.",
        "You pull the lever and a wall opens down, concealing a passage.",
        "$n pulls the lever and a wall opens down, concealing a passage.",
            "The wall slides down, concealing a passage."
        );
        return TRUE;
      }
      break;
  case 9050:
      if ((cmd != CMD_TWIST) && (cmd != CMD_TURN))
        return FALSE;
      if (cmd == CMD_TWIST) {
      if (!strcasecmp(buf, "lid")) {
        ch->openUniqueDoor(DIR_DOWN, DOOR_UNIQUE_DEF,
          "",
          "",
        "As you twist the lid, a panel in the floor slides open beneath your feet.",
        "$n twists the barrel lid causing a panel of floor to slowly slide open.",
          "A panel in the ceiling overhead slowly slides open.",
        "As you twist the lid, the trapdoor in the floor slowly slides closed.",
        "$n twists the barrel lid causing the trapdoor in the floor to slowly close.",
          "The trapdoor overhead slowly slides closed."
        );
        return TRUE;
      }
    } else {
      if (!strcasecmp(buf, "bracket")) {
        ch->openUniqueDoor(DIR_WEST, DOOR_UNIQUE_DEF,
              "",
              "",
            "You reach out and turn the bracket.  A passage to the west is revealed!",
            "$n fiddles with something.  A passage to the west is revealed!",
            "A passage to the east is revealed.",
            "You reach out and turn the bracket.  The passage to the west is concealed!",
            "$n fiddles with something.  The passage to the west is concealed!",
            "The passage to the east is concealed."
        );
        return TRUE;
      }  
    }
    break;
  case 9064:
      if (cmd != CMD_TURN)
        return FALSE;
      if (!strcasecmp(buf, "bracket")) {
        ch->openUniqueDoor(DIR_EAST, DOOR_UNIQUE_DEF,
              "",
              "",
            "You reach out and turn the bracket.  A passage to the east is revealed!",
            "$n fiddles with something.  A passage to the east is revealed!",
            "A passage to the west is revealed.",
            "You reach out and turn the bracket.  The passage to the east is concealed!",
            "$n fiddles with something.  The passage to the east is concealed!",
            "The passage to the west is concealed."
        );
        return TRUE;
      }
      break;
    case 9390:
      if ((cmd != CMD_PULL) && (cmd != CMD_PUSH) && (cmd != CMD_SHOVE))
        return FALSE;
      if (!strcasecmp(buf, "brush")) {
        ch->openUniqueDoor(DIR_NORTH, DOOR_UNIQUE_DEF,
            "",
            "",
        "You move the brush aside.",
        "$n moves some brush aside.",
            "Some brush has been moved aside, revealing a new path.",
        "You move the brush.",
        "$n moves some brush.",
            "Some brush has been moved, concealing a path."
        );
        return TRUE;
      }
      break;
    case 9391:
      if ((cmd != CMD_PULL) && (cmd != CMD_PUSH) && (cmd != CMD_SHOVE))
        return FALSE;
      if (!strcasecmp(buf, "brush")) {
        ch->openUniqueDoor(DIR_SOUTH, DOOR_UNIQUE_DEF,
            "",
            "",
        "You move the brush aside.",
        "$n moves some brush aside.",
            "Some brush has been moved aside, revealing a new path.",
        "You move the brush.",
        "$n moves some brush.",
            "Some brush has been moved, concealing a path."
        );
        return TRUE;
      }
      break;
    case 9581:
      if ((cmd != CMD_PULL) && (cmd != CMD_PRESS))
        return FALSE;
      if (!strcasecmp(buf, "tape")) {
        ch->openUniqueDoor(DIR_NORTH, DOOR_UNIQUE_DEF,
            "",
            "",
        "You peel the tape back, opening a rip in the back of the tent.",
        "$n fiddles with something, opening a rip in the back of the tent.",
            "A rip in the back of the tent has been opened.",
        "You press the tape back over the rip, closing the breach.",
        "$n fiddles with something, sealing the rip in the back of the tent.",
            "A rip in the back of the tent has been closed."
        );
        return TRUE;
      }
      break;
    case 9582:
      if ((cmd != CMD_PULL) && (cmd != CMD_PRESS))
        return FALSE;
      if (!strcasecmp(buf, "tape")) {
        ch->openUniqueDoor(DIR_SOUTH, DOOR_UNIQUE_DEF,
            "",
            "",
        "You peel the tape back, opening a rip in the back of the tent.",
        "$n fiddles with something, opening a rip in the back of the tent.",
            "A rip in the back of the tent has been opened.",
        "You press the tape back over the rip, closing the breach.",
        "$n fiddles with something, sealing the rip in the back of the tent.",
            "A rip in the back of the tent has been closed."
        );
        return TRUE;
      }
      break;
    case 13111:
      if (cmd != CMD_PUSH)
	return FALSE;
      if(!strcasecmp(buf, "kelp")) {
	ch->openUniqueDoor(DIR_DOWN, DOOR_UNIQUE_DEF,
			   "",
			   "",
			   "You push aside the kelp and try to clear a path.",
			   "$n pushes aside some kelp, trying to clear a path.",
			   "A path through the kelp has been opened.",
			   "You push the kelp strands back into place.",
			   "$n pushes some kelp strands back into place.",
			   "A path through the help has been closed."
			   );
	return TRUE;
      }
      break;
    case 9626:
      if (cmd != CMD_PUSH)
	return FALSE;
      if(!strcasecmp(buf, "kelp")) {
	ch->openUniqueDoor(DIR_UP, DOOR_UNIQUE_DEF,
			   "",
			   "",
			   "You push aside the kelp and try to clear a path.",
			   "$n pushes aside some kelp, trying to clear a path.",
			   "A path through the kelp has been opened.",
			   "You push the kelp strands back into place.",
			   "$n pushes some kelp strands back into place.",
			   "A path through the help has been closed."
			   );
	return TRUE;
      }
      break;
    case 9625:
      if(cmd!=CMD_PUSH)
	return FALSE;
      if(!strcasecmp(buf, "kelp")) {
	ch->openUniqueDoor(DIR_DOWN, DOOR_UNIQUE_DEF,
			   "",
			   "",
			   "You push aside the kelp and try to clear a path.",
			   "$n pushes aside some kelp, trying to clear a path.",
			   "A path through the kelp has been opened.",
			   "You push the kelp strands back into place.",
			   "$n pushes some kelp strands back into place.",
			   "A path through the help has been closed."
			   );
	ch->openUniqueDoor(DIR_UP, DOOR_UNIQUE_DEF,
			   "",
			   "",
			   "",
			   "",
			   "",
			   "",
			   "",
			   ""
			   );
	return TRUE;
      }
      break;
    case 10111:
      if ((cmd != CMD_PULL) && (cmd != CMD_PRESS) && (cmd != CMD_PUSH))
        return FALSE;
      if (!strcasecmp(buf, "branch")) {
        act("You press on the branch.",
            TRUE,ch,0,0,TO_CHAR);
        act("$n fiddles with something.",
            TRUE,ch,0,0,TO_ROOM);
        ch->openUniqueDoor(DIR_UP, DOOR_UNIQUE_DEF,
            "",
            "",
            "A strange rumbling is heard and a stairway descends from the foliage.",
            "$n's action causes a small stairway to descend from the foliage!",
            "Something causes the concealed stairway to drop down into the forest below.",
            "A strange rumbling is heard and the stairway ascends into the foliage.",
            "$n's action causes a stairway to ascend back up into the foliage.",
            "The concealed stairway ascends into the foliage nearby."
        );
        return TRUE;
      }
      break;
    case 10144:
      if ((cmd != CMD_PULL) && (cmd != CMD_PRESS) && (cmd != CMD_PUSH))
        return FALSE;
      if (!strcasecmp(buf, "branch")) {
        act("You press on the branch.",
            TRUE,ch,0,0,TO_CHAR);
        act("$n fiddles with something.",
            TRUE,ch,0,0,TO_ROOM);
        ch->openUniqueDoor(DIR_DOWN, DOOR_UNIQUE_DEF,
            "",
            "",
            "A strange rumbling is heard and a stairway descends toward the forest below.",
            "$n's action causes the small stairway to descend into the forest below.", 
            "A rumbling noise is heard and a stairway descends out of the foliage nearby.",
            "Your action makes the concealed stairway ascend back into the foliage nearby.",
            "$n's action causes a stairway to ascend back up into the foliage nearby.",
            "A rumbling noise is heard and the stairway disappears into the dense foliage overhead for unknown reasons."
        );
        return TRUE;
      }
      break;
    case 10721:
      if (cmd != CMD_TURN)
        return FALSE;
      if (!strcasecmp(buf, "bracket")) {
        ch->openUniqueDoor(DIR_SOUTH, DOOR_UNIQUE_DEF,
            "",
            "",
            "You turn the bracket.  A passage to the south is revealed!",
            "$n fiddles with something.  A passage to the south is revealed!",
            "A passage to the north is revealed.",
            "You turn the bracket.  The passage to the south is concealed!",
            "$n fiddles with something.  The passage to the south is concealed!",
            "The passage to the north is concealed."
        );
        return TRUE;
      }
      break;
    case 10722:
      if (cmd != CMD_TURN)
        return FALSE;
      if (!strcasecmp(buf, "bracket")) {
        ch->openUniqueDoor(DIR_SOUTH, DOOR_UNIQUE_DEF,
            "",
            "",
            "You turn the bracket.  A passage to the south is revealed!",
            "$n fiddles with something.  A passage to the south is revealed!",
            "A passage to the north is revealed.",
            "You turn the bracket.  The passage to the south is concealed!",
            "$n fiddles with something.  The passage to the south is concealed!",
            "The passage to the north is concealed."
        );
        return TRUE;
      }
      break;
    case 10727:
      if (cmd != CMD_PUSH)
        return FALSE;
      if (!strcasecmp(buf, "lever")) {
        ch->openUniqueDoor(DIR_NORTH, DOOR_UNIQUE_DEF,
            "",
            "",
            "You push the lever.  A passage to the north has been revealed!",
            "$n fiddles with something.  A passage to the north has been revealed!",
            "A passage to the south has been revealed!",
            "You push the lever.  The passage to the north has been concealed.",
            "$n fiddles with something.  The passage to the north has been concealed.",
            "The passage to the south has been concealed."
        );
        return TRUE;
      }
      break;
    case 10730:
      if (cmd != CMD_PUSH)
        return FALSE;
      if (!strcasecmp(buf, "lever")) {
        ch->openUniqueDoor(DIR_NORTH, DOOR_UNIQUE_DEF,
            "",
            "",
            "You push the lever.  A passage to the north has been revealed!",
            "$n fiddles with something.  A passage to the north has been revealed!",
            "A passage to the south has been revealed!",
            "You push the lever.  The passage to the north has been concealed.",
            "$n fiddles with something.  The passage to the north has been concealed.",
            "The passage to the south has been concealed."
        );
        return TRUE;
      }
      break;
    case 10752:
      if (cmd != CMD_TURN)
        return FALSE;
      if (!strcasecmp(buf, "bracket")) {
        ch->openUniqueDoor(DIR_SOUTHEAST, DOOR_UNIQUE_DEF,
            "",
            "",
            "You turn the bracket.  A passage to the southeast is revealed!",
            "$n fiddles with something.  A passage to the southeast is revealed!",
            "A passage to the northwest is revealed.",
            "You turn the bracket.  The passage to the southeast is concealed!",
            "$n fiddles with something.  The passage to the southeast is concealed!",
            "The passage to the northwest is concealed."
        );
        return TRUE;
      }
      break;
    case 10753:
      if (cmd != CMD_TURN)
        return FALSE;
      if (!strcasecmp(buf, "bracket")) {
        ch->openUniqueDoor(DIR_SOUTHWEST, DOOR_UNIQUE_DEF,
            "",
            "",
            "You turn the bracket.  A passage to the southwest is revealed!",
            "$n fiddles with something.  A passage to the southwest is revealed!",
            "A passage to the northeast is revealed.",
            "You turn the bracket.  The passage to the southwest is concealed!",
            "$n fiddles with something.  The passage to the southwest is concealed!",
            "The passage to the northeast is concealed."
        );
        return TRUE;
      }
      break;
    case 10759:
      if (cmd != CMD_PUSH) 
        return FALSE;
      if (!strcasecmp(buf, "footrest")) {
        ch->openUniqueDoor(DIR_NORTHWEST, DOOR_UNIQUE_DEF,
            "",
            "",
            "You push on the footrest.  A passage to the northwest is revealed.",
            "$n pushes on the footrest.  A passage to the northwest is revealed.",
            "A passage to the southeast is revealed.",
            "You push on the footrest.  The passage to the northwest is concealed.",
            "$n pushes on the footrest.  The passage to the northwest is concealed.",
            "The passage to the southeast is concealed."
        );
        return TRUE;
      }
      break;    
    case 10760:
      if (cmd != CMD_PUSH)
        return FALSE;
      if (!strcasecmp(buf, "footrest")) {
        ch->openUniqueDoor(DIR_NORTHEAST, DOOR_UNIQUE_DEF,
            "",
            "",
            "You push on the footrest.  A passage to the northeast is revealed.",
            "$n pushes on the footrest.  A passage to the northeast is revealed.",
            "A passage to the southwest is revealed.",
            "You push on the footrest.  The passage to the northeast is concealed.",
            "$n pushes on the footrest.  The passage to the northeast is concealed.",
            "The passage to the southwest is concealed."
        );
        return TRUE;
      }
      break;
    case 10764:
      if (cmd != CMD_PUSH)
        return FALSE;
      if (!strcasecmp(buf, "stone")) {
        ch->openUniqueDoor(DIR_SOUTH, DOOR_UNIQUE_DEF,
            "",
            "",
            "You push on the loose stone.  A passage to the south is revealed.",
            "$n fiddles with something.  A passage to the south is revealed.",
            "A passage to the north is revealed.",
            "You push on the loose stone.  The passage to the south is concealed.",
            "$n fiddles with something.  The passage to the south is concealed.",
            "The passage to the north is concealed."
        );
        return TRUE;
      }
      break;
    case 10772:
      if (cmd != CMD_PUSH)
        return FALSE;
      if (!strcasecmp(buf, "stone")) {
        ch->openUniqueDoor(DIR_NORTH, DOOR_UNIQUE_DEF,
            "",
            "",
            "You push on the loose stone.  A passage to the north is revealed.",
            "$n fiddles with something.  A passage to the north is revealed.",
            "A passage to the south is revealed.",
            "You push on the loose stone.  The passage to the north is concealed.",
            "$n fiddles with something.  The passage to the north is concealed.",
            "The passage to the south is concealed."
        );
        return TRUE;
      }
      break;
    case 10782:
      if (cmd != CMD_TURN)
        return FALSE;
      if (!strcasecmp(buf, "bracket")) {
        ch->openUniqueDoor(DIR_SOUTH, DOOR_UNIQUE_DEF,
            "",
            "",
            "You reach out and turn the bracket.  A passage to the south is revealed!",
            "$n fiddles with something.  A passage to the south is revealed!",
            "A passage to the north is revealed.",
            "You reach out and turn the bracket.  The passage to the south is concealed!",
            "$n fiddles with something.  The passage to the south is concealed!",
            "The passage to the north is concealed."
        );
        return TRUE;
      }
      break;
    case 10785:
      if (cmd != CMD_TURN)
        return FALSE;
      if (!strcasecmp(buf, "bracket")) {
        ch->openUniqueDoor(DIR_NORTH, DOOR_UNIQUE_DEF,
            "",
            "",
            "You reach out and turn the bracket.  A passage to the north is revealed!",
            "$n fiddles with something.  A passage to the north is revealed!",
            "A passage to the south is revealed.",
            "You reach out and turn the bracket.  The passage to the north is concealed!",
            "$n fiddles with something.  The passage to the north is concealed!",
            "The passage to the south is concealed."
        );
        return TRUE;
      }
      break;
    case 10790:
      if (cmd != CMD_PUSH)
        return FALSE;
      if (!strcasecmp(buf, "stone")) {
        ch->openUniqueDoor(DIR_EAST, DOOR_UNIQUE_DEF,
            "",
            "",
            "You push on the loose stone.  A passage to the east is revealed.",
            "$n fiddles with something.  A passage to the east is revealed.",
            "A passage to the west is revealed.",
            "You push on the loose stone.  The passage to the east is concealed.",
            "$n fiddles with something.  The passage to the east is concealed.",
            "The passage to the west is concealed."
        );
        return TRUE;
      }
      break;
    case 10791:
      if ((cmd != CMD_PUSH) && (cmd!= CMD_PULL))
        return FALSE;
      if (cmd == CMD_PUSH) {
        if (!strcasecmp(buf, "lever")) {
          ch->openUniqueDoor(DIR_WEST, DOOR_UNIQUE_OPEN_ONLY,
              "",
              "Nothing happens as the lever is already pushed forward.",
            "You push the lever.  A passage to the west has been revealed!",
            "$n fiddles with something.  A passage to the west has been revealed!",
            "A passage to the east has been revealed!",
              "",
              "",
              ""
          );
          return TRUE;
        }
      } else {
        if (!strcasecmp(buf, "lever")) {
          ch->openUniqueDoor(DIR_WEST, DOOR_UNIQUE_CLOSE_ONLY,
              "Nothing happens as the lever is already pulled erect.",
              "",
              "",
              "",
              "",
            "You pull on the lever.  The passage to the west has been concealed.",
            "$n fiddles with something.  The passage to the west has been concealed.",
            "The passage to the east has been concealed."
          );
          return TRUE;
        }
      }
      break;
    case 10814:
      if (cmd != CMD_PUSH)
        return FALSE;
      if (!strcasecmp(buf, "stone")) {
        ch->openUniqueDoor(DIR_EAST, DOOR_UNIQUE_DEF,
            "",
            "",
            "You push on the loose stone.  A passage to the east is revealed.",
            "$n fiddles with something.  A passage to the east is revealed.",
            "A passage to the west is revealed.",
            "You push on the loose stone.  The passage to the east is concealed.",
            "$n fiddles with something.  The passage to the east is concealed.",
            "The passage to the west is concealed."
        );
        return TRUE;
      }
      break;
    case 10815:
      if (cmd != CMD_PUSH)
        return FALSE;
      if (!strcasecmp(buf, "stone")) {
        ch->openUniqueDoor(DIR_WEST, DOOR_UNIQUE_DEF,
            "",
            "",
            "You push on the loose stone.  A passage to the west is revealed.",
            "$n fiddles with something.  A passage to the west is revealed.",
            "A passage to the east is revealed.",
            "You push on the loose stone.  The passage to the west is concealed.",
            "$n fiddles with something.  The passage to the west is concealed.",
            "The passage to the east is concealed."
        );
        return TRUE;
      }
      break;
    case 10820:
      if ((cmd != CMD_PUSH) && (cmd!= CMD_PULL))
        return FALSE;
      if (cmd == CMD_PUSH) {
        if (!strcasecmp(buf, "lever")) {
          ch->openUniqueDoor(DIR_EAST, DOOR_UNIQUE_OPEN_ONLY,
              "",
              "Nothing happens as the lever is already pushed forward.",
            "You push the lever.  A passage to the east has been revealed!",
            "$n fiddles with something.  A passage to the east has been revealed!",
            "A passage to the west has been revealed!",
              "",
              "",
              ""
          );
          return TRUE;
        }
      } else {
        if (!strcasecmp(buf, "lever")) {
          ch->openUniqueDoor(DIR_EAST, DOOR_UNIQUE_CLOSE_ONLY,
              "Nothing happens as the lever is already pulled erect.",
              "",
              "",
              "",
              "",
            "You pull on the lever.  The passage to the east has been concealed.",
            "$n fiddles with something.  The passage to the east has been concealed.",
            "The passage to the west has been concealed."
          );
          return TRUE;
        }
      }
      break;
    case 10821:
      if (cmd != CMD_TURN)
        return FALSE;
      if (!strcasecmp(buf, "bracket")) {
        ch->openUniqueDoor(DIR_WEST, DOOR_UNIQUE_DEF,
              "",
              "",
            "You reach out and turn the bracket.  A passage to the west is revealed!",
            "$n fiddles with something.  A passage to the west is revealed!",
            "A passage to the east is revealed.",
            "You reach out and turn the bracket.  The passage to the west is concealed!",
            "$n fiddles with something.  The passage to the west is concealed!",
            "The passage to the east is concealed."
        );
        return TRUE;
      }
      break;
    case 14152:
      if (cmd != CMD_SAY && cmd != CMD_SAY2)
        return FALSE;
      if (!strcasecmp(buf, "time")) {
        ch->doSay(arg);
        ch->openUniqueDoor(DIR_NORTH, DOOR_UNIQUE_DEF,
            "",
            "",
            "A strange rumbling is heard.  Your words have caused a boulder to roll aside.", 
            "A strange rumbling is heard.  $n's words have caused a boulder to roll aside.", 
            "A boulder to the south has rolled aside.",
            "A strange rumbling is heard.  Your words cause a large boulder to conceal a path.",
            "A strange rumbling is heard.  $n's words cause a large boulder to conceal a path.",
            "A boulder rolls forward blocking the way south."
        );
        return TRUE;
      }
      break;
    case 14153:
      if (cmd != CMD_SAY && cmd != CMD_SAY2)
        return FALSE;
      if (!strcasecmp(buf, "time")) {
        ch->doSay(arg);
        ch->openUniqueDoor(DIR_SOUTH, DOOR_UNIQUE_DEF,
            "",
            "",
            "A strange rumbling is heard.  Your words have caused a boulder to roll aside.", 
            "A strange rumbling is heard.  $n's words have caused a boulder to roll aside.", 
            "A boulder to the north has rolled aside.",
            "A strange rumbling is heard.  Your words cause a large boulder to conceal a path.",
            "A strange rumbling is heard.  $n's words cause a large boulder to conceal a path.",
            "A boulder rolls forward blocking the way north."
        );
        return TRUE;
      }
      break;
  case 14296:
    if (cmd != CMD_SAY && cmd != CMD_SAY2)
      return FALSE;
    if (!strcasecmp(buf, "knowledge")) {
      ch->doSay(arg);
      ch->openUniqueDoor(DIR_NORTH, DOOR_UNIQUE_DEF,
			 "",
			 "",
			 "Your words have caused the stone to the north to slide open.",
			 "$n's words have caused the stone to the north to slide open.",
			 "A stone panel in the south wall slides open with a rumble.",
			 "Your words cause the stone to the north to slide closed.",
			 "$n's words cause the stone to the north to slide closed.",
            "A stone panel in the south wall slides closed with a rumble."
			 );
      return TRUE;
    }
  case 14299:
    if (cmd != CMD_SAY && cmd != CMD_SAY2)
      return FALSE;
    if (!strcasecmp(buf, "knowledge")) {
      ch->doSay(arg);
      ch->openUniqueDoor(DIR_SOUTH, DOOR_UNIQUE_DEF,
			 "",
			 "",
			 "Your words have caused the stone to the south to slide open.",
			 "$n's words have caused the stone to the south to slide open.",
			 "A stone panel in the north wall slides open with a rumble.",
			 "Your words cause the stone to the south to slide closed.",
			 "$n's words cause the stone to the south to slide closed.",
            "A stone panel in the north wall slides closed with a rumble."
			 );
      return TRUE;
    }
    case 14302:
      if ((cmd != CMD_PUSH) && (cmd!= CMD_PULL))
        return FALSE;
      if (cmd == CMD_PUSH) {
        if (!strcasecmp(buf, "stone")) {
          ch->openUniqueDoor(DIR_EAST, DOOR_UNIQUE_OPEN_ONLY,
            "",
            "The stone is already pushed aside.",
            "You push the stone aside.  A passage to the east is revealed.",
            "$n pushes a stone aside.  A passage to the east is revealed.",
            "A stone to the west is pushed aside revealing a passage.",
            "",
            "",
            ""
          );
          return TRUE;
        }
      } else {
        if (!strcasecmp(buf, "stone")) {
          ch->openUniqueDoor(DIR_EAST, DOOR_UNIQUE_CLOSE_ONLY,
            "The stone is already pulled into place.",
            "",
            "",
            "",
            "",
          "You pull the stone back in place concealing the way east.",
          "$n pulls a stone upright concealing the way east.",
            "A large stone is pulled into place blocking the way west."
          );
          return TRUE;
        }
      }
      break;
    case 14319:
      if ((cmd != CMD_PUSH) && (cmd!= CMD_PULL))
        return FALSE;
      if (cmd == CMD_PUSH) {
        if (!strcasecmp(buf, "stone")) {
          ch->openUniqueDoor(DIR_WEST, DOOR_UNIQUE_OPEN_ONLY,
            "",
            "The stone is already pushed aside.",
            "You push the stone aside.  A passage to the west is revealed.",
            "$n pushes a stone aside.  A passage to the west is revealed.",
            "A stone to the east is pushed aside revealing a passage.",
            "",
            "",
            ""
          );
          return TRUE;
        }
      } else {
        if (!strcasecmp(buf, "stone")) {
          ch->openUniqueDoor(DIR_WEST, DOOR_UNIQUE_CLOSE_ONLY,
            "The stone is already pulled into place.",
            "",
            "",
            "",
            "",
          "You pull the stone back in place concealing the way west.",
          "$n pulls a stone upright concealing the way west.",
            "A large stone is pulled into place blocking the way east."
          );
          return TRUE;
        }
      }
      break;
    case 15257:
      if ((cmd != CMD_PUSH) && (cmd!= CMD_PULL))
        return FALSE;
      if (cmd == CMD_PUSH) {
        if (!strcasecmp(buf, "shrub")) {
          ch->openUniqueDoor(DIR_WEST, DOOR_UNIQUE_OPEN_ONLY,
              "",
              "Nothing happens as the passageway is already revealed.",
              "You pull the shrub aside.  Your action reveals a passageway to the river.",
              "$n pulls the shrub aside revealing a passage to the river.",
              "Some shrubs are moved aside revealing a forest trail.",
              "",
              "",
              ""
          );
          return TRUE;
        }
      } else {
        if (!strcasecmp(buf, "shrub")) {
          ch->openUniqueDoor(DIR_WEST, DOOR_UNIQUE_CLOSE_ONLY_FORCE,
              "The shrubbery has already been pulled into place.",
              "",
              "",
              "",
              "",
              "You pull the shrub back into place, concealing a path to the river.",
              "$n pulls the shrub back into place, concealing a path to the river.",
              "Some shrubs are moved, concealing a path."
          );
          return TRUE;
        }
      }
      break;
    case 15293:
      if ((cmd != CMD_PUSH) && (cmd!= CMD_PULL))
        return FALSE;
      if (cmd == CMD_PUSH) {
        if (!strcasecmp(buf, "shrub")) {
          ch->openUniqueDoor(DIR_EAST, DOOR_UNIQUE_OPEN_ONLY,
              "",
              "Nothing happens as the path is already revealed.",
              "You pull the shrub aside.  Your action reveals a path to a small, forest trail.",
              "$n pulls the shrub aside revealing a path to a small, forest trail.",
              "Some shrubs are moved aside revealing a path to the river.",
              "",
              "",
              ""
          );
          return TRUE;
        }
      } else {
        if (!strcasecmp(buf, "shrub")) {
          ch->openUniqueDoor(DIR_EAST, DOOR_UNIQUE_CLOSE_ONLY_FORCE,
              "The shrubbery has already been pulled into place.",
              "",
              "",
              "",
              "",
              "You pull the shrub back into place, concealing a path to the small, forest trail.",
              "$n pulls the shrub back into place, concealing a path to the small, forest trail.",
              "Some shrubs are moved, concealing a path to the river."
          );
          return TRUE;
        }
      }
      break;
    case 15986:
      if (cmd != CMD_PRESS)
        return FALSE;
      if (!strcasecmp(buf, "button")) {
        ch->openUniqueDoor(DIR_UP, DOOR_UNIQUE_DEF,
            "",
            "",
            "You push a button and a staircase folds out of the wall.",
            "$n pushes a button and a staircase folds out of the wall.",
            "A staircase folds out of the wall.",
            "You push a button and a staircase folds into the wall.",
            "$n pushes a button and a staircase folds into the wall.",
            "A staircase folds into the wall."
        );
        return TRUE;
      }
      break;
    case 15996:
      if (cmd != CMD_PRESS)
        return FALSE;
      if (!strcasecmp(buf, "button")) {
        ch->openUniqueDoor(DIR_DOWN, DOOR_UNIQUE_DEF,
            "",
            "",
            "You push a button and a staircase folds out of the wall.",
            "$n pushes a button and a staircase folds out of the wall.",
            "A staircase folds out of the wall.",
            "You push a button and a staircase folds into the wall.",
            "$n pushes a button and a staircase folds into the wall.",
            "A staircase folds into the wall."
        );
        return TRUE;
      }
      break;
    case 15984:
      if (cmd != CMD_PRESS)
        return FALSE;
      if (!strcasecmp(buf, "button")) {
        ch->openUniqueDoor(DIR_UP, DOOR_UNIQUE_DEF,
            "",
            "",
            "You push a button and a staircase folds out of the wall.",
            "$n pushes a button and a staircase folds out of the wall.",
            "A staircase folds out of the wall.",
            "You push a button and a staircase folds into the wall.",
            "$n pushes a button and a staircase folds into the wall.",
            "A staircase folds into the wall."
        );
        return TRUE;
      }
      break;
    case 15992:
      if (cmd != CMD_PRESS)
        return FALSE;
      if (!strcasecmp(buf, "button")) {
        ch->openUniqueDoor(DIR_DOWN, DOOR_UNIQUE_DEF,
            "",
            "",
            "You push a button and a staircase folds out of the wall.",
            "$n pushes a button and a staircase folds out of the wall.",
            "A staircase folds out of the wall.",
            "You push a button and a staircase folds into the wall.",
            "$n pushes a button and a staircase folds into the wall.",
            "A staircase folds into the wall."
        );
        return TRUE;
      }
      break;

 
    case 16238:
      if ((cmd != CMD_PUSH) && (cmd!= CMD_PULL))
        return FALSE;
      if (cmd == CMD_PUSH) {
        if (!strcasecmp(buf, "anatomy")) {
          ch->openUniqueDoor(DIR_EAST,DOOR_UNIQUE_OPEN_ONLY,
              "",
              "Nothing happens as the bookshelf has already been opened.",
              "You push the book on anatomy inward.  A bookshelf slides open revealing a hidden passage.",
              "$n pushes the book on anatomy inward.  A bookshelf slides open revealing a hidden passage.",
              "You hear a rumbling as a bookshelf slides open revealing a passage.",
              "",
              "",
              ""
          );
          return TRUE;
        }
      } else {
        if (!strcasecmp(buf, "anatomy")) {
          ch->openUniqueDoor(DIR_EAST,DOOR_UNIQUE_CLOSE_ONLY,
               "Nothing happens as the bookshelf has already been closed.",
              "",
              "",
              "",
              "",
              "You pull the book towards yourself.  A bookshelf slides shut concealing a hidden passage.",
              "$n pulls a book back into line.  A bookshelf slides shut concealing a hidden passage.",
              "You hear a rumbling as a bookshelf slides shut hiding a passage."
          );
          return TRUE;
        }
      }
      break;
    case 16249:
      if ((cmd != CMD_PUSH) && (cmd!= CMD_PULL))
        return FALSE;
      if (cmd == CMD_PUSH) {
        if (!strcasecmp(buf, "anatomy")) {
          ch->openUniqueDoor(DIR_WEST,DOOR_UNIQUE_OPEN_ONLY,
              "",
              "Nothing happens as the bookshelf has already been opened.",
              "You push the book on anatomy inward.  A bookshelf slides open revealing a hidden passage.",
              "$n pushes a book on anatomy inwards.  A bookshelf slides open revealing a hidden passage.",
              "You hear a rumbling as a bookshelf slides open revealing a passage.",
              "",
              "",
              ""
          );
          return TRUE;
        }
      } else {
        if (!strcasecmp(buf, "anatomy")) {
          ch->openUniqueDoor(DIR_WEST,DOOR_UNIQUE_CLOSE_ONLY,
              "Nothing happens as the bookshelf has already been closed.",
              "",
              "",
              "",
              "",
              "You pull the book towards yourself.  A bookshelf slides shut concealing a hidden passage.",
              "$n pulls a book back into line.  A bookshelf slides shut concealing a hidden passage.",
              "You hear a rumbling as a bookshelf slides shut hiding a passage."
          );
          return TRUE;
        }
      }
      break;
    case 20541:
      if ((cmd != CMD_USE) && (cmd != CMD_TURN) && (cmd != CMD_OPERATE))
        return FALSE;
      if (!strcasecmp(buf, "winch")) {
        act("You operate the winch.",
            TRUE,ch,0,0,TO_CHAR);
        act("$n operates the winch.",
            TRUE,ch,0,0,TO_ROOM);
        ch->openUniqueDoor(DIR_NORTHWEST,DOOR_UNIQUE_DEF,
             "",
             "",
            "The mighty portcullis rises up!",
            "The mighty portcullis rises up!",
            "With a rumble, the mighty portcullis rises up!",
            "The mighty portcullis lowers, barring the way!",
            "The mighty portcullis lowers, barring the way!",
            "The mighty portcullis slowly lowers, barring the way."
        );
        return TRUE;
      }
      break;
    case 20582:
      if (cmd != CMD_CHIP)
        return FALSE;
      if (!strcasecmp(buf, "ice")) {
        ch->openUniqueDoor(DIR_UP,DOOR_UNIQUE_OPEN_ONLY,
            "",
            "All the loose ice has been chipped away already.",
            "Chipping the ice away, you find toe and hand holds in the rock below.",
            "Chipping the ice away, $n finds toe and hand holds in the rock below.",
            "A large amount of ice crumbles away beneath you!",
            "",
            "",
            ""
        );
        return TRUE;
      }
      break;
    case 27103:
    case 27104:
    case 27106:
    case 27107:
    case 27108:
    case 27111:
    case 27113:
    case 27116:
    case 27118:
    case 27122:
    case 27124:
    case 27128:
    case 27129:
      if ((cmd != CMD_DIG) && (cmd != CMD_COVER) && (cmd != CMD_FILL))
        return FALSE;
      if (cmd == CMD_DIG) {
        if (!strcasecmp(buf, "grave")) {
          ch->openUniqueDoor(DIR_DOWN,DOOR_UNIQUE_OPEN_ONLY,
            "",
            "The grave has already been dug up!",
            "Digging at the earth below you find a shallow grave.",
            "$n reveals a shallow grave as they dig at the earth.",
            "The earth above is slowly removed to reveal the sky.",
            "",
            "",
            ""
          );
          return TRUE;
        }
      } else {
        if (!strcasecmp(buf, "grave")) {
          ch->openUniqueDoor(DIR_DOWN,DOOR_UNIQUE_CLOSE_ONLY,
            "The grave is already covered.",
            "",
            "",
            "",
            "",
            "You cover the shallow grave with loose earth.",
            "$n covers the shallow grave with loose earth.",
            "Loose earth cascades from above slowly sealing the grave."
          );
          return TRUE;
        }
      }
      break;
  case 27134:
  case 27135:
  case 27136:
  case 27137:
  case 27138:
  case 27139:
  case 27140:
  case 27141:
  case 27142:
  case 27143:
  case 27144:
  case 27145:
  case 27146:
      if (cmd != CMD_DIG)
        return FALSE;
      if (!strcasecmp(buf, "earth")) {
        ch->openUniqueDoor(DIR_UP,DOOR_UNIQUE_OPEN_ONLY,
            "",
            "The earth overhead is already removed.",
            "You dig your way through the earth overhead opening the way.",
            "$n furiously digs at the earth overhead until an opening appears.",
            "The ground below you suddenly collapses in on itself and a hole appears.",
            "",
            "",
            ""
        );
        return TRUE;
      }
      break;
    case 27305:
      if ((cmd != CMD_PUSH) && (cmd != CMD_PULL))
        return FALSE;
      if (cmd == CMD_PUSH) {
        if(!strcasecmp(buf, "crate")){
          ch->openUniqueDoor(DIR_DOWN,DOOR_UNIQUE_OPEN_ONLY,
               "",
               "Nothing happens as the crate is already not covering the hole.",
               "You push the crate aside revealing a hole leading downward.",
               "$n pushes the crate aside revealing a hole leading downward.",
               "Something is moved about above you revealing a hold leading upward.",
               "",
               "",
               ""
          );
          return TRUE;
        }
      } else {
        if(!strcasecmp(buf, "crate")){
          ch->openUniqueDoor(DIR_DOWN,DOOR_UNIQUE_CLOSE_ONLY,
              "Nothing happens as the crate is already covering the hole.",
              "",
              "",
              "",
              "",
              "You pull the crate back over the hole.",
              "$n pulls the crate back over the hole.",
              "Something very large is pulled over the hole above you, blocking the way."
          );
          return TRUE;
        }
      }
      break;
    case 27306:
      if ((cmd != CMD_PUSH) && (cmd != CMD_PULL))
        return FALSE;
      if (cmd == CMD_PUSH) {
        if(!strcasecmp(buf, "crate")){
          ch->openUniqueDoor(DIR_UP,DOOR_UNIQUE_OPEN_ONLY,
              "",
              "Nothing happens as the crate is already not covering the hole.",
          "You push the crate aside and reveal a dark hole.",
          "$n pushes the crate aside revealing a dark hole.",
              "Something below you is moved, revealing a hole.",
              "",
              "",
              ""
          );
          return TRUE;
        }
      } else {
        if (!strcasecmp(buf, "crate")) {
          ch->openUniqueDoor(DIR_UP,DOOR_UNIQUE_CLOSE_ONLY,
            "Nothing happens as the crate is already covering the hole.",
            "",
            "",
            "",
            "",
            "You pull the crate back over the dark hole.",
            "$n pulls the crate back over the dark hole.",
            "Something below you is moved, and you are no longer able to fit through the hole."
          );
          return TRUE;
        }
      }
      break;
  case 27828:
      if (cmd != CMD_PUSH)
        return FALSE;
      if (!strcasecmp(buf, "button")) {
        ch->openUniqueDoor(DIR_UP, DOOR_UNIQUE_DEF,
              "",
              "",
          "A rumbling noise fills the air and a set of handholds emerge from the wall.",
          "As $n pushes the button, a set of handholds emerge from the wall with a rumbling sounds.",
              "You hear a rumbling sound from below you.",
          "The handholds on the wall retract into the stone with a grating sound.",
          "As $n pushes the button, the handholds on the wall retract into the stone.",
              "You hear a grating sound below you."
        );
        return TRUE;
      }
      break;
  case 27890:
      if (cmd != CMD_PULL)
        return FALSE;
      if (!strcasecmp(buf, "lever")) {
        ch->openUniqueDoor(DIR_DOWN, DOOR_UNIQUE_DEF,
              "",
              "",
          "A rumbling noise fills the air and a set of handholds emerge from the wall below.",
          "As $n pushes the button, a set of handholds emerge from the wall below with a rumbling sounds.",
              "You hear a rumbling sound and handholds suddenly emerge from the wall.",
          "The handholds on the wall below retract into the stone with a grating sound.",
          "As $n pushes the button, the handholds on the wall below retract into the stone.",
              "You hear a grating and the handholds retract into the wall suddenly."
        );
        return TRUE;
      }
      break;
    case 33679:
      if ((cmd != CMD_PULL) && (cmd != CMD_PUSH))
        return FALSE;
      if (!strcasecmp(buf, "brambles")) {
        act("You pull on the brambles.",
            TRUE,ch,0,0,TO_CHAR);
        act("$n pulls on some brambles.",
            TRUE,ch,0,0,TO_ROOM);
        ch->openUniqueDoor(DIR_SOUTHWEST, DOOR_UNIQUE_DEF,
            "",
            "",
            "You reveal a side trail to the southwest.",
            "$n's action reveals a side trail to the southwest.",
            "Something causes an opening in the brambles to the southwest to appear, revealing a side trail.",
            "You conceal the side trail to the southwest.",
            "$n's action causes the side trail to the southwest to be concealed.",
            "Something causes an opening in the brambles to the southwest to close, concealing a side trail."
        );
        return TRUE;
      }
      break;
    case 33690:
      if ((cmd != CMD_PULL) && (cmd != CMD_PUSH))
        return FALSE;
      if (!strcasecmp(buf, "brambles")) {
        act("You pull on the brambles.",
            TRUE,ch,0,0,TO_CHAR);
        act("$n pulls on some brambles.",
            TRUE,ch,0,0,TO_ROOM);
        ch->openUniqueDoor(DIR_NORTHEAST, DOOR_UNIQUE_DEF,
            "",
            "",
            "You reveal an opening to the northeast.",
            "$n's action reveals an opening to the northeast.",
            "Something pushes aside the brambles to the northeast, revealing a opening.",
            "You conceal the opening to the northeast.",
            "$n's action causes the opening to the northeast to be concealed.",
            "Something pulls some brambles to the northeast closed, concealing an opening."
        );
        return TRUE;
      }
      break;
    default:
      vlogf(LOG_LOW, format("Unsupported room (%d) in secretDoors") %  rp->number);
      return FALSE;
  }
  return FALSE;
}
