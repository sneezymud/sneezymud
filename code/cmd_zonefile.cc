/*****************************************************************************

  SneezyMUD++ - All rights reserved, SneezyMUD Coding Team.

  "cmd_zonefile.cc"
  Primary functions and routines used for builder online creation of zonefiles.

  Created 1/1/00 - Lapsos(William A. Perrotto III)

 *****************************************************************************/

#include "stdsneezy.h"

void doSaveZoneFile(TBeing *, const string &);

void TBeing::doZonefile(const string & tStArg)
{
  string tStString(""),
         tStBuffer("");

  if (!hasWizPower(POWER_ZONEFILE_UTILITY)) {
    sendTo("You have not been given this power yet.\n\r");
    return;
  }

  if (tStArg.empty()) {
    sendTo("Syntax:\n\r");
    sendTo("zonefile save : Save current zonefile status.\n\r");
#if 0
    sendTo("zonefile load : Load current zonefile status.\n\r");
    sendTo("zonefile modify <field> SEE HELP FILE\n\r");
#endif
  }

  two_arg(tStArg, tStString, tStBuffer);

  if (is_abbrev(tStString, "save")) {
    doSaveZoneFile(this, tStBuffer);
    return;
  }

#if 0
  if (is_abbrev(tStString, "load")) {
    doLoadZoneFile(this, tStBuffer);
    return;
  }
#endif
}

const char SAVEZONEFILE_REPLYTO[] = "mithros@spasm.stanford.edu";

void doSaveZoneFile(TBeing *ch, const string & tArg)
{
  unsigned int  zValue,
                roomIndex,
                roomStart;
  FILE         *tFile;
  char          tString[256],
                tBuffer[256];
  TRoom        *tRoom;
  string        tStString("");

  if (!ch->isImmortal() || !ch->desc || !ch->isPc())
    return;

  if ((zValue = ch->roomp->getZone()) > zone_table.size()) {
    vlogf(LOG_BUG, "Immortal in invalid zone [%s]", ch->getName());
    ch->sendTo("You are in an invalid zone, how did you get there?!?\n\r");
    return;
  }

  if (zone_table[zValue].enabled) {
    ch->sendTo("I'm sorry, this zone is enabled so you cannot do this for it.\n\r");
    return;
  }

  if (zone_table[zValue].top != ch->desc->blockastart &&
      zone_table[zValue].top != ch->desc->blockbstart &&
      !ch->hasWizPower(POWER_WIZARD)) {
    ch->sendTo("I'm sorry, this zone doesn't belong to you...\n\r");
    return;
  }

  sprintf(tString, "immortals/%s/zonefile", ch->getName());

  if (!(tFile = fopen(tString, "w"))) {
    ch->sendTo("Something went wrong, tell a coder what you did.\n\r");
    vlogf(LOG_FILE, "Unable to create file for zonefile writing.  [%s]", tString);
    return;
  }

  roomStart = (zValue <= 0 ? 0 : (zone_table[zValue - 1].top + 1));

  tStString += "Saving Header.\n\r";

  sprintf(tString, "#%d\n", roomStart);
  fputs(tString, tFile);
  sprintf(tString, "%s~\n", zone_table[zValue].name);
  fputs(tString, tFile);
  sprintf(tString, "%d %d %d %d\n", zone_table[zValue].top, 30, 2, 0);
  fputs(tString, tFile);

  tStString += "Saving Door Status:"; // 'D' Command

  for (roomIndex = roomStart; roomIndex < (unsigned) (zone_table[zValue].top + 1); roomIndex++) {
    if (!(tRoom = real_roomp(roomIndex)))
      continue;

    roomDirData *tExit;
    TRoom *tERoom;

    for (int exitIndex = 0; exitIndex < MAX_DIR; exitIndex++) {
      if (!(tExit = tRoom->dir_option[exitIndex]) ||
          tExit->door_type <= DOOR_NONE ||
          !(tExit->condition & EX_CLOSED))
        continue;

      sprintf(tString, "D 0 %d %d %d",
              roomIndex, exitIndex,
              ((tExit->condition & EX_LOCKED) ? 2 : 1));
      sprintf(tBuffer, "%-30s %s -> %s\n", tString,
              tRoom->getNameNOC(ch).c_str(),
              ((tERoom = real_roomp(tExit->to_room)) ?
               tERoom->getNameNOC(ch).c_str() : "Unknown"));
      fputs(tBuffer, tFile);
    }

    tStString += '.';
  }

  tStString += "\n\rSaving Object Status:"; // 'O' Command & 'P' Command

  for (roomIndex = roomStart; roomIndex < (unsigned) (zone_table[zValue].top + 1); roomIndex++) {
    if (!(tRoom = real_roomp(roomIndex)))
      continue;

    TThing *tThing;
    TObj   *tObj;

    for (tThing = tRoom->stuff; tThing; tThing = tThing->nextThing) {
      if (!(tObj = dynamic_cast<TObj *>(tThing)) ||
          (tObj->getSnum() < 0) ||
          tObj->canWear(ITEM_TAKE))
        continue;

      sprintf(tString, "O 0 %d %d %d",
              tObj->getSnum(), tObj->max_exist, roomIndex);
      sprintf(tBuffer, "%-30s %s\n", tString,
              (tObj->getNameNOC(ch).c_str() ? tObj->getNameNOC(ch).c_str() : "Unknown"));
      fputs(tBuffer, tFile);

      if (tObj->stuff) {
        TThing *sThing;
        TObj   *sObj;

        for (sThing = tObj->stuff; sThing; sThing = sThing->nextThing) {
          if (!(sObj = dynamic_cast<TObj *>(sThing)) ||
              sObj->getSnum() < 0)
            continue;

          sprintf(tString, "P 1 %d %d %d",
                  sObj->getSnum(), sObj->max_exist, tObj->getSnum());
          sprintf(tBuffer, "%-35s %s inside %s\n", tString,
                  (sObj->getNameNOC(ch).c_str() ? sObj->getNameNOC(ch).c_str() : "Unknown"),
                  (tObj->getNameNOC(ch).c_str() ? tObj->getNameNOC(ch).c_str() : "Unknown"));
          fputs(tBuffer, tFile);
        }
      }
    }

    tStString += '.';
  }

  tStString += "\n\rSaving Booty Status:"; // 'B' Command & 'P' Command

  for (roomIndex = roomStart; roomIndex < (unsigned) (zone_table[zValue].top + 1); roomIndex++) {
    if (!(tRoom = real_roomp(roomIndex)))
      continue;

    TThing *tThing;
    TObj   *tObj;

    for (tThing = tRoom->stuff; tThing; tThing = tThing->nextThing) {
      if (!(tObj = dynamic_cast<TObj *>(tThing)) ||
          (tObj->getSnum() < 0) ||
          !tObj->canWear(ITEM_TAKE))
        continue;

      sprintf(tString, "B 0 %d %d %d",
              tObj->getSnum(), tObj->max_exist, roomIndex);
      sprintf(tBuffer, "%-30s %s\n", tString,
              (tObj->getNameNOC(ch).c_str() ? tObj->getNameNOC(ch).c_str() : "Unknown"));
      fputs(tBuffer, tFile);

      if (tObj->stuff) {
        TThing *sThing;
        TObj   *sObj;

        for (sThing = tObj->stuff; sThing; sThing = sThing->nextThing) {
          if (!(sObj = dynamic_cast<TObj *>(sThing)) ||
              sObj->getSnum() < 0)
            continue;

          sprintf(tString, "P 1 %d %d %d",
                  sObj->getSnum(), sObj->max_exist, tObj->getSnum());
          sprintf(tBuffer, "%-35s %s inside %s\n", tString,
                  (sObj->getNameNOC(ch).c_str() ? sObj->getNameNOC(ch).c_str() : "Unknown"),
                  (tObj->getNameNOC(ch).c_str() ? tObj->getNameNOC(ch).c_str() : "Unknown"));
          fputs(tBuffer, tFile);
        }
      }
    }

    tStString += '.';
  }

  tStString += "\n\rSaving Mobile Status:"; // 'M' Command & 'G' Command & 'E' Command

  for (roomIndex = roomStart; roomIndex < (unsigned) (zone_table[zValue].top + 1); roomIndex++) {
    if (!(tRoom = real_roomp(roomIndex)))
      continue;

    TThing   *tThing;
    TMonster *tMob;

    for (tThing = tRoom->stuff; tThing; tThing = tThing->nextThing) {
      if (!(tMob = dynamic_cast<TMonster *>(tThing)) ||
          tMob->getSnum() < 0)
        continue;

      sprintf(tString, "M 0 %d %d %d",
              tMob->getSnum(), tMob->max_exist, roomIndex);
      sprintf(tBuffer, "%-30s %s\n", tString,
              (tMob->getNameNOC(ch).c_str() ? tMob->getNameNOC(ch).c_str() : "Unknown"));
      fputs(tBuffer, tFile);

      if (tMob->stuff) {
        TThing *sThing;
        TObj   *sObj;

        for (sThing = tMob->stuff; sThing; sThing = sThing->nextThing) {
          if (!(sObj = dynamic_cast<TObj *>(sThing)) ||
              sObj->getSnum() < 0)
            continue;

          sprintf(tString, "G 1 %d %d",
                  sObj->getSnum(), sObj->max_exist);
          sprintf(tBuffer, "%-32s %s given to %s\n", tString,
                  (sObj->getNameNOC(ch).c_str() ? sObj->getNameNOC(ch).c_str() : "Unknown"),
                  (tMob->getNameNOC(ch).c_str() ? tMob->getNameNOC(ch).c_str() : "Unknown"));
          fputs(tBuffer, tFile);
        }
      }

      TObj *sObj;

      for (wearSlotT wearIndex = MIN_WEAR; wearIndex < MAX_WEAR; wearIndex++) {
        if (!(sObj = dynamic_cast<TObj *>(tMob->equipment[wearIndex])) ||
            sObj->getSnum() < 0)
          continue;

        sprintf(tString, "E 1 %d %d %d",
                sObj->getSnum(), sObj->max_exist, wearIndex);
        sprintf(tBuffer, "%-34s %s equipped by %s [%s]\n", tString,
                (sObj->getNameNOC(ch).c_str() ? sObj->getNameNOC(ch).c_str() : "Unknown"),
                (tMob->getNameNOC(ch).c_str() ? tMob->getNameNOC(ch).c_str() : "Unknown"),
                bodyParts[wearIndex]);
        fputs(tBuffer, tFile);
      }
    }
    tStString += '.';
  }

  tStString += '\n';
  fputs("S\n", tFile);
  fclose(tFile);

  sprintf(tString, "/usr/lib/sendmail -f%s %s < immortals/%s/zonefile",
          SAVEZONEFILE_REPLYTO,
          ch->desc->account->email, ch->getNameNOC(ch).c_str());
  vsystem(tString);
  sprintf(tString, "cp -f immortals/%s/zonefile tmp/%s.output",
          ch->getNameNOC(ch).c_str(), ch->getNameNOC(ch).c_str());
  vsystem(tString);
  tStString += "A copy of your zonefile is in your output.  Use 'viewoutput' to see it.\n\r";
  ch->sendTo(tStString.c_str());
}
