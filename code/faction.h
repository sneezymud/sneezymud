//////////////////////////////////////////////////////////////////////////
//
// SneezyMUD - All rights reserved, SneezyMUD Coding Team
//
// $Log: faction.h,v $
// Revision 5.1  1999/10/16 04:31:17  batopr
// new branch
//
// Revision 1.1  1999/09/12 17:24:04  sneezy
// Initial revision
//
//
//////////////////////////////////////////////////////////////////////////


#ifndef __FACTION_H
#define __FACTION_H

enum factionTypeT {
     FACT_UNDEFINED = -1,
     FACT_NONE = 0,    // this is unaffiliated
     FACT_BROTHERHOOD,
     FACT_CULT,
     FACT_SNAKE,
     MAX_FACTIONS,
};
extern factionTypeT & operator++(factionTypeT &c, int);
const factionTypeT MIN_FACTION = factionTypeT(0);

// DO NOT CHANGE.  affects charFile
const factionTypeT ABS_MAX_FACTION    = factionTypeT(6);

enum offTypeT {
    OFF_HURT,
    OFF_HELP
};

const int FACT_LEADER_SLOTS  = 4; // leader = 0, 3 subleaders

// caravan flags
const unsigned int CARAVAN_DEST_GH            = (1<<0);
const unsigned int CARAVAN_DEST_BM            = (1<<1);
const unsigned int CARAVAN_DEST_LOG           = (1<<2);
const unsigned int CARAVAN_DEST_AMBER         = (1<<3);
const unsigned int CARAVAN_CUR_DEST_GH        = (1<<4);
const unsigned int CARAVAN_CUR_DEST_BM        = (1<<5);
const unsigned int CARAVAN_CUR_DEST_LOG       = (1<<6);
const unsigned int CARAVAN_CUR_DEST_AMBER     = (1<<7);

const unsigned short CAR_GH_HOME      = 432;
const unsigned short CAR_BM_HOME      = 1395;
const unsigned short CAR_LOG_HOME     = 3768;
const unsigned short CAR_AMBER_HOME   = 8713;
// const unsigned short CAR_AMBER_HOME   = 3076;

const int MIN_CARAVAN_INTERVAL        = 10;
const int CARAVAN_TRADE               = 100;

class TFactionInfo {
  public:
  char * faction_name;
  char * leader[FACT_LEADER_SLOTS];
  char * faction_password;
  double faction_array[MAX_FACTIONS][2];
  double faction_power;
  long faction_wealth;
  double faction_tithe;
  int caravan_interval;
  int caravan_counter;
  unsigned int caravan_flags;
  int caravan_value;
  int caravan_defense;
  int caravan_attempts;
  int caravan_successes;

  TFactionInfo() {
    int i;
    faction_name = NULL;
    faction_password= NULL;
    for (i = 0; i < FACT_LEADER_SLOTS; i++)
      leader[i] = NULL;
  }
  ~TFactionInfo() {
    int i;
    delete [] faction_name;
    delete [] faction_password;
    for (i = 0; i < FACT_LEADER_SLOTS; i++)
      delete [] leader[i];
  }
};

extern TFactionInfo FactionInfo[MAX_FACTIONS];
extern double avg_faction_power;
extern const char * CaravanDestination(int);

const char * const FACTION_FILE      = "faction/faction_info";
const char * const FACTION_BAK       = "faction/faction_info.bak";

const char * const FACT_LIST_BROTHER   = "faction/fact_list.1";
const char * const FACT_LIST_CULT      = "faction/fact_list.2";
const char * const FACT_LIST_SNAKE     = "faction/fact_list.3";
  
// the amount of faction % help received for donating 1 talen to your faction
const float TITHE_FACTOR   = (0.0003);

enum deityTypeT {
     DEITY_NONE,
     DEITY_MEZAN,
     DEITY_LUNA,
     DEITY_SASUKEY,
     DEITY_SINSUKEY,
     DEITY_ICON,
     DEITY_ELYON,
     DEITY_JEVON,
     DEITY_OMNON,
     DEITY_MENANON,
     DEITY_AMANA,
     DEITY_MALSHYRA,
     DEITY_SHROUD ,
     DEITY_LESPRIT,
     DEITY_TALANA ,
     DEITY_SALUREL,
     MAX_DEITIES
};

enum personTypeT {
    FIRST_PERSON,
    SECOND_PERSON,
    THIRD_PERSON
};

extern int load_factions();
extern void save_factions();
extern factionTypeT factionNumber(const char *);
extern char *get_faction_leader_name(char *);
extern void save_faction_file(TBeing *);
extern void read_faction_file(TBeing *);
extern spellNumT your_deity_val;
extern int bestFactionPower();
extern void recalcFactionPower();
extern void launch_caravans();
extern void sendToFaction(factionTypeT, const char *, const char *);

#endif
