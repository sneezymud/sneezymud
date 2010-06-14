//////////////////////////////////////////////////////////////////////////
//
// SneezyMUD - All rights reserved, SneezyMUD Coding Team
//
//      "cmd_stat.cc" - The stat command
//  
//////////////////////////////////////////////////////////////////////////

#include "extern.h"
#include "handler.h"
#include "room.h"
#include "being.h"
#include "low.h"
#include "account.h"
#include "materials.h"
#include "disease.h"
#include "spec_rooms.h"
#include "monster.h"
#include "combat.h"
#include "spec_mobs.h"
#include "person.h"
#include "guild.h"
#include "skills.h"
#include "shop.h"

extern int eqHpBonus(const TPerson *);
extern int baseHp();
extern float classHpPerLevel(const TPerson *);
extern int ageHpMod(const TPerson *);
extern int getObjLoadPotential(const int obj_num);

void TBeing::statZone(const sstring &zoneNumber)
{
  // this probably has the potential to be wildly innaccurate now
  
  int zone_num;
  int count_descriptions = 0;
  int count_titles = 0;
  int count_extras = 0;
  int count_flags[7] = {0, 0, 0, 0, 0, 0, 0};
  int count_rooms = 0;
  int room_start, room_end;
  // int count_non_ethereal = 0;
  int room_loop = 0;
  sstring out("");
  sstring sb;
  TRoom *roomp_current;
  
  int count_mobs_in_block = 0;
  int count_objs_in_block = 0;
  
  if (zoneNumber.empty()) {
    if (!roomp) {
      vlogf(LOG_BUG, "statZone called by being with no current room.");
      return;
    }

    zone_num = roomp->getZoneNum();
  } else {
    zone_num = convertTo<int>(zoneNumber);
  }

  if (zone_num < 0 || zone_num >= (signed int) zone_table.size()) {
    sendTo("Zone number incorrect.\n\r");
    return;
  }
  
  zoneData zoned = zone_table[zone_num];
  
  room_start = (zone_num ? zone_table[zone_num - 1].top + 1 : 0);
  room_end = zoned.top;
  
  for (room_loop = room_start; room_loop < (room_end + 1); room_loop++) {
    if ((roomp_current = real_roomp(room_loop))) {
      ++count_rooms;
      
      if (roomp_current->getDescr() && strncmp(roomp_current->getDescr(), "Empty", 5)) {
        // count of non 'Empty' descriptions
        ++count_descriptions;
      }
      
      if (roomp_current->name) {
        // count titles
        sstring name_num = format("%d") % roomp_current->number;
        if (name_num.find(roomp_current->name) != sstring::npos) {
          // the room name is the same as the room vnum... consider it untitled
        } else {
          ++count_titles;
        }
      }
      
      if (roomp_current->ex_description) {
        // count extra descriptions
        ++count_extras;
      }
      
      if (roomp_current->isRoomFlag(ROOM_DEATH    ))// Count DEATH_ROOM flags
        count_flags[0]++;
      if (roomp_current->isRoomFlag(ROOM_NO_FLEE  ))// Count NO_FLEE flags
        count_flags[1]++;
      if (roomp_current->isRoomFlag(ROOM_PEACEFUL ))// Count PEACEFUL flags
        count_flags[2]++;
      if (roomp_current->isRoomFlag(ROOM_NO_HEAL  ))// Count NO_HEAL flags
        count_flags[3]++;
      if (roomp_current->isRoomFlag(ROOM_SAVE_ROOM))// Count SAVE_ROOM flags
        count_flags[4]++;
      if (roomp_current->isRoomFlag(ROOM_INDOORS  ))// Count INDOOR flags
        count_flags[5]++;
      if (roomp_current->isRoomFlag(ROOM_PEACEFUL) &&
          !roomp_current->isRoomFlag(ROOM_NO_HEAL))// If is Peaceful should ALWAYS be no-heal
        count_flags[6]++;
      
      // if (roomp_current->getSectorType() != SECT_ASTRAL_ETHREAL) {
      //   ++count_non_ethereal;
      // }
    }
  }
  
  int mob_lvl_avg_block = 0;
  int mob_lvl_low_block = 0;
  int mob_lvl_high_block = 0;
  for (unsigned int mi = 0; mi < mob_index.size(); mi++) {
    if (mob_index[mi].virt < room_start || mob_index[mi].virt > room_end)
      continue;
    // sendTo(format("Mob %u %i %s\n\r") % mi % mob_index[mi].virt % mob_index[mi].short_desc);
    if (!mob_lvl_low_block)
      mob_lvl_low_block = (int) mob_index[mi].level;
    mob_lvl_avg_block += (int) mob_index[mi].level;
    mob_lvl_low_block = min(mob_lvl_low_block, (int) mob_index[mi].level);
    mob_lvl_high_block = max(mob_lvl_high_block, (int) mob_index[mi].level);
    ++count_mobs_in_block;
  }
  if (count_mobs_in_block)
    mob_lvl_avg_block /= count_mobs_in_block;
  
  for (unsigned int oi = 0; oi < obj_index.size(); oi++) {
    if (obj_index[oi].virt < room_start || obj_index[oi].virt > room_end)
      continue;
    // sendTo(format("Obj %u %i %s\n\r") % oi % obj_index[oi].virt % obj_index[oi].short_desc);
    ++count_objs_in_block;
  }
  
  out += "<g>General Zone Info<1>\n\r";
  out += format("Name:         <c>%-s<1>\n\r") % zoned.name;
  out += format("Zone num:     <c>%-3d<1>       Active:   <c>%-s<1>\n\r") % zone_num % (zoned.enabled ? "Enabled" : "Disabled");
  out += format("Start room:   <c>%-5d<1>     End room: <c>%-5d<1>\n\r") % room_start % room_end;
  sstring reset_mode;
  switch (zoned.reset_mode) {
    case 0:
      reset_mode = "<c>Never reset<1>";
      break;
    case 1:
      reset_mode = "<c>Reset when empty<1>";
      break;
    case 2:
      reset_mode = "<c>Reset at lifespan<1>";
      break;
    default:
      reset_mode = format("<r>Unknown mode<1> <R>%i<1>") % zoned.reset_mode;
      break;
  }
  out += format("Reset mode:   %s\n\r") % reset_mode;
  sb = format("<c>%i<1>/<c>%i<1>") % zoned.lifespan % zoned.age;
  out += format("Lifespan/age: %-21s Block size/used: <c>%i<1>/<c>%i<1>\n\r\n\r")  % sb % ((room_end - room_start) + 1) % count_rooms;
  
  out += "<g>Room Info<1>\n\r";
  out += format("Descriptions: <c>%-d<1>\n\r") % count_descriptions;
  out += format("Titles:       <c>%-d<1>\n\r") % count_titles;
  out += format("Extras:       <c>%-d<1>\n\r") % count_extras;
  out += format("Indoors:      <c>%-d<1>\n\r\n\r") % count_flags[5];
  //out += format("Non-ethereal sectors: <c>%d<1>\n\r\n\r") % count_non_ethereal;
  
  int mob_lvl_low = 0;
  int mob_lvl_high = 0;
  int mob_lvl_avg_unique = 0;
  int mob_lvl_avg_total = 0;
  
  std::map<int,int>::iterator iter;
  for (iter = zoned.stat_mobs.begin(); iter != zoned.stat_mobs.end(); iter++ ) {
    if (!mob_lvl_low)
      mob_lvl_low = (int) mob_index[iter->first].level;
    mob_lvl_low = min(mob_lvl_low, (int) mob_index[iter->first].level);
    mob_lvl_high = max(mob_lvl_high, (int) mob_index[iter->first].level);
    mob_lvl_avg_unique += (int) mob_index[iter->first].level;
    mob_lvl_avg_total += ((int) mob_index[iter->first].level) * iter->second;
  }
  if (zoned.stat_mobs_unique)
    mob_lvl_avg_unique /= zoned.stat_mobs_unique;
  if (zoned.stat_mobs_total)
    mob_lvl_avg_total /= zoned.stat_mobs_total;
  out += "<g>Mob Info<1>\n\r";
  out += format("Unique mobs in zonefile:    <c>%-4d<1>  Low/avg/high: <c>%d<1>/<c>%d<1>/<c>%d<1>\n\r") % zoned.stat_mobs_unique % mob_lvl_low % mob_lvl_avg_unique % mob_lvl_high;
  out += format("Total mobs in zonefile:     <c>%-4d<1>  Avg:          <c>%d<1>\n\r") % zoned.stat_mobs_total % mob_lvl_avg_total;
  out += format("Unique mobs in block:       <c>%-4d<1>  Low/avg/high: <c>%d<1>/<c>%d<1>/<c>%d<1>\n\r") % count_mobs_in_block % mob_lvl_low_block % mob_lvl_avg_block % mob_lvl_high_block;
  TBeing *b;
  TMonster *monst;
  int response_scripts_block = 0;
  int response_scripts_zonefile = 0;
  for (b = character_list; b; b = b->next) {
    if ((monst = dynamic_cast<TMonster *>(b)) && monst->resps && monst->resps->respList) {
      if (!(monst->mobVnum() < room_start || monst->mobVnum() > room_end)) {
        ++response_scripts_block;
      }
      if (zoned.stat_mobs[monst->getMobIndex()]) {
        ++response_scripts_zonefile;
      }
    }
  }
  out += format("Response mobs in zonefile:  <c>%-4d<1>  Response mobs in block: <c>%-4d<1>\n\r\n\r") % response_scripts_zonefile % response_scripts_block;
  
  out += "<g>Object Info<1>\n\r";
  out += format("Unique objects in zonefile: <c>%-d<1>\n\r") % zoned.stat_objs_unique;
  out += format("Unique objects in block:    <c>%-d<1>\n\r") % count_objs_in_block;
  
  desc->page_string(out, SHOWNOW_NO, ALLOWREP_YES);
}

void TBeing::statZoneMobs(sstring zoneNumber)
{
  int zone_num;
  sstring out("");
  
  if (zoneNumber.empty()) {
    if (!roomp) {
      vlogf(LOG_BUG, "statZone called by being with no current room.");
      return;
    }
    zone_num = roomp->getZoneNum();
  } else {
    zone_num = convertTo<int>(zoneNumber);
  }
  
  if (zone_num < 0 || zone_num >= (signed int) zone_table.size()) {
    sendTo("Zone number incorrect.\n\r");
    return;
  }
  
  zoneData zoned = zone_table[zone_num];
  
  int count = 0;
  std::map<int,int>::iterator iter;
  
  out += "<g>Zonefile Mobile Report<1>\n\r";
  out += format("<g>Zone name:<1>  %-s\n\r") % zoned.name;
  out += format("<g>Zone num:<1>   %-3d       <g>Active:<1>   %-s\n\r\n\r") % zone_num % (zoned.enabled ? "Enabled" : "Disabled");
  
  out += "      <c>Vnum   Max Count Class    Level Name<1>\n\r";
  for (iter = zoned.stat_mobs.begin(); iter != zoned.stat_mobs.end(); iter++ ) {
    ++count;
    // grab the class data, do funny stuff to display possible multi-classes with abbreviations
    sstring classy;
    bool got_class = FALSE;
    bool reset = FALSE;
    for (classIndT cl = MIN_CLASS_IND; cl < MAX_CLASSES; cl++) {
      if (IS_SET((int) mob_index[iter->first].Class, 1<<cl)) {
        if (got_class && !reset) {
          // mob is multi-classed, erase and start over with abbreviations
          cl = MIN_CLASS_IND;
          classy.clear();
          reset = TRUE;
        }
        if (got_class) {
          classy.append(classInfo[cl].abbr);
        } else {
          classy = classInfo[cl].name;
        }
        got_class = TRUE;
      }
    }
    if (classy.size() == 0) {
      classy = format("<R>%-8d<1>") % mob_index[iter->first].Class;
      vlogf(LOG_BUG, format("Unknown class bit (%d) in TBeing::statZoneMobs.") % mob_index[iter->first].Class);
    }
    
    // output the row
    out += format("<c>%-4d<1> %5d %5d %5d %-8s %5ld %s\n\r") % count 
    % mob_index[iter->first].virt % mob_index[iter->first].max_exist 
    % mob_index[iter->first].getNumber() % classy
    % mob_index[iter->first].level % mob_index[iter->first].name;
    // out += format("<c>%3d)<1> %5d  %s\n\r") % count % mob_index[iter->first].virt % mob_index[iter->first].short_desc;
  }
  
  desc->page_string(out, SHOWNOW_NO, ALLOWREP_YES);
}

void TBeing::statZoneObjs(sstring zoneNumber)
{
  int zone_num;
  sstring out("");
  
  if (zoneNumber.empty()) {
    if (!roomp) {
      vlogf(LOG_BUG, "statZone called by being with no current room.");
      return;
    }
    zone_num = roomp->getZoneNum();
  } else {
    zone_num = convertTo<int>(zoneNumber);
  }
  
  if (zone_num < 0 || zone_num >= (signed int) zone_table.size()) {
    sendTo("Zone number incorrect.\n\r");
    return;
  }
  
  zoneData zoned = zone_table[zone_num];
  
  int count = 0;
  std::map<int,int>::iterator iter;
  out += "<g>Zonefile Object Report<1>\n\r";
  out += format("<g>Zone name:<1>  %-s\n\r") % zoned.name;
  out += format("<g>Zone num:<1>   %-3d       <g>Active:<1>   %-s\n\r\n\r") % zone_num % (zoned.enabled ? "Enabled" : "Disabled");
  
  if (!hasWizPower(POWER_SHOW_TRUSTED)) {
    out += "      <c>Vnum Type              Name<1>\n\r";
  } else {
    out += "      <c>Vnum   Max Count  Value Type              Level Name<1>\n\r";
  }
  for (iter = zoned.stat_objs.begin(); iter != zoned.stat_objs.end(); iter++ ) {
    ++count;
    
    // try to speed things up - don't init an object unless we really need to
    // it's only used to determine level for certain item types
    // leaving the rest blank also helps with readability
    sstring olevel;
    if (obj_index[iter->first].itemtype == ITEM_WEAPON
        || obj_index[iter->first].itemtype == ITEM_ARMOR
        || obj_index[iter->first].itemtype == ITEM_WORN
        || obj_index[iter->first].itemtype == ITEM_HANDGONNE
        || obj_index[iter->first].itemtype == ITEM_HOLY_SYM
        || obj_index[iter->first].itemtype == ITEM_MARTIAL_WEAPON
        || obj_index[iter->first].itemtype == ITEM_JEWELRY
        || obj_index[iter->first].itemtype == ITEM_CANNON
        || obj_index[iter->first].itemtype == ITEM_GUN
        || obj_index[iter->first].itemtype == ITEM_ARROW) {
      TObj *obj = read_object(obj_index[iter->first].virt, VIRTUAL);
      olevel = format("%d") % (int) obj->objLevel();
      delete obj;
    }
    sstring itype = ItemInfo[obj_index[iter->first].itemtype]->name;
    
    if (!hasWizPower(POWER_SHOW_TRUSTED)) {
      out += format("<c>%-4d<1> %5d %-17s %s\n\r") % count % obj_index[iter->first].virt % itype % obj_index[iter->first].name;
    } else {
      out += format("<c>%-4d<1> %5d %5d %5d %6d %-17s %5s %s\n\r") % count 
        % obj_index[iter->first].virt % obj_index[iter->first].max_exist
        % obj_index[iter->first].getNumber() % obj_index[iter->first].value
        % itype % olevel % obj_index[iter->first].name;
    }
  }
  desc->page_string(out, SHOWNOW_NO, ALLOWREP_YES);
}

void TBeing::statRoom(TRoom *rmp)
{
  sstring str;
  sstring tmp_str;
  sstring buf2, buf3, buf4;
  extraDescription *e;
  TThing *t=NULL;
  int counter = 0, volume;

  if (!limitPowerCheck(CMD_EDIT, rmp->number)) {
    sendTo("You are not allowed to stat this room, sorry.\n\r");
    return;
  }


  str = format("%sRoom name:%s %s, %sOf zone:%s %d. %sV-Number:%s %d, %sR-number:%s %d\n\r") %
    cyan() % norm() % rmp->name %
    cyan() % norm() % rmp->getZoneNum() %
    cyan() % norm() % rmp->number %
    cyan() % norm() % in_room;

  str += format("%sRoom Coords:%s %d, %d, %d\n\r") %
    cyan() % norm() %
    rmp->getXCoord() % rmp->getYCoord() % rmp->getZCoord();

  unsigned int shop_nr=0;
  for (shop_nr = 0; (shop_nr < shop_index.size()) && 
	 (shop_index[shop_nr].in_room != rmp->number); shop_nr++);

  if (shop_nr < shop_index.size())
    str += format("%sShop Number:%s %i, %sShop Keeper:%s %i\n\r") %
      cyan() % norm() % shop_nr % cyan() % norm() % 
      mob_index[shop_index[shop_nr].keeper].virt;

  str += format("%sSector type:%s %s") %
    cyan() % norm() % TerrainInfo[rmp->getSectorType()]->name;

  str += format("  %sSpecial procedure:%s ") % cyan() % norm();

  str += format("%s\n\r") % ((rmp->spec) ? roomSpecials[rmp->spec].name : "None");

  str += format("%sRoom flags:%s ") % cyan() % norm();

  str += sprintbit((long) rmp->getRoomFlags(), room_bits);
  str += "\n\r";

  str += format("%sRoom flag bit vector:%s ") % cyan() % norm();

  str += format("%d\n\r") % ((unsigned int) rmp->getRoomFlags());

  str += format("%sDescription:%s\n\r") % cyan() % norm();
  tmp_str = rmp->getDescr();
  if (tmp_str.empty()) {
    str += "NO DESCRIPTION\n\r";
  } else {
    str += tmp_str.toCRLF();
  }

  str += format("%sExtra description keywords(s):%s") % cyan() % norm();
  if (rmp->ex_description) {
    str += "\n\r";
    for (e = rmp->ex_description; e; e = e->next) {
      str += e->keyword;
      str += "\n\r";
    }
    str += "\n\r";
  } else {
    str += " NONE\n\r";
  }

  buf3 = format("%d") % rmp->getRoomHeight();
  buf4 = format("%d") % rmp->getMoblim();
  str += format("%sLight:%s %d   %sRoom Height:%s %s    %sMaximum capacity:%s %s\n\r") %
    cyan() % norm() % rmp->getLight() %
    cyan() % norm() % ((rmp->getRoomHeight() <= 0) ? "Unlimited" : buf3) %
    cyan() % norm() % ((rmp->getMoblim()) ? buf4 : "Infinite");

  if (rmp->isWaterSector() || rmp->isUnderwaterSector()) {
    str += format("%sRiver direction:%s %s") % 
      cyan() % norm() %
      ((rmp->getRiverDir() < 0) ? "None" : dirs[rmp->getRiverDir()]);

    str += format("   %sRiver speed:%s ") % cyan() % norm();
    if (rmp->getRiverSpeed() >= 1)
      str += format("Every %d heartbeat%s\n\r") %
	rmp->getRiverSpeed() % ((rmp->getRiverSpeed() != 1) ? "s." : ".");
    else
      str += format("no current.\n\r");

    str += format("%sFish caught:%s %i\n\r") %
      cyan() % norm() % rmp->getFished();

  }
  if (rmp->isForestSector())
    str += format("Number of logs harvested: %i\n\r")
      % rmp->getLogsHarvested();
  if ((rmp->getTeleTarg() > 0) && (rmp->getTeleTime() > 0)) {
    str += format("%sTeleport speed:%s Every %d heartbeats.  %sTo room:%s %d  %sLook?%s %s.\n\r") %
      cyan() % norm() % rmp->getTeleTime() %
      cyan() % norm() % rmp->getTeleTarg() %
      cyan() % norm() % (rmp->getTeleLook() ? "yes" : "no");
  }
  str += format("%s------- Chars present -------%s\n\r") % cyan() % norm();
  counter = 0;
  for(StuffIter it=rmp->stuff.begin();it!=rmp->stuff.end() && (t=*it);++it) {
    // canSee prevents seeing invis gods of higher level
    if (dynamic_cast<TBeing *>(t) && canSee(t)) {
      counter++;
      if (counter > 15) {
         str += "Too Many In Room to Stat More\n\r";
         break;
      } else {
        str += format("%s%s   (%s)\n\r") %
	  t->getName() % (dynamic_cast<TPerson *>(t) ? "(PC)" : "(NPC)") %
	  t->name;
      }
    }
  }
  str += format("%s--------- Born Here ---------%s\n\r") % cyan() % norm();
  counter = 0;
  for (t = rmp->tBornInsideMe; t; t = t->nextBorn) {
    TMonster *tMonster;

    if ((tMonster = dynamic_cast<TMonster *>(t))) {
      counter++;

      if (counter > 5) {
        str += "Too Many Creators Born In Room To Show More\n\r";
        break;
      } else {
        str += format("[%6d] %s\n\r") % tMonster->mobVnum() % tMonster->getName();
      }
    }
  }
  str += format("%s--------- Contents ---------%s\n\r") % cyan() % norm();
  counter = 0;
  volume = 0;
  buf2="";
  for(StuffIter it=rmp->stuff.begin();it!=rmp->stuff.end() && (t=*it);++it) {
    if (!dynamic_cast<TBeing *>(t)) {
      volume += t->getVolume();
      counter++;
      if (counter > 30) {
        buf2 += "Too Many In Room to Stat More\n\r";
        break;
      } else {
        buf2 += format("%s   (%s)\n\r") % t->getName() % t->name;
      }
    }
  }
  str += format("%sTotal Volume:%s %s\n\r") %
    cyan() % norm() % volumeDisplay(volume);
  str += buf2;

  str += format("%s------- Exits defined -------%s\n\r") % cyan() % norm();
  dirTypeT dir;
  for (dir = MIN_DIR; dir < MAX_DIR; dir++) {
    if (!rmp->dir_option[dir]) {
      continue;
    } else {
      str+=format("%sDirection:%s %-10s    %sDoor Type:%s %-12s     %sTo-Room:%s %d\n\r") %
        cyan() % norm() %
	dirs[dir] %
        cyan() % norm() %
        door_types[rmp->dir_option[dir]->door_type] %
        cyan() % norm() %
	rmp->dir_option[dir]->to_room;
      if (rmp->dir_option[dir]->door_type != DOOR_NONE) {
        str += format("%sWeight:%s %d      %sExit Flags:%s %s\n\r%sKeywords:%s %s\n\r") %
          cyan() % norm() %
          rmp->dir_option[dir]->weight % 
          cyan() % norm() %
          sprintbit(rmp->dir_option[dir]->condition, exit_bits) %
          cyan() % norm() %
	  rmp->dir_option[dir]->keyword;
        if ((rmp->dir_option[dir]->key > 0) || 
             (rmp->dir_option[dir]->lock_difficulty >= 0)) {
          str += format("%sKey Number:%s %d     %sLock Difficulty:%s %d\n\r") %
            cyan() % norm() %
	    rmp->dir_option[dir]->key %
            cyan() % norm() %
            rmp->dir_option[dir]->lock_difficulty;
        }
        if (IS_SET(rmp->dir_option[dir]->condition, EX_TRAPPED)) {
          buf2 = sprinttype(rmp->dir_option[dir]->trap_info, trap_types);
          str += format("%sTrap type:%s %s,  %sTrap damage:%s %d (d8)\n\r") % 
            cyan() % norm() %
	    buf2 %
            cyan() % norm() %
            rmp->dir_option[dir]->trap_dam;
        }
      } else if (IS_SET(rmp->dir_option[dir]->condition, EX_SLOPED_UP)) {
        str += format("%sSloped:%s Up\n\r") % cyan() % norm();
      } else if (IS_SET(rmp->dir_option[dir]->condition, EX_SLOPED_DOWN)) {
        str += format("%sSloped:%s Down\n\r") % cyan() % norm();
      }
      str += format("%sDescription:%s\n\r") % cyan() % norm();
      tmp_str = rmp->dir_option[dir]->description;
      if (tmp_str.empty()) {
        str += "UNDEFINED\n\r";
      } else {
        str += tmp_str.toCRLF();
      }
    }
  }
  desc->page_string(str);
  return;
}

void TBeing::statObj(const TObj *j)
{
  extraDescription *e;
  TThing *t=NULL;
  int i;
  sstring str;

  if (!limitPowerCheck(CMD_OEDIT, j->getSnum())) {
    sendTo("You are not allowed to stat that object, sorry.\n\r");
    return;
  }

  str = format("Object name: [%s], R-number: [%d], V-number: [%d] Item type: ") %
    j->name % j->number % obj_index[j->getItemIndex()].virt;

  str += ItemInfo[j->itemType()]->name;
  str += "\n\r";

  for (unsigned int zone = 0; zone < zone_table.size(); zone++) {
    if(obj_index[j->getItemIndex()].virt <= zone_table[zone].top){
      str += format("Zone: %s\n\r") % zone_table[zone].name;
      break;
    }    
  }

  str += format("Short description: %s\n\rLong description:\n\r%s\n\r") %
    ((j->shortDescr) ? j->shortDescr : "None") %
    ((j->getDescr()) ? j->getDescr() : "None");

  if (j->action_description) {
    str += "Action Description: ";
    str += j->action_description;
    str += "\n\r";
  }

  str += format("Action pointer: %s\n\r") % (j->act_ptr ? "YES" : "no");

  if (j->ex_description) {
    str += "Extra description keyword(s):\n\r----------\n\r";

    for (e = j->ex_description; e; e = e->next) {
      str += e->keyword;
      str += "\n\r";
    }
    str += "----------\n\r";
  } else {
    str += "Extra description keyword(s): None.\n\r";
  }

  if (j->owners) {
    str += format("Owners: [%s]\n\r") % j->owners;
  }

  str += "Can be worn on: ";
  str += sprintbit(j->obj_flags.wear_flags, wear_bits);
  str += "\n\r";

  str += "Set char bits : ";
  str += sprintbit_64(j->obj_flags.bitvector, affected_bits);
  str += "\n\r";

  str += "Extra flags   : ";
  str += sprintbit(j->getObjStat(), extra_bits);
  str += "\n\r";

  str += format("Can be seen   : %d\n\r") % int(j->canBeSeen);

  str += format("Volume: %d, Weight: %.1f, Value: %d, Cost/day: %d\n\r") %
    j->getVolume() % j->getWeight() %
    j->obj_flags.cost % j->rentCost();

  str += format("Indexed Cost: %d, Suggested Price: %d, Material Value: %d\n\r") %
    obj_index[j->getItemIndex()].value % j->suggestedPrice() %
    (int)(j->getWeight() * 10.0 * material_nums[j->getMaterial()].price);


  str += format("Decay: %d, Max Struct: %d, Struct Left: %d, Depreciation: %d\n\r") %
    j->obj_flags.decay_time % j->getMaxStructPoints() %
    j->getStructPoints() % j->getDepreciation();

  str += format("Light: %3d          Material Type: %s (%i)\n\r") %
    j->getLight() % material_nums[j->getMaterial()].mat_name %
    j->getMaterial();

  if (j->inRoom() != Room::NOWHERE)
    str += format("In Room: %d\n\r") % j->inRoom();
  else if (j->parent)
    str += format("Inside: %s\n\r") % j->parent->getName();
  else if (j->stuckIn)
    str += format("Stuck-In: %s (slot=%d)\n\r") %
      j->stuckIn->getName() % j->eq_stuck;
  else if (j->equippedBy)
    str += format("Equipped-by: %s (slot=%d)\n\r") %
      j->equippedBy->getName() % j->eq_pos;
  else
    str += "UNKNOWN LOCATION !!!!!!\n\r";

  str += format("Carried weight: %.1f   Carried volume: %d\n\r") %
    j->getCarriedWeight() % j->getCarriedVolume();

  str += j->statObjInfo();

  str += format("\n\rSpecial procedure: %s   ") % (j->spec ? objSpecials[GET_OBJ_SPE_INDEX(j->spec)].name : "none");

  if(j->stuff.empty())
    str += "Contains : Nothing\n\r";
  else {
    str += "Contains :\n\r";
    for(StuffIter it=j->stuff.begin();it!=j->stuff.end() && (t=*it);++it) {
      //      str += fname(t->name);
      str += t->shortDescr;
      str += " (";
      str += t->name;
      str += ")";
      str += "\n\r";
    }
  }

  str += "Can affect char:\n\r";
  for (i = 0; i < MAX_OBJ_AFFECT; i++) {
    if (j->affected[i].location == APPLY_SPELL) {
      if (discArray[j->affected[i].modifier]) {
        str += format("    Affect:  %s: %s by %ld\n\r") %
          apply_types[j->affected[i].location].name %
          discArray[j->affected[i].modifier]->name %
          j->affected[i].modifier2;
      } else {
        vlogf(LOG_BUG, format("BOGUS AFFECT (%ld) on %s") %
          j->affected[i].modifier % j->getName());
      }
    } else if (j->affected[i].location == APPLY_DISCIPLINE) {
      if (discNames[j->affected[i].modifier].disc_num) {
        str += format("    Affect:  %s: %s by %ld\n\r") %
          apply_types[j->affected[i].location].name %
          discNames[j->affected[i].modifier].name %
          j->affected[i].modifier2;
      } else {
        vlogf(LOG_BUG, format("BOGUS AFFECT (%ld) on %s") %
          j->affected[i].modifier % j->getName());
      }
    } else if (j->affected[i].location == APPLY_IMMUNITY) {
      str += format("    Affect:  %s: %s by %ld\n\r") %
        apply_types[j->affected[i].location].name %
        immunity_names[j->affected[i].modifier] %
        j->affected[i].modifier2;
    } else if (j->affected[i].location != APPLY_NONE) {
      if (!strcmp(apply_types[j->affected[i].location].name, "Magic Affect")) {
        for (unsigned long nr = 0; ; ++nr) {
          // loop over all item perma-affect flags
          if (*affected_bits[nr] == '\n')
            break;
          if (1<<nr & j->affected[i].modifier) {
            // item has affect
            if (*affected_bits[nr]) {
              str += format("    Affect:  Magic Affect of %s (%d)\n\r") % affected_bits[nr] % int(1<<nr);
            } else {
              str += format("    Affect:  Magic Affect of %d\n\r") % int(1<<nr);
            }
          }
        }
      } else {
        str += format("    Affect:  %s by %ld\n\r") % apply_types[j->affected[i].location].name % j->affected[i].modifier;
      }
    }
  }
  desc->page_string(str);
  return;
}

void TBeing::statObjForDivman(const TObj *j)
{
  TThing *t=NULL;
  sstring str = "\n\r";
  sstring sitem = j->shortDescr;
  
  str += format("%s is %s made of %s.\n\r") % sitem.cap() % ItemInfo[j->itemType()]->common_name % material_nums[j->getMaterial()].mat_name;
  
  for (unsigned int zone = 0; zone < zone_table.size(); zone++) {
    if(obj_index[j->getItemIndex()].virt <= zone_table[zone].top) {
      str += format("It is from %s.\n\r") % zone_table[zone].name;
      break;
    }    
  }
  
  str += (j->wear_flags_to_sentence());
  
  str += "Its extra flags are: " + sprintbit(j->getObjStat(), extra_bits) + "\n\r\n\r";
  
  str += format("%s modifies light by %d.\n\r") % sitem.cap() % j->getLight();
  str += format("At least %d units of light are needed to see it.\n\r") % int(j->canBeSeen);
  if (j->obj_flags.decay_time < 0) {
    str += "It will not decay.\n\r";
  } else {
    str += format("It will decay in approximately %d MUD hours.\n\r") % j->obj_flags.decay_time;
  }
  str += format("Current structure:    %-5d  Weight in pounds:       %-10.2f  \n\r") % j->getStructPoints() % j->getWeight();
  str += format("Maximum structure:    %-5d  Volume in cubic inches: %d\n\r") % j->getMaxStructPoints() % j->getVolume();
  str += format("Base value in talens: %d\n\r") % j->obj_flags.cost;
  str += j->statObjInfo();
  if (j->spec) {
    str += format("It possesses the special trait known as %s.\n\r\n\r") % objSpecials[GET_OBJ_SPE_INDEX(j->spec)].name;
  } else {
    str += "It has not been imbued with with any special traits.\n\r\n\r";
  }
  
  if(!j->stuff.empty()){
    str += sitem.cap() + " contains:\n\r";
    for(StuffIter it=j->stuff.begin();it!=j->stuff.end() && (t=*it);++it) {
      str += t->shortDescr;
      str += "\n\r";
    }
    str += "\n\r";
  }
  
  sstring buf = "";
  for (int i = 0; i < MAX_OBJ_AFFECT; i++) {
    if (j->affected[i].location == APPLY_SPELL) {
      if (discArray[j->affected[i].modifier]) {
        buf += format("    Affect:  %s: %s by %ld\n\r") % apply_types[j->affected[i].location].name % discArray[j->affected[i].modifier]->name % j->affected[i].modifier2;
      } else {
        vlogf(LOG_BUG, format("BOGUS AFFECT (%ld) on %s") %
          j->affected[i].modifier % j->getName());
      }
    } else if (j->affected[i].location == APPLY_DISCIPLINE) {
     if (discNames[j->affected[i].modifier].disc_num) {
        buf += format("    Affect:  %s: %s by %ld\n\r") % apply_types[j->affected[i].location].name % discNames[j->affected[i].modifier].name % j->affected[i].modifier2;
      } else {
        vlogf(LOG_BUG, format("BOGUS AFFECT (%ld) on %s") %
          j->affected[i].modifier % j->getName());
      }
    } else if (j->affected[i].location == APPLY_IMMUNITY) {
      buf += format("    Affect:  %s: %s by %ld\n\r") % apply_types[j->affected[i].location].name % immunity_names[j->affected[i].modifier] % j->affected[i].modifier2;
    } else if (j->affected[i].location != APPLY_NONE) {
      if (!strcmp(apply_types[j->affected[i].location].name, "Magic Affect")) {
        for (unsigned long nr = 0; ; ++nr) {
          // loop over all item perma-affect flags
          if (*affected_bits[nr] == '\n')
            break;
          if (1<<nr & j->affected[i].modifier) {
            // item has affect
            if (*affected_bits[nr]) {
              buf += format("    Affect:  Magic Affect of %s\n\r") % affected_bits[nr];
            } else {
              buf += format("    Affect:  Magic Affect of %d\n\r") % int(1<<nr);
            }
          }
        }
      } else {
        buf += format("    Affect:  %s by %ld\n\r") % apply_types[j->affected[i].location].name % j->affected[i].modifier;
      }
    }
  }
  if (buf.empty()) {
    str += "It has no other affects.\n\r";
  } else {
    str += "It can affect you with:\n\r";
    str += buf;
  }
  
  str += "\n\r";       
  str += "The cloud of smoke is quickly dispersed and the air is clear.\n\r";
  desc->page_string(str);
  return;
}

void TBeing::statBeing(TBeing *k)
{
  sstring str = "";
  sstring buf2, buf3;
  TGuild *f = NULL;
  const TMonster *km = dynamic_cast<const TMonster *>(k);
  resp *respy;
  followData *fol;
  charList *list;
  struct time_info_data playing_time;
  int objused;
  int i;

  if (!limitPowerCheck(CMD_MEDIT, k->number)) {
    sendTo("You are not allowed to stat this being, sorry.\n\r");
    return;
  }

  switch (k->player.sex) {
    case SEX_NEUTER:
      str = format("%sNEUTRAL-SEX%s ") % cyan() % norm();
      break;
    case SEX_MALE:
      str = format("%sMALE%s ") % cyan() % norm();
      break;
    case SEX_FEMALE:
      str = format("%sFEMALE%s ") % cyan() % norm();
      break;
  }

  bool is_player=dynamic_cast<const TPerson *>(k);

  if (km) {
    str += format("%s - Name : %s [M-Num: %d]\n\r     In-Room[%d] Old-Room[%d] Birth-Room[%d] V-Number[%d]\n\r") %
      (is_player ? "PC" : "NPC") %
      k->name % k->number % k->in_room % km->oldRoom % km->brtRoom %
      (k->number >= 0 ? mob_index[km->getMobIndex()].virt : -1);
  } else {
    str += format("%s - Name : %s [%s: %d] Room[%d]\n\r") %
      (is_player ? "PC" : "NPC") %
      k->name % (is_player ? "PID  " : "M-Num") %
      (is_player ? k->getPlayerID() : k->number) % k->in_room;
  }

  str += "-----------------------------------------------------------------------------\n\r";
  if (km) {
    str += format("%sShort description:%s %s\n\r") %
      cyan() % norm() %
      (km->shortDescr ? km->shortDescr : "NONE");
    str += format("%sLong description:%s\n\r%s") %
      cyan() % norm() %
      (km->player.longDescr ? km->player.longDescr : "NONE");
  } else {
    Descriptor *d = k->desc;

    if (d && k->isPc() && k->GetMaxLevel() > MAX_MORT) {
      str += format("%sIMM Office  :%s %d\n\r") % cyan() % norm() % d->office;

      if (d->blockastart) {
        str += format("%sIMM Block A :%s %d - %d\n\r") % cyan() % norm() % d->blockastart % d->blockaend;
      }

      if (d->blockbstart) {
        str += format("%sIMM Block B :%s %d - %d\n\r") % cyan() % norm() % d->blockbstart % d->blockbend;
      }
    }
  }

  buf2 = "";
  for (classIndT ijc = MIN_CLASS_IND; ijc < MAX_CLASSES; ijc++) {
    if (k->hasClass(1<<ijc)) {
      buf2 += classInfo[ijc].name.cap();
      buf2 += " ";
    }
  }

  str += format("%sClass       :%s %-28s\n\r") % cyan() % norm() % buf2;

  str += format("%sLevel       :%s [") % cyan() % norm();
  for (classIndT i = MIN_CLASS_IND; i < MAX_CLASSES; i++) {
    str += format("%c%c:%d ") %
      classInfo[i].name.cap()[0] % classInfo[i].name.cap()[1] %
      int(k->getLevel(i));
  }
  str += "]\n\r";

  str += format("%sRace        :%s %-10s") %
    cyan() % norm() % k->getMyRace()->getSingularName();

  str += format("%sHome:%s %-17s") % 
    cyan() % norm() % home_terrains[k->player.hometerrain];

  if (k->desc && k->desc->account) {
    if (IS_SET(k->desc->account->flags, TAccount::IMMORTAL) &&
        !hasWizPower(POWER_VIEW_IMM_ACCOUNTS)) {
      str += "Account : *** Information Concealed ***\n\r";
    } else {
      str += format("%sAccount: %s%s\n\r") % purple() %
        k->desc->account->name % norm();
    }
  } else {
    str += "\n\r";
  }

  if (k->isPc()) {
    sstring birth_buf, logon_buf;

    birth_buf = asctime(localtime(&(k->player.time->birth)));
    // Chop off trailing \n from the output of localtime
    birth_buf = birth_buf.substr(0, birth_buf.length() - 1);

    logon_buf = asctime(localtime(&(k->player.time->logon)));
    // Chop off trailing \n from the output of localtime
    logon_buf = logon_buf.substr(0, logon_buf.length() - 1);

    GameTime::realTimePassed((time(0) - k->player.time->logon) +
			     k->player.time->played, 0, &playing_time);
    str += format("%sBirth       : %s%s     %sLogon:%s %s\n\r") %
      cyan() % norm() % birth_buf % cyan() % norm() % logon_buf;
    str += format("%sPlaying time:%s %d days, %d hours.      %sBase age:%s %d    %sAge Mod:%s %d\n\r") %
      cyan() % norm() % int(playing_time.day) % int(playing_time.hours) %
      cyan() % norm() % int(k->getBaseAge()) %
      cyan() % norm() % int(k->age_mod);
    if (!k->desc) {
      str += format("%sWARNING%s, player is offline, age will not be accurate.\n\r") % red() % norm();
    }

    str += format("%sPlayer age  :%s %d years, %d months, %d days, %d hours\n\r\n\r") %
      cyan() % norm() % int(k->age()->year) % int(k->age()->month) %
      int(k->age()->day) % int(k->age()->hours);
  }

  buf3 = format("[%5.2f]") % k->getExp();
  buf2 = format("[%5.2f]") % k->getMaxExp();
  str += format("%sDef Rnd:%s [%5d]   %sExp      :%s %-16s %sMax Exp :%s %-13s\n\r") %
    cyan() % norm() % k->defendRound(NULL) %
    cyan() % norm() % buf3 %
    cyan() % norm() % buf2;

  buf2 = format("[%5d]") % k->getMoney();
  buf3 = format("[%5d]") % k->getBank();
  str += format("%sVision :%s [%5d]   %sTalens   :%s %-16s %sBank Bal:%s %-13s\n\r") %
    cyan() % norm() % k->visionBonus %
    cyan() % norm() % buf2 %
    cyan() % norm() % buf3;

  buf2 = format("[%5d]") % k->getHitroll();
  buf3 = format("[%5d]") % k->getDamroll();
  str += format("%sAtt Rnd:%s [%5d]   %sHitroll  :%s %-16s %sDamroll :%s %-13s\n\r") %
    cyan() % norm() % k->attackRound(NULL) %
    cyan() % norm() % buf2 %
    cyan() % norm() % buf3;

  str += format("%sPiety  :%s [%5.1f]   %sLifeForce:%s [%5d]\n\r") %
    cyan() % norm() % k->getPiety() %
    cyan() % norm() % k->getLifeforce();

  buf2 = format("[%5d]") % k->getHit();
  buf3 = format("[%5d]") % k->getMove();
  str += format("%sMana   :%s [%5d]   %sHP       :%s %-16s %sMove    :%s %-13s\n\r") %
    cyan() % norm() % k->getMana() %
    cyan() % norm() % buf2 %
    cyan() % norm() % buf3;

  buf2 = format("[%5d]") % k->hitLimit();
  buf3 = format("[%5d]") % k->moveLimit();
  str += format("%sMaxMana:%s [%5d]   %sMax HP   :%s %-16s %sMax Move:%s %-13s\n\r") %
    cyan() % norm() % k->manaLimit() %
    cyan() % norm() % buf2 %
    cyan() % norm() % buf3;

  if(dynamic_cast<TPerson *>(k)){
    buf2 = format("[%5d]") % ageHpMod(dynamic_cast<TPerson *>(k));
    buf3 = format("[%f]") % k->getConHpModifier();
    str += format("%sEq HP  :%s [%5d]   %sAge HP   :%s %-16s %sConHpMod:%s %-13s\n\r") %
      cyan() % norm() % eqHpBonus(dynamic_cast<TPerson *>(k)) %
      cyan() % norm() % buf2 %
      cyan() % norm() % buf3;
  }

  buf2 = format("[%5d]") % k->visibility();
  buf3 = format("[%5.1f]") % k->getWeight();
  str += format("%sHeight :%s [%5d]   %sWt. (lbs):%s %-14s %sVisibility:%s %-13s\n\r") %
    cyan() % norm() % k->getHeight() %
    cyan() % norm() % buf3 %
    cyan() % norm() % buf2;

  buf2 = format("[%5d]") % int(k->specials.conditions[FULL]);
  buf3 = format("[%5d]") % int(k->specials.conditions[DRUNK]);
  str += format("%sThirst :%s [%5d]   %sHunger   :%s %-16s %sDrunk   :%s %-13s\n\r") %
    cyan() % norm() % int(k->specials.conditions[THIRST]) %
    cyan() % norm() % buf2 %
    cyan() % norm() % buf3;

  str += format("%sPee    :%s [%5d]   %sPoop     :%s [%5d]\n\r") %
    cyan() % norm() % int(k->specials.conditions[PEE]) %
    cyan() % norm() % int(k->specials.conditions[POOP]);

  buf2 = format("[%5d]") % k->getArmor();
  buf3 = format("[%5d]") % noise(k);
  str += format("%sLight  :%s [%5d]   %sNoise    :%s %-16s %sArmor   :%s %-13s\n\r") %
    cyan() % norm() % k->getLight() %
    cyan() % norm() % buf3 %
    cyan() % norm() % buf2;

  buf2 = format("[%5d]") % k->eyeSight(k->roomp);
  buf3 = format("[%5d]") % k->getSpellHitroll();
  str += format("%sProt.  :%s [%5d]   %sEyesight :%s %-11s %sSpell Hitroll:%s %-13s\n\r") %
    cyan() % norm() % k->getProtection() %
    cyan() % norm() % buf2 %
    cyan() % norm() % buf3;

  if (km && !(polyed == POLY_TYPE_DISGUISE)) {
    str += format("%sNumber of attacks :%s %.1f") %
      cyan() % norm() % km->getMult();
    str += format("        %sNPC Damage:%s %.1f+%d%%.\n\r") %
      cyan() % norm() % km->getDamLevel() % km->getDamPrecision();
    double bd = km->baseDamage();
    int chg = (int) (bd * km->getDamPrecision() / 100);
    str += format("%sNPC Damage range  :%s %d-%d.\n\r") % cyan() % norm() %
      max(1, (int) bd-chg) % max(1, (int) bd+chg);
  } else {
    if (k->hasClass(CLASS_MONK)) {
      str += format("%sNumber of attacks:%s %.2f\n\r") %
        cyan() % norm() % k->getMult();
    }

    float fx, fy;
    k->blowCount(false, fx, fy);
    str += format("%sPrim attacks:%s %.2f, %sOff attacks:%s %.2f\n\r") %
      cyan() % norm() % fx %
      cyan() % norm() % fy;

    int dam=0;
    int prim_min=9999, prim_max=0;
    int sec_min=9999, sec_max=0;
    for(i=0;i<100;++i){
      dam=k->getWeaponDam(this, k->heldInPrimHand(), HAND_PRIMARY);
      if(dam<prim_min)
	prim_min=dam;
      if(dam>prim_max)
	prim_max=dam;

      dam=k->getWeaponDam(this, k->heldInSecHand(), HAND_SECONDARY);
      if(dam<sec_min)
	sec_min=dam;
      if(dam>sec_max)
	sec_max=dam;
    }

    str += format("%sPrim damage:%s %i-%i, %sOff damage:%s %i-%i\n\r") %
      cyan() % norm() % prim_min % prim_max %
      cyan() % norm() % sec_min % sec_max;

    str += format("%sApproximate damage per round:%s %i-%i\n\r") %
      cyan() % norm() %
      (int)((fx*(float)prim_min)+((fy*(float)sec_min))) %
      (int)((fx*(float)prim_max)+((fy*(float)sec_max)));
  }
  if (toggleInfo[TOG_TESTCODE5]->toggle && k->newguild()) {
    if(k->isPc()) {
      str += format("%sFaction:%s %s%s,   %sRank :%s %s%s\n\r") %
        cyan() % norm() % k->newguild()->getName() % norm() %
        cyan() % norm() % k->rank() % norm();
    }
  } else {
    str += format("%sFaction:%s %s,   %sFaction Percent:%s %.4f\n\r") %
      cyan() % norm() % FactionInfo[k->getFaction()].faction_name %
      cyan() % norm() % k->getPerc();
#if FACTIONS_IN_USE
    str += format("%sPerc_0:%s %.4f   %sPerc_1:%s %.4f   %sPerc_2:%s %.4f   %sPerc_3:%s %.4f\n\r") %
      cyan() % norm() % k->getPercX(FACT_NONE) %
      cyan() % norm() % k->getPercX(FACT_BROTHERHOOD) %
      cyan() % norm() % k->getPercX(FACT_CULT) %
      cyan() % norm() % k->getPercX(FACT_SNAKE);
#endif
  }
//  str += format("%sFaction :%s %s\n\r") %
//    cyan() % norm() % FactionInfo[k->getFaction()].faction_name;

  str += "Stats    :";
  str += k->chosenStats.printStatHeader();

  statTypeT ik;

  str += "Race     :";
  for(ik = MIN_STAT; ik<MAX_STATS_USED; ik++) {
    str += format(" %3d ") % k->getStat(STAT_RACE, ik);
  }
  str += "\n\r";

  str += "Chosen   :";
  for(ik = MIN_STAT; ik<MAX_STATS_USED; ik++) {
    str += format(" %3d ") % k->getStat(STAT_CHOSEN, ik);
  }
  str += "\n\r";

  str += "Age      :";
  for(ik = MIN_STAT; ik<MAX_STATS_USED; ik++) {
    str += format(" %3d ") % k->getStat(STAT_AGE, ik);
  }
  str += "\n\r";

  str += "Territory:";
  for(ik = MIN_STAT; ik<MAX_STATS_USED; ik++) {
    str += format(" %3d ") % k->getStat(STAT_TERRITORY, ik);
  }
  str += "\n\r";

  str += "Natural  :";
  for(ik=MIN_STAT; ik<MAX_STATS_USED; ik++) {
    str += format(" %3d ") % k->getStat(STAT_NATURAL, ik);
  }
  str += "\n\r";
    
  str += "Current  :";
  for(ik = MIN_STAT; ik<MAX_STATS_USED; ik++) {
    str += format(" %3d ") % k->getStat(STAT_CURRENT, ik);
  }
  str += "\n\r";

  // only show captive info when needed
  if (k->getCaptiveOf() || k->getCaptive()) {
    str += format("%sCaptive Of:%s %s         %sCaptives :%s ") %
       cyan() % norm() %
       (k->getCaptiveOf() ? k->getCaptiveOf()->getName() : "NO ONE") %
       cyan() % norm();
    if (!k->getCaptive())
      str += "NONE\n\r";
    else {
      TBeing *x1;
      for (x1 = k->getCaptive(); x1; x1 = x1->getNextCaptive()) {
        str += x1->getName();
        str += " ";
      }
      str += "\n\r";
    }
  }
  str += format("Master is '%s'") %
          ((k->master) ? k->master->getName() : "NOBODY");
  str += "           Followers are:";
  for (fol = k->followers; fol; fol = fol->next) {
    str += fol->follower->getName();
  }
  str += "\n\r";

  if (km) {
    buf2 = sprinttype(km->getPosition(), position_types);
    buf3 = sprinttype((km->default_pos), position_types);
    str += format("%sPosition:%s %s  %sFighting:%s %s  %sDefault Position:%s %s\n\r") %
      cyan() % norm() % buf2 %
      cyan() % norm() % (km->fight() ? km->fight()->getName() : "Nobody") %
      cyan() % norm() % buf3;

    str += format("%sNPC flags:%s ") % cyan() % norm();
    if (km->specials.act) {
      str += sprintbit(km->specials.act, action_bits);
      str += "\n\r";
    } else {
      str += "None\n\r";
    }
  } else {
    buf2 = sprinttype(k->getPosition(), position_types);
    str += format("%sPosition:%s %s  %sFighting:%s %s\n\r") %
          cyan() % norm() % buf2 %
          cyan() % norm() % (k->fight() ? k->fight()->getName() : "Nobody");
  } 
  if (k->desc) {
    str += format("\n\r%sFlags (Specials Act):%s ") % cyan() % norm();
    str += sprintbit(k->desc->plr_act, player_bits);
    str += "\n\r";
  }

  str += format("%sCarried weight:%s %.1f   %sCarried volume:%s %d\n\r") %
          cyan() % norm() % k->getCarriedWeight() %
          cyan() % norm() % k->getCarriedVolume();

  immuneTypeT ij;
  for (ij = MIN_IMMUNE;ij < MAX_IMMUNES; ij++) {
    if (k->getImmunity(ij) == 0 || !*immunity_names[ij])
      continue;
    if (k->getImmunity(ij) > 0)
      buf2 = format("%d%% resistant to %s.\n\r") %
        k->getImmunity(ij) % immunity_names[ij];
    if (k->getImmunity(ij) < 0)
      buf2 = format("%d%% susceptible to %s.\n\r") %
        -k->getImmunity(ij) % immunity_names[ij];
    str += buf2;
  }

  if (!k->isPc()) {
    const TMonster *tmons = dynamic_cast<const TMonster *>(k);
    str += format("%sAction pointer:%s %s") % 
      cyan() % norm() %
      ((tmons->act_ptr ? "YES" : "no") );
    str += format("    %sSpecial Procedure:%s %s\n\r") %
      cyan() % norm() %
      ((tmons->spec) ? mob_specials[GET_MOB_SPE_INDEX(tmons->spec)].name : "none");
    str += format("%sAnger:%s %d/%d     %sMalice:%s %d/%d     %sSuspicion:%s %d/%d   %sGreed:%s %d/%d\n\r") %
      cyan() % norm() %
      tmons->anger() % tmons->defanger() %
      cyan() % norm() %
      tmons->malice() % tmons->defmalice() %
      cyan() % norm() %
      tmons->susp() % tmons->defsusp() %
      cyan() % norm() %
      tmons->greed() % tmons->defgreed();
    str += format("%sHates:%s " ) % cyan() % norm();
    if (IS_SET(tmons->hatefield, HATE_CHAR)) {
      if (tmons->hates.clist) {
        for (list = tmons->hates.clist; list; list = list->next) {
          if (list->name) {
            str += list->name;
            str += " ";
          }
        }
      }
    }
    if (IS_SET(tmons->hatefield, HATE_RACE)) {
      if (tmons->hates.race != -1) {
        str += Races[tmons->hates.race]->getSingularName();
        str += "(Race) ";
      }
    }
    if (IS_SET(tmons->hatefield, HATE_SEX)) {
      switch (tmons->hates.sex) {
        case SEX_NEUTER:
          str += "SEX_NEUTER ";
          break;
        case SEX_MALE:
          str += "SEX_MALE ";
          break;
        case SEX_FEMALE:
          str += "SEX_FEMALE ";
          break;
      }
    }
    str += "    ";

    str += format("%sFears:%s " ) % cyan() % norm();
    if (IS_SET(tmons->fearfield, FEAR_CHAR)) {
      if (tmons->fears.clist) {
        for (list = tmons->fears.clist; list; list = list->next) {
          if (list->name) {
            str += list->name;
            str += " ";
          }
        }
      }
    }
    if (IS_SET(tmons->fearfield, FEAR_RACE)) {
      if (tmons->fears.race != -1) {
        str += Races[tmons->fears.race]->getSingularName();
        str += "(Race) ";
      }
    }
    if (IS_SET(tmons->fearfield, FEAR_SEX)) {
      switch (tmons->fears.sex) {
        case SEX_NEUTER:
          str += "SEX_NEUTER ";
          break;
        case SEX_MALE:
          str += "SEX_MALE ";
          break;
        case SEX_FEMALE:
          str += "SEX_FEMALE ";
          break;
      }
    }
    if (IS_SET(tmons->fearfield, FEAR_CLASS)) {
       str += format("CLASS=%d ") % tmons->fears.Class;
    }
    if (IS_SET(tmons->fearfield, FEAR_VNUM)) {
       str += format("VNUM=%d ") % tmons->fears.vnum;
    }
    if (IS_SET(tmons->specials.act, ACT_HUNTING)) {
      str += format("\n\r%sHunting:%s %s  %spersist:%s %d  %sorigin:%s %d  %shunt distance:%s %d") %
        cyan() % norm() %
        (tmons->specials.hunting ? tmons->specials.hunting->getName() : "Unknown") %
        cyan() % norm() %
        tmons->persist %
        cyan() % norm() %
        tmons->oldRoom %
        cyan() % norm() %
        tmons->hunt_dist;
    } else if (tmons->specials.hunting) {
      str += format("\n\r%sTracking:%s %s  %spersist:%s %d  %sorigin:%s %d  %srange:%s %d") %
        cyan() % norm() %
        tmons->specials.hunting->getName() %
        cyan() % norm() %
        tmons->persist %
        cyan() % norm() %
        tmons->oldRoom %
        cyan() % norm() %
        tmons->hunt_dist;
    }
    str += format("\n\r%sAI Target:%s %s  %sRandom:%s %s") %
      cyan() % norm() %
      (tmons->targ() ? tmons->targ()->getName() : "-") %
      cyan() % norm() %
      (tmons->opinion.random ? tmons->opinion.random->getName() : "-");

  /*if (tmons->loadCom.size() > 0) {
      str += "\n\rThis mob has the following zonefile load commands:";
      for(unsigned int iLoad = 0; iLoad < tmons->loadCom.size(); iLoad++) {
        const resetCom *p = &tmons->loadCom[iLoad];
        str += format("\n\r%c %i %i %i %i %i %c") % p->command % p->if_flag % p->arg1 % p->arg2 % p->arg3 % p->arg4 % p->character;
      }
    }*/

    str += "\n\r";
  } else {
    // PCs only
    if (k->specials.hunting) {
      str += format("%sHunting:%s %s\n\r") %
        cyan() % norm() %
        k->specials.hunting->getName();
    }
    const TPerson *tper = dynamic_cast<const TPerson *>(k);
    if (tper) {
      str += format("%sTitle:%s\n\r%s%s\n\r") %
        cyan() % norm() % tper->title % norm();
    }
  }

  str += format("%sAffected by:%s ") % cyan() % norm();
  str += sprintbit_64(k->specials.affectedBy, affected_bits);
  str += "\n\r\n\r";

  str += format("%sBody part          Hth Max Flgs  StuckIn%s\n\r") %
    cyan() % norm();
  str += format("%s----------------------------------------%s\n\r") %
    cyan() % norm();
  wearSlotT il;
  for (il = MIN_WEAR; il < MAX_WEAR; il++) {
    if (il == HOLD_RIGHT || il == HOLD_LEFT)
      continue;
    if (k->slotChance(il)) {
      buf2 = format("[%s]") % k->describeBodySlot(il);
      str += format("%-18s %-3d %-3d %-5d %s\n\r") %
        buf2 % k->getCurLimbHealth(il) %
        k->getMaxLimbHealth(il) %
        k->getLimbFlags(il) %
        (k->getStuckIn(il) ? k->getStuckIn(il)->getName() : "None");
    }
  }

  if (km) {
    if (km->resps && km->resps->respList) {
      str += format("%sResponse(s):\n\r------------%s\n\r") % cyan() % norm();
      for (respy = km->resps->respList; respy; respy = respy->next) {
        if (respy->cmd < MAX_CMD_LIST) {
          str += format("%s %s\n\r") % commandArray[respy->cmd]->name %
            respy->args;
        } else if (respy->cmd == CMD_RESP_ROOM_ENTER) {
          str += "roomenter\n\r";
        } else if (respy->cmd == CMD_RESP_PACKAGE) {
          str += format("package %s\n\r") % respy->args;
        } else if (respy->cmd == CMD_RESP_KILLED) {
          str += format("killed %s\n\r") % respy->args;
        } else if (respy->cmd == CMD_GENERIC_CREATED) {
          str += format("created %s\n\r") % respy->args;
        } else if (respy->cmd == CMD_RESP_TRIGGER) {
          str += format("trigger %s\n\r") % respy->args;
        } else if (respy->cmd == CMD_RESP_STARTFIGHT) {
          str += format("startfight %s\n\r") % respy->args;
        } else if (respy->cmd == CMD_RESP_ENDMODE) {
          str += format("endmode %s\n\r") % respy->args;
        } else {
          str += format("%d %s\n\r") % respy->cmd % respy->args;
        }
      }
      str += format("%s------------%s\n\r") % cyan() % norm();

      if (km->resps->respMemory) {
        str += format("%sResponse Memory:\n\r----------------\n\r%s") %
          cyan() % norm();

        for (RespMemory *rMem = km->resps->respMemory; rMem; rMem = rMem->next)
          if (rMem->cmd < MAX_CMD_LIST) {
            str += format("%s %s %s\n\r") %
              (rMem->name ? rMem->name : "Unknown") %
              commandArray[rMem->cmd]->name %
              (rMem->args ? rMem->args : "");
	  } else if (rMem->cmd == CMD_RESP_ROOM_ENTER) {
            str += format("%s %s %s\n\r") %
              (rMem->name ? rMem->name : "Unknown") %
              "roomenter" %
              (rMem->args ? rMem->args : "");
          } else {
            str += format("%s %d %s\n\r") %
              (rMem->name ? rMem->name : "Unknown") %
              rMem->cmd %
              (rMem->args ? rMem->args : "");
          }

        str += format("----------------\n\r%s") % cyan() % norm();
      }
    } else
      str += format("%sResponse(s):%s None.\n\r") % cyan() % norm();
  }

  str += format("\n\r%sAffecting Spells:\n\r-----------------%s\n\r") %
    cyan() % norm();
  affectedData *aff, *af2;
  for (aff = k->affected; aff; aff = af2) {
    // technically, shouldn't need to save next, but apparently
    // some operations below "might" cause aff to be deleted
    // not sure which ones though - Bat 4/28/98
    af2 = aff->next;

    switch (aff->type) {
      case SKILL_TRACK:
      case SKILL_SEEKWATER:
      case SPELL_GUST:
      case SPELL_DUST_STORM:
      case SPELL_TORNADO:
      case SKILL_QUIV_PALM:
      case SKILL_SHOULDER_THROW:
      case SPELL_CALL_LIGHTNING_DEIKHAN:
      case SPELL_CALL_LIGHTNING:
      case SPELL_LIGHTNING_BREATH:
      case SPELL_GUSHER:
      case SPELL_AQUALUNG:
      case SPELL_AQUATIC_BLAST:
      case SPELL_CARDIAC_STRESS:
      case SPELL_ICY_GRIP:
      case SPELL_ARCTIC_BLAST:
      case SPELL_ICE_STORM:
      case SPELL_FROST_BREATH:
      case SPELL_WATERY_GRAVE:
      case SPELL_TSUNAMI:
      case SPELL_CHLORINE_BREATH:
      case SPELL_DUST_BREATH:
      case SPELL_POISON_DEIKHAN:
      case SPELL_POISON:
      case SPELL_ACID_BREATH:
      case SPELL_ACID_BLAST:
      case SKILL_BODYSLAM:
      case SKILL_SPIN:
      case SKILL_CHARGE:
      case SKILL_SMITE:
      case SPELL_METEOR_SWARM:
      case SPELL_EARTHQUAKE_DEIKHAN:
      case SPELL_EARTHQUAKE:
      case SPELL_PILLAR_SALT:
      case SPELL_FIREBALL:
      case SPELL_HANDS_OF_FLAME:
      case SPELL_INFERNO:
      case SPELL_HELLFIRE:
      case SPELL_RAIN_BRIMSTONE_DEIKHAN:
      case SPELL_RAIN_BRIMSTONE:
      case SPELL_FLAMESTRIKE:
      case SPELL_FIRE_BREATH:
      case SPELL_SPONTANEOUS_COMBUST:
      case SPELL_FLAMING_SWORD:
      case SPELL_FLARE:
      case SPELL_MYSTIC_DARTS:
      case SPELL_STUNNING_ARROW:
      case SPELL_COLOR_SPRAY:
      case SPELL_SAND_BLAST:
      case SPELL_PEBBLE_SPRAY:
      case SPELL_LAVA_STREAM:
      case SPELL_DEATH_MIST:
      case SPELL_FLATULENCE:
      case SPELL_SLING_SHOT:
      case SPELL_GRANITE_FISTS:
      case SPELL_STICKS_TO_SNAKES:
      case SPELL_DISTORT: // shaman
      case SPELL_DEATHWAVE: // shaman
      case SPELL_SOUL_TWIST: // shaman
      case SPELL_SQUISH: // shaman
      case SPELL_ENERGY_DRAIN:
      case SPELL_LICH_TOUCH: // shaman
      case SPELL_SYNOSTODWEOMER:
      case SPELL_HARM_DEIKHAN:
      case SPELL_HARM:
      case SPELL_HARM_LIGHT_DEIKHAN:
      case SPELL_HARM_SERIOUS_DEIKHAN:
      case SPELL_HARM_CRITICAL_DEIKHAN:
      case SPELL_HARM_LIGHT:
      case SPELL_HARM_SERIOUS:
      case SPELL_HARM_CRITICAL:
      case SPELL_WITHER_LIMB:
      case SPELL_BLEED:
      case SKILL_KICK_DEIKHAN:
      case SKILL_KICK_THIEF:
      case SKILL_KICK_MONK:
      case SKILL_KICK:
      case SKILL_SPRINGLEAP:
      case SKILL_DEATHSTROKE:
      case SKILL_BASH_DEIKHAN:
      case SKILL_BASH:
      case SPELL_BONE_BREAKER:
      case SPELL_PARALYZE:
      case SPELL_PARALYZE_LIMB:
      case SPELL_INFECT_DEIKHAN:
      case SPELL_INFECT:
      case SKILL_CHOP:
      case SPELL_DISEASE:
      case SPELL_SUFFOCATE:
      case SKILL_GARROTTE:
      case SKILL_STABBING:
      case SKILL_BACKSTAB:
      case SKILL_THROATSLIT:
      case SKILL_HEADBUTT:
      case SKILL_STOMP:
      case SKILL_BRAWL_AVOIDANCE:
      case SPELL_BLAST_OF_FURY:
      case SKILL_CHI:
      case SKILL_TRIP:
      case SPELL_FUMBLE:
      case SPELL_BLINDNESS:
      case SPELL_GARMULS_TAIL:
      case SPELL_SORCERERS_GLOBE:
      case SPELL_FAERIE_FIRE:
      case SPELL_STUPIDITY:
      case SPELL_ILLUMINATE:
      case SPELL_DETECT_MAGIC:
      case SPELL_MATERIALIZE:
      case SPELL_BLOOD_BOIL:
      case SPELL_DJALLA:
      case SPELL_LEGBA:
      case SPELL_PROTECTION_FROM_EARTH:
      case SPELL_PROTECTION_FROM_AIR:
      case SPELL_PROTECTION_FROM_FIRE:
      case SPELL_PROTECTION_FROM_WATER:
      case SPELL_PROTECTION_FROM_ELEMENTS:
      case SPELL_INFRAVISION:
      case SPELL_IDENTIFY:
      case SPELL_POWERSTONE:
      case SPELL_FAERIE_FOG:
      case SPELL_TELEPORT:
      case SPELL_KNOT:
      case SPELL_SENSE_LIFE:
      case SPELL_SENSE_LIFE_SHAMAN: // shaman
      case SPELL_CALM:
      case SPELL_ACCELERATE:
      case SPELL_CHEVAL: // shaman
      case SPELL_CELERITE:
      case SPELL_LEVITATE:
      case SPELL_FEATHERY_DESCENT:
      case SPELL_STEALTH:
      case SPELL_GILLS_OF_FLESH:
      case SPELL_TELEPATHY:
      case SPELL_ROMBLER: // shaman
      case SPELL_INTIMIDATE: //shaman
      case SPELL_CLEANSE: // shaman
      case SPELL_FEAR:
      case SPELL_SLUMBER:
      case SPELL_CONJURE_EARTH:
      case SPELL_CONJURE_AIR:
      case SPELL_CONJURE_FIRE:
      case SPELL_CONJURE_WATER:
      case SPELL_DISPEL_MAGIC:
      case SPELL_CHASE_SPIRIT: // shaman
      case SPELL_ENHANCE_WEAPON:
      case SPELL_GALVANIZE:
      case SPELL_DETECT_INVISIBLE:
      case SPELL_DETECT_SHADOW: // shaman
      case SPELL_DISPEL_INVISIBLE:
      case SPELL_FARLOOK:
      case SPELL_FALCON_WINGS:
      case SPELL_INVISIBILITY:
      case SPELL_ENSORCER:
      case SPELL_EYES_OF_FERTUMAN:
      case SPELL_COPY:
      case SPELL_HASTE:
      case SPELL_IMMOBILIZE:
      case SPELL_FLY:
      case SPELL_ANTIGRAVITY:
      case SPELL_DIVINATION:
      case SPELL_SHATTER:
      case SKILL_SCRIBE:
      case SPELL_SPONTANEOUS_GENERATION:
      case SPELL_STONE_SKIN:
      case SPELL_TRAIL_SEEK:
      case SPELL_FLAMING_FLESH:
      case SPELL_ATOMIZE:
      case SPELL_ANIMATE:
      case SPELL_BIND:
      case SPELL_ENLIVEN:
      case SPELL_TRUE_SIGHT:
      case SPELL_CLOUD_OF_CONCEALMENT:
      case SPELL_POLYMORPH:
      case SPELL_SILENCE:
      case SPELL_BREATH_OF_SARAHAGE:
      case SPELL_PLASMA_MIRROR:
      case SPELL_THORNFLESH:
      case SPELL_ETHER_GATE:
      case SPELL_HEAL_LIGHT:
      case SPELL_HEALING_GRASP:
      case SPELL_CREATE_FOOD:
      case SPELL_CREATE_WATER:
      case SPELL_ARMOR:
      case SPELL_BLESS:
      case SPELL_CLOT:
      case SPELL_HEAL_SERIOUS:
      case SPELL_STERILIZE:
      case SPELL_EXPEL:
      case SPELL_CURE_DISEASE:
      case SPELL_CURSE:
      case SPELL_REMOVE_CURSE:
      case SPELL_CURE_POISON:
      case SPELL_HEAL_CRITICAL:
      case SPELL_SALVE:
      case SPELL_REFRESH:
      case SPELL_NUMB:
      case SPELL_PLAGUE_LOCUSTS:
      case SPELL_CURE_BLINDNESS:
      case SPELL_SUMMON:
      case SPELL_HEAL:
      case SPELL_WORD_OF_RECALL:
      case SPELL_SANCTUARY:
      case SPELL_RELIVE:
      case SPELL_CURE_PARALYSIS:
      case SPELL_SECOND_WIND:
      case SPELL_HEROES_FEAST:
      case SPELL_ASTRAL_WALK:
      case SPELL_PORTAL:
      case SPELL_HEAL_FULL:
      case SPELL_HEAL_CRITICAL_SPRAY:
      case SPELL_HEAL_SPRAY:
      case SPELL_HEAL_FULL_SPRAY:
      case SPELL_RESTORE_LIMB:
      case SPELL_KNIT_BONE:
      case SKILL_RESCUE:
      case SKILL_BLACKSMITHING:
      case SKILL_REPAIR_MAGE:
      case SKILL_REPAIR_MONK:
      case SKILL_REPAIR_CLERIC:
      case SKILL_REPAIR_DEIKHAN:
      case SKILL_REPAIR_SHAMAN:
      case SKILL_REPAIR_THIEF:
      case SKILL_BLACKSMITHING_ADVANCED:
      case SKILL_MEND:
      case SKILL_DISARM:
      case SKILL_DUAL_WIELD:
      case SKILL_POWERMOVE:
      case SKILL_PARRY_WARRIOR:
      case SKILL_RIPOSTE:
      case SKILL_BERSERK:
      case SKILL_SWITCH_OPP:
      case SKILL_KNEESTRIKE:
      case SKILL_SHOVE:
      case SKILL_RETREAT:
      case SKILL_GRAPPLE:
      case SKILL_DOORBASH:
      case SKILL_TRANCE_OF_BLADES:
      case SKILL_WEAPON_RETENTION:
      case SKILL_CLOSE_QUARTERS_FIGHTING:
      case SKILL_HIKING:
      case SKILL_FORAGE:
      case SKILL_TRANSFORM_LIMB:
      case SKILL_BEAST_SOOTHER:
      case SPELL_ROOT_CONTROL:
      case SKILL_BEFRIEND_BEAST:
      case SKILL_TRANSFIX:
      case SKILL_SKIN:
      case SKILL_BUTCHER:
      case SPELL_LIVING_VINES:
      case SKILL_BEAST_SUMMON:
      case SKILL_BARKSKIN:
      case SPELL_ENTHRALL_SPECTRE:
      case SPELL_ENTHRALL_GHAST:
      case SPELL_ENTHRALL_GHOUL:
      case SPELL_ENTHRALL_DEMON:
      case SPELL_CREATE_WOOD_GOLEM:
      case SPELL_CREATE_ROCK_GOLEM:
      case SPELL_CREATE_IRON_GOLEM:
      case SPELL_CREATE_DIAMOND_GOLEM:
      case SPELL_STORMY_SKIES:
      case SPELL_TREE_WALK:
      case SKILL_BEAST_CHARM:
      case SPELL_SHAPESHIFT:
      case SPELL_CHRISM:
      case SPELL_CLARITY:
      case SKILL_CONCEALMENT:
      case SKILL_APPLY_HERBS:
      case SKILL_DIVINATION:
      case SKILL_ENCAMP:
      case SKILL_FISHLORE:
      case SPELL_HEAL_LIGHT_DEIKHAN:
      case SKILL_CHIVALRY:
      case SPELL_ARMOR_DEIKHAN:
      case SPELL_BLESS_DEIKHAN:
      case SPELL_EXPEL_DEIKHAN:
      case SPELL_CLOT_DEIKHAN:
      case SPELL_STERILIZE_DEIKHAN:
      case SPELL_REMOVE_CURSE_DEIKHAN:
      case SPELL_CURSE_DEIKHAN:
      case SKILL_RESCUE_DEIKHAN:
      case SKILL_TAUNT:
      case SPELL_CURE_DISEASE_DEIKHAN:
      case SPELL_CREATE_FOOD_DEIKHAN:
      case SPELL_HEAL_SERIOUS_DEIKHAN:
      case SPELL_CURE_POISON_DEIKHAN:
      case SKILL_DISARM_DEIKHAN:
      case SPELL_HEAL_CRITICAL_DEIKHAN:
      case SKILL_SWITCH_DEIKHAN:
      case SKILL_RETREAT_DEIKHAN:
      case SKILL_SHOVE_DEIKHAN:
      case SKILL_RIDE:
      case SKILL_ALCOHOLISM:
      case SKILL_FISHING:
      case SKILL_LOGGING:
      case SKILL_CALM_MOUNT:
      case SKILL_TRAIN_MOUNT:
      case SKILL_ADVANCED_RIDING:
      case SKILL_RIDE_DOMESTIC:
      case SKILL_RIDE_NONDOMESTIC:
      case SKILL_RIDE_WINGED:
      case SPELL_CREATE_WATER_DEIKHAN:
      case SKILL_RIDE_EXOTIC:
      case SPELL_HEROES_FEAST_DEIKHAN:
      case SPELL_REFRESH_DEIKHAN:
      case SPELL_SALVE_DEIKHAN:
      case SKILL_LAY_HANDS:
      case SPELL_NUMB_DEIKHAN:
      case SKILL_YOGINSA:
      case SKILL_CINTAI:
      case SKILL_OOMLAT:
      case SKILL_ADVANCED_KICKING:
      case SKILL_DISARM_MONK:
      case SKILL_GROUNDFIGHTING:
      case SKILL_DUFALI:
      case SKILL_RETREAT_MONK:
      case SKILL_SNOFALTE:
      case SKILL_COUNTER_MOVE:
      case SKILL_SWITCH_MONK:
      case SKILL_JIRIN:
      case SKILL_KUBO:
      case SKILL_CATFALL:
      case SKILL_CATLEAP:
      case SKILL_WOHLIN:
      case SKILL_VOPLAT:
      case SKILL_BLINDFIGHTING:
      case SKILL_CRIT_HIT:
      case SKILL_FEIGN_DEATH:
      case SKILL_BLUR:
      case SKILL_CHAIN_ATTACK:
      case SKILL_HURL:
      case SKILL_DEFENESTRATE:
      case SKILL_BONEBREAK:
      case SKILL_SWINDLE:
      case SKILL_SNEAK:
      case SKILL_RETREAT_THIEF:
      case SKILL_PICK_LOCK:
      case SKILL_SEARCH:
      case SKILL_SPY:
      case SKILL_SWITCH_THIEF:
      case SKILL_STEAL:
      case SKILL_DETECT_TRAP:
      case SKILL_SUBTERFUGE:
      case SKILL_DISARM_TRAP:
      case SKILL_CUDGEL:
      case SKILL_HIDE:
      case SKILL_POISON_WEAPON:
      case SKILL_DISGUISE:
      case SKILL_DODGE_THIEF:
      case SKILL_SET_TRAP_CONT:
      case SKILL_SET_TRAP_DOOR:
      case SKILL_SET_TRAP_MINE:
      case SKILL_SET_TRAP_GREN:
      case SKILL_SET_TRAP_ARROW:
      case SKILL_DUAL_WIELD_THIEF:
      case SKILL_DISARM_THIEF:
      case SKILL_COUNTER_STEAL:
      case SPELL_DANCING_BONES:
      case SPELL_CONTROL_UNDEAD:
      case SPELL_RESURRECTION:
      case SPELL_VOODOO:
      case SKILL_BREW:
      case SPELL_VAMPIRIC_TOUCH:
      case SPELL_LIFE_LEECH:
      case SKILL_TURN:
      case SKILL_SIGN:
      case SKILL_SWIM:
      case SKILL_CONS_UNDEAD:
      case SKILL_CONS_VEGGIE:
      case SKILL_CONS_DEMON:
      case SKILL_CONS_ANIMAL:
      case SKILL_CONS_REPTILE:
      case SKILL_CONS_PEOPLE:
      case SKILL_CONS_GIANT:
      case SKILL_CONS_OTHER:
      case SKILL_READ_MAGIC:
      case SKILL_BANDAGE:
      case SKILL_CLIMB:
      case SKILL_FAST_HEAL:
      case SKILL_EVALUATE:
      case SKILL_TACTICS:
      case SKILL_DISSECT:
      case SKILL_DEFENSE:
      case SKILL_ADVANCED_DEFENSE:
      case SKILL_OFFENSE:
      case SKILL_WHITTLE:
      case SKILL_WIZARDRY:
      case SKILL_RITUALISM:
      case SKILL_MEDITATE:
      case SKILL_DEVOTION:
      case SKILL_PENANCE:
      case SKILL_SLASH_PROF:
      case SKILL_PIERCE_PROF:
      case SKILL_BLUNT_PROF:
      case SKILL_BAREHAND_PROF:
      case SKILL_SLASH_SPEC:
      case SKILL_BLUNT_SPEC:
      case SKILL_PIERCE_SPEC:
      case SKILL_BAREHAND_SPEC:
      case SKILL_RANGED_SPEC:
      case SKILL_RANGED_PROF:
      case SKILL_FAST_LOAD:
      case SKILL_SHARPEN:
      case SKILL_DULL:
      case SKILL_ATTUNE:
      case SKILL_STAVECHARGE:
      case SKILL_SACRIFICE:
      case SPELL_SHIELD_OF_MISTS:
      case SPELL_SHADOW_WALK:
      case SPELL_RAZE:
      case SPELL_HYPNOSIS:
      case SKILL_PSITELEPATHY:
      case SKILL_TELE_SIGHT:
      case SKILL_TELE_VISION:
      case SKILL_MIND_FOCUS:
      case SKILL_PSI_BLAST:
      case SKILL_MIND_THRUST:
      case SKILL_PSYCHIC_CRUSH:
      case SKILL_KINETIC_WAVE:
      case SKILL_MIND_PRESERVATION:
      case SKILL_TELEKINESIS:
      case SKILL_PSIDRAIN:
      case SKILL_MANA:
      case SKILL_IRON_FIST:
      case SKILL_IRON_FLESH:
      case SKILL_IRON_SKIN:
      case SKILL_IRON_BONES:
      case SKILL_IRON_MUSCLES:
      case SKILL_IRON_LEGS:
      case SKILL_IRON_WILL:
      case SKILL_PLANT:
      case SPELL_EMBALM:
      case SKILL_GUTTER_CANT:
      case SKILL_GNOLL_JARGON:
      case SKILL_TROGLODYTE_PIDGIN:
      case SKILL_TROLLISH:
      case SKILL_BULLYWUGCROAK:
      case SKILL_AVIAN:
      case SKILL_FISHBURBLE:
#if 1
      case SPELL_EARTHMAW:
      case SPELL_CREEPING_DOOM:
      case SPELL_FERAL_WRATH:
      case SPELL_SKY_SPIRIT:
#endif
        if (!discArray[aff->type]) {
          vlogf(LOG_BUG, format("BOGUS AFFECT (%d) on %s") %
            aff->type % k->getName());
          k->affectRemove(aff);
          break;
        }

        str += format("Spell : '%s'\n\r") % discArray[aff->type]->name;
        if (aff->location == APPLY_IMMUNITY) {
          str += format("     Modifies %s to %s by %ld points\n\r") %
            apply_types[aff->location].name %
            immunity_names[aff->modifier] %
            aff->modifier2;
        } else if (aff->location == APPLY_SPELL) {
          str += format("     Modifies %s (%s) by %ld points\n\r") %
            apply_types[aff->location].name %
            (discArray[aff->modifier] ? discArray[aff->modifier]->name : "BOGUS") %
            aff->modifier2;
        } else if (aff->location == APPLY_DISCIPLINE) {
          str += format("     Modifies %s (%s) by %ld points\n\r" ) %
            apply_types[aff->location].name %
            (discNames[aff->modifier].disc_num ? discNames[aff->modifier].properName : "BOGUS") %
            aff->modifier2;
        } else {
          str += format("     Modifies %s by %ld points\n\r") %
            apply_types[aff->location].name % aff->modifier;
        }
        str += format("     Expires in %6d updates, Bits set: %s\n\r\n\r") %
          aff->duration % sprintbit_64(aff->bitvector, affected_bits);
        break;

      case AFFECT_DISEASE:
        str += format("Disease: '%s'\n\r") % DiseaseInfo[affToDisease(*aff)].name;
        str += format("     Expires in %d updates.  Status = %d.\n\r") %
          aff->duration % aff->level;
        break;

      case AFFECT_DUMMY:
        str += "Dummy Affect: \n\r";
        str += format("     Expires in %d updates.  Status = %d.\n\r") %
          aff->duration % aff->level;
        break;

      case AFFECT_WAS_INDOORS:
        str += "Was indoors (immune to frostbite): \n\r";
        str += format("     Expires in %d updates.  Status = %d.\n\r") %
          aff->duration % aff->level;
        break;

      case AFFECT_FREE_DEATHS:
        str += "Free Deaths: \n\r";
        str += format("     Remaining %ld.  Status = %d.\n\r") %
          aff->modifier % aff->level;
        break;

      case AFFECT_HORSEOWNED:
        str += "Horse-owned: \n\r";
        str += format("     Expires in %d updates.\n\r") %
          aff->duration;
        break;

      case AFFECT_PLAYERKILL:
        str += "Player-Killer: \n\r";
        str += format("     Expires in %d updates.\n\r") %
          aff->duration;
        break;

      case AFFECT_PLAYERLOOT:
        str += "Player-Looter: \n\r";
        str += format("     Expires in %d updates.\n\r") %
          aff->duration;
        break;

      case AFFECT_TEST_FIGHT_MOB:
        str += "Test Fight Mob: \n\r";
        str += format("     Remaining %ld.  Status = %d.\n\r") %
          aff->modifier % aff->level;
        break;

      case AFFECT_SKILL_ATTEMPT:
        str += "Skill Attempt: \n\r";
        str += format("     Expires in %d updates.  Skill = %d.\n\r") %
          aff->duration % (int) aff->bitvector; 
        break;

      case AFFECT_NEWBIE:
        str += "Got Newbie Equipment: \n\r";
        str += format("     Expires in %d updates.  Status = %d.\n\r") %
          aff->duration % aff->level;
        break;

      case AFFECT_DRUNK:
        str += "Drunken slumber: \n\r";
        str += format("     Expires in %d updates.  Status = %d.\n\r") %
          aff->duration % aff->level;
        break;

      case AFFECT_DRUG:
        str += format("%s: \n\r") % drugTypes[aff->modifier2].name;
        str += format("     Modifies %s by %ld points\n\r") %
          apply_types[aff->location].name % aff->modifier;
        str += format("     Expires in %d updates.  Status = %d.\n\r") %
          aff->duration % aff->level;
        str += format("renew %i\n\r") % aff->renew;
        break;

      case AFFECT_COMBAT:
        if (aff->modifier == COMBAT_SOLO_KILL) {
          str += format("Combat: '%s'\n\r") % (aff->be ? static_cast<TBeing *>(aff->be)->getName() : "No aff->be!");
          str += format("     Expires in %d updates.  Status = %d.\n\r") % aff->duration % aff->level;
        }else if (aff->modifier == COMBAT_RESTRICT_XP) {
          str += format("Restricted Experience: '%s'\n\r") % (aff->be ? static_cast<char *>((void*)aff->be) : "No aff->be!");
          str += format("     Expires in %d updates.\n\r") % aff->duration;
        }
        break;

      case AFFECT_PET:
        str += format("Pet of: '%s'\n\r") % (aff->be ? (char *) aff->be : "No aff->be!");
        str += format("     Expires in %d updates.  Status = %d.\n\r") %
          aff->duration % aff->level;
        break;

      case AFFECT_CHARM:
        str += format("Charm of: '%s'\n\r") % (aff->be ? (char *) aff->be : "No aff->be!");
        str += format("     Expires in %d updates.  Status = %d.\n\r") %
          aff->duration % aff->level;
        break;

      case AFFECT_THRALL:
        str += format("Thrall of: '%s'\n\r") % (aff->be ? (char *) aff->be : "No aff->be!");
        str += format("     Expires in %d updates.  Status = %d.\n\r") %
          aff->duration % aff->level;
        break;

      case AFFECT_ORPHAN_PET:
        str += format("Orphan pet of: '%s'\n\r") % (aff->be ? (char *) aff->be : "No aff->be!");
        str += format("     Expires in %d updates.  Status = %d.\n\r") %
          aff->duration % aff->level;
        break;

      case AFFECT_TRANSFORMED_ARMS:
      case AFFECT_TRANSFORMED_HANDS:
      case AFFECT_TRANSFORMED_LEGS:
      case AFFECT_TRANSFORMED_HEAD:
      case AFFECT_TRANSFORMED_NECK:
        str += "Spell : 'Transformed Limb'\n\r";
        str += format("     Modifies %s by %ld points\n\r") %
          apply_types[aff->location].name % aff->modifier;
        str += format("     Expires in %6d updates, Bits set: %s\n\r") %
          aff->duration % sprintbit_64(aff->bitvector, affected_bits);
        break;

      case AFFECT_GROWTH_POTION:
        str += "Spell : 'Growth'\n\r";
        str += format("     Modifies %s by %ld points\n\r") %
          apply_types[aff->location].name % aff->modifier;
        str += format("     Expires in %6d updates, Bits set: %s\n\r") %
          aff->duration % sprintbit_64(aff->bitvector, affected_bits);
        break;
	
      case AFFECT_WARY:
	str += "State: Wary\n\r";
	str += "     Decreases chance of multiple cudgels\n\r";
	break;

      case AFFECT_DEFECTED:
	str += "Player recently defected from a faction.\n\r";
	str += format("     Expires in %6d updates.\n\r") % aff->duration;
	break;
	
      case AFFECT_OFFER:
	f = get_guild_by_ID(aff->modifier);
	if (!f) {
	  vlogf(LOG_FACT, "char had faction offer from non-existant faction in cmd_stat");
	  break;
	}
	str += format("Received offer to join %s (%d).\n\r") %
          f->getName() % f->ID;
	str += format("     Expires in %6d updates.\n\r") % aff->duration;
	break;
	
      case AFFECT_OBJECT_USED:
        objused = aff->modifier;

	str += format("Used magical object: %s\n\r") %
          obj_index[objused].short_desc;
        str += format("     Expires in %6d updates.\n\r") % aff->duration;
        break;

      case AFFECT_BITTEN_BY_VAMPIRE:
	str += "Bitten by vampire.\n\r";
	str += format("     Expires in %6d updates.\n\r") % aff->duration;
	break;


      case AFFECT_IMMORTAL_BLESSING:
	str += "Immortal's Blessing.\n\r";
        str += format("     Modifies %s by %ld points\n\r") %
          apply_types[aff->location].name % aff->modifier;
	str += format("     Expires in %6d updates.\n\r") % aff->duration;
	break;
      case AFFECT_PEEL_BLESSING:
	str += "Peel's Blessing.\n\r";
        str += format("     Modifies %s by %ld points\n\r") %
          apply_types[aff->location].name % aff->modifier;
	str += format("     Expires in %6d updates.\n\r") % aff->duration;
	break;
      case AFFECT_VASCO_BLESSING:
	str += "Vasco's Blessing.\n\r";
        str += format("     Modifies %s by %ld points\n\r") %
          apply_types[aff->location].name % aff->modifier;
	str += format("     Expires in %6d updates.\n\r") % aff->duration;
	break;
      case AFFECT_CORAL_BLESSING:
	str += "Coral's Blessing.\n\r";
        str += format("     Modifies %s by %ld points\n\r") %
          apply_types[aff->location].name % aff->modifier;
	str += format("     Expires in %6d updates.\n\r") % aff->duration;
	break;
      case AFFECT_ANGUS_BLESSING:
	str += "Angus's Blessing.\n\r";
        str += format("     Modifies %s by %ld points\n\r") %
          apply_types[aff->location].name % aff->modifier;
	str += format("     Expires in %6d updates.\n\r") % aff->duration;
	break;
      case AFFECT_JESUS_BLESSING:
	str += "Jesus's Blessing.\n\r";
        str += format("     Modifies %s by %ld points\n\r") %
          apply_types[aff->location].name % aff->modifier;
	str += format("     Expires in %6d updates.\n\r") % aff->duration;
	break;
      case AFFECT_DAMESCENA_BLESSING:
	str += "Damescena's Blessing.\n\r";
        str += format("     Modifies %s by %ld points\n\r") %
          apply_types[aff->location].name % aff->modifier;
	str += format("     Expires in %6d updates.\n\r") % aff->duration;
	break;
      case AFFECT_BUMP_BLESSING:
	str += "Bump's Blessing.\n\r";
        str += format("     Modifies %s by %ld points\n\r") %
          apply_types[aff->location].name % aff->modifier;
	str += format("     Expires in %6d updates.\n\r") % aff->duration;
      case AFFECT_MAROR_BLESSING:
	str += "Maror's Blessing.\n\r";
        str += format("     Modifies %s by %ld points\n\r") %
          apply_types[aff->location].name % aff->modifier;
	str += format("     Expires in %6d updates.\n\r") % aff->duration;
	break;
    case AFFECT_DASH_BLESSING:
      str += "Dash's Blessing.\n\r";
      str += format("     Modifies %s by %ld points\n\r") %
	apply_types[aff->location].name % aff->modifier;
      str += format("     Expires in %6d updates.\n\r") % aff->duration;
      break;
      case AFFECT_DEIRDRE_BLESSING:
	str += "Deirdre's Blessing.\n\r";
        str += format("     Modifies %s by %ld points\n\r") %
          apply_types[aff->location].name % aff->modifier;
	str += format("     Expires in %6d updates.\n\r") % aff->duration;
	break;
      case AFFECT_GARTHAGK_BLESSING:
	str += "Garthagk's Blessing.\n\r";
        str += format("     Modifies %s by %ld points\n\r") %
          apply_types[aff->location].name % aff->modifier;
	str += format("     Expires in %6d updates.\n\r") % aff->duration;
	break;
      case AFFECT_MERCURY_BLESSING:
	str += "Mercury's Blessing.\n\r";
        str += format("     Modifies %s by %ld points\n\r") %
          apply_types[aff->location].name % aff->modifier;
	str += format("     Expires in %6d updates.\n\r") % aff->duration;
	break;
      case AFFECT_METROHEP_BLESSING:
	str += "Metrohep's Blessing.\n\r";
        str += format("     Modifies %s by %ld points\n\r") %
          apply_types[aff->location].name % aff->modifier;
	str += format("     Expires in %6d updates.\n\r") % aff->duration;
	break;
      case AFFECT_MAGDALENA_BLESSING:
	str += "Magdalena's Blessing.\n\r";
        str += format("     Modifies %s by %ld points\n\r") %
          apply_types[aff->location].name % aff->modifier;
	str += format("     Expires in %6d updates.\n\r") % aff->duration;
	break;
      case AFFECT_MACROSS_BLESSING:
        str += "Macross's Blessing.\n\r";
        str += format("     Modifies %s by %ld points\n\r") %
          apply_types[aff->location].name % aff->modifier;
        str += format("     Expires in %6d updates.\n\r") % aff->duration;
        break;
      case AFFECT_PAPPY_BLESSING:
        str += "Pappy's Blessing.\n\r";
        str += format("     Modifies %s by %ld points\n\r") %
          apply_types[aff->location].name % aff->modifier;
        str += format("     Expires in %6d updates.\n\r") % aff->duration;
      break;
      case AFFECT_STAFFA_BLESSING:
        str += "Staffa's Blessing.\n\r";
        str += format("     Modifies %s by %ld points\n\r") %
          apply_types[aff->location].name % aff->modifier;
        str += format("     Expires in %6d updates.\n\r") % aff->duration;
      break;
      case AFFECT_PREENED:
        str += "Preened.\n\r";
        str += format("     Enables %s for winged, feathered creatures.\n\r") %
          sprintbit_64(aff->bitvector, affected_bits);
        str += format("     Expires in %6d updates.\n\r") % aff->duration;
      break;
      case AFFECT_WET:
        str += "Wet.\n\r";
        str += format("     Covered by %d fluid ounces of water.\n\r") % aff->modifier;
      break;

      case LAST_ODDBALL_AFFECT:
      case LAST_TRANSFORMED_LIMB:
      case LAST_BREATH_WEAPON:
      case DAMAGE_GUST:
      case DAMAGE_TRAP_TNT:
      case DAMAGE_ELECTRIC:
      case DAMAGE_TRAP_FROST:
      case DAMAGE_FROST:
      case DAMAGE_DROWN:
      case DAMAGE_WHIRLPOOL:
      case DAMAGE_HEMORRAGE:
      case DAMAGE_IMPALE:
      case DAMAGE_TRAP_POISON:
      case DAMAGE_ACID:
      case DAMAGE_TRAP_ACID:
      case DAMAGE_COLLISION:
      case DAMAGE_FALL:
      case DAMAGE_TRAP_BLUNT:
      case DAMAGE_TRAP_FIRE:
      case DAMAGE_FIRE:
      case DAMAGE_DISRUPTION:
      case DAMAGE_DRAIN:
      case DAMAGE_TRAP_ENERGY:
      case DAMAGE_KICK_HEAD:
      case DAMAGE_KICK_SHIN:
      case DAMAGE_KICK_SIDE:
      case DAMAGE_KICK_SOLAR:
      case DAMAGE_TRAP_DISEASE:
      case DAMAGE_SUFFOCATION:
      case DAMAGE_TRAP_SLASH:
      case DAMAGE_ARROWS:
      case DAMAGE_TRAP_PIERCE:
      case DAMAGE_DISEMBOWLED_HR:
      case DAMAGE_DISEMBOWLED_VR:
      case DAMAGE_EATTEN:
      case DAMAGE_HACKED:
      case DAMAGE_KNEESTRIKE_FOOT:
      case DAMAGE_HEADBUTT_FOOT:
      case DAMAGE_KNEESTRIKE_SHIN:
      case DAMAGE_KNEESTRIKE_KNEE:
      case DAMAGE_KNEESTRIKE_THIGH:
      case DAMAGE_HEADBUTT_LEG:
      case DAMAGE_KNEESTRIKE_SOLAR:
      case DAMAGE_HEADBUTT_BODY:
      case DAMAGE_KNEESTRIKE_CROTCH:      
      case DAMAGE_HEADBUTT_CROTCH:
      case DAMAGE_HEADBUTT_THROAT:
      case DAMAGE_KNEESTRIKE_CHIN:
      case DAMAGE_HEADBUTT_JAW:
      case DAMAGE_KNEESTRIKE_FACE:
      case DAMAGE_CAVED_SKULL:
      case DAMAGE_RIPPED_OUT_HEART:
      case DAMAGE_HEADBUTT_SKULL:
      case DAMAGE_STARVATION:
      case DAMAGE_STOMACH_WOUND:
      case DAMAGE_RAMMED:
      case DAMAGE_BEHEADED:
      case DAMAGE_NORMAL:
      case DAMAGE_TRAP_SLEEP:
      case DAMAGE_TRAP_TELEPORT:
      case MAX_SKILL:
      case TYPE_WATER:
      case TYPE_AIR:
      case TYPE_EARTH:
      case TYPE_FIRE:
      case TYPE_KICK:
      case TYPE_CLAW:
      case TYPE_SLASH:
      case TYPE_CLEAVE:
      case TYPE_SLICE:
      case TYPE_BEAR_CLAW:
      case TYPE_MAUL:
      case TYPE_SMASH:
      case TYPE_WHIP:
      case TYPE_CRUSH:
      case TYPE_BLUDGEON:
      case TYPE_SMITE:
      case TYPE_HIT:
      case TYPE_FLAIL:
      case TYPE_PUMMEL:
      case TYPE_THRASH:
      case TYPE_THUMP:
      case TYPE_WALLOP:
      case TYPE_BATTER:
      case TYPE_BEAT:
      case TYPE_STRIKE:
      case TYPE_POUND:
      case TYPE_CLUB:
      case TYPE_PIERCE:
      case TYPE_STAB:
      case TYPE_STING:
      case TYPE_THRUST:
      case TYPE_SPEAR:
      case TYPE_BEAK:
      case TYPE_BITE:
      case TYPE_SHOOT:
      case TYPE_CANNON:
      case TYPE_SHRED:
      case TYPE_UNDEFINED:
      case TYPE_MAX_HIT:
      case ABSOLUTE_MAX_SKILL:
        vlogf(LOG_BUG, format("BOGUS AFFECT (%d) on %s") % aff->type % k->getName());
        k->affectRemove(aff);
        break;
    }
  }
  if (k->task) {
    str += format("Player is busy '%s'.\n\r") % tasks[k->task->task].name;
    str += format("Time left:    %6d updates     Orignal argument:  %s\n\r") %
      k->task->timeLeft % k->task->orig_arg;
    str += format("Was in room:  %6d             Status/Flags:      %6d/%6d\n\r") %
      k->task->wasInRoom % k->task->status % k->task->flags;
  }
  for (i = 1; i < MAX_TOG_INDEX;i++) {
    if (k->hasQuestBit(i))  {
      str += format("%sToggle Set:%s (%d) %s\n\r") %
        cyan() % norm() %
        i % TogIndex[i].name;
    }
  }
#if 0
  // spams too much, use "powers xxx" instead
  wizPowerT ipow;
  for (ipow = MIN_POWER_INDEX; ipow < MAX_POWER_INDEX;ipow++) {
    if (k->hasWizPower(ipow))  {
      str += format("Wiz-Power Set: (%d) %s\n\r") % mapWizPowerToFile(ipow) % getWizPowerName(ipow);
    }
  }
#endif
  if (k->desc) {
    str += format("%sClient:%s %s\n\r") %
      cyan() % norm() %
      (k->desc->m_bIsClient ? "Yes" : "No");
  }
  
  if (km) {
    if (km->sounds) {
      str += format("%sLocal Sound:%s\n\r%s") %
        cyan() % norm() %
        km->sounds;
    }
    if (km->distantSnds) {
      str += format("%sDistant Sound:%s\n\r%s") %
        cyan() % norm() %
        km->distantSnds;
    }
  }
  desc->page_string(str);
  return;
}

void TBeing::doStat(const sstring &)
{
  return;
}

void TPerson::doStat(const sstring &argument)
{
  sstring arg1, arg2, arg3;
  sstring tmp_arg;
  TBeing *k = NULL;
  TObj *j = NULL;
  int count, parm = 0;
  int foundNum = FALSE;

  if (!isImmortal()) {
    incorrectCommand();
    return;
  }
  
  if (!desc)
    return;
  
  if (!hasWizPower(POWER_STAT)) {
    sendTo("Sorry, you lack the power to use the stat command.\n\r");
    return;
  }
  
  if (argument.empty()) {
    sendTo("Usage :\n\r");
    sendTo("        stat mob <name or vnum>\n\r");
    sendTo("        stat obj <name or vnum>\n\r");
    sendTo("        stat room\n\r");
    sendTo("        stat zone <number>\n\r");
    sendTo("        stat zone mobs <number>\n\r");
    sendTo("        stat zone objs <number>\n\r");
    sendTo("        stat <name>\n\r");
    sendTo("        stat <name> discipline <name or number>\n\r");
    sendTo("        stat <name> skill <name or number>\n\r");
    sendTo("        stat <name> donebasic\n\r");
    return;
  }
  
  tmp_arg = argument;
  tmp_arg = one_argument(tmp_arg, arg1);
  tmp_arg = one_argument(tmp_arg, arg2);
  sstring whitespace = " \n\r\t";
  size_t start = tmp_arg.find_first_not_of(whitespace);
  if (start != sstring::npos) {
    size_t end = tmp_arg.find_last_not_of(whitespace);
    arg3 = tmp_arg.substr(start, end - start + 1);
  } else {
    arg3 = "";
  }
  
  if (arg1 == "mob") {
    // ***** begin stat mob
    if (!hasWizPower(POWER_STAT_MOBILES)) {
      sendTo("Sorry, you lack the power to stat mobiles.\n\r");
      return;
    }
    if (is_number(arg2) && (parm = convertTo<int>(arg2))) {
      // check by vnum
      TMonster *tMonster;
      unsigned int rnum = real_mobile(parm);
      if (rnum >= 0 && rnum < mob_index.size()) {
        if ((tMonster = read_mobile(rnum, REAL))) {
          statBeing(tMonster);
          delete tMonster;
          tMonster = NULL;
          return;
        } else {
          sendTo("No mobile found with that vnum.\n\r");
          return;
        }
      }
      sendTo("Bad value for mobile vnum.\n\r");
      return;
    }
    // check by name
    count = 1; // looks like not all of the get_ routines incrememnt count
    if (!(k = get_char_room(arg2, in_room))) {
      if (!(k = get_char_vis_world(this, arg2, &count, EXACT_YES))) {
        if (!(k = get_char_vis_world(this, arg2, &count, EXACT_NO))) {
          sendTo("No mobile by that name in The World.\n\r");
          return;
        }
      }
    }
    statBeing(k);
    return;
    // ***** end stat mob
    
  } else if (arg1 == "obj") {
    // ***** begin stat obj
    if (!hasWizPower(POWER_STAT_OBJECT)) {
      sendTo("Sorry, you lack the power to stat objects.\n\r");
      return;
    }
    if (is_number(arg2) && (parm = convertTo<int>(arg2))) {
      // check by vnum
      TObj *tObj;
      unsigned int rnum = real_object(parm);
      if (rnum >= 0 && rnum < obj_index.size()) {
        if ((tObj = read_object(rnum, REAL))) {
          statObj(tObj);
          delete tObj;
          tObj = NULL;
          return;
        } else {
          sendTo("No object found with that vnum.\n\r");
          return;
        }
      }
      sendTo("Bad value for object vnum.\n\r");
      return;
    }
    count = 1;
    if ((j = get_obj_vis_accessible(this, arg2)) || (j = get_obj_vis(this, arg2.c_str(), &count, EXACT_NO))) {
      statObj(j);
      return;
    } else {
      sendTo("No object by that name in the World.\n\r");
      return;
    }
    // ***** end stat obj
    
  } else if (arg1 == "room") {
    // ***** begin stat room
    statRoom(roomp);
    return;
    // ***** end stat room
    
  } else if (arg1 == "zone") {
    // ***** begin stat zone
    if (!hasWizPower(POWER_STAT_OBJECT) || !hasWizPower(POWER_STAT_MOBILES)) {
      sendTo("Sorry, you lack the power to stat zones.\n\r");
      return;
    }
    // mobs and objs options added to spit out items from the zonefile
    // kind of like show mobs or show objs does it based on the zone's vnum range
    if (is_abbrev(arg2, "mobs")) {
      statZoneMobs(arg3);
    } else if (is_abbrev(arg2, "objs")) {
      statZoneObjs(arg3);
    } else {
      statZone(arg2);
    }
    return;
    // ***** end stat zone
    
  } else {
    // // ***** begin stat player
    if (!hasWizPower(POWER_STAT_SKILL)) {
      sendTo("Sorry, you lack the power to stat players.\n\r");
      return;
    }
    count = 1;
    if (is_abbrev(arg2, "discipline")) {
      // ***** begin stat discipline
      // search in mob room, pc in world, mob in world
      if (!(k = get_char_room(arg1, in_room))) {
        if (!(k = get_pc_world(this, arg1, EXACT_NO))) {
          if (!(k = get_char_vis_world(this, arg1, &count, EXACT_YES))) {
            if (!(k = get_char_vis_world(this, arg1, &count, EXACT_NO))) {
              sendTo("That person could not be found in the World.\n\r");
              return;
            }
          }
        }
      }
      sstring cap_name = k->getName();
      cap_name = cap_name.cap();
      if (!k->discs) {
        sendTo(COLOR_MOBS, format("%s does not have any disciplines allocated yet.\n\r") % cap_name);
        return;
      }
      discNumT dnt;
      CDiscipline *cd;
      if (arg3.empty()) {
        // discipline summary
        sendTo(COLOR_MOBS, format("%s has the following disciplines:\n\r\n\r") % cap_name);
        sendTo(COLOR_MOBS, "                    <c>Discipline Num   Current Natural<1>\n\r");
        for (dnt = MIN_DISC; dnt < MAX_DISCS; dnt++) {
          if (!(cd = k->getDiscipline(dnt))) {
            continue;
          }
          if (cd->getNatLearnedness() == 0 && cd->getLearnedness() == 0) 
            continue;
          sendTo(COLOR_MOBS, format("%30s %3d :     %3d     %3d\n\r") % discNames[dnt].properName % mapDiscToFile(dnt) % cd->getLearnedness()  % cd->getNatLearnedness());
          /* sendTo(COLOR_MOBS, format("%30s : Current (%d) Natural (%d).\n\r") % discNames[dnt].properName % cd->getLearnedness()  % cd->getNatLearnedness()); */
        }
        return;
      }
      
      if (is_number(arg3)) {
        // search by number
        // should search by mapped disc number because that's what someone would probably be entering here, no?
        // why are these mapped, anyway?
        parm = convertTo<int>(arg3);
        dnt = mapFileToDisc(parm);
        if (dnt < MIN_DISC || dnt >= MAX_DISCS) {
          sendTo("Not a valid discipline number.\n\r");
          return;
        }
      } else {
        // search by name
        foundNum = FALSE;
        // search for exact name
        for (dnt = MIN_DISC; dnt < MAX_DISCS; dnt++) {
          if (is_exact_name(arg3, discNames[dnt].name)) {
            foundNum = TRUE;
            break;
          }
        }
        // search by abbr name (probably need to account for disciplines that are partial names of others somehow)
        if (!foundNum) {
          for (dnt = MIN_DISC; dnt < MAX_DISCS; dnt++) {
            if (isname(arg3, discNames[dnt].name)) {
              foundNum = TRUE;
              break;
            }
          } 
        }
        if (!foundNum) {
          sendTo("No discipline by that name found.\n\r");
          return;
        }
      }
      
      sendTo(COLOR_MOBS, format("<c>%s<1> is discipline number <c>%d<1>.\n\r") % discNames[dnt].properName % mapDiscToFile(dnt));
      
      if (!(cd = k->getDiscipline(dnt))) {
        sendTo(COLOR_MOBS, format("%s does not appear to have <c>%s<1>.\n\r") % cap_name % discNames[dnt].properName);
        return;
      }
      sendTo(COLOR_MOBS, format("%s's learning in <c>%s<1>: Current (%d) Natural (%d).\n\r") % cap_name % discNames[dnt].properName % cd->getLearnedness() % cd->getNatLearnedness());
      return;
      
      // ***** end stat discipline
    } else if (is_abbrev(arg2, "skill")) {
      // ***** begin stat skill
      // search in mob room, pc in world, mob in world
      if (!(k = get_char_room(arg1, in_room))) {
        if (!(k = get_pc_world(this, arg1, EXACT_NO))) {
          if (!(k = get_char_vis_world(this, arg1, &count, EXACT_YES))) {
            if (!(k = get_char_vis_world(this, arg1, &count, EXACT_NO))) {
              sendTo("That person could not be found in the World.\n\r");
              return;
            }
          }
        }
      }
      sstring cap_name = k->getName();
      cap_name = cap_name.cap();
      spellNumT snt;
      if (is_number(arg3)) {
        // search by number
        parm = convertTo<int>(arg3);
        if ((parm < MIN_SPELL) || (parm >= MAX_SKILL)) {
          sendTo("Not a valid skill number.\n\r");
          return;
        }
        snt = spellNumT(parm);
      } else {
        // search by name
        foundNum = FALSE;
        for (snt = MIN_SPELL; snt < MAX_SKILL; snt++) {
          if (hideThisSpell(snt)) {
            continue;
          }
          if (is_exact_name(arg3, discArray[snt]->name)) {
            foundNum = TRUE;
            break;
          }
        }
        if (!foundNum) {
          sstring buf;
          for (snt = MIN_SPELL; snt < MAX_SKILL; snt++) {
            if (hideThisSpell(snt)) {
              continue;
            }
            buf = discArray[snt]->name;
            // kludge since chivalry < chi  in discarray
            if ((arg3 == "chi") && (buf != "chi")) {
              continue;
            }
            // kludge since stealth < steal in discarray
            if ((arg3 == "steal") && (buf != "steal")) {
              continue;
            }
            // kludge since paralyze limb < paralyze in discarray
            if ((arg3 == "paralyze") && (buf != "paralyze")) {
              continue;
            }
            if (isname(arg3, discArray[snt]->name)) {
              foundNum = TRUE;
              break;
            }
          } 
        }
        if (!foundNum) {
          sendTo("No skill by that name found.\n\r");
          return;
        }
        sendTo(COLOR_MOBS, format("<c>%s<1> is skill number <c>%d<1>.\n\r") % ((sstring)(discArray[snt]->name ? discArray[snt]->name : "unknown")).cap() % snt);
      }
      
      if (!k->doesKnowSkill(snt)) {
        if (discArray[snt]) {
          sendTo(COLOR_MOBS, format("%s does not appear to know <c>%s<1>.\n\r") % cap_name % (discArray[snt]->name ? discArray[snt]->name : "unknown"));
        } else {
          sendTo(COLOR_MOBS, format("%s does not appear to know that skill.\n\r") % cap_name);
        }
        return;
      }
      CSkill *sk = k->getSkill(snt);
      if (!sk) {
        if (discArray[snt]) {
          sendTo(COLOR_MOBS, format("%s does not appear to know <c>%s<1>.\n\r") % cap_name % (discArray[snt]->name ? discArray[snt]->name : "unknown"));
        } else {
          sendTo(COLOR_MOBS, format("%s does not appear to know that skill.\n\r") % cap_name);
        }
        return;
      }
      sendTo(COLOR_MOBS, format("%s's <c>%s<1>: Raw (stored) learning:  Current (%d) Natural (%d).\n\r") % cap_name % discArray[snt]->name % k->getRawSkillValue(snt) % k->getRawNatSkillValue(snt));
      sendTo(COLOR_MOBS, format("%s's <c>%s<1>: Actual (used) learning: Current (%d) Natural (%d) Max (%d).\n\r") % cap_name % discArray[snt]->name % k->getSkillValue(snt) % k->getNatSkillValue(snt) % k->getMaxSkillValue(snt));

      time_t ct = sk->lastUsed;
      char *tmstr = (char *) asctime(localtime(&ct));
      *(tmstr + strlen(tmstr) - 1) = '\0';
      sendTo(COLOR_MOBS, format("%s's <c>%s<1>: Last increased:         %s\n\r") % cap_name % discArray[snt]->name % tmstr);
      return;
      // ***** end stat skill
      
    } else if (is_abbrev(arg2, "donebasic")) {
      // ***** begin stat donebasic
      // search for pc in world
      if (!(k = get_pc_world(this, arg1, EXACT_YES))) {
        if (!(k = get_pc_world(this, arg1, EXACT_NO))) {
          // add an object search maybe
          sendTo("That person could not be found in the World.\n\r");
          return;
        }
      }
      sstring cap_name = k->getName();
      cap_name = cap_name.cap();
      sendTo(COLOR_MOBS, format("Basic discipline completion data for %s.\n\r") % k->getName());
      sendTo("Lvl    Class\n\r");
      sstring buf;
      for (count = 0; count < MAX_CLASSES; count++) {
        if (!hasClass(count))
          continue;
        if (k->player.doneBasic[count]) {
          buf = format("%d") % k->player.doneBasic[count];
        } else {
          buf = "NA";
        }
        sendTo(format("<c>%2s<1>  :  %s\n\r") % buf % classInfo[count].name.cap());
      }
      return;
      // ***** end stat donebasic
    } else {
      // empty or invalid arg2
      // look in room for mob, room for obj, world for mob, world for obj
      if (!(k = get_char_room(arg1, in_room))) {
        if (!(j = get_obj_vis_accessible(this, arg1))) {
          if (!(k = get_pc_world(this, arg1, EXACT_NO))) {
            if (!(k = get_char_vis_world(this, arg1, &count, EXACT_YES))) {
              if (!(k = get_char_vis_world(this, arg1, &count, EXACT_NO))) {
                if (!(j = get_obj_vis(this, arg1.c_str(), &count, EXACT_NO))) {
                  sendTo("No such mobile or object could be found in the World.\n\r");
                  return;
                }
              }
            }
          }
        }
      }
      if (k) {
        statBeing(k);
      } else if (j) {
        statObj(j);
      } else {
        vlogf(LOG_BUG, format("doStat fell through looking for %s.") % arg1);
      }
    }
  }  
}
