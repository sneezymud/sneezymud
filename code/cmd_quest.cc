//////////////////////////////////////////////////////////////////////////
//
// SneezyMUD - All rights reserved, SneezyMUD Coding Team
//
// $Log: cmd_quest.cc,v $
// Revision 5.1  1999/10/16 04:31:17  batopr
// new branch
//
// Revision 1.2  1999/09/27 14:12:24  lapsos
// Fixed typo that was preventing execution.
//
// Revision 1.1  1999/09/12 17:24:04  sneezy
// Initial revision
//
//
//////////////////////////////////////////////////////////////////////////


#include "stdsneezy.h"


bool TBeing::hasQuestBit(int) const
{
  return false;
}

void TBeing::setQuestBit(int)
{
}

void TBeing::remQuestBit(int)
{
}

bool TPerson::hasQuestBit(int value) const
{
  if (value < 0 || value >= MAX_TOG_INDEX) {
    vlogf(6, "Bad check of hasQuestBit(%d)", value);
    return FALSE;
  }

  return (toggles[value]);
}

void TPerson::setQuestBit(int value)
{
  if (value < 0 || value >= MAX_TOG_INDEX) {
    vlogf(6, "Bad check of setQuestBit(%d)", value);
    return;
  }

  toggles[value] |= 0x1;
}

void TPerson::remQuestBit(int value)
{
  if (value < 0 || value >= MAX_TOG_INDEX) {
    vlogf(6, "Bad check of remQuestBit(%d)", value);
    return;
  }

  toggles[value] &= ~(0x1);
}

void TBeing::doMortalQuest(const char *tArg)
{
  /********************************************
   Quest information is no longer housed here.
   use: lib/mobdata/responses/help/tog_number
   to add help to a response quest.
   ********************************************/
  sendTo("Your current quest status:\n\r");

  if (GetMaxLevel() > MAX_MORT) {
    char buf[256];
    const char * t2 = one_argument(tArg, buf);
    char   questPath[256];
    string tStString("");

    // check "immorts" for "quest real 3"
    if (is_abbrev(buf, "real")) {
      int questNumber = atoi(t2);
      if (questNumber < 0 || questNumber >= MAX_TOG_INDEX) {
        sendTo("Invalid quest value.\n\r");
        return;
      }

      sprintf(questPath, "mobdata/responses/help/%d", questNumber);
      if (file_to_string(questPath, tStString, true))
        desc->page_string(tStString.c_str(), 0);
      else
        sendTo("No such quest helpfile seems to exist.\n\r");

      return;
    }
  }

  int questNumber = atoi(tArg);
  unsigned int totFound = 0;
  int questRes = -1;
  char   questPath[256];
  string tStString("");

  if (questNumber <= 0)
    questNumber = 1;

  for (int questIndex = (MAX_TOG_INDEX - 1); questIndex > -1; questIndex--) {
    if (hasQuestBit(questIndex)) {
      sprintf(questPath, "mobdata/responses/help/%d", questIndex);
      FILE *fp = fopen(questPath, "r");
      if (fp) {
        totFound++;
        fclose(fp);
        if (questNumber == (int) totFound)
          questRes = questIndex;
      }
    }
  }

  sendTo("You have %d total current quest goals.\n\r", totFound);
  if (questRes == -1) {
    sendTo("You don't seem to have a quest goal #%d\n\r", questNumber);
    return;
  }

  sprintf(questPath, "mobdata/responses/help/%d", questRes);
  if (file_to_string(questPath, tStString, true))
    desc->page_string(tStString.c_str(), 0);
  // else condition not needed, it should be valid based on above logic
}

