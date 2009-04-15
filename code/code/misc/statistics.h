//////////////////////////////////////////////////////////////////////////
//
// SneezyMUD - All rights reserved, SneezyMUD Coding Team
//
//////////////////////////////////////////////////////////////////////////


#ifndef __STATS_H
#define __STATS_H

#include <algorithm>

#include "enum.h"
#include "being.h"

using std::min;
using std::max;

const int PC_STAT  = 0;
const int MOB_STAT = 1;

const char * const STATS_FILE     = "txt/stats";
const char * const STATS_BAK      = "txt/stats.bak";

class GameStats {
  public:
    int deaths[71][2];

    int levels[MAX_CLASSES][50];
    long time_levels[MAX_CLASSES][50];

    unsigned long damage[2];
    unsigned long ac_absorb[2];
    unsigned long combat_crit_suc;
    unsigned long combat_crit_suc_pass;
    unsigned long combat_crit_fail;
    unsigned long combat_crit_fail_pass;
    unsigned long combat_blows[2];
    unsigned long combat_hits[2];
    unsigned long combat_level[2];
    unsigned long combat_damage[2];
    unsigned int aggro_attempts;
    unsigned int aggro_successes;

    unsigned int hit_gained;
    unsigned int hit_gained_attempts;
    unsigned int move_gained;
    unsigned int move_gained_attempts;
    unsigned int mana_gained;
    unsigned int mana_gained_attempts;
    float piety_gained;
    unsigned int piety_gained_attempts;

    float equip;
    double max_exist;
    double xp_modif;
    double damage_modifier;
    int absorb_damage_divisor[2];
    long logins;
    unsigned int mobs_1_5;
    unsigned int mobs_6_10;
    unsigned int mobs_11_15;
    unsigned int mobs_16_20;
    unsigned int mobs_21_25;
    unsigned int mobs_26_30;
    unsigned int mobs_31_40;
    unsigned int mobs_41_50;
    unsigned int mobs_51_60;
    unsigned int mobs_61_70;
    unsigned int mobs_71_100;
    unsigned int mobs_101_127;
    unsigned int act_1_5;
    unsigned int act_6_10;
    unsigned int act_11_15;
    unsigned int act_16_20;
    unsigned int act_21_25;
    unsigned int act_26_30;
    unsigned int act_31_40;
    unsigned int act_41_50;
    unsigned int act_51_60;
    unsigned int act_61_70;
    unsigned int act_71_100;
    unsigned int act_101_127;
    time_t first_login;
    time_t useage_timer;
    unsigned int num_users;
    unsigned int useage_iters;

    GameStats() :
      combat_crit_suc(0),
      combat_crit_suc_pass(0),
      combat_crit_fail(0),
      combat_crit_fail_pass(0),
      aggro_attempts(0),
      aggro_successes(0),
      hit_gained(0),
      hit_gained_attempts(0),
      move_gained(0),
      move_gained_attempts(0),
      mana_gained(0),
      mana_gained_attempts(0),
      piety_gained(0.0),
      piety_gained_attempts(0),
      equip(0.0),
      max_exist(0.0),
      xp_modif(1.0),
      damage_modifier(1.0),
      logins(0),
      mobs_1_5(0),
      mobs_6_10(0),
      mobs_11_15(0),
      mobs_16_20(0),
      mobs_21_25(0),
      mobs_26_30(0),
      mobs_31_40(0),
      mobs_41_50(0),
      mobs_51_60(0),
      mobs_61_70(0),
      mobs_71_100(0),
      mobs_101_127(0),
      act_1_5(0),
      act_6_10(0),
      act_11_15(0),
      act_16_20(0),
      act_21_25(0),
      act_26_30(0),
      act_31_40(0),
      act_41_50(0),
      act_51_60(0),
      act_61_70(0),
      act_71_100(0),
      act_101_127(0),
      num_users(0),
      useage_iters(0)
    {
      memset(deaths, 0, sizeof(deaths));
      memset(levels, 0, sizeof(levels));
      memset(time_levels, 0, sizeof(time_levels));
      memset(absorb_damage_divisor, 0, sizeof(absorb_damage_divisor));
      memset(ac_absorb, 0, sizeof(ac_absorb));
      memset(damage, 0, sizeof(damage));
      memset(combat_blows, 0, sizeof(combat_blows));
      memset(combat_hits, 0, sizeof(combat_hits));
      memset(combat_level, 0, sizeof(combat_level));
      memset(combat_damage, 0, sizeof(combat_damage));

      first_login = time(0);
      useage_timer = time(0);
    }
};

extern GameStats stats;
extern unsigned int  gDescriptorUpdates;
extern unsigned long gTimeGameLoopStarted;
extern unsigned int  gHBsSinceReboot;
extern unsigned int  gMaxReadHB;
extern unsigned int  gMaxWriteHB;
extern unsigned int  gMaxReadRound;
extern unsigned int  gMaxWriteRound;
extern unsigned int  gMaxReadProcess;
extern unsigned int  gMaxWriteProcess;
extern unsigned long gBytesRead;
extern unsigned long gBytesSent;
extern unsigned int  gReadThisHB;
extern unsigned int  gWriteThisHB;

extern unsigned int help_used_num;
extern unsigned int typo_used_num;
extern unsigned int bug_used_num;
extern unsigned int idea_used_num;
extern unsigned int news_used_num;
extern unsigned int wizlist_used_num;
extern unsigned int wiznews_used_num;

extern unsigned int total_help_number;
extern unsigned int total_deaths;
extern unsigned int total_player_kills;

extern long gold_statistics[MAX_MONEY_TYPE][MAX_IMMORT];
extern long gold_positive[MAX_MONEY_TYPE][MAX_IMMORT];

class TGoldModifier {
  private:
    float tMin,
          tMax,
          tCurrent;

  public:
    float valAssign    (float tCheck) {
      return (tCurrent = max(tMin, min(tMax, tCheck)));
    }
    float * operator & (            ) { return &tCurrent; }
    float   operator  =(float tCheck) { return valAssign(tCurrent =  tCheck); }
    float   operator +=(float tCheck) { return valAssign(tCurrent += tCheck); }
    float   operator -=(float tCheck) { return valAssign(tCurrent -= tCheck); }
    bool    operator ==(float tCheck) { return valAssign(tCurrent == tCheck); }
    bool    operator > (float tCheck) { return (tCurrent >  tCheck); }
    bool    operator >=(float tCheck) { return (tCurrent >= tCheck); }
    bool    operator < (float tCheck) { return (tCurrent >  tCheck); }
    bool    operator <=(float tCheck) { return (tCurrent >= tCheck); }

    float   getVal() { return tCurrent; }
    void    setMM(float tNMin, float tNMax) {
      tMin = tNMin;
      tMax = tNMax;
    }

    TGoldModifier() :
      tMin(0.0),
      tMax(100.0),
      tCurrent(100.0)
      {}

    TGoldModifier(int tCheck, int tNMin, int tNMax) :
      tMin(tNMin),
      tMax(tNMax),
      tCurrent(tCheck)
      {}
};
extern TGoldModifier gold_modifier[MAX_MONEY_TYPE];

#if 1
extern int getNetGold(moneyTypeT);
extern unsigned int getPosGold(moneyTypeT);

extern int getNetGoldGlobal();
extern unsigned int getPosGoldGlobal();
extern int getNetGoldShops();
extern unsigned int getPosGoldShops();
extern int getNetGoldBudget();
extern unsigned int getPosGoldBudget();
#else
extern float getNetGold(moneyTypeT);
extern float getPosGold(moneyTypeT);

extern float getNetGoldGlobal();
extern float getPosGoldGlobal();
extern float getNetGoldShops();
extern float getPosGoldShops();
extern float getNetGoldBudget();
extern float getPosGoldBudget();
#endif

#endif
