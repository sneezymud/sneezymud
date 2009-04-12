#ifndef __GUILD_H
#define __GUILD_H


const int GUILD_CREATE_LEVEL = 25;

const int NUM_MAX_RANK       =10; // 0-9, ranks
const int DEFAULT_RANKS      =3;
const int MAX_GUILD_ID        =200;
const int MAX_GUILD_COLORS    = 3;

const unsigned int GUILD_ACTIVE                = (1<<0); 
const unsigned int GUILD_LOCKED                = (1<<1);
const unsigned int GUILD_OPEN_RECRUITMENT      = (1<<2);
const unsigned int GUILD_HIDDEN                = (1<<3);
const unsigned int GUILD_HIDE_MEMBERS          = (1<<4);
const unsigned int GUILD_HIDE_LEADERS          = (1<<5);
const unsigned int GUILD_HIDE_RANKS            = (1<<6);

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


class TRelation {
 public:
  int targ_fact;
  int relation;
};

class TGuild {
 public:
  char * proper_name;
  int ID;
  char * slang_name;
  char * keywords;
  char * password;
  char * rank[NUM_MAX_RANK];
  unsigned int permissions[NUM_MAX_RANK];
  unsigned int flags;
  factionTypeT faction_affiliation;
  int corp_id;
  int treasury;
  int ranks;
  int alignx;
  int aligny;
  int actx;
  int acty;
  double power;
  int colors[3];
  deityTypeT patron;
  std::vector<TRelation *>relations;
  
 public:
  int getRelation(int);
  int getRelation(TGuild *);
  void setRelation(int, int);
  void setRelation(TGuild *, int);
  const char * getName() {return (proper_name) ? proper_name : "(null)";}
  const char * getShortName() {return (slang_name) ? slang_name : "(null)";}



  TGuild() {
    int i;
    relations.clear();
    proper_name = NULL;
    slang_name = NULL;
    keywords = NULL;
    password = NULL;

    faction_affiliation=FACT_NONE;

    for(i = 0; i < NUM_MAX_RANK; i++) {
      rank[i] = NULL;
    }
  }
  ~TGuild() {
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

const char * const GUILD_FILE      = "faction/newfactions";
const char * const GUILD_BAK       = "faction/newfactions.bak";


extern int load_guilds();
extern void save_guilds();
extern int get_unused_ID();
extern TGuild * get_guild(const char *);
extern TGuild * get_guild_by_ID(int);
extern TGuild * get_guild_by_keywords(const char *);
extern bool remove_guild(const char *);
extern bool remove_guild_by_ID(int);
extern bool remove_guild_by_keywords(const char *);
extern char * display_permission(unsigned int);
extern char * display_guild_flags(unsigned int);


#endif
