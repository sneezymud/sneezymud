//////////////////////////////////////////////////////////////////////////
//
// SneezyMUD - All rights reserved, SneezyMUD Coding Team
//
//////////////////////////////////////////////////////////////////////////


#ifndef __CHARFILE_H
#define __CHARFILE_H

#include "being.h"

class charFile {
  public:
   byte sex;
   byte level[MAX_SAVED_CLASSES];
   ubyte race;
   byte screen;
   attack_mode_t attack_type;
   float weight;
   ush_int height;
   byte  doneBasic[MAX_SAVED_CLASSES];
   ush_int Class;
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
   char hpColor[20], manaColor[20], moveColor[20];
   char moneyColor[20], expColor[20];
   char roomColor[20], oppColor[20];
   char tankColor[20];
   char pietyColor[20], lifeforceColor[20];
   sh_int hometown;
   ubyte hometerrain;
   sh_int load_room;            /* Which room to place char in  */
   sh_int body_health[MAX_HUMAN_WEAR];
   unsigned int p_type;
   sh_int bad_login;
   ush_int base_age;
   sh_int age_mod;
   ush_int wimpy;
   unsigned int autobits;
   unsigned int best_rent_credit;

   sh_int stats[MAX_STATS];
   // pointData data
   sh_int mana;
   sh_int maxMana;
   double piety;
   sh_int lifeforce;
   sh_int hit;
   sh_int maxHit;
   sh_int move;
   sh_int maxMove;
   int money;
   int bankmoney;
   double exp;
   double max_exp;
   sh_int spellHitroll;
   sh_int hitroll;
   sbyte damroll;
   sh_int armor;   
   // end of pointData data
   ubyte fatigue;
   int hero_num;
   saveAffectedData affected[MAX_AFFECT];
   double f_percent;  // faction stuff
   double f_percx[ABS_MAX_FACTION];
   byte   f_type;
   unsigned int f_actions;
   //new faction stuff
   int whichfaction;
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
