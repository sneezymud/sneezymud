//////////////////////////////////////////////////////////////////////////
//
// SneezyMUD - All rights reserved, SneezyMUD Coding Team
//
//      "cmd_stat.cc" - The stat command
//  
//////////////////////////////////////////////////////////////////////////

#include "stdsneezy.h"
#include "disease.h"

extern int eqHpBonus(const TPerson *);
extern int baseHp();
extern float classHpPerLevel(const TPerson *);
extern int ageHpMod(const TPerson *);


void TBeing::statZone(const char *zoneNumber)
{
  int zNum,
    cnDesc[3]  = {0, 0, 0},
    cnTitle[3] = {0, 0, 0},
    cnExtra[2] = {0, 0},
    cnFlags[7] = {0, 0, 0, 0, 0, 0, 0},
    rzCount    = 0;
  sstring sb("");
  unsigned long int rNums[2] = {0, 0},
                    Runner = 0;
  char tString[256];
  TRoom *curRoomCntr;

  if (!zoneNumber || !*zoneNumber) {
    if (!roomp) {
      vlogf(LOG_BUG, "statZone called by being with no current room.");
      return;
    }

    zNum = roomp->getZoneNum();
  } else
    zNum = convertTo<int>(zoneNumber);

  if (zNum < 0 || zNum >= (signed int) zone_table.size()) {
    sendTo("Zone number incorrect.\n\r");
    return;
  }

  zoneData &zne = zone_table[zNum];

  Runner = rNums[0] = (zNum ? zone_table[zNum - 1].top + 1 : 0);
  rNums[1] = zne.top;

  sb += "Basic Information:\n\r--------------------\n\r";
  sprintf(tString, "Zone Num: %3d     Active: %s\n\r",
          zNum, (zne.enabled ? "Enabled" : "Disabled"));
  sb += tString;

  for (; Runner < (rNums[1] + 1); Runner++)
    if ((curRoomCntr = real_roomp(Runner))) {
      rzCount++;

      if (curRoomCntr->getDescr()) {// Count Descriptions
        cnDesc[0]++;

        if (!strncmp(curRoomCntr->getDescr(), "Empty", 5))
          cnDesc[2]++;
      } else
        cnDesc[1]++;

      if (curRoomCntr->name) {// Count Titles
        cnTitle[0]++;

        sprintf(tString, "%d", curRoomCntr->number);

        if (strstr(curRoomCntr->name, tString))
          cnTitle[2]++;
      } else
        cnTitle[1]++;

      if (curRoomCntr->ex_description)// Count Rooms with extra descriptions
        cnExtra[0]++;
      else
        cnExtra[1]++;

      if (curRoomCntr->isRoomFlag(ROOM_DEATH    ))// Count DEATH_ROOM flags
        cnFlags[0]++;
      if (curRoomCntr->isRoomFlag(ROOM_NO_FLEE  ))// Count NO_FLEE flags
        cnFlags[1]++;
      if (curRoomCntr->isRoomFlag(ROOM_PEACEFUL ))// Count PEACEFUL flags
        cnFlags[2]++;
      if (curRoomCntr->isRoomFlag(ROOM_NO_HEAL  ))// Count NO_HEAL flags
        cnFlags[3]++;
      if (curRoomCntr->isRoomFlag(ROOM_SAVE_ROOM))// Count SAVE_ROOM flags
        cnFlags[4]++;
      if (curRoomCntr->isRoomFlag(ROOM_INDOORS  ))// Count INDOOR flags
        cnFlags[5]++;
      if (curRoomCntr->isRoomFlag(ROOM_PEACEFUL) &&
          !curRoomCntr->isRoomFlag(ROOM_NO_HEAL))// If is Peaceful should ALWAYS be no-heal
        cnFlags[6]++;
    }

  sprintf(tString, "S-Room: %5lu     E-Room: %5lu     Total:(%lu/%d)\n\r",
          rNums[0], rNums[1], (rNums[1] - rNums[0] + 1), rzCount);
  sb += tString;
  sb += "Key Information:\n\r--------------------\n\r";
  sprintf(tString, "DescrCount: %3d     NoDescr: %3d     InDescr: %3d\n\r",
          cnDesc[0], cnDesc[1], cnDesc[2]);
  sb += tString;
  sprintf(tString, "TitleCount: %3d     NoTitle: %3d     InTitle: %3d\n\r",
          cnTitle[0], cnTitle[1], cnTitle[2]);
  sb += tString;
  sprintf(tString, "ExtraCount: %3d     NoExtra: %3d     (Room Counts)\n\r",
          cnExtra[0], cnExtra[1]);
  sb += tString;
  sb += "Key Flags:\n\r--------------------\n\r";
  if (cnFlags[0]) {
    sprintf(tString, "Death-Rooms: %3d\n\r", cnFlags[0]);
    sb += tString;
  }
  if (cnFlags[1]) {
    sprintf(tString, "No-Flee    : %3d\n\r", cnFlags[1]);
    sb += tString;
  }
  if (cnFlags[2]) {
    sprintf(tString, "Peaceful   : %3d\n\r", cnFlags[2]);
    sb += tString;
  }
  if (cnFlags[3]) {
    sprintf(tString, "No-Heal    : %3d\n\r", cnFlags[3]);
    sb += tString;
  }
  if (cnFlags[6]) {
    sprintf(tString, "...CRITICAL: +Peaceful !No-Heal: %3d\n\r", cnFlags[6]);
    sb += tString;
  }
  if (cnFlags[4]) {
    sprintf(tString, "Save-Room%c : %3d\n\r", (cnFlags[4] > 1 ? 's' : ' '), cnFlags[4]);
    sb += tString;
  }
  if (cnFlags[5]) {
    sprintf(tString, "Indoors    : %3d\n\r", cnFlags[5]);
    sb += tString;
  }

  desc->page_string(sb, SHOWNOW_NO, ALLOWREP_YES);
}

void TBeing::statRoom(TRoom *rmp)
{
  sstring str;
  sstring tmp_str;
  sstring buf2;
  char buf3[80], buf4[80];
  extraDescription *e;
  TThing *t;
  int counter = 0, volume;

  if (!limitPowerCheck(CMD_EDIT, rmp->number)) {
    sendTo("You are not allowed to stat this room, sorry.\n\r");
    return;
  }


  str = fmt("Room name: %s, Of zone : %d. V-Number : %d, R-number : %d\n\r") %
    rmp->name % rmp->getZoneNum() % rmp->number % in_room;

  str += fmt("Room Coords: %d, %d, %d\n\r") %
    rmp->getXCoord() % rmp->getYCoord() % rmp->getZCoord();

  str += fmt("Sector type : %s ") % TerrainInfo[rmp->getSectorType()]->name;

  str += "Special procedure : ";

  str += fmt("%s") % ((rmp->funct) ? "Exists\n\r" : "No\n\r");

  str += "Room flags: ";

  str += sprintbit((long) rmp->getRoomFlags(), room_bits);;
  str += "\n\r";

  str += "Room flag bit vector: ";

  str += fmt("%d\n\r") % ((unsigned int) rmp->getRoomFlags());

  str += "Description:\n\r";
  tmp_str = rmp->getDescr();
  if (tmp_str.empty()) {
    str += "NO DESCRIPTION\n\r";
  } else {
    str += tmp_str.toCRLF();
  }

  str += "Extra description keywords(s): ";
  if (rmp->ex_description) {
    str += "\n\r";
    for (e = rmp->ex_description; e; e = e->next) {
      str += e->keyword;
      str += "\n\r";
    }
    str += "\n\r";
  } else {
    str += "None.\n\r";
  }

  sprintf(buf3, "%d", rmp->getRoomHeight());
  sprintf(buf4, "%d", rmp->getMoblim());
  str += fmt("Light : %d   Room Height : %s    Maximum capacity : %s\n\r") %
    rmp->getLight() %
    ((rmp->getRoomHeight() <= 0) ? "unlimited" : buf3) %
    ((rmp->getMoblim()) ? buf4 : "Infinite");

  if (rmp->isWaterSector() || rmp->isUnderwaterSector()) {
    str += fmt("River direction : %s") % 
      ((rmp->getRiverDir() < 0) ? "None" : dirs[rmp->getRiverDir()]);

    if (rmp->getRiverSpeed() >= 1)
      str += fmt("   River speed : Every %d heartbeat%s\n\r") %
	rmp->getRiverSpeed() % ((rmp->getRiverSpeed() != 1) ? "s." : ".");
    else
      str += fmt("   River speed : no current.\n\r");

    str += fmt("Fish caught : %i\n\r") % rmp->getFished();

  }
  if ((rmp->getTeleTarg() > 0) && (rmp->getTeleTime() > 0)) {
    str += fmt("Teleport speed : Every %d heartbeats. To room : %d. Look? %s.\n\r") %
      rmp->getTeleTime() % rmp->getTeleTarg() %
      (rmp->getTeleLook() ? "yes" : "no");
  }
  str += "------- Chars present -------\n\r";
  counter = 0;
  for (t = rmp->getStuff(); t; t = t->nextThing) {
    // canSee prevents seeing invis gods of higher level
    if (dynamic_cast<TBeing *>(t) && canSee(t)) {
      counter++;
      if (counter > 15) {
         str += "Too Many In Room to Stat More\n\r";
         break;
      } else {
        str += fmt("%s%s   (%s)\n\r") %
	  t->getName() % (dynamic_cast<TPerson *>(t) ? "(PC)" : "(NPC)") %
	  t->name;
      }
    }
  }
  str += "--------- Born Here ---------\n\r";
  counter = 0;
  for (t = rmp->tBornInsideMe; t; t = t->nextBorn) {
    TMonster *tMonster;

    if ((tMonster = dynamic_cast<TMonster *>(t))) {
      counter++;

      if (counter > 5) {
        str += "Too Many Creators Born In Room To Show More\n\r";
        break;
      } else {
        str += fmt("[%6d] %s\n\r") % tMonster->mobVnum() % tMonster->getName();
      }
    }
  }
  str += "--------- Contents ---------\n\r";
  counter = 0;
  volume = 0;
  buf2="";
  for (t = rmp->getStuff(); t; t = t->nextThing) {
    if (!dynamic_cast<TBeing *>(t)) {
      volume += t->getVolume();
      counter++;
      if (counter > 20) {
        buf2 += "Too Many In Room to Stat More\n\r";
        break;
      } else {
        buf2 += fmt("%s   (%s)\n\r") % t->getName() % t->name;
      }
    }
  }
  str += fmt("Total Volume: %s\n\r") % volumeDisplay(volume);
  str += buf2;

  str += "------- Exits defined -------\n\r";
  dirTypeT dir;
  for (dir = MIN_DIR; dir < MAX_DIR; dir++) {
    if (!rmp->dir_option[dir])
      continue;
    else {
      str+=fmt("Direction : %-10s    Door Type : %-12s     To-Room : %d\n\r") %
	dirs[dir] % door_types[rmp->dir_option[dir]->door_type] %
	rmp->dir_option[dir]->to_room;
      if (rmp->dir_option[dir]->door_type != DOOR_NONE) {
        str += fmt("Weight : %d      Exit Flags : %s\n\rKeywords : %s\n\r") %
              rmp->dir_option[dir]->weight % 
	  sprintbit(rmp->dir_option[dir]->condition, exit_bits) %
	  rmp->dir_option[dir]->keyword;
        if ((rmp->dir_option[dir]->key > 0) || 
             (rmp->dir_option[dir]->lock_difficulty >= 0)) {
          str += fmt("Key Number : %d     Lock Difficulty: %d\n\r") %
	    rmp->dir_option[dir]->key % rmp->dir_option[dir]->lock_difficulty;
        }
        if (IS_SET(rmp->dir_option[dir]->condition, EX_TRAPPED)) {
          sprinttype(rmp->dir_option[dir]->trap_info, trap_types, buf3);
          str += fmt("Trap type : %s,  Trap damage : %d (d8)\n\r") % 
	    buf3 % rmp->dir_option[dir]->trap_dam;
        }
      } else if (IS_SET(rmp->dir_option[dir]->condition, EX_SLOPED_UP)) {
        str += "Sloped: Up\n\r";
      } else if (IS_SET(rmp->dir_option[dir]->condition, EX_SLOPED_DOWN)) {
        str += "Sloped: Down\n\r";
      }
      str += "Description:\n\r  ";
      if (rmp->dir_option[dir]->description)
        str += rmp->dir_option[dir]->description;
      else
        str += "UNDEFINED\n\r";
    }
  }
  desc->page_string(str);
  return;
}

void TBeing::statObj(const TObj *j)
{
  char buf[256];
  extraDescription *e;
  TThing *t;
  int i;
  sstring str;

  if (!limitPowerCheck(CMD_OEDIT, j->getSnum())) {
    sendTo("You are not allowed to stat that object, sorry.\n\r");
    return;
  }

  
  sprintf(buf, "Object name: [%s], R-number: [%d], V-number: [%d] Item type: ",
       j->name, j->number, obj_index[j->getItemIndex()].virt);
  str = buf;

  str += ItemInfo[j->itemType()]->name;
  str += "\n\r";

  for (unsigned int zone = 0; zone < zone_table.size(); zone++) {
    if(obj_index[j->getItemIndex()].virt <= zone_table[zone].top){
      sprintf(buf, "Zone: %s\n\r", zone_table[zone].name);
      break;
    }    
  }
  str += buf;

  sprintf(buf, "Short description: %s\n\rLong description:\n\r%s\n\r",
        ((j->shortDescr) ? j->shortDescr : "None"),
        ((j->getDescr()) ? j->getDescr() : "None"));
  str += buf;

  if (j->action_description) {
    str += "Action Description: ";
    str += j->action_description;
    str += "\n\r";
  }

  sprintf(buf, "Action pointer: %s\n\r", (j->act_ptr ? "YES" : "no") );
  str += buf;

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
    sprintf(buf, "Owners: [%s]\n\r", j->owners);
    str += buf;
  }

  str += "Can be worn on :";
  str += sprintbit(j->obj_flags.wear_flags, wear_bits);
  str += "\n\r";

  str += "Set char bits  :";
  str += sprintbit(j->obj_flags.bitvector, affected_bits);
  str += "\n\r";

  str += "Extra flags: ";
  str += sprintbit(j->getObjStat(), extra_bits);
  str += "\n\r";

  sprintf(buf, "Can be seen : %d\n\r", j->canBeSeen);
  str += buf;

  sprintf(buf, "Volume: %d, Weight: %.1f, Value: %d, Cost/day: %d\n\r",
    j->getVolume(),
    j->getWeight(), j->obj_flags.cost,
    j->rentCost());
  str += buf;

  sprintf(buf, "Decay :%d, Max Struct :%d, Struct Left %d, Depreciation %d\n\r",
    j->obj_flags.decay_time,
    j->getMaxStructPoints(),
    j->getStructPoints(),
    j->getDepreciation());
  str += buf;

  sprintf(buf, "Light: %3d          Material Type : %s\n\r",
       j->getLight(),  material_nums[j->getMaterial()].mat_name);
  str += buf;

  if (j->inRoom() != ROOM_NOWHERE)
    sprintf(buf, "In Room: %d\n\r", j->inRoom());
  else if (j->parent)
    sprintf(buf, "Inside: %s\n\r", j->parent->getName());
  else if (j->stuckIn)
    sprintf(buf, "Stuck-In: %s (slot=%d)\n\r", j->stuckIn->getName(), j->eq_stuck);
  else if (j->equippedBy)
    sprintf(buf, "Equipped-by: %s (slot=%d)\n\r", j->equippedBy->getName(), j->eq_pos);
  else
    sprintf(buf, "UNKNOWN LOCATION !!!!!!\n\r");
  str += buf;

  sprintf(buf, "Carried weight: %.1f   Carried volume: %d\n\r",
          j->getCarriedWeight(), j->getCarriedVolume());
  str += buf;

  str += j->statObjInfo();

  sprintf(buf, "\n\rSpecial procedure : %s   ", j->spec ? objSpecials[GET_OBJ_SPE_INDEX(j->spec)].name : "none");;
  str += buf;

  if (!j->getStuff())
    str += "Contains : Nothing\n\r";
  else {
    str += "Contains :\n\r";
    for (t = j->getStuff(); t; t = t->nextThing) {
      //      str += fname(t->name);
      str += t->shortDescr;
      str += " (";
      str += t->name;
      str += ")";
      str += "\n\r";
    }
  }

  str += "Can affect char :\n\r";
  for (i = 0; i < MAX_OBJ_AFFECT; i++) {
    if (j->affected[i].location == APPLY_SPELL) {
      if (discArray[j->affected[i].modifier]) {
        sprintf(buf, "   Affects:  %s: %s by %ld\n\r",
            apply_types[j->affected[i].location].name,
            discArray[j->affected[i].modifier]->name,
            j->affected[i].modifier2);
        str += buf;
      } else
        vlogf(LOG_BUG, fmt("BOGUS AFFECT (%d) on %s") %  j->affected[i].modifier % 
              j->getName());
    } else if (j->affected[i].location == APPLY_DISCIPLINE) {
     if (discNames[j->affected[i].modifier].disc_num) {
        sprintf(buf, "   Affects:  %s: %s by %ld\n\r",
            apply_types[j->affected[i].location].name,
            discNames[j->affected[i].modifier].practice,
            j->affected[i].modifier2);
        str += buf;
      } else
        vlogf(LOG_BUG, fmt("BOGUS AFFECT (%d) on %s") %  j->affected[i].modifier %
              j->getName());
    } else if (j->affected[i].location == APPLY_IMMUNITY) {
      sprintf(buf, "   Affects:  %s: %s by %ld\n\r",apply_types[j->affected[i].location].name,
        immunity_names[j->affected[i].modifier], j->affected[i].modifier2);
      str += buf;
    } else if (j->affected[i].location != APPLY_NONE) {
      sprintf(buf, "   Affects:  %s by %ld\n\r",apply_types[j->affected[i].location].name,
        j->affected[i].modifier);
      str += buf;
    }
  }
  desc->page_string(str);
  return;
}

void TBeing::statObjForDivman(const TObj *j)
{
  char buf[256];
  TThing *t;
  int i;
  sstring str;


  for (unsigned int zone = 0; zone < zone_table.size(); zone++) {
    if(obj_index[j->getItemIndex()].virt <= zone_table[zone].top){
      sprintf(buf, "The item is from %s.\n\r", zone_table[zone].name);
      break;
    }    
  }
  str += buf;

  sprintf(buf, "\n\r%s is a %s.", j->shortDescr, ItemInfo[j->itemType()]->name);
  str += buf;
  str += "\n\r";

  str += "It can be worn on: ";
  str += sprintbit(j->obj_flags.wear_flags, wear_bits);
  str += ".\n\r";

  str += "The item sets the character bits: ";
  str += sprintbit(j->obj_flags.bitvector, affected_bits);
  str += ".\n\r";

  str += "It's extra flags are: ";
  str += sprintbit(j->getObjStat(), extra_bits);
  str += ".\n\r";

  sprintf(buf, "%s modifies can be seen by %d.\n\r", j->shortDescr, j->canBeSeen);
  str += buf;

  sprintf(buf, "It has a volume of %d, it weighs %.1f, and has a value of %d talens.\n\r",
    j->getVolume(),
    j->getWeight(), j->obj_flags.cost);
  str += buf;

  sprintf(buf, "It will decay in %d, and it's structure is %d/%d.\n\r",
    j->obj_flags.decay_time,
    j->getStructPoints(),
    j->getMaxStructPoints());
  str += buf;

  sprintf(buf, "Light is modified by %3d and %s is made of %s.\n\r",
       j->getLight(),  j->shortDescr, material_nums[j->getMaterial()].mat_name);
  str += buf;

  str += j->statObjInfo();

  sprintf(buf, "\n\rIt has %s for a special procedure.\n\r", j->spec ? objSpecials[GET_OBJ_SPE_INDEX(j->spec)].name : "nothing added");;
  str += buf;

  if (!j->getStuff())
    str += "It contains nothing...\n\r";
  else {
    str += "The item contains: \n\r";
    for (t = j->getStuff(); t; t = t->nextThing) {
      str += fname(t->name);
      str += "\n\r";
    }
  }

  str += "The item can affect you by:\n\r";
  for (i = 0; i < MAX_OBJ_AFFECT; i++) {
    if (j->affected[i].location == APPLY_SPELL) {
      if (discArray[j->affected[i].modifier]) {
        sprintf(buf, "   Affects:  %s: %s by %ld.\n\r",
            apply_types[j->affected[i].location].name,
            discArray[j->affected[i].modifier]->name,
            j->affected[i].modifier2);
        str += buf;
      } else
        vlogf(LOG_BUG, fmt("BOGUS AFFECT (%d) on %s") %  j->affected[i].modifier % 
              j->getName());
    } else if (j->affected[i].location == APPLY_DISCIPLINE) {
     if (discNames[j->affected[i].modifier].disc_num) {
        sprintf(buf, "   Affects:  %s: %s by %ld.\n\r",
            apply_types[j->affected[i].location].name,
            discNames[j->affected[i].modifier].practice,
            j->affected[i].modifier2);
        str += buf;
      } else
        vlogf(LOG_BUG, fmt("BOGUS AFFECT (%d) on %s") %  j->affected[i].modifier %
              j->getName());
    } else if (j->affected[i].location == APPLY_IMMUNITY) {
      sprintf(buf, "   Affects:  %s: %s by %ld.\n\r",apply_types[j->affected[i].location].name,
        immunity_names[j->affected[i].modifier], j->affected[i].modifier2);
      str += buf;
    } else if (j->affected[i].location != APPLY_NONE) {
      sprintf(buf, "   Affects:  %s by %ld.\n\r",apply_types[j->affected[i].location].name,
        j->affected[i].modifier);
      str += buf;
    }
  }
  str += "\n\r";
  str += "The cloud of smoke is quickly dispersed and the air is clear.\n\r";
  desc->page_string(str);
  return;
}

void TBeing::statBeing(TBeing *k)
{
  affectedData *aff, *af2;
  char buf[MAX_STRING_LENGTH];
  char buf2[256];
  char buf3[256];
  TBeing *x1;
  TFaction *f = NULL;
   const TMonster *km = dynamic_cast<const TMonster *>(k);
  char *birth, *logon;
  char birth_buf[40], logon_buf[40];
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


  *buf = *buf2 = *buf3 = *birth_buf = *logon_buf = '\0';

  switch (k->player.sex) {
    case SEX_NEUTER:
      sprintf(buf + strlen(buf),"<c>NEUTRAL-SEX<z> ");
      break;
    case SEX_MALE:
      sprintf(buf + strlen(buf),"<c>MALE<z> ");
      break;
    case SEX_FEMALE:
      sprintf(buf + strlen(buf),"<c>FEMALE<z> ");
      break;
  }

  bool is_player=dynamic_cast<const TPerson *>(k);

  if (km)
    sprintf(buf + strlen(buf), "%s - Name : %s [M-Num: %d]\n\r     In-Room[%d] Old-Room[%d] Birth-Room[%d] V-Number[%d]\n\r",
            (is_player ? "PC" : "NPC"),
            k->name, k->number, k->in_room, km->oldRoom, km->brtRoom,
            k->number >= 0 ? mob_index[km->getMobIndex()].virt : -1);
  else
    sprintf(buf + strlen(buf), "%s - Name : %s [%s: %d] Room[%d]\n\r",
            (is_player ? "PC" : "NPC"),
            k->name, (is_player ? "PID  " : "M-Num"),
	    (is_player ? k->getPlayerID() : k->number), k->in_room);


  sprintf(buf + strlen(buf),"-----------------------------------------------------------------------------\n\r");
  if (km) {
    sprintf(buf + strlen(buf),"Short description: %s\n\r",
	   (km->shortDescr ? km->shortDescr : "None"));
    sprintf(buf + strlen(buf),"Long description: %s",
    	   (km->player.longDescr ? km->player.longDescr : "None"));

  } else {
    Descriptor *d = k->desc;

    if (d && k->isPc() && k->GetMaxLevel() > MAX_MORT) {
      sprintf(buf + strlen(buf), "IMM: Office: %d\n\r", d->office);

      if (d->blockastart)
        sprintf(buf + strlen(buf), "IMM: BlockA: %d - %d\n\r", d->blockastart, d->blockaend);

      if (d->blockbstart)
        sprintf(buf + strlen(buf), "IMM: BlockB: %d - %d\n\r", d->blockbstart, d->blockbend);
    }
  }

  *buf2 = '\0';
  for (classIndT ijc = MIN_CLASS_IND; ijc < MAX_CLASSES; ijc++)
    if (k->hasClass(1<<ijc))
      sprintf(buf2 + strlen(buf2), "%s ", classInfo[ijc].name.cap().c_str());

  sprintf(buf + strlen(buf),"<c>Class :<z> %-28s\n\r", buf2);

  sprintf(buf + strlen(buf), "<c>Level :<z> [");
  for(classIndT i=MIN_CLASS_IND;i<MAX_CLASSES;i++){
    sprintf(buf + strlen(buf), "%c%c:%d ",
	    classInfo[i].name.cap()[0], classInfo[i].name.cap()[1],
	    k->getLevel(i));
  }
  strcat(buf, "]\n\r");

  strcat(buf, "<c>Race  :<z> ");
  strcat(buf, k->getMyRace()->getSingularName().c_str());

  sprintf(buf + strlen(buf), "\t%sHome :%s %s", 
          cyan(), home_terrains[k->player.hometerrain], norm());

  if (k->desc && k->desc->account) {
    if (IS_SET(k->desc->account->flags, ACCOUNT_IMMORTAL) &&
        !hasWizPower(POWER_VIEW_IMM_ACCOUNTS)) {
      sprintf(buf + strlen(buf), "\tAccount : *** Information Concealed ***\n\r");
    } else {
      sprintf(buf + strlen(buf), "\t%sAccount : %s%s\n\r", purple(), k->desc->account->name, norm());
    }
  } else
    strcat(buf, "\n\r");

  if (k->isPc()) {
    birth = asctime(localtime(&(k->player.time.birth)));
    *(birth + strlen(birth) - 1) = '\0';
    strcpy(birth_buf, birth);
    logon = asctime(localtime(&(k->player.time.logon)));
    *(logon + strlen(logon) - 1) = '\0';
    strcpy(logon_buf, logon);
    realTimePassed((time(0) - k->player.time.logon) +
                    k->player.time.played, 0, &playing_time);
    sprintf(buf + strlen(buf), "%sBirth :%s %s    %sLogon   :%s %s\n\r",
       cyan(), norm(), birth_buf, cyan(), norm(), logon_buf);
    sprintf(buf + strlen(buf), "%sPlaying time :%s %d days, %d hours.     <c>Base age:<z> %d    Age Mod: %d\n\r",
        cyan(), norm(), playing_time.day, playing_time.hours,
                k->getBaseAge(), k->age_mod);
    if (!k->desc)
      sprintf(buf + strlen(buf), "%sWARNING%s, player is offline, age will not be accurate.\n\r", red(), norm());

    sprintf(buf + strlen(buf), "%sPlayer age   :%s %d years, %d months, %d days, %d hours\n\r\n\r",
        cyan(), norm(),
        k->age()->year, k->age()->month, k->age()->day, k->age()->hours);
  }
  sprintf(buf3, "[%.2f]", k->getExp());
  sprintf(buf2, "[%.2f]", k->getMaxExp());
  sprintf(buf + strlen(buf), "%sDefRnd:%s [%3d]  %sExp    :%s %-10s  %sMax Exp:%s %-10s\n\r",
          cyan(), norm(), k->defendRound(NULL),  cyan(), norm(), buf3, cyan(), norm(), buf2);
  sprintf(buf2, "[%d]", k->getMoney());
  sprintf(buf3, "[%d]", k->getBank());
  sprintf(buf + strlen(buf), "%sVision:%s [%3d]  %sTalens :%s %-10s  %sBank    :%s %-10s\n\r",
              cyan(), norm(), k->visionBonus,
              cyan(), norm(), buf2, cyan(), norm(), buf3);

  sprintf(buf2, "[%d]", k->getHitroll());
  sprintf(buf3, "[%d]", k->getDamroll());
  sprintf(buf + strlen(buf), "%sAttRnd:%s [%3d]  %sHitroll:%s %-10s  %sDamroll :%s %-10s\n\r",
          cyan(), norm(),
          k->attackRound(NULL),
          cyan(), norm(), buf2, cyan(), norm(), buf3);
  sprintf(buf2, "[%d]", k->getHit());
  sprintf(buf3, "[%d]", k->getMove());
  if (k->hasClass(CLASS_CLERIC) || k->hasClass(CLASS_DEIKHAN))
    sprintf(buf + strlen(buf), "%sPiety :%s [%5.1f]%sHit    :%s %-10s  %sMove    :%s %-10s\n\r",
      cyan(), norm(), k->getPiety(),
      cyan(), norm(), buf2, cyan(), norm(), buf3);
  else if (k->hasClass(CLASS_SHAMAN))
    sprintf(buf + strlen(buf), "%sLifef.:%s[%6d]%sHit    :%s %-10s  %sMove    :%s %-10s\n\r",
      cyan(), norm(), k->getLifeforce(),
      cyan(), norm(), buf2, cyan(), norm(), buf3);
  else
    sprintf(buf + strlen(buf), "%sMana  :%s [%3d]  %sHit    :%s %-10s  %sMove    :%s %-10s\n\r",
      cyan(), norm(), k->getMana(),
      cyan(), norm(), buf2, cyan(), norm(), buf3);
  sprintf(buf2, "[%d]", k->hitLimit());
  sprintf(buf3, "[%d]", k->moveLimit());
  sprintf(buf + strlen(buf), "%sMxMana:%s [%3d]  %sMaxHit :%s %-10s  %sMaxMove :%s %-10s\n\r",
      cyan(), norm(), k->manaLimit(),
      cyan(), norm(), buf2, cyan(), norm(), buf3);
  sprintf(buf2, "[%d]", ageHpMod(dynamic_cast<TPerson *>(k)));

  if(dynamic_cast<TPerson *>(k)){
    sprintf(buf3, "[%f]", k->getConHpModifier());
    sprintf(buf + strlen(buf), "%sEqHp  :%s [%3d]  %sAgeHp  :%s %-10s  %sConHpMod:%s %-10s\n\r",
	    cyan(), norm(), eqHpBonus(dynamic_cast<TPerson *>(k)),
	    cyan(), norm(), buf2, cyan(), norm(), buf3);
  }

  sprintf(buf2, "[%d]", k->visibility());
  sprintf(buf3, "[%5.1f lbs]", k->getWeight());
  sprintf(buf + strlen(buf), "%sHeight:%s [%3d]  %sWeight :%s %-11s %sVisibility :%s %-10s\n\r",
      cyan(), norm(), k->getHeight(),
      cyan(), norm(), buf3, cyan(), norm(), buf2);
  sprintf(buf2, "[%d]", k->specials.conditions[FULL]);
  sprintf(buf3, "[%d]", k->specials.conditions[DRUNK]);
  sprintf(buf + strlen(buf), "%sThirst:%s [%3d]  %sHunger :%s %-10s  %sDrunk   :%s %-10s\n\r",
      cyan(), norm(), k->specials.conditions[THIRST],
      cyan(), norm(), buf2, cyan(), norm(), buf3);

  sprintf(buf3, "[%d]", k->specials.conditions[POOP]);
  sprintf(buf + strlen(buf), "%sPee   :%s [%3d]  %sPoop   :%s %-10s\n\r",
	  cyan(), norm(), k->specials.conditions[PEE], 
	  cyan(), norm(), buf3);

  sprintf(buf2, "[%d]", k->getArmor());
  sprintf(buf3, "[%d]", noise(k));
  sprintf(buf + strlen(buf), "%sLight :%s [%3d]  %sNoise  :%s %-10s  %sArmor   :%s %-10s\n\r",
      cyan(), norm(), k->getLight(),
      cyan(), norm(), buf3, cyan(), norm(), buf2);
  sprintf(buf2, "[%d]", k->eyeSight(k->roomp));
  sprintf(buf3, "[%d]", k->getSpellHitroll());
  sprintf(buf + strlen(buf), "%sProt. :%s [%3d]  %sEyesight:%s %-10s %sSpell Hitroll: %s %-10s\n\r",
      cyan(), norm(), k->getProtection(),
      cyan(), norm(), buf2, cyan(), norm(), buf3);

  if (km && !(polyed == POLY_TYPE_DISGUISE)) {
    sprintf(buf + strlen(buf), "Number of attacks : %.1f", km->getMult());
    sprintf(buf + strlen(buf), "        NPC Damage: %.1f+%d%%.\n\r",
        km->getDamLevel(), km->getDamPrecision());
    double bd = km->baseDamage();
    int chg = (int) (bd * km->getDamPrecision() / 100);
    sprintf(buf + strlen(buf), "  NPC Damage range: %d-%d.\n\r",
        max(1, (int) bd-chg), max(1, (int) bd+chg));
  } else {
    if (k->hasClass(CLASS_MONK)) {
      sprintf(buf + strlen(buf), "Number of attacks : %.2f\n\r", k->getMult());
    }

    float fx, fy;
    k->blowCount(false, fx, fy);
    sprintf(buf + strlen(buf), "Prim attacks: %.2f, Off attacks: %.2f\n\r",
          fx, fy);

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

    sprintf(buf + strlen(buf),"Prim damage: %i-%i, Off damage: %i-%i\n\r",
	    prim_min, prim_max, sec_min, sec_max);


    sprintf(buf + strlen(buf), "Approximate damage per round: %i-%i\n\r",
	    (int)((fx*(float)prim_min)+((fy*(float)sec_min))),
	    (int)((fx*(float)prim_max)+((fy*(float)sec_max))));
	    

  }
  if (TestCode5 && k->newfaction()) {
    if(k->isPc()) {
      sprintf(buf + strlen(buf), "%sFaction :%s %s%s,   %sRank :%s %s%s\n\r",
	      cyan(), norm(), k->newfaction()->getName(), norm(),
	      cyan(), norm(), k->rank(), norm());
    }
  } else {
    
    sprintf(buf + strlen(buf), "%sFaction :%s %s,   %sFaction Percent :%s %.4f\n\r",
	    cyan(), norm(), FactionInfo[k->getFaction()].faction_name,
	    cyan(), norm(), k->getPerc());
#if FACTIONS_IN_USE
    sprintf(buf + strlen(buf), "%sPerc_0 :%s %.4f   %sPerc_1 :%s %.4f   %sPerc_2 :%s %.4f   %sPerc_3 :%s %.4f\n\r",
	    cyan(), norm(), k->getPercX(FACT_NONE),
	    cyan(), norm(), k->getPercX(FACT_BROTHERHOOD),
	    cyan(), norm(), k->getPercX(FACT_CULT),
	    cyan(), norm(), k->getPercX(FACT_SNAKE));
#endif
  }
//  sprintf(buf + strlen(buf), "%sFaction :%s %s\n\r",
//    cyan(), norm(), FactionInfo[k->getFaction()].faction_name);

  sprintf(buf + strlen(buf),"Stats    :");
  sprintf(buf + strlen(buf),k->chosenStats.printStatHeader().c_str());

    statTypeT ik;

    sprintf(buf + strlen(buf),"Race     :");
    for(ik = MIN_STAT; ik<MAX_STATS_USED; ik++) {
      sprintf(buf + strlen(buf), " %3d ", k->getStat(STAT_RACE, ik));
    }
    strcat(buf, "\n\r");

    sprintf(buf + strlen(buf),"Chosen   :");
    for(ik = MIN_STAT; ik<MAX_STATS_USED; ik++) {
      sprintf(buf + strlen(buf), " %3d ", k->getStat(STAT_CHOSEN, ik));
    }
    strcat(buf, "\n\r");

    sprintf(buf + strlen(buf),"Age      :");
    for(ik = MIN_STAT; ik<MAX_STATS_USED; ik++) {
      sprintf(buf + strlen(buf), " %3d ", k->getStat(STAT_AGE, ik));
    }
    strcat(buf, "\n\r");

    sprintf(buf + strlen(buf),"Territory:");
    for(ik = MIN_STAT; ik<MAX_STATS_USED; ik++) {
      sprintf(buf + strlen(buf), " %3d ", k->getStat(STAT_TERRITORY, ik));
    }
    strcat(buf, "\n\r");

    sprintf(buf + strlen(buf),"Natural  :");

    for(ik=MIN_STAT; ik<MAX_STATS_USED; ik++) {
      sprintf(buf + strlen(buf), " %3d ", k->getStat(STAT_NATURAL, ik));
    }
    strcat(buf, "\n\r");
    
    sprintf(buf + strlen(buf),"Current  :");
    for(ik = MIN_STAT; ik<MAX_STATS_USED; ik++) {
      sprintf(buf + strlen(buf), " %3d ", k->getStat(STAT_CURRENT, ik));
    }
    strcat(buf, "\n\r");


  // only show captive info when needed
  if (k->getCaptiveOf() || k->getCaptive()) {
    sprintf(buf + strlen(buf), "%sCaptive Of:%s %s         %sCaptives :%s ",
       cyan(), norm(),
       (k->getCaptiveOf() ? k->getCaptiveOf()->getName() : "NO ONE"),
       cyan(), norm());
    if (!k->getCaptive())
      strcat(buf, "NONE\n\r");
    else {
      for (x1 = k->getCaptive(); x1; x1 = x1->getNextCaptive()) {
        strcat(buf, x1->getName());
        strcat(buf, " ");
      }
      strcat(buf, "\n\r");
    }
  }
  sprintf(buf + strlen(buf), "Master is '%s'",
          ((k->master) ? k->master->getName() : "NOBODY"));
  strcat(buf, "           Followers are:");
  for (fol = k->followers; fol; fol = fol->next)
    strcat(buf, fol->follower->getName());
  strcat(buf,"\n\r");

  if (km) {
    sprinttype(km->getPosition(), position_types, buf2);
    sprinttype((km->default_pos), position_types, buf3);
    sprintf(buf + strlen(buf), "%sPosition:%s %s, %sFighting:%s %s, %sDefault Position :%s %s\n\r",
          cyan(), norm(), buf2, cyan(), norm(),
          (km->fight() ? km->fight()->getName() : "Nobody"),
          cyan(), norm(), buf3);

    strcat(buf, "NPC flags: ");
    if (km->specials.act) {
      strcat(buf, sprintbit(km->specials.act, action_bits).c_str());
      strcat(buf, "\n\r");
    } else {
      strcat(buf, "None\n\r");
    }
  } else {
    sprinttype(k->getPosition(), position_types, buf2);
    sprintf(buf + strlen(buf), "%sPosition:%s %s, %sFighting:%s %s\n\r",
          cyan(), norm(), buf2, cyan(), norm(),
          (k->fight() ? k->fight()->getName() : "Nobody"));
  } 
  if (k->desc) {
    strcat(buf, "\n\rFlags (Specials Act): ");
    strcat(buf, sprintbit(k->desc->plr_act, player_bits).c_str());
    strcat(buf, "\n\r");
  }

  sprintf(buf + strlen(buf), "Carried weight: %.1f   Carried volume: %d\n\r",
          k->getCarriedWeight(), k->getCarriedVolume());

  immuneTypeT ij;
  for (ij = MIN_IMMUNE;ij < MAX_IMMUNES; ij++) {
    if (k->getImmunity(ij) == 0 || !*immunity_names[ij])
      continue;
    if (k->getImmunity(ij) > 0)
      sprintf(buf2,"%d%% resistant to %s.\n\r", k->getImmunity(ij),
         immunity_names[ij]);
    if (k->getImmunity(ij) < 0)
      sprintf(buf2,"%d%% susceptible to %s.\n\r", -k->getImmunity(ij),
         immunity_names[ij]);
    strcat(buf, buf2);
  }

  if (!k->isPc()) {
    const TMonster *tmons = dynamic_cast<const TMonster *>(k);
    sprintf(buf + strlen(buf), "  Action pointer: %s", 
         (tmons->act_ptr ? "YES" : "no") );
    sprintf(buf + strlen(buf), "    Special Procedure:  %s\n\r",
         (tmons->spec) ? mob_specials[GET_MOB_SPE_INDEX(tmons->spec)].name : "none");
    sprintf(buf + strlen(buf), "Anger: %d/%d     Malice: %d/%d     Suspicion: %d/%d   Greed: %d/%d\n\r",
         tmons->anger(), tmons->defanger(),
         tmons->malice(), tmons->defmalice(),
         tmons->susp(), tmons->defsusp(),
         tmons->greed(), tmons->defgreed());
    sprintf(buf + strlen(buf), "Hates: ");
    if (IS_SET(tmons->hatefield, HATE_CHAR)) {
      if (tmons->hates.clist) {
        for (list = tmons->hates.clist; list; list = list->next) {
          if (list->name)
            strcat(buf, list->name);
        }
      }
    }
    if (IS_SET(tmons->hatefield, HATE_RACE)) {
      if (tmons->hates.race != -1) {
        strcat(buf, Races[tmons->hates.race]->getSingularName().c_str());
        strcat(buf, "(Race) ");
      }
    }
    if (IS_SET(tmons->hatefield, HATE_SEX)) {
      switch (tmons->hates.sex) {
        case SEX_NEUTER:
          strcat(buf, "SEX_NEUTER ");
          break;
        case SEX_MALE:
          strcat(buf, "SEX_MALE ");
          break;
        case SEX_FEMALE:
          strcat(buf, "SEX_FEMALE ");
          break;
      }
    }
    strcat(buf, "    ");

    sprintf(buf + strlen(buf), "Fears: ");
    if (IS_SET(tmons->fearfield, FEAR_CHAR)) {
      if (tmons->fears.clist) {
        for (list = tmons->fears.clist; list; list = list->next) {
          if (list->name)
            strcat(buf, list->name);
        }
      }
    }
    if (IS_SET(tmons->fearfield, FEAR_RACE)) {
      if (tmons->fears.race != -1) {
        strcat(buf, Races[tmons->fears.race]->getSingularName().c_str());
        strcat(buf, "(Race) ");
      }
    }
    if (IS_SET(tmons->fearfield, FEAR_SEX)) {
      switch (tmons->fears.sex) {
        case SEX_NEUTER:
          strcat(buf, "SEX_NEUTER ");
          break;
        case SEX_MALE:
          strcat(buf, "SEX_MALE ");
          break;
        case SEX_FEMALE:
          strcat(buf, "SEX_FEMALE ");
          break;
      }
    }
    if (IS_SET(tmons->fearfield, FEAR_CLASS)) {
       sprintf(buf + strlen(buf), "CLASS=%d ", tmons->fears.Class);
    }
    if (IS_SET(tmons->fearfield, FEAR_VNUM)) {
       sprintf(buf + strlen(buf), "VNUM=%d ", tmons->fears.vnum);
    }
    if (IS_SET(tmons->specials.act, ACT_HUNTING)) {
      sprintf(buf + strlen(buf),
            "\n\rHunting: %s, persist: %d, origin: %d, hunt distance: %d",
            tmons->specials.hunting ? tmons->specials.hunting->getName() : "Unknown",
            tmons->persist,
            tmons->oldRoom, tmons->hunt_dist);
    } else if (tmons->specials.hunting) {
      sprintf(buf + strlen(buf),
                 "\n\rTracking: %s, persist: %d, origin: %d, range: %d",
                 tmons->specials.hunting->getName(), tmons->persist,
                 tmons->oldRoom, tmons->hunt_dist);
    }
    sprintf(buf + strlen(buf), "\n\rAI Target: %s, Random: %s",
       tmons->targ() ? tmons->targ()->getName() : "-",
       tmons->opinion.random ? tmons->opinion.random->getName() : "-");
    strcat(buf, "\n\r");
  } else {
    // PCs only
    if (k->specials.hunting) {
      sprintf(buf + strlen(buf), "Hunting: %s\n\r",
                 k->specials.hunting->getName());
    }
    const TPerson *tper = dynamic_cast<const TPerson *>(k);
    if (tper) {
      char tbf[256], *gt;
      // to prevent the titel from colorizing, mangle it slightly
      strcpy(tbf, tper->title);
      for(gt = tbf; *gt; gt++) {
        if (*gt == '>')
          *gt = '|';
      }
  
      sprintf(buf + strlen(buf), "Title:\n\r%s\n\r", tbf);
    }
  }

  strcat(buf, "Affected by: ");
  strcat(buf, sprintbit(k->specials.affectedBy, affected_bits).c_str());
  strcat(buf, "\n\r");

  strcat(buf, "\n\rBody part          Hth Max Flgs StuckIn\n\r");
  strcat(buf, "-----------------------------------\n\r");
  wearSlotT il;
  for (il = MIN_WEAR; il < MAX_WEAR; il++) {
    if (il == HOLD_RIGHT || il == HOLD_LEFT)
      continue;
    if (k->slotChance(il)) {
      sprintf(buf2, "[%s]", k->describeBodySlot(il).c_str());
      sprintf(buf + strlen(buf), "%-18s %-3d %-3d %-5d %s\n\r",
          buf2, k->getCurLimbHealth(il), k->getMaxLimbHealth(il),
                  k->getLimbFlags(il),
          (k->getStuckIn(il) ?
                       k->getStuckIn(il)->getName():
                       "None"));
    }
  }

  if (km) {
    if (km->resps && km->resps->respList) {
      sprintf(buf + strlen(buf),"Response(s):\n\r----------\n\r");
      for (respy = km->resps->respList; respy; respy = respy->next) {
        if (respy->cmd < MAX_CMD_LIST) {
          sprintf(buf + strlen(buf),"%s %s\n\r", commandArray[respy->cmd]->name, respy->args);
        } else if (respy->cmd == CMD_RESP_ROOM_ENTER) {
          sprintf(buf + strlen(buf),"roomenter\n\r");
        } else if (respy->cmd == CMD_RESP_PACKAGE) {
          sprintf(buf + strlen(buf),"package %s\n\r", respy->args);
        } else {
          sprintf(buf + strlen(buf),"%d %s\n\r", respy->cmd, respy->args);
        }
      }
      sprintf(buf + strlen(buf),"----------\n\r");

      if (km->resps->respMemory) {
        sprintf(buf + strlen(buf), "Response Memory:\n\r----------\n\r");

        for (RespMemory *rMem = km->resps->respMemory; rMem; rMem = rMem->next)
          if (rMem->cmd < MAX_CMD_LIST) {
            sprintf(buf + strlen(buf), "%s %s %s\n\r",
                    (rMem->name ? rMem->name : "Unknown"),
                    commandArray[rMem->cmd]->name,
                    (rMem->args ? rMem->args : ""));
	  } else if (rMem->cmd == CMD_RESP_ROOM_ENTER) {
            sprintf(buf + strlen(buf), "%s %s %s\n\r",
                    (rMem->name ? rMem->name : "Unknown"),
                    "roomenter",
                    (rMem->args ? rMem->args : ""));
          } else {
            sprintf(buf + strlen(buf), "%s %d %s\n\r",
                    (rMem->name ? rMem->name : "Unknown"),
                    rMem->cmd,
                    (rMem->args ? rMem->args : ""));
          }

        sprintf(buf + strlen(buf),"----------\n\r");
      }
    } else
      sprintf(buf + strlen(buf),"Response(s): None.\n\r");
  }

  strcat(buf, "\n\rAffecting Spells:\n\r--------------\n\r");
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
#if 1
      case SPELL_EARTHMAW:
      case SPELL_CREEPING_DOOM:
      case SPELL_FERAL_WRATH:
      case SPELL_SKY_SPIRIT:
#endif
        if (!discArray[aff->type]) {
          vlogf(LOG_BUG, fmt("BOGUS AFFECT (%d) on %s") %  aff->type % k->getName());
          k->affectRemove(aff);
          break;
        }

        sprintf(buf + strlen(buf), "Spell : '%s'\n\r", 
                    discArray[aff->type]->name);
        if (aff->location == APPLY_IMMUNITY)
          sprintf(buf + strlen(buf), "     Modifies %s to %s by %ld points\n\r",
            apply_types[aff->location].name, 
              immunity_names[aff->modifier], aff->modifier2);
        else if (aff->location == APPLY_SPELL)
          sprintf(buf + strlen(buf), "     Modifies %s (%s) by %ld points\n\r", apply_types[aff->location].name, (discArray[aff->modifier] ? discArray[aff->modifier]->name : "BOGUS"), aff->modifier2);
        else if (aff->location == APPLY_DISCIPLINE)
          sprintf(buf + strlen(buf), "     Modifies %s (%s) by %ld points\n\r" , apply_types[aff->location].name,  (discNames[aff->modifier].disc_num ? discNames[aff->modifier].practice : "BOGUS"), aff->modifier2);
        else
          sprintf(buf + strlen(buf), "     Modifies %s by %ld points\n\r",
            apply_types[aff->location].name, aff->modifier);
        sprintf(buf + strlen(buf), "     Expires in %6d updates, Bits set ",
          aff->duration);
        strcpy(buf2, sprintbit(aff->bitvector, affected_bits).c_str());
        strcat(buf2, "\n\r");
        strcat(buf, buf2);

        break;
      case AFFECT_DISEASE:
        sprintf(buf + strlen(buf), "Disease: '%s'\n\r",
                DiseaseInfo[affToDisease(*aff)].name);
        sprintf(buf + strlen(buf), 
                  "     Expires in %d updates.  Status = %d.\n\r",
            aff->duration , aff->level);
        break;
      case AFFECT_DUMMY:
        sprintf(buf + strlen(buf), "Dummy Affect: \n\r");
        sprintf(buf + strlen(buf), 
                  "     Expires in %d updates.  Status = %d.\n\r",
            aff->duration , aff->level);
        break;
      case AFFECT_WAS_INDOORS:
        sprintf(buf + strlen(buf), "Was indoors (immune to frostbite): \n\r");
        sprintf(buf + strlen(buf),
		"     Expires in %d updates.  Status = %d.\n\r",
		aff->duration , aff->level);
        break;
      case AFFECT_FREE_DEATHS:
        sprintf(buf + strlen(buf), "Free Deaths: \n\r");
        sprintf(buf + strlen(buf), 
                  "     Remaining %ld.  Status = %d.\n\r",
            aff->modifier, aff->level);
        break;
      case AFFECT_HORSEOWNED:
        sprintf(buf + strlen(buf), "Horse-owned: \n\r");
        sprintf(buf + strlen(buf),
                  "     Expires in %d updates.\n\r",
            aff->duration);
        break;
      case AFFECT_PLAYERKILL:
        sprintf(buf + strlen(buf), "Player-Killer: \n\r");
        sprintf(buf + strlen(buf),
                  "     Expires in %d updates.\n\r",
            aff->duration);
        break;
      case AFFECT_PLAYERLOOT:
        sprintf(buf + strlen(buf), "Player-Looter: \n\r");
        sprintf(buf + strlen(buf),
                  "     Expires in %d updates.\n\r",
                aff->duration);
        break;
      case AFFECT_TEST_FIGHT_MOB:
        sprintf(buf + strlen(buf), "Test Fight Mob: \n\r");
        sprintf(buf + strlen(buf), 
                  "     Remaining %ld.  Status = %d.\n\r",
            aff->modifier, aff->level);
        break;
      case AFFECT_SKILL_ATTEMPT:
        sprintf(buf + strlen(buf), "Skill Attempt: \n\r");
        sprintf(buf + strlen(buf),
                  "     Expires in %d updates.  Skill = %d.\n\r",
             aff->duration , (int) aff->bitvector); 
        break;
      case AFFECT_NEWBIE:
        sprintf(buf + strlen(buf), "Got Newbie Equipment: \n\r");
        sprintf(buf + strlen(buf),
                  "     Expires in %d updates.  Status = %d.\n\r",
            aff->duration , aff->level);
        break;
      case AFFECT_DRUNK:
        sprintf(buf + strlen(buf), "Drunken slumber: \n\r");
        sprintf(buf + strlen(buf), 
                  "     Expires in %d updates.  Status = %d.\n\r",
            aff->duration , aff->level);
        break;
      case AFFECT_DRUG:
        sprintf(buf + strlen(buf), "%s: \n\r", drugTypes[aff->modifier2].name);
        sprintf(buf + strlen(buf), "     Modifies %s by %ld points\n\r",
        	apply_types[aff->location].name, aff->modifier);
        sprintf(buf + strlen(buf),
                  "     Expires in %d updates.  Status = %d.\n\r",
        	aff->duration, aff->level);
        sprintf(buf + strlen(buf), "renew %i\n\r", aff->renew);
        break;
      case AFFECT_COMBAT:
        sprintf(buf + strlen(buf), "Combat: '%s'\n\r", 
            aff->be ? aff->be->getName() : "No aff->be!");
        sprintf(buf + strlen(buf), 
                  "     Expires in %d updates.  Status = %d.\n\r",
            aff->duration , aff->level);
        break;
      case AFFECT_PET:
        sprintf(buf + strlen(buf), "pet of: '%s'\n\r", ((char *) aff->be));
        sprintf(buf + strlen(buf), 
                  "     Expires in %d updates.  Status = %d.\n\r",
            aff->duration , aff->level);
        break;
      case AFFECT_CHARM:
        sprintf(buf + strlen(buf), "charm of: '%s'\n\r", ((char *) aff->be));
        sprintf(buf + strlen(buf), 
                  "     Expires in %d updates.  Status = %d.\n\r",
            aff->duration , aff->level);
        break;
      case AFFECT_THRALL:
        sprintf(buf + strlen(buf), "thrall of: '%s'\n\r", ((char *) aff->be));
        sprintf(buf + strlen(buf), 
                  "     Expires in %d updates.  Status = %d.\n\r",
            aff->duration , aff->level);
        break;
      case AFFECT_ORPHAN_PET:
        sprintf(buf + strlen(buf), "orphan pet of: '%s'\n\r", ((char *) aff->be));
        sprintf(buf + strlen(buf),
                  "     Expires in %d updates.  Status = %d.\n\r",
            aff->duration , aff->level);
        break;
      case AFFECT_TRANSFORMED_ARMS:
      case AFFECT_TRANSFORMED_HANDS:
      case AFFECT_TRANSFORMED_LEGS:
      case AFFECT_TRANSFORMED_HEAD:
      case AFFECT_TRANSFORMED_NECK:
        sprintf(buf + strlen(buf), "Spell : 'Transformed Limb'\n\r");
        sprintf(buf + strlen(buf), "     Modifies %s by %ld points\n\r",
                    apply_types[aff->location].name, aff->modifier);
        sprintf(buf + strlen(buf), "     Expires in %6d updates, Bits set ",
                    aff->duration);
        strcpy(buf2, sprintbit(aff->bitvector, affected_bits).c_str());
        strcat(buf2, "\n\r");
        strcat(buf, buf2);
        break;

      case AFFECT_GROWTH_POTION:
        sprintf(buf + strlen(buf), "Spell : 'Growth'\n\r");
        sprintf(buf + strlen(buf), "     Modifies %s by %ld points\n\r",
                apply_types[aff->location].name, aff->modifier);
        sprintf(buf + strlen(buf), "     Expires in %6d updates, Bits set ",
                aff->duration);
        strcpy(buf2, sprintbit(aff->bitvector, affected_bits).c_str());
        strcat(buf2, "\n\r");
        strcat(buf, buf2);
        break;
	
      case AFFECT_WARY:
	sprintf(buf + strlen(buf), "State: Wary\n\r");
	sprintf(buf + strlen(buf), "     Decreases chance of multiple cudgels\n\r");
	break;

      case AFFECT_DEFECTED:
	sprintf(buf + strlen(buf), "Player recently defected from a faction.\n\r");
	sprintf(buf + strlen(buf), "     Expires in %6d updates.\n\r", aff->duration);
	break;
	
      case AFFECT_OFFER:
 
	f = get_faction_by_ID(aff->modifier);
	if (!f) {
	  vlogf(LOG_FACT, "char had faction offer from non-existant faction in cmd_stat");
	  break;
	}
	sprintf(buf + strlen(buf), "Received offer to join %s (%d).\n\r",f->getName(), f->ID);
	sprintf(buf + strlen(buf), "     Expires in %6d updates.\n\r", aff->duration);
	break;
	
      case AFFECT_OBJECT_USED:
        objused = aff->modifier;

	sprintf(buf + strlen(buf), "Used magical object: %s\n\r", obj_index[objused].short_desc);
        sprintf(buf + strlen(buf), "     Expires in %6d updates.\n\r", aff->duration);
        break;


      case AFFECT_BITTEN_BY_VAMPIRE:
	sprintf(buf+strlen(buf), "Bitten by vampire.\n\r");
	sprintf(buf+strlen(buf), "Expires in %6d updates.\n\r", aff->duration);
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
      case TYPE_UNDEFINED:
      case TYPE_MAX_HIT:
      case ABSOLUTE_MAX_SKILL:
        vlogf(LOG_BUG, fmt("BOGUS AFFECT (%d) on %s") %  aff->type % k->getName());
        k->affectRemove(aff);
        break;
    }
  }
  if (k->task) {
    sprintf(buf + strlen(buf), "Player is busy '%s'.\n\r", tasks[k->task->task].name);
    sprintf(buf + strlen(buf), "Time left:    %6d updates     Orignal argument:  %s\n\r",
        k->task->timeLeft, k->task->orig_arg);
    sprintf(buf + strlen(buf), "Was in room:  %6d             Status/Flags:      %6d/%6d\n\r",
        k->task->wasInRoom, k->task->status, k->task->flags);
  }
  for (i = 1; i < MAX_TOG_INDEX;i++) {
    if (k->hasQuestBit(i)) 
      sprintf(buf + strlen(buf), "Toggle Set: (%d) %s\n\r", i, TogIndex[i].name.c_str());
  }
#if 0
  // spams too much, use "powers xxx" instead
  wizPowerT ipow;
  for (ipow = MIN_POWER_INDEX; ipow < MAX_POWER_INDEX;ipow++) {
    if (k->hasWizPower(ipow)) 
      sprintf(buf + strlen(buf), "Wiz-Power Set: (%d) %s\n\r", mapWizPowerToFile(ipow), getWizPowerName(ipow).c_str());
  }
#endif
  if (k->desc)
    sprintf(buf + strlen(buf), "Client : %s\n\r", k->desc->m_bIsClient ? "Yes" : "No");
  
  if (km) {
    if (km->sounds)
      sprintf(buf + strlen(buf), "Local Sound:\n\r%s", km->sounds);
    if (km->distantSnds)
      sprintf(buf + strlen(buf), "Distant Sound:\n\r%s", km->distantSnds);
  }
  desc->page_string(buf);
  return;
}

void TBeing::doStat(const char *)
{
  return;
}


void TPerson::doStat(const char *argument)
{
  char arg1[256], buf[256], buf2[256], skbuf[80], namebuf[80];
  const char *tmp_arg;
  TBeing *k = NULL;
  TObj *j = NULL;
  int count, parm = 0;
  int foundNum = FALSE;

  if (!hasWizPower(POWER_STAT)) {
    sendTo("Sorry, you lack the power to stat things.\n\r");
    return;
  }

  if (!isImmortal())
    return;

  if (!desc)
    return;

  tmp_arg = argument;
  tmp_arg = one_argument(tmp_arg, buf);
  tmp_arg = one_argument(tmp_arg, skbuf);
  tmp_arg = one_argument(tmp_arg, namebuf);
  strcpy(arg1, argument);

  if (!*arg1) {
    sendTo("Stats on who or what?\n\r");
    return;
  } else if (is_abbrev(skbuf, "skill")) {
    if (!hasWizPower(POWER_STAT_SKILL)) {
      sendTo("You can not type stat <argument> <skill>.\n\r");
      return;
    }
    if (!namebuf) {
      sendTo("Syntax: stat <char name> <skill> <value>\n\r");
      return;
    }
    count = 1;
    if (!(k = get_char_room(buf, in_room))) {
      if (!(k = get_pc_world(this, buf, EXACT_NO))) {
        if (!(k = get_char_vis_world(this, buf, &count, EXACT_YES))) {
          if (!(k = get_char_vis_world(this, buf, &count, EXACT_NO))) {
            sendTo("Syntax: stat <char name> <skill> <value>\n\r");
            return;
          }
        }
      }
    }

    spellNumT snt;
    if ((parm = convertTo<int>(namebuf))) {
      snt = spellNumT(parm);
    } else {
      foundNum = FALSE;
      for (snt = MIN_SPELL; snt < MAX_SKILL; snt++) {
        if (hideThisSpell(snt))
          continue;
        if (is_exact_name(namebuf, discArray[snt]->name)) {
          if (!(k->getSkill(snt)))
            continue;
          foundNum = TRUE;
          break;
        }
      }
      if (!foundNum) {
        for (snt = MIN_SPELL; snt < MAX_SKILL; snt++) {
          if (hideThisSpell(snt))
            continue;
          strcpy(buf2,discArray[snt]->name);
          // kludge since chivalry < chi  in discarray
          if (!strcmp(namebuf, "chi") && strcmp(buf2, "chi"))
            continue;
          // kludge since stealth < steal in discarray
          if (!strcmp(namebuf, "steal") && strcmp(buf2, "steal"))
            continue;
          // kludge since paralyze limb < paralyze in discarray
          if (!strcmp(namebuf, "paralyze") && strcmp(buf2, "paralyze"))
             continue;
          if (isname(namebuf, discArray[snt]->name)) {
            if (!(k->getSkill(snt))) 
              continue;
            break;
          }
        } 
      }
    }
    if ((snt < MIN_SPELL) || (snt >= MAX_SKILL)) {
      sendTo(fmt("Not a good skill number (%d) or the being doesnt have the skill!\n\r") % snt);
      sendTo("Syntax: stat <char name> <skill> <value>\n\r");
      return;
    }

    if (!k->doesKnowSkill(snt)) {
      if (discArray[snt])
        sendTo(COLOR_MOBS, fmt("%s doesnt appear to know that skill (%s).\n\r") % k->getName() % (discArray[snt]->name ? discArray[snt]->name : "unknown"));
      else
        sendTo(COLOR_MOBS, fmt("%s doesnt appear to know that skill.\n\r") % k->getName());
      return;
    }
    CSkill *sk = k->getSkill(snt);
    if (!sk) {
      if (discArray[snt])
        sendTo(COLOR_MOBS, fmt("%s doesnt appear to have that skill (%s).\n\r") % k->getName() % (discArray[snt]->name ? discArray[snt]->name : "unknown"));
      else
        sendTo(COLOR_MOBS, fmt("%s doesnt appear to know that skill.\n\r") % k->getName());
       return;
    }
    sendTo(COLOR_MOBS, fmt("%s's %s Raw (stored) Learning: Current (%d) Natural (%d).\n\r") % k->getName() % discArray[snt]->name % k->getRawSkillValue(snt) % k->getRawNatSkillValue(snt));
    sendTo(COLOR_MOBS, fmt("%s's %s Actual (used) Learning: Current (%d) Natural (%d) Max (%d).\n\r") % k->getName() % discArray[snt]->name % k->getSkillValue(snt) % k->getNatSkillValue(snt) % k->getMaxSkillValue(snt));

    time_t ct = sk->lastUsed;
    char * tmstr = (char *) asctime(localtime(&ct));
    *(tmstr + strlen(tmstr) - 1) = '\0';
    sendTo(COLOR_MOBS, fmt("%s's %s Last Increased: %s\n\r") % k->getName() % discArray[snt]->name % tmstr);

    return;
  } else if (is_abbrev(skbuf, "discipline")) {
    if (!hasWizPower(POWER_STAT_SKILL)) {
      sendTo("You can not type stat <argument> <discipline>.\n\r");
      return;
    }

    count = 1;

    if (!(k = get_char_room(buf, in_room))) {
      if (!(k = get_pc_world(this, buf, EXACT_NO))) {
        if (!(k = get_char_vis_world(this, buf, &count, EXACT_NO))) {
          sendTo("Syntax: stat <char name> <discipline> <value>\n\r");
          return;
        }
      }
    }

    CDiscipline *cd;
    
    if (!namebuf && !k->isPc() && !k->desc) {
      sendTo(COLOR_MOBS, fmt("%s has the following disciplines:\n\r\n\r") % k->getName());
      discNumT dnt;
      for (dnt = MIN_DISC; dnt < MAX_DISCS; dnt++) {
        if (!(cd = k->getDiscipline(dnt)))
          break;
        sendTo(COLOR_MOBS, fmt("Discpline %20.20s : Current (%d) Natural (%d).\n\r") % discNames[dnt].practice % cd->getLearnedness()  % cd->getNatLearnedness());
      }
      return;
    } else if (!namebuf) {
      sendTo("Syntax: stat <char name> <discipline> <value>\n\r");
      return;
    }

    discNumT dnt = mapFileToDisc(convertTo<int>(namebuf));
    if (dnt == DISC_NONE) {
      sendTo("Not a good discipline!\n\r");
      return;
    }

    if (!k->discs) {
      sendTo(COLOR_MOBS, fmt("%s does not have disciplines allocated yet!\n\r") % k->getName());
      return;
    }

    if (!(cd = k->getDiscipline(dnt))) {
       sendTo(COLOR_MOBS, fmt("%s doesnt appear to have that disipline.\n\r") % k->getName());
       return;
    }
    sendTo(COLOR_MOBS, fmt("%s's %s Used Learning: Current (%d) Natural (%d).\n\r") % k->getName() % discNames[dnt].practice % cd->getLearnedness() % cd->getNatLearnedness());
    return;
  } else if (is_abbrev(skbuf, "donebasic")) {
    if (!hasWizPower(POWER_STAT_SKILL)) {
      sendTo("You can not type stat <argument> <donebasic>.\n\r");
      return;
    }
    if (!namebuf) {
      sendTo("Syntax: stat <char name> <donebasic>\n\r");
      return;
    }
    count = 1;
    if (!(k = get_char_room(buf, in_room))) {
      if (!(k = get_pc_world(this, buf, EXACT_NO))) {
        if (!(k = get_char_vis_world(this, buf, &count, EXACT_NO))) {
          sendTo("Syntax: stat <char name> <donebasic>\n\r");
          return;
        }
      }
    }
    for (count = 0; count < MAX_CLASSES; count++) {
      sendTo(fmt("%-25.25s  :  %d\n\r") % classInfo[count].name.cap() % k->player.doneBasic[count]);
    }
    return;
  } else if (!strcmp("room", arg1)) {
    statRoom(roomp);
    return;
  } else if (!strcmp("zone", buf)) {
    statZone(skbuf);
    return;
  } else {
    count = 1;

    if (((j = get_obj_vis_accessible(this, arg1)) ||
          (j = get_obj_vis(this, arg1, &count, EXACT_NO))) &&
          ((k = get_char_room(arg1, in_room)) == NULL) &&
          ((k = get_pc_world(this, arg1, EXACT_NO)) == NULL)) {
      if (!hasWizPower(POWER_STAT_OBJECT)) {
        sendTo("Sorry, you lack the power to stat objects.\n\r");
        return;
      }
      statObj(j);
      return;
    }

    if ((k = get_char_room(arg1, in_room)) || 
        (k = get_char_vis_world(this, arg1, &count, EXACT_NO))) {
      if (!hasWizPower(POWER_STAT_MOBILES)) {
        sendTo("Sorry, you lack the power to stat mobiles.\n\r");
        return;
      }
      statBeing(k);
      return;
    }

    if (is_number(arg1)) {
      TObj         *tObj=NULL;
      TMonster     *tMonster=NULL;
      unsigned int  tValue;

      if (hasWizPower(POWER_STAT_OBJECT) &&
          ((tValue = real_object(convertTo<int>(arg1))) < obj_index.size()) &&
          tValue >= 0 && (tObj = read_object(tValue, REAL))) {
        statObj(tObj);
        delete tObj;
        tObj = NULL;

        return;
      }

      if (hasWizPower(POWER_STAT_MOBILES) &&
          ((tValue = real_mobile(convertTo<int>(arg1))) < mob_index.size()) &&
          tValue >= 0 && (tMonster = read_mobile(tValue, REAL))) {
        statBeing(tMonster);
        delete tMonster;
        tMonster = NULL;

        return;
      }
    }

    sendTo("No mobile or object by that name in The World.\n\r");
  }
}




