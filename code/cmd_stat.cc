//////////////////////////////////////////////////////////////////////////
//
// SneezyMUD - All rights reserved, SneezyMUD Coding Team
//
// $Log: cmd_stat.cc,v $
// Revision 1.1  1999/09/12 17:24:04  sneezy
// Initial revision
//
//
//////////////////////////////////////////////////////////////////////////


////////////////////////////////////////////////////////////////////////// 
//
//      SneezyMUD++ - All rights reserved, SneezyMUD Coding Team
//
//      "stat_command.cc" - The stat command
//  
//////////////////////////////////////////////////////////////////////////

#include "stdsneezy.h"
#include "disease.h"

void TBeing::statZone(const char *zoneNumber)
{
  int zNum,
    cnDesc[2]  = {0, 0},
    cnTitle[2] = {0, 0},
    cnExtra[2] = {0, 0},
    cnFlags[7] = {0, 0, 0, 0, 0, 0, 0},
    rzCount    = 0;
  string sb("");
  unsigned long int rNums[2] = {0, 0},
                    Runner = 0;
  char tString[256];
  TRoom *curRoomCntr;

  if (!zoneNumber || !*zoneNumber) {
    if (!roomp) {
      vlogf(7, "statZone called by being with no current room.");
      return;
    }

    zNum = roomp->getZone();
  } else
    zNum = atoi(zoneNumber);

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

      if (curRoomCntr->getDescr())// Count Descriptions
        cnDesc[0]++;
      else
        cnDesc[1]++;

      if (curRoomCntr->name)// Count Titles
        cnTitle[0]++;
      else
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
  sprintf(tString, "DescrCount: %3d     NoDescr: %3d\n\r",
          cnDesc[0], cnDesc[1]);
  sb += tString;
  sprintf(tString, "TitleCount: %3d     NoTitle: %3d\n\r",
          cnTitle[0], cnTitle[1]);
  sb += tString;
  sprintf(tString, "ExtraCount: %3d     NoExtra: %3d     (Room Counts)\n\r",
          cnExtra[0], cnExtra[1]);
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

  desc->page_string(sb.c_str(), 0, TRUE);
}

void TBeing::statRoom(TRoom *rmp)
{
  string str;
  char buf2[256];
  char buf3[80], buf4[80];
  extraDescription *e;
  TThing *t;
  int counter = 0;

  sprintf(buf2,"Room name: %s, Of zone : %d. V-Number : %d, R-number : %d\n\r",
        rmp->name, rmp->getZone(), rmp->number, in_room);
  str = buf2;

  sprintf(buf2,"Sector type : %s ", TerrainInfo[rmp->getSectorType()]->name);
  str += buf2;

  str += "Special procedure : ";

  sprintf(buf2, "%s", (rmp->funct) ? "Exists\n\r" : "No\n\r");
  str += buf2;

  str += "Room flags: ";

  sprintbit((long) rmp->getRoomFlags(), room_bits, buf2);
  str += buf2;
  str += "\n\r";

  str += "Description:\n\r";
  if (rmp->getDescr())
    str += rmp->getDescr();
  else
    str += "NO DESCRIPTION\n\r";

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
  sprintf(buf2,"Light : %d   Room Height : %s    Maximum capacity : %s\n\r", 
                    rmp->getLight(),
                    ((rmp->getRoomHeight() <= 0) ? "unlimited" : buf3),
                    (rmp->getMoblim()) ? buf4 : "Infinite");
  str += buf2;

  if (rmp->isWaterSector() || rmp->isUnderwaterSector()) {
    sprintf(buf2, "River direction : %s", ((rmp->getRiverDir() < 0) ? 
              "None" : dirs[rmp->getRiverDir()]));
    str += buf2;
    if (rmp->getRiverSpeed() >= 1)
      sprintf(buf2,"   River speed : Every %d heartbeat%s\n\r",
               rmp->getRiverSpeed(), ((rmp->getRiverSpeed() != 1) ? "s." : "."));
    else
      sprintf(buf2,"   River speed : no current.\n\r");
    str += buf2;
  }
  if ((rmp->getTeleTarg() > 0) && (rmp->getTeleTime() > 0)) {
    sprintf(buf2,"Teleport speed : Every %d heartbeats. To room : %d. Look? %s.\n\r",
       rmp->getTeleTime(), rmp->getTeleTarg(), 
           (rmp->getTeleLook() ? "yes" : "no"));
    str += buf2;
  }
  str += "------- Chars present -------\n\r";
  counter = 0;
  for (t = rmp->stuff; t; t = t->nextThing) {
    // canSee prevents seeing invis gods of higher level
    if (dynamic_cast<TBeing *>(t) && canSee(t)) {
      counter++;
      if (counter > 15) {
         str += "Too Many In Room to Stat More\n\r";
      } else {
        sprintf(buf2, "%s%s   (%s)\n\r", 
           t->getName(), (dynamic_cast<TPerson *>(t) ? "(PC)" : "(NPC)"), t->name);
        str += buf2;
      }
    }
  }
  str += "--------- Contents ---------\n\r";
  counter = 0;
  for (t = rmp->stuff; t; t = t->nextThing) {
    if (!dynamic_cast<TBeing *>(t)) {
      counter++;
      if (counter > 20) {
        str += "Too Many In Room to Stat More\n\r";
      } else {
        sprintf(buf2, "%s   (%s)\n\r", t->getName(), t->name);
        str += buf2;
      }
    }
  }
  str += "------- Exits defined -------\n\r";
  dirTypeT dir;
  for (dir = MIN_DIR; dir < MAX_DIR; dir++) {
    if (!rmp->dir_option[dir])
      continue;
    else {
      sprintf(buf2,"Direction : %-10s    Door Type : %-12s     To-Room : %d\n\r",
         dirs[dir],door_types[rmp->dir_option[dir]->door_type],
         rmp->dir_option[dir]->to_room);
      str += buf2;
      if (rmp->dir_option[dir]->door_type != DOOR_NONE) {
        sprintbit(rmp->dir_option[dir]->condition, exit_bits, buf3);
        sprintf(buf2, "Weight : %d      Exit Flags : %s\n\rKeywords : %s\n\r",
              rmp->dir_option[dir]->weight, buf3, rmp->dir_option[dir]->keyword);
        str += buf2;
        if ((rmp->dir_option[dir]->key > 0) || 
             (rmp->dir_option[dir]->lock_difficulty >= 0)) {
          sprintf(buf2,"Key Number : %d     Lock Difficulty: %d\n\r",
               rmp->dir_option[dir]->key, rmp->dir_option[dir]->lock_difficulty);
          str += buf2;
        }
        if (IS_SET(rmp->dir_option[dir]->condition, EX_TRAPPED)) {
          sprinttype(rmp->dir_option[dir]->trap_info, trap_types, buf3);
          sprintf(buf2, "Trap type : %s,  Trap damage : %d (d8)\n\r", buf3, rmp->dir_option[dir]->trap_dam);
          str += buf2;
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
  desc->page_string(str.c_str(), 0);
  return;
}

void TBeing::statObj(const TObj *j)
{
  char buf[256];
  extraDescription *e;
  TThing *t;
  int i;
  string str;
  
  sprintf(buf, "Object name: [%s], R-number: [%d], V-number: [%d] Item type: ",
       j->name, j->number, obj_index[j->getItemIndex()].virt);
  str = buf;

  str += ItemInfo[j->itemType()]->name;
  str += "\n\r";
  sprintf(buf, "Short description: %s\n\rLong description:\n\r%s\n\r",
        ((j->shortDescr) ? j->shortDescr : "None"),
        ((j->getDescr()) ? j->getDescr() : "None"));
  str += buf;

  if (j->action_description) {
    sprintf(buf, "Action Description: %s\n\r", j->action_description);
    str += buf;
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
  sprintbit(j->obj_flags.wear_flags, wear_bits, buf);
  str += buf;
  str += "\n\r";

  str += "Set char bits  :";
  sprintbit(j->obj_flags.bitvector, affected_bits, buf);
  str += buf;
  str += "\n\r";

  str += "Extra flags: ";
  sprintbit(j->getObjStat(), extra_bits, buf);
  str += buf;
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

  if (!j->stuff)
    str += "Contains : Nothing\n\r";
  else {
    str += "Contains :\n\r";
    for (t = j->stuff; t; t = t->nextThing) {
      str += fname(t->name);
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
        vlogf(10, "BOGUS AFFECT (%d) on %s", j->affected[i].modifier, 
              j->getName());
    } else if (j->affected[i].location == APPLY_DISCIPLINE) {
     if (discNames[j->affected[i].modifier].disc_num) {
        sprintf(buf, "   Affects:  %s: %s by %ld\n\r",
            apply_types[j->affected[i].location].name,
            discNames[j->affected[i].modifier].practice,
            j->affected[i].modifier2);
        str += buf;
      } else
        vlogf(10, "BOGUS AFFECT (%d) on %s", j->affected[i].modifier,
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
  for (i = 0; i < MAX_SWING_AFFECT; i++) {
    if (j->oneSwing[i].type != TYPE_UNDEFINED) {
      sprintf(buf, "   One-Swing Affect: %s\n\r",
           affected_bits[j->oneSwing[i].bitvector]);
      str += buf;
      sprintf(buf, "        Effects: %s by %ld\n\r",
           apply_types[j->oneSwing[i].location].name, 
           j->oneSwing[i].modifier);
      str += buf;
    }
  }
  desc->page_string(str.c_str(), 0);
  return;
}

void TBeing::statBeing(TBeing *k)
{
  affectedData *aff, *af2;
  char buf[MAX_STRING_LENGTH];
  char buf2[256];
  char buf3[256];
  TBeing *x1;
  char *birth, *logon;
  char birth_buf[40], logon_buf[40];
  resp *respy;
  followData *fol;
  charList *list;
  struct time_info_data playing_time;
  int i;

  
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
  sprintf(buf + strlen(buf)," %s - Name : %s [R-Number%d], In room [%d]\n\r",
         (dynamic_cast<const TPerson *>(k) ? "PC" : "NPC"), k->name, k->number, k->in_room);
  sprintf(buf + strlen(buf),"-----------------------------------------------------------------------------\n\r");
  const TMonster *km = dynamic_cast<const TMonster *>(k);
  if (km) {
    sprintf(buf + strlen(buf),"V-Number [%d]\n\r", mob_index[km->getMobIndex()].virt);
    sprintf(buf + strlen(buf),"Short description: %s\n\r",
	   (km->shortDescr ? km->shortDescr : "None"));
    sprintf(buf + strlen(buf),"Long description: %s",
    	   (km->player.longDescr ? km->player.longDescr : "None"));

    if (km->resps && km->resps->respList) {
      sprintf(buf + strlen(buf),"Response(s):\n\r----------\n\r");
      for (respy = km->resps->respList; respy; respy = respy->next) {
        if (respy->cmd < MAX_CMD_LIST) {
          sprintf(buf + strlen(buf),"%s %s\n\r", commandArray[respy->cmd]->name, respy->args);
        } else if (respy->cmd == CMD_RESP_ROOM_ENTER) {
          sprintf(buf + strlen(buf),"roomenter\n\r");
        } else if (respy->cmd == CMD_RESP_PACKAGE) {
          sprintf(buf + strlen(buf),"dummy %s\n\r", respy->args);
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
      sendTo(buf + strlen(buf),"Response(s): None.\n\r");
  }
  *buf2 = '\0';
  for (classIndT ijc = MIN_CLASS_IND; ijc < MAX_CLASSES; ijc++)
    if (k->hasClass(1<<ijc))
      sprintf(buf2 + strlen(buf2), "%s ", classNames[ijc].capName);

  sprintf(buf + strlen(buf),"<c>Class :<z> %-28s", buf2);
  sprintf(buf + strlen(buf),"<c>Level   :<z> [M%d C%d W%d T%d A%d D%d K%d R%d]\n\r",
         k->getLevel(MAGE_LEVEL_IND), k->getLevel(CLERIC_LEVEL_IND),
         k->getLevel(WARRIOR_LEVEL_IND), k->getLevel(THIEF_LEVEL_IND),
         k->getLevel(SHAMAN_LEVEL_IND), k->getLevel(DEIKHAN_LEVEL_IND),
         k->getLevel(MONK_LEVEL_IND), k->getLevel(RANGER_LEVEL_IND));

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
    sprintf(buf + strlen(buf), "%sPlaying time :%s %d days, %d hours.     <c>Base age:<z> %d\n\r",
        cyan(), norm(), playing_time.day, playing_time.hours,
                k->getBaseAge());
    if (!k->desc)
      sprintf(buf + strlen(buf), "%sWARNING%s, player is offline, age will not be accurate.\n\r", red(), norm());

    sprintf(buf + strlen(buf), "%sPlayer age   :%s %d years, %d months, %d days, %d hours\n\r\n\r",
        cyan(), norm(),
        k->age()->year, k->age()->month, k->age()->day, k->age()->hours);
  }
  sprintf(buf3, "[%.2f]", k->getExp());
  sprintf(buf + strlen(buf), "%sDefRnd:%s [%3d]  %sExp    :%s %-10s  %sBLANK   :%s [%d]\n\r",
          cyan(), norm(), k->defendRound(NULL),  cyan(), norm(), buf3, cyan(), norm(), 0);
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
    sprintf(buf + strlen(buf), "%sPiety :%s [%4.1f]%sHit    :%s %-10s  %sMove    :%s %-10s\n\r",
      cyan(), norm(), k->getPiety(),
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
//  sprintf(buf + strlen(buf), "%sFaction :%s %s\n\r",
//    cyan(), norm(), FactionInfo[k->getFaction()].faction_name);

  sprintf(buf + strlen(buf),"Stats  :");
  sprintf(buf + strlen(buf),k->chosenStats.printStatHeader().c_str());
  sprintf(buf + strlen(buf),"Chosen:");
  sprintf(buf + strlen(buf),k->chosenStats.printRawStats(this).c_str());
  sprintf(buf + strlen(buf),"Natural:");
  statTypeT ik;
  for(ik=MIN_STAT; ik<MAX_STATS_USED; ik++) {
    sprintf(buf + strlen(buf), " %3d ", k->getStat(STAT_NATURAL, ik));
  }
  strcat(buf, "\n\r");

  sprintf(buf + strlen(buf),"Current:");
  sprintf(buf + strlen(buf),k->curStats.printRawStats(this).c_str());

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
  if (km) {
    strcat(buf, "NPC flags: ");
    if (km->specials.act) {
      sprintbit(km->specials.act, action_bits, buf2);
      strcat(buf, buf2);
      strcat(buf, "\n\r");
    } else {
      strcat(buf, "None\n\r");
    }
    sprinttype(km->getPosition(), position_types, buf2);
    sprinttype((km->default_pos), position_types, buf3);
    sprintf(buf + strlen(buf), "%sPosition:%s %s, %sFighting:%s %s, %sDefault Position :%s %s\n\r",
          cyan(), norm(), buf2, cyan(), norm(),
          (km->fight() ? km->fight()->getName() : "Nobody"),
          cyan(), norm(), buf3);
  } else {
    sprinttype(k->getPosition(), position_types, buf2);
    sprintf(buf + strlen(buf), "%sPosition:%s %s, %sFighting:%s %s\n\r",
          cyan(), norm(), buf2, cyan(), norm(),
          (k->fight() ? k->fight()->getName() : "Nobody"));
  } 
  if (k->desc) {
    strcat(buf, "\n\rFlags (Specials Act): ");
    sprintbit(k->desc->plr_act, player_bits, buf2);
  }
  strcat(buf, buf2);
  strcat(buf, "\n\r");

  if (km) {
    sprintf(buf + strlen(buf), "Number of attacks : %.1f", km->getMult());
    sprintf(buf + strlen(buf), "        NPC Damage: %.1f+%d%%.\n\r",
        km->getDamLevel(), km->getDamPrecision());
    double bd = km->baseDamage();
    int chg = (int) (bd * km->getDamPrecision() / 100);
    sprintf(buf + strlen(buf), "  NPC Damage range: %d-%d.\n\r",
        max(1, (int) bd-chg), max(1, (int) bd+chg));
  } else if (k->hasClass(CLASS_MONK)) {
    sprintf(buf + strlen(buf), "Number of attacks : %.2f\n\r", k->getMult());
  } else {
    float fx, fy;
    k->blowCount(false, fx, fy);
    sprintf(buf + strlen(buf), "Prim attacks: %.2f, Off attacks: %.2f\n\r",
          fx, fy);
  }

  sprintf(buf + strlen(buf), "Carried weight: %.1f   Carried volume: %d\n\r",
          k->getCarriedWeight(), k->getCarriedVolume());

  sprintf(buf + strlen(buf), "Master is '%s'",
          ((k->master) ? k->master->getName() : "NOBODY"));
  strcat(buf, "           Followers are:");
  for (fol = k->followers; fol; fol = fol->next)
    strcat(buf, fol->follower->getName());
  strcat(buf,"\n\r");
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

  sprintbit(k->specials.affectedBy, affected_bits, buf2);
  strcat(buf, "Affected by: ");
  strcat(buf2, "\n\r");
  strcat(buf, buf2);

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

  strcat(buf, "\n\rAffecting Spells:\n\r--------------\n\r");
  for (aff = k->affected; aff; aff = af2) {
    // technically, shouldn't need to save next, but apparently
    // some operations below "might" cause aff to be deleted
    // not sure which ones though - Bat 4/28/98
    af2 = aff->next;
    if (aff->type == AFFECT_DISEASE) {
      sprintf(buf + strlen(buf), "Disease: '%s'\n\r",
                DiseaseInfo[DISEASE_INDEX(aff->modifier)].name);
      sprintf(buf + strlen(buf), 
                "     Expires in %d updates.  Status = %d.\n\r",
          aff->duration , aff->level);
    } else if (aff->type == AFFECT_PET) {
      sprintf(buf + strlen(buf), "pet of: '%s'\n\r", ((char *) aff->be));
      sprintf(buf + strlen(buf), 
                "     Expires in %d updates.  Status = %d.\n\r",
          aff->duration , aff->level);
    } else if (aff->type == AFFECT_ORPHAN_PET) {
      sprintf(buf + strlen(buf), "orphan pet of: '%s'\n\r", ((char *) aff->be));
      sprintf(buf + strlen(buf),
                "     Expires in %d updates.  Status = %d.\n\r",
          aff->duration , aff->level);

    } else if (aff->type == AFFECT_FREE_DEATHS) {
      sprintf(buf + strlen(buf), "Free Deaths: \n\r");
      sprintf(buf + strlen(buf), 
                "     Remaining %ld.  Status = %d.\n\r",
          aff->modifier, aff->level);
    } else if (aff->type == AFFECT_TEST_FIGHT_MOB) {
      sprintf(buf + strlen(buf), "Test Fight Mob: \n\r");
      sprintf(buf + strlen(buf), 
                "     Remaining %ld.  Status = %d.\n\r",
          aff->modifier, aff->level);
    } else if (aff->type == AFFECT_DUMMY) {
      sprintf(buf + strlen(buf), "Dummy Affect: \n\r");
      sprintf(buf + strlen(buf), 
                "     Expires in %d updates.  Status = %d.\n\r",
          aff->duration , aff->level);
    } else if (aff->type == AFFECT_SKILL_ATTEMPT) {
      sprintf(buf + strlen(buf), "Skill Attempt: \n\r");
      sprintf(buf + strlen(buf),
                "     Expires in %d updates.  Skill = %d.\n\r",
           aff->duration , (int) aff->bitvector); 
    } else if (aff->type == AFFECT_NEWBIE) {
      sprintf(buf + strlen(buf), "Got Newbie Equipment: \n\r");
      sprintf(buf + strlen(buf),
                "     Expires in %d updates.  Status = %d.\n\r",
          aff->duration , aff->level);
    } else if (aff->type == AFFECT_PLAYERKILL) {
      sprintf(buf + strlen(buf), "Player-Killer: \n\r");
      sprintf(buf + strlen(buf),
                "     Expires in %d updates.\n\r",
          aff->duration);
    } else if (aff->type == AFFECT_DRUNK) {
      sprintf(buf + strlen(buf), "Drunken slumber: \n\r");
      sprintf(buf + strlen(buf), 
                "     Expires in %d updates.  Status = %d.\n\r",
          aff->duration , aff->level);
    } else if (aff->type == AFFECT_DRUG) {
      sprintf(buf + strlen(buf), "%s: \n\r", drugTypes[aff->modifier2].name);
      sprintf(buf + strlen(buf), "     Modifies %s by %ld points\n\r",
      	apply_types[aff->location].name, aff->modifier);
      sprintf(buf + strlen(buf),
                "     Expires in %d updates.  Status = %d.\n\r",
      	aff->duration, aff->level);
      sprintf(buf + strlen(buf), "renew %i\n\r", aff->renew);
    } else if (aff->type == AFFECT_COMBAT) {
      sprintf(buf + strlen(buf), "Combat: '%s'\n\r", aff->be->getName());
      sprintf(buf + strlen(buf), 
                "     Expires in %d updates.  Status = %d.\n\r",
          aff->duration , aff->level);
    } else if (aff->type == AFFECT_TRANSFORMED_ARMS ||
                   aff->type == AFFECT_TRANSFORMED_HANDS ||
                   aff->type == AFFECT_TRANSFORMED_LEGS ||
                   aff->type == AFFECT_TRANSFORMED_HEAD ||
                   aff->type == AFFECT_TRANSFORMED_NECK) {
      sprintf(buf + strlen(buf), "Spell : 'Transformed Limb'\n\r");
      sprintf(buf + strlen(buf), "     Modifies %s by %ld points\n\r",
                  apply_types[aff->location].name, aff->modifier);
      sprintf(buf + strlen(buf), "     Expires in %6d updates, Bits set ",
                  aff->duration);
      sprintbit(aff->bitvector, affected_bits, buf2);
      strcat(buf2, "\n\r");
      strcat(buf, buf2);
    } else if (aff->type >= MIN_SPELL && aff->type < MAX_SKILL) {
      if (discArray[aff->type]) {
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
        sprintbit(aff->bitvector, affected_bits, buf2);
        strcat(buf2, "\n\r");
        strcat(buf, buf2);

      } else {
        vlogf(10, "BOGUS AFFECT (%d) on %s", aff->type, k->getName());
        k->affectRemove(aff);
      }
    } else {
      vlogf(10, "BOGUS AFFECT (%d) on %s", aff->type, k->getName());
      k->affectRemove(aff);
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
      sprintf(buf + strlen(buf), "Toggle Set: (%d) %s\n\r", i, TogIndex[i].name);
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
    sprintf(buf + strlen(buf), "Client : %s\n\r", k->desc->client ? "Yes" : "No");
  
  if (km) {
    if (km->sounds)
      sprintf(buf + strlen(buf), "Local Sound:\n\r%s", km->sounds);
    if (km->distantSnds)
      sprintf(buf + strlen(buf), "Distant Sound:\n\r%s", km->distantSnds);
  }
  desc->page_string(buf, 0);
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
  only_argument(argument, arg1);

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
    if ((parm = atoi(namebuf))) {
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
      sendTo("Not a good skill number (%d) or the being doesnt have the skill!\n\r", snt);
      sendTo("Syntax: stat <char name> <skill> <value>\n\r");
      return;
    }

    if (!k->doesKnowSkill(snt)) {
      if (discArray[snt])
        sendTo(COLOR_MOBS, "%s doesnt appear to know that skill (%s).\n\r", k->getName(), (discArray[snt]->name ? discArray[snt]->name : "unknown"));
      else
        sendTo(COLOR_MOBS, "%s doesnt appear to know that skill.\n\r", k->getName());
      return;
    }
    CSkill *sk = k->getSkill(snt);
    if (!sk) {
      if (discArray[snt])
        sendTo(COLOR_MOBS, "%s doesnt appear to have that skill (%s).\n\r", k->getName(), (discArray[snt]->name ? discArray[snt]->name : "unknown"));
      else
        sendTo(COLOR_MOBS, "%s doesnt appear to know that skill.\n\r", k->getName());
       return;
    }
    sendTo(COLOR_MOBS, "%s's %s Raw (stored) Learning: Current (%d) Natural (%d).\n\r", k->getName(), discArray[snt]->name, k->getRawSkillValue(snt), k->getRawNatSkillValue(snt));
    sendTo(COLOR_MOBS, "%s's %s Actual (used) Learning: Current (%d) Natural (%d) Max (%d).\n\r", k->getName(), discArray[snt]->name, k->getSkillValue(snt), k->getNatSkillValue(snt), k->getMaxSkillValue(snt));

    time_t ct = sk->lastUsed;
    char * tmstr = (char *) asctime(localtime(&ct));
    *(tmstr + strlen(tmstr) - 1) = '\0';
    sendTo(COLOR_MOBS, "%s's %s Last Increased: %s\n\r", k->getName(), discArray[snt]->name, tmstr);

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
      sendTo(COLOR_MOBS, "%s has the following disciplines:\n\r\n\r", k->getName());
      discNumT dnt;
      for (dnt = MIN_DISC; dnt < MAX_DISCS; dnt++) {
        if (!(cd = k->getDiscipline(dnt)))
          break;
        sendTo(COLOR_MOBS, "Discpline %20.20s : Current (%d) Natural (%d).\n\r", discNames[dnt].practice, cd->getLearnedness() , cd->getNatLearnedness());
      }
      return;
    } else if (!namebuf) {
      sendTo("Syntax: stat <char name> <discipline> <value>\n\r");
      return;
    }

    discNumT dnt = mapFileToDisc(atoi(namebuf));
    if (dnt == DISC_NONE) {
      sendTo("Not a good discipline!\n\r");
      return;
    }

    if (!k->discs) {
      sendTo(COLOR_MOBS, "%s does not have disciplines allocated yet!\n\r", k->getName());
      return;
    }

    if (!(cd = k->getDiscipline(dnt))) {
       sendTo(COLOR_MOBS, "%s doesnt appear to have that disipline.\n\r", k->getName());
       return;
    }
    sendTo(COLOR_MOBS, "%s's %s Used Learning: Current (%d) Natural (%d).\n\r", k->getName(), discNames[dnt].practice, cd->getLearnedness(), cd->getNatLearnedness());
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
      sendTo("%-25.25s  :  %d\n\r", classNames[count].capName, k->player.doneBasic[count]);
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
    } else
      sendTo("No mobile or object by that name in The World.\n\r");
  }
}
