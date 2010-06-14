//////////////////////////////////////////////////////////////////////////
//
// SneezyMUD - All rights reserved, SneezyMUD Coding Team
//
//////////////////////////////////////////////////////////////////////////

#include "extern.h"
#include "low.h"
#include "account.h"
#include "statistics.h"
#include "socket.h"
#include "mail.h"
#include "games.h"
#include "cmd_trophy.h"
#include "database.h"
#include "colorstring.h"

// General arrays used for character creation

// 3 groups, hard-coded, have to add another if more traits are added
const int TRAIT_GROUP_SIZE=MAX_TRAITS/3+(MAX_TRAITS%3>0?1:0); 

// keep this list ordered by point value (for cosmetics)
TTraits traits[MAX_TRAITS+1] = {
  {0,0, "", "", 0,0}, 
  {TOG_IS_COWARD, 10,       "cowardice", 
   "You flee combat if you get below 1/2 hit points.", 0,0},
  {TOG_IS_BLIND, 6,         "blindness",
   "Your vision has been damaged and you are permanently blind.", 0,1},
  {TOG_IS_ASTHMATIC, 6,     "asthma",
   "You have asthma and thus are easily winded.", 0,0},
  {TOG_IS_MUTE, 6,          "mute",
   "Your throatbox has been damaged and you are unable to speak.", 0,1},
  {TOG_IS_NARCOLEPTIC, 3,   "narcolepsy",
   "You have narcolepsy and fall asleep uncontrollably.", 0,0},
  {TOG_IS_COMBUSTIBLE, 3,   "combustible",
   "You are prone to spontaneous combustion.", 0,0},
  {TOG_IS_HEMOPHILIAC, 3,   "hemophilia",
   "You have hemophilia and your wounds do not clot naturally.", 0,0},
  {TOG_IS_NECROPHOBIC, 3,   "necrophobia",
   "You have necrophobia and are terrified at the sight of dead things.", 0,0},
  {TOG_IS_ALCOHOLIC, 3,     "alcoholism",
   "You are an alcoholic and feel a constant urge to drink booze.", 0,0},
  {TOG_HAS_TOURETTES, 1,    "tourettes",
   "You involuntarily insult other people.", 0,1},
  {TOG_PERMA_DEATH_CHAR, 0, "perma-death",
   "You only have one life to live and if you die your game is over.", 0,1},
  {TOG_REAL_AGING, 0,       "real aging",
   "You will suffer the affects of old age as you get older.", 0,1},
  {TOG_IS_HEALTHY, -3,      "healthy",
   "You are particularly healthy and resistant to disease.", 0,0},
  {TOG_IS_AMBIDEXTROUS, -6, "ambidextrous",
   "You are able to use both hands with equal facility.", 0,0},
  {TOG_HAS_NIGHTVISION, -10,"nightvision",
   "You have excellent nightvision.", 0,0},
  {TOG_PSIONICIST, -200,    "psionics",
   "You have innate psionic abilities.", 0,1},
  {TOG_FAE_TOUCHED, 0,    "fae-touched",
   "You gain random bonus stats, but gain experience at half speed.\n\r        (requires one level 50 character of your current race)", 1,1}
};

// the list of available territories for a race
territoryT humanTerr[] = { HOME_TER_HUMAN_URBAN, HOME_TER_HUMAN_VILLAGER, HOME_TER_HUMAN_PLAINS,
                        HOME_TER_HUMAN_RECLUSE, HOME_TER_HUMAN_HILL, HOME_TER_HUMAN_MOUNTAIN,
                        HOME_TER_HUMAN_FOREST, HOME_TER_HUMAN_MARINER };
territoryT elfTerr[] = { HOME_TER_ELF_URBAN, HOME_TER_ELF_TRIBE, HOME_TER_ELF_PLAINS, HOME_TER_ELF_SNOW,
                        HOME_TER_ELF_RECLUSE, HOME_TER_ELF_WOOD, HOME_TER_ELF_SEA };
territoryT dwarfTerr[] = { HOME_TER_DWARF_URBAN, HOME_TER_DWARF_VILLAGER, HOME_TER_DWARF_RECLUSE,
                        HOME_TER_DWARF_HILL, HOME_TER_DWARF_MOUNTAIN };
territoryT gnomeTerr[] = { HOME_TER_GNOME_URBAN, HOME_TER_GNOME_VILLAGER, HOME_TER_GNOME_HILL,
                        HOME_TER_GNOME_SWAMP };
territoryT ogreTerr[] = { HOME_TER_OGRE_VILLAGER, HOME_TER_OGRE_PLAINS, HOME_TER_OGRE_HILL };
territoryT hobbitTerr[] = { HOME_TER_HOBBIT_URBAN, HOME_TER_HOBBIT_SHIRE, HOME_TER_HOBBIT_GRASSLANDS,
                        HOME_TER_HOBBIT_HILL, HOME_TER_HOBBIT_WOODLAND, HOME_TER_HOBBIT_MARITIME };
territoryT goblinTerr[] = { HOME_TER_GOBLIN_URBAN, HOME_TER_GOBLIN_VILLAGER, HOME_TER_GOBLIN_RECLUSE,
                            HOME_TER_GOBLIN_MOUNTAIN, HOME_TER_GOBLIN_FOREST };
territoryT gnollTerr[] = {   HOME_TER_GNOLL_VILLAGER, HOME_TER_GNOLL_PLAINS, HOME_TER_GNOLL_HILL,
                            HOME_TER_GNOLL_FOREST, HOME_TER_GNOLL_MARINER };
territoryT troglodyteTerr[] = { HOME_TER_TROG_VILLAGER, HOME_TER_TROG_PLAINS, HOME_TER_TROG_HILL, HOME_TER_TROG_MOUNTAIN };
territoryT orcTerr[] = { HOME_TER_ORC_URBAN, HOME_TER_ORC_VILLAGER, HOME_TER_ORC_RECLUSE, HOME_TER_ORC_MOUNTAIN,
                          HOME_TER_ORC_FOREST };
territoryT frogmanTerr[] = { HOME_TER_FROGMAN_VILLAGER, HOME_TER_FROGMAN_PLAINS, HOME_TER_FROGMAN_MOUNTAIN,
                              HOME_TER_FROGMAN_FOREST };
territoryT fishmanTerr[] = { HOME_TER_FISHMAN_URBAN, HOME_TER_FISHMAN_VILLAGER, HOME_TER_FISHMAN_RECLUSE,
                        HOME_TER_FISHMAN_MOUNTAIN, HOME_TER_FISHMAN_FOREST, HOME_TER_FISHMAN_MARINER };
territoryT birdmanTerr[] = { HOME_TER_BIRDMAN_URBAN, HOME_TER_BIRDMAN_VILLAGER, HOME_TER_BIRDMAN_MOUNTAIN, 
                          HOME_TER_BIRDMAN_FOREST, HOME_TER_BIRDMAN_MARINER };
territoryT trollTerr[] = { HOME_TER_TROLL_URBAN, HOME_TER_TROLL_VILLAGER, HOME_TER_TROLL_RECLUSE,
                          HOME_TER_TROLL_HILL, HOME_TER_TROLL_MOUNTAIN };

// struct for holding race-specific data
typedef struct _TPlayerRace
{
  race_t race;
  sstring name;
  int num50any;
  territoryT *territories;
  int cTerritories;
  short hometown;
  const sstring terrHelp;
  int disableTrait; // change to array if more are needed
} TPlayerRace;

// our list of playable races
TPlayerRace nannyRaces[] = {
  // base races
  { RACE_HUMAN, "Human", 0, humanTerr, cElements(humanTerr), Room::NEWBIE, "help/territory help human", 0},
  { RACE_GNOME, "Gnome", 0, gnomeTerr, cElements(gnomeTerr), Room::NEWBIE, "help/territory help gnome", 0},
  { RACE_ELVEN, "Elf", 0, elfTerr, cElements(elfTerr), Room::NEWBIE, "help/territory help elf", 0},
  { RACE_OGRE, "Ogre", 0, ogreTerr, cElements(ogreTerr), Room::NEWBIE, "help/territory help ogre", 0},
  { RACE_DWARF, "Dwarf", 0, dwarfTerr, cElements(dwarfTerr), Room::NEWBIE, "help/territory help dwarf", 0},
  { RACE_HOBBIT, "Hobbit", 0, hobbitTerr, cElements(hobbitTerr), Room::NEWBIE, "help/territory help hobbit", 0},

  // extended races
  { RACE_GOBLIN, "Goblin", 0, goblinTerr, cElements(goblinTerr), Room::GH_INN, "help/territory help goblin", 1},
  { RACE_GNOLL, "Gnoll", 0, gnollTerr, cElements(gnollTerr), Room::GH_INN, "help/territory help gnoll", 0},
  { RACE_TROG, "Troglodyte", 0, troglodyteTerr, cElements(troglodyteTerr), Room::GH_INN, "help/territory help troglodyte", 0},
  { RACE_ORC, "Orc", 0, orcTerr, cElements(orcTerr), Room::GH_INN, "help/territory help orc", 1},

  // advanced races
  { RACE_FROGMAN, "Bullywug", 1, frogmanTerr, cElements(frogmanTerr), Room::BULLYWUG_INN, "help/territory help bullywug", 0},
  { RACE_FISHMAN, "Kalysian", 1, fishmanTerr, cElements(fishmanTerr), Room::KALYSIA_INN, "help/territory help kalysian", 0},
  { RACE_BIRDMAN, "Aarakocra", 1, birdmanTerr, cElements(birdmanTerr), Room::AERIE_INN, "help/territory help aarakocra", 0},
  { RACE_TROLL, "Troll", 1, trollTerr, cElements(trollTerr), Room::TROLL_INN, "help/territory help troll", 0},
};

// a struct for holding data about customizable stats
typedef struct _TStatGroup
{
  char shortcut;
  statTypeT stat;
  sstring shortName;
  sstring desc;
} TStatGroup;

// each set of stats is ogranized 3 to a group
TStatGroup combatStats[3] = {
  { 'S', STAT_STR, "(%sS%s)%strength%s       ",
  "(S)trength affects your ability to manipulate weight and your combat damage." },
  { 'B', STAT_BRA, "(%sB%s)%srawn%s          ",
  "(B)rawn affects your ability to wear armor and your hardiness." },
  { 'C', STAT_CON, "(%sC%s)%sonstitution%s   ",
  "(C)onstitution affects your endurance and your life force." }
};

TStatGroup combat2Stats[3] = {
  { 'D', STAT_DEX, "(%sD%s)%sexterity%s      ",
  "(D)exterity affects volume manipulation and offensive skill abilities." },
  { 'A', STAT_AGI, "(%sA%s)%sgility%s        ",
  "(A)gility affects your defensive combat abilities." },
  { 'S', STAT_SPE, "(%sS%s)%speed%s          ",
  "(S)peed affects how fast you are able to do things." }
};

TStatGroup mentalStats[3] = {
  { 'I', STAT_INT, "(%sI%s)%sntelligence%s   ",
  "(I)ntelligence affects your maximum total learning." },
  { 'W', STAT_WIS, "(%sW%s)%sisdom%s         ",
  "(W)isdom affects how fast you will learn your skills." },
  { 'F', STAT_FOC, "(%sF%s)%socus%s          ",
  "(F)ocus affects the success rate of your skills." }
};

TStatGroup utilityStats[3] = {
  { 'P', STAT_PER, "(%sP%s)%serception%s     ",
  "(P)erception affects your abilities to see and evaluate." },
  { 'C', STAT_CHA, "(%sC%s)%sharisma%s       ",
  "(C)harisma affects your ability to lead others including control pets." },
  { 'K', STAT_KAR, "(%sK%s)%sarma%s          ",
  "(K)arma affects your luck." }
};

// holds all of the seperate stat groups together
TStatGroup * statGroups[4] = { combatStats, combat2Stats, mentalStats, utilityStats };
const sstring statGroupNames[4] = { "power", "grace", "mental", "utility" };

// description strings so we can talk about a particular stat and how good the player is
const sstring statDescPos[MAX_STATS] = { "strong", "brawny", "healthy", "coordinated", "graceful", "smart", "wise", "attentive", "observant", "charming", "lucky", "fast", "", "" };
const sstring statDescNeg[MAX_STATS] = { "weak", "feeble", "sickly", "clumsy", "akward", "dumb", "foolish", "distracted", "oblivious", "charmless", "unlucky", "slow", "", "" };
const sstring statAmountAdj[10] = { "", "a tiny bit", "very slightly", "slightly", "somewhat", "fairly", "very", "extremely", "exceptionally", "astoundingly" };

// The nanny states.  Arranged for future alterations without major headaches
typedef struct _TNannyState
{
  connectStateT state;
  int menu;
  connectStateT (*nannyInputFunction)(Descriptor * desc, sstring & output, const sstring input);
  void (*nannyOutputFunction)(Descriptor * desc);
  const sstring helpFile;
} TNannyState;

// definitions for menu entries
#define NANNY_MENU_NONE 0  // just show 'press return to continue'
#define NANNY_MENU_HELP 1  // show help available
#define NANNY_MENU_FRWD 2  // show 'on to next'
#define NANNY_MENU_DPAD 4  // show 'done and goto launchpad'
#define NANNY_MENU_BACK 8  // show 'back a menu'
#define NANNY_MENU_BPAD 16 // show 'back to launchpad'
#define NANNY_MENU_DISC 32 // show 'to disconnect, press ~'
#define NANNY_MENU_PROM 64 // show the prompt -->
#define NANNY_MENU_HNOS 128 // don't show, but allow help

// standardly used menus
#define NANNY_MENU_BASE (NANNY_MENU_DISC|NANNY_MENU_PROM)
#define NANNY_MENU_BASE_B (NANNY_MENU_BASE|NANNY_MENU_BPAD)
#define NANNY_MENU_BASE_H (NANNY_MENU_BASE|NANNY_MENU_HELP)
#define NANNY_MENU_BASE_HB (NANNY_MENU_BASE_H|NANNY_MENU_BPAD)
#define NANNY_MENU_BASE_HBK (NANNY_MENU_BASE_H|NANNY_MENU_BPAD|NANNY_MENU_FRWD)
#define NANNY_MENU_BASE_HK (NANNY_MENU_BASE_H|NANNY_MENU_BACK|NANNY_MENU_FRWD)
#define NANNY_MENU_BASE_HD (NANNY_MENU_BASE_H|NANNY_MENU_BACK|NANNY_MENU_DPAD)
#define NANNY_MENU_BASE_HNOS (NANNY_MENU_BASE|NANNY_MENU_HNOS)

// End General arrays used for character creation

// used for character creation stats
bonusStatPoints::bonusStatPoints() :
  total(0),
  combat(0),
  combat2(0),
  learn(0),
  util(0)
{
}

void bonusStatPoints::clear()
{
  total = combat = combat2 = learn = util = 0;
}

void Descriptor::zeroChosenStats()
{
  character->chosenStats.zero();
}

void Descriptor::setChosenStat(statTypeT stat, int val)
{
  character->chosenStats.values[stat] = val;
}

int Descriptor::totalChosenStats() const
{ 
  return character->chosenStats.total();
}

bool Descriptor::isDefaultChosenStats() const 
{ 
  return character->chosenStats.isDefault();
}

int Descriptor::getChosenStat(statTypeT stat) const 
{ 
  return character->chosenStats.values[stat];
}

int Descriptor::getRacialStat(statTypeT stat) const 
{ 
  return character->race->getBaseStats().values[stat];
}

int Descriptor::getTerritoryStat(statTypeT stat) const 
{ 
  return territory_adjustment(character->player.hometerrain, stat);
}
// end functions used for character creation stats

// *** Begin nanny functions ***

// allows a user to choose a character name
connectStateT nannyName_input(Descriptor * desc, sstring & output, const sstring input)
{
  mud_assert(desc->character != NULL, "Character NULL where it shouldn't be");

  sstring name = input.word(0);
  sstring display;
  char parsed_name[80];

  if (_parse_name(name.c_str(), parsed_name))
    output = "Illegal name, please try another.\n\r";
  else if (desc->checkForCharacter(parsed_name))
    output = "That name is already in use, please try another.\n\r";
  else // the name is set
  {
    desc->character->name = mud_str_dup(sstring(parsed_name).cap());
    return desc->connected == CON_CREATION_NAME ? CON_CREATION_DISCLAIM1: CON_CREATION_LAUNCHPAD;
  }
  return desc->connected;
}

// display for name choices
void nannyName_output(Descriptor * desc)
{
  desc->writeToQ("Please choose a character name.\n\r");
}

// reads our disclaimers for a char
void nannyDisclaimer_output(Descriptor * desc)
{
  const static sstring disclaimers[] = { "objdata/books/disclaimer.1", "objdata/books/disclaimer.2", "objdata/books/disclaimer.3" };
  int whichDis = desc->connected - CON_CREATION_DISCLAIM1;
  sstring str;

  if (whichDis < 0 || whichDis > (int)cElements(disclaimers))
    return;

  if (!file_to_sstring(disclaimers[whichDis].c_str(), str))
    str = format("Error: failed to load file %s.  Please contact an admin.") % disclaimers[whichDis];

  // swap color sstrings
  str = colorString(desc->character, desc, str, NULL, COLOR_BASIC,  false);
  desc->writeToQ(str);
}

// takes Y or N only, discos on N
connectStateT nannyMultiplaywarn_input(Descriptor * desc, sstring & output, const sstring input)
{
  sstring choice = input.upper();
  if (choice == "Y")
  {
    output = "\n\rOk, you have agreed to follow the rules concerning multiplay.\n\r";
    return CON_CREATION_RESET;
  }
  else if (choice == "N")
  {
    // somehow this error message doesnt get displayed.
    output = "\n\rI'm sorry, you MUST agree to the terms and conditions of our rules\n\r";
    output += format("before we allow you to play a character on %s. Please reconsider\n\r") % MUD_NAME;
    output += "and come back soon!\n\r";
    return CON_CREATION_ERROR;
  }
  return desc->connected;
}

// harps at characters about not multiplaying
void nannyMultiplaywarn_output(Descriptor * desc)
{
  desc->writeToQ("\a\n\r");
  desc->writeToQ("*************************************************************************\n\r");
  desc->writeToQ("*  The characters within an account MUST NOT interact with each other,  *\n\r");
  desc->writeToQ("*  aside from sharing equipment and money.  It is prohibited to         *\n\r");
  desc->writeToQ("*  use a character in your account to act as an agent in the retrieval  *\n\r");
  desc->writeToQ("*  of the corpse of another of your characters, or to reduce or         *\n\r");
  desc->writeToQ("*  eliminate a dangerous situation faced by another character in your   *\n\r");
  desc->writeToQ("*  account.  Infractions of this rule WILL RESULT in the ELIMINATION    *\n\r");
  desc->writeToQ("*  OF ALL CHARACTERS INVOLVED.                                          *\n\r");
  desc->writeToQ("*                                                                       *\n\r");
  desc->writeToQ("*  It is expected that you will familiarize yourself with the rules     *\n\r");
  desc->writeToQ("*  detailed in the help files. Be sure to read HELP RULES and all       *\n\r");
  desc->writeToQ("*  pertinent help files listed within.                                  *\n\r");
  desc->writeToQ("*************************************************************************\n\r");
  desc->writeToQ("\n\r\n\r");
  desc->writeToQ("Do you agree to the above terms and conditions regarding the rules? [Y/N]\n\r");
}

// lets a player choose a particular class
// add multiclass support later
// currently, no classes have any race/hometerrain restrictions so this makes it easy
connectStateT nannyClass_input(Descriptor * desc, sstring & output, const sstring input)
{
  if (!input.empty())
  {
    int iChoice = convertTo<int>(input) - 1;
    if (iChoice >= 0 && iChoice < MAX_CLASSES && classInfo[iChoice].enabled)
    {
      desc->character->setClass(classInfo[iChoice].class_num);
      return CON_CREATION_LAUNCHPAD;
    }
  }
  output = "Invalid Choice!";
  return desc->connected;
}

// shows all of the classes available
void nannyClass_output(Descriptor * desc)
{
  sstring sbuf;

  // display choices
  sbuf += "Please pick one of the following choices for your class.\n\r";
  sbuf += "Your current class is marked with an 'X'.\n\r\n\r";

  for(int i=0; i < MAX_CLASSES; ++i)
  {
    if(!classInfo[i].enabled)
      continue;
    sbuf += format("[%c] %2i. %-24s") %
      (desc->character->hasClass(classInfo[i].class_num) ? 'X' : ' ') %
      (i+1) % classInfo[i].name.cap();
    if(i%2)
      sbuf += "\n\r";
  }

  sbuf += "\n\rThere are advantages and disadvantages to each choice.\n\r";
  desc->writeToQ(sbuf);
}

void nannyReset_output(Descriptor * desc)
{
  desc->writeToQ("Applying default settings to this character...\n\rYou may then customize these settings to your liking.\n\r");
}

// clears the character before launchpad
connectStateT nannyReset_input(Descriptor * desc, sstring & output, const sstring input)
{
  // basic stuff
  desc->character->setFaction(FACT_NONE);

  // sex
  desc->character->setSex(SEX_MALE);

  // handed
  desc->character->addPlayerAction(PLR_RT_HANDED);

  // race
  desc->character->setRace(RACE_HUMAN);

  // home terrain
  desc->character->player.hometerrain = HOME_TER_HUMAN_URBAN;

  // class
  desc->character->setClass(classInfo[WARRIOR_LEVEL_IND].class_num);

  // chosen points
  desc->zeroChosenStats();

  // bonus points
  desc->bonus_points.clear();

  // toggles for traits
  for(int iToggle = 0;iToggle < (int)cElements(traits); iToggle++)
    if (desc->character->hasQuestBit(traits[iToggle].tog))
      desc->character->remQuestBit(traits[iToggle].tog);

  return CON_CREATION_LAUNCHPAD;
}

// sets the handedness of the char
connectStateT nannyHand_input(Descriptor * desc, sstring & output, const sstring input)
{
  if (!input.empty())
  {
    int iChoice = convertTo<int>(input);
    if (iChoice == 1)
      desc->character->addPlayerAction(PLR_RT_HANDED);
    else if (iChoice == 2)
      desc->character->remPlayerAction(PLR_RT_HANDED);
    else
      output = "Invalid Choice!";
    if (iChoice == 1 || iChoice == 2)
      return CON_CREATION_LAUNCHPAD;
  }
  return desc->connected;
}

// prints choices for handed-ness
void nannyHand_output(Descriptor * desc)
{
  desc->writeToQ("Now you get to pick your handedness. The hand you pick as your primary\n\r");
  desc->writeToQ("hand will be the strongest, and be able to do more things than your\n\r");
  desc->writeToQ(format("secondary hand.  Your character is currently %shanded.\n\r") %
                  (desc->character->isRightHanded() ? "Right" : "Left"));
  desc->writeToQ("Pick your primary hand:\n\r1. Right\n\r2. Left\n\r");
}

// sets the sex of the char
connectStateT nannySex_input(Descriptor * desc, sstring & output, const sstring input)
{
  if (!input.empty())
  {
    static sexTypeT sexes[] = { SEX_MALE, SEX_FEMALE };
    int iChoice = convertTo<int>(input) - 1;
    if (iChoice >= 0 && iChoice < (int)cElements(sexes))
    {
      desc->character->setSex(sexes[iChoice]);
      return CON_CREATION_LAUNCHPAD;
    }
    output = "Invalid Choice!";
  }
  return desc->connected;
}

// prints choices for sex
void nannySex_output(Descriptor * desc)
{
  desc->writeToQ(format("Please choose your gender.  You are currently %s.\n\r") %
    (desc->character->getSex() == SEX_MALE ? "Male" : "Female"));
  desc->writeToQ("1. Male\n\r2. Female\n\r");
}


// simple function to ensure menu choices are consistient
int setNannyRaces(int *races, int cRaces, int num50s, Descriptor * desc)
{
  int cOut = 0;
  for(int iRace = 0; iRace < (int)cElements(nannyRaces) && cOut < cRaces; iRace++)
    if (num50s >= nannyRaces[iRace].num50any)
      races[cOut++] = iRace;
  return cOut;
}

// prints player races
void nannyRace_output(Descriptor * desc)
{
  desc->writeToQ("Please pick one of the following races.\n\rYour current race selection is marked with an 'X'.\n\r\n\r");

  int num50any = numFifties(RACE_NORACE, false, desc->account->name);
  int races[cElements(nannyRaces)];
  int cRaces = setNannyRaces(races, cElements(races), num50any, desc);
  sstring raceString;

  // print the race choices - races[iRace] is index into nannyRaces
  for(int iRace = 0; iRace < cRaces; iRace++)
  {
    raceString += format("[%c] %2i. %-24s") %
      ((desc->character->getRace() == nannyRaces[races[iRace]].race) ? 'X' : ' ') %
      (iRace+1) % nannyRaces[races[iRace]].name;
    if (iRace % 2)
      raceString += "\n\r";
  }

  desc->writeToQ(raceString);
  desc->writeToQ(format("\n\rThe choice of race is very important on %s.\n\r") % MUD_NAME);
  desc->writeToQ("There are advantages and disadvantages to each choice.\n\r");
  desc->writeToQ("Among other factors, race will majorly affect your stats,\n\r");
  desc->writeToQ("immunities, racial skills, and the supply of available equipment.\n\r");
}

// sets player race
connectStateT nannyRace_input(Descriptor * desc, sstring & output, const sstring input)
{
  if (!input.empty())
  {
    int num50any = numFifties(RACE_NORACE, false, desc->account->name);
    int iChoice = convertTo<int>(input)-1;
    int races[cElements(nannyRaces)];
    int cRaces = setNannyRaces(races, cElements(races), num50any, desc);

    if (iChoice >= 0 && iChoice < cRaces)
    {
      desc->character->setRace(nannyRaces[races[iChoice]].race);
      output = format("Okay Race set to %s.") % nannyRaces[races[iChoice]].name;
      desc->character->player.hometown = nannyRaces[races[iChoice]].hometown;
      desc->character->player.hometerrain = nannyRaces[races[iChoice]].territories[0];
      return (connectStateT)(desc->connected+1);
    }
    output = "Invalid Choice!";
  }
  return desc->connected;
}

// prints a races territories
void nannyTerritory_output(Descriptor * desc)
{
  int iRace;
  sstring terrString;

  for(iRace = 0; iRace < (int)cElements(nannyRaces); iRace++)
    if (desc->character->getRace() == nannyRaces[iRace].race)
      break;
  if (iRace >= (int)cElements(nannyRaces))
    return;

  desc->writeToQ(format("Please pick one of the following homelands for your %s.\n\rYour current homeland selection is marked with an 'X'.\n\r\n\r") % nannyRaces[iRace].name);

  for(int iTerr = 0; iTerr < nannyRaces[iRace].cTerritories; iTerr++)
  {
    terrString += format("[%c] %2i. %-24s") %
      (desc->character->player.hometerrain == nannyRaces[iRace].territories[iTerr] ? 'X' : ' ') %
      (iTerr+1) % home_terrains[nannyRaces[iRace].territories[iTerr]];
    if (iTerr % 2)
      terrString += "\n\r";
  }

  desc->writeToQ(terrString);
  desc->writeToQ("\n\rYour choice of homeland will slightly impact the statistics of your character.\n\r");
}

// sets player territory
connectStateT nannyTerritory_input(Descriptor * desc, sstring & output, const sstring input)
{
  if (!input.empty())
  {
    int iRace = 0;
    int iChoice = convertTo<int>(input)-1;

    for(iRace = 0; iRace < (int)cElements(nannyRaces); iRace++)
      if (desc->character->getRace() == nannyRaces[iRace].race)
        break;
    if (iRace >= (int)cElements(nannyRaces))
      return CON_CREATION_ERROR;

    if (iChoice >= 0 && iChoice < nannyRaces[iRace].cTerritories)
    {
      desc->character->player.hometerrain = nannyRaces[iRace].territories[iChoice];
      output = format("Okay Homeland set to %s.") % home_terrains[nannyRaces[iRace].territories[iChoice]];
      return CON_CREATION_LAUNCHPAD;
    }
    output = "Invalid Choice!";
  }
  return desc->connected;
}

// returns true if the character can be allowed to enter the game with these settings
bool nannyLaunchpad_allowdone(Descriptor * desc, sstring & output)
{
  bool perma = desc->character->hasQuestBit(TOG_PERMA_DEATH_CHAR);
  int num50race = numFifties(desc->character->getRace(), perma, desc->account->name);
  int num50any = numFifties(RACE_NORACE, false, desc->account->name);
  int iRace;

  // set race (yet agian)
  for(iRace = 0; iRace < (int)cElements(nannyRaces); iRace++)
    if (desc->character->getRace() == nannyRaces[iRace].race)
      break;
  mud_assert(desc->character->getRace() == nannyRaces[iRace].race, "Character race isnt set properly!");

  // check for an illegal race
  if (nannyRaces[iRace].num50any > num50any)
  {
    output = format("The %s race requires more level 50 characters than you have.") % nannyRaces[iRace].name;
    return false;
  }

  // check for a trait which you arent qualified for
  for(int iTrait = 1; iTrait < MAX_TRAITS+1; iTrait++)
    if (desc->character->hasQuestBit(traits[iTrait].tog))
      if (traits[iTrait].num50any > num50any)
      {
        output = format("The %s trait requires more level 50 characters than you have.") % traits[iTrait].name;
        return false;
      }
      else if (traits[iTrait].num50race > num50race)
      {
        output = format("The %s trait requires more level 50%s %ss than you have.") %
                  traits[iTrait].name %
                  (perma ? " perma-death" : "") %
                  nannyRaces[iRace].name;
        return false;
      }
      else if (nannyRaces[iRace].disableTrait == iTrait)
      {
        output = format("The %s trait is not allowed for %ss.") % traits[iTrait].name % nannyRaces[iRace].name;
        return false;
      }

  // check for negative stats
  int extra = desc->bonus_points.total - desc->totalChosenStats();
  if (extra < 0)
  {
    output = format("You have over-spent your stat customizations by %d points.") % (-extra);
    return false;
  }

  return true;
}

// this is the main launchplace for customization
connectStateT nannyLaunchpad_input(Descriptor * desc, sstring & output, const sstring input)
{
  if (input == "1")
    return CON_CREATION_RENAME;
  else if (input == "2")
    return CON_CREATION_RACE;
  else if (input == "3")
    return CON_CREATION_CLASS;
  else if (input == "4")
    return CON_CREATION_SEX;
  else if (input == "5")
    return CON_CREATION_HANDS;
  else if (input == "6")
    return CON_CREATION_TRAITS1;
  else if (input == "7")
    return CON_CREATION_CUSTOMIZE_START;
  else if (input.upper() == "C")
    return CON_CREATION_CONFIG_CODE;
  else if (input.upper() == "R")
    return CON_CREATION_RESET;
  else if (input.upper() == "D" && nannyLaunchpad_allowdone(desc, output))
  {
    TDatabase db(DB_SNEEZY);

    desc->character->convertAbilities();
    desc->character->affectTotal();

    if (desc->character->hasQuestBit(TOG_FAE_TOUCHED))
    {
      int num_fifties = numFifties(desc->character->getRace(),
          desc->character->hasQuestBit(TOG_PERMA_DEATH_CHAR),
          desc->character->desc->account->name);
      if (num_fifties > 0)
      {
        num_fifties = min(num_fifties, 26);
        desc->character->addToRandomStat(50+(num_fifties-1)*2);
      }
    }

    vlogf(LOG_PIO, format("%s [%s] new player.") %  desc->character->getName() % desc->host);
    desc->character->saveChar(Room::AUTO_RENT);
    db.query("insert into player (name) values (lower('%s'))", desc->character->getName());
    AccountStats::player_count++;

    return CON_CREATION_DONE;
  }
  return desc->connected;
}

// print out all of the available customizations
void nannyLaunchpad_output(Descriptor * desc)
{
  sstring output;

  // first, ensure that bonus points have been split out into their categories properly
  int *bonusPoints[4] = { &desc->bonus_points.combat, &desc->bonus_points.combat2, &desc->bonus_points.learn, &desc->bonus_points.util };
  int limit = desc->bonus_points.total / 4;
  int remainder = desc->bonus_points.total % 4;
  int increment = remainder > 0 ? 1 : -1;
  for(int iBonus = 0;iBonus < (int)cElements(bonusPoints); iBonus++)
  {
    *bonusPoints[iBonus] = limit;
    if (remainder)
    {
      (*bonusPoints[iBonus]) += increment;
      remainder -= increment;
    }
  }

  // Name
  desc->writeToQ("Below are your current character settings.\n\rChoose a number to customize your character.\n\r");
  desc->writeToQ(format("1. Name               : %-24s\n\r") % desc->character->getName());

  // Race and Homeland
  output = "None";
  for(int iRace = 0; iRace < (int)cElements(nannyRaces); iRace++)
    if (desc->character->getRace() == nannyRaces[iRace].race)
      output = nannyRaces[iRace].name;
  output += '/';
  output += home_terrains[desc->character->player.hometerrain];
  desc->writeToQ(format("2. Race/Homeland      : %-24s\n\r") % output);

  // Class
  desc->writeToQ(format("3. Class              : %-24s\n\r") % desc->character->getProfName());

  // Sex
  output = (desc->character->getSex() == SEX_MALE ? "Male" : "Female");
  desc->writeToQ(format("4. Sex                : %-24s\n\r") % output);

  // Handedness
  output = (desc->character->isRightHanded() ? "Right" : "Left");
  desc->writeToQ(format("5. Primary Hand       : %-24s\n\r") % output);

  // Traits
  output = "";
  for (int iTrait = 1;iTrait < MAX_TRAITS+1; iTrait++)
    if (desc->character->hasQuestBit(traits[iTrait].tog))
    {
      if (!output.empty())
        output += ", ";
      output += traits[iTrait].name;
    }
  if (output.empty())
    output = "None";
  else if (output.length() > 34)
    output = "Many";
  desc->writeToQ(format("6. Traits             : %-24s\n\r") % output);

  // Stat Customizations
  output = "Default";
  int extra = desc->bonus_points.total - desc->totalChosenStats();
  if (extra > 0)
    output = format("%d bonus points unspent") % extra;
  else if (extra < 0)
    output = format("%d penalty points left") % (-extra);
  else if (!desc->isDefaultChosenStats())
    output = "Customized";
  desc->writeToQ(format("7. Stats              : %-24s\n\r") % output);

  // check if we can be done
  if (nannyLaunchpad_allowdone(desc, output))
      desc->writeToQ(format("\n\rChoose %sD%s to complete character creation and enter the game.") % desc->red() % desc->norm());
  else
      desc->writeToQ(format("\n\r%s") % output);
  desc->writeToQ(format("\n\rChoose %sR%s to reset your customizations.\n\r") % desc->red() % desc->norm());
  desc->writeToQ(format("Choose %sC%s to enter a creation code.\n\r") % desc->red() % desc->norm());
}

// print out all of the traits for this trate page
void nannyTraits_output(Descriptor * desc)
{
  sstring buf;
  int group = desc->connected - CON_CREATION_TRAITS1;
  int startval = max(group*TRAIT_GROUP_SIZE+1,0);
  int endval = min(startval+TRAIT_GROUP_SIZE-1, MAX_TRAITS);
  int num50race = numFifties(desc->character->getRace(), desc->character->hasQuestBit(TOG_PERMA_DEATH_CHAR), desc->account->name);
  int num50any = numFifties(RACE_NORACE, false, desc->account->name);
  int iRace;

  buf="You may choose some distinct traits for your character if you wish.\n\r";
  buf+="You will receive bonus (or penalty) points to apply to your statistics.\n\r";
  buf+="Traits you have selected are marked with an 'X', unavailable have a '*'.\n\r\n\r";

  // get our current race
  for(iRace = 0; iRace < (int)cElements(nannyRaces); iRace++)
    if (desc->character->getRace() == nannyRaces[iRace].race)
      break;
  if (iRace >= (int)cElements(nannyRaces))
    return;

  // show traits
  for(int i=startval;i<=endval;++i){
    char check = ' ';
    if (desc->character->hasQuestBit(traits[i].tog))
      check = 'X';
    else if (traits[i].num50any > num50any || traits[i].num50race > num50race || nannyRaces[iRace].disableTrait == i)
      check = '*';
    buf+=format("[%c] %2i. %-12s (%3i points)\n\r        %s\n\r") %
      check % i %
      traits[i].name %
      traits[i].points % traits[i].desc;
  }

  buf+=format("\n\rBonus points          [%3d]\n\r") % desc->bonus_points.total;

  desc->writeToQ(buf);
}

// adds/removes traits
connectStateT nannyTraits_input(Descriptor * desc, sstring & output, const sstring input)
{
  mud_assert(desc->character != NULL, "Character NULL where it shouldn't be");
  int num50race = numFifties(desc->character->getRace(), desc->character->hasQuestBit(TOG_PERMA_DEATH_CHAR), desc->account->name);
  int num50any = numFifties(RACE_NORACE, false, desc->account->name);
  int iTrait = convertTo<int>(input);
  if (input.empty() || iTrait < 1 || iTrait > MAX_TRAITS)
  {
    output = "Invalid Choice!";
  }
  else if (desc->character->hasQuestBit(traits[iTrait].tog))
  {
    desc->character->remQuestBit(traits[iTrait].tog);
    desc->bonus_points.total -= traits[iTrait].points;
  }
  else if (traits[iTrait].num50race > num50race)
  {
    output = format("Invalid Choice! %d L50 character%s needed for %s (race-specific).") %
      traits[iTrait].num50race % (traits[iTrait].num50race > 1 ? "s are" : " is" ) % traits[iTrait].name;
  }
  else if (traits[iTrait].num50any > num50any)
  {
    output = format("Invalid Choice! %d L50 character%s needed for %s.") %
      traits[iTrait].num50any % (traits[iTrait].num50any > 1 ? "s are" : " is" ) % traits[iTrait].name;
  }
  else
  {
    desc->character->setQuestBit(traits[iTrait].tog);
    desc->bonus_points.total += traits[iTrait].points;
  }
  return desc->connected;
}

// rules for stats
void nannyStatRules_output(Descriptor * desc)
{
  sstring output;

  output += format("Welcome to the %s stat customization process.\n\r") % MUD_NAME;
  output += "All changes apply on top of your existing stats given your race and homeland.\n\r";
  output += "REALIZE THAT A ZERO STAT IS THE NORMAL AMOUNT FOR YOU RACE AND HOMELAND.\n\r";
  output += "Your stats are split into four groups: power and grace, mental, and utility.\n\r";
  output += "Each group has 3 characteristics which can have points distributed in the group.\n\r";
  output += "Also, each individual stat may only be modified up or down at most 25 points.\n\r";

  desc->writeToQ(output);
}

// customize stats
void nannyStats_output(Descriptor * desc)
{
  mud_assert(desc->connected >= CON_CREATION_CUSTOMIZE_COMBAT, "nannyStats_output desc->connected is unexpected value");
  mud_assert(desc->connected <= CON_CREATION_CUSTOMIZE_UTIL, "nannyStats_output desc->connected is unexpected value");

  int bonusPoints[4] = { desc->bonus_points.combat, desc->bonus_points.combat2, desc->bonus_points.learn, desc->bonus_points.util };
  int iGroup = desc->connected-CON_CREATION_CUSTOMIZE_COMBAT;
  TStatGroup *stats = statGroups[iGroup];
  const sstring statsName = statGroupNames[iGroup];

  desc->writeToQ(format("Your current %s characteristics are: \n\r\n\r") % statsName);

  sstring output;
  int totalStats = 0;

  // loop over stats
  for (int iStat = 0; iStat < 3; iStat++)
  {
    int statAmount = desc->getChosenStat(stats[iStat].stat);
    int realStatAmount = desc->getRacialStat(stats[iStat].stat) +
                          desc->getTerritoryStat(stats[iStat].stat) +
                          statAmount;
    realStatAmount -= 100;
    const sstring statDesc[MAX_STATS] = realStatAmount < 0 ? statDescNeg : statDescPos;
    int iAdj = max(realStatAmount, -realStatAmount) / cElements(statAmountAdj);

    output += format(stats[iStat].shortName) % desc->cyan() % desc->norm() % desc->cyan() % desc->norm();
    output += format("[%3d] ") % statAmount;

    if (iAdj < 1)
      output += "(about average)";
    else if (iAdj < (int)cElements(statAmountAdj))
      output += format("(%s %s)") % statAmountAdj[iAdj] % statDesc[stats[iStat].stat];
    else
      output += format("(off the chart %s)") % statDesc[stats[iStat].stat];

    output += "\n\r";
    totalStats += statAmount;
  }

  desc->writeToQ(output);
  desc->writeToQ("\n\r");

  // show descriptions
  for (int iStat = 0; iStat < 3; iStat++)
  {
    desc->writeToQ(format("%s\n\r") % stats[iStat].desc);
  }

  // show free points
  desc->writeToQ(format("\n\rYou have %d free %s stat points.\n\r") % (bonusPoints[iGroup]-totalStats) % statsName);
  bool canLeave = (bonusPoints[iGroup]-totalStats) >= 0;

  // show some example or something? for syntax

  // print dynamic menus
  if (!canLeave)
    desc->writeToQ(format("\n\rYou cannot finish customizing %s until you have spent your %d points.") % statsName % (bonusPoints[iGroup]-totalStats));
  desc->writeToQ(format("\n\rType %s?%s to see a help file for help on this choice.") % desc->red() % desc->norm());
  if (canLeave && desc->connected < CON_CREATION_CUSTOMIZE_UTIL)
    desc->writeToQ(format("\n\rType %sC%s to continue on to the next menu.") % desc->red() % desc->norm());
  if (canLeave && desc->connected >= CON_CREATION_CUSTOMIZE_UTIL)
    desc->writeToQ(format("\n\rType %sC%s to finish and go to the main character menu.") % desc->red() % desc->norm());
  if (canLeave && desc->connected > CON_CREATION_CUSTOMIZE_COMBAT)
    desc->writeToQ(format("\n\rType %s/%s to go back to the previous menu.") % desc->red() % desc->norm());
  if (canLeave && desc->connected <= CON_CREATION_CUSTOMIZE_COMBAT)
    desc->writeToQ(format("\n\rType %s/%s to go back to the main character menu.") % desc->red() % desc->norm());
}

// customize stats (this has to use its own custom menu to prevent leaving the stage with negative stats
connectStateT nannyStats_input(Descriptor * desc, sstring & output, const sstring input)
{
  int iGroup = desc->connected-CON_CREATION_CUSTOMIZE_COMBAT;
  int bonusPoints[4] = { desc->bonus_points.combat, desc->bonus_points.combat2, desc->bonus_points.learn, desc->bonus_points.util };
  const sstring statsName = statGroupNames[iGroup];
  TStatGroup *stats = statGroups[iGroup];
  int totalStats = 0;
  for(int iStat = 0; iStat < 3; iStat++)
      totalStats += desc->getChosenStat(stats[iStat].stat);
  bool canLeave = (bonusPoints[iGroup]-totalStats) >= 0;

  if (!canLeave)
    output = format("\n\rYou cannot finish customizing %s until you have spent your %d points.") % statsName % (bonusPoints[iGroup]-totalStats);

  if (!input.empty() && input.length() < 7 && // 7 randomly chosen for sanity
    (input[0] == '+' || input[0] == '-'))
  {
    TStatGroup *stats = statGroups[iGroup];
    int amount = convertTo<int>(input);
    char lastChar = input.upper()[input.length()-1];
    statTypeT stat = STAT_EXT;
    for(int iStat = 0; iStat < 3; iStat++)
      if (lastChar == stats[iStat].shortcut)
        stat = stats[iStat].stat;

    int newChosen = desc->getChosenStat(stat) + amount;

    if (stat == STAT_EXT || amount == 0)
    {
      output = "Invalid Choice!";
    }
    else if (newChosen > 25 || newChosen < -25)
    {
      output = "You cannot adjust your stat customizations by more than +/- 25 points.";
    }
    else
    {
      output = format("Adjusting your %s stat by %d.") % statsName % amount;
      desc->setChosenStat(stat, newChosen);
    }
  }
  else if (canLeave && input.upper() == "C")
    return (desc->connected == CON_CREATION_CUSTOMIZE_UTIL) ? CON_CREATION_LAUNCHPAD : (connectStateT)(desc->connected+1);
  else if (canLeave && input.upper() == "/")
    return (desc->connected == CON_CREATION_CUSTOMIZE_COMBAT) ? CON_CREATION_LAUNCHPAD : (connectStateT)(desc->connected-1);

  return desc->connected;
}

// finally, what a player sees then they are done
void nannyDone_output(Descriptor * desc)
{
  sstring output = "Congratulations, you have finished the character creation process.\n\r";
  output += format("If you are a newcomer to %s, %stake a minute to read this screen%s.\n\r\n\r") % MUD_NAME % desc->orange() % desc->norm();

  output += "Upon connecting, you will want to check your initial terminal options.\n\r";
  output += "Know that the game will automatically set some of these options for you.\n\r";
  output += "These include: prompts, toggles, terminal size, color.\n\r";
  output += "Your initial settings are just defaults and you can change them easily.\n\r";
  output += format("Good help files to read are <%sTOGGLE%s>, <%sPROMPT%s> and <%sCOLOR%s>.\n\r") % desc->orange() % desc->norm() % desc->orange() % desc->norm() % desc->orange() % desc->norm();
  output += format("You should also %sread the newbie guide%s and %swear your equipment%s.\n\r") % desc->orange() % desc->norm() %desc-> orange() % desc->norm();

  output += "For further orientation, use the help system, newbie helpers and immortal staff.\n\r";
  output += "Check the SneezyMUD website's Help section for more: http://www.sneezymud.com\n\r";
  output += "In contacting immortals, be aware that our immortal staff is not\n\r";
  output += "allowed to help you discover The World.  However, they are allowed\n\r";
  output += "and encouraged to help you with command problems and general orientation.\n\r\n\r";

  output += "You may also wish to check out our web site, forums and newsletter.\n\r";
  output += format("The staff of %s hope that you enjoy your stay.\n\r") % MUD_NAME;
  
  desc->writeToQ(output);
}

// the last stage we go to motd to login
connectStateT nannyDone_input(Descriptor * desc, sstring & output, const sstring input)
{
  desc->sendMotd(FALSE);

  return CON_RMOTD;
}




int rbits(int a, int b)
{
  int ret=0;

  for(int i=0;i<b;++i){
    if(a & 1<<i){
      ret = ret | 1<<((b-1)-i);
    }
  }
  return ret;
}

connectStateT nannyCode_input(Descriptor * desc, sstring & output, const sstring input)
{
  if(input.length()==24){
    sstring buf="";
    unsigned long stats3=strtoul(input.substr(0,6).c_str(), NULL, 36);
    unsigned long stats2=strtoul(input.substr(6,6).c_str(), NULL, 36);
    unsigned long stats1=strtoul(input.substr(12,6).c_str(), NULL, 36);
    unsigned long other=strtoul(input.substr(18,6).c_str(), NULL, 36);

    nannyRace_input(desc, buf, format("%i") % GET_BITS_CORRECT(other, 3, 4));
    output += buf + "\n\r";
    buf="";

    nannyTerritory_input(desc, buf, format("%i") % GET_BITS_CORRECT(other, 6, 3));
    output += buf + "\n\r";
    buf="";
    
    nannyClass_input(desc, buf, format("%i") % GET_BITS_CORRECT(other, 9, 3));
    output += buf + "\n\r";
    buf="";

    nannySex_input(desc, buf, format("%i") % (GET_BITS_CORRECT(other, 10, 1)+1));
    output += buf + "\n\r";
    buf="";

    nannyHand_input(desc, buf, format("%i") % (GET_BITS_CORRECT(other, 11, 1)+1));
    output += buf + "\n\r";
    buf="";    
    
    for(int i=12;i<28;++i){
      if(GET_BITS_CORRECT(other, i, 1)){
        nannyTraits_input(desc, buf, format("%i") % (i-11));
	      output += buf + "\n\r";
	      buf="";
      }
    }

    desc->connected=CON_CREATION_CUSTOMIZE_COMBAT;
    nannyStats_input(desc, buf, format("%+is") % (GET_BITS_CORRECT(stats1, 5, 6)-25));
    nannyStats_input(desc, buf, format("%+ib") % (GET_BITS_CORRECT(stats1, 11, 6)-25));
    nannyStats_input(desc, buf, format("%+ic") % (GET_BITS_CORRECT(stats1, 17, 6)-25));

    desc->connected=CON_CREATION_CUSTOMIZE_COMBAT2;
    nannyStats_input(desc, buf, format("%+id") % (GET_BITS_CORRECT(stats1, 23, 6)-25));
    nannyStats_input(desc, buf, format("%+ia") % (GET_BITS_CORRECT(stats2, 5, 6)-25));
    nannyStats_input(desc, buf, format("%+is") % (GET_BITS_CORRECT(stats3, 23, 6)-25));

    desc->connected=CON_CREATION_CUSTOMIZE_LEARN;
    nannyStats_input(desc, buf, format("%+ii") % (GET_BITS_CORRECT(stats2, 11, 6)-25));
    nannyStats_input(desc, buf, format("%+iw") % (GET_BITS_CORRECT(stats2, 17, 6)-25));
    nannyStats_input(desc, buf, format("%+if") % (GET_BITS_CORRECT(stats2, 23, 6)-25));

    desc->connected=CON_CREATION_CUSTOMIZE_UTIL;
    nannyStats_input(desc, buf, format("%+ip") % (GET_BITS_CORRECT(stats3, 5, 6)-25));
    nannyStats_input(desc, buf, format("%+ic") % (GET_BITS_CORRECT(stats3, 11, 6)-25));
    nannyStats_input(desc, buf, format("%+ik") % (GET_BITS_CORRECT(stats3, 17, 6)-25));
  }
  return CON_CREATION_LAUNCHPAD;
}

void nannyCode_output(Descriptor * desc)
{
  desc->writeToQ("Please enter your creation code.\n\r");
}


// common menu options for all of our character creation menus
void nannyCommonMenu(Descriptor * desc, int menu)
{
  if (menu == NANNY_MENU_NONE)
  {
    desc->writeToQ("\n\r[Press Return to continue]\n\r");
    return;
  }
  if (menu & NANNY_MENU_HELP)
    desc->writeToQ(format("\n\rType %s?%s to see a help file for help on this choice.") % desc->red() % desc->norm());
  if (menu & NANNY_MENU_FRWD)
    desc->writeToQ(format("\n\rType %sC%s to continue on to the next menu.") % desc->red() % desc->norm());
  if (menu & NANNY_MENU_DPAD)
    desc->writeToQ(format("\n\rType %sC%s to finish and go to the main character menu.") % desc->red() % desc->norm());
  if (menu & NANNY_MENU_BACK)
    desc->writeToQ(format("\n\rType %s/%s to go back to the previous menu.") % desc->red() % desc->norm());
  if (menu & NANNY_MENU_BPAD)
    desc->writeToQ(format("\n\rType %s/%s to go back to the main character menu.") % desc->red() % desc->norm());
  if (menu & NANNY_MENU_DISC)
    desc->writeToQ(format("\n\rType %s~%s to disconnect.") % desc->red() % desc->norm());
  if (menu & NANNY_MENU_PROM)
    desc->writeToQ("\n\r\n\r--> ");
}

// Note: this array should match the entries from CON_CREATION_START to CON_CREATION_MAX
// It should only be used by creation_nanny
TNannyState creationNannyData[CON_CREATION_MAX-CON_CREATION_START] = {

  // beginning junk - 4 pages of legal garbage and name choosing
  { CON_CREATION_NAME, NANNY_MENU_BASE, nannyName_input, nannyName_output, "" },
  { CON_CREATION_DISCLAIM1, NANNY_MENU_NONE, NULL, nannyDisclaimer_output, "" },
  { CON_CREATION_DISCLAIM2, NANNY_MENU_NONE, NULL, nannyDisclaimer_output, "" },
  { CON_CREATION_DISCLAIM3, NANNY_MENU_NONE, NULL, nannyDisclaimer_output, "" },
  { CON_CREATION_MULTIWARN, NANNY_MENU_PROM, nannyMultiplaywarn_input, nannyMultiplaywarn_output, "" },
  { CON_CREATION_RESET, NANNY_MENU_NONE, nannyReset_input, nannyReset_output, "" },

  // the launchpad - all customizations start from here
  { CON_CREATION_LAUNCHPAD, NANNY_MENU_BASE, nannyLaunchpad_input, nannyLaunchpad_output, "" },

  // redo name
  { CON_CREATION_RENAME, NANNY_MENU_BASE_B, nannyName_input, nannyName_output, "" },

  // customize sex
  { CON_CREATION_SEX, NANNY_MENU_BASE_HB, nannySex_input, nannySex_output, "help/gender" },

  // customize hand (and you're gonna need it)
  { CON_CREATION_HANDS, NANNY_MENU_BASE_HB, nannyHand_input, nannyHand_output, "help/handedness" },

  // customize race & homeland
  { CON_CREATION_RACE, NANNY_MENU_BASE_HBK, nannyRace_input, nannyRace_output, "help/races overview" },
  { CON_CREATION_HOMETERRAIN, NANNY_MENU_BASE_HD, nannyTerritory_input, nannyTerritory_output, "+" },

  // customize class
  { CON_CREATION_CLASS, NANNY_MENU_BASE_HB, nannyClass_input, nannyClass_output, "help/classes overview" },

  // customize traits
  { CON_CREATION_TRAITS1, NANNY_MENU_BASE_HBK, nannyTraits_input, nannyTraits_output, "help/traits overview" },
  { CON_CREATION_TRAITS2, NANNY_MENU_BASE_HK, nannyTraits_input, nannyTraits_output, "help/traits overview" },
  { CON_CREATION_TRAITS3, NANNY_MENU_BASE_HD, nannyTraits_input, nannyTraits_output, "help/traits overview" },

  // customize stats
  { CON_CREATION_CUSTOMIZE_START, NANNY_MENU_NONE, NULL, nannyStatRules_output, "" },
  { CON_CREATION_CUSTOMIZE_COMBAT, NANNY_MENU_BASE_HNOS, nannyStats_input, nannyStats_output, "help/characteristics overview" },
  { CON_CREATION_CUSTOMIZE_COMBAT2, NANNY_MENU_BASE_HNOS, nannyStats_input, nannyStats_output, "help/characteristics overview" },
  { CON_CREATION_CUSTOMIZE_LEARN, NANNY_MENU_BASE_HNOS, nannyStats_input, nannyStats_output, "help/characteristics overview" },
  { CON_CREATION_CUSTOMIZE_UTIL, NANNY_MENU_BASE_HNOS, nannyStats_input, nannyStats_output, "help/characteristics overview" },

  // finally done
  { CON_CREATION_DONE, NANNY_MENU_NONE, nannyDone_input, nannyDone_output, "" },

  { CON_CREATION_CONFIG_CODE, NANNY_MENU_BASE, nannyCode_input, nannyCode_output, "" },
};


// returns DELETE_THIS if descriptor is to be deleted
int Descriptor::creation_nanny(sstring arg)
{
  // the format goes: they type stuff, it goes here, we spit back out some text
  // eventually, one of these functions spits out CON_RMOTD and we can be done

  if (connected < CON_CREATION_START || connected >= CON_CREATION_MAX)
    return DELETE_THIS; //something went wrong here

  // any processing that needs to happen before every character creation state goes here

  // process the state input
  sstring output;
  sstring input = arg.trim().upper().word(0);
  TNannyState *nannyState = &(creationNannyData[connected-CON_CREATION_START]);

  // process regular menu input
  if (input == "~" && nannyState->menu & NANNY_MENU_DISC)
    connected = CON_CREATION_ERROR;
  else if (input == "/" && nannyState->menu & NANNY_MENU_BPAD)
    connected = CON_CREATION_LAUNCHPAD;
  else if (input == "/" && nannyState->menu & NANNY_MENU_BACK)
    connected = (connectStateT)(connected-1);
  else if (input == "C" && nannyState->menu & NANNY_MENU_FRWD)
    connected = (connectStateT)(connected+1);
  else if (input == "C" && nannyState->menu & NANNY_MENU_DPAD)
    connected = CON_CREATION_LAUNCHPAD;
  else if (input == "?" && (nannyState->menu & NANNY_MENU_HELP || nannyState->menu & NANNY_MENU_HNOS))
  {
    const sstring *phelpFile = &(nannyState->helpFile);
    if (*phelpFile == "+") // SPECIAL CASE: helpfile is a lookup for a race territory file
      for(int iRace = 0; iRace < (int)cElements(nannyRaces); iRace++)
        if (nannyRaces[iRace].race == character->getRace())
          phelpFile = &(nannyRaces[iRace].terrHelp);

    file_to_sstring(phelpFile->c_str(), output);
    output = colorString(character, this, output, NULL, COLOR_BASIC,  false);
    character->cls();
    character->fullscreen();
    page_string(output, SHOWNOW_YES);
    return 0;
  }
  else if (nannyState->nannyInputFunction == NULL)
    connected = (connectStateT)(connected+1);
  else
    connected = nannyState->nannyInputFunction(this, output, arg.trim());

  // any processing that needs to happen after every character creation state goes here

  // clear the screen for display, showing input results
  character->cls();
  character->fullscreen();
  if (!output.empty())
    writeToQ(output);
  writeToQ("\n\r");

  // error return
  if (connected == CON_CREATION_ERROR)
    return DELETE_THIS;
  // we've dropped out of char creation
  else if (connected < CON_CREATION_START || connected >= CON_CREATION_MAX)
    return 0;

  // print new choices
  creationNannyData[connected-CON_CREATION_START].nannyOutputFunction(this);

  // print common menu choices
  nannyCommonMenu(this, creationNannyData[connected-CON_CREATION_START].menu);

  // continue as normal
  return 0;
}

