//////////////////////////////////////////////////////////////////////////
//
// SneezyMUD - All rights reserved, SneezyMUD Coding Team
//
//////////////////////////////////////////////////////////////////////////

#pragma once

// some faction code is bad  (faction%) without fully enabled factions
#define FACTIONS_IN_USE 0

#include "spells.h"

class sstring;
class TBeing;

const int FACT_LEADER_SLOTS = 4;  // leader = 0, 3 subleaders

enum factionTypeT {
  FACT_UNDEFINED = -1,
  FACT_NONE = 0,  // this is unaffiliated
  FACT_BROTHERHOOD,
  FACT_CULT,
  FACT_SNAKE,
  MAX_FACTIONS,
};
extern factionTypeT& operator++(factionTypeT& c, int);
const factionTypeT MIN_FACTION = factionTypeT(0);

// DO NOT CHANGE.  affects charFile
const factionTypeT ABS_MAX_FACTION = factionTypeT(6);

enum offTypeT {
  OFF_HURT,
  OFF_HELP
};

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
  DEITY_SHROUD,
  DEITY_LESPRIT,
  DEITY_TALANA,
  DEITY_SALUREL,
  MAX_DEITIES
};

// caravan flags
const unsigned int CARAVAN_DEST_GH = (1 << 0);
const unsigned int CARAVAN_DEST_BM = (1 << 1);
const unsigned int CARAVAN_DEST_LOG = (1 << 2);
const unsigned int CARAVAN_DEST_AMBER = (1 << 3);
const unsigned int CARAVAN_CUR_DEST_GH = (1 << 4);
const unsigned int CARAVAN_CUR_DEST_BM = (1 << 5);
const unsigned int CARAVAN_CUR_DEST_LOG = (1 << 6);
const unsigned int CARAVAN_CUR_DEST_AMBER = (1 << 7);

const unsigned short CAR_GH_HOME = 432;
const unsigned short CAR_BM_HOME = 1395;
const unsigned short CAR_LOG_HOME = 3768;
const unsigned short CAR_AMBER_HOME = 8713;
// const unsigned short CAR_AMBER_HOME   = 3076;

const int MIN_CARAVAN_INTERVAL = 10;
const int CARAVAN_TRADE = 100;

class TFactionInfo {
  public:
    char* faction_name{nullptr};
    char* leader[FACT_LEADER_SLOTS]{nullptr};
    char* faction_password{nullptr};
    double faction_array[MAX_FACTIONS][2]{{0.0}};
    double faction_power{0.0};
    int corp_id{0};
    double faction_tithe{0.0};
    int caravan_interval{0};
    int caravan_counter{0};
    unsigned int caravan_flags{0};
    int caravan_value{0};
    int caravan_defense{0};
    int caravan_attempts{0};
    int caravan_successes{0};

    int getMoney() const;
    void addToMoney(int);
    void setMoney(int);

    TFactionInfo() = default;
    TFactionInfo(const TFactionInfo&) = delete;
    TFactionInfo& operator=(const TFactionInfo&) = delete;
    TFactionInfo(TFactionInfo&&) = delete;
    TFactionInfo& operator=(TFactionInfo&&) = delete;
    ~TFactionInfo() {
      delete[] faction_name;
      delete[] faction_password;
      for (char* l : leader)
        delete[] l;
    }
};

extern TFactionInfo FactionInfo[MAX_FACTIONS];
extern double avg_faction_power;
extern const char* CaravanDestination(int);

const char* const FACTION_FILE = "faction/faction_info";
const char* const FACTION_BAK = "faction/faction_info.bak";

const int FACT_BOARD_BROTHER = 1387;
const int FACT_BOARD_SERPENT = 8878;
const int FACT_BOARD_LOGRUS = 3864;

// the amount of faction % help received for donating 1 talen to your faction
const float TITHE_FACTOR = (0.0003);

enum personTypeT {
  FIRST_PERSON,
  SECOND_PERSON,
  THIRD_PERSON
};

extern int load_factions();
extern void save_factions();
extern factionTypeT factionNumber(const sstring);
extern char* get_faction_leader_name(char*);
extern void save_faction_file(TBeing*);
extern void read_faction_file(TBeing*);
extern spellNumT your_deity_val;
extern int bestFactionPower();
extern void recalcFactionPower();
extern void launch_caravans();
extern void sendToFaction(factionTypeT, TBeing*, const char*);
