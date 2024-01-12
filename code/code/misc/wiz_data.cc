//////////////////////////////////////////////////////////////////////////
//
//    SneezyMUD - All rights reserved, SneezyMUD Coding Team
//    wiz_data.cc : load/save immortal information
//
//////////////////////////////////////////////////////////////////////////

extern "C" {
#include <cstdio>

#include <unistd.h>
#include <sys/stat.h>
}

#include "being.h"
#include "extern.h"
#include "monster.h"
#include "person.h"
#include "database.h"

void TBeing::wizFileRead() {
  Descriptor* d = nullptr;

  // don't use isImmortal, save always
  if (!(GetMaxLevel() > MAX_MORT) || !(d = desc))
    return;

  TDatabase db(DB_SNEEZY);

  db.query(
    "select setsev, office, blockastart, blockaend, blockbstart, blockbend, "
    "player_id from wizdata "
    "where player_id = %i",
    getPlayerID());

  db.fetchRow();

  d->severity = convertTo<int>(db["setsev"]);
  d->office = convertTo<int>(db["office"]);
  d->blockastart = convertTo<int>(db["blockastart"]);
  d->blockaend = convertTo<int>(db["blockaend"]);
  d->blockbstart = convertTo<int>(db["blockbstart"]);
  d->blockbend = convertTo<int>(db["blockbend"]);

  if (should_be_logged(this))
    vlogf(LOG_IIO, format("Loaded %s's wizard file.") % getName());

  TPerson* tPerson = dynamic_cast<TPerson*>(this);

  if (tPerson && !tPerson->tLogFile && should_be_logged(tPerson)) {
    sstring tString;

    tString = format("mutable/immortals/%s/logfile") % name;

    if (!(tPerson->tLogFile = fopen(tString.c_str(), "a")))
      vlogf(LOG_FILE, format("Unable to open Log File for %s") % name);
    else
      tPerson->logf("Logging in...");
  }
}

void TMonster::wizFileSave() { return; }

void TPerson::wizFileSave() {
  Descriptor* d = nullptr;

  if (!(d = desc))
    return;
  if (d->connected != CON_PLYNG)  // semi arbitrary, but here for sanity
    return;

  TDatabase db(DB_SNEEZY);

  db.query(
    "replace into wizdata (setsev, office, blockastart, blockaend, "
    "blockbstart, blockbend, player_id) "
    "values (%i, %i, %i,%i, %i, %i, %i)",
    d->severity, d->office, d->blockastart, d->blockaend, d->blockbstart,
    d->blockbend, getPlayerID());
}

void TBeing::doOffice(sstring arg) {
  // don't use isImmortal, save always
  if (!(GetMaxLevel() > MAX_MORT) || !hasWizPower(POWER_GOTO))
    return;

  if (arg == "") {
    sendTo("You must supply a name as an argument (case-sensitive).\n\r");
    return;
  }

  TDatabase db(DB_SNEEZY);

  db.query(
    "select w.office, p.name from wizdata w join player p on w.player_id = "
    "p.id where p.name = '%s'",
    arg.c_str());
  db.fetchRow();
  if (db["name"].empty()) {
    sendTo(format("Unable to find player named %s.\n\r") % arg);
    return;
  }
  if (db["office"].empty()) {
    sendTo(format("%s has no office.\n\r") % arg);
    return;
  }
  sendTo(
    format("The office of %s is %d.\n\r") % arg % convertTo<int>(db["office"]));
}
