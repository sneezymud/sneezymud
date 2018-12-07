//////////////////////////////////////////////////////////////////////////
//
//    SneezyMUD - All rights reserved, SneezyMUD Coding Team
//    wiz_data.cc : load/save immortal information
//
//////////////////////////////////////////////////////////////////////////

extern "C" {
#include <stdio.h>

#include <unistd.h>
#include <sys/stat.h>
}

#include "being.h"
#include "extern.h"
#include "monster.h"
#include "person.h"
#include "database.h"

void TBeing::wizFileRead()
{
  Descriptor *d = NULL;

  // don't use isImmortal, save always
  if (!(GetMaxLevel() > MAX_MORT) || !(d = desc))
    return;

  TDatabase db(DB_SNEEZY);

  db.query("select setsev, office, blockastart, blockaend, blockbstart, blockbend, player_id from wizdata "
           "where player_id = %i", getPlayerID());

  db.fetchRow();

  d->severity     = (int) convertTo<int>(db["setsev"]);
  d->office       = (int) convertTo<int>(db["office"]);
  d->blockastart  = (int) convertTo<int>(db["blockastart"]);
  d->blockaend    = (int) convertTo<int>(db["blockaend"]);
  d->blockbstart  = (int) convertTo<int>(db["blockbstart"]);
  d->blockbend    = (int) convertTo<int>(db["blockbend"]);

  if (should_be_logged(this))
    vlogf(LOG_IIO, format("Loaded %s's wizard file.") %  getName());

  TPerson * tPerson = dynamic_cast<TPerson *>(this);

  if (tPerson && !tPerson->tLogFile && should_be_logged(tPerson)) {
    sstring tString;

    tString = format("immortals/%s/logfile") % name;

    if (!(tPerson->tLogFile = fopen(tString.c_str(), "a")))
      vlogf(LOG_FILE, format("Unable to open Log File for %s") %  name);
    else
      tPerson->logf("Logging in...");
  }
}

void TMonster::wizFileSave()
{
  return;
}

void TPerson::wizFileSave()
{
  Descriptor *d = NULL;

  if (!(d = desc))
    return;
  if (d->connected != CON_PLYNG)  // semi arbitrary, but here for sanity
    return;

  TDatabase db(DB_SNEEZY);

  db.query("replace into wizdata (setsev, office, blockastart, blockaend, blockbstart, blockbend, player_id) "
            "values (%i, %i, %i,%i, %i, %i, %i)", d->severity, d->office, d->blockastart, 
            d->blockaend, d->blockbstart, d->blockbend, getPlayerID());

}


void TBeing::doOffice(sstring arg)
{
  sstring buf;
  // don't use isImmortal, save always
  if (!(GetMaxLevel() > MAX_MORT) || !hasWizPower(POWER_GOTO))
    return;

  if (arg == "") {
    sendTo("You must supply a name as an argument (case-sensitive).\n\r");
    return;
  }

  TDatabase db(DB_SNEEZY);

  db.query("select office from wizdata where player_id = (select id from player where name = '%s')", arg.c_str());
  db.fetchRow();
  buf = format("The office of %s is %d.\n\r") % arg % (int)convertTo<int>(db["office"]);
  sendTo(buf);
}

