//////////////////////////////////////////////////////////////////////////
//
// SneezyMUD - All rights reserved, SneezyMUD Coding Team
//
// $Log: charfile.h,v $
// Revision 1.1  1999/09/12 17:24:04  sneezy
// Initial revision
//
//
//////////////////////////////////////////////////////////////////////////


#ifndef __CHARFILE_H
#define __CHARFILE_H

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
   sh_int hometown;
   ubyte hometerrain;
   sh_int load_room;            /* Which room to place char in  */
   sh_int body_health[MAX_HUMAN_WEAR];
   sh_int p_type;
   sh_int bad_login;
   ush_int base_age;
   sh_int age_mod;
   ush_int wimpy;
   unsigned int autobits;
   unsigned int best_rent_credit;

   sh_int stats[MAX_STATS];
   pointData points;
   ubyte fatigue;
   int hero_num;
   saveAffectedData affected[MAX_AFFECT];
   double f_percent;  // faction stuff
   double f_percx[ABS_MAX_FACTION];
   byte   f_type;
   unsigned int f_actions;
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

  charFile();
  ~charFile();
};

#endif
