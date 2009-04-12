//////////////////////////////////////////////////////////////////////////
//
//    SneezyMUD - All rights reserved, SneezyMUD Coding Team
//    wiz_data.cc : load/save immortal information
//
//////////////////////////////////////////////////////////////////////////

extern "C" {
#include <unistd.h>
#include <sys/stat.h>
}

#include "being.h"
#include "extern.h"
#include "monster.h"
#include "person.h"

class wizSaveData {
  public:
    int setsev,
        office,
        blockastart,
        blockaend,
        blockbstart,
        blockbend;

    wizSaveData();
};

wizSaveData::wizSaveData() :
  setsev(0),
  office(0),
  blockastart(0),
  blockaend(0),
  blockbstart(0),
  blockbend(0)
{}

void TBeing::wizFileRead()
{
  FILE *fp;
  sstring buf, buf2;
  Descriptor *d = NULL;
  wizSaveData saveData;

  // don't use isImmortal, save always
  if (!(GetMaxLevel() > MAX_MORT) || !(d = desc))
    return;

  buf = format("immortals/%s/wizdata") % getName();
  fp = fopen(buf.c_str(), "r");
  if (!fp) {
    buf2 = format("immortals/%s") % getName();
    fp = fopen(buf2.c_str(), "r");
    if (!fp) {	// no immort directory 
      if (mkdir(buf2.c_str(), 0770)) {
	sendTo("Unable to create a wizard directory for you.\n\r");
	vlogf(LOG_FILE, format("Unable to create a wizard directory for %s.") %  getName());
      } else
	sendTo("Wizard directory created...\n\r");
    } else
      fclose(fp);	// player has no wizard data, but has immort directory 

    return;
  }
  if (fread(&saveData, sizeof(saveData), 1, fp) != 1) {
    vlogf(LOG_BUG, format("Corrupt wiz save file for %s") %  getName());
    fclose(fp);
    return;
  } 
  fclose(fp);

  d->severity    = saveData.setsev;
  d->office      = saveData.office;
  d->blockastart = saveData.blockastart;
  d->blockaend   = saveData.blockaend;
  d->blockbstart = saveData.blockbstart;
  d->blockbend   = saveData.blockbend;

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
  FILE *fp;
  sstring buf;
  Descriptor *d = NULL;
  wizSaveData saveData;

  if (!(d = desc))
    return;
  if (d->connected != CON_PLYNG)  // semi arbitrary, but here for sanity
    return;

  buf = format("immortals/%s/wizdata") % getName();
  unlink(buf.c_str());
  if (!(fp = fopen(buf.c_str(), "wa+"))) {
    buf = format("immortals/%s") % getName();
    if (!(fp = fopen(buf.c_str(), "r"))) {	// no immort directory 
      if (mkdir(buf.c_str(), 0770))
	sendTo("Unable to create a wizard directory for you.  Tell Brutius or Batopr.\n\r");
      else
	sendTo("Wizard directory created... type 'save' again.\n\r");
    } else
      fclose(fp);	// player has no wizard data, but has immort directory 

    return;
  }

  saveData.setsev      = d->severity;
  saveData.office      = d->office;
  saveData.blockastart = d->blockastart;
  saveData.blockaend   = d->blockaend;
  saveData.blockbstart = d->blockbstart;
  saveData.blockbend   = d->blockbend;

  fwrite(&saveData, sizeof(wizSaveData), 1, fp);   
  fclose(fp);
}


void TBeing::doOffice(sstring arg)
{
  FILE *fp;
  sstring buf, buf2, buf3;
  Descriptor *d = NULL;
  wizSaveData saveData;

  // don't use isImmortal, save always
  if (!(GetMaxLevel() > MAX_MORT) || !(d = desc) || !hasWizPower(POWER_GOTO))
    return;

  if (arg == "") {
    sendTo("You must supply a name as an argument (case-sensitive).\n\r");
    return;
  }

  buf = format("immortals/%s/wizdata") % arg;
  fp = fopen(buf.c_str(), "r");
  if (!fp) {
	  sendTo(format("Unable to open file for %s (case sensitive!).\n\r") % arg);
    return;
  }
  if (fread(&saveData, sizeof(saveData), 1, fp) != 1) {
    vlogf(LOG_BUG, format("Corrupt wiz save file for %s") % arg);
    fclose(fp);
    return;
  } 
  fclose(fp);
  buf3 = format("The office of %s is %d.\n\r") % arg % saveData.office;
  sendTo(buf3);
}

