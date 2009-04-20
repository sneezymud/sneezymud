////////////////////////////////////////////////////////////////////////// 
//
//      SneezyMUD++ - All rights reserved, SneezyMUD Coding Team
//
//      "newbie.cc" - All commands reserved for newbies
//  
//////////////////////////////////////////////////////////////////////////

#include "being.h"
#include "client.h"
#include "person.h"
#include "colorstring.h"
#include "monster.h"
#include "account.h"
#include "cmd_message.h"
#include "spec_mobs.h"

void TBeing::doNewbieEqLoad(race_t num, unsigned short num2, bool initNum)
{
  race_t raceNum;
  unsigned short classNum;

  if (isImmortal() || !isPc()) {
    if (!initNum) {
      raceNum = num;
      classNum = num2;
    } else {
      raceNum = getRace();
      classNum = getClass();
    }
  } else if (desc) {
    if (spec && spec == SPEC_NEWBIE_EQUIPPER) {
      raceNum = num;
      classNum = num2;
    } else {
      raceNum = getRace();
      classNum = getClass();
    }
  } else if (spec && spec == SPEC_NEWBIE_EQUIPPER) {
    raceNum = num;
    classNum = num2;
  } else {
    vlogf(LOG_BUG, format("Something called doNewbieEqLoad when it shouldnt %s") %  getName());
    return;
  }

  // basic races
  int hobbitGear[] = {980, 982, 105, 983, 983, 984, 984, 985, 986, 986, 987, 989, 1001, 1009, 1009, 1010, 0};
  int dwarfGear[] = {990, 992, 105, 993, 993, 994, 994, 995, 996, 996, 997, 998, 998, 999, 1001, 1009, 1009, 1010, 0};
  int ogreGear[] = {970, 972, 105, 973, 973, 974, 974, 975, 976, 976, 977, 978, 978, 979, 1001, 1009, 1009, 1010, 0};
  int gnomeGear[] = {960, 962, 105, 963, 963, 964, 964, 965, 966, 966, 967, 968, 968, 969, 1001, 1009, 1010, 0};
  int elvenGear[] = {950, 952, 105, 953, 953, 954, 954, 955, 956, 956, 957, 958, 958, 959, 1001, 1009, 1009, 1010, 0};
  int humanGear[] = {1000, 1002, 105, 1003, 1003, 1004, 1004, 1005, 1006, 1006, 1007, 1008, 1008, 1011, 1001, 1009, 1009, 1010, 0};

  // advanced races
  int trollGear[] = {30924, 30925, 30926, 30926, 30927, 30927, 30928, 30928, 30929, 30930, 30931, 30931, 30932, 30932, 30933, 30934, 30935, 0};
  int birdGear[] = {44825, 44828, 44828, 44829, 44830, 44830, 44831, 44832, 44832, 44833, 44833, 44834, 44836, 1001, 1009, 1009, 1010, 105, 0};
  int bullyGear[] = {4320, 4321, 4321, 4322, 4323, 4323, 4324, 4325, 4325, 4326, 4326, 4327, 4328, 1001, 1009, 1009, 1010, 105, 0};
  int fishGear[] = {44767, 44768, 44768, 44769, 44770, 44770, 44771, 44772, 44772, 44773, 44773, 44774, 44775, 44776, 44777, 44777, 44778, 105, 0};

  int races[] = { RACE_HUMAN, RACE_HOBBIT, RACE_DWARF, RACE_OGRE, RACE_GNOME, RACE_ELVEN,
                  RACE_GOBLIN, RACE_ORC, RACE_TROG, RACE_GNOLL, RACE_FISHMAN, RACE_FROGMAN, RACE_BIRDMAN, RACE_TROLL };
  int *gear[] = { humanGear, hobbitGear, dwarfGear, ogreGear, gnomeGear, elvenGear,
                  gnomeGear, humanGear, hobbitGear, ogreGear, fishGear, bullyGear, birdGear, trollGear };
  int *myGear = humanGear; // default to humans

  for(unsigned int iRace = 0;iRace < cElements(races); iRace++)
    if (raceNum == races[iRace])
    {
      myGear = gear[iRace];
      break;
    }

  for(int iGear = 0; myGear[iGear]; iGear++)
  {
    // rings never loaded for monks
    //if ((myGear[iGear] == 1009 || myGear[iGear] == 30926) &&
    //  (((classNum & CLASS_MONK) && initNum) || hasClass(CLASS_MONK)))
    //  continue;

    int realNum = real_object(myGear[iGear]);
    if (realNum <= 0)
      continue;
    TObj *newbieObj = read_object(realNum, REAL);
    if (!newbieObj)
      continue;

    if (!canUseEquipment(newbieObj, SILENT_YES))
      delete newbieObj;
    else
      *this += *newbieObj;
  }

  return;
}

// ask a newbie question (or reply with answer)
// intended for newbies to get help, without having to use shout or direct tells to gods
void TBeing::doNewbie(const sstring &arg)
{
  sstring message = arg.trim();
  bool isNewbieHelper = isImmortal() || isPlayerAction(PLR_NEWBIEHELP);
  bool isNewbie = (!isNewbieHelper && desc && (time(0)-desc->account->birth) < NEWBIE_PURGATORY_LENGTH);

  if (!isNewbieHelper && !isNewbie)
  {
    sendTo("You cannot use the newbie help channel unless you are a newbie or a newbie helper.\n\r");
    return;
  }

  if (!isImmortal() && !canSpeak())
  {
    sendTo("You are unable to speak at this time and cannot ask newbie questions.\n\r");
    return;
  }

  if (message.empty())
  {
    sendTo("What is your newbie question?\n\r");
    return;
  }

  // trim the string down to 200 chars
  if (message.length() > 200)
  {
    sendTo("Your newbie chat was too long and has been truncated for brevity.\n\r");
    message.resize(200);
  }

  const char *header = isNewbie ? "You ask the experts: %s" : "You advise to newbies: %s";
  const char *title = isNewbie ? "Newbie" : "Expert";
  sendTo(format(header) % colorString(this, desc, message, NULL, COLOR_BASIC, TRUE, TRUE));

  for (Descriptor *d = descriptor_list; d; d = d->next)
  {
    if (d->character == this || d->connected != CON_PLYNG)
      continue;
    if (d->ignored.isIgnored(desc))
      continue;

    TBeing *person = (dynamic_cast<TMonster *>(d->character) && d->original) ? d->original : d->character;
    if (!person)
      continue;
    bool newbieHelper = person->isImmortal() || person->isPlayerAction(PLR_NEWBIEHELP);
    bool newbie = (!newbieHelper && (time(0)-d->account->birth) < NEWBIE_PURGATORY_LENGTH);

    if (!newbieHelper && !newbie)
      continue;

    sstring str = colorString(this, d, message, NULL, COLOR_COMM, FALSE);
    str.convertStringColor("<c>");
    act((format("%s%s $n: %s%s%s") % d->purple() % title % d->cyan() % str % d->norm()), 0, this, 0, person, TO_VICT);

    // hack: newbie channel looks like a telepathy chat for sneezyclient
    if (!d->m_bIsClient && IS_SET(d->prompt_d.type, PROMPT_CLIENT_PROMPT))
      d->clientf(format("%d|%s|%s") % CLIENT_TELEPATHY % colorString(person, d, (format("%s %s") % title % getName()), NULL, COLOR_NONE, FALSE) % str);
  }
  return;
}
