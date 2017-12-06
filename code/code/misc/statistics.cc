//////////////////////////////////////////////////////////////////////////
//
// SneezyMUD - All rights reserved, SneezyMUD Coding Team
//
//  Statisitics for the gaming system - SneezyMUD
//
//////////////////////////////////////////////////////////////////////

#include <stdio.h>

#include "being.h"
#include "statistics.h"
#include "systemtask.h"
#include "extern.h"
#include "process.h"

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
TGoldModifier gold_modifier[MAX_MONEY_TYPE];

void SetupStaticGoldModifiers()
{
  gold_modifier[GOLD_XFER           ].setMM(0.01, 100.0);
  gold_modifier[GOLD_INCOME         ].setMM(0.25, 100.0);
  gold_modifier[GOLD_REPAIR         ].setMM(0.01, 100.0);
  gold_modifier[GOLD_SHOP           ].setMM(0.01, 100.0);
  gold_modifier[GOLD_COMM           ].setMM(0.01, 100.0);
  gold_modifier[GOLD_HOSPITAL       ].setMM(0.01, 100.0);
  gold_modifier[GOLD_GAMBLE         ].setMM(0.01, 100.0);
  gold_modifier[GOLD_RENT           ].setMM(0.01, 100.0);
  gold_modifier[GOLD_DUMP           ].setMM(0.01, 100.0);
  gold_modifier[GOLD_TITHE          ].setMM(0.01, 100.0);
  gold_modifier[GOLD_SHOP_SYMBOL    ].setMM(0.01, 100.0);
  gold_modifier[GOLD_SHOP_WEAPON    ].setMM(0.01, 100.0);
  gold_modifier[GOLD_SHOP_ARMOR     ].setMM(0.01, 100.0);
  gold_modifier[GOLD_SHOP_PET       ].setMM(0.01, 100.0);
  gold_modifier[GOLD_SHOP_FOOD      ].setMM(0.01, 100.0);
  gold_modifier[GOLD_SHOP_COMPONENTS].setMM(0.01, 100.0);
  gold_modifier[GOLD_SHOP_RESPONSES ].setMM(0.01, 100.0);
}

int init_game_stats(void)
{
  int i, j;
  FILE *fp;
  char buf[256];

  stats.absorb_damage_divisor[MOB_STAT] = 2;
  stats.absorb_damage_divisor[PC_STAT] = 4;

  stats.equip = 0.7;   // this affects the global load potential of things
  stats.global_lp_target = stats.equip;  // When we want to change the load potential, this is what we change.
  stats.global_lp_target_changerate = 0.0;  // This is how fast the load potential changes.
  stats.max_exist = 1.2;  // this affects the MAX number of a thing allowed

  stats.burnrate = 1.0;  // this affects the rate at which things burn

  // 1.40 resulted in 16-20 days playtime to L50
  // 1.05 resulted in 25-30 day to L50 (4.1)
  // 0.80 had reasonable rages for 4.5 beta
  // 5.2 will be more challenging
  // july 2001 - these stands need to be adjusted for speed changes...
  // rounds are slower now, so we need to eat up a 5/3 adjustment
  // see comm.h for the first part of the compensation
  stats.xp_modif = 0.65;   // people had is too easy in 5.0-5.1
  //stats.xp_modif = 0.86;

  // this affects damage applied.
  // it should be used to slow down or speed up fights
  // i.e. lowering it causes less damage to be applied, so fights take longer
  // c.f. balance notes for complete discussion
  // value of 1.0 makes fair fights take about 30 rounds = 90 seconds
  // a value of 0.75 should make for 120 second fights
  // 5.0-5.1 was too easy and too fast
  // people could level to 50 in 2-6 play days
  // this should be better for 5.2
  stats.damage_modifier = 0.65;
  //stats.damage_modifier = 0.86;

  if (!(fp = fopen(STATS_FILE,"r"))) {
    vlogf(LOG_BUG, "Unable to open txt/stat file");
    return FALSE;
  } else {
    if (fscanf(fp, "%ld\n", &stats.logins) != 1) {
      vlogf(LOG_BUG, "bad stats.logins");
    }

    if (fscanf(fp, "%d %d\n", &repair_number, &total_help_number) != 2) {
      repair_number = 0;
      total_help_number = 0;
      vlogf(LOG_BUG, "bad repair_number");
      vlogf(LOG_BUG, "bad help_number");
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
        vlogf(LOG_BUG, format("bad gold info, resetting %d") %  i);
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
        vlogf(LOG_BUG, format("bad gold info, resetting %d") %  i);
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
        vlogf(LOG_BUG, format("bad gold modifier info, resetting %d") %  i);
        int j;
        for (j = 0; j < MAX_MONEY_TYPE; j++)
          gold_modifier[j] = 1.0;
    }

    SetupStaticGoldModifiers();

    if (fscanf(fp, "%lf\n", &stats.equip) != 1) {
      vlogf(LOG_BUG, "bad value for equipment load potential");
    }

    for (i = 0; i < 50; i++) {
      for (j = 0; j < MAX_CLASSES; j++) {
        if (fscanf(fp, "%d %ld ", 
               &stats.levels[j][i], &stats.time_levels[j][i]) != 2) {
          vlogf(LOG_BUG, format("Bad level info, class %d, lev %d") %  j % (i+1));
        }
      }
    }

    long fl=(long)stats.first_login;
    if (fscanf(fp, "%ld\n", &fl) != 1) {
      vlogf(LOG_BUG, "Bad first_login info, resetting.");
      time_t tnow;
      time(&tnow);
      stats.first_login = tnow;
      stats.logins = 0;
    }

    if (fscanf(fp, "%lf", &stats.global_lp_target) != 1) {
      vlogf(LOG_BUG, "bad value for equipment load potemtial target value");
    }

    if (fscanf(fp, "%lf\n", &stats.global_lp_target_changerate) != 1) {
      vlogf(LOG_BUG, "bad value for rate of change of equipment load potential");
    }

    if (fscanf(fp, "%lf\n", &stats.burnrate) != 1) {
      vlogf(LOG_BUG, "bad value for burn rate");
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
         gold_modifier[GOLD_INCOME].getVal(),
         gold_modifier[GOLD_SHOP].getVal(),
         gold_modifier[GOLD_REPAIR].getVal(),
         gold_modifier[GOLD_COMM].getVal(),
         gold_modifier[GOLD_HOSPITAL].getVal(),
         gold_modifier[GOLD_GAMBLE].getVal(),
         gold_modifier[GOLD_RENT].getVal(),
         gold_modifier[GOLD_TITHE].getVal(),
         gold_modifier[GOLD_SHOP_FOOD].getVal(),
         gold_modifier[GOLD_SHOP_COMPONENTS].getVal(),
         gold_modifier[GOLD_SHOP_SYMBOL].getVal(),
         gold_modifier[GOLD_SHOP_ARMOR].getVal(),
         gold_modifier[GOLD_SHOP_WEAPON].getVal(),
         gold_modifier[GOLD_SHOP_PET].getVal(),
         gold_modifier[GOLD_SHOP_RESPONSES].getVal(),
         gold_modifier[GOLD_DUMP].getVal());

    fprintf(fp, "%.18lf\n", stats.equip);

    for (i = 0; i < 50; i++) {
      for (j = 0; j < MAX_CLASSES; j++) {
        fprintf(fp, "%d %ld ", stats.levels[j][i], stats.time_levels[j][i]);
      }
      fprintf(fp, "\n");
    }

    fprintf(fp, "%ld\n", (long) stats.first_login);

    fprintf(fp, "%.18lf %.18lf\n", stats.global_lp_target, stats.global_lp_target_changerate);

    fprintf(fp, "%.18lf\n", stats.burnrate);
 
    fclose(fp);
  } else {
    vlogf(LOG_BUG, format("Error writing %s") %  STATS_FILE);
  }
}

// this returns a number indicating how long we "think" it should
// take to get to level "lev"
// it is somewhat arbitrary
//
int secs_to_level(int lev)
{
  lev--;
  if (lev <= 0)
    return 0;

  // how much damage (average/round) am I doing
  float dam_level = 0.9 * lev;

  // what is the average mob have for hp
  float avg_mob_hp = (11.0 + 4.5) * lev;

  // average combat length
  float rounds_combat = avg_mob_hp / dam_level;

  // converted to number of seconds
  int time_combat = (int) (rounds_combat * Pulse::COMBAT / Pulse::ONE_SECOND);

  // now figure out regen
  // assume that a fiar fight consumes 100% of players HP each kill
  int avg_pc_hp = 15 + (10*lev);

  // numer of hps gained back per tick
  int regen_per_tick;
  regen_per_tick = (int) (stats.hit_gained_attempts == 0 ? 1 :
        ((float) stats.hit_gained / (float) stats.hit_gained_attempts));

  float ticks_regen = (float) avg_pc_hp / (float) regen_per_tick;

  int secs_regen = (int) (ticks_regen * (Pulse::UPDATE/2) / Pulse::ONE_SECOND);

  int tot_time = secs_regen + time_combat;

  // and i have to kill a bunch of mobs
  tot_time *= kills_to_level(lev);

  // don't forget to add in how long it took me to get to last level.
  tot_time += secs_to_level(lev);

  return tot_time;
}

void TBeing::doGamestats(const sstring &arg)
{
  sstring buf, buf2;
  int lev, i;
  sstring str;

  if (powerCheck(POWER_GAMESTATS))
    return;

  buf=arg.word(0);
  buf2=arg.word(1);


  if(buf.empty()){
    sendTo("Syntax: gamestats <attributes | combat | equipment | levels | trivia>\n\r");
    return;
  } else if (is_abbrev(buf, "attributes")) {
    int temp_stat = getStat(STAT_CURRENT, STAT_STR);
    double plot1 = 0;
    double plot2 = 0;
    double curve;
    if(buf2.empty()){
      curve = 1.4;
    } else
      curve = convertTo<float>(buf2);
    for(int tmpint = 5; tmpint <= 205; tmpint += 5) {
      setStat(STAT_CURRENT, STAT_STR, tmpint);
      plot1 = plotStat(STAT_CURRENT, STAT_STR, .80, 1.25, 1.00, curve);
      plot2 = plotStat(STAT_CURRENT, STAT_STR, 0.0, 100.0, 50.0, curve);
      buf = format("Stat Value: %5.2f     Plot1: %5.2f    Plot2: %5.2f%c\n\r") % (double)tmpint % plot1 % plot2 % '%';
      sendTo(buf);
    }
    setStat(STAT_CURRENT, STAT_STR, temp_stat);
    return;
  } else if (is_abbrev(buf, "combat")) {
    int tot_dam = stats.damage[PC_STAT] + stats.damage[MOB_STAT];
    buf = format("Total damage taken  : %d\n\r") % tot_dam;
    str += buf;

    buf = format("\tMob damage taken    : %ld   (%5.2f%c)\n\r") %
      stats.damage[MOB_STAT] % (tot_dam ? (100.0 * stats.damage[MOB_STAT] / tot_dam) : 0) % '%';
    str += buf;
    buf = format("\tPC  damage taken    : %ld   (%5.2f%c)\n\r") %
         stats.damage[PC_STAT] % (tot_dam ? (100.0 * stats.damage[PC_STAT] / tot_dam) : 0) % '%';
    str += buf;

    tot_dam = stats.combat_damage[PC_STAT] + stats.combat_damage[MOB_STAT];
    buf = format("Combat damage only  : %d\n\r") % tot_dam;
    str += buf;
    buf = format("\tMob combat damage taken    : %ld   (%5.2f%c)\n\r") %
         stats.combat_damage[MOB_STAT] % (tot_dam ? (100.0 * stats.combat_damage[MOB_STAT] / tot_dam) : 0) % '%';
    str += buf;
    buf = format("\tPC combat damage taken     : %ld   (%5.2f%c)\n\r") %
         stats.combat_damage[PC_STAT] % (tot_dam ? (100.0 * stats.combat_damage[PC_STAT] / tot_dam) : 0) % '%';
    str += buf;

    int tot_ac = stats.ac_absorb[MOB_STAT] + stats.ac_absorb[PC_STAT];
    buf = format("AC  damage absorbed : %d\n\r") % tot_ac;
    str += buf;

    int mob_dam = stats.ac_absorb[MOB_STAT] + stats.combat_damage[MOB_STAT];
    buf = format("\tMob AC absorb       : %ld   (%5.2f%c of all abs) (%5.2f%c of mob ComDam)\n\r") %
        stats.ac_absorb[MOB_STAT] % (tot_ac ? stats.ac_absorb[MOB_STAT] * 100.0 / tot_ac : 0) % '%' %
      (mob_dam ? stats.ac_absorb[MOB_STAT] * 100.0 / mob_dam : 0) % '%';
    str += buf;

    int pc_dam = stats.ac_absorb[PC_STAT] + stats.combat_damage[PC_STAT];
    buf = format("\tPC AC absorb        : %ld   (%5.2f%c of all abs) (%5.2f%c of PC ComDam)\n\r") %
      stats.ac_absorb[PC_STAT] % (tot_ac ? stats.ac_absorb[PC_STAT] * 100.0 / tot_ac : 0) % '%' %
      (pc_dam ? stats.ac_absorb[PC_STAT] * 100.0 / pc_dam : 0) % '%';
    str += buf;
    buf = format("Current absorbtion constants : TBeing %d, PC %d\n\r\n\r") %
      stats.absorb_damage_divisor[MOB_STAT] %
      stats.absorb_damage_divisor[PC_STAT];
    str += buf;

    long tot_blows = stats.combat_blows[PC_STAT] + stats.combat_blows[MOB_STAT];
    buf = format("Total Combat blows        : %ld\n\r") % tot_blows;
    str += buf;
    buf = format("\tMob combat blows           : %ld   (%5.2f%c)\n\r") %
      stats.combat_blows[MOB_STAT] %
      (tot_blows ? (100.0 * stats.combat_blows[MOB_STAT] / tot_blows) : 0) %
      '%';
    str += buf;
    buf = format("\tPC combat blows            : %ld   (%5.2f%c)\n\r") %
      stats.combat_blows[PC_STAT] %
      (tot_blows ? (100.0 * stats.combat_blows[PC_STAT] / tot_blows) : 0) %
      '%';
    str += buf;

    long tot_hits = stats.combat_hits[PC_STAT] + stats.combat_hits[MOB_STAT];
    buf = format("Total Combat hits         : %ld  (%5.2f%c)\n\r") %
      tot_hits % (tot_blows == 0 ? 0.0 : (100.0 * tot_hits / tot_blows)) % '%';
    str += buf;
    buf = format("\tMob combat hits            : %ld   (%5.2f%c)  (%5.2f%c hit rate)\n\r") %
      stats.combat_hits[MOB_STAT] % (tot_hits ? (100.0 * stats.combat_hits[MOB_STAT] / tot_hits) : 0) % '%' %
      (stats.combat_blows[MOB_STAT] ? stats.combat_hits[MOB_STAT] * 100.0 / stats.combat_blows[MOB_STAT] : 0) % '%';
    str += buf;
    buf = format("\tPC combat hits             : %ld   (%5.2f%c)  (%5.2f%c hit rate)\n\r") %
      stats.combat_hits[PC_STAT] % (tot_hits ? (100.0 * stats.combat_hits[PC_STAT] / tot_hits) : 0 ) % '%' %
      (stats.combat_blows[PC_STAT] ? stats.combat_hits[PC_STAT] * 100.0 / stats.combat_blows[PC_STAT] : 0) % '%';
    str += buf;

    int tot_lev = stats.combat_level[PC_STAT] + stats.combat_level[MOB_STAT];
    buf = format("Average Combat level      : %5.2f\n\r") %
      (tot_blows ?  ((double) tot_lev / (double) tot_blows) : 0.0);
    str += buf;
    buf = format("\tMob combat level           : %5.2f\n\r") %
         (stats.combat_blows[MOB_STAT] ?  ((double) stats.combat_level[MOB_STAT] / (double) stats.combat_blows[MOB_STAT]) : 0.0);
    str += buf;
    buf = format("\tPC combat level            : %5.2f\n\r") %
      (stats.combat_blows[PC_STAT] ?  ((double) stats.combat_level[PC_STAT] / (double) stats.combat_blows[PC_STAT]) : 0.0);
    str += buf;

    buf = format("Average Combat damage     : %5.2f\n\r") %
      (tot_hits ?  ((double) tot_dam / (double) tot_hits) : 0.0);
    str += buf;
    buf = format("\tMob avg. combat dam.       : %5.2f\n\r") %
      (stats.combat_hits[MOB_STAT] ?  ((double) stats.combat_damage[MOB_STAT] / (double) stats.combat_hits[MOB_STAT]) : 0.0);
    str += buf;
    buf = format("\tPC avg. combat dam.        : %5.2f\n\r") %
      (stats.combat_hits[PC_STAT] ?  ((double) stats.combat_damage[PC_STAT] / (double) stats.combat_hits[PC_STAT]) : 0.0);
    str += buf;

    buf = format("Total crit-success checks : %ld  (%5.2f%c of hits)\n\r") %
      stats.combat_crit_suc %
      (tot_hits == 0 ? 0.0 : 
       (100.0 * stats.combat_crit_suc / tot_hits)) % '%';
    str += buf;
    buf = format("Total crit-fail checks    : %ld  (%5.2f%c of misses)\n\r") %
           stats.combat_crit_fail %
      ((tot_blows - tot_hits) == 0 ? 0.0 : 
       (100.0 * stats.combat_crit_fail / (tot_blows - tot_hits))) % '%';
    str += buf;
    buf = format("Total crit-success passes : %ld  (%5.2f%c of hits)\n\r") %
      stats.combat_crit_suc_pass %
      (tot_hits == 0 ? 0.0 : 
       (100.0 * stats.combat_crit_suc_pass / tot_hits)) % '%';
    str += buf;
    buf = format("Total crit-fail passes    : %ld  (%5.2f%c of misses)\n\r") %
            stats.combat_crit_fail_pass %
      ((tot_blows - tot_hits) == 0 ? 0.0 : 
       (100.0 * stats.combat_crit_fail_pass / (tot_blows - tot_hits))) % '%';
    str += buf;
    str += "\n\r";
    buf = format("Mobiles have tried to aggro : %d times.\n\r") % stats.aggro_attempts;
    str += buf;
    buf = format("Mobiles have aggro'd        : %d times.\n\r") % stats.aggro_successes;
    str += buf;
    if (desc)
      desc->page_string(str, SHOWNOW_NO, ALLOWREP_YES);
    return;
  } else if (is_abbrev(buf, "equipment")) {
    sendTo(format("Current Equipment Load Modifier : %4.2f\n\r") % stats.equip);
    sendTo(format("Current Max-Exist Modifier      : %4.2f\n\r") % stats.max_exist);
    sendTo(format("Current Mob-Money Modifier      : %4.2f\n\r") % gold_modifier[GOLD_INCOME].getVal());
    sendTo(format("Current Mob-XP Modifier         : %4.2f\n\r") % stats.xp_modif);
    sendTo(format("Current Damage Modifier         : %4.2f\n\r") % stats.damage_modifier);
    return;
  } else if (is_abbrev(buf, "trivia")) {
    sendTo(format("Average HP regen              : %4.2f  (attempts : %d)\n\r") %        (stats.hit_gained_attempts == 0 ? 0.0 :
        ((float) stats.hit_gained / (float) stats.hit_gained_attempts)) %
        stats.hit_gained_attempts);
    sendTo(format("Average MV regen              : %4.2f  (attempts : %d)\n\r") %        (stats.move_gained_attempts == 0 ? 0.0 :
        ((float) stats.move_gained / (float) stats.move_gained_attempts)) %
        stats.move_gained_attempts);
    sendTo(format("Average mana regen            : %4.2f  (attempts : %d)\n\r") %        (stats.mana_gained_attempts == 0 ? 0.0 :
        ((float) stats.mana_gained / (float) stats.mana_gained_attempts)) %
        stats.mana_gained_attempts);
    sendTo(format("Average piety regen           : %4.2f  (attempts : %d)\n\r") %        (stats.piety_gained_attempts == 0 ? 0.0 :
        (stats.piety_gained / (float) stats.piety_gained_attempts)) %
        stats.piety_gained_attempts);
    return;
  } else if (is_abbrev(buf, "levels")) {
    if(buf2.empty()){
      sendTo("Syntax : gamestats levels <level>\n\r");
      return;
    }
    lev = convertTo<int>(buf2);
    if ((lev >= 0) && (lev < 71)) {
      sendTo(format("Mobile Deaths for level %d, %ld\n\r") % lev % stats.deaths[lev][1]);
      sendTo(format("PC  Deaths for level %d, %ld\n\r") % lev % stats.deaths[lev][0]);

      if (lev >= 1 && lev <= 50) {
        sendTo("PC Leveling Data:\n\r");
        unsigned int factor = secs_to_level(lev);
        sendTo(format("Desired leveling time: %s\n\r") %
           secsToString(factor));

        for (i=0;i<MAX_CLASSES;i++) {
          time_t factor = 0;
          if (stats.levels[i][lev-1])
            factor = SECS_PER_REAL_MIN * stats.time_levels[i][lev-1] / stats.levels[i][lev-1];

          sendTo(format("Class: %-10.10s :    number %3d,  avg. time: %s\n\r") %
            classInfo[i].name % stats.levels[i][lev-1] %
            secsToString(factor));
        }
      }
      return;
    } else {
      sendTo("Please use a level from 0 - 70 in your level checking.\n\r");
      return;
    }
  }
  sendTo("Syntax: gamestats <attributes | combat | equipment | levels | trivia>\n\r");
}

int getNetGold(moneyTypeT mtt)
{
  int net_gold = 0;
  int i;
  for (i = 0; i < MAX_MORT; i++)
    net_gold += gold_statistics[mtt][i];
  return net_gold;
}

unsigned int getPosGold(moneyTypeT mtt)
{
  unsigned int pos_gold = 0;
  int i;
  for (i = 0; i < MAX_MORT; i++)
    pos_gold += gold_positive[mtt][i];
  return pos_gold;
}

int getNetGoldGlobal()
{
  int net_gold = 0;
  int i;
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
  int i;
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
  int i;
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
  int i;
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
  int i;
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
  int i;
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

// procCheckGoldStats
procCheckGoldStats::procCheckGoldStats(const int &p)
{
  trigger_pulse=p;
  name="procCheckGoldStats";
}

void procCheckGoldStats::run(const TPulse &) const
{
  // insure we have enough data to take accurate reading
  unsigned int pos_gold = getPosGoldGlobal();
  if (pos_gold < 5000000U)
//  if (pos_gold < 2000000U)
    return;

  int net_gold = getNetGoldGlobal();
  int net_gold_all_shops = getNetGoldShops();
  int net_gold_budget = getNetGoldBudget();
  unsigned int pos_gold_all_shops = getPosGoldShops();
  unsigned int pos_gold_budget = getPosGoldBudget();

  bool should_reset = false;

  // want shops to make money (roughly 5% of total)
  if (net_gold_all_shops > 0) {
    // shops are giving out too much money
    gold_modifier[GOLD_SHOP] -= 0.01;
//    vlogf(LOG_BUG, format("ECONOMY: shop modifier lowered. %d %u %.2f") %  net_gold_all_shops % pos_gold_all_shops % gold_modifier[GOLD_SHOP].getVal());
    should_reset = true;
  } else if ((unsigned int) -net_gold_all_shops > pos_gold_all_shops/10) {
    // shops are making too much money
    gold_modifier[GOLD_SHOP] += 0.01;
//    vlogf(LOG_BUG, format("ECONOMY: shop modifier raised. %d %u %.2f") %  net_gold_all_shops % pos_gold_all_shops % gold_modifier[GOLD_SHOP].getVal());
    should_reset = true;
  }

  // overall, would like players to be gaining slightly on gold (2% target)
  float target_income = 0.02;
  if ((unsigned int) net_gold_budget < ((target_income - 0.03) * pos_gold_budget)) {
    // players losing money
    gold_modifier[GOLD_INCOME] += 0.01;
//    vlogf(LOG_BUG, format("ECONOMY: income modifier raised. %d %u %.2f") %  net_gold_budget % pos_gold_budget % gold_modifier[GOLD_INCOME].getVal());
    should_reset = true;
  } else if ((unsigned int) net_gold_budget > ((target_income + 0.03) * pos_gold_budget)) {
    // players making too much
    gold_modifier[GOLD_INCOME] -= 0.01;
//    vlogf(LOG_BUG, format("ECONOMY: income modifier lowered. %d %u %.2f") %  net_gold_budget % pos_gold_budget % gold_modifier[GOLD_INCOME].getVal());
    should_reset = true;
  }
  
  // good drain:
  // components and symbol purchasing
  // armor and weapon purchasing (upgrading)
  // purchases from response mobs (in general, components)
  // money spent repairing eq
  // food purchases
  // income money : unrecovered corpse?

  // rent costs are really off budget
  // we definitely do not want rent to be significant part of economy
  float target_rent = 0.10;

  // get the drain associated with renting
  int rent_drain = getPosGold(GOLD_RENT) - getNetGold(GOLD_RENT);
  
  // realize that we are flucuating these costs based on these costs
  // which is rather problematic
  // we essentially want the "true" rent drain
  int adj_rent = (int) (rent_drain / gold_modifier[GOLD_RENT].getVal());

  int total_drain = pos_gold - net_gold;

  if (adj_rent < (int) ((target_rent - .05) * total_drain)) {
    // rent is too small a drain
    // reduce the cost of renting so that folks will be able to rent more
    gold_modifier[GOLD_RENT] -= 0.01;
    should_reset = true;
  } else if (adj_rent > (int) ((target_rent + .05) * total_drain)) {
    // rent is too large a drain
    // raise the cost of renting so that folks will be able to rent less
    gold_modifier[GOLD_RENT] += 0.01;
    should_reset = true;
  }

#if 0
  // desire money from eq be no more than 25% of total
  // that is, most money comes from raw loads on mobs (commods, gold, etc)
  const double target_eq = 0.25;
  unsigned int pos_gold_shop_arm = getPosGold(GOLD_SHOP_ARMOR);
  unsigned int pos_gold_shop_weap = getPosGold(GOLD_SHOP_WEAPON);
  double eq_factor = (double) (pos_gold_shop_arm + pos_gold_shop_weap) /
                     pos_gold;
  if (eq_factor < (target_eq - 0.05)) {
    // too little money from EQ
    stats.equip += 0.01;
    should_reset = true;
  } else if (eq_factor > (target_eq + 0.05)) {
    // too much money from EQ
    stats.equip -= 0.01;
    should_reset = true;
  }
#endif

  if (should_reset) {
    memset(&gold_statistics, 0, sizeof(gold_statistics));
    memset(&gold_positive, 0, sizeof(gold_positive));
    save_game_stats();
  }
}
