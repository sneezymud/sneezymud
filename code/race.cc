//////////////////////////////////////////////////////////////////////////
//
// SneezyMUD - All rights reserved, SneezyMUD Coding Team
//
// race.cc
//
//////////////////////////////////////////////////////////////////////////

#include "stdsneezy.h"
#include "race.h"
#include "obj_player_corpse.h"
#include "obj_corpse.h"


const char * RaceNames[MAX_RACIAL_TYPES] = {
  "RACE_NORACE", "RACE_HUMAN", "RACE_ELVEN", "RACE_DWARF", "RACE_HOBBIT",
  "RACE_GNOME", "RACE_OGRE",

/* end of player races */

  "RACE_PEGASUS", "RACE_LYCANTH", "RACE_DRAGON", "RACE_UNDEAD", "RACE_ORC",
  "RACE_INSECT", "RACE_ARACHNID", "RACE_DINOSAUR", "RACE_FISH","RACE_BIRD",
  "RACE_GIANT", "RACE_BIRDMAN", "RACE_PARASITE","RACE_SLIME", "RACE_DEMON",
  "RACE_SNAKE", "RACE_HIPPOPOTAMUS", "RACE_TREE", "RACE_VEGGIE", "RACE_ELEMENT",
  "RACE_ANT", "RACE_DEVIL", "RACE_FROGMAN", "RACE_GOBLIN", "RACE_TROLL",
  "RACE_ANGEL", "RACE_MFLAYER", "RACE_PRIMATE", "RACE_FAERIE", "RACE_DROW",
  "RACE_GOLEM", "RACE_BANSHEE", "RACE_PANTATH", "RACE_MERMAID", "RACE_RODENT",
  "RACE_FISHMAN", "RACE_TYTAN", "RACE_WOODELF", "RACE_FELINE", "RACE_CANINE",
  "RACE_HORSE", "RACE_AMPHIB", "RACE_VAMPIRE", "RACE_REPTILE", "RACE_UNCERT",
  "RACE_VAMPIREBAT", "RACE_OCTOPUS", "RACE_CRUSTACEAN", "RACE_MOSS", "RACE_BOVINE",
  "RACE_GOAT", "RACE_SHEEP", "RACE_DEER", "RACE_BEAR", "RACE_WEASEL",
  "RACE_SQUIRREL", "RACE_RABBIT", "RACE_BADGER", "RACE_OTTER", "RACE_BEAVER",
  "RACE_PIG", "RACE_BOAR", "RACE_TURTLE", "RACE_GIRAFFE", "RACE_CENTIPEDE",
  "RACE_MOUND", "RACE_PIERCER", "RACE_ORB", "RACE_MANTICORE", "RACE_GRIFFON",
  "RACE_SPHINX", "RACE_SHEDU", "RACE_LAMMASU", "RACE_DJINN", "RACE_PHOENIX",
  "RACE_DRAGONNE", "RACE_HIPPOGRIFF", "RACE_RUST_MON", "RACE_LION",
  "RACE_TIGER", "RACE_LEOPARD", "RACE_COUGAR", "RACE_FROG", "RACE_ELEPHANT",
  "RACE_RHINO", "RACE_NAGA", "RACE_OTYUGH", "RACE_OX", "RACE_GREMLIN",
  "RACE_OWLBEAR", "RACE_CHIMERA", "RACE_SATYR", "RACE_DRYAD", "RACE_BUGBEAR",
  "RACE_MINOTAUR", "RACE_GORGON", "RACE_KOBOLD", "RACE_BASILISK",
  "RACE_LIZARD_MAN", "RACE_CENTAUR", "RACE_GOPHER", "RACE_LAMIA",
  "RACE_SAHUAGIN", "RACE_BAT", "RACE_PYGMY", "RACE_WYVERN", "RACE_KUOTOA",
  "RACE_BAANTA", "RACE_GNOLL", "RACE_HOBGOBLIN", "RACE_MIMIC", "RACE_MEDUSA",
  "RACE_PENGUIN", "RACE_OSTRICH", "RACE_TROG", "RACE_COATL", "RACE_SIMAL",
  "RACE_WYVELIN", "RACE_FLYINSECT", "RACE_RATMAN",
};

const char * const Lores[MAX_LORES] =
{
  "ANIMAL",
  "VEGGIE",
  "DIABOLIC",
  "REPTILE",
  "UNDEAD",
  "GIANT",
  "PEOPLE",
  "OTHER"
};

const char * const talents[MAX_TALENTS] =
{
  "fast regen",
};

Race *Races[MAX_RACIAL_TYPES];

// listRaces() prints out a formatted list of all the races with their index
// as part of the 'show race' command.
void listRaces(TBeing *caller)
{
  if (!caller->desc)
    return;

  char buf[MAX_STRING_LENGTH];
  *buf = '\0';

  sprintf(buf + strlen(buf), "Valid Races\n\r");
  sprintf(buf + strlen(buf), "-----------\n\r");
  for(race_t race = RACE_NORACE; race < MAX_RACIAL_TYPES; race++) {

    // Format to 3 columns.
    sprintf(buf + strlen(buf), "%3d %-15s%s", race, RaceNames[race], (race%3 ? "\t" : "\n\r"));

  }
  caller->desc->page_string(buf);
}

// Constructors.  A race can be initialized either with no argument or with
// a specified race.  Theoretically, it should always be called with an
// argument so there is no ambiguity, but if it is, it defaults to human.
Race::Race(race_t aRace) :
  corpse_const(0.2),
  singular_name("NoName"),
  plural_name("NoNames"),
  proper_name("A_NoName"),
  raceType(aRace),
  Kingdom(LORE_OTHER),
  talents(0),
  baseAge(0),
  ageNumDice(0),
  ageDieSize(0),
  baseMaleHeight(0),
  maleHtNumDice(0),
  maleHtDieSize(0),
  baseMaleWeight(0),
  maleWtNumDice(0),
  maleWtDieSize(0),
  baseFemaleHeight(0),
  femaleHtNumDice(0),
  femaleHtDieSize(0),
  baseFemaleWeight(0),
  femaleWtNumDice(0),
  femaleWtDieSize(0),
  hpMod(0),
  moveMod(0),
  manaMod(0),
  racialCharacteristics(0),
  searchMod(0),
  lineOfSightMod(0),
  visionBonus(0),
  drinkMod(1.0),
  foodMod(1.0),
  moveMessageIn("has arrived"),
  moveMessageOut("leaves"),
  bodyType(BODY_HUMANOID),
  naturalImmunities(),
  baseStats(),
  tDissectItem()
{
  initRace(RaceNames[aRace]);   // reads in the appropriate race file.
}

Race::Race() :
  corpse_const(0.2),
  singular_name("NoName"),
  plural_name("NoNames"),
  proper_name("A_NoName"),
  raceType(RACE_NORACE),
  Kingdom(LORE_OTHER),
  talents(0),
  baseAge(0),
  ageNumDice(0),
  ageDieSize(0),
  baseMaleHeight(0),
  maleHtNumDice(0),
  maleHtDieSize(0),
  baseMaleWeight(0),
  maleWtNumDice(0),
  maleWtDieSize(0),
  baseFemaleHeight(0),
  femaleHtNumDice(0),
  femaleHtDieSize(0),
  baseFemaleWeight(0),
  femaleWtNumDice(0),
  femaleWtDieSize(0),
  hpMod(0),
  moveMod(0),
  manaMod(0),
  racialCharacteristics(0),
  searchMod(0),
  lineOfSightMod(0),
  visionBonus(0),
  drinkMod(1.0),
  foodMod(1.0),
  moveMessageIn("has arrived"),
  moveMessageOut("leaves"),
  bodyType(BODY_HUMANOID),
  naturalImmunities(),
  baseStats()
{
  initRace("RACE_HUMAN");
}

Race::Race(const Race &a) :
  corpse_const(a.corpse_const),
  singular_name(a.singular_name),
  plural_name(a.plural_name),
  proper_name(a.proper_name),
  raceType(a.raceType),
  Kingdom(a.Kingdom),
  talents(a.talents),
  baseAge(a.baseAge),
  ageNumDice(a.ageNumDice),
  ageDieSize(a.ageDieSize),
  baseMaleHeight(a.baseMaleHeight),
  maleHtNumDice(a.maleHtNumDice),
  maleHtDieSize(a.maleHtDieSize),
  baseMaleWeight(a.baseMaleWeight),
  maleWtNumDice(a.maleWtNumDice),
  maleWtDieSize(a.maleWtDieSize),
  baseFemaleHeight(a.baseFemaleHeight),
  femaleHtNumDice(a.femaleHtNumDice),
  femaleHtDieSize(a.femaleHtDieSize),
  baseFemaleWeight(a.baseFemaleWeight),
  femaleWtNumDice(a.femaleWtNumDice),
  femaleWtDieSize(a.femaleWtDieSize),
  hpMod(a.hpMod),
  moveMod(a.moveMod),
  manaMod(a.manaMod),
  racialCharacteristics(a.racialCharacteristics),
  searchMod(a.searchMod),
  lineOfSightMod(a.lineOfSightMod),
  visionBonus(a.visionBonus),
  drinkMod(a.drinkMod),
  foodMod(a.foodMod),
  moveMessageIn(a.moveMessageIn),
  moveMessageOut(a.moveMessageOut),
  bodyType(a.bodyType),
  naturalImmunities(a.naturalImmunities),
  baseStats(a.baseStats)
{
}

Race & Race::operator=(const Race &a)
{
  if (this == &a) return *this;

  corpse_const = a.corpse_const;
  singular_name = a.singular_name;
  plural_name = a.plural_name;
  proper_name = a.proper_name;
  raceType = a.raceType;
  Kingdom = a.Kingdom;
  talents = a.talents;
  baseAge = a.baseAge;
  ageNumDice = a.ageNumDice;
  ageDieSize = a.ageDieSize;
  baseMaleHeight = a.baseMaleHeight;
  maleHtNumDice = a.maleHtNumDice;
  maleHtDieSize = a.maleHtDieSize;
  baseMaleWeight = a.baseMaleWeight;
  maleWtNumDice = a.maleWtNumDice;
  maleWtDieSize = a.maleWtDieSize;
  baseFemaleHeight = a.baseFemaleHeight;
  femaleHtNumDice = a.femaleHtNumDice;
  femaleHtDieSize = a.femaleHtDieSize;
  baseFemaleWeight = a.baseFemaleWeight;
  femaleWtNumDice = a.femaleWtNumDice;
  femaleWtDieSize = a.femaleWtDieSize;
  hpMod = a.hpMod;
  moveMod = a.moveMod;
  manaMod = a.manaMod;
  racialCharacteristics = a.racialCharacteristics;
  searchMod = a.searchMod;
  lineOfSightMod = a.lineOfSightMod;
  visionBonus = a.visionBonus;
  drinkMod = a.drinkMod;
  foodMod = a.foodMod;
  moveMessageIn = a.moveMessageIn;
  moveMessageOut = a.moveMessageOut;
  bodyType = a.bodyType;
  naturalImmunities = a.naturalImmunities;
  baseStats = a.baseStats;

  return *this;
}

Race::~Race()
{
}

// initRace() takes in a race name and finds the appropriate file in
// /mud/code/lib/races, parses each line and initializes the object
// with the information.  Specific keywords are searched for, so the
// file can contain generic notes that won't affect initialization.

void Race::initRace(const char *whichRace)
{
  char keyword[256], value[256];
  char buf[256];
  char aFilename[256];
  FILE * raceFile;

  sprintf(aFilename, "races/%s", whichRace);
  vlogf(LOG_FILE, fmt("initRace races/%s") %  whichRace);

  raceFile = fopen(aFilename, "r");
  if (!raceFile) {
    sprintf(aFilename, "races/RACE_HUMAN");
    raceFile = fopen(aFilename, "r");
  }
  mud_assert(raceFile != NULL, "No default race file");

  // Basically we just start looking for keywords and then assign values
  // into the appropriate data member.
  // COSMO STRING
  sstring buf_sstring;
  const char *buf2;
  
  while (fgets(buf, 256, raceFile)) {
    buf2 = one_argument(buf, keyword);
    strcpy(buf, buf2);
    if (!keyword || !*keyword || *keyword == '#')
      continue;

    buf_sstring = buf;
    size_t end_whitespace = buf_sstring.find_last_of("\n");
    if (end_whitespace != sstring::npos)
      buf_sstring.erase(end_whitespace);
    trimString(buf_sstring);

    //Names
    if (!strcasecmp(keyword, "singname")) {
      singular_name = buf_sstring;
    } else if (!strcasecmp(keyword, "plurname")) {
      plural_name = buf_sstring;
    } else if (!strcasecmp(keyword, "propname")) {
      proper_name = buf_sstring;
    }

    //Lore
    else if (!strcasecmp(keyword, "lore")) {
      one_argument(buf, value);
      if (!strcasecmp(value, "animal"))
	Kingdom = LORE_ANIMAL;
      else if (!strcasecmp(value, "veggie"))
	Kingdom = LORE_VEGGIE;
      else if (!strcasecmp(value, "diabolic"))
	Kingdom = LORE_DIABOLIC;
      else if (!strcasecmp(value, "reptile"))
	Kingdom = LORE_REPTILE;
      else if (!strcasecmp(value, "undead"))
	Kingdom = LORE_UNDEAD;
      else if (!strcasecmp(value, "giant"))
	Kingdom = LORE_GIANT;
      else if (!strcasecmp(value, "people"))
	Kingdom = LORE_PEOPLE;
      else if (!strcasecmp(value, "other"))
	Kingdom = LORE_OTHER;
      else {
        vlogf(LOG_LOW, fmt("Bad lore %s, defined for %s") %  value % whichRace);
        Kingdom = LORE_PEOPLE;
      }
    }

    // talents
    else if (!strcasecmp(keyword, "talent")) {
      one_argument(buf, value);
      if (!strcasecmp(value, "fast_regen"))
	addToTalents(TALENT_FAST_REGEN);
    }

    // Dimensional stuff.  Used to set individual values later.
    else if (!strcasecmp(keyword,  "age")) {
      if (sscanf(buf, " %d+%dd%d",
         &baseAge, &ageNumDice, &ageDieSize) != 3) {
        vlogf(LOG_LOW, fmt("Bad format for age on %s") %  whichRace);
      }
    }
    else if (!strcasecmp(keyword,  "maleht")) {
      if (sscanf(buf, " %d+%dd%d",
         &baseMaleHeight, &maleHtNumDice, &maleHtDieSize) != 3) {
        vlogf(LOG_LOW, fmt("Bad format for male height on %s") %  whichRace);
      }
    }
    else if (!strcasecmp(keyword,  "femaleht")) {
      if (sscanf(buf, " %d+%dd%d",
         &baseFemaleHeight, &femaleHtNumDice, &femaleHtDieSize) != 3) {
        vlogf(LOG_LOW, fmt("Bad format for female height on %s") %  whichRace);
      }
    }
    else if (!strcasecmp(keyword,  "malewt")) {
      if (sscanf(buf, " %d+%dd%d",
         &baseMaleWeight, &maleWtNumDice, &maleWtDieSize) != 3) {
        vlogf(LOG_LOW, fmt("Bad format for male weight on %s") %  whichRace);
      }
    }
    else if (!strcasecmp(keyword,  "femalewt")) {
      if (sscanf(buf, " %d+%dd%d",
         &baseFemaleWeight, &femaleWtNumDice, &femaleWtDieSize) != 3) {
        vlogf(LOG_LOW, fmt("Bad format for male weight on %s") %  whichRace);
      }
    }
    else if (!strcasecmp(keyword,  "corpse")) {
      if (sscanf(buf, " %f", &corpse_const) != 1) {
        vlogf(LOG_LOW, fmt("Bad format for corpse const on %s") %  whichRace);
      }
    }

    // Point Modifiers
    else if (!strcasecmp(keyword, "hpmod"))
      sscanf(buf, " %d", &hpMod);
    else if (!strcasecmp(keyword, "movemod"))
      sscanf(buf, " %d", &moveMod);
    else if (!strcasecmp(keyword, "manamod"))
      sscanf(buf, " %d", &manaMod);
    else if (!strcasecmp(keyword, "search"))
      sscanf(buf, " %d", &searchMod);
    else if (!strcasecmp(keyword, "sight"))  // adds to scan range
      sscanf(buf, " %d", &lineOfSightMod);
    else if (!strcasecmp(keyword, "vision"))  // adds to eyesight
      sscanf(buf, " %d", &visionBonus);
    else if (!strcasecmp(keyword, "drinkmod"))  // multiplier for drink fills
      sscanf(buf, " %f", &drinkMod);
    else if (!strcasecmp(keyword, "foodmod"))  // multiplier for food fills
      sscanf(buf, " %f", &foodMod);

    // Racial characteristics
    else if (!strcasecmp(keyword, "dumbanimal"))
      racialCharacteristics |= DUMBANIMAL;
    else if (!strcasecmp(keyword, "invertibrate"))
      racialCharacteristics |= BONELESS;
    else if (!strcasecmp(keyword, "winged"))
      racialCharacteristics |= WINGED;
    else if (!strcasecmp(keyword, "canclimb"))
      racialCharacteristics |= CLIMBER;
    else if (!strcasecmp(keyword, "extraplanar"))
      racialCharacteristics |= EXTRAPLANAR;
    else if (!strcasecmp(keyword, "aquatic"))
      racialCharacteristics |= AQUATIC;
    else if (!strcasecmp(keyword, "fourlegged"))
      racialCharacteristics |= FOURLEGGED;
    else if (!strcasecmp(keyword, "coldblooded"))
      racialCharacteristics |= COLDBLOODED;
    else if (!strcasecmp(keyword, "ridable"))
      racialCharacteristics |= RIDABLE;

    // Movement Messages
    else if (!strcasecmp(keyword, "movein"))
      moveMessageIn=buf_sstring;
    else if (!strcasecmp(keyword, "moveout"))
      moveMessageOut=buf_sstring;

    // Body stuff
    else if (!strcasecmp(keyword, "body")) {
      one_argument(buf, value);
      if (!strcasecmp(value, "humanoid"))
        bodyType = BODY_HUMANOID;
      else if (!strcasecmp(value, "otyugh"))
        bodyType = BODY_OTYUGH;
      else if (!strcasecmp(value, "owlbear"))
        bodyType = BODY_OWLBEAR;
      else if (!strcasecmp(value, "minotaur"))
        bodyType = BODY_MINOTAUR;
      else if (!strcasecmp(value, "insectoid"))
        bodyType = BODY_INSECTOID;
      else if (!strcasecmp(value, "piercer"))
        bodyType = BODY_PIERCER;
      else if (!strcasecmp(value, "moss"))
        bodyType = BODY_MOSS;
      else if (!strcasecmp(value, "elemental"))
        bodyType = BODY_ELEMENTAL;
      else if (!strcasecmp(value, "kuotoa"))
        bodyType = BODY_KUOTOA;
      else if (!strcasecmp(value, "crustacean"))
        bodyType = BODY_CRUSTACEAN;
      else if (!strcasecmp(value, "djinn"))
        bodyType = BODY_DJINN;
      else if (!strcasecmp(value, "mermaid"))
        bodyType = BODY_MERMAID;
      else if (!strcasecmp(value, "frogman"))
        bodyType = BODY_FROGMAN;
      else if (!strcasecmp(value, "manticore"))
        bodyType = BODY_MANTICORE;
      else if (!strcasecmp(value, "griffon"))
        bodyType = BODY_GRIFFON;
      else if (!strcasecmp(value, "shedu"))
        bodyType = BODY_SHEDU;
      else if (!strcasecmp(value, "sphinx"))
        bodyType = BODY_SPHINX;
      else if (!strcasecmp(value, "centaur"))
        bodyType = BODY_CENTAUR;
      else if (!strcasecmp(value, "lamia"))
        bodyType = BODY_LAMIA;
      else if (!strcasecmp(value, "lammasu"))
        bodyType = BODY_LAMMASU;
      else if (!strcasecmp(value, "wyvern"))
        bodyType = BODY_WYVERN;
      else if (!strcasecmp(value, "dragonne"))
        bodyType = BODY_DRAGONNE;
      else if (!strcasecmp(value, "hippogriff"))
        bodyType = BODY_HIPPOGRIFF;
      else if (!strcasecmp(value, "chimera"))
        bodyType = BODY_CHIMERA;
      else if (!strcasecmp(value, "dragon"))
        bodyType = BODY_DRAGON;
      else if (!strcasecmp(value, "fish"))
        bodyType = BODY_FISH;
      else if (!strcasecmp(value, "snake"))
        bodyType = BODY_SNAKE;
      else if (!strcasecmp(value, "naga"))
        bodyType = BODY_NAGA;
      else if (!strcasecmp(value, "spider"))
        bodyType = BODY_SPIDER;
      else if (!strcasecmp(value, "centipede"))
        bodyType = BODY_CENTIPEDE;
      else if (!strcasecmp(value, "octopus"))
        bodyType = BODY_OCTOPUS;
      else if (!strcasecmp(value, "bird"))
        bodyType = BODY_BIRD;
      else if (!strcasecmp(value, "bat"))
        bodyType = BODY_BAT;
      else if (!strcasecmp(value, "tree"))
        bodyType = BODY_TREE;
      else if (!strcasecmp(value, "parasite"))
        bodyType = BODY_PARASITE;
      else if (!strcasecmp(value, "slime"))
        bodyType = BODY_SLIME;
      else if (!strcasecmp(value, "orb"))
        bodyType = BODY_ORB;
      else if (!strcasecmp(value, "veggie"))
        bodyType = BODY_VEGGIE;
      else if (!strcasecmp(value, "demon"))
        bodyType = BODY_DEMON;
      else if (!strcasecmp(value, "lion"))
        bodyType = BODY_LION;
      else if (!strcasecmp(value, "feline"))
        bodyType = BODY_FELINE;
      else if (!strcasecmp(value, "wyvelin"))
        bodyType = BODY_WYVELIN;
      else if (!strcasecmp(value, "fourlegs"))
        bodyType = BODY_FOUR_LEG;
      else if (!strcasecmp(value, "reptile"))
        bodyType = BODY_REPTILE;
      else if (!strcasecmp(value, "dinosaur"))
        bodyType = BODY_DINOSAUR;
      else if (!strcasecmp(value, "pig"))
        bodyType = BODY_PIG;
      else if (!strcasecmp(value, "turtle"))
        bodyType = BODY_TURTLE;
      else if (!strcasecmp(value, "fourhoof"))
        bodyType = BODY_FOUR_HOOF;
      else if (!strcasecmp(value, "elephant"))
        bodyType = BODY_ELEPHANT;
      else if (!strcasecmp(value, "baanta"))
        bodyType = BODY_BAANTA;
      else if (!strcasecmp(value, "amphibean"))
        bodyType = BODY_AMPHIBEAN;
      else if (!strcasecmp(value, "frog"))
        bodyType = BODY_FROG;
      else if (!strcasecmp(value, "mimic"))
        bodyType = BODY_MIMIC;
      else if (!strcasecmp(value, "medusa"))
        bodyType = BODY_MEDUSA;
      else if (!strcasecmp(value, "golem"))
        bodyType = BODY_GOLEM;
      else if (!strcasecmp(value, "coatl"))
        bodyType = BODY_COATL;
      else if (!strcasecmp(value, "simal"))
        bodyType = BODY_SIMAL;
      else if (!strcasecmp(value, "pegasus"))
        bodyType = BODY_PEGASUS;
      else if (!strcasecmp(value, "ant"))
        bodyType = BODY_ANT;
      else {
        vlogf(LOG_LOW, fmt("Unknown body on %s") %  whichRace);
        bodyType = BODY_HUMANOID;
      }
    }

#if 1
    // Build the natural immunity list.
    else if (!strncasecmp(keyword, "immune", 6)) {
      naturalImmunities.setImmunity(keyword, convertTo<int>(buf));
    }
#endif

    // Base Statistics
    else if (!strcasecmp(keyword, "str"))
      baseStats.set(STAT_STR, convertTo<int>(buf));
    else if (!strcasecmp(keyword, "bra"))
      baseStats.set(STAT_BRA,convertTo<int>(buf));
    else if (!strcasecmp(keyword, "con"))
      baseStats.set(STAT_CON,convertTo<int>(buf));
    else if (!strcasecmp(keyword, "dex"))
      baseStats.set(STAT_DEX,convertTo<int>(buf));
    else if (!strcasecmp(keyword, "agi"))
      baseStats.set(STAT_AGI,convertTo<int>(buf));
    else if (!strcasecmp(keyword, "int"))
      baseStats.set(STAT_INT,convertTo<int>(buf));
    else if (!strcasecmp(keyword, "wis"))
      baseStats.set(STAT_WIS,convertTo<int>(buf));
    else if (!strcasecmp(keyword, "foc"))
      baseStats.set(STAT_FOC,convertTo<int>(buf));
    else if (!strcasecmp(keyword, "per"))
      baseStats.set(STAT_PER,convertTo<int>(buf));
    else if (!strcasecmp(keyword, "cha"))
      baseStats.set(STAT_CHA,convertTo<int>(buf));
    else if (!strcasecmp(keyword, "kar") )
      baseStats.set(STAT_KAR,convertTo<int>(buf));
    else if (!strcasecmp(keyword, "spe"))
      baseStats.set(STAT_SPE,convertTo<int>(buf));
    else if (!strcasecmp(keyword, "DISSECT_INFO")) {
      if (!tDissectItem[0].loadItem)
        sscanf(buf, " %d %d %d",
               &tDissectItem[0].loadItem,
               &tDissectItem[0].count,
               &tDissectItem[0].amount);
      else
        sscanf(buf, " %d %d %d",
               &tDissectItem[1].loadItem,
               &tDissectItem[1].count,
               &tDissectItem[1].amount);
    } else if (!strcasecmp(keyword, "DISSECT_MSGA")) {
      if (tDissectItem[0].message_to_self.empty())
        tDissectItem[0].message_to_self = buf_sstring;
      else
        tDissectItem[1].message_to_self = buf_sstring;
    } else if (!strcasecmp(keyword, "DISSECT_MSGB")) {
      if (tDissectItem[0].message_to_others.empty())
        tDissectItem[0].message_to_others = buf_sstring;
      else
        tDissectItem[1].message_to_others = buf_sstring;
    }
  }
// COSMO STRING
//  delete buf_sstring;
  fclose(raceFile);
  //  vlogf(LOG_FILE, "Racefile fclose.");

}

// showTo() is called by immortal.cc's doShow command.  It takes a single
// argument which is a pointer to the person who called the show command.

void Race::showTo(TBeing *caller)
{
  char buf[256];
  sstring str;

  if (!caller->desc)
    return;

  str = "Singular Name: ";
  str += singular_name;
  str += "\tPlural Name: ";
  str += plural_name;
  str += "\tProper Name: ";
  str += proper_name;
  str += "\n\r";

  sprintf(buf, "Race is of type %s\n\r", RaceNames[raceType]);
  str += buf;

  sprintf(buf, "Race belongs to the following Lore: %s\n\r", Lores[Kingdom]);
  str += buf;

  sprintf(buf, "Race has body type %s\n\r", bodyNames[bodyType]);
  str += buf;

  sprintf(buf, "Age = %d+%dd%d\n\r", baseAge, ageNumDice, ageDieSize);
  str += buf;

  str += "Male Dimensions:    ";
  sprintf(buf, "Height: %d+%dd%d\tWeight: %d+%dd%d\n\r",
		    baseMaleHeight, maleHtNumDice, maleHtDieSize,
		    baseMaleWeight, maleWtNumDice, maleWtDieSize);
  str += buf;

  str += "Female Dimensions:  ";
  sprintf(buf, "Height: %d+%dd%d\tWeight: %d+%dd%d\n\r",
		    baseFemaleHeight, femaleHtNumDice, femaleHtDieSize,
		    baseFemaleWeight, femaleWtNumDice, femaleWtDieSize);
  str += buf;

  str += "Modifiers:\n\r";
  sprintf(buf, "\tHP:\t%d\tMV:\t%d\tMANA:\t%d\n\r", hpMod, moveMod, manaMod);
  str += buf;

  sprintf(buf, "\tSearch:\t%d\tSight:\t%d\tVision:\t%d\n\r", 
        searchMod, lineOfSightMod, visionBonus);
  str += buf;

  sprintf(buf, "\tDrink:\t%.2f\tFood:\t%.2f\n\r", 
	  	    drinkMod, foodMod);
  str += buf;

  str += sstring("Move In: ") + moveMessageIn + sstring("\n\r");
  str += sstring("Move Out: ") + moveMessageOut + sstring("\n\r");

  str += sstring("Limb for: slash: ") + getBodyLimbSlash();
  str += sstring(", pierce: ") + getBodyLimbPierce();
  str += sstring(", blunt: ") + getBodyLimbBlunt() + sstring("\n\r");

  if(isDumbAnimal())
    str += "CHARACTERISTIC: \tDUMB_ANIMAL\n\r";
  if(hasNoBones())
    str += "CHARACTERISTIC: \tNO_BONES\n\r";
  if(hasMagicFly())
    str += "CHARACTERISTIC: \tMAGICFLY\n\r";
  if(hasNaturalClimb())
    str += "CHARACTERISTIC: \tCLIMBER\n\r";
  if(isHumanoid())
    str += "CHARACTERISTIC: \tHUMANOID\n\r";
  if(isExtraPlanar())
    str += "CHARACTERISTIC: \tEXTRAPLANAR\n\r";
  if(isAquatic())
    str += "CHARACTERISTIC: \tAQUATIC\n\r";
  if(isFourLegged())
    str += "CHARACTERISTIC: \tFOURLEGGED\n\r";
  if(isWinged())
    str += "CHARACTERISTIC: \tWINGED\n\r";
  if(isColdBlooded())
    str += "CHARACTERISTIC: \tCOLDBLOODED\n\r";
  if(isRidable())
    str += "CHARACTERISTIC: \tRIDABLE\n\r";

  // talent handling
  unsigned int i;
  for (i = 0; i < MAX_TALENTS; i++) {
    if (hasTalent(1<<i)) {
      sprintf(buf, "TALENT: \t%s\n\r", ::talents[i]);
      str+= buf;
    }
  }

  // immunities
  immuneTypeT itt;
  for (itt = MIN_IMMUNE; itt < MAX_IMMUNES; itt++) {
    byte val = naturalImmunities.getImmunity(itt);
    if (val) {
      sprintf(buf, "IMMUNITY: \t%s by %d\n\r", immunity_names[itt], val);
      str += buf;
    }
  }

  str += baseStats.showStats(caller);

  caller->desc->page_string(str);
  return;
}

// initNoRace() just dumps in bogus information and zeros out racial
// statistics.  I guess I did this here since two constructors call it.

void Race::initNoRace()
{
}

// generateHeight() calculates a height for a being based on race and the
// numbers read in at initialization for the race.  This is called upon
// character creation.  Hopefully mob creation will call this too.

int Race::generateHeight(sexTypeT sex) const
{
  if (sex == SEX_FEMALE)
    return baseFemaleHeight + (dice(femaleHtNumDice,femaleHtDieSize));
  else
    return baseMaleHeight + (dice(maleHtNumDice,maleHtDieSize));
}
  
// generateWeight() calculates a weight for a being based on race and the
// numbers read in at initialization for the race.  This is called upon
// character creation.  Hopefully mob creation will call this too.

int Race::generateWeight(sexTypeT sex) const
{
  if(sex == SEX_FEMALE)
    return baseFemaleWeight + (dice(femaleWtNumDice,femaleWtDieSize));
  else
    return baseMaleWeight + (dice(maleWtNumDice,maleWtDieSize));
}

// generateAge() calculates an age for a being based on race and the
// numbers read in at initialization for the race.  This is called upon
// character creation.  Hopefully mob creation will call this too.

int Race::generateAge() const
{
  return baseAge + (dice(ageNumDice,ageDieSize));
};

// makeCorpse() should be called by TBeing::makeCorpse().  Right now, it
// doesn't do much other than setting the corpse race.  We may do more with
// it once we figure out how we are going to save corpses for silly mortals.

TCorpse *Race::makeCorpse() const
{
  TCorpse *corpse = new TCorpse();

  corpse->setCorpseRace(getRace());
  return corpse;
}

TPCorpse *Race::makePCorpse() const
{
  TObj *obj = read_object(10558, VIRTUAL);
  TPCorpse *corpse = dynamic_cast<TPCorpse *>(obj);

  if (corpse) {
#if 0
    corpse->setCorpseRace(getRace());
    corpse->setNextGlobal(NULL);
    corpse->setNext(NULL);
    corpse->setPrevious(NULL);
    corpse->setOwner("");
    corpse->setRoomNum(0);
    corpse->setNumInRoom(0);
#endif
  } else {
    corpse = NULL;
    vlogf(LOG_BUG,"Problem in making corpses in makePCorpse");
  }
  return corpse;
}

// getRaceIndex() takes a Race name and generates its index in the
// Races array.

race_t TBeing::getRaceIndex(const char *ccName) const
{
  for(race_t index=RACE_NORACE;index < MAX_RACIAL_TYPES; index++)
    if(!strcmp(ccName, RaceNames[index]))
      return index;
  return RACE_NORACE;
}

race_t TBeing::getRace() const
{
  //  if (is_disguised == TRUE && disguise_race) 
  //  return disguise_race->getRace();
  return race->getRace();
}

Race * TBeing::getMyRace() const
{
  return race;
}

bool TBeing::isSameRace(const TBeing *ch) const
{
  return (getRace() == ch->getRace());
}

const Stats & Race::getBaseStats() const
{
  return baseStats;
}

const sstring Race::getBodyLimbBlunt() const
{
  switch (bodyType) {
    case BODY_AMPHIBEAN:
      return "leg";
    case BODY_VEGGIE:
      return "root";
    case BODY_DEMON:
      return "talon";
    case BODY_TREE:
      return "branch";
    case BODY_SNAKE:
    case BODY_COATL:
      return "tail";
    case BODY_SPIDER:
      return "fore-leg";
    case BODY_DRAGON:
    case BODY_BIRD:
    case BODY_BAT:
    case BODY_WYVERN:
    case BODY_WYVELIN:
      return "wing";
    case BODY_SLIME:
    case BODY_PARASITE:
    case BODY_OCTOPUS:
    case BODY_OTYUGH:
      return "tentacle";
    case BODY_FOUR_LEG:
    case BODY_REPTILE:
    case BODY_FELINE:
    case BODY_PIG:
    case BODY_LION:
    case BODY_TURTLE:
    case BODY_CENTIPEDE:
    case BODY_MANTICORE:
    case BODY_GRIFFON:
    case BODY_SPHINX:
    case BODY_SHEDU:
    case BODY_LAMMASU:
    case BODY_DRAGONNE:
    case BODY_HIPPOGRIFF:
    case BODY_OWLBEAR:
    case BODY_CHIMERA:
    case BODY_MINOTAUR:
      return "paw";
    case BODY_FISH:
      return "fin";
    case BODY_FROGMAN:
      return "webbed hand";
    case BODY_FOUR_HOOF:
    case BODY_PEGASUS:
      return "hoof";
    case BODY_INSECTOID:
    case BODY_ANT:
      return "fore-leg";
    case BODY_DINOSAUR:
    case BODY_FROG:
    case BODY_ELEPHANT:
      return "leg";
    case BODY_MOSS:
      return "appendage";
    case BODY_PIERCER:
    case BODY_ORB:
    case BODY_NAGA:
      return "body";
    case BODY_NONE:
    case BODY_HUMANOID:
    case BODY_ELEMENTAL:
    case BODY_KUOTOA:
    case BODY_CRUSTACEAN:
    case BODY_DJINN:
    case BODY_MERMAID:
    case BODY_CENTAUR:
    case BODY_SIMAL:
    case BODY_LAMIA:
    case BODY_BAANTA:
    case BODY_MIMIC:
    case BODY_MEDUSA:
    case BODY_GOLEM:
    case MAX_BODY_TYPES:
      return "hand";
  }
  return "";
}

const sstring Race::getBodyLimbPierce() const
{
  switch (bodyType) {
    case BODY_DEMON:
    case BODY_REPTILE:
      return "claw";
    case BODY_WYVELIN:
    case BODY_FELINE:
      return "claws";
    case BODY_SPIDER:
    case BODY_PARASITE:
    case BODY_INSECTOID:
      return "stinger";
    case BODY_ANT:
      return "mandible";
    case BODY_BIRD:
    case BODY_GRIFFON:
      return "beak";
    case BODY_SLIME:
      return "tentacle";
    case BODY_OCTOPUS:
      return "suckers";
    case BODY_MOSS:
      return "spores";
    case BODY_PIERCER:
      return "point";
    case BODY_ELEPHANT:
      return "tusk";
    case BODY_NONE:
    case BODY_HUMANOID:
    case BODY_GOLEM:
    case BODY_OTYUGH:
    case BODY_OWLBEAR:
    case BODY_MINOTAUR:
    case BODY_ELEMENTAL:
    case BODY_KUOTOA:
    case BODY_CRUSTACEAN:
    case BODY_DJINN:
    case BODY_MERMAID:
    case BODY_FROGMAN:
    case BODY_MANTICORE:
    case BODY_SHEDU:
    case BODY_SPHINX:
    case BODY_CENTAUR:
    case BODY_SIMAL:
    case BODY_LAMIA:
    case BODY_LAMMASU:
    case BODY_WYVERN:
    case BODY_DRAGONNE:
    case BODY_HIPPOGRIFF:
    case BODY_CHIMERA:
    case BODY_DRAGON:
    case BODY_FISH:
    case BODY_SNAKE:
    case BODY_COATL:
    case BODY_NAGA:
    case BODY_CENTIPEDE:
    case BODY_BAT:
    case BODY_TREE:
    case BODY_ORB:
    case BODY_VEGGIE:
    case BODY_LION:
    case BODY_FOUR_LEG:
    case BODY_DINOSAUR:
    case BODY_PIG:
    case BODY_TURTLE:
    case BODY_FOUR_HOOF:
    case BODY_PEGASUS:
    case BODY_BAANTA:
    case BODY_AMPHIBEAN:
    case BODY_FROG:
    case BODY_MIMIC:
    case BODY_MEDUSA:
    case MAX_BODY_TYPES:
      return "teeth";
  }
  return "";
}

const sstring Race::getBodyLimbSlash() const
{
  switch (bodyType) {
    case BODY_OCTOPUS:
      return "suckers";
    case BODY_FELINE:
    case BODY_WYVELIN:
      return "claws";
    case BODY_MOSS:
      return "appendage";
    case BODY_FOUR_HOOF:
    case BODY_PEGASUS:
    case BODY_ELEPHANT:
      return "horn";
    case BODY_ANT:
      return "tarsal claw";
    case BODY_NONE:
    case BODY_HUMANOID:
    case BODY_GOLEM:
    case BODY_OTYUGH:
    case BODY_OWLBEAR:
    case BODY_MINOTAUR:
    case BODY_INSECTOID:
    case BODY_PIERCER:
    case BODY_ELEMENTAL:
    case BODY_KUOTOA:
    case BODY_CRUSTACEAN:
    case BODY_DJINN:
    case BODY_MERMAID:
    case BODY_FROGMAN:
    case BODY_MANTICORE:
    case BODY_GRIFFON:
    case BODY_SHEDU:
    case BODY_SPHINX:
    case BODY_CENTAUR:
    case BODY_SIMAL:
    case BODY_LAMIA:
    case BODY_LAMMASU:
    case BODY_WYVERN:
    case BODY_DRAGONNE:
    case BODY_HIPPOGRIFF:
    case BODY_CHIMERA:
    case BODY_DRAGON:
    case BODY_FISH:
    case BODY_SNAKE:
    case BODY_COATL:
    case BODY_NAGA:
    case BODY_SPIDER:
    case BODY_CENTIPEDE:
    case BODY_BIRD:
    case BODY_BAT:
    case BODY_TREE:
    case BODY_PARASITE:
    case BODY_SLIME:
    case BODY_ORB:
    case BODY_VEGGIE:
    case BODY_DEMON:
    case BODY_LION:
    case BODY_FOUR_LEG:
    case BODY_REPTILE:
    case BODY_DINOSAUR:
    case BODY_PIG:
    case BODY_TURTLE:
    case BODY_BAANTA:
    case BODY_AMPHIBEAN:
    case BODY_FROG:
    case BODY_MIMIC:
    case BODY_MEDUSA:
    case MAX_BODY_TYPES:
      return "claw";
  }
  return "";
}

race_t & operator++(race_t &c, int)
{
  return c = (c == MAX_RACIAL_TYPES) ? RACE_NORACE : race_t(c+1);
}

const char * Race::moveIn() const
{
  static char buf[256];
  strcpy(buf, moveMessageIn.c_str());
  return buf;
}

const char * Race::moveOut() const
{
  static char buf[256];
  strcpy(buf, moveMessageOut.c_str());
  return buf;
}

race_t Race::getRace() const
{
  return raceType;
}

int Race::getHpMod() const
{
  return hpMod;
}

int Race::getMoveMod() const
{
  return moveMod;
}

int Race::getManaMod() const
{
  return manaMod;
}

float Race::getDrinkMod() const
{
  return drinkMod;
}

void Race::setDrinkMod(float n)
{
  drinkMod = n;
}

float Race::getFoodMod() const
{
  return foodMod;
}

void Race::setFoodMod(float n)
{
  foodMod = n;
}

bool Race::isAnimal() const
{
  return Kingdom == LORE_ANIMAL;
}

bool Race::isVeggie() const
{
  return Kingdom == LORE_VEGGIE;
}

bool Race::isDiabolic() const
{
  return Kingdom == LORE_DIABOLIC;
}

bool Race::isReptile() const
{
  return Kingdom == LORE_REPTILE;
}

bool Race::isUndead() const
{
  return Kingdom == LORE_UNDEAD;
}

bool Race::isGiantish() const
{
  return Kingdom == LORE_GIANT;
}

bool Race::isPeople() const
{
  return Kingdom == LORE_PEOPLE;
}

bool Race::isOther() const
{
  return Kingdom == LORE_OTHER;
}

bool Race::hasNoBones() const
{
  return racialCharacteristics & BONELESS;
}

bool Race::hasMagicFly() const
{
  return racialCharacteristics & MAGICFLY;
}

bool Race::hasNaturalClimb() const
{
  return racialCharacteristics & CLIMBER;
}

int Race::getBaseMaleHeight() const
{
  return baseMaleHeight;
}

int Race::getMaleHtNumDice() const
{
  return maleHtNumDice;
}

int Race::getMaleHtDieSize() const
{
  return maleHtDieSize;
}

sstring Race::getSingularName() const
{
  return singular_name;
}

sstring Race::getPluralName() const
{
  return plural_name;
}

sstring Race::getProperName() const
{
  return proper_name;
}

int Race::getLOS() const
{
  return lineOfSightMod;
}

void Race::setLOS(int x)
{
  lineOfSightMod = x;
}

float Race::getCorpseConst() const
{
  return corpse_const;
}

void Race::setCorpseConst(float n)
{
  corpse_const = n;
}

unsigned int Race::getTalents() const
{
  return talents;
}

void Race::setTalents(unsigned int n)
{ talents = n;
}

bool Race::hasTalent(unsigned int n) const
{
  return talents & n;
}

void Race::addToTalents(unsigned int n)
{
  talents |= n;
}

void Race::remTalent(unsigned int n)
{
  talents &= ~n;
}


const Immunities & Race::getImmunities() const
{
  return naturalImmunities;
}

void Race::setBodyType(body_t body_type)
{
  bodyType = body_type;
}

body_t Race::getBodyType() const
{
  return bodyType;
}

bool Race::isHumanoid() const
{
  return bodyType == BODY_HUMANOID;
}

bool Race::isLycanthrope() const
{
  return raceType == RACE_LYCANTH;
}

bool Race::isExtraPlanar() const
{
  return racialCharacteristics & EXTRAPLANAR;
}

bool Race::isAquatic() const
{
  return racialCharacteristics & AQUATIC;
}

bool Race::isFourLegged() const
{
  return racialCharacteristics & FOURLEGGED;
}

bool Race::isWinged() const
{
  return racialCharacteristics & WINGED;
}

bool Race::isColdBlooded() const
{
  return racialCharacteristics & COLDBLOODED;
}

bool Race::isRidable() const
{
  return racialCharacteristics & RIDABLE;
}

bool Race::isDumbAnimal() const
{
  return racialCharacteristics & DUMBANIMAL;
}


//              hit location (constants.cc)
