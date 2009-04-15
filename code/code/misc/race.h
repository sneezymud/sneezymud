//////////////////////////////////////////////////////////////////////////
//
// SneezyMUD - All rights reserved, SneezyMUD Coding Team
//
// race.h
//
// The basic design for the handling of Races is the Flyweight Pattern as
// explained in Design Patterns: Elements of Reusable Object-Oriented Software
// ISBN 0-201-63361-2
//
//////////////////////////////////////////////////////////////////////////

#ifndef __RACE_H
#define __RACE_H

#ifndef __BODY_H
#include "body.h"
#endif

#include "cmd_dissect.h"
#include "immunity.h"
#include "stats.h"

// forward declarations
class TCorpse;

enum race_t {
/* Race -- Npc, otherwise */
  RACE_NORACE,          /*   0 */       RACE_HUMAN,     /*   1 */
  RACE_ELVEN,           /*   2 */       RACE_DWARF,     /*   3 */
  RACE_HOBBIT,          /*   4 */       RACE_GNOME,     /*   5 */
  RACE_OGRE,            /*   6 */

/* end of basic player races */

  RACE_PEGASUS,         /*   7 */       RACE_LYCANTH,   /*   8 */
  RACE_DRAGON,          /*   9 */       RACE_UNDEAD,    /*  10 */
  RACE_ORC,             /*  11 */       RACE_INSECT,    /*  12 */
  RACE_ARACHNID,        /*  13 */       RACE_DINOSAUR,  /*  14 */
  RACE_FISH,            /*  15 */       RACE_BIRD,      /*  16 */
  RACE_GIANT,           /*  17 */       RACE_BIRDMAN,   /*  18 */
  RACE_PARASITE,        /*  19 */       RACE_SLIME,     /*  20 */
  RACE_DEMON,           /*  21 */       RACE_SNAKE,     /*  22 */
  RACE_HIPPOPOTAMUS,    /*  23 */       RACE_TREE,      /*  24 */
  RACE_VEGGIE,          /*  25 */       RACE_ELEMENT,   /*  26 */
  RACE_ANT,             /*  27 */       RACE_DEVIL,     /*  28 */
  RACE_FROGMAN,         /*  29 */       RACE_GOBLIN,    /*  30 */
  RACE_TROLL,           /*  31 */       RACE_ANGEL,     /*  32 */
  RACE_MFLAYER,         /*  33 */       RACE_PRIMATE,   /*  34 */
  RACE_FAERIE,          /*  35 */       RACE_DROW,      /*  36 */
  RACE_GOLEM,           /*  37 */       RACE_BANSHEE,   /*  38 */
  RACE_PANTATH,         /*  39 */       RACE_MERMAID,   /*  40 */
  RACE_RODENT,          /*  41 */       RACE_FISHMAN,   /*  42 */
  RACE_TYTAN,           /*  43 */       RACE_WOODELF,   /*  44 */
  RACE_FELINE,          /*  45 */       RACE_CANINE,    /*  46 */
  RACE_HORSE,           /*  47 */       RACE_AMPHIB,    /*  48 */
  RACE_VAMPIRE,         /*  49 */       RACE_REPTILE,   /*  50 */
  RACE_UNCERT,          /*  51 */       RACE_VAMPIREBAT,/*  52 */
  RACE_OCTOPUS,         /*  53 */       RACE_CRUSTACEAN,/*  54 */
  RACE_MOSS,            /*  55 */       RACE_BOVINE,    /*  56 */
  RACE_GOAT,            /*  57 */       RACE_SHEEP,     /*  58 */
  RACE_DEER,            /*  59 */       RACE_BEAR,      /*  60 */
  RACE_WEASEL,          /*  61 */       RACE_SQUIRREL,  /*  62 */
  RACE_RABBIT,          /*  63 */       RACE_BADGER,    /*  64 */
  RACE_OTTER,           /*  65 */       RACE_BEAVER,    /*  66 */
  RACE_PIG,             /*  67 */       RACE_BOAR,      /*  68 */
  RACE_TURTLE,          /*  69 */       RACE_GIRAFFE,   /*  70 */
  RACE_CENTIPEDE,       /*  71 */       RACE_MOUND,     /*  72 */
  RACE_PIERCER,         /*  73 */       RACE_ORB,       /*  74 */
  RACE_MANTICORE,       /*  75 */       RACE_GRIFFON,   /*  76 */
  RACE_SPHINX,          /*  77 */       RACE_SHEDU,     /*  78 */
  RACE_LAMMASU,         /*  79 */       RACE_DJINN,     /*  80 */
  RACE_PHOENIX,         /*  81 */       RACE_DRAGONNE,  /*  82 */
  RACE_HIPPOGRIFF,      /*  83 */       RACE_RUST_MON,  /*  84 */
  RACE_LION,            /*  85 */       RACE_TIGER,     /*  86 */
  RACE_LEOPARD,         /*  87 */       RACE_COUGAR,    /*  88 */
  RACE_FROG,            /*  89 */       RACE_ELEPHANT,  /*  90 */
  RACE_RHINO,           /*  91 */       RACE_NAGA,      /*  92 */
  RACE_OTYUGH,          /*  93 */       RACE_OX,        /*  94 */
  RACE_GREMLIN,         /*  95 */       RACE_OWLBEAR,   /*  96 */
  RACE_CHIMERA,         /*  97 */       RACE_SATYR,     /*  98 */
  RACE_DRYAD,           /*  99 */       RACE_BUGBEAR,   /* 100 */
  RACE_MINOTAUR,        /* 101 */       RACE_GORGON,    /* 102 */
  RACE_KOBOLD,          /* 103 */       RACE_BASILISK,  /* 104 */
  RACE_LIZARD_MAN,      /* 105 */       RACE_CENTAUR,   /* 106 */
  RACE_GOPHER,          /* 107 */       RACE_LAMIA,     /* 108 */
  RACE_SAHUAGIN,        /* 109 */       RACE_BAT,       /* 110 */
  RACE_PYGMY,           /* 111 */       RACE_WYVERN,    /* 112 */
  RACE_KUOTOA,          /* 113 */       RACE_BAANTA,    /* 114 */
  RACE_GNOLL,           /* 115 */       RACE_HOBGOBLIN, /* 116 */
  RACE_MIMIC,           /* 117 */       RACE_MEDUSA,    /* 118 */
  RACE_PENGUIN,         /* 119 */       RACE_OSTRICH,   /* 120 */
  RACE_TROG,            /* 121 */       RACE_COATL,     /* 122 */
  RACE_SIMAL,           /* 123 */       RACE_WYVELIN,   /* 124 */
  RACE_FLYINSECT,       /* 125 */       RACE_RATMEN,    /* 126 */
  // see remove list in oldrace.cc before adding here

  MAX_RACIAL_TYPES
};
extern race_t & operator++(race_t &, int);

// This Class will be used to define a template of sorts for the various races
// which exist in Sneezy.  Races should be defined in a flat file in
// /mud/code/lib/races.

const int MAX_NAMELENGTH = 15;
extern const char *RaceNames[MAX_RACIAL_TYPES];

const unsigned int DUMBANIMAL  = (1<<0);
const unsigned int BONELESS    = (1<<1);
const unsigned int WINGED      = (1<<2);
const unsigned int CLIMBER     = (1<<3);
const unsigned int EXTRAPLANAR = (1<<4);
const unsigned int AQUATIC     = (1<<5);
const unsigned int FOURLEGGED  = (1<<6);
const unsigned int COLDBLOODED = (1<<7);
const unsigned int RIDABLE     = (1<<8);
const unsigned int MAGICFLY    = (1<<9);
const unsigned int FEATHERED   = (1<<10);

enum lore_t {
  LORE_ANIMAL,
  LORE_VEGGIE,
  LORE_DIABOLIC,
  LORE_REPTILE,
  LORE_UNDEAD,
  LORE_GIANT,
  LORE_PEOPLE,
  LORE_OTHER,
  MAX_LORES
};

extern const char * const Lores[MAX_LORES];

void listRaces(TBeing *caller);

const unsigned int TALENT_FAST_REGEN     = (1<<0);
const unsigned int TALENT_FISHEATER      = (1<<1);
const unsigned int TALENT_MEATEATER      = (1<<2);
const unsigned int TALENT_TATTOOED       = (1<<3);
const unsigned int TALENT_GARBAGEEATER   = (1<<4);
const unsigned int TALENT_LIMB_REGROWTH  = (1<<5);
const unsigned int TALENT_INSECT_EATER   = (1<<6);
const unsigned int TALENT_FROGSLIME_SKIN = (1<<7);
const unsigned int TALENT_MUSK           = (1<<8);

const unsigned int MAX_TALENTS = 9;  // move and change

extern const char * const talents[MAX_TALENTS];

// these are regular toggles from toggle.h, but ever being from that race automatically gets it
const unsigned int MAX_RACIAL_TOGGLES = 3;

class Race {
  friend class TBeing;
  friend class TPCorpse;
  friend class Stats;

public:
  Race(race_t aRace);
  Race(const Race &);
  Race & operator=(const Race &);
  ~Race();
private:
  Race();
  void initRace(const char *whichRace);
  void initNoRace();

public:
  void showTo(TBeing *caller);

  void setBodyType(body_t body_type);
  body_t getBodyType() const;

  race_t getRace() const;
  int getHpMod() const;
  int getMoveMod() const;
  int getManaMod() const;

  float getDrinkMod() const;
  void setDrinkMod(float n);
  float getFoodMod() const;
  void setFoodMod(float n);

  const Stats & getBaseStats() const;

  bool isAnimal() const;
  bool isVeggie() const;
  bool isDiabolic() const;
  bool isReptile() const;
  bool isUndead() const;
  bool isGiantish() const;
  bool isPeople() const;
  bool isOther() const;

  bool hasNoBones() const;
  bool hasMagicFly() const;
  bool hasNaturalClimb() const;

  bool isHumanoid() const;
  bool isLycanthrope() const;
  bool isExtraPlanar() const;
  bool isAquatic() const;
  bool isFourLegged() const;
  bool isWinged() const;
  bool isColdBlooded() const;
  bool isRidable() const;
  bool isDumbAnimal() const;
  bool isFeathered() const;

  const char *moveIn() const;
  const char *moveOut() const;

  int getBaseMaleHeight() const;
  int getMaleHtNumDice() const;
  int getMaleHtDieSize() const;
  int getMinWeight(sexTypeT) const;
  int getMaxWeight(sexTypeT) const;
  sstring getSingularName() const;
  sstring getPluralName() const;
  sstring getProperName() const;
  int getLOS() const;
  void setLOS(int x);

  int generateAge() const;
  int generateHeight(sexTypeT sex) const;
  int generateWeight(sexTypeT sex) const;

  TCorpse *makeCorpse() const;
  TPCorpse *makePCorpse() const;

  float getCorpseConst() const;
  void setCorpseConst(float n);

  const sstring getBodyLimbBlunt() const;
  const sstring getBodyLimbPierce(TBeing *) const;
  const sstring getBodyLimbSlash() const;

  unsigned int getTalents() const;
  void setTalents(unsigned int n);
  bool hasTalent(unsigned int n) const;
  void addToTalents(unsigned int n);
  void remTalent(unsigned int n);

  const Immunities & getImmunities() const;
  
  int getBaseArmor() const; // gets the basic armor class of this race (usually 1000)

  int getGarbles() const; // gets the garbles which members of this race have by default

  void applyToggles(TBeing *character) const; // applies toggles common to all members of this race to character

private:
  float corpse_const;
  sstring singular_name;
  sstring plural_name;
  sstring proper_name;
  race_t raceType;
  lore_t Kingdom;
  unsigned int talents;
  unsigned int garbles;
  unsigned int toggles[MAX_RACIAL_TOGGLES];
  unsigned int cToggles;

  // Dimensions base + NumDice d DieSize
  int baseAge;		int ageNumDice;		int ageDieSize;
  int baseMaleHeight;	int maleHtNumDice;	int maleHtDieSize;
  int baseMaleWeight;	int maleWtNumDice;	int maleWtDieSize;
  int baseFemaleHeight;	int femaleHtNumDice;	int femaleHtDieSize;
  int baseFemaleWeight;	int femaleWtNumDice;	int femaleWtDieSize;

  // Point modifiers
  int hpMod;
  int moveMod;
  int manaMod;

  // Racial Characteristics - bit vector
  // DUMBANIMAL, BONELESS, WINGED, CLIMBER, EXTRAPLANAR
  // AQUATIC, FOURLEGGED, COLDBLOODED, RIDABLE, MAGICFLY
  unsigned short racialCharacteristics;

  // Visibility stuff
  int searchMod;
  int lineOfSightMod;
  int visionBonus;

  // consumption stuff
  float drinkMod;
  float foodMod;

  // Movement messages
  sstring moveMessageIn;
  sstring moveMessageOut;

  // Limb Information
  body_t bodyType;
  //Limb *body;

  // Immunities
  Immunities naturalImmunities;

  // Base Stats
  Stats baseStats;

   public:
  // Dissection Information
  dissectInfo tDissectItem[2];
}; 

extern Race *Races[MAX_RACIAL_TYPES];

#endif
