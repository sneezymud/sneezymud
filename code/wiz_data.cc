//////////////////////////////////////////////////////////////////////////
//
// SneezyMUD - All rights reserved, SneezyMUD Coding Team
//
// $Log: wiz_data.cc,v $
// Revision 5.1  1999/10/16 04:31:17  batopr
// new branch
//
// Revision 1.1  1999/09/12 17:24:04  sneezy
// Initial revision
//
//
//////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////
//
//    SneezyMUD 4.1 - All rights reserved, SneezyMUD Coding Team
//    wiz_data.cc : load/save immortal information
//
//////////////////////////////////////////////////////////////////////////

extern "C" {
#include <unistd.h>
#include <sys/stat.h>
}


#include "stdsneezy.h"

class wizSaveData {
  public:
    char poofin[1024];
    char poofout[1024];
    char longDescr[1024];
    int setsev;

    wizSaveData();
};

wizSaveData::wizSaveData() :
  setsev(0)
{
  *poofin = *poofout = *longDescr = '\0';
}

void TBeing::wizFileRead()
{
  FILE *fp;
  char buf[256], buf2[256];
  Descriptor *d = NULL;
  wizSaveData saveData;

  // don't use isImmortal, save always
  if (!(GetMaxLevel() > MAX_MORT) || !(d = desc))
    return;

  sprintf(buf, "immortals/%s/wizdata", getName());
  fp = fopen(buf, "r");
  if (!fp) {
    sprintf(buf2, "immortals/%s", getName());
    fp = fopen(buf2, "r");
    if (!fp) {	// no immort directory 
      if (mkdir(buf2, 0770)) {
	sendTo("Unable to create a wizard directory for you.\n\r");
	vlogf(10, "Unable to create a wizard directory for %s.", getName());
      } else
	sendTo("Wizard directory created...\n\r");
    } else
      fclose(fp);	// player has no wizard data, but has immort directory 

    return;
  }
  if (fread(&saveData, sizeof(saveData), 1, fp) != 1) {
    vlogf(10, "Corrupt wiz save file for %s", getName());
    fclose(fp);
    return;
  } 
  fclose(fp);

  d->severity = saveData.setsev;

  delete [] d->poof.poofin;
  d->poof.poofin = dsearch(saveData.poofin);

  delete [] d->poof.poofout;
  d->poof.poofout = dsearch(saveData.poofout);

  delete [] player.longDescr;
  if (*(saveData.longDescr))
    player.longDescr = mud_str_dup(saveData.longDescr);
  else
    player.longDescr = NULL;

  if (should_be_logged(this))
    vlogf(0, "Loaded %s's wizard file.", getName());
}

void TMonster::wizFileSave()
{
  return;
}

void TPerson::wizFileSave()
{
  FILE *fp;
  char buf[128];
  Descriptor *d = NULL;
  wizSaveData saveData;

  if (!(d = desc))
    return;
  if (d->connected != CON_PLYNG)  // semi arbitrary, but here for sanity
    return;

  sprintf(buf, "immortals/%s/wizdata", getName());
  unlink(buf);
  if (!(fp = fopen(buf, "wa+"))) {
    sprintf(buf, "immortals/%s", getName());
    if (!(fp = fopen(buf, "r"))) {	// no immort directory 
      if (mkdir(buf, 0770))
	sendTo("Unable to create a wizard directory for you.  Tell Brutius or Batopr.\n\r");
      else
	sendTo("Wizard directory created... type 'save' again.\n\r");
    } else
      fclose(fp);	// player has no wizard data, but has immort directory 

    return;
  }

  if (player.longDescr)
     strcpy(saveData.longDescr, player.longDescr);
 
  if (d->poof.poofin)
    strcpy(saveData.poofin, d->poof.poofin);

  if (d->poof.poofout)
    strcpy(saveData.poofout, d->poof.poofout);

  saveData.setsev = d->severity;

  fwrite(&saveData, sizeof(wizSaveData), 1, fp);   
  fclose(fp);
}
