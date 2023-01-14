/*****************************************************************************

  SneezyMUD++ - All rights reserved, SneezyMUD Coding Team.

  "cmd_zonefile.cc"
  Primary functions and routines used for builder online creation of zonefiles.

  Created 1/1/00 - Lapsos(William A. Perrotto III)

 *****************************************************************************/

#include <stdio.h>
#include <fstream>
#include <boost/filesystem.hpp>

#include "extern.h"
#include "room.h"
#include "being.h"
#include "monster.h"
#include "db.h"
#include "database.h"

// #define MAIL_ZONEFILE

namespace {
  namespace fs = boost::filesystem;
#ifdef MAIL_ZONEFILE
  const char SAVEZONEFILE_REPLYTO[] = "damescena@sneezy.saw.net";
#endif

  int findBeginningOfLastBlock(TBeing& ch) {
    int lastBlockVnum = 0;
    fs::directory_iterator end_itr;  // default construction yields past-the-end
    for (fs::directory_iterator itr("zonefiles"); itr != end_itr; ++itr) {
      const auto& dirent = *itr;
      int current = 0;
      try {
        current = std::stoi(dirent.path().filename().string());
      } catch (const std::invalid_argument& e) {
        vlogf(LOG_BUG, format("Nonnumeric junk in zonefiles dir: %s") %
                         dirent.path().filename().string());
        ch.sendTo(format("Nonnumeric junk in zonefiles dir: %s\n\r") %
                  dirent.path().filename().string());
        return 0;
      }
      if (current > lastBlockVnum)
        lastBlockVnum = current;
    }
    return lastBlockVnum;
  }

  bool shrinkZonefile(TBeing& ch, int begin, int size) {
    std::string path = (format("zonefiles/%d") % begin).str();
    int newBegin = begin + size;

    // read entire file into string
    std::string contents;
    if (std::ifstream is{path, std::ios::binary | std::ios::ate}) {
      auto size = is.tellg();
      contents = std::string(size, '\0');  // construct string to stream size
      is.seekg(0);
      is.read(&contents[0], size);
    }

    if (std::fstream file{path, std::ios::out}) {
      file << "#" << std::to_string(newBegin) << std::endl;
      file << contents.substr(contents.find('\n') + 1);
    }
    fs::rename(fs::path((format("zonefiles/%d") % begin).str()),
      fs::path((format("zonefiles/%d") % newBegin).str()));
    return true;
  }

  void createNewZonefile(int begin, int size, const std::string& name) {
    std::string path = (format("zonefiles/%d") % begin).str();
    std::fstream file(path, std::ios::out);
    file << "#" << begin << std::endl;
    file << name << "~" << std::endl;
    file << begin + size - 1 << " 35 2 0" << std::endl;
    file << "S" << std::endl;
  }

  void doNewZoneFile(TBeing& ch, const sstring& args) {
    if (!ch.hasWizPower(POWER_LOW)) {
      ch.sendTo("You need LOW powers to create new zones.\n\r");
      return;
    }

    int size = 0;
    try {
      size = std::stoi(args.word(0));
    } catch (const std::invalid_argument&) {
      ch.sendTo("Provide the number of rooms in the new zone");
      return;
    }

    std::string name = args.substr(args.find(' ') + 1);
    if (name.empty()) {
      ch.sendTo("Provide a name for the new zone");
      return;
    }

    int lastBlockBegin = findBeginningOfLastBlock(ch);
    if (lastBlockBegin == 0)
      return;

    // I hope we won't crash midway
    if (shrinkZonefile(ch, lastBlockBegin, size))
      createNewZonefile(lastBlockBegin, size, name);

    // temporarily remove last block from the list so that the ordering would be
    // correct
    zoneData lastBlock = zone_table.back();
    zone_table.pop_back();
    int seq = zone_table.size();
    TDatabase db(DB_SNEEZY);
    bootOneZone(db, lastBlockBegin, seq);
    zone_table.push_back(lastBlock);

    ch.sendTo(format("Success! new zone: %s with vnums from %d to %d") % name %
              lastBlockBegin % (lastBlockBegin + size - 1));
  }

  void doSaveZoneFile(TBeing* ch, const sstring& tArg) {
    unsigned int zValue, roomIndex, roomStart;
    FILE* tFile;
    char tString[256], tBuffer[512];
    TRoom* tRoom;
    sstring tStString("");

    if (!ch->isImmortal() || !ch->desc || !ch->isPc())
      return;

    if ((zValue = ch->roomp->getZoneNum()) > zone_table.size()) {
      vlogf(LOG_BUG, format("Immortal in invalid zone [%s]") % ch->getName());
      ch->sendTo("You are in an invalid zone, how did you get there?!?\n\r");
      return;
    }

    if (zone_table[zValue].enabled && !ch->hasWizPower(POWER_WIZARD)) {
      ch->sendTo(
        "I'm sorry, this zone is enabled so you cannot do this for it.\n\r");
      return;
    }

    if (((zValue > 0) ? (zone_table[zValue - 1].top + 1) : 0) !=
          ch->desc->blockastart &&
        ((zValue > 0) ? (zone_table[zValue - 1].top + 1) : 0) !=
          ch->desc->blockbstart &&
        !ch->hasWizPower(POWER_WIZARD)) {
      ch->sendTo("I'm sorry, this zone doesn't belong to you...\n\r");
      return;
    }

    sprintf(tString, "immortals/%s/zonefile", ch->getName().c_str());

    if (!(tFile = fopen(tString, "w"))) {
      ch->sendTo("Something went wrong, tell a coder what you did.\n\r");
      vlogf(LOG_FILE,
        format("Unable to create file for zonefile writing.  [%s]") % tString);
      return;
    }

    roomStart = (zValue <= 0 ? 0 : (zone_table[zValue - 1].top + 1));

    tStString += "Saving Header.\n\r";

    sprintf(tString, "#%d\n", roomStart);
    fputs(tString, tFile);
    sprintf(tString, "%s~\n", zone_table[zValue].name.c_str());
    fputs(tString, tFile);
    sprintf(tString, "%d %d %d %d\n", zone_table[zValue].top, 30, 2, 0);
    fputs(tString, tFile);

    tStString += "Saving Door Status:";  // 'D' Command

    for (roomIndex = roomStart;
         roomIndex < (unsigned)(zone_table[zValue].top + 1); roomIndex++) {
      if (!(tRoom = real_roomp(roomIndex)))
        continue;

      roomDirData* tExit;
      TRoom* tERoom;

      for (int exitIndex = 0; exitIndex < MAX_DIR; exitIndex++) {
        if (!(tExit = tRoom->dir_option[exitIndex]) ||
            tExit->door_type <= DOOR_NONE || !(tExit->condition & EXIT_CLOSED))
          continue;

        sprintf(tString, "D 0 %d %d %d", roomIndex, exitIndex,
          ((tExit->condition & EXIT_LOCKED) ? 2 : 1));
        sprintf(tBuffer, "%-20s %s -> %s\n", tString,
          tRoom->getNameNOC(ch).c_str(),
          ((tERoom = real_roomp(tExit->to_room))
              ? tERoom->getNameNOC(ch).c_str()
              : "Unknown"));
        fputs(tBuffer, tFile);
      }

      tStString += '.';
    }

    tStString += "\n\rSaving Object Status:";  // 'O' Command & 'P' Command

    for (roomIndex = roomStart;
         roomIndex < (unsigned)(zone_table[zValue].top + 1); roomIndex++) {
      if (!(tRoom = real_roomp(roomIndex)))
        continue;

      TThing *tThing = NULL, *tThing2 = NULL;
      TObj *tObj, *tObj2;
      int iMaxExist = 0;

      for (StuffIter it = tRoom->stuff.begin();
           it != tRoom->stuff.end() && (tThing = *it); ++it) {
        if (!(tObj = dynamic_cast<TObj*>(tThing)) || (tObj->getSnum() < 0) ||
            tObj->canWear(ITEM_WEAR_TAKE))
          continue;

        for (StuffIter it = tRoom->stuff.begin();
             it != tRoom->stuff.end() && (tThing2 = *it); ++it) {
          if (!(tObj2 = dynamic_cast<TObj*>(tThing2)) ||
              tObj2->canWear(ITEM_WEAR_TAKE) ||
              tObj2->getSnum() != tObj->getSnum())
            continue;

          iMaxExist++;
        }

        sprintf(tString, "O 0 %d %d %d", tObj->getSnum(),
          min(iMaxExist, tObj->max_exist), roomIndex);
        sprintf(tBuffer, "%-23s %s\n", tString,
          (tObj->getNameNOC(ch).c_str() ? tObj->getNameNOC(ch).c_str()
                                        : "Unknown"));
        fputs(tBuffer, tFile);

        if (!tObj->stuff.empty()) {
          TThing *sThing = NULL, *sThing2 = NULL;
          TObj *sObj, *sObj2;

          iMaxExist = 0;

          for (StuffIter it = tObj->stuff.begin();
               it != tObj->stuff.end() && (sThing = *it); ++it) {
            if (!(sObj = dynamic_cast<TObj*>(sThing)) || sObj->getSnum() < 0)
              continue;

            for (StuffIter it = tObj->stuff.begin();
                 it != tObj->stuff.end() && (sThing2 = *it); ++it) {
              if (!(sObj2 = dynamic_cast<TObj*>(sThing2)) ||
                  sObj2->getSnum() != sObj->getSnum())
                continue;

              iMaxExist++;
            }

            sprintf(tString, "P 1 %d %d %d", sObj->getSnum(),
              min(iMaxExist, sObj->max_exist), tObj->getSnum());
            sprintf(tBuffer, "%-25s %s\n", tString,
              (sObj->getNameNOC(ch).c_str() ? sObj->getNameNOC(ch).c_str()
                                            : "Unknown"));
            fputs(tBuffer, tFile);
          }
        }
      }

      tStString += '.';
    }

    tStString += "\n\rSaving Booty Status:";  // 'B' Command & 'P' Command

    for (roomIndex = roomStart;
         roomIndex < (unsigned)(zone_table[zValue].top + 1); roomIndex++) {
      if (!(tRoom = real_roomp(roomIndex)))
        continue;

      TThing *tThing = NULL, *tThing2 = NULL;
      TObj *tObj, *tObj2;
      int iMaxExist = 0;

      for (StuffIter it = tRoom->stuff.begin();
           it != tRoom->stuff.end() && (tThing = *it); ++it) {
        if (!(tObj = dynamic_cast<TObj*>(tThing)) || (tObj->getSnum() < 0) ||
            !tObj->canWear(ITEM_WEAR_TAKE))
          continue;

        for (StuffIter it = tRoom->stuff.begin();
             it != tRoom->stuff.end() && (tThing2 = *it); ++it) {
          if (!(tObj2 = dynamic_cast<TObj*>(tThing2)) ||
              !tObj2->canWear(ITEM_WEAR_TAKE) ||
              tObj2->getSnum() != tObj->getSnum())
            continue;

          iMaxExist++;
        }

        sprintf(tString, "B 0 %d %d %d", tObj->getSnum(),
          min(iMaxExist, tObj->max_exist), roomIndex);
        sprintf(tBuffer, "%-23s %s\n", tString,
          (tObj->getNameNOC(ch).c_str() ? tObj->getNameNOC(ch).c_str()
                                        : "Unknown"));
        fputs(tBuffer, tFile);

        if (!tObj->stuff.empty()) {
          TThing *sThing = NULL, *sThing2 = NULL;
          TObj *sObj, *sObj2;

          iMaxExist = 0;

          for (StuffIter it = tObj->stuff.begin();
               it != tObj->stuff.end() && (sThing = *it); ++it) {
            if (!(sObj = dynamic_cast<TObj*>(sThing)) || sObj->getSnum() < 0)
              continue;

            for (StuffIter it = tObj->stuff.begin();
                 it != tObj->stuff.end() && (sThing2 = *it); ++it) {
              if (!(sObj2 = dynamic_cast<TObj*>(sThing2)) ||
                  sObj2->getSnum() != sObj->getSnum())
                continue;

              iMaxExist++;
            }

            sprintf(tString, "P 1 %d %d %d", sObj->getSnum(),
              min(iMaxExist, sObj->max_exist), tObj->getSnum());
            sprintf(tBuffer, "%-25s %s\n", tString,
              (sObj->getNameNOC(ch).c_str() ? sObj->getNameNOC(ch).c_str()
                                            : "Unknown"));
            fputs(tBuffer, tFile);
          }
        }
      }

      tStString += '.';
    }

    tStString +=
      "\n\rSaving Mobile Status:";  // 'M' Command & 'G' Command & 'E' Command

    for (roomIndex = roomStart;
         roomIndex < (unsigned)(zone_table[zValue].top + 1); roomIndex++) {
      if (!(tRoom = real_roomp(roomIndex)))
        continue;

      TThing *tThing = NULL, *tThing2 = NULL;
      TMonster *tMob, *tMob2;
      int iMaxExist = 0;

      for (StuffIter it = tRoom->stuff.begin();
           it != tRoom->stuff.end() && (tThing = *it); ++it) {
        if (!(tMob = dynamic_cast<TMonster*>(tThing)) || tMob->getSnum() < 0)
          continue;

        for (StuffIter it = tRoom->stuff.begin();
             it != tRoom->stuff.end() && (tThing2 = *it); ++it) {
          if (!(tMob2 = dynamic_cast<TMonster*>(tThing2)) ||
              tMob2->getSnum() != tMob->getSnum())
            continue;

          iMaxExist++;
        }

        sprintf(tString, "M 0 %d %d %d", tMob->getSnum(),
          min(iMaxExist, tMob->max_exist), roomIndex);
        sprintf(tBuffer, "%-23s %s\n", tString,
          (tMob->getNameNOC(ch).c_str() ? tMob->getNameNOC(ch).c_str()
                                        : "Unknown"));
        fputs(tBuffer, tFile);

        if (!tMob->stuff.empty()) {
          TThing* sThing = NULL;
          TObj* sObj;

          for (StuffIter it = tMob->stuff.begin();
               it != tMob->stuff.end() && (sThing = *it); ++it) {
            if (!(sObj = dynamic_cast<TObj*>(sThing)) || sObj->getSnum() < 0)
              continue;

            sprintf(tString, "G 1 %d %d", sObj->getSnum(), sObj->max_exist);
            sprintf(tBuffer, "%-24s %s\n", tString,
              (sObj->getNameNOC(ch).c_str() ? sObj->getNameNOC(ch).c_str()
                                            : "Unknown"));
            fputs(tBuffer, tFile);
          }
        }

        TObj* sObj;

        for (wearSlotT wearIndex = MIN_WEAR; wearIndex < MAX_WEAR;
             wearIndex++) {
          if (!(sObj = dynamic_cast<TObj*>(tMob->equipment[wearIndex])) ||
              sObj->getSnum() < 0)
            continue;

          sprintf(tString, "E 1 %d %d %d", sObj->getSnum(), sObj->max_exist,
            mapSlotToFile(wearIndex));
          sprintf(tBuffer, "%-25s %s [%s]\n", tString,
            (sObj->getNameNOC(ch).c_str() ? sObj->getNameNOC(ch).c_str()
                                          : "Unknown"),
            bodyParts[wearIndex]);
          fputs(tBuffer, tFile);
        }
      }
      tStString += '.';
    }

    tStString += '\n';
    fputs("S\n", tFile);
    fclose(tFile);

#ifdef MAIL_ZONEFILE
    if (((zValue > 0) ? (zone_table[zValue - 1].top + 1) : 0) ==
          ch->desc->blockastart ||
        ((zValue > 0) ? (zone_table[zValue - 1].top + 1) : 0) ==
          ch->desc->blockbstart) {
      sprintf(tString, "/usr/sbin/sendmail -f%s %s < immortals/%s/zonefile",
        SAVEZONEFILE_REPLYTO, ch->desc->account->email,
        ch->getNameNOC(ch).c_str());
      vsystem(tString);
    }
#endif

    sprintf(tString, "cp -f immortals/%s/zonefile tmp/%s.output",
      ch->getNameNOC(ch).c_str(), ch->getNameNOC(ch).c_str());
    vsystem(tString);
    tStString +=
      "A copy of your zonefile is in your output.  Use 'viewoutput' to see "
      "it.\n\r";
    ch->sendTo(tStString);
  }

  void doLoadZoneFile(TBeing* tBeing, const sstring& tArg) {
#if 0
    unsigned int  zValue,
                  roomIndex,
                  roomStart;
    FILE         *tFile;
    char          tString[256],
    tBuffer[256];
    TRoom        *tRoom;
    sstring        tStString("");

    if (!tBeing->isImmortal() || !tBeing->desc || !tBeing->isPc())
      return;

    if ((zValue = ch->roomp->getZoneNum()) > zone_table.size()) {
      vlogf(LOG_BUG, format("Immortal in invalid zone [%s]") %  tBeing->getName());
      tBeing->sendTo("You are in an invalid zone, how did you get there?!?\n\r");
      return;
    }

    if (zone_table[zValue].enabled) {
      tBeing->sendTo("I'm sorry, this zone is enabled so you cannot do this for it.\n\r");
      return;
    }

    if (((zValue > 0) ? (zone_table[zValue - 1].top + 1) : 0)
        != ch->desc->blockastart &&
        ((zValue > 0) ? (zone_table[zValue - 1].top + 1) : 0)
        != ch->desc->blockbstart) {
      tBeing->sendTo("I'm sorry, this zone doesn't belong to you...\n\r");
      return;
    }

    sprintf(tString, "immortals/%s/zonefile", ch->getName());

    if (!(tFile = fopen(tString, "r"))) {
      tBeing->sendTo("Either you have no zonefile or something is horribly wrong.\n\r");
      return;
    }

    roomStart = (zValue <= 0 ? 0 : (zone_table[zValue - 1].top + 1));

    tStString += "Puring Zone\n";

    for (roomIndex = roomSrart; roomIndex < (unsigned) (zone_table[zValue].top + 1); roomIndex++) {
      if (!(tRoom = real_roomp(roomIndex)))
        continue;

      TThing * tThing = NULL;

      while ((tThing = tRoom->getStuff())) {
        --(*tThing);
        delete tThing;
        tThing = NULL;
      }

      tStString += ".";
    }

    tStString += "\n\rLoading zonefile";

    char tChar,
         *tString = NULL;

    fscanf(tFile, " #%*d\n");

    if (*(tString = fread_string(tFile)) != '$') {
      while (!feof(tFile)) {
        tChar = fgetc(tFile);

      }
    }

    tStString += "\n\rDone.\n\r";

    delete tString;
    tString = NULL;
    tStString += "A copy of your zonefile is in your output.  Use 'viewoutput' to see it.\n\r";

    tBeing->sendTo(tStString);

#endif
  }
}  // namespace

void TBeing::doZonefile(const sstring& tStArg) {
  sstring tStString(""), tStBuffer("");

  if (!hasWizPower(POWER_ZONEFILE_UTILITY)) {
    sendTo("You have not been given this power yet.\n\r");
    return;
  }

  if (tStArg.empty()) {
    sendTo("Syntax:\n\r");
    sendTo("zonefile save : Save current zonefile status.\n\r");
    sendTo("zonefile load : Load current zonefile status.\n\r");
    sendTo(
      "zonefile new <n> <builder - zone title>: Split <n> rooms off the last "
      "block, creating a new zone named <builder - zone title>.\n\r");
#if 0
    sendTo("zonefile modify <field> SEE HELP FILE\n\r");
#endif
  }

  std::string cmd = tStArg.word(0);
  std::string rest = tStArg.substr(tStArg.find(' ') + 1);

  if (is_abbrev(cmd, "save")) {
    doSaveZoneFile(this, rest);
  } else if (is_abbrev(cmd, "load") && getName() == "Lapsos") {
    doLoadZoneFile(this, rest);
  } else if (is_abbrev(cmd, "new")) {
    doNewZoneFile(*this, rest);
  }
}
