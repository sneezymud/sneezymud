//////////////////////////////////////////////////////////////////////////
//
// SneezyMUD - All rights reserved, SneezyMUD Coding Team
//
// $Log: statistics.cc,v $
// Revision 1.9  1999/10/14 00:38:21  batopr
// Added the saving/loading of stats.equip from file
//
// Revision 1.8  1999/10/12 17:06:21  batopr
// Change gold to need 5M (from 2M) before shifting
//
// Revision 1.7  1999/10/12 01:12:45  batopr
// Corrected shop stuff to exclude pets
//
// Revision 1.6  1999/10/07 17:39:38  batopr
// typo fix
//
// Revision 1.5  1999/10/07 16:00:58  batopr
// Shifted some gold statistics stuff to functions
//
// Revision 1.4  1999/10/06 23:46:50  batopr
// Boosted target profit ratio to 2% (from 1%) for PCs
//
// Revision 1.3  1999/09/22 19:05:50  cosmo
// Took out peels .3 cap on GOLD_INCOME, just made the mud not modify for it in
// monster.cc. so it will still float but wont be used till we evaluate it
//
// Revision 1.2  1999/09/22 17:48:22  peel
// Put a floor on gold_modifier[GOLD_INCOME] of .3, temporary economy 'fix'
//
// Revision 1.1  1999/09/12 17:24:04  sneezy
// Initial revision
//
//
//////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////
//
//  Statisitics for the gaming system - SneezyMUD
//
//  Original revision - Jan 14, 1994
//
//////////////////////////////////////////////////////////////////////

#include "stdsneezy.h"
#include "statistics.h"
#include "systemtask.h"

GameStats stats;
// statistics for info command 
unsigned int gDescriptorUpdates = 0;
unsigned long gTimeGameLoopStarted = 0;
unsigned int gHBsSinceReboot = 0;
unsigned int gReadThisHB = 0;
unsigned int gWriteThisHB = 0;
unsigned int gMaxReadHB = 0;
unsigned int gMaxWriteHB = 0;
unsigned int gMaxReadRound = 0;
unsigned int gMaxWriteRound = 0;
unsigned int gMaxReadProcess = 0;
unsigned int gMaxWriteProcess = 0;
unsigned long gBytesRead = 0;
unsigned long gBytesSent = 0;

long gold_statistics[MAX_MONEY_TYPE][MAX_IMMORT];
long gold_positive[MAX_MONEY_TYPE][MAX_IMMORT];
float gold_modifier[MAX_MONEY_TYPE];

bool auto_deletion;
bool rent_only_deletion;
bool nuke_inactive_mobs;

int init_game_stats(void)
{
  int i, j;
  FILE *fp;
  char buf[256];

  stats.absorb_damage_divisor[MOB_STAT] = 2;
  stats.absorb_damage_divisor[PC_STAT] = 4;

  stats.equip = 0.7;   // this affects the load rate of things
  stats.max_exist = 1.0;  // this affects the MAX number of a thing allowed

  // 1.40 resulted in 16-20 days playtime to L50
  // 1.05 resulted in 25-30 day to L50 (4.1)
  // 0.80 had reasonable rages for 4.5 beta
  stats.xp_modif = 0.75;   // this affects xp mobs will have

  // this affects damage applied.
  // it should be used to slow down or speed up fights
  // i.e. lowering it causes less damage to be applied, so fights take longer
  // c.f. balance notes for complete discussion
  // value of 1.0 makes fair fights take about 30 rounds = 90 seconds
  // a value of 0.75 should make for 120 second fights
  stats.damage_modifier = 0.75;

  auto_deletion = FALSE;
  rent_only_deletion = TRUE;
  nuke_inactive_mobs = FALSE;

  if (!(fp = fopen(STATS_FILE,"r"))) {
    vlogf(10, "Unable to open txt/stat file");
    return FALSE;
  } else {
    if (fscanf(fp, "%ld\n", &stats.logins) != 1) {
      vlogf(5, "bad stats.logins");
    }

    if (fscanf(fp, "%d %d\n", &repair_number, &total_help_number) != 2) {
      repair_number = 0;
      total_help_number = 0;
      vlogf(5, "bad repair_number");
      vlogf(5, "bad help_number");
    }

    for (i= 0; i < MAX_IMMORT; i++) {
      if (fscanf(fp, " %ld %ld %ld %ld %ld %ld %ld %ld %ld %ld %ld %ld %ld %ld %ld %ld\n",
             &gold_statistics[GOLD_INCOME][i], 
             &gold_statistics[GOLD_SHOP][i], 
             &gold_statistics[GOLD_REPAIR][i],
             &gold_statistics[GOLD_COMM][i], 
             &gold_statistics[GOLD_HOSPITAL][i], 
             &gold_statistics[GOLD_GAMBLE][i],
             &gold_statistics[GOLD_RENT][i],
             &gold_statistics[GOLD_TITHE][i],
             &gold_statistics[GOLD_SHOP_FOOD][i],
             &gold_statistics[GOLD_SHOP_COMPONENTS][i],
             &gold_statistics[GOLD_SHOP_SYMBOL][i],
             &gold_statistics[GOLD_SHOP_ARMOR][i],
             &gold_statistics[GOLD_SHOP_WEAPON][i],
             &gold_statistics[GOLD_SHOP_PET][i],
             &gold_statistics[GOLD_SHOP_RESPONSES][i],
             &gold_statistics[GOLD_DUMP][i]) != 16) {
        vlogf(5, "bad gold info, resetting %d", i);
        int j;
        for (j = 0; j < MAX_MONEY_TYPE; j++)
          gold_statistics[j][i] = 0;
      }
      if (fscanf(fp, " %ld %ld %ld %ld %ld %ld %ld %ld %ld %ld %ld %ld %ld %ld %ld %ld\n",
             &gold_positive[GOLD_INCOME][i], 
             &gold_positive[GOLD_SHOP][i], 
             &gold_positive[GOLD_REPAIR][i],
             &gold_positive[GOLD_COMM][i], 
             &gold_positive[GOLD_HOSPITAL][i], 
             &gold_positive[GOLD_GAMBLE][i],
             &gold_positive[GOLD_RENT][i],
             &gold_positive[GOLD_TITHE][i],
             &gold_positive[GOLD_SHOP_FOOD][i],
             &gold_positive[GOLD_SHOP_COMPONENTS][i],
             &gold_positive[GOLD_SHOP_SYMBOL][i],
             &gold_positive[GOLD_SHOP_ARMOR][i],
             &gold_positive[GOLD_SHOP_WEAPON][i],
             &gold_positive[GOLD_SHOP_PET][i],
             &gold_positive[GOLD_SHOP_RESPONSES][i],
             &gold_positive[GOLD_DUMP][i]) != 16) {
        vlogf(5, "bad gold info, resetting %d", i);
        int j;
        for (j = 0; j < MAX_MONEY_TYPE; j++)
          gold_positive[j][i] = 0;
      }
    }
    if (fscanf(fp, " %f %f %f %f %f %f %f %f %f %f %f %f %f %f %f %f\n",
             &gold_modifier[GOLD_INCOME], 
             &gold_modifier[GOLD_SHOP], 
             &gold_modifier[GOLD_REPAIR],
             &gold_modifier[GOLD_COMM], 
             &gold_modifier[GOLD_HOSPITAL], 
             &gold_modifier[GOLD_GAMBLE],
             &gold_modifier[GOLD_RENT],
             &gold_modifier[GOLD_TITHE],
             &gold_modifier[GOLD_SHOP_FOOD],
             &gold_modifier[GOLD_SHOP_COMPONENTS],
             &gold_modifier[GOLD_SHOP_SYMBOL],
             &gold_modifier[GOLD_SHOP_ARMOR],
             &gold_modifier[GOLD_SHOP_WEAPON],
             &gold_modifier[GOLD_SHOP_PET],
             &gold_modifier[GOLD_SHOP_RESPONSES],
             &gold_modifier[GOLD_DUMP]) != 16) {
        vlogf(5, "bad gold modifier info, resetting %d", i);
        int j;
        for (j = 0; j < MAX_MONEY_TYPE; j++)
          gold_modifier[j] = 1.0;
    }

    if (fscanf(fp, "%f\n", &stats.equip) != 1) {
      vlogf(5, "bad value for equipment load rate");
    }

    for (i = 0; i < 50; i++) {
      for (j = 0; j < MAX_CLASSES; j++) {
        if (fscanf(fp, "%d %ld ", 
               &stats.levels[j][i], &stats.time_levels[j][i]) != 2) {
          vlogf(5, "Bad level info, class %d, lev %d", j, i+1);
        }
      }
    }

    if (fscanf(fp, "%ld\n", &stats.first_login) != 1) {
      vlogf(5, "Bad first_login info, resetting.");
      time_t tnow;
      time(&tnow);
      stats.first_login = tnow;
      stats.logins = 0;
    }
    fclose(fp);

    sprintf(buf, "cp %s %s", STATS_FILE, STATS_BAK);
    vsystem(buf);
    return TRUE;
  }
}

void save_game_stats(void)
{
  FILE *fp;
  int i, j;

  if ((fp = fopen(STATS_FILE,"w+")) != NULL) {
    fprintf(fp, "%ld\n", stats.logins);
 
    fprintf(fp, "%d %d\n", repair_number, total_help_number);

    for (i= 0; i < MAX_IMMORT; i++) {
      fprintf(fp, "%ld %ld %ld %ld %ld %ld %ld %ld %ld %ld %ld %ld %ld %ld %ld %ld\n", 
         gold_statistics[GOLD_INCOME][i],
         gold_statistics[GOLD_SHOP][i],
         gold_statistics[GOLD_REPAIR][i],
         gold_statistics[GOLD_COMM][i],
         gold_statistics[GOLD_HOSPITAL][i],
         gold_statistics[GOLD_GAMBLE][i],
         gold_statistics[GOLD_RENT][i],
         gold_statistics[GOLD_TITHE][i],
	 gold_statistics[GOLD_SHOP_FOOD][i],
	 gold_statistics[GOLD_SHOP_COMPONENTS][i],
         gold_statistics[GOLD_SHOP_SYMBOL][i],
         gold_statistics[GOLD_SHOP_ARMOR][i],
         gold_statistics[GOLD_SHOP_WEAPON][i],
         gold_statistics[GOLD_SHOP_PET][i],
         gold_statistics[GOLD_SHOP_RESPONSES][i],
         gold_statistics[GOLD_DUMP][i]);
      fprintf(fp, "%ld %ld %ld %ld %ld %ld %ld %ld %ld %ld %ld %ld %ld %ld %ld %ld\n",
         gold_positive[GOLD_INCOME][i],
         gold_positive[GOLD_SHOP][i],
         gold_positive[GOLD_REPAIR][i],
         gold_positive[GOLD_COMM][i],
         gold_positive[GOLD_HOSPITAL][i],
         gold_positive[GOLD_GAMBLE][i],
         gold_positive[GOLD_RENT][i],
         gold_positive[GOLD_TITHE][i],
         gold_positive[GOLD_SHOP_FOOD][i],
         gold_positive[GOLD_SHOP_COMPONENTS][i],
         gold_positive[GOLD_SHOP_SYMBOL][i],
         gold_positive[GOLD_SHOP_ARMOR][i],
         gold_positive[GOLD_SHOP_WEAPON][i],
         gold_positive[GOLD_SHOP_PET][i],
         gold_positive[GOLD_SHOP_RESPONSES][i],
         gold_positive[GOLD_DUMP][i]);
    }
    fprintf(fp, "%.2f %.2f %.2f %.2f %.2f %.2f %.2f %.2f %.2f %.2f %.2f %.2f %.2f %.2f %.2f %.2f\n",
         gold_modifier[GOLD_INCOME],
         gold_modifier[GOLD_SHOP],
         gold_modifier[GOLD_REPAIR],
         gold_modifier[GOLD_COMM],
         gold_modifier[GOLD_HOSPITAL],
         gold_modifier[GOLD_GAMBLE],
         gold_modifier[GOLD_RENT],
         gold_modifier[GOLD_TITHE],
         gold_modifier[GOLD_SHOP_FOOD],
         gold_modifier[GOLD_SHOP_COMPONENTS],
         gold_modifier[GOLD_SHOP_SYMBOL],
         gold_modifier[GOLD_SHOP_ARMOR],
         gold_modifier[GOLD_SHOP_WEAPON],
         gold_modifier[GOLD_SHOP_PET],
         gold_modifier[GOLD_SHOP_RESPONSES],
         gold_modifier[GOLD_DUMP]);

    fprintf(fp, "%.2f\n", stats.equip);

    for (i = 0; i < 50; i++) {
      for (j = 0; j < MAX_CLASSES; j++) {
        fprintf(fp, "%d %ld ", stats.levels[j][i], stats.time_levels[j][i]);
      }
      fprintf(fp, "\n");
    }

    fprintf(fp, "%ld\n", stats.first_login);
 
    fclose(fp);
  } else {
    vlogf(9, "Error writing %s", STATS_FILE);
  }
}

void TBeing::doGamestats(const char *arg)
{
  char buf[256], buf2[256];
  int lev, i;
  string str;

  if (powerCheck(POWER_GAMESTATS))
    return;

  arg = two_arg(arg, buf, buf2);

  if (!buf || !*buf) {
    sendTo("Syntax: gamestats <combat | equipment | level | trivia | statistics>\n\r");
    return;
  } else if (is_abbrev(buf, "combat")) {
    int tot_dam = stats.damage[PC_STAT] + stats.damage[MOB_STAT];
    sprintf(buf, "Total damage taken  : %d\n\r", tot_dam);
    str += buf;

    sprintf(buf, "\tMob damage taken    : %ld   (%5.2f%%)\n\r", 
         stats.damage[MOB_STAT], (tot_dam ? (100.0 * stats.damage[MOB_STAT] / tot_dam) : 0));
    str += buf;
    sprintf(buf, "\tPC  damage taken    : %ld   (%5.2f%%)\n\r", 
         stats.damage[PC_STAT], (tot_dam ? (100.0 * stats.damage[PC_STAT] / tot_dam) : 0));
    str += buf;

    tot_dam = stats.combat_damage[PC_STAT] + stats.combat_damage[MOB_STAT];
    sprintf(buf, "Combat damage only  : %d\n\r", tot_dam);
    str += buf;
    sprintf(buf, "\tMob combat damage taken    : %ld   (%5.2f%%)\n\r", 
         stats.combat_damage[MOB_STAT], (tot_dam ? (100.0 * stats.combat_damage[MOB_STAT] / tot_dam) : 0));
    str += buf;
    sprintf(buf, "\tPC combat damage taken     : %ld   (%5.2f%%)\n\r", 
         stats.combat_damage[PC_STAT], (tot_dam ? (100.0 * stats.combat_damage[PC_STAT] / tot_dam) : 0));
    str += buf;

    int tot_ac = stats.ac_absorb[MOB_STAT] + stats.ac_absorb[PC_STAT];
    sprintf(buf, "AC  damage absorbed : %d\n\r", tot_ac);
    str += buf;

    int mob_dam = stats.ac_absorb[MOB_STAT] + stats.combat_damage[MOB_STAT];
    sprintf(buf, "\tMob AC absorb       : %ld   (%5.2f%% of all abs) (%5.2f%% of mob ComDam)\n\r", 
        stats.ac_absorb[MOB_STAT], tot_ac ? stats.ac_absorb[MOB_STAT] * 100.0 / tot_ac : 0,
        mob_dam ? stats.ac_absorb[MOB_STAT] * 100.0 / mob_dam : 0);
    str += buf;

    int pc_dam = stats.ac_absorb[PC_STAT] + stats.combat_damage[PC_STAT];
    sprintf(buf, "\tPC AC absorb        : %ld   (%5.2f%% of all abs) (%5.2f%% of PC ComDam)\n\r", 
        stats.ac_absorb[PC_STAT], tot_ac ? stats.ac_absorb[PC_STAT] * 100.0 / tot_ac : 0,
        pc_dam ? stats.ac_absorb[PC_STAT] * 100.0 / pc_dam : 0);
    str += buf;
    sprintf(buf, "Current absorbtion constants : TBeing %d, PC %d\n\r\n\r",
	  stats.absorb_damage_divisor[MOB_STAT],
           stats.absorb_damage_divisor[PC_STAT]);
    str += buf;

    long tot_blows = stats.combat_blows[PC_STAT] + stats.combat_blows[MOB_STAT];
    sprintf(buf, "Total Combat blows        : %ld\n\r", tot_blows);
    str += buf;
    sprintf(buf, "\tMob combat blows           : %ld   (%5.2f%%)\n\r", 
         stats.combat_blows[MOB_STAT], (tot_blows ? (100.0 * stats.combat_blows[MOB_STAT] / tot_blows) : 0));
    str += buf;
    sprintf(buf, "\tPC combat blows            : %ld   (%5.2f%%)\n\r", 
         stats.combat_blows[PC_STAT], (tot_blows ? (100.0 * stats.combat_blows[PC_STAT] / tot_blows) : 0));
    str += buf;

    long tot_hits = stats.combat_hits[PC_STAT] + stats.combat_hits[MOB_STAT];
    sprintf(buf, "Total Combat hits         : %ld  (%5.2f%%)\n\r", 
               tot_hits, (tot_blows == 0 ? 0.0 : (100.0 * tot_hits / tot_blows)));
    str += buf;
    sprintf(buf, "\tMob combat hits            : %ld   (%5.2f%%)  (%5.2f%% hit rate)\n\r", 
         stats.combat_hits[MOB_STAT], (tot_hits ? (100.0 * stats.combat_hits[MOB_STAT] / tot_hits) : 0),
        stats.combat_blows[MOB_STAT] ? stats.combat_hits[MOB_STAT] * 100.0 / stats.combat_blows[MOB_STAT] : 0);
    str += buf;
    sprintf(buf, "\tPC combat hits             : %ld   (%5.2f%%)  (%5.2f%% hit rate)\n\r", 
         stats.combat_hits[PC_STAT], (tot_hits ? (100.0 * stats.combat_hits[PC_STAT] / tot_hits) : 0),
        stats.combat_blows[PC_STAT] ? stats.combat_hits[PC_STAT] * 100.0 / stats.combat_blows[PC_STAT] : 0);
    str += buf;

    int tot_lev = stats.combat_level[PC_STAT] + stats.combat_level[MOB_STAT];
    sprintf(buf, "Average Combat level      : %5.2f\n\r", 
         (tot_blows ?  ((double) tot_lev / (double) tot_blows) : 0.0));
    str += buf;
    sprintf(buf, "\tMob combat level           : %5.2f\n\r", 
         (stats.combat_blows[MOB_STAT] ?  ((double) stats.combat_level[MOB_STAT] / (double) stats.combat_blows[MOB_STAT]) : 0.0));
    str += buf;
    sprintf(buf, "\tPC combat level            : %5.2f\n\r", 
         (stats.combat_blows[PC_STAT] ?  ((double) stats.combat_level[PC_STAT] / (double) stats.combat_blows[PC_STAT]) : 0.0));
    str += buf;

    sprintf(buf, "Average Combat damage     : %5.2f\n\r", 
         (tot_hits ?  ((double) tot_dam / (double) tot_hits) : 0.0));
    str += buf;
    sprintf(buf, "\tMob avg. combat dam.       : %5.2f\n\r", 
         (stats.combat_hits[MOB_STAT] ?  ((double) stats.combat_damage[MOB_STAT] / (double) stats.combat_hits[MOB_STAT]) : 0.0));
    str += buf;
    sprintf(buf, "\tPC avg. combat dam.        : %5.2f\n\r", 
         (stats.combat_hits[PC_STAT] ?  ((double) stats.combat_damage[PC_STAT] / (double) stats.combat_hits[PC_STAT]) : 0.0));
    str += buf;

    sprintf(buf, "Total crit-success checks : %ld  (%5.2f%% of hits)\n\r",
           stats.combat_crit_suc,
           (tot_hits == 0 ? 0.0 : 
              (100.0 * stats.combat_crit_suc / tot_hits)));
    str += buf;
    sprintf(buf, "Total crit-fail checks    : %ld  (%5.2f%% of misses)\n\r",
           stats.combat_crit_fail,
           ((tot_blows - tot_hits) == 0 ? 0.0 : 
               (100.0 * stats.combat_crit_fail / (tot_blows - tot_hits))));
    str += buf;
    sprintf(buf, "Total crit-success passes : %ld  (%5.2f%% of hits)\n\r", 
            stats.combat_crit_suc_pass,
           (tot_hits == 0 ? 0.0 : 
              (100.0 * stats.combat_crit_suc_pass / tot_hits)));
    str += buf;
    sprintf(buf, "Total crit-fail passes    : %ld  (%5.2f%% of misses)\n\r", 
            stats.combat_crit_fail_pass,
           ((tot_blows - tot_hits) == 0 ? 0.0 : 
              (100.0 * stats.combat_crit_fail_pass / (tot_blows - tot_hits))));
    str += buf;
    sprintf(buf, "\n\r");
    str += buf;
    sprintf(buf, "Mobiles have tried to aggro : %d times.\n\r", stats.aggro_attempts);
    str += buf;
    sprintf(buf, "Mobiles have aggro'd        : %d times.\n\r", stats.aggro_successes);
    str += buf;
    if (desc)
      desc->page_string(str.c_str(), 0, true);
    return;
  } else if (is_abbrev(buf, "equipment")) {
    sendTo("Current Equipment Load Modifier : %4.2f\n\r", stats.equip);
    sendTo("Current Max-Exist Modifier      : %4.2f\n\r", stats.max_exist);
    sendTo("Current Mob-Money Modifier      : %4.2f\n\r", gold_modifier[GOLD_INCOME]);
    sendTo("Current Mob-XP Modifier         : %4.2f\n\r", stats.xp_modif);
    sendTo("Current Damage Modifier         : %4.2f\n\r", stats.damage_modifier);
    return;
  } else if (is_abbrev(buf, "trivia")) {
    sendTo("Average HP regen              : %4.2f  (attempts : %d)\n\r",
        (stats.hit_gained_attempts == 0 ? 0.0 :
        ((float) stats.hit_gained / (float) stats.hit_gained_attempts)),
        stats.hit_gained_attempts);
    sendTo("Average MV regen              : %4.2f  (attempts : %d)\n\r",
        (stats.move_gained_attempts == 0 ? 0.0 :
        ((float) stats.move_gained / (float) stats.move_gained_attempts)),
        stats.move_gained_attempts);
    sendTo("Average mana regen            : %4.2f  (attempts : %d)\n\r",
        (stats.mana_gained_attempts == 0 ? 0.0 :
        ((float) stats.mana_gained / (float) stats.mana_gained_attempts)),
        stats.mana_gained_attempts);
    sendTo("Average piety regen           : %4.2f  (attempts : %d)\n\r",
        (stats.piety_gained_attempts == 0 ? 0.0 :
        (stats.piety_gained / (float) stats.piety_gained_attempts)),
        stats.piety_gained_attempts);
    return;
  } else if (is_abbrev(buf, "levels")) {
    if (!buf2 || !*buf2) {
      sendTo("Syntax : gamestats levels <level>\n\r");
      return;
    }
    lev = atoi(buf2);
    if ((lev >= 0) && (lev < 70)) {
      sendTo("Mobile Deaths for level %d, %ld\n\r", lev, stats.deaths[lev][1]);
      sendTo("PC  Deaths for level %d, %ld\n\r", lev, stats.deaths[lev][0]);

      if (lev >= 1 && lev <= 50) {
        sendTo("PC Leveling Data:\n\r");
        unsigned int factor = secs_to_level(lev);
        sendTo("Desired leveling time: %s\n\r",
           secsToString(factor).c_str());

        for (i=0;i<MAX_CLASSES;i++) {
          time_t factor = 0;
          if (stats.levels[i][lev-1])
            factor = SECS_PER_REAL_MIN * stats.time_levels[i][lev-1] / stats.levels[i][lev-1];

          sendTo("Class: %-10.10s :    number %3d,  avg. time: %s\n\r",
            classNames[i].name, stats.levels[i][lev-1],
            secsToString(factor).c_str());
        }
      }
      return;
    } else {
      sendTo("Please use a level from 0 - 70 in your level checking.\n\r");
      return;
    }
  } else if (is_abbrev(buf, "statistics")) {
    systask->AddTask(this, SYSTEM_STATISTICS, "");
    return;
  }
  sendTo("Syntax: gamestats <combat | equipment | level | trivia | statistics>\n\r");
  return;
}

int getNetGold(moneyTypeT mtt)
{
  int net_gold = 0;
  unsigned int i;
  for (i = 0; i < MAX_MORT; i++)
    net_gold += gold_statistics[mtt][i];
  return net_gold;
}

unsigned int getPosGold(moneyTypeT mtt)
{
  unsigned int pos_gold = 0;
  unsigned int i;
  for (i = 0; i < MAX_MORT; i++)
    pos_gold += gold_positive[mtt][i];
  return pos_gold;
}

int getNetGoldGlobal()
{
  int net_gold = 0;
  unsigned int i;
  for (i = 0; i < MAX_MORT; i++)
    net_gold += gold_statistics[GOLD_INCOME][i] + 
                gold_statistics[GOLD_COMM][i] +
                gold_statistics[GOLD_RENT][i] +
                gold_statistics[GOLD_REPAIR][i] +
                gold_statistics[GOLD_HOSPITAL][i] +
                gold_statistics[GOLD_GAMBLE][i] +
                gold_statistics[GOLD_TITHE][i] +
                gold_statistics[GOLD_DUMP][i] +
                gold_statistics[GOLD_SHOP_FOOD][i] +
                gold_statistics[GOLD_SHOP_COMPONENTS][i] +
                gold_statistics[GOLD_SHOP_SYMBOL][i] +
                gold_statistics[GOLD_SHOP_ARMOR][i] +
                gold_statistics[GOLD_SHOP_WEAPON][i] +
                gold_statistics[GOLD_SHOP_RESPONSES][i] +
                gold_statistics[GOLD_SHOP_PET][i] +
                gold_statistics[GOLD_SHOP][i];


  return net_gold;
}

unsigned int getPosGoldGlobal()
{
  unsigned int pos_gold = 0;
  unsigned int i;
  for (i = 0; i < MAX_MORT; i++)
    pos_gold += gold_positive[GOLD_INCOME][i] + 
                gold_positive[GOLD_COMM][i] +
                gold_positive[GOLD_RENT][i] +
                gold_positive[GOLD_REPAIR][i] +
                gold_positive[GOLD_HOSPITAL][i] +
                gold_positive[GOLD_GAMBLE][i] +
                gold_positive[GOLD_DUMP][i] +
                gold_positive[GOLD_TITHE][i] +
                gold_positive[GOLD_SHOP_FOOD][i] +
                gold_positive[GOLD_SHOP_COMPONENTS][i] +
                gold_positive[GOLD_SHOP_SYMBOL][i] +
                gold_positive[GOLD_SHOP_ARMOR][i] +
                gold_positive[GOLD_SHOP_WEAPON][i] +
                gold_positive[GOLD_SHOP_PET][i] +
                gold_positive[GOLD_SHOP_RESPONSES][i] +
                gold_positive[GOLD_SHOP][i];

  return pos_gold;
}

// we exclude the shop_pet value in this since it's off-budget, and the
// shop modifier doesn't factor into that price anyways
int getNetGoldShops()
{
  int net_gold = 0;
  unsigned int i;
  for (i = 0; i < MAX_MORT; i++)
    net_gold += gold_statistics[GOLD_SHOP_FOOD][i] +
                gold_statistics[GOLD_SHOP_COMPONENTS][i] +
                gold_statistics[GOLD_SHOP_SYMBOL][i] +
                gold_statistics[GOLD_SHOP_ARMOR][i] +
                gold_statistics[GOLD_SHOP_WEAPON][i] +
                gold_statistics[GOLD_SHOP_RESPONSES][i] +
                gold_statistics[GOLD_SHOP][i];

  return net_gold;
}

unsigned int getPosGoldShops()
{
  unsigned int pos_gold = 0;
  unsigned int i;
  for (i = 0; i < MAX_MORT; i++)
    pos_gold += gold_positive[GOLD_SHOP_FOOD][i] +
                gold_positive[GOLD_SHOP_COMPONENTS][i] +
                gold_positive[GOLD_SHOP_SYMBOL][i] +
                gold_positive[GOLD_SHOP_ARMOR][i] +
                gold_positive[GOLD_SHOP_WEAPON][i] +
                gold_positive[GOLD_SHOP_RESPONSES][i] +
                gold_positive[GOLD_SHOP][i];

  return pos_gold;
}

// we don't want income values floating high because PCs spend money
// on bad things (rent, pets)
int getNetGoldBudget()
{
  int net_gold = 0;
  unsigned int i;
  for (i = 0; i < MAX_MORT; i++)
    net_gold += gold_statistics[GOLD_INCOME][i] + 
                gold_statistics[GOLD_COMM][i] +
                gold_statistics[GOLD_REPAIR][i] +
                gold_statistics[GOLD_HOSPITAL][i] +
                gold_statistics[GOLD_GAMBLE][i] +
                gold_statistics[GOLD_TITHE][i] +
                gold_statistics[GOLD_DUMP][i] +
                gold_statistics[GOLD_SHOP_FOOD][i] +
                gold_statistics[GOLD_SHOP_COMPONENTS][i] +
                gold_statistics[GOLD_SHOP_SYMBOL][i] +
                gold_statistics[GOLD_SHOP_ARMOR][i] +
                gold_statistics[GOLD_SHOP_WEAPON][i] +
                gold_statistics[GOLD_SHOP_RESPONSES][i] +
                gold_statistics[GOLD_SHOP][i];


  return net_gold;
}

unsigned int getPosGoldBudget()
{
  unsigned int pos_gold = 0;
  unsigned int i;
  for (i = 0; i < MAX_MORT; i++)
    pos_gold += gold_positive[GOLD_INCOME][i] + 
                gold_positive[GOLD_COMM][i] +
                gold_positive[GOLD_REPAIR][i] +
                gold_positive[GOLD_HOSPITAL][i] +
                gold_positive[GOLD_GAMBLE][i] +
                gold_positive[GOLD_DUMP][i] +
                gold_positive[GOLD_TITHE][i] +
                gold_positive[GOLD_SHOP_FOOD][i] +
                gold_positive[GOLD_SHOP_COMPONENTS][i] +
                gold_positive[GOLD_SHOP_SYMBOL][i] +
                gold_positive[GOLD_SHOP_ARMOR][i] +
                gold_positive[GOLD_SHOP_WEAPON][i] +
                gold_positive[GOLD_SHOP_RESPONSES][i] +
                gold_positive[GOLD_SHOP][i];

  return pos_gold;
}

void checkGoldStats()
{
  // insure we have enough data to take accurate reading
  unsigned int pos_gold = getPosGoldGlobal();
  if (pos_gold < 5000000U)
    return;

  int net_gold_shop_comp = getNetGold(GOLD_SHOP_COMPONENTS);
  int net_gold_shop_sym = getNetGold(GOLD_SHOP_SYMBOL);
  int net_gold_shop_arm = getNetGold(GOLD_SHOP_ARMOR);
  int net_gold_shop_weap = getNetGold(GOLD_SHOP_WEAPON);
  int net_gold_shop_pet = getNetGold(GOLD_SHOP_PET);
  int net_gold_shop_food = getNetGold(GOLD_SHOP_FOOD);
  int net_gold_shop_resp = getNetGold(GOLD_SHOP_RESPONSES);
  int net_gold_repair = getNetGold(GOLD_REPAIR);
  int net_gold_shop = getNetGold(GOLD_SHOP);
  int net_gold_income = getNetGold(GOLD_INCOME);
  int net_gold = getNetGoldGlobal();
  int net_gold_all_shops = getNetGoldShops();
  int net_gold_budget = getNetGoldBudget();
  unsigned int pos_gold_shop_food = getPosGold(GOLD_SHOP_FOOD);
  unsigned int pos_gold_shop_comp = getPosGold(GOLD_SHOP_COMPONENTS);
  unsigned int pos_gold_shop_sym = getPosGold(GOLD_SHOP_SYMBOL);
  unsigned int pos_gold_shop_arm = getPosGold(GOLD_SHOP_ARMOR);
  unsigned int pos_gold_shop_weap = getPosGold(GOLD_SHOP_WEAPON);
  unsigned int pos_gold_shop_pet = getPosGold(GOLD_SHOP_PET);
  unsigned int pos_gold_shop_resp = getPosGold(GOLD_SHOP_RESPONSES);
  unsigned int pos_gold_repair = getPosGold(GOLD_REPAIR);
  unsigned int pos_gold_shop = getPosGold(GOLD_SHOP);
  unsigned int pos_gold_income = getPosGold(GOLD_INCOME);
  unsigned int pos_gold_all_shops = getPosGoldShops();
  unsigned int pos_gold_budget = getPosGoldBudget();

  bool should_reset = false;

  // want shops to make money (roughly 5% of total)
  if (net_gold_all_shops > 0) {
    // shops are giving out too much money
    gold_modifier[GOLD_SHOP] -= 0.01;
//    vlogf(5, "ECONOMY: shop modifier lowered. %d %u %.2f", net_gold_all_shops, pos_gold_all_shops, gold_modifier[GOLD_SHOP]);
    should_reset = true;
  } else if ((unsigned int) -net_gold_all_shops > pos_gold_all_shops/10) {
    // shops are making too much money
    gold_modifier[GOLD_SHOP] += 0.01;
//    vlogf(5, "ECONOMY: shop modifier raised. %d %u %.2f", net_gold_all_shops, pos_gold_all_shops, gold_modifier[GOLD_SHOP]);
    should_reset = true;
  }

  // overall, would like players to be gaining slightly on gold (2% target)
  float target_income = 0.02;
  if ((unsigned int) net_gold_budget < ((target_income - 0.03) * pos_gold_budget)) {
    // players losing money
    gold_modifier[GOLD_INCOME] += 0.01;
//    vlogf(5, "ECONOMY: income modifier raised. %d %u %.2f", net_gold_budget, pos_gold_budget, gold_modifier[GOLD_INCOME]);
    should_reset = true;
  } else if ((unsigned int) net_gold_budget > ((target_income + 0.03) * pos_gold_budget)) {
    // players making too much
    gold_modifier[GOLD_INCOME] -= 0.01;
//    vlogf(5, "ECONOMY: income modifier lowered. %d %u %.2f", net_gold_budget, pos_gold_budget, gold_modifier[GOLD_INCOME]);
    should_reset = true;
  }
  
  // good drain:
  // components and symbol purchasing
  // armor and weapon purchasing (upgrading)
  // purchases from response mobs (in general, components)
  // money spent repairing eq
  // food purchases
  // income money : unrecovered corpse?

  // bad drains
  // rent: means have goods beyond appropriate
  // pets: purchasing a "group" to raise their power
  // other shop stuff: potions, scrolls, etc?
    // power increasers

  // the goal is to make 90% of the money drained come from the good
  // drains.  Rent is off budget, as is transfering (splitting, giving coins)
  // so we don't need to leave tremendous slack in the system
  const float target_drain = 0.90;

  // We will have the repair modifier self-adjust in order to drive the
  // economy to the desired value
#if 0
  int good_drain = (pos_gold_repair - net_gold_repair);
  good_drain += (pos_gold_income - net_gold_income);
  good_drain += (pos_gold_shop - net_gold_shop);
  good_drain += (pos_gold_shop_food - net_gold_shop_food);
  good_drain += (pos_gold_shop_comp - net_gold_shop_comp);
  good_drain += (pos_gold_shop_sym - net_gold_shop_sym);
  good_drain += (pos_gold_shop_arm - net_gold_shop_arm);
  good_drain += (pos_gold_shop_weap - net_gold_shop_weap);
  good_drain += (pos_gold_shop_resp - net_gold_shop_resp);
#else
  int good_drain = (pos_gold_budget - net_gold_budget);
#endif

  int total_drain = pos_gold - net_gold;
  if (good_drain < (int) ((target_drain - .05) * total_drain)) {
    // repair is too small a drain
    gold_modifier[GOLD_REPAIR] += 0.01;
//    vlogf(5, "ECONOMY: repair modifier raised. %d %d %.2f", good_drain, total_drain, gold_modifier[GOLD_REPAIR]);
    should_reset = true;
  } else if (good_drain > (int) ((target_drain + .05) * total_drain)) {
    // repair is too large a drain
    gold_modifier[GOLD_REPAIR] -= 0.01;
//    vlogf(5, "ECONOMY: repair modifier lowered. %d %d %.2f", good_drain, total_drain, gold_modifier[GOLD_REPAIR]);
    should_reset = true;
  }

  if (should_reset) {
    memset(&gold_statistics, 0, sizeof(gold_statistics));
    memset(&gold_positive, 0, sizeof(gold_positive));
    save_game_stats();
  }
}
