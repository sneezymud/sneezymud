//////////////////////////////////////////////////////////////////////////
//
// SneezyMUD - All rights reserved, SneezyMUD Coding Team
//
// $Log: cmd_save.cc,v $
// Revision 1.1  1999/09/12 17:24:04  sneezy
// Initial revision
//
//
//////////////////////////////////////////////////////////////////////////


/*****************************************************************************

  SneezyMUD++ - All rights reserved, SneezyMUD Coding Team.

  "cmd_save.cc"
  Primary function and rountines used by the save command.

  Created 5/17/?? - ??(??)

******************************************************************************/

#include "stdsneezy.h"

const char SAVEZONEFILE_REPLYTO[] = "mithros@spasm.stanford.edu";

void doSaveZoneFile(TBeing *ch, const char *tArg)
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

  if (!ch->hasWizPower(POWER_ZONEFILE_UTILITY)) {
    ch->sendTo("You have not been given this power yet.\n\r");
    return;
  }

  if ((zValue = ch->roomp->getZone()) > zone_table.size()) {
    vlogf(7, "Immortal in invalid zone [%s]", ch->getName());
    ch->sendTo("You are in an invalid zone, how did you get there?!?\n\r");
    return;
  }

  if (zone_table[zValue].enabled) {
    ch->sendTo("I'm sorry, this zone is enabled so you cannot do this for it.\n\r");
    return;
  }

  sprintf(tString, "immortals/%s/zonefile", ch->getName());

  if (!(tFile = fopen(tString, "w"))) {
    ch->sendTo("Something went wrong, tell a coder what you did.\n\r");
    vlogf(7, "Unable to create file for zonefile writing.  [%s]", tString);
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

      TObj *sObj;;

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

void doSaveMOEdit(TBeing *ch, const char *tArg)
{
  string  tStThing(""),
          tStValue(""),
          tStArg(tArg);
  int     tValue;
  bool    saveMethod = false;
  char    tString[256];
  TThing *tThing;

  tStArg = two_arg(tStArg, tStThing, tStValue);

  if (!ch->isImmortal() || !ch->desc || !ch->isPc())
    return;

  if (is_abbrev(tStThing.c_str(), "zonefile")) {
    doSaveZoneFile(ch, tArg);
    return;
  }

  if (!ch->hasWizPower(POWER_MEDIT) &&
      !ch->hasWizPower(POWER_OEDIT)) {
    ch->sendTo("You cannot use this, go away little one.\n\r");
    return;
  }

  if (!ch->roomp) {
    vlogf(7, "Player doing save without a room!  [%s]", ch->getName());
    return;
  }

  if (!(tThing = searchLinkedList(tStThing, ch->roomp->stuff)) &&
      !(tThing = searchLinkedList(tStThing, ch->stuff))) {
    ch->sendTo("I don't see that here, do you?\n\r");
    return;
  }

  if (dynamic_cast<TBeing *>(tThing) &&
      (dynamic_cast<TBeing *>(tThing))->isPc()) {
    ch->sendTo("Kinky.  Did you buy them dinner first?\n\r");
    (dynamic_cast<TBeing *>(tThing))->sendTo("%s just tried to save you for later use!\n\r",
                                             ch->getName());
    return;
  }

  if (dynamic_cast<TMonster *>(tThing))
    saveMethod = false;
  else if (dynamic_cast<TObj *>(tThing))
    saveMethod = true;
  else {
    ch->sendTo("You cannot save that, go away little one.\n\r");
    return;
  }

  if (tStValue.empty()) {
    if (!ch->hasWizPower(POWER_WIZARD)) {
      ch->sendTo("Syntax: save <thing> <vnum>\n\r");
      return;
    }

    if (!saveMethod)
      tValue = (dynamic_cast<TMonster *>(tThing))->mobVnum();
    else
      tValue = (dynamic_cast<TObj *>(tThing))->objVnum();
  } else
    tValue = atoi(tStValue.c_str());

  sprintf(tString, "save %s %d", tStThing.c_str(), tValue);

  if (!saveMethod) {
    if (ch->hasWizPower(POWER_MEDIT))
      ch->doMedit(tString);
    else
      ch->sendTo("You do not have access to medit.  Therefore you cannot save mobiles.\n\r");
  } else {
    if (ch->hasWizPower(POWER_OEDIT))
      ch->doOEdit(tString);
    else
      ch->sendTo("You do not have access to oedit.  Therefore you cannot save objects.\n\r");
  }
}

void TBeing::doSave(silentTypeT silent, const char *tArg = NULL)
{
  objCost  tCost;
  TPerson *tPerson;
  TThing  *tThing,
          *tEq[MAX_WEAR],
          *tObj;
  wearSlotT wearIndex;

  verifyWeightVolume();

  if (!isPc())
    return;

  if (!desc) {
    vlogf(10, "%s tried to doSave while link dead.", getName());
    return;
  }

  if (isImmortal()) {
    if (!tArg || !*tArg) {
      wizFileSave();

      if (!IS_SET(desc->account->flags, ACCOUNT_IMMORTAL)) {
        // log the account as an immortal one.
        SET_BIT(desc->account->flags, ACCOUNT_IMMORTAL);
        desc->saveAccount();
      }
    } else {
      doSaveMOEdit(this, tArg);
      return;
    }
  }

  if (!silent)
    sendTo("Saving.\n\r");

  if (dynamic_cast<TMonster *>(this) && IS_SET(specials.act, ACT_POLYSELF)) {
    if (!(tPerson = desc->original)) {
      vlogf(8, "BAD SAVE OF POLY!");
      return;
    }

    tThing         = tPerson->stuff;
    tPerson->stuff = stuff;

    for (wearIndex = MIN_WEAR; wearIndex < MAX_WEAR; wearIndex++) {
      tEq[wearIndex]                = tPerson->equipment[wearIndex];
      tPerson->equipment[wearIndex] = equipment[wearIndex];
    }
    tPerson->setExp(getExp());
    tPerson->setMoney(getMoney());
    tPerson->classSpecificStuff();
    tPerson->recepOffer(NULL, &tCost);
    tPerson->saveRent(&tCost, FALSE, 0);

    // Lets try something diffrent
    // save the room they are in so they come back there.
    // check the room for HAVE_TO_WALK and don't save them there as this implies
    // beyond locked door
#if 1
    saveChar(ROOM_AUTO_RENT);
#else
    if (roomp->isRoomFlag(HAVE_TO_WALK))
      saveChar(ROOM_AUTO_RENT);
    else
      saveChar(in_room);
#endif

    tPerson->stuff = tThing;
    for (wearIndex = MIN_WEAR; wearIndex < MAX_WEAR; wearIndex++) {
      tPerson->equipment[wearIndex] = tEq[wearIndex];
      if (equipment[wearIndex] && equipment[wearIndex]->in_room != -1) {
        tObj = equipment[wearIndex];
        equipment[wearIndex] = 0;
        --(*tObj);
        equipChar(tObj, wearIndex, SILENT_YES);        // equip the correct slot
      }
    }
    return;
  } else {
    classSpecificStuff();
    recepOffer(NULL, &tCost);
    dynamic_cast<TPerson *>(this)->saveRent(&tCost, FALSE, 0);

    // Lets try something diffrent
    // save the room they are in so they come back there.
    // check the room for HAVE_TO_WALK and don't save them there as this implies
    // beyond locked door
#if 1
    saveChar(ROOM_AUTO_RENT);
#else
    if (roomp->isRoomFlag(HAVE_TO_WALK))
      saveChar(ROOM_AUTO_RENT);
    else
      saveChar(in_room);
#endif
  }
}
