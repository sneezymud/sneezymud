//////////////////////////////////////////////////////////////////////////
//
// SneezyMUD - All rights reserved, SneezyMUD Coding Team
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


const int FACTION_CREATE_LEVEL = 25;


const int FACT_LEADER_SLOTS  = 4; // leader = 0, 3 subleaders
const int NUM_MAX_RANK       =10; // 0-9, ranks
const int DEFAULT_RANKS      =3;
const int MAX_FACT_ID        =200; // maximum factions
const int MAX_FACT_COLORS    = 3;

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

// bits for new faction flags / faction rank permissions

const unsigned int FACT_ACTIVE                = (1<<0); 
const unsigned int FACT_LOCKED                = (1<<1);
const unsigned int FACT_OPEN_RECRUITMENT      = (1<<2);
const unsigned int FACT_HIDDEN                = (1<<3);
const unsigned int FACT_HIDE_MEMBERS          = (1<<4);
const unsigned int FACT_HIDE_LEADERS          = (1<<5);
const unsigned int FACT_HIDE_RANKS            = (1<<6);

const unsigned int PERM_RECRUIT               = (1<<0); // R
const unsigned int PERM_PROMOTE               = (1<<1); // P
const unsigned int PERM_TREASURER             = (1<<2); // T
const unsigned int PERM_EDIT                  = (1<<3); // E
const unsigned int PERM_LOCK                  = (1<<4); // L
const unsigned int PERM_AMBASSADOR            = (1<<5); // A
const unsigned int PERM_SCRIBE                = (1<<6); // S

const int RELATION_NONE = 0;
const int RELATION_PEACE = 1;
const int RELATION_WAR = -1;

// new faction class - dash 2001

class TRelation {
 public:
  int targ_fact;
  int relation;
};

class TFaction {
 public:
  char * proper_name;
  int ID;
  char * slang_name;
  char * keywords;
  char * password;
  char * rank[NUM_MAX_RANK];
  unsigned int permissions[NUM_MAX_RANK];
  unsigned int flags;
  int treasury;
  int ranks;
  int alignx;
  int aligny;
  int actx;
  int acty;
  double power;
  int colors[3];
  deityTypeT patron;
  vector<TRelation *>relations;
  
 public:
  int getRelation(int);
  int getRelation(TFaction *);
  void setRelation(int, int);
  void setRelation(TFaction *, int);
  const char * getName() {return (proper_name) ? proper_name : "(null)";}
  const char * getShortName() {return (slang_name) ? slang_name : "(null)";}



  TFaction() {
    int i;
    relations.clear();
    proper_name = NULL;
    slang_name = NULL;
    keywords = NULL;
    password = NULL;

    for(i = 0; i < NUM_MAX_RANK; i++) {
      rank[i] = NULL;
    }
  }
  ~TFaction() {
    int i;
    relations.clear();
    if (proper_name)
      delete [] proper_name;
    if (slang_name)
      delete [] slang_name;
    if (keywords)
      delete [] keywords;
    if (password)
      delete [] password;

    for(i = 0; i < NUM_MAX_RANK; i++) {
      if (rank[i])
	delete [] rank[i];
    }
  }
};

class TFactionInfo {
 public:
  char * faction_name;
  char * leader[FACT_LEADER_SLOTS];
  char * faction_password;
  double faction_array[MAX_FACTIONS][2];
  double faction_power;
  int corp_id;
  double faction_tithe;
  int caravan_interval;
  int caravan_counter;
  unsigned int caravan_flags;
  int caravan_value;
  int caravan_defense;
  int caravan_attempts;
  int caravan_successes;

  int getMoney() const;
  void addToMoney(int);
  void setMoney(int);

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
const char * const NEWFACT_FILE      = "faction/newfactions";
const char * const NEWFACT_BAK       = "faction/newfactions.bak";


const char * const FACT_LIST_BROTHER   = "faction/fact_list.1";
const char * const FACT_LIST_CULT      = "faction/fact_list.2";
const char * const FACT_LIST_SNAKE     = "faction/fact_list.3";

const int FACT_BOARD_BROTHER=1387;
const int FACT_BOARD_SERPENT=8878;
const int FACT_BOARD_LOGRUS =3864;
  
// the amount of faction % help received for donating 1 talen to your faction
const float TITHE_FACTOR   = (0.0003);


enum personTypeT {
    FIRST_PERSON,
    SECOND_PERSON,
    THIRD_PERSON
};

// new faction functions
//extern int add_faction(const char *);
//extern int edit_faction(const char *);
extern int load_newfactions();
extern void save_newfactions();
extern int get_unused_ID();
extern TFaction * get_faction(const char *);
extern TFaction * get_faction_by_ID(int);
extern TFaction * get_faction_by_keywords(const char *);
extern char * display_permission(unsigned int);
extern char * display_faction_flags(unsigned int);
// end new faction functions

extern int load_factions();
extern void save_factions();
extern factionTypeT factionNumber(const sstring);
extern char *get_faction_leader_name(char *);
extern void save_faction_file(TBeing *);
extern void read_faction_file(TBeing *);
extern spellNumT your_deity_val;
extern int bestFactionPower();
extern void recalcFactionPower();
extern void launch_caravans();
extern void sendToFaction(factionTypeT, const char *, const char *);

#endif
