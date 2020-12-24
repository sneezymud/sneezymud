//////////////////////////////////////////////////////////////////////////
//
// SneezyMUD - All rights reserved, SneezyMUD Coding Team
//
//     spell_info.cc : All functions related to SpellInfo class
//
//     Copyright (c) 1998, SneezyMUD Development Team
//     All Rights reserved.
//
/////////////////////////////////////////////////////////////////

#include "extern.h"
#include "spell2.h"
#include "toggle.h"
#include "stats.h"

spellInfo *discArray[MAX_SKILL+1];

spellInfo::spellInfo(skillUseClassT styp,
  discNumT discipline, 
  discNumT assDiscipline,
  statTypeT modifierStat,
  const char *n, taskDiffT cast_diff, lag_t l, positionTypeT pos, 
  manaCostT mana,
  lifeforceCostT lifeforce,
  pietyCostT align,
  unsigned int t,
  symbolStressT h, 
  const char *fa, 
  const char *far, 
  const char *fas, 
  const char *fasr,
  discStartT starting,
  discLearnT learning,
  discStartDoT learnDoStarting,
  discLearnDoT learningAmt, 
  discStartDoT secLearnDoStart,
  discLearnDoT secLearnDoAmt,
  int learningDoDiff, float modifier, unsigned int ctyp, unsigned int tgl) :
  name(n),
  modifierStat(modifierStat),
  start(starting),
  learn(learning),
  uses(0),
  levels(0),
  learned(0),
  damage(0),
  pot_damage(0),
  pot_victims(0),
  pot_level(0),
  victims(0),
  crits(0),
  critf(0),
  success(0),
  potSuccess(0),
  fail(0),
  focusValue(0),
  newAttempts(0),
  lowAttempts(0),
  midAttempts(0),
  goodAttempts(0),
  highAttempts(0),
  engAttempts(0),
  genFail(0),
  focFail(0),
  engFail(0),
  saves(0),
  learnAttempts(0),
  learnSuccess(0),
  learnLearn(0),
  learnLevel(0),
  learnFail(0),
  learnBoost(0),
  learnDiscSuccess(0),
  learnAdvDiscSuccess(0),
  mobUses(0),
  mobLevels(0),
  mobLearned(0),
  mobDamage(0),
  mobVictims(0),
  mobCrits(0),
  mobCritf(0),
  mobSuccess(0),
  potSuccessMob(0),
  mobFail(0),
  mobSaves(0),
  immUses(0),
  immLevels(0),
  immLearned(0),
  immDamage(0),
  immVictims(0),
  immCrits(0),
  immCritf(0),
  immSuccess(0),
  potSuccessImm(0),
  immFail(0),
  immSaves(0),
  lag(l),
  typ(styp),
  task(cast_diff),
  minPosition(pos),
  minMana(mana),
  minLifeforce(lifeforce),
  minPiety(align/4.0),  // we wanted a float, but had the enum as an int
  targets(t),
  holyStrength(h),
  fadeAway(fa),
  fadeAwayRoom(far),
  fadeAwaySoon(fas),
  fadeAwaySoonRoom(fasr),
  alignMod(modifier),
  comp_types(ctyp),
  toggle(tgl),
  disc(discipline),
  assDisc(assDiscipline),
  startLearnDo(learnDoStarting),
  amtLearnDo(learningAmt),
  learnDoDiff(learningDoDiff),
  secStartLearnDo(secLearnDoStart),
  secAmtLearnDo(secLearnDoAmt)
{
}

spellInfo::spellInfo(const spellInfo &a) :
  name(a.name),
  modifierStat(a.modifierStat),
  start(a.start),
  learn(a.learn),
  uses(a.uses),
  levels(a.levels),
  learned(a.learned),
  damage(a.damage),
  pot_damage(a.pot_damage),
  pot_victims(a.pot_victims),
  pot_level(a.pot_level),
  victims(a.victims),
  crits(a.crits),
  critf(a.critf),
  success(a.success),
  potSuccess(a.potSuccess),
  fail(a.fail),
  focusValue(a.focusValue),
  newAttempts(a.newAttempts),
  lowAttempts(a.lowAttempts),
  midAttempts(a.midAttempts),
  goodAttempts(a.goodAttempts),
  highAttempts(a.highAttempts),
  engAttempts(a.engAttempts),
  genFail(a.genFail),
  focFail(a.focFail),
  engFail(a.engFail),
  saves(a.saves),
  learnAttempts(a.learnAttempts),
  learnSuccess(a.learnSuccess),
  learnLearn(a.learnLearn),
  learnLevel(a.learnLevel),
  learnFail(a.learnFail),
  learnBoost(a.learnBoost),
  learnDiscSuccess(a.learnDiscSuccess),
  learnAdvDiscSuccess(a.learnAdvDiscSuccess),
  mobUses(a.mobUses),
  mobLevels(a.mobLevels),
  mobLearned(a.mobLearned),
  mobDamage(a.mobDamage),
  mobVictims(a.mobVictims),
  mobCrits(a.mobCrits),
  mobCritf(a.mobCritf),
  mobSuccess(a.mobSuccess),
  potSuccessMob(a.potSuccessMob),
  mobFail(a.mobFail),
  mobSaves(a.mobSaves),
  immUses(a.immUses),
  immLevels(a.immLevels),
  immLearned(a.immLearned),
  immDamage(a.immDamage),
  immVictims(a.immVictims),
  immCrits(a.immCrits),
  immCritf(a.immCritf),
  immSuccess(a.immSuccess),
  potSuccessImm(a.potSuccessImm),
  immFail(a.immFail),
  immSaves(a.immSaves),
  lag(a.lag),
  typ(a.typ),
  task(a.task),
  minPosition(a.minPosition),
  minMana(a.minMana),
  minLifeforce(a.minLifeforce),
  minPiety(a.minPiety),
  targets(a.targets),
  holyStrength(a.holyStrength),
  fadeAway(a.fadeAway),
  fadeAwayRoom(a.fadeAwayRoom),
  fadeAwaySoon(a.fadeAwaySoon),
  fadeAwaySoonRoom(a.fadeAwaySoonRoom),
  alignMod(a.alignMod),
  comp_types(a.comp_types),
  toggle(a.toggle),
  disc(a.disc),
  assDisc(a.assDisc),
  startLearnDo(a.startLearnDo),
  amtLearnDo(a.amtLearnDo),
  learnDoDiff(a.learnDoDiff),
  secStartLearnDo(a.secStartLearnDo),
  secAmtLearnDo(a.secAmtLearnDo)
{
  sectorData = a.sectorData;
  weatherData = a.weatherData;
}

spellInfo & spellInfo::operator = (const spellInfo &a)
{
  if (this == &a) return *this;

  name = a.name;
  modifierStat = a.modifierStat;
  start = a.start;
  learn = a.learn;
  uses = a.uses;
  levels = a.levels;
  learned = a.learned;
  damage = a.damage;
  pot_damage = a.pot_damage;
  pot_victims = a.pot_victims;
  pot_level = a.pot_level;
  victims = a.victims;
  crits = a.crits;
  critf = a.critf;
  success = a.success;
  potSuccess = a.potSuccess;
  fail = a.fail;
  focusValue = a.focusValue;
  newAttempts = a.newAttempts;
  lowAttempts = a.lowAttempts;
  midAttempts = a.midAttempts;
  goodAttempts = a.goodAttempts;
  highAttempts = a.highAttempts;
  engAttempts = a.engAttempts;
  genFail = a.genFail;
  focFail = a.focFail;
  engFail = a.engFail;
  saves = a.saves;
  learnAttempts = a.learnAttempts;
  learnSuccess = a.learnSuccess;
  learnLearn = a.learnLearn;
  learnLevel = a.learnLevel;
  learnFail = a.learnFail;
  learnBoost = a.learnBoost;
  learnDiscSuccess = a.learnDiscSuccess;
  learnAdvDiscSuccess = a.learnAdvDiscSuccess;
  mobUses = a.mobUses;
  mobLevels = a.mobLevels;
  mobLearned = a.mobLearned;
  mobDamage = a.mobDamage;
  mobVictims = a.mobVictims;
  mobCrits = a.mobCrits;
  mobCritf = a.mobCritf;
  mobSuccess = a.mobSuccess;
  potSuccessMob = a.potSuccessMob;
  mobFail = a.mobFail;
  mobSaves = a.mobSaves;
  immUses = a.immUses;
  immLevels = a.immLevels;
  immLearned = a.immLearned;
  immDamage = a.immDamage;
  immVictims = a.immVictims;
  immCrits = a.immCrits;
  immCritf = a.immCritf;
  immSuccess = a.immSuccess;
  potSuccessImm = a.potSuccessImm;
  immFail = a.immFail;
  immSaves = a.immSaves;
  lag = a.lag;
  typ = a.typ;
  task = a.task;
  minPosition = a.minPosition;
  minMana = a.minMana;
  minLifeforce = a.minLifeforce;
  minPiety = a.minPiety;
  targets = a.targets;
  holyStrength = a.holyStrength;
  fadeAway = a.fadeAway;
  fadeAwayRoom = a.fadeAwayRoom;
  fadeAwaySoon = a.fadeAwaySoon;
  fadeAwaySoonRoom = a.fadeAwaySoonRoom;
  alignMod = a.alignMod;
  comp_types = a.comp_types;
  toggle = a.toggle;
  disc = a.disc;
  assDisc = a.assDisc;
  startLearnDo = a.startLearnDo;
  amtLearnDo = a.amtLearnDo;
  learnDoDiff = a.learnDoDiff;
  secStartLearnDo = a.secStartLearnDo;
  secAmtLearnDo = a.secAmtLearnDo;

  sectorData.erase(sectorData.begin(), sectorData.end());
  sectorData = a.sectorData;
  weatherData.erase(weatherData.begin(), weatherData.end());
  weatherData = a.weatherData;

  return *this;
}

spellInfo::~spellInfo()
{
  sectorData.erase(sectorData.begin(), sectorData.end());
  weatherData.erase(weatherData.begin(), weatherData.end());
}

void buildSpellArray()
{
  // this memset is done so that unassigned discArrays are set to NULL
  memset((char *) discArray, 0, sizeof(discArray));

// MAGE CLASS

  // disc_mage

  discArray[SPELL_GUST] = new spellInfo(SPELL_MAGE, DISC_MAGE, DISC_AIR, STAT_INT, "gust", TASK_NORMAL, LAG_1, POSITION_SITTING, MANA_10, LIFEFORCE_0, PRAY_0, TAR_VIOLENT | TAR_FIGHT_VICT | TAR_CHAR_ROOM | TAR_SELF_NONO, SYMBOL_STRESS_0, "", "", "", "", START_1, LEARN_100, START_DO_30, LEARN_DO_5, START_DO_NO, LEARN_DO_NO, LEARN_DIFF_SPELLS, 0.04, COMP_GESTURAL | COMP_GESTURAL_RANDOM | COMP_VERBAL | COMP_VERBAL_RANDOM | SPELL_TASKED, 0);

  discArray[SPELL_SLING_SHOT] = new spellInfo(SPELL_MAGE, DISC_MAGE, DISC_EARTH, STAT_INT, "sling shot", TASK_EASY, LAG_1, POSITION_SITTING, MANA_15, LIFEFORCE_0, PRAY_0, TAR_CHAR_ROOM | TAR_VIOLENT | TAR_FIGHT_VICT | TAR_SELF_NONO , SYMBOL_STRESS_0, "", "", "", "", START_1, LEARN_100, START_DO_30, LEARN_DO_5, START_DO_NO, LEARN_DO_NO, LEARN_DIFF_SPELLS, 0.04, COMP_GESTURAL | COMP_GESTURAL_RANDOM | COMP_VERBAL | COMP_VERBAL_RANDOM | SPELL_TASKED, 0);

  discArray[SPELL_GUSHER] = new spellInfo(SPELL_MAGE, DISC_MAGE, DISC_WATER, STAT_INT, "gusher", TASK_EASY, LAG_1, POSITION_SITTING, MANA_20, LIFEFORCE_0, PRAY_0, TAR_CHAR_ROOM | TAR_SELF_NONO | TAR_VIOLENT | TAR_FIGHT_VICT , SYMBOL_STRESS_0, "", "", "", "", START_1, LEARN_100, START_DO_30, LEARN_DO_5, START_DO_NO, LEARN_DO_NO, LEARN_DIFF_SPELLS, 0.02, COMP_GESTURAL | COMP_GESTURAL_RANDOM | COMP_VERBAL | COMP_VERBAL_RANDOM | SPELL_TASKED, 0);

  discArray[SPELL_HANDS_OF_FLAME] = new spellInfo(SPELL_MAGE, DISC_MAGE, DISC_FIRE, STAT_INT, "hands of flame", TASK_EASY, LAG_1, POSITION_SITTING, MANA_20, LIFEFORCE_0, PRAY_0, TAR_CHAR_ROOM | TAR_SELF_NONO | TAR_VIOLENT | TAR_FIGHT_VICT , SYMBOL_STRESS_0, "", "", "", "", START_10, LEARN_100, START_DO_30, LEARN_DO_5, START_DO_NO, LEARN_DO_NO, LEARN_DIFF_SPELLS, 0.02, COMP_GESTURAL | COMP_GESTURAL_RANDOM | COMP_VERBAL | COMP_VERBAL_RANDOM | SPELL_TASKED, 0);

  discArray[SPELL_MYSTIC_DARTS] = new spellInfo(SPELL_MAGE, DISC_MAGE, DISC_SORCERY, STAT_INT, "mystic darts", TASK_NORMAL, LAG_1, POSITION_SITTING, MANA_23, LIFEFORCE_0, PRAY_0, TAR_CHAR_ROOM | TAR_SELF_NONO | TAR_VIOLENT | TAR_FIGHT_VICT , SYMBOL_STRESS_0, "", "", "", "", START_12, LEARN_25, START_DO_30, LEARN_DO_5, START_DO_NO, LEARN_DO_NO, LEARN_DIFF_SPELLS, 0.04, COMP_GESTURAL | COMP_GESTURAL_RANDOM | COMP_VERBAL | COMP_VERBAL_RANDOM | SPELL_TASKED, 0);


  discArray[SPELL_FLARE] = new spellInfo(SPELL_MAGE, DISC_MAGE, DISC_FIRE, STAT_EXT, "flare", TASK_TRIVIAL, LAG_1, POSITION_CRAWLING, MANA_25, LIFEFORCE_0, PRAY_0, TAR_IGNORE, SYMBOL_STRESS_0, "", "", "", "", START_6, LEARN_35, START_DO_45, LEARN_DO_7, START_DO_NO, LEARN_DO_NO, LEARN_DIFF_SPELLS, 0.01, COMP_GESTURAL | COMP_GESTURAL_RANDOM | COMP_VERBAL | COMP_VERBAL_RANDOM | COMP_MATERIAL | COMP_MATERIAL_END | SPELL_TASKED, 0);

  discArray[SPELL_SORCERERS_GLOBE] = new spellInfo(SPELL_MAGE, DISC_MAGE, DISC_SORCERY, STAT_INT, "sorcerers globe", TASK_TRIVIAL, LAG_1, POSITION_CRAWLING, MANA_15, LIFEFORCE_0, PRAY_0, TAR_CHAR_ROOM | TAR_FIGHT_SELF, SYMBOL_STRESS_0, "The air around you seems to become less dense.", "The globe about $n wavers, then collapses altogether.", "The air around you seems to become less dense.", "$n's magic globe wavers.", START_1, LEARN_8, START_DO_50, LEARN_DO_5, START_DO_NO, LEARN_DO_NO, LEARN_DIFF_SPELLS, 0.01, COMP_GESTURAL | COMP_GESTURAL_RANDOM | COMP_VERBAL | COMP_VERBAL_RANDOM | COMP_MATERIAL | COMP_MATERIAL_END | SPELL_TASKED, 0);

  discArray[SPELL_FAERIE_FIRE] = new spellInfo(SPELL_MAGE, DISC_MAGE, DISC_FIRE, STAT_INT, "faerie fire", TASK_EASY, LAG_2, POSITION_SITTING, MANA_15, LIFEFORCE_0, PRAY_0, TAR_VIOLENT | TAR_FIGHT_VICT | TAR_CHAR_ROOM | TAR_SELF_NONO | TAR_SELF_NONO, SYMBOL_STRESS_0, "The pink aura around your body fades away.", "$n's pink aura fades away.", "The pink aura around your body flickers slightly.", "The pink aura around $n's body flickers slightly.", START_15, LEARN_14, START_DO_35, LEARN_DO_7, START_DO_NO, LEARN_DO_NO, LEARN_DIFF_SPELLS, 0.03, COMP_GESTURAL | COMP_GESTURAL_RANDOM | COMP_VERBAL | COMP_VERBAL_RANDOM | COMP_MATERIAL | COMP_MATERIAL_END | SPELL_TASKED, 0);

  discArray[SPELL_ILLUMINATE] = new spellInfo(SPELL_MAGE, DISC_MAGE, DISC_ALCHEMY, STAT_INT, "illuminate", TASK_TRIVIAL, LAG_1, POSITION_CRAWLING, MANA_20, LIFEFORCE_0, PRAY_0, TAR_OBJ_INV | TAR_OBJ_EQUIP, SYMBOL_STRESS_0, "", "", "", "", START_3, LEARN_17, START_DO_35, LEARN_DO_7, START_DO_NO, LEARN_DO_NO, LEARN_DIFF_SPELLS, 0.0, COMP_GESTURAL | COMP_GESTURAL_RANDOM | COMP_VERBAL | COMP_VERBAL_RANDOM | COMP_MATERIAL | COMP_MATERIAL_END | SPELL_TASKED, 0);

  discArray[SPELL_DETECT_MAGIC] = new spellInfo(SPELL_MAGE, DISC_MAGE, DISC_ALCHEMY, STAT_INT, "detect magic", TASK_TRIVIAL, LAG_1, POSITION_CRAWLING, MANA_15, LIFEFORCE_0, PRAY_0, TAR_CHAR_ROOM | TAR_FIGHT_SELF, SYMBOL_STRESS_0, "Your eyes don't tingle anymore.", "$n's eyes don't twinkle anymore.", "You blink as your eyes sting for a moment.", "$n's eyes seem to water a bit.", START_5, LEARN_17, START_DO_40, LEARN_DO_5, START_DO_NO, LEARN_DO_NO, LEARN_DIFF_SPELLS, 0.0, COMP_GESTURAL | COMP_GESTURAL_RANDOM | COMP_VERBAL | COMP_VERBAL_RANDOM | COMP_MATERIAL | COMP_MATERIAL_END | SPELL_TASKED, 0);

  discArray[SPELL_STUNNING_ARROW] = new spellInfo(SPELL_MAGE, DISC_MAGE, DISC_SORCERY, STAT_INT, "stunning arrow", TASK_NORMAL, LAG_1, POSITION_SITTING, MANA_45, LIFEFORCE_0, PRAY_0, TAR_CHAR_ROOM | TAR_SELF_NONO | TAR_VIOLENT | TAR_FIGHT_VICT , SYMBOL_STRESS_0, "", "", "", "", START_34, LEARN_25, START_DO_40, LEARN_DO_5, START_DO_NO, LEARN_DO_NO, LEARN_DIFF_SPELLS, 0.04, COMP_GESTURAL | COMP_GESTURAL_RANDOM | COMP_VERBAL | COMP_VERBAL_RANDOM | COMP_MATERIAL | COMP_MATERIAL_END | SPELL_TASKED, 0);

  discArray[SPELL_MATERIALIZE] = new spellInfo(SPELL_MAGE, DISC_MAGE, DISC_ALCHEMY, STAT_EXT, "materialize", TASK_TRIVIAL, LAG_1, POSITION_CRAWLING, MANA_15, LIFEFORCE_0, PRAY_0, TAR_NAME, SYMBOL_STRESS_0, "", "", "", "", START_15, LEARN_25, START_DO_40, LEARN_DO_7, START_DO_NO, LEARN_DO_NO, LEARN_DIFF_SPELLS, 0.0, COMP_GESTURAL | COMP_GESTURAL_RANDOM | COMP_VERBAL | COMP_VERBAL_RANDOM | SPELL_TASKED, 0);

  discArray[SPELL_PROTECTION_FROM_EARTH] = new spellInfo(SPELL_MAGE, DISC_MAGE, DISC_EARTH, STAT_INT, "protection from earth", TASK_TRIVIAL, LAG_2, POSITION_CRAWLING, MANA_15, LIFEFORCE_0, PRAY_0, TAR_CHAR_ROOM | TAR_FIGHT_SELF, SYMBOL_STRESS_0, "You feel slightly less protected.", "", "", "", START_24, LEARN_20, START_DO_50, LEARN_DO_5, START_DO_NO, LEARN_DO_NO, LEARN_DIFF_SPELLS, 0.01, COMP_GESTURAL | COMP_GESTURAL_RANDOM | COMP_VERBAL | COMP_VERBAL_RANDOM | COMP_MATERIAL | COMP_MATERIAL_END | SPELL_TASKED, 0);

  discArray[SPELL_PROTECTION_FROM_AIR] = new spellInfo(SPELL_MAGE, DISC_MAGE, DISC_AIR, STAT_INT, "protection from air", TASK_TRIVIAL, LAG_1, POSITION_CRAWLING, MANA_15, LIFEFORCE_0, PRAY_0, TAR_CHAR_ROOM | TAR_FIGHT_SELF, SYMBOL_STRESS_0, "You feel slightly less protected.", "", "", "", START_25, LEARN_14, START_DO_50, LEARN_DO_5, START_DO_NO, LEARN_DO_NO, LEARN_DIFF_SPELLS, 0.01, COMP_GESTURAL | COMP_GESTURAL_RANDOM | COMP_VERBAL | COMP_VERBAL_RANDOM | COMP_MATERIAL | COMP_MATERIAL_END | SPELL_TASKED, 0);

  discArray[SPELL_PROTECTION_FROM_FIRE] = new spellInfo(SPELL_MAGE, DISC_MAGE, DISC_FIRE, STAT_INT, "protection from fire", TASK_TRIVIAL, LAG_1, POSITION_CRAWLING, MANA_15, LIFEFORCE_0, PRAY_0, TAR_CHAR_ROOM | TAR_FIGHT_SELF, SYMBOL_STRESS_0, "You feel slightly less protected.", "", "", "", START_25, LEARN_14, START_DO_50, LEARN_DO_5, START_DO_NO, LEARN_DO_NO, LEARN_DIFF_SPELLS, 0.01, COMP_GESTURAL | COMP_GESTURAL_RANDOM | COMP_VERBAL | COMP_VERBAL_RANDOM | COMP_MATERIAL | COMP_MATERIAL_END | SPELL_TASKED, 0);

  discArray[SPELL_PROTECTION_FROM_WATER] = new spellInfo(SPELL_MAGE, DISC_MAGE, DISC_WATER, STAT_INT, "protection from water", TASK_TRIVIAL, LAG_1, POSITION_CRAWLING, MANA_15, LIFEFORCE_0, PRAY_0, TAR_CHAR_ROOM | TAR_FIGHT_SELF, SYMBOL_STRESS_0, "You feel slightly less protected.", "", "", "", START_27, LEARN_23, START_DO_50, LEARN_DO_5, START_DO_NO, LEARN_DO_NO, LEARN_DIFF_SPELLS, 0.01, COMP_GESTURAL | COMP_GESTURAL_RANDOM | COMP_VERBAL | COMP_VERBAL_RANDOM | COMP_MATERIAL | COMP_MATERIAL_END | SPELL_TASKED, 0);

  discArray[SPELL_PROTECTION_FROM_ELEMENTS] = new spellInfo(SPELL_MAGE, DISC_MAGE, DISC_SORCERY, STAT_INT, "protection from elements", TASK_EASY, LAG_2, POSITION_CRAWLING, MANA_15, LIFEFORCE_0, PRAY_0, TAR_CHAR_ROOM | TAR_FIGHT_SELF, SYMBOL_STRESS_0, "You feel slightly less protected.", "", "", "", START_28, LEARN_20, START_DO_50, LEARN_DO_5, START_DO_NO, LEARN_DO_NO, LEARN_DIFF_SPELLS, 0.01, COMP_GESTURAL | COMP_GESTURAL_RANDOM | COMP_VERBAL | COMP_VERBAL_RANDOM | COMP_MATERIAL | COMP_MATERIAL_END | SPELL_TASKED, 0);

  discArray[SPELL_PEBBLE_SPRAY] = new spellInfo(SPELL_MAGE, DISC_MAGE, DISC_EARTH, STAT_INT, "pebble spray", TASK_EASY, LAG_1, POSITION_SITTING, MANA_25, LIFEFORCE_0, PRAY_0, TAR_AREA | TAR_FIGHT_VICT | TAR_VIOLENT | TAR_IGNORE, SYMBOL_STRESS_0, "", "", "", "", START_15, LEARN_15, START_DO_45, LEARN_DO_5, START_DO_NO, LEARN_DO_NO, LEARN_DIFF_SPELLS, 0.04, COMP_GESTURAL | COMP_GESTURAL_RANDOM | COMP_VERBAL | COMP_VERBAL_RANDOM | COMP_MATERIAL | COMP_MATERIAL_END | SPELL_TASKED, 0);

  discArray[SPELL_ARCTIC_BLAST] = new spellInfo(SPELL_MAGE, DISC_MAGE, DISC_WATER, STAT_INT, "arctic blast", TASK_NORMAL, LAG_1, POSITION_SITTING, MANA_25, LIFEFORCE_0, PRAY_0, TAR_AREA | TAR_IGNORE | TAR_VIOLENT, SYMBOL_STRESS_0, "", "", "", "", START_16, LEARN_15, START_DO_45, LEARN_DO_5, START_DO_NO, LEARN_DO_NO, LEARN_DIFF_SPELLS, 0.02, COMP_GESTURAL | COMP_GESTURAL_RANDOM | COMP_VERBAL | COMP_VERBAL_RANDOM | COMP_MATERIAL | COMP_MATERIAL_END | SPELL_TASKED, 0);

  discArray[SPELL_COLOR_SPRAY] = new spellInfo(SPELL_MAGE, DISC_MAGE, DISC_SORCERY, STAT_INT, "color spray", TASK_DIFFICULT, LAG_1, POSITION_SITTING, MANA_38, LIFEFORCE_0, PRAY_0, TAR_AREA | TAR_IGNORE | TAR_FIGHT_VICT | TAR_VIOLENT, SYMBOL_STRESS_0, "", "", "", "", START_35, LEARN_10, START_DO_45, LEARN_DO_5, START_DO_NO, LEARN_DO_NO, LEARN_DIFF_SPELLS, 0.04, COMP_GESTURAL | COMP_GESTURAL_RANDOM | COMP_VERBAL | COMP_VERBAL_RANDOM | COMP_MATERIAL | COMP_MATERIAL_END | SPELL_TASKED, 0);

  discArray[SPELL_INFRAVISION] = new spellInfo(SPELL_MAGE, DISC_MAGE, DISC_FIRE, STAT_INT, "infravision", TASK_TRIVIAL, LAG_2, POSITION_CRAWLING, MANA_20, LIFEFORCE_0, PRAY_0, TAR_CHAR_ROOM | TAR_FIGHT_SELF, SYMBOL_STRESS_0, "Your eyes lose their red glow.", "$n's eyes don't glow red anymore.", "Your eyes seem less sensitive to heat.", "$n's red eyes flicker.", START_44, LEARN_15, START_DO_50, LEARN_DO_5, START_DO_NO, LEARN_DO_NO, LEARN_DIFF_SPELLS, 0.0, COMP_GESTURAL | COMP_GESTURAL_RANDOM | COMP_VERBAL | COMP_VERBAL_RANDOM | COMP_MATERIAL | COMP_MATERIAL_END | SPELL_TASKED, 0);

  discArray[SPELL_IDENTIFY] = new spellInfo(SPELL_MAGE, DISC_MAGE, DISC_ALCHEMY, STAT_EXT, "identify", TASK_TRIVIAL, LAG_2, POSITION_CRAWLING, MANA_20, LIFEFORCE_0, PRAY_0, TAR_OBJ_INV | TAR_OBJ_EQUIP | TAR_CHAR_ROOM, SYMBOL_STRESS_0, "", "", "", "", START_40, LEARN_20, START_DO_50, LEARN_DO_5, START_DO_NO, LEARN_DO_NO, LEARN_DIFF_SPELLS, 0.0, COMP_GESTURAL | COMP_GESTURAL_RANDOM | COMP_VERBAL | COMP_VERBAL_RANDOM | COMP_MATERIAL | COMP_MATERIAL_END | SPELL_TASKED, 0);

  discArray[SPELL_POWERSTONE] = new spellInfo(SPELL_MAGE, DISC_MAGE, DISC_ALCHEMY, STAT_EXT, "powerstone", TASK_EASY, LAG_2, POSITION_CRAWLING, MANA_25, LIFEFORCE_0, PRAY_0, TAR_OBJ_INV, SYMBOL_STRESS_0, "", "", "", "", START_55, LEARN_15, START_DO_50, LEARN_DO_10, START_DO_NO, LEARN_DO_NO, LEARN_DIFF_SPELLS, 0.0, COMP_GESTURAL | COMP_GESTURAL_RANDOM | COMP_VERBAL | COMP_VERBAL_RANDOM | COMP_MATERIAL | COMP_MATERIAL_END | SPELL_TASKED, TOG_HAS_POWERSTONE);

  discArray[SPELL_FAERIE_FOG] = new spellInfo(SPELL_MAGE, DISC_MAGE, DISC_WATER, STAT_INT, "faerie fog", TASK_EASY, LAG_2, POSITION_CRAWLING, MANA_20, LIFEFORCE_0, PRAY_0, TAR_IGNORE, SYMBOL_STRESS_0, "", "", "", "", START_52, LEARN_13, START_DO_40, LEARN_DO_5, START_DO_NO, LEARN_DO_NO, LEARN_DIFF_SPELLS, 0.0, COMP_GESTURAL | COMP_GESTURAL_RANDOM | COMP_VERBAL | COMP_VERBAL_RANDOM | COMP_MATERIAL | COMP_MATERIAL_END | SPELL_TASKED, 0);

  discArray[SPELL_TELEPORT] = new spellInfo(SPELL_MAGE, DISC_MAGE, DISC_SORCERY, STAT_INT, "teleport", TASK_EASY, LAG_1, POSITION_SITTING, MANA_25, LIFEFORCE_0, PRAY_0, TAR_CHAR_ROOM | TAR_FIGHT_SELF, SYMBOL_STRESS_0, "", "", "", "", START_58, LEARN_15, START_DO_40, LEARN_DO_5, START_DO_NO, LEARN_DO_NO, LEARN_DIFF_SPELLS, 0.0, COMP_GESTURAL | COMP_GESTURAL_RANDOM | COMP_VERBAL | COMP_VERBAL_RANDOM | COMP_MATERIAL | COMP_MATERIAL_END | SPELL_TASKED, 0);

  discArray[SPELL_POLYMORPH] = new spellInfo(SPELL_MAGE, DISC_SPIRIT, DISC_SPIRIT, STAT_INT, "polymorph", TASK_EASY, LAG_2, POSITION_CRAWLING, MANA_40, LIFEFORCE_0, PRAY_0, TAR_NAME, SYMBOL_STRESS_0, "You are not able to hold this form any longer.", "", "You do not know how much longer you can hold this form.", "", START_40, LEARN_10, START_DO_50, LEARN_DO_4, START_DO_NO, LEARN_DO_NO, LEARN_DIFF_SPELLS, 0.0, COMP_GESTURAL | COMP_GESTURAL_RANDOM | COMP_VERBAL | COMP_VERBAL_RANDOM | COMP_MATERIAL | COMP_MATERIAL_END | SPELL_TASKED, 0);

  discArray[SPELL_SENSE_LIFE] = new spellInfo(SPELL_MAGE, DISC_MAGE, DISC_SPIRIT, STAT_INT, "sense life", TASK_EASY, LAG_1, POSITION_CRAWLING, MANA_15, LIFEFORCE_0, PRAY_0, TAR_CHAR_ROOM | TAR_FIGHT_SELF, SYMBOL_STRESS_0, "The aqua blue aura fades from your eyes.", "The aqua blue aura fades from $n's eyes.", "The aqua blue in your eyes flickers slightly.", "The aqua blue in $n's eyes flickers slightly.", START_24, LEARN_18, START_DO_40, LEARN_DO_5, START_DO_NO, LEARN_DO_NO, LEARN_DIFF_SPELLS, 0.04, COMP_GESTURAL | COMP_GESTURAL_RANDOM | COMP_VERBAL | COMP_VERBAL_RANDOM | COMP_MATERIAL | COMP_MATERIAL_END | SPELL_TASKED, 0);

  discArray[SPELL_CALM] = new spellInfo(SPELL_MAGE, DISC_MAGE, DISC_SPIRIT, STAT_CHA, "calm", TASK_NORMAL, LAG_1, POSITION_CRAWLING, MANA_25, LIFEFORCE_0, PRAY_0, TAR_CHAR_ROOM | TAR_SELF_NONO | TAR_VIOLENT, SYMBOL_STRESS_0, "You aren't so calm anymore.", "$n doesn't seem as calm as $e was before.", "You are feeling less calm.", "$n is losing $s calm.", START_58, LEARN_16, START_DO_50, LEARN_DO_5, START_DO_NO, LEARN_DO_NO, LEARN_DIFF_SPELLS, 0.04, COMP_GESTURAL | COMP_GESTURAL_RANDOM | COMP_VERBAL | COMP_VERBAL_RANDOM | COMP_MATERIAL | COMP_MATERIAL_END | SPELL_TASKED, 0);

  discArray[SPELL_ACCELERATE] = new spellInfo(SPELL_MAGE, DISC_MAGE, DISC_SPIRIT, STAT_INT, "accelerate", TASK_EASY, LAG_2, POSITION_CRAWLING, MANA_30, LIFEFORCE_0, PRAY_0, TAR_CHAR_ROOM | TAR_FIGHT_SELF, SYMBOL_STRESS_0, "You don't move with as much ease.", "$n doesn't move with as much ease.", "", "", START_32, LEARN_18, START_DO_40, LEARN_DO_5, START_DO_NO, LEARN_DO_NO, LEARN_DIFF_SPELLS, 0.04, COMP_GESTURAL | COMP_GESTURAL_RANDOM | COMP_VERBAL | COMP_VERBAL_RANDOM | COMP_MATERIAL | COMP_MATERIAL_END |SPELL_TASKED, 0);

 discArray[SPELL_DUST_STORM] = new spellInfo(SPELL_MAGE, DISC_MAGE, DISC_AIR, STAT_INT, "dust storm", TASK_NORMAL, LAG_1, POSITION_SITTING, MANA_17, LIFEFORCE_0, PRAY_0, TAR_AREA | TAR_FIGHT_VICT | TAR_VIOLENT | TAR_IGNORE, SYMBOL_STRESS_0, "", "", "", "", START_33, LEARN_13, START_DO_40, LEARN_DO_5, START_DO_NO, LEARN_DO_NO, LEARN_DIFF_SPELLS, 0.04, COMP_GESTURAL | COMP_GESTURAL_RANDOM | COMP_VERBAL | COMP_VERBAL_RANDOM | COMP_MATERIAL | COMP_MATERIAL_END | SPELL_TASKED, 0);

  discArray[SPELL_LEVITATE] = new spellInfo(SPELL_MAGE, DISC_MAGE, DISC_AIR, STAT_INT, "levitate", TASK_EASY, LAG_2, POSITION_SITTING, MANA_25, LIFEFORCE_0, PRAY_0, TAR_FIGHT_SELF | TAR_CHAR_ROOM, SYMBOL_STRESS_0, "You lose your gift of levitation.", "$n sinks back onto the $g.", "You begin to feel the tug of The World again.", "", START_32, LEARN_11, START_DO_40, LEARN_DO_5, START_DO_NO, LEARN_DO_NO, LEARN_DIFF_SPELLS, 0.04, COMP_GESTURAL | COMP_VERBAL | COMP_MATERIAL | COMP_MATERIAL_END | SPELL_TASKED, 0);

  discArray[SPELL_FEATHERY_DESCENT] = new spellInfo(SPELL_MAGE, DISC_MAGE, DISC_AIR, STAT_INT, "feathery descent", TASK_EASY, LAG_1, POSITION_CRAWLING, MANA_25, LIFEFORCE_0, PRAY_0, TAR_CHAR_ROOM | TAR_FIGHT_SELF, SYMBOL_STRESS_0, "You don't seem to be light as a feather anymore.", "$n doesn't seem to be light as a feather any more.", "You feel slightly heavier.", "", START_20, LEARN_8, START_DO_40, LEARN_DO_5, START_DO_NO, LEARN_DO_NO, LEARN_DIFF_SPELLS, 0.04, COMP_GESTURAL | COMP_GESTURAL_RANDOM | COMP_VERBAL | COMP_VERBAL_RANDOM | COMP_MATERIAL | COMP_MATERIAL_END | SPELL_TASKED, 0);

  discArray[SPELL_STEALTH] = new spellInfo(SPELL_MAGE, DISC_MAGE, DISC_SPIRIT, STAT_INT, "stealth", TASK_EASY, LAG_2, POSITION_CRAWLING, MANA_20, LIFEFORCE_0, PRAY_0, TAR_CHAR_ROOM | TAR_FIGHT_SELF, SYMBOL_STRESS_0, "You seem to be making a bit more noise now.", "$n is making much more noise than before.", "", "", START_47, LEARN_13, START_DO_45, LEARN_DO_5, START_DO_NO, LEARN_DO_NO, LEARN_DIFF_SPELLS, 0.04, COMP_GESTURAL | COMP_GESTURAL_RANDOM | COMP_VERBAL | COMP_VERBAL_RANDOM | COMP_MATERIAL | COMP_MATERIAL_END | SPELL_TASKED, 0);

  discArray[SPELL_GRANITE_FISTS] = new spellInfo(SPELL_MAGE, DISC_MAGE, DISC_EARTH, STAT_INT, "granite fists", TASK_NORMAL, LAG_1, POSITION_SITTING, MANA_30, LIFEFORCE_0, PRAY_0, TAR_CHAR_ROOM | TAR_VIOLENT | TAR_FIGHT_VICT | TAR_SELF_NONO, SYMBOL_STRESS_0, "", "", "", "", START_31, LEARN_9, START_DO_40, LEARN_DO_5, START_DO_NO, LEARN_DO_NO, LEARN_DIFF_SPELLS, 0.05, COMP_GESTURAL | COMP_GESTURAL_RANDOM | COMP_VERBAL | COMP_VERBAL_RANDOM | COMP_MATERIAL | COMP_MATERIAL_END | SPELL_TASKED, 0);

  discArray[SPELL_ICY_GRIP] = new spellInfo(SPELL_MAGE, DISC_MAGE, DISC_WATER, STAT_INT, "icy grip", TASK_NORMAL, LAG_1, POSITION_SITTING, MANA_30, LIFEFORCE_0, PRAY_0, TAR_CHAR_ROOM | TAR_SELF_NONO | TAR_VIOLENT | TAR_FIGHT_VICT , SYMBOL_STRESS_0, "", "", "", "", START_40, LEARN_9, START_DO_40, LEARN_DO_5, START_DO_NO, LEARN_DO_NO, LEARN_DIFF_SPELLS, 0.08, COMP_GESTURAL | COMP_GESTURAL_RANDOM | COMP_VERBAL | COMP_VERBAL_RANDOM | COMP_MATERIAL | COMP_MATERIAL_END | SPELL_TASKED, 0);

  discArray[SPELL_GILLS_OF_FLESH] = new spellInfo(SPELL_MAGE, DISC_MAGE, DISC_WATER, STAT_INT, "gills of flesh", TASK_EASY, LAG_2, POSITION_CRAWLING, MANA_20, LIFEFORCE_0, PRAY_0, TAR_CHAR_ROOM | TAR_FIGHT_SELF, SYMBOL_STRESS_0, "The gills on your neck disappear.", "The gills on $n's neck disappear.", "You begin to wonder how much longer you can breathe water.", "", START_26, LEARN_13, START_DO_45, LEARN_DO_5, START_DO_NO, LEARN_DO_NO, LEARN_DIFF_SPELLS, 0.02, COMP_GESTURAL | COMP_GESTURAL_RANDOM | COMP_VERBAL | COMP_VERBAL_RANDOM | COMP_MATERIAL | COMP_MATERIAL_END | SPELL_TASKED, 0);

  discArray[SPELL_TELEPATHY] = new spellInfo(SPELL_MAGE, DISC_MAGE, DISC_SPIRIT, STAT_EXT, "telepathy", TASK_NORMAL, LAG_0, POSITION_CRAWLING, MANA_20, LIFEFORCE_0, PRAY_0, TAR_NAME, SYMBOL_STRESS_0, "", "", "", "", START_47, LEARN_15, START_DO_50, LEARN_DO_5, START_DO_NO, LEARN_DO_NO, LEARN_DIFF_SPELLS, 0.04, COMP_GESTURAL | COMP_GESTURAL_RANDOM | COMP_VERBAL | COMP_VERBAL_RANDOM | COMP_MATERIAL | COMP_MATERIAL_END | SPELL_TASKED, 0);

  discArray[SPELL_FEAR] = new spellInfo(SPELL_MAGE, DISC_MAGE, DISC_SPIRIT, STAT_CHA, "fear", TASK_NORMAL, LAG_2, POSITION_SITTING, MANA_25, LIFEFORCE_0, PRAY_0, TAR_CHAR_ROOM | TAR_SELF_NONO | TAR_VIOLENT | TAR_FIGHT_VICT , SYMBOL_STRESS_0, "You are much less fearful.", "$n doesn't seem as fearful anymore.", "You are about ready to face your fears.", "", START_42, LEARN_11, START_DO_45, LEARN_DO_5, START_DO_NO, LEARN_DO_NO, LEARN_DIFF_SPELLS, 0.04, COMP_GESTURAL | COMP_GESTURAL_RANDOM | COMP_VERBAL | COMP_VERBAL_RANDOM | COMP_MATERIAL | COMP_MATERIAL_END | SPELL_TASKED, 0) ;

  discArray[SPELL_SLUMBER] = new spellInfo(SPELL_MAGE, DISC_MAGE, DISC_SPIRIT, STAT_EXT, "slumber", TASK_NORMAL, LAG_3, POSITION_SITTING, MANA_35, LIFEFORCE_0, PRAY_0, TAR_CHAR_ROOM | TAR_SELF_NONO | TAR_FIGHT_VICT | TAR_VIOLENT, SYMBOL_STRESS_0, "Your magical exhaustion ceases.", "", "", "", START_39, LEARN_11, START_DO_45, LEARN_DO_5, START_DO_NO, LEARN_DO_NO, LEARN_DIFF_SPELLS, 0.04, COMP_GESTURAL | COMP_GESTURAL_RANDOM | COMP_VERBAL | COMP_VERBAL_RANDOM | COMP_MATERIAL | COMP_MATERIAL_END | SPELL_TASKED, 0);

  discArray[SPELL_CONJURE_EARTH] = new spellInfo(SPELL_MAGE, DISC_MAGE, DISC_EARTH, STAT_EXT, "conjure elemental earth", TASK_NORMAL, LAG_1, POSITION_CRAWLING, MANA_40, LIFEFORCE_0, PRAY_0, TAR_IGNORE, SYMBOL_STRESS_0, "", "", "", "", START_63, LEARN_15, START_DO_50, LEARN_DO_5, START_DO_NO, LEARN_DO_NO, LEARN_DIFF_SPELLS, 0.0, COMP_GESTURAL | COMP_GESTURAL_RANDOM | COMP_VERBAL | COMP_VERBAL_RANDOM | COMP_MATERIAL | COMP_MATERIAL_END | SPELL_TASKED, 0);

  discArray[SPELL_CONJURE_FIRE] = new spellInfo(SPELL_MAGE, DISC_MAGE, DISC_FIRE, STAT_EXT, "conjure elemental fire", TASK_NORMAL, LAG_1, POSITION_CRAWLING, MANA_40, LIFEFORCE_0, PRAY_0, TAR_IGNORE, SYMBOL_STRESS_0, "", "", "", "", START_57, LEARN_13, START_DO_50, LEARN_DO_5, START_DO_NO, LEARN_DO_NO, LEARN_DIFF_SPELLS, 0.0, COMP_GESTURAL | COMP_GESTURAL_RANDOM | COMP_VERBAL | COMP_VERBAL_RANDOM | COMP_MATERIAL | COMP_MATERIAL_END | SPELL_TASKED, 0);

  discArray[SPELL_CONJURE_WATER] = new spellInfo(SPELL_MAGE, DISC_MAGE, DISC_WATER, STAT_EXT, "conjure elemental water", TASK_NORMAL, LAG_1, POSITION_CRAWLING, MANA_40, LIFEFORCE_0, PRAY_0, TAR_IGNORE, SYMBOL_STRESS_0, "", "", "", "", START_71, LEARN_10, START_DO_50, LEARN_DO_5, START_DO_NO, LEARN_DO_NO, LEARN_DIFF_SPELLS, 0.0, COMP_GESTURAL | COMP_GESTURAL_RANDOM | COMP_VERBAL | COMP_VERBAL_RANDOM | COMP_MATERIAL | COMP_MATERIAL_END | SPELL_TASKED, 0);

  discArray[SPELL_CONJURE_AIR] = new spellInfo(SPELL_MAGE, DISC_MAGE, DISC_AIR, STAT_EXT, "conjure elemental air", TASK_NORMAL, LAG_1, POSITION_CRAWLING, MANA_40, LIFEFORCE_0, PRAY_0, TAR_IGNORE, SYMBOL_STRESS_0, "", "", "", "", START_35, LEARN_15, START_DO_50, LEARN_DO_5, START_DO_NO, LEARN_DO_NO, LEARN_DIFF_SPELLS, 0.00, COMP_GESTURAL | COMP_GESTURAL_RANDOM | COMP_VERBAL | COMP_VERBAL_RANDOM | COMP_MATERIAL | COMP_MATERIAL_END | SPELL_TASKED, 0);

  discArray[SPELL_DISPEL_MAGIC] = new spellInfo(SPELL_MAGE, DISC_MAGE, DISC_ALCHEMY, STAT_EXT, "dispel magic", TASK_NORMAL, LAG_1, POSITION_SITTING, MANA_50, LIFEFORCE_0, PRAY_0, TAR_CHAR_ROOM | TAR_FIGHT_VICT | TAR_OBJ_INV | TAR_OBJ_ROOM, SYMBOL_STRESS_0, "", "", "", "", START_61, LEARN_10, START_DO_40, LEARN_DO_5, START_DO_NO, LEARN_DO_NO, LEARN_DIFF_SPELLS, 0.04, COMP_GESTURAL | COMP_GESTURAL_RANDOM | COMP_VERBAL | COMP_VERBAL_RANDOM | COMP_MATERIAL | COMP_MATERIAL_END | SPELL_TASKED, 0);

  discArray[SPELL_ENHANCE_WEAPON] = new spellInfo(SPELL_MAGE, DISC_MAGE, DISC_ALCHEMY, STAT_EXT, "enhance weapon", TASK_EASY, LAG_2, POSITION_CRAWLING, MANA_50, LIFEFORCE_0, PRAY_0, TAR_OBJ_INV | TAR_OBJ_EQUIP, SYMBOL_STRESS_0, "", "", "", "", START_72, LEARN_6, START_DO_60, LEARN_DO_5, START_DO_NO, LEARN_DO_NO, LEARN_DIFF_SPELLS, 0.0, COMP_GESTURAL | COMP_GESTURAL_RANDOM | COMP_VERBAL | COMP_VERBAL_RANDOM | COMP_MATERIAL | COMP_MATERIAL_END | SPELL_TASKED, 0);

  discArray[SPELL_GALVANIZE] = new spellInfo(SPELL_MAGE, DISC_MAGE, DISC_ALCHEMY, STAT_EXT, "galvanize", TASK_EASY, LAG_1, POSITION_CRAWLING, MANA_50, LIFEFORCE_0, PRAY_0, TAR_OBJ_INV | TAR_OBJ_EQUIP, SYMBOL_STRESS_0, "", "", "", "", START_55, LEARN_11, START_DO_40, LEARN_DO_10, START_DO_NO, LEARN_DO_NO, LEARN_DIFF_SPELLS, 0.0, COMP_GESTURAL | COMP_GESTURAL_RANDOM | COMP_VERBAL | COMP_VERBAL_RANDOM | COMP_MATERIAL | COMP_MATERIAL_END | SPELL_TASKED, TOG_HAS_GALVANIZE);

  discArray[SPELL_DETECT_INVISIBLE] = new spellInfo(SPELL_MAGE, DISC_MAGE, DISC_SPIRIT, STAT_INT, "detect invisibility", TASK_EASY, LAG_1, POSITION_CRAWLING, MANA_15, LIFEFORCE_0, PRAY_0, TAR_CHAR_ROOM | TAR_FIGHT_SELF, SYMBOL_STRESS_0, "The yellow aura fades from your eyes.", "The yellow aura fades from $n's eyes.", "The yellow aura in your eyes flickers slightly.", "The yellow aura in $n's eyes flickers slightly.", START_12, LEARN_7, START_DO_40, LEARN_DO_5, START_DO_NO, LEARN_DO_NO, LEARN_DIFF_SPELLS, 0.04, COMP_GESTURAL | COMP_GESTURAL_RANDOM | COMP_VERBAL | COMP_VERBAL_RANDOM | COMP_MATERIAL | COMP_MATERIAL_END | SPELL_TASKED, 0);

  discArray[SPELL_DISPEL_INVISIBLE] = new spellInfo(SPELL_MAGE, DISC_MAGE, DISC_SPIRIT, STAT_EXT, "dispel invisible", TASK_EASY, LAG_1, POSITION_CRAWLING, MANA_30, LIFEFORCE_0, PRAY_0, TAR_CHAR_ROOM | TAR_OBJ_INV | TAR_OBJ_ROOM, SYMBOL_STRESS_0, "", "", "", "", START_50, LEARN_12, START_DO_40, LEARN_DO_5, START_DO_NO, LEARN_DO_NO, LEARN_DIFF_SPELLS, 0.04, COMP_GESTURAL | COMP_GESTURAL_RANDOM | COMP_VERBAL | COMP_VERBAL_RANDOM | COMP_MATERIAL | COMP_MATERIAL_END| SPELL_TASKED, 0);

  discArray[SPELL_TORNADO] = new spellInfo(SPELL_MAGE, DISC_MAGE, DISC_AIR, STAT_INT, "tornado", TASK_NORMAL, LAG_1, POSITION_SITTING, MANA_23, LIFEFORCE_0, PRAY_0, TAR_FIGHT_VICT | TAR_AREA | TAR_VIOLENT | TAR_IGNORE, SYMBOL_STRESS_0, "", "", "", "", START_43, LEARN_10, START_DO_40, LEARN_DO_5, START_DO_NO, LEARN_DO_NO, LEARN_DIFF_SPELLS, 0.05, COMP_GESTURAL | COMP_GESTURAL_RANDOM | COMP_VERBAL | COMP_VERBAL_RANDOM | COMP_MATERIAL | COMP_MATERIAL_ALMOST_END | SPELL_TASKED, TOG_HAS_TORNADO);

  discArray[SPELL_SAND_BLAST] = new spellInfo(SPELL_MAGE, DISC_MAGE, DISC_EARTH, STAT_INT, "sand blast", TASK_DIFFICULT, LAG_2, POSITION_SITTING, MANA_35, LIFEFORCE_0, PRAY_0, TAR_AREA | TAR_FIGHT_VICT | TAR_VIOLENT | TAR_IGNORE, SYMBOL_STRESS_0, "", "", "", "", START_58, LEARN_8, START_DO_40, LEARN_DO_5, START_DO_NO, LEARN_DO_NO, LEARN_DIFF_SPELLS, 0.04, COMP_GESTURAL | COMP_GESTURAL_RANDOM | COMP_VERBAL | COMP_VERBAL_RANDOM | COMP_MATERIAL | COMP_MATERIAL_END | SPELL_TASKED, 0);

  discArray[SPELL_ICE_STORM] = new spellInfo(SPELL_MAGE, DISC_MAGE, DISC_WATER, STAT_INT, "ice storm", TASK_NORMAL, LAG_1, POSITION_SITTING, MANA_35, LIFEFORCE_0, PRAY_0, TAR_AREA | TAR_IGNORE | TAR_VIOLENT, SYMBOL_STRESS_0, "", "", "", "", START_58, LEARN_12, START_DO_40, LEARN_DO_5, START_DO_NO, LEARN_DO_NO, LEARN_DIFF_SPELLS,0.02, COMP_GESTURAL | COMP_GESTURAL_RANDOM | COMP_VERBAL | COMP_VERBAL_RANDOM | COMP_MATERIAL | COMP_MATERIAL_END | SPELL_TASKED, TOG_HAS_ICE_STORM);

  discArray[SPELL_FLAMING_SWORD] = new spellInfo(SPELL_MAGE, DISC_MAGE, DISC_FIRE, STAT_INT, "flaming sword", TASK_NORMAL, LAG_1, POSITION_SITTING, MANA_40, LIFEFORCE_0, PRAY_0, TAR_CHAR_ROOM | TAR_SELF_NONO | TAR_VIOLENT | TAR_FIGHT_VICT , SYMBOL_STRESS_0, "", "", "", "", START_55, LEARN_20, START_DO_40, LEARN_DO_5, START_DO_NO, LEARN_DO_NO, LEARN_DIFF_SPELLS, 0.05, COMP_GESTURAL | COMP_GESTURAL_RANDOM | COMP_VERBAL | COMP_VERBAL_RANDOM | COMP_MATERIAL | COMP_MATERIAL_END | SPELL_TASKED, 0);
 
  discArray[SPELL_ACID_BLAST] = new spellInfo(SPELL_MAGE, DISC_MAGE, DISC_SORCERY, STAT_INT, "acid blast", TASK_NORMAL, LAG_2, POSITION_SITTING, MANA_53, LIFEFORCE_0, PRAY_0, TAR_AREA | TAR_IGNORE | TAR_FIGHT_VICT | TAR_VIOLENT, SYMBOL_STRESS_0, "", "", "", "", START_75, LEARN_9, START_DO_40, LEARN_DO_5, START_DO_NO, LEARN_DO_NO, LEARN_DIFF_SPELLS, 0.04, COMP_GESTURAL | COMP_GESTURAL_RANDOM | COMP_VERBAL | COMP_VERBAL_RANDOM | COMP_MATERIAL | COMP_MATERIAL_END | SPELL_TASKED, 0);

  discArray[SPELL_FIREBALL] = new spellInfo(SPELL_MAGE, DISC_MAGE, DISC_FIRE, STAT_INT, "fireball", TASK_NORMAL, LAG_2, POSITION_SITTING, MANA_33, LIFEFORCE_0, PRAY_0, TAR_AREA | TAR_IGNORE | TAR_VIOLENT, SYMBOL_STRESS_0, "", "", "", "", START_40, LEARN_15, START_DO_45, LEARN_DO_5, START_DO_NO, LEARN_DO_NO, LEARN_DIFF_SPELLS, 0.04, COMP_GESTURAL | COMP_GESTURAL_RANDOM | COMP_VERBAL | COMP_VERBAL_RANDOM | COMP_MATERIAL | COMP_MATERIAL_END | SPELL_TASKED, TOG_HAS_FIREBALL);

  discArray[SPELL_FARLOOK] = new spellInfo(SPELL_MAGE, DISC_MAGE, DISC_ALCHEMY, STAT_INT, "farlook", TASK_EASY, LAG_1, POSITION_CRAWLING, MANA_20, LIFEFORCE_0, PRAY_0, TAR_CHAR_VIS_WORLD, SYMBOL_STRESS_0, "", "", "", "", START_75, LEARN_7, START_DO_45, LEARN_DO_5, START_DO_NO, LEARN_DO_NO, LEARN_DIFF_SPELLS, 0.0, COMP_GESTURAL | COMP_GESTURAL_RANDOM | COMP_VERBAL | COMP_VERBAL_RANDOM | COMP_MATERIAL | COMP_MATERIAL_END | SPELL_TASKED, 0);

  discArray[SPELL_FALCON_WINGS] = new spellInfo(SPELL_MAGE, DISC_MAGE, DISC_AIR, STAT_INT, "falcon wings", TASK_EASY, LAG_2, POSITION_CRAWLING, MANA_20, LIFEFORCE_0, PRAY_0, TAR_CHAR_ROOM | TAR_FIGHT_SELF, SYMBOL_STRESS_0, "The last of your feathers fall from your arms!", "The feather's on $n's arms have completely fallen off.","A few of the feathers on your arm begin to fall off.","A few feathers fall from $n's arms.", START_53, LEARN_7, START_DO_40, LEARN_DO_5, START_DO_NO, LEARN_DO_NO, LEARN_DIFF_SPELLS, 0.04, COMP_GESTURAL | COMP_GESTURAL_RANDOM | COMP_VERBAL | COMP_VERBAL_RANDOM | COMP_MATERIAL | COMP_MATERIAL_END | SPELL_TASKED, 0);

  discArray[SPELL_INVISIBILITY] = new spellInfo(SPELL_MAGE, DISC_MAGE, DISC_SORCERY, STAT_INT, "invisibility", TASK_EASY, LAG_2, POSITION_CRAWLING, MANA_30, LIFEFORCE_0, PRAY_0, TAR_CHAR_ROOM | TAR_OBJ_INV | TAR_OBJ_ROOM | TAR_OBJ_EQUIP | TAR_FIGHT_SELF, SYMBOL_STRESS_0, "You fade back into view.", "Suddenly, $n materializes from out of nowhere!", "", "", START_51, LEARN_10, START_DO_45, LEARN_DO_5, START_DO_NO, LEARN_DO_NO, LEARN_DIFF_SPELLS, 0.04, COMP_GESTURAL | COMP_GESTURAL_RANDOM | COMP_VERBAL | COMP_VERBAL_RANDOM | COMP_MATERIAL | COMP_MATERIAL_END | SPELL_TASKED, 0);

#if 1
  discArray[SPELL_ENSORCER] = new spellInfo(SPELL_MAGE, DISC_MAGE, DISC_SORCERY, STAT_CHA, "ensorcer", TASK_NORMAL, LAG_1, POSITION_CRAWLING, MANA_40, LIFEFORCE_0, PRAY_0, TAR_CHAR_ROOM | TAR_SELF_NONO | TAR_FIGHT_VICT | TAR_VIOLENT, SYMBOL_STRESS_0, "You shake your head really hard and free yourself from the charm.", "$n shakes $s head really hard and frees $mself from the charm.", "You momentarily become cognizant of The World around you.", "You see a brief flicker of intelligence in $n's eyes.", START_43, LEARN_11, START_DO_45, LEARN_DO_5, START_DO_NO, LEARN_DO_NO, LEARN_DIFF_SPELLS, 0.04, COMP_GESTURAL | COMP_GESTURAL_RANDOM | COMP_VERBAL | COMP_VERBAL_RANDOM | COMP_MATERIAL | COMP_MATERIAL_END | SPELL_TASKED, 0);
#endif

  discArray[SPELL_EYES_OF_FERTUMAN] = new spellInfo(SPELL_MAGE, DISC_MAGE, DISC_ALCHEMY, STAT_EXT, "eyes of Fertuman", TASK_EASY, LAG_1, POSITION_CRAWLING, MANA_30, LIFEFORCE_0, PRAY_0, TAR_CHAR_ROOM | TAR_FIGHT_SELF, SYMBOL_STRESS_0, "", "", "", "", START_65, LEARN_13, START_DO_50, LEARN_DO_5, START_DO_NO, LEARN_DO_NO, LEARN_DIFF_SPELLS, 0.0, COMP_GESTURAL | COMP_GESTURAL_RANDOM | COMP_VERBAL | COMP_VERBAL_RANDOM | COMP_MATERIAL | COMP_MATERIAL_END | SPELL_TASKED, 0);

  discArray[SPELL_COPY] = new spellInfo(SPELL_MAGE, DISC_MAGE, DISC_ALCHEMY, STAT_EXT, "copy", TASK_NORMAL, LAG_3, POSITION_CRAWLING, MANA_40, LIFEFORCE_0, PRAY_0, TAR_OBJ_INV | TAR_OBJ_EQUIP, SYMBOL_STRESS_0, "", "", "", "", START_70, LEARN_6, START_DO_35, LEARN_DO_5, START_DO_NO, LEARN_DO_NO, LEARN_DIFF_SPELLS, 0.0, COMP_GESTURAL | COMP_GESTURAL_RANDOM | COMP_VERBAL | COMP_VERBAL_RANDOM | COMP_MATERIAL | COMP_MATERIAL_END | SPELL_TASKED, 0);

  discArray[SPELL_HASTE] = new spellInfo(SPELL_MAGE, DISC_MAGE, DISC_SPIRIT, STAT_INT, "haste", TASK_NORMAL, LAG_2, POSITION_CRAWLING, MANA_50, LIFEFORCE_0, PRAY_0, TAR_CHAR_ROOM | TAR_FIGHT_SELF, SYMBOL_STRESS_0, "You lose the bounce in your step.", "$n loses the bounce in $s step.", "", "", START_68, LEARN_17, START_DO_50, LEARN_DO_5, START_DO_NO, LEARN_DO_NO, LEARN_DIFF_SPELLS, 0.04, COMP_GESTURAL | COMP_GESTURAL_RANDOM | COMP_VERBAL | COMP_VERBAL_RANDOM | COMP_MATERIAL | COMP_MATERIAL_END | SPELL_TASKED, 0);

  discArray[SKILL_REPAIR_MAGE] = new spellInfo(SKILL_MAGE, DISC_MAGE, DISC_BLACKSMITHING, STAT_EXT, "mage repair", TASK_NORMAL, LAG_0, POSITION_SITTING, MANA_0, LIFEFORCE_0, PRAY_0, 0, SYMBOL_STRESS_0, "", "", "", "", START_50, LEARN_3, START_DO_1, LEARN_DO_1, START_DO_NO, LEARN_DO_NO, LEARN_DIFF_SKILLS, 0.0, 0, 0);


// disc_air

  discArray[SPELL_IMMOBILIZE] = new spellInfo(SPELL_MAGE, DISC_AIR, DISC_AIR, STAT_EXT, "immobilize", TASK_NORMAL, LAG_2, POSITION_SITTING,MANA_20, LIFEFORCE_0, PRAY_0, TAR_VIOLENT | TAR_FIGHT_VICT  | TAR_CHAR_ROOM | TAR_SELF_NONO, SYMBOL_STRESS_0, "", "", "", "", START_37, LEARN_9, START_DO_45, LEARN_DO_5, START_DO_NO, LEARN_DO_NO, LEARN_DIFF_SPELLS, 0.06, COMP_GESTURAL | COMP_GESTURAL_RANDOM | COMP_VERBAL | COMP_VERBAL_RANDOM | COMP_MATERIAL | COMP_MATERIAL_END | SPELL_TASKED, 0);

  discArray[SPELL_SUFFOCATE] = new spellInfo(SPELL_MAGE, DISC_AIR, DISC_AIR, STAT_INT, "suffocate", TASK_NORMAL, LAG_2, POSITION_SITTING, MANA_27, LIFEFORCE_0, PRAY_0, TAR_VIOLENT | TAR_FIGHT_VICT  | TAR_CHAR_ROOM | TAR_SELF_NONO, SYMBOL_STRESS_0, "", "", "", "", START_60, LEARN_4, START_DO_45, LEARN_DO_5, START_DO_NO, LEARN_DO_NO, LEARN_DIFF_SPELLS, 0.08, COMP_GESTURAL | COMP_GESTURAL_RANDOM | COMP_VERBAL | COMP_VERBAL_RANDOM | COMP_MATERIAL | COMP_MATERIAL_END | SPELL_TASKED, 0);

  discArray[SPELL_FLY] = new spellInfo(SPELL_MAGE, DISC_AIR, DISC_AIR, STAT_INT, "flight", TASK_EASY, LAG_1, POSITION_SITTING, MANA_30, LIFEFORCE_0, PRAY_0, TAR_CHAR_ROOM | TAR_FIGHT_SELF, SYMBOL_STRESS_0, "Your ability to fly leaves you.", "$n loses $s ability to fly.", "You feel slightly heavier.", "$n doesn't seem as flight worthy as before.", START_32, LEARN_11, START_DO_50, LEARN_DO_5, START_DO_NO, LEARN_DO_NO, LEARN_DIFF_SPELLS, 0.04, COMP_GESTURAL | COMP_GESTURAL_RANDOM | COMP_VERBAL | COMP_VERBAL_RANDOM | COMP_MATERIAL | COMP_MATERIAL_END | SPELL_TASKED, 0);
 
  discArray[SPELL_ANTIGRAVITY] = new spellInfo(SPELL_MAGE, DISC_AIR, DISC_AIR, STAT_INT, "antigravity", TASK_EASY, LAG_1, POSITION_CRAWLING, MANA_50, LIFEFORCE_0, PRAY_0, TAR_AREA | TAR_IGNORE, SYMBOL_STRESS_0, "The forces of nature have resumed their regular strength", "$n sinks back onto the $g.", "You begin to feel the tug of The World again.", "", START_67, LEARN_10, START_DO_50, LEARN_DO_5, START_DO_NO, LEARN_DO_NO, LEARN_DIFF_SPELLS, 0.04, COMP_GESTURAL | COMP_GESTURAL_RANDOM | COMP_VERBAL | COMP_VERBAL_RANDOM | COMP_MATERIAL | COMP_MATERIAL_END | SPELL_TASKED, 0);

// disc_alchemy

  discArray[SPELL_DIVINATION] = new spellInfo(SPELL_MAGE, DISC_ALCHEMY, DISC_ALCHEMY, STAT_EXT, "divination", TASK_EASY, LAG_1, POSITION_CRAWLING, MANA_35, LIFEFORCE_0, PRAY_0, TAR_OBJ_INV | TAR_CHAR_ROOM, SYMBOL_STRESS_0, "", "", "", "", START_68, LEARN_10, START_DO_55, LEARN_DO_5, START_DO_NO, LEARN_DO_NO, LEARN_DIFF_SPELLS, 0.0, COMP_GESTURAL | COMP_GESTURAL_RANDOM | COMP_VERBAL | COMP_VERBAL_RANDOM | COMP_MATERIAL | COMP_MATERIAL_END | SPELL_TASKED, 0);

  discArray[SPELL_SHATTER] = new spellInfo(SPELL_MAGE, DISC_ALCHEMY, DISC_ALCHEMY, STAT_EXT, "shatter", TASK_NORMAL, LAG_1, POSITION_SITTING, MANA_40, LIFEFORCE_0, PRAY_0, TAR_CHAR_ROOM, SYMBOL_STRESS_0, "", "", "", "", START_8, LEARN_34, START_DO_50, LEARN_DO_5, START_DO_NO, LEARN_DO_NO, LEARN_DIFF_SPELLS, 0.04, COMP_GESTURAL | COMP_GESTURAL_RANDOM | COMP_VERBAL | COMP_VERBAL_RANDOM | COMP_MATERIAL | COMP_MATERIAL_END |SPELL_TASKED, 0);

  discArray[SKILL_SCRIBE] = new spellInfo(SPELL_MAGE, DISC_ALCHEMY, DISC_ALCHEMY, STAT_EXT, "scribe", TASK_NORMAL, LAG_0, POSITION_STANDING, MANA_0, LIFEFORCE_0, PRAY_0, 0, SYMBOL_STRESS_0, "", "", "", "", START_1, LEARN_1, START_DO_1, LEARN_DO_5, START_DO_NO, LEARN_DO_NO, LEARN_DIFF_SPELLS, 0.0, 0, 0);

  discArray[SPELL_SPONTANEOUS_GENERATION] = new spellInfo(SPELL_MAGE, DISC_ALCHEMY, DISC_ALCHEMY, STAT_EXT, "spontaneous generation", TASK_EASY, LAG_1, POSITION_CRAWLING, MANA_35, LIFEFORCE_0, PRAY_0, TAR_NAME, SYMBOL_STRESS_0, "", "", "", "", START_25, LEARN_5, START_DO_50, LEARN_DO_5, START_DO_NO, LEARN_DO_NO, LEARN_DIFF_SPELLS, 0.0, COMP_GESTURAL | COMP_VERBAL | SPELL_TASKED, 0);

  discArray[SKILL_STAVECHARGE] = new spellInfo(SKILL_MAGE, DISC_ALCHEMY, DISC_ALCHEMY, STAT_EXT, "charge stave", TASK_EASY, LAG_0, POSITION_SITTING, MANA_0, LIFEFORCE_0, PRAY_0, 0, SYMBOL_STRESS_0, "", "", "", "", START_1, LEARN_1, START_DO_20, LEARN_DO_2, START_DO_NO, LEARN_DO_NO, LEARN_DIFF_SPELLS, 0.0, 0, 0);

// disc_earth

  discArray[SPELL_METEOR_SWARM] = new spellInfo(SPELL_MAGE, DISC_EARTH, DISC_EARTH, STAT_INT, "meteor swarm", TASK_NORMAL, LAG_3, POSITION_SITTING, MANA_40, LIFEFORCE_0, PRAY_0, TAR_CHAR_ROOM | TAR_SELF_NONO | TAR_VIOLENT | TAR_FIGHT_VICT , SYMBOL_STRESS_0, "", "", "", "", START_50, LEARN_5, START_DO_50, LEARN_DO_5, START_DO_NO, LEARN_DO_NO, LEARN_DIFF_SPELLS, 0.08, COMP_GESTURAL | COMP_GESTURAL_RANDOM | COMP_VERBAL | COMP_VERBAL_RANDOM | COMP_MATERIAL | COMP_MATERIAL_END | SPELL_TASKED, 0);

  discArray[SPELL_LAVA_STREAM] = new spellInfo(SPELL_MAGE, DISC_EARTH, DISC_EARTH, STAT_INT, "lava stream", TASK_DIFFICULT, LAG_3, POSITION_SITTING, MANA_50, LIFEFORCE_0, PRAY_0, TAR_AREA | TAR_FIGHT_VICT | TAR_VIOLENT | TAR_IGNORE, SYMBOL_STRESS_0, "", "", "", "", START_75, LEARN_5, START_DO_50, LEARN_DO_5, START_DO_NO, LEARN_DO_NO, LEARN_DIFF_SPELLS, 0.04, COMP_GESTURAL | COMP_GESTURAL_RANDOM | COMP_VERBAL | COMP_VERBAL_RANDOM | COMP_MATERIAL | COMP_MATERIAL_END | SPELL_TASKED, 0);

  discArray[SPELL_STONE_SKIN] = new spellInfo(SPELL_MAGE, DISC_EARTH, DISC_EARTH, STAT_INT, "stone skin", TASK_EASY, LAG_1, POSITION_CRAWLING, MANA_40, LIFEFORCE_0, PRAY_0, TAR_CHAR_ROOM | TAR_SELF_ONLY, SYMBOL_STRESS_0, "Your skin returns to normal.", "$n's skin returns to normal.", "Your skin doesn't feel as hard as rock anymore.", "$n's skin doesn't seem to be as hard as rock anymore.", START_1, LEARN_14, START_DO_50, LEARN_DO_5, START_DO_NO, LEARN_DO_NO, LEARN_DIFF_SPELLS, 0.04, COMP_GESTURAL | COMP_GESTURAL_RANDOM | COMP_VERBAL | COMP_VERBAL_RANDOM | COMP_MATERIAL | COMP_MATERIAL_END | SPELL_TASKED, TOG_HAS_STONESKIN);

  discArray[SPELL_TRAIL_SEEK] = new spellInfo(SPELL_MAGE, DISC_EARTH, DISC_EARTH, STAT_EXT, "trail seek", TASK_NORMAL, LAG_1, POSITION_CRAWLING, MANA_20, LIFEFORCE_0, PRAY_0, TAR_CHAR_ROOM | TAR_FIGHT_SELF, SYMBOL_STRESS_0, "The blue hue fades from your eyes.", "$n's eyes lose their blue hue.", "You seem to be less attuned to your senses.", "", START_1, LEARN_10, START_DO_50, LEARN_DO_5, START_DO_NO, LEARN_DO_NO, LEARN_DIFF_SPELLS, 0.01, COMP_GESTURAL | COMP_GESTURAL_RANDOM | COMP_VERBAL | COMP_VERBAL_RANDOM | COMP_MATERIAL | COMP_MATERIAL_END | SPELL_TASKED, 0);

// disc_fire

  discArray[SPELL_INFERNO] = new spellInfo(SPELL_MAGE, DISC_FIRE, DISC_FIRE, STAT_INT, "inferno", TASK_NORMAL, LAG_1, POSITION_SITTING, MANA_54, LIFEFORCE_0, PRAY_0, TAR_CHAR_ROOM | TAR_SELF_NONO | TAR_VIOLENT | TAR_FIGHT_VICT , SYMBOL_STRESS_0, "", "", "", "", START_50, LEARN_5, START_DO_50, LEARN_DO_3, START_DO_NO, LEARN_DO_NO, LEARN_DIFF_SPELLS, 0.06, COMP_GESTURAL | COMP_GESTURAL_RANDOM | COMP_VERBAL | COMP_VERBAL_RANDOM | COMP_MATERIAL | COMP_MATERIAL_INIT | SPELL_TASKED, 0);

  discArray[SPELL_HELLFIRE] = new spellInfo(SPELL_MAGE, DISC_FIRE, DISC_FIRE, STAT_INT, "hellfire", TASK_DIFFICULT, LAG_2, POSITION_SITTING, MANA_47, LIFEFORCE_0, PRAY_0, TAR_AREA | TAR_IGNORE | TAR_VIOLENT, SYMBOL_STRESS_0, "", "", "", "", START_75, LEARN_5, START_DO_50, LEARN_DO_5, START_DO_NO, LEARN_DO_NO, LEARN_DIFF_SPELLS, 0.04, COMP_GESTURAL | COMP_GESTURAL_RANDOM | COMP_VERBAL | COMP_VERBAL_RANDOM | COMP_MATERIAL | COMP_MATERIAL_END | SPELL_TASKED, 0);

  discArray[SPELL_FLAMING_FLESH] = new spellInfo(SPELL_MAGE, DISC_FIRE, DISC_FIRE, STAT_INT, "flaming flesh", TASK_EASY, LAG_1, POSITION_CRAWLING, MANA_20, LIFEFORCE_0, PRAY_0, TAR_CHAR_ROOM | TAR_SELF_ONLY, SYMBOL_STRESS_0, "The ring of fire surrounding your body fades away.", "The ring of fire surrounding $n's body fades away.", "Your ring of fire doesn't seem quite as hot as before.", "It seems cooler in here.", START_1, LEARN_14, START_DO_50, LEARN_DO_5, START_DO_NO, LEARN_DO_NO, LEARN_DIFF_SPELLS, 0.0, COMP_GESTURAL | COMP_GESTURAL_RANDOM | COMP_VERBAL | COMP_VERBAL_RANDOM | COMP_MATERIAL | COMP_MATERIAL_END | SPELL_TASKED, 0);

// disc_sorcery

  discArray[SPELL_BLAST_OF_FURY] = new spellInfo(SPELL_MAGE, DISC_SORCERY, DISC_SORCERY, STAT_INT, "blast of fury", TASK_NORMAL, LAG_2, POSITION_SITTING, MANA_60, LIFEFORCE_0, PRAY_0, TAR_CHAR_ROOM | TAR_SELF_NONO | TAR_VIOLENT | TAR_FIGHT_VICT , SYMBOL_STRESS_0, "", "","", "", START_20, LEARN_50, START_DO_45, LEARN_DO_5, START_DO_NO, LEARN_DO_NO, LEARN_DIFF_SPELLS, 0.04, COMP_GESTURAL | COMP_GESTURAL_RANDOM | COMP_VERBAL | COMP_VERBAL_RANDOM | COMP_MATERIAL | COMP_MATERIAL_END | SPELL_TASKED, 0);
  
  discArray[SPELL_ENERGY_DRAIN] = new spellInfo(SPELL_MAGE, DISC_SORCERY, DISC_SORCERY, STAT_INT, "energy drain", TASK_DIFFICULT, LAG_3, POSITION_SITTING, MANA_40, LIFEFORCE_0, PRAY_0, TAR_CHAR_ROOM | TAR_SELF_NONO | TAR_VIOLENT | TAR_FIGHT_VICT , SYMBOL_STRESS_0, "", "", "", "", START_40, LEARN_5, START_DO_50, LEARN_DO_2, START_DO_NO, LEARN_DO_NO, LEARN_DIFF_SPELLS, 0.04, COMP_GESTURAL | COMP_VERBAL | COMP_MATERIAL  | COMP_MATERIAL_END | SPELL_TASKED, 0);

  discArray[SPELL_ATOMIZE] = new spellInfo(SPELL_MAGE, DISC_SORCERY, DISC_SORCERY, STAT_INT, "atomize", TASK_DIFFICULT, LAG_3, POSITION_SITTING, MANA_75, LIFEFORCE_0, PRAY_0, TAR_CHAR_ROOM | TAR_SELF_NONO | TAR_VIOLENT | TAR_FIGHT_VICT , SYMBOL_STRESS_0, "", "", "", "", START_80, LEARN_5, START_DO_35, LEARN_DO_5, START_DO_NO, LEARN_DO_NO, LEARN_DIFF_SPELLS, 0.04, COMP_GESTURAL | COMP_VERBAL | COMP_MATERIAL | SPELL_TASKED | COMP_MATERIAL_INIT, 0);

  discArray[SPELL_ANIMATE] = new spellInfo(SPELL_MAGE, DISC_SORCERY, DISC_SORCERY, STAT_EXT, "animate", TASK_EASY, LAG_2, POSITION_CRAWLING, MANA_40, LIFEFORCE_0, PRAY_0, TAR_IGNORE, SYMBOL_STRESS_0, "", "", "", "", START_1, LEARN_10, START_DO_50, LEARN_DO_5, START_DO_NO, LEARN_DO_NO, LEARN_DIFF_SPELLS, 0.0, COMP_GESTURAL | COMP_GESTURAL_RANDOM | COMP_VERBAL | COMP_VERBAL_RANDOM | COMP_MATERIAL | COMP_MATERIAL_END | SPELL_TASKED, 0); 

  discArray[SPELL_BIND] = new spellInfo(SPELL_MAGE, DISC_SORCERY, DISC_SORCERY, STAT_INT, "bind", TASK_NORMAL, LAG_1, POSITION_SITTING, MANA_50, LIFEFORCE_0, PRAY_0, TAR_CHAR_ROOM | TAR_SELF_NONO | TAR_VIOLENT | TAR_FIGHT_VICT , SYMBOL_STRESS_0, "The webs seem to fall away from your body.", "The webs fall away from $n's body.", "The webs seem less sticky.", "The webs surrounding $n seem less sticky.", START_1, LEARN_50, START_DO_50, LEARN_DO_5, START_DO_NO, LEARN_DO_NO, LEARN_DIFF_SPELLS, 0.02, COMP_GESTURAL | COMP_VERBAL | COMP_MATERIAL | COMP_MATERIAL_END | SPELL_TASKED, 0);


// disc_spirit

  discArray[SPELL_FUMBLE] = new spellInfo(SPELL_MAGE, DISC_SPIRIT, DISC_SPIRIT, STAT_INT, "fumble", TASK_DIFFICULT, LAG_1, POSITION_SITTING, MANA_35, LIFEFORCE_0, PRAY_0, TAR_CHAR_ROOM | TAR_SELF_NONO | TAR_VIOLENT | TAR_FIGHT_VICT, SYMBOL_STRESS_0, "You feel more in control and less clumsy.", "$n stops fumbling about.", "", "", START_60, LEARN_10, START_DO_50, LEARN_DO_5, START_DO_NO, LEARN_DO_NO, LEARN_DIFF_SPELLS, 0.03, COMP_GESTURAL | COMP_GESTURAL_RANDOM | COMP_VERBAL | COMP_VERBAL_RANDOM | COMP_MATERIAL | COMP_MATERIAL_END | SPELL_TASKED, 0);

  discArray[SPELL_TRUE_SIGHT] = new spellInfo(SPELL_MAGE, DISC_SPIRIT, DISC_SPIRIT, STAT_INT, "true sight", TASK_EASY, LAG_1, POSITION_CRAWLING, MANA_25, LIFEFORCE_0, PRAY_0, TAR_CHAR_ROOM | TAR_FIGHT_SELF, SYMBOL_STRESS_0, "The silver aura in your eyes fades.", "the silver aura in $n's eyes fades.", "The silver aura in your eyes flickers slightly.", "The silver aura in $n's eyes fades slightly.", START_1, LEARN_10, START_DO_40, LEARN_DO_5, START_DO_NO, LEARN_DO_NO, LEARN_DIFF_SPELLS, 0.04, COMP_GESTURAL | COMP_GESTURAL_RANDOM | COMP_VERBAL | COMP_VERBAL_RANDOM | COMP_MATERIAL | COMP_MATERIAL_END | SPELL_TASKED, 0);

  discArray[SPELL_CLOUD_OF_CONCEALMENT] = new spellInfo(SPELL_MAGE, DISC_SPIRIT, DISC_SPIRIT, STAT_INT, "cloud of concealment", TASK_EASY, LAG_2, POSITION_CRAWLING, MANA_50, LIFEFORCE_0, PRAY_0, TAR_AREA | TAR_IGNORE, SYMBOL_STRESS_0, "The mystic vapors surrounding you disappear.", "$n materializes out of thin air.", "The vapor around you begins to dissipate.", "", START_20, LEARN_10, START_DO_40, LEARN_DO_5, START_DO_NO, LEARN_DO_NO, LEARN_DIFF_SPELLS, 0.04, COMP_GESTURAL | COMP_GESTURAL_RANDOM | COMP_VERBAL | COMP_VERBAL_RANDOM | COMP_MATERIAL | COMP_MATERIAL_END | SPELL_TASKED, 0);


  discArray[SPELL_SILENCE] = new spellInfo(SPELL_MAGE, DISC_SPIRIT, DISC_SPIRIT, STAT_INT, "silence", TASK_NORMAL, LAG_2, POSITION_SITTING, MANA_30, LIFEFORCE_0, PRAY_0, TAR_CHAR_ROOM | TAR_VIOLENT, SYMBOL_STRESS_0, "Your muzzle disappears.", "", "", "", START_80, LEARN_10, START_DO_35, LEARN_DO_5, START_DO_NO, LEARN_DO_NO, LEARN_DIFF_SPELLS, 0.04, COMP_GESTURAL | COMP_GESTURAL_RANDOM | COMP_VERBAL | COMP_VERBAL_RANDOM | COMP_MATERIAL | COMP_MATERIAL_END | SPELL_TASKED, 0);

  discArray[SPELL_KNOT] = new spellInfo(SPELL_MAGE, DISC_SPIRIT, DISC_SPIRIT, STAT_EXT, "knot", TASK_EASY, LAG_1, POSITION_SITTING, MANA_200, LIFEFORCE_0, PRAY_0, TAR_SELF_ONLY | TAR_FIGHT_SELF | TAR_CHAR_ROOM, SYMBOL_STRESS_0, "", "", "", "", START_85, LEARN_15, START_DO_40, LEARN_DO_5, START_DO_NO, LEARN_DO_NO, LEARN_DIFF_SPELLS, 0.0, COMP_GESTURAL | COMP_GESTURAL_RANDOM | COMP_VERBAL | COMP_VERBAL_RANDOM | SPELL_TASKED, 0);



// disc_water

  discArray[SPELL_WATERY_GRAVE] = new spellInfo(SPELL_MAGE, DISC_WATER, DISC_WATER, STAT_INT, "watery grave", TASK_NORMAL, LAG_1, POSITION_SITTING, MANA_40, LIFEFORCE_0, PRAY_0, TAR_CHAR_ROOM | TAR_SELF_NONO | TAR_VIOLENT | TAR_FIGHT_VICT , SYMBOL_STRESS_0, "", "", "", "", START_25, LEARN_3, START_DO_40, LEARN_DO_5, START_DO_NO, LEARN_DO_NO, LEARN_DIFF_SPELLS, 0.05, COMP_GESTURAL | COMP_GESTURAL_RANDOM | COMP_VERBAL | COMP_VERBAL_RANDOM | COMP_MATERIAL | COMP_MATERIAL_END | SPELL_TASKED, 0);

  discArray[SPELL_TSUNAMI] = new spellInfo(SPELL_MAGE, DISC_WATER, DISC_WATER, STAT_INT, "tsunami", TASK_DIFFICULT, LAG_2, POSITION_SITTING, MANA_50, LIFEFORCE_0, PRAY_0, TAR_AREA | TAR_IGNORE | TAR_VIOLENT, SYMBOL_STRESS_0, "", "", "", "", START_50, LEARN_2, START_DO_40, LEARN_DO_5, START_DO_NO, LEARN_DO_NO, LEARN_DIFF_SPELLS, 0.04, COMP_GESTURAL | COMP_GESTURAL_RANDOM | COMP_VERBAL | COMP_VERBAL_RANDOM | COMP_MATERIAL | COMP_MATERIAL_END | SPELL_TASKED, 0);

  discArray[SPELL_BREATH_OF_SARAHAGE] = new spellInfo(SPELL_MAGE, DISC_WATER, DISC_WATER, STAT_INT, "breath of Sarahage", TASK_NORMAL, LAG_2, POSITION_CRAWLING, MANA_35, LIFEFORCE_0, PRAY_0, TAR_AREA | TAR_IGNORE, SYMBOL_STRESS_0, "Your ability to breathe underwater leaves you.", "$n gasps briefly.", "You begin to wonder how much longer you can breathe underwater.", "", START_1, LEARN_3, START_DO_45, LEARN_DO_5, START_DO_NO, LEARN_DO_NO, LEARN_DIFF_SPELLS, 0.02, COMP_GESTURAL | COMP_GESTURAL_RANDOM | COMP_VERBAL | COMP_VERBAL_RANDOM | COMP_MATERIAL | COMP_MATERIAL_END | SPELL_TASKED, 0);

  discArray[SPELL_PLASMA_MIRROR] = new spellInfo(SPELL_MAGE, DISC_WATER, DISC_WATER, STAT_INT, "plasma mirror", TASK_EASY, LAG_3, POSITION_CRAWLING, MANA_50, LIFEFORCE_0, PRAY_0, TAR_SELF_ONLY | TAR_FIGHT_SELF | TAR_CHAR_ROOM, SYMBOL_STRESS_0, "The swirls of plasma about you disperse.", "The swirls of plasma about $n disperse.", "The plasma about you swirls more slowly now.", "", START_30, LEARN_4, START_DO_45, LEARN_DO_2, START_DO_NO, LEARN_DO_NO, LEARN_DIFF_SPELLS, 0.02, COMP_GESTURAL | COMP_GESTURAL_RANDOM | COMP_VERBAL | COMP_VERBAL_RANDOM | COMP_MATERIAL | COMP_MATERIAL_END | SPELL_TASKED, 0);

  discArray[SPELL_GARMULS_TAIL] = new spellInfo(SPELL_MAGE, DISC_WATER, DISC_WATER, STAT_EXT, "Garmul's tail", TASK_NORMAL, LAG_3, POSITION_CRAWLING, MANA_20, LIFEFORCE_0, PRAY_0, TAR_FIGHT_SELF | TAR_CHAR_ROOM, SYMBOL_STRESS_0, "You cease to move with fluid motion.", "$n ceases to move with fluid motion.", "Your movement is becoming less fluid.", "", START_5, LEARN_4, START_DO_45, LEARN_DO_2, START_DO_NO, LEARN_DO_NO, LEARN_DIFF_SPELLS, 0.02, COMP_GESTURAL | COMP_GESTURAL_RANDOM | COMP_VERBAL | COMP_VERBAL_RANDOM | COMP_MATERIAL | COMP_MATERIAL_END | SPELL_TASKED, 0);


// CLERIC CLASS
// disc_cleric
  discArray[SPELL_HEAL_LIGHT] = new spellInfo(SPELL_CLERIC, DISC_CLERIC, DISC_CURES, STAT_WIS, "heal light", TASK_EASY, LAG_2, POSITION_SITTING, MANA_0, LIFEFORCE_0, PRAY_100, TAR_CHAR_ROOM | TAR_FIGHT_SELF, SYMBOL_STRESS_12, "", "", "", "", START_1, LEARN_10, START_DO_50, LEARN_DO_5, START_DO_NO, LEARN_DO_NO, LEARN_DIFF_PRAYERS, 0.02, COMP_GESTURAL | SPELL_TASKED | SPELL_TASKED_EVERY, 0);

  discArray[SPELL_HARM_LIGHT] = new spellInfo(SPELL_CLERIC, DISC_CLERIC, DISC_AFFLICTIONS, STAT_WIS, "harm light", TASK_NORMAL, LAG_1, POSITION_SITTING, MANA_0, LIFEFORCE_0, PRAY_100, TAR_CHAR_ROOM | TAR_FIGHT_VICT | TAR_SELF_NONO | TAR_VIOLENT, SYMBOL_STRESS_5, "", "", "", "", START_1, LEARN_10, START_DO_50, LEARN_DO_5, START_DO_NO, LEARN_DO_NO, LEARN_DIFF_PRAYERS, 0.01, COMP_GESTURAL | COMP_VERBAL, 0);

  discArray[SPELL_CREATE_FOOD] = new spellInfo(SPELL_CLERIC, DISC_CLERIC, DISC_HAND_OF_GOD, STAT_EXT, "create food", TASK_TRIVIAL, LAG_1, POSITION_CRAWLING, MANA_0, LIFEFORCE_0, PRAY_025, TAR_IGNORE, SYMBOL_STRESS_5, "", "", "", "", START_11, LEARN_25, START_DO_40, LEARN_DO_10, START_DO_NO, LEARN_DO_NO, LEARN_DIFF_PRAYERS, 0.0, COMP_GESTURAL | COMP_VERBAL, 0);

  discArray[SPELL_CREATE_WATER] = new spellInfo(SPELL_CLERIC, DISC_CLERIC, DISC_HAND_OF_GOD, STAT_EXT, "create water", TASK_TRIVIAL, LAG_2, POSITION_CRAWLING, MANA_0, LIFEFORCE_0, PRAY_025, TAR_OBJ_INV, SYMBOL_STRESS_5, "", "","", "", START_11, LEARN_25, START_DO_40, LEARN_DO_10, START_DO_NO, LEARN_DO_NO, LEARN_DIFF_PRAYERS, 0.0, COMP_GESTURAL | COMP_VERBAL | SPELL_TASKED | SPELL_TASKED_EVERY, 0);


  discArray[SPELL_ARMOR] = new spellInfo(SPELL_CLERIC, DISC_CLERIC, DISC_AEGIS, STAT_WIS, "armor", TASK_TRIVIAL, LAG_1, POSITION_SITTING, MANA_0, LIFEFORCE_0, PRAY_025, TAR_CHAR_ROOM | TAR_FIGHT_SELF, SYMBOL_STRESS_15, "The protection around your body fades.", "The protection around $n fades.", "You feel slightly less protected.", "", START_1, LEARN_8, START_DO_40, LEARN_DO_5, START_DO_NO, LEARN_DO_NO, LEARN_DIFF_PRAYERS, 0.2, COMP_GESTURAL | COMP_VERBAL, 0);

  discArray[SPELL_BLESS] = new spellInfo(SPELL_CLERIC, DISC_CLERIC, DISC_AEGIS, STAT_WIS, "bless", TASK_TRIVIAL, LAG_1, POSITION_SITTING, MANA_0, LIFEFORCE_0, PRAY_050, TAR_OBJ_INV | TAR_OBJ_EQUIP | TAR_CHAR_ROOM |TAR_FIGHT_SELF, SYMBOL_STRESS_10, "You feel less holy.", "$n doesn't seem as holy.", "You feel like your deities are getting bored assisting you.", "", START_1, LEARN_33, START_DO_30, LEARN_DO_5, START_DO_NO, LEARN_DO_NO, LEARN_DIFF_PRAYERS, 0.05, COMP_GESTURAL | COMP_VERBAL, 0);

  discArray[SPELL_CLOT] = new spellInfo(SPELL_CLERIC, DISC_CLERIC, DISC_CURES, STAT_EXT, "clot", TASK_TRIVIAL, LAG_1, POSITION_SITTING, MANA_0, LIFEFORCE_0, PRAY_100, TAR_CHAR_ROOM | TAR_FIGHT_SELF, SYMBOL_STRESS_15, "", "", "", "", START_6, LEARN_10, START_DO_40, LEARN_DO_5, START_DO_NO, LEARN_DO_NO, LEARN_DIFF_PRAYERS, 0.04, COMP_GESTURAL | COMP_VERBAL, 0);

  discArray[SPELL_RAIN_BRIMSTONE] = new spellInfo(SPELL_CLERIC, DISC_CLERIC, DISC_WRATH, STAT_WIS, "rain brimstone", TASK_NORMAL, LAG_2, POSITION_SITTING, MANA_0, LIFEFORCE_0, PRAY_150, TAR_CHAR_ROOM | TAR_VIOLENT | TAR_FIGHT_VICT | TAR_SELF_NONO, SYMBOL_STRESS_10, "", "", "", "", START_5, LEARN_20, START_DO_20, LEARN_DO_5, START_DO_NO, LEARN_DO_NO, LEARN_DIFF_PRAYERS, 0.01, COMP_GESTURAL | COMP_VERBAL, 0);

  discArray[SPELL_HEAL_SERIOUS] = new spellInfo(SPELL_CLERIC, DISC_CLERIC, DISC_CURES, STAT_WIS, "heal serious", TASK_EASY, LAG_2, POSITION_SITTING, MANA_0, LIFEFORCE_0, PRAY_150, TAR_CHAR_ROOM | TAR_FIGHT_SELF, SYMBOL_STRESS_18, "", "", "", "", START_18, LEARN_5, START_DO_30, LEARN_DO_5, START_DO_NO, LEARN_DO_NO, LEARN_DIFF_PRAYERS, 0.03, COMP_GESTURAL | COMP_VERBAL | SPELL_TASKED | SPELL_TASKED_EVERY, 0);

  discArray[SPELL_HARM_SERIOUS] = new spellInfo(SPELL_CLERIC, DISC_CLERIC, DISC_AFFLICTIONS, STAT_WIS, "harm serious", TASK_NORMAL, LAG_1, POSITION_SITTING, MANA_0, LIFEFORCE_0, PRAY_150, TAR_CHAR_ROOM | TAR_SELF_NONO | TAR_FIGHT_VICT | TAR_VIOLENT , SYMBOL_STRESS_20, "", "", "", "", START_38, LEARN_5, START_DO_20, LEARN_DO_5, START_DO_NO, LEARN_DO_NO, LEARN_DIFF_PRAYERS, 0.02, COMP_GESTURAL | COMP_VERBAL, 0);

  discArray[SPELL_STERILIZE] = new spellInfo(SPELL_CLERIC, DISC_CLERIC, DISC_CURES, STAT_EXT, "sterilize", TASK_EASY, LAG_1, POSITION_SITTING, MANA_0, LIFEFORCE_0, PRAY_150, TAR_CHAR_ROOM, SYMBOL_STRESS_15, "", "", "", "", START_21, LEARN_5, START_DO_30, LEARN_DO_5, START_DO_NO, LEARN_DO_NO, LEARN_DIFF_PRAYERS, 0.04, COMP_GESTURAL | COMP_VERBAL, 0);

  discArray[SPELL_EXPEL] = new spellInfo(SPELL_CLERIC, DISC_CLERIC, DISC_CURES, STAT_EXT, "expel", TASK_EASY, LAG_2, POSITION_SITTING, MANA_0, LIFEFORCE_0, PRAY_500, TAR_CHAR_ROOM, SYMBOL_STRESS_30, "", "", "", "", START_51, LEARN_5, START_DO_50, LEARN_DO_10, START_DO_NO, LEARN_DO_NO, LEARN_DIFF_PRAYERS, 0.04, COMP_GESTURAL | COMP_VERBAL, 0);

  discArray[SPELL_CURE_DISEASE] = new spellInfo(SPELL_CLERIC, DISC_CLERIC, DISC_CURES, STAT_EXT, "cure disease", TASK_EASY, LAG_2, POSITION_SITTING, MANA_0, LIFEFORCE_0, PRAY_450, TAR_CHAR_ROOM | TAR_FIGHT_SELF, SYMBOL_STRESS_50, "", "", "", "", START_30, LEARN_5, START_DO_40, LEARN_DO_10, START_DO_NO, LEARN_DO_NO, LEARN_DIFF_PRAYERS,  0.02, COMP_GESTURAL | COMP_VERBAL, 0);

  discArray[SPELL_CURSE] = new spellInfo(SPELL_CLERIC, DISC_CLERIC, DISC_AFFLICTIONS, STAT_EXT, "curse", TASK_TRIVIAL, LAG_1, POSITION_FIGHTING, MANA_0, LIFEFORCE_0, PRAY_050, TAR_VIOLENT | TAR_CHAR_ROOM | TAR_OBJ_ROOM | TAR_OBJ_INV | TAR_OBJ_EQUIP | TAR_FIGHT_VICT | TAR_SELF_NONO, SYMBOL_STRESS_15, "You don't feel damned anymore.", "", "", "", START_51, LEARN_10, START_DO_20, LEARN_DO_5, START_DO_NO, LEARN_DO_NO, LEARN_DIFF_PRAYERS, 0.05, COMP_GESTURAL | COMP_VERBAL , 0);

  discArray[SPELL_REMOVE_CURSE] = new spellInfo(SPELL_CLERIC, DISC_CLERIC, DISC_AEGIS, STAT_EXT, "remove curse", TASK_EASY, LAG_2, POSITION_SITTING, MANA_0, LIFEFORCE_0, PRAY_100, TAR_CHAR_ROOM | TAR_FIGHT_SELF | TAR_OBJ_INV | TAR_OBJ_EQUIP | TAR_OBJ_ROOM, SYMBOL_STRESS_25, "", "", "", "", START_27, LEARN_5, START_DO_50, LEARN_DO_5, START_DO_NO, LEARN_DO_NO, LEARN_DIFF_PRAYERS, 0.04, COMP_GESTURAL | COMP_VERBAL, 0);

  discArray[SPELL_CURE_POISON] = new spellInfo(SPELL_CLERIC, DISC_CLERIC, DISC_AEGIS, STAT_EXT, "cure poison", TASK_EASY, LAG_2, POSITION_SITTING, MANA_0, LIFEFORCE_0, PRAY_100, TAR_CHAR_ROOM | TAR_FIGHT_SELF, SYMBOL_STRESS_35, "", "", "", "", START_11, LEARN_5, START_DO_30, LEARN_DO_10, START_DO_NO, LEARN_DO_NO, LEARN_DIFF_PRAYERS, 0.02, COMP_GESTURAL | COMP_VERBAL, 0);

  discArray[SPELL_HEAL_CRITICAL] = new spellInfo(SPELL_CLERIC, DISC_CLERIC, DISC_CURES, STAT_WIS, "heal critical", TASK_EASY, LAG_2, POSITION_SITTING, MANA_0, LIFEFORCE_0, PRAY_200, TAR_CHAR_ROOM | TAR_FIGHT_SELF, SYMBOL_STRESS_25, "", "", "", "", START_36, LEARN_5, START_DO_20, LEARN_DO_2, START_DO_NO, LEARN_DO_NO, LEARN_DIFF_PRAYERS, 0.04, COMP_GESTURAL | COMP_VERBAL | SPELL_TASKED | SPELL_TASKED_EVERY, 0);

  discArray[SPELL_SALVE] = new spellInfo(SPELL_CLERIC, DISC_CLERIC, DISC_CURES, STAT_EXT, "salve", TASK_TRIVIAL, LAG_1, POSITION_SITTING, MANA_0, LIFEFORCE_0, PRAY_150, TAR_CHAR_ROOM | TAR_FIGHT_SELF, SYMBOL_STRESS_10, "", "", "", "", START_15, LEARN_8, START_DO_30, LEARN_DO_2, START_DO_NO, LEARN_DO_NO, LEARN_DIFF_PRAYERS, 0.02, COMP_GESTURAL | COMP_VERBAL, 0);

  discArray[SPELL_POISON] = new spellInfo(SPELL_CLERIC, DISC_CLERIC, DISC_AFFLICTIONS, STAT_EXT, "poison", TASK_NORMAL, LAG_1, POSITION_SITTING, MANA_0, LIFEFORCE_0, PRAY_100, TAR_CHAR_ROOM | TAR_OBJ_INV | TAR_FIGHT_VICT | TAR_VIOLENT | TAR_FIGHT_VICT | TAR_SELF_NONO, SYMBOL_STRESS_45, "You feel much healthier.", "The color returns to $n's cheeks.", "", "", START_59, LEARN_10, START_DO_30, LEARN_DO_2, START_DO_NO, LEARN_DO_NO, LEARN_DIFF_PRAYERS, 0.02, COMP_GESTURAL | COMP_VERBAL, 0);

  discArray[SPELL_HARM_CRITICAL] = new spellInfo(SPELL_CLERIC, DISC_CLERIC, DISC_AFFLICTIONS, STAT_WIS, "harm critical", TASK_NORMAL, LAG_2, POSITION_SITTING, MANA_0, LIFEFORCE_0, PRAY_200, TAR_CHAR_ROOM | TAR_SELF_NONO | TAR_FIGHT_VICT | TAR_VIOLENT , SYMBOL_STRESS_35, "", "", "", "", START_56, LEARN_10, START_DO_20, LEARN_DO_5, START_DO_NO, LEARN_DO_NO, LEARN_DIFF_PRAYERS, 0.03, COMP_GESTURAL | COMP_VERBAL, 0);

  discArray[SPELL_INFECT] = new spellInfo(SPELL_CLERIC, DISC_CLERIC, DISC_AFFLICTIONS, STAT_EXT, "infect", TASK_NORMAL, LAG_1, POSITION_SITTING, MANA_0, LIFEFORCE_0, PRAY_150, TAR_CHAR_ROOM | TAR_FIGHT_VICT | TAR_VIOLENT | TAR_SELF_NONO, SYMBOL_STRESS_40, "", "", "", "", START_63, LEARN_8, START_DO_20, LEARN_DO_5, START_DO_NO, LEARN_DO_NO, LEARN_DIFF_PRAYERS, 0.04, COMP_GESTURAL | COMP_VERBAL, 0);

  discArray[SPELL_REFRESH] = new spellInfo(SPELL_CLERIC, DISC_CLERIC, DISC_AEGIS, STAT_EXT, "refresh", TASK_EASY, LAG_2, POSITION_SITTING, MANA_0, LIFEFORCE_0, PRAY_100, TAR_CHAR_ROOM | TAR_FIGHT_SELF, SYMBOL_STRESS_20, "", "", "", "", START_32, LEARN_7, START_DO_20, LEARN_DO_5, START_DO_NO, LEARN_DO_NO, LEARN_DIFF_PRAYERS, 0.02, COMP_GESTURAL | COMP_VERBAL, 0);

  discArray[SPELL_NUMB] = new spellInfo(SPELL_CLERIC, DISC_CLERIC, DISC_AFFLICTIONS, STAT_WIS, "numb", TASK_EASY, LAG_2, POSITION_SITTING, MANA_0, LIFEFORCE_0, PRAY_100, TAR_CHAR_ROOM | TAR_FIGHT_VICT | TAR_VIOLENT | TAR_SELF_NONO, SYMBOL_STRESS_30, "", "", "", "", START_59, LEARN_5, START_DO_40, LEARN_DO_5, START_DO_NO, LEARN_DO_NO, LEARN_DIFF_PRAYERS, 0.05, COMP_GESTURAL | COMP_VERBAL, 0);

  discArray[SPELL_DISEASE] = new spellInfo(SPELL_CLERIC, DISC_CLERIC, DISC_AFFLICTIONS, STAT_EXT, "disease", TASK_NORMAL, LAG_2, POSITION_SITTING, MANA_0, LIFEFORCE_0, PRAY_150, TAR_CHAR_ROOM | TAR_VIOLENT | TAR_FIGHT_VICT | TAR_SELF_NONO, SYMBOL_STRESS_45, "", "", "", "", START_56, LEARN_4, START_DO_40, LEARN_DO_2, START_DO_NO, LEARN_DO_NO, LEARN_DIFF_PRAYERS, 0.05, COMP_GESTURAL | COMP_VERBAL, 0);

  discArray[SPELL_FLAMESTRIKE] = new spellInfo(SPELL_CLERIC, DISC_CLERIC, DISC_WRATH, STAT_WIS, "flamestrike", TASK_NORMAL, LAG_3, POSITION_SITTING, MANA_0, LIFEFORCE_0, PRAY_200, TAR_CHAR_ROOM | TAR_VIOLENT | TAR_FIGHT_VICT | TAR_SELF_NONO , SYMBOL_STRESS_30, "", "", "", "", START_51, LEARN_10, START_DO_25, LEARN_DO_2, START_DO_NO, LEARN_DO_NO, LEARN_DIFF_PRAYERS,  0.04, COMP_GESTURAL | COMP_VERBAL, 0);

  discArray[SPELL_PLAGUE_LOCUSTS] = new spellInfo(SPELL_CLERIC, DISC_CLERIC, DISC_WRATH, STAT_WIS, "plague of locusts", TASK_NORMAL, LAG_3, POSITION_SITTING, MANA_0, LIFEFORCE_0, PRAY_500, TAR_CHAR_ROOM | TAR_VIOLENT | TAR_FIGHT_VICT | TAR_SELF_NONO, SYMBOL_STRESS_85, "$n circles the area once and then dissipates.", "$n circles the area once and then dissipates.", "", "", START_81, LEARN_5, START_DO_40, LEARN_DO_3, START_DO_NO, LEARN_DO_NO, LEARN_DIFF_PRAYERS, 0.01, COMP_GESTURAL | COMP_VERBAL, 0);

  discArray[SPELL_CURE_BLINDNESS] = new spellInfo(SPELL_CLERIC, DISC_CLERIC, DISC_AEGIS, STAT_EXT, "cure blindness", TASK_EASY, LAG_2, POSITION_SITTING, MANA_0, LIFEFORCE_0, PRAY_250, TAR_CHAR_ROOM | TAR_FIGHT_SELF, SYMBOL_STRESS_40, "", "", "", "", START_45, LEARN_5, START_DO_40, LEARN_DO_5, START_DO_NO, LEARN_DO_NO, LEARN_DIFF_PRAYERS, 0.02, COMP_GESTURAL | COMP_VERBAL, 0);

  discArray[SPELL_SUMMON] = new spellInfo(SPELL_CLERIC, DISC_CLERIC, DISC_HAND_OF_GOD, STAT_EXT, "summon", TASK_EASY, LAG_2, POSITION_CRAWLING, MANA_0, LIFEFORCE_0, PRAY_300, TAR_CHAR_WORLD, SYMBOL_STRESS_75, "", "", "", "", START_76, LEARN_10, START_DO_50, LEARN_DO_5, START_DO_NO, LEARN_DO_NO, LEARN_DIFF_PRAYERS, 0.0, COMP_GESTURAL | COMP_VERBAL, 0);

  discArray[SPELL_HEAL] = new spellInfo(SPELL_CLERIC, DISC_CLERIC, DISC_CURES, STAT_WIS, "heal", TASK_NORMAL, LAG_2, POSITION_SITTING, MANA_0, LIFEFORCE_0, PRAY_250, TAR_CHAR_ROOM | TAR_FIGHT_SELF, SYMBOL_STRESS_48, "", "", "", "", START_65, LEARN_5, START_DO_20, LEARN_DO_2, START_DO_NO, LEARN_DO_NO, LEARN_DIFF_PRAYERS, 0.06, COMP_GESTURAL | COMP_VERBAL | SPELL_TASKED | SPELL_TASKED_EVERY, 0);

  discArray[SPELL_PARALYZE_LIMB] = new spellInfo(SPELL_CLERIC, DISC_CLERIC, DISC_AFFLICTIONS, STAT_WIS, "paralyze limb", TASK_EASY, LAG_3, POSITION_SITTING, MANA_0, LIFEFORCE_0, PRAY_200, TAR_CHAR_ROOM | TAR_FIGHT_VICT | TAR_VIOLENT | TAR_SELF_NONO, SYMBOL_STRESS_50, "", "", "", "", START_86, LEARN_10, START_DO_20, LEARN_DO_2, START_DO_NO, LEARN_DO_NO, LEARN_DIFF_PRAYERS, 0.05, COMP_GESTURAL | COMP_VERBAL, 0);

  discArray[SPELL_WORD_OF_RECALL] = new spellInfo(SPELL_CLERIC, DISC_CLERIC, DISC_HAND_OF_GOD, STAT_EXT, "word of recall", TASK_TRIVIAL, LAG_3, POSITION_SITTING, MANA_0, LIFEFORCE_0, PRAY_500, TAR_CHAR_ROOM | TAR_FIGHT_SELF, SYMBOL_STRESS_100, "", "", "", "", START_83, LEARN_10, START_DO_40, LEARN_DO_5, START_DO_NO, LEARN_DO_NO, LEARN_DIFF_PRAYERS, 0.0, COMP_GESTURAL | COMP_VERBAL, 0);

  discArray[SPELL_HARM] = new spellInfo(SPELL_CLERIC, DISC_CLERIC, DISC_AFFLICTIONS, STAT_WIS, "harm", TASK_NORMAL, LAG_3, POSITION_SITTING, MANA_0, LIFEFORCE_0, PRAY_250, TAR_CHAR_ROOM | TAR_FIGHT_VICT | TAR_SELF_NONO | TAR_VIOLENT , SYMBOL_STRESS_50, "", "", "", "", START_79, LEARN_10, START_DO_20, LEARN_DO_3, START_DO_NO, LEARN_DO_NO, LEARN_DIFF_PRAYERS, 0.06, COMP_GESTURAL | COMP_VERBAL, 0);

  discArray[SPELL_BLINDNESS] = new spellInfo(SPELL_CLERIC, DISC_CLERIC, DISC_AFFLICTIONS, STAT_EXT, "blindness", TASK_NORMAL, LAG_2, POSITION_SITTING, MANA_0, LIFEFORCE_0, PRAY_250, TAR_CHAR_ROOM | TAR_VIOLENT | TAR_FIGHT_VICT | TAR_SELF_NONO, SYMBOL_STRESS_80, "You can see again!", "", "", "", START_84, LEARN_6, START_DO_20, LEARN_DO_2, START_DO_NO, LEARN_DO_NO, LEARN_DIFF_PRAYERS, 0.02, COMP_GESTURAL | COMP_VERBAL, 0);

  discArray[SKILL_REPAIR_CLERIC] = new spellInfo(SKILL_CLERIC, DISC_CLERIC, DISC_BLACKSMITHING, STAT_EXT, "cleric repair", TASK_NORMAL, LAG_0, POSITION_SITTING, MANA_0, LIFEFORCE_0, PRAY_0, 0, SYMBOL_STRESS_0, "", "", "", "", START_50, LEARN_3, START_DO_1, LEARN_DO_1, START_DO_NO, LEARN_DO_NO, LEARN_DIFF_SKILLS, 0.0, 0, 0);


// disc_wrath

  discArray[SPELL_PILLAR_SALT] = new spellInfo(SPELL_CLERIC, DISC_WRATH, DISC_WRATH, STAT_WIS, "pillar of salt", TASK_NORMAL, LAG_3, POSITION_SITTING, MANA_0, LIFEFORCE_0, PRAY_200, TAR_CHAR_ROOM | TAR_VIOLENT | TAR_FIGHT_VICT | TAR_SELF_NONO, SYMBOL_STRESS_65, "", "", "", "", START_1, LEARN_4, START_DO_20, LEARN_DO_2, START_DO_NO, LEARN_DO_NO, LEARN_DIFF_PRAYERS, 0.01, COMP_GESTURAL | COMP_VERBAL, 0);

  discArray[SPELL_EARTHQUAKE] = new spellInfo(SPELL_CLERIC, DISC_WRATH, DISC_WRATH, STAT_WIS, "earthquake", TASK_DIFFICULT, LAG_3, POSITION_SITTING, MANA_0, LIFEFORCE_0, PRAY_400, TAR_AREA | TAR_IGNORE | TAR_VIOLENT, SYMBOL_STRESS_90, "", "", "", "", START_26, LEARN_4, START_DO_20, LEARN_DO_2, START_DO_NO, LEARN_DO_NO, LEARN_DIFF_PRAYERS, 0.04, COMP_GESTURAL | COMP_VERBAL, TOG_HAS_EARTHQUAKE);
 
  discArray[SPELL_CALL_LIGHTNING] = new spellInfo(SPELL_CLERIC, DISC_WRATH, DISC_WRATH, STAT_WIS, "call lightning", TASK_DIFFICULT, LAG_2, POSITION_SITTING, MANA_0, LIFEFORCE_0, PRAY_300, TAR_CHAR_ROOM | TAR_VIOLENT | TAR_FIGHT_VICT | TAR_SELF_NONO , SYMBOL_STRESS_50, "", "", "", "", START_51, LEARN_4, START_DO_30, LEARN_DO_3, START_DO_NO, LEARN_DO_NO, LEARN_DIFF_PRAYERS, 0.06, COMP_GESTURAL | COMP_VERBAL, 0);

  discArray[SPELL_SPONTANEOUS_COMBUST] = new spellInfo(SPELL_CLERIC, DISC_WRATH, DISC_WRATH, STAT_WIS, "spontaneous combust", TASK_DIFFICULT, LAG_5, POSITION_SITTING, MANA_0, LIFEFORCE_0, PRAY_400, TAR_CHAR_ROOM | TAR_VIOLENT | TAR_FIGHT_VICT | TAR_SELF_NONO , SYMBOL_STRESS_75, "", "", "", "", START_76, LEARN_4, START_DO_30, LEARN_DO_2, START_DO_NO, LEARN_DO_NO, LEARN_DIFF_PRAYERS, 0.06, COMP_GESTURAL | COMP_VERBAL, 0);

// disc_afflictions


  discArray[SPELL_BLEED] = new spellInfo(SPELL_CLERIC, DISC_AFFLICTIONS, DISC_AFFLICTIONS, STAT_EXT, "bleed", TASK_EASY, LAG_1, POSITION_SITTING, MANA_0, LIFEFORCE_0, PRAY_100, TAR_CHAR_ROOM | TAR_FIGHT_VICT | TAR_VIOLENT | TAR_SELF_NONO, SYMBOL_STRESS_35, "", "", "", "", START_1, LEARN_4, START_DO_30, LEARN_DO_2, START_DO_NO, LEARN_DO_NO, LEARN_DIFF_PRAYERS, 0.04, COMP_GESTURAL | COMP_VERBAL, 0);

  discArray[SPELL_PARALYZE] = new spellInfo(SPELL_CLERIC, DISC_AFFLICTIONS, DISC_AFFLICTIONS, STAT_WIS, "paralyze", TASK_NORMAL, LAG_3, POSITION_SITTING, MANA_0, LIFEFORCE_0, PRAY_300, TAR_CHAR_ROOM | TAR_VIOLENT | TAR_FIGHT_VICT | TAR_SELF_NONO , SYMBOL_STRESS_95, "Your body unstiffens and you can feel the blood circulating again.", "", "", "", START_26, LEARN_4, START_DO_20, LEARN_DO_1, START_DO_NO, LEARN_DO_NO, LEARN_DIFF_PRAYERS, 0.02, COMP_GESTURAL | COMP_VERBAL, 0);

  discArray[SPELL_BONE_BREAKER] = new spellInfo(SPELL_CLERIC, DISC_AFFLICTIONS, DISC_AFFLICTIONS, STAT_WIS, "bone breaker", TASK_NORMAL, LAG_3, POSITION_SITTING, MANA_0, LIFEFORCE_0, PRAY_300, TAR_CHAR_ROOM | TAR_FIGHT_VICT | TAR_VIOLENT | TAR_SELF_NONO, SYMBOL_STRESS_85, "", "", "", "", START_51, LEARN_4, START_DO_20, LEARN_DO_1, START_DO_NO, LEARN_DO_NO, LEARN_DIFF_PRAYERS, 0.06, COMP_GESTURAL | COMP_VERBAL, 0);

  discArray[SPELL_WITHER_LIMB] = new spellInfo(SPELL_CLERIC, DISC_AFFLICTIONS, DISC_AFFLICTIONS, STAT_WIS, "wither limb", TASK_NORMAL, LAG_3, POSITION_SITTING, MANA_0, LIFEFORCE_0, PRAY_250, TAR_CHAR_ROOM | TAR_FIGHT_VICT | TAR_VIOLENT | TAR_SELF_NONO, SYMBOL_STRESS_70, "", "", "", "", START_76, LEARN_4, START_DO_20, LEARN_DO_2, START_DO_NO, LEARN_DO_NO, LEARN_DIFF_PRAYERS, 0.06, COMP_GESTURAL | COMP_VERBAL, 0);


// disc_aegis

  discArray[SPELL_SANCTUARY] = new spellInfo(SPELL_CLERIC, DISC_AEGIS, DISC_AEGIS, STAT_WIS, "sanctuary", TASK_EASY, LAG_3, POSITION_SITTING, MANA_0, LIFEFORCE_0, PRAY_500, TAR_CHAR_ROOM | TAR_FIGHT_SELF, SYMBOL_STRESS_75, "The white aura around your body fades.", "The white aura around $n's body fades.", "The white aura around your body flickers slightly.", "The white aura around $n's body flickers slightly.", START_1, LEARN_20, START_DO_20, LEARN_DO_2, START_DO_NO, LEARN_DO_NO, LEARN_DIFF_PRAYERS, 0.06, COMP_GESTURAL | COMP_VERBAL, 0);

  discArray[SPELL_CURE_PARALYSIS] = new spellInfo(SPELL_CLERIC, DISC_AEGIS, DISC_AEGIS, STAT_EXT, "cure paralysis", TASK_EASY, LAG_2, POSITION_SITTING, MANA_0, LIFEFORCE_0, PRAY_300, TAR_CHAR_ROOM | TAR_FIGHT_VICT, SYMBOL_STRESS_75, "", "", "", "", START_40, LEARN_4, START_DO_50, LEARN_DO_5, START_DO_NO, LEARN_DO_NO, LEARN_DIFF_PRAYERS, 0.02, COMP_GESTURAL | COMP_VERBAL, 0);

  discArray[SPELL_SECOND_WIND] = new spellInfo(SPELL_CLERIC, DISC_AEGIS, DISC_AEGIS, STAT_EXT, "second wind", TASK_EASY, LAG_2, POSITION_SITTING, MANA_0, LIFEFORCE_0, PRAY_300, TAR_CHAR_ROOM | TAR_FIGHT_SELF, SYMBOL_STRESS_45, "", "", "", "", START_26, LEARN_20, START_DO_30, LEARN_DO_2, START_DO_NO, LEARN_DO_NO, LEARN_DIFF_PRAYERS, 0.01, COMP_GESTURAL | COMP_VERBAL, 0);

  discArray[SPELL_RELIVE] = new spellInfo(SPELL_CLERIC, DISC_AEGIS, DISC_AEGIS, STAT_EXT, "relive", TASK_NORMAL, LAG_2, POSITION_SITTING, MANA_0, LIFEFORCE_0, PRAY_300, TAR_CHAR_ROOM, SYMBOL_STRESS_100, "", "", "", "", START_80, LEARN_5, START_DO_50, LEARN_DO_5, START_DO_NO, LEARN_DO_NO, LEARN_DIFF_PRAYERS, 0.02, COMP_GESTURAL | COMP_VERBAL, 0);


// disc_hand_of_god
 
  discArray[SPELL_HEROES_FEAST] = new spellInfo(SPELL_CLERIC, DISC_HAND_OF_GOD, DISC_HAND_OF_GOD, STAT_EXT, "heroes feast", TASK_TRIVIAL, LAG_2, POSITION_CRAWLING, MANA_0, LIFEFORCE_0, PRAY_075, TAR_AREA | TAR_IGNORE, SYMBOL_STRESS_15, "", "", "", "", START_1, LEARN_10, START_DO_20, LEARN_DO_5, START_DO_NO, LEARN_DO_NO, LEARN_DIFF_PRAYERS, 0.1, COMP_GESTURAL | COMP_VERBAL, 0);

  discArray[SPELL_ASTRAL_WALK] = new spellInfo(SPELL_CLERIC, DISC_HAND_OF_GOD, DISC_HAND_OF_GOD, STAT_EXT, "astral walk", TASK_EASY, LAG_2, POSITION_CRAWLING, MANA_0, LIFEFORCE_0, PRAY_200, TAR_CHAR_WORLD, SYMBOL_STRESS_50, "", "", "", "", START_33, LEARN_3, START_DO_40, LEARN_DO_5, START_DO_NO, LEARN_DO_NO, LEARN_DIFF_PRAYERS, 0.0, COMP_GESTURAL | COMP_VERBAL, 0);

  discArray[SPELL_PORTAL] = new spellInfo(SPELL_CLERIC, DISC_HAND_OF_GOD, DISC_HAND_OF_GOD, STAT_EXT, "portal", TASK_NORMAL, LAG_3, POSITION_CRAWLING, MANA_0, LIFEFORCE_0, PRAY_300, TAR_NAME , SYMBOL_STRESS_75, "", "", "", "", START_80, LEARN_5, START_DO_50, LEARN_DO_5, START_DO_NO, LEARN_DO_NO, LEARN_DIFF_PRAYERS, 0.0, COMP_GESTURAL | COMP_VERBAL, 0);

// disc_cures
  discArray[SPELL_HEAL_FULL] = new spellInfo(SPELL_CLERIC, DISC_CURES, DISC_CURES, STAT_WIS, "heal full", TASK_NORMAL, LAG_2, POSITION_SITTING, MANA_0, LIFEFORCE_0, PRAY_300, TAR_CHAR_ROOM | TAR_FIGHT_SELF, SYMBOL_STRESS_60, "", "", "", "", START_12, LEARN_6, START_DO_20, LEARN_DO_3, START_DO_NO, LEARN_DO_NO, LEARN_DIFF_PRAYERS, 0.07, COMP_GESTURAL | COMP_VERBAL | SPELL_TASKED | SPELL_TASKED_EVERY, 0);

  discArray[SPELL_HEAL_CRITICAL_SPRAY] = new spellInfo(SPELL_CLERIC, DISC_CURES, DISC_CURES, STAT_WIS, "heal critical spray", TASK_EASY, LAG_3, POSITION_SITTING, MANA_0, LIFEFORCE_0, PRAY_250, TAR_AREA | TAR_IGNORE, SYMBOL_STRESS_70, "", "", "", "", START_46, LEARN_6, START_DO_30, LEARN_DO_5, START_DO_NO, LEARN_DO_NO, LEARN_DIFF_PRAYERS, 0.04, COMP_GESTURAL | COMP_VERBAL, 0);

  discArray[SPELL_HEAL_SPRAY] = new spellInfo(SPELL_CLERIC, DISC_CURES, DISC_CURES, STAT_WIS, "heal spray", TASK_NORMAL, LAG_3, POSITION_SITTING, MANA_0, LIFEFORCE_0, PRAY_300, TAR_AREA | TAR_IGNORE, SYMBOL_STRESS_85, "", "", "", "", START_60, LEARN_6, START_DO_30, LEARN_DO_5, START_DO_NO, LEARN_DO_NO, LEARN_DIFF_PRAYERS, 0.06, COMP_GESTURAL | COMP_VERBAL, 0);

  discArray[SPELL_HEAL_FULL_SPRAY] = new spellInfo(SPELL_CLERIC, DISC_CURES, DISC_CURES, STAT_WIS, "heal full spray", TASK_NORMAL, LAG_3, POSITION_SITTING, MANA_0, LIFEFORCE_0, PRAY_350, TAR_AREA | TAR_IGNORE, SYMBOL_STRESS_100, "", "", "", "", START_84, LEARN_6, START_DO_30, LEARN_DO_5, START_DO_NO, LEARN_DO_NO, LEARN_DIFF_PRAYERS, 0.07, COMP_GESTURAL | COMP_VERBAL, 0);

  discArray[SPELL_RESTORE_LIMB] = new spellInfo(SPELL_CLERIC, DISC_CURES, DISC_CURES, STAT_EXT, "restore limb", TASK_EASY, LAG_2, POSITION_SITTING, MANA_0, LIFEFORCE_0, PRAY_250, TAR_CHAR_ROOM | TAR_FIGHT_SELF, SYMBOL_STRESS_80, "", "", "", "", START_1, LEARN_8, START_DO_50, LEARN_DO_5, START_DO_NO, LEARN_DO_NO, LEARN_DIFF_PRAYERS, 0.02, COMP_GESTURAL | COMP_VERBAL, 0);

  discArray[SPELL_KNIT_BONE] = new spellInfo(SPELL_CLERIC, DISC_CLERIC, DISC_CURES, STAT_EXT, "knit bone", TASK_EASY, LAG_2, POSITION_SITTING, MANA_0, LIFEFORCE_0, PRAY_300, TAR_CHAR_ROOM | TAR_FIGHT_SELF, SYMBOL_STRESS_70, "", "", "", "", START_99, LEARN_100, START_DO_40, LEARN_DO_5, START_DO_NO, LEARN_DO_NO, LEARN_DIFF_PRAYERS, 0.04, COMP_GESTURAL | COMP_VERBAL, 0);


// WARRIOR CLASS

// disc_warrior
  discArray[SKILL_KICK] = new spellInfo(SKILL_WARRIOR, DISC_WARRIOR, DISC_DUELING, STAT_STR, "kick", TASK_EASY, LAG_3, POSITION_STANDING, MANA_0, LIFEFORCE_0, PRAY_0, 0, SYMBOL_STRESS_0, "", "", "", "", START_1, LEARN_5, START_DO_1, LEARN_DO_1, START_DO_NO, LEARN_DO_NO, LEARN_DIFF_SKILLS, 0.0, 0, 0);

  discArray[SKILL_BASH] = new spellInfo(SKILL_WARRIOR, DISC_WARRIOR, DISC_BRAWLING, STAT_STR, "bash", TASK_EASY, LAG_3, POSITION_CRAWLING, MANA_0, LIFEFORCE_0, PRAY_0, 0, SYMBOL_STRESS_0, "", "", "", "", START_1, LEARN_5, START_DO_1, LEARN_DO_1, START_DO_NO, LEARN_DO_NO, LEARN_DIFF_SKILLS, 0.0, 0, 0);

  discArray[SKILL_TRIP] = new spellInfo(SKILL_WARRIOR, DISC_WARRIOR, DISC_BRAWLING, STAT_DEX, "trip", TASK_NORMAL, LAG_3, POSITION_CRAWLING, MANA_0, LIFEFORCE_0, PRAY_0, 0, SYMBOL_STRESS_0, "", "", "", "", START_1, LEARN_5, START_DO_1, LEARN_DO_1, START_DO_NO, LEARN_DO_NO, LEARN_DIFF_SKILLS, 0.0, 0, 0);

  discArray[SKILL_HEADBUTT] = new spellInfo(SKILL_WARRIOR, DISC_WARRIOR, DISC_BRAWLING, STAT_STR, "headbutt", TASK_NORMAL, LAG_4, POSITION_CRAWLING, MANA_0, LIFEFORCE_0, PRAY_0, 0, SYMBOL_STRESS_0, "", "", "", "", START_41, LEARN_5, START_DO_1, LEARN_DO_2, START_DO_NO, LEARN_DO_NO, LEARN_DIFF_SKILLS, 0.0, 0, 0);

  discArray[SKILL_RESCUE] = new spellInfo(SKILL_WARRIOR, DISC_WARRIOR, DISC_DUELING, STAT_EXT, "rescue", TASK_EASY, LAG_2, POSITION_CRAWLING, MANA_0, LIFEFORCE_0, PRAY_0, 0, SYMBOL_STRESS_0, "", "", "", "", START_11, LEARN_3, START_DO_10, LEARN_DO_1, START_DO_NO, LEARN_DO_NO, LEARN_DIFF_SKILLS, 0.15, 0, 0);

  discArray[SKILL_BLACKSMITHING] = new spellInfo(SKILL_WARRIOR, DISC_WARRIOR, DISC_BLACKSMITHING, STAT_EXT, "blacksmithing", TASK_NORMAL, LAG_0, POSITION_SITTING, MANA_0, LIFEFORCE_0, PRAY_0, 0, SYMBOL_STRESS_0, "", "", "", "", START_50, LEARN_3, START_DO_1, LEARN_DO_1, START_DO_NO, LEARN_DO_NO, LEARN_DIFF_SKILLS, 0.0, 0, 0);

  discArray[SKILL_DISARM] = new spellInfo(SKILL_WARRIOR, DISC_WARRIOR, DISC_DUELING, STAT_EXT, "disarm", TASK_DANGEROUS, LAG_2, POSITION_CRAWLING, MANA_0, LIFEFORCE_0, PRAY_0, 0, SYMBOL_STRESS_0, "", "", "", "", START_45, LEARN_5, START_DO_1, LEARN_DO_2, START_DO_NO, LEARN_DO_NO, LEARN_DIFF_SKILLS, 0.0, 0, 0);

  discArray[SKILL_BERSERK] = new spellInfo(SKILL_WARRIOR, DISC_WARRIOR, DISC_SOLDIERING, STAT_EXT, "berserk", TASK_NORMAL, LAG_2, POSITION_CRAWLING, MANA_0, LIFEFORCE_0, PRAY_0, 0, SYMBOL_STRESS_0, "", "", "", "", START_1, LEARN_2, START_DO_1, LEARN_DO_1, START_DO_NO, LEARN_DO_NO, LEARN_DIFF_SKILLS, 0.0, 0, 0);

  discArray[SKILL_SWITCH_OPP] = new spellInfo(SKILL_WARRIOR, DISC_WARRIOR, DISC_DUELING, STAT_EXT, "switch opponents", TASK_EASY, LAG_2, POSITION_CRAWLING, MANA_0, LIFEFORCE_0, PRAY_0, 0, SYMBOL_STRESS_0, "", "", "", "", START_61, LEARN_3, START_DO_10, LEARN_DO_2, START_DO_NO, LEARN_DO_NO, LEARN_DIFF_SKILLS, 0.0, 0, 0);

  discArray[SKILL_KNEESTRIKE] = new spellInfo(SKILL_WARRIOR, DISC_WARRIOR, DISC_DUELING, STAT_STR, "kneestrike", TASK_NORMAL, LAG_4, POSITION_STANDING, MANA_0, LIFEFORCE_0, PRAY_0, 0, SYMBOL_STRESS_0, "", "", "", "", START_60, LEARN_4, START_DO_1, LEARN_DO_2, START_DO_NO, LEARN_DO_NO, LEARN_DIFF_SKILLS, 0.0, 0, 0);


// disc_dueling

  discArray[SKILL_RETREAT] = new spellInfo(SKILL_WARRIOR, DISC_DUELING, DISC_DUELING, STAT_EXT, "retreat", TASK_EASY, LAG_0, POSITION_CRAWLING, MANA_0, LIFEFORCE_0, PRAY_0, 0, SYMBOL_STRESS_0, "", "", "", "", START_1, LEARN_1, START_DO_1, LEARN_DO_1, START_DO_NO, LEARN_DO_NO, LEARN_DIFF_UNUSUAL, 0.0, 0, 0);

  discArray[SKILL_SHOVE] = new spellInfo(SKILL_WARRIOR, DISC_DUELING, DISC_DUELING, STAT_EXT, "shove", TASK_NORMAL, LAG_3, POSITION_CRAWLING, MANA_0, LIFEFORCE_0, PRAY_0, 0, SYMBOL_STRESS_0, "", "", "", "", START_21, LEARN_4, START_DO_1, LEARN_DO_2, START_DO_NO, LEARN_DO_NO, LEARN_DIFF_SKILLS, 0.0, 0, 0);

  discArray[SKILL_PARRY_WARRIOR] = new spellInfo(SKILL_WARRIOR, DISC_DUELING, DISC_DUELING, STAT_EXT, "parry", TASK_EASY, LAG_0, POSITION_DEAD, MANA_0, LIFEFORCE_0, PRAY_0, 0, SYMBOL_STRESS_0, "", "", "", "", START_1, LEARN_1,START_DO_1, LEARN_DO_1, START_DO_NO, LEARN_DO_NO, LEARN_DIFF_UNUSUAL, 0.0, 0, 0);

  discArray[SKILL_RIPOSTE] = new spellInfo(SKILL_WARRIOR, DISC_DUELING, DISC_DUELING, STAT_EXT, "riposte", TASK_NORMAL, LAG_0, POSITION_DEAD, MANA_0, LIFEFORCE_0, PRAY_0, 0, SYMBOL_STRESS_0, "", "", "", "", START_50, LEARN_2,START_DO_1, LEARN_DO_1, START_DO_NO, LEARN_DO_NO, LEARN_DIFF_UNUSUAL, 0.0, 0, 0);

  discArray[SKILL_TRANCE_OF_BLADES] = new spellInfo(SKILL_WARRIOR, DISC_DUELING, DISC_DUELING, STAT_EXT, "defensive trance", TASK_NORMAL, LAG_4, POSITION_CRAWLING, MANA_0, LIFEFORCE_0, PRAY_0, 0, SYMBOL_STRESS_0, "","","","", START_75, LEARN_4, START_DO_10, LEARN_DO_1, START_DO_NO, LEARN_DO_NO, LEARN_DIFF_SKILLS, 0.0, 0, 0);

  discArray[SKILL_WEAPON_RETENTION] = new spellInfo(SKILL_WARRIOR, DISC_DUELING, DISC_DUELING, STAT_EXT, "weapon retention", TASK_EASY, LAG_4, POSITION_CRAWLING, MANA_0, LIFEFORCE_0, PRAY_0, 0, SYMBOL_STRESS_0, "","","","", START_75, LEARN_4, START_DO_10, LEARN_DO_1, START_DO_NO, LEARN_DO_NO, LEARN_DIFF_SKILLS, 0.0, 0, 0);


// disc_brawling

  discArray[SKILL_GRAPPLE] = new spellInfo(SKILL_WARRIOR, DISC_BRAWLING, DISC_BRAWLING, STAT_STR, "grapple", TASK_NORMAL, LAG_4, POSITION_CRAWLING, MANA_0, LIFEFORCE_0, PRAY_0, 0, SYMBOL_STRESS_0, "", "", "", "", START_1, LEARN_3, START_DO_10, LEARN_DO_2, START_DO_NO, LEARN_DO_NO, LEARN_DIFF_SKILLS, 0.0, 0, 0);

  discArray[SKILL_STOMP] = new spellInfo(SKILL_WARRIOR, DISC_BRAWLING, DISC_BRAWLING, STAT_STR, "stomp", TASK_NORMAL, LAG_4, POSITION_CRAWLING, MANA_0, LIFEFORCE_0, PRAY_0, 0, SYMBOL_STRESS_0, "", "", "", "", START_50, LEARN_2, START_DO_10, LEARN_DO_1, START_DO_NO, LEARN_DO_NO, LEARN_DIFF_SKILLS, 0.0, 0, 0);

  discArray[SKILL_BRAWL_AVOIDANCE] = new spellInfo(SKILL_WARRIOR, DISC_BRAWLING, DISC_BRAWLING, STAT_EXT, "brawl avoidance", TASK_NORMAL, LAG_4, POSITION_CRAWLING, MANA_0, LIFEFORCE_0, PRAY_0, 0, SYMBOL_STRESS_0, "", "", "", "", START_75, LEARN_4, START_DO_10, LEARN_DO_1, START_DO_NO, LEARN_DO_NO, LEARN_DIFF_SKILLS, 0.0, 0, 0);

  discArray[SKILL_TAUNT] = new spellInfo(SKILL_WARRIOR, DISC_BRAWLING, DISC_BRAWLING, STAT_CHA, "taunting", TASK_DIFFICULT, LAG_2, POSITION_CRAWLING, MANA_0, LIFEFORCE_0, PRAY_0, 0, SYMBOL_STRESS_0, "", "", "", "", START_55, LEARN_4, START_DO_10, LEARN_DO_1, START_DO_NO, LEARN_DO_NO, LEARN_DIFF_SKILLS, 0.0, 0, 0);

  discArray[SKILL_CLOSE_QUARTERS_FIGHTING] = new spellInfo(SKILL_WARRIOR, DISC_BRAWLING, DISC_BRAWLING, STAT_EXT, "close quarters fighting", TASK_NORMAL, LAG_4, POSITION_CRAWLING, MANA_0, LIFEFORCE_0, PRAY_0, 0, SYMBOL_STRESS_0, "", "", "", "", START_85, LEARN_7, START_DO_10, LEARN_DO_1, START_DO_NO, LEARN_DO_NO, LEARN_DIFF_SKILLS, 0.0, 0, 0);


  discArray[SKILL_BODYSLAM] = new spellInfo(SKILL_WARRIOR, DISC_BRAWLING, DISC_BRAWLING, STAT_STR, "bodyslam", TASK_NORMAL, LAG_4, POSITION_CRAWLING, MANA_0, LIFEFORCE_0, PRAY_0, 0, SYMBOL_STRESS_0, "", "", "", "", START_30, LEARN_4, START_DO_10, LEARN_DO_1, START_DO_NO, LEARN_DO_NO, LEARN_DIFF_SKILLS, 0.0, 0, 0);

  discArray[SKILL_SPIN] = new spellInfo(SKILL_WARRIOR, DISC_BRAWLING, DISC_BRAWLING, STAT_STR, "spin", TASK_NORMAL, LAG_6, POSITION_CRAWLING, MANA_0, LIFEFORCE_0, PRAY_0, 0, SYMBOL_STRESS_0, "", "", "", "", START_45, LEARN_4, START_DO_10, LEARN_DO_1, START_DO_NO, LEARN_DO_NO, LEARN_DIFF_SKILLS, 0.0, 0, 0);
// disc_soldiering

  discArray[SKILL_DOORBASH] = new spellInfo(SKILL_WARRIOR, DISC_SOLDIERING, DISC_SOLDIERING, STAT_EXT, "doorbash", TASK_EASY, LAG_3, POSITION_CRAWLING, MANA_0, LIFEFORCE_0, PRAY_0, 0, SYMBOL_STRESS_0, "You shake the cobwebs from your brain.", "$n shakes $s head and comes back to reality.", "", "", START_1, LEARN_2, START_DO_10, LEARN_DO_1, START_DO_NO, LEARN_DO_NO, LEARN_DIFF_SKILLS, 0.0, 0, 0);

  discArray[SKILL_DEATHSTROKE] = new spellInfo(SKILL_WARRIOR, DISC_SOLDIERING, DISC_SOLDIERING, STAT_STR, "deathstroke", TASK_NORMAL, LAG_5, POSITION_CRAWLING, MANA_0, LIFEFORCE_0, PRAY_0, 0, SYMBOL_STRESS_0, "", "", "", "", START_51, LEARN_2, START_DO_10, LEARN_DO_2, START_DO_NO, LEARN_DO_NO, LEARN_DIFF_SKILLS, 0.0, 0, 0);

  discArray[SKILL_DUAL_WIELD] = new spellInfo(SKILL_WARRIOR, DISC_SOLDIERING, DISC_SOLDIERING, STAT_EXT, "dual wield", TASK_EASY, LAG_0, POSITION_CRAWLING, MANA_0, LIFEFORCE_0, PRAY_0, 0, SYMBOL_STRESS_0, "", "", "", "", START_1, LEARN_5, START_DO_1, LEARN_DO_1, START_DO_NO, LEARN_DO_NO, LEARN_DIFF_UNUSUAL, 0.0, 0, 0);

  discArray[SKILL_POWERMOVE] = new spellInfo(SKILL_WARRIOR, DISC_SOLDIERING, DISC_SOLDIERING, STAT_EXT, "power move", TASK_EASY, LAG_0, POSITION_STANDING, MANA_0, LIFEFORCE_0, PRAY_0, 0, SYMBOL_STRESS_0, "", "", "", "", START_1, LEARN_1, START_DO_1, LEARN_DO_1, START_DO_NO, LEARN_DO_NO, LEARN_DIFF_SKILLS, 0.0, 0, 0);

// disc_blacksmithing

  discArray[SKILL_BLACKSMITHING_ADVANCED] = new spellInfo(SKILL_WARRIOR, DISC_BLACKSMITHING, DISC_BLACKSMITHING, STAT_EXT, "advanced blacksmithing", TASK_NORMAL, LAG_0, POSITION_SITTING, MANA_0, LIFEFORCE_0, PRAY_0, 0, SYMBOL_STRESS_0, "", "", "", "", START_1, LEARN_1, START_DO_1, LEARN_DO_1, START_DO_NO, LEARN_DO_NO, LEARN_DIFF_SKILLS, 0.0, 0, 0);


// no new spells will just let someone  do armor


// RANGER CLASS

// disc_ranger


  discArray[SKILL_BEAST_SOOTHER] = new spellInfo(SPELL_RANGER, DISC_RANGER, DISC_ANIMAL, STAT_EXT, "beast soother", TASK_NORMAL, LAG_3, POSITION_CRAWLING, MANA_0, LIFEFORCE_0, PRAY_0, TAR_SELF_NONO | TAR_CHAR_ROOM, SYMBOL_STRESS_0, "", "", "", "", START_6, LEARN_4, START_DO_40, LEARN_DO_5, START_DO_NO, LEARN_DO_NO, LEARN_DIFF_SPELLS, 0.04, COMP_VERBAL | COMP_MATERIAL, 0);

  discArray[SKILL_BEFRIEND_BEAST] = new spellInfo(SPELL_RANGER, DISC_RANGER, DISC_ANIMAL, STAT_EXT, "befriend beast", TASK_DIFFICULT, LAG_3, POSITION_CRAWLING, MANA_0, LIFEFORCE_0, PRAY_0, TAR_SELF_NONO | TAR_CHAR_ROOM, SYMBOL_STRESS_0, "", "", "", "", START_25, LEARN_3, START_DO_40, LEARN_DO_5, START_DO_NO, LEARN_DO_NO, LEARN_DIFF_SPELLS, 0.02, COMP_VERBAL | COMP_MATERIAL, 0);

  discArray[SKILL_BEAST_SUMMON] = new spellInfo(SPELL_RANGER, DISC_RANGER, DISC_ANIMAL, STAT_EXT, "beast summon", TASK_DANGEROUS, LAG_3, POSITION_SITTING, MANA_0, LIFEFORCE_0, PRAY_0, TAR_NAME, SYMBOL_STRESS_0, "", "", "", "", START_56, LEARN_5, START_DO_40, LEARN_DO_5, START_DO_NO, LEARN_DO_NO, LEARN_DIFF_SPELLS, 0.0, COMP_VERBAL | COMP_MATERIAL, 0);

  discArray[SKILL_BARKSKIN] = new spellInfo(SPELL_RANGER, DISC_RANGER, DISC_PLANTS, STAT_EXT, "barkskin", TASK_EASY, LAG_3, POSITION_CRAWLING, MANA_0, LIFEFORCE_0, PRAY_0, TAR_CHAR_ROOM| TAR_FIGHT_SELF, SYMBOL_STRESS_0, "Your skin loses its barklike qualities.", "$n's skin loses its barklike qualities.", "Your skin seems slightly less like bark.", "$n's skin seems slightly less like wood.", START_1, LEARN_10, START_DO_20, LEARN_DO_5, START_DO_NO, LEARN_DO_NO, LEARN_DIFF_SPELLS, 0.04, COMP_VERBAL | COMP_GESTURAL | COMP_MATERIAL | COMP_MATERIAL_END, TOG_HAS_BARKSKIN);


// disc_fight_ranger

// disc_nature

  discArray[SPELL_TREE_WALK] = new spellInfo(SPELL_RANGER, DISC_NATURE, DISC_NATURE, STAT_EXT, "tree walk", TASK_NORMAL, LAG_3, POSITION_CRAWLING, MANA_0, LIFEFORCE_0, PRAY_0, TAR_NAME, SYMBOL_STRESS_0, "", "", "", "", START_66, LEARN_35, START_DO_40, LEARN_DO_2, START_DO_NO, LEARN_DO_NO, LEARN_DIFF_SPELLS, 0.04, COMP_GESTURAL | COMP_VERBAL | COMP_MATERIAL, 0);

// disc_animal

  discArray[SKILL_BEAST_CHARM] = new spellInfo(SPELL_RANGER, DISC_ANIMAL, DISC_ANIMAL, STAT_EXT, "beast charm", TASK_DIFFICULT, LAG_3, POSITION_CRAWLING, MANA_0, LIFEFORCE_0, PRAY_0, TAR_SELF_NONO | TAR_CHAR_ROOM, SYMBOL_STRESS_0, "", "", "", "", START_1, LEARN_2, START_DO_30, LEARN_DO_5, START_DO_NO, LEARN_DO_NO, LEARN_DIFF_SPELLS, 0.02, COMP_VERBAL | COMP_MATERIAL, 0);

#if 1
  discArray[SPELL_FERAL_WRATH] = new spellInfo(SPELL_RANGER, DISC_ANIMAL, DISC_ANIMAL, STAT_EXT, "feral wrath", TASK_NORMAL, LAG_3, POSITION_SITTING, MANA_0, LIFEFORCE_0, PRAY_0, TAR_CHAR_ROOM | TAR_SELF_ONLY, SYMBOL_STRESS_0, "You lose your feral wrath.", "The feral look in $n's eyes fades completely.", "Your feral wrath begins to fade.", "The feral look in $n's eyes begins to fade.", START_41, LEARN_4, START_DO_40, LEARN_DO_5, START_DO_NO, LEARN_DO_NO, LEARN_DIFF_SPELLS, 0.04, COMP_GESTURAL | COMP_VERBAL | COMP_MATERIAL | COMP_MATERIAL_END, 0);

  discArray[SPELL_SKY_SPIRIT] = new spellInfo(SPELL_RANGER, DISC_ANIMAL, DISC_ANIMAL, STAT_EXT, "sky spirit", TASK_NORMAL, LAG_3, POSITION_SITTING, MANA_10, LIFEFORCE_0, PRAY_0, TAR_CHAR_ROOM | TAR_SELF_NONO | TAR_FIGHT_VICT | TAR_VIOLENT, SYMBOL_STRESS_0, "", "", "", "", START_41, LEARN_4, START_DO_40, LEARN_DO_5, START_DO_NO, LEARN_DO_NO, LEARN_DIFF_SPELLS, 0.04, COMP_GESTURAL | COMP_VERBAL | COMP_MATERIAL, 0);
#endif

//disc_plants

#if 1
#endif

  discArray[SKILL_APPLY_HERBS] = new spellInfo(SKILL_RANGER, DISC_PLANTS, DISC_PLANTS, STAT_EXT, "apply herbs", TASK_NORMAL, LAG_0, POSITION_CRAWLING, MANA_0, LIFEFORCE_0, PRAY_0, 0, SYMBOL_STRESS_0, "", "", "", "", START_1, LEARN_2, START_DO_1, LEARN_DO_1, START_DO_NO, LEARN_DO_NO, LEARN_DIFF_SKILLS, 0.0, 0, 0);


// disc_deikhan

  discArray[SPELL_HEAL_LIGHT_DEIKHAN] = new spellInfo(SPELL_DEIKHAN, DISC_DEIKHAN, DISC_DEIKHAN_ABSOLUTION, STAT_WIS, "heal light", TASK_EASY, LAG_2, POSITION_SITTING, MANA_0, LIFEFORCE_0, PRAY_100, TAR_CHAR_ROOM | TAR_FIGHT_SELF, SYMBOL_STRESS_12, "", "", "", "", START_1, LEARN_10, START_DO_40, LEARN_DO_5, START_DO_NO, LEARN_DO_NO, LEARN_DIFF_PRAYERS, 0.02, COMP_GESTURAL | COMP_VERBAL | SPELL_TASKED | SPELL_TASKED_EVERY, 0);

  discArray[SKILL_CHIVALRY] = new spellInfo(SKILL_DEIKHAN, DISC_DEIKHAN, DISC_MOUNTED, STAT_EXT, "chivalry", TASK_EASY, LAG_0, POSITION_DEAD, MANA_0, LIFEFORCE_0, PRAY_0, 0, SYMBOL_STRESS_0, "", "", "", "", START_11, LEARN_2, START_DO_10, LEARN_DO_1, START_DO_NO, LEARN_DO_NO, LEARN_DIFF_UNUSUAL, 0.0, 0, 0);

  discArray[SPELL_HARM_LIGHT_DEIKHAN] = new spellInfo(SPELL_DEIKHAN, DISC_DEIKHAN, DISC_DEIKHAN_VENGEANCE, STAT_WIS, "harm light", TASK_NORMAL, LAG_2, POSITION_SITTING, MANA_0, LIFEFORCE_0, PRAY_100, TAR_CHAR_ROOM | TAR_SELF_NONO | TAR_FIGHT_VICT | TAR_VIOLENT, SYMBOL_STRESS_5, "", "", "", "", START_1, LEARN_10, START_DO_20, LEARN_DO_2, START_DO_NO, LEARN_DO_NO, LEARN_DIFF_PRAYERS, 0.01, COMP_GESTURAL | COMP_VERBAL, 0);

  discArray[SPELL_ARMOR_DEIKHAN] = new spellInfo(SPELL_DEIKHAN, DISC_DEIKHAN, DISC_DEIKHAN_GUARDIAN, STAT_WIS, "armor", TASK_TRIVIAL, LAG_1, POSITION_SITTING, MANA_0, LIFEFORCE_0, PRAY_025, TAR_CHAR_ROOM | TAR_FIGHT_SELF, SYMBOL_STRESS_15, "The protection around your body fades.", "The protection around $n fades.", "You feel slightly less protected.", "", START_1, LEARN_10, START_DO_30, LEARN_DO_5, START_DO_NO, LEARN_DO_NO, LEARN_DIFF_PRAYERS, 0.2, COMP_GESTURAL | COMP_VERBAL, 0);

  discArray[SPELL_BLESS_DEIKHAN] = new spellInfo(SPELL_DEIKHAN, DISC_DEIKHAN, DISC_DEIKHAN_GUARDIAN, STAT_WIS, "bless", TASK_TRIVIAL, LAG_1, POSITION_SITTING, MANA_0, LIFEFORCE_0, PRAY_050, TAR_OBJ_INV | TAR_OBJ_EQUIP | TAR_CHAR_ROOM |TAR_FIGHT_SELF, SYMBOL_STRESS_10, "You feel less holy.", "$n doesn't seem as holy.", "You feel like your deities are getting bored assisting you.", "", START_1, LEARN_33, START_DO_30, LEARN_DO_4, START_DO_NO, LEARN_DO_NO, LEARN_DIFF_PRAYERS, 0.05, COMP_GESTURAL | COMP_VERBAL, 0);

  discArray[SKILL_BASH_DEIKHAN] = new spellInfo(SKILL_DEIKHAN, DISC_DEIKHAN, DISC_DEIKHAN_MARTIAL, STAT_STR, "bash-deikhan", TASK_EASY, LAG_3, POSITION_CRAWLING, MANA_0, LIFEFORCE_0, PRAY_0, 0, SYMBOL_STRESS_0, "", "", "", "", START_1, LEARN_5, START_DO_1, LEARN_DO_1, START_DO_NO, LEARN_DO_NO, LEARN_DIFF_SKILLS, 0.0, 0, 0);

  discArray[SPELL_EXPEL_DEIKHAN] = new spellInfo(SPELL_DEIKHAN, DISC_DEIKHAN, DISC_DEIKHAN_ABSOLUTION, STAT_EXT, "expel", TASK_EASY, LAG_4, POSITION_SITTING, MANA_0, LIFEFORCE_0, PRAY_500, TAR_CHAR_ROOM, SYMBOL_STRESS_100, "", "", "", "", START_51, LEARN_5, START_DO_50, LEARN_DO_5, START_DO_NO, LEARN_DO_NO, LEARN_DIFF_PRAYERS, 0.04, COMP_GESTURAL | COMP_VERBAL, 0);

  discArray[SPELL_CLOT_DEIKHAN] = new spellInfo(SPELL_DEIKHAN, DISC_DEIKHAN, DISC_DEIKHAN_ABSOLUTION, STAT_EXT, "clot", TASK_EASY, LAG_2, POSITION_SITTING, MANA_0, LIFEFORCE_0, PRAY_100, TAR_CHAR_ROOM, SYMBOL_STRESS_15, "", "", "", "", START_16, LEARN_10, START_DO_20, LEARN_DO_3, START_DO_NO, LEARN_DO_NO, LEARN_DIFF_PRAYERS, 0.04, COMP_GESTURAL | COMP_VERBAL, 0);

  discArray[SPELL_STERILIZE_DEIKHAN] = new spellInfo(SPELL_DEIKHAN, DISC_DEIKHAN, DISC_DEIKHAN_ABSOLUTION, STAT_EXT, "sterilize", TASK_EASY, LAG_3, POSITION_SITTING, MANA_0, LIFEFORCE_0, PRAY_150, TAR_CHAR_ROOM, SYMBOL_STRESS_15, "", "", "", "", START_21, LEARN_5, START_DO_30, LEARN_DO_5, START_DO_NO, LEARN_DO_NO, LEARN_DIFF_PRAYERS, 0.04, COMP_GESTURAL | COMP_VERBAL, 0);

  discArray[SPELL_REMOVE_CURSE_DEIKHAN] = new spellInfo(SPELL_DEIKHAN, DISC_DEIKHAN, DISC_DEIKHAN_ABSOLUTION, STAT_EXT, "remove curse", TASK_EASY, LAG_3, POSITION_SITTING, MANA_0, LIFEFORCE_0, PRAY_100, TAR_CHAR_ROOM | TAR_OBJ_INV | TAR_OBJ_EQUIP | TAR_OBJ_ROOM, SYMBOL_STRESS_25, "", "", "", "", START_27, LEARN_5, START_DO_30, LEARN_DO_5, START_DO_NO, LEARN_DO_NO, LEARN_DIFF_PRAYERS, 0.04, COMP_GESTURAL | COMP_VERBAL, 0);

  discArray[SPELL_CURSE_DEIKHAN] = new spellInfo(SPELL_DEIKHAN, DISC_DEIKHAN, DISC_DEIKHAN_VENGEANCE, STAT_EXT, "curse", TASK_EASY, LAG_3, POSITION_FIGHTING, MANA_0, LIFEFORCE_0, PRAY_050, TAR_CHAR_ROOM | TAR_OBJ_ROOM | TAR_OBJ_INV | TAR_OBJ_EQUIP | TAR_VIOLENT | TAR_FIGHT_VICT, SYMBOL_STRESS_15, "You don't feel damned anymore.", "", "", "", START_51, LEARN_10, START_DO_10, LEARN_DO_2, START_DO_NO, LEARN_DO_NO, LEARN_DIFF_PRAYERS, 0.05, COMP_GESTURAL | COMP_VERBAL , 0);

  discArray[SKILL_RESCUE_DEIKHAN] = new spellInfo(SKILL_DEIKHAN, DISC_DEIKHAN, DISC_DEIKHAN_GUARDIAN, STAT_EXT, "rescue", TASK_EASY, LAG_2, POSITION_CRAWLING, MANA_0, LIFEFORCE_0, PRAY_0, 0, SYMBOL_STRESS_0, "", "", "", "", START_11, LEARN_2, START_DO_10, LEARN_DO_1, START_DO_NO, LEARN_DO_NO, LEARN_DIFF_SKILLS, 0.15, 0, 0);

  discArray[SPELL_INFECT_DEIKHAN] = new spellInfo(SPELL_DEIKHAN, DISC_DEIKHAN, DISC_DEIKHAN_VENGEANCE, STAT_EXT, "infect", TASK_EASY, LAG_3, POSITION_SITTING, MANA_0, LIFEFORCE_0, PRAY_150, TAR_CHAR_ROOM | TAR_FIGHT_VICT | TAR_VIOLENT | TAR_SELF_NONO, SYMBOL_STRESS_40, "", "", "", "", START_63, LEARN_8, START_DO_20, LEARN_DO_5, START_DO_NO, LEARN_DO_NO, LEARN_DIFF_PRAYERS, 0.04, COMP_GESTURAL | COMP_VERBAL, 0);

  discArray[SPELL_CURE_DISEASE_DEIKHAN] = new spellInfo(SPELL_DEIKHAN, DISC_DEIKHAN, DISC_DEIKHAN_ABSOLUTION, STAT_EXT, "cure disease", TASK_EASY, LAG_4, POSITION_SITTING, MANA_0, LIFEFORCE_0, PRAY_450, TAR_CHAR_ROOM | TAR_FIGHT_SELF, SYMBOL_STRESS_50, "", "", "", "", START_30, LEARN_5, START_DO_40, LEARN_DO_5, START_DO_NO, LEARN_DO_NO, LEARN_DIFF_PRAYERS, 0.02, COMP_GESTURAL | COMP_VERBAL, 0);

  discArray[SPELL_CREATE_FOOD_DEIKHAN] = new spellInfo(SPELL_DEIKHAN, DISC_DEIKHAN, DISC_DEIKHAN_ABSOLUTION, STAT_EXT, "create food", TASK_TRIVIAL, LAG_1, POSITION_CRAWLING, MANA_0, LIFEFORCE_0, PRAY_025, TAR_IGNORE, SYMBOL_STRESS_5, "", "", "", "", START_11, LEARN_10, START_DO_40, LEARN_DO_2, START_DO_NO, LEARN_DO_NO, LEARN_DIFF_PRAYERS, 0.0, COMP_GESTURAL | COMP_VERBAL, 0);

  discArray[SPELL_CREATE_WATER_DEIKHAN] = new spellInfo(SPELL_DEIKHAN, DISC_DEIKHAN, DISC_DEIKHAN_ABSOLUTION, STAT_EXT, "create water", TASK_TRIVIAL, LAG_2, POSITION_CRAWLING, MANA_0, LIFEFORCE_0, PRAY_025, TAR_OBJ_INV, SYMBOL_STRESS_5, "", "", "", "", START_11, LEARN_10, START_DO_40, LEARN_DO_5, START_DO_NO, LEARN_DO_NO, LEARN_DIFF_PRAYERS, 0.0, COMP_GESTURAL | COMP_VERBAL | SPELL_TASKED | SPELL_TASKED_EVERY, 0);

  discArray[SPELL_HEAL_SERIOUS_DEIKHAN] = new spellInfo(SPELL_DEIKHAN, DISC_DEIKHAN, DISC_DEIKHAN_ABSOLUTION, STAT_WIS, "heal serious", TASK_EASY, LAG_2, POSITION_SITTING, MANA_0, LIFEFORCE_0, PRAY_150, TAR_CHAR_ROOM | TAR_FIGHT_SELF, SYMBOL_STRESS_18, "", "", "", "", START_41, LEARN_5, START_DO_30, LEARN_DO_2, START_DO_NO, LEARN_DO_NO, LEARN_DIFF_PRAYERS, 0.03, COMP_GESTURAL | COMP_VERBAL |  SPELL_TASKED | SPELL_TASKED_EVERY, 0);

  discArray[SPELL_CURE_POISON_DEIKHAN] = new spellInfo(SPELL_DEIKHAN, DISC_DEIKHAN, DISC_DEIKHAN_ABSOLUTION, STAT_EXT, "cure poison", TASK_EASY, LAG_3, POSITION_SITTING, MANA_0, LIFEFORCE_0, PRAY_100, TAR_CHAR_ROOM | TAR_FIGHT_SELF, SYMBOL_STRESS_35, "", "", "", "", START_11, LEARN_5, START_DO_40, LEARN_DO_5, START_DO_NO, LEARN_DO_NO, LEARN_DIFF_PRAYERS, 0.02, COMP_GESTURAL | COMP_VERBAL, 0);

  discArray[SKILL_CHARGE] = new spellInfo(SKILL_DEIKHAN, DISC_DEIKHAN, DISC_MOUNTED, STAT_CHA, "charge", TASK_NORMAL, LAG_4, POSITION_SITTING, MANA_0, LIFEFORCE_0, PRAY_0, 0, SYMBOL_STRESS_0, "", "", "", "", START_26, LEARN_2, START_DO_1, LEARN_DO_1, START_DO_NO, LEARN_DO_NO, LEARN_DIFF_SKILLS, 0.0, 0, 0);

  discArray[SPELL_HARM_SERIOUS_DEIKHAN] = new spellInfo(SPELL_DEIKHAN, DISC_DEIKHAN, DISC_DEIKHAN_VENGEANCE, STAT_WIS, "harm serious", TASK_NORMAL, LAG_3, POSITION_SITTING, MANA_0, LIFEFORCE_0, PRAY_150, TAR_CHAR_ROOM | TAR_SELF_NONO | TAR_FIGHT_VICT | TAR_VIOLENT , SYMBOL_STRESS_20, "", "", "", "", START_39, LEARN_8, START_DO_20, LEARN_DO_2, START_DO_NO, LEARN_DO_NO, LEARN_DIFF_PRAYERS, 0.02, COMP_GESTURAL | COMP_VERBAL, 0);

  discArray[SPELL_POISON_DEIKHAN] = new spellInfo(SPELL_DEIKHAN, DISC_DEIKHAN, DISC_DEIKHAN_VENGEANCE, STAT_EXT, "poison", TASK_EASY, LAG_3, POSITION_SITTING, MANA_0, LIFEFORCE_0, PRAY_100, TAR_CHAR_ROOM | TAR_OBJ_INV | TAR_FIGHT_VICT | TAR_VIOLENT | TAR_FIGHT_VICT | TAR_SELF_NONO, SYMBOL_STRESS_45, "You feel much healthier.", "The color returns to $n's cheeks.", "", "", START_59, LEARN_10, START_DO_30, LEARN_DO_2, START_DO_NO, LEARN_DO_NO, LEARN_DIFF_PRAYERS, 0.02, COMP_GESTURAL | COMP_VERBAL, 0);

  discArray[SKILL_DISARM_DEIKHAN] = new spellInfo(SKILL_DEIKHAN, DISC_DEIKHAN, DISC_DEIKHAN_MARTIAL, STAT_EXT, "disarm", TASK_EASY, LAG_2, POSITION_CRAWLING, MANA_0, LIFEFORCE_0, PRAY_0, 0, SYMBOL_STRESS_0, "", "", "", "", START_35, LEARN_3, START_DO_10, LEARN_DO_1, START_DO_NO, LEARN_DO_NO, LEARN_DIFF_SKILLS, 0.0, 0, 0);

  discArray[SPELL_HEAL_CRITICAL_DEIKHAN] = new spellInfo(SPELL_DEIKHAN, DISC_DEIKHAN, DISC_DEIKHAN_ABSOLUTION, STAT_WIS, "heal critical", TASK_EASY, LAG_2, POSITION_SITTING, MANA_0, LIFEFORCE_0, PRAY_200, TAR_CHAR_ROOM | TAR_FIGHT_SELF, SYMBOL_STRESS_25, "", "", "", "", START_76, LEARN_5, START_DO_20, LEARN_DO_2, START_DO_NO, LEARN_DO_NO, LEARN_DIFF_PRAYERS, 0.04, COMP_GESTURAL | COMP_VERBAL | SPELL_TASKED | SPELL_TASKED_EVERY, 0);

  discArray[SPELL_HARM_CRITICAL_DEIKHAN] = new spellInfo(SPELL_DEIKHAN, DISC_DEIKHAN, DISC_DEIKHAN_VENGEANCE, STAT_WIS, "harm critical", TASK_NORMAL, LAG_3, POSITION_SITTING, MANA_0, LIFEFORCE_0, PRAY_200, TAR_CHAR_ROOM | TAR_SELF_NONO | TAR_FIGHT_VICT | TAR_VIOLENT , SYMBOL_STRESS_35, "", "", "", "", START_56, LEARN_10, START_DO_20, LEARN_DO_2, START_DO_NO, LEARN_DO_NO, LEARN_DIFF_PRAYERS, 0.03, COMP_GESTURAL | COMP_VERBAL, 0);

  discArray[SKILL_REPAIR_DEIKHAN] = new spellInfo(SKILL_DEIKHAN, DISC_DEIKHAN, DISC_BLACKSMITHING, STAT_EXT, "deikhan repair", TASK_NORMAL, LAG_0, POSITION_SITTING, MANA_0, LIFEFORCE_0, PRAY_0, 0, SYMBOL_STRESS_0, "", "", "", "", START_50, LEARN_3, START_DO_1, LEARN_DO_1, START_DO_NO, LEARN_DO_NO, LEARN_DIFF_SKILLS, 0.0, 0, 0);

  discArray[SKILL_AURA_MIGHT] = new spellInfo(SKILL_DEIKHAN, DISC_DEIKHAN, DISC_DEIKHAN_MARTIAL, STAT_EXT, "aura of might", TASK_NORMAL, LAG_2, POSITION_SITTING, MANA_0, LIFEFORCE_0, PRAY_0, 0, SYMBOL_STRESS_0, "You stop projecting an aura of might.", "", "", "", START_70, LEARN_4, START_DO_NO, LEARN_DO_NO, START_DO_NO, LEARN_DO_NO, LEARN_DIFF_SKILLS, 0.0, 0, 0);

  discArray[SKILL_AURA_REGENERATION] = new spellInfo(SKILL_DEIKHAN, DISC_DEIKHAN, DISC_DEIKHAN_ABSOLUTION, STAT_EXT, "aura of regeneration", TASK_NORMAL, LAG_2, POSITION_SITTING, MANA_0, LIFEFORCE_0, PRAY_0, 0, SYMBOL_STRESS_0, "You stop projecting an aura of regeneration.", "", "", "", START_30, LEARN_4, START_DO_NO, LEARN_DO_NO, START_DO_NO, LEARN_DO_NO, LEARN_DIFF_SKILLS, 0.0, 0, 0);

// disc_martial_deikhan

  discArray[SKILL_SWITCH_DEIKHAN] = new spellInfo(SKILL_DEIKHAN, DISC_DEIKHAN_MARTIAL, DISC_DEIKHAN_MARTIAL, STAT_EXT, "switch opponents", TASK_EASY, LAG_2, POSITION_CRAWLING, MANA_0, LIFEFORCE_0, PRAY_0, 0, SYMBOL_STRESS_0, "", "", "", "", START_21, LEARN_3, START_DO_10, LEARN_DO_1, START_DO_NO, LEARN_DO_NO, LEARN_DIFF_UNUSUAL, 0.0, 0, 0);

  discArray[SKILL_RETREAT_DEIKHAN] = new spellInfo(SKILL_DEIKHAN, DISC_DEIKHAN_MARTIAL, DISC_DEIKHAN_MARTIAL, STAT_EXT, "retreat", TASK_EASY, LAG_0, POSITION_CRAWLING, MANA_0, LIFEFORCE_0, PRAY_0, 0, SYMBOL_STRESS_0, "", "", "", "", START_65, LEARN_3, START_DO_1, LEARN_DO_1, START_DO_NO, LEARN_DO_NO, LEARN_DIFF_UNUSUAL, 0.0, 0, 0);

  discArray[SKILL_SHOVE_DEIKHAN] = new spellInfo(SKILL_DEIKHAN, DISC_DEIKHAN_MARTIAL, DISC_DEIKHAN_MARTIAL, STAT_EXT, "shove", TASK_EASY, LAG_3, POSITION_CRAWLING, MANA_0, LIFEFORCE_0, PRAY_0, 0, SYMBOL_STRESS_0, "", "", "", "", START_33, LEARN_3, START_DO_20, LEARN_DO_1, START_DO_NO, LEARN_DO_NO, LEARN_DIFF_SKILLS, 0.0, 0, 0);

  discArray[SKILL_2H_SPEC_DEIKHAN] = new spellInfo(SKILL_DEIKHAN, DISC_DEIKHAN_MARTIAL, DISC_DEIKHAN_MARTIAL, STAT_EXT, "2h specialization", TASK_NORMAL, LAG_0, POSITION_CRAWLING, MANA_0, LIFEFORCE_0, PRAY_0, 0, SYMBOL_STRESS_0, "", "", "", "", START_1, LEARN_2, START_DO_10, LEARN_DO_2, START_DO_NO, LEARN_DO_NO, LEARN_DIFF_UNUSUAL, 0.0, 0, 0);
  
// disc_mount
  discArray[SKILL_CALM_MOUNT] = new spellInfo(SKILL_DEIKHAN, DISC_MOUNTED, DISC_MOUNTED, STAT_EXT, "calm mount", TASK_NORMAL, LAG_0, POSITION_CRAWLING, MANA_0, LIFEFORCE_0, PRAY_0, 0, SYMBOL_STRESS_0, "", "", "", "", START_1, LEARN_2, START_DO_1, LEARN_DO_1, START_DO_NO, LEARN_DO_NO, LEARN_DIFF_SKILLS, 0.0, 0, 0);

  discArray[SKILL_TRAIN_MOUNT] = new spellInfo(SKILL_DEIKHAN, DISC_MOUNTED, DISC_MOUNTED, STAT_EXT, "train mount", TASK_EASY, LAG_0, POSITION_CRAWLING, MANA_0, LIFEFORCE_0, PRAY_0, 0, SYMBOL_STRESS_0, "", "", "", "", START_26, LEARN_2, START_DO_1, LEARN_DO_1, START_DO_NO, LEARN_DO_NO, LEARN_DIFF_SKILLS, 0.0, 0, 0);

  discArray[SKILL_ADVANCED_RIDING] = new spellInfo(SKILL_DEIKHAN, DISC_MOUNTED, DISC_MOUNTED, STAT_EXT, "advanced riding", TASK_NORMAL, LAG_0, POSITION_CRAWLING, MANA_0, LIFEFORCE_0, PRAY_0, 0, SYMBOL_STRESS_0, "", "", "", "", START_46,LEARN_2, START_DO_1, LEARN_DO_1, START_DO_NO, LEARN_DO_NO, LEARN_DIFF_SKILLS, 0.0, 0, 0);

  discArray[SKILL_RIDE_DOMESTIC] = new spellInfo(SKILL_DEIKHAN, DISC_MOUNTED, DISC_MOUNTED, STAT_EXT, "ride domestic", TASK_TRIVIAL, LAG_0, POSITION_CRAWLING, MANA_0, LIFEFORCE_0, PRAY_0, 0, SYMBOL_STRESS_0, "", "", "", "", START_5, LEARN_2, START_DO_1, LEARN_DO_1, START_DO_NO, LEARN_DO_NO, LEARN_DIFF_SKILLS, 0.0, 0, 0);

  discArray[SKILL_RIDE_NONDOMESTIC] = new spellInfo(SKILL_DEIKHAN, DISC_MOUNTED, DISC_MOUNTED, STAT_EXT, "ride non-domestic", TASK_EASY, LAG_0, POSITION_CRAWLING, MANA_0, LIFEFORCE_0, PRAY_0, 0, SYMBOL_STRESS_0, "", "", "", "", START_36, LEARN_2, START_DO_1, LEARN_DO_1, START_DO_NO, LEARN_DO_NO, LEARN_DIFF_SKILLS, 0.0, 0, 0);

  discArray[SKILL_RIDE_WINGED] = new spellInfo(SKILL_DEIKHAN, DISC_MOUNTED, DISC_MOUNTED, STAT_EXT, "ride winged", TASK_NORMAL, LAG_0, POSITION_CRAWLING, MANA_0, LIFEFORCE_0, PRAY_0, 0, SYMBOL_STRESS_0, "", "", "", "", START_66, LEARN_3, START_DO_1, LEARN_DO_1, START_DO_NO, LEARN_DO_NO, LEARN_DIFF_SKILLS, 0.0, 0, 0);

  discArray[SKILL_RIDE_EXOTIC] = new spellInfo(SKILL_DEIKHAN, DISC_MOUNTED, DISC_MOUNTED, STAT_EXT, "ride exotic", TASK_NORMAL, LAG_0, POSITION_CRAWLING, MANA_0, LIFEFORCE_0, PRAY_0, 0, SYMBOL_STRESS_0, "", "", "", "", START_85, LEARN_7, START_DO_1, LEARN_DO_1, START_DO_NO, LEARN_DO_NO, LEARN_DIFF_SKILLS, 0.0, 0, 0);


// disc_guardian_deikhan

  discArray[SPELL_SYNOSTODWEOMER] = new spellInfo(SPELL_DEIKHAN, DISC_DEIKHAN_GUARDIAN, DISC_DEIKHAN_GUARDIAN, STAT_WIS, "synostodweomer", TASK_EASY,LAG_3, POSITION_CRAWLING, MANA_0, LIFEFORCE_0, PRAY_300, TAR_CHAR_ROOM, SYMBOL_STRESS_65, "Synostodweomer leaves you and your hitpoints return to normal.", "$n's Synostodweomer fades.","","", START_76, LEARN_4, START_DO_30, LEARN_DO_5, START_DO_NO, LEARN_DO_NO, LEARN_DIFF_PRAYERS, 0.0, COMP_GESTURAL | COMP_VERBAL, 0);

  discArray[SKILL_DIVINE_GRACE] = new spellInfo(SKILL_DEIKHAN, DISC_DEIKHAN_GUARDIAN, DISC_DEIKHAN_GUARDIAN, STAT_EXT, "divine grace", TASK_NORMAL,LAG_0, POSITION_DEAD, MANA_0, LIFEFORCE_0, PRAY_0, 0, SYMBOL_STRESS_0, "", "","","", START_1, LEARN_1, START_DO_NO, LEARN_DO_NO, START_DO_NO, LEARN_DO_NO, LEARN_DIFF_SKILLS, 0.0, 0, 0);

  discArray[SKILL_DIVINE_RESCUE] = new spellInfo(SKILL_DEIKHAN, DISC_DEIKHAN_GUARDIAN, DISC_DEIKHAN_GUARDIAN, STAT_EXT, "divine rescue", TASK_NORMAL,LAG_0, POSITION_DEAD, MANA_0, LIFEFORCE_0, PRAY_0, 0, SYMBOL_STRESS_0, "", "","","", START_40, LEARN_5, START_DO_NO, LEARN_DO_NO, START_DO_NO, LEARN_DO_NO, LEARN_DIFF_SKILLS, 0.0, 0, 0);

  discArray[SKILL_GUARDIANS_LIGHT] = new spellInfo(SKILL_DEIKHAN, DISC_DEIKHAN_GUARDIAN, DISC_DEIKHAN_GUARDIAN, STAT_EXT, "guardians light", TASK_NORMAL,LAG_0, POSITION_DEAD, MANA_0, LIFEFORCE_0, PRAY_0, 0, SYMBOL_STRESS_0, "", "","","", START_60, LEARN_5, START_DO_NO, LEARN_DO_NO, START_DO_NO, LEARN_DO_NO, LEARN_DIFF_SKILLS, 0.0, 0, 0);

  discArray[SKILL_AURA_GUARDIAN] = new spellInfo(SKILL_DEIKHAN, DISC_DEIKHAN_GUARDIAN, DISC_DEIKHAN_GUARDIAN, STAT_EXT, "aura of the guardian", TASK_NORMAL, LAG_2, POSITION_SITTING, MANA_0, LIFEFORCE_0, PRAY_0, 0, SYMBOL_STRESS_0, "You stop projecting an aura of the guardian.", "", "", "", START_30, LEARN_4, START_DO_NO, LEARN_DO_NO, START_DO_NO, LEARN_DO_NO, LEARN_DIFF_SKILLS, 0.0, 0, 0);

// disc_absolution_deikhan

  discArray[SPELL_REFRESH_DEIKHAN] = new spellInfo(SPELL_DEIKHAN, DISC_DEIKHAN_ABSOLUTION, DISC_DEIKHAN_ABSOLUTION, STAT_EXT, "refresh", TASK_EASY, LAG_2, POSITION_SITTING, MANA_0, LIFEFORCE_0, PRAY_100, TAR_CHAR_ROOM | TAR_FIGHT_SELF, SYMBOL_STRESS_20, "", "", "", "", START_1, LEARN_5, START_DO_20, LEARN_DO_2, START_DO_NO, LEARN_DO_NO, LEARN_DIFF_PRAYERS, 0.02, COMP_GESTURAL | COMP_VERBAL, 0);

  discArray[SPELL_SALVE_DEIKHAN] = new spellInfo(SPELL_DEIKHAN, DISC_DEIKHAN_ABSOLUTION, DISC_DEIKHAN_ABSOLUTION, STAT_EXT, "salve", TASK_TRIVIAL, LAG_3, POSITION_SITTING, MANA_0, LIFEFORCE_0, PRAY_200, TAR_CHAR_ROOM, SYMBOL_STRESS_10, "", "", "", "", START_11, LEARN_5, START_DO_20, LEARN_DO_2, START_DO_NO, LEARN_DO_NO, LEARN_DIFF_PRAYERS, 0.02, COMP_GESTURAL | COMP_VERBAL, 0);

  discArray[SPELL_HEROES_FEAST_DEIKHAN] = new spellInfo(SPELL_DEIKHAN, DISC_DEIKHAN_ABSOLUTION, DISC_DEIKHAN_ABSOLUTION, STAT_EXT, "heroes feast", TASK_TRIVIAL, LAG_3, POSITION_CRAWLING, MANA_0, LIFEFORCE_0, PRAY_075, TAR_AREA | TAR_IGNORE, SYMBOL_STRESS_15, "", "", "", "", START_31, LEARN_4, START_DO_20, LEARN_DO_3, START_DO_NO, LEARN_DO_NO, LEARN_DIFF_PRAYERS, 0.1, COMP_GESTURAL | COMP_VERBAL, 0);

  discArray[SKILL_LAY_HANDS] = new spellInfo(SKILL_DEIKHAN, DISC_DEIKHAN_ABSOLUTION, DISC_DEIKHAN_ABSOLUTION, STAT_EXT, "lay hands", TASK_EASY, LAG_1, POSITION_STANDING, MANA_0, LIFEFORCE_0, PRAY_0, 0, SYMBOL_STRESS_0, "", "", "", "", START_51, LEARN_4, START_DO_35, LEARN_DO_2, START_DO_NO, LEARN_DO_NO, LEARN_DIFF_SKILLS, 0.4, 0, 0);

  discArray[SKILL_AURA_ABSOLUTION] = new spellInfo(SKILL_DEIKHAN, DISC_DEIKHAN_ABSOLUTION, DISC_DEIKHAN_ABSOLUTION, STAT_EXT, "aura of absolution", TASK_NORMAL, LAG_2, POSITION_SITTING, MANA_0, LIFEFORCE_0, PRAY_0, 0, SYMBOL_STRESS_0, "You stop projecting an aura of absolution.", "", "", "", START_40, LEARN_4, START_DO_NO, LEARN_DO_NO, START_DO_NO, LEARN_DO_NO, LEARN_DIFF_SKILLS, 0.0, 0, 0);

// disc_vengeance_deikhan

  discArray[SPELL_HARM_DEIKHAN] = new spellInfo(SPELL_DEIKHAN, DISC_DEIKHAN_VENGEANCE, DISC_DEIKHAN_VENGEANCE, STAT_WIS, "harm", TASK_NORMAL, LAG_3, POSITION_SITTING, MANA_0, LIFEFORCE_0, PRAY_250, TAR_CHAR_ROOM | TAR_FIGHT_VICT | TAR_SELF_NONO | TAR_VIOLENT , SYMBOL_STRESS_50, "", "", "", "", START_41, LEARN_4, START_DO_20, LEARN_DO_2, START_DO_NO, LEARN_DO_NO, LEARN_DIFF_PRAYERS, 0.06, COMP_GESTURAL | COMP_VERBAL, 0);

  discArray[SKILL_SMITE] = new spellInfo(SKILL_DEIKHAN, DISC_DEIKHAN_VENGEANCE, DISC_DEIKHAN_VENGEANCE, STAT_WIS, "smite", TASK_NORMAL, LAG_3, POSITION_STANDING, MANA_0, LIFEFORCE_0, PRAY_0, 0, SYMBOL_STRESS_0, "", "", "", "", START_51, LEARN_4, START_DO_50, LEARN_DO_3, START_DO_NO, LEARN_DO_NO, LEARN_DIFF_SKILLS, 0.0, 0, 0);

  discArray[SPELL_NUMB_DEIKHAN] = new spellInfo(SPELL_DEIKHAN, DISC_DEIKHAN_VENGEANCE, DISC_DEIKHAN_VENGEANCE, STAT_WIS, "numb", TASK_EASY, LAG_2, POSITION_SITTING, MANA_0, LIFEFORCE_0, PRAY_100, TAR_CHAR_ROOM | TAR_FIGHT_VICT | TAR_VIOLENT | TAR_SELF_NONO, SYMBOL_STRESS_30, "", "", "", "", START_1, LEARN_4, START_DO_40, LEARN_DO_5, START_DO_NO, LEARN_DO_NO, LEARN_DIFF_PRAYERS, 0.05, COMP_GESTURAL | COMP_VERBAL, 0);

  discArray[SKILL_AURA_VENGEANCE] = new spellInfo(SKILL_DEIKHAN, DISC_DEIKHAN_VENGEANCE, DISC_DEIKHAN_VENGEANCE, STAT_EXT, "aura of vengeance", TASK_NORMAL, LAG_2, POSITION_SITTING, MANA_0, LIFEFORCE_0, PRAY_0, 0, SYMBOL_STRESS_0, "You stop projecting an aura of vengeance.", "", "", "", START_10, LEARN_4, START_DO_NO, LEARN_DO_NO, START_DO_NO, LEARN_DO_NO, LEARN_DIFF_SKILLS, 0.0, 0, 0);

// CLASS MONK

// disc_monk

  discArray[SKILL_YOGINSA] = new spellInfo(SKILL_MONK, DISC_MONK, DISC_MEDITATION_MONK, STAT_EXT, "yoginsa", TASK_TRIVIAL, LAG_0, POSITION_DEAD, MANA_0, LIFEFORCE_0, PRAY_0, 0, SYMBOL_STRESS_0, "", "", "", "", START_1, LEARN_1, START_DO_1, LEARN_DO_1, START_DO_NO, LEARN_DO_NO, LEARN_DIFF_SKILLS, 0.0, 0, 0);

  discArray[SKILL_CINTAI] = new spellInfo(SKILL_MONK, DISC_MONK, DISC_FOCUSED_ATTACKS, STAT_EXT, "cintai", TASK_EASY, LAG_0, POSITION_DEAD, MANA_0, LIFEFORCE_0, PRAY_0, 0, SYMBOL_STRESS_0, "", "", "", "", START_5, LEARN_2, START_DO_1, LEARN_DO_1, START_DO_NO, LEARN_DO_NO, LEARN_DIFF_SKILLS, 0.0, 0, 0);

  discArray[SKILL_OOMLAT] = new spellInfo(SKILL_MONK, DISC_MONK, DISC_MINDBODY, STAT_EXT, "Oomlat Philosophy", TASK_EASY, LAG_0, POSITION_DEAD, MANA_0, LIFEFORCE_0, PRAY_0, 0, SYMBOL_STRESS_0, "", "", "", "", START_1, LEARN_1, START_DO_1, LEARN_DO_1, START_DO_NO, LEARN_DO_NO, LEARN_DIFF_SKILLS, 0.0, 0, 0);

  discArray[SKILL_KICK_MONK] = new spellInfo(SKILL_MONK, DISC_MONK, DISC_LEVERAGE, STAT_STR, "kick", TASK_EASY, LAG_3, POSITION_STANDING, MANA_0, LIFEFORCE_0, PRAY_0, 0, SYMBOL_STRESS_0, "", "", "", "", START_5, LEARN_5, START_DO_1, LEARN_DO_1, START_DO_NO, LEARN_DO_NO, LEARN_DIFF_SKILLS, 0.0, 0, 0);

  discArray[SKILL_ADVANCED_KICKING] = new spellInfo(SKILL_MONK, DISC_MONK, DISC_LEVERAGE, STAT_EXT, "advanced kicking", TASK_EASY, LAG_0, POSITION_DEAD, MANA_0, LIFEFORCE_0, PRAY_0, 0, SYMBOL_STRESS_0, "", "", "", "", START_25, LEARN_2, START_DO_10, LEARN_DO_1, START_DO_NO, LEARN_DO_NO, LEARN_DIFF_SKILLS, 0.0, 0, TOG_HAS_ADVANCED_KICKING);

  discArray[SKILL_GROUNDFIGHTING] = new spellInfo(SKILL_MONK, DISC_MONK, DISC_LEVERAGE, STAT_EXT, "groundfighting", TASK_NORMAL, LAG_0, POSITION_DEAD, MANA_0, LIFEFORCE_0, PRAY_0, 0, SYMBOL_STRESS_0, "", "", "", "", START_10, LEARN_3, START_DO_1, LEARN_DO_1, START_DO_NO, LEARN_DO_NO, LEARN_DIFF_SKILLS, 0.0, 0, 0);

  discArray[SKILL_SPRINGLEAP] = new spellInfo(SKILL_MONK, DISC_MONK, DISC_LEVERAGE, STAT_DEX, "springleap", TASK_NORMAL, LAG_2, POSITION_RESTING, MANA_0, LIFEFORCE_0, PRAY_0, 0, SYMBOL_STRESS_0, "", "", "", "", START_20, LEARN_3, START_DO_1, LEARN_DO_3, START_DO_NO, LEARN_DO_NO, LEARN_DIFF_SKILLS, 0.0, 0, 0);

  discArray[SKILL_RETREAT_MONK] = new spellInfo(SKILL_MONK, DISC_MONK, DISC_MONK, STAT_EXT, "retreat", TASK_EASY, LAG_0, POSITION_CRAWLING, MANA_0, LIFEFORCE_0, PRAY_0, 0, SYMBOL_STRESS_0, "", "", "", "", START_30, LEARN_2, START_DO_1, LEARN_DO_1, START_DO_NO, LEARN_DO_NO, LEARN_DIFF_UNUSUAL, 0.0, 0, 0);

  discArray[SKILL_SNOFALTE] = new spellInfo(SKILL_MONK, DISC_MONK, DISC_MEDITATION_MONK, STAT_EXT, "snofalte", TASK_EASY, LAG_0, POSITION_DEAD, MANA_0, LIFEFORCE_0, PRAY_0, 0, SYMBOL_STRESS_0, "", "", "", "", START_40, LEARN_2, START_DO_1, LEARN_DO_1, START_DO_NO, LEARN_DO_NO, LEARN_DIFF_SKILLS, 0.0, 0, 0);

  discArray[SKILL_COUNTER_MOVE] = new spellInfo(SKILL_MONK, DISC_MONK, DISC_LEVERAGE, STAT_EXT, "counter move", TASK_EASY, LAG_1, POSITION_RESTING, MANA_0, LIFEFORCE_0, PRAY_0, 0, SYMBOL_STRESS_0, "", "", "", "", START_50, LEARN_2, START_DO_1, LEARN_DO_1, START_DO_NO, LEARN_DO_NO, LEARN_DIFF_SKILLS, 0.0, 0, 0);

  discArray[SKILL_SWITCH_MONK] = new spellInfo(SKILL_MONK, DISC_MONK, DISC_LEVERAGE, STAT_EXT, "switch opponents", TASK_EASY, LAG_2, POSITION_CRAWLING, MANA_0, LIFEFORCE_0, PRAY_0, 0, SYMBOL_STRESS_0, "", "", "", "", START_50, LEARN_100, START_DO_1, LEARN_DO_2, START_DO_NO, LEARN_DO_NO, LEARN_DIFF_UNUSUAL, 0.0, 0, 0);
 
  discArray[SKILL_JIRIN] = new spellInfo(SKILL_MONK, DISC_MONK, DISC_MINDBODY, STAT_EXT, "jirin", TASK_NORMAL, LAG_0, POSITION_DEAD, MANA_0, LIFEFORCE_0, PRAY_0, 0, SYMBOL_STRESS_0, "", "", "", "", START_1, LEARN_1, START_DO_1, LEARN_DO_1, START_DO_NO, LEARN_DO_NO, LEARN_DIFF_SKILLS, 0.0, 0, 0);

  discArray[SKILL_KUBO] = new spellInfo(SKILL_MONK, DISC_MONK, DISC_MEDITATION_MONK, STAT_EXT, "kubo", TASK_NORMAL, LAG_0, POSITION_DEAD, MANA_0, LIFEFORCE_0, PRAY_0, 0, SYMBOL_STRESS_0, "", "", "", "", START_1, LEARN_1, START_DO_1, LEARN_DO_1, START_DO_NO, LEARN_DO_NO, LEARN_DIFF_SKILLS, 0.0, 0, 0);

  discArray[SKILL_DUFALI] = new spellInfo(SKILL_MONK, DISC_MONK, DISC_MEDITATION_MONK, STAT_EXT, "dufali", TASK_NORMAL, LAG_0, POSITION_DEAD, MANA_0, LIFEFORCE_0, PRAY_0, 0, SYMBOL_STRESS_0, "", "", "", "", START_70, LEARN_4, START_DO_NO, LEARN_DO_NO, START_DO_NO, LEARN_DO_NO, LEARN_DIFF_SKILLS, 0.0, 0, 0);

  discArray[SKILL_CHOP] = new spellInfo(SKILL_MONK, DISC_MONK, DISC_FOCUSED_ATTACKS, STAT_STR, "chop", TASK_NORMAL, LAG_4, POSITION_STANDING, MANA_0, LIFEFORCE_0, PRAY_0, 0, SYMBOL_STRESS_0, "", "", "", "", START_51, LEARN_2, START_DO_1, LEARN_DO_2, START_DO_NO, LEARN_DO_NO, LEARN_DIFF_SKILLS, 0.0, 0, 0);

  discArray[SKILL_DISARM_MONK] = new spellInfo(SKILL_MONK, DISC_MONK, DISC_LEVERAGE, STAT_EXT, "disarm", TASK_EASY, LAG_2, POSITION_CRAWLING, MANA_0, LIFEFORCE_0, PRAY_0, 0, SYMBOL_STRESS_0, "", "", "", "", START_1, LEARN_2, START_DO_1, LEARN_DO_4, START_DO_NO, LEARN_DO_NO, LEARN_DIFF_SKILLS, 0.0, 0, 0);

  discArray[SKILL_CHI] = new spellInfo(SKILL_MONK, DISC_MONK, DISC_MEDITATION_MONK, STAT_INT, "chi", TASK_NORMAL, LAG_4, POSITION_CRAWLING, MANA_0, LIFEFORCE_0, PRAY_0, 0, SYMBOL_STRESS_0, "You cease to radiate your chi.", "", "You feel your chi running low.", "", START_1, LEARN_1, START_DO_1, LEARN_DO_1, START_DO_NO, LEARN_DO_NO, LEARN_DIFF_SKILLS, 0.0, 0, 0);

  discArray[SKILL_CATFALL] = new spellInfo(SKILL_MONK, DISC_MONK, DISC_MINDBODY, STAT_EXT, "catfall", TASK_NORMAL, LAG_0,POSITION_DEAD, MANA_0, LIFEFORCE_0, PRAY_0, 0, SYMBOL_STRESS_0, "", "", "", "", START_1, LEARN_2, START_DO_1, LEARN_DO_5, START_DO_NO, LEARN_DO_NO, LEARN_DIFF_SKILLS, 0.0, 0, TOG_HAS_CATFALL);

  discArray[SKILL_REPAIR_MONK] = new spellInfo(SKILL_MONK, DISC_MONK, DISC_BLACKSMITHING, STAT_EXT, "monk repair", TASK_NORMAL, LAG_0, POSITION_SITTING, MANA_0, LIFEFORCE_0, PRAY_0, 0, SYMBOL_STRESS_0, "", "", "", "", START_50, LEARN_3, START_DO_1, LEARN_DO_1, START_DO_NO, LEARN_DO_NO, LEARN_DIFF_SKILLS, 0.0, 0, 0);

  discArray[SKILL_CATLEAP] = new spellInfo(SKILL_MONK, DISC_MONK, DISC_MINDBODY, STAT_EXT, "catleap", TASK_NORMAL, LAG_0, POSITION_STANDING, MANA_0, LIFEFORCE_0, PRAY_0, 0, SYMBOL_STRESS_0, "", "", "", "", START_1, LEARN_2, START_DO_1, LEARN_DO_5, START_DO_NO, LEARN_DO_NO, LEARN_DIFF_SKILLS, 0.0, 0, 0);

// disc_chidoki

  discArray[SKILL_WOHLIN] = new spellInfo(SKILL_MONK, DISC_MEDITATION_MONK, DISC_MEDITATION_MONK, STAT_EXT, "wohlin meditation", TASK_EASY, LAG_0, POSITION_DEAD, MANA_0, LIFEFORCE_0, PRAY_0, 0, SYMBOL_STRESS_0, "", "", "", "", START_1, LEARN_1, START_DO_1, LEARN_DO_1, START_DO_NO, LEARN_DO_NO, LEARN_DIFF_SKILLS, 0.0, 0, 0);

  discArray[SKILL_VOPLAT] = new spellInfo(SKILL_MONK, DISC_MEDITATION_MONK, DISC_MEDITATION_MONK, STAT_EXT, "voplat", TASK_NORMAL, LAG_0, POSITION_DEAD, MANA_0, LIFEFORCE_0, PRAY_0, 0, SYMBOL_STRESS_0, "", "", "", "", START_1, LEARN_1, START_DO_NO, LEARN_DO_NO, START_DO_NO, LEARN_DO_NO, LEARN_DIFF_SKILLS, 0.0, 0, 0);

  discArray[SKILL_BLINDFIGHTING] = new spellInfo(SKILL_MONK, DISC_MEDITATION_MONK, DISC_MEDITATION_MONK, STAT_EXT, "blindfighting", TASK_NORMAL, LAG_0, POSITION_DEAD, MANA_0, LIFEFORCE_0, PRAY_0, 0, SYMBOL_STRESS_0, "", "", "", "", START_1, LEARN_1, START_DO_NO, LEARN_DO_NO, START_DO_NO, LEARN_DO_NO, LEARN_DIFF_SKILLS, 0.0, 0, 0);

// disc_karoki


  discArray[SKILL_BONEBREAK] = new spellInfo(SKILL_MONK, DISC_LEVERAGE, DISC_LEVERAGE, STAT_STR, "bonebreak", TASK_EASY, LAG_4, POSITION_CRAWLING, MANA_0, LIFEFORCE_0, PRAY_0, 0, SYMBOL_STRESS_0, "", "", "", "", START_75, LEARN_4, START_DO_1, LEARN_DO_10, START_DO_NO, LEARN_DO_NO, LEARN_DIFF_SKILLS, 0.0, 0, 0);

  discArray[SKILL_DEFENESTRATE] = new spellInfo(SKILL_MONK, DISC_LEVERAGE, DISC_LEVERAGE, STAT_STR, "defenestrate", TASK_EASY, LAG_4, POSITION_CRAWLING, MANA_0, LIFEFORCE_0, PRAY_0, 0, SYMBOL_STRESS_0, "", "", "", "", START_50, LEARN_2, START_DO_1, LEARN_DO_5, START_DO_NO, LEARN_DO_NO, LEARN_DIFF_SKILLS, 0.0, 0, 0);

  discArray[SKILL_HURL] = new spellInfo(SKILL_MONK, DISC_LEVERAGE, DISC_LEVERAGE, STAT_STR, "hurl", TASK_NORMAL, LAG_4, POSITION_CRAWLING, MANA_0, LIFEFORCE_0, PRAY_0, 0, SYMBOL_STRESS_0, "", "", "", "", START_1, LEARN_2, START_DO_1, LEARN_DO_2, START_DO_NO, LEARN_DO_NO, LEARN_DIFF_SKILLS, 0.0, 0, 0);

  discArray[SKILL_SHOULDER_THROW] = new spellInfo(SKILL_MONK, DISC_LEVERAGE, DISC_LEVERAGE, STAT_STR, "shoulder throw", TASK_NORMAL, LAG_4, POSITION_CRAWLING, MANA_0, LIFEFORCE_0, PRAY_0, 0, SYMBOL_STRESS_0, "", "", "", "", START_35, LEARN_2, START_DO_1, LEARN_DO_2, START_DO_NO, LEARN_DO_NO, LEARN_DIFF_SKILLS, 0.0, 0, 0);

  discArray[SKILL_CHAIN_ATTACK] = new spellInfo(SKILL_MONK, DISC_LEVERAGE, DISC_LEVERAGE, STAT_EXT, "chain attack", TASK_TRIVIAL, LAG_0, POSITION_STANDING, MANA_0, LIFEFORCE_0, PRAY_0, 0, SYMBOL_STRESS_0, "", "", "", "", START_1, LEARN_1, START_DO_1, LEARN_DO_1, START_DO_NO, LEARN_DO_NO, LEARN_DIFF_SKILLS, 0.0, 0, 0);

// disc_taedoki

  discArray[SKILL_FEIGN_DEATH] = new spellInfo(SKILL_MONK, DISC_MINDBODY, DISC_MINDBODY, STAT_EXT, "feign death", TASK_EASY, LAG_2, POSITION_CRAWLING, MANA_0, LIFEFORCE_0, PRAY_0, 0, SYMBOL_STRESS_0, "", "", "", "", START_1, LEARN_1, START_DO_10, LEARN_DO_5, START_DO_NO, LEARN_DO_NO, LEARN_DIFF_SKILLS, 0.0, 0, 0);

  discArray[SKILL_BLUR] = new spellInfo(SKILL_MONK, DISC_MINDBODY, DISC_MINDBODY, STAT_EXT, "blur", TASK_TRIVIAL, LAG_0, POSITION_STANDING, MANA_0, LIFEFORCE_0, PRAY_0, 0, SYMBOL_STRESS_0, "", "", "", "", START_1, LEARN_1, START_DO_1, LEARN_DO_1, START_DO_NO, LEARN_DO_NO, LEARN_DIFF_SKILLS, 0.0, 0, 0);


// disc_fattacks

  discArray[SKILL_QUIV_PALM] = new spellInfo(SKILL_MONK, DISC_FOCUSED_ATTACKS, DISC_MEDITATION_MONK, STAT_EXT, "quivering palm", TASK_NORMAL, LAG_4, POSITION_STANDING, MANA_0, LIFEFORCE_0, PRAY_0, 0, SYMBOL_STRESS_0, "", "", "", "", START_51, LEARN_8, START_DO_1, LEARN_DO_2, START_DO_NO, LEARN_DO_NO, LEARN_DIFF_SKILLS, 0.0, 0, 0);

  discArray[SKILL_CRIT_HIT] = new spellInfo(SKILL_MONK, DISC_FOCUSED_ATTACKS, DISC_FOCUSED_ATTACKS, STAT_EXT, "critical hitting", TASK_TRIVIAL, LAG_0, POSITION_STANDING, MANA_0, LIFEFORCE_0, PRAY_0, 0, SYMBOL_STRESS_0, "", "", "", "", START_1, LEARN_1, START_DO_1, LEARN_DO_1, START_DO_NO, LEARN_DO_NO, LEARN_DIFF_SKILLS, 0.0, 0, 0);

// disc iron body
  discArray[SKILL_IRON_FIST] = new spellInfo(SKILL_MONK, DISC_IRON_BODY, DISC_IRON_BODY, STAT_EXT, "iron fist", TASK_TRIVIAL, LAG_0, POSITION_STANDING, MANA_0, LIFEFORCE_0, PRAY_0, 0, SYMBOL_STRESS_0, "", "", "", "", START_1, LEARN_1, START_DO_1, LEARN_DO_1, START_DO_NO, LEARN_DO_NO, LEARN_DIFF_SKILLS, 0.0, 0, 0);

  discArray[SKILL_IRON_FLESH] = new spellInfo(SKILL_MONK, DISC_IRON_BODY, DISC_IRON_BODY, STAT_EXT, "iron flesh", TASK_NORMAL, LAG_0, POSITION_STANDING, MANA_0, LIFEFORCE_0, PRAY_0, 0, SYMBOL_STRESS_0, "", "", "", "", START_20, LEARN_2, START_DO_NO, LEARN_DO_NO, START_DO_NO, LEARN_DO_NO, LEARN_DIFF_SKILLS, 0.0, 0, 0);

  discArray[SKILL_IRON_SKIN] = new spellInfo(SKILL_MONK, DISC_IRON_BODY, DISC_IRON_BODY, STAT_EXT, "iron skin", TASK_NORMAL, LAG_0, POSITION_STANDING, MANA_0, LIFEFORCE_0, PRAY_0, 0, SYMBOL_STRESS_0, "", "", "", "", START_30, LEARN_2, START_DO_NO, LEARN_DO_NO, START_DO_NO, LEARN_DO_NO, LEARN_DIFF_SKILLS, 0.0, 0, 0);

  discArray[SKILL_IRON_BONES] = new spellInfo(SKILL_MONK, DISC_IRON_BODY, DISC_IRON_BODY, STAT_EXT, "iron bones", TASK_NORMAL, LAG_0, POSITION_STANDING, MANA_0, LIFEFORCE_0, PRAY_0, 0, SYMBOL_STRESS_0, "", "", "", "", START_40, LEARN_2, START_DO_NO, LEARN_DO_NO, START_DO_NO, LEARN_DO_NO, LEARN_DIFF_SKILLS, 0.0, 0, 0);

  discArray[SKILL_IRON_MUSCLES] = new spellInfo(SKILL_MONK, DISC_IRON_BODY, DISC_IRON_BODY, STAT_EXT, "iron muscles", TASK_NORMAL, LAG_0, POSITION_STANDING, MANA_0, LIFEFORCE_0, PRAY_0, 0, SYMBOL_STRESS_0, "", "", "", "", START_50, LEARN_2, START_DO_NO, LEARN_DO_NO, START_DO_NO, LEARN_DO_NO, LEARN_DIFF_SKILLS, 0.0, 0, 0);

  discArray[SKILL_IRON_LEGS] = new spellInfo(SKILL_MONK, DISC_IRON_BODY, DISC_IRON_BODY, STAT_EXT, "iron legs", TASK_NORMAL, LAG_0, POSITION_STANDING, MANA_0, LIFEFORCE_0, PRAY_0, 0, SYMBOL_STRESS_0, "", "", "", "", START_60, LEARN_3, START_DO_NO, LEARN_DO_NO, START_DO_NO, LEARN_DO_NO, LEARN_DIFF_SKILLS, 0.0, 0, 0);

  discArray[SKILL_IRON_WILL] = new spellInfo(SKILL_MONK, DISC_IRON_BODY, DISC_IRON_BODY, STAT_EXT, "iron will", TASK_NORMAL, LAG_0, POSITION_STANDING, MANA_0, LIFEFORCE_0, PRAY_0, 0, SYMBOL_STRESS_0, "", "", "", "", START_70, LEARN_4, START_DO_NO, LEARN_DO_NO, START_DO_NO, LEARN_DO_NO, LEARN_DIFF_SKILLS, 0.0, 0, 0);

// CLASS THIEF
// disc_thief

  discArray[SKILL_TRACK] = new spellInfo(SKILL_THIEF, DISC_THIEF, DISC_STEALTH, STAT_EXT, "track", TASK_EASY, LAG_0, POSITION_CRAWLING, MANA_0, LIFEFORCE_0, PRAY_0, 0, SYMBOL_STRESS_0, "", "", "", "", START_27, LEARN_3, START_DO_1, LEARN_DO_1, START_DO_NO, LEARN_DO_NO, LEARN_DIFF_SKILLS, 0.0, 0, 0);

  discArray[SKILL_SWINDLE] = new spellInfo(SKILL_THIEF, DISC_THIEF, DISC_LOOTING, STAT_EXT, "swindle", TASK_EASY, LAG_0, POSITION_CRAWLING, MANA_0, LIFEFORCE_0, PRAY_0, 0, SYMBOL_STRESS_0, "", "", "", "", START_1, LEARN_1, START_DO_1, LEARN_DO_1, START_DO_NO, LEARN_DO_NO, LEARN_DIFF_UNUSUAL, 0.0, 0, 0);

  discArray[SKILL_SNEAK] = new spellInfo(SKILL_THIEF, DISC_THIEF, DISC_STEALTH, STAT_EXT, "sneak", TASK_EASY, LAG_1, POSITION_CRAWLING, MANA_0, LIFEFORCE_0, PRAY_0, 0, SYMBOL_STRESS_0, "", "", "", "", START_1, LEARN_5, START_DO_1, LEARN_DO_1, START_DO_NO, LEARN_DO_NO, LEARN_DIFF_SKILLS, 0.0, 0, 0);

  discArray[SKILL_STABBING] = new spellInfo(SKILL_THIEF, DISC_THIEF, DISC_MURDER, STAT_STR, "stab", TASK_EASY, LAG_3, POSITION_CRAWLING, MANA_0, LIFEFORCE_0, PRAY_0, 0, SYMBOL_STRESS_0, "", "", "", "", START_33, LEARN_3, START_DO_1, LEARN_DO_2, START_DO_NO, LEARN_DO_NO, LEARN_DIFF_SKILLS,0.0, 0, 0);

  discArray[SKILL_RETREAT_THIEF] = new spellInfo(SKILL_THIEF, DISC_THIEF, DISC_THIEF_FIGHT, STAT_EXT, "retreat", TASK_NORMAL, LAG_0, POSITION_CRAWLING, MANA_0, LIFEFORCE_0, PRAY_0, 0, SYMBOL_STRESS_0, "", "", "", "", START_1, LEARN_1,START_DO_1, LEARN_DO_2, START_DO_NO, LEARN_DO_NO, LEARN_DIFF_UNUSUAL, 0.0, 0, 0);

  discArray[SKILL_KICK_THIEF] = new spellInfo(SKILL_THIEF, DISC_THIEF, DISC_THIEF_FIGHT, STAT_STR, "kick", TASK_EASY, LAG_3, POSITION_STANDING, MANA_0, LIFEFORCE_0, PRAY_0, 0, SYMBOL_STRESS_0, "", "", "", "", START_1, LEARN_5, START_DO_1, LEARN_DO_1, START_DO_NO, LEARN_DO_NO, LEARN_DIFF_SKILLS, 0.0, 0, 0);

  discArray[SKILL_PICK_LOCK] = new spellInfo(SKILL_THIEF, DISC_THIEF, DISC_TRAPS, STAT_EXT, "picklock", TASK_NORMAL, LAG_0, POSITION_CRAWLING, MANA_0, LIFEFORCE_0, PRAY_0, 0, SYMBOL_STRESS_0, "", "", "", "", START_31, LEARN_5, START_DO_1, LEARN_DO_5, START_DO_NO, LEARN_DO_NO, LEARN_DIFF_UNUSUAL,0.0, 0, 0);

  discArray[SKILL_BACKSTAB] = new spellInfo(SKILL_THIEF, DISC_THIEF, DISC_MURDER, STAT_DEX, "backstab", TASK_NORMAL, LAG_5, POSITION_CRAWLING, MANA_0, LIFEFORCE_0, PRAY_0, 0, SYMBOL_STRESS_0, "", "", "", "", START_1, LEARN_1, START_DO_1, LEARN_DO_2, START_DO_NO, LEARN_DO_NO, LEARN_DIFF_SKILLS, 0.0, 0, 0);

  discArray[SKILL_THROATSLIT] = new spellInfo(SKILL_THIEF, DISC_MURDER, DISC_MURDER, STAT_DEX, "slit", TASK_NORMAL, LAG_6, POSITION_CRAWLING, MANA_0, LIFEFORCE_0, PRAY_0, 0, SYMBOL_STRESS_0, "", "", "", "", START_1, LEARN_1, START_DO_1, LEARN_DO_2, START_DO_NO, LEARN_DO_NO, LEARN_DIFF_SKILLS, 0.0, 0, 0);

  discArray[SKILL_SEARCH] = new spellInfo(SKILL_THIEF, DISC_THIEF, DISC_LOOTING, STAT_EXT, "search", TASK_EASY, LAG_0, POSITION_CRAWLING, MANA_0, LIFEFORCE_0, PRAY_0, 0, SYMBOL_STRESS_0, "", "", "", "", START_1, LEARN_1, START_DO_1, LEARN_DO_1, START_DO_NO, LEARN_DO_NO, LEARN_DIFF_UNUSUAL, 0.0, 0, 0);

  discArray[SKILL_SPY] = new spellInfo(SKILL_THIEF, DISC_THIEF, DISC_STEALTH, STAT_EXT, "spy", TASK_EASY, LAG_0, POSITION_CRAWLING, MANA_0, LIFEFORCE_0, PRAY_0, 0, SYMBOL_STRESS_0, "You cease your espionage activities.", "", "", "", START_61, LEARN_5, START_DO_1, LEARN_DO_2, START_DO_NO, LEARN_DO_NO, LEARN_DIFF_SKILLS, 0.0, 0, 0);

  discArray[SKILL_SWITCH_THIEF] = new spellInfo(SKILL_THIEF, DISC_THIEF, DISC_THIEF_FIGHT, STAT_EXT, "switch opponents", TASK_EASY, LAG_2, POSITION_CRAWLING, MANA_0, LIFEFORCE_0, PRAY_0, 0, SYMBOL_STRESS_0, "", "", "", "", START_21, LEARN_3, START_DO_10, LEARN_DO_3, START_DO_NO, LEARN_DO_NO, LEARN_DIFF_UNUSUAL, 0.0, 0, 0);

  discArray[SKILL_STEAL] = new spellInfo(SKILL_THIEF, DISC_THIEF, DISC_LOOTING, STAT_EXT, "steal", TASK_NORMAL, LAG_2, POSITION_CRAWLING, MANA_0, LIFEFORCE_0, PRAY_0, 0, SYMBOL_STRESS_0, "", "", "", "", START_1, LEARN_1, START_DO_10, LEARN_DO_2, START_DO_NO, LEARN_DO_NO, LEARN_DIFF_SKILLS, 0.0, 0, 0);

  // detect trap can go long way before learn by do takes over
  // it's mostly book learning
  discArray[SKILL_DETECT_TRAP] = new spellInfo(SKILL_THIEF, DISC_THIEF, DISC_TRAPS, STAT_EXT, "detect trap", TASK_EASY, LAG_0, POSITION_CRAWLING, MANA_0, LIFEFORCE_0, PRAY_0, 0, SYMBOL_STRESS_0, "", "", "", "", START_1, LEARN_3, START_DO_60, LEARN_DO_2, START_DO_NO, LEARN_DO_NO, LEARN_DIFF_SKILLS, 0.0, 0, 0);

  discArray[SKILL_SUBTERFUGE] = new spellInfo(SKILL_THIEF, DISC_THIEF, DISC_STEALTH, STAT_EXT, "subterfuge", TASK_NORMAL, LAG_2, POSITION_CRAWLING, MANA_0, LIFEFORCE_0, PRAY_0, 0, SYMBOL_STRESS_0, "", "", "", "", START_41, LEARN_2, START_DO_20, LEARN_DO_5, START_DO_NO, LEARN_DO_NO, LEARN_DIFF_SKILLS, 0.0, 0, 0);


  discArray[SKILL_DISARM_TRAP] = new spellInfo(SKILL_THIEF, DISC_THIEF, DISC_TRAPS, STAT_EXT, "disarm trap", TASK_NORMAL, LAG_0, POSITION_CRAWLING, MANA_0, LIFEFORCE_0, PRAY_0, 0, SYMBOL_STRESS_0, "", "", "", "", START_33, LEARN_3, START_DO_25, LEARN_DO_5, START_DO_NO, LEARN_DO_NO, LEARN_DIFF_SKILLS, 0.0, 0, 0);

  discArray[SKILL_CUDGEL] = new spellInfo(SKILL_THIEF, DISC_THIEF, DISC_MURDER, STAT_EXT, "cudgel", TASK_EASY, LAG_4, POSITION_CRAWLING, MANA_0, LIFEFORCE_0, PRAY_0, 0, SYMBOL_STRESS_0, "", "", "", "", START_66, LEARN_3, START_DO_1, LEARN_DO_2, START_DO_NO, LEARN_DO_NO, LEARN_DIFF_SKILLS, 0.0, 0, 0);

  discArray[SKILL_HIDE] = new spellInfo(SKILL_THIEF, DISC_THIEF, DISC_STEALTH, STAT_EXT, "hide", TASK_EASY, LAG_1, POSITION_CRAWLING, MANA_0, LIFEFORCE_0, PRAY_0, 0, SYMBOL_STRESS_0, "", "", "", "", START_50, LEARN_3, START_DO_1, LEARN_DO_2, START_DO_NO, LEARN_DO_NO, LEARN_DIFF_SKILLS, 0.0, 0, 0);

  discArray[SKILL_DISARM_THIEF] = new spellInfo(SKILL_THIEF, DISC_THIEF, DISC_THIEF_FIGHT, STAT_EXT, "disarm", TASK_NORMAL, LAG_2, POSITION_CRAWLING, MANA_0, LIFEFORCE_0, PRAY_0, 0, SYMBOL_STRESS_0, "", "", "", "", START_33, LEARN_3, START_DO_1, LEARN_DO_4, START_DO_NO, LEARN_DO_NO, LEARN_DIFF_SKILLS, 0.0, 0, 0);

// disc_thief_fight

  discArray[SKILL_DODGE_THIEF] = new spellInfo(SKILL_THIEF, DISC_THIEF_FIGHT, DISC_THIEF_FIGHT, STAT_EXT, "dodge", TASK_EASY, LAG_0, POSITION_DEAD, MANA_0, LIFEFORCE_0, PRAY_0, 0, SYMBOL_STRESS_0, "", "", "", "", START_1, LEARN_1,START_DO_1, LEARN_DO_1, START_DO_NO, LEARN_DO_NO, LEARN_DIFF_UNUSUAL, 0.0, 0, 0);

  discArray[SKILL_DUAL_WIELD_THIEF] = new spellInfo(SKILL_THIEF, DISC_THIEF_FIGHT, DISC_THIEF_FIGHT, STAT_EXT, "dual wield", TASK_NORMAL, LAG_0, POSITION_CRAWLING, MANA_0, LIFEFORCE_0, PRAY_0, 0, SYMBOL_STRESS_0, "", "", "", "", START_50, LEARN_2, START_DO_1, LEARN_DO_1, START_DO_NO, LEARN_DO_NO, LEARN_DIFF_UNUSUAL, 0.0, 0, 0);

  discArray[SKILL_GARROTTE] = new spellInfo(SKILL_THIEF, DISC_THIEF_FIGHT, DISC_THIEF_FIGHT, STAT_EXT, "garrotte", TASK_NORMAL, LAG_3, POSITION_CRAWLING, MANA_0, LIFEFORCE_0, PRAY_0, 0, SYMBOL_STRESS_0, "", "", "", "", START_1, LEARN_3, START_DO_1, LEARN_DO_2, START_DO_NO, LEARN_DO_NO, LEARN_DIFF_SKILLS, 0.0, 0, 0);

  discArray[SKILL_REPAIR_THIEF] = new spellInfo(SKILL_THIEF, DISC_THIEF, DISC_BLACKSMITHING, STAT_EXT, "thief repair", TASK_NORMAL, LAG_0, POSITION_SITTING, MANA_0, LIFEFORCE_0, PRAY_0, 0, SYMBOL_STRESS_0, "", "", "", "", START_50, LEARN_3, START_DO_1, LEARN_DO_1, START_DO_NO, LEARN_DO_NO, LEARN_DIFF_SKILLS, 0.0, 0, 0);


// disc_murder


// disc_looting
  discArray[SKILL_COUNTER_STEAL] = new spellInfo(SKILL_THIEF, DISC_LOOTING, DISC_LOOTING, STAT_EXT, "counter steal", TASK_NORMAL, LAG_0, POSITION_DEAD, MANA_0, LIFEFORCE_0, PRAY_0, 0, SYMBOL_STRESS_0, "", "", "", "", START_1, LEARN_1,START_DO_1, LEARN_DO_1, START_DO_NO, LEARN_DO_NO, LEARN_DIFF_UNUSUAL, 0.0, 0, 0);

  discArray[SKILL_PLANT] = new spellInfo(SKILL_THIEF, DISC_LOOTING, DISC_LOOTING, STAT_EXT, "plant", TASK_NORMAL, LAG_0, POSITION_DEAD, MANA_0, LIFEFORCE_0, PRAY_0, 0, SYMBOL_STRESS_0, "", "", "", "", START_50, LEARN_3,START_DO_1, LEARN_DO_1, START_DO_NO, LEARN_DO_NO, LEARN_DIFF_UNUSUAL, 0.0, 0, 0);


// disc_poison

  discArray[SKILL_POISON_WEAPON] = new spellInfo(SKILL_THIEF, DISC_POISONS, DISC_POISONS, STAT_EXT, "poison weapons", TASK_EASY, LAG_4, POSITION_CRAWLING, MANA_0, LIFEFORCE_0, PRAY_0, 0, SYMBOL_STRESS_0, "", "", "", "", START_1, LEARN_1, START_DO_1, LEARN_DO_2, START_DO_NO, LEARN_DO_NO, LEARN_DIFF_UNUSUAL, 0.0, 0, 0);

// disc_stealth

  discArray[SKILL_CONCEALMENT] = new spellInfo(SKILL_THIEF, DISC_STEALTH, DISC_STEALTH, STAT_EXT, "concealment", TASK_NORMAL, LAG_0, POSITION_CRAWLING, MANA_0, LIFEFORCE_0, PRAY_0, 0, SYMBOL_STRESS_0, "Your path is no longer concealed.", "$n's path is no longer concealed.", "", "", START_1, LEARN_1, START_DO_10, LEARN_DO_1, START_DO_NO, LEARN_DO_NO, LEARN_DIFF_SKILLS, 0.0, 0, 0);


  discArray[SKILL_DISGUISE] = new spellInfo(SKILL_THIEF, DISC_STEALTH, DISC_STEALTH, STAT_EXT, "disguise", TASK_NORMAL, LAG_0, POSITION_STANDING, MANA_0, LIFEFORCE_0, PRAY_0, 0, SYMBOL_STRESS_0, "You can not hold your disguise any longer.", "", "You do not know how much longer you can keep this shape.", "", START_1, LEARN_1, START_DO_10, LEARN_DO_2, START_DO_NO, LEARN_DO_NO, LEARN_DIFF_SKILLS, 0.0, 0, 0);


// disc_traps

  discArray[SKILL_SET_TRAP_ARROW] = new spellInfo(SKILL_THIEF, DISC_TRAPS, DISC_TRAPS, STAT_EXT, "set arrow trap",TASK_NORMAL, LAG_0, POSITION_CRAWLING, MANA_0, LIFEFORCE_0, PRAY_0, 0, SYMBOL_STRESS_0, "", "", "", "", START_1, LEARN_3, START_DO_40, LEARN_DO_2, START_DO_NO, LEARN_DO_NO, LEARN_DIFF_SKILLS, 0.0, 0, 0);

  discArray[SKILL_SET_TRAP_CONT] = new spellInfo(SKILL_THIEF, DISC_TRAPS, DISC_TRAPS, STAT_EXT, "set container trap",TASK_NORMAL, LAG_0, POSITION_CRAWLING, MANA_0, LIFEFORCE_0, PRAY_0, 0, SYMBOL_STRESS_0, "", "", "", "", START_1, LEARN_3, START_DO_40, LEARN_DO_2, START_DO_NO, LEARN_DO_NO, LEARN_DIFF_SKILLS, 0.0, 0, 0);

  discArray[SKILL_SET_TRAP_DOOR] = new spellInfo(SKILL_THIEF, DISC_TRAPS, DISC_TRAPS, STAT_EXT, "set door trap",TASK_NORMAL, LAG_0, POSITION_CRAWLING, MANA_0, LIFEFORCE_0, PRAY_0, 0, SYMBOL_STRESS_0, "", "", "", "", START_30, LEARN_3, START_DO_40, LEARN_DO_2, START_DO_NO, LEARN_DO_NO, LEARN_DIFF_SKILLS, 0.0, 0, 0);

  discArray[SKILL_SET_TRAP_MINE] = new spellInfo(SKILL_THIEF, DISC_TRAPS, DISC_TRAPS, STAT_EXT, "set mine trap",TASK_NORMAL, LAG_0, POSITION_CRAWLING, MANA_0, LIFEFORCE_0, PRAY_0, 0, SYMBOL_STRESS_0, "", "", "", "", START_60, LEARN_3, START_DO_40, LEARN_DO_2, START_DO_NO, LEARN_DO_NO, LEARN_DIFF_SKILLS, 0.0, 0, 0);

  discArray[SKILL_SET_TRAP_GREN] = new spellInfo(SKILL_THIEF, DISC_TRAPS, DISC_TRAPS, STAT_EXT, "set grenade trap",TASK_NORMAL, LAG_0, POSITION_CRAWLING, MANA_0, LIFEFORCE_0, PRAY_0, 0, SYMBOL_STRESS_0, "", "", "", "", START_80, LEARN_5, START_DO_40, LEARN_DO_2, START_DO_NO, LEARN_DO_NO, LEARN_DIFF_SKILLS, 0.0, 0, 0);

// disc_advanced_adventuring
  discArray[SKILL_HIKING] = new spellInfo(SKILL_GENERAL, DISC_ADVANCED_ADVENTURING, DISC_ADVANCED_ADVENTURING, STAT_EXT, "hiking", TASK_EASY, LAG_0, POSITION_CRAWLING, MANA_0, LIFEFORCE_0, PRAY_0, 0, SYMBOL_STRESS_0, "", "", "", "", START_1, LEARN_1, START_DO_NO, LEARN_DO_NO, START_DO_NO, LEARN_DO_NO, LEARN_DIFF_SKILLS, 0.0, 0, 0);

  discArray[SKILL_FORAGE] = new spellInfo(SKILL_GENERAL, DISC_ADVANCED_ADVENTURING, DISC_ADVANCED_ADVENTURING, STAT_EXT, "forage", TASK_EASY, LAG_0, POSITION_CRAWLING, MANA_0, LIFEFORCE_0, PRAY_0, 0, SYMBOL_STRESS_0, "You are able to forage again.", "", "", "", START_1, LEARN_5, START_DO_30, LEARN_DO_5, START_DO_NO, LEARN_DO_NO, LEARN_DIFF_SKILLS, 0.0, 0, 0);

  discArray[SKILL_SEEKWATER] = new spellInfo(SKILL_GENERAL, DISC_ADVANCED_ADVENTURING, DISC_ADVANCED_ADVENTURING, STAT_EXT, "seekwater", TASK_EASY, LAG_0, POSITION_CRAWLING, MANA_0, LIFEFORCE_0, PRAY_0, 0, SYMBOL_STRESS_0, "", "", "", "", START_6, LEARN_5, START_DO_1, LEARN_DO_5, START_DO_NO, LEARN_DO_NO, LEARN_DIFF_SKILLS, 0.0, 0, 0);

  discArray[SKILL_DIVINATION] = new spellInfo(SKILL_GENERAL, DISC_ADVANCED_ADVENTURING, DISC_ADVANCED_ADVENTURING, STAT_EXT, "divine", TASK_EASY, LAG_0, POSITION_CRAWLING, MANA_0, LIFEFORCE_0, PRAY_0, 0, SYMBOL_STRESS_0, "", "", "", "", START_51, LEARN_2, START_DO_10, LEARN_DO_1, START_DO_NO, LEARN_DO_NO, LEARN_DIFF_SKILLS, 0.0, 0, 0);

  discArray[SKILL_ENCAMP] = new spellInfo(SKILL_GENERAL, DISC_ADVENTURING, DISC_ADVANCED_ADVENTURING, STAT_EXT, "encamp", TASK_EASY, LAG_0, POSITION_CRAWLING, MANA_0, LIFEFORCE_0, PRAY_0, 0, SYMBOL_STRESS_0, "", "", "", "", START_26, LEARN_2, START_DO_1, LEARN_DO_2, START_DO_NO, LEARN_DO_NO, LEARN_DIFF_SKILLS, 0.0, 0, 0);

  discArray[SKILL_SKIN] = new spellInfo(SKILL_GENERAL, DISC_ADVANCED_ADVENTURING, DISC_ADVANCED_ADVENTURING, STAT_EXT, "skinning", TASK_EASY, LAG_0, POSITION_CRAWLING, MANA_0, LIFEFORCE_0, PRAY_0, 0, SYMBOL_STRESS_0, "", "", "", "", START_26, LEARN_2, START_DO_10, LEARN_DO_1, START_DO_NO, LEARN_DO_NO, LEARN_DIFF_UNUSUAL, 0.0, 0, 0);

  discArray[SKILL_FISHLORE] = new spellInfo(SKILL_GENERAL, DISC_ADVANCED_ADVENTURING, DISC_ADVANCED_ADVENTURING, STAT_EXT, "fishlore", TASK_EASY, LAG_0, POSITION_SITTING, MANA_0, LIFEFORCE_0, PRAY_0, 0, SYMBOL_STRESS_0, "", "", "", "", START_1, LEARN_1, START_DO_1, LEARN_DO_1, START_DO_NO, LEARN_DO_NO, LEARN_DIFF_UNUSUAL, 0.0, 0, 0);

  discArray[SKILL_TROLLISH] = new spellInfo(SKILL_GENERAL, DISC_ADVANCED_ADVENTURING, DISC_ADVANCED_ADVENTURING, STAT_EXT, "trollish", TASK_NORMAL, LAG_0, POSITION_SITTING, MANA_0, LIFEFORCE_0, PRAY_0, 0, SYMBOL_STRESS_0, "", "", "", "", START_50, LEARN_2, START_DO_25, LEARN_DO_1, START_DO_NO, LEARN_DO_NO, LEARN_DIFF_SKILLS, 0.0, 0, 0);

  discArray[SKILL_BULLYWUGCROAK] = new spellInfo(SKILL_GENERAL, DISC_ADVANCED_ADVENTURING, DISC_ADVANCED_ADVENTURING, STAT_EXT, "bullycroak", TASK_NORMAL, LAG_0, POSITION_SITTING, MANA_0, LIFEFORCE_0, PRAY_0, 0, SYMBOL_STRESS_0, "", "", "", "", START_50, LEARN_2, START_DO_25, LEARN_DO_1, START_DO_NO, LEARN_DO_NO, LEARN_DIFF_SKILLS, 0.0, 0, 0);

  discArray[SKILL_AVIAN] = new spellInfo(SKILL_GENERAL, DISC_ADVANCED_ADVENTURING, DISC_ADVANCED_ADVENTURING, STAT_EXT, "avian", TASK_NORMAL, LAG_0, POSITION_SITTING, MANA_0, LIFEFORCE_0, PRAY_0, 0, SYMBOL_STRESS_0, "", "", "", "", START_50, LEARN_2, START_DO_25, LEARN_DO_1, START_DO_NO, LEARN_DO_NO, LEARN_DIFF_SKILLS, 0.0, 0, 0);

  discArray[SKILL_FISHBURBLE] = new spellInfo(SKILL_GENERAL, DISC_ADVANCED_ADVENTURING, DISC_ADVANCED_ADVENTURING, STAT_EXT, "fish burble", TASK_NORMAL, LAG_0, POSITION_SITTING, MANA_0, LIFEFORCE_0, PRAY_0, 0, SYMBOL_STRESS_0, "", "", "", "", START_50, LEARN_2, START_DO_25, LEARN_DO_1, START_DO_NO, LEARN_DO_NO, LEARN_DIFF_SKILLS, 0.0, 0, 0);

// disc_adventuring

  discArray[SKILL_BUTCHER] = new spellInfo(SKILL_GENERAL, DISC_ADVENTURING, DISC_ADVENTURING, STAT_EXT, "butcher", TASK_EASY, LAG_0, POSITION_CRAWLING, MANA_0, LIFEFORCE_0, PRAY_0, 0, SYMBOL_STRESS_0, "", "", "", "", START_1, LEARN_2, START_DO_10, LEARN_DO_1, START_DO_NO, LEARN_DO_NO, LEARN_DIFF_UNUSUAL, 0.0, 0, 0);

  discArray[SKILL_FISHING] = new spellInfo(SKILL_GENERAL, DISC_ADVENTURING, DISC_ADVENTURING, STAT_EXT, "fishing", TASK_EASY, LAG_0, POSITION_SITTING, MANA_0, LIFEFORCE_0, PRAY_0, 0, SYMBOL_STRESS_0, "", "", "", "", START_1, LEARN_1, START_DO_1, LEARN_DO_1, START_DO_NO, LEARN_DO_NO, LEARN_DIFF_UNUSUAL, 0.0, 0, 0);

  discArray[SKILL_LOGGING] = new spellInfo(SKILL_GENERAL, DISC_ADVENTURING, DISC_ADVENTURING, STAT_EXT, "lumberjack", TASK_EASY, LAG_0, POSITION_SITTING, MANA_0, LIFEFORCE_0, PRAY_0, 0, SYMBOL_STRESS_0, "", "", "", "", START_1, LEARN_1, START_DO_1, LEARN_DO_1, START_DO_NO, LEARN_DO_NO, LEARN_DIFF_UNUSUAL, 0.0, 0, 0);
  
  discArray[SKILL_ALCOHOLISM] = new spellInfo(SKILL_GENERAL, DISC_ADVENTURING, DISC_ADVENTURING, STAT_EXT, "alcoholism", TASK_TRIVIAL, LAG_0, POSITION_CRAWLING, MANA_0, LIFEFORCE_0, PRAY_0, 0, SYMBOL_STRESS_0, "", "", "", "", START_1, LEARN_4, START_DO_1, LEARN_DO_2, START_DO_NO, LEARN_DO_NO, LEARN_DIFF_SKILLS, 0.0, 0, 0);

  discArray[SKILL_RIDE] = new spellInfo(SKILL_GENERAL, DISC_ADVENTURING, DISC_ADVENTURING, STAT_EXT, "ride", TASK_TRIVIAL, LAG_0, POSITION_CRAWLING, MANA_0, LIFEFORCE_0, PRAY_0, 0, SYMBOL_STRESS_0, "", "", "", "", START_1, LEARN_4, START_DO_25, LEARN_DO_2, START_DO_NO, LEARN_DO_NO, LEARN_DIFF_SKILLS, 0.0, 0, 0);

  discArray[SKILL_SIGN] = new spellInfo(SKILL_GENERAL, DISC_ADVENTURING, DISC_ADVENTURING, STAT_EXT, "sign", TASK_EASY, LAG_0, POSITION_CRAWLING, MANA_0, LIFEFORCE_0, PRAY_0, 0, SYMBOL_STRESS_0, "", "", "", "", START_26, LEARN_4, START_DO_1, LEARN_DO_1, START_DO_NO, LEARN_DO_NO, LEARN_DIFF_UNUSUAL, 0.0, 0, 0);

  discArray[SKILL_SWIM] = new spellInfo(SKILL_GENERAL, DISC_ADVENTURING, DISC_ADVENTURING, STAT_EXT, "swim", TASK_NORMAL, LAG_0, POSITION_CRAWLING, MANA_0, LIFEFORCE_0, PRAY_0, 0, SYMBOL_STRESS_0, "", "", "", "", START_1, LEARN_1, START_DO_40, LEARN_DO_5, START_DO_NO, LEARN_DO_NO, LEARN_DIFF_SKILLS, 0.0, 0, 0);

  discArray[SKILL_CONS_UNDEAD] = new spellInfo(SKILL_GENERAL, DISC_ADVENTURING, DISC_ADVENTURING, STAT_EXT, "know undead", TASK_EASY, LAG_0, POSITION_DEAD, MANA_0, LIFEFORCE_0, PRAY_0, 0, SYMBOL_STRESS_0, "", "", "", "", START_61, LEARN_5, START_DO_1, LEARN_DO_2, START_DO_NO, LEARN_DO_NO, LEARN_DIFF_UNUSUAL, 0.0, 0, 0);

  discArray[SKILL_CONS_VEGGIE] = new spellInfo(SKILL_GENERAL, DISC_ADVENTURING, DISC_ADVENTURING, STAT_EXT, "know veggie", TASK_EASY, LAG_0, POSITION_DEAD, MANA_0, LIFEFORCE_0, PRAY_0, 0, SYMBOL_STRESS_0, "", "", "", "", START_21, LEARN_5, START_DO_1, LEARN_DO_2, START_DO_NO, LEARN_DO_NO, LEARN_DIFF_UNUSUAL, 0.0, 0, 0);

  discArray[SKILL_CONS_DEMON] = new spellInfo(SKILL_GENERAL, DISC_ADVENTURING, DISC_ADVENTURING, STAT_EXT, "know demon", TASK_EASY, LAG_0, POSITION_DEAD, MANA_0, LIFEFORCE_0, PRAY_0, 0, SYMBOL_STRESS_0, "", "", "", "", START_81, LEARN_5, START_DO_1, LEARN_DO_2, START_DO_NO, LEARN_DO_NO, LEARN_DIFF_UNUSUAL, 0.0, 0, 0);

  discArray[SKILL_CONS_ANIMAL] = new spellInfo(SKILL_GENERAL, DISC_ADVENTURING, DISC_ADVENTURING, STAT_EXT, "know animal", TASK_EASY, LAG_0, POSITION_DEAD, MANA_0, LIFEFORCE_0, PRAY_0, 0, SYMBOL_STRESS_0, "", "", "", "", START_1, LEARN_5, START_DO_1, LEARN_DO_2, START_DO_NO, LEARN_DO_NO, LEARN_DIFF_UNUSUAL, 0.0, 0, 0);

  discArray[SKILL_CONS_REPTILE] = new spellInfo(SKILL_GENERAL, DISC_ADVENTURING, DISC_ADVENTURING, STAT_EXT, "know reptile", TASK_EASY, LAG_0, POSITION_DEAD, MANA_0, LIFEFORCE_0, PRAY_0, 0, SYMBOL_STRESS_0, "", "", "", "", START_31, LEARN_5, START_DO_1, LEARN_DO_2, START_DO_NO, LEARN_DO_NO, LEARN_DIFF_UNUSUAL, 0.0, 0, 0);

  discArray[SKILL_CONS_PEOPLE] = new spellInfo(SKILL_GENERAL, DISC_ADVENTURING, DISC_ADVENTURING, STAT_EXT, "know people", TASK_EASY, LAG_0, POSITION_DEAD, MANA_0, LIFEFORCE_0, PRAY_0, 0, SYMBOL_STRESS_0, "", "", "", "", START_1, LEARN_2, START_DO_1, LEARN_DO_1, START_DO_NO, LEARN_DO_NO, LEARN_DIFF_UNUSUAL, 0.0, 0, 0);

  discArray[SKILL_CONS_GIANT] = new spellInfo(SKILL_GENERAL, DISC_ADVENTURING, DISC_ADVENTURING, STAT_EXT, "know giantkin", TASK_EASY, LAG_0, POSITION_DEAD, MANA_0, LIFEFORCE_0, PRAY_0, 0, SYMBOL_STRESS_0, "", "", "", "", START_41, LEARN_5, START_DO_1, LEARN_DO_2, START_DO_NO, LEARN_DO_NO, LEARN_DIFF_UNUSUAL, 0.0, 0, 0);

  discArray[SKILL_CONS_OTHER] = new spellInfo(SKILL_GENERAL, DISC_ADVENTURING, DISC_ADVENTURING, STAT_EXT, "know other", TASK_EASY, LAG_0, POSITION_DEAD, MANA_0, LIFEFORCE_0, PRAY_0, 0, SYMBOL_STRESS_0, "", "", "", "", START_51, LEARN_5, START_DO_1, LEARN_DO_5, START_DO_NO, LEARN_DO_NO, LEARN_DIFF_UNUSUAL, 0.0, 0, 0);

  discArray[SKILL_READ_MAGIC] = new spellInfo(SKILL_GENERAL, DISC_ADVENTURING, DISC_ADVENTURING, STAT_EXT, "read magic", TASK_EASY, LAG_0, POSITION_DEAD, MANA_0, LIFEFORCE_0, PRAY_0, 0, SYMBOL_STRESS_0, "", "", "", "", START_1, LEARN_1, START_DO_1, LEARN_DO_5, START_DO_NO, LEARN_DO_NO, LEARN_DIFF_SKILLS, 0.0, 0, 0);

  discArray[SKILL_BANDAGE] = new spellInfo(SKILL_GENERAL, DISC_ADVENTURING, DISC_ADVENTURING, STAT_EXT, "bandage", TASK_EASY, LAG_1, POSITION_DEAD, MANA_0, LIFEFORCE_0, PRAY_0, 0, SYMBOL_STRESS_0, "", "", "", "", START_1, LEARN_10, START_DO_30, LEARN_DO_5, START_DO_NO, LEARN_DO_NO, LEARN_DIFF_UNUSUAL, 0.3, 0, 0);

  discArray[SKILL_CLIMB] = new spellInfo(SKILL_GENERAL, DISC_ADVENTURING, DISC_ADVENTURING, STAT_EXT, "climbing", TASK_EASY, LAG_0, POSITION_DEAD, MANA_0, LIFEFORCE_0, PRAY_0, 0, SYMBOL_STRESS_0, "", "", "", "", START_1, LEARN_3, START_DO_20, LEARN_DO_5, START_DO_NO, LEARN_DO_NO, LEARN_DIFF_UNUSUAL, 0.0, 0, 0);

  discArray[SKILL_DISSECT] = new spellInfo(SKILL_GENERAL, DISC_ADVENTURING, DISC_ADVENTURING, STAT_EXT, "dissect", TASK_EASY, LAG_1, POSITION_SITTING, MANA_0, LIFEFORCE_0, PRAY_0, 0, SYMBOL_STRESS_0, "", "", "", "", START_1, LEARN_4, START_DO_10, LEARN_DO_1, START_DO_NO, LEARN_DO_NO, LEARN_DIFF_UNUSUAL, 0.0, 0, 0);

  discArray[SKILL_EVALUATE] = new spellInfo(SKILL_GENERAL, DISC_ADVENTURING, DISC_ADVENTURING, STAT_EXT, "evaluate", TASK_EASY, LAG_0, POSITION_SITTING, MANA_0, LIFEFORCE_0, PRAY_0, 0, SYMBOL_STRESS_0, "", "", "", "", START_51, LEARN_4, START_DO_1, LEARN_DO_2, START_DO_NO, LEARN_DO_NO, LEARN_DIFF_UNUSUAL,0.0, 0, 0);

  discArray[SKILL_TACTICS] = new spellInfo(SKILL_GENERAL, DISC_ADVENTURING, DISC_ADVENTURING, STAT_EXT, "tactics", TASK_EASY, LAG_0, POSITION_SITTING, MANA_0, LIFEFORCE_0, PRAY_0, 0, SYMBOL_STRESS_0, "", "", "", "", START_1, LEARN_5, START_DO_NO, LEARN_DO_NO, START_DO_NO, LEARN_DO_NO, LEARN_DIFF_UNUSUAL, 0.0, 0, 0);

  discArray[SKILL_DEFENSE] = new spellInfo(SKILL_GENERAL, DISC_ADVENTURING, DISC_ADVENTURING, STAT_EXT, "defense", TASK_EASY, LAG_0, POSITION_SITTING, MANA_0, LIFEFORCE_0, PRAY_0, 0, SYMBOL_STRESS_0, "", "", "", "", START_1, LEARN_1, START_DO_1, LEARN_DO_1, START_DO_NO, LEARN_DO_NO, LEARN_DIFF_UNUSUAL, 0.0, 0, 0);

  discArray[SKILL_OFFENSE] = new spellInfo(SKILL_GENERAL, DISC_ADVENTURING, DISC_ADVENTURING, STAT_EXT, "offense", TASK_EASY, LAG_0, POSITION_SITTING, MANA_0, LIFEFORCE_0, PRAY_0, 0, SYMBOL_STRESS_0, "", "", "", "", START_1, LEARN_1, START_DO_1, LEARN_DO_1, START_DO_NO, LEARN_DO_NO, LEARN_DIFF_UNUSUAL, 0.0, 0, 0);

  discArray[SKILL_WHITTLE] = new spellInfo(SKILL_GENERAL, DISC_ADVENTURING, DISC_ADVENTURING, STAT_EXT, "whittle", TASK_EASY, LAG_0, POSITION_STANDING, MANA_0, LIFEFORCE_0, PRAY_0, 0, SYMBOL_STRESS_0, "", "", "", "", START_50, LEARN_2, START_DO_1, LEARN_DO_1, START_DO_NO, LEARN_DO_NO, LEARN_DIFF_UNUSUAL, 0.0, 0, 0);

  discArray[SKILL_MEND] = new spellInfo(SKILL_GENERAL, DISC_ADVENTURING, DISC_BLACKSMITHING, STAT_EXT, "mend", TASK_EASY, LAG_0, POSITION_SITTING, MANA_0, LIFEFORCE_0, PRAY_0, 0, SYMBOL_STRESS_0, "", "", "", "", START_26, LEARN_3, START_DO_1, LEARN_DO_1, START_DO_NO, LEARN_DO_NO, LEARN_DIFF_SKILLS, 0.0, 0, 0);

  discArray[SKILL_GUTTER_CANT] = new spellInfo(SKILL_GENERAL, DISC_ADVENTURING, DISC_ADVENTURING, STAT_EXT, "gutter cant", TASK_EASY, LAG_0, POSITION_SITTING, MANA_0, LIFEFORCE_0, PRAY_0, 0, SYMBOL_STRESS_0, "", "", "", "", START_65, LEARN_4, START_DO_25, LEARN_DO_1, START_DO_NO, LEARN_DO_NO, LEARN_DIFF_UNUSUAL, 0.0, 0, 0);

  discArray[SKILL_GNOLL_JARGON] = new spellInfo(SKILL_GENERAL, DISC_ADVENTURING, DISC_ADVENTURING, STAT_EXT, "gnoll jargon", TASK_EASY, LAG_0, POSITION_SITTING, MANA_0, LIFEFORCE_0, PRAY_0, 0, SYMBOL_STRESS_0, "", "", "", "", START_65, LEARN_4, START_DO_25, LEARN_DO_1, START_DO_NO, LEARN_DO_NO, LEARN_DIFF_UNUSUAL, 0.0, 0, 0);
  
  discArray[SKILL_TROGLODYTE_PIDGIN] = new spellInfo(SKILL_GENERAL, DISC_ADVENTURING, DISC_ADVENTURING, STAT_EXT, "troglodyte pidgin", TASK_EASY, LAG_0, POSITION_SITTING, MANA_0, LIFEFORCE_0, PRAY_0, 0, SYMBOL_STRESS_0, "", "", "", "", START_65, LEARN_4, START_DO_25, LEARN_DO_1, START_DO_NO, LEARN_DO_NO, LEARN_DIFF_UNUSUAL, 0.0, 0, 0);

  // disc_combat

  discArray[SKILL_SLASH_PROF] = new spellInfo(SKILL_GENERAL, DISC_COMBAT, DISC_COMBAT, STAT_EXT, "slash proficiency", TASK_NORMAL, LAG_0, POSITION_DEAD, MANA_0, LIFEFORCE_0, PRAY_0, 0, SYMBOL_STRESS_0, "", "", "", "", START_1, LEARN_1, START_DO_1, LEARN_DO_1, START_DO_NO, LEARN_DO_NO, LEARN_DIFF_UNUSUAL, 0.0, 0, 0);

  discArray[SKILL_RANGED_PROF] = new spellInfo(SKILL_GENERAL, DISC_COMBAT, DISC_COMBAT, STAT_EXT, "ranged proficiency", TASK_NORMAL, LAG_0, POSITION_DEAD, MANA_0, LIFEFORCE_0, PRAY_0, 0, SYMBOL_STRESS_0, "", "", "", "", START_51, LEARN_3, START_DO_1, LEARN_DO_5, START_DO_NO, LEARN_DO_NO, LEARN_DIFF_UNUSUAL, 0.0, 0, 0);

  discArray[SKILL_PIERCE_PROF] = new spellInfo(SKILL_GENERAL, DISC_COMBAT, DISC_COMBAT, STAT_EXT, "pierce proficiency", TASK_NORMAL, LAG_0, POSITION_DEAD, MANA_0, LIFEFORCE_0, PRAY_0, 0, SYMBOL_STRESS_0, "", "", "", "", START_1, LEARN_1, START_DO_1, LEARN_DO_1, START_DO_NO, LEARN_DO_NO, LEARN_DIFF_UNUSUAL, 0.0, 0, 0);

  discArray[SKILL_BLUNT_PROF] = new spellInfo(SKILL_GENERAL, DISC_COMBAT, DISC_COMBAT, STAT_EXT, "blunt proficiency", TASK_NORMAL, LAG_0, POSITION_DEAD, MANA_0, LIFEFORCE_0, PRAY_0, 0, SYMBOL_STRESS_0, "", "", "", "", START_1, LEARN_1, START_DO_1, LEARN_DO_1, START_DO_NO, LEARN_DO_NO, LEARN_DIFF_UNUSUAL, 0.0, 0, 0);

  discArray[SKILL_SHARPEN] = new spellInfo(SKILL_WARRIOR, DISC_COMBAT, DISC_COMBAT, STAT_EXT, "sharpen", TASK_NORMAL, LAG_0, POSITION_SITTING, MANA_0, LIFEFORCE_0, PRAY_0, 0, SYMBOL_STRESS_0, "", "", "", "", START_51, LEARN_2, START_DO_10, LEARN_DO_2, START_DO_NO, LEARN_DO_NO, LEARN_DIFF_UNUSUAL, 0.0, 0, 0);

  discArray[SKILL_DULL] = new spellInfo(SKILL_WARRIOR, DISC_COMBAT, DISC_COMBAT, STAT_EXT, "smooth", TASK_NORMAL, LAG_0, POSITION_SITTING, MANA_0, LIFEFORCE_0, PRAY_0, 0, SYMBOL_STRESS_0, "", "", "", "", START_51, LEARN_2, START_DO_10, LEARN_DO_2, START_DO_NO, LEARN_DO_NO, LEARN_DIFF_UNUSUAL, 0.0, 0, 0);

  discArray[SKILL_BAREHAND_PROF] = new spellInfo(SKILL_GENERAL, DISC_COMBAT, DISC_COMBAT, STAT_EXT, "barehand proficiency", TASK_NORMAL, LAG_0, POSITION_DEAD, MANA_0, LIFEFORCE_0, PRAY_0, 0, SYMBOL_STRESS_0, "", "", "", "", START_1, LEARN_1, START_DO_1, LEARN_DO_1, START_DO_NO, LEARN_DO_NO, LEARN_DIFF_UNUSUAL, 0.0, 0, 0);

  // disc_defense

  discArray[SKILL_ADVANCED_DEFENSE] = new spellInfo(SKILL_GENERAL, DISC_DEFENSE, DISC_DEFENSE, STAT_EXT, "advanced defense", TASK_EASY, LAG_0, POSITION_SITTING, MANA_0, LIFEFORCE_0, PRAY_0, 0, SYMBOL_STRESS_0, "", "", "", "", START_1, LEARN_1, START_DO_1, LEARN_DO_1, START_DO_NO, LEARN_DO_NO, LEARN_DIFF_UNUSUAL, 0.0, 0, 0);

  discArray[SKILL_FOCUSED_AVOIDANCE] = new spellInfo(SKILL_GENERAL, DISC_DEFENSE, DISC_DEFENSE, STAT_EXT, "focused avoidance", TASK_EASY, LAG_0, POSITION_SITTING, MANA_0, LIFEFORCE_0, PRAY_0, 0, SYMBOL_STRESS_0, "", "", "", "", START_1, LEARN_1, START_DO_1, LEARN_DO_1, START_DO_NO, LEARN_DO_NO, LEARN_DIFF_UNUSUAL, 0.0, 0, 0);

  discArray[SKILL_TOUGHNESS] = new spellInfo(SKILL_GENERAL, DISC_DEFENSE, DISC_DEFENSE, STAT_EXT, "toughness", TASK_EASY, LAG_0, POSITION_SITTING, MANA_0, LIFEFORCE_0, PRAY_0, 0, SYMBOL_STRESS_0, "", "", "", "", START_1, LEARN_1, START_DO_1, LEARN_DO_1, START_DO_NO, LEARN_DO_NO, LEARN_DIFF_UNUSUAL, 0.0, 0, 0);

  // disc_offense

  discArray[SKILL_ADVANCED_OFFENSE] = new spellInfo(SKILL_GENERAL, DISC_OFFENSE, DISC_OFFENSE, STAT_EXT, "advanced offense", TASK_EASY, LAG_0, POSITION_SITTING, MANA_0, LIFEFORCE_0, PRAY_0, 0, SYMBOL_STRESS_0, "", "", "", "", START_1, LEARN_1, START_DO_1, LEARN_DO_1, START_DO_NO, LEARN_DO_NO, LEARN_DIFF_UNUSUAL, 0.0, 0, 0);

  discArray[SKILL_INEVITABILITY] = new spellInfo(SKILL_GENERAL, DISC_OFFENSE, DISC_OFFENSE, STAT_EXT, "inevitability", TASK_EASY, LAG_0, POSITION_SITTING, MANA_0, LIFEFORCE_0, PRAY_0, 0, SYMBOL_STRESS_0, "", "", "", "", START_1, LEARN_1, START_DO_1, LEARN_DO_1, START_DO_NO, LEARN_DO_NO, LEARN_DIFF_UNUSUAL, 0.0, 0, 0);

  // disc_wizardry

  discArray[SKILL_WIZARDRY] = new spellInfo(SPELL_MAGE, DISC_WIZARDRY, DISC_WIZARDRY, STAT_EXT, "wizardry", TASK_EASY, LAG_0, POSITION_SLEEPING, MANA_0, LIFEFORCE_0, PRAY_0, 0, SYMBOL_STRESS_0, "", "", "", "", START_1, LEARN_1, START_DO_NO, LEARN_DO_NO, START_DO_NO, LEARN_DO_NO, LEARN_DIFF_UNUSUAL, 0.0, 0, 0);

// disc_lore
// disc_theology

  discArray[SKILL_ATTUNE] = new spellInfo(SKILL_CLERIC_TYPES, DISC_THEOLOGY, DISC_THEOLOGY, STAT_EXT, "attune", TASK_NORMAL, LAG_0, POSITION_SITTING, MANA_0, LIFEFORCE_0, PRAY_0, 0, SYMBOL_STRESS_0, "", "", "", "", START_10, LEARN_10, START_DO_50, LEARN_DO_2, START_DO_NO, LEARN_DO_NO, LEARN_DIFF_UNUSUAL, 0.0, SPELL_IGNORE_POSITION, 0);

  discArray[SKILL_MANA] = new spellInfo(SKILL_MAGE_TYPES, DISC_LORE, DISC_LORE, STAT_EXT, "mana", TASK_NORMAL, LAG_0, POSITION_RESTING, MANA_0, LIFEFORCE_0, PRAY_0, 0, SYMBOL_STRESS_0, "", "", "", "", START_1, LEARN_1, START_DO_1, LEARN_DO_1, START_DO_NO, LEARN_DO_NO, LEARN_DIFF_UNUSUAL, 0.0, SPELL_IGNORE_POSITION, 0);

  discArray[SKILL_MEDITATE] = new spellInfo(SKILL_MAGE_TYPES, DISC_LORE, DISC_LORE, STAT_EXT, "meditate", TASK_EASY, LAG_0, POSITION_RESTING, MANA_0, LIFEFORCE_0, PRAY_0, 0, SYMBOL_STRESS_0, "", "", "", "", START_1, LEARN_1, START_DO_1, LEARN_DO_1, START_DO_NO, LEARN_DO_NO, LEARN_DIFF_UNUSUAL, 0.0, SPELL_IGNORE_POSITION, 0);

  // SHAMAN STARTS HERE ***************************************************************************************

  // ritualism

  discArray[SKILL_RITUALISM] = new spellInfo(SPELL_SHAMAN, DISC_RITUALISM, DISC_RITUALISM, STAT_EXT, "ritualism", TASK_EASY, LAG_0, POSITION_SLEEPING, MANA_0, LIFEFORCE_0, PRAY_0, 0, SYMBOL_STRESS_0, "", "", "", "", START_1, LEARN_1, START_DO_NO, LEARN_DO_NO, START_DO_NO, LEARN_DO_NO, LEARN_DIFF_UNUSUAL, 0.0, 0, 0);

  // shaman basic

  discArray[SPELL_CHASE_SPIRIT] = new spellInfo(SPELL_SHAMAN, DISC_SHAMAN, DISC_SHAMAN_ARMADILLO, STAT_INT, "chase spirits", TASK_EASY, LAG_1, POSITION_SITTING, MANA_0, LIFEFORCE_70, PRAY_0, TAR_CHAR_ROOM | TAR_FIGHT_VICT | TAR_OBJ_INV | TAR_OBJ_ROOM, SYMBOL_STRESS_0, "", "", "", "", START_61, LEARN_10, START_DO_40, LEARN_DO_5, START_DO_NO, LEARN_DO_NO, LEARN_DIFF_SPELLS, 0.04, COMP_GESTURAL | COMP_GESTURAL_RANDOM | COMP_VERBAL | COMP_VERBAL_RANDOM | COMP_MATERIAL | COMP_MATERIAL_END | SPELL_TASKED, 0);

  discArray[SPELL_FLATULENCE] = new spellInfo(SPELL_SHAMAN, DISC_SHAMAN, DISC_SHAMAN_SKUNK, STAT_INT, "flatulence", TASK_EASY, LAG_1, POSITION_SITTING, MANA_0, LIFEFORCE_50, PRAY_0, TAR_AREA | TAR_FIGHT_VICT | TAR_VIOLENT | TAR_IGNORE, SYMBOL_STRESS_0, "", "", "", "", START_15, LEARN_15, START_DO_45, LEARN_DO_5, START_DO_NO, LEARN_DO_NO, LEARN_DIFF_SPELLS, 0.04, COMP_GESTURAL | COMP_GESTURAL_RANDOM | COMP_VERBAL | COMP_VERBAL_RANDOM | COMP_MATERIAL | COMP_MATERIAL_END | SPELL_TASKED, 0);

  discArray[SPELL_STUPIDITY] = new spellInfo(SPELL_SHAMAN, DISC_SHAMAN, DISC_SHAMAN_SPIDER, STAT_INT, "stupidity", TASK_EASY, LAG_2, POSITION_SITTING, MANA_0, LIFEFORCE_30, PRAY_0, TAR_VIOLENT | TAR_FIGHT_VICT | TAR_CHAR_ROOM | TAR_SELF_NONO | TAR_SELF_NONO, SYMBOL_STRESS_0, "You suddenly feel brighter.", "$n seems a little more intelligent than before.", "The fog surrounding you lifts a little.", "The fog surrounding $n lifts a little.", START_15, LEARN_14, START_DO_35, LEARN_DO_7, START_DO_NO, LEARN_DO_NO, LEARN_DIFF_SPELLS, 0.03, COMP_GESTURAL | COMP_GESTURAL_RANDOM | COMP_VERBAL | COMP_VERBAL_RANDOM | COMP_MATERIAL | COMP_MATERIAL_END | SPELL_TASKED, 0);

  discArray[SPELL_DISTORT] = new spellInfo(SPELL_SHAMAN, DISC_SHAMAN, DISC_SHAMAN_FROG, STAT_INT, "distort", TASK_NORMAL, LAG_1, POSITION_SITTING, MANA_0, LIFEFORCE_25, PRAY_0, TAR_CHAR_ROOM | TAR_SELF_NONO | TAR_VIOLENT | TAR_FIGHT_VICT , SYMBOL_STRESS_0, "", "", "", "", START_12, LEARN_25, START_DO_40, LEARN_DO_5, START_DO_NO, LEARN_DO_NO, LEARN_DIFF_SPELLS, 0.04, COMP_GESTURAL | COMP_GESTURAL_RANDOM | COMP_VERBAL | COMP_VERBAL_RANDOM | SPELL_TASKED, 0);

  discArray[SPELL_LEGBA] = new spellInfo(SPELL_SHAMAN, DISC_SHAMAN, DISC_SHAMAN_ARMADILLO, STAT_INT, "legba's guidance", TASK_EASY, LAG_2, POSITION_CRAWLING, MANA_0, LIFEFORCE_70, PRAY_0, TAR_FIGHT_SELF | TAR_CHAR_ROOM, SYMBOL_STRESS_0, "", "", "", "", START_80, LEARN_10, START_DO_30, LEARN_DO_5, START_DO_NO, LEARN_DO_NO, LEARN_DIFF_SPELLS, 0.04, COMP_GESTURAL | COMP_GESTURAL_RANDOM | COMP_VERBAL | COMP_VERBAL_RANDOM | COMP_MATERIAL | COMP_MATERIAL_END | SPELL_TASKED, 0);

  discArray[SPELL_DJALLA] = new spellInfo(SPELL_SHAMAN, DISC_SHAMAN, DISC_SHAMAN_ARMADILLO, STAT_INT, "djalla's protection", TASK_EASY, LAG_2, POSITION_CRAWLING, MANA_0, LIFEFORCE_60, PRAY_0, TAR_FIGHT_SELF | TAR_CHAR_ROOM, SYMBOL_STRESS_0, "", "", "", "", START_75, LEARN_10, START_DO_30, LEARN_DO_5, START_DO_NO, LEARN_DO_NO, LEARN_DIFF_SPELLS, 0.04, COMP_GESTURAL | COMP_GESTURAL_RANDOM | COMP_VERBAL | COMP_VERBAL_RANDOM | COMP_MATERIAL | COMP_MATERIAL_END | SPELL_TASKED, 0);

  discArray[SPELL_SENSE_LIFE_SHAMAN] = new spellInfo(SPELL_SHAMAN, DISC_SHAMAN, DISC_SHAMAN_ARMADILLO, STAT_INT, "sense presence", TASK_EASY, LAG_1, POSITION_CRAWLING, MANA_0, LIFEFORCE_30, PRAY_0, TAR_CHAR_ROOM | TAR_FIGHT_SELF, SYMBOL_STRESS_0, "The pale blue aura fades from your eyes.", "The pale blue aura fades from $n's eyes.", "The pale blue in your eyes flickers slightly.", "The pale blue in $n's eyes flickers slightly.", START_24, LEARN_18, START_DO_40, LEARN_DO_5, START_DO_NO, LEARN_DO_NO, LEARN_DIFF_SPELLS, 0.04, COMP_GESTURAL | COMP_GESTURAL_RANDOM | COMP_VERBAL | COMP_VERBAL_RANDOM | COMP_MATERIAL | COMP_MATERIAL_END | SPELL_TASKED, 0);

  discArray[SPELL_DETECT_SHADOW] = new spellInfo(SPELL_SHAMAN, DISC_SHAMAN, DISC_SHAMAN_ARMADILLO, STAT_INT, "detect shadow", TASK_EASY, LAG_2, POSITION_CRAWLING, MANA_0, LIFEFORCE_70, PRAY_0, TAR_CHAR_ROOM | TAR_FIGHT_SELF | TAR_CHAR_ROOM, SYMBOL_STRESS_0, "Your eyes sting a little.", "$n seems to be having some vision problems.", "", "", START_85, LEARN_10, START_DO_40, LEARN_DO_5, START_DO_NO, LEARN_DO_NO, LEARN_DIFF_SPELLS, 0.04, COMP_GESTURAL | COMP_GESTURAL_RANDOM | COMP_VERBAL | COMP_VERBAL_RANDOM | COMP_MATERIAL | COMP_MATERIAL_END | SPELL_TASKED, 0);

  discArray[SPELL_INTIMIDATE] = new spellInfo(SPELL_SHAMAN, DISC_SHAMAN, DISC_SHAMAN_SKUNK, STAT_INT, "intimidate", TASK_NORMAL, LAG_2, POSITION_SITTING, MANA_0, LIFEFORCE_40, PRAY_0, TAR_CHAR_ROOM | TAR_SELF_NONO | TAR_VIOLENT | TAR_FIGHT_VICT , SYMBOL_STRESS_0, "You are less intimidated.", "$n doesn't seem as intimidated as $e was before.", "", "", START_42, LEARN_11, START_DO_45, LEARN_DO_5, START_DO_NO, LEARN_DO_NO, LEARN_DIFF_SPELLS, 0.04, COMP_GESTURAL | COMP_GESTURAL_RANDOM | COMP_VERBAL | COMP_VERBAL_RANDOM | COMP_MATERIAL | COMP_MATERIAL_END | SPELL_TASKED, 0) ;

  discArray[SPELL_ROMBLER] = new spellInfo(SPELL_SHAMAN, DISC_SHAMAN, DISC_SHAMAN, STAT_INT, "romble", TASK_EASY, LAG_1, POSITION_CRAWLING, MANA_0, LIFEFORCE_20, PRAY_0, TAR_NAME, SYMBOL_STRESS_0, "", "", "", "", START_47, LEARN_15, START_DO_50, LEARN_DO_5, START_DO_NO, LEARN_DO_NO, LEARN_DIFF_SPELLS, 0.04, COMP_GESTURAL | COMP_GESTURAL_RANDOM | COMP_VERBAL | COMP_VERBAL_RANDOM | COMP_MATERIAL | COMP_MATERIAL_END | SPELL_TASKED, 0);

  discArray[SPELL_EMBALM] = new spellInfo(SPELL_SHAMAN, DISC_SHAMAN, DISC_SHAMAN, STAT_EXT, "embalm", TASK_EASY, LAG_1, POSITION_CRAWLING, MANA_0, LIFEFORCE_20, PRAY_0, TAR_OBJ_ROOM | TAR_SELF_NONO, SYMBOL_STRESS_0, "", "", "", "", START_47, LEARN_15, START_DO_50, LEARN_DO_5, START_DO_NO, LEARN_DO_NO, LEARN_DIFF_SPELLS, 0.04, COMP_GESTURAL | COMP_GESTURAL_RANDOM | COMP_VERBAL | COMP_VERBAL_RANDOM | COMP_MATERIAL | COMP_MATERIAL_END | SPELL_TASKED, 0);


  discArray[SKILL_SACRIFICE] = new spellInfo(SKILL_SHAMAN, DISC_SHAMAN, DISC_SHAMAN, STAT_EXT, "sacrifice", TASK_EASY, LAG_0, POSITION_SITTING, MANA_0, LIFEFORCE_0, PRAY_0, 0, SYMBOL_STRESS_0, "", "", "", "", START_1, LEARN_2, START_DO_35, LEARN_DO_2, START_DO_NO, LEARN_DO_NO, LEARN_DIFF_SKILLS, 0.0, 0, 0);

  discArray[SPELL_CHRISM] = new spellInfo(SPELL_SHAMAN, DISC_SHAMAN, DISC_SHAMAN_ALCHEMY, STAT_INT, "chrism", TASK_TRIVIAL, LAG_1, POSITION_CRAWLING, MANA_0, LIFEFORCE_40, PRAY_0, TAR_NAME, SYMBOL_STRESS_0, "", "", "", "", START_15, LEARN_25, START_DO_40, LEARN_DO_7, START_DO_NO, LEARN_DO_NO, LEARN_DIFF_SPELLS, 0.0, COMP_GESTURAL | COMP_GESTURAL_RANDOM | COMP_VERBAL | COMP_VERBAL_RANDOM | SPELL_TASKED, 0);

  discArray[SPELL_VAMPIRIC_TOUCH] = new spellInfo(SPELL_SHAMAN, DISC_SHAMAN, DISC_SHAMAN_SKUNK, STAT_INT, "vampiric touch", TASK_DIFFICULT, LAG_2, POSITION_SITTING, MANA_0, LIFEFORCE_80, PRAY_0, TAR_CHAR_ROOM | TAR_SELF_NONO | TAR_VIOLENT | TAR_FIGHT_VICT , SYMBOL_STRESS_0, "", "", "", "", START_80, LEARN_5, START_DO_50, LEARN_DO_2, START_DO_NO, LEARN_DO_NO, LEARN_DIFF_SPELLS, 0.04, COMP_GESTURAL | COMP_VERBAL | COMP_MATERIAL  | COMP_MATERIAL_END | SPELL_TASKED, 0);

  discArray[SPELL_CHEVAL] = new spellInfo(SPELL_SHAMAN, DISC_SHAMAN, DISC_SHAMAN_ARMADILLO, STAT_INT, "cheval", TASK_EASY, LAG_2, POSITION_CRAWLING, MANA_0, LIFEFORCE_60, PRAY_0, TAR_SELF_ONLY | TAR_FIGHT_SELF | TAR_CHAR_ROOM, SYMBOL_STRESS_0, "The loa are growing tired of your body.", "$n seems to be calming down.", "", "", START_30, LEARN_10, START_DO_40, LEARN_DO_5, START_DO_NO, LEARN_DO_NO, LEARN_DIFF_SPELLS, 0.04, COMP_GESTURAL | COMP_GESTURAL_RANDOM | COMP_VERBAL | COMP_VERBAL_RANDOM | COMP_MATERIAL | COMP_MATERIAL_END | SPELL_TASKED, 0);

  discArray[SPELL_SQUISH] = new spellInfo(SPELL_SHAMAN, DISC_SHAMAN, DISC_SHAMAN_SPIDER, STAT_INT, "squish", TASK_NORMAL, LAG_1, POSITION_SITTING, MANA_0, LIFEFORCE_50, PRAY_0, TAR_CHAR_ROOM | TAR_SELF_NONO | TAR_VIOLENT | TAR_FIGHT_VICT , SYMBOL_STRESS_0, "", "", "", "", START_15, LEARN_5, START_DO_40, LEARN_DO_1, START_DO_NO, LEARN_DO_NO, LEARN_DIFF_SPELLS, 0.04, COMP_GESTURAL | COMP_VERBAL | COMP_MATERIAL  | COMP_MATERIAL_END | SPELL_TASKED, 0);

  discArray[SPELL_SOUL_TWIST] = new spellInfo(SPELL_SHAMAN, DISC_SHAMAN, DISC_SHAMAN_SKUNK, STAT_INT, "soul twister", TASK_NORMAL, LAG_1, POSITION_SITTING, MANA_0, LIFEFORCE_60, PRAY_0, TAR_CHAR_ROOM | TAR_SELF_NONO | TAR_VIOLENT | TAR_FIGHT_VICT , SYMBOL_STRESS_0, "", "", "", "", START_60, LEARN_5, START_DO_40, LEARN_DO_1, START_DO_NO, LEARN_DO_NO, LEARN_DIFF_SPELLS, 0.04, COMP_GESTURAL | COMP_VERBAL | COMP_MATERIAL  | COMP_MATERIAL_END | SPELL_TASKED, 0);

  discArray[SPELL_VOODOO] = new spellInfo(SPELL_SHAMAN, DISC_SHAMAN, DISC_SHAMAN_CONTROL, STAT_EXT, "voodoo", TASK_EASY, LAG_2, POSITION_SITTING, MANA_0, LIFEFORCE_50, PRAY_0, TAR_OBJ_ROOM | TAR_SELF_NONO | TAR_VIOLENT | TAR_FIGHT_VICT , SYMBOL_STRESS_0, "", "", "", "", START_21, LEARN_8, START_DO_40, LEARN_DO_5, START_DO_NO, LEARN_DO_NO, LEARN_DIFF_SPELLS, 0.04, COMP_GESTURAL | COMP_VERBAL | COMP_MATERIAL  | COMP_MATERIAL_END | SPELL_TASKED, 0);

  discArray[SPELL_DANCING_BONES] = new spellInfo(SPELL_SHAMAN, DISC_SHAMAN, DISC_SHAMAN_CONTROL, STAT_EXT, "dancing bones", TASK_EASY, LAG_3, POSITION_SITTING, MANA_0, LIFEFORCE_110, PRAY_0, TAR_OBJ_ROOM | TAR_SELF_NONO | TAR_VIOLENT | TAR_FIGHT_VICT , SYMBOL_STRESS_0, "", "", "", "", START_70, LEARN_8, START_DO_40, LEARN_DO_5, START_DO_NO, LEARN_DO_NO, LEARN_DIFF_SPELLS, 0.04, COMP_GESTURAL | COMP_VERBAL | COMP_MATERIAL  | COMP_MATERIAL_END | SPELL_TASKED, 0);

  discArray[SPELL_SHIELD_OF_MISTS] = new spellInfo(SPELL_SHAMAN, DISC_SHAMAN, DISC_SHAMAN, STAT_INT, "shield of mists", TASK_EASY, LAG_1, POSITION_CRAWLING, MANA_0, LIFEFORCE_20, PRAY_0, TAR_CHAR_ROOM | TAR_FIGHT_SELF, SYMBOL_STRESS_0, "The mist surrounding you has faded into memory.", "The mist surrounding $n fades to memory.", "The mist surrounding you starts to become transparent.", "The mist surrounding $n disperses.", START_1, LEARN_8, START_DO_50, LEARN_DO_5, START_DO_NO, LEARN_DO_NO, LEARN_DIFF_SPELLS, 0.01, COMP_GESTURAL | COMP_GESTURAL_RANDOM | COMP_VERBAL | COMP_VERBAL_RANDOM | COMP_MATERIAL | COMP_MATERIAL_END | SPELL_TASKED, 0);

  discArray[SPELL_LIFE_LEECH] = new spellInfo(SPELL_SHAMAN, DISC_SHAMAN, DISC_SHAMAN_SPIDER, STAT_INT, "life leech", TASK_EASY, LAG_1, POSITION_SITTING, MANA_0, LIFEFORCE_20, PRAY_0, TAR_CHAR_ROOM | TAR_SELF_NONO | TAR_VIOLENT | TAR_FIGHT_VICT , SYMBOL_STRESS_0, "", "", "", "", START_3, LEARN_5, START_DO_25, LEARN_DO_1, START_DO_NO, LEARN_DO_NO, LEARN_DIFF_SPELLS, 0.04, COMP_GESTURAL | COMP_VERBAL | COMP_MATERIAL  | COMP_MATERIAL_END | SPELL_TASKED, 0);

  discArray[SPELL_ENTHRALL_SPECTRE] = new spellInfo(SPELL_SHAMAN, DISC_SHAMAN, DISC_SHAMAN_CONTROL, STAT_EXT, "enthrall spectre", TASK_EASY, LAG_1, POSITION_CRAWLING, MANA_0, LIFEFORCE_70, PRAY_0, TAR_IGNORE, SYMBOL_STRESS_0, "", "", "", "", START_35, LEARN_15, START_DO_50, LEARN_DO_5, START_DO_NO, LEARN_DO_NO, LEARN_DIFF_SPELLS, 0.00, COMP_GESTURAL | COMP_GESTURAL_RANDOM | COMP_VERBAL | COMP_VERBAL_RANDOM | COMP_MATERIAL | COMP_MATERIAL_END | SPELL_TASKED, 0);

  discArray[SPELL_ENTHRALL_GHAST] = new spellInfo(SPELL_SHAMAN, DISC_SHAMAN, DISC_SHAMAN_CONTROL, STAT_EXT, "enthrall ghast", TASK_EASY, LAG_1, POSITION_CRAWLING, MANA_0, LIFEFORCE_90, PRAY_0, TAR_IGNORE, SYMBOL_STRESS_0, "", "", "", "", START_57, LEARN_13, START_DO_50, LEARN_DO_5, START_DO_NO, LEARN_DO_NO, LEARN_DIFF_SPELLS, 0.0, COMP_GESTURAL | COMP_GESTURAL_RANDOM | COMP_VERBAL | COMP_VERBAL_RANDOM | COMP_MATERIAL | COMP_MATERIAL_END | SPELL_TASKED, 0);

  discArray[SPELL_ENTHRALL_GHOUL] = new spellInfo(SPELL_SHAMAN, DISC_SHAMAN, DISC_SHAMAN_CONTROL, STAT_EXT, "enthrall ghoul", TASK_EASY, LAG_1, POSITION_CRAWLING, MANA_0, LIFEFORCE_120, PRAY_0, TAR_IGNORE, SYMBOL_STRESS_0, "", "", "", "", START_80, LEARN_15, START_DO_50, LEARN_DO_5, START_DO_NO, LEARN_DO_NO, LEARN_DIFF_SPELLS, 0.0, COMP_GESTURAL | COMP_GESTURAL_RANDOM | COMP_VERBAL | COMP_VERBAL_RANDOM | COMP_MATERIAL | COMP_MATERIAL_END | SPELL_TASKED, 0);

  discArray[SKILL_REPAIR_SHAMAN] = new spellInfo(SKILL_SHAMAN, DISC_SHAMAN, DISC_BLACKSMITHING, STAT_EXT, "shaman repair", TASK_EASY, LAG_0, POSITION_SITTING, MANA_0, LIFEFORCE_0, PRAY_0, 0, SYMBOL_STRESS_0, "", "", "", "", START_50, LEARN_3, START_DO_1, LEARN_DO_1, START_DO_NO, LEARN_DO_NO, LEARN_DIFF_SKILLS, 0.0, 0, 0);

  // shaman control

  discArray[SPELL_ENTHRALL_DEMON] = new spellInfo(SPELL_SHAMAN, DISC_SHAMAN_CONTROL, DISC_SHAMAN_CONTROL, STAT_EXT, "enthrall demon", TASK_EASY, LAG_1, POSITION_CRAWLING, MANA_0, LIFEFORCE_150, PRAY_0, TAR_IGNORE, SYMBOL_STRESS_0, "", "", "", "", START_41, LEARN_10, START_DO_50, LEARN_DO_5, START_DO_NO, LEARN_DO_NO, LEARN_DIFF_SPELLS, 0.0, COMP_GESTURAL | COMP_GESTURAL_RANDOM | COMP_VERBAL | COMP_VERBAL_RANDOM | COMP_MATERIAL | COMP_MATERIAL_END | SPELL_TASKED, 0);

  discArray[SPELL_CREATE_WOOD_GOLEM] = new spellInfo(SPELL_SHAMAN, DISC_SHAMAN_CONTROL, DISC_SHAMAN_CONTROL, STAT_EXT, "create wood golem", TASK_NORMAL, LAG_1, POSITION_CRAWLING, MANA_0, LIFEFORCE_170, PRAY_0, TAR_IGNORE, SYMBOL_STRESS_0, "", "", "", "", START_60, LEARN_10, START_DO_50, LEARN_DO_5, START_DO_NO, LEARN_DO_NO, LEARN_DIFF_SPELLS, 0.0, COMP_GESTURAL | COMP_GESTURAL_RANDOM | COMP_VERBAL | COMP_VERBAL_RANDOM | COMP_MATERIAL | COMP_MATERIAL_END | SPELL_TASKED, 0);

  discArray[SPELL_CREATE_ROCK_GOLEM] = new spellInfo(SPELL_SHAMAN, DISC_SHAMAN_CONTROL, DISC_SHAMAN_CONTROL, STAT_EXT, "create rock golem", TASK_NORMAL, LAG_1, POSITION_CRAWLING, MANA_0, LIFEFORCE_190, PRAY_0, TAR_IGNORE, SYMBOL_STRESS_0, "", "", "", "", START_70, LEARN_10, START_DO_50, LEARN_DO_5, START_DO_NO, LEARN_DO_NO, LEARN_DIFF_SPELLS, 0.0, COMP_GESTURAL | COMP_GESTURAL_RANDOM | COMP_VERBAL | COMP_VERBAL_RANDOM | COMP_MATERIAL | COMP_MATERIAL_END | SPELL_TASKED, 0);

  discArray[SPELL_CREATE_IRON_GOLEM] = new spellInfo(SPELL_SHAMAN, DISC_SHAMAN_CONTROL, DISC_SHAMAN_CONTROL, STAT_EXT, "create iron golem", TASK_NORMAL, LAG_1, POSITION_CRAWLING, MANA_0, LIFEFORCE_190, PRAY_0, TAR_IGNORE, SYMBOL_STRESS_0, "", "", "", "", START_75, LEARN_10, START_DO_50, LEARN_DO_5, START_DO_NO, LEARN_DO_NO, LEARN_DIFF_SPELLS, 0.0, COMP_GESTURAL | COMP_GESTURAL_RANDOM | COMP_VERBAL | COMP_VERBAL_RANDOM | COMP_MATERIAL | COMP_MATERIAL_END | SPELL_TASKED, 0);

  discArray[SPELL_CREATE_DIAMOND_GOLEM] = new spellInfo(SPELL_SHAMAN, DISC_SHAMAN_CONTROL, DISC_SHAMAN_CONTROL, STAT_EXT, "create diamond golem", TASK_NORMAL, LAG_1, POSITION_CRAWLING, MANA_0, LIFEFORCE_210, PRAY_0, TAR_IGNORE, SYMBOL_STRESS_0, "", "", "", "", START_85, LEARN_10, START_DO_50, LEARN_DO_5, START_DO_NO, LEARN_DO_NO, LEARN_DIFF_SPELLS, 0.0, COMP_GESTURAL | COMP_GESTURAL_RANDOM | COMP_VERBAL | COMP_VERBAL_RANDOM | COMP_MATERIAL | COMP_MATERIAL_END | SPELL_TASKED, 0);

  discArray[SPELL_RESURRECTION] = new spellInfo(SPELL_SHAMAN, DISC_SHAMAN_CONTROL, DISC_SHAMAN_CONTROL, STAT_EXT, "resurrection", TASK_DANGEROUS, LAG_5, POSITION_SITTING, MANA_0, LIFEFORCE_330, PRAY_0, TAR_OBJ_ROOM | TAR_SELF_NONO | TAR_VIOLENT | TAR_FIGHT_VICT , SYMBOL_STRESS_0, "", "", "", "", START_80, LEARN_5, START_DO_30, LEARN_DO_5, START_DO_NO, LEARN_DO_NO, LEARN_DIFF_SPELLS, 0.04, COMP_GESTURAL | COMP_VERBAL | COMP_MATERIAL  | COMP_MATERIAL_END | SPELL_TASKED, 0);

  // shaman spider

  discArray[SPELL_ROOT_CONTROL] = new spellInfo(SPELL_SHAMAN, DISC_SHAMAN_SPIDER, DISC_SHAMAN_SPIDER, STAT_INT, "root control", TASK_NORMAL, LAG_3, POSITION_SITTING, MANA_15, LIFEFORCE_0, PRAY_0, TAR_CHAR_ROOM | TAR_VIOLENT | TAR_FIGHT_VICT | TAR_SELF_NONO, SYMBOL_STRESS_0, "", "", "", "", START_1, LEARN_5, START_DO_20, LEARN_DO_5, START_DO_NO, LEARN_DO_NO, LEARN_DIFF_SPELLS, 0.04, COMP_GESTURAL | COMP_VERBAL | COMP_MATERIAL, 0);


  discArray[SKILL_TRANSFIX] = new spellInfo(SPELL_SHAMAN, DISC_SHAMAN_SPIDER, DISC_SHAMAN_SPIDER, STAT_INT, "transfix", TASK_DIFFICULT, LAG_3, POSITION_FIGHTING, MANA_0, LIFEFORCE_0, PRAY_0, TAR_SELF_NONO | TAR_CHAR_ROOM, SYMBOL_STRESS_0, "You're not transfixed anymore.", "$n doesn't seem to be transfixed anymore.", "You seem slightly more like yourself.", "$n seems slightly more like $mself.", START_21, LEARN_5, START_DO_20, LEARN_DO_5, START_DO_NO, LEARN_DO_NO, LEARN_DIFF_SPELLS, 0.02, COMP_VERBAL | COMP_MATERIAL, 0); 

  discArray[SPELL_LIVING_VINES] = new spellInfo(SPELL_SHAMAN, DISC_SHAMAN_SPIDER, DISC_SHAMAN_SPIDER, STAT_INT, "living vines", TASK_NORMAL, LAG_3, POSITION_SITTING, MANA_0, LIFEFORCE_0, PRAY_0, TAR_CHAR_ROOM, SYMBOL_STRESS_0, "", "", "", "", START_41, LEARN_4, START_DO_40, LEARN_DO_5, START_DO_NO, LEARN_DO_NO, LEARN_DIFF_SPELLS, 0.04, COMP_GESTURAL | COMP_VERBAL | COMP_MATERIAL, 0);



  discArray[SPELL_RAZE] = new spellInfo(SPELL_SHAMAN, DISC_SHAMAN_SPIDER, DISC_SHAMAN_SPIDER, STAT_INT, "raze", TASK_DANGEROUS, LAG_3, POSITION_SITTING, MANA_0, LIFEFORCE_400, PRAY_0, TAR_CHAR_ROOM | TAR_SELF_NONO | TAR_VIOLENT | TAR_FIGHT_VICT , SYMBOL_STRESS_0, "", "", "", "", START_80, LEARN_5, START_DO_35, LEARN_DO_5, START_DO_NO, LEARN_DO_NO, LEARN_DIFF_SPELLS, 0.04, COMP_GESTURAL | COMP_VERBAL | COMP_MATERIAL | SPELL_TASKED | COMP_MATERIAL_INIT, 0);

  discArray[SPELL_STICKS_TO_SNAKES] = new spellInfo(SPELL_SHAMAN, DISC_SHAMAN_SPIDER, DISC_SHAMAN_SPIDER, STAT_INT, "sticks to snakes", TASK_NORMAL, LAG_3, POSITION_SITTING, MANA_0, LIFEFORCE_130, PRAY_0, TAR_CHAR_ROOM | TAR_SELF_NONO | TAR_VIOLENT | TAR_FIGHT_VICT , SYMBOL_STRESS_0, "", "", "", "", START_1, LEARN_5, START_DO_NO, LEARN_DO_5, START_DO_NO, LEARN_DO_NO, LEARN_DIFF_SPELLS, 0.04, COMP_GESTURAL | COMP_VERBAL | COMP_MATERIAL | SPELL_TASKED | COMP_MATERIAL_INIT, 0);

  discArray[SPELL_HYPNOSIS] = new spellInfo(SPELL_SHAMAN, DISC_SHAMAN_SPIDER, DISC_SHAMAN_SPIDER, STAT_CHA, "hypnosis", TASK_NORMAL, LAG_1, POSITION_CRAWLING, MANA_0, LIFEFORCE_300, PRAY_0, TAR_CHAR_ROOM | TAR_SELF_NONO | TAR_FIGHT_VICT | TAR_VIOLENT, SYMBOL_STRESS_0, "You shake your head really hard and free yourself from the hypnosis.", "$n shakes $s head really hard and frees $mself from the hypnosis.", "You momentarily become cognizant of the world around you.", "You see a brief flicker of intelligence in $n's eyes.", START_50, LEARN_11, START_DO_40, LEARN_DO_5, START_DO_NO, LEARN_DO_NO, LEARN_DIFF_SPELLS, 0.04, COMP_GESTURAL | COMP_GESTURAL_RANDOM | COMP_VERBAL | COMP_VERBAL_RANDOM | COMP_MATERIAL | COMP_MATERIAL_END | SPELL_TASKED, 0);

  discArray[SPELL_CLARITY] = new spellInfo(SPELL_SHAMAN, DISC_SHAMAN_SPIDER, DISC_SHAMAN_SPIDER, STAT_INT, "clarity", TASK_EASY, LAG_1, POSITION_CRAWLING, MANA_0, LIFEFORCE_150, PRAY_0, TAR_CHAR_ROOM | TAR_SELF_ONLY, SYMBOL_STRESS_0, "The green film on your eyes disolves.", "The green film on $n's eyes disolves.", "The film in your eyes becomes more transparent.", "", START_10, LEARN_8, START_DO_40, LEARN_DO_5, START_DO_NO, LEARN_DO_NO, LEARN_DIFF_SPELLS, 0.04, COMP_GESTURAL | COMP_GESTURAL_RANDOM | COMP_VERBAL | COMP_VERBAL_RANDOM | COMP_MATERIAL | COMP_MATERIAL_END | SPELL_TASKED, 0);

  discArray[SPELL_CONTROL_UNDEAD] = new spellInfo(SPELL_SHAMAN, DISC_SHAMAN_SPIDER, DISC_SHAMAN_SPIDER, STAT_INT, "control undead", TASK_NORMAL, LAG_1, POSITION_CRAWLING, MANA_0, LIFEFORCE_240, PRAY_0, TAR_CHAR_ROOM | TAR_SELF_NONO, SYMBOL_STRESS_0, "", "", "", "", START_70, LEARN_5, START_DO_30, LEARN_DO_3, START_DO_NO, LEARN_DO_NO, LEARN_DIFF_SPELLS, 0.04, COMP_GESTURAL | COMP_GESTURAL_RANDOM | COMP_VERBAL | COMP_VERBAL_RANDOM | COMP_MATERIAL | COMP_MATERIAL_END | SPELL_TASKED, 0);

  // shaman frog

  discArray[SKILL_TRANSFORM_LIMB] = new spellInfo(SPELL_SHAMAN, DISC_SHAMAN_FROG, DISC_SHAMAN_FROG, STAT_EXT, "transform limb", TASK_NORMAL, LAG_3, POSITION_CRAWLING, MANA_0, LIFEFORCE_0, PRAY_0, TAR_NAME, SYMBOL_STRESS_0, "Your limbs tingles as the transforming magic starts to leave your body.", "$n'slimbs shimmer and then start to transform back into their original form.", "Your limbs start to tingle as the magic ones momentarily resemble their original form.", "$n's limbs shimmer and momentarily resemble their original form.", START_21, LEARN_4, START_DO_20, LEARN_DO_5, START_DO_NO, LEARN_DO_NO, LEARN_DIFF_SPELLS, 0.04, COMP_MATERIAL, 0);


  discArray[SPELL_CREEPING_DOOM] = new spellInfo(SPELL_SHAMAN, DISC_SHAMAN_FROG, DISC_SHAMAN_FROG, STAT_INT, "creeping doom", TASK_NORMAL, LAG_3, POSITION_SITTING, MANA_10, LIFEFORCE_0, PRAY_0, TAR_CHAR_ROOM | TAR_SELF_NONO | TAR_FIGHT_VICT | TAR_VIOLENT, SYMBOL_STRESS_0, "", "", "", "", START_41, LEARN_4, START_DO_40, LEARN_DO_5, START_DO_NO, LEARN_DO_NO, LEARN_DIFF_SPELLS, 0.04, COMP_GESTURAL | COMP_VERBAL | COMP_MATERIAL, 0);

  discArray[SPELL_STORMY_SKIES] = new spellInfo(SPELL_SHAMAN, DISC_SHAMAN_FROG, DISC_SHAMAN_FROG, STAT_INT, "stormy skies", TASK_DANGEROUS, LAG_4, POSITION_SITTING, MANA_0, LIFEFORCE_170, PRAY_0, TAR_CHAR_ROOM | TAR_SELF_NONO | TAR_VIOLENT | TAR_FIGHT_VICT , SYMBOL_STRESS_0, "", "", "", "", START_30, LEARN_3, START_DO_40, LEARN_DO_5, START_DO_NO, LEARN_DO_NO, LEARN_DIFF_SPELLS, 0.05, COMP_GESTURAL | COMP_GESTURAL_RANDOM | COMP_VERBAL | COMP_VERBAL_RANDOM | COMP_MATERIAL | COMP_MATERIAL_END | SPELL_TASKED, 0);

  discArray[SPELL_AQUATIC_BLAST] = new spellInfo(SPELL_SHAMAN, DISC_SHAMAN_FROG, DISC_SHAMAN_FROG, STAT_INT, "aquatic blast", TASK_DANGEROUS, LAG_4, POSITION_SITTING, MANA_0, LIFEFORCE_230, PRAY_0, TAR_CHAR_ROOM | TAR_SELF_NONO | TAR_VIOLENT | TAR_FIGHT_VICT , SYMBOL_STRESS_0, "", "", "", "", START_50, LEARN_20, START_DO_40, LEARN_DO_5, START_DO_NO, LEARN_DO_NO, LEARN_DIFF_SPELLS, 0.05, COMP_GESTURAL | COMP_GESTURAL_RANDOM | COMP_VERBAL | COMP_VERBAL_RANDOM | COMP_MATERIAL | COMP_MATERIAL_END | SPELL_TASKED, 0);

  discArray[SPELL_DEATHWAVE] = new spellInfo(SPELL_SHAMAN, DISC_SHAMAN_FROG, DISC_SHAMAN_FROG, STAT_INT, "death wave", TASK_DANGEROUS, LAG_4, POSITION_SITTING, MANA_0, LIFEFORCE_160, PRAY_0, TAR_CHAR_ROOM | TAR_SELF_NONO | TAR_VIOLENT | TAR_FIGHT_VICT , SYMBOL_STRESS_0, "", "", "", "", START_75, LEARN_20, START_DO_40, LEARN_DO_5, START_DO_NO, LEARN_DO_NO, LEARN_DIFF_SPELLS, 0.05, COMP_GESTURAL | COMP_GESTURAL_RANDOM | COMP_VERBAL | COMP_VERBAL_RANDOM | COMP_MATERIAL | COMP_MATERIAL_END | SPELL_TASKED, 0);

  discArray[SPELL_SHAPESHIFT] = new spellInfo(SPELL_SHAMAN, DISC_SHAMAN_FROG, DISC_SHAMAN_FROG, STAT_INT, "shapeshift", TASK_NORMAL, LAG_2, POSITION_CRAWLING, MANA_0, LIFEFORCE_200, PRAY_0, TAR_NAME, SYMBOL_STRESS_0, "You are not able to hold this form any longer.", "", "You do not know how much longer you can hold this form.", "", START_5, LEARN_10, START_DO_50, LEARN_DO_4, START_DO_NO, LEARN_DO_NO, LEARN_DIFF_SPELLS, 0.0, COMP_GESTURAL | COMP_GESTURAL_RANDOM | COMP_VERBAL | COMP_VERBAL_RANDOM | COMP_MATERIAL | COMP_MATERIAL_END | SPELL_TASKED, 0);


  // shaman skunk

  discArray[SPELL_BLOOD_BOIL] = new spellInfo(SPELL_SHAMAN, DISC_SHAMAN_SKUNK, DISC_SHAMAN_SKUNK, STAT_INT, "boiling blood", TASK_NORMAL, LAG_2, POSITION_SITTING, MANA_0, LIFEFORCE_150, PRAY_0, TAR_CHAR_ROOM | TAR_SELF_NONO | TAR_VIOLENT | TAR_FIGHT_VICT , SYMBOL_STRESS_0, "", "","", "", START_20, LEARN_50, START_DO_45, LEARN_DO_5, START_DO_NO, LEARN_DO_NO, LEARN_DIFF_SPELLS, 0.04, COMP_GESTURAL | COMP_GESTURAL_RANDOM | COMP_VERBAL | COMP_VERBAL_RANDOM | COMP_MATERIAL | COMP_MATERIAL_END | SPELL_TASKED, 0);

  discArray[SPELL_CLEANSE] = new spellInfo(SPELL_SHAMAN, DISC_SHAMAN_SKUNK, DISC_SHAMAN_SKUNK, STAT_INT, "cleanse", TASK_EASY, LAG_2, POSITION_SITTING, MANA_0, LIFEFORCE_300, PRAY_0, TAR_CHAR_ROOM | TAR_FIGHT_SELF, SYMBOL_STRESS_0, "", "", "", "", START_1, LEARN_5, START_DO_40, LEARN_DO_10, START_DO_NO, LEARN_DO_NO, LEARN_DIFF_SPELLS, 0.04, COMP_GESTURAL | COMP_GESTURAL_RANDOM | COMP_VERBAL | COMP_VERBAL_RANDOM | COMP_MATERIAL | COMP_MATERIAL_END | SPELL_TASKED, 0);

  discArray[SPELL_DEATH_MIST] = new spellInfo(SPELL_SHAMAN, DISC_SHAMAN_SKUNK, DISC_SHAMAN_SKUNK, STAT_INT, "death mist", TASK_NORMAL, LAG_3, POSITION_SITTING, MANA_0, LIFEFORCE_260, PRAY_0, TAR_AREA | TAR_FIGHT_VICT | TAR_VIOLENT | TAR_IGNORE, SYMBOL_STRESS_0, "", "", "", "", START_25, LEARN_5, START_DO_50, LEARN_DO_5, START_DO_NO, LEARN_DO_NO, LEARN_DIFF_SPELLS, 0.04, COMP_GESTURAL | COMP_GESTURAL_RANDOM | COMP_VERBAL | COMP_VERBAL_RANDOM | COMP_MATERIAL | COMP_MATERIAL_END | SPELL_TASKED, 0);

  discArray[SPELL_CARDIAC_STRESS] = new spellInfo(SPELL_SHAMAN, DISC_SHAMAN_SKUNK, DISC_SHAMAN_SKUNK, STAT_INT, "coronary", TASK_DANGEROUS, LAG_4, POSITION_SITTING, MANA_0, LIFEFORCE_240, PRAY_0, TAR_CHAR_ROOM | TAR_SELF_NONO | TAR_VIOLENT | TAR_FIGHT_VICT , SYMBOL_STRESS_0, "", "", "", "", START_75, LEARN_20, START_DO_40, LEARN_DO_5, START_DO_NO, LEARN_DO_NO, LEARN_DIFF_SPELLS, 0.05, COMP_GESTURAL | COMP_GESTURAL_RANDOM | COMP_VERBAL | COMP_VERBAL_RANDOM | COMP_MATERIAL | COMP_MATERIAL_END | SPELL_TASKED, 0);

  discArray[SPELL_LICH_TOUCH] = new spellInfo(SPELL_SHAMAN, DISC_SHAMAN_SKUNK, DISC_SHAMAN_SKUNK, STAT_INT, "lich touch", TASK_DANGEROUS, LAG_3, POSITION_SITTING, MANA_0, LIFEFORCE_120, PRAY_0, TAR_CHAR_ROOM | TAR_SELF_NONO | TAR_VIOLENT | TAR_FIGHT_VICT , SYMBOL_STRESS_0, "", "", "", "", START_50, LEARN_5, START_DO_50, LEARN_DO_2, START_DO_NO, LEARN_DO_NO, LEARN_DIFF_SPELLS, 0.04, COMP_GESTURAL | COMP_VERBAL | COMP_MATERIAL  | COMP_MATERIAL_END | SPELL_TASKED, 0);

  discArray[SKILL_TURN] = new spellInfo(SKILL_SHAMAN, DISC_SHAMAN_SKUNK, DISC_SHAMAN_SKUNK, STAT_EXT, "turn", TASK_NORMAL, LAG_3, POSITION_STANDING, MANA_0, LIFEFORCE_0, PRAY_0, 0, SYMBOL_STRESS_0, "", "", "", "", START_86, LEARN_8, START_DO_1, LEARN_DO_1, START_DO_NO, LEARN_DO_NO, LEARN_DIFF_SKILLS, 0.0, 0, 0);


  // shaman armadillo

  discArray[SPELL_EARTHMAW] = new spellInfo(SPELL_SHAMAN, DISC_SHAMAN_ARMADILLO, DISC_SHAMAN_ARMADILLO, STAT_INT, "earthmaw", TASK_NORMAL, LAG_3, POSITION_SITTING, MANA_10, LIFEFORCE_0, PRAY_0, TAR_CHAR_ROOM | TAR_SELF_NONO | TAR_FIGHT_VICT | TAR_VIOLENT, SYMBOL_STRESS_0, "", "", "", "", START_41, LEARN_4, START_DO_40, LEARN_DO_5, START_DO_NO, LEARN_DO_NO, LEARN_DIFF_SPELLS, 0.04, COMP_GESTURAL | COMP_VERBAL | COMP_MATERIAL, 0);

  discArray[SPELL_CELERITE] = new spellInfo(SPELL_SHAMAN, DISC_SHAMAN_ARMADILLO, DISC_SHAMAN_ARMADILLO, STAT_INT, "celerite", TASK_EASY, LAG_2, POSITION_CRAWLING, MANA_0, LIFEFORCE_260, PRAY_0, TAR_SELF_ONLY | TAR_FIGHT_SELF | TAR_CHAR_ROOM, SYMBOL_STRESS_0, "You dont seem to be as protected as you once were.", "$n seems to be slowing down.", "", "", START_30, LEARN_5, START_DO_40, LEARN_DO_5, START_DO_NO, LEARN_DO_NO, LEARN_DIFF_SPELLS, 0.04, COMP_GESTURAL | COMP_GESTURAL_RANDOM | COMP_VERBAL | COMP_VERBAL_RANDOM | COMP_MATERIAL | COMP_MATERIAL_END | SPELL_TASKED, 0);

  discArray[SPELL_AQUALUNG] = new spellInfo(SPELL_SHAMAN, DISC_SHAMAN_ARMADILLO, DISC_SHAMAN_ARMADILLO, STAT_INT, "aqualung", TASK_EASY, LAG_2, POSITION_CRAWLING, MANA_0, LIFEFORCE_100, PRAY_0, TAR_CHAR_ROOM | TAR_FIGHT_SELF, SYMBOL_STRESS_0, "The transparent globe around your head disappears.", "The transparent globe around $n's head disappears.", "You begin to wonder how much longer you can breathe water.", "", START_1, LEARN_13, START_DO_45, LEARN_DO_5, START_DO_NO, LEARN_DO_NO, LEARN_DIFF_SPELLS, 0.02, COMP_GESTURAL | COMP_GESTURAL_RANDOM | COMP_VERBAL | COMP_VERBAL_RANDOM | COMP_MATERIAL | COMP_MATERIAL_END | SPELL_TASKED, 0);

  discArray[SPELL_THORNFLESH] = new spellInfo(SPELL_SHAMAN, DISC_SHAMAN_ARMADILLO, DISC_SHAMAN_ARMADILLO, STAT_INT, "thornflesh", TASK_EASY, LAG_3, POSITION_CRAWLING, MANA_0, LIFEFORCE_300, PRAY_0, TAR_SELF_ONLY | TAR_FIGHT_SELF | TAR_CHAR_ROOM, SYMBOL_STRESS_0, "The thorns on your body fade away.", "The thorns on $n's body start to fade.", "The thorns on your body start to fade.", "", START_40, LEARN_4, START_DO_45, LEARN_DO_2, START_DO_NO, LEARN_DO_NO, LEARN_DIFF_SPELLS, 0.02, COMP_GESTURAL | COMP_GESTURAL_RANDOM | COMP_VERBAL | COMP_VERBAL_RANDOM | COMP_MATERIAL | COMP_MATERIAL_END | SPELL_TASKED, 0);

  discArray[SPELL_SHADOW_WALK] = new spellInfo(SPELL_SHAMAN, DISC_SHAMAN_ARMADILLO, DISC_SHAMAN_ARMADILLO, STAT_INT, "shadow walk", TASK_EASY, LAG_2, POSITION_CRAWLING, MANA_0, LIFEFORCE_300, PRAY_0, TAR_CHAR_ROOM | TAR_SELF_ONLY, SYMBOL_STRESS_0, "You no longer walk in the shadows.", "$n blinks into view.", "", "", START_60, LEARN_8, START_DO_45, LEARN_DO_5, START_DO_NO, LEARN_DO_NO, LEARN_DIFF_SPELLS, 0.04, COMP_GESTURAL | COMP_GESTURAL_RANDOM | COMP_VERBAL | COMP_VERBAL_RANDOM | COMP_MATERIAL | COMP_MATERIAL_END | SPELL_TASKED, 0);

  // shaman alchemy

  discArray[SKILL_BREW] = new spellInfo(SPELL_SHAMAN, DISC_SHAMAN_ALCHEMY, DISC_SHAMAN_ALCHEMY, STAT_EXT, "brew", TASK_EASY, LAG_0, POSITION_STANDING, MANA_0, LIFEFORCE_0, PRAY_0, 0, SYMBOL_STRESS_0, "", "", "", "", START_1, LEARN_1, START_DO_1, LEARN_DO_5, START_DO_NO, LEARN_DO_NO, LEARN_DIFF_UNUSUAL, 0.0, 0, 0);

  // healing abilities
  discArray[SPELL_HEALING_GRASP] = new spellInfo(SPELL_SHAMAN, DISC_SHAMAN_HEALING, DISC_SHAMAN_HEALING, STAT_INT, "healing grasp", TASK_EASY, LAG_2, POSITION_SITTING, MANA_0, LIFEFORCE_100, PRAY_0, TAR_CHAR_ROOM | TAR_FIGHT_SELF, SYMBOL_STRESS_0, "", "", "", "", START_1, LEARN_1, START_DO_35, LEARN_DO_5, START_DO_NO, LEARN_DO_NO, LEARN_DIFF_SPELLS, 0.02, COMP_GESTURAL | COMP_GESTURAL_RANDOM | COMP_VERBAL | COMP_VERBAL_RANDOM | COMP_MATERIAL | COMP_MATERIAL_END | SPELL_TASKED | SPELL_TASKED_EVERY, 0);

  discArray[SPELL_ENLIVEN] = new spellInfo(SPELL_SHAMAN, DISC_SHAMAN_HEALING, DISC_SHAMAN_HEALING, STAT_INT, "enliven", TASK_EASY, LAG_2, POSITION_CRAWLING, MANA_0, LIFEFORCE_150, PRAY_0, TAR_CHAR_ROOM | TAR_FIGHT_SELF, SYMBOL_STRESS_0, "You don't seem as lively.", "$n doesn't seem as lively.", "", "", START_1, LEARN_17, START_DO_50, LEARN_DO_5, START_DO_NO, LEARN_DO_NO, LEARN_DIFF_SPELLS, 0.04, COMP_GESTURAL | COMP_GESTURAL_RANDOM | COMP_VERBAL | COMP_VERBAL_RANDOM | COMP_MATERIAL | COMP_MATERIAL_END | SPELL_TASKED, 0);


  // SHAMAN END HERE ***********************************************************************************




  // disc_piety

  discArray[SKILL_DEVOTION] = new spellInfo(SKILL_CLERIC_TYPES, DISC_FAITH, DISC_FAITH, STAT_EXT, "devotion", TASK_NORMAL, LAG_0, POSITION_RESTING, MANA_0, LIFEFORCE_0, PRAY_0, 0, SYMBOL_STRESS_0, "", "", "", "", START_1, LEARN_1, START_DO_NO, LEARN_DO_NO, START_DO_NO, LEARN_DO_NO, LEARN_DIFF_UNUSUAL, 0.0, 0, 0);

// disc_theology

  discArray[SKILL_PENANCE] = new spellInfo(SKILL_CLERIC_TYPES, DISC_THEOLOGY, DISC_THEOLOGY, STAT_EXT, "penance", TASK_EASY, LAG_0, POSITION_RESTING, MANA_0, LIFEFORCE_0, PRAY_0, 0, SYMBOL_STRESS_0, "", "", "", "", START_1, LEARN_1, START_DO_1, LEARN_DO_1, START_DO_NO, LEARN_DO_NO, LEARN_DIFF_UNUSUAL, 0.0, SPELL_IGNORE_POSITION, 0);


  // disc_slash

  discArray[SKILL_SLASH_SPEC] = new spellInfo(SKILL_WARRIOR, DISC_SLASH, DISC_SLASH, STAT_EXT, "slash specialization", TASK_NORMAL, LAG_0, POSITION_DEAD, MANA_0, LIFEFORCE_0, PRAY_0, 0, SYMBOL_STRESS_0, "", "", "", "", START_1, LEARN_1, START_DO_1, LEARN_DO_1, START_DO_NO, LEARN_DO_NO, LEARN_DIFF_UNUSUAL, 0.0, 0, 0);


  // disc_blunt

  discArray[SKILL_BLUNT_SPEC] = new spellInfo(SKILL_WARRIOR, DISC_BLUNT, DISC_BLUNT, STAT_EXT, "blunt specialization", TASK_NORMAL, LAG_0, POSITION_DEAD, MANA_0, LIFEFORCE_0, PRAY_0, 0, SYMBOL_STRESS_0, "", "", "", "", START_1, LEARN_1, START_DO_1, LEARN_DO_1, START_DO_NO, LEARN_DO_NO, LEARN_DIFF_UNUSUAL, 0.0, 0, 0);

  // disc_pierce

  discArray[SKILL_PIERCE_SPEC] = new spellInfo(SKILL_WARRIOR, DISC_PIERCE, DISC_PIERCE, STAT_EXT, "pierce specialization", TASK_NORMAL, LAG_0, POSITION_DEAD, MANA_0, LIFEFORCE_0, PRAY_0, 0, SYMBOL_STRESS_0, "", "", "", "", START_1, LEARN_1, START_DO_1, LEARN_DO_1, START_DO_NO, LEARN_DO_NO, LEARN_DIFF_UNUSUAL, 0.0, 0, 0);

  // disc_ranged

  discArray[SKILL_RANGED_SPEC] = new spellInfo(SKILL_WARRIOR, DISC_RANGED, DISC_RANGED, STAT_EXT, "ranged specialization", TASK_NORMAL, LAG_0, POSITION_DEAD, MANA_0, LIFEFORCE_0, PRAY_0, 0, SYMBOL_STRESS_0, "", "", "", "", START_1, LEARN_1, START_DO_1, LEARN_DO_2, START_DO_NO, LEARN_DO_NO, LEARN_DIFF_UNUSUAL, 0.0, 0, 0);

  discArray[SKILL_FAST_LOAD] = new spellInfo(SKILL_WARRIOR, DISC_RANGED, DISC_RANGED, STAT_EXT, "fast load", TASK_NORMAL, LAG_0, POSITION_DEAD, MANA_0, LIFEFORCE_0, PRAY_0, 0, SYMBOL_STRESS_0, "", "", "", "", START_1, LEARN_2, START_DO_1, LEARN_DO_4, START_DO_NO, LEARN_DO_NO, LEARN_DIFF_UNUSUAL, 0.0, 0, 0);

  // disc_barehand

  discArray[SKILL_BAREHAND_SPEC] = new spellInfo(SKILL_MONK, DISC_BAREHAND, DISC_BAREHAND, STAT_EXT, "barehand specialization", TASK_NORMAL, LAG_0, POSITION_DEAD, MANA_0, LIFEFORCE_0, PRAY_0, 0, SYMBOL_STRESS_0, "", "", "", "", START_1, LEARN_1, START_DO_1, LEARN_DO_1, START_DO_NO, LEARN_DO_NO, LEARN_DIFF_UNUSUAL, 0.0, 0, 0);

#if 0
  discArray[SKILL_ARMOR_USE] = new spellInfo(SKILL_GENERAL, DISC_COMBAT, DISC_COMBAT, "armor proficiency", TASK_NORMAL, LAG_0, POSITION_DEAD, MANA_0, LIFEFORCE_0, PRAY_0, 0, SYMBOL_STRESS_0, "", "", "", "", START_1, LEARN_1, START_DO_1, LEARN_DO_1, START_DO_NO, LEARN_DO_NO, LEARN_DIFF_UNUSUAL, 0.0, 0, 0);

  discArray[SPELL_CHAIN_LIGHTNING] = new spellInfo(SPELL_MAGE, DISC_SORCERY, DISC_SORCERY, "chain lightning", TASK_NORMAL, LAG_1, POSITION_SITTING, MANA_40, 0,  TAR_AREA | TAR_IGNORE | TAR_FIGHT_VICT | TAR_VIOLENT, SYMBOL_STRESS_0, "", "", "", "", START_60, LEARN_5, START_DO_50, LEARN_DO_2, START_DO_NO, LEARN_DO_NO, LEARN_DIFF_SPELLS, 0.04, COMP_GESTURAL | COMP_VERBAL | COMP_MATERIAL, 0);

  discArray[SPELL_REPAIR] = new spellInfo(SPELL_MAGE, DISC_ALCHEMY, DISC_ALCHEMY, "repair", LAG_2, POSITION_CRAWLING, MANA_50, LIFEFORCE_0, PRAY_0, TAR_OBJ_INV | TAR_OBJ_EQUIP, SYMBOL_STRESS_0, "", "", "", "", START_65, LEARN_5, START_DO_50, LEARN_DO_5, 0.0, COMP_GESTURAL | COMP_VERBAL | COMP_MATERIAL, 0);

  discArray[SKILL_CASTING] = new spellInfo(SPELL_MAGE, DISC_WIZARDRY, DISC_WIZARDRY, "casting", TASK_NORMAL, LAG_0, POSITION_RESTING, MANA_0, LIFEFORCE_0, PRAY_0, 0, SYMBOL_STRESS_0, "", "", "", "", START_1, LEARN_100, START_DO_1, LEARN_DO_1, START_DO_NO, LEARN_DO_NO, LEARN_DIFF_UNUSUAL, 0.0, 0, 0);

  discArray[SKILL_PRAYING] = new spellInfo(SKILL_CLERIC_TYPES, DISC_FAITH, DISC_FAITH, "praying", TASK_NORMAL, LAG_0, POSITION_RESTING, MANA_0, LIFEFORCE_0, PRAY_0, 0, SYMBOL_STRESS_0, "", "", "", "", START_1, LEARN_100, START_DO_1, LEARN_DO_1, START_DO_NO, LEARN_DO_NO, LEARN_DIFF_UNUSUAL, 0.0, 0, 0);

  discArray[SKILL_MASS_FORAGE] = new spellInfo(SKILL_RANGER, DISC_SURVIVAL, DISC_SURVIVAL, "mass forage", TASK_NORMAL, LAG_0, POSITION_CRAWLING, MANA_0, LIFEFORCE_0, PRAY_0, 0, SYMBOL_STRESS_0, "", "", "", "", START_26, LEARN_4, START_DO_10, LEARN_DO_1, START_DO_NO, LEARN_DO_NO, LEARN_DIFF_SKILLS, 0.0, 0, 0);

  discArray[SKILL_TAN] = new spellInfo(SKILL_RANGER, DISC_SURVIVAL, DISC_SURVIVAL, "tanning", TASK_NORMAL, LAG_0, POSITION_CRAWLING, MANA_0, LIFEFORCE_0, PRAY_0, 0, SYMBOL_STRESS_0, "", "", "", "", START_1, LEARN_4, START_DO_1, LEARN_DO_2, START_DO_NO, LEARN_DO_NO, LEARN_DIFF_SKILLS, 0.0, 0, 0);

#endif
  

  // psionics
  discArray[SKILL_PSITELEPATHY] = new spellInfo(SKILL_GENERAL, DISC_PSIONICS, DISC_PSIONICS, STAT_INT, "psionic telepathy", TASK_EASY, LAG_0, POSITION_STANDING, MANA_10, LIFEFORCE_0, PRAY_0, 0, SYMBOL_STRESS_0, "", "", "", "", START_1, LEARN_5, START_DO_1, LEARN_DO_1, START_DO_NO, LEARN_DO_NO, LEARN_DIFF_SKILLS, 0.0, 0, 0);

  discArray[SKILL_TELE_SIGHT] = new spellInfo(SKILL_GENERAL, DISC_PSIONICS, DISC_PSIONICS, STAT_INT, "telepathic sight", TASK_NORMAL, LAG_0, POSITION_STANDING, MANA_0, LIFEFORCE_0, PRAY_0, 0, SYMBOL_STRESS_0, "", "", "", "", START_10, LEARN_100, START_DO_NO, LEARN_DO_NO, START_DO_NO, LEARN_DO_NO, LEARN_DIFF_SKILLS, 0.0, 0, 0);

  discArray[SKILL_TELE_VISION] = new spellInfo(SKILL_GENERAL, DISC_PSIONICS, DISC_PSIONICS, STAT_INT, "telepathic vision", TASK_NORMAL, LAG_3, POSITION_STANDING, MANA_40, LIFEFORCE_0, PRAY_0, 0, SYMBOL_STRESS_0, "", "", "", "", START_70, LEARN_4, START_DO_1, LEARN_DO_1, START_DO_NO, LEARN_DO_NO, LEARN_DIFF_SKILLS, 0.0, 0, 0);

  discArray[SKILL_MIND_FOCUS] = new spellInfo(SKILL_GENERAL, DISC_PSIONICS, DISC_PSIONICS, STAT_INT, "mind focus", TASK_NORMAL, LAG_1, POSITION_STANDING, MANA_50, LIFEFORCE_0, PRAY_0, 0, SYMBOL_STRESS_0, "", "", "", "", START_25, LEARN_2, START_DO_1, LEARN_DO_1, START_DO_NO, LEARN_DO_NO, LEARN_DIFF_SKILLS, 0.0, 0, 0);

  discArray[SKILL_PSI_BLAST] = new spellInfo(SKILL_GENERAL, DISC_PSIONICS, DISC_PSIONICS, STAT_INT, "psionic blast", TASK_NORMAL, LAG_3, POSITION_STANDING, MANA_20, LIFEFORCE_0, PRAY_0, 0, SYMBOL_STRESS_0, "", "", "", "", START_20, LEARN_2, START_DO_1, LEARN_DO_1, START_DO_NO, LEARN_DO_NO, LEARN_DIFF_SKILLS, 0.0, 0, 0);

  discArray[SKILL_MIND_THRUST] = new spellInfo(SKILL_GENERAL, DISC_PSIONICS, DISC_PSIONICS, STAT_INT, "mind thrust", TASK_NORMAL, LAG_3, POSITION_STANDING, MANA_25, LIFEFORCE_0, PRAY_0, 0, SYMBOL_STRESS_0, "", "", "", "", START_40, LEARN_2, START_DO_1, LEARN_DO_1, START_DO_NO, LEARN_DO_NO, LEARN_DIFF_SKILLS, 0.0, 0, 0);

  discArray[SKILL_PSYCHIC_CRUSH] = new spellInfo(SKILL_GENERAL, DISC_PSIONICS, DISC_PSIONICS, STAT_INT, "psychic crush", TASK_NORMAL, LAG_3, POSITION_STANDING, MANA_30, LIFEFORCE_0, PRAY_0, 0, SYMBOL_STRESS_0, "", "", "", "", START_60, LEARN_3, START_DO_1, LEARN_DO_1, START_DO_NO, LEARN_DO_NO, LEARN_DIFF_SKILLS, 0.0, 0, 0);

  discArray[SKILL_KINETIC_WAVE] = new spellInfo(SKILL_GENERAL, DISC_PSIONICS, DISC_PSIONICS, STAT_INT, "kinetic wave", TASK_NORMAL, LAG_3, POSITION_STANDING, MANA_40, LIFEFORCE_0, PRAY_0, 0, SYMBOL_STRESS_0, "", "", "", "", START_80, LEARN_5, START_DO_1, LEARN_DO_1, START_DO_NO, LEARN_DO_NO, LEARN_DIFF_SKILLS, 0.0, 0, 0);

  discArray[SKILL_MIND_PRESERVATION] = new spellInfo(SKILL_GENERAL, DISC_PSIONICS, DISC_PSIONICS, STAT_INT, "mind preservation", TASK_NORMAL, LAG_0, POSITION_STANDING, MANA_0, LIFEFORCE_0, PRAY_0, 0, SYMBOL_STRESS_0, "", "", "", "", START_1, LEARN_1, START_DO_NO, LEARN_DO_NO, START_DO_NO, LEARN_DO_NO, LEARN_DIFF_SKILLS, 0.0, 0, 0);

  discArray[SKILL_TELEKINESIS] = new spellInfo(SKILL_GENERAL, DISC_PSIONICS, DISC_PSIONICS, STAT_INT, "telekinesis", TASK_NORMAL, LAG_1, POSITION_STANDING, MANA_0, LIFEFORCE_0, PRAY_0, 0, SYMBOL_STRESS_0, "", "", "", "", START_1, LEARN_1, START_DO_1, LEARN_DO_1, START_DO_NO, LEARN_DO_NO, LEARN_DIFF_SKILLS, 0.0, 0, 0);

  discArray[SKILL_PSIDRAIN] = new spellInfo(SKILL_GENERAL, DISC_PSIONICS, DISC_PSIONICS, STAT_INT, "psionic drain", TASK_DIFFICULT, LAG_1, POSITION_STANDING, MANA_0, LIFEFORCE_0, PRAY_0, 0, SYMBOL_STRESS_0, "","","","", START_1, LEARN_1, START_DO_1, LEARN_DO_1, START_DO_NO, LEARN_DO_NO, LEARN_DIFF_SKILLS, 0.0, 0, 0);

// end psionics


  // last_discipline

  discArray[MAX_SKILL] = new spellInfo(SPELL_NOCLASS, DISC_NONE, DISC_NONE, STAT_INT, "\n", TASK_NORMAL, LAG_0, POSITION_DEAD, MANA_0, LIFEFORCE_0, PRAY_0, 0, SYMBOL_STRESS_0, "", "", "", "", START_0, LEARN_0, START_DO_NO, LEARN_DO_NO, START_DO_NO, LEARN_DO_NO, 0, 0.0, 0, 0);

  for(int i=MIN_SPELL;i<MAX_SKILL;i++){
    if(!discArray[i] || !*discArray[i]->name)
      continue;

    if((((101-discArray[i]->start)*discArray[i]->learn)) < 100){
      vlogf(LOG_BUG, format("skill '%s' has bad learning (start: %i, learn: %i)")%
	    discArray[i]->name % discArray[i]->start % discArray[i]->learn);
    }
  }

  
}






