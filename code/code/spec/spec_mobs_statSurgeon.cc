#include "monster.h"

const int COST = 1000000;

// for COST the surgeon will rebalance 2 stats within racial norms
//  group I: STR, BRA, CON
//  group II: DEX, AGI, SPE
//  'buy <stat1> <stat2> <changeinstat1>'
int statSurgeon(TBeing *ch, cmdTypeT cmd, const char *arg, TMonster *surg, TObj *)
{
  if (cmd != CMD_BUY) {
    return FALSE;
  }

  sstring sarg = arg;
  std::vector <statTypeT> stats;
  int i, j;
  for (i = 0;i < 2;i++) {
    if (is_abbrev(sarg.word(i), "speed"))
      stats.push_back(STAT_SPE);
    else if (is_abbrev(sarg.word(i), "agility"))
      stats.push_back(STAT_AGI); 
    else if (is_abbrev(sarg.word(i), "dexterity"))
      stats.push_back(STAT_DEX); 
    else if (is_abbrev(sarg.word(i), "strength"))
      stats.push_back(STAT_STR); 
    else if (is_abbrev(sarg.word(i), "brawn"))
      stats.push_back(STAT_BRA); 
    else if (is_abbrev(sarg.word(i), "constitution"))
      stats.push_back(STAT_CON); 
    else {
      surg->doSay("Hm, I don't quite follow you there.");
      return TRUE;
    }
  }
// catch both same or changeBy 0
  int changeBy = convertTo<int>(sarg.word(2));
  if (stats[0] == stats[1] || changeBy == 0) {
    surg->doSay("I doubt you really want to pay for me to do something that will have no effect.");
    surg->doSay("Don't waste my time.");
    return TRUE;
  }
  
  std::vector <int> group1;
  group1.push_back(STAT_SPE);
  group1.push_back(STAT_DEX);
  group1.push_back(STAT_AGI);
  
  std::vector <int> group2;
  group2.push_back(STAT_STR);
  group2.push_back(STAT_BRA);
  group2.push_back(STAT_CON);

  int ct1 = 0, ct2 = 0;
  for (i = 0;i < 2; i++) {
    for (j = 0; j < 3; j++) {
      if (stats[i] == group1[j]) ct1++;
      else if (stats[i] == group2[j]) ct2++;
    }
  }
  if (ct1 < 2 && ct2 < 2) {
    surg->doSay("I just don't think that I'm up to that challenge.");
    surg->doSay("Just THINK of how much I could CHARGE for that!");
    return TRUE;
  }

  int st1 = ch->getStat(STAT_CHOSEN, stats[0]);
  int st2 = ch->getStat(STAT_CHOSEN, stats[1]);
  if ( (st1 + changeBy) > 25 || (st1 + changeBy) < -25 ||
       (st2 - changeBy) > 25 || (st2 - changeBy) < -25) {
    surg->doSay("I just don't think that I'm up to that challenge.");
    surg->doSay("Just THINK of how much I could CHARGE for that!");
    return TRUE;
  }
 
  if (ch->getMoney() >= COST) {
    ch->addToMoney((-COST), GOLD_SHOP_RESPONSES);
    surg->doSay("Thanks... ah yes, I do love gold.");
  } else {
    surg->doSay("So sorry!  Is seems that you can't afford my services after all.");
    return TRUE;
  }
  
  act("You touch $P on the forehead.", TRUE, surg, NULL, ch, TO_CHAR);
  act("$P touches you on the forehead.", TRUE, ch, NULL, surg, TO_CHAR);
  ch->doSleep("");
  act("$n mumbles something and touches $P on the forehead.", TRUE, surg, NULL, ch, TO_ROOM);
  act("You operate on $P.", TRUE, surg, NULL, ch, TO_CHAR);
  act("$n pulls out a long thin blade and recites words in a strange language as he repeatedly cuts and miraculously heals $P.", TRUE, surg, 0, ch, TO_ROOM);
  
  ch->setStat(STAT_CHOSEN, stats[0], st1 + changeBy);
  ch->setStat(STAT_CHOSEN, stats[1], st2 - changeBy);
  
  surg->doSay("That's that then.");
  surg->doSay("Hope I didn't forget anything.");
  ch->doSave(SILENT_NO);
  return TRUE;
}
