//////////////////////////////////////////////////////////////////////////
//
// SneezyMUD - All rights reserved, SneezyMUD Coding Team
//
//    "show.cc" - Functions related to showing something
//                                                                      //
//////////////////////////////////////////////////////////////////////////

#include <dirent.h>

#include "stdsneezy.h"
#include "disc_looting.h"
#include "combat.h"
#include "obj_component.h"
#include "cmd_dissect.h"
#include "disc_alchemy.h"
#include "obj_open_container.h"
#include "obj_component.h"

static void print_room(int rnum, TRoom *rp, sstring &sb, struct show_room_zone_struct *)
{
  char buf[10240];
  int dink, bits, scan;

  sprintf(buf, "%5d %4d %-12s     %s\n\r", rp->number, rnum,
        TerrainInfo[rp->getSectorType()]->name, (rp->name ? rp->name : "Empty"));
  if (rp->getRoomFlags()) {
    strcat(buf, "    [");

    dink = 0;
    for (bits = rp->getRoomFlags(), scan = 0; bits; scan++) {
      if (bits & (1 << scan)) {
        if (dink)
          strcat(buf, " ");
        if (scan < MAX_ROOM_BITS)
          strcat(buf, room_bits[scan]);
        dink = 1;
        bits ^= (1 << scan);
      }
    }
    strcat(buf, "]\n\r");
  }

  sb += buf;
}

static void show_room_zone(int rnum, TRoom *rp, sstring &, struct
			   show_room_zone_struct *srzs)
{
  char buf[MAX_STRING_LENGTH];

  *buf = '\0';

  if (!rp || rp->number < srzs->bottom || rp->number > srzs->top)
    return;            // optimize later

  if (srzs->blank && (srzs->lastblank + 1 != rp->number)) {
    sprintf(buf, "rooms %d-%d are blank.\n\r", srzs->startblank,
	    srzs->lastblank);
    srzs->sb += buf;
    srzs->blank = 0;
  }
  if (!rp->name) {
    vlogf(LOG_BUG, fmt("room %d's name is screwed!\n\r") %  rp->number);
    return;
  } else if (1 == sscanf(rp->name, "%d", &srzs->lastblank) && srzs->lastblank
	     == rp->number) {
    if (!srzs->blank) {
      srzs->startblank = srzs->lastblank;
      srzs->blank = 1;
    }
    return;
  } else if (srzs->blank) {
    sprintf(buf, "rooms %d-%d are blank.\n\r", srzs->startblank,
	    srzs->lastblank);
    srzs->sb += buf;
    srzs->blank = 0;
  }
  print_room(rnum, rp, srzs->sb, NULL);
}

static void print_lit_room(int rnum, TRoom *rp, sstring &sb, struct show_room_zone_struct *)
{
  if (rp && rp->isRoomFlag(ROOM_ALWAYS_LIT))
    print_room(rnum, rp, sb, NULL);
}

static void print_save_room(int rnum, TRoom *rp, sstring &sb, struct show_room_zone_struct *)
{
  if (rp && rp->isRoomFlag(ROOM_SAVE_ROOM))
    print_room(rnum, rp, sb, NULL);
}

static void print_death_room(int rnum, TRoom *rp, sstring &sb, struct show_room_zone_struct *)
{
  if (rp && rp->isRoomFlag(ROOM_DEATH))
    print_room(rnum, rp, sb, NULL);
}

static void print_hospital_room(int rnum, TRoom *rp, sstring &sb, struct show_room_zone_struct *)
{
  if (rp && rp->isRoomFlag(ROOM_HOSPITAL))
    print_room(rnum, rp, sb, NULL);
}

static void print_noheal_room(int rnum, TRoom *rp, sstring &sb, struct show_room_zone_struct *)
{
  if (rp && rp->isRoomFlag(ROOM_NO_HEAL))
    print_room(rnum, rp, sb, NULL);
}

static void print_arena_room(int rnum, TRoom *rp, sstring &sb, struct show_room_zone_struct *)
{
  if (rp && rp->isRoomFlag(ROOM_ARENA))
    print_room(rnum, rp, sb, NULL);
}

static void print_noflee_room(int rnum, TRoom *rp, sstring &sb, struct show_room_zone_struct *)
{
  if (rp && rp->isRoomFlag(ROOM_NO_FLEE))
    print_room(rnum, rp, sb, NULL);
}

static void print_private_room(int rnum, TRoom *rp, sstring &sb, struct show_room_zone_struct *)
{
  if (rp && rp->isRoomFlag(ROOM_PRIVATE))
    print_room(rnum, rp, sb, NULL);
}

unsigned long int showFreeMobObj(int shFrZoneNumber, sstring *sb,
                                 bool isMobileF, bool shFrLoop=false)
{
  if (shFrZoneNumber < 0 || shFrZoneNumber >= ((signed int) zone_table.size())) {
    *sb += "Zone Number incorect.\n\r";
    return 0;
  }
                int shFrTop = 0,
                    shFrBot = 0,
                    shFrTopR = -1,
                    shFrBotR = -1;
  unsigned long int shFrTotalCount[2] = {0, 0};
  zoneData          &zd = zone_table[shFrZoneNumber];

  if (!zd.enabled)
    return 0;

  shFrTop = zd.top;
  shFrBot = (shFrZoneNumber ? zone_table[shFrZoneNumber - 1].top + 1: 0);

  int  shFrCountSize = (shFrTop - shFrBot + 1),
       shFrCountMax  = (isMobileF ? mob_index.size() : obj_index.size());
  bool shFrCountList[shFrCountSize];
  char tString[256];

  for (int Runner = 0; Runner < shFrCountSize; Runner++)
    shFrCountList[Runner] = false;

  shFrTotalCount[0] = shFrCountSize;

  for (int Runner = 0; Runner < shFrCountMax; Runner++) {
    if (( isMobileF && (mob_index[Runner].virt < shFrBot || mob_index[Runner].virt > shFrTop)) ||
        (!isMobileF && (obj_index[Runner].virt < shFrBot || obj_index[Runner].virt > shFrTop)))
      continue;

    int shFrWalkVirt = (isMobileF ? mob_index[Runner].virt : obj_index[Runner].virt) - shFrBot;

    shFrCountList[max(min(shFrWalkVirt, (shFrCountSize - 1)), 0)] = true;
    shFrTotalCount[0]--;
  }

  if (shFrTotalCount[0] > 0) {
    if (shFrLoop) {
      sprintf(tString, "**** Zone: %d\n\r", shFrZoneNumber);
      *sb += tString;
    }

    for (int Runner = 0; Runner < shFrCountSize; Runner++) {
      if (shFrCountList[Runner]) {
        if (shFrBotR != -1) {
          shFrTotalCount[1] = (shFrTopR - shFrBotR + 1);
          sprintf(tString, "%5d - %5d : %5lu Free\n\r",
                  shFrBotR, shFrTopR, shFrTotalCount[1]);
          *sb += tString;
        }

        shFrBotR = shFrTopR = -1;
      } else {
        if (shFrBotR == -1)
          shFrBotR = shFrTopR = (Runner + shFrBot);
        else
          shFrTopR = (Runner + shFrBot);

        if (Runner == (shFrCountSize - 1)) {
          shFrTotalCount[1] = (shFrTopR - shFrBotR + 1);
          sprintf(tString, "%5d - %5d : %5lu Free\n\r",
                  shFrBotR, shFrTopR, shFrTotalCount[1]);
          *sb += tString;
        }
      }
    }

    sprintf(tString, "----- Total Count: %5lu\n\r", shFrTotalCount[0]);
    *sb += tString;

    return shFrTotalCount[0];
  }

  return 0;
}

// Does major searching and returns the following:
// Dissection loads
// 'Nature' loads
// Scriptfile loads
sstring showComponentTechnical(const int tValue)
{
  sstring         tStString(""),
                 tStBuffer("");
  char           tString[256],
                 tBuffer[256];
  int            tMobNum;
  struct dirent *tDir;
  DIR           *tDirInfo;
  FILE          *tFile;

  // Check for dissection loads.
  // This doesn't check for hard-coded ones such as 'by race' and such.
  for (unsigned int tDissectIndex = 0; tDissectIndex < dissect_array.size(); tDissectIndex++)
    if ((dissect_array[tDissectIndex].loadItem == (unsigned) tValue)) {
      tMobNum = real_mobile(tDissectIndex);

      if (tMobNum < 0 || tMobNum > (signed) mob_index.size())
        strcpy(tBuffer, "[Unknown]");
      else
        strcpy(tBuffer, mob_index[tMobNum].name);

      sprintf(tString, "Dissect Load: %d %s\n\r", tDissectIndex, tBuffer);
      tStString += tString;
    }

  // Check for natural loads.  Unfortunatly it's easy to do a double entry here
  // so we have to be careful.
  for (unsigned int tCompIndex = 0; tCompIndex < component_placement.size(); tCompIndex++)
    if (component_placement[tCompIndex].number == tValue &&
        (component_placement[tCompIndex].place_act & CACT_PLACE)) {
      if (component_placement[tCompIndex].room2 == -1)
        tBuffer[0] = '\0';
      else
        sprintf(tBuffer, "-%d", component_placement[tCompIndex].room2);

      sprintf(tString, "Natural Load: Room%s %d%s\n\r",
              (!tBuffer[0] ? "" : "s"),
              component_placement[tCompIndex].room1,
              tBuffer);
      tStString += tString;
    }

  // Check for script loads.  This will go through ALL of the scripts and check.
  // We only do this on !PROD because of the lag it will generate, and I do mean a
  // LOT of lag it will make.
  if (gamePort != PROD_GAMEPORT) {
    if (!(tDirInfo = opendir("mobdata/responses"))) {
      vlogf(LOG_FILE, "Unable to dirwalk directory mobdata/resposnes");
      tStString += "ERROR.  Unable to open mobdata/responses for reading.";
      return tStString;
    }

    while ((tDir = readdir(tDirInfo))) {
      if (!strcmp(tDir->d_name, ".") || !strcmp(tDir->d_name, ".."))
        continue;

      sprintf(tBuffer, "mobdata/responses/%s", tDir->d_name);

      if (!(tFile = fopen(tBuffer, "r")))
        continue;

      while (fgets(tString, 256, tFile)) {
        char *tChar = tString;

        for (; isspace(*tChar) || *tChar == '\t'; tChar++);

        sprintf(tBuffer, "load %d;\n", tValue);

        if (!strcmp(tChar, tBuffer)) {
          tMobNum = real_mobile(convertTo<int>(tDir->d_name));

          if (tMobNum < 0 || tMobNum > (signed) mob_index.size())
            strcpy(tString, "[Unknown]");
          else
            strcpy(tString, mob_index[tMobNum].name);

          sprintf(tBuffer, "Script: %s %s\n\r",
                  tDir->d_name, tString);
          tStString += tBuffer;

          // Don't show the same entry twice.
          break;
        }
      }

      fclose(tFile);
    }

    closedir(tDirInfo);
  }

  return tStString;
}

void TBeing::doShow(const char *)
{
  return;
}

void TPerson::doShow(const char *argument)
{
  char      buf[256],
            zonenum[256],
            buf2[256];
  int       bottom = 0,
            top    = 0;
  sstring    sb;
  TBeing   *ch     = NULL,
           *b;
  TMonster *k;
  TObj     *obj;

  if (!isImmortal())
    return;

  if (!hasWizPower(POWER_SHOW)) {
    sendTo("You lack the power to show things.\n\r");
    return;
  }

  argument = one_argument(argument, buf);

// Show Race:
//   First, check for the race option.  Assign the second argument to target.
//   If target has a sstring (ie. not empty), check to see if they passed an
//   index number.  If so, call the appropriate race's showTo method and
//   return.  Make sure the number is less than the MAX_RACIAL_TYPES, else
//   send them a list of valid races.
//
//   If the index passed is 0, it will fail the first check, but they might
//   be checking on the RACE_NORACE stats, so see if they passed "0".  If so,
//   call NORACE's showTo method.  Otherwise, pass the sstring to the
//   getRaceIndex() function.  If it returns a valid index, call the
//   appropriate showTo method.  Otherwise, let it fall out of the "if" and
//   list all valid races.

  if (is_abbrev(buf, "races")) {
    if (!hasWizPower(POWER_SHOW_MOB)) {
      sendTo("You lack the power to show mob information.\n\r");
      return;
    }
    argument = one_argument(argument, buf);
    if (buf && *buf) {
      int raceIndex = -1;

      if (is_number(buf)) {
        raceIndex = convertTo<int>(buf);

        if (raceIndex > 0 && raceIndex < MAX_RACIAL_TYPES && Races[raceIndex]) {
          Races[raceIndex]->showTo(this);
          return;
        }
      } else {
        for (raceIndex = RACE_NORACE; raceIndex < MAX_RACIAL_TYPES; raceIndex++)
          if (Races[raceIndex] && is_abbrev(buf, Races[raceIndex]->getSingularName())) {
            Races[raceIndex]->showTo(this);
            return;
          }
      }
      /*
      int raceIndex = convertTo<int>(buf);
      if (raceIndex < MAX_RACIAL_TYPES) {
        if ((raceIndex) && (Races[raceIndex])) {
          Races[raceIndex]->showTo(this);
          return;
        } else {
          if (!strcmp(buf, "0") && (Races[raceIndex])){
            Races[raceIndex]->showTo(this);
            return;
          } else {
            raceIndex = getRaceIndex(buf);
            if ((raceIndex >= RACE_NORACE) && (Races[raceIndex])) {
              Races[raceIndex]->showTo(this);
              return;
            }
          }
        }
      }
      */
    }
    listRaces(this);
    return;
  }

  if (is_abbrev(buf, "factions")) {
    sb += "Faction                   Power    Wealth   Tithe\n\r";
    factionTypeT i;
    for (i = MIN_FACTION; i < MAX_FACTIONS; i++) {
      sprintf(buf, "%-25.25s %7.2f %-7i %5.2f\n\r",
          FactionInfo[i].faction_name,
          FactionInfo[i].faction_power,
          FactionInfo[i].getMoney(),
          FactionInfo[i].faction_tithe);
      sb += buf;
      sprintf(buf, "      %s%-15.15s%s %-15.15s %-15.15s %-15.15s\n\r",
          blue(), FactionInfo[i].leader[0], norm(),
          FactionInfo[i].leader[1],
          FactionInfo[i].leader[2],FactionInfo[i].leader[3]);
      sb += buf;
      sprintf(buf, "      %sCaravan:%s interval: %d, counter: %d, value: %d, defense: %d\n\r",
          blue(), norm(),
          FactionInfo[i].caravan_interval,
          FactionInfo[i].caravan_counter,
          FactionInfo[i].caravan_value,
          FactionInfo[i].caravan_defense);
      sb += buf;
      sprintf(buf, "             : attempts: %d, successes: %d\n\r",
          FactionInfo[i].caravan_attempts,
          FactionInfo[i].caravan_successes);
      sb += buf;
      sprintf(buf, "      %sHelp Ratio:%s %.1f, %.1f, %.1f, %.1f\n\r",
          blue(), norm(),
          FactionInfo[i].faction_array[FACT_NONE][OFF_HELP],
          FactionInfo[i].faction_array[FACT_BROTHERHOOD][OFF_HELP],
          FactionInfo[i].faction_array[FACT_CULT][OFF_HELP],
          FactionInfo[i].faction_array[FACT_SNAKE][OFF_HELP]);
      sb += buf;
      sprintf(buf, "      %sHarm Ratio:%s %.1f, %.1f, %.1f, %.1f\n\r",
          blue(), norm(),
          FactionInfo[i].faction_array[FACT_NONE][OFF_HURT],
          FactionInfo[i].faction_array[FACT_BROTHERHOOD][OFF_HURT],
          FactionInfo[i].faction_array[FACT_CULT][OFF_HURT],
          FactionInfo[i].faction_array[FACT_SNAKE][OFF_HURT]);
      sb += buf;
      sprintf(buf, "\n\r");
      sb += buf;
    }
    sprintf(buf, "average power: %5.2f\n\r", avg_faction_power);
    sb += buf;
  } else if (is_abbrev(buf, "fights")) {
    sb += "Combatant                      Fighting                       Room\n\r";
    sb += "------------------------------------------------------------------\n\r";
    for (ch = gCombatList; ch; ch = ch->next_fighting) {
      sprintf(buf, "%-30s %-30s %d\n\r", ch->getName(), ch->fight()->getName(), ch->inRoom());
      sb += buf;
    }
  } else if (is_abbrev(buf, "trapped")) {
    sb += "Trapped Containers\n\r";
    sb += "-------------------------------------\n\r";
    for(TObjIter iter=object_list.begin();iter!=object_list.end();++iter){
      TOpenContainer *tc = dynamic_cast<TOpenContainer *>(*iter);
      if (tc && tc->isContainerFlag(CONT_TRAPPED)) {
        do_where_thing(this, tc, FALSE, sb);
      }
    }
  } else if (is_abbrev(buf, "zones")) {
    bottom = 0;
    sb += "Zone#    Name                                life  age     rooms      act Lvl\n\r";

    // Using argument which was returned from one_argument earlier in doShow
    // to add addition functionality for zone searching - Russ 11/07/98
    for (; isspace(*argument); argument++);

    unsigned int zone;
    for (zone = 0; zone < zone_table.size(); zone++) {
      zoneData &zd = zone_table[zone];
      if (!*argument ||
           isname(argument, zd.name) ||
	  (!strcasecmp(argument, "disabled") && !zd.enabled) ||
	  (!strcasecmp(argument, "active") && zd.zone_value==-1)) {
        if (zd.enabled)
          strcpy(buf2, zd.name);
        else
          sprintf(buf2, "DISABLED: %s", zd.name);
       
        sprintf(buf, "%3d %-38.38s %4dm %4dm %6d-%-6d %3d %.1f\n\r", 
             zone, buf2, zd.lifespan, zd.age, bottom, zd.top, 
             zd.zone_value,
             (zd.num_mobs ? zd.mob_levels/zd.num_mobs : 0));
        sb += buf;
      }
      bottom = zd.top + 1;
    }
  } else if (is_abbrev(buf, "objects")) {
    if (!hasWizPower(POWER_SHOW_OBJ)) {
      sendTo("You lack the power to show obj information.\n\r");
      return;
    }
    if (*argument) {
      const char *tString;

      tString = argument;
      tString = one_argument(tString, buf);
      tString = one_argument(tString, buf2);

      if (*buf && is_abbrev(buf, "type")) {
        ubyte itemType = (unsigned char)(*buf2 ? convertTo<int>(buf2) : 0);

        if (!hasWizPower(POWER_SHOW_TRUSTED)) {
          sb += "VNUM  rnum   names\n\r";
        } else {
          sb += "VNUM  count max_exist str AC value names\n\r";
        }

        if (!*buf2 || !is_number(buf2) || top > MAX_OBJ_TYPES) {
          sendTo("Syntax: show objects type <type>\n\rSee OEDIT for item numbers.\n\r");
          return;
        }

        for (unsigned int objectIndex = 0;
             objectIndex < obj_index.size();
             objectIndex++) {
          if (obj_index[objectIndex].itemtype != itemType ||
              obj_index[objectIndex].virt < 0)
            continue;

          TObj *tObj = read_object(obj_index[objectIndex].virt, VIRTUAL);
          strcpy(buf, tObj->getNameForShow(false, true, this).c_str());

          if (!hasWizPower(POWER_SHOW_TRUSTED))
            sprintf(buf2, "%5d %5d   %s\n\r",
                    obj_index[objectIndex].virt, objectIndex, buf);
          else
            sprintf(buf2, "%5d %3d    %5d   %3d %2d %5d %s\n\r",
                    obj_index[objectIndex].virt, obj_index[objectIndex].getNumber(),
                    obj_index[objectIndex].max_exist,
                    obj_index[objectIndex].max_struct,
                    max(obj_index[objectIndex].armor, (sh_int) 0),
                    obj_index[objectIndex].value,
                    buf);

          sb += buf2;
        }

        if (desc)
          desc->page_string(sb, SHOWNOW_NO, ALLOWREP_YES);

        return;
      }
    }

    int zone = -1;

    // If we gave no zone, lets Auto-select our present one.
    // Else find the one the player wants.
    if (*argument) {
      strcpy(zonenum, argument);
      sscanf(zonenum, "%i", &zone);
    } else
      zone = roomp->getZoneNum();
    if ((zone < 0 || zone >= (signed int) zone_table.size()) && !*zonenum) {
      sb += "That is not a valid zone_number\n\r";
      if (desc)
        desc->page_string(sb);

      return;
    }
    if (zone >= 0) {
      bottom = zone ? (zone_table[zone - 1].top + 1) : 0;
      top = zone_table[zone].top;
    }
    if (!hasWizPower(POWER_SHOW_TRUSTED)) {
      sb += "VNUM  rnum   names\n\r";
    } else {
      sb += "VNUM  count max_exist str AC value names\n\r";
    }
    unsigned int objnx;
    for (objnx = 0; objnx < obj_index.size(); objnx++) {

      if (zone >= 0 && 
          (obj_index[objnx].virt < bottom || obj_index[objnx].virt > top) 
       || zone < 0 && !isname(zonenum, obj_index[objnx].name))
        continue;

      obj = read_object(objnx, REAL);
      sprintf(buf2, "%s", obj->getNameForShow(false, true, this).c_str());
      delete obj;

      if (!hasWizPower(POWER_SHOW_TRUSTED)) {
        sprintf(buf, "%5d %5d   %s\n\r", obj_index[objnx].virt, objnx, buf2);
      } else {
        sprintf(buf, "%5d %3d    %5d   %3d %2d %5d %s\n\r", 
              obj_index[objnx].virt, obj_index[objnx].getNumber(),
              obj_index[objnx].max_exist, 
               obj_index[objnx].max_struct,
               max(obj_index[objnx].armor, (sh_int) 0),
               obj_index[objnx].value,
              buf2);
      }
      sb += buf;
    }
  } else if (is_abbrev(buf, "mobiles")) {
    if (!hasWizPower(POWER_SHOW_MOB)) {
      sendTo("You lack the power to show mob information.\n\r");
      return;
    }
    strcpy(zonenum, argument);

    if (is_abbrev(zonenum, "pets")) {
      sb += "Pet                            Master\n\r";
      sb += "-------------------------------------\n\r";
      for (b = character_list; b; b = b->next) {
        if (b->master && dynamic_cast<TMonster *>(b)) {
          sprintf(buf, "%-30s", b->getNameNOC(this).c_str());
          sb += buf;
          sprintf(buf, " %s%s\n\r",
              b->master->getNameNOC(this).c_str(), 
              b->master->isPc() ? " (PC)" : "");
          sb += buf;
        }  
      }
      if (desc)
        desc->page_string(sb, SHOWNOW_NO, ALLOWREP_YES);

      return;
    } else if (is_abbrev(zonenum, "haters")){
      sb += "Hating Mobs\n\r";
      sb += "-------------------------------------\n\r";

      for (b = character_list; b; b = b->next) {
	TMonster *tmons = dynamic_cast<TMonster *>(b);
	sstring haters;
	charList *list;
	
	if (tmons && IS_SET(tmons->hatefield, HATE_CHAR) &&
	    tmons->hates.clist){
	  for (list = tmons->hates.clist; list; list = list->next) {
	    if (list->name){
	      haters+=list->name;
	      haters+=" ";
	    }
	  }
	}
	
	
	if(!haters.empty()){
	  sprintf(buf, "%-20.20s (room: %5d) Hates: %s", 
		  stripColorCodes(tmons->getName()).c_str(), 
		  tmons->inRoom(), haters.c_str());
	  sb += buf;
	  sb += "\n\r";
	}
      }
      if (desc)
        desc->page_string(sb, SHOWNOW_NO, ALLOWREP_YES);
      return;
    } else if (is_abbrev(zonenum, "hunters")) {
      sb += "Hunting Mobs\n\r";
      sb += "-------------------------------------\n\r";
      for (b = character_list; b; b = b->next) {
        if (!b->isPc() && IS_SET(b->specials.act, ACT_HUNTING) && 
             (ch = b->specials.hunting)) {
          TMonster *tmons = dynamic_cast<TMonster *>(b);
          sprintf(buf, "%-20.20s (room: %5d)    %-20.20s (room: %5d) %7s\n\r", 
                        tmons->getName(), tmons->inRoom(),
                        ch->getName(), ch->inRoom(),
                        (tmons->Hates(ch, NULL) ? "(HATED)" : ""));
          sb += buf;
          sprintf(buf, "       persist: %d, range: %d, origin: %d\n\r",
              tmons->persist, tmons->hunt_dist,
              tmons->oldRoom);
          sb += buf;
        }
      }
      if (desc)
        desc->page_string(sb, SHOWNOW_NO, ALLOWREP_YES);
      return;
    } else if (is_abbrev(zonenum, "response")) {
      sb += "Response Mobs\n\r";
      sb += "-------------------------------------\n\r";
      for (b = character_list; b; b = b->next) {
        if ((k = dynamic_cast<TMonster *>(b)) && 
            k->resps && k->resps->respList) {
          sprintf(buf, "%-30.30s (room: %5d)\n\r",
                      b->getName(), b->in_room);
          sb += buf;
        }
      }
      if (desc)
        desc->page_string(sb, SHOWNOW_NO, ALLOWREP_YES);
      return;
    } else if (is_abbrev(zonenum, "bounty")) {
      struct bounty_hunt_struct *job;

      sb += "Bounty Hunter\n\r";
      sb += "-------------------------------------\n\r";
      for (b = character_list; b; b = b->next) {
        if (b->spec == SPEC_BOUNTY_HUNTER && b->act_ptr) {
          job = (bounty_hunt_struct *) b->act_ptr;
          if (job && job->hunted_item && *job->hunted_item)
            sprintf(buf, "%-30.30s (room: %5d)     item: %20.20s\n\r",
                        b->getName(), b->in_room, job->hunted_item);
          else if (job && job->hunted_victim && *job->hunted_victim)
            sprintf(buf, "%-30.30s (room: %5d)     vict: %20.20s\n\r",
                        b->getName(), b->in_room, job->hunted_victim);
          else
            sprintf(buf, "%-30.30s (room: %5d)     BOGUS\n\r", 
                        b->getName(), b->in_room);
          sb += buf;
        }
      }
      if (desc)
        desc->page_string(sb, SHOWNOW_NO, ALLOWREP_YES);
      return;
    }

    if (*argument) {
      argument = one_argument(argument, buf);
      argument = one_argument(argument, buf2);

      if (*buf && is_abbrev(buf, "race")) {
        top = (*buf2 ? convertTo<int>(buf2) : -1);

        if (!hasWizPower(POWER_SHOW_TRUSTED)) {
          sb += "VNUM  level class aff names\n\r";
        } else {
          sb += "VNUM  max  count level class aff names\n\r";
        }

        if (!*buf2 || !is_number(buf2) || top < 0 || top > MAX_RACIAL_TYPES) {
          sendTo("Syntax: show mobiles race <race>\n\rSee HELP RACES for race numbers.\n\r");
          return;
        }

        for (unsigned int mobileIndex = 0;
             mobileIndex < mob_index.size();
             mobileIndex++) {
          if (mob_index[mobileIndex].race != top ||
              mob_index[mobileIndex].virt < 0)
            continue;

          if (!hasWizPower(POWER_SHOW_TRUSTED))
            sprintf(buf2, "%5d %3ld   %3ld  %3ld %s\n\r",
                    mob_index[mobileIndex].virt, mob_index[mobileIndex].level,
                    mob_index[mobileIndex].Class, mob_index[mobileIndex].faction,
                    mob_index[mobileIndex].name);
          else
            sprintf(buf2, "%5d %4d  %3d   %3ld   %3ld  %3ld %s\n\r",
                    mob_index[mobileIndex].virt, mob_index[mobileIndex].max_exist,
                    mob_index[mobileIndex].getNumber(), mob_index[mobileIndex].level,
                    mob_index[mobileIndex].Class, mob_index[mobileIndex].faction,
                    mob_index[mobileIndex].name);

          sb += buf2;
        }

        if (desc)
          desc->page_string(sb, SHOWNOW_NO, ALLOWREP_YES);

        return;
      }
    }

    int zone = -1;

    if (!*zonenum)
      zone = roomp->getZoneNum();
    else
      sscanf(zonenum, "%i", &zone);

    if ((zone < 0 || zone >= (signed int) zone_table.size()) && !*zonenum) {
      sb += "That is not a valid zone_number\n\r";
      if (desc)
        desc->page_string(sb, SHOWNOW_NO, ALLOWREP_YES);
      return;
    }
    if (zone >= 0) {
      bottom = zone ? (zone_table[zone - 1].top + 1) : 0;
      top = zone_table[zone].top;
    }
    if (!hasWizPower(POWER_SHOW_TRUSTED)) {
      sb += "VNUM  level class aff names\n\r";
    } else {
      sb += "VNUM  max  count level class aff names\n\r";
    }
    unsigned int objnx;
    for (objnx = 0; objnx < mob_index.size(); objnx++) {

      if (zone >= 0 && (mob_index[objnx].virt < bottom || mob_index[objnx].virt > top) || zone < 0 && !isname(zonenum, mob_index[objnx].name))
    continue;

      if (!hasWizPower(POWER_SHOW_TRUSTED)) {
        sprintf(buf, "%5d %3ld   %3ld  %3ld %s\n\r",
               mob_index[objnx].virt, mob_index[objnx].level,
               mob_index[objnx].Class, mob_index[objnx].faction,
               mob_index[objnx].name);
      } else {
        sprintf(buf, "%5d %4d  %3d   %3ld   %3ld  %3ld %s\n\r", 
               mob_index[objnx].virt, mob_index[objnx].max_exist,
               mob_index[objnx].getNumber(), mob_index[objnx].level, 
               mob_index[objnx].Class, mob_index[objnx].faction,
               mob_index[objnx].name);
      }
      sb += buf;
    }
  } else if (is_abbrev(buf, "maxed")) {
    if (!hasWizPower(POWER_SHOW_OBJ) || !hasWizPower(POWER_SHOW_TRUSTED)) {
      sendTo("You lack the power to show maxed obj information.\n\r");
      return;
    }

    sb += "VNUM  count max_exist str AC value names\n\r";

    unsigned int objnx;
    for (objnx = 0; objnx < obj_index.size(); objnx++) {
      if(obj_index[objnx].getNumber()<obj_index[objnx].max_exist) continue;
      obj = read_object(obj_index[objnx].virt, VIRTUAL);
      sprintf(buf2, "%s", obj->getNameForShow(false, true, this).c_str());
      delete obj;

      sprintf(buf, "%5d %3d    %5d   %3d %2d %5d %s\n\r", 
              obj_index[objnx].virt, obj_index[objnx].getNumber(),
              obj_index[objnx].max_exist, 
	      obj_index[objnx].max_struct,
	      max(obj_index[objnx].armor, (sh_int) 0),
	      obj_index[objnx].value,
              buf2);
      sb += buf;
    }
  } else if (is_abbrev(buf, "overmax")) {
    if (!hasWizPower(POWER_SHOW_OBJ) || !hasWizPower(POWER_SHOW_TRUSTED)) {
      sendTo("You lack the power to show maxed obj information.\n\r");
      return;
    }

    sb += "VNUM  count max_exist str AC value names\n\r";

    unsigned int objnx;
    for (objnx = 0; objnx < obj_index.size(); objnx++) {
      if(obj_index[objnx].getNumber()<=obj_index[objnx].max_exist) continue;
      obj = read_object(obj_index[objnx].virt, VIRTUAL);
      sprintf(buf2, "%s", obj->getNameForShow(false, true, this).c_str());
      delete obj;

      sprintf(buf, "%5d %3d    %5d   %3d %2d %5d %s\n\r", 
              obj_index[objnx].virt, obj_index[objnx].getNumber(),
              obj_index[objnx].max_exist, 
	      obj_index[objnx].max_struct,
	      max(obj_index[objnx].armor, (sh_int) 0),
	      obj_index[objnx].value,
              buf2);
      sb += buf;
    }
  } else if (is_abbrev(buf, "rooms")) {
    strcpy(zonenum, argument);

    sb += "VNUM  rnum type         name [BITS]\n\r";
    if (is_abbrev(zonenum, "death"))
      room_iterate(room_db, print_death_room, sb, NULL);
    else if (is_abbrev(zonenum, "lit"))
      room_iterate(room_db, print_lit_room, sb, NULL);
    else if (is_abbrev(zonenum, "saverooms"))
      room_iterate(room_db, print_save_room, sb, NULL);
    else if (is_abbrev(zonenum, "hospital"))
      room_iterate(room_db, print_hospital_room, sb, NULL);
    else if (is_abbrev(zonenum, "noheal"))
      room_iterate(room_db, print_noheal_room, sb, NULL);
    else if (is_abbrev(zonenum, "private"))
      room_iterate(room_db, print_private_room, sb, NULL);
    else if (is_abbrev(zonenum, "noflee"))
      room_iterate(room_db, print_noflee_room, sb, NULL);
    else if (is_abbrev(zonenum, "arena"))
      room_iterate(room_db, print_arena_room, sb, NULL);
    else if (isalpha(zonenum[0])){
      register int i;
      for (i = 0; i < WORLD_SIZE; i++) {
	TRoom *temp = real_roomp(i);
	if (temp && temp->name && strstr(temp->name, zonenum)){
	  print_room(i, temp, sb, NULL);
	}
      }
    } else {
      int zone;
      if (1 != sscanf(zonenum, "%i", &zone))
        zone = roomp->getZoneNum();
      if (zone < 0 || zone >= (signed int) zone_table.size())
        sb += "Zone number too high or too low.\n\r";
      else {
        struct show_room_zone_struct srzs;
        srzs.bottom = zone ? (zone_table[zone - 1].top + 1) : 0;
        srzs.top = zone_table[zone].top;

        srzs.blank = 0;
        room_iterate(room_db, show_room_zone, sb, &srzs);

        if (srzs.blank) {
          sb += srzs.sb;
          sprintf(buf, "rooms %d-%d are blank.\n\r", srzs.startblank, srzs.lastblank);
          sb += buf;
          srzs.blank = 0;
        } else
          sb += srzs.sb;
      }
    }
  } else if (is_abbrev(buf, "materials")) {
    int matnum=-1, i;

    strcpy(buf2, argument);
    if(*buf2){  
      // one material
      for(i=0;i<200;++i){
	if(material_nums[i].mat_name[0] &&
	   is_abbrev(buf2, material_nums[i].mat_name)){
	  matnum=i;
	}
      }

      if(matnum==-1)
	matnum=convertTo<int>(buf2);

      sb += describeMaterial(matnum);
    } else {
      // list materials
      sb += "Material Material\n\r";
      sb += "Number   Name\n\r";
      sb += "------------------\n\r";
      for(i=0;i<200;++i){
	if(material_nums[i].mat_name[0]){
	  buf[0]='\0';
	  sprintf(buf, "%-9i %s\n\r", i, 
		  sstring(material_nums[i].mat_name).uncap().c_str());
	  sb += buf;
	}
      }
    }
  } else if (is_abbrev(buf, "free")) {
    bool shFrError = false,
         isMobileF = true;
    unsigned long int shFrTotalCount = 0;

    for (; isspace(*argument); argument++);

    if (!*argument) {
      sb += "Syntax: show free <mob/obj> <#> <#>\n\r";
      shFrError = true;
    } else {
      argument = one_argument(argument, buf2); // get <mob/obj>

      if (!is_abbrev(buf2, "mobiles") && !is_abbrev(buf2, "objects")) {
        sb += "Syntax: show free <mob/obj> <#> <#>\n\r";
        shFrError = true;
      } else {
        if (is_abbrev(buf2, "objects"))
          isMobileF = false;

        for (; isspace(*argument); argument++);

        if (*argument) {
          argument = one_argument(argument, buf2); // get <all/#>

          if (is_abbrev(buf2, "all")) {
            if (!hasWizPower(POWER_SHOW_TRUSTED)) {
              sb += "Syntax: show free <mob/obj> <#> <#>\n\r";
              shFrError = true;
            } else {
              bottom = 0;
              top = zone_table.size() - 1;
            }
          } else {
            bottom = convertTo<int>(buf2);
            for (; isspace(*argument); argument++);

            if (!*argument)
              top = bottom;
            else {
              argument = one_argument(argument, buf2); // get 2nd <#>
              top = convertTo<int>(buf2);
            }
          }
        } else {
          if (!roomp) {
            vlogf(LOG_BUG, "show free called by being with no current room.");
            return;
          }

          top = bottom = roomp->getZoneNum();
        }
      }
    }

    if (!shFrError && ((bottom < 0 || top > ((signed int) zone_table.size() - 1)) ||
                       top < bottom)) {
      sb += "Zone number incorrect.\n\r";
      shFrError = true;
    }

    if (!shFrError) {
      if (top == bottom)
        sprintf(buf2, "%d", top);
      else if (!hasWizPower(POWER_SHOW_TRUSTED))
        sprintf(buf2, "%d/%d", bottom, top);
      else
        sprintf(buf2, "%d,...,%d", bottom, top);

      sprintf(buf, "Showing Free %s Entires in Zone: %s\n\r--------------------\n\r",
              (isMobileF ? "Mobiles" : "Objects"), buf2);
      sb += buf;

      if (top == bottom)
        shFrTotalCount += showFreeMobObj(top, &sb, isMobileF);
      else if (!hasWizPower(POWER_SHOW_TRUSTED)) {
        shFrTotalCount += showFreeMobObj(top, &sb, isMobileF);
        shFrTotalCount += showFreeMobObj(bottom, &sb, isMobileF);
      } else
        for (int Runner = bottom; Runner < (top + 1); Runner++)
          shFrTotalCount += showFreeMobObj(Runner, &sb, isMobileF, true);

      sprintf(buf, "Total Count of %s: %lu\n\r",
              (isMobileF ? "Mobiles" : "Objects"), shFrTotalCount);
      sb += buf;
    }
  } else if (is_abbrev(buf, "created")) {
    char tString[256],
         tBuffer[256],
         tBuf[256];
    int  tTotalCount = 0;

    for (; isspace(*argument); argument++);

    sstring tStArgument(argument),
           tStType(""),
           tStItemType("");

    tStType=tStArgument.word(0);
    tStItemType=tStArgument.word(1);

    if (tStType.empty() ||
        (!is_abbrev(tStType, "materialize") &&
         !is_abbrev(tStType, "spontaneous")))
      sb = "Syntax: show created <materialize/spontaneous>\n\r";
    else {
      int maxCost = (is_abbrev(tStType, "materialize") ? MATERIALIZE_PRICE : SPONT_PRICE),
          minCost = (is_abbrev(tStType, "materialize") ? -1 : MATERIALIZE_PRICE);

      if (is_abbrev(tStType, "materialize"))
        sb = "Materialize Objects:\n\r";
      else
        sb = "Spontaneous Generation Objects:\n\r";

      for (int tObjectIndex = 0; tObjectIndex < (signed) obj_index.size(); tObjectIndex++)
        if (!alchemy_create_deny(tObjectIndex) &&
	    obj_index[tObjectIndex].value <= maxCost &&
            obj_index[tObjectIndex].value >  minCost &&
            (tStItemType.empty() ||
             is_abbrev(tStItemType, ItemInfo[obj_index[tObjectIndex].itemtype]->name))) {
          strcpy(tBuffer, obj_index[tObjectIndex].short_desc);

          if (colorString(this, desc, tBuffer, NULL, COLOR_NONE, TRUE).length() > 40) {
            tBuffer[38] = '\0';
            strcat(tBuffer, "...<z>");
          }

          // This corrects the 'have color code will misalign' problem.
          int factualSpace = strlen(tBuffer) - strlen(colorString(this, desc, tBuffer,
                                                                  NULL, COLOR_NONE,
                                                                  TRUE).c_str());

          sprintf(tBuf, "[%%5d] [%%4d] %%-%ds (%%s)\n\r", (40 + factualSpace));

          sprintf(tString, tBuf,
                  obj_index[tObjectIndex].virt, obj_index[tObjectIndex].value,
                  tBuffer,
                  ItemInfo[obj_index[tObjectIndex].itemtype]->name);
           sb += tString;
           tTotalCount++;
        }

      sprintf(tString, "Total Count: %d\n\r", tTotalCount);
      sb += "\n\r";
      sb += tString;

      tStArgument = tString;
      tStArgument += "\n\r";
      tStArgument += sb;
      sb = tStArgument;
    }
  } else if (is_abbrev(buf, "components")) {
    TComponent *tComponent = NULL;
    int         tValue = -1;
    char        tString[256],
                tBuffer[256];

    for (; isspace(*argument); argument++);

    if (*argument)
      for (int tCompIndex = 0; tCompIndex < (signed) CompInfo.size() && tValue == -1; tCompIndex++)
        if (discArray[CompInfo[tCompIndex].spell_num] &&
            is_abbrev(argument, discArray[CompInfo[tCompIndex].spell_num]->name)) {
          tValue = CompInfo[tCompIndex].spell_num;
          break;
        }

    sb = "Showing Component Information:\n\r";

    for (int tObjectIndex = 0; tObjectIndex < (signed) obj_index.size(); tObjectIndex++)
      if (obj_index[tObjectIndex].itemtype == ITEM_COMPONENT)
        if ((tComponent = dynamic_cast<TComponent *>(read_object(obj_index[tObjectIndex].virt, VIRTUAL)))) {
          if (tValue == -1 || tComponent->getComponentSpell() == tValue) {
            int tError = (tComponent->getComponentSpell() <= TYPE_UNDEFINED ? 0 :
                          (!discArray[tComponent->getComponentSpell()] ? -1 : 1));

            tBuffer[0] = '\0';

            if (tComponent->isComponentType(COMP_DECAY))
              strcat(tBuffer, "D");
            else
              strcat(tBuffer, " ");

            if (tComponent->isComponentType(COMP_POTION))
              strcat(tBuffer, "B");
            else
              strcat(tBuffer, " ");

            if (tComponent->isComponentType(COMP_SCRIBE))
              strcat(tBuffer, "S");
            else
              strcat(tBuffer, " ");

            sprintf(tString, "%25s [%s] %s\n\r",
                    (!tError ? "Undefined" : (tError == -1 ? "UNKNOWN/BOGUS" :
                     discArray[tComponent->getComponentSpell()]->name)),
                    tBuffer, tComponent->getName());
            sb += tString;

            // This little function has alot of slap to it.  We don't want
            // to call this 20+ times in rapid succession or we will probably
            // mimic 'where leather'.  Therefore we Only show it when the user
            // has requested a specific spell type.  This way we are really
            // limited to at Most 3 iterations.
            if (*argument)
              sb += showComponentTechnical(tComponent->objVnum());
          }

          delete tComponent;
          tComponent = NULL;
        }
  } else if (is_abbrev(buf, "newfactions")) {
    for (; isspace(*argument); argument++);
    show_faction(mud_str_dup(argument));
  } else {
    sb += "Usage:\n\r";
    sb += "  show zones (<zonename> | \"disabled\")\n\r";
    sb += "  show objects (zone#|name|maxed)\n\r";
    sb += "  show (maxed|overmax)\n\r";
    sb += "  show mobiles (zone#|name|\"pets\"|\"hunters\"|\"bounty\"|\"response\")\n\r";

    sb += "  show free (mobiles|objects) (zone#/all) <zone#>\n\r";

    sb += "  show rooms (zone#)\n\r";
    sb += "  show rooms (\"death\"|\"saverooms\"|\"lit\"|\"noflee\"|\"private\"|\"hospital\"|\"noheal\")\n\r";
    sb += "  show <races | factions | trapped | fights >\n\r";
    sb += "  show materials (<material number>)\n\r";
    sb += "  show created <materialize/spontaneous> <itemtype>\n\r";
    sb += "  show components <spellname>\n\r";
    sb += "  show factions [<faction name> | <faction ID>]\n\r";
  }

  if (desc)
    desc->page_string(sb, SHOWNOW_NO, ALLOWREP_YES);
  return;
}






// ----- New Show Code Below Here:
#if 0
      case  1: // zones
        ShowZones();
        break;
      case  2: // objects
        ShowObjects();
        break;
      case  3: // mobiles
        ShowMobiles();
        break;
      case  4: // free
        ShowFree();
        break;
      case  5: // rooms
        ShowRooms();
        break;
      case  6: // created
        ShowCreated();
        break;
      case  7: // components
        ShowComponents();
        break;
      case  8: // materials
        ShowMaterials();
        break;
      case  9: // races
        ShowRaces();
        break;
      case 10: // factions
        ShowFactions();
        break;
      case 11: // trapped
        ShowTrapped();
        break;
      case 12: // fights
        ShopFights();
        break;










unsigned long int showFreeMobObj(int shFrZoneNumber, sstring *sb,
                                 bool isMobileF, bool shFrLoop=false)
{
  if (shFrZoneNumber < 0 || shFrZoneNumber >= ((signed int) zone_table.size())) {
    *sb += "Zone Number incorect.\n\r";
    return 0;
  }
                int shFrTop = 0,
                    shFrBot = 0,
                    shFrTopR = -1,
                    shFrBotR = -1;
  unsigned long int shFrTotalCount[2] = {0, 0};
  zoneData          &zd = zone_table[shFrZoneNumber];

  if (!zd.enabled)
    return 0;

  shFrTop = zd.top;
  shFrBot = (shFrZoneNumber ? zone_table[shFrZoneNumber - 1].top + 1: 0);

  int  shFrCountSize = (shFrTop - shFrBot + 1),
       shFrCountMax  = (isMobileF ? mob_index.size() : obj_index.size());
  bool shFrCountList[shFrCountSize];
  char tString[256];

  for (int Runner = 0; Runner < shFrCountSize; Runner++)
    shFrCountList[Runner] = false;

  shFrTotalCount[0] = shFrCountSize;

  for (int Runner = 0; Runner < shFrCountMax; Runner++) {
    if (( isMobileF && (mob_index[Runner].virt < shFrBot || mob_index[Runner].virt > shFrTop)) ||
        (!isMobileF && (obj_index[Runner].virt < shFrBot || obj_index[Runner].virt > shFrTop)))
      continue;

    int shFrWalkVirt = (isMobileF ? mob_index[Runner].virt : obj_index[Runner].virt) - shFrBot;

    shFrCountList[max(min(shFrWalkVirt, (shFrCountSize - 1)), 0)] = true;
    shFrTotalCount[0]--;
  }

  if (shFrTotalCount[0] > 0) {
    if (shFrLoop) {
      sprintf(tString, "**** Zone: %d\n\r", shFrZoneNumber);
      *sb += tString;
    }

    for (int Runner = 0; Runner < shFrCountSize; Runner++) {
      if (shFrCountList[Runner]) {
        if (shFrBotR != -1) {
          shFrTotalCount[1] = (shFrTopR - shFrBotR + 1);
          sprintf(tString, "%5d - %5d : %5lu Free\n\r",
                  shFrBotR, shFrTopR, shFrTotalCount[1]);
          *sb += tString;
        }

        shFrBotR = shFrTopR = -1;
      } else {
        if (shFrBotR == -1)
          shFrBotR = shFrTopR = (Runner + shFrBot);
        else
          shFrTopR = (Runner + shFrBot);

        if (Runner == (shFrCountSize - 1)) {
          shFrTotalCount[1] = (shFrTopR - shFrBotR + 1);
          sprintf(tString, "%5d - %5d : %5lu Free\n\r",
                  shFrBotR, shFrTopR, shFrTotalCount[1]);
          *sb += tString;
        }
      }
    }

    sprintf(tString, "----- Total Count: %5lu\n\r", shFrTotalCount[0]);
    *sb += tString;

    return shFrTotalCount[0];
  }

  return 0;
}

// Does major searching and returns the following:
// Dissection loads
// 'Nature' loads
// Scriptfile loads
sstring showComponentTechnical(const int tValue)
{
  sstring         tStString(""),
                 tStBuffer("");
  char           tString[256],
                 tBuffer[256];
  int            tMobNum;
  struct dirent *tDir;
  DIR           *tDirInfo;
  FILE          *tFile;

  // Check for dissection loads.
  // This doesn't check for hard-coded ones such as 'by race' and such.
  for (unsigned int tDissectIndex = 0; tDissectIndex < dissect_array.size(); tDissectIndex++)
    if ((dissect_array[tDissectIndex].loadItem == (unsigned) tValue)) {
      tMobNum = real_mobile(tDissectIndex);

      if (tMobNum < 0 || tMobNum > (signed) mob_index.size())
        strcpy(tBuffer, "[Unknown]");
      else
        strcpy(tBuffer, mob_index[tMobNum].name);

      sprintf(tString, "Dissect Load: %d %s\n\r", tDissectIndex, tBuffer);
      tStString += tString;
    }

  // Check for natural loads.  Unfortunatly it's easy to do a double entry here
  // so we have to be careful.
  for (unsigned int tCompIndex = 0; tCompIndex < component_placement.size(); tCompIndex++)
    if (component_placement[tCompIndex].number == tValue &&
        (component_placement[tCompIndex].place_act & CACT_PLACE)) {
      if (component_placement[tCompIndex].room2 == -1)
        tBuffer[0] = '\0';
      else
        sprintf(tBuffer, "-%d", component_placement[tCompIndex].room2);

      sprintf(tString, "Natural Load: Room%s %d%s\n\r",
              (!tBuffer[0] ? "" : "s"),
              component_placement[tCompIndex].room1,
              tBuffer);
      tStString += tString;
    }

  // Check for script loads.  This will go through ALL of the scripts and check.
  // We only do this on !PROD because of the lag it will generate, and I do mean a
  // LOT of lag it will make.
  if (gamePort != PROD_GAMEPORT) {
    if (!(tDirInfo = opendir("mobdata/responses"))) {
      vlogf(LOG_FILE, "Unable to dirwalk directory mobdata/resposnes");
      tStString += "ERROR.  Unable to open mobdata/responses for reading.";
      return tStString;
    }

    while ((tDir = readdir(tDirInfo))) {
      if (!strcmp(tDir->d_name, ".") || !strcmp(tDir->d_name, ".."))
        continue;

      sprintf(tBuffer, "mobdata/responses/%s", tDir->d_name);

      if (!(tFile = fopen(tBuffer, "r")))
        continue;

      while (fgets(tString, 256, tFile)) {
        char *tChar = tString;

        for (; isspace(*tChar) || *tChar == '\t'; tChar++);

        sprintf(tBuffer, "load %d;\n", tValue);

        if (!strcmp(tChar, tBuffer)) {
          tMobNum = real_mobile(convertTo<int>(tDir->d_name));

          if (tMobNum < 0 || tMobNum > (signed) mob_index.size())
            strcpy(tString, "[Unknown]");
          else
            strcpy(tString, mob_index[tMobNum].name);

          sprintf(tBuffer, "Script: %s %s\n\r",
                  tDir->d_name, tString);
          tStString += tBuffer;

          // Don't show the same entry twice.
          break;
        }
      }

      fclose(tFile);
    }

    closedir(tDirInfo);
  }

  return tStString;
}




const char SHOW_LIST[][15] =
{
  "zones",      //  1
  "objects",    //  2
  "mobiles",    //  3
  "free",       //  4
  "rooms",      //  5
  "created",    //  6
  "components", //  7
  "materials",  //  8
  "races",      //  9
  "factions",   // 10
  "trapped",    // 11
  "fights",     // 12
  "\n"
};

const int MAX_SHOW_LIST_ENTIRES = 13;

void TBeing::doShow(const char *)
{
  return;
}

void TPerson::doShow(sstring tStString)
{
  bool   tError        = false;
  int    tSelection    = 0;
  char   tString[1024] = "\0";
  sstring tSb("");

  if (tStString.empty())
    tError = true;
  else
    bisect_arg(tStString.c_str(), &tSelection, tString, SHOW_LIST);

  if (!in_range(1, MAX_SHOW_LIST_ENTRIES))
    tError = true;

  if (!tError) {
    sstring tStArg(tString),
           tStFirst(""),
      tStSecond("");

    tStFirst=tStArg.word(0);
    tStSecond=tStArg.word(1);

    switch (tSelection) {
      case  1: // zones
        ShowZones();
        break;
      case  2: // objects
        ShowObjects();
        break;
      case  3: // mobiles
        ShowMobiles();
        break;
      case  4: // free
        ShowFree();
        break;
      case  5: // rooms
        ShowRooms();
        break;
      case  6: // created
        ShowCreated();
        break;
      case  7: // components
        ShowComponents();
        break;
      case  8: // materials
        ShowMaterials();
        break;
      case  9: // races
        ShowRaces();
        break;
      case 10: // factions
        ShowFactions();
        break;
      case 11: // trapped
        ShowTrapped();
        break;
      case 12: // fights
        ShopFights();
        break;
      default:
        vlogf(LOG_BUG, fmt("Unregistered entry in show: %d") %  tSelection);
        break;
    }
  } else {
    tSb += "Syntax: show <type> <arguments>\n\r";
    tSb += "  zones (<zonename> | \"disabled\")\n\r";
    tSb += "  objects (zone#|name)\n\r";
    tSb += "  mobiles (zone#|name|\"pets\"|\"hunters\"|\"bounty\"|\"response\")\n\r";
    tSb += "  free (mobiles|objects) (zone#/all) (zone#)\n\r";
    tSb += "  rooms (zone#|flagname{See EDIT FLAGS})\n\r";
    tSb += "  created (\"materialize\"|\"spontaneous\") (itemtype)\n\r";
    tSb += "  components (spellname)\n\r";
    tSb += "  materials (materia#)\n\r";
    tSb += "  (races|factions|trapped|fights)\n\r";
  }

  if (desc)
    desc->page_string(tSb, SHOWNOW_NO, ALLOWREP_YES);
}
#endif
