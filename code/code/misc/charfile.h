//////////////////////////////////////////////////////////////////////////
//
// SneezyMUD - All rights reserved, SneezyMUD Coding Team
//
//////////////////////////////////////////////////////////////////////////


#ifndef __CHARFILE_H
#define __CHARFILE_H

#include "limbs.h"
#include "stats.h"
#include "ansi.h"
#include "faction.h"
#include "obj.h"
#include "being.h"

class charFile {
  public:
   byte sex;
   byte level[MAX_SAVED_CLASSES];
   ubyte race;
   byte screen;
   attack_mode_t attack_type;
   float weight;
   unsigned short height;
   byte  doneBasic[MAX_SAVED_CLASSES];
   unsigned short Class;
   int played;    /* Number of secs played in total */
   int body_flags[MAX_HUMAN_WEAR];
   int conditions[MAX_COND_TYPE];

   char title[80];
   char name[20];
   char aname[10];
   char pwd[11];

   time_t birth;
   time_t last_logon;

   char description[500];
   char lastHost[40];
   char obsolete_prompt_colors[200];
   short hometown;
   ubyte hometerrain;
   short load_room;            /* Which room to place char in  */
   short body_health[MAX_HUMAN_WEAR];
   unsigned int obsolete_p_type;
   short bad_login;
   unsigned short base_age;
   short age_mod;
   unsigned short wimpy;
   unsigned int autobits;
   unsigned int best_rent_credit;

   short stats[MAX_STATS];
   // pointData data
   short mana;
   short maxMana;
   double piety;
   short lifeforce;
   short hit;
   short maxHit;
   short move;
   short maxMove;
   int money;
   int bankmoney;
   double exp;
   double max_exp;
   short spellHitroll;
   short hitroll;
   sbyte damroll;
   short armor;   
   // end of pointData data
   ubyte fatigue;
   int hero_num;
   saveAffectedData affected[MAX_AFFECT];
   double f_percent;  // faction stuff
   double f_percx[ABS_MAX_FACTION];
   byte   f_type;
   unsigned int f_actions;
   //new faction stuff
   int whichguild;
   int align_ge;
   int align_lc;
   // end new faction

   aliasData alias[16];
   pracData practices;

   byte disc_learning[MAX_SAVED_DISCS];
   byte skills[ABSOLUTE_MAX_SKILL];
  
   char pColor;

   unsigned int plr_act;          /* ACT Flags                    */
   unsigned int flags;
   unsigned int plr_color;        /* Color Flags                    */
   colorSubT plr_colorSub;    /* color substitute */
   unsigned int plr_colorOff;   /* colors to substitute for */
   int temp1;
   int temp2;
   int temp3;
   int temp4;

  charFile();
  ~charFile();
};

#endif
