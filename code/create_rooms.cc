/*****************************************************************************
 **                                                                         **
 ** SneezyMUD  - All rights reserved, SneezyMUD Coding Team.                **
 **                                                                         **
 ** Original edit code from Silly, May 1992                                 **
 **                                                                         **
 ** Updated originally by Russ(Brutius)                                     **
 **                                                                         **
 ** Handles the creation/saving/loading of rooms for builders               **
 **                                                                         **
 *****************************************************************************/

#include "stdsneezy.h"
#include "obj_table.h"
#include "obj_seethru.h"
#include "database.h"

const char *room_fields[] =
{
  "description",  //  1
  "exdscr",       //  2
  "exit",         //  3
  "extra",        //  4
  "flags",        //  5
  "height",       //  6
  "line",         //  7
  "max_capacity", //  8
  "name",         //  9
  "river",        // 10
  "sector_type",  // 11
  "teleport",     // 12
  "copy",         // 13
  "replace",      // 14
  "list",         // 15
  "autoformat",   // 16
  "\n"
};

// I would have just made this a short list of the room_fields[]
// but it was cleaner to do it this way.
const char *copy_fieldT[] =
{
  "description", //  1
  "name",        //  2
  "extras",      //  3
  "flags",       //  4
  "sector",      //  5
  "height",      //  6
  "capacity",    //  7
  "all",         //  8
  "\n"
};

int room_length[] =
{
  1024,
  512,
  50,
  512,
  50,
  50,
  50,
  50,
  80,
  50,
  50,
  100,
  50,
};

void make_room_coords(TRoom *rorig, TRoom *tdest, int dir){
  tdest->setXCoord(rorig->getXCoord());
  tdest->setYCoord(rorig->getYCoord());
  tdest->setZCoord(rorig->getZCoord());

  switch(dir){
    case 0:
      tdest->setYCoord(tdest->getYCoord()+1);
      break;
    case 1:
      tdest->setXCoord(tdest->getXCoord()+1);
      break;
    case 2:
      tdest->setYCoord(tdest->getYCoord()-1);
      break;
    case 3:
      tdest->setXCoord(tdest->getXCoord()-1);
      break;
    case 4:
      tdest->setZCoord(tdest->getZCoord()+1);
      break;
    case 5:
      tdest->setZCoord(tdest->getZCoord()-1);
      break;
    case 6:
      tdest->setXCoord(tdest->getXCoord()+1);
      tdest->setYCoord(tdest->getYCoord()+1);
      break;
    case 7:
      tdest->setYCoord(tdest->getYCoord()+1);
      tdest->setXCoord(tdest->getXCoord()-1);
      break;
    case 8:
      tdest->setYCoord(tdest->getYCoord()-1);
      tdest->setXCoord(tdest->getXCoord()+1);
      break;
    case 9:
      tdest->setXCoord(tdest->getXCoord()-1);
      tdest->setYCoord(tdest->getYCoord()-1);
      break;    
  }
}


static void update_room_menu(const TBeing *ch)
{
  const char *edit_menu_basic =
 "    1) Name                       2) Description\n\r"
 "    3) Flags                      4) Sector Type\n\r"
 "    5) Exits                      6) Extra Description\n\r"
 "    7) Maximum Capacity           8) Room Height\n\r"
 "    9) Delete an exit            10) Delete all extra descs\n\r"
 "\n\r";
  const char *edit_menu_advanced =
 "%s 1)%s %s\n\r"
 "%s 2)%s %sDescription%s\n\r"
 "%s 3)%s %sFlags%s\n\r"
 "%s 4)%s %s\n\r"
 "%s 5)%s %sExits%s:%s\n\r"
 "%s 6)%s %sExtra Description(s)%s\n\r"
 "%s 7)%s %sMax Capacity%s: %d\n\r"
 "%s 8)%s %sRoom Height%s: %d\n\r"
 "%s 9)%s %sDelete an Exit%s\n\r"
 "%s10)%s %sDelete All Extra Descriptions%s\n\r";

  ch->sendTo(VT_HOMECLR);
  ch->sendTo(fmt(VT_CURSPOS) % 1 % 1);
  ch->sendTo(fmt("Room Name: %s") % ch->roomp->name);
  ch->sendTo(fmt(VT_CURSPOS) % 2 % 1);
  ch->sendTo(fmt("Number: %d") % ch->roomp->number);
  ch->sendTo(fmt(VT_CURSPOS) % 3 % 1);
  ch->sendTo(fmt("Sector Type: %s") % TerrainInfo[ch->roomp->getSectorType()]->name);
  ch->sendTo(fmt(VT_CURSPOS) % 5 % 1);
  ch->sendTo("Menu:\n\r");

  if (IS_SET(ch->desc->autobits, AUTO_TIPS)) {
    dirTypeT     tExit;
    roomDirData *tData;
    sstring       tStString("");

    const char *exDirs[] =
    {
      "N", "E", "S", "W", "U",
      "D", "NE", "NW", "SE", "SW"
    };

    for (tExit = MIN_DIR; tExit < MAX_DIR; tExit++)
      if ((tData = ch->roomp->exitDir(tExit)) && tData->to_room != ROOM_NOWHERE) {
        tStString += " ";
        tStString += exDirs[tExit];
      }

    if (tStString.empty()) {
      tStString += " ";
      tStString += "NONE";
    }

    ch->sendTo(fmt(edit_menu_advanced) %
               ch->cyan() % ch->norm() % ch->roomp->name %
               ch->cyan() % ch->norm() % ch->purple() % ch->norm() %
               ch->cyan() % ch->norm() % ch->purple() % ch->norm() %
               ch->cyan() % ch->norm() % TerrainInfo[ch->roomp->getSectorType()]->name %
               ch->cyan() % ch->norm() % ch->purple() % ch->norm() % tStString %
               ch->cyan() % ch->norm() % ch->purple() % ch->norm() %
               ch->cyan() % ch->norm() % ch->purple() % ch->norm() % ch->roomp->getMoblim() %
               ch->cyan() % ch->norm() % ch->purple() % ch->norm() % ch->roomp->getRoomHeight() %
               ch->cyan() % ch->norm() % ch->purple() % ch->norm() %
               ch->cyan() % ch->norm() % ch->purple() % ch->norm());
  } else
    ch->sendTo(edit_menu_basic);

  ch->sendTo("--> ");
}

void TBeing::doEdit(const char *)
{
  sendTo("Mobs can't edit.\n\r");
}

void TPerson::doEdit(const char *arg)
{
  int field,     dtype,     
      dkey,      rspeed,    rdir,      new_dir,
      dcond,     dldiff,    dweight,   tele_room,
      tele_time, tele_look, moblim,    s_type,
      rheight,   cRoom;
  dirTypeT dir;
  int exroom;
  doorTypeT doortype;
  sectorTypeT sectype;
  sstring tStr;
  sstring regStr;
  sstring line, garbled;
  sstring word, stripped_word;
  sstring punctuation = ".!?;:"; 
  sstring newDescr = "";
  size_t swlen = 0;
  bool was_word = false;
  long r_flags;
  sstring tStString("");
  char sstring[512],
       Buf[256],
       tString[256],
       tTextLns[4][256] = {"\0", "\0", "\0", "\0"};
  const char *tBuf;
  extraDescription *ed,
                   *prev;
  TRoom *newrp, *newrpTo;
  FILE  *tFile;

  if (!hasWizPower(POWER_EDIT)) {
    incorrectCommand();
    return;
  }

  if (!isImmortal())
    return;

  // Someone forced us to do this, this is a no-no.  We have to make
  // calls to the desc, and if it doesn't exist....*crash*
  if (!desc)
    return;

  if (!in_range(roomp->number, desc->blockastart, desc->blockaend) &&
      !in_range(roomp->number, desc->blockbstart, desc->blockbend) &&
      roomp->number != desc->office && !hasWizPower(POWER_REDIT_ENABLED)) {
    sendTo("This room is not yours...Sorry\n\r");
    return;
  }

  if (!limitPowerCheck(CMD_EDIT, roomp->number)) {
    sendTo("You are not allowed to edit this room, sorry.\n\r");
    return;
  }


  bisect_arg(arg, &field, sstring, room_fields);

  /******************** NOTES ********************
   * Even after much sucessful testing I still   *
   * don't trust the line/copy loops.  Until     *
   * they have been Fully proven there is a      *
   * safty catch to limit them to 15 iterations  *
   *                              -Lapsos        *
   ***********************************************/

  if (!field || field > 16) {
    tStr = "\0";
    tStr += "Supported Fields:\n\r";
    tStr += "  description             Will prompt for text.\n\r";
    tStr += "  exdscr <dir>            Will prompt for text.\n\r";
    tStr += "  exit <dir> <type> <door_cond> <lock_diff> <weight> <key> <to_room>\n\r";
    tStr += "  extra <trigger words>   Will prompt for text.\n\r";
    tStr += "  flags rawbv ##          old edit fs ## ?? format.\n\r";
    tStr += "        1-??              Toggle flag ? on/off.\n\r";
    tStr += "  height <height>         Set room height.\n\r";
    tStr += "  line <exit/dir list>    Creates a line of room exits.\n\r";
    tStr += "  max_capacity ##         Set room max capacity.\n\r";
    tStr += "  name                    Will prompt for text.\n\r";
    tStr += "  river <speed> <dir>     Speed must be divisible by 15, 0 means no current.\n\r";
    tStr += "  sector_type <sector>    Will set room sector type.\n\r";
    tStr += "  teleport <time> <to_room> <look_flag>  Time must be divisible by 10.\n\r";
    tStr += "    <dir> = 0=n, 1=e, 2=s, 3=w, 4=u, 5=d, 6=ne, 7=nw, 8=se, 9=sw\n\r";
    tStr += "  copy <field> <room(s)>  Will copy <field> into room(s) specified.\n\r";
    tStr += "  replace <desc/extra> <\"extra\"/\"text\"> <\"text\"> <\"text\">";
    tStr += "  list <2>                Will list all rooms in the rooms file.\n\r";
    tStr += "  autoformat              Will automatically format room to standards.\n\r";
    tStr += "Please see HELP EDIT for more information.\n\r";
    sendTo(tStr);
    return;
  }

  r_flags = -1;
  s_type  = -1;

  switch (field) {
    case  1: // Description
      desc->str = &roomp->descr;
      desc->max_str = MAX_STRING_LENGTH;
#if 0
// as is, this will cause it to send the descr as a "bug"
      if (desc->m_bIsClient) {
        desc->clientf(fmt("%d") % CLIENT_STARTEDIT % 4000);
      }
#endif
      break;
    case  2: // Exdscr
      if (sscanf(sstring, "%d", &rdir) != 1) {
        tStr = "\0";
        for (dir = MIN_DIR; dir < MAX_DIR; dir++)
          if (roomp->dir_option[dir]) {
            sprintf(sstring,
                    "%d) %s %s -To> %d\n\rLock Diff:%d  Weight:%d  Key:%d\n\rTrap Dam: %d\n\r",
                    dir, dirs[dir],
                    roomp->dir_option[dir]->keyword,
                    roomp->dir_option[dir]->to_room,
                    roomp->dir_option[dir]->lock_difficulty,
                    roomp->dir_option[dir]->weight,
                    roomp->dir_option[dir]->key,
                    roomp->dir_option[dir]->trap_dam);
            tStr += sstring;
          }
        sendTo(tStr);
        return;
      }
      if ((rdir < MIN_DIR) || (rdir >= MAX_DIR)) {
        sendTo("Incorrect direction.\n\r");
        sendTo(fmt("Must enter %d-%d.  I will ask for text.\n\r") % MIN_DIR % (MAX_DIR-1));
        return;
      }
      sstring[0] = 0;
      if (roomp->dir_option[rdir]) {
        desc->str = &roomp->dir_option[rdir]->description;
      } else {
        roomp->dir_option[rdir] = new roomDirData();
        desc->str = &roomp->dir_option[rdir]->description;
      }
      break;
    case  3: // Exit
      // 2 Formats now:
      // edit exit <dir> <door> <cond> <lock-diff> <weight> <key> <to-room>
      // edit exit copy <to-room> <exit-list|to-room-list>

      // Copy Extension.
      tBuf = sstring;
      half_chop(tBuf, Buf, tString);

      if (*tBuf && is_abbrev(tBuf, "copy")) {
        int toRoom = convertTo<int>(tBuf);  // from the <to-room> block

        if (strcmp(getName(), "Lapsos") != 0) {
          sendTo("Don't use this option yet.  It is still under development.\n\r");
          return;
        }

        if (!*tString) {
          sendTo("Syntax: edit copy <to-room> <exit-list|to-room-list>\n\r");
          return;
        }

        if (!(newrpTo = real_roomp(toRoom))) {
          if (toRoom < 0 || toRoom >= WORLD_SIZE) {
            sendTo(fmt("Invalid room number.  Must be between 0 and %d.\n\r") %
                   (WORLD_SIZE - 1));
            return;
          }

	  if (!limitPowerCheck(CMD_EDIT, toRoom)) {
	    sendTo("You are not allowed to make an exit out to that room, sorry.\n\r");
	    return;
	  }


          CreateOneRoom(toRoom);

          if (!(newrpTo = real_roomp(toRoom))) {
            sendTo("For some reason that room neither exists nor could it be created.\n\r");
            return;
          }

          sendTo("You create the room for the exit(s).\n\r");
          newrpTo->setRoomFlags(roomp->getRoomFlags());
          newrpTo->setSectorType(roomp->getSectorType());
          newrpTo->setRoomHeight(roomp->getRoomHeight());
        }

        tBuf = tString;
        half_chop(tBuf, Buf, tString);
        r_flags = 0;

        while (*Buf && r_flags < 15) {
          if (isdigit(*Buf)) {
            if (sscanf(Buf, "%d-%d", &new_dir, &rdir) != 2) {
              sendTo("Incorrect Form.  Should be direction-exitroom\n\r");
              return;
            }

            if ((new_dir = convertTo<int>(Buf)) < MIN_DIR || new_dir >= MAX_DIR) {
              sendTo(fmt("Direction must be between %d and %d.\n\r") % MIN_DIR % (MAX_DIR - 1));
              return;
            }
          } else {
            char tStEmpty[256];

            if (sscanf(Buf, "%s-%d", sstring, &rdir) != 2) {
              sendTo("Incorrect Form.  Should be direction-exitroom\n\r");
              return;
            }

            bisect_arg(sstring, &new_dir, tStEmpty, scandirs);
            new_dir--;

            if (new_dir < MIN_DIR || new_dir > MAX_DIR) {
              sendTo("Incorrect Direction name.\n\r");
              return;
            }
          }

          if (!(newrp = real_roomp(rdir))) {
            if (rdir < 0 || rdir >= WORLD_SIZE) {
              sendTo(fmt("Invalid room number.  Must be between 0 and %d.\n\r") %
                     (WORLD_SIZE - 1));
              return;
            }

	    if (!limitPowerCheck(CMD_EDIT, rdir)) {
	      sendTo("You are not allowed to make an exit out to that room, sorry.\n\r");
	      return;
	    }


            CreateOneRoom(rdir);

            if (!(newrp = real_roomp(rdir))) {
              sendTo(fmt("For some reason room %d neither exists nor could it be created.\n\r") %
                     rdir);
              return;
            }

            sendTo(fmt("You create room %d for the exit(s).\n\r") % rdir);
            newrp->setRoomFlags(roomp->getRoomFlags());
            newrp->setSectorType(roomp->getSectorType());
            newrp->setRoomHeight(roomp->getRoomHeight());
          }

          int rvs_dir = rev_dir[new_dir];

          if (!newrpTo->dir_option[new_dir]) {
            newrpTo->dir_option[new_dir] = new roomDirData();

            if (roomp->dir_option[new_dir]) {
              newrpTo->dir_option[new_dir]->door_type =
                roomp->dir_option[new_dir]->door_type;
              newrpTo->dir_option[new_dir]->condition =
                roomp->dir_option[new_dir]->condition;
              newrpTo->dir_option[new_dir]->lock_difficulty =
                roomp->dir_option[new_dir]->lock_difficulty;
              newrpTo->dir_option[new_dir]->weight =
                roomp->dir_option[new_dir]->weight;
              newrpTo->dir_option[new_dir]->key =
                roomp->dir_option[new_dir]->key;
            }

            newrpTo->dir_option[new_dir]->to_room = rdir;
          }

          if (!newrp->dir_option[rvs_dir]) {
            newrp->dir_option[rvs_dir] = new roomDirData();

            if (roomp->dir_option[new_dir]) {
              newrp->dir_option[rvs_dir]->door_type =
                roomp->dir_option[new_dir]->door_type;
              newrp->dir_option[rvs_dir]->condition =
                roomp->dir_option[new_dir]->condition;
              newrp->dir_option[rvs_dir]->lock_difficulty =
                roomp->dir_option[new_dir]->lock_difficulty;
              newrp->dir_option[rvs_dir]->weight =
                roomp->dir_option[new_dir]->weight;
              newrp->dir_option[rvs_dir]->key =
                roomp->dir_option[new_dir]->key;
            }

            newrp->dir_option[rvs_dir]->to_room = toRoom;
          }

          tBuf = tString;
          half_chop(tBuf, Buf, tString);
        }

        return;
      }

      // Normal creation process.
      if (sscanf(sstring, "%d %d %d %d %d %d %d", 
          &rdir, &dtype, &dcond, &dldiff, &dweight, &dkey, &exroom) != 7) {
        sendTo("Syntax : edit exit <dir> <door_type> <condition> <lock difficulty> <weight> <key> <exitroom>.\n\r");
        return;
      }
      if ((rdir < MIN_DIR) || (rdir >= MAX_DIR)) {
        sendTo(fmt("Direction must be between %d and %d.\n\r") % MIN_DIR % (MAX_DIR-1));
        return;
      }
      if ((dtype < DOOR_NONE) || (dtype >= MAX_DOOR_TYPES)) {
        sendTo(fmt("Door_type must be between %d and %d.\n\r") % DOOR_NONE % (MAX_DOOR_TYPES-1));
        return;
      }
      doortype = doorTypeT(dtype);
      if (roomp->dir_option[rdir]) {
        sendTo("modifying exit\n\r");
        roomp->dir_option[rdir]->door_type       = doortype;
        roomp->dir_option[rdir]->condition       = dcond;
        roomp->dir_option[rdir]->lock_difficulty = dldiff;
        roomp->dir_option[rdir]->weight          = dweight;
        roomp->dir_option[rdir]->key             = dkey;
        if (real_roomp(exroom)) {
          roomp->dir_option[rdir]->to_room = exroom;
          return;
        } else {
          sendTo("Deleting exit.\n\r");
          delete roomp->dir_option[rdir];
          roomp->dir_option[rdir] = 0;
          return;
        }
      }

      if (!limitPowerCheck(CMD_EDIT, exroom)) {
	sendTo("You are not allowed to make an exit out to that room, sorry.\n\r");
	return;
      }

      if (!real_roomp(exroom)) {
        if ((exroom > -1) && (exroom < WORLD_SIZE)) {
          sendTo("Exit room does not exist. Creating room....");
          CreateOneRoom(exroom);
          sendTo("Done.\n\r");
          sendTo("Setting up all flags for new room.\n\r");
          if (!(newrp = real_roomp(exroom))) {
            sendTo("Something went wrong, tell a coder!\n\r");
            return;
          }
          // If were creating a new room lets make it a small duplicate of
          // our current room.  Flags, Sector, and Room Height.
          newrp->setRoomFlags(roomp->getRoomFlags());
          newrp->setSectorType(roomp->getSectorType());
          newrp->setRoomHeight(roomp->getRoomHeight());	
	  make_room_coords(roomp, newrp, rdir);  
        } else {
          sendTo(fmt("Rooms need to be positive numbers less than %d.\n\r") % (WORLD_SIZE-1));
          return;
        }
      }
      sendTo("New exit\n\r");
      roomp->dir_option[rdir] = new roomDirData();
      roomp->dir_option[rdir]->door_type = doortype;
      roomp->dir_option[rdir]->condition = dcond;
      roomp->dir_option[rdir]->lock_difficulty = dldiff;
      roomp->dir_option[rdir]->weight = dweight;
      roomp->dir_option[rdir]->key = dkey;
      roomp->dir_option[rdir]->to_room = exroom;

      if ((roomp->dir_option[rdir]->door_type) != DOOR_NONE) {
        sstring[0] = 0;
        sendTo("Enter keywords, 1 line only. \n\r");
        sendTo("Terminate with a '~' on the SAME LINE.\n\r");
        desc->str = &roomp->dir_option[rdir]->keyword;
        break;
      }
      newrp = real_roomp(exroom);
      sendTo("Fixing opposite directions.\n\r");
      new_dir = rev_dir[rdir];
      if (newrp->dir_option[new_dir]) {
        sendTo("Exit back into room already exists...");
        if (newrp->dir_option[new_dir]->to_room == in_room)
          sendTo("And is back into the correct room.\n\r");
        else
          sendTo(fmt("And exits into incorrect room [%d].\n\r") % newrp->dir_option[new_dir]->to_room);
      } else {
        sendTo("Making new exit back into this room.\n\r");
        newrp->dir_option[new_dir] = new roomDirData();
        newrp->dir_option[new_dir]->to_room = in_room;
      }
      newrp->dir_option[new_dir]->keyword = roomp->dir_option[rdir]->keyword;
      newrp->dir_option[new_dir]->door_type = doortype;
      newrp->dir_option[new_dir]->condition = dcond;
      if (dcond & EX_SLOPED_UP) {
        newrp->dir_option[new_dir]->condition &= ~EX_SLOPED_UP;
        newrp->dir_option[new_dir]->condition |= EX_SLOPED_DOWN;
      }
      if (dcond & EX_SLOPED_DOWN) {
        newrp->dir_option[new_dir]->condition &= ~EX_SLOPED_DOWN;
        newrp->dir_option[new_dir]->condition |= EX_SLOPED_UP;
      }
      newrp->dir_option[new_dir]->lock_difficulty = dldiff;
      newrp->dir_option[new_dir]->weight = dweight;
      newrp->dir_option[new_dir]->key = dkey;
      return;
    case  4: // Extra Description
      if (!*sstring) {
        sendTo("You have to supply a keyword.\n\r");
        sendTo("Existing keywords:\n\r");
        for (ed = roomp->ex_description; ed; ed = ed->next) {
          sendTo(fmt("%s\n\r") % ed->keyword);
        }
        return;
      }
      // try to locate extra description 
      for (prev = ed = roomp->ex_description;; prev = ed, ed = ed->next)
        if (!ed) {
          ed = new extraDescription();
          ed->next = roomp->ex_description;
          roomp->ex_description = ed;
          ed->keyword = mud_str_dup(sstring);
          ed->description = NULL;
          desc->str = &ed->description;
          sendTo("New field.\n\r");
          sendTo("Terminate with a '~' on a NEW LINE.\n\r");
          break;
        } else if (!strcasecmp(ed->keyword, sstring)) {
          sendTo(fmt("Current description:\n\r%s\n\r") % ed->description);
          sendTo("This description has been deleted.  If you needed to modify it, simply readd it.\n\r");
          sendTo("Press return to proceed.\n\r");
          if (prev == ed) {
            roomp->ex_description = ed->next;
            delete ed;
          } else {
            prev->next = ed->next;
            delete ed;
          }
          return;
        }
      desc->max_str = MAX_STRING_LENGTH;
      return;
    case  5: // Flags
      int j;
      // If no argument was given, display all possible room flags and
      // there current state.
      if (!*sstring) {
        for (j = 0; j < MAX_ROOM_BITS; j++) {
          sendTo(fmt("%-2d [%s] %-30s%s") % (j + 1) %
                 ((roomp->getRoomFlags() & (1 << j)) ? "X" : " ") %
                 room_bits[j] % ((j % 2) == 1 ? "\n\r" : ""));
        }
        if (((j-1) % 2) != 1)
          sendTo("\n\r");
        return;
      }
      // If they didn't pass a number, it might have been a flag name or
      // rawbv.
      if (!is_number(sstring)) {
        char sstringB[2][256];
        const char *sstringC;
        sstringC = sstring;
        sstringC = one_argument(sstringC, sstringB[0], cElements(sstringB[0])); // rawbv/flag-name
                  one_argument(sstringC, sstringB[1], cElements(sstringB[1])); // on/off
        // Old style edit fs ???  format.
        if (is_abbrev(sstringB[0], "rawbv")) {
          sstringC = one_argument(sstringC, sstringB[1], cElements(sstringB[1]));
          if (sscanf(sstringB[1], "%lu", &r_flags) != 1) {
            sendTo("I'm afraid flags must be a number...\n\r");
            return;
          }
          roomp->setRoomFlags(r_flags);
          return;
        }
        // This is for: edit flags INDOORS on  mode forcing.
        if (*sstringB[1] && !is_abbrev("off", sstringB[1]) &&
                           !is_abbrev("on", sstringB[1])) {
          sendTo("Mode is on or off, nothing else...\n\r");
          return;
        }
        // Must have been a flag name, lets find it and toggle it.
        // Or force it if they gave us that.
        for (j = 0; j < MAX_ROOM_BITS; j++) {
          if (is_abbrev(room_bits[j], sstringB[0])) {
            // This is the value that flag actually represents.
            // 1, 2, 4, 8, 16, 32, ....
            r_flags = (1 << j);
            if (((*sstringB[1] && is_abbrev("off", sstringB[1])) ||
                !*sstringB[1]) && (roomp->getRoomFlags() & r_flags)) {
              sendTo(fmt("Removing Flag %s\n\r") % room_bits[j]);
              r_flags = roomp->getRoomFlags() - r_flags;
              roomp->setRoomFlags(r_flags);
              return;
            } else if (((*sstringB[1] && is_abbrev("on", sstringB[1])) ||
                       !*sstringB[1]) && !(roomp->getRoomFlags() & r_flags)) {
              sendTo(fmt("Adding Flag %s\n\r") % room_bits[j]);
              r_flags = roomp->getRoomFlags() + r_flags;
              roomp->setRoomFlags(r_flags);
              return;
            }
            sendTo("Flag was already set as you wanted it...\n\r");
            return;
          }
        }
        sendTo("Was unable to find the flag you requested...\n\r");
        return;
      }
      if (sscanf(sstring, "%lu", &r_flags) != 1 || r_flags < 1 ||
          r_flags >= MAX_ROOM_BITS) {
        sendTo("Invalid flags...try again.\n\r");
        return;
      }
      // This is where we get a bit condensed.
      // If flag number is 1 then we actually want the flag value
      // for flag 0, flag - 1, which is (1 << (flag-1))
      r_flags--;
      // First.  Are we removing or adding it?  Give messages depending.
      sendTo(fmt("%s flag %s.\n\r") %
             ((roomp->getRoomFlags() & (1 << r_flags)) ?
              "Removing" : "Adding") %
             room_bits[r_flags]);
      // We either add it to or remove it from the room flags.
      r_flags = (1 << r_flags);

      if (roomp->isRoomFlag(r_flags))
        roomp->setRoomFlagBit(r_flags);
      else
        roomp->removeRoomFlagBit(r_flags);

      roomp->setRoomFlags(r_flags);
      return;
      break;
    case  6: // Height
      if (sscanf(sstring, "%d", &rheight) != 1 ||
          (rheight < -1 || rheight == 0 || rheight > 1000)) {
        sendTo("Invalid height...try again.\n\r");
        sendTo("A height of -1 means Unlimited.\n\r");
        sendTo("Indoor room height is 1 through 1000, 0 is Invalid.\n\r");
        return;
      }
      if (rheight == -1) {
        if (roomp->isRoomFlag(ROOM_INDOORS)) {
          sendTo("Room set Indoors, removing INDOORS flag...\n\r");
          roomp->removeRoomFlagBit(ROOM_INDOORS);
          roomp->initLight();
        }
      } else {
        if (!roomp->isRoomFlag(ROOM_INDOORS)) {
          sendTo("Room not set Indoors, adding INDOORS flag...\n\r");
          roomp->setRoomFlagBit(ROOM_INDOORS);
          roomp->initLight();
        }
      }
      sendTo(fmt("Setting Room Height to %d.\n\r") % rheight);
      roomp->setRoomHeight(rheight);
      return;
      break;
    case  7: // Line
      /*
      if (strcmp(getName(), "Lapsos")) {
        sendTo("This option is not yet coded.  Soon...\n\r");
        return;
      }
      */

      // Works like:
      //   edit line 100 n 101 e 102 s 103
      //     Create an exit from room 100 North into room 101
      //     Create an exit from room 101 East  into room 102
      //     Create an exit from room 102 South into room 103

      // Get the first room in the list.
      tBuf = sstring;
      for (; isspace(*tBuf); tBuf++);
      half_chop(tBuf, Buf, sstring);
      tBuf = sstring;
      r_flags = 0;
      // Loop while we got rooms to do.
      if (!*Buf) {
        sendTo("Please give me a list of rooms and exits.  See help for more info.\n\r");
        return;
      }
      while (*Buf && ++r_flags < 15) {
        cRoom = convertTo<int>(Buf);

        // Make sure room number is valid.  Must be a number, and between
        // 0 and WORLD_SIZE.
        if (is_number(Buf) && (cRoom > -1) && (cRoom < WORLD_SIZE)) {
          // Does the room in question exist?  If not, make it.
          if (!(newrp = real_roomp(cRoom))) {
            CreateOneRoom(cRoom);
            // This Can happen, such as running out of memory or something else
            // just as critical.  Safty first.
            if (!(newrp = real_roomp(cRoom))) {
              sendTo("Something went wrong, tell a coder.\n\r");
              return;
            }
            newrp->setRoomFlags(roomp->getRoomFlags());
            newrp->setSectorType(roomp->getSectorType());
            newrp->setRoomHeight(roomp->getRoomHeight());
          }
          if (zone_table[newrp->getZoneNum()].enabled &&
              !hasWizPower(POWER_REDIT_ENABLED) &&
              newrp->getZoneNum() != ROOM_IMPERIA) {
            sendTo("I'm afraid you cannot use this field on an enabled zone.\n\r");
            return;
          }

          // Now we get the exit direction.
          half_chop(tBuf, Buf, sstring);
          tBuf = sstring;
          if (!*Buf) {
            sendTo("And which direction should this exit go?\n\r");
            return;
          }
          for (dir = MIN_DIR; dir < MAX_DIR; dir++)
            if (is_abbrev(scandirs[dir], Buf))
              break;
          if (dir == MAX_DIR) {
            sendTo("Incorrect Direction, Try again.\n\r");
            return;
          }

          // Now we get the Exit room, this will also be our start room on
          // the next loop through.
          half_chop(tBuf, Buf, sstring);
          tBuf = sstring;
          new_dir = convertTo<int>(Buf);
          // Now we check the new room.
          if (is_number(Buf) && (new_dir > -1) && (new_dir < WORLD_SIZE)) {
            if (!(newrpTo = real_roomp(new_dir))) {
              CreateOneRoom(new_dir);
              if (!(newrpTo = real_roomp(new_dir))) {
                sendTo("Something went wrong, tell a coder.\n\r");
                return;
              }
              newrpTo->setRoomFlags(newrp->getRoomFlags());
              newrpTo->setSectorType(newrp->getSectorType());
              newrpTo->setRoomHeight(newrp->getRoomHeight());
            }
            if (zone_table[newrpTo->getZoneNum()].enabled &&
                !hasWizPower(POWER_REDIT_ENABLED) &&
                newrpTo->getZoneNum() != ROOM_IMPERIA) {
              sendTo("I'm afraid you cannot use this field on an enabled zone.\n\r");
              return;
            }
            if (newrp == newrpTo) {
              sendTo("Shouldn't try and make an exit back into the same room.\n\r");
              return;
            }

            // ... Actual Exit Creation Start

            // Create initial exit.
            if (!newrp->dir_option[dir])
              newrp->dir_option[dir] = new roomDirData();
            else {
              TRoom *zRoom;
              if (!(zRoom = real_roomp(newrp->dir_option[dir]->to_room))) {
                sendTo("Error.  Exit exists but exit room Doesn't!\n\r");
                return;
              } else {
                delete zRoom->dir_option[rev_dir[dir]];
                zRoom->dir_option[rev_dir[dir]] = NULL;
              }
            }
            newrp->dir_option[dir]->door_type       = DOOR_NONE;
            newrp->dir_option[dir]->condition       = 0;
            newrp->dir_option[dir]->lock_difficulty = 0;
            newrp->dir_option[dir]->weight          = 0;
            newrp->dir_option[dir]->key             = -1;
            newrp->dir_option[dir]->trap_info       = 0;
            newrp->dir_option[dir]->to_room         = new_dir;
            newrp->dir_option[dir]->description     = NULL;
            newrp->dir_option[dir]->keyword         = NULL;

            // Create exit back.
            dir = rev_dir[dir];
            if (!newrpTo->dir_option[dir])
              newrpTo->dir_option[dir] = new roomDirData();
            else {
              TRoom *zRoom;
              if (!(zRoom = real_roomp(newrpTo->dir_option[dir]->to_room))) {
                sendTo("Error.  Exit exists but exit room Doesn't!\n\r");
                return;
              } else {
                delete zRoom->dir_option[rev_dir[dir]];
                zRoom->dir_option[rev_dir[dir]] = 0;
              }
            }
            newrpTo->dir_option[dir]->door_type       = DOOR_NONE;
            newrpTo->dir_option[dir]->condition       = 0;
            newrpTo->dir_option[dir]->lock_difficulty = 0;
            newrpTo->dir_option[dir]->weight          = 0;
            newrpTo->dir_option[dir]->key             = -1;
            newrpTo->dir_option[dir]->trap_info       = 0;
            newrpTo->dir_option[dir]->to_room         = cRoom;
            newrpTo->dir_option[dir]->description     = NULL;
            newrpTo->dir_option[dir]->keyword         = NULL;

            // ... Actual Exit Creation Ending


          } else {
            if (!is_number(Buf))
              sendTo("Exit room must be a number.\n\r");
            else
              sendTo("Exit room number incorrect.\n\r");
            return;
          }
        } else {
          if (!is_number(Buf))
            sendTo("Room must be a number.\n\r");
          else
            sendTo("Room number incorrect.\n\r");
          return;
        }
        if (!*sstring) {
          sendTo("Done.\n\r");
          return;
        }
      }
      return;
      break;
    case  8: // Max_Capacity
      if (sscanf(sstring, "%d", &moblim) != 1) 
        sendTo("edit max_capacity <mob_limit>\n\r");
      else 
        roomp->setMoblim(moblim);
      return;
      break;
    case  9: // Name
      tStString = sstring;
      stSpaceOut(tStString);

      if (!tStString.empty()) {
        delete [] roomp->name;
        sendTo(fmt("New Room Title: %s\n\r") % tStString);
        roomp->name = mud_str_dup(tStString);

        return;
      }

      desc->str = &roomp->name;
      break;
    case 10: // River
      rspeed = 0;
      rdir = 0;
      sscanf(sstring, "%d %d ", &rspeed, &rdir);
      if ((rdir >= MIN_DIR) && (rdir < MAX_DIR)) {
        roomp->setRiverSpeed(rspeed);
        roomp->setRiverDir(mapFileToDir(rdir));
      } else
        sendTo(fmt("Direction must be between %d and %d.\n\r") % MIN_DIR % (MAX_DIR-1));
      return;
      break;
    case 11: // Sector_Type
      // If no argument was given, display all possible sector types.
      if (!*sstring) {
        int j;
        for (j = 0; j < MAX_SECTOR_TYPES; j++) {
          if (!*TerrainInfo[j]->name)
            continue;

          sendTo(fmt("%-2d %-26s%s") % (j+1) % TerrainInfo[j]->name %
                 ((j % 3) == 2 ? "\n\r" : ""));
        }
        if (((j-1) % 3) != 2)
          sendTo("\n\r");
        return;
      }
      sscanf(sstring, "%d", &s_type);
      s_type--;
      if ((s_type < 0) || (s_type >= MAX_SECTOR_TYPES)) {
        sendTo("That sector choice is invalid, please try again.\n\r");
        return;
      }
      sectype = sectorTypeT(s_type);
      roomp->setSectorType(sectype);

      if (roomp->isWaterSector()) {
        sendTo("You should set the speed and flow for this river.\n\r");
        sendTo("Setting to 0(north) and 0(speed) for default.\n\r");
        roomp->setRiverSpeed(0);
        roomp->setRiverDir(DIR_NORTH);
      }
      return;
      break;
    case 12: // Teleport
      tele_room = -1;
      tele_time = -1;
      tele_look = -1;
      sscanf(sstring, "%d %d %d", &tele_time, &tele_room, &tele_look);
      if (tele_room < 0 || tele_time < 0 || tele_look < 0) 
        sendTo("edit tele <time> <room_nr> <look-flag>\n\r");
      else {
        roomp->setTeleTime(tele_time);
        roomp->setTeleTarg(tele_room);
        roomp->setTeleLook(tele_look);
      }
      return;
      break;
    case 13: // Copy
      /*
      if (strcmp("Lapsos", getName())) {
        sendTo("Please don't use this option yet, it is still being tested.\n\r");
        return;
      }
      */

      // edit copy <field> <rooms>
      // What we do is this:
      // Set tBuf to <<field> <rooms>>
      // dissect it to get <field> then set tBuf to <rooms>
      // Then we just make sure we got some rooms and not a lot of spaces.
      tBuf  = sstring;
      bisect_arg(tBuf, &field, sstring, copy_fieldT);
      cRoom = -1;
      tBuf  = sstring;
      for (; isspace(*tBuf); tBuf++);

      // Means we went out of bounds from what copy_fieldT has.
      if (field > 8 ) {
        sendTo("I didn't quite understand that, perhaps you will try again?\n\r");
        return;
      }
      // No room list.
      if (!*tBuf || !tBuf) {
        sendTo("I'm afraid you need to give me a list of rooms.\n\r");
        return;
      }
      // 1 == Description, do we have one?
      if ((field == 1) && !roomp->getDescr()) {
        sendTo("This room doesn't have a description, nothing to copy!\n\r");
        return;
      }
      // 2 == Name, do we have one?
      if ((field == 2) && !roomp->name) {
        sendTo("No name on this room, nothing to copy!\n\r");
        return;
      }
      // 3 == Extra Descriptions, do we have at least one?
      if ((field == 3) && !roomp->ex_description) {
        sendTo("No extra descriptions to copy.\n\r");
        return;
      }
      // Get the first room in the list.
      half_chop(tBuf, Buf, sstring);
      tBuf = sstring;
      r_flags = 0;
      // Loop while we got rooms to do.
      while (*Buf && ++r_flags < 15) {
        cRoom = convertTo<int>(Buf);

        // Make sure room number is valid.  Must be a number, and between
        // 0 and WORLD_SIZE, and we don't want to modify OUR room.
        if (is_number(Buf) && (cRoom > -1) && (cRoom < WORLD_SIZE) &&
            cRoom != roomp->number) {

          // Does the room in question exist?  If not, make it.
          if (!(newrp = real_roomp(cRoom))) {
            CreateOneRoom(cRoom);
            // This Can happen, such as running out of memory or something else
            // just as critical.  Safty first.
            if (!(newrp = real_roomp(cRoom))) {
              sendTo("Something went wrong, tell a coder.\n\r");
              return;
            }
          }
          if (zone_table[newrp->getZoneNum()].enabled &&
              !hasWizPower(POWER_REDIT_ENABLED) &&
              newrp->getZoneNum() != ROOM_IMPERIA) {
            sendTo("I'm afraid you cannot use this field on an enabled zone.\n\r");
            continue;
          }

          if (field == 1 || (field == 8 && roomp->getDescr())) { // Description
            if (newrp->getDescr())
              delete [] newrp->descr;
            newrp->descr = mud_str_dup(roomp->getDescr());
          }
          if (field == 2 || (field == 8 && roomp->name)) { // Name
            if (newrp->name)
              delete [] newrp->name;
            newrp->name = mud_str_dup(roomp->name);
          }
          if (field == 3 || (field == 8 && roomp->ex_description)) { // Extra Descriptions
            extraDescription *teDesc = NULL;
            // Go through our rooms extra descriptions one at a time.
            for (ed = roomp->ex_description; ed; ed = ed->next) {
              // We also make sure were not adding this to a room that already
              // HAS an extra by those trigger words.
              for (prev = newrp->ex_description; prev; teDesc = prev, prev = prev->next) {
                // Check for simularity.
                if (!strcasecmp(ed->keyword, prev->keyword)) {
                  // We need to get rid of this one, because were going to overwrite it.
                  // First alienate it, then delete it.
                  if (teDesc)
                    teDesc->next = prev->next;
                  else
                    newrp->ex_description = prev->next; 
                  delete prev;
                  break;
                  //prev = teDesc->next;
                }
              }
              // Copy this extra to the new room.
              prev = new extraDescription();
              prev->next = newrp->ex_description;
              newrp->ex_description = prev;
              prev->keyword = mud_str_dup(ed->keyword);
              prev->description = mud_str_dup(ed->description);
            }
          }
          if (field == 4 || field == 8) { // Flags
            newrp->setRoomFlags(roomp->getRoomFlags());
          }
          if (field == 5 || field == 8) { // Sector Type
            newrp->setSectorType(roomp->getSectorType());
          }
          if (field == 6 || field == 8) { // Height
            newrp->setRoomHeight(roomp->getRoomHeight());
          }
          if (field == 7 || field == 8) { // Capacity
            newrp->setMoblim(roomp->getMoblim());
          }
        } else {
          if (!is_number(Buf))
            sendTo("Room must be a number.\n\r");
          else if (cRoom == roomp->number)
            sendTo("Doesn't work to copy something back onto itself...\n\r");
          else
            sendTo("Room number incorrect.\n\r");
        }
        // Get the next room in the list.
        half_chop(tBuf, Buf, sstring);
        tBuf = sstring;
      }
      sendTo("Done.\n\r");
      if (r_flags >= 15)
        sendTo("Edit copy hit end marker, error apparently...\n\r");
      return;
      break;
    case 14: // edit replace <desc/extra> <"extra"/"text"> <"text"> <"text">
      /*
      if (strcmp("Lapsos", getName())) {
        sendTo("Please don't use this option yet, it is still being tested.\n\r");
        return;
      }
      */

      strcpy(tTextLns[0], "[]A-Za-z0-9~`!@#$%&*()_+-={}[;\':,./<>? ]");
      sprintf(Buf, "%%s \"%%%s\" \"%%%s\" \"%%%s\"", tTextLns[0], tTextLns[0], tTextLns[0]);
      tTextLns[0][0] = '\0';

      dcond = sscanf(sstring, Buf, tTextLns[0], tTextLns[1], tTextLns[2], tTextLns[3]);

      if (((!is_abbrev(tTextLns[0], "description") ||                    dcond < 2) &&
           (!is_abbrev(tTextLns[0], "extra"      ) || !tTextLns[2][0] || dcond < 3)) ||
          !tTextLns[1][0]) {
        sendTo("Syntax: edit replace <desc/extra> <\"extra\"/\"text\"> <\"text\"> <\"text\">\n\r");
        return;
      }

      if (is_abbrev(tTextLns[0], "description")) {
        if (!roomp->descr) {
          sendTo("Room doesn't have a description, cannot use replace.\n\r");
          return;
        }

        tStr = roomp->descr;

        if (tStr.find(tTextLns[1]) == sstring::npos) {
          sendTo("Couldn't find pattern in description.\n\r");
          return;
        }

        tStr.replace(tStr.find(tTextLns[1]), strlen(tTextLns[1]), tTextLns[2]);

        delete [] roomp->descr;
        roomp->descr = mud_str_dup(tStr);
      } else {
        for (ed = roomp->ex_description, s_type = 1; ed; ed = ed->next) {
          if (isname(tTextLns[1], ed->keyword)) {
            tStr = ed->description;
            s_type = 0;
          }

          if (s_type == 0)
            break;
        }

        if (!ed) {
          sendTo("Wasn't able to find an extra by that name.\n\r");
          return;
        }

        if (tStr.find(tTextLns[2]) == sstring::npos) {
          sendTo("Couldn't find pattern in extra description.\n\r");
          return;
        }

        tStr.replace(tStr.find(tTextLns[2]), strlen(tTextLns[2]), tTextLns[3]);

        delete [] ed->description;
        ed->description = mud_str_dup(tStr);
      }
      return;
      break;
    case 15: // edit list <2>
      sprintf(tString, "immortals/%s/rooms%s",
              getNameNOC(this).cap().c_str(),
              (*sstring ? "_2" : ""));

      if (!(tFile = fopen(tString, "r"))) {
        sendTo(fmt("You don't have a %srooms file.\n\r") %
               (*sstring ? "2nd " : ""));
      } else {
        tStr = "Room List:\n\r";

        while (!feof(tFile)) {
          if (fscanf(tFile, "#%d\n\r", &cRoom) != 1) {
            fgets(tString, 256, tFile);
            continue;
          }

          sprintf(tString, "%6d: ", cRoom);
          tStr += tString;
          tBuf = fread_string(tFile);

          if (tBuf && *tBuf) {
            tStr += tBuf;
            tStr += "\n\r";
          } else
            tStr += "Unknown\n\r";

          if (tBuf) {
            delete [] tBuf;
            tBuf = NULL;
          }
	}

        desc->page_string(tStr);
      }
      return;
      break;
    case 16:
      //dash marker
      if (!roomp->descr) {
	sendTo("Room doesn't have a description, cannot use autoformatter.\n\r");
	return;
      }

      regStr = roomp->descr;

      line = "  "; // intial extra space

      for(int i=0;!regStr.word(i).empty();++i){
        // count the number of unprintable characters in each word
        word = regStr.word(i);
        stripped_word = stripColorCodes(word);
        swlen = stripped_word.length();

        if ((line.length() + 1) + (word.length() + 1) >= 80) {
          size_t last_char = 0;

	  line += "\n\r";
          newDescr += line;

          // check if the word ends with punctuation
          stripped_word += " ";
          last_char = stripped_word.find_last_not_of(" ");

          if (stripped_word.find_first_of(punctuation, last_char) !=
              stripped_word.npos) {
            word += " "; // and add an extra space to the end.
          }
          line = word;
        } else { // word fits ok on line
          size_t last_char = 0;

          // check if the word ends with punctuation
          stripped_word += " ";
          last_char = stripped_word.find_last_not_of(" ");

          if (stripped_word.find_first_of(punctuation, last_char) !=
              stripped_word.npos) {
            word += " "; // and add an extra space to the end.
          }
          // if the length of the stripped word > 0
          // and the previous word was a real word, then append a space to the
          // line before appending the word.
          if (swlen) {
            if (was_word) {
              line += " ";
            }
            was_word = true;
          } else {
            was_word = false;
          }
          line += word;
        }
      }

      newDescr += line;
      newDescr += "\n\r";

      delete [] roomp->descr;
      roomp->descr = mud_str_dup(newDescr);

      sendTo("Room has been formatted.\n\r");

#if 0
      tStr = roomp->descr;

      // Ok what do we want here:
      // A) Description should start with two spaces
      // B) There should be two spaces and only two spaces after each sentence end
      // C) Lines should be no longer than 80 characters, and should not break in the
      //    middle of a word.
      
      whitespace = " \n\r";

      punctuation = ".!?;:"; 
      colors = "<";

      size_t bgin, look_at;

      line = " "; // intial extra spaces
      while (tStr != "") {


	bgin = 0;

	bgin = tStr.find_first_not_of(whitespace);
	look_at = tStr.find_first_of(whitespace, bgin);
	
	if (look_at != sstring::npos) {
	  // normal, return the part between
	  word = tStr.substr(bgin, look_at - bgin);
	  a2 = tStr.substr(look_at);
	  tStr = a2;
	} else if (bgin != sstring::npos) {
	  // sstring had no terminator
	  word = tStr.substr(bgin);
	  tStr = "";
	} else {
	  // whole sstring was whitespace
	  word = "";
	  tStr = "";
	}

        num_ctags_word = 0;
	wbgin = word.find_first_of(colors);
        cwordend = word.npos;
        while (wbgin != cwordend) {
          num_ctags_word++;
          wbgin = word.find_first_of(colors, wbgin+1);
        }

	if ((line.length() + 1) + (word.length() + 1) >= 80) {// word is too long, end line and start on next
          // if this is the last word of the desc, don't append a CR/LF
          if (tStr != "" ) {
            line += "\n\r";
          }
	  newDescr += line;
	  line = word;
	} else { // word fits ok on line
	  if (word.find_first_of(punctuation) != sstring::npos) { // word has punctuation
	    word += " "; // so add extra spaces to the end.
	    // note: this is sort of a hack, because words like sjdgh:jdsgh will trigger it...
	    // but if they want to put crap like that in they can format it themselves, damnit.
	  }
          // if the word contains only color codes, don't append a space
          // to the line.
          if ((word.length() != (3 * num_ctags_word))) {
            line += " ";
          }
	  line += word;
	}
      }
      newDescr += line;
      newDescr += "\n\r";
     

      delete [] roomp->descr;
      roomp->descr = mud_str_dup(newDescr);

      sendTo("Room has been formatted.\n\r");
#endif

      return;
      break;
    default:
      sendTo("I'm so confused :-)\n\r");
      return;
      break;
  }

  if (*desc->str) 
    delete [] (*desc->str);
  
  if (*sstring) {		// there was a sstring in the argument array 
    if (strlen(sstring) > (unsigned int) room_length[field - 1]) {
      sendTo("String too long - truncated.\n\r");
      *(sstring + room_length[field - 1]) = '\0';
    }
    *(desc->str) = mud_str_dup(sstring);
    sendTo("Ok.\n\r");
  } else {			// there was no sstring. enter sstring mode 
    sendTo(fmt("Enter sstring.  Terminate with '~' on %s.\n\r") %
           ((field == 1 || field == 2) ? "NEW LINE" : "SAME LINE"));
    *desc->str = 0;
    desc->max_str = room_length[field - 1];
  }
}

// Below are the saving and loading commands for the rooms 

void TBeing::doRload(const char *)
{
  sendTo("Yeah, right.   Stupid mobs!\n\r");
}

void TPerson::doRload(const char *argument)
{
  int    tStart = 0,
         tEnd = 0;
  int   tSec = 1;
  sstring tStArg(argument),
         tStString(""),
         tStBuffer("");

  if (!hasWizPower(POWER_RLOAD)) {
    incorrectCommand();
    return;
  }

  if (!desc || !isImmortal())
    return;

  stSpaceOut(tStArg);
  tStString=tStArg.word(0);
  tStBuffer=tStArg.word(1);

  if (tStString.empty() || tStString[0] == '1') {
    tStart = desc->blockastart;
    tEnd   = desc->blockaend;
    tSec=1;
  } else if (tStString[0] == '2') {
    tStart = desc->blockbstart;
    tEnd   = desc->blockbend;
    tSec=2;
  } else if (is_abbrev(tStString, "backup")) {
    sstring tStExtra(""),
           tStStandard("");
    bool   tStandard = false;

    tStExtra=tStBuffer.word(0);
    tStStandard=tStBuffer.word(1);

    if (!tStExtra.empty()) {
      if (tStExtra[0] == '1') {
        if (!tStStandard.empty() && is_abbrev(tStStandard, "standard"))
          tStandard = true;
      } else if (tStExtra[0] == '2') {
	tSec=2;

        if (!tStStandard.empty() && is_abbrev(tStStandard, "standard"))
          tStandard = true;
      } else if (is_abbrev(tStExtra, "standard"))
        tStandard = true;
      else {
        sendTo("Syntax: redit load backup <\"1\"/\"2\"/\"standard\"> <\"standard\">\n\r");
        return;
      }
    }

    // backup is a normal save with block+100
    if (!tSec){
      tStart = desc->blockastart;
      tEnd   = desc->blockaend;
      tSec=101;
    } else {
      tStart = desc->blockbstart;
      tEnd   = desc->blockbend;
      tSec=102;
    }

    sendTo("Restoring backup.\n\r");
  } else {
    sendTo("Syntax: redit load <\"1\"/\"2\">\n\r");
    return;
  }

  if (tStart <= 0 || tEnd <= 0) {
    sendTo("You have no rooms assigned in that block...Sorry.\n\r");
    return;
  }

  if (tStart > tEnd)
    sendTo("Your room block is messed up.  Talk with Head Low immediatly!\n\r");
  else
    RoomLoad(this, tStart, tEnd, tSec);
}

void TBeing::doRsave(const char *)
{
  sendTo("Mobs can't save rooms.\n\r");
}

// Below is the vt100 room editor written by DM of SillyMUD and Epic  
// fame. All changes have been done by myself, or Batopr - Russ                  

static const char *exit_menu = "    1) North                      2) East\n\r"
"    3) South                      4) West\n\r"
"    5) Up                         6) Down\n\r"
"    7) Northeast                  8) Northwest\n\r"
"    9) Southeast                 10) Southwest\n\r"
"\n\r";

static void ChangeMaxCap(TRoom *rp, TBeing *ch, const char *arg, editorEnterTypeT type)
{
  int num;

  num = convertTo<int>(arg);

  if (type != ENTER_CHECK)
    if (!*arg || (*arg == '\n')) {
      ch->specials.edit = MAIN_MENU;
      update_room_menu(ch);
      return;
    }
  if (type != ENTER_CHECK) {
    if ((num < 0) || (num > 100)) {
      ch->sendTo("Please enter a number from 0 - 100.\n\r");
      ch->sendTo("Entering 0 will make the room have an infinite capacity.\n\r");
      return;
    }
    rp->setMoblim(num);
    ch->specials.edit = MAIN_MENU;
    update_room_menu(ch);
    return;
  }
  ch->sendTo(VT_HOMECLR);
  ch->sendTo(fmt("Current room maximum capacity: %d") % rp->getMoblim());
  ch->sendTo("\n\r\n\rNew maximum capacity: ");
  return;
}

static void ChangeRoomHeight(TRoom *rp, TBeing *ch, const char *arg, editorEnterTypeT type)
{
  int num = convertTo<int>(arg);

  if (type != ENTER_CHECK)
    if (!*arg || (*arg == '\n')) {
      ch->specials.edit = MAIN_MENU;
      update_room_menu(ch);
      return;
    }
  if (type != ENTER_CHECK) {
    if ((num < -1) || (num > 1000) || (num == 0)) {
      ch->sendTo("Please enter a number from -1 to 1000.\n\r");
      ch->sendTo("Entering -1 will make the room have an unlimited height.\n\r");
      ch->sendTo("Unlimited heights are for outdoor rooms ONLY.\n\r");
      ch->sendTo("Do not set a height of 0.\n\r");
      return;
    }
    rp->setRoomHeight(num);
    ch->specials.edit = MAIN_MENU;
    update_room_menu(ch);
    return;
  }
  ch->sendTo(VT_HOMECLR);
  ch->sendTo(fmt("Current room height: %d\n\r\n\r") % rp->getRoomHeight());
  ch->sendTo("A value of -1 implies an unlimited height.\n\r");
  ch->sendTo("Unlimited heights are for outdoor rooms ONLY.\n\r");
  ch->sendTo("\n\r\n\rNew Room Height: ");
  return;
}

static void ChangeRoomFlags(TRoom *rp, TBeing *ch, const char *arg, editorEnterTypeT type)
{
  int row, update;
  char buf[255];
  unsigned int i;
  int j;

  if (type != ENTER_CHECK) {
    if (!*arg || (*arg == '\n')) {
      ch->specials.edit = MAIN_MENU;
      update_room_menu(ch);
      return;
    }
  }
  update = convertTo<int>(arg);
  update--;
  if (type != ENTER_CHECK) {
    if (update < 0 || update >= MAX_ROOM_BITS)
      return;
    i = 1 << update;
    if (i == ROOM_BEING_EDITTED) {
      ch->sendTo("No, no, bad!\n\r");
      return;
    }

    if (rp->isRoomFlag(i)) {
      if (i == ROOM_INDOORS) {
        ch->sendTo("Reiniting lights and setting room height unlimited.\n\r");
	rp->initLight();
        rp->setRoomHeight(-1);
      }
      rp->removeRoomFlagBit(i);
    } else {
      if (i == ROOM_INDOORS) {
        ch->sendTo("Reiniting lights and setting default room height.\n\r");
	rp->initLight();
        rp->setRoomHeight(100);
      }
      rp->setRoomFlagBit(i);
    }
  }
  ch->sendTo(VT_HOMECLR);
  ch->sendTo("Room Flags:");

  row = 0;
  for (j = 0; j < MAX_ROOM_BITS; j++) {
    sprintf(buf, VT_CURSPOS, row + 4, ((j & 1) ? 45 : 5));
    if (j & 1)
      row++;
    ch->sendTo(buf);
    ch->sendTo(fmt("%-2d [%s] %s") % (j + 1) % ((rp->getRoomFlags() & (1 << j)) ? "X" : " ") % room_bits[j]);
  }
  ch->sendTo(fmt(VT_CURSPOS) % 20 % 1);
  // could have just made the saveroom flag an invalid option but someday someone will
  // need it unset so i added a warning about keeping the flag set
  ch->sendTo("Make sure the SAVE ROOM flag is set unless you KNOW you don't need it set.\n\r");
  ch->sendTo("If you aren't sure that you want the SAVE ROOM flag on, then you do!\n\r");
  ch->sendTo("Select the number to toggle, <C/R> to return to main menu.\n\r--> ");
}

void TBeing::doRedit(const char *)
{
  sendTo("Mobs can't edit rooms.\n\r");
}

void TPerson::doRedit(const char *argument)
{
  if (!hasWizPower(POWER_REDIT)) {
    incorrectCommand();
    return;
  }
  if (!desc || (!isImmortal()) || !roomp)
    return;

  if (!in_range(roomp->number, desc->blockastart, desc->blockaend) &&
      !in_range(roomp->number, desc->blockbstart, desc->blockbend) &&
      roomp->number != desc->office && !hasWizPower(POWER_REDIT_ENABLED)) {
    sendTo("This room is not yours...Sorry\n\r");
    return;
  }

  if (!limitPowerCheck(CMD_REDIT, roomp->number)) {
    sendTo("You are not allowed to redit this room, sorry.\n\r");
    return;
  }



  for (; isspace(*argument); argument++);
  if (*argument) {
    doEdit(argument);
    return;
  }

  if (roomp->isRoomFlag(ROOM_BEING_EDITTED)) {
    sendTo("Someone is already editing this room, sorry.\n\r");
    return;
  }

  stopFollower(false);
  specials.edit = MAIN_MENU;
  desc->connected = CON_REDITING;
  roomp->setRoomFlagBit(ROOM_BEING_EDITTED);
  act("$n has begun editing.", TRUE, this, 0, 0, TO_ROOM);
  update_room_menu(this);
}

static void ChangeRoomName(TRoom *rp, TBeing *ch, const char *arg, editorEnterTypeT type)
{
  if (type != ENTER_CHECK) {
    if (!*arg || (*arg == '\n')) {
      ch->specials.edit = MAIN_MENU;
      update_room_menu(ch);
      return;
    }
    delete [] rp->name;
    rp->name = mud_str_dup(arg);
    ch->specials.edit = MAIN_MENU;
    update_room_menu(ch);
    return;
  }
  ch->sendTo(VT_HOMECLR);

  ch->sendTo(fmt("Current Room Name: %s") % rp->name);
  ch->sendTo("\n\r\n\rNew Room Name: ");

  return;
}

static void ChangeRoomDesc(TRoom *rp, TBeing *ch, const char *, editorEnterTypeT type)
{
  sstring descr_str;

  descr_str = rp->getDescr();
  descr_str = descr_str.toCRLF();

  if (type != ENTER_CHECK) {
    ch->specials.edit = MAIN_MENU;
    update_room_menu(ch);
    return;
  }
  ch->sendTo(VT_HOMECLR);

  ch->sendTo("Current Room Description:\n\r");
  ch->sendTo(descr_str.c_str());
  ch->sendTo("\n\r\n\rNew Room Description:\n\r");
  ch->sendTo("(Terminate with a ~ on a NEW LINE. Press <C/R> again to continue)\n\r");
  delete [] rp->getDescr();
  rp->setDescr(NULL);
  ch->desc->str = &rp->descr;
  ch->desc->max_str = MAX_STRING_LENGTH;
  return;
}

static void ChangeRoomType(TRoom *rp, TBeing *ch, const char *arg, editorEnterTypeT type)
{
  int row, update;
  char buf[255];

  if (type != ENTER_CHECK)
    if (!*arg || (*arg == '\n')) {
      ch->specials.edit = MAIN_MENU;
      update_room_menu(ch);
      return;
    }
  update = convertTo<int>(arg);
  update--;

  if (type != ENTER_CHECK) {
    switch (ch->specials.edit) {
      case CHANGE_ROOM_TYPE:
	if (update < 0 || update >= MAX_SECTOR_TYPES)
	  return;
	else {
	  rp->setSectorType(sectorTypeT(update));
          if (rp->isWaterSector()) {
	    ch->sendTo("\n\rRiver Speed: ");
	    ch->specials.edit = CHANGE_ROOM_TYPE2;
	  } else {
	    ch->specials.edit = MAIN_MENU;
	    update_room_menu(ch);
	  }
	  return;
	}
      case CHANGE_ROOM_TYPE2:
	rp->setRiverSpeed(update + 1);
	ch->sendTo(fmt("\n\rRiver Direction (%d - %d): ") % MIN_DIR % (MAX_DIR-1));
	ch->specials.edit = CHANGE_ROOM_TYPE3;
	return;
      case CHANGE_ROOM_TYPE3:
	update++;
	if (update < MIN_DIR || update >= MAX_DIR) {
	  ch->sendTo(fmt("Direction must be between %d and %d.\n\r") % MIN_DIR % (MAX_DIR-1));
	  return;
	}
	rp->setRiverDir(dirTypeT(update));
	ch->specials.edit = MAIN_MENU;
	update_room_menu(ch);
	return;
      default:
        return;
    }
  }
  ch->sendTo(VT_HOMECLR);
  ch->sendTo(fmt("Sector Type: %s") % TerrainInfo[rp->getSectorType()]->name);

  row = 0;
  sectorTypeT i;
  for (i = MIN_SECTOR_TYPE; i < MAX_SECTOR_TYPES; i++) {
    if (!*TerrainInfo[i]->name)
      continue;
    sprintf(buf, VT_CURSPOS, row + 2, (((i % 3) * 25) + 5));
    if ((i%3) == 2)
      row++;
    ch->sendTo(buf);
    ch->sendTo(fmt("%-2d %s") % (i+1) % TerrainInfo[i]->name);
  }
  ch->sendTo(fmt(VT_CURSPOS) % 23 % 1);
  ch->sendTo("Select the number to set to, <C/R> to return to main menu.\n\r--> ");
}

static void finishRoom(TRoom *rp, TBeing *ch, dirTypeT dir)
{
  roomDirData *exitp;
  TRoom *newrp;

  if (!(exitp = rp->dir_option[dir])) {
    ch->sendTo("Blah!\n\r");
    vlogf(LOG_EDIT, "Bad stuff in finishRoom!  No dir_option!");
    return;
  }
  if (!(newrp = real_roomp(exitp->to_room))) {
    ch->sendTo("Exit room does not exist. Creating room....");
    CreateOneRoom(exitp->to_room);
    ch->sendTo("Copying flags, sector type and room height to new room.\n\r");
    if (!(newrp = real_roomp(exitp->to_room))) {
      vlogf(LOG_EDIT, "BOGUSNESS in finishRoom()::medit()");
      ch->sendTo("Error occured!!!!\n\r");
      ch->specials.edit = MAIN_MENU;
      update_room_menu(ch);
      return;
    }
    newrp->setRoomFlags(rp->getRoomFlags());
    newrp->setSectorType(rp->getSectorType());
    newrp->setRoomHeight(rp->getRoomHeight());
    make_room_coords(rp, newrp, dir);
    newrp->removeRoomFlagBit(ROOM_BEING_EDITTED);
    ch->sendTo("Done.\n\r");
  }
  ch->sendTo("Fixing opposite directions.\n\r");
  dirTypeT new_dir = rev_dir[dir];
  if (newrp->dir_option[new_dir]) {
    ch->sendTo("Exit back into room already exists...");
    if (newrp->dir_option[new_dir]->to_room == ch->in_room) {
      ch->sendTo("and is back into this room.\n\r");
      ch->sendTo("Copying door information to other side.\n\r");
      newrp->dir_option[new_dir]->door_type = rp->dir_option[dir]->door_type;
      newrp->dir_option[new_dir]->condition = rp->dir_option[dir]->condition;
      if (rp->dir_option[dir]->condition & EX_SLOPED_UP) {
        newrp->dir_option[new_dir]->condition &= ~EX_SLOPED_UP;
        newrp->dir_option[new_dir]->condition |= EX_SLOPED_DOWN;
      }
      if (rp->dir_option[dir]->condition & EX_SLOPED_DOWN) {
        newrp->dir_option[new_dir]->condition &= ~EX_SLOPED_DOWN;
        newrp->dir_option[new_dir]->condition |= EX_SLOPED_UP;
      }
      newrp->dir_option[new_dir]->lock_difficulty = rp->dir_option[dir]->lock_difficulty;
      newrp->dir_option[new_dir]->weight = rp->dir_option[dir]->weight;
      newrp->dir_option[new_dir]->key = rp->dir_option[dir]->key;
      newrp->dir_option[new_dir]->keyword = mud_str_dup(rp->dir_option[dir]->keyword);
    } else {
      ch->sendTo(fmt("And exits into incorrect room [%d].\n\r") % newrp->dir_option[new_dir]->to_room);
    }
  } else {
    ch->sendTo("Making new exit back into this room.\n\r");
    newrp->dir_option[new_dir] = new roomDirData();
    newrp->dir_option[new_dir]->to_room = ch->in_room;
    ch->sendTo("Copying door information to back_exit.\n\r");
    newrp->dir_option[new_dir]->door_type = rp->dir_option[dir]->door_type;
    newrp->dir_option[new_dir]->condition = rp->dir_option[dir]->condition;
    if (rp->dir_option[dir]->condition & EX_SLOPED_UP) {
      newrp->dir_option[new_dir]->condition &= ~EX_SLOPED_UP;
      newrp->dir_option[new_dir]->condition |= EX_SLOPED_DOWN;
    }
    if (rp->dir_option[dir]->condition & EX_SLOPED_DOWN) {
      newrp->dir_option[new_dir]->condition &= ~EX_SLOPED_DOWN;
      newrp->dir_option[new_dir]->condition |= EX_SLOPED_UP;
    }
    newrp->dir_option[new_dir]->lock_difficulty = rp->dir_option[dir]->lock_difficulty;
    newrp->dir_option[new_dir]->weight = rp->dir_option[dir]->weight;
    newrp->dir_option[new_dir]->key = rp->dir_option[dir]->key;
    newrp->dir_option[new_dir]->keyword = mud_str_dup(rp->dir_option[dir]->keyword);
  }
  ch->specials.edit = MAIN_MENU;
  update_room_menu(ch);
  return;
}

static void ChangeExitSlopedStatus(TRoom *rp, TBeing *ch, const char *arg, editorEnterTypeT type)
{
  dirTypeT dir    = DIR_NONE;
  int      update = 0;

  switch (ch->specials.edit) {
    case CHANGE_ROOM_DIR_SLOPED_NORTH:
      dir = DIR_NORTH;
      break;
    case CHANGE_ROOM_DIR_SLOPED_EAST:
      dir = DIR_EAST;
      break;
    case CHANGE_ROOM_DIR_SLOPED_SOUTH:
      dir = DIR_SOUTH;
      break;
    case CHANGE_ROOM_DIR_SLOPED_WEST:
      dir = DIR_WEST;
      break;
    case CHANGE_ROOM_DIR_SLOPED_UP:
      dir = DIR_UP;
      break;
    case CHANGE_ROOM_DIR_SLOPED_DOWN:
      dir = DIR_DOWN;
      break;
    case CHANGE_ROOM_DIR_SLOPED_NE:
      dir = DIR_NORTHEAST;
      break;
    case CHANGE_ROOM_DIR_SLOPED_NW:
      dir = DIR_NORTHWEST;
      break;
    case CHANGE_ROOM_DIR_SLOPED_SE:
      dir = DIR_SOUTHEAST;
      break;
    case CHANGE_ROOM_DIR_SLOPED_SW:
      dir = DIR_SOUTHWEST;
      break;
    case MAX_DIR:
    case DIR_NONE:
    case DIR_BOGUS:
      ChangeExitSlopedStatus(rp, ch, "", ENTER_CHECK);
    default:
      return;
  }

  if (type != ENTER_CHECK) {
    if (!*arg || (*arg == '\n')) {
      finishRoom(rp, ch, dir);
      return;
    }

    update = convertTo<int>(arg);

    if (update != 9 && update != 10)
      return;

    unsigned int newBit = (1 << (update - 1));

    if (IS_SET(rp->dir_option[dir]->condition, newBit))
      REMOVE_BIT(rp->dir_option[dir]->condition, newBit);
    else
      SET_BIT(rp->dir_option[dir]->condition, newBit);

    if (IS_SET(rp->dir_option[dir]->condition, EX_SLOPED_UP) &&
        IS_SET(rp->dir_option[dir]->condition, EX_SLOPED_DOWN))
      if (newBit == EX_SLOPED_UP)
        REMOVE_BIT(rp->dir_option[dir]->condition, EX_SLOPED_DOWN);
      else
        REMOVE_BIT(rp->dir_option[dir]->condition, EX_SLOPED_UP);
  } else if (!rp->dir_option[dir]) {
    rp->dir_option[dir] = new roomDirData();
    rp->dir_option[dir]->condition = 0;
  }

  ch->sendTo(VT_HOMECLR);
  ch->sendTo("Sloped Status:\n\r");
  ch->sendTo(fmt("\n\r9  [%c] Sloped Up") %
             (IS_SET(rp->dir_option[dir]->condition, EX_SLOPED_UP) ? 'X' : ' '));
  ch->sendTo(fmt("\n\r10 [%c] Sloped Down") %
             (IS_SET(rp->dir_option[dir]->condition, EX_SLOPED_DOWN) ? 'X' : ' '));
  ch->sendTo(fmt(VT_CURSPOS) % 20 % 1);
  ch->sendTo("Select the number to toggle, <C/R> to return to continue.\n\r--> ");
}

static void ChangeKeyNumber(TRoom *rp, TBeing *ch, const char *arg, editorEnterTypeT type)
{
  dirTypeT dir = DIR_NONE;
  int update;

  switch (ch->specials.edit) {
    case CHANGE_ROOM_KEY_NORTH:
      dir = DIR_NORTH;
      break;
    case CHANGE_ROOM_KEY_EAST:
      dir = DIR_EAST;
      break;
    case CHANGE_ROOM_KEY_SOUTH:
      dir = DIR_SOUTH;
      break;
    case CHANGE_ROOM_KEY_WEST:
      dir = DIR_WEST;
      break;
    case CHANGE_ROOM_KEY_UP:
      dir = DIR_UP;
      break;
    case CHANGE_ROOM_KEY_DOWN:
      dir = DIR_DOWN;
      break;
    case CHANGE_ROOM_KEY_NE:
      dir = DIR_NORTHEAST;
      break;
    case CHANGE_ROOM_KEY_NW:
      dir = DIR_NORTHWEST;
      break;
    case CHANGE_ROOM_KEY_SE:
      dir = DIR_SOUTHEAST;
      break;
    case CHANGE_ROOM_KEY_SW:
      dir = DIR_SOUTHWEST;
      break;
    default:
      return;
  }

  if (type != ENTER_CHECK) {
    update = convertTo<int>(arg);

    if (update < 0) {
      ch->sendTo("\n\rKey number must be greater than 0.\n\r");
      ch->sendTo("\n\rKey Number (0 for none): ");
      return;
    }
    rp->dir_option[dir]->key = update;

    finishRoom(rp, ch, dir);
    return;
  }
  ch->sendTo("\n\r\n\rEnter The virtual number of the key that will open this door.\n\r");
  ch->sendTo("A value of 0 means the door must be picked, doorbashed, etc.\n\r");
  ch->sendTo("Remember that keys loaded from OEdit will not operate properly.\n\r");
  ch->sendTo("\n\r\n\rKey Number : ");
}

static void ChangeExitLockDiff(TRoom *rp, TBeing *ch, const char *arg, editorEnterTypeT type)
{
  dirTypeT dir = DIR_NONE;

  switch (ch->specials.edit) {
    case CHANGE_ROOM_LOCK_NORTH:
      dir = DIR_NORTH;
      break;
    case CHANGE_ROOM_LOCK_EAST:
      dir = DIR_EAST;
      break;
    case CHANGE_ROOM_LOCK_SOUTH:
      dir = DIR_SOUTH;
      break;
    case CHANGE_ROOM_LOCK_WEST:
      dir = DIR_WEST;
      break;
    case CHANGE_ROOM_LOCK_UP:
      dir = DIR_UP;
      break;
    case CHANGE_ROOM_LOCK_DOWN:
      dir = DIR_DOWN;
      break;
    case CHANGE_ROOM_LOCK_NE:
      dir = DIR_NORTHEAST;
      break;
    case CHANGE_ROOM_LOCK_NW:
      dir = DIR_NORTHWEST;
      break;
    case CHANGE_ROOM_LOCK_SE:
      dir = DIR_SOUTHEAST;
      break;
    case CHANGE_ROOM_LOCK_SW:
      dir = DIR_SOUTHWEST;
      break;
    default:
      return;
  }
  if (type != ENTER_CHECK) {
    int diff;
    diff = convertTo<int>(arg);
    if ((diff < 0) || (diff > 100)) {
      ch->sendTo("Please enter a number from 0 to 100.\n\r");
      return;
    }

    rp->dir_option[dir]->lock_difficulty = diff;

    switch (dir) {
      case DIR_NORTH:
        ch->specials.edit = CHANGE_ROOM_KEY_NORTH;
        break;
      case DIR_EAST:
        ch->specials.edit = CHANGE_ROOM_KEY_EAST;
        break;
      case DIR_SOUTH:
        ch->specials.edit = CHANGE_ROOM_KEY_SOUTH;
        break;
      case DIR_WEST:
        ch->specials.edit = CHANGE_ROOM_KEY_WEST;
        break;
      case DIR_UP:
        ch->specials.edit = CHANGE_ROOM_KEY_UP;
        break;
      case DIR_DOWN:
        ch->specials.edit = CHANGE_ROOM_KEY_DOWN;
        break;
      case DIR_NORTHEAST:
        ch->specials.edit = CHANGE_ROOM_KEY_NE;
        break;
      case DIR_NORTHWEST:
        ch->specials.edit = CHANGE_ROOM_KEY_NW;
        break;
      case DIR_SOUTHEAST:
        ch->specials.edit = CHANGE_ROOM_KEY_SE;
        break;
      case DIR_SOUTHWEST:
        ch->specials.edit = CHANGE_ROOM_KEY_SW;
        break;
      case MAX_DIR:
      case DIR_NONE:
      case DIR_BOGUS:
        ChangeExitLockDiff(rp, ch, "", ENTER_CHECK);
        return;
    }
    ChangeKeyNumber(rp, ch, "", ENTER_CHECK);
    return;
  }
  ch->sendTo("\n\r\n\rEnter the difficulty of the lock.\n\r");
  ch->sendTo("A value of 100 implies that the lock is unpickable.\n\r");
  ch->sendTo("\n\r\n\rLock Difficulty: ");
  return;
}

static void ChangeExitWeight(TRoom *rp, TBeing *ch, const char *arg, editorEnterTypeT type)
{
  dirTypeT dir = DIR_NONE;

  switch (ch->specials.edit) {
    case CHANGE_ROOM_WEIGHT_NORTH:
      dir = DIR_NORTH;
      break;
    case CHANGE_ROOM_WEIGHT_EAST:
      dir = DIR_EAST;
      break;
    case CHANGE_ROOM_WEIGHT_SOUTH:
      dir = DIR_SOUTH;
      break;
    case CHANGE_ROOM_WEIGHT_WEST:
      dir = DIR_WEST;
      break;
    case CHANGE_ROOM_WEIGHT_UP:
      dir = DIR_UP;
      break;
    case CHANGE_ROOM_WEIGHT_DOWN:
      dir = DIR_DOWN;
      break;
    case CHANGE_ROOM_WEIGHT_NE:
      dir = DIR_NORTHEAST;
      break;
    case CHANGE_ROOM_WEIGHT_NW:
      dir = DIR_NORTHWEST;
      break;
    case CHANGE_ROOM_WEIGHT_SE:
      dir = DIR_SOUTHEAST;
      break;
    case CHANGE_ROOM_WEIGHT_SW:
      dir = DIR_SOUTHWEST;
      break;
    default:
      return;
  }
  if (type != ENTER_CHECK) {
    int weight;

    weight = convertTo<int>(arg);
    if ((weight <= 0) || (weight > 50)) {
      ch->sendTo("Please enter a number between 1 and 50\n\r");
      return;
    }
    rp->dir_option[dir]->weight = weight;

    switch (dir) {
      case DIR_NORTH:
        ch->specials.edit = CHANGE_ROOM_LOCK_NORTH;
        break;
      case DIR_EAST:
        ch->specials.edit = CHANGE_ROOM_LOCK_EAST;
        break;
      case DIR_SOUTH:
        ch->specials.edit = CHANGE_ROOM_LOCK_SOUTH;
        break;
      case DIR_WEST:
        ch->specials.edit = CHANGE_ROOM_LOCK_WEST;
        break;
      case DIR_UP:
        ch->specials.edit = CHANGE_ROOM_LOCK_UP;
        break;
      case DIR_DOWN:
        ch->specials.edit = CHANGE_ROOM_LOCK_DOWN;
        break;
      case DIR_NORTHEAST:
        ch->specials.edit = CHANGE_ROOM_LOCK_NE;
        break;
      case DIR_NORTHWEST:
        ch->specials.edit = CHANGE_ROOM_LOCK_NW;
        break;
      case DIR_SOUTHEAST:
        ch->specials.edit = CHANGE_ROOM_LOCK_SE;
        break;
      case DIR_SOUTHWEST:
        ch->specials.edit = CHANGE_ROOM_LOCK_SW;
        break;
      case MAX_DIR:
      case DIR_NONE:
      case DIR_BOGUS:
        ChangeExitWeight(rp, ch, "", ENTER_CHECK);
        return;
    }
    if (IS_SET(rp->dir_option[dir]->condition, EX_LOCKED)) {
      ChangeExitLockDiff(rp, ch, "", ENTER_CHECK);
      return;
    } else {
      rp->dir_option[dir]->lock_difficulty = -1;
      rp->dir_option[dir]->key = 0;
      finishRoom(rp, ch, dir);
      return;
    }
  }
  ch->sendTo(fmt("\n\r\n\rEnter the weight you wish the %s to have.\n\r") %
    ((rp->dir_option[dir]->keyword) ? fname(rp->dir_option[dir]->keyword) :
      "BOGUS DOOR KEYWORD"));
  ch->sendTo("\n\rNew Weight: ");
  return;
}

static void ChangeExitKeyword(TRoom *rp, TBeing *ch, const char *arg, editorEnterTypeT type)
{
  dirTypeT dir = DIR_NONE;

  switch (ch->specials.edit) {
    case CHANGE_ROOM_KEYWORD_NORTH:
      dir = DIR_NORTH;
      break;
    case CHANGE_ROOM_KEYWORD_EAST:
      dir = DIR_EAST;
      break;
    case CHANGE_ROOM_KEYWORD_SOUTH:
      dir = DIR_SOUTH;
      break;
    case CHANGE_ROOM_KEYWORD_WEST:
      dir = DIR_WEST;
      break;
    case CHANGE_ROOM_KEYWORD_UP:
      dir = DIR_UP;
      break;
    case CHANGE_ROOM_KEYWORD_DOWN:
      dir = DIR_DOWN;
      break;
    case CHANGE_ROOM_KEYWORD_NE:
      dir = DIR_NORTHEAST;
      break;
    case CHANGE_ROOM_KEYWORD_NW:
      dir = DIR_NORTHWEST;
      break;
    case CHANGE_ROOM_KEYWORD_SE:
      dir = DIR_SOUTHEAST;
      break;
    case CHANGE_ROOM_KEYWORD_SW:
      dir = DIR_SOUTHWEST;
      break;
    default:
      return;
  }
  if (type != ENTER_CHECK) {

    if (!arg || !*arg || !strcmp(arg, "\n")) {
      ChangeExitKeyword(rp,ch,"", ENTER_CHECK);
      return;
    }
    if (!rp->dir_option[dir]) {
      ChangeExitKeyword(rp, ch, "", ENTER_CHECK);
      vlogf(LOG_EDIT, "Bad event in ChangeExitKeyword");
      return;
    }

    delete [] rp->dir_option[dir]->keyword;

    rp->dir_option[dir]->keyword = mud_str_dup(arg);

    switch (dir) {
      case DIR_NORTH:
        ch->specials.edit = CHANGE_ROOM_WEIGHT_NORTH;
        break;
      case DIR_EAST:
        ch->specials.edit = CHANGE_ROOM_WEIGHT_EAST;
        break;
      case DIR_SOUTH:
        ch->specials.edit = CHANGE_ROOM_WEIGHT_SOUTH;
        break;
      case DIR_WEST:
        ch->specials.edit = CHANGE_ROOM_WEIGHT_WEST;
        break;
      case DIR_UP:
        ch->specials.edit = CHANGE_ROOM_WEIGHT_UP;
        break;
      case DIR_DOWN:
        ch->specials.edit = CHANGE_ROOM_WEIGHT_DOWN;
        break;
      case DIR_NORTHEAST:
        ch->specials.edit = CHANGE_ROOM_WEIGHT_NE;
        break;
      case DIR_NORTHWEST:
        ch->specials.edit = CHANGE_ROOM_WEIGHT_NW;
        break;
      case DIR_SOUTHEAST:
        ch->specials.edit = CHANGE_ROOM_WEIGHT_SE;
        break;
      case DIR_SOUTHWEST:
        ch->specials.edit = CHANGE_ROOM_WEIGHT_SW;
        break;
      case MAX_DIR:
      case DIR_NONE:
      case DIR_BOGUS:
        ChangeExitKeyword(rp, ch, "", ENTER_CHECK);
        return;
    }
    ChangeExitWeight(rp, ch, "", ENTER_CHECK);
    return;
  }
  if (rp->dir_option[dir]->keyword) {
    ch->sendTo(fmt("Former Keywords: %s\n\r") % rp->dir_option[dir]->keyword);
    ch->sendTo("    You will need to retype the Former Keyword if you want to keep it.\n\r");
  }
  ch->sendTo("New Keywords: ");
  return;
}

static void ChangeExitCondition(TRoom *rp, TBeing *ch, const char *arg, editorEnterTypeT type)
{
  int update, row;
  dirTypeT dir = DIR_NONE;
  unsigned int i = 0;
  int j = 0;
  char buf[255];

  switch (ch->specials.edit) {
    case CHANGE_ROOM_CONDITION_NORTH:
      dir = DIR_NORTH;
      break;
    case CHANGE_ROOM_CONDITION_EAST:
      dir = DIR_EAST;
      break;
    case CHANGE_ROOM_CONDITION_SOUTH:
      dir = DIR_SOUTH;
      break;
    case CHANGE_ROOM_CONDITION_WEST:
      dir = DIR_WEST;
      break;
    case CHANGE_ROOM_CONDITION_UP:
      dir = DIR_UP;
      break;
    case CHANGE_ROOM_CONDITION_DOWN:
      dir = DIR_DOWN;
      break;
    case CHANGE_ROOM_CONDITION_NE:
      dir = DIR_NORTHEAST;
      break;
    case CHANGE_ROOM_CONDITION_NW:
      dir = DIR_NORTHWEST;
      break;
    case CHANGE_ROOM_CONDITION_SE:
      dir = DIR_SOUTHEAST;
      break;
    case CHANGE_ROOM_CONDITION_SW:
      dir = DIR_SOUTHWEST;
      break;
    case MAX_DIR:
    case DIR_NONE:
    case DIR_BOGUS:
      ChangeExitCondition(rp, ch, "", ENTER_CHECK);
      return;
    default:
      return;
  }

  if (type != ENTER_CHECK) {
    if (!*arg || (*arg == '\n')) {
      switch (dir) {
	case DIR_NORTH:
	  ch->specials.edit = CHANGE_ROOM_KEYWORD_NORTH;
	  break;
	case DIR_EAST:
	  ch->specials.edit = CHANGE_ROOM_KEYWORD_EAST;
	  break;
	case DIR_SOUTH:
	  ch->specials.edit = CHANGE_ROOM_KEYWORD_SOUTH;
	  break;
	case DIR_WEST:
	  ch->specials.edit = CHANGE_ROOM_KEYWORD_WEST;
	  break;
	case DIR_UP:
	  ch->specials.edit = CHANGE_ROOM_KEYWORD_UP;
	  break;
	case DIR_DOWN:
	  ch->specials.edit = CHANGE_ROOM_KEYWORD_DOWN;
	  break;
	case DIR_NORTHEAST:
	  ch->specials.edit = CHANGE_ROOM_KEYWORD_NE;
	  break;
	case DIR_NORTHWEST:
	  ch->specials.edit = CHANGE_ROOM_KEYWORD_NW;
	  break;
	case DIR_SOUTHEAST:
	  ch->specials.edit = CHANGE_ROOM_KEYWORD_SE;
	  break;
	case DIR_SOUTHWEST:
	  ch->specials.edit = CHANGE_ROOM_KEYWORD_SW;
	  break;
        case MAX_DIR:
        case DIR_NONE:
        case DIR_BOGUS:
          ChangeExitCondition(rp, ch, "", ENTER_CHECK);
          return;
      }
      ChangeExitKeyword(rp, ch, "", ENTER_CHECK);
      return;
    }
    update = convertTo<int>(arg);

    if (update <= 0 || update >= MAX_DOOR_CONDITIONS)   
      return;
    i = 1 << (update-1);

    if (i == EX_TRAPPED) {
      ch->sendTo("Door traps are set in the zone file.\n\r");
      return;
    }
    if (i == EX_NOENTER) {
      ch->sendTo("Don't set this on doors.\n\r");
      return;
    }

    if (IS_SET(rp->dir_option[dir]->condition, i))
      REMOVE_BIT(rp->dir_option[dir]->condition, i);
    else
      SET_BIT(rp->dir_option[dir]->condition, i);

    if (IS_SET(rp->dir_option[dir]->condition, EX_DESTROYED)) {
      if (IS_SET(rp->dir_option[dir]->condition, EX_CLOSED)) {
        ch->sendTo("Destroyed doors can't be closed.\n\r");
        REMOVE_BIT(rp->dir_option[dir]->condition, EX_CLOSED);
      }
      if (IS_SET(rp->dir_option[dir]->condition, EX_LOCKED)) {
        ch->sendTo("Destroyed doors can't be locked.\n\r");
        REMOVE_BIT(rp->dir_option[dir]->condition, EX_LOCKED);
      }
      if (IS_SET(rp->dir_option[dir]->condition, EX_SECRET)) {
        ch->sendTo("Destroyed doors can't be secret.\n\r");
        REMOVE_BIT(rp->dir_option[dir]->condition, EX_SECRET);
      }
    }
    if (IS_SET(rp->dir_option[dir]->condition, EX_CAVED_IN)) {
      if (!IS_SET(rp->dir_option[dir]->condition, EX_CLOSED)) {
        ch->sendTo("Caved-in exits must be closed.\n\r");
        SET_BIT(rp->dir_option[dir]->condition, EX_CLOSED);
      }
      if (IS_SET(rp->dir_option[dir]->condition, EX_LOCKED)) {
        ch->sendTo("Caved-In exits can't be locked.\n\r");
        REMOVE_BIT(rp->dir_option[dir]->condition, EX_LOCKED);
      }
      if (IS_SET(rp->dir_option[dir]->condition, EX_SECRET)) {
        ch->sendTo("Caved-In exits can't be secret.\n\r");
        REMOVE_BIT(rp->dir_option[dir]->condition, EX_SECRET);
      }
    }
  } else if (!rp->dir_option[dir]) {
    rp->dir_option[dir] = new roomDirData();
    rp->dir_option[dir]->condition = 0;
  }
  ch->sendTo(VT_HOMECLR);
  ch->sendTo("Exit Flags:");

  row = 0;
  for (j = 0; j < MAX_DOOR_CONDITIONS; j++) {
    sprintf(buf, VT_CURSPOS, row + 4, ((j & 1) ? 45 : 5));
    if (j & 1)
      row++;
    ch->sendTo(buf);
    ch->sendTo(fmt("%-2d [%s] %s") % (j + 1) % (((rp->dir_option[dir]->condition & (1 << j)) ? "X" : " ")) %
	  exit_bits[j]);
  }

  ch->sendTo(fmt(VT_CURSPOS) % 20 % 1);
  ch->sendTo("Select the number to toggle, <C/R> to return to continue.\n\r--> ");
}

static void ChangeExitType(TRoom *rp, TBeing *ch, const char *arg, editorEnterTypeT type)
{
  int i, row, update;
  dirTypeT dir = DIR_NONE;
  char buf[255];

  switch (ch->specials.edit) {
    case CHANGE_ROOM_TYPE_NORTH:
      dir = DIR_NORTH;
      break;
    case CHANGE_ROOM_TYPE_EAST:
      dir = DIR_EAST;
      break;
    case CHANGE_ROOM_TYPE_SOUTH:
      dir = DIR_SOUTH;
      break;
    case CHANGE_ROOM_TYPE_WEST:
      dir = DIR_WEST;
      break;
    case CHANGE_ROOM_TYPE_UP:
      dir = DIR_UP;
      break;
    case CHANGE_ROOM_TYPE_DOWN:
      dir = DIR_DOWN;
      break;
    case CHANGE_ROOM_TYPE_NE:
      dir = DIR_NORTHEAST;
      break;
    case CHANGE_ROOM_TYPE_NW:
      dir = DIR_NORTHWEST;
      break;
    case CHANGE_ROOM_TYPE_SE:
      dir = DIR_SOUTHEAST;
      break;
    case CHANGE_ROOM_TYPE_SW:
      dir = DIR_SOUTHWEST;
      break;
    default:
      return;
  }
  if (type != ENTER_CHECK) {
    update = convertTo<int>(arg);
    update--;

    if (update < DOOR_NONE || update >= MAX_DOOR_TYPES)
      return;

    rp->dir_option[dir]->door_type = doorTypeT(update);

    if (rp->dir_option[dir]->door_type == DOOR_NONE) {
      rp->dir_option[dir]->condition = 0;
      rp->dir_option[dir]->weight = -1;
      rp->dir_option[dir]->lock_difficulty = -1;
      rp->dir_option[dir]->key = 0;
      rp->dir_option[dir]->description = NULL;
      rp->dir_option[dir]->keyword = mud_str_dup("");

      switch (dir) {
        case DIR_NORTH:
          ch->specials.edit = CHANGE_ROOM_DIR_SLOPED_NORTH;
          break;
        case DIR_EAST:
          ch->specials.edit = CHANGE_ROOM_DIR_SLOPED_EAST;
          break;
        case DIR_SOUTH:
          ch->specials.edit = CHANGE_ROOM_DIR_SLOPED_SOUTH;
          break;
        case DIR_WEST:
          ch->specials.edit = CHANGE_ROOM_DIR_SLOPED_WEST;
          break;
        case DIR_UP:
          ch->specials.edit = CHANGE_ROOM_DIR_SLOPED_UP;
          break;
        case DIR_DOWN:
          ch->specials.edit = CHANGE_ROOM_DIR_SLOPED_DOWN;
          break;
        case DIR_NORTHWEST:
          ch->specials.edit = CHANGE_ROOM_DIR_SLOPED_NW;
          break;
        case DIR_NORTHEAST:
          ch->specials.edit = CHANGE_ROOM_DIR_SLOPED_NE;
          break;
        case DIR_SOUTHWEST:
          ch->specials.edit = CHANGE_ROOM_DIR_SLOPED_SW;
          break;
        case DIR_SOUTHEAST:
          ch->specials.edit = CHANGE_ROOM_DIR_SLOPED_SE;
          break;
        case MAX_DIR:
        case DIR_NONE:
        case DIR_BOGUS:
          ChangeExitType(rp, ch, "", ENTER_CHECK);
          return;
      }

      ChangeExitSlopedStatus(rp, ch, "", ENTER_CHECK);
      return;
    } else {
      switch (dir) {
        case DIR_NORTH:
          ch->specials.edit = CHANGE_ROOM_CONDITION_NORTH;
          break;
        case DIR_EAST:
          ch->specials.edit = CHANGE_ROOM_CONDITION_EAST;
          break;
        case DIR_SOUTH:
          ch->specials.edit = CHANGE_ROOM_CONDITION_SOUTH;
          break;
        case DIR_WEST:
          ch->specials.edit = CHANGE_ROOM_CONDITION_WEST;
          break;
        case DIR_UP:
          ch->specials.edit = CHANGE_ROOM_CONDITION_UP;
          break;
        case DIR_DOWN:
          ch->specials.edit = CHANGE_ROOM_CONDITION_DOWN;
          break;
        case DIR_NORTHEAST:
          ch->specials.edit = CHANGE_ROOM_CONDITION_NE;
          break;
        case DIR_NORTHWEST:
          ch->specials.edit = CHANGE_ROOM_CONDITION_NW;
          break;
        case DIR_SOUTHEAST:
          ch->specials.edit = CHANGE_ROOM_CONDITION_SE;
          break;
        case DIR_SOUTHWEST:
          ch->specials.edit = CHANGE_ROOM_CONDITION_SW;
          break;
        case MAX_DIR:
        case DIR_NONE:
        case DIR_BOGUS:
          ChangeExitType(rp, ch, "", ENTER_CHECK);
          return;
      }
      ChangeExitCondition(rp, ch, "", ENTER_CHECK);
      return;
    }
  }

  ch->sendTo(VT_HOMECLR);
  ch->sendTo(fmt("Door Type: %s") % door_types[rp->dir_option[dir]->door_type]);

  row = 0;
  for (i = 0; i < MAX_DOOR_TYPES; i++) {
    sprintf(buf, VT_CURSPOS, row + 3, (((i% 3) * 25) + 5));
    if ((i%3) == 2)
      row++;
    ch->sendTo(buf);
    ch->sendTo(fmt("%-2d %s") % (i+1) % door_types[i]);
  }
  ch->sendTo(fmt(VT_CURSPOS) % 23 % 1);
  ch->sendTo("Select the door type.\n\r--> ");
}

static void ChangeExitNumber(TRoom *rp, TBeing *ch, const char *arg, editorEnterTypeT type)
{
  dirTypeT dir = DIR_NONE;
  int update;

  switch (ch->specials.edit) {
    case CHANGE_ROOM_EXIT_NORTH:
      dir = DIR_NORTH;
      break;
    case CHANGE_ROOM_EXIT_EAST:
      dir = DIR_EAST;
      break;
    case CHANGE_ROOM_EXIT_SOUTH:
      dir = DIR_SOUTH;
      break;
    case CHANGE_ROOM_EXIT_WEST:
      dir = DIR_WEST;
      break;
    case CHANGE_ROOM_EXIT_UP:
      dir = DIR_UP;
      break;
    case CHANGE_ROOM_EXIT_DOWN:
      dir = DIR_DOWN;
      break;
    case CHANGE_ROOM_EXIT_NE:
      dir = DIR_NORTHEAST;
      break;
    case CHANGE_ROOM_EXIT_NW:
      dir = DIR_NORTHWEST;
      break;
    case CHANGE_ROOM_EXIT_SE:
      dir = DIR_SOUTHEAST;
      break;
    case CHANGE_ROOM_EXIT_SW:
      dir = DIR_SOUTHWEST;
      break;
    default:
      return;
  }
  if (type == ENTER_CHECK)
    return;

  update = convertTo<int>(arg);

  if (update < 0 || update > 49999) {
    ch->sendTo("\n\rRoom number must be between 0 and 49999.\n\r");
    ch->sendTo("\n\rExit to Room: ");
    return;
  }

  if (!ch->limitPowerCheck(CMD_REDIT, update)) {
    ch->sendTo("\n\rYou are not allowed to make an exit out to that room, sorry.\n\r");
    ch->sendTo("\n\rExit to Room: ");
    return;
  }

  if (!rp->dir_option[dir]) {
    vlogf(LOG_EDIT, "Bad news in redit!  no dir where it should be!");
    ch->specials.edit = MAIN_MENU;
    update_room_menu(ch);
  }
  rp->dir_option[dir]->to_room = update;

  switch (dir) {
    case DIR_NORTH:
      ch->specials.edit = CHANGE_ROOM_TYPE_NORTH;
      break;
    case DIR_EAST:
      ch->specials.edit = CHANGE_ROOM_TYPE_EAST;
      break;
    case DIR_SOUTH:
      ch->specials.edit = CHANGE_ROOM_TYPE_SOUTH;
      break;
    case DIR_WEST:
      ch->specials.edit = CHANGE_ROOM_TYPE_WEST;
      break;
    case DIR_UP:
      ch->specials.edit = CHANGE_ROOM_TYPE_UP;
      break;
    case DIR_DOWN:
      ch->specials.edit = CHANGE_ROOM_TYPE_DOWN;
      break;
    case DIR_NORTHEAST:
      ch->specials.edit = CHANGE_ROOM_TYPE_NE;
      break;
    case DIR_NORTHWEST:
      ch->specials.edit = CHANGE_ROOM_TYPE_NW;
      break;
    case DIR_SOUTHEAST:
      ch->specials.edit = CHANGE_ROOM_TYPE_SE;
      break;
    case DIR_SOUTHWEST:
      ch->specials.edit = CHANGE_ROOM_TYPE_SW;
      break;
    case MAX_DIR:
    case DIR_NONE:
    case DIR_BOGUS:
      return;
  }
  ChangeExitType(rp, ch, "", ENTER_CHECK);
}

static void ChangeExitDir(TRoom *rp, TBeing *ch, const char *arg, editorEnterTypeT type)
{
  int update;

  if (type != ENTER_CHECK) {
    if (!*arg || (*arg == '\n')) {
      ch->specials.edit = MAIN_MENU;
      update_room_menu(ch);
      return;
    }
    update = convertTo<int>(arg) - 1;
    if (update <= -1 || update > 9) {
      ChangeExitDir(rp, ch, "", ENTER_CHECK);
      return;
    }
    if (!(rp->dir_option[update])) {
      ch->sendTo("\n\rMaking new exit.");
      rp->dir_option[update] = new roomDirData();
    } else {
      ch->sendTo(fmt("\n\rModifying exit.  Current exit to room %i.") % 
		 rp->dir_option[update]->to_room);
    }
    ch->sendTo("\n\r\n\rExit to Room: ");

    switch (update) {
      case DIR_NORTH:
	ch->specials.edit = CHANGE_ROOM_EXIT_NORTH;
	break;
      case DIR_EAST:
	ch->specials.edit = CHANGE_ROOM_EXIT_EAST;
	break;
      case DIR_SOUTH:
	ch->specials.edit = CHANGE_ROOM_EXIT_SOUTH;
	break;
      case DIR_WEST:
	ch->specials.edit = CHANGE_ROOM_EXIT_WEST;
	break;
      case DIR_UP:
	ch->specials.edit = CHANGE_ROOM_EXIT_UP;
	break;
      case DIR_DOWN:
	ch->specials.edit = CHANGE_ROOM_EXIT_DOWN;
	break;
      case DIR_NORTHEAST:
	ch->specials.edit = CHANGE_ROOM_EXIT_NE;
	break;
      case DIR_NORTHWEST:
	ch->specials.edit = CHANGE_ROOM_EXIT_NW;
	break;
      case DIR_SOUTHEAST:
	ch->specials.edit = CHANGE_ROOM_EXIT_SE;
	break;
      case DIR_SOUTHWEST:
	ch->specials.edit = CHANGE_ROOM_EXIT_SW;
	break;
      case MAX_DIR:
      case DIR_NONE:
      case DIR_BOGUS:
	ChangeExitDir(rp, ch, "", ENTER_CHECK);
	return;
    }
    ChangeExitNumber(rp, ch, "", ENTER_CHECK);
    return;
  }
  ch->sendTo(VT_HOMECLR);
  ch->sendTo(fmt("Room Name: %s") % rp->name);
  ch->sendTo(fmt(VT_CURSPOS) % 1 % 40);
  ch->sendTo(fmt("Room Number: %d") % rp->number);
  ch->sendTo(fmt(VT_CURSPOS) % 4 % 1);
  ch->sendTo(exit_menu);
  ch->sendTo("--> ");
  return;
}

static void DeleteExit(TRoom *rp, TBeing *ch, const char *arg, editorEnterTypeT type)
{
  int update;

  if (type != ENTER_CHECK) {
    if (!*arg || (*arg == '\n')) {
      ch->specials.edit = MAIN_MENU;
      update_room_menu(ch);
      return;
    }

    update = convertTo<int>(arg);
    update--;

    if (update < MIN_DIR || update >= MAX_DIR)
      return; 

    if (rp->dir_option[update]) {
      rp->dir_option[update] = 0;
      delete rp->dir_option[update];
      ch->sendTo("Deleting exit.\n\r");
      ch->specials.edit = MAIN_MENU;
      update_room_menu(ch);
      return;
    } else {
      ch->sendTo("There is no exit in that direction.\n\r");
      ch->sendTo("Enter another direction, or press <ENTER> to exit.\n\r");
      return;
    }

    return;
  }
  ch->sendTo(VT_HOMECLR);
  ch->describeWeather(ch->in_room);
  ch->listExits(ch->roomp);
  ch->sendTo("\n\r");

#if 1
  dirTypeT     tExit;
  roomDirData *tData;

  for (tExit = MIN_DIR; tExit < MAX_DIR; tExit++)
    if ((tData = rp->exitDir(tExit)) && tData->to_room != ROOM_NOWHERE) {
      ch->sendTo(fmt("  %2d) %-9s     ") % (tExit + 1) % dirs[tExit]);

      if ((tExit % 2))
        ch->sendTo("\n\r");
    } else if (!(tExit % 2))
      ch->sendTo("                    ");
    else
      ch->sendTo("\n\r");

  if (!(tExit % 2))
    ch->sendTo("\n\r");
#else
  ch->sendTo(exit_menu);
#endif

  ch->sendTo("Choose exit to delete.\n\r--> ");
  return;
}

static void DeleteExtraDesc(TRoom *rp, TBeing *ch)
{
  extraDescription *exptr, *nptr;

  if (rp->ex_description) {
    for (exptr = rp->ex_description; exptr; exptr = nptr) {
      nptr = exptr->next;
      delete exptr;
    }
    rp->ex_description = NULL;
  }
  update_room_menu(ch);
}

static void change_room_extra(TRoom *rp, TBeing *ch, const char *arg, editorEnterTypeT type)
{
  extraDescription *ed, *prev;

  if (type != ENTER_CHECK) {
    if (!*arg || (*arg == '\n')) {
      ch->specials.edit = MAIN_MENU;
      update_room_menu(ch);
      return;
    }
    for (prev = ed = rp->ex_description;; prev = ed, ed = ed->next) {
      if (!ed) {
        ed = new extraDescription();
	ed->next = rp->ex_description;
	rp->ex_description = ed;
        ed->keyword = mud_str_dup(arg);
	ed->description = NULL;
	ch->desc->str = &ed->description;
	ch->sendTo("Enter the description. Terminate with a '~' on a NEW LINE.\n\r");
	break;
      } else if (!strcasecmp(ed->keyword, arg)) {
        ch->sendTo(fmt("Current description:\n\r%s\n\r") % ed->description);
        ch->sendTo("This description has been deleted.  If you needed to modify it, simply readd it.\n\r");
        ch->sendTo("Press return to proceed.\n\r");
        if (prev == ed) {
          rp->ex_description = ed->next;
          delete ed;
        } else {
          prev->next = ed->next;
          delete ed;
        }
        return;
      }
    }
    ch->desc->max_str = MAX_STRING_LENGTH;
    return;
  }
  ch->sendTo("Existing keywords:\n\r");
  for ( ed = rp->ex_description;ed ; ed = ed->next) {
    ch->sendTo(fmt("%s\n\r") % ed->keyword);
  }
  ch->sendTo("\n\rEnter the keyword for the extra description.\n\r--> ");
  ch->specials.edit = CHANGE_ROOM_ROOM_EXDESC;
  return;
}

static void RoomSave(TBeing *ch, int start, int end, int useSecond)
{
  char temp[2048], dots[500];
  int rstart, rend, i, k, x;
  extraDescription *exptr;
  TRoom *rp;
  roomDirData *rdd;
  char *newline;
  TDatabase db(DB_IMMORTAL);

  rstart = start;
  rend = end;

  if (((rstart <= -1) || (rend <= -1)) ||
      ((rstart > WORLD_SIZE) || (rend > WORLD_SIZE))) {
    ch->sendTo("I don't know those room #s.  Make sure they are all contiguous.\n\r");
    return;
  }
  ch->sendTo("Saving.\n\r");
  strcpy(dots, "\0");

  db.query("delete from room where owner='%s' and block=%i", 
	   ch->getName(), useSecond);
  db.query("delete from roomexit where owner='%s' and block=%i",
	   ch->getName(), useSecond);
  db.query("delete from roomextra where owner='%s' and block=%i", 
	   ch->getName(), useSecond);

  for (i = rstart; i <= rend; i++) {
    rp = real_roomp(i);
    if (rp == NULL)
      continue;

    strcat(dots, ".");

    x = 0;

    if (!rp->getDescr()) {
      rp->setDescr(mud_str_dup("Empty\n"));
    }
    for (k = 0; k <= (int) strlen(rp->getDescr()); k++) {
      if (rp->getDescr()[k] != 13)
	temp[x++] = rp->getDescr()[k];
    }
    temp[x] = '\0';

    db.query("insert into room (owner, block, vnum,x,y,z,name,description,room_flag,sector,teletime,teletarg,telelook,river_speed,river_dir,capacity,height) values ('%s',%i,%i,%i,%i,%i, '%s','%s',%i,%i,%i,%i,%i,%i,%i,%i,%i)",
	     ch->getName(), useSecond,
	     rp->number, 0, 0, 0, rp->name, temp?temp:"", 
	     rp->getRoomFlags(),
	     mapSectorToFile(rp->getSectorType()), 
	     rp->getTeleTime(), rp->getTeleTarg(),
	     rp->getTeleLook(), rp->getRiverSpeed(), rp->getRiverDir(),
	     rp->getMoblim(), rp->getRoomHeight());

    dirTypeT j;
    for (j = MIN_DIR; j < MAX_DIR; j++) {
      rdd = rp->dir_option[j];
      if (rdd) {
	temp[0]='\0';
	if (rdd->description) {
	  for (k = 0, x = 0; k <= (int) strlen(rdd->description); k++) {
	    if (rdd->description[k] != 13)
	      temp[x++] = rdd->description[k];
	  }
	  temp[x] = '\0';
	}

	if (rdd->keyword) {
	  if (strlen(rdd->keyword) > 0) {
	    // strip off unwanted carriage returns. - Russ 
	    newline = strchr(rdd->keyword, '\n');
	    newline = '\0';
	  }
	}

	sstring keyword=rdd->keyword;
	sstring descr=temp;

	db.query("insert into roomexit (owner,block, vnum,direction,name,description,type,condition_flag,lock_difficulty,weight,key_num,destination) values ('%s', %i, %i, %i,'%s','%s',%i,%i,%i,%i,%i,%i)", 
		 ch->getName(), useSecond,
		 rp->number, mapDirToFile(j), keyword.c_str(),
		 descr.c_str(),
		 rdd->door_type, rdd->condition, rdd->lock_difficulty,
		 rdd->weight, rdd->key, rdd->to_room);

      }
    }


    for (exptr = rp->ex_description; exptr; exptr = exptr->next) {
      x = 0;
      if (exptr->description) {
	for (k = 0; k <= (int) strlen(exptr->description); k++) {
	  if (exptr->description[k] != 13)
	    temp[x++] = exptr->description[k];
	}
	temp[x] = '\0';

	db.query("insert into roomextra (owner, block, vnum, name, description) values ('%s',%i,%i,'%s','%s')", ch->getName(), useSecond, rp->number, exptr->keyword, temp);
      }
    }

  }
  ch->sendTo(dots);
  ch->sendTo("\n\rDone.\n\r");
}

void RoomLoad(TBeing *ch, int start, int end, int useSecond)
{
  int     vnum;
  bool    found = FALSE;
  TRoom  *rp,
         *rp2;
  TThing *t;
  int tmp;
  extraDescription *new_descr;

  TDatabase db(DB_IMMORTAL);
  TDatabase db_exits(DB_IMMORTAL);
  TDatabase db_extras(DB_IMMORTAL);

  ch->sendTo("Searching and loading rooms\n\r");

  db.query("select vnum, x, y, z, name, description, room_flag, sector, teletime, teletarg, telelook, river_speed, river_dir, capacity, height from room where owner='%s' and block=%i and vnum >= %i and vnum <= %i order by vnum asc", ch->getName(), useSecond, start, end);

  db_exits.query("select vnum, direction, name, description, type, condition_flag, lock_difficulty, weight, key_num, destination from roomexit where owner='%s' and block=%i and vnum >= %i and vnum <= %i order by vnum asc", ch->getName(), useSecond, start, end);
  db_exits.fetchRow();

  db_extras.query("select vnum, name, description from roomextra where owner='%s' and block=%i and vnum >= %i and vnum <= %i order by vnum asc", ch->getName(), useSecond, start, end);
  db_extras.fetchRow();

  while(db.fetchRow()){
    vnum=convertTo<int>(db["vnum"]);

    if ((vnum >= start) && (vnum <= end)) {
      if (vnum >= end)
	found = TRUE;

      if (!(rp = real_roomp(vnum))) {	// empty room 
	rp2 = new TRoom(vnum);
        rp2->putInDb(vnum);
	ch->sendTo("+");
      } else {
	rp2 = new TRoom(vnum);

        sstring tStString(ch->msgVariables(MSG_RLOAD, (TThing *)NULL));
        tStString += "\n\r";

        sendrpf(rp, tStString.c_str());

        while ((t = rp->getStuff())) {
          --(*t);
          *rp2 += *t;
        }
        delete rp;
        rp = NULL;
        rp2->putInDb(vnum);
	ch->sendTo("-");
      }
      rp2->number = vnum;


      rp2->setXCoord(convertTo<int>(db["x"]));
      rp2->setYCoord(convertTo<int>(db["y"]));
      rp2->setZCoord(convertTo<int>(db["z"]));

      rp2->name=mud_str_dup(db["name"]);
      rp2->setDescr(mud_str_dup(db["description"]));
      
      if (!zone_table.empty()) {
	//      fscanf(fl, " %*d ");  // this is the "zone" value - unused?
	unsigned int z;
	for (z = 0; rp2->number>zone_table[z].top && z<zone_table.size(); z++);
	
	if (z >= zone_table.size()) {
	  vlogf(LOG_EDIT, fmt("Room %d is outside of any zone.\n") % rp2->number);
	  exit(0);
	}
	rp2->setZoneNum(z);
      }
      rp2->setRoomFlags(convertTo<int>(db["room_flag"]));
      
      rp2->setSectorType(mapFileToSector(convertTo<int>(db["sector"])));
      rp2->setTeleTime(convertTo<int>(db["teleTime"]));
      rp2->setTeleTarg(convertTo<int>(db["teleTarg"]));
      rp2->setTeleLook(convertTo<int>(db["teleLook"]));
      
      rp2->setRiverSpeed(convertTo<int>(db["river_speed"]));
      rp2->setRiverDir(mapFileToDir(convertTo<int>(db["river_dir"])));
      rp2->setMoblim(convertTo<int>(db["capacity"]));
      
      rp2->setRoomHeight(convertTo<int>(db["height"]));
      
      rp2->spec = 0;
      rp2->setLight(0);
      rp2->setHasWindow(0);
      
      rp2->ex_description = NULL;

      while(convertTo<int>(db_extras["vnum"]) == rp2->number){
	new_descr = new extraDescription();
	new_descr->keyword = mud_str_dup(db_extras["name"]);
	if (!new_descr->keyword || !*new_descr->keyword)
	  vlogf(LOG_EDIT, fmt("No keyword in room %d\n") %  rp2->number);

	new_descr->description = mud_str_dup(db_extras["description"]);
	if (!new_descr->description || !*new_descr->description)
	  vlogf(LOG_LOW, fmt("No desc in room %d\n") %  rp2->number);

	new_descr->next = rp2->ex_description;
	rp2->ex_description = new_descr;
	
	if(!db_extras.fetchRow())
	  break;
      }

      dirTypeT dir;
      for (dir = MIN_DIR; dir < MAX_DIR; dir++)
	rp2->dir_option[dir] = 0;

      while(convertTo<int>(db_exits["vnum"]) == rp2->number){
	dir=mapFileToDir(convertTo<int>(db_exits["direction"]));

	rp2->dir_option[dir] = new roomDirData();

	if(!db_exits["name"].empty())
	  rp2->dir_option[dir]->keyword = mud_str_dup(db_exits["name"]);
	else
	  rp2->dir_option[dir]->keyword = NULL;

	if(!db_exits["description"].empty())
	  rp2->dir_option[dir]->description = mud_str_dup(db_exits["description"]);
	else
	  rp2->dir_option[dir]->description = NULL;

	tmp=convertTo<int>(db_exits["type"]);
	if (tmp < 0 || tmp >= MAX_DOOR_TYPES) {
	  vlogf(LOG_LOW,fmt("bogus door type (%d) in room (%d) dir %d.") % 
		tmp % rp2->number % dir);
	  return;
	}
	rp2->dir_option[dir]->door_type = doorTypeT(tmp);
	if ((tmp == DOOR_NONE) && (rp2->dir_option[dir]->keyword)){
	  if (strcmp(rp2->dir_option[dir]->keyword, "_unique_door_"))
	    vlogf(LOG_LOW,fmt("non-door with name in room %d") % rp2->number);
	}
	if ((tmp != DOOR_NONE) && !(rp2->dir_option[dir]->keyword)){
	  vlogf(LOG_LOW,fmt("door with no name in room %d") % rp2->number);
	}

	rp2->dir_option[dir]->condition = convertTo<int>(db_exits["condition_flag"]);
	rp2->dir_option[dir]->lock_difficulty= convertTo<int>(db_exits["lock_difficulty"]);;
	rp2->dir_option[dir]->weight= convertTo<int>(db_exits["weight"]);
	rp2->dir_option[dir]->key = convertTo<int>(db_exits["key_num"]);

	rp2->dir_option[dir]->to_room = convertTo<int>(db_exits["destination"]);

	if (IS_SET(rp2->dir_option[dir]->condition, EX_SECRET) && 
	    canSeeThruDoor(rp2->dir_option[dir])) {
	  if (IS_SET(rp2->dir_option[dir]->condition, EX_CLOSED)){
	    //vlogf(LOG_LOW, fmt("See thru door set secret. (%d, %d)") %  room % dir);
	  } else
	    vlogf(LOG_LOW, fmt("Secret door saved as open. (%d, %d)") % 
		  rp2->number % dir);
	}
	if(!db_exits.fetchRow())
	  break;
      }
      
      rp2->initLight();
    } else {
      ch->sendTo(fmt("Room %d found, but not in load range!  Skipping.\n\r") % vnum);

    }
  }

  if (!found) 
    ch->sendTo("The room number(s) that you specified could not all be found.\n\r");
  else 
    ch->sendTo("\n\rDone.\n\r");
}

void CreateOneRoom(int loc_nr)
{
  char buf[256];
  TRoom *rp;

  allocate_room(loc_nr);
  rp = real_roomp(loc_nr);

  rp->number = loc_nr;
  if (!zone_table.empty()) {
    unsigned int z;
    for (z = 0; rp->number > zone_table[z].top && z < zone_table.size(); z++);

    if (z >= zone_table.size()) {
      vlogf(LOG_EDIT, fmt("Room %d is outside of any zone.\n") %  rp->number);
      z--;
    }
    rp->setZoneNum(z);
  }
  sprintf(buf, "%d", loc_nr);
  rp->name = mud_str_dup(buf);
  rp->setDescr(mud_str_dup("Empty\n"));

  rp->initLight();

  rp->setSectorType(SECT_ASTRAL_ETHREAL);
  
  // default the save room flag to 'on'
  rp->setRoomFlagBit(ROOM_SAVE_ROOM);
  
  roomCount++;
}

void TRoom::loadOne(FILE *fl, bool tinyfile)
{
  char chk[50];
  int tmp;
  extraDescription *new_descr;

  if(fscanf(fl, "%d ", &tmp))
    x=tmp;
  else 
    x=0;

  if(fscanf(fl, "%d ", &tmp))
    y=tmp;
  else
    y=0;

  if(fscanf(fl, "%d\n", &tmp))
    z=tmp;
  else 
    z=0;

  name = fread_string(fl);


  setDescr(fread_string(fl));

  if (!zone_table.empty()) {
    fscanf(fl, " %*d ");
    unsigned int z;
    for (z = 0; number > zone_table[z].top && z < zone_table.size(); z++);

    if (z >= zone_table.size()) {
      vlogf(LOG_EDIT, fmt("Room %d is outside of any zone.\n") %  number);
      exit(0);
    }
    zone = &zone_table[z];
  }
  fscanf(fl, " %d ", &tmp);
  roomFlags = tmp;

  fscanf(fl, " %d ", &tmp);

  if (tmp == -1) {
    fscanf(fl, " %d", &tmp);
    teleTime = tmp;
    fscanf(fl, " %d", &tmp);
    teleTarg = tmp;
    fscanf(fl, " %d", &tmp);
    teleLook = tmp;
    fscanf(fl, " %d", &tmp);
  } else {
    teleTime = 0;
    teleTarg = 0;
    teleLook = 0;
  }
  setSectorType(mapFileToSector(tmp));

  fscanf(fl, " %d ", &tmp);
  riverSpeed = tmp;
  fscanf(fl, " %d ", &tmp);
  riverDir = mapFileToDir(tmp);
  fscanf(fl, " %d ", &tmp);
  moblim = tmp;
  fscanf(fl, " %d ", &tmp);
  setRoomHeight(tmp);
  spec = 0;
  setLight(0);
  hasWindow = 0;

  dirTypeT dir;
  for (dir = MIN_DIR; dir < MAX_DIR; dir++)
    dir_option[dir] = 0;

  ex_description = NULL;

  while (fscanf(fl, "%s\n", chk) == 1) {
    switch (*chk) {
      case 'D': // Exits in a direction
        setup_dir(fl, number, mapFileToDir(convertTo<int>(chk + 1)), this);
        break;
      case 'E': // Extra description
        new_descr = new extraDescription();
        new_descr->keyword = fread_string(fl);
        if (!new_descr->keyword || !*new_descr->keyword)
          vlogf(LOG_EDIT, fmt("No keyword in room %d\n") %  number);

        new_descr->description = fread_string(fl);
        if (!new_descr->description || !*new_descr->description)
          vlogf(LOG_LOW, fmt("No desc in room %d\n") %  number);

        new_descr->next = ex_description;
        ex_description = new_descr;

        break;
      case 'S':  // end of current room
        roomCount++;
        return;
      default:
        vlogf(LOG_EDIT, fmt("Unknown auxiliary code `%s' in room load of #%d") %  chk % number);
        break;
    }
  }
}

void TRoom::initLight()
{
  TThing *ch;
  int found = FALSE;

  if (!IS_SET(roomFlags, ROOM_INDOORS))
    setLight(outdoorLight());
  else if (getHasWindow()) {
    int best = 0, curr = 0;
    for (ch = getStuff();ch;ch = ch->nextThing) {
      TSeeThru *obj = dynamic_cast<TSeeThru *>(ch);
      if (!obj) continue;
      if (obj->givesOutsideLight()) {
        found = TRUE;
        curr = obj->getLightFromOutside();
        if (curr > best)
          best = curr;
      } else if (obj)
        found = TRUE;
    }
    if (!found) {
      vlogf(LOG_EDIT, fmt("Room (%s:%d) missing expected window.  Resetting.") %  name % number);
      setHasWindow(FALSE);
    }
    setLight(best);
  } else
    setLight(0);

  for (ch = getStuff(); ch; ch = ch->nextThing) {
    addToLight(ch->getLight());

    // things on tables also count toward room light
    TTable * ttab = dynamic_cast<TTable *>(ch);
    if (ttab) {
      TThing *tt;
      for (tt = ttab->rider; tt; tt = tt->nextRider) {
        addToLight(tt->getLight());
      }
    }
  }
}

TRoom *room_find_or_create(int key)
{
  TRoom *rv;

  if ((rv = real_roomp(key)))
    return rv;

  rv = new TRoom(key);

  room_db[key] = rv;

  return rv;
}

sstring TRoom::describeGroundWeather() const
{
  if(!isUnderwaterSector() && !isWaterSector() && 
     getWeather() == WEATHER_SNOWY){
    // non-water sector in the snow
    return "<W>snow-covered<1>";
  } else if(isArcticSector()){
    // non-snowing arctic sector OR water sector
    return "<W>icy<1>";
  }

  if(!isUnderwaterSector() && !isWaterSector() &&
     !isSwampSector() && !isBeachSector() &&
     getWeather() == WEATHER_RAINY){
    // non-water/mud/sand sector in the rain
    return "<B>rain-slick<1>";
  }

  return "";
}

sstring TRoom::describeGroundType() const
{
  if (isUnderwaterSector())
    return "ocean floor";
  else if (isCitySector())
    return "street";
  else if (isRoadSector())
    return "road";
  else if (isWaterSector())
    return "water";
  else if (isSwampSector())
    return "mud";
  else if (isBeachSector())
    return "sand";
  else if (IS_SET(getRoomFlags(), ROOM_INDOORS))
    return "floor";
  else
    return "ground";
}

sstring TRoom::describeGround() const
{
  return fmt("%s%s%s") % describeGroundWeather() % 
    (describeGroundWeather().empty()?"":" ") % describeGroundType();
}

dirTypeT mapFileToDir(int num)
{
  switch(num) {
    case 0:
      return DIR_NORTH;
    case 1:
      return DIR_EAST;
    case 2:
      return DIR_SOUTH;
    case 3:
      return DIR_WEST;
    case 4:
      return DIR_UP;
    case 5:
      return DIR_DOWN;
    case 6:
      return DIR_NORTHEAST;
    case 7:
      return DIR_NORTHWEST;
    case 8:
      return DIR_SOUTHEAST;
    case 9:
      return DIR_SOUTHWEST;
  }
  return DIR_NONE;
}

int mapDirToFile(dirTypeT dir)
{
  switch (dir) {
    case DIR_NORTH:
      return 0;
    case DIR_EAST:
      return 1;
    case DIR_SOUTH:
      return 2;
    case DIR_WEST:
      return 3;
    case DIR_UP:
      return 4;
    case DIR_DOWN:
      return 5;
    case DIR_NORTHEAST:
      return 6;
    case DIR_NORTHWEST:
      return 7;
    case DIR_SOUTHEAST:
      return 8;
    case DIR_SOUTHWEST:
      return 9;
    case MAX_DIR:
    case DIR_BOGUS:
    case DIR_NONE:
      break;
  }
  return -1;
}

void room_edit(TBeing *ch, const char *arg)
{
  TRoom *rp = ch->roomp;

  if (ch->specials.edit == MAIN_MENU) {
    if (!*arg || *arg == '\n') {
      ch->desc->connected = CON_PLYNG;
      ch->roomp->removeRoomFlagBit(ROOM_BEING_EDITTED);
      act("$n has returned from editing.", TRUE, ch, 0, 0, TO_ROOM);

      // reset the terminal bars
      if (ch->vt100() || ch->ansi())
        ch->doCls(false);
      return;
    }
    switch (convertTo<int>(arg)) {
      case 0:
	update_room_menu(ch);
	return;
      case 1:
	ch->specials.edit = CHANGE_NAME;
	ChangeRoomName(rp, ch, "", ENTER_CHECK);
	return;
      case 2:
	ch->specials.edit = CHANGE_LONG_DESC;
	ChangeRoomDesc(rp, ch, "", ENTER_CHECK);
	return;
      case 3:
	ch->specials.edit = CHANGE_ROOM_FLAGS;
	ChangeRoomFlags(rp, ch, "", ENTER_CHECK);
	return;
      case 4:
	ch->specials.edit = CHANGE_ROOM_TYPE;
	ChangeRoomType(rp, ch, "", ENTER_CHECK);
	return;
      case 5:
	ch->specials.edit = CHANGE_ROOM_EXIT;
	ChangeExitDir(rp, ch, "", ENTER_CHECK);
	return;
      case 6:
	ch->specials.edit = CHANGE_ROOM_EXTRA;
	change_room_extra(rp, ch, "", ENTER_CHECK);
	return;
      case 7:
	ch->specials.edit = CHANGE_ROOM_TUNNEL;
	ChangeMaxCap(rp, ch, "", ENTER_CHECK);
	return;
      case 8:
        ch->specials.edit = CHANGE_ROOM_ROOM_HEIGHT;
        ChangeRoomHeight(rp, ch, "", ENTER_CHECK);
        return;
      case 9:
	ch->specials.edit = DELETE_ROOM_EXITS;
	DeleteExit(rp, ch, "", ENTER_CHECK);
	return;
      case 10:
	ch->specials.edit = MAIN_MENU;
	DeleteExtraDesc(rp, ch);
	return;
      default:
	update_room_menu(ch);
	return;
    }
  }
  switch (ch->specials.edit) {
    case DELETE_ROOM_EXITS:
      DeleteExit(rp, ch, arg, ENTER_REENTRANT);
      return;
    case CHANGE_ROOM_TUNNEL:
      ChangeMaxCap(rp, ch, arg, ENTER_REENTRANT);
      return;
    case CHANGE_ROOM_ROOM_HEIGHT:
      ChangeRoomHeight(rp, ch, arg, ENTER_REENTRANT);
      return;
    case CHANGE_NAME:
      ChangeRoomName(rp, ch, arg, ENTER_REENTRANT);
      return;
    case CHANGE_LONG_DESC:
      ChangeRoomDesc(rp, ch, arg, ENTER_REENTRANT);
      return;
    case CHANGE_ROOM_FLAGS:
      ChangeRoomFlags(rp, ch, arg, ENTER_REENTRANT);
      return;
    case CHANGE_ROOM_TYPE:
    case CHANGE_ROOM_TYPE2:
    case CHANGE_ROOM_TYPE3:
      ChangeRoomType(rp, ch, arg, ENTER_REENTRANT);
      return;
    case CHANGE_ROOM_EXIT:
      ChangeExitDir(rp, ch, arg, ENTER_REENTRANT);
      return;
    case CHANGE_ROOM_EXTRA:
    case CHANGE_ROOM_ROOM_EXDESC:
      change_room_extra(rp, ch, arg, ENTER_REENTRANT);
      return;
    case CHANGE_ROOM_TYPE_NORTH:
    case CHANGE_ROOM_TYPE_EAST:
    case CHANGE_ROOM_TYPE_SOUTH:
    case CHANGE_ROOM_TYPE_WEST:
    case CHANGE_ROOM_TYPE_UP:
    case CHANGE_ROOM_TYPE_DOWN:
    case CHANGE_ROOM_TYPE_NE:
    case CHANGE_ROOM_TYPE_NW:
    case CHANGE_ROOM_TYPE_SE:
    case CHANGE_ROOM_TYPE_SW:
      ChangeExitType(rp, ch, arg, ENTER_REENTRANT);
      return;
    case CHANGE_ROOM_CONDITION_NORTH:
    case CHANGE_ROOM_CONDITION_EAST:
    case CHANGE_ROOM_CONDITION_SOUTH:
    case CHANGE_ROOM_CONDITION_WEST:
    case CHANGE_ROOM_CONDITION_UP:
    case CHANGE_ROOM_CONDITION_DOWN:
    case CHANGE_ROOM_CONDITION_NE:
    case CHANGE_ROOM_CONDITION_NW:
    case CHANGE_ROOM_CONDITION_SE:
    case CHANGE_ROOM_CONDITION_SW:
      ChangeExitCondition(rp, ch, arg, ENTER_REENTRANT);
      return;
    case CHANGE_ROOM_EXIT_NORTH:
    case CHANGE_ROOM_EXIT_EAST:
    case CHANGE_ROOM_EXIT_SOUTH:
    case CHANGE_ROOM_EXIT_WEST:
    case CHANGE_ROOM_EXIT_UP:
    case CHANGE_ROOM_EXIT_DOWN:
    case CHANGE_ROOM_EXIT_NE:
    case CHANGE_ROOM_EXIT_NW:
    case CHANGE_ROOM_EXIT_SE:
    case CHANGE_ROOM_EXIT_SW:
      ChangeExitNumber(rp, ch, arg, ENTER_REENTRANT);
      return;
    case CHANGE_ROOM_KEY_NORTH:
    case CHANGE_ROOM_KEY_EAST:
    case CHANGE_ROOM_KEY_SOUTH:
    case CHANGE_ROOM_KEY_WEST:
    case CHANGE_ROOM_KEY_UP:
    case CHANGE_ROOM_KEY_DOWN:
    case CHANGE_ROOM_KEY_NE:
    case CHANGE_ROOM_KEY_NW:
    case CHANGE_ROOM_KEY_SE:
    case CHANGE_ROOM_KEY_SW:
      ChangeKeyNumber(rp, ch, arg, ENTER_REENTRANT);
      return;
    case CHANGE_ROOM_KEYWORD_NORTH:
    case CHANGE_ROOM_KEYWORD_EAST:
    case CHANGE_ROOM_KEYWORD_SOUTH:
    case CHANGE_ROOM_KEYWORD_WEST:
    case CHANGE_ROOM_KEYWORD_UP:
    case CHANGE_ROOM_KEYWORD_DOWN:
    case CHANGE_ROOM_KEYWORD_NE:
    case CHANGE_ROOM_KEYWORD_NW:
    case CHANGE_ROOM_KEYWORD_SE:
    case CHANGE_ROOM_KEYWORD_SW:
      ChangeExitKeyword(rp, ch, arg, ENTER_REENTRANT);
      return;
    case CHANGE_ROOM_LOCK_NORTH:
    case CHANGE_ROOM_LOCK_EAST:
    case CHANGE_ROOM_LOCK_SOUTH:
    case CHANGE_ROOM_LOCK_WEST:
    case CHANGE_ROOM_LOCK_UP:
    case CHANGE_ROOM_LOCK_DOWN:
    case CHANGE_ROOM_LOCK_NE:
    case CHANGE_ROOM_LOCK_NW:
    case CHANGE_ROOM_LOCK_SE:
    case CHANGE_ROOM_LOCK_SW:
      ChangeExitLockDiff(rp, ch, arg, ENTER_REENTRANT);
      return;
    case CHANGE_ROOM_WEIGHT_NORTH:
    case CHANGE_ROOM_WEIGHT_EAST:
    case CHANGE_ROOM_WEIGHT_SOUTH:
    case CHANGE_ROOM_WEIGHT_WEST:
    case CHANGE_ROOM_WEIGHT_UP:
    case CHANGE_ROOM_WEIGHT_DOWN:
    case CHANGE_ROOM_WEIGHT_NE:
    case CHANGE_ROOM_WEIGHT_NW:
    case CHANGE_ROOM_WEIGHT_SE:
    case CHANGE_ROOM_WEIGHT_SW:
      ChangeExitWeight(rp, ch, arg, ENTER_REENTRANT);
      return;
    case CHANGE_ROOM_DIR_SLOPED_NORTH:
    case CHANGE_ROOM_DIR_SLOPED_EAST:
    case CHANGE_ROOM_DIR_SLOPED_SOUTH:
    case CHANGE_ROOM_DIR_SLOPED_WEST:
    case CHANGE_ROOM_DIR_SLOPED_UP:
    case CHANGE_ROOM_DIR_SLOPED_DOWN:
    case CHANGE_ROOM_DIR_SLOPED_NE:
    case CHANGE_ROOM_DIR_SLOPED_NW:
    case CHANGE_ROOM_DIR_SLOPED_SE:
    case CHANGE_ROOM_DIR_SLOPED_SW:
      ChangeExitSlopedStatus(rp, ch, arg, ENTER_REENTRANT);
      return;
    default:
      vlogf(LOG_EDIT, fmt("Got to bad spot in room_edit (Error: %d)") % ch->specials.edit);
      return;
  }
}

void TPerson::doRsave(const char *argument)
{
  int    tStart = 0,
         tEnd   = 0;
  int   tSec   = 1;
  //  char   tString[256];
  sstring tStArg(argument),
         tStString(""),
         tStBuffer("");

  if (!hasWizPower(POWER_RSAVE)) {
    incorrectCommand();
    return;
  }

  if (!desc || !isImmortal())
    return;

  stSpaceOut(tStArg);
  tStString=tStArg.word(0);
  tStBuffer=tStArg.word(1);

  if (tStString.empty() || tStString[0] == '1') {
    tStart = desc->blockastart;
    tEnd   = desc->blockaend;
    tSec=1;
  } else if (tStString[0] == '2') {
    tStart = desc->blockbstart;
    tEnd   = desc->blockbend;
    tSec   = 2;
  } else if (is_abbrev(tStString, "backup")) {
    // backup save just does a normal save with block+100
    if (tStBuffer.empty() || tStBuffer[0] == '1'){
      tStart = desc->blockastart;
      tEnd   = desc->blockaend;
      tSec=101;
    } else if (tStBuffer[0] == '2'){
      tStart = desc->blockbstart;
      tEnd   = desc->blockbend;
      tSec   = 102;
    } else {
      sendTo("Syntax: redit save backup <\"1\"/\"2\">\n\r");
      return;
    }

    sendTo("Creating backup.\n\r");
  } else {
    sendTo("Syntax: redit save <\"1\"1/\"2\">\n\r");
    return;
  }

  if (tStart <= 0 || tEnd <= 0) {
    sendTo("You have no rooms assigned in that block...Sorry.\n\r");
    return;
  }

  if (tStart > tEnd)
    sendTo("Your room block is messed up.  Talk with Head Low immediatly!\n\r");
  else {
    bool bHasRooms = false;

    for (int iRunner = tStart; iRunner != (tEnd + 1); iRunner++)
      if (real_roomp(iRunner)) {
        bHasRooms = true;
        break;
      }

    if (!bHasRooms) {
      sendTo("No rooms within that range currently exist in the world, aborting rsave.\n\r");
    } else {
      RoomSave(this, tStart, tEnd, tSec);
    }
  }
}
