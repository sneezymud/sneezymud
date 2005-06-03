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

#include "stdsneezy.h"

spellInfo *discArray[MAX_SKILL+1];

spellInfo::spellInfo(skillUseClassT styp,
  discNumT discipline, 
  discNumT assDiscipline, 
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

  discArray[SPELL_GUST] = new spellInfo(SPELL_MAGE, DISC_MAGE, DISC_AIR, "gust", TASK_NORMAL, LAG_1, POSITION_SITTING, MANA_10, LIFEFORCE_0, PRAY_0, TAR_VIOLENT | TAR_FIGHT_VICT | TAR_CHAR_ROOM | TAR_SELF_NONO, SYMBOL_STRESS_0, "", "", "", "", START_1, LEARN_100, START_DO_30, LEARN_DO_5, START_DO_NO, LEARN_DO_NO, LEARN_DIFF_SPELLS, 0.04, COMP_GESTURAL | COMP_GESTURAL_RANDOM | COMP_VERBAL | COMP_VERBAL_RANDOM | SPELL_TASKED, 0);

  discArray[SPELL_SLING_SHOT] = new spellInfo(SPELL_MAGE, DISC_MAGE, DISC_EARTH, "sling shot", TASK_EASY, LAG_1, POSITION_SITTING, MANA_15, LIFEFORCE_0, PRAY_0, TAR_CHAR_ROOM | TAR_VIOLENT | TAR_FIGHT_VICT | TAR_SELF_NONO , SYMBOL_STRESS_0, "", "", "", "", START_1, LEARN_100, START_DO_30, LEARN_DO_5, START_DO_NO, LEARN_DO_NO, LEARN_DIFF_SPELLS, 0.04, COMP_GESTURAL | COMP_GESTURAL_RANDOM | COMP_VERBAL | COMP_VERBAL_RANDOM | SPELL_TASKED, 0);

  discArray[SPELL_GUSHER] = new spellInfo(SPELL_MAGE, DISC_MAGE, DISC_WATER, "gusher", TASK_EASY, LAG_1, POSITION_SITTING, MANA_20, LIFEFORCE_0, PRAY_0, TAR_CHAR_ROOM | TAR_SELF_NONO | TAR_VIOLENT | TAR_FIGHT_VICT , SYMBOL_STRESS_0, "", "", "", "", START_1, LEARN_100, START_DO_30, LEARN_DO_5, START_DO_NO, LEARN_DO_NO, LEARN_DIFF_SPELLS, 0.02, COMP_GESTURAL | COMP_GESTURAL_RANDOM | COMP_VERBAL | COMP_VERBAL_RANDOM | SPELL_TASKED, 0);

  discArray[SPELL_HANDS_OF_FLAME] = new spellInfo(SPELL_MAGE, DISC_MAGE, DISC_FIRE, "hands of flame", TASK_EASY, LAG_1, POSITION_SITTING, MANA_20, LIFEFORCE_0, PRAY_0, TAR_CHAR_ROOM | TAR_SELF_NONO | TAR_VIOLENT | TAR_FIGHT_VICT , SYMBOL_STRESS_0, "", "", "", "", START_10, LEARN_100, START_DO_30, LEARN_DO_5, START_DO_NO, LEARN_DO_NO, LEARN_DIFF_SPELLS, 0.02, COMP_GESTURAL | COMP_GESTURAL_RANDOM | COMP_VERBAL | COMP_VERBAL_RANDOM | SPELL_TASKED, 0);

  discArray[SPELL_MYSTIC_DARTS] = new spellInfo(SPELL_MAGE, DISC_MAGE, DISC_SORCERY, "mystic darts", TASK_NORMAL, LAG_1, POSITION_SITTING, MANA_23, LIFEFORCE_0, PRAY_0, TAR_CHAR_ROOM | TAR_SELF_NONO | TAR_VIOLENT | TAR_FIGHT_VICT , SYMBOL_STRESS_0, "", "", "", "", START_12, LEARN_25, START_DO_30, LEARN_DO_5, START_DO_NO, LEARN_DO_NO, LEARN_DIFF_SPELLS, 0.04, COMP_GESTURAL | COMP_GESTURAL_RANDOM | COMP_VERBAL | COMP_VERBAL_RANDOM | SPELL_TASKED, 0);


  discArray[SPELL_FLARE] = new spellInfo(SPELL_MAGE, DISC_MAGE, DISC_FIRE, "flare", TASK_TRIVIAL, LAG_1, POSITION_CRAWLING, MANA_25, LIFEFORCE_0, PRAY_0, TAR_IGNORE, SYMBOL_STRESS_0, "", "", "", "", START_6, LEARN_35, START_DO_45, LEARN_DO_7, START_DO_NO, LEARN_DO_NO, LEARN_DIFF_SPELLS, 0.01, COMP_GESTURAL | COMP_GESTURAL_RANDOM | COMP_VERBAL | COMP_VERBAL_RANDOM | COMP_MATERIAL | COMP_MATERIAL_END | SPELL_TASKED, 0);

  discArray[SPELL_SORCERERS_GLOBE] = new spellInfo(SPELL_MAGE, DISC_MAGE, DISC_SORCERY, "sorcerers globe", TASK_EASY, LAG_1, POSITION_CRAWLING, MANA_15, LIFEFORCE_0, PRAY_0, TAR_CHAR_ROOM | TAR_FIGHT_SELF, SYMBOL_STRESS_0, "The air around you seems to become less dense.", "The globe about $n wavers, then collapses altogether.", "The air around you seems to become less dense.", "$n's magic globe wavers.", START_1, LEARN_8, START_DO_50, LEARN_DO_5, START_DO_NO, LEARN_DO_NO, LEARN_DIFF_SPELLS, 0.01, COMP_GESTURAL | COMP_GESTURAL_RANDOM | COMP_VERBAL | COMP_VERBAL_RANDOM | COMP_MATERIAL | COMP_MATERIAL_END | SPELL_TASKED, 0);

  discArray[SPELL_FAERIE_FIRE] = new spellInfo(SPELL_MAGE, DISC_MAGE, DISC_FIRE, "faerie fire", TASK_EASY, LAG_2, POSITION_SITTING, MANA_15, LIFEFORCE_0, PRAY_0, TAR_VIOLENT | TAR_FIGHT_VICT | TAR_CHAR_ROOM | TAR_SELF_NONO | TAR_SELF_NONO, SYMBOL_STRESS_0, "The pink aura around your body fades away.", "$n's pink aura fades away.", "The pink aura around your body flickers slightly.", "The pink aura around $n's body flickers slightly.", START_15, LEARN_14, START_DO_35, LEARN_DO_7, START_DO_NO, LEARN_DO_NO, LEARN_DIFF_SPELLS, 0.03, COMP_GESTURAL | COMP_GESTURAL_RANDOM | COMP_VERBAL | COMP_VERBAL_RANDOM | COMP_MATERIAL | COMP_MATERIAL_END | SPELL_TASKED, 0);

  discArray[SPELL_ILLUMINATE] = new spellInfo(SPELL_MAGE, DISC_MAGE, DISC_ALCHEMY, "illuminate", TASK_TRIVIAL, LAG_1, POSITION_CRAWLING, MANA_20, LIFEFORCE_0, PRAY_0, TAR_OBJ_INV | TAR_OBJ_EQUIP, SYMBOL_STRESS_0, "", "", "", "", START_3, LEARN_17, START_DO_35, LEARN_DO_7, START_DO_NO, LEARN_DO_NO, LEARN_DIFF_SPELLS, 0.0, COMP_GESTURAL | COMP_GESTURAL_RANDOM | COMP_VERBAL | COMP_VERBAL_RANDOM | COMP_MATERIAL | COMP_MATERIAL_END | SPELL_TASKED, 0);

  discArray[SPELL_DETECT_MAGIC] = new spellInfo(SPELL_MAGE, DISC_MAGE, DISC_ALCHEMY, "detect magic", TASK_TRIVIAL, LAG_1, POSITION_CRAWLING, MANA_15, LIFEFORCE_0, PRAY_0, TAR_CHAR_ROOM | TAR_FIGHT_SELF, SYMBOL_STRESS_0, "Your eyes don't tingle anymore.", "$n's eyes don't twinkle anymore.", "You blink as your eyes sting for a moment.", "$n's eyes seem to water a bit.", START_5, LEARN_17, START_DO_40, LEARN_DO_5, START_DO_NO, LEARN_DO_NO, LEARN_DIFF_SPELLS, 0.0, COMP_GESTURAL | COMP_GESTURAL_RANDOM | COMP_VERBAL | COMP_VERBAL_RANDOM | COMP_MATERIAL | COMP_MATERIAL_END | SPELL_TASKED, 0);

  discArray[SPELL_STUNNING_ARROW] = new spellInfo(SPELL_MAGE, DISC_MAGE, DISC_SORCERY, "stunning arrow", TASK_NORMAL, LAG_1, POSITION_SITTING, MANA_45, LIFEFORCE_0, PRAY_0, TAR_CHAR_ROOM | TAR_SELF_NONO | TAR_VIOLENT | TAR_FIGHT_VICT , SYMBOL_STRESS_0, "", "", "", "", START_34, LEARN_25, START_DO_40, LEARN_DO_5, START_DO_NO, LEARN_DO_NO, LEARN_DIFF_SPELLS, 0.04, COMP_GESTURAL | COMP_GESTURAL_RANDOM | COMP_VERBAL | COMP_VERBAL_RANDOM | COMP_MATERIAL | COMP_MATERIAL_END | SPELL_TASKED, 0);

  discArray[SPELL_MATERIALIZE] = new spellInfo(SPELL_MAGE, DISC_MAGE, DISC_ALCHEMY, "materialize", TASK_TRIVIAL, LAG_1, POSITION_CRAWLING, MANA_15, LIFEFORCE_0, PRAY_0, TAR_NAME, SYMBOL_STRESS_0, "", "", "", "", START_15, LEARN_25, START_DO_40, LEARN_DO_7, START_DO_NO, LEARN_DO_NO, LEARN_DIFF_SPELLS, 0.0, COMP_GESTURAL | COMP_GESTURAL_RANDOM | COMP_VERBAL | COMP_VERBAL_RANDOM | SPELL_TASKED, 0);

  discArray[SPELL_PROTECTION_FROM_EARTH] = new spellInfo(SPELL_MAGE, DISC_MAGE, DISC_EARTH, "protection from earth", TASK_EASY, LAG_2, POSITION_CRAWLING, MANA_15, LIFEFORCE_0, PRAY_0, TAR_CHAR_ROOM | TAR_FIGHT_SELF, SYMBOL_STRESS_0, "You feel slightly less protected.", "", "", "", START_24, LEARN_20, START_DO_50, LEARN_DO_5, START_DO_NO, LEARN_DO_NO, LEARN_DIFF_SPELLS, 0.01, COMP_GESTURAL | COMP_GESTURAL_RANDOM | COMP_VERBAL | COMP_VERBAL_RANDOM | COMP_MATERIAL | COMP_MATERIAL_END | SPELL_TASKED, 0);

  discArray[SPELL_PROTECTION_FROM_AIR] = new spellInfo(SPELL_MAGE, DISC_MAGE, DISC_AIR, "protection from air", TASK_EASY, LAG_1, POSITION_CRAWLING, MANA_15, LIFEFORCE_0, PRAY_0, TAR_CHAR_ROOM | TAR_FIGHT_SELF, SYMBOL_STRESS_0, "You feel slightly less protected.", "", "", "", START_25, LEARN_14, START_DO_50, LEARN_DO_5, START_DO_NO, LEARN_DO_NO, LEARN_DIFF_SPELLS, 0.01, COMP_GESTURAL | COMP_GESTURAL_RANDOM | COMP_VERBAL | COMP_VERBAL_RANDOM | COMP_MATERIAL | COMP_MATERIAL_END | SPELL_TASKED, 0);

  discArray[SPELL_PROTECTION_FROM_FIRE] = new spellInfo(SPELL_MAGE, DISC_MAGE, DISC_FIRE,"protection from fire", TASK_EASY, LAG_1, POSITION_CRAWLING, MANA_15, LIFEFORCE_0, PRAY_0, TAR_CHAR_ROOM | TAR_FIGHT_SELF, SYMBOL_STRESS_0, "You feel slightly less protected.", "", "", "", START_25, LEARN_14, START_DO_50, LEARN_DO_5, START_DO_NO, LEARN_DO_NO, LEARN_DIFF_SPELLS, 0.01, COMP_GESTURAL | COMP_GESTURAL_RANDOM | COMP_VERBAL | COMP_VERBAL_RANDOM | COMP_MATERIAL | COMP_MATERIAL_END | SPELL_TASKED, 0);

  discArray[SPELL_PROTECTION_FROM_WATER] = new spellInfo(SPELL_MAGE, DISC_MAGE, DISC_WATER, "protection from water", TASK_EASY, LAG_1, POSITION_CRAWLING, MANA_15, LIFEFORCE_0, PRAY_0, TAR_CHAR_ROOM | TAR_FIGHT_SELF, SYMBOL_STRESS_0, "You feel slightly less protected.", "", "", "", START_27, LEARN_23, START_DO_50, LEARN_DO_5, START_DO_NO, LEARN_DO_NO, LEARN_DIFF_SPELLS, 0.01, COMP_GESTURAL | COMP_GESTURAL_RANDOM | COMP_VERBAL | COMP_VERBAL_RANDOM | COMP_MATERIAL | COMP_MATERIAL_END | SPELL_TASKED, 0);

  discArray[SPELL_PROTECTION_FROM_ELEMENTS] = new spellInfo(SPELL_MAGE, DISC_MAGE, DISC_SORCERY, "protection from elements", TASK_NORMAL, LAG_2, POSITION_CRAWLING, MANA_15, LIFEFORCE_0, PRAY_0, TAR_CHAR_ROOM | TAR_FIGHT_SELF, SYMBOL_STRESS_0, "You feel slightly less protected.", "", "", "", START_28, LEARN_20, START_DO_50, LEARN_DO_5, START_DO_NO, LEARN_DO_NO, LEARN_DIFF_SPELLS, 0.01, COMP_GESTURAL | COMP_GESTURAL_RANDOM | COMP_VERBAL | COMP_VERBAL_RANDOM | COMP_MATERIAL | COMP_MATERIAL_END | SPELL_TASKED, 0);

  discArray[SPELL_PEBBLE_SPRAY] = new spellInfo(SPELL_MAGE, DISC_MAGE, DISC_EARTH, "pebble spray", TASK_EASY, LAG_1, POSITION_SITTING, MANA_25, LIFEFORCE_0, PRAY_0, TAR_AREA | TAR_FIGHT_VICT | TAR_VIOLENT | TAR_IGNORE, SYMBOL_STRESS_0, "", "", "", "", START_15, LEARN_15, START_DO_45, LEARN_DO_5, START_DO_NO, LEARN_DO_NO, LEARN_DIFF_SPELLS, 0.04, COMP_GESTURAL | COMP_GESTURAL_RANDOM | COMP_VERBAL | COMP_VERBAL_RANDOM | COMP_MATERIAL | COMP_MATERIAL_END | SPELL_TASKED, 0);

  discArray[SPELL_ARCTIC_BLAST] = new spellInfo(SPELL_MAGE, DISC_MAGE, DISC_WATER, "arctic blast", TASK_NORMAL, LAG_1, POSITION_SITTING, MANA_25, LIFEFORCE_0, PRAY_0, TAR_AREA | TAR_IGNORE | TAR_VIOLENT, SYMBOL_STRESS_0, "", "", "", "", START_16, LEARN_15, START_DO_45, LEARN_DO_5, START_DO_NO, LEARN_DO_NO, LEARN_DIFF_SPELLS, 0.02, COMP_GESTURAL | COMP_GESTURAL_RANDOM | COMP_VERBAL | COMP_VERBAL_RANDOM | COMP_MATERIAL | COMP_MATERIAL_END | SPELL_TASKED, 0);

  discArray[SPELL_COLOR_SPRAY] = new spellInfo(SPELL_MAGE, DISC_MAGE, DISC_SORCERY, "color spray", TASK_DIFFICULT, LAG_1, POSITION_SITTING, MANA_38, LIFEFORCE_0, PRAY_0, TAR_AREA | TAR_IGNORE | TAR_FIGHT_VICT | TAR_VIOLENT, SYMBOL_STRESS_0, "", "", "", "", START_35, LEARN_10, START_DO_45, LEARN_DO_5, START_DO_NO, LEARN_DO_NO, LEARN_DIFF_SPELLS, 0.04, COMP_GESTURAL | COMP_GESTURAL_RANDOM | COMP_VERBAL | COMP_VERBAL_RANDOM | COMP_MATERIAL | COMP_MATERIAL_END | SPELL_TASKED, 0);

  discArray[SPELL_INFRAVISION] = new spellInfo(SPELL_MAGE, DISC_MAGE, DISC_FIRE, "infravision", TASK_EASY, LAG_2, POSITION_CRAWLING, MANA_20, LIFEFORCE_0, PRAY_0, TAR_CHAR_ROOM | TAR_FIGHT_SELF, SYMBOL_STRESS_0, "Your eyes lose their red glow.", "$n's eyes don't glow red anymore.", "Your eyes seem less sensitive to heat.", "$n's red eyes flicker.", START_44, LEARN_15, START_DO_50, LEARN_DO_5, START_DO_NO, LEARN_DO_NO, LEARN_DIFF_SPELLS, 0.0, COMP_GESTURAL | COMP_GESTURAL_RANDOM | COMP_VERBAL | COMP_VERBAL_RANDOM | COMP_MATERIAL | COMP_MATERIAL_END | SPELL_TASKED, 0);

  discArray[SPELL_IDENTIFY] = new spellInfo(SPELL_MAGE, DISC_MAGE, DISC_ALCHEMY, "identify", TASK_EASY, LAG_2, POSITION_CRAWLING, MANA_20, LIFEFORCE_0, PRAY_0, TAR_OBJ_INV | TAR_OBJ_EQUIP | TAR_CHAR_ROOM, SYMBOL_STRESS_0, "", "", "", "", START_40, LEARN_20, START_DO_50, LEARN_DO_5, START_DO_NO, LEARN_DO_NO, LEARN_DIFF_SPELLS, 0.0, COMP_GESTURAL | COMP_GESTURAL_RANDOM | COMP_VERBAL | COMP_VERBAL_RANDOM | COMP_MATERIAL | COMP_MATERIAL_END | SPELL_TASKED, 0);

  discArray[SPELL_POWERSTONE] = new spellInfo(SPELL_MAGE, DISC_MAGE, DISC_ALCHEMY, "powerstone", TASK_NORMAL, LAG_2, POSITION_CRAWLING, MANA_25, LIFEFORCE_0, PRAY_0, TAR_OBJ_INV, SYMBOL_STRESS_0, "", "", "", "", START_55, LEARN_15, START_DO_50, LEARN_DO_10, START_DO_NO, LEARN_DO_NO, LEARN_DIFF_SPELLS, 0.0, COMP_GESTURAL | COMP_GESTURAL_RANDOM | COMP_VERBAL | COMP_VERBAL_RANDOM | COMP_MATERIAL | COMP_MATERIAL_END | SPELL_TASKED, TOG_HAS_POWERSTONE);

  discArray[SPELL_FAERIE_FOG] = new spellInfo(SPELL_MAGE, DISC_MAGE, DISC_WATER, "faerie fog", TASK_NORMAL, LAG_2, POSITION_CRAWLING, MANA_20, LIFEFORCE_0, PRAY_0, TAR_IGNORE, SYMBOL_STRESS_0, "", "", "", "", START_52, LEARN_13, START_DO_40, LEARN_DO_5, START_DO_NO, LEARN_DO_NO, LEARN_DIFF_SPELLS, 0.0, COMP_GESTURAL | COMP_GESTURAL_RANDOM | COMP_VERBAL | COMP_VERBAL_RANDOM | COMP_MATERIAL | COMP_MATERIAL_END | SPELL_TASKED, 0);

  discArray[SPELL_TELEPORT] = new spellInfo(SPELL_MAGE, DISC_MAGE, DISC_SORCERY, "teleport", TASK_EASY, LAG_1, POSITION_SITTING, MANA_25, LIFEFORCE_0, PRAY_0, TAR_CHAR_ROOM | TAR_FIGHT_SELF, SYMBOL_STRESS_0, "", "", "", "", START_58, LEARN_15, START_DO_40, LEARN_DO_5, START_DO_NO, LEARN_DO_NO, LEARN_DIFF_SPELLS, 0.0, COMP_GESTURAL | COMP_GESTURAL_RANDOM | COMP_VERBAL | COMP_VERBAL_RANDOM | COMP_MATERIAL | COMP_MATERIAL_END | SPELL_TASKED, 0);

  discArray[SPELL_POLYMORPH] = new spellInfo(SPELL_MAGE, DISC_SPIRIT, DISC_SPIRIT, "polymorph", TASK_NORMAL, LAG_2, POSITION_CRAWLING, MANA_40, LIFEFORCE_0, PRAY_0, TAR_NAME, SYMBOL_STRESS_0, "You are not able to hold this form any longer.", "", "You do not know how much longer you can hold this form.", "", START_40, LEARN_10, START_DO_50, LEARN_DO_4, START_DO_NO, LEARN_DO_NO, LEARN_DIFF_SPELLS, 0.0, COMP_GESTURAL | COMP_GESTURAL_RANDOM | COMP_VERBAL | COMP_VERBAL_RANDOM | COMP_MATERIAL | COMP_MATERIAL_END | SPELL_TASKED, 0);

  discArray[SPELL_SENSE_LIFE] = new spellInfo(SPELL_MAGE, DISC_MAGE, DISC_SPIRIT, "sense life", TASK_EASY, LAG_1, POSITION_CRAWLING, MANA_15, LIFEFORCE_0, PRAY_0, TAR_CHAR_ROOM | TAR_FIGHT_SELF, SYMBOL_STRESS_0, "The aqua blue aura fades from your eyes.", "The aqua blue aura fades from $n's eyes.", "The aqua blue in your eyes flickers slightly.", "The aqua blue in $n's eyes flickers slightly.", START_24, LEARN_18, START_DO_40, LEARN_DO_5, START_DO_NO, LEARN_DO_NO, LEARN_DIFF_SPELLS, 0.04, COMP_GESTURAL | COMP_GESTURAL_RANDOM | COMP_VERBAL | COMP_VERBAL_RANDOM | COMP_MATERIAL | COMP_MATERIAL_END | SPELL_TASKED, 0);

  discArray[SPELL_CALM] = new spellInfo(SPELL_MAGE, DISC_MAGE, DISC_SPIRIT, "calm", TASK_NORMAL, LAG_1, POSITION_CRAWLING, MANA_25, LIFEFORCE_0, PRAY_0, TAR_CHAR_ROOM | TAR_SELF_NONO | TAR_VIOLENT, SYMBOL_STRESS_0, "You aren't so calm anymore.", "$n doesn't seem as calm as $e was before.", "You are feeling less calm.", "$n is losing $s calm.", START_58, LEARN_16, START_DO_50, LEARN_DO_5, START_DO_NO, LEARN_DO_NO, LEARN_DIFF_SPELLS, 0.04, COMP_GESTURAL | COMP_GESTURAL_RANDOM | COMP_VERBAL | COMP_VERBAL_RANDOM | COMP_MATERIAL | COMP_MATERIAL_END | SPELL_TASKED, 0);

  discArray[SPELL_ACCELERATE] = new spellInfo(SPELL_MAGE, DISC_MAGE, DISC_SPIRIT, "accelerate", TASK_DIFFICULT, LAG_2, POSITION_CRAWLING, MANA_30, LIFEFORCE_0, PRAY_0, TAR_CHAR_ROOM | TAR_FIGHT_SELF, SYMBOL_STRESS_0, "You don't move with as much ease.", "$n doesn't move with as much ease.", "", "", START_32, LEARN_18, START_DO_40, LEARN_DO_5, START_DO_NO, LEARN_DO_NO, LEARN_DIFF_SPELLS, 0.04, COMP_GESTURAL | COMP_GESTURAL_RANDOM | COMP_VERBAL | COMP_VERBAL_RANDOM | COMP_MATERIAL | COMP_MATERIAL_END |SPELL_TASKED, 0);

 discArray[SPELL_DUST_STORM] = new spellInfo(SPELL_MAGE, DISC_MAGE, DISC_AIR, "dust storm", TASK_NORMAL, LAG_1, POSITION_SITTING, MANA_17, LIFEFORCE_0, PRAY_0, TAR_AREA | TAR_FIGHT_VICT | TAR_VIOLENT | TAR_IGNORE, SYMBOL_STRESS_0, "", "", "", "", START_33, LEARN_13, START_DO_40, LEARN_DO_5, START_DO_NO, LEARN_DO_NO, LEARN_DIFF_SPELLS, 0.04, COMP_GESTURAL | COMP_GESTURAL_RANDOM | COMP_VERBAL | COMP_VERBAL_RANDOM | COMP_MATERIAL | COMP_MATERIAL_END | SPELL_TASKED, 0);

  discArray[SPELL_LEVITATE] = new spellInfo(SPELL_MAGE, DISC_MAGE, DISC_AIR, "levitate", TASK_NORMAL, LAG_2, POSITION_SITTING, MANA_25, LIFEFORCE_0, PRAY_0, TAR_FIGHT_SELF | TAR_CHAR_ROOM, SYMBOL_STRESS_0, "You lose your gift of levitation.", "$n sinks back onto the $g.", "You begin to feel the tug of The World again.", "", START_32, LEARN_11, START_DO_40, LEARN_DO_5, START_DO_NO, LEARN_DO_NO, LEARN_DIFF_SPELLS, 0.04, COMP_GESTURAL | COMP_VERBAL | COMP_MATERIAL | COMP_MATERIAL_END | SPELL_TASKED, 0);

  discArray[SPELL_FEATHERY_DESCENT] = new spellInfo(SPELL_MAGE, DISC_MAGE, DISC_AIR, "feathery descent", TASK_EASY, LAG_1, POSITION_CRAWLING, MANA_25, LIFEFORCE_0, PRAY_0, TAR_CHAR_ROOM | TAR_FIGHT_SELF, SYMBOL_STRESS_0, "You don't seem to be light as a feather anymore.", "$n doesn't seem to be light as a feather any more.", "You feel slightly heavier.", "", START_20, LEARN_8, START_DO_40, LEARN_DO_5, START_DO_NO, LEARN_DO_NO, LEARN_DIFF_SPELLS, 0.04, COMP_GESTURAL | COMP_GESTURAL_RANDOM | COMP_VERBAL | COMP_VERBAL_RANDOM | COMP_MATERIAL | COMP_MATERIAL_END | SPELL_TASKED, 0);

  discArray[SPELL_STEALTH] = new spellInfo(SPELL_MAGE, DISC_MAGE, DISC_SPIRIT, "stealth", TASK_EASY, LAG_2, POSITION_CRAWLING, MANA_20, LIFEFORCE_0, PRAY_0, TAR_CHAR_ROOM | TAR_FIGHT_SELF, SYMBOL_STRESS_0, "You seem to be making a bit more noise now.", "$n is making much more noise than before.", "", "", START_47, LEARN_13, START_DO_45, LEARN_DO_5, START_DO_NO, LEARN_DO_NO, LEARN_DIFF_SPELLS, 0.04, COMP_GESTURAL | COMP_GESTURAL_RANDOM | COMP_VERBAL | COMP_VERBAL_RANDOM | COMP_MATERIAL | COMP_MATERIAL_END | SPELL_TASKED, 0);

  discArray[SPELL_GRANITE_FISTS] = new spellInfo(SPELL_MAGE, DISC_MAGE, DISC_EARTH, "granite fists", TASK_NORMAL, LAG_1, POSITION_SITTING, MANA_30, LIFEFORCE_0, PRAY_0, TAR_CHAR_ROOM | TAR_VIOLENT | TAR_FIGHT_VICT | TAR_SELF_NONO, SYMBOL_STRESS_0, "", "", "", "", START_31, LEARN_9, START_DO_40, LEARN_DO_5, START_DO_NO, LEARN_DO_NO, LEARN_DIFF_SPELLS, 0.05, COMP_GESTURAL | COMP_GESTURAL_RANDOM | COMP_VERBAL | COMP_VERBAL_RANDOM | COMP_MATERIAL | COMP_MATERIAL_END | SPELL_TASKED, 0);

  discArray[SPELL_ICY_GRIP] = new spellInfo(SPELL_MAGE, DISC_MAGE, DISC_WATER, "icy grip", TASK_NORMAL, LAG_1, POSITION_SITTING, MANA_30, LIFEFORCE_0, PRAY_0, TAR_CHAR_ROOM | TAR_SELF_NONO | TAR_VIOLENT | TAR_FIGHT_VICT , SYMBOL_STRESS_0, "", "", "", "", START_40, LEARN_9, START_DO_40, LEARN_DO_5, START_DO_NO, LEARN_DO_NO, LEARN_DIFF_SPELLS, 0.08, COMP_GESTURAL | COMP_GESTURAL_RANDOM | COMP_VERBAL | COMP_VERBAL_RANDOM | COMP_MATERIAL | COMP_MATERIAL_END | SPELL_TASKED, 0);

  discArray[SPELL_GILLS_OF_FLESH] = new spellInfo(SPELL_MAGE, DISC_MAGE, DISC_WATER, "gills of flesh", TASK_EASY, LAG_2, POSITION_CRAWLING, MANA_20, LIFEFORCE_0, PRAY_0, TAR_CHAR_ROOM | TAR_FIGHT_SELF, SYMBOL_STRESS_0, "The gills on your neck disappear.", "The gills on $n's neck disappear.", "You begin to wonder how much longer you can breathe water.", "", START_26, LEARN_13, START_DO_45, LEARN_DO_5, START_DO_NO, LEARN_DO_NO, LEARN_DIFF_SPELLS, 0.02, COMP_GESTURAL | COMP_GESTURAL_RANDOM | COMP_VERBAL | COMP_VERBAL_RANDOM | COMP_MATERIAL | COMP_MATERIAL_END | SPELL_TASKED, 0);

  discArray[SPELL_TELEPATHY] = new spellInfo(SPELL_MAGE, DISC_MAGE, DISC_SPIRIT, "telepathy", TASK_NORMAL, LAG_0, POSITION_CRAWLING, MANA_20, LIFEFORCE_0, PRAY_0, TAR_NAME, SYMBOL_STRESS_0, "", "", "", "", START_47, LEARN_15, START_DO_50, LEARN_DO_5, START_DO_NO, LEARN_DO_NO, LEARN_DIFF_SPELLS, 0.04, COMP_GESTURAL | COMP_GESTURAL_RANDOM | COMP_VERBAL | COMP_VERBAL_RANDOM | COMP_MATERIAL | COMP_MATERIAL_END | SPELL_TASKED, 0);

  discArray[SPELL_FEAR] = new spellInfo(SPELL_MAGE, DISC_MAGE, DISC_SPIRIT, "fear", TASK_DIFFICULT, LAG_2, POSITION_SITTING, MANA_25, LIFEFORCE_0, PRAY_0, TAR_CHAR_ROOM | TAR_SELF_NONO | TAR_VIOLENT | TAR_FIGHT_VICT , SYMBOL_STRESS_0, "You are much less fearful.", "$n doesn't seem as fearful anymore.", "You are about ready to face your fears.", "", START_42, LEARN_11, START_DO_45, LEARN_DO_5, START_DO_NO, LEARN_DO_NO, LEARN_DIFF_SPELLS, 0.04, COMP_GESTURAL | COMP_GESTURAL_RANDOM | COMP_VERBAL | COMP_VERBAL_RANDOM | COMP_MATERIAL | COMP_MATERIAL_END | SPELL_TASKED, 0) ;

  discArray[SPELL_SLUMBER] = new spellInfo(SPELL_MAGE, DISC_MAGE, DISC_SPIRIT, "slumber", TASK_DIFFICULT, LAG_3, POSITION_SITTING, MANA_35, LIFEFORCE_0, PRAY_0, TAR_CHAR_ROOM | TAR_SELF_NONO | TAR_FIGHT_VICT | TAR_VIOLENT, SYMBOL_STRESS_0, "Your magical exhaustion ceases.", "", "", "", START_39, LEARN_11, START_DO_45, LEARN_DO_5, START_DO_NO, LEARN_DO_NO, LEARN_DIFF_SPELLS, 0.04, COMP_GESTURAL | COMP_GESTURAL_RANDOM | COMP_VERBAL | COMP_VERBAL_RANDOM | COMP_MATERIAL | COMP_MATERIAL_END | SPELL_TASKED, 0);

  discArray[SPELL_CONJURE_EARTH] = new spellInfo(SPELL_MAGE, DISC_MAGE, DISC_EARTH, "conjure elemental earth", TASK_DIFFICULT, LAG_1, POSITION_CRAWLING, MANA_40, LIFEFORCE_0, PRAY_0, TAR_IGNORE, SYMBOL_STRESS_0, "", "", "", "", START_63, LEARN_15, START_DO_50, LEARN_DO_5, START_DO_NO, LEARN_DO_NO, LEARN_DIFF_SPELLS, 0.0, COMP_GESTURAL | COMP_GESTURAL_RANDOM | COMP_VERBAL | COMP_VERBAL_RANDOM | COMP_MATERIAL | COMP_MATERIAL_END | SPELL_TASKED, 0);

  discArray[SPELL_CONJURE_FIRE] = new spellInfo(SPELL_MAGE, DISC_MAGE, DISC_FIRE, "conjure elemental fire", TASK_DIFFICULT, LAG_1, POSITION_CRAWLING, MANA_40, LIFEFORCE_0, PRAY_0, TAR_IGNORE, SYMBOL_STRESS_0, "", "", "", "", START_57, LEARN_13, START_DO_50, LEARN_DO_5, START_DO_NO, LEARN_DO_NO, LEARN_DIFF_SPELLS, 0.0, COMP_GESTURAL | COMP_GESTURAL_RANDOM | COMP_VERBAL | COMP_VERBAL_RANDOM | COMP_MATERIAL | COMP_MATERIAL_END | SPELL_TASKED, 0);

  discArray[SPELL_CONJURE_WATER] = new spellInfo(SPELL_MAGE, DISC_MAGE, DISC_WATER, "conjure elemental water", TASK_DIFFICULT, LAG_1, POSITION_CRAWLING, MANA_40, LIFEFORCE_0, PRAY_0, TAR_IGNORE, SYMBOL_STRESS_0, "", "", "", "", START_71, LEARN_10, START_DO_50, LEARN_DO_5, START_DO_NO, LEARN_DO_NO, LEARN_DIFF_SPELLS, 0.0, COMP_GESTURAL | COMP_GESTURAL_RANDOM | COMP_VERBAL | COMP_VERBAL_RANDOM | COMP_MATERIAL | COMP_MATERIAL_END | SPELL_TASKED, 0);

  discArray[SPELL_CONJURE_AIR] = new spellInfo(SPELL_MAGE, DISC_MAGE, DISC_AIR, "conjure elemental air", TASK_DIFFICULT, LAG_1, POSITION_CRAWLING, MANA_40, LIFEFORCE_0, PRAY_0, TAR_IGNORE, SYMBOL_STRESS_0, "", "", "", "", START_35, LEARN_15, START_DO_50, LEARN_DO_5, START_DO_NO, LEARN_DO_NO, LEARN_DIFF_SPELLS, 0.00, COMP_GESTURAL | COMP_GESTURAL_RANDOM | COMP_VERBAL | COMP_VERBAL_RANDOM | COMP_MATERIAL | COMP_MATERIAL_END | SPELL_TASKED, 0);

  discArray[SPELL_DISPEL_MAGIC] = new spellInfo(SPELL_MAGE, DISC_MAGE, DISC_ALCHEMY, "dispel magic", TASK_DIFFICULT, LAG_1, POSITION_SITTING, MANA_50, LIFEFORCE_0, PRAY_0, TAR_CHAR_ROOM | TAR_FIGHT_VICT | TAR_OBJ_INV | TAR_OBJ_ROOM, SYMBOL_STRESS_0, "", "", "", "", START_61, LEARN_10, START_DO_40, LEARN_DO_5, START_DO_NO, LEARN_DO_NO, LEARN_DIFF_SPELLS, 0.04, COMP_GESTURAL | COMP_GESTURAL_RANDOM | COMP_VERBAL | COMP_VERBAL_RANDOM | COMP_MATERIAL | COMP_MATERIAL_END | SPELL_TASKED, 0);

  discArray[SPELL_ENHANCE_WEAPON] = new spellInfo(SPELL_MAGE, DISC_MAGE, DISC_ALCHEMY, "enhance weapon", TASK_NORMAL, LAG_2, POSITION_CRAWLING, MANA_50, LIFEFORCE_0, PRAY_0, TAR_OBJ_INV | TAR_OBJ_EQUIP, SYMBOL_STRESS_0, "", "", "", "", START_72, LEARN_6, START_DO_60, LEARN_DO_5, START_DO_NO, LEARN_DO_NO, LEARN_DIFF_SPELLS, 0.0, COMP_GESTURAL | COMP_GESTURAL_RANDOM | COMP_VERBAL | COMP_VERBAL_RANDOM | COMP_MATERIAL | COMP_MATERIAL_END | SPELL_TASKED, 0);

  discArray[SPELL_GALVANIZE] = new spellInfo(SPELL_MAGE, DISC_MAGE, DISC_ALCHEMY, "galvanize", TASK_NORMAL, LAG_1, POSITION_CRAWLING, MANA_50, LIFEFORCE_0, PRAY_0, TAR_OBJ_INV | TAR_OBJ_EQUIP, SYMBOL_STRESS_0, "", "", "", "", START_55, LEARN_11, START_DO_40, LEARN_DO_10, START_DO_NO, LEARN_DO_NO, LEARN_DIFF_SPELLS, 0.0, COMP_GESTURAL | COMP_GESTURAL_RANDOM | COMP_VERBAL | COMP_VERBAL_RANDOM | COMP_MATERIAL | COMP_MATERIAL_END | SPELL_TASKED, TOG_HAS_GALVANIZE);

  discArray[SPELL_DETECT_INVISIBLE] = new spellInfo(SPELL_MAGE, DISC_MAGE, DISC_SPIRIT, "detect invisibility", TASK_EASY, LAG_1, POSITION_CRAWLING, MANA_15, LIFEFORCE_0, PRAY_0, TAR_CHAR_ROOM | TAR_FIGHT_SELF, SYMBOL_STRESS_0, "The yellow aura fades from your eyes.", "The yellow aura fades from $n's eyes.", "The yellow aura in your eyes flickers slightly.", "The yellow aura in $n's eyes flickers slightly.", START_12, LEARN_7, START_DO_40, LEARN_DO_5, START_DO_NO, LEARN_DO_NO, LEARN_DIFF_SPELLS, 0.04, COMP_GESTURAL | COMP_GESTURAL_RANDOM | COMP_VERBAL | COMP_VERBAL_RANDOM | COMP_MATERIAL | COMP_MATERIAL_END | SPELL_TASKED, 0);

  discArray[SPELL_DISPEL_INVISIBLE] = new spellInfo(SPELL_MAGE, DISC_MAGE, DISC_SPIRIT,"dispel invisible", TASK_NORMAL, LAG_1, POSITION_CRAWLING, MANA_30, LIFEFORCE_0, PRAY_0, TAR_CHAR_ROOM | TAR_OBJ_INV | TAR_OBJ_ROOM, SYMBOL_STRESS_0, "", "", "", "", START_50, LEARN_12, START_DO_40, LEARN_DO_5, START_DO_NO, LEARN_DO_NO, LEARN_DIFF_SPELLS, 0.04, COMP_GESTURAL | COMP_GESTURAL_RANDOM | COMP_VERBAL | COMP_VERBAL_RANDOM | COMP_MATERIAL | COMP_MATERIAL_END| SPELL_TASKED, 0);

  discArray[SPELL_TORNADO] = new spellInfo(SPELL_MAGE, DISC_MAGE, DISC_AIR, "tornado", TASK_NORMAL, LAG_1, POSITION_SITTING, MANA_23, LIFEFORCE_0, PRAY_0, TAR_FIGHT_VICT | TAR_AREA | TAR_VIOLENT | TAR_IGNORE, SYMBOL_STRESS_0, "", "", "", "", START_43, LEARN_10, START_DO_40, LEARN_DO_5, START_DO_NO, LEARN_DO_NO, LEARN_DIFF_SPELLS, 0.05, COMP_GESTURAL | COMP_GESTURAL_RANDOM | COMP_VERBAL | COMP_VERBAL_RANDOM | COMP_MATERIAL | COMP_MATERIAL_ALMOST_END | SPELL_TASKED, TOG_HAS_TORNADO);

  discArray[SPELL_SAND_BLAST] = new spellInfo(SPELL_MAGE, DISC_MAGE, DISC_EARTH, "sand blast", TASK_DIFFICULT, LAG_2, POSITION_SITTING, MANA_35, LIFEFORCE_0, PRAY_0, TAR_AREA | TAR_FIGHT_VICT | TAR_VIOLENT | TAR_IGNORE, SYMBOL_STRESS_0, "", "", "", "", START_58, LEARN_8, START_DO_40, LEARN_DO_5, START_DO_NO, LEARN_DO_NO, LEARN_DIFF_SPELLS, 0.04, COMP_GESTURAL | COMP_GESTURAL_RANDOM | COMP_VERBAL | COMP_VERBAL_RANDOM | COMP_MATERIAL | COMP_MATERIAL_END | SPELL_TASKED, 0);

  discArray[SPELL_ICE_STORM] = new spellInfo(SPELL_MAGE, DISC_MAGE, DISC_WATER, "ice storm", TASK_NORMAL, LAG_1, POSITION_SITTING, MANA_35, LIFEFORCE_0, PRAY_0, TAR_AREA | TAR_IGNORE | TAR_VIOLENT, SYMBOL_STRESS_0, "", "", "", "", START_58, LEARN_12, START_DO_40, LEARN_DO_5, START_DO_NO, LEARN_DO_NO, LEARN_DIFF_SPELLS,0.02, COMP_GESTURAL | COMP_GESTURAL_RANDOM | COMP_VERBAL | COMP_VERBAL_RANDOM | COMP_MATERIAL | COMP_MATERIAL_END | SPELL_TASKED, TOG_HAS_ICE_STORM);

  discArray[SPELL_FLAMING_SWORD] = new spellInfo(SPELL_MAGE, DISC_MAGE, DISC_FIRE, "flaming sword", TASK_NORMAL, LAG_1, POSITION_SITTING, MANA_40, LIFEFORCE_0, PRAY_0, TAR_CHAR_ROOM | TAR_SELF_NONO | TAR_VIOLENT | TAR_FIGHT_VICT , SYMBOL_STRESS_0, "", "", "", "", START_55, LEARN_20, START_DO_40, LEARN_DO_5, START_DO_NO, LEARN_DO_NO, LEARN_DIFF_SPELLS, 0.05, COMP_GESTURAL | COMP_GESTURAL_RANDOM | COMP_VERBAL | COMP_VERBAL_RANDOM | COMP_MATERIAL | COMP_MATERIAL_END | SPELL_TASKED, 0);
 
  discArray[SPELL_ACID_BLAST] = new spellInfo(SPELL_MAGE, DISC_MAGE, DISC_SORCERY,"acid blast", TASK_NORMAL, LAG_2, POSITION_SITTING, MANA_53, LIFEFORCE_0, PRAY_0, TAR_AREA | TAR_IGNORE | TAR_FIGHT_VICT | TAR_VIOLENT, SYMBOL_STRESS_0, "", "", "", "", START_75, LEARN_9, START_DO_40, LEARN_DO_5, START_DO_NO, LEARN_DO_NO, LEARN_DIFF_SPELLS, 0.04, COMP_GESTURAL | COMP_GESTURAL_RANDOM | COMP_VERBAL | COMP_VERBAL_RANDOM | COMP_MATERIAL | COMP_MATERIAL_END | SPELL_TASKED, 0);

  discArray[SPELL_FIREBALL] = new spellInfo(SPELL_MAGE, DISC_MAGE, DISC_FIRE, "fireball", TASK_NORMAL, LAG_2, POSITION_SITTING, MANA_33, LIFEFORCE_0, PRAY_0, TAR_AREA | TAR_IGNORE | TAR_VIOLENT, SYMBOL_STRESS_0, "", "", "", "", START_40, LEARN_15, START_DO_45, LEARN_DO_5, START_DO_NO, LEARN_DO_NO, LEARN_DIFF_SPELLS, 0.04, COMP_GESTURAL | COMP_GESTURAL_RANDOM | COMP_VERBAL | COMP_VERBAL_RANDOM | COMP_MATERIAL | COMP_MATERIAL_END | SPELL_TASKED, TOG_HAS_FIREBALL);

  discArray[SPELL_FARLOOK] = new spellInfo(SPELL_MAGE, DISC_MAGE, DISC_ALCHEMY, "farlook", TASK_DIFFICULT, LAG_1, POSITION_CRAWLING, MANA_20, LIFEFORCE_0, PRAY_0, TAR_CHAR_VIS_WORLD, SYMBOL_STRESS_0, "", "", "", "", START_75, LEARN_7, START_DO_45, LEARN_DO_5, START_DO_NO, LEARN_DO_NO, LEARN_DIFF_SPELLS, 0.0, COMP_GESTURAL | COMP_GESTURAL_RANDOM | COMP_VERBAL | COMP_VERBAL_RANDOM | COMP_MATERIAL | COMP_MATERIAL_END | SPELL_TASKED, 0);

  discArray[SPELL_FALCON_WINGS] = new spellInfo(SPELL_MAGE, DISC_MAGE, DISC_AIR, "falcon wings", TASK_EASY, LAG_2, POSITION_CRAWLING, MANA_20, LIFEFORCE_0, PRAY_0, TAR_CHAR_ROOM | TAR_FIGHT_SELF, SYMBOL_STRESS_0, "The last of your feathers fall from your arms!", "The feather's on $n's arms have completely fallen off.","A few of the feathers on your arm begin to fall off.","A few feathers fall from $n's arms.", START_53, LEARN_7, START_DO_40, LEARN_DO_5, START_DO_NO, LEARN_DO_NO, LEARN_DIFF_SPELLS, 0.04, COMP_GESTURAL | COMP_GESTURAL_RANDOM | COMP_VERBAL | COMP_VERBAL_RANDOM | COMP_MATERIAL | COMP_MATERIAL_END | SPELL_TASKED, 0);

  discArray[SPELL_INVISIBILITY] = new spellInfo(SPELL_MAGE, DISC_MAGE, DISC_SORCERY, "invisibility", TASK_NORMAL, LAG_2, POSITION_CRAWLING, MANA_30, LIFEFORCE_0, PRAY_0, TAR_CHAR_ROOM | TAR_OBJ_INV | TAR_OBJ_ROOM | TAR_OBJ_EQUIP | TAR_FIGHT_SELF, SYMBOL_STRESS_0, "You fade back into view.", "Suddenly, $n materializes from out of nowhere!", "", "", START_51, LEARN_10, START_DO_45, LEARN_DO_5, START_DO_NO, LEARN_DO_NO, LEARN_DIFF_SPELLS, 0.04, COMP_GESTURAL | COMP_GESTURAL_RANDOM | COMP_VERBAL | COMP_VERBAL_RANDOM | COMP_MATERIAL | COMP_MATERIAL_END | SPELL_TASKED, 0);

#if 1
  discArray[SPELL_ENSORCER] = new spellInfo(SPELL_MAGE, DISC_MAGE, DISC_SORCERY, "ensorcer", TASK_DIFFICULT, LAG_1, POSITION_CRAWLING, MANA_40, LIFEFORCE_0, PRAY_0, TAR_CHAR_ROOM | TAR_SELF_NONO | TAR_FIGHT_VICT | TAR_VIOLENT, SYMBOL_STRESS_0, "You shake your head really hard and free yourself from the charm.", "$n shakes $s head really hard and frees $mself from the charm.", "You momentarily become cognizant of The World around you.", "You see a brief flicker of intelligence in $n's eyes.", START_43, LEARN_11, START_DO_45, LEARN_DO_5, START_DO_NO, LEARN_DO_NO, LEARN_DIFF_SPELLS, 0.04, COMP_GESTURAL | COMP_GESTURAL_RANDOM | COMP_VERBAL | COMP_VERBAL_RANDOM | COMP_MATERIAL | COMP_MATERIAL_END | SPELL_TASKED, 0);
#endif

  discArray[SPELL_EYES_OF_FERTUMAN] = new spellInfo(SPELL_MAGE, DISC_MAGE, DISC_ALCHEMY, "eyes of Fertuman", TASK_NORMAL, LAG_1, POSITION_CRAWLING, MANA_30, LIFEFORCE_0, PRAY_0, TAR_NAME, SYMBOL_STRESS_0, "", "", "", "", START_65, LEARN_13, START_DO_50, LEARN_DO_5, START_DO_NO, LEARN_DO_NO, LEARN_DIFF_SPELLS, 0.0, COMP_GESTURAL | COMP_GESTURAL_RANDOM | COMP_VERBAL | COMP_VERBAL_RANDOM | COMP_MATERIAL | COMP_MATERIAL_END | SPELL_TASKED, 0);

  discArray[SPELL_COPY] = new spellInfo(SPELL_MAGE, DISC_MAGE, DISC_ALCHEMY, "copy", TASK_DIFFICULT, LAG_3, POSITION_CRAWLING, MANA_40, LIFEFORCE_0, PRAY_0, TAR_OBJ_INV | TAR_OBJ_EQUIP, SYMBOL_STRESS_0, "", "", "", "", START_70, LEARN_6, START_DO_35, LEARN_DO_5, START_DO_NO, LEARN_DO_NO, LEARN_DIFF_SPELLS, 0.0, COMP_GESTURAL | COMP_GESTURAL_RANDOM | COMP_VERBAL | COMP_VERBAL_RANDOM | COMP_MATERIAL | COMP_MATERIAL_END | SPELL_TASKED, 0);

  discArray[SPELL_HASTE] = new spellInfo(SPELL_MAGE, DISC_MAGE, DISC_SPIRIT, "haste", TASK_DIFFICULT, LAG_2, POSITION_CRAWLING, MANA_50, LIFEFORCE_0, PRAY_0, TAR_CHAR_ROOM | TAR_FIGHT_SELF, SYMBOL_STRESS_0, "You lose the bounce in your step.", "$n loses the bounce in $s step.", "", "", START_68, LEARN_17, START_DO_50, LEARN_DO_5, START_DO_NO, LEARN_DO_NO, LEARN_DIFF_SPELLS, 0.04, COMP_GESTURAL | COMP_GESTURAL_RANDOM | COMP_VERBAL | COMP_VERBAL_RANDOM | COMP_MATERIAL | COMP_MATERIAL_END | SPELL_TASKED, 0);

  discArray[SKILL_REPAIR_MAGE] = new spellInfo(SKILL_MAGE, DISC_MAGE, DISC_BLACKSMITHING, "mage repair", TASK_NORMAL, LAG_0, POSITION_SITTING, MANA_0, LIFEFORCE_0, PRAY_0, 0, SYMBOL_STRESS_0, "", "", "", "", START_50, LEARN_3, START_DO_1, LEARN_DO_1, START_DO_NO, LEARN_DO_NO, LEARN_DIFF_SKILLS, 0.0, 0, 0);


// disc_air

  discArray[SPELL_IMMOBILIZE] = new spellInfo(SPELL_MAGE, DISC_AIR, DISC_AIR, "immobilize", TASK_DIFFICULT, LAG_2, POSITION_SITTING,MANA_20, LIFEFORCE_0, PRAY_0, TAR_VIOLENT | TAR_FIGHT_VICT  | TAR_CHAR_ROOM | TAR_SELF_NONO, SYMBOL_STRESS_0, "", "", "", "", START_37, LEARN_9, START_DO_45, LEARN_DO_5, START_DO_NO, LEARN_DO_NO, LEARN_DIFF_SPELLS, 0.06, COMP_GESTURAL | COMP_GESTURAL_RANDOM | COMP_VERBAL | COMP_VERBAL_RANDOM | COMP_MATERIAL | COMP_MATERIAL_END | SPELL_TASKED, 0);

  discArray[SPELL_SUFFOCATE] = new spellInfo(SPELL_MAGE, DISC_AIR, DISC_AIR, "suffocate", TASK_DIFFICULT, LAG_2, POSITION_SITTING, MANA_27, LIFEFORCE_0, PRAY_0, TAR_VIOLENT | TAR_FIGHT_VICT  | TAR_CHAR_ROOM | TAR_SELF_NONO, SYMBOL_STRESS_0, "", "", "", "", START_60, LEARN_4, START_DO_45, LEARN_DO_5, START_DO_NO, LEARN_DO_NO, LEARN_DIFF_SPELLS, 0.08, COMP_GESTURAL | COMP_GESTURAL_RANDOM | COMP_VERBAL | COMP_VERBAL_RANDOM | COMP_MATERIAL | COMP_MATERIAL_END | SPELL_TASKED, 0);

  discArray[SPELL_FLY] = new spellInfo(SPELL_MAGE, DISC_AIR, DISC_AIR, "flight", TASK_NORMAL, LAG_1, POSITION_SITTING, MANA_30, LIFEFORCE_0, PRAY_0, TAR_CHAR_ROOM | TAR_FIGHT_SELF, SYMBOL_STRESS_0, "Your ability to fly leaves you.", "$n loses $s ability to fly.", "You feel slightly heavier.", "$n doesn't seem as flight worthy as before.", START_32, LEARN_11, START_DO_50, LEARN_DO_5, START_DO_NO, LEARN_DO_NO, LEARN_DIFF_SPELLS, 0.04, COMP_GESTURAL | COMP_GESTURAL_RANDOM | COMP_VERBAL | COMP_VERBAL_RANDOM | COMP_MATERIAL | COMP_MATERIAL_END | SPELL_TASKED, 0);
 
  discArray[SPELL_ANTIGRAVITY] = new spellInfo(SPELL_MAGE, DISC_AIR, DISC_AIR, "antigravity", TASK_NORMAL, LAG_1, POSITION_CRAWLING, MANA_50, LIFEFORCE_0, PRAY_0, TAR_AREA | TAR_IGNORE, SYMBOL_STRESS_0, "The forces of nature have resumed their regular strength", "$n sinks back onto the $g.", "You begin to feel the tug of The World again.", "", START_67, LEARN_10, START_DO_50, LEARN_DO_5, START_DO_NO, LEARN_DO_NO, LEARN_DIFF_SPELLS, 0.04, COMP_GESTURAL | COMP_GESTURAL_RANDOM | COMP_VERBAL | COMP_VERBAL_RANDOM | COMP_MATERIAL | COMP_MATERIAL_END | SPELL_TASKED, 0);

// disc_alchemy

  discArray[SPELL_DIVINATION] = new spellInfo(SPELL_MAGE, DISC_ALCHEMY, DISC_ALCHEMY, "divination", TASK_NORMAL, LAG_1, POSITION_CRAWLING, MANA_35, LIFEFORCE_0, PRAY_0, TAR_OBJ_INV | TAR_CHAR_ROOM, SYMBOL_STRESS_0, "", "", "", "", START_68, LEARN_10, START_DO_55, LEARN_DO_5, START_DO_NO, LEARN_DO_NO, LEARN_DIFF_SPELLS, 0.0, COMP_GESTURAL | COMP_GESTURAL_RANDOM | COMP_VERBAL | COMP_VERBAL_RANDOM | COMP_MATERIAL | COMP_MATERIAL_END | SPELL_TASKED, 0);

  discArray[SPELL_SHATTER] = new spellInfo(SPELL_MAGE, DISC_ALCHEMY, DISC_ALCHEMY, "shatter", TASK_NORMAL, LAG_1, POSITION_SITTING, MANA_40, LIFEFORCE_0, PRAY_0, TAR_CHAR_ROOM, SYMBOL_STRESS_0, "", "", "", "", START_8, LEARN_34, START_DO_50, LEARN_DO_5, START_DO_NO, LEARN_DO_NO, LEARN_DIFF_SPELLS, 0.04, COMP_GESTURAL | COMP_GESTURAL_RANDOM | COMP_VERBAL | COMP_VERBAL_RANDOM | COMP_MATERIAL | COMP_MATERIAL_END |SPELL_TASKED, 0);

  discArray[SKILL_SCRIBE] = new spellInfo(SPELL_MAGE, DISC_ALCHEMY, DISC_ALCHEMY, "scribe", TASK_NORMAL, LAG_0, POSITION_STANDING, MANA_0, LIFEFORCE_0, PRAY_0, 0, SYMBOL_STRESS_0, "", "", "", "", START_1, LEARN_1, START_DO_1, LEARN_DO_5, START_DO_NO, LEARN_DO_NO, LEARN_DIFF_SPELLS, 0.0, 0, 0);

  discArray[SPELL_SPONTANEOUS_GENERATION] = new spellInfo(SPELL_MAGE, DISC_ALCHEMY, DISC_ALCHEMY, "spontaneous generation", TASK_NORMAL, LAG_1, POSITION_CRAWLING, MANA_35, LIFEFORCE_0, PRAY_0, TAR_NAME, SYMBOL_STRESS_0, "", "", "", "", START_25, LEARN_5, START_DO_50, LEARN_DO_5, START_DO_NO, LEARN_DO_NO, LEARN_DIFF_SPELLS, 0.0, COMP_GESTURAL | COMP_VERBAL | SPELL_TASKED, 0);

  discArray[SKILL_STAVECHARGE] = new spellInfo(SKILL_MAGE, DISC_ALCHEMY, DISC_ALCHEMY, "charge stave", TASK_NORMAL, LAG_0, POSITION_SITTING, MANA_0, LIFEFORCE_0, PRAY_0, 0, SYMBOL_STRESS_0, "", "", "", "", START_1, LEARN_1, START_DO_20, LEARN_DO_2, START_DO_NO, LEARN_DO_NO, LEARN_DIFF_SPELLS, 0.0, 0, 0);

// disc_earth

  discArray[SPELL_METEOR_SWARM] = new spellInfo(SPELL_MAGE, DISC_EARTH, DISC_EARTH, "meteor swarm", TASK_NORMAL, LAG_3, POSITION_SITTING, MANA_40, LIFEFORCE_0, PRAY_0, TAR_CHAR_ROOM | TAR_SELF_NONO | TAR_VIOLENT | TAR_FIGHT_VICT , SYMBOL_STRESS_0, "", "", "", "", START_50, LEARN_5, START_DO_50, LEARN_DO_5, START_DO_NO, LEARN_DO_NO, LEARN_DIFF_SPELLS, 0.08, COMP_GESTURAL | COMP_GESTURAL_RANDOM | COMP_VERBAL | COMP_VERBAL_RANDOM | COMP_MATERIAL | COMP_MATERIAL_END | SPELL_TASKED, 0);

  discArray[SPELL_LAVA_STREAM] = new spellInfo(SPELL_MAGE, DISC_EARTH, DISC_EARTH, "lava stream", TASK_DIFFICULT, LAG_3, POSITION_SITTING, MANA_50, LIFEFORCE_0, PRAY_0, TAR_AREA | TAR_FIGHT_VICT | TAR_VIOLENT | TAR_IGNORE, SYMBOL_STRESS_0, "", "", "", "", START_75, LEARN_5, START_DO_50, LEARN_DO_5, START_DO_NO, LEARN_DO_NO, LEARN_DIFF_SPELLS, 0.04, COMP_GESTURAL | COMP_GESTURAL_RANDOM | COMP_VERBAL | COMP_VERBAL_RANDOM | COMP_MATERIAL | COMP_MATERIAL_END | SPELL_TASKED, 0);

  discArray[SPELL_STONE_SKIN] = new spellInfo(SPELL_MAGE, DISC_EARTH, DISC_EARTH, "stone skin", TASK_EASY, LAG_1, POSITION_CRAWLING, MANA_40, LIFEFORCE_0, PRAY_0, TAR_CHAR_ROOM | TAR_SELF_ONLY, SYMBOL_STRESS_0, "Your skin returns to normal.", "$n's skin returns to normal.", "Your skin doesn't feel as hard as rock anymore.", "$n's skin doesn't seem to be as hard as rock anymore.", START_1, LEARN_14, START_DO_50, LEARN_DO_5, START_DO_NO, LEARN_DO_NO, LEARN_DIFF_SPELLS, 0.04, COMP_GESTURAL | COMP_GESTURAL_RANDOM | COMP_VERBAL | COMP_VERBAL_RANDOM | COMP_MATERIAL | COMP_MATERIAL_END | SPELL_TASKED, TOG_HAS_STONESKIN);

  discArray[SPELL_TRAIL_SEEK] = new spellInfo(SPELL_MAGE, DISC_EARTH, DISC_EARTH, "trail seek", TASK_NORMAL, LAG_1, POSITION_CRAWLING, MANA_20, LIFEFORCE_0, PRAY_0, TAR_CHAR_ROOM | TAR_FIGHT_SELF, SYMBOL_STRESS_0, "The blue hue fades from your eyes.", "$n's eyes lose their blue hue.", "You seem to be less attuned to your senses.", "", START_1, LEARN_10, START_DO_50, LEARN_DO_5, START_DO_NO, LEARN_DO_NO, LEARN_DIFF_SPELLS, 0.01, COMP_GESTURAL | COMP_GESTURAL_RANDOM | COMP_VERBAL | COMP_VERBAL_RANDOM | COMP_MATERIAL | COMP_MATERIAL_END | SPELL_TASKED, 0);

// disc_fire

  discArray[SPELL_INFERNO] = new spellInfo(SPELL_MAGE, DISC_FIRE, DISC_FIRE, "inferno", TASK_NORMAL, LAG_1, POSITION_SITTING, MANA_54, LIFEFORCE_0, PRAY_0, TAR_CHAR_ROOM | TAR_SELF_NONO | TAR_VIOLENT | TAR_FIGHT_VICT , SYMBOL_STRESS_0, "", "", "", "", START_50, LEARN_5, START_DO_50, LEARN_DO_3, START_DO_NO, LEARN_DO_NO, LEARN_DIFF_SPELLS, 0.06, COMP_GESTURAL | COMP_GESTURAL_RANDOM | COMP_VERBAL | COMP_VERBAL_RANDOM | COMP_MATERIAL | COMP_MATERIAL_INIT | SPELL_TASKED, 0);

  discArray[SPELL_HELLFIRE] = new spellInfo(SPELL_MAGE, DISC_FIRE, DISC_FIRE, "hellfire", TASK_DIFFICULT, LAG_2, POSITION_SITTING, MANA_47, LIFEFORCE_0, PRAY_0, TAR_AREA | TAR_IGNORE | TAR_VIOLENT, SYMBOL_STRESS_0, "", "", "", "", START_75, LEARN_5, START_DO_50, LEARN_DO_5, START_DO_NO, LEARN_DO_NO, LEARN_DIFF_SPELLS, 0.04, COMP_GESTURAL | COMP_GESTURAL_RANDOM | COMP_VERBAL | COMP_VERBAL_RANDOM | COMP_MATERIAL | COMP_MATERIAL_END | SPELL_TASKED, 0);

  discArray[SPELL_FLAMING_FLESH] = new spellInfo(SPELL_MAGE, DISC_FIRE, DISC_FIRE, "flaming flesh", TASK_NORMAL, LAG_1, POSITION_CRAWLING, MANA_20, LIFEFORCE_0, PRAY_0, TAR_CHAR_ROOM | TAR_SELF_ONLY, SYMBOL_STRESS_0, "The ring of fire surrounding your body fades away.", "The ring of fire surrounding $n's body fades away.", "Your ring of fire doesn't seem quite as hot as before.", "It seems cooler in here.", START_1, LEARN_14, START_DO_50, LEARN_DO_5, START_DO_NO, LEARN_DO_NO, LEARN_DIFF_SPELLS, 0.0, COMP_GESTURAL | COMP_GESTURAL_RANDOM | COMP_VERBAL | COMP_VERBAL_RANDOM | COMP_MATERIAL | COMP_MATERIAL_END | SPELL_TASKED, 0);

// disc_sorcery

  discArray[SPELL_BLAST_OF_FURY] = new spellInfo(SPELL_MAGE, DISC_SORCERY, DISC_SORCERY, "blast of fury", TASK_NORMAL, LAG_2, POSITION_SITTING, MANA_60, LIFEFORCE_0, PRAY_0, TAR_CHAR_ROOM | TAR_SELF_NONO | TAR_VIOLENT | TAR_FIGHT_VICT , SYMBOL_STRESS_0, "", "","", "", START_20, LEARN_50, START_DO_45, LEARN_DO_5, START_DO_NO, LEARN_DO_NO, LEARN_DIFF_SPELLS, 0.04, COMP_GESTURAL | COMP_GESTURAL_RANDOM | COMP_VERBAL | COMP_VERBAL_RANDOM | COMP_MATERIAL | COMP_MATERIAL_END | SPELL_TASKED, 0);
  
  discArray[SPELL_ENERGY_DRAIN] = new spellInfo(SPELL_MAGE, DISC_SORCERY, DISC_SORCERY, "energy drain", TASK_DIFFICULT, LAG_3, POSITION_SITTING, MANA_40, LIFEFORCE_0, PRAY_0, TAR_CHAR_ROOM | TAR_SELF_NONO | TAR_VIOLENT | TAR_FIGHT_VICT , SYMBOL_STRESS_0, "", "", "", "", START_40, LEARN_5, START_DO_50, LEARN_DO_2, START_DO_NO, LEARN_DO_NO, LEARN_DIFF_SPELLS, 0.04, COMP_GESTURAL | COMP_VERBAL | COMP_MATERIAL  | COMP_MATERIAL_END | SPELL_TASKED, 0);

  discArray[SPELL_ATOMIZE] = new spellInfo(SPELL_MAGE, DISC_SORCERY, DISC_SORCERY, "atomize", TASK_DIFFICULT, LAG_3, POSITION_SITTING, MANA_75, LIFEFORCE_0, PRAY_0, TAR_CHAR_ROOM | TAR_SELF_NONO | TAR_VIOLENT | TAR_FIGHT_VICT , SYMBOL_STRESS_0, "", "", "", "", START_80, LEARN_5, START_DO_35, LEARN_DO_5, START_DO_NO, LEARN_DO_NO, LEARN_DIFF_SPELLS, 0.04, COMP_GESTURAL | COMP_VERBAL | COMP_MATERIAL | SPELL_TASKED | COMP_MATERIAL_INIT, 0);

  discArray[SPELL_ANIMATE] = new spellInfo(SPELL_MAGE, DISC_SORCERY, DISC_SORCERY, "animate", TASK_DIFFICULT, LAG_2, POSITION_CRAWLING, MANA_40, LIFEFORCE_0, PRAY_0, TAR_IGNORE, SYMBOL_STRESS_0, "", "", "", "", START_1, LEARN_10, START_DO_50, LEARN_DO_5, START_DO_NO, LEARN_DO_NO, LEARN_DIFF_SPELLS, 0.0, COMP_GESTURAL | COMP_GESTURAL_RANDOM | COMP_VERBAL | COMP_VERBAL_RANDOM | COMP_MATERIAL | COMP_MATERIAL_END | SPELL_TASKED, 0); 

  discArray[SPELL_BIND] = new spellInfo(SPELL_MAGE, DISC_SORCERY, DISC_SORCERY, "bind", TASK_NORMAL, LAG_1, POSITION_SITTING, MANA_50, LIFEFORCE_0, PRAY_0, TAR_CHAR_ROOM | TAR_SELF_NONO | TAR_VIOLENT | TAR_FIGHT_VICT , SYMBOL_STRESS_0, "The webs seem to fall away from your body.", "The webs fall away from $n's body.", "The webs seem less sticky.", "The webs surrounding $n seem less sticky.", START_1, LEARN_50, START_DO_50, LEARN_DO_5, START_DO_NO, LEARN_DO_NO, LEARN_DIFF_SPELLS, 0.02, COMP_GESTURAL | COMP_VERBAL | COMP_MATERIAL | COMP_MATERIAL_END | SPELL_TASKED, 0);


// disc_spirit

  discArray[SPELL_FUMBLE] = new spellInfo(SPELL_MAGE, DISC_SPIRIT, DISC_SPIRIT, "fumble", TASK_DIFFICULT, LAG_1, POSITION_SITTING, MANA_35, LIFEFORCE_0, PRAY_0, TAR_CHAR_ROOM | TAR_VIOLENT, SYMBOL_STRESS_0, "", "", "", "", START_60, LEARN_10, START_DO_50, LEARN_DO_5, START_DO_NO, LEARN_DO_NO, LEARN_DIFF_SPELLS, 0.03, COMP_GESTURAL | COMP_GESTURAL_RANDOM | COMP_VERBAL | COMP_VERBAL_RANDOM | COMP_MATERIAL | COMP_MATERIAL_END | SPELL_TASKED, 0);

  discArray[SPELL_TRUE_SIGHT] = new spellInfo(SPELL_MAGE, DISC_SPIRIT, DISC_SPIRIT, "true sight", TASK_NORMAL, LAG_1, POSITION_CRAWLING, MANA_25, LIFEFORCE_0, PRAY_0, TAR_CHAR_ROOM | TAR_FIGHT_SELF, SYMBOL_STRESS_0, "The silver aura in your eyes fades.", "the silver aura in $n's eyes fades.", "The silver aura in your eyes flickers slightly.", "The silver aura in $n's eyes fades slightly.", START_1, LEARN_10, START_DO_40, LEARN_DO_5, START_DO_NO, LEARN_DO_NO, LEARN_DIFF_SPELLS, 0.04, COMP_GESTURAL | COMP_GESTURAL_RANDOM | COMP_VERBAL | COMP_VERBAL_RANDOM | COMP_MATERIAL | COMP_MATERIAL_END | SPELL_TASKED, 0);

  discArray[SPELL_CLOUD_OF_CONCEALMENT] = new spellInfo(SPELL_MAGE, DISC_SPIRIT, DISC_SPIRIT, "cloud of concealment", TASK_DIFFICULT, LAG_2, POSITION_CRAWLING, MANA_50, LIFEFORCE_0, PRAY_0, TAR_AREA | TAR_IGNORE, SYMBOL_STRESS_0, "The mystic vapors surrounding you disappear.", "$n materializes out of thin air.", "The vapor around you begins to dissipate.", "", START_20, LEARN_10, START_DO_40, LEARN_DO_5, START_DO_NO, LEARN_DO_NO, LEARN_DIFF_SPELLS, 0.04, COMP_GESTURAL | COMP_GESTURAL_RANDOM | COMP_VERBAL | COMP_VERBAL_RANDOM | COMP_MATERIAL | COMP_MATERIAL_END | SPELL_TASKED, 0);


  discArray[SPELL_SILENCE] = new spellInfo(SPELL_MAGE, DISC_SPIRIT, DISC_SPIRIT, "silence", TASK_DIFFICULT, LAG_2, POSITION_SITTING, MANA_30, LIFEFORCE_0, PRAY_0, TAR_CHAR_ROOM | TAR_VIOLENT, SYMBOL_STRESS_0, "Your muzzle disappears.", "", "", "", START_80, LEARN_10, START_DO_35, LEARN_DO_5, START_DO_NO, LEARN_DO_NO, LEARN_DIFF_SPELLS, 0.04, COMP_GESTURAL | COMP_GESTURAL_RANDOM | COMP_VERBAL | COMP_VERBAL_RANDOM | COMP_MATERIAL | COMP_MATERIAL_END | SPELL_TASKED, 0);

  discArray[SPELL_KNOT] = new spellInfo(SPELL_MAGE, DISC_SPIRIT, DISC_SPIRIT, "knot", TASK_EASY, LAG_1, POSITION_SITTING, MANA_200, LIFEFORCE_0, PRAY_0, TAR_SELF_ONLY | TAR_FIGHT_SELF | TAR_CHAR_ROOM, SYMBOL_STRESS_0, "", "", "", "", START_85, LEARN_15, START_DO_40, LEARN_DO_5, START_DO_NO, LEARN_DO_NO, LEARN_DIFF_SPELLS, 0.0, COMP_GESTURAL | COMP_GESTURAL_RANDOM | COMP_VERBAL | COMP_VERBAL_RANDOM | SPELL_TASKED, 0);



// disc_water

  discArray[SPELL_WATERY_GRAVE] = new spellInfo(SPELL_MAGE, DISC_WATER, DISC_WATER, "watery grave", TASK_NORMAL, LAG_1, POSITION_SITTING, MANA_40, LIFEFORCE_0, PRAY_0, TAR_CHAR_ROOM | TAR_SELF_NONO | TAR_VIOLENT | TAR_FIGHT_VICT , SYMBOL_STRESS_0, "", "", "", "", START_25, LEARN_3, START_DO_40, LEARN_DO_5, START_DO_NO, LEARN_DO_NO, LEARN_DIFF_SPELLS, 0.05, COMP_GESTURAL | COMP_GESTURAL_RANDOM | COMP_VERBAL | COMP_VERBAL_RANDOM | COMP_MATERIAL | COMP_MATERIAL_END | SPELL_TASKED, 0);

  discArray[SPELL_TSUNAMI] = new spellInfo(SPELL_MAGE, DISC_WATER, DISC_WATER, "tsunami", TASK_DIFFICULT, LAG_2, POSITION_SITTING, MANA_50, LIFEFORCE_0, PRAY_0, TAR_AREA | TAR_IGNORE | TAR_VIOLENT, SYMBOL_STRESS_0, "", "", "", "", START_50, LEARN_2, START_DO_40, LEARN_DO_5, START_DO_NO, LEARN_DO_NO, LEARN_DIFF_SPELLS, 0.04, COMP_GESTURAL | COMP_GESTURAL_RANDOM | COMP_VERBAL | COMP_VERBAL_RANDOM | COMP_MATERIAL | COMP_MATERIAL_END | SPELL_TASKED, 0);

  discArray[SPELL_BREATH_OF_SARAHAGE] = new spellInfo(SPELL_MAGE, DISC_WATER, DISC_WATER, "breath of Sarahage", TASK_DIFFICULT, LAG_2, POSITION_CRAWLING, MANA_35, LIFEFORCE_0, PRAY_0, TAR_AREA | TAR_IGNORE, SYMBOL_STRESS_0, "Your ability to breathe underwater leaves you.", "$n gasps briefly.", "You begin to wonder how much longer you can breathe underwater.", "", START_1, LEARN_3, START_DO_45, LEARN_DO_5, START_DO_NO, LEARN_DO_NO, LEARN_DIFF_SPELLS, 0.02, COMP_GESTURAL | COMP_GESTURAL_RANDOM | COMP_VERBAL | COMP_VERBAL_RANDOM | COMP_MATERIAL | COMP_MATERIAL_END | SPELL_TASKED, 0);

  discArray[SPELL_PLASMA_MIRROR] = new spellInfo(SPELL_MAGE, DISC_WATER, DISC_WATER, "plasma mirror", TASK_DIFFICULT, LAG_3, POSITION_CRAWLING, MANA_50, LIFEFORCE_0, PRAY_0, TAR_SELF_ONLY | TAR_FIGHT_SELF | TAR_CHAR_ROOM, SYMBOL_STRESS_0, "The swirls of plasma about you disperse.", "The swirls of plasma about $n disperse.", "The plasma about you swirls more slowly now.", "", START_30, LEARN_4, START_DO_45, LEARN_DO_2, START_DO_NO, LEARN_DO_NO, LEARN_DIFF_SPELLS, 0.02, COMP_GESTURAL | COMP_GESTURAL_RANDOM | COMP_VERBAL | COMP_VERBAL_RANDOM | COMP_MATERIAL | COMP_MATERIAL_END | SPELL_TASKED, 0);

  discArray[SPELL_GARMULS_TAIL] = new spellInfo(SPELL_MAGE, DISC_WATER, DISC_WATER, "Garmul's tail", TASK_NORMAL, LAG_3, POSITION_CRAWLING, MANA_20, LIFEFORCE_0, PRAY_0, TAR_FIGHT_SELF | TAR_CHAR_ROOM, SYMBOL_STRESS_0, "You cease to move with fluid motion.", "$n ceases to move with fluid motion.", "Your movement is becoming less fluid.", "", START_5, LEARN_4, START_DO_45, LEARN_DO_2, START_DO_NO, LEARN_DO_NO, LEARN_DIFF_SPELLS, 0.02, COMP_GESTURAL | COMP_GESTURAL_RANDOM | COMP_VERBAL | COMP_VERBAL_RANDOM | COMP_MATERIAL | COMP_MATERIAL_END | SPELL_TASKED, 0);

// CLERIC CLASS
// disc_cleric
  discArray[SPELL_HEAL_LIGHT] = new spellInfo(SPELL_CLERIC, DISC_CLERIC, DISC_CURES, "heal light", TASK_EASY, LAG_2, POSITION_SITTING, MANA_0, LIFEFORCE_0, PRAY_100, TAR_CHAR_ROOM | TAR_FIGHT_SELF, SYMBOL_STRESS_12, "", "", "", "", START_1, LEARN_10, START_DO_50, LEARN_DO_5, START_DO_NO, LEARN_DO_NO, LEARN_DIFF_PRAYERS, 0.02, COMP_GESTURAL | SPELL_TASKED | SPELL_TASKED_EVERY, 0);

  discArray[SPELL_HARM_LIGHT] = new spellInfo(SPELL_CLERIC, DISC_CLERIC, DISC_AFFLICTIONS, "harm light", TASK_NORMAL, LAG_1, POSITION_SITTING, MANA_0, LIFEFORCE_0, PRAY_100, TAR_CHAR_ROOM | TAR_FIGHT_VICT | TAR_SELF_NONO | TAR_VIOLENT, SYMBOL_STRESS_5, "", "", "", "", START_1, LEARN_10, START_DO_50, LEARN_DO_5, START_DO_NO, LEARN_DO_NO, LEARN_DIFF_PRAYERS, 0.01, COMP_GESTURAL | COMP_VERBAL, 0);

  discArray[SPELL_CREATE_FOOD] = new spellInfo(SPELL_CLERIC, DISC_CLERIC, DISC_HAND_OF_GOD,"create food", TASK_TRIVIAL, LAG_1, POSITION_CRAWLING, MANA_0, LIFEFORCE_0, PRAY_025, TAR_IGNORE, SYMBOL_STRESS_5, "", "", "", "", START_11, LEARN_25, START_DO_40, LEARN_DO_10, START_DO_NO, LEARN_DO_NO, LEARN_DIFF_PRAYERS, 0.0, COMP_GESTURAL | COMP_VERBAL, 0);

  discArray[SPELL_CREATE_WATER] = new spellInfo(SPELL_CLERIC, DISC_CLERIC, DISC_HAND_OF_GOD, "create water", TASK_TRIVIAL, LAG_2, POSITION_CRAWLING, MANA_0, LIFEFORCE_0, PRAY_025, TAR_OBJ_INV, SYMBOL_STRESS_5, "", "","", "", START_11, LEARN_25, START_DO_40, LEARN_DO_10, START_DO_NO, LEARN_DO_NO, LEARN_DIFF_PRAYERS, 0.0, COMP_GESTURAL | COMP_VERBAL | SPELL_TASKED | SPELL_TASKED_EVERY, 0);


  discArray[SPELL_ARMOR] = new spellInfo(SPELL_CLERIC, DISC_CLERIC, DISC_AEGIS, "armor", TASK_TRIVIAL, LAG_1, POSITION_SITTING, MANA_0, LIFEFORCE_0, PRAY_025, TAR_CHAR_ROOM | TAR_FIGHT_SELF, SYMBOL_STRESS_15, "The protection around your body fades.", "The protection around $n fades.", "You feel slightly less protected.", "", START_1, LEARN_8, START_DO_40, LEARN_DO_5, START_DO_NO, LEARN_DO_NO, LEARN_DIFF_PRAYERS, 0.2, COMP_GESTURAL | COMP_VERBAL, 0);

  discArray[SPELL_BLESS] = new spellInfo(SPELL_CLERIC, DISC_CLERIC, DISC_AEGIS, "bless", TASK_TRIVIAL, LAG_1, POSITION_SITTING, MANA_0, LIFEFORCE_0, PRAY_050, TAR_OBJ_INV | TAR_OBJ_EQUIP | TAR_CHAR_ROOM |TAR_FIGHT_SELF, SYMBOL_STRESS_10, "You feel less holy.", "$n doesn't seem as holy.", "You feel like your deities are getting bored assisting you.", "", START_1, LEARN_33, START_DO_30, LEARN_DO_5, START_DO_NO, LEARN_DO_NO, LEARN_DIFF_PRAYERS, 0.05, COMP_GESTURAL | COMP_VERBAL, 0);

  discArray[SPELL_CLOT] = new spellInfo(SPELL_CLERIC, DISC_CLERIC, DISC_CURES, "clot", TASK_TRIVIAL, LAG_1, POSITION_SITTING, MANA_0, LIFEFORCE_0, PRAY_100, TAR_CHAR_ROOM | TAR_FIGHT_SELF, SYMBOL_STRESS_15, "", "", "", "", START_6, LEARN_10, START_DO_40, LEARN_DO_5, START_DO_NO, LEARN_DO_NO, LEARN_DIFF_PRAYERS, 0.04, COMP_GESTURAL | COMP_VERBAL, 0);

  discArray[SPELL_RAIN_BRIMSTONE] = new spellInfo(SPELL_CLERIC, DISC_CLERIC, DISC_WRATH, "rain brimstone", TASK_NORMAL, LAG_2, POSITION_SITTING, MANA_0, LIFEFORCE_0, PRAY_150, TAR_CHAR_ROOM | TAR_VIOLENT | TAR_FIGHT_VICT | TAR_SELF_NONO, SYMBOL_STRESS_10, "", "", "", "", START_5, LEARN_20, START_DO_20, LEARN_DO_5, START_DO_NO, LEARN_DO_NO, LEARN_DIFF_PRAYERS, 0.01, COMP_GESTURAL | COMP_VERBAL, 0);

  discArray[SPELL_HEAL_SERIOUS] = new spellInfo(SPELL_CLERIC, DISC_CLERIC, DISC_CURES, "heal serious", TASK_NORMAL, LAG_2, POSITION_SITTING, MANA_0, LIFEFORCE_0, PRAY_150, TAR_CHAR_ROOM | TAR_FIGHT_SELF, SYMBOL_STRESS_18, "", "", "", "", START_18, LEARN_5, START_DO_30, LEARN_DO_5, START_DO_NO, LEARN_DO_NO, LEARN_DIFF_PRAYERS, 0.03, COMP_GESTURAL | COMP_VERBAL | SPELL_TASKED | SPELL_TASKED_EVERY, 0);

  discArray[SPELL_HARM_SERIOUS] = new spellInfo(SPELL_CLERIC, DISC_CLERIC, DISC_AFFLICTIONS, "harm serious", TASK_NORMAL, LAG_1, POSITION_SITTING, MANA_0, LIFEFORCE_0, PRAY_150, TAR_CHAR_ROOM | TAR_SELF_NONO | TAR_FIGHT_VICT | TAR_VIOLENT , SYMBOL_STRESS_20, "", "", "", "", START_38, LEARN_5, START_DO_20, LEARN_DO_5, START_DO_NO, LEARN_DO_NO, LEARN_DIFF_PRAYERS, 0.02, COMP_GESTURAL | COMP_VERBAL, 0);

  discArray[SPELL_STERILIZE] = new spellInfo(SPELL_CLERIC, DISC_CLERIC, DISC_CURES, "sterilize", TASK_EASY, LAG_1, POSITION_SITTING, MANA_0, LIFEFORCE_0, PRAY_150, TAR_CHAR_ROOM, SYMBOL_STRESS_15, "", "", "", "", START_21, LEARN_5, START_DO_30, LEARN_DO_5, START_DO_NO, LEARN_DO_NO, LEARN_DIFF_PRAYERS, 0.04, COMP_GESTURAL | COMP_VERBAL, 0);

  discArray[SPELL_EXPEL] = new spellInfo(SPELL_CLERIC, DISC_CLERIC, DISC_CURES, "expel", TASK_EASY, LAG_2, POSITION_SITTING, MANA_0, LIFEFORCE_0, PRAY_500, TAR_CHAR_ROOM, SYMBOL_STRESS_30, "", "", "", "", START_51, LEARN_5, START_DO_50, LEARN_DO_10, START_DO_NO, LEARN_DO_NO, LEARN_DIFF_PRAYERS, 0.04, COMP_GESTURAL | COMP_VERBAL, 0);

  discArray[SPELL_CURE_DISEASE] = new spellInfo(SPELL_CLERIC, DISC_CLERIC, DISC_CURES, "cure disease", TASK_EASY, LAG_2, POSITION_SITTING, MANA_0, LIFEFORCE_0, PRAY_450, TAR_CHAR_ROOM | TAR_FIGHT_SELF, SYMBOL_STRESS_50, "", "", "", "", START_30, LEARN_5, START_DO_40, LEARN_DO_10, START_DO_NO, LEARN_DO_NO, LEARN_DIFF_PRAYERS,  0.02, COMP_GESTURAL | COMP_VERBAL, 0);

  discArray[SPELL_CURSE] = new spellInfo(SPELL_CLERIC, DISC_CLERIC, DISC_AFFLICTIONS, "curse", TASK_NORMAL, LAG_1, POSITION_FIGHTING, MANA_0, LIFEFORCE_0, PRAY_050, TAR_VIOLENT | TAR_CHAR_ROOM | TAR_OBJ_ROOM | TAR_OBJ_INV | TAR_OBJ_EQUIP | TAR_FIGHT_VICT | TAR_SELF_NONO, SYMBOL_STRESS_15, "You don't feel damned anymore.", "", "", "", START_51, LEARN_10, START_DO_20, LEARN_DO_5, START_DO_NO, LEARN_DO_NO, LEARN_DIFF_PRAYERS, 0.05, COMP_GESTURAL | COMP_VERBAL , 0);

  discArray[SPELL_REMOVE_CURSE] = new spellInfo(SPELL_CLERIC, DISC_CLERIC, DISC_AEGIS, "remove curse", TASK_EASY, LAG_2, POSITION_SITTING, MANA_0, LIFEFORCE_0, PRAY_100, TAR_CHAR_ROOM | TAR_FIGHT_SELF | TAR_OBJ_INV | TAR_OBJ_EQUIP | TAR_OBJ_ROOM, SYMBOL_STRESS_25, "", "", "", "", START_27, LEARN_5, START_DO_50, LEARN_DO_5, START_DO_NO, LEARN_DO_NO, LEARN_DIFF_PRAYERS, 0.04, COMP_GESTURAL | COMP_VERBAL, 0);

  discArray[SPELL_CURE_POISON] = new spellInfo(SPELL_CLERIC, DISC_CLERIC, DISC_AEGIS, "cure poison", TASK_EASY, LAG_2, POSITION_SITTING, MANA_0, LIFEFORCE_0, PRAY_100, TAR_CHAR_ROOM | TAR_FIGHT_SELF, SYMBOL_STRESS_35, "", "", "", "", START_11, LEARN_5, START_DO_30, LEARN_DO_10, START_DO_NO, LEARN_DO_NO, LEARN_DIFF_PRAYERS, 0.02, COMP_GESTURAL | COMP_VERBAL, 0);

  discArray[SPELL_HEAL_CRITICAL] = new spellInfo(SPELL_CLERIC, DISC_CLERIC, DISC_CURES, "heal critical", TASK_NORMAL, LAG_2, POSITION_SITTING, MANA_0, LIFEFORCE_0, PRAY_200, TAR_CHAR_ROOM | TAR_FIGHT_SELF, SYMBOL_STRESS_25, "", "", "", "", START_36, LEARN_5, START_DO_20, LEARN_DO_2, START_DO_NO, LEARN_DO_NO, LEARN_DIFF_PRAYERS, 0.04, COMP_GESTURAL | COMP_VERBAL | SPELL_TASKED | SPELL_TASKED_EVERY, 0);

  discArray[SPELL_SALVE] = new spellInfo(SPELL_CLERIC, DISC_CLERIC, DISC_CURES, "salve", TASK_TRIVIAL, LAG_1, POSITION_SITTING, MANA_0, LIFEFORCE_0, PRAY_150, TAR_CHAR_ROOM | TAR_FIGHT_SELF, SYMBOL_STRESS_10, "", "", "", "", START_15, LEARN_8, START_DO_30, LEARN_DO_2, START_DO_NO, LEARN_DO_NO, LEARN_DIFF_PRAYERS, 0.02, COMP_GESTURAL | COMP_VERBAL, 0);

  discArray[SPELL_POISON] = new spellInfo(SPELL_CLERIC, DISC_CLERIC, DISC_AFFLICTIONS, "poison", TASK_NORMAL, LAG_1, POSITION_SITTING, MANA_0, LIFEFORCE_0, PRAY_100, TAR_CHAR_ROOM | TAR_OBJ_INV | TAR_FIGHT_VICT | TAR_VIOLENT | TAR_FIGHT_VICT | TAR_SELF_NONO, SYMBOL_STRESS_45, "You feel much healthier.", "The color returns to $n's cheeks.", "", "", START_59, LEARN_10, START_DO_30, LEARN_DO_2, START_DO_NO, LEARN_DO_NO, LEARN_DIFF_PRAYERS, 0.02, COMP_GESTURAL | COMP_VERBAL, 0);

  discArray[SPELL_HARM_CRITICAL] = new spellInfo(SPELL_CLERIC, DISC_CLERIC, DISC_AFFLICTIONS, "harm critical", TASK_NORMAL, LAG_2, POSITION_SITTING, MANA_0, LIFEFORCE_0, PRAY_200, TAR_CHAR_ROOM | TAR_SELF_NONO | TAR_FIGHT_VICT | TAR_VIOLENT , SYMBOL_STRESS_35, "", "", "", "", START_56, LEARN_10, START_DO_20, LEARN_DO_5, START_DO_NO, LEARN_DO_NO, LEARN_DIFF_PRAYERS, 0.03, COMP_GESTURAL | COMP_VERBAL, 0);

  discArray[SPELL_INFECT] = new spellInfo(SPELL_CLERIC, DISC_CLERIC, DISC_AFFLICTIONS, "infect", TASK_DIFFICULT, LAG_1, POSITION_SITTING, MANA_0, LIFEFORCE_0, PRAY_150, TAR_CHAR_ROOM | TAR_FIGHT_VICT | TAR_VIOLENT | TAR_SELF_NONO, SYMBOL_STRESS_40, "", "", "", "", START_63, LEARN_8, START_DO_20, LEARN_DO_5, START_DO_NO, LEARN_DO_NO, LEARN_DIFF_PRAYERS, 0.04, COMP_GESTURAL | COMP_VERBAL, 0);

  discArray[SPELL_REFRESH] = new spellInfo(SPELL_CLERIC, DISC_CLERIC, DISC_AEGIS, "refresh", TASK_EASY, LAG_2, POSITION_SITTING, MANA_0, LIFEFORCE_0, PRAY_100, TAR_CHAR_ROOM | TAR_FIGHT_SELF, SYMBOL_STRESS_20, "", "", "", "", START_32, LEARN_7, START_DO_20, LEARN_DO_5, START_DO_NO, LEARN_DO_NO, LEARN_DIFF_PRAYERS, 0.02, COMP_GESTURAL | COMP_VERBAL, 0);

  discArray[SPELL_NUMB] = new spellInfo(SPELL_CLERIC, DISC_CLERIC, DISC_AFFLICTIONS, "numb", TASK_DANGEROUS, LAG_2, POSITION_SITTING, MANA_0, LIFEFORCE_0, PRAY_100, TAR_CHAR_ROOM | TAR_FIGHT_VICT | TAR_VIOLENT | TAR_SELF_NONO, SYMBOL_STRESS_30, "", "", "", "", START_59, LEARN_5, START_DO_40, LEARN_DO_5, START_DO_NO, LEARN_DO_NO, LEARN_DIFF_PRAYERS, 0.05, COMP_GESTURAL | COMP_VERBAL, 0);

  discArray[SPELL_DISEASE] = new spellInfo(SPELL_CLERIC, DISC_CLERIC, DISC_AFFLICTIONS, "disease", TASK_DIFFICULT, LAG_2, POSITION_SITTING, MANA_0, LIFEFORCE_0, PRAY_150, TAR_CHAR_ROOM | TAR_VIOLENT | TAR_FIGHT_VICT | TAR_SELF_NONO, SYMBOL_STRESS_45, "", "", "", "", START_56, LEARN_4, START_DO_40, LEARN_DO_2, START_DO_NO, LEARN_DO_NO, LEARN_DIFF_PRAYERS, 0.05, COMP_GESTURAL | COMP_VERBAL, 0);

  discArray[SPELL_FLAMESTRIKE] = new spellInfo(SPELL_CLERIC, DISC_CLERIC, DISC_WRATH, "flamestrike", TASK_NORMAL, LAG_3, POSITION_SITTING, MANA_0, LIFEFORCE_0, PRAY_200, TAR_CHAR_ROOM | TAR_VIOLENT | TAR_FIGHT_VICT | TAR_SELF_NONO , SYMBOL_STRESS_30, "", "", "", "", START_51, LEARN_10, START_DO_25, LEARN_DO_2, START_DO_NO, LEARN_DO_NO, LEARN_DIFF_PRAYERS,  0.04, COMP_GESTURAL | COMP_VERBAL, 0);

  discArray[SPELL_PLAGUE_LOCUSTS] = new spellInfo(SPELL_CLERIC, DISC_CLERIC, DISC_WRATH, "plague of locusts", TASK_NORMAL, LAG_3, POSITION_SITTING, MANA_0, LIFEFORCE_0, PRAY_500, TAR_CHAR_ROOM | TAR_VIOLENT | TAR_FIGHT_VICT | TAR_SELF_NONO, SYMBOL_STRESS_85, "$n circles the area once and then dissipates.", "$n circles the area once and then dissipates.", "", "", START_81, LEARN_5, START_DO_40, LEARN_DO_3, START_DO_NO, LEARN_DO_NO, LEARN_DIFF_PRAYERS, 0.01, COMP_GESTURAL | COMP_VERBAL, 0);

  discArray[SPELL_CURE_BLINDNESS] = new spellInfo(SPELL_CLERIC, DISC_CLERIC, DISC_AEGIS, "cure blindness", TASK_EASY, LAG_2, POSITION_SITTING, MANA_0, LIFEFORCE_0, PRAY_250, TAR_CHAR_ROOM | TAR_FIGHT_SELF, SYMBOL_STRESS_40, "", "", "", "", START_45, LEARN_5, START_DO_40, LEARN_DO_5, START_DO_NO, LEARN_DO_NO, LEARN_DIFF_PRAYERS, 0.02, COMP_GESTURAL | COMP_VERBAL, 0);

  discArray[SPELL_SUMMON] = new spellInfo(SPELL_CLERIC, DISC_CLERIC, DISC_HAND_OF_GOD, "summon", TASK_DIFFICULT, LAG_2, POSITION_CRAWLING, MANA_0, LIFEFORCE_0, PRAY_300, TAR_CHAR_WORLD, SYMBOL_STRESS_75, "", "", "", "", START_76, LEARN_10, START_DO_50, LEARN_DO_5, START_DO_NO, LEARN_DO_NO, LEARN_DIFF_PRAYERS, 0.0, COMP_GESTURAL | COMP_VERBAL, 0);

  discArray[SPELL_HEAL] = new spellInfo(SPELL_CLERIC, DISC_CLERIC, DISC_CURES, "heal", TASK_NORMAL, LAG_2, POSITION_SITTING, MANA_0, LIFEFORCE_0, PRAY_250, TAR_CHAR_ROOM | TAR_FIGHT_SELF, SYMBOL_STRESS_48, "", "", "", "", START_65, LEARN_5, START_DO_20, LEARN_DO_2, START_DO_NO, LEARN_DO_NO, LEARN_DIFF_PRAYERS, 0.06, COMP_GESTURAL | COMP_VERBAL | SPELL_TASKED | SPELL_TASKED_EVERY, 0);

  discArray[SPELL_PARALYZE_LIMB] = new spellInfo(SPELL_CLERIC, DISC_CLERIC, DISC_AFFLICTIONS, "paralyze limb", TASK_DANGEROUS, LAG_3, POSITION_SITTING, MANA_0, LIFEFORCE_0, PRAY_200, TAR_CHAR_ROOM | TAR_FIGHT_VICT | TAR_VIOLENT | TAR_SELF_NONO, SYMBOL_STRESS_50, "", "", "", "", START_86, LEARN_10, START_DO_20, LEARN_DO_2, START_DO_NO, LEARN_DO_NO, LEARN_DIFF_PRAYERS, 0.05, COMP_GESTURAL | COMP_VERBAL, 0);

  discArray[SPELL_WORD_OF_RECALL] = new spellInfo(SPELL_CLERIC, DISC_CLERIC, DISC_HAND_OF_GOD, "word of recall", TASK_EASY, LAG_3, POSITION_SITTING, MANA_0, LIFEFORCE_0, PRAY_500, TAR_CHAR_ROOM | TAR_FIGHT_SELF, SYMBOL_STRESS_100, "", "", "", "", START_83, LEARN_10, START_DO_40, LEARN_DO_5, START_DO_NO, LEARN_DO_NO, LEARN_DIFF_PRAYERS, 0.0, COMP_GESTURAL | COMP_VERBAL, 0);

  discArray[SPELL_HARM] = new spellInfo(SPELL_CLERIC, DISC_CLERIC, DISC_AFFLICTIONS, "harm", TASK_NORMAL, LAG_3, POSITION_SITTING, MANA_0, LIFEFORCE_0, PRAY_250, TAR_CHAR_ROOM | TAR_FIGHT_VICT | TAR_SELF_NONO | TAR_VIOLENT , SYMBOL_STRESS_50, "", "", "", "", START_79, LEARN_10, START_DO_20, LEARN_DO_3, START_DO_NO, LEARN_DO_NO, LEARN_DIFF_PRAYERS, 0.06, COMP_GESTURAL | COMP_VERBAL, 0);

  discArray[SPELL_BLINDNESS] = new spellInfo(SPELL_CLERIC, DISC_CLERIC, DISC_AFFLICTIONS,"blindness", TASK_DANGEROUS, LAG_2, POSITION_SITTING, MANA_0, LIFEFORCE_0, PRAY_250, TAR_CHAR_ROOM | TAR_VIOLENT | TAR_FIGHT_VICT | TAR_SELF_NONO, SYMBOL_STRESS_80, "You can see again!", "", "", "", START_84, LEARN_6, START_DO_20, LEARN_DO_2, START_DO_NO, LEARN_DO_NO, LEARN_DIFF_PRAYERS, 0.02, COMP_GESTURAL | COMP_VERBAL, 0);

  discArray[SKILL_REPAIR_CLERIC] = new spellInfo(SKILL_CLERIC, DISC_CLERIC, DISC_BLACKSMITHING, "cleric repair", TASK_NORMAL, LAG_0, POSITION_SITTING, MANA_0, LIFEFORCE_0, PRAY_0, 0, SYMBOL_STRESS_0, "", "", "", "", START_50, LEARN_3, START_DO_1, LEARN_DO_1, START_DO_NO, LEARN_DO_NO, LEARN_DIFF_SKILLS, 0.0, 0, 0);


// disc_wrath

  discArray[SPELL_PILLAR_SALT] = new spellInfo(SPELL_CLERIC, DISC_WRATH, DISC_WRATH, "pillar of salt", TASK_NORMAL, LAG_3, POSITION_SITTING, MANA_0, LIFEFORCE_0, PRAY_200, TAR_CHAR_ROOM | TAR_VIOLENT | TAR_FIGHT_VICT | TAR_SELF_NONO, SYMBOL_STRESS_65, "", "", "", "", START_1, LEARN_4, START_DO_20, LEARN_DO_2, START_DO_NO, LEARN_DO_NO, LEARN_DIFF_PRAYERS, 0.01, COMP_GESTURAL | COMP_VERBAL, 0);

  discArray[SPELL_EARTHQUAKE] = new spellInfo(SPELL_CLERIC, DISC_WRATH, DISC_WRATH, "earthquake", TASK_DIFFICULT, LAG_3, POSITION_SITTING, MANA_0, LIFEFORCE_0, PRAY_400, TAR_AREA | TAR_IGNORE | TAR_VIOLENT, SYMBOL_STRESS_90, "", "", "", "", START_26, LEARN_4, START_DO_20, LEARN_DO_2, START_DO_NO, LEARN_DO_NO, LEARN_DIFF_PRAYERS, 0.04, COMP_GESTURAL | COMP_VERBAL, TOG_HAS_EARTHQUAKE);
 
  discArray[SPELL_CALL_LIGHTNING] = new spellInfo(SPELL_CLERIC, DISC_WRATH, DISC_WRATH, "call lightning", TASK_DIFFICULT, LAG_2, POSITION_SITTING, MANA_0, LIFEFORCE_0, PRAY_300, TAR_CHAR_ROOM | TAR_VIOLENT | TAR_FIGHT_VICT | TAR_SELF_NONO , SYMBOL_STRESS_50, "", "", "", "", START_51, LEARN_4, START_DO_30, LEARN_DO_3, START_DO_NO, LEARN_DO_NO, LEARN_DIFF_PRAYERS, 0.06, COMP_GESTURAL | COMP_VERBAL, 0);

  discArray[SPELL_SPONTANEOUS_COMBUST] = new spellInfo(SPELL_CLERIC, DISC_WRATH, DISC_WRATH, "spontaneous combust", TASK_DIFFICULT, LAG_5, POSITION_SITTING, MANA_0, LIFEFORCE_0, PRAY_400, TAR_CHAR_ROOM | TAR_VIOLENT | TAR_FIGHT_VICT | TAR_SELF_NONO , SYMBOL_STRESS_75, "", "", "", "", START_76, LEARN_4, START_DO_30, LEARN_DO_2, START_DO_NO, LEARN_DO_NO, LEARN_DIFF_PRAYERS, 0.06, COMP_GESTURAL | COMP_VERBAL, 0);

// disc_afflictions


  discArray[SPELL_BLEED] = new spellInfo(SPELL_CLERIC, DISC_AFFLICTIONS, DISC_AFFLICTIONS, "bleed", TASK_DANGEROUS, LAG_1, POSITION_SITTING, MANA_0, LIFEFORCE_0, PRAY_100, TAR_CHAR_ROOM | TAR_FIGHT_VICT | TAR_VIOLENT | TAR_SELF_NONO, SYMBOL_STRESS_35, "", "", "", "", START_1, LEARN_4, START_DO_30, LEARN_DO_2, START_DO_NO, LEARN_DO_NO, LEARN_DIFF_PRAYERS, 0.04, COMP_GESTURAL | COMP_VERBAL, 0);

  discArray[SPELL_PARALYZE] = new spellInfo(SPELL_CLERIC, DISC_AFFLICTIONS, DISC_AFFLICTIONS, "paralyze", TASK_DANGEROUS, LAG_3, POSITION_SITTING, MANA_0, LIFEFORCE_0, PRAY_300, TAR_CHAR_ROOM | TAR_VIOLENT | TAR_FIGHT_VICT | TAR_SELF_NONO , SYMBOL_STRESS_95, "Your body unstiffens and you can feel the blood circulating again.", "", "", "", START_26, LEARN_4, START_DO_20, LEARN_DO_1, START_DO_NO, LEARN_DO_NO, LEARN_DIFF_PRAYERS, 0.02, COMP_GESTURAL | COMP_VERBAL, 0);

  discArray[SPELL_BONE_BREAKER] = new spellInfo(SPELL_CLERIC, DISC_AFFLICTIONS, DISC_AFFLICTIONS, "bone breaker", TASK_DANGEROUS, LAG_3, POSITION_SITTING, MANA_0, LIFEFORCE_0, PRAY_300, TAR_CHAR_ROOM | TAR_FIGHT_VICT | TAR_VIOLENT | TAR_SELF_NONO, SYMBOL_STRESS_85, "", "", "", "", START_51, LEARN_4, START_DO_20, LEARN_DO_1, START_DO_NO, LEARN_DO_NO, LEARN_DIFF_PRAYERS, 0.06, COMP_GESTURAL | COMP_VERBAL, 0);

  discArray[SPELL_WITHER_LIMB] = new spellInfo(SPELL_CLERIC, DISC_AFFLICTIONS, DISC_AFFLICTIONS, "wither limb", TASK_DANGEROUS, LAG_3, POSITION_SITTING, MANA_0, LIFEFORCE_0, PRAY_250, TAR_CHAR_ROOM | TAR_FIGHT_VICT | TAR_VIOLENT | TAR_SELF_NONO, SYMBOL_STRESS_70, "", "", "", "", START_76, LEARN_4, START_DO_20, LEARN_DO_2, START_DO_NO, LEARN_DO_NO, LEARN_DIFF_PRAYERS, 0.06, COMP_GESTURAL | COMP_VERBAL, 0);


// disc_aegis

  discArray[SPELL_SANCTUARY] = new spellInfo(SPELL_CLERIC, DISC_AEGIS, DISC_AEGIS, "sanctuary", TASK_EASY, LAG_3, POSITION_SITTING, MANA_0, LIFEFORCE_0, PRAY_500, TAR_CHAR_ROOM | TAR_FIGHT_SELF, SYMBOL_STRESS_75, "The white aura around your body fades.", "The white aura around $n's body fades.", "The white aura around your body flickers slightly.", "The white aura around $n's body flickers slightly.", START_1, LEARN_20, START_DO_20, LEARN_DO_2, START_DO_NO, LEARN_DO_NO, LEARN_DIFF_PRAYERS, 0.06, COMP_GESTURAL | COMP_VERBAL, 0);

  discArray[SPELL_CURE_PARALYSIS] = new spellInfo(SPELL_CLERIC, DISC_AEGIS, DISC_AEGIS, "cure paralysis", TASK_EASY, LAG_2, POSITION_SITTING, MANA_0, LIFEFORCE_0, PRAY_300, TAR_CHAR_ROOM | TAR_FIGHT_VICT, SYMBOL_STRESS_75, "", "", "", "", START_40, LEARN_4, START_DO_50, LEARN_DO_5, START_DO_NO, LEARN_DO_NO, LEARN_DIFF_PRAYERS, 0.02, COMP_GESTURAL | COMP_VERBAL, 0);

  discArray[SPELL_SECOND_WIND] = new spellInfo(SPELL_CLERIC, DISC_AEGIS, DISC_AEGIS, "second wind", TASK_NORMAL, LAG_2, POSITION_SITTING, MANA_0, LIFEFORCE_0, PRAY_300, TAR_CHAR_ROOM | TAR_FIGHT_SELF, SYMBOL_STRESS_45, "", "", "", "", START_26, LEARN_20, START_DO_30, LEARN_DO_2, START_DO_NO, LEARN_DO_NO, LEARN_DIFF_PRAYERS, 0.01, COMP_GESTURAL | COMP_VERBAL, 0);

  discArray[SPELL_RELIVE] = new spellInfo(SPELL_CLERIC, DISC_AEGIS, DISC_AEGIS, "relive", TASK_DIFFICULT, LAG_2, POSITION_SITTING, MANA_0, LIFEFORCE_0, PRAY_300, TAR_CHAR_ROOM, SYMBOL_STRESS_100, "", "", "", "", START_80, LEARN_5, START_DO_50, LEARN_DO_5, START_DO_NO, LEARN_DO_NO, LEARN_DIFF_PRAYERS, 0.02, COMP_GESTURAL | COMP_VERBAL, 0);


// disc_hand_of_god
 
  discArray[SPELL_HEROES_FEAST] = new spellInfo(SPELL_CLERIC, DISC_HAND_OF_GOD, DISC_HAND_OF_GOD, "heroes feast", TASK_EASY, LAG_2, POSITION_CRAWLING, MANA_0, LIFEFORCE_0, PRAY_075, TAR_AREA | TAR_IGNORE, SYMBOL_STRESS_15, "", "", "", "", START_1, LEARN_10, START_DO_20, LEARN_DO_5, START_DO_NO, LEARN_DO_NO, LEARN_DIFF_PRAYERS, 0.1, COMP_GESTURAL | COMP_VERBAL, 0);

  discArray[SPELL_ASTRAL_WALK] = new spellInfo(SPELL_CLERIC, DISC_HAND_OF_GOD, DISC_HAND_OF_GOD, "astral walk", TASK_DIFFICULT, LAG_2, POSITION_CRAWLING, MANA_0, LIFEFORCE_0, PRAY_200, TAR_CHAR_WORLD, SYMBOL_STRESS_50, "", "", "", "", START_33, LEARN_3, START_DO_40, LEARN_DO_5, START_DO_NO, LEARN_DO_NO, LEARN_DIFF_PRAYERS, 0.0, COMP_GESTURAL | COMP_VERBAL, 0);

  discArray[SPELL_PORTAL] = new spellInfo(SPELL_CLERIC, DISC_HAND_OF_GOD, DISC_HAND_OF_GOD, "portal", TASK_DANGEROUS, LAG_3, POSITION_CRAWLING, MANA_0, LIFEFORCE_0, PRAY_300, TAR_NAME , SYMBOL_STRESS_75, "", "", "", "", START_80, LEARN_5, START_DO_50, LEARN_DO_5, START_DO_NO, LEARN_DO_NO, LEARN_DIFF_PRAYERS, 0.0, COMP_GESTURAL | COMP_VERBAL, 0);

// disc_cures
  discArray[SPELL_HEAL_FULL] = new spellInfo(SPELL_CLERIC, DISC_CURES, DISC_CURES, "heal full", TASK_NORMAL, LAG_2, POSITION_SITTING, MANA_0, LIFEFORCE_0, PRAY_300, TAR_CHAR_ROOM | TAR_FIGHT_SELF, SYMBOL_STRESS_60, "", "", "", "", START_12, LEARN_6, START_DO_20, LEARN_DO_3, START_DO_NO, LEARN_DO_NO, LEARN_DIFF_PRAYERS, 0.07, COMP_GESTURAL | COMP_VERBAL | SPELL_TASKED | SPELL_TASKED_EVERY, 0);

  discArray[SPELL_HEAL_CRITICAL_SPRAY] = new spellInfo(SPELL_CLERIC, DISC_CURES, DISC_CURES, "heal critical spray", TASK_DIFFICULT, LAG_3, POSITION_SITTING, MANA_0, LIFEFORCE_0, PRAY_250, TAR_AREA | TAR_IGNORE, SYMBOL_STRESS_70, "", "", "", "", START_46, LEARN_6, START_DO_30, LEARN_DO_5, START_DO_NO, LEARN_DO_NO, LEARN_DIFF_PRAYERS, 0.04, COMP_GESTURAL | COMP_VERBAL, 0);

  discArray[SPELL_HEAL_SPRAY] = new spellInfo(SPELL_CLERIC, DISC_CURES, DISC_CURES, "heal spray", TASK_DANGEROUS, LAG_3, POSITION_SITTING, MANA_0, LIFEFORCE_0, PRAY_300, TAR_AREA | TAR_IGNORE, SYMBOL_STRESS_85, "", "", "", "", START_60, LEARN_6, START_DO_30, LEARN_DO_5, START_DO_NO, LEARN_DO_NO, LEARN_DIFF_PRAYERS, 0.06, COMP_GESTURAL | COMP_VERBAL, 0);

  discArray[SPELL_HEAL_FULL_SPRAY] = new spellInfo(SPELL_CLERIC, DISC_CURES, DISC_CURES, "heal full spray", TASK_DANGEROUS, LAG_3, POSITION_SITTING, MANA_0, LIFEFORCE_0, PRAY_350, TAR_AREA | TAR_IGNORE, SYMBOL_STRESS_100, "", "", "", "", START_84, LEARN_6, START_DO_30, LEARN_DO_5, START_DO_NO, LEARN_DO_NO, LEARN_DIFF_PRAYERS, 0.07, COMP_GESTURAL | COMP_VERBAL, 0);

  discArray[SPELL_RESTORE_LIMB] = new spellInfo(SPELL_CLERIC, DISC_CURES, DISC_CURES, "restore limb", TASK_NORMAL, LAG_2, POSITION_SITTING, MANA_0, LIFEFORCE_0, PRAY_250, TAR_CHAR_ROOM | TAR_FIGHT_SELF, SYMBOL_STRESS_80, "", "", "", "", START_1, LEARN_8, START_DO_50, LEARN_DO_5, START_DO_NO, LEARN_DO_NO, LEARN_DIFF_PRAYERS, 0.02, COMP_GESTURAL | COMP_VERBAL, 0);

  discArray[SPELL_KNIT_BONE] = new spellInfo(SPELL_CLERIC, DISC_CLERIC, DISC_CURES, "knit bone", TASK_NORMAL, LAG_2, POSITION_SITTING, MANA_0, LIFEFORCE_0, PRAY_300, TAR_CHAR_ROOM | TAR_FIGHT_SELF, SYMBOL_STRESS_70, "", "", "", "", START_99, LEARN_100, START_DO_40, LEARN_DO_5, START_DO_NO, LEARN_DO_NO, LEARN_DIFF_PRAYERS, 0.04, COMP_GESTURAL | COMP_VERBAL, 0);


// WARRIOR CLASS

// disc_warrior
  discArray[SKILL_KICK] = new spellInfo(SKILL_WARRIOR, DISC_WARRIOR, DISC_DUELING, "kick", TASK_EASY, LAG_3, POSITION_STANDING, MANA_0, LIFEFORCE_0, PRAY_0, 0, SYMBOL_STRESS_0, "", "", "", "", START_1, LEARN_5, START_DO_1, LEARN_DO_1, START_DO_NO, LEARN_DO_NO, LEARN_DIFF_SKILLS, 0.0, 0, 0);

  discArray[SKILL_BASH] = new spellInfo(SKILL_WARRIOR, DISC_WARRIOR, DISC_BRAWLING, "bash", TASK_NORMAL, LAG_3, POSITION_CRAWLING, MANA_0, LIFEFORCE_0, PRAY_0, 0, SYMBOL_STRESS_0, "", "", "", "", START_1, LEARN_5, START_DO_1, LEARN_DO_1, START_DO_NO, LEARN_DO_NO, LEARN_DIFF_SKILLS, 0.0, 0, 0);

  discArray[SKILL_TRIP] = new spellInfo(SKILL_WARRIOR, DISC_WARRIOR, DISC_BRAWLING, "trip", TASK_NORMAL, LAG_3, POSITION_CRAWLING, MANA_0, LIFEFORCE_0, PRAY_0, 0, SYMBOL_STRESS_0, "", "", "", "", START_1, LEARN_5, START_DO_1, LEARN_DO_1, START_DO_NO, LEARN_DO_NO, LEARN_DIFF_SKILLS, 0.0, 0, 0);

  discArray[SKILL_HEADBUTT] = new spellInfo(SKILL_WARRIOR, DISC_WARRIOR, DISC_BRAWLING, "headbutt", TASK_NORMAL, LAG_4, POSITION_CRAWLING, MANA_0, LIFEFORCE_0, PRAY_0, 0, SYMBOL_STRESS_0, "", "", "", "", START_41, LEARN_5, START_DO_1, LEARN_DO_2, START_DO_NO, LEARN_DO_NO, LEARN_DIFF_SKILLS, 0.0, 0, 0);

  discArray[SKILL_RESCUE] = new spellInfo(SKILL_WARRIOR, DISC_WARRIOR, DISC_DUELING, "rescue", TASK_NORMAL, LAG_2, POSITION_CRAWLING, MANA_0, LIFEFORCE_0, PRAY_0, 0, SYMBOL_STRESS_0, "", "", "", "", START_11, LEARN_3, START_DO_10, LEARN_DO_1, START_DO_NO, LEARN_DO_NO, LEARN_DIFF_SKILLS, 0.15, 0, 0);

  discArray[SKILL_BLACKSMITHING] = new spellInfo(SKILL_WARRIOR, DISC_WARRIOR, DISC_BLACKSMITHING, "blacksmithing", TASK_NORMAL, LAG_0, POSITION_SITTING, MANA_0, LIFEFORCE_0, PRAY_0, 0, SYMBOL_STRESS_0, "", "", "", "", START_50, LEARN_3, START_DO_1, LEARN_DO_1, START_DO_NO, LEARN_DO_NO, LEARN_DIFF_SKILLS, 0.0, 0, 0);

  discArray[SKILL_DISARM] = new spellInfo(SKILL_WARRIOR, DISC_WARRIOR, DISC_DUELING, "disarm", TASK_DANGEROUS, LAG_2, POSITION_CRAWLING, MANA_0, LIFEFORCE_0, PRAY_0, 0, SYMBOL_STRESS_0, "", "", "", "", START_35, LEARN_3, START_DO_1, LEARN_DO_2, START_DO_NO, LEARN_DO_NO, LEARN_DIFF_SKILLS, 0.0, 0, 0);

  discArray[SKILL_BERSERK] = new spellInfo(SKILL_WARRIOR, DISC_WARRIOR, DISC_SOLDIERING, "berserk", TASK_NORMAL, LAG_2, POSITION_CRAWLING, MANA_0, LIFEFORCE_0, PRAY_0, 0, SYMBOL_STRESS_0, "", "", "", "", START_1, LEARN_2, START_DO_1, LEARN_DO_1, START_DO_NO, LEARN_DO_NO, LEARN_DIFF_SKILLS, 0.0, 0, 0);

  discArray[SKILL_SWITCH_OPP] = new spellInfo(SKILL_WARRIOR, DISC_WARRIOR, DISC_DUELING, "switch opponents", TASK_NORMAL, LAG_2, POSITION_CRAWLING, MANA_0, LIFEFORCE_0, PRAY_0, 0, SYMBOL_STRESS_0, "", "", "", "", START_61, LEARN_3, START_DO_10, LEARN_DO_2, START_DO_NO, LEARN_DO_NO, LEARN_DIFF_SKILLS, 0.0, 0, 0);

 discArray[SKILL_KNEESTRIKE] = new spellInfo(SKILL_WARRIOR, DISC_WARRIOR, DISC_DUELING,"kneestrike", TASK_NORMAL, LAG_4, POSITION_STANDING, MANA_0, LIFEFORCE_0, PRAY_0, 0, SYMBOL_STRESS_0, "", "", "", "", START_60, LEARN_4, START_DO_1, LEARN_DO_2, START_DO_NO, LEARN_DO_NO, LEARN_DIFF_SKILLS, 0.0, 0, 0);


// disc_dueling

  discArray[SKILL_RETREAT] = new spellInfo(SKILL_WARRIOR, DISC_DUELING, DISC_DUELING, "retreat", TASK_NORMAL, LAG_0, POSITION_CRAWLING, MANA_0, LIFEFORCE_0, PRAY_0, 0, SYMBOL_STRESS_0, "", "", "", "", START_1, LEARN_1, START_DO_1, LEARN_DO_1, START_DO_NO, LEARN_DO_NO, LEARN_DIFF_UNUSUAL, 0.0, 0, 0);

  discArray[SKILL_SHOVE] = new spellInfo(SKILL_WARRIOR, DISC_DUELING, DISC_DUELING, "shove", TASK_NORMAL, LAG_3, POSITION_CRAWLING, MANA_0, LIFEFORCE_0, PRAY_0, 0, SYMBOL_STRESS_0, "", "", "", "", START_21, LEARN_4, START_DO_1, LEARN_DO_2, START_DO_NO, LEARN_DO_NO, LEARN_DIFF_SKILLS, 0.0, 0, 0);

  discArray[SKILL_PARRY_WARRIOR] = new spellInfo(SKILL_WARRIOR, DISC_DUELING, DISC_DUELING, "parry", TASK_NORMAL, LAG_0, POSITION_DEAD, MANA_0, LIFEFORCE_0, PRAY_0, 0, SYMBOL_STRESS_0, "", "", "", "", START_1, LEARN_1,START_DO_1, LEARN_DO_1, START_DO_NO, LEARN_DO_NO, LEARN_DIFF_UNUSUAL, 0.0, 0, 0);

  discArray[SKILL_TRANCE_OF_BLADES] = new spellInfo(SKILL_WARRIOR, DISC_DUELING, DISC_DUELING, "defensive trance", TASK_DIFFICULT, LAG_4, POSITION_CRAWLING, MANA_0, LIFEFORCE_0, PRAY_0, 0, SYMBOL_STRESS_0, "","","","", START_75, LEARN_4, START_DO_10, LEARN_DO_1, START_DO_NO, LEARN_DO_NO, LEARN_DIFF_SKILLS, 0.0, 0, 0);

  discArray[SKILL_WEAPON_RETENTION] = new spellInfo(SKILL_WARRIOR, DISC_DUELING, DISC_DUELING, "weapon retention", TASK_DIFFICULT, LAG_4, POSITION_CRAWLING, MANA_0, LIFEFORCE_0, PRAY_0, 0, SYMBOL_STRESS_0, "","","","", START_75, LEARN_4, START_DO_10, LEARN_DO_1, START_DO_NO, LEARN_DO_NO, LEARN_DIFF_SKILLS, 0.0, 0, 0);


// disc_brawling

  discArray[SKILL_GRAPPLE] = new spellInfo(SKILL_WARRIOR, DISC_BRAWLING, DISC_BRAWLING, "grapple", TASK_NORMAL, LAG_4, POSITION_CRAWLING, MANA_0, LIFEFORCE_0, PRAY_0, 0, SYMBOL_STRESS_0, "", "", "", "", START_1, LEARN_3, START_DO_10, LEARN_DO_2, START_DO_NO, LEARN_DO_NO, LEARN_DIFF_SKILLS, 0.0, 0, 0);

  discArray[SKILL_STOMP] = new spellInfo(SKILL_WARRIOR, DISC_BRAWLING, DISC_BRAWLING, "stomp", TASK_NORMAL, LAG_4, POSITION_CRAWLING, MANA_0, LIFEFORCE_0, PRAY_0, 0, SYMBOL_STRESS_0, "", "", "", "", START_50, LEARN_2, START_DO_10, LEARN_DO_1, START_DO_NO, LEARN_DO_NO, LEARN_DIFF_SKILLS, 0.0, 0, 0);

  discArray[SKILL_BRAWL_AVOIDANCE] = new spellInfo(SKILL_WARRIOR, DISC_BRAWLING, DISC_BRAWLING, "brawl avoidance", TASK_NORMAL, LAG_4, POSITION_CRAWLING, MANA_0, LIFEFORCE_0, PRAY_0, 0, SYMBOL_STRESS_0, "", "", "", "", START_75, LEARN_4, START_DO_10, LEARN_DO_1, START_DO_NO, LEARN_DO_NO, LEARN_DIFF_SKILLS, 0.0, 0, 0);

  discArray[SKILL_CLOSE_QUARTERS_FIGHTING] = new spellInfo(SKILL_WARRIOR, DISC_BRAWLING, DISC_BRAWLING, "close quarters fighting", TASK_NORMAL, LAG_4, POSITION_CRAWLING, MANA_0, LIFEFORCE_0, PRAY_0, 0, SYMBOL_STRESS_0, "", "", "", "", START_85, LEARN_4, START_DO_10, LEARN_DO_1, START_DO_NO, LEARN_DO_NO, LEARN_DIFF_SKILLS, 0.0, 0, 0);


  discArray[SKILL_BODYSLAM] = new spellInfo(SKILL_WARRIOR, DISC_BRAWLING, DISC_BRAWLING, "bodyslam", TASK_NORMAL, LAG_4, POSITION_CRAWLING, MANA_0, LIFEFORCE_0, PRAY_0, 0, SYMBOL_STRESS_0, "", "", "", "", START_30, LEARN_4, START_DO_10, LEARN_DO_1, START_DO_NO, LEARN_DO_NO, LEARN_DIFF_SKILLS, 0.0, 0, 0);

  discArray[SKILL_SPIN] = new spellInfo(SKILL_WARRIOR, DISC_BRAWLING, DISC_BRAWLING, "spin", TASK_NORMAL, LAG_6, POSITION_CRAWLING, MANA_0, LIFEFORCE_0, PRAY_0, 0, SYMBOL_STRESS_0, "", "", "", "", START_45, LEARN_4, START_DO_10, LEARN_DO_1, START_DO_NO, LEARN_DO_NO, LEARN_DIFF_SKILLS, 0.0, 0, 0);
// disc_soldiering

  discArray[SKILL_DOORBASH] = new spellInfo(SKILL_WARRIOR, DISC_SOLDIERING, DISC_SOLDIERING, "doorbash", TASK_NORMAL, LAG_3, POSITION_CRAWLING, MANA_0, LIFEFORCE_0, PRAY_0, 0, SYMBOL_STRESS_0, "You shake the cobwebs from your brain.", "$n shakes $s head and comes back to reality.", "", "", START_1, LEARN_2, START_DO_10, LEARN_DO_1, START_DO_NO, LEARN_DO_NO, LEARN_DIFF_SKILLS, 0.0, 0, 0);

  discArray[SKILL_DEATHSTROKE] = new spellInfo(SKILL_WARRIOR, DISC_SOLDIERING, DISC_SOLDIERING, "deathstroke", TASK_NORMAL, LAG_5, POSITION_CRAWLING, MANA_0, LIFEFORCE_0, PRAY_0, 0, SYMBOL_STRESS_0, "", "", "", "", START_51, LEARN_2, START_DO_10, LEARN_DO_2, START_DO_NO, LEARN_DO_NO, LEARN_DIFF_SKILLS, 0.0, 0, 0);

  discArray[SKILL_DUAL_WIELD] = new spellInfo(SKILL_WARRIOR, DISC_SOLDIERING, DISC_SOLDIERING, "dual wield", TASK_NORMAL, LAG_0, POSITION_CRAWLING, MANA_0, LIFEFORCE_0, PRAY_0, 0, SYMBOL_STRESS_0, "", "", "", "", START_1, LEARN_5, START_DO_1, LEARN_DO_1, START_DO_NO, LEARN_DO_NO, LEARN_DIFF_UNUSUAL, 0.0, 0, 0);

  discArray[SKILL_POWERMOVE] = new spellInfo(SKILL_WARRIOR, DISC_SOLDIERING, DISC_SOLDIERING, "power move", TASK_NORMAL, LAG_0, POSITION_STANDING, MANA_0, LIFEFORCE_0, PRAY_0, 0, SYMBOL_STRESS_0, "", "", "", "", START_1, LEARN_1, START_DO_1, LEARN_DO_1, START_DO_NO, LEARN_DO_NO, LEARN_DIFF_SKILLS, 0.0, 0, 0);

// disc_blacksmithing

  discArray[SKILL_BLACKSMITHING_ADVANCED] = new spellInfo(SKILL_WARRIOR, DISC_BLACKSMITHING, DISC_BLACKSMITHING, "advanced blacksmithing", TASK_NORMAL, LAG_0, POSITION_SITTING, MANA_0, LIFEFORCE_0, PRAY_0, 0, SYMBOL_STRESS_0, "", "", "", "", START_1, LEARN_1, START_DO_1, LEARN_DO_1, START_DO_NO, LEARN_DO_NO, LEARN_DIFF_SKILLS, 0.0, 0, 0);


// no new spells will just let someone  do armor


// RANGER CLASS

// disc_ranger


  discArray[SKILL_BEAST_SOOTHER] = new spellInfo(SPELL_RANGER, DISC_RANGER, DISC_ANIMAL, "beast soother", TASK_NORMAL, LAG_3, POSITION_CRAWLING, MANA_0, LIFEFORCE_0, PRAY_0, TAR_SELF_NONO | TAR_CHAR_ROOM, SYMBOL_STRESS_0, "", "", "", "", START_6, LEARN_4, START_DO_40, LEARN_DO_5, START_DO_NO, LEARN_DO_NO, LEARN_DIFF_SPELLS, 0.04, COMP_VERBAL | COMP_MATERIAL, 0);

  discArray[SKILL_BEFRIEND_BEAST] = new spellInfo(SPELL_RANGER, DISC_RANGER, DISC_ANIMAL, "befriend beast", TASK_DIFFICULT, LAG_3, POSITION_CRAWLING, MANA_0, LIFEFORCE_0, PRAY_0, TAR_SELF_NONO | TAR_CHAR_ROOM, SYMBOL_STRESS_0, "", "", "", "", START_25, LEARN_3, START_DO_40, LEARN_DO_5, START_DO_NO, LEARN_DO_NO, LEARN_DIFF_SPELLS, 0.02, COMP_VERBAL | COMP_MATERIAL, 0);

  discArray[SKILL_BEAST_SUMMON] = new spellInfo(SPELL_RANGER, DISC_RANGER, DISC_ANIMAL, "beast summon", TASK_DANGEROUS, LAG_3, POSITION_SITTING, MANA_0, LIFEFORCE_0, PRAY_0, TAR_NAME, SYMBOL_STRESS_0, "", "", "", "", START_56, LEARN_5, START_DO_40, LEARN_DO_5, START_DO_NO, LEARN_DO_NO, LEARN_DIFF_SPELLS, 0.0, COMP_VERBAL | COMP_MATERIAL, 0);

  discArray[SKILL_BARKSKIN] = new spellInfo(SPELL_RANGER, DISC_RANGER, DISC_PLANTS, "barkskin", TASK_EASY, LAG_3, POSITION_CRAWLING, MANA_0, LIFEFORCE_0, PRAY_0, TAR_CHAR_ROOM| TAR_FIGHT_SELF, SYMBOL_STRESS_0, "Your skin loses its barklike qualities.", "$n's skin loses its barklike qualities.", "Your skin seems slightly less like bark.", "$n's skin seems slightly less like wood.", START_1, LEARN_10, START_DO_20, LEARN_DO_5, START_DO_NO, LEARN_DO_NO, LEARN_DIFF_SPELLS, 0.04, COMP_VERBAL | COMP_GESTURAL | COMP_MATERIAL | COMP_MATERIAL_END, TOG_HAS_BARKSKIN);


// disc_fight_ranger

// disc_nature

  discArray[SPELL_TREE_WALK] = new spellInfo(SPELL_RANGER, DISC_NATURE, DISC_NATURE, "tree walk", TASK_NORMAL, LAG_3, POSITION_CRAWLING, MANA_0, LIFEFORCE_0, PRAY_0, TAR_NAME, SYMBOL_STRESS_0, "", "", "", "", START_66, LEARN_35, START_DO_40, LEARN_DO_2, START_DO_NO, LEARN_DO_NO, LEARN_DIFF_SPELLS, 0.04, COMP_GESTURAL | COMP_VERBAL | COMP_MATERIAL, 0);

// disc_animal

  discArray[SKILL_BEAST_CHARM] = new spellInfo(SPELL_RANGER, DISC_ANIMAL, DISC_ANIMAL, "beast charm", TASK_DIFFICULT, LAG_3, POSITION_CRAWLING, MANA_0, LIFEFORCE_0, PRAY_0, TAR_SELF_NONO | TAR_CHAR_ROOM, SYMBOL_STRESS_0, "", "", "", "", START_1, LEARN_2, START_DO_30, LEARN_DO_5, START_DO_NO, LEARN_DO_NO, LEARN_DIFF_SPELLS, 0.02, COMP_VERBAL | COMP_MATERIAL, 0);

#if 1
  discArray[SPELL_FERAL_WRATH] = new spellInfo(SPELL_RANGER, DISC_ANIMAL, DISC_ANIMAL, "feral wrath", TASK_NORMAL, LAG_3, POSITION_SITTING, MANA_0, LIFEFORCE_0, PRAY_0, TAR_CHAR_ROOM | TAR_SELF_ONLY, SYMBOL_STRESS_0, "You lose your feral wrath.", "The feral look in $n's eyes fades completely.", "Your feral wrath begins to fade.", "The feral look in $n's eyes begins to fade.", START_41, LEARN_4, START_DO_40, LEARN_DO_5, START_DO_NO, LEARN_DO_NO, LEARN_DIFF_SPELLS, 0.04, COMP_GESTURAL | COMP_VERBAL | COMP_MATERIAL | COMP_MATERIAL_END, 0);

  discArray[SPELL_SKY_SPIRIT] = new spellInfo(SPELL_RANGER, DISC_ANIMAL, DISC_ANIMAL, "sky spirit", TASK_NORMAL, LAG_3, POSITION_SITTING, MANA_10, LIFEFORCE_0, PRAY_0, TAR_CHAR_ROOM | TAR_SELF_NONO | TAR_FIGHT_VICT | TAR_VIOLENT, SYMBOL_STRESS_0, "", "", "", "", START_41, LEARN_4, START_DO_40, LEARN_DO_5, START_DO_NO, LEARN_DO_NO, LEARN_DIFF_SPELLS, 0.04, COMP_GESTURAL | COMP_VERBAL | COMP_MATERIAL, 0);
#endif

//disc_plants

#if 1
#endif

  discArray[SKILL_APPLY_HERBS] = new spellInfo(SKILL_RANGER, DISC_PLANTS, DISC_PLANTS, "apply herbs", TASK_NORMAL, LAG_0, POSITION_CRAWLING, MANA_0, LIFEFORCE_0, PRAY_0, 0, SYMBOL_STRESS_0, "", "", "", "", START_1, LEARN_2, START_DO_1, LEARN_DO_1, START_DO_NO, LEARN_DO_NO, LEARN_DIFF_SKILLS, 0.0, 0, 0);


// disc_deikhan

  discArray[SKILL_KICK_DEIKHAN] = new spellInfo(SKILL_DEIKHAN, DISC_DEIKHAN, DISC_DEIKHAN_FIGHT, "kick", TASK_EASY, LAG_3, POSITION_STANDING, MANA_0, LIFEFORCE_0, PRAY_0, 0, SYMBOL_STRESS_0, "", "", "", "", START_1, LEARN_5, START_DO_1, LEARN_DO_1, START_DO_NO, LEARN_DO_NO, LEARN_DIFF_SKILLS, 0.0, 0, 0);

  discArray[SPELL_HEAL_LIGHT_DEIKHAN] = new spellInfo(SPELL_DEIKHAN, DISC_DEIKHAN, DISC_DEIKHAN_CURES,"heal light", TASK_EASY, LAG_2, POSITION_SITTING, MANA_0, LIFEFORCE_0, PRAY_100, TAR_CHAR_ROOM | TAR_FIGHT_SELF, SYMBOL_STRESS_12, "", "", "", "", START_1, LEARN_10, START_DO_40, LEARN_DO_5, START_DO_NO, LEARN_DO_NO, LEARN_DIFF_PRAYERS, 0.02, COMP_GESTURAL | COMP_VERBAL | SPELL_TASKED | SPELL_TASKED_EVERY, 0);

  discArray[SKILL_CHIVALRY] = new spellInfo(SKILL_DEIKHAN, DISC_DEIKHAN, DISC_MOUNTED, "chivalry", TASK_NORMAL, LAG_0, POSITION_DEAD, MANA_0, LIFEFORCE_0, PRAY_0, 0, SYMBOL_STRESS_0, "", "", "", "", START_11, LEARN_2, START_DO_10, LEARN_DO_1, START_DO_NO, LEARN_DO_NO, LEARN_DIFF_UNUSUAL, 0.0, 0, 0);

  discArray[SPELL_HARM_LIGHT_DEIKHAN] = new spellInfo(SPELL_DEIKHAN, DISC_DEIKHAN, DISC_DEIKHAN_WRATH, "harm light", TASK_NORMAL, LAG_2, POSITION_SITTING, MANA_0, LIFEFORCE_0, PRAY_100, TAR_CHAR_ROOM | TAR_SELF_NONO | TAR_FIGHT_VICT | TAR_VIOLENT, SYMBOL_STRESS_5, "", "", "", "", START_1, LEARN_10, START_DO_20, LEARN_DO_2, START_DO_NO, LEARN_DO_NO, LEARN_DIFF_PRAYERS, 0.01, COMP_GESTURAL | COMP_VERBAL, 0);

  discArray[SPELL_ARMOR_DEIKHAN] = new spellInfo(SPELL_DEIKHAN, DISC_DEIKHAN, DISC_DEIKHAN_AEGIS, "armor", TASK_TRIVIAL, LAG_1, POSITION_SITTING, MANA_0, LIFEFORCE_0, PRAY_025, TAR_CHAR_ROOM | TAR_FIGHT_SELF, SYMBOL_STRESS_15, "The protection around your body fades.", "The protection around $n fades.", "You feel slightly less protected.", "", START_1, LEARN_10, START_DO_30, LEARN_DO_5, START_DO_NO, LEARN_DO_NO, LEARN_DIFF_PRAYERS, 0.2, COMP_GESTURAL | COMP_VERBAL, 0);

  discArray[SPELL_BLESS_DEIKHAN] = new spellInfo(SPELL_DEIKHAN, DISC_DEIKHAN, DISC_DEIKHAN_AEGIS, "bless", TASK_TRIVIAL, LAG_1, POSITION_SITTING, MANA_0, LIFEFORCE_0, PRAY_050, TAR_OBJ_INV | TAR_OBJ_EQUIP | TAR_CHAR_ROOM |TAR_FIGHT_SELF, SYMBOL_STRESS_10, "You feel less holy.", "$n doesn't seem as holy.", "You feel like your deities are getting bored assisting you.", "", START_1, LEARN_33, START_DO_30, LEARN_DO_4, START_DO_NO, LEARN_DO_NO, LEARN_DIFF_PRAYERS, 0.05, COMP_GESTURAL | COMP_VERBAL, 0);

  discArray[SKILL_BASH_DEIKHAN] = new spellInfo(SKILL_DEIKHAN, DISC_DEIKHAN, DISC_DEIKHAN_FIGHT, "bash-deikhan", TASK_NORMAL, LAG_3, POSITION_CRAWLING, MANA_0, LIFEFORCE_0, PRAY_0, 0, SYMBOL_STRESS_0, "", "", "", "", START_1, LEARN_5, START_DO_1, LEARN_DO_1, START_DO_NO, LEARN_DO_NO, LEARN_DIFF_SKILLS, 0.0, 0, 0);

  discArray[SPELL_EXPEL_DEIKHAN] = new spellInfo(SPELL_DEIKHAN, DISC_DEIKHAN, DISC_DEIKHAN_AEGIS, "expel", TASK_EASY, LAG_4, POSITION_SITTING, MANA_0, LIFEFORCE_0, PRAY_500, TAR_CHAR_ROOM, SYMBOL_STRESS_100, "", "", "", "", START_51, LEARN_5, START_DO_50, LEARN_DO_5, START_DO_NO, LEARN_DO_NO, LEARN_DIFF_PRAYERS, 0.04, COMP_GESTURAL | COMP_VERBAL, 0);

  discArray[SPELL_CLOT_DEIKHAN] = new spellInfo(SPELL_DEIKHAN, DISC_DEIKHAN, DISC_DEIKHAN_AEGIS, "clot", TASK_EASY, LAG_2, POSITION_SITTING, MANA_0, LIFEFORCE_0, PRAY_100, TAR_CHAR_ROOM, SYMBOL_STRESS_15, "", "", "", "", START_16, LEARN_10, START_DO_20, LEARN_DO_3, START_DO_NO, LEARN_DO_NO, LEARN_DIFF_PRAYERS, 0.04, COMP_GESTURAL | COMP_VERBAL, 0);

  discArray[SPELL_RAIN_BRIMSTONE_DEIKHAN] = new spellInfo(SPELL_DEIKHAN, DISC_DEIKHAN, DISC_DEIKHAN_WRATH, "rain brimstone", TASK_NORMAL, LAG_2, POSITION_SITTING, MANA_0, LIFEFORCE_0, PRAY_150, TAR_CHAR_ROOM | TAR_VIOLENT | TAR_FIGHT_VICT | TAR_SELF_NONO, SYMBOL_STRESS_10, "", "", "", "", START_16, LEARN_10, START_DO_10, LEARN_DO_2, START_DO_NO, LEARN_DO_NO, LEARN_DIFF_PRAYERS, 0.01, COMP_GESTURAL | COMP_VERBAL, 0);

  discArray[SPELL_STERILIZE_DEIKHAN] = new spellInfo(SPELL_DEIKHAN, DISC_DEIKHAN, DISC_DEIKHAN_CURES, "sterilize", TASK_EASY, LAG_3, POSITION_SITTING, MANA_0, LIFEFORCE_0, PRAY_150, TAR_CHAR_ROOM, SYMBOL_STRESS_15, "", "", "", "", START_21, LEARN_5, START_DO_30, LEARN_DO_5, START_DO_NO, LEARN_DO_NO, LEARN_DIFF_PRAYERS, 0.04, COMP_GESTURAL | COMP_VERBAL, 0);

  discArray[SPELL_REMOVE_CURSE_DEIKHAN] = new spellInfo(SPELL_DEIKHAN, DISC_DEIKHAN, DISC_DEIKHAN_AEGIS, "remove curse", TASK_NORMAL, LAG_3, POSITION_SITTING, MANA_0, LIFEFORCE_0, PRAY_100, TAR_CHAR_ROOM | TAR_OBJ_INV | TAR_OBJ_EQUIP | TAR_OBJ_ROOM, SYMBOL_STRESS_25, "", "", "", "", START_27, LEARN_5, START_DO_30, LEARN_DO_5, START_DO_NO, LEARN_DO_NO, LEARN_DIFF_PRAYERS, 0.04, COMP_GESTURAL | COMP_VERBAL, 0);

  discArray[SPELL_CURSE_DEIKHAN] = new spellInfo(SPELL_DEIKHAN, DISC_DEIKHAN, DISC_DEIKHAN_WRATH, "curse", TASK_NORMAL, LAG_3, POSITION_FIGHTING, MANA_0, LIFEFORCE_0, PRAY_050, TAR_CHAR_ROOM | TAR_OBJ_ROOM | TAR_OBJ_INV | TAR_OBJ_EQUIP | TAR_VIOLENT | TAR_FIGHT_VICT, SYMBOL_STRESS_15, "You don't feel damned anymore.", "", "", "", START_51, LEARN_10, START_DO_10, LEARN_DO_2, START_DO_NO, LEARN_DO_NO, LEARN_DIFF_PRAYERS, 0.05, COMP_GESTURAL | COMP_VERBAL , 0);

  discArray[SKILL_RESCUE_DEIKHAN] = new spellInfo(SKILL_DEIKHAN, DISC_DEIKHAN, DISC_DEIKHAN_FIGHT, "rescue", TASK_NORMAL, LAG_2, POSITION_CRAWLING, MANA_0, LIFEFORCE_0, PRAY_0, 0, SYMBOL_STRESS_0, "", "", "", "", START_11, LEARN_2, START_DO_10, LEARN_DO_1, START_DO_NO, LEARN_DO_NO, LEARN_DIFF_SKILLS, 0.15, 0, 0);

  discArray[SKILL_SMITE] = new spellInfo(SKILL_DEIKHAN, DISC_DEIKHAN, DISC_DEIKHAN_FIGHT, "smite", TASK_NORMAL, LAG_3, POSITION_STANDING, MANA_0, LIFEFORCE_0, PRAY_0, 0, SYMBOL_STRESS_0, "Your deity empowers you to smite once more.", "", "", "", START_51, LEARN_2, START_DO_50, LEARN_DO_3, START_DO_NO, LEARN_DO_NO, LEARN_DIFF_SKILLS, 0.0, 0, 0);

  discArray[SPELL_INFECT_DEIKHAN] = new spellInfo(SPELL_DEIKHAN, DISC_DEIKHAN, DISC_DEIKHAN_WRATH, "infect", TASK_DIFFICULT, LAG_3, POSITION_SITTING, MANA_0, LIFEFORCE_0, PRAY_150, TAR_CHAR_ROOM | TAR_FIGHT_VICT | TAR_VIOLENT | TAR_SELF_NONO, SYMBOL_STRESS_40, "", "", "", "", START_63, LEARN_8, START_DO_20, LEARN_DO_5, START_DO_NO, LEARN_DO_NO, LEARN_DIFF_PRAYERS, 0.04, COMP_GESTURAL | COMP_VERBAL, 0);

  discArray[SPELL_CURE_DISEASE_DEIKHAN] = new spellInfo(SPELL_DEIKHAN, DISC_DEIKHAN, DISC_DEIKHAN_CURES, "cure disease", TASK_NORMAL, LAG_4, POSITION_SITTING, MANA_0, LIFEFORCE_0, PRAY_450, TAR_CHAR_ROOM | TAR_FIGHT_SELF, SYMBOL_STRESS_50, "", "", "", "", START_30, LEARN_5, START_DO_40, LEARN_DO_5, START_DO_NO, LEARN_DO_NO, LEARN_DIFF_PRAYERS, 0.02, COMP_GESTURAL | COMP_VERBAL, 0);

  discArray[SPELL_CREATE_FOOD_DEIKHAN] = new spellInfo(SPELL_DEIKHAN, DISC_DEIKHAN, DISC_DEIKHAN_AEGIS, "create food", TASK_TRIVIAL, LAG_1, POSITION_CRAWLING, MANA_0, LIFEFORCE_0, PRAY_025, TAR_IGNORE, SYMBOL_STRESS_5, "", "", "", "", START_11, LEARN_10, START_DO_40, LEARN_DO_2, START_DO_NO, LEARN_DO_NO, LEARN_DIFF_PRAYERS, 0.0, COMP_GESTURAL | COMP_VERBAL, 0);

  discArray[SPELL_CREATE_WATER_DEIKHAN] = new spellInfo(SPELL_DEIKHAN, DISC_DEIKHAN, DISC_DEIKHAN_AEGIS, "create water", TASK_TRIVIAL, LAG_2, POSITION_CRAWLING, MANA_0, LIFEFORCE_0, PRAY_025, TAR_OBJ_INV, SYMBOL_STRESS_5, "", "", "", "", START_11, LEARN_10, START_DO_40, LEARN_DO_5, START_DO_NO, LEARN_DO_NO, LEARN_DIFF_PRAYERS, 0.0, COMP_GESTURAL | COMP_VERBAL | SPELL_TASKED | SPELL_TASKED_EVERY, 0);

  discArray[SPELL_HEAL_SERIOUS_DEIKHAN] = new spellInfo(SPELL_DEIKHAN, DISC_DEIKHAN, DISC_DEIKHAN_CURES, "heal serious", TASK_EASY, LAG_2, POSITION_SITTING, MANA_0, LIFEFORCE_0, PRAY_150, TAR_CHAR_ROOM | TAR_FIGHT_SELF, SYMBOL_STRESS_18, "", "", "", "", START_41, LEARN_5, START_DO_30, LEARN_DO_2, START_DO_NO, LEARN_DO_NO, LEARN_DIFF_PRAYERS, 0.03, COMP_GESTURAL | COMP_VERBAL |  SPELL_TASKED | SPELL_TASKED_EVERY, 0);

  discArray[SPELL_CURE_POISON_DEIKHAN] = new spellInfo(SPELL_DEIKHAN, DISC_DEIKHAN, DISC_DEIKHAN_CURES, "cure poison", TASK_EASY, LAG_3, POSITION_SITTING, MANA_0, LIFEFORCE_0, PRAY_100, TAR_CHAR_ROOM | TAR_FIGHT_SELF, SYMBOL_STRESS_35, "", "", "", "", START_11, LEARN_5, START_DO_40, LEARN_DO_5, START_DO_NO, LEARN_DO_NO, LEARN_DIFF_PRAYERS, 0.02, COMP_GESTURAL | COMP_VERBAL, 0);

  discArray[SKILL_CHARGE] = new spellInfo(SKILL_DEIKHAN, DISC_DEIKHAN, DISC_MOUNTED, "charge", TASK_NORMAL, LAG_4, POSITION_SITTING, MANA_0, LIFEFORCE_0, PRAY_0, 0, SYMBOL_STRESS_0, "", "", "", "", START_26, LEARN_2, START_DO_1, LEARN_DO_1, START_DO_NO, LEARN_DO_NO, LEARN_DIFF_SKILLS, 0.0, 0, 0);

  discArray[SPELL_HARM_SERIOUS_DEIKHAN] = new spellInfo(SPELL_DEIKHAN, DISC_DEIKHAN, DISC_DEIKHAN_CURES, "harm serious", TASK_NORMAL, LAG_3, POSITION_SITTING, MANA_0, LIFEFORCE_0, PRAY_150, TAR_CHAR_ROOM | TAR_SELF_NONO | TAR_FIGHT_VICT | TAR_VIOLENT , SYMBOL_STRESS_20, "", "", "", "", START_39, LEARN_8, START_DO_20, LEARN_DO_2, START_DO_NO, LEARN_DO_NO, LEARN_DIFF_PRAYERS, 0.02, COMP_GESTURAL | COMP_VERBAL, 0);

  discArray[SPELL_POISON_DEIKHAN] = new spellInfo(SPELL_DEIKHAN, DISC_DEIKHAN, DISC_DEIKHAN_WRATH, "poison", TASK_NORMAL, LAG_3, POSITION_SITTING, MANA_0, LIFEFORCE_0, PRAY_100, TAR_CHAR_ROOM | TAR_OBJ_INV | TAR_FIGHT_VICT | TAR_VIOLENT | TAR_FIGHT_VICT | TAR_SELF_NONO, SYMBOL_STRESS_45, "You feel much healthier.", "The color returns to $n's cheeks.", "", "", START_59, LEARN_10, START_DO_30, LEARN_DO_2, START_DO_NO, LEARN_DO_NO, LEARN_DIFF_PRAYERS, 0.02, COMP_GESTURAL | COMP_VERBAL, 0);

  discArray[SKILL_DISARM_DEIKHAN] = new spellInfo(SKILL_DEIKHAN, DISC_DEIKHAN, DISC_DEIKHAN_FIGHT, "disarm", TASK_DANGEROUS, LAG_2, POSITION_CRAWLING, MANA_0, LIFEFORCE_0, PRAY_0, 0, SYMBOL_STRESS_0, "", "", "", "", START_35, LEARN_3, START_DO_10, LEARN_DO_1, START_DO_NO, LEARN_DO_NO, LEARN_DIFF_SKILLS, 0.0, 0, 0);


  discArray[SPELL_HEAL_CRITICAL_DEIKHAN] = new spellInfo(SPELL_DEIKHAN, DISC_DEIKHAN, DISC_DEIKHAN_CURES, "heal critical", TASK_NORMAL, LAG_2, POSITION_SITTING, MANA_0, LIFEFORCE_0, PRAY_200, TAR_CHAR_ROOM | TAR_FIGHT_SELF, SYMBOL_STRESS_25, "", "", "", "", START_76, LEARN_5, START_DO_20, LEARN_DO_2, START_DO_NO, LEARN_DO_NO, LEARN_DIFF_PRAYERS, 0.04, COMP_GESTURAL | COMP_VERBAL | SPELL_TASKED | SPELL_TASKED_EVERY, 0);

  discArray[SPELL_HARM_CRITICAL_DEIKHAN] = new spellInfo(SPELL_DEIKHAN, DISC_DEIKHAN, DISC_DEIKHAN_WRATH, "harm critical", TASK_NORMAL, LAG_3, POSITION_SITTING, MANA_0, LIFEFORCE_0, PRAY_200, TAR_CHAR_ROOM | TAR_SELF_NONO | TAR_FIGHT_VICT | TAR_VIOLENT , SYMBOL_STRESS_35, "", "", "", "", START_56, LEARN_10, START_DO_20, LEARN_DO_2, START_DO_NO, LEARN_DO_NO, LEARN_DIFF_PRAYERS, 0.03, COMP_GESTURAL | COMP_VERBAL, 0);

  discArray[SKILL_REPAIR_DEIKHAN] = new spellInfo(SKILL_DEIKHAN, DISC_DEIKHAN, DISC_BLACKSMITHING, "deikhan repair", TASK_NORMAL, LAG_0, POSITION_SITTING, MANA_0, LIFEFORCE_0, PRAY_0, 0, SYMBOL_STRESS_0, "", "", "", "", START_50, LEARN_3, START_DO_1, LEARN_DO_1, START_DO_NO, LEARN_DO_NO, LEARN_DIFF_SKILLS, 0.0, 0, 0);


// disc_fight_deikhan

  discArray[SKILL_SWITCH_DEIKHAN] = new spellInfo(SKILL_DEIKHAN, DISC_DEIKHAN_FIGHT, DISC_DEIKHAN_FIGHT, "switch opponents", TASK_NORMAL, LAG_2, POSITION_CRAWLING, MANA_0, LIFEFORCE_0, PRAY_0, 0, SYMBOL_STRESS_0, "", "", "", "", START_21, LEARN_3, START_DO_10, LEARN_DO_1, START_DO_NO, LEARN_DO_NO, LEARN_DIFF_UNUSUAL, 0.0, 0, 0);

  discArray[SKILL_RETREAT_DEIKHAN] = new spellInfo(SKILL_DEIKHAN, DISC_DEIKHAN_FIGHT, DISC_DEIKHAN_FIGHT, "retreat", TASK_NORMAL, LAG_0, POSITION_CRAWLING, MANA_0, LIFEFORCE_0, PRAY_0, 0, SYMBOL_STRESS_0, "", "", "", "", START_65, LEARN_3, START_DO_1, LEARN_DO_1, START_DO_NO, LEARN_DO_NO, LEARN_DIFF_UNUSUAL, 0.0, 0, 0);

  discArray[SKILL_SHOVE_DEIKHAN] = new spellInfo(SKILL_DEIKHAN, DISC_DEIKHAN_FIGHT, DISC_DEIKHAN_FIGHT, "shove", TASK_NORMAL, LAG_3, POSITION_CRAWLING, MANA_0, LIFEFORCE_0, PRAY_0, 0, SYMBOL_STRESS_0, "", "", "", "", START_33, LEARN_3, START_DO_20, LEARN_DO_1, START_DO_NO, LEARN_DO_NO, LEARN_DIFF_SKILLS, 0.0, 0, 0);

// disc_mount
  discArray[SKILL_CALM_MOUNT] = new spellInfo(SKILL_DEIKHAN, DISC_MOUNTED, DISC_MOUNTED, "calm mount", TASK_NORMAL, LAG_0, POSITION_CRAWLING, MANA_0, LIFEFORCE_0, PRAY_0, 0, SYMBOL_STRESS_0, "", "", "", "", START_1, LEARN_2, START_DO_1, LEARN_DO_1, START_DO_NO, LEARN_DO_NO, LEARN_DIFF_SKILLS, 0.0, 0, 0);

  discArray[SKILL_TRAIN_MOUNT] = new spellInfo(SKILL_DEIKHAN, DISC_MOUNTED, DISC_MOUNTED, "train mount", TASK_NORMAL, LAG_0, POSITION_CRAWLING, MANA_0, LIFEFORCE_0, PRAY_0, 0, SYMBOL_STRESS_0, "", "", "", "", START_26, LEARN_2, START_DO_1, LEARN_DO_1, START_DO_NO, LEARN_DO_NO, LEARN_DIFF_SKILLS, 0.0, 0, 0);

  discArray[SKILL_ADVANCED_RIDING] = new spellInfo(SKILL_DEIKHAN, DISC_MOUNTED, DISC_MOUNTED, "advanced riding", TASK_NORMAL, LAG_0, POSITION_CRAWLING, MANA_0, LIFEFORCE_0, PRAY_0, 0, SYMBOL_STRESS_0, "", "", "", "", START_46,LEARN_2, START_DO_1, LEARN_DO_1, START_DO_NO, LEARN_DO_NO, LEARN_DIFF_SKILLS, 0.0, 0, 0);

  discArray[SKILL_RIDE_DOMESTIC] = new spellInfo(SKILL_DEIKHAN, DISC_MOUNTED, DISC_MOUNTED, "ride domestic", TASK_TRIVIAL, LAG_0, POSITION_CRAWLING, MANA_0, LIFEFORCE_0, PRAY_0, 0, SYMBOL_STRESS_0, "", "", "", "", START_5, LEARN_2, START_DO_1, LEARN_DO_1, START_DO_NO, LEARN_DO_NO, LEARN_DIFF_SKILLS, 0.0, 0, 0);

  discArray[SKILL_RIDE_NONDOMESTIC] = new spellInfo(SKILL_DEIKHAN, DISC_MOUNTED, DISC_MOUNTED, "ride non-domestic", TASK_EASY, LAG_0, POSITION_CRAWLING, MANA_0, LIFEFORCE_0, PRAY_0, 0, SYMBOL_STRESS_0, "", "", "", "", START_36, LEARN_2, START_DO_1, LEARN_DO_1, START_DO_NO, LEARN_DO_NO, LEARN_DIFF_SKILLS, 0.0, 0, 0);

  discArray[SKILL_RIDE_WINGED] = new spellInfo(SKILL_DEIKHAN, DISC_MOUNTED, DISC_MOUNTED, "ride winged", TASK_DIFFICULT, LAG_0, POSITION_CRAWLING, MANA_0, LIFEFORCE_0, PRAY_0, 0, SYMBOL_STRESS_0, "", "", "", "", START_66, LEARN_3, START_DO_1, LEARN_DO_1, START_DO_NO, LEARN_DO_NO, LEARN_DIFF_SKILLS, 0.0, 0, 0);

  discArray[SKILL_RIDE_EXOTIC] = new spellInfo(SKILL_DEIKHAN, DISC_MOUNTED, DISC_MOUNTED, "ride exotic", TASK_NORMAL, LAG_0, POSITION_CRAWLING, MANA_0, LIFEFORCE_0, PRAY_0, 0, SYMBOL_STRESS_0, "", "", "", "", START_85, LEARN_7, START_DO_1, LEARN_DO_1, START_DO_NO, LEARN_DO_NO, LEARN_DIFF_SKILLS, 0.0, 0, 0);


// disc_aegis_deikhan

 discArray[SPELL_HEROES_FEAST_DEIKHAN] = new spellInfo(SPELL_DEIKHAN, DISC_DEIKHAN_AEGIS, DISC_DEIKHAN_AEGIS, "heroes feast", TASK_NORMAL, LAG_3, POSITION_CRAWLING, MANA_0, LIFEFORCE_0, PRAY_075, TAR_AREA | TAR_IGNORE, SYMBOL_STRESS_15, "", "", "", "", START_26, LEARN_2, START_DO_20, LEARN_DO_3, START_DO_NO, LEARN_DO_NO, LEARN_DIFF_PRAYERS, 0.1, COMP_GESTURAL | COMP_VERBAL, 0);

  discArray[SPELL_REFRESH_DEIKHAN] = new spellInfo(SPELL_DEIKHAN, DISC_DEIKHAN_AEGIS, DISC_DEIKHAN_AEGIS, "refresh", TASK_NORMAL, LAG_2, POSITION_SITTING, MANA_0, LIFEFORCE_0, PRAY_100, TAR_CHAR_ROOM | TAR_FIGHT_SELF, SYMBOL_STRESS_20, "", "", "", "", START_1, LEARN_4, START_DO_20, LEARN_DO_2, START_DO_NO, LEARN_DO_NO, LEARN_DIFF_PRAYERS, 0.02, COMP_GESTURAL | COMP_VERBAL, 0);

  discArray[SPELL_SYNOSTODWEOMER] = new spellInfo(SPELL_DEIKHAN, DISC_DEIKHAN_AEGIS, DISC_DEIKHAN_AEGIS, "synostodweomer", TASK_NORMAL,LAG_4, POSITION_CRAWLING, MANA_0, LIFEFORCE_0, PRAY_500, TAR_CHAR_ROOM, SYMBOL_STRESS_75, "Synostodweomer leaves you and your hitpoints return to normal", "","","", START_76, LEARN_4, START_DO_30, LEARN_DO_5, START_DO_NO, LEARN_DO_NO, LEARN_DIFF_PRAYERS, 0.0, COMP_GESTURAL | COMP_VERBAL, 0);

// disc_deikhan_cures

  discArray[SPELL_SALVE_DEIKHAN] = new spellInfo(SPELL_DEIKHAN, DISC_DEIKHAN_CURES, DISC_DEIKHAN_CURES, "salve", TASK_TRIVIAL, LAG_3, POSITION_SITTING, MANA_0, LIFEFORCE_0, PRAY_200, TAR_CHAR_ROOM, SYMBOL_STRESS_10, "", "", "", "", START_1, LEARN_5, START_DO_20, LEARN_DO_2, START_DO_NO, LEARN_DO_NO, LEARN_DIFF_PRAYERS, 0.02, COMP_GESTURAL | COMP_VERBAL, 0);

  discArray[SKILL_LAY_HANDS] = new spellInfo(SKILL_DEIKHAN, DISC_DEIKHAN_CURES, DISC_DEIKHAN_CURES, "lay hands", TASK_NORMAL, LAG_1, POSITION_STANDING, MANA_0, LIFEFORCE_0, PRAY_0, 0, SYMBOL_STRESS_0, "", "", "", "", START_1, LEARN_1, START_DO_35, LEARN_DO_2, START_DO_NO, LEARN_DO_NO, LEARN_DIFF_SKILLS, 0.4, 0, 0);

// disc_wrath_deikhan

  discArray[SPELL_HARM_DEIKHAN] = new spellInfo(SPELL_DEIKHAN, DISC_DEIKHAN_WRATH, DISC_DEIKHAN_WRATH, "harm", TASK_NORMAL, LAG_3, POSITION_SITTING, MANA_0, LIFEFORCE_0, PRAY_250, TAR_CHAR_ROOM | TAR_FIGHT_VICT | TAR_SELF_NONO | TAR_VIOLENT , SYMBOL_STRESS_50, "", "", "", "", START_26, LEARN_4, START_DO_20, LEARN_DO_2, START_DO_NO, LEARN_DO_NO, LEARN_DIFF_PRAYERS, 0.06, COMP_GESTURAL | COMP_VERBAL, 0);

  discArray[SPELL_NUMB_DEIKHAN] = new spellInfo(SPELL_DEIKHAN, DISC_DEIKHAN_WRATH, DISC_DEIKHAN_WRATH, "numb", TASK_DANGEROUS, LAG_2, POSITION_SITTING, MANA_0, LIFEFORCE_0, PRAY_100, TAR_CHAR_ROOM | TAR_FIGHT_VICT | TAR_VIOLENT | TAR_SELF_NONO, SYMBOL_STRESS_30, "", "", "", "", START_1, LEARN_4, START_DO_40, LEARN_DO_5, START_DO_NO, LEARN_DO_NO, LEARN_DIFF_PRAYERS, 0.05, COMP_GESTURAL | COMP_VERBAL, 0);

  discArray[SPELL_EARTHQUAKE_DEIKHAN] = new spellInfo(SPELL_DEIKHAN, DISC_DEIKHAN_WRATH, DISC_DEIKHAN_WRATH, "earthquake", TASK_DIFFICULT, LAG_4, POSITION_SITTING, MANA_0, LIFEFORCE_0, PRAY_500, TAR_AREA | TAR_IGNORE | TAR_VIOLENT, SYMBOL_STRESS_90, "", "", "", "", START_51, LEARN_4, START_DO_20, LEARN_DO_2, START_DO_NO, LEARN_DO_NO, LEARN_DIFF_PRAYERS, 0.04, COMP_GESTURAL | COMP_VERBAL, 0);

  discArray[SPELL_CALL_LIGHTNING_DEIKHAN] = new spellInfo(SPELL_DEIKHAN, DISC_DEIKHAN_WRATH, DISC_DEIKHAN_WRATH, "call lightning", TASK_DIFFICULT, LAG_2, POSITION_SITTING, MANA_0, LIFEFORCE_0, PRAY_350, TAR_CHAR_ROOM | TAR_VIOLENT | TAR_FIGHT_VICT | TAR_SELF_NONO , SYMBOL_STRESS_50, "", "", "", "", START_76, LEARN_4, START_DO_20, LEARN_DO_2, START_DO_NO, LEARN_DO_NO, LEARN_DIFF_PRAYERS, 0.06, COMP_GESTURAL | COMP_VERBAL, 0);

// CLASS MONK

// disc_monk

  discArray[SKILL_YOGINSA] = new spellInfo(SKILL_MONK, DISC_MONK, DISC_MEDITATION_MONK, "yoginsa", TASK_NORMAL, LAG_0, POSITION_DEAD, MANA_0, LIFEFORCE_0, PRAY_0, 0, SYMBOL_STRESS_0, "", "", "", "", START_1, LEARN_1, START_DO_1, LEARN_DO_1, START_DO_NO, LEARN_DO_NO, LEARN_DIFF_SKILLS, 0.0, 0, 0);

 discArray[SKILL_CINTAI] = new spellInfo(SKILL_MONK, DISC_MONK, DISC_FOCUSED_ATTACKS, "cintai", TASK_NORMAL, LAG_0, POSITION_DEAD, MANA_0, LIFEFORCE_0, PRAY_0, 0, SYMBOL_STRESS_0, "", "", "", "", START_5, LEARN_2, START_DO_1, LEARN_DO_1, START_DO_NO, LEARN_DO_NO, LEARN_DIFF_SKILLS, 0.0, 0, 0);

  discArray[SKILL_OOMLAT] = new spellInfo(SKILL_MONK, DISC_MONK, DISC_MINDBODY, "Oomlat Philosophy", TASK_NORMAL, LAG_0, POSITION_DEAD, MANA_0, LIFEFORCE_0, PRAY_0, 0, SYMBOL_STRESS_0, "", "", "", "", START_1, LEARN_1, START_DO_1, LEARN_DO_1, START_DO_NO, LEARN_DO_NO, LEARN_DIFF_SKILLS, 0.0, 0, 0);

  discArray[SKILL_KICK_MONK] = new spellInfo(SKILL_MONK, DISC_MONK, DISC_LEVERAGE, "kick", TASK_EASY, LAG_3, POSITION_STANDING, MANA_0, LIFEFORCE_0, PRAY_0, 0, SYMBOL_STRESS_0, "", "", "", "", START_5, LEARN_5, START_DO_1, LEARN_DO_1, START_DO_NO, LEARN_DO_NO, LEARN_DIFF_SKILLS, 0.0, 0, 0);

  discArray[SKILL_ADVANCED_KICKING] = new spellInfo(SKILL_MONK, DISC_MONK, DISC_LEVERAGE, "advanced kicking", TASK_NORMAL, LAG_0, POSITION_DEAD, MANA_0, LIFEFORCE_0, PRAY_0, 0, SYMBOL_STRESS_0, "", "", "", "", START_25, LEARN_2, START_DO_10, LEARN_DO_1, START_DO_NO, LEARN_DO_NO, LEARN_DIFF_SKILLS, 0.0, 0, TOG_HAS_ADVANCED_KICKING);

  discArray[SKILL_GROUNDFIGHTING] = new spellInfo(SKILL_MONK, DISC_MONK, DISC_LEVERAGE, "groundfighting", TASK_NORMAL, LAG_0, POSITION_DEAD, MANA_0, LIFEFORCE_0, PRAY_0, 0, SYMBOL_STRESS_0, "", "", "", "", START_10, LEARN_3, START_DO_1, LEARN_DO_1, START_DO_NO, LEARN_DO_NO, LEARN_DIFF_SKILLS, 0.0, 0, 0);

  discArray[SKILL_SPRINGLEAP] = new spellInfo(SKILL_MONK, DISC_MONK, DISC_LEVERAGE, "springleap", TASK_NORMAL, LAG_2, POSITION_RESTING, MANA_0, LIFEFORCE_0, PRAY_0, 0, SYMBOL_STRESS_0, "", "", "", "", START_20, LEARN_3, START_DO_1, LEARN_DO_3, START_DO_NO, LEARN_DO_NO, LEARN_DIFF_SKILLS, 0.0, 0, 0);

  discArray[SKILL_RETREAT_MONK] = new spellInfo(SKILL_MONK, DISC_MONK, DISC_MONK, "retreat", TASK_NORMAL, LAG_0, POSITION_CRAWLING, MANA_0, LIFEFORCE_0, PRAY_0, 0, SYMBOL_STRESS_0, "", "", "", "", START_30, LEARN_2, START_DO_1, LEARN_DO_1, START_DO_NO, LEARN_DO_NO, LEARN_DIFF_UNUSUAL, 0.0, 0, 0);

  discArray[SKILL_SNOFALTE] = new spellInfo(SKILL_MONK, DISC_MONK, DISC_MEDITATION_MONK, "snofalte", TASK_NORMAL, LAG_0, POSITION_DEAD, MANA_0, LIFEFORCE_0, PRAY_0, 0, SYMBOL_STRESS_0, "", "", "", "", START_40, LEARN_2, START_DO_1, LEARN_DO_1, START_DO_NO, LEARN_DO_NO, LEARN_DIFF_SKILLS, 0.0, 0, 0);

  discArray[SKILL_COUNTER_MOVE] = new spellInfo(SKILL_MONK, DISC_MONK, DISC_LEVERAGE, "counter move", TASK_NORMAL, LAG_1, POSITION_RESTING, MANA_0, LIFEFORCE_0, PRAY_0, 0, SYMBOL_STRESS_0, "", "", "", "", START_50, LEARN_2, START_DO_1, LEARN_DO_1, START_DO_NO, LEARN_DO_NO, LEARN_DIFF_SKILLS, 0.0, 0, 0);

  discArray[SKILL_SWITCH_MONK] = new spellInfo(SKILL_MONK, DISC_MONK, DISC_LEVERAGE, "switch opponents", TASK_NORMAL, LAG_2, POSITION_CRAWLING, MANA_0, LIFEFORCE_0, PRAY_0, 0, SYMBOL_STRESS_0, "", "", "", "", START_50, LEARN_100, START_DO_1, LEARN_DO_2, START_DO_NO, LEARN_DO_NO, LEARN_DIFF_UNUSUAL, 0.0, 0, 0);
 
  discArray[SKILL_JIRIN] = new spellInfo(SKILL_MONK, DISC_MONK, DISC_MINDBODY, "jirin", TASK_NORMAL, LAG_0, POSITION_DEAD, MANA_0, LIFEFORCE_0, PRAY_0, 0, SYMBOL_STRESS_0, "", "", "", "", START_1, LEARN_1, START_DO_1, LEARN_DO_1, START_DO_NO, LEARN_DO_NO, LEARN_DIFF_SKILLS, 0.0, 0, 0);

  discArray[SKILL_KUBO] = new spellInfo(SKILL_MONK, DISC_MONK, DISC_MEDITATION_MONK, "kubo", TASK_NORMAL, LAG_0, POSITION_DEAD, MANA_0, LIFEFORCE_0, PRAY_0, 0, SYMBOL_STRESS_0, "", "", "", "", START_1, LEARN_1, START_DO_1, LEARN_DO_1, START_DO_NO, LEARN_DO_NO, LEARN_DIFF_SKILLS, 0.0, 0, 0);

  discArray[SKILL_DUFALI] = new spellInfo(SKILL_MONK, DISC_MONK, DISC_MEDITATION_MONK, "dufali", TASK_NORMAL, LAG_0, POSITION_DEAD, MANA_0, LIFEFORCE_0, PRAY_0, 0, SYMBOL_STRESS_0, "", "", "", "", START_70, LEARN_4, START_DO_NO, LEARN_DO_NO, START_DO_NO, LEARN_DO_NO, LEARN_DIFF_SKILLS, 0.0, 0, 0);

  discArray[SKILL_CHOP] = new spellInfo(SKILL_MONK, DISC_MONK, DISC_FOCUSED_ATTACKS, "chop", TASK_NORMAL, LAG_4, POSITION_STANDING, MANA_0, LIFEFORCE_0, PRAY_0, 0, SYMBOL_STRESS_0, "", "", "", "", START_51, LEARN_2, START_DO_1, LEARN_DO_2, START_DO_NO, LEARN_DO_NO, LEARN_DIFF_SKILLS, 0.0, 0, 0);

  discArray[SKILL_DISARM_MONK] = new spellInfo(SKILL_MONK, DISC_MONK, DISC_LEVERAGE, "disarm", TASK_DANGEROUS, LAG_2, POSITION_CRAWLING, MANA_0, LIFEFORCE_0, PRAY_0, 0, SYMBOL_STRESS_0, "", "", "", "", START_1, LEARN_2, START_DO_1, LEARN_DO_4, START_DO_NO, LEARN_DO_NO, LEARN_DIFF_SKILLS, 0.0, 0, 0);

  discArray[SKILL_CHI] = new spellInfo(SKILL_MONK, DISC_MONK, DISC_MEDITATION_MONK, "chi", TASK_NORMAL, LAG_4, POSITION_CRAWLING, MANA_0, LIFEFORCE_0, PRAY_0, 0, SYMBOL_STRESS_0, "You cease to radiate your chi.", "", "You feel your chi running low.", "", START_1, LEARN_1, START_DO_1, LEARN_DO_1, START_DO_NO, LEARN_DO_NO, LEARN_DIFF_SKILLS, 0.0, 0, 0);

  discArray[SKILL_CATFALL] = new spellInfo(SKILL_MONK, DISC_MONK, DISC_MINDBODY, "catfall", TASK_NORMAL, LAG_0,POSITION_DEAD, MANA_0, LIFEFORCE_0, PRAY_0, 0, SYMBOL_STRESS_0, "", "", "", "", START_1, LEARN_2, START_DO_1, LEARN_DO_5, START_DO_NO, LEARN_DO_NO, LEARN_DIFF_SKILLS, 0.0, 0, TOG_HAS_CATFALL);

  discArray[SKILL_REPAIR_MONK] = new spellInfo(SKILL_MONK, DISC_MONK, DISC_BLACKSMITHING, "monk repair", TASK_NORMAL, LAG_0, POSITION_SITTING, MANA_0, LIFEFORCE_0, PRAY_0, 0, SYMBOL_STRESS_0, "", "", "", "", START_50, LEARN_3, START_DO_1, LEARN_DO_1, START_DO_NO, LEARN_DO_NO, LEARN_DIFF_SKILLS, 0.0, 0, 0);

  discArray[SKILL_CATLEAP] = new spellInfo(SKILL_MONK, DISC_MONK, DISC_MINDBODY, "catleap", TASK_NORMAL, LAG_0, POSITION_STANDING, MANA_0, LIFEFORCE_0, PRAY_0, 0, SYMBOL_STRESS_0, "", "", "", "", START_1, LEARN_2, START_DO_1, LEARN_DO_5, START_DO_NO, LEARN_DO_NO, LEARN_DIFF_SKILLS, 0.0, 0, 0);

// disc_chidoki

  discArray[SKILL_WOHLIN] = new spellInfo(SKILL_MONK, DISC_MEDITATION_MONK, DISC_MEDITATION_MONK, "wohlin meditation", TASK_NORMAL, LAG_0, POSITION_DEAD, MANA_0, LIFEFORCE_0, PRAY_0, 0, SYMBOL_STRESS_0, "", "", "", "", START_1, LEARN_1, START_DO_1, LEARN_DO_1, START_DO_NO, LEARN_DO_NO, LEARN_DIFF_SKILLS, 0.0, 0, 0);

  discArray[SKILL_VOPLAT] = new spellInfo(SKILL_MONK, DISC_MEDITATION_MONK, DISC_MEDITATION_MONK, "voplat", TASK_NORMAL, LAG_0, POSITION_DEAD, MANA_0, LIFEFORCE_0, PRAY_0, 0, SYMBOL_STRESS_0, "", "", "", "", START_1, LEARN_1, START_DO_NO, LEARN_DO_NO, START_DO_NO, LEARN_DO_NO, LEARN_DIFF_SKILLS, 0.0, 0, 0);

  discArray[SKILL_BLINDFIGHTING] = new spellInfo(SKILL_MONK, DISC_MEDITATION_MONK, DISC_MEDITATION_MONK, "blindfighting", TASK_NORMAL, LAG_0, POSITION_DEAD, MANA_0, LIFEFORCE_0, PRAY_0, 0, SYMBOL_STRESS_0, "", "", "", "", START_1, LEARN_1, START_DO_NO, LEARN_DO_NO, START_DO_NO, LEARN_DO_NO, LEARN_DIFF_SKILLS, 0.0, 0, 0);

// disc_karoki


  discArray[SKILL_HURL] = new spellInfo(SKILL_MONK, DISC_LEVERAGE, DISC_LEVERAGE, "hurl", TASK_NORMAL, LAG_4, POSITION_CRAWLING, MANA_0, LIFEFORCE_0, PRAY_0, 0, SYMBOL_STRESS_0, "", "", "", "", START_1, LEARN_2, START_DO_1, LEARN_DO_2, START_DO_NO, LEARN_DO_NO, LEARN_DIFF_SKILLS, 0.0, 0, 0);

  discArray[SKILL_SHOULDER_THROW] = new spellInfo(SKILL_MONK, DISC_LEVERAGE, DISC_LEVERAGE, "shoulder throw", TASK_NORMAL, LAG_4, POSITION_CRAWLING, MANA_0, LIFEFORCE_0, PRAY_0, 0, SYMBOL_STRESS_0, "", "", "", "", START_35, LEARN_2, START_DO_1, LEARN_DO_2, START_DO_NO, LEARN_DO_NO, LEARN_DIFF_SKILLS, 0.0, 0, 0);

  discArray[SKILL_CHAIN_ATTACK] = new spellInfo(SKILL_MONK, DISC_LEVERAGE, DISC_LEVERAGE, "chain attack", TASK_TRIVIAL, LAG_0, POSITION_STANDING, MANA_0, LIFEFORCE_0, PRAY_0, 0, SYMBOL_STRESS_0, "", "", "", "", START_1, LEARN_1, START_DO_1, LEARN_DO_1, START_DO_NO, LEARN_DO_NO, LEARN_DIFF_SKILLS, 0.0, 0, 0);

// disc_taedoki

  discArray[SKILL_FEIGN_DEATH] = new spellInfo(SKILL_MONK, DISC_MINDBODY, DISC_MINDBODY, "feign death", TASK_NORMAL, LAG_2, POSITION_CRAWLING, MANA_0, LIFEFORCE_0, PRAY_0, 0, SYMBOL_STRESS_0, "", "", "", "", START_1, LEARN_1, START_DO_10, LEARN_DO_5, START_DO_NO, LEARN_DO_NO, LEARN_DIFF_SKILLS, 0.0, 0, 0);

  discArray[SKILL_BLUR] = new spellInfo(SKILL_MONK, DISC_MINDBODY, DISC_MINDBODY, "blur", TASK_TRIVIAL, LAG_0, POSITION_STANDING, MANA_0, LIFEFORCE_0, PRAY_0, 0, SYMBOL_STRESS_0, "", "", "", "", START_1, LEARN_1, START_DO_1, LEARN_DO_1, START_DO_NO, LEARN_DO_NO, LEARN_DIFF_SKILLS, 0.0, 0, 0);


// disc_fattacks

  discArray[SKILL_QUIV_PALM] = new spellInfo(SKILL_MONK, DISC_FOCUSED_ATTACKS, DISC_MEDITATION_MONK, "quivering palm", TASK_NORMAL, LAG_4, POSITION_STANDING, MANA_0, LIFEFORCE_0, PRAY_0, 0, SYMBOL_STRESS_0, "", "", "", "", START_51, LEARN_8, START_DO_1, LEARN_DO_2, START_DO_NO, LEARN_DO_NO, LEARN_DIFF_SKILLS, 0.0, 0, 0);

  discArray[SKILL_CRIT_HIT] = new spellInfo(SKILL_MONK, DISC_FOCUSED_ATTACKS, DISC_FOCUSED_ATTACKS, "critical hitting", TASK_TRIVIAL, LAG_0, POSITION_STANDING, MANA_0, LIFEFORCE_0, PRAY_0, 0, SYMBOL_STRESS_0, "", "", "", "", START_1, LEARN_1, START_DO_1, LEARN_DO_1, START_DO_NO, LEARN_DO_NO, LEARN_DIFF_SKILLS, 0.0, 0, 0);

// disc iron body
  discArray[SKILL_IRON_FIST] = new spellInfo(SKILL_MONK, DISC_IRON_BODY, DISC_IRON_BODY, "iron fist", TASK_TRIVIAL, LAG_0, POSITION_STANDING, MANA_0, LIFEFORCE_0, PRAY_0, 0, SYMBOL_STRESS_0, "", "", "", "", START_1, LEARN_1, START_DO_1, LEARN_DO_1, START_DO_NO, LEARN_DO_NO, LEARN_DIFF_SKILLS, 0.0, 0, 0);

  discArray[SKILL_IRON_FLESH] = new spellInfo(SKILL_MONK, DISC_IRON_BODY, DISC_IRON_BODY, "iron flesh", TASK_NORMAL, LAG_0, POSITION_STANDING, MANA_0, LIFEFORCE_0, PRAY_0, 0, SYMBOL_STRESS_0, "", "", "", "", START_20, LEARN_2, START_DO_NO, LEARN_DO_NO, START_DO_NO, LEARN_DO_NO, LEARN_DIFF_SKILLS, 0.0, 0, 0);

  discArray[SKILL_IRON_SKIN] = new spellInfo(SKILL_MONK, DISC_IRON_BODY, DISC_IRON_BODY, "iron skin", TASK_NORMAL, LAG_0, POSITION_STANDING, MANA_0, LIFEFORCE_0, PRAY_0, 0, SYMBOL_STRESS_0, "", "", "", "", START_30, LEARN_2, START_DO_NO, LEARN_DO_NO, START_DO_NO, LEARN_DO_NO, LEARN_DIFF_SKILLS, 0.0, 0, 0);

  discArray[SKILL_IRON_BONES] = new spellInfo(SKILL_MONK, DISC_IRON_BODY, DISC_IRON_BODY, "iron bones", TASK_NORMAL, LAG_0, POSITION_STANDING, MANA_0, LIFEFORCE_0, PRAY_0, 0, SYMBOL_STRESS_0, "", "", "", "", START_40, LEARN_2, START_DO_NO, LEARN_DO_NO, START_DO_NO, LEARN_DO_NO, LEARN_DIFF_SKILLS, 0.0, 0, 0);

  discArray[SKILL_IRON_MUSCLES] = new spellInfo(SKILL_MONK, DISC_IRON_BODY, DISC_IRON_BODY, "iron muscles", TASK_NORMAL, LAG_0, POSITION_STANDING, MANA_0, LIFEFORCE_0, PRAY_0, 0, SYMBOL_STRESS_0, "", "", "", "", START_50, LEARN_2, START_DO_NO, LEARN_DO_NO, START_DO_NO, LEARN_DO_NO, LEARN_DIFF_SKILLS, 0.0, 0, 0);

  discArray[SKILL_IRON_LEGS] = new spellInfo(SKILL_MONK, DISC_IRON_BODY, DISC_IRON_BODY, "iron legs", TASK_NORMAL, LAG_0, POSITION_STANDING, MANA_0, LIFEFORCE_0, PRAY_0, 0, SYMBOL_STRESS_0, "", "", "", "", START_60, LEARN_3, START_DO_NO, LEARN_DO_NO, START_DO_NO, LEARN_DO_NO, LEARN_DIFF_SKILLS, 0.0, 0, 0);

  discArray[SKILL_IRON_WILL] = new spellInfo(SKILL_MONK, DISC_IRON_BODY, DISC_IRON_BODY, "iron will", TASK_NORMAL, LAG_0, POSITION_STANDING, MANA_0, LIFEFORCE_0, PRAY_0, 0, SYMBOL_STRESS_0, "", "", "", "", START_70, LEARN_4, START_DO_NO, LEARN_DO_NO, START_DO_NO, LEARN_DO_NO, LEARN_DIFF_SKILLS, 0.0, 0, 0);

// CLASS THIEF
// disc_thief

  discArray[SKILL_TRACK] = new spellInfo(SKILL_THIEF, DISC_THIEF, DISC_STEALTH, "track", TASK_NORMAL, LAG_0, POSITION_CRAWLING, MANA_0, LIFEFORCE_0, PRAY_0, 0, SYMBOL_STRESS_0, "", "", "", "", START_27, LEARN_3, START_DO_1, LEARN_DO_1, START_DO_NO, LEARN_DO_NO, LEARN_DIFF_SKILLS, 0.0, 0, 0);

  discArray[SKILL_SWINDLE] = new spellInfo(SKILL_THIEF, DISC_THIEF, DISC_LOOTING, "swindle", TASK_NORMAL, LAG_0, POSITION_CRAWLING, MANA_0, LIFEFORCE_0, PRAY_0, 0, SYMBOL_STRESS_0, "", "", "", "", START_1, LEARN_1, START_DO_1, LEARN_DO_1, START_DO_NO, LEARN_DO_NO, LEARN_DIFF_UNUSUAL, 0.0, 0, 0);

  discArray[SKILL_SNEAK] = new spellInfo(SKILL_THIEF, DISC_THIEF, DISC_STEALTH, "sneak", TASK_EASY, LAG_1, POSITION_CRAWLING, MANA_0, LIFEFORCE_0, PRAY_0, 0, SYMBOL_STRESS_0, "", "", "", "", START_1, LEARN_5, START_DO_1, LEARN_DO_1, START_DO_NO, LEARN_DO_NO, LEARN_DIFF_SKILLS, 0.0, 0, 0);

  discArray[SKILL_STABBING] = new spellInfo(SKILL_THIEF, DISC_THIEF, DISC_MURDER, "stab", TASK_EASY, LAG_3, POSITION_CRAWLING, MANA_0, LIFEFORCE_0, PRAY_0, 0, SYMBOL_STRESS_0, "", "", "", "", START_33, LEARN_3, START_DO_1, LEARN_DO_2, START_DO_NO, LEARN_DO_NO, LEARN_DIFF_SKILLS,0.0, 0, 0);

  discArray[SKILL_RETREAT_THIEF] = new spellInfo(SKILL_THIEF, DISC_THIEF, DISC_THIEF_FIGHT, "retreat", TASK_NORMAL, LAG_0, POSITION_CRAWLING, MANA_0, LIFEFORCE_0, PRAY_0, 0, SYMBOL_STRESS_0, "", "", "", "", START_1, LEARN_1,START_DO_1, LEARN_DO_2, START_DO_NO, LEARN_DO_NO, LEARN_DIFF_UNUSUAL, 0.0, 0, 0);

  discArray[SKILL_KICK_THIEF] = new spellInfo(SKILL_THIEF, DISC_THIEF, DISC_THIEF_FIGHT, "kick", TASK_EASY, LAG_3, POSITION_STANDING, MANA_0, LIFEFORCE_0, PRAY_0, 0, SYMBOL_STRESS_0, "", "", "", "", START_1, LEARN_5, START_DO_1, LEARN_DO_1, START_DO_NO, LEARN_DO_NO, LEARN_DIFF_SKILLS, 0.0, 0, 0);

  discArray[SKILL_PICK_LOCK] = new spellInfo(SKILL_THIEF, DISC_THIEF, DISC_TRAPS, "picklock", TASK_NORMAL, LAG_0, POSITION_CRAWLING, MANA_0, LIFEFORCE_0, PRAY_0, 0, SYMBOL_STRESS_0, "", "", "", "", START_31, LEARN_5, START_DO_1, LEARN_DO_5, START_DO_NO, LEARN_DO_NO, LEARN_DIFF_UNUSUAL,0.0, 0, 0);

  discArray[SKILL_BACKSTAB] = new spellInfo(SKILL_THIEF, DISC_THIEF, DISC_MURDER, "backstab", TASK_NORMAL, LAG_5, POSITION_CRAWLING, MANA_0, LIFEFORCE_0, PRAY_0, 0, SYMBOL_STRESS_0, "", "", "", "", START_1, LEARN_1, START_DO_1, LEARN_DO_2, START_DO_NO, LEARN_DO_NO, LEARN_DIFF_SKILLS, 0.0, 0, 0);

  discArray[SKILL_THROATSLIT] = new spellInfo(SKILL_THIEF, DISC_MURDER, DISC_MURDER, "slit", TASK_NORMAL, LAG_6, POSITION_CRAWLING, MANA_0, LIFEFORCE_0, PRAY_0, 0, SYMBOL_STRESS_0, "", "", "", "", START_1, LEARN_1, START_DO_1, LEARN_DO_2, START_DO_NO, LEARN_DO_NO, LEARN_DIFF_SKILLS, 0.0, 0, 0);

  discArray[SKILL_SEARCH] = new spellInfo(SKILL_THIEF, DISC_THIEF, DISC_LOOTING, "search", TASK_NORMAL, LAG_0, POSITION_CRAWLING, MANA_0, LIFEFORCE_0, PRAY_0, 0, SYMBOL_STRESS_0, "", "", "", "", START_1, LEARN_1, START_DO_1, LEARN_DO_1, START_DO_NO, LEARN_DO_NO, LEARN_DIFF_UNUSUAL, 0.0, 0, 0);

  discArray[SKILL_SPY] = new spellInfo(SKILL_THIEF, DISC_THIEF, DISC_STEALTH, "spy", TASK_NORMAL, LAG_0, POSITION_CRAWLING, MANA_0, LIFEFORCE_0, PRAY_0, 0, SYMBOL_STRESS_0, "You cease your espionage activities.", "", "", "", START_61, LEARN_5, START_DO_1, LEARN_DO_2, START_DO_NO, LEARN_DO_NO, LEARN_DIFF_SKILLS, 0.0, 0,
 0);

  discArray[SKILL_SWITCH_THIEF] = new spellInfo(SKILL_THIEF, DISC_THIEF, DISC_THIEF_FIGHT, "switch opponents", TASK_NORMAL, LAG_2, POSITION_CRAWLING, MANA_0, LIFEFORCE_0, PRAY_0, 0, SYMBOL_STRESS_0, "", "", "", "", START_21, LEARN_3, START_DO_10, LEARN_DO_3, START_DO_NO, LEARN_DO_NO, LEARN_DIFF_UNUSUAL, 0.0, 0, 0);

  discArray[SKILL_STEAL] = new spellInfo(SKILL_THIEF, DISC_THIEF, DISC_LOOTING, "steal", TASK_NORMAL, LAG_2, POSITION_CRAWLING, MANA_0, LIFEFORCE_0, PRAY_0, 0, SYMBOL_STRESS_0, "", "", "", "", START_1, LEARN_1, START_DO_10, LEARN_DO_2, START_DO_NO, LEARN_DO_NO, LEARN_DIFF_SKILLS, 0.0, 0, 0);

  // detect trap can go long way before learn by do takes over
  // it's mostly book learning
  discArray[SKILL_DETECT_TRAP] = new spellInfo(SKILL_THIEF, DISC_THIEF, DISC_TRAPS, "detect trap", TASK_NORMAL, LAG_0, POSITION_CRAWLING, MANA_0, LIFEFORCE_0, PRAY_0, 0, SYMBOL_STRESS_0, "", "", "", "", START_1, LEARN_3, START_DO_60, LEARN_DO_2, START_DO_NO, LEARN_DO_NO, LEARN_DIFF_SKILLS, 0.0, 0, 0);

  discArray[SKILL_SUBTERFUGE] = new spellInfo(SKILL_THIEF, DISC_THIEF, DISC_STEALTH, "subterfuge", TASK_NORMAL, LAG_2, POSITION_CRAWLING, MANA_0, LIFEFORCE_0, PRAY_0, 0, SYMBOL_STRESS_0, "", "", "", "", START_41, LEARN_2, START_DO_20, LEARN_DO_5, START_DO_NO, LEARN_DO_NO, LEARN_DIFF_SKILLS, 0.0, 0, 0);


  discArray[SKILL_DISARM_TRAP] = new spellInfo(SKILL_THIEF, DISC_THIEF, DISC_TRAPS, "disarm trap", TASK_NORMAL, LAG_0, POSITION_CRAWLING, MANA_0, LIFEFORCE_0, PRAY_0, 0, SYMBOL_STRESS_0, "", "", "", "", START_33, LEARN_3, START_DO_25, LEARN_DO_5, START_DO_NO, LEARN_DO_NO, LEARN_DIFF_SKILLS, 0.0, 0, 0);

  discArray[SKILL_CUDGEL] = new spellInfo(SKILL_THIEF, DISC_THIEF, DISC_MURDER, "cudgel", TASK_NORMAL, LAG_4, POSITION_CRAWLING, MANA_0, LIFEFORCE_0, PRAY_0, 0, SYMBOL_STRESS_0, "", "", "", "", START_66, LEARN_3, START_DO_1, LEARN_DO_2, START_DO_NO, LEARN_DO_NO, LEARN_DIFF_SKILLS, 0.0, 0, 0);

  discArray[SKILL_HIDE] = new spellInfo(SKILL_THIEF, DISC_THIEF, DISC_STEALTH, "hide", TASK_NORMAL, LAG_1, POSITION_CRAWLING, MANA_0, LIFEFORCE_0, PRAY_0, 0, SYMBOL_STRESS_0, "", "", "", "", START_50, LEARN_3, START_DO_1, LEARN_DO_2, START_DO_NO, LEARN_DO_NO, LEARN_DIFF_SKILLS, 0.0, 0, 0);

  discArray[SKILL_DISARM_THIEF] = new spellInfo(SKILL_THIEF, DISC_THIEF, DISC_THIEF_FIGHT, "disarm", TASK_DANGEROUS, LAG_2, POSITION_CRAWLING, MANA_0, LIFEFORCE_0, PRAY_0, 0, SYMBOL_STRESS_0, "", "", "", "", START_33, LEARN_3, START_DO_1, LEARN_DO_4, START_DO_NO, LEARN_DO_NO, LEARN_DIFF_SKILLS, 0.0, 0, 0);

// disc_thief_fight

  discArray[SKILL_DODGE_THIEF] = new spellInfo(SKILL_THIEF, DISC_THIEF_FIGHT, DISC_THIEF_FIGHT, "dodge", TASK_NORMAL, LAG_0, POSITION_DEAD, MANA_0, LIFEFORCE_0, PRAY_0, 0, SYMBOL_STRESS_0, "", "", "", "", START_1, LEARN_1,START_DO_1, LEARN_DO_1, START_DO_NO, LEARN_DO_NO, LEARN_DIFF_UNUSUAL, 0.0, 0, 0);

  discArray[SKILL_DUAL_WIELD_THIEF] = new spellInfo(SKILL_THIEF, DISC_THIEF_FIGHT, DISC_THIEF_FIGHT, "dual wield", TASK_NORMAL, LAG_0, POSITION_CRAWLING, MANA_0, LIFEFORCE_0, PRAY_0, 0, SYMBOL_STRESS_0, "", "", "", "", START_50, LEARN_2, START_DO_1, LEARN_DO_1, START_DO_NO, LEARN_DO_NO, LEARN_DIFF_UNUSUAL, 0.0, 0, 0);

  discArray[SKILL_GARROTTE] = new spellInfo(SKILL_THIEF, DISC_THIEF_FIGHT, DISC_THIEF_FIGHT, "garrotte", TASK_NORMAL, LAG_3, POSITION_CRAWLING, MANA_0, LIFEFORCE_0, PRAY_0, 0, SYMBOL_STRESS_0, "", "", "", "", START_1, LEARN_3, START_DO_1, LEARN_DO_2, START_DO_NO, LEARN_DO_NO, LEARN_DIFF_SKILLS, 0.0, 0, 0);

  discArray[SKILL_REPAIR_THIEF] = new spellInfo(SKILL_THIEF, DISC_THIEF, DISC_BLACKSMITHING, "thief repair", TASK_NORMAL, LAG_0, POSITION_SITTING, MANA_0, LIFEFORCE_0, PRAY_0, 0, SYMBOL_STRESS_0, "", "", "", "", START_50, LEARN_3, START_DO_1, LEARN_DO_1, START_DO_NO, LEARN_DO_NO, LEARN_DIFF_SKILLS, 0.0, 0, 0);


// disc_murder


// disc_looting
  discArray[SKILL_COUNTER_STEAL] = new spellInfo(SKILL_THIEF, DISC_LOOTING, DISC_LOOTING, "counter steal", TASK_NORMAL, LAG_0, POSITION_DEAD, MANA_0, LIFEFORCE_0, PRAY_0, 0, SYMBOL_STRESS_0, "", "", "", "", START_1, LEARN_1,START_DO_1, LEARN_DO_1, START_DO_NO, LEARN_DO_NO, LEARN_DIFF_UNUSUAL, 0.0, 0, 0);

  discArray[SKILL_PLANT] = new spellInfo(SKILL_THIEF, DISC_LOOTING, DISC_LOOTING, "plant", TASK_NORMAL, LAG_0, POSITION_DEAD, MANA_0, LIFEFORCE_0, PRAY_0, 0, SYMBOL_STRESS_0, "", "", "", "", START_50, LEARN_3,START_DO_1, LEARN_DO_1, START_DO_NO, LEARN_DO_NO, LEARN_DIFF_UNUSUAL, 0.0, 0, 0);


// disc_poison

  discArray[SKILL_POISON_WEAPON] = new spellInfo(SKILL_THIEF, DISC_POISONS, DISC_POISONS, "poison weapons", TASK_NORMAL, LAG_4, POSITION_CRAWLING, MANA_0, LIFEFORCE_0, PRAY_0, 0, SYMBOL_STRESS_0, "", "", "", "", START_1, LEARN_1, START_DO_1, LEARN_DO_2, START_DO_NO, LEARN_DO_NO, LEARN_DIFF_UNUSUAL, 0.0, 0, 0);

// disc_stealth

  discArray[SKILL_CONCEALMENT] = new spellInfo(SKILL_THIEF, DISC_STEALTH, DISC_STEALTH, "concealment", TASK_NORMAL, LAG_0, POSITION_CRAWLING, MANA_0, LIFEFORCE_0, PRAY_0, 0, SYMBOL_STRESS_0, "Your path is no longer concealed.", "$n's path is no longer concealed.", "", "", START_1, LEARN_1, START_DO_10, LEARN_DO_1, START_DO_NO, LEARN_DO_NO, LEARN_DIFF_SKILLS, 0.0, 0, 0);


  discArray[SKILL_DISGUISE] = new spellInfo(SKILL_THIEF, DISC_STEALTH, DISC_STEALTH, "disguise", TASK_NORMAL, LAG_0, POSITION_STANDING, MANA_0, LIFEFORCE_0, PRAY_0, 0, SYMBOL_STRESS_0, "You can not hold your disguise any longer.", "", "You do not know how much longer you can keep this shape.", "", START_1, LEARN_1, START_DO_10, LEARN_DO_2, START_DO_NO, LEARN_DO_NO, LEARN_DIFF_SKILLS, 0.0, 0, 0);


// disc_traps

  discArray[SKILL_SET_TRAP_ARROW] = new spellInfo(SKILL_THIEF, DISC_TRAPS, DISC_TRAPS, "set arrow trap",TASK_NORMAL, LAG_0, POSITION_CRAWLING, MANA_0, LIFEFORCE_0, PRAY_0, 0, SYMBOL_STRESS_0, "", "", "", "", START_1, LEARN_3, START_DO_40, LEARN_DO_2, START_DO_NO, LEARN_DO_NO, LEARN_DIFF_SKILLS, 0.0, 0, 0);

  discArray[SKILL_SET_TRAP_CONT] = new spellInfo(SKILL_THIEF, DISC_TRAPS, DISC_TRAPS, "set container trap",TASK_NORMAL, LAG_0, POSITION_CRAWLING, MANA_0, LIFEFORCE_0, PRAY_0, 0, SYMBOL_STRESS_0, "", "", "", "", START_1, LEARN_3, START_DO_40, LEARN_DO_2, START_DO_NO, LEARN_DO_NO, LEARN_DIFF_SKILLS, 0.0, 0, 0);

  discArray[SKILL_SET_TRAP_DOOR] = new spellInfo(SKILL_THIEF, DISC_TRAPS, DISC_TRAPS, "set door trap",TASK_NORMAL, LAG_0, POSITION_CRAWLING, MANA_0, LIFEFORCE_0, PRAY_0, 0, SYMBOL_STRESS_0, "", "", "", "", START_30, LEARN_3, START_DO_40, LEARN_DO_2, START_DO_NO, LEARN_DO_NO, LEARN_DIFF_SKILLS, 0.0, 0, 0);

  discArray[SKILL_SET_TRAP_MINE] = new spellInfo(SKILL_THIEF, DISC_TRAPS, DISC_TRAPS, "set mine trap",TASK_NORMAL, LAG_0, POSITION_CRAWLING, MANA_0, LIFEFORCE_0, PRAY_0, 0, SYMBOL_STRESS_0, "", "", "", "", START_60, LEARN_3, START_DO_40, LEARN_DO_2, START_DO_NO, LEARN_DO_NO, LEARN_DIFF_SKILLS, 0.0, 0, 0);

  discArray[SKILL_SET_TRAP_GREN] = new spellInfo(SKILL_THIEF, DISC_TRAPS, DISC_TRAPS, "set grenade trap",TASK_NORMAL, LAG_0, POSITION_CRAWLING, MANA_0, LIFEFORCE_0, PRAY_0, 0, SYMBOL_STRESS_0, "", "", "", "", START_80, LEARN_5, START_DO_40, LEARN_DO_2, START_DO_NO, LEARN_DO_NO, LEARN_DIFF_SKILLS, 0.0, 0, 0);

// disc_advanced_adventuring
  discArray[SKILL_HIKING] = new spellInfo(SKILL_GENERAL, DISC_ADVANCED_ADVENTURING, DISC_ADVANCED_ADVENTURING, "hiking", TASK_NORMAL, LAG_0, POSITION_CRAWLING, MANA_0, LIFEFORCE_0, PRAY_0, 0, SYMBOL_STRESS_0, "", "", "", "", START_1, LEARN_1, START_DO_NO, LEARN_DO_NO, START_DO_NO, LEARN_DO_NO, LEARN_DIFF_SKILLS, 0.0, 0, 0);

  discArray[SKILL_FORAGE] = new spellInfo(SKILL_GENERAL, DISC_ADVANCED_ADVENTURING, DISC_ADVANCED_ADVENTURING, "forage", TASK_NORMAL, LAG_0, POSITION_CRAWLING, MANA_0, LIFEFORCE_0, PRAY_0, 0, SYMBOL_STRESS_0, "You are able to forage again.", "", "", "", START_1, LEARN_5, START_DO_30, LEARN_DO_5, START_DO_NO, LEARN_DO_NO, LEARN_DIFF_SKILLS, 0.0, 0, 0);

  discArray[SKILL_SEEKWATER] = new spellInfo(SKILL_GENERAL, DISC_ADVANCED_ADVENTURING, DISC_ADVANCED_ADVENTURING, "seekwater", TASK_NORMAL, LAG_0, POSITION_CRAWLING, MANA_0, LIFEFORCE_0, PRAY_0, 0, SYMBOL_STRESS_0, "", "", "", "", START_6, LEARN_5, START_DO_1, LEARN_DO_5, START_DO_NO, LEARN_DO_NO, LEARN_DIFF_SKILLS, 0.0, 0, 0);

  discArray[SKILL_DIVINATION] = new spellInfo(SKILL_GENERAL, DISC_ADVANCED_ADVENTURING, DISC_ADVANCED_ADVENTURING, "divine", TASK_NORMAL, LAG_0, POSITION_CRAWLING, MANA_0, LIFEFORCE_0, PRAY_0, 0, SYMBOL_STRESS_0, "", "", "", "", START_51, LEARN_2, START_DO_10, LEARN_DO_1, START_DO_NO, LEARN_DO_NO, LEARN_DIFF_SKILLS, 0.0, 0, 0);

  discArray[SKILL_ENCAMP] = new spellInfo(SKILL_GENERAL, DISC_ADVANCED_ADVENTURING, DISC_ADVANCED_ADVENTURING, "encamp", TASK_NORMAL, LAG_0, POSITION_CRAWLING, MANA_0, LIFEFORCE_0, PRAY_0, 0, SYMBOL_STRESS_0, "", "", "", "", START_26, LEARN_2, START_DO_1, LEARN_DO_2, START_DO_NO, LEARN_DO_NO, LEARN_DIFF_SKILLS, 0.0, 0, 0);

  discArray[SKILL_SKIN] = new spellInfo(SKILL_GENERAL, DISC_ADVANCED_ADVENTURING, DISC_ADVANCED_ADVENTURING, "skinning", TASK_NORMAL, LAG_0, POSITION_CRAWLING, MANA_0, LIFEFORCE_0, PRAY_0, 0, SYMBOL_STRESS_0, "", "", "", "", START_26, LEARN_2, START_DO_10, LEARN_DO_1, START_DO_NO, LEARN_DO_NO, LEARN_DIFF_UNUSUAL, 0.0, 0, 0);


// disc_adventuring

  discArray[SKILL_BUTCHER] = new spellInfo(SKILL_GENERAL, DISC_ADVENTURING, DISC_ADVENTURING, "butcher", TASK_NORMAL, LAG_0, POSITION_CRAWLING, MANA_0, LIFEFORCE_0, PRAY_0, 0, SYMBOL_STRESS_0, "", "", "", "", START_1, LEARN_2, START_DO_10, LEARN_DO_1, START_DO_NO, LEARN_DO_NO, LEARN_DIFF_UNUSUAL, 0.0, 0, 0);

  discArray[SKILL_FISHING] = new spellInfo(SKILL_GENERAL, DISC_ADVENTURING, DISC_ADVENTURING, "fishing", TASK_NORMAL, LAG_0, POSITION_SITTING, MANA_0, LIFEFORCE_0, PRAY_0, 0, SYMBOL_STRESS_0, "", "", "", "", START_1, LEARN_1, START_DO_1, LEARN_DO_1, START_DO_NO, LEARN_DO_NO, LEARN_DIFF_UNUSUAL, 0.0, 0, 0);

  discArray[SKILL_ALCOHOLISM] = new spellInfo(SKILL_GENERAL, DISC_ADVENTURING, DISC_ADVENTURING, "alcoholism", TASK_TRIVIAL, LAG_0, POSITION_CRAWLING, MANA_0, LIFEFORCE_0, PRAY_0, 0, SYMBOL_STRESS_0, "", "", "", "", START_1, LEARN_4, START_DO_1, LEARN_DO_2, START_DO_NO, LEARN_DO_NO, LEARN_DIFF_SKILLS, 0.0, 0, 0);

  discArray[SKILL_RIDE] = new spellInfo(SKILL_GENERAL, DISC_ADVENTURING, DISC_ADVENTURING, "ride", TASK_TRIVIAL, LAG_0, POSITION_CRAWLING, MANA_0, LIFEFORCE_0, PRAY_0, 0, SYMBOL_STRESS_0, "", "", "", "", START_1, LEARN_4, START_DO_25, LEARN_DO_2, START_DO_NO, LEARN_DO_NO, LEARN_DIFF_SKILLS, 0.0, 0, 0);

  discArray[SKILL_SIGN] = new spellInfo(SKILL_GENERAL, DISC_ADVENTURING, DISC_ADVENTURING, "sign", TASK_NORMAL, LAG_0, POSITION_CRAWLING, MANA_0, LIFEFORCE_0, PRAY_0, 0, SYMBOL_STRESS_0, "", "", "", "", START_26, LEARN_4, START_DO_1, LEARN_DO_1, START_DO_NO, LEARN_DO_NO, LEARN_DIFF_UNUSUAL, 0.0, 0, 0);

  discArray[SKILL_SWIM] = new spellInfo(SKILL_GENERAL, DISC_ADVENTURING, DISC_ADVENTURING, "swim", TASK_NORMAL, LAG_0, POSITION_CRAWLING, MANA_0, LIFEFORCE_0, PRAY_0, 0, SYMBOL_STRESS_0, "", "", "", "", START_1, LEARN_1, START_DO_40, LEARN_DO_5, START_DO_NO, LEARN_DO_NO, LEARN_DIFF_SKILLS, 0.0, 0, 0);

  discArray[SKILL_CONS_UNDEAD] = new spellInfo(SKILL_GENERAL, DISC_ADVENTURING, DISC_ADVENTURING, "know undead", TASK_NORMAL, LAG_0, POSITION_DEAD, MANA_0, LIFEFORCE_0, PRAY_0, 0, SYMBOL_STRESS_0, "", "", "", "", START_61, LEARN_5, START_DO_1, LEARN_DO_2, START_DO_NO, LEARN_DO_NO, LEARN_DIFF_UNUSUAL, 0.0, 0, 0);

  discArray[SKILL_CONS_VEGGIE] = new spellInfo(SKILL_GENERAL, DISC_ADVENTURING, DISC_ADVENTURING, "know veggie", TASK_NORMAL, LAG_0, POSITION_DEAD, MANA_0, LIFEFORCE_0, PRAY_0, 0, SYMBOL_STRESS_0, "", "", "", "", START_21, LEARN_5, START_DO_1, LEARN_DO_2, START_DO_NO, LEARN_DO_NO, LEARN_DIFF_UNUSUAL, 0.0, 0, 0);

  discArray[SKILL_CONS_DEMON] = new spellInfo(SKILL_GENERAL, DISC_ADVENTURING, DISC_ADVENTURING, "know demon", TASK_NORMAL, LAG_0, POSITION_DEAD, MANA_0, LIFEFORCE_0, PRAY_0, 0, SYMBOL_STRESS_0, "", "", "", "", START_81, LEARN_5, START_DO_1, LEARN_DO_2, START_DO_NO, LEARN_DO_NO, LEARN_DIFF_UNUSUAL, 0.0, 0, 0);

  discArray[SKILL_CONS_ANIMAL] = new spellInfo(SKILL_GENERAL, DISC_ADVENTURING, DISC_ADVENTURING, "know animal", TASK_NORMAL, LAG_0, POSITION_DEAD, MANA_0, LIFEFORCE_0, PRAY_0, 0, SYMBOL_STRESS_0, "", "", "", "", START_1, LEARN_5, START_DO_1, LEARN_DO_2, START_DO_NO, LEARN_DO_NO, LEARN_DIFF_UNUSUAL, 0.0, 0, 0);

  discArray[SKILL_CONS_REPTILE] = new spellInfo(SKILL_GENERAL, DISC_ADVENTURING, DISC_ADVENTURING, "know reptile", TASK_NORMAL, LAG_0, POSITION_DEAD, MANA_0, LIFEFORCE_0, PRAY_0, 0, SYMBOL_STRESS_0, "", "", "", "", START_31, LEARN_5, START_DO_1, LEARN_DO_2, START_DO_NO, LEARN_DO_NO, LEARN_DIFF_UNUSUAL, 0.0, 0, 0);

  discArray[SKILL_CONS_PEOPLE] = new spellInfo(SKILL_GENERAL, DISC_ADVENTURING, DISC_ADVENTURING, "know people", TASK_NORMAL, LAG_0, POSITION_DEAD, MANA_0, LIFEFORCE_0, PRAY_0, 0, SYMBOL_STRESS_0, "", "", "", "", START_1, LEARN_2, START_DO_1, LEARN_DO_1, START_DO_NO, LEARN_DO_NO, LEARN_DIFF_UNUSUAL, 0.0, 0, 0);

  discArray[SKILL_CONS_GIANT] = new spellInfo(SKILL_GENERAL, DISC_ADVENTURING, DISC_ADVENTURING, "know giantkin", TASK_NORMAL, LAG_0, POSITION_DEAD, MANA_0, LIFEFORCE_0, PRAY_0, 0, SYMBOL_STRESS_0, "", "", "", "", START_41, LEARN_5, START_DO_1, LEARN_DO_2, START_DO_NO, LEARN_DO_NO, LEARN_DIFF_UNUSUAL, 0.0, 0, 0);

  discArray[SKILL_CONS_OTHER] = new spellInfo(SKILL_GENERAL, DISC_ADVENTURING, DISC_ADVENTURING, "know other", TASK_NORMAL, LAG_0, POSITION_DEAD, MANA_0, LIFEFORCE_0, PRAY_0, 0, SYMBOL_STRESS_0, "", "", "", "", START_51, LEARN_5, START_DO_1, LEARN_DO_5, START_DO_NO, LEARN_DO_NO, LEARN_DIFF_UNUSUAL, 0.0, 0, 0);

  discArray[SKILL_READ_MAGIC] = new spellInfo(SKILL_GENERAL, DISC_ADVENTURING, DISC_ADVENTURING, "read magic", TASK_NORMAL, LAG_0, POSITION_DEAD, MANA_0, LIFEFORCE_0, PRAY_0, 0, SYMBOL_STRESS_0, "", "", "", "", START_1, LEARN_1, START_DO_1, LEARN_DO_5, START_DO_NO, LEARN_DO_NO, LEARN_DIFF_SKILLS, 0.0, 0, 0);

  discArray[SKILL_BANDAGE] = new spellInfo(SKILL_GENERAL, DISC_ADVENTURING, DISC_ADVENTURING, "bandage", TASK_NORMAL, LAG_1, POSITION_DEAD, MANA_0, LIFEFORCE_0, PRAY_0, 0, SYMBOL_STRESS_0, "", "", "", "", START_1, LEARN_10, START_DO_30, LEARN_DO_5, START_DO_NO, LEARN_DO_NO, LEARN_DIFF_UNUSUAL, 0.3, 0, 0);

  discArray[SKILL_CLIMB] = new spellInfo(SKILL_GENERAL, DISC_ADVENTURING, DISC_ADVENTURING, "climbing", TASK_NORMAL, LAG_0, POSITION_DEAD, MANA_0, LIFEFORCE_0, PRAY_0, 0, SYMBOL_STRESS_0, "", "", "", "", START_1, LEARN_3, START_DO_20, LEARN_DO_5, START_DO_NO, LEARN_DO_NO, LEARN_DIFF_UNUSUAL, 0.0, 0, 0);

  discArray[SKILL_DISSECT] = new spellInfo(SKILL_GENERAL, DISC_ADVENTURING, DISC_ADVENTURING, "dissect", TASK_NORMAL, LAG_1, POSITION_SITTING, MANA_0, LIFEFORCE_0, PRAY_0, 0, SYMBOL_STRESS_0, "", "", "", "", START_1, LEARN_4, START_DO_10, LEARN_DO_1, START_DO_NO, LEARN_DO_NO, LEARN_DIFF_UNUSUAL, 0.0, 0, 0);

  discArray[SKILL_EVALUATE] = new spellInfo(SKILL_GENERAL, DISC_ADVENTURING, DISC_ADVENTURING, "evaluate", TASK_NORMAL, LAG_0, POSITION_SITTING, MANA_0, LIFEFORCE_0, PRAY_0, 0, SYMBOL_STRESS_0, "", "", "", "", START_51, LEARN_4, START_DO_1, LEARN_DO_2, START_DO_NO, LEARN_DO_NO, LEARN_DIFF_UNUSUAL,0.0, 0, 0);

  discArray[SKILL_TACTICS] = new spellInfo(SKILL_GENERAL, DISC_ADVENTURING, DISC_ADVENTURING, "tactics", TASK_NORMAL, LAG_0, POSITION_SITTING, MANA_0, LIFEFORCE_0, PRAY_0, 0, SYMBOL_STRESS_0, "", "", "", "", START_1, LEARN_5, START_DO_NO, LEARN_DO_NO, START_DO_NO, LEARN_DO_NO, LEARN_DIFF_UNUSUAL, 0.0, 0, 0);

  discArray[SKILL_DEFENSE] = new spellInfo(SKILL_GENERAL, DISC_ADVENTURING, DISC_ADVENTURING, "defense", TASK_NORMAL, LAG_0, POSITION_SITTING, MANA_0, LIFEFORCE_0, PRAY_0, 0, SYMBOL_STRESS_0, "", "", "", "", START_1, LEARN_1, START_DO_1, LEARN_DO_1, START_DO_NO, LEARN_DO_NO, LEARN_DIFF_UNUSUAL, 0.0, 0, 0);

  discArray[SKILL_ADVANCED_DEFENSE] = new spellInfo(SKILL_GENERAL, DISC_DEFENSE, DISC_DEFENSE, "advanced defense", TASK_NORMAL, LAG_0, POSITION_SITTING, MANA_0, LIFEFORCE_0, PRAY_0, 0, SYMBOL_STRESS_0, "", "", "", "", START_1, LEARN_1, START_DO_1, LEARN_DO_1, START_DO_NO, LEARN_DO_NO, LEARN_DIFF_UNUSUAL, 0.0, 0, 0);

  discArray[SKILL_OFFENSE] = new spellInfo(SKILL_GENERAL, DISC_ADVENTURING, DISC_ADVENTURING, "offense", TASK_NORMAL, LAG_0, POSITION_SITTING, MANA_0, LIFEFORCE_0, PRAY_0, 0, SYMBOL_STRESS_0, "", "", "", "", START_1, LEARN_1, START_DO_1, LEARN_DO_1, START_DO_NO, LEARN_DO_NO, LEARN_DIFF_UNUSUAL, 0.0, 0, 0);

  discArray[SKILL_WHITTLE] = new spellInfo(SKILL_GENERAL, DISC_ADVENTURING, DISC_ADVENTURING, "whittle", TASK_NORMAL, LAG_0, POSITION_STANDING, MANA_0, LIFEFORCE_0, PRAY_0, 0, SYMBOL_STRESS_0, "", "", "", "", START_50, LEARN_2, START_DO_1, LEARN_DO_1, START_DO_NO, LEARN_DO_NO, LEARN_DIFF_UNUSUAL, 0.0, 0, 0);

  discArray[SKILL_MEND] = new spellInfo(SKILL_GENERAL, DISC_ADVENTURING, DISC_BLACKSMITHING, "mend", TASK_NORMAL, LAG_0, POSITION_SITTING, MANA_0, LIFEFORCE_0, PRAY_0, 0, SYMBOL_STRESS_0, "", "", "", "", START_26, LEARN_3, START_DO_1, LEARN_DO_1, START_DO_NO, LEARN_DO_NO, LEARN_DIFF_SKILLS, 0.0, 0, 0);


  // disc_combat

  discArray[SKILL_SLASH_PROF] = new spellInfo(SKILL_GENERAL, DISC_COMBAT, DISC_COMBAT, "slash proficiency", TASK_NORMAL, LAG_0, POSITION_DEAD, MANA_0, LIFEFORCE_0, PRAY_0, 0, SYMBOL_STRESS_0, "", "", "", "", START_1, LEARN_1, START_DO_1, LEARN_DO_1, START_DO_NO, LEARN_DO_NO, LEARN_DIFF_UNUSUAL, 0.0, 0, 0);

  discArray[SKILL_RANGED_PROF] = new spellInfo(SKILL_GENERAL, DISC_COMBAT, DISC_COMBAT, "ranged proficiency", TASK_NORMAL, LAG_0, POSITION_DEAD, MANA_0, LIFEFORCE_0, PRAY_0, 0, SYMBOL_STRESS_0, "", "", "", "", START_51, LEARN_3, START_DO_1, LEARN_DO_5, START_DO_NO, LEARN_DO_NO, LEARN_DIFF_UNUSUAL, 0.0, 0, 0);

  discArray[SKILL_PIERCE_PROF] = new spellInfo(SKILL_GENERAL, DISC_COMBAT, DISC_COMBAT, "pierce proficiency", TASK_NORMAL, LAG_0, POSITION_DEAD, MANA_0, LIFEFORCE_0, PRAY_0, 0, SYMBOL_STRESS_0, "", "", "", "", START_1, LEARN_1, START_DO_1, LEARN_DO_1, START_DO_NO, LEARN_DO_NO, LEARN_DIFF_UNUSUAL, 0.0, 0, 0);

  discArray[SKILL_BLUNT_PROF] = new spellInfo(SKILL_GENERAL, DISC_COMBAT, DISC_COMBAT, "blunt proficiency", TASK_NORMAL, LAG_0, POSITION_DEAD, MANA_0, LIFEFORCE_0, PRAY_0, 0, SYMBOL_STRESS_0, "", "", "", "", START_1, LEARN_1, START_DO_1, LEARN_DO_1, START_DO_NO, LEARN_DO_NO, LEARN_DIFF_UNUSUAL, 0.0, 0, 0);

  discArray[SKILL_SHARPEN] = new spellInfo(SKILL_WARRIOR, DISC_COMBAT, DISC_COMBAT, "sharpen", TASK_NORMAL, LAG_0, POSITION_SITTING, MANA_0, LIFEFORCE_0, PRAY_0, 0, SYMBOL_STRESS_0, "", "", "", "", START_51, LEARN_2, START_DO_10, LEARN_DO_2, START_DO_NO, LEARN_DO_NO, LEARN_DIFF_UNUSUAL, 0.0, 0, 0);

  discArray[SKILL_DULL] = new spellInfo(SKILL_WARRIOR, DISC_COMBAT, DISC_COMBAT, "smooth", TASK_NORMAL, LAG_0, POSITION_SITTING, MANA_0, LIFEFORCE_0, PRAY_0, 0, SYMBOL_STRESS_0, "", "", "", "", START_51, LEARN_2, START_DO_10, LEARN_DO_2, START_DO_NO, LEARN_DO_NO, LEARN_DIFF_UNUSUAL, 0.0, 0, 0);

  discArray[SKILL_BAREHAND_PROF] = new spellInfo(SKILL_GENERAL, DISC_COMBAT, DISC_COMBAT, "barehand proficiency", TASK_NORMAL, LAG_0, POSITION_DEAD, MANA_0, LIFEFORCE_0, PRAY_0, 0, SYMBOL_STRESS_0, "", "", "", "", START_1, LEARN_1, START_DO_1, LEARN_DO_1, START_DO_NO, LEARN_DO_NO, LEARN_DIFF_UNUSUAL, 0.0, 0, 0);

  // disc_wizardry

  discArray[SKILL_WIZARDRY] = new spellInfo(SPELL_MAGE, DISC_WIZARDRY, DISC_WIZARDRY, "wizardry", TASK_NORMAL, LAG_0, POSITION_SLEEPING, MANA_0, LIFEFORCE_0, PRAY_0, 0, SYMBOL_STRESS_0, "", "", "", "", START_1, LEARN_1, START_DO_NO, LEARN_DO_NO, START_DO_NO, LEARN_DO_NO, LEARN_DIFF_UNUSUAL, 0.0, 0, 0);

// disc_lore
// disc_theology

  discArray[SKILL_ATTUNE] = new spellInfo(SKILL_CLERIC_TYPES, DISC_THEOLOGY, DISC_THEOLOGY, "attune", TASK_NORMAL, LAG_0, POSITION_SITTING, MANA_0, LIFEFORCE_0, PRAY_0, 0, SYMBOL_STRESS_0, "", "", "", "", START_10, LEARN_10, START_DO_50, LEARN_DO_2, START_DO_NO, LEARN_DO_NO, LEARN_DIFF_UNUSUAL, 0.0, SPELL_IGNORE_POSITION, 0);

  discArray[SKILL_MANA] = new spellInfo(SKILL_MAGE_TYPES, DISC_LORE, DISC_LORE, "mana", TASK_NORMAL, LAG_0, POSITION_RESTING, MANA_0, LIFEFORCE_0, PRAY_0, 0, SYMBOL_STRESS_0, "", "", "", "", START_1, LEARN_1, START_DO_1, LEARN_DO_1, START_DO_NO, LEARN_DO_NO, LEARN_DIFF_UNUSUAL, 0.0, SPELL_IGNORE_POSITION, 0);

  discArray[SKILL_MEDITATE] = new spellInfo(SKILL_MAGE_TYPES, DISC_LORE, DISC_LORE, "meditate", TASK_NORMAL, LAG_0, POSITION_RESTING, MANA_0, LIFEFORCE_0, PRAY_0, 0, SYMBOL_STRESS_0, "", "", "", "", START_1, LEARN_1, START_DO_1, LEARN_DO_1, START_DO_NO, LEARN_DO_NO, LEARN_DIFF_UNUSUAL, 0.0, SPELL_IGNORE_POSITION, 0);

  // SHAMAN STARTS HERE ***************************************************************************************

  // ritualism

  discArray[SKILL_RITUALISM] = new spellInfo(SPELL_SHAMAN, DISC_RITUALISM, DISC_RITUALISM, "ritualism", TASK_NORMAL, LAG_0, POSITION_SLEEPING, MANA_0, LIFEFORCE_0, PRAY_0, 0, SYMBOL_STRESS_0, "", "", "", "", START_1, LEARN_1, START_DO_NO, LEARN_DO_NO, START_DO_NO, LEARN_DO_NO, LEARN_DIFF_UNUSUAL, 0.0, 0, 0);

  // shaman basic

  discArray[SPELL_CHASE_SPIRIT] = new spellInfo(SPELL_SHAMAN, DISC_SHAMAN, DISC_SHAMAN_ARMADILLO, "chase spirits", TASK_NORMAL, LAG_1, POSITION_SITTING, MANA_0, LIFEFORCE_70, PRAY_0, TAR_CHAR_ROOM | TAR_FIGHT_VICT | TAR_OBJ_INV | TAR_OBJ_ROOM, SYMBOL_STRESS_0, "", "", "", "", START_61, LEARN_10, START_DO_40, LEARN_DO_5, START_DO_NO, LEARN_DO_NO, LEARN_DIFF_SPELLS, 0.04, COMP_GESTURAL | COMP_GESTURAL_RANDOM | COMP_VERBAL | COMP_VERBAL_RANDOM | COMP_MATERIAL | COMP_MATERIAL_END | SPELL_TASKED, 0);

  discArray[SPELL_FLATULENCE] = new spellInfo(SPELL_SHAMAN, DISC_SHAMAN, DISC_SHAMAN_SKUNK, "flatulence", TASK_EASY, LAG_1, POSITION_SITTING, MANA_0, LIFEFORCE_50, PRAY_0, TAR_AREA | TAR_FIGHT_VICT | TAR_VIOLENT | TAR_IGNORE, SYMBOL_STRESS_0, "", "", "", "", START_15, LEARN_15, START_DO_45, LEARN_DO_5, START_DO_NO, LEARN_DO_NO, LEARN_DIFF_SPELLS, 0.04, COMP_GESTURAL | COMP_GESTURAL_RANDOM | COMP_VERBAL | COMP_VERBAL_RANDOM | COMP_MATERIAL | COMP_MATERIAL_END | SPELL_TASKED, 0);

  discArray[SPELL_STUPIDITY] = new spellInfo(SPELL_SHAMAN, DISC_SHAMAN, DISC_SHAMAN_SPIDER, "stupidity", TASK_EASY, LAG_2, POSITION_SITTING, MANA_0, LIFEFORCE_30, PRAY_0, TAR_VIOLENT | TAR_FIGHT_VICT | TAR_CHAR_ROOM | TAR_SELF_NONO | TAR_SELF_NONO, SYMBOL_STRESS_0, "You suddenly feel brighter.", "$n seems a little more intelligent than before.", "The fog surrounding you lifts a little.", "The fog surrounding $n lifts a little.", START_15, LEARN_14, START_DO_35, LEARN_DO_7, START_DO_NO, LEARN_DO_NO, LEARN_DIFF_SPELLS, 0.03, COMP_GESTURAL | COMP_GESTURAL_RANDOM | COMP_VERBAL | COMP_VERBAL_RANDOM | COMP_MATERIAL | COMP_MATERIAL_END | SPELL_TASKED, 0);

  discArray[SPELL_DISTORT] = new spellInfo(SPELL_SHAMAN, DISC_SHAMAN, DISC_SHAMAN_FROG, "distort", TASK_NORMAL, LAG_1, POSITION_SITTING, MANA_0, LIFEFORCE_25, PRAY_0, TAR_CHAR_ROOM | TAR_SELF_NONO | TAR_VIOLENT | TAR_FIGHT_VICT , SYMBOL_STRESS_0, "", "", "", "", START_12, LEARN_25, START_DO_40, LEARN_DO_5, START_DO_NO, LEARN_DO_NO, LEARN_DIFF_SPELLS, 0.04, COMP_GESTURAL | COMP_GESTURAL_RANDOM | COMP_VERBAL | COMP_VERBAL_RANDOM | SPELL_TASKED, 0);

  discArray[SPELL_LEGBA] = new spellInfo(SPELL_SHAMAN, DISC_SHAMAN, DISC_SHAMAN_ARMADILLO, "legba's guidance", TASK_NORMAL, LAG_2, POSITION_CRAWLING, MANA_0, LIFEFORCE_70, PRAY_0, TAR_FIGHT_SELF | TAR_CHAR_ROOM, SYMBOL_STRESS_0, "", "", "", "", START_80, LEARN_10, START_DO_30, LEARN_DO_5, START_DO_NO, LEARN_DO_NO, LEARN_DIFF_SPELLS, 0.04, COMP_GESTURAL | COMP_GESTURAL_RANDOM | COMP_VERBAL | COMP_VERBAL_RANDOM | COMP_MATERIAL | COMP_MATERIAL_END | SPELL_TASKED, 0);

  discArray[SPELL_DJALLA] = new spellInfo(SPELL_SHAMAN, DISC_SHAMAN, DISC_SHAMAN_ARMADILLO, "djalla's protection", TASK_NORMAL, LAG_2, POSITION_CRAWLING, MANA_0, LIFEFORCE_60, PRAY_0, TAR_FIGHT_SELF | TAR_CHAR_ROOM, SYMBOL_STRESS_0, "", "", "", "", START_75, LEARN_10, START_DO_30, LEARN_DO_5, START_DO_NO, LEARN_DO_NO, LEARN_DIFF_SPELLS, 0.04, COMP_GESTURAL | COMP_GESTURAL_RANDOM | COMP_VERBAL | COMP_VERBAL_RANDOM | COMP_MATERIAL | COMP_MATERIAL_END | SPELL_TASKED, 0);

  discArray[SPELL_SENSE_LIFE_SHAMAN] = new spellInfo(SPELL_SHAMAN, DISC_SHAMAN, DISC_SHAMAN_ARMADILLO, "sense presence", TASK_EASY, LAG_1, POSITION_CRAWLING, MANA_0, LIFEFORCE_30, PRAY_0, TAR_CHAR_ROOM | TAR_FIGHT_SELF, SYMBOL_STRESS_0, "The pale blue aura fades from your eyes.", "The pale blue aura fades from $n's eyes.", "The pale blue in your eyes flickers slightly.", "The pale blue in $n's eyes flickers slightly.", START_24, LEARN_18, START_DO_40, LEARN_DO_5, START_DO_NO, LEARN_DO_NO, LEARN_DIFF_SPELLS, 0.04, COMP_GESTURAL | COMP_GESTURAL_RANDOM | COMP_VERBAL | COMP_VERBAL_RANDOM | COMP_MATERIAL | COMP_MATERIAL_END | SPELL_TASKED, 0);

  discArray[SPELL_DETECT_SHADOW] = new spellInfo(SPELL_SHAMAN, DISC_SHAMAN, DISC_SHAMAN_ARMADILLO, "detect shadow", TASK_NORMAL, LAG_2, POSITION_CRAWLING, MANA_0, LIFEFORCE_70, PRAY_0, TAR_CHAR_ROOM | TAR_FIGHT_SELF | TAR_CHAR_ROOM, SYMBOL_STRESS_0, "Your eyes sting a little.", "$n seems to be having some vision problems.", "", "", START_85, LEARN_10, START_DO_40, LEARN_DO_5, START_DO_NO, LEARN_DO_NO, LEARN_DIFF_SPELLS, 0.04, COMP_GESTURAL | COMP_GESTURAL_RANDOM | COMP_VERBAL | COMP_VERBAL_RANDOM | COMP_MATERIAL | COMP_MATERIAL_END | SPELL_TASKED, 0);

  discArray[SPELL_INTIMIDATE] = new spellInfo(SPELL_SHAMAN, DISC_SHAMAN, DISC_SHAMAN_SKUNK, "intimidate", TASK_DIFFICULT, LAG_2, POSITION_SITTING, MANA_0, LIFEFORCE_40, PRAY_0, TAR_CHAR_ROOM | TAR_SELF_NONO | TAR_VIOLENT | TAR_FIGHT_VICT , SYMBOL_STRESS_0, "You are less intimidated.", "$n doesn't seem as intimidated as $e was before.", "", "", START_42, LEARN_11, START_DO_45, LEARN_DO_5, START_DO_NO, LEARN_DO_NO, LEARN_DIFF_SPELLS, 0.04, COMP_GESTURAL | COMP_GESTURAL_RANDOM | COMP_VERBAL | COMP_VERBAL_RANDOM | COMP_MATERIAL | COMP_MATERIAL_END | SPELL_TASKED, 0) ;

  discArray[SPELL_ROMBLER] = new spellInfo(SPELL_SHAMAN, DISC_SHAMAN, DISC_SHAMAN, "romble", TASK_EASY, LAG_1, POSITION_CRAWLING, MANA_0, LIFEFORCE_20, PRAY_0, TAR_NAME, SYMBOL_STRESS_0, "", "", "", "", START_47, LEARN_15, START_DO_50, LEARN_DO_5, START_DO_NO, LEARN_DO_NO, LEARN_DIFF_SPELLS, 0.04, COMP_GESTURAL | COMP_GESTURAL_RANDOM | COMP_VERBAL | COMP_VERBAL_RANDOM | COMP_MATERIAL | COMP_MATERIAL_END | SPELL_TASKED, 0);

  discArray[SPELL_EMBALM] = new spellInfo(SPELL_SHAMAN, DISC_SHAMAN, DISC_SHAMAN, "embalm", TASK_EASY, LAG_1, POSITION_CRAWLING, MANA_0, LIFEFORCE_20, PRAY_0, TAR_OBJ_ROOM | TAR_SELF_NONO, SYMBOL_STRESS_0, "", "", "", "", START_47, LEARN_15, START_DO_50, LEARN_DO_5, START_DO_NO, LEARN_DO_NO, LEARN_DIFF_SPELLS, 0.04, COMP_GESTURAL | COMP_GESTURAL_RANDOM | COMP_VERBAL | COMP_VERBAL_RANDOM | COMP_MATERIAL | COMP_MATERIAL_END | SPELL_TASKED, 0);


  discArray[SKILL_SACRIFICE] = new spellInfo(SKILL_SHAMAN, DISC_SHAMAN, DISC_SHAMAN, "sacrifice", TASK_DANGEROUS, LAG_0, POSITION_SITTING, MANA_0, LIFEFORCE_0, PRAY_0, 0, SYMBOL_STRESS_0, "", "", "", "", START_1, LEARN_2, START_DO_35, LEARN_DO_2, START_DO_NO, LEARN_DO_NO, LEARN_DIFF_SKILLS, 0.0, 0, 0);

  discArray[SPELL_CHRISM] = new spellInfo(SPELL_SHAMAN, DISC_SHAMAN, DISC_SHAMAN_ALCHEMY, "chrism", TASK_TRIVIAL, LAG_1, POSITION_CRAWLING, MANA_0, LIFEFORCE_40, PRAY_0, TAR_NAME, SYMBOL_STRESS_0, "", "", "", "", START_15, LEARN_25, START_DO_40, LEARN_DO_7, START_DO_NO, LEARN_DO_NO, LEARN_DIFF_SPELLS, 0.0, COMP_GESTURAL | COMP_GESTURAL_RANDOM | COMP_VERBAL | COMP_VERBAL_RANDOM | SPELL_TASKED, 0);

  discArray[SPELL_VAMPIRIC_TOUCH] = new spellInfo(SPELL_SHAMAN, DISC_SHAMAN, DISC_SHAMAN_SKUNK, "vampiric touch", TASK_DIFFICULT, LAG_2, POSITION_SITTING, MANA_0, LIFEFORCE_80, PRAY_0, TAR_CHAR_ROOM | TAR_SELF_NONO | TAR_VIOLENT | TAR_FIGHT_VICT , SYMBOL_STRESS_0, "", "", "", "", START_80, LEARN_5, START_DO_50, LEARN_DO_2, START_DO_NO, LEARN_DO_NO, LEARN_DIFF_SPELLS, 0.04, COMP_GESTURAL | COMP_VERBAL | COMP_MATERIAL  | COMP_MATERIAL_END | SPELL_TASKED, 0);

  discArray[SPELL_CHEVAL] = new spellInfo(SPELL_SHAMAN, DISC_SHAMAN, DISC_SHAMAN_ARMADILLO, "cheval", TASK_NORMAL, LAG_2, POSITION_CRAWLING, MANA_0, LIFEFORCE_60, PRAY_0, TAR_SELF_ONLY | TAR_FIGHT_SELF | TAR_CHAR_ROOM, SYMBOL_STRESS_0, "The loa are growing tired of your body.", "$n seems to be calming down.", "", "", START_30, LEARN_10, START_DO_40, LEARN_DO_5, START_DO_NO, LEARN_DO_NO, LEARN_DIFF_SPELLS, 0.04, COMP_GESTURAL | COMP_GESTURAL_RANDOM | COMP_VERBAL | COMP_VERBAL_RANDOM | COMP_MATERIAL | COMP_MATERIAL_END | SPELL_TASKED, 0);

  discArray[SPELL_SQUISH] = new spellInfo(SPELL_SHAMAN, DISC_SHAMAN, DISC_SHAMAN_SPIDER, "squish", TASK_NORMAL, LAG_1, POSITION_SITTING, MANA_0, LIFEFORCE_50, PRAY_0, TAR_CHAR_ROOM | TAR_SELF_NONO | TAR_VIOLENT | TAR_FIGHT_VICT , SYMBOL_STRESS_0, "", "", "", "", START_15, LEARN_5, START_DO_40, LEARN_DO_1, START_DO_NO, LEARN_DO_NO, LEARN_DIFF_SPELLS, 0.04, COMP_GESTURAL | COMP_VERBAL | COMP_MATERIAL  | COMP_MATERIAL_END | SPELL_TASKED, 0);

  discArray[SPELL_SOUL_TWIST] = new spellInfo(SPELL_SHAMAN, DISC_SHAMAN, DISC_SHAMAN_SKUNK, "soul twister", TASK_NORMAL, LAG_1, POSITION_SITTING, MANA_0, LIFEFORCE_60, PRAY_0, TAR_CHAR_ROOM | TAR_SELF_NONO | TAR_VIOLENT | TAR_FIGHT_VICT , SYMBOL_STRESS_0, "", "", "", "", START_60, LEARN_5, START_DO_40, LEARN_DO_1, START_DO_NO, LEARN_DO_NO, LEARN_DIFF_SPELLS, 0.04, COMP_GESTURAL | COMP_VERBAL | COMP_MATERIAL  | COMP_MATERIAL_END | SPELL_TASKED, 0);

  discArray[SPELL_VOODOO] = new spellInfo(SPELL_SHAMAN, DISC_SHAMAN, DISC_SHAMAN_CONTROL, "voodoo", TASK_NORMAL, LAG_2, POSITION_SITTING, MANA_0, LIFEFORCE_50, PRAY_0, TAR_OBJ_ROOM | TAR_SELF_NONO | TAR_VIOLENT | TAR_FIGHT_VICT , SYMBOL_STRESS_0, "", "", "", "", START_21, LEARN_8, START_DO_40, LEARN_DO_5, START_DO_NO, LEARN_DO_NO, LEARN_DIFF_SPELLS, 0.04, COMP_GESTURAL | COMP_VERBAL | COMP_MATERIAL  | COMP_MATERIAL_END | SPELL_TASKED, 0);

  discArray[SPELL_DANCING_BONES] = new spellInfo(SPELL_SHAMAN, DISC_SHAMAN, DISC_SHAMAN_CONTROL, "dancing bones", TASK_NORMAL, LAG_3, POSITION_SITTING, MANA_0, LIFEFORCE_110, PRAY_0, TAR_OBJ_ROOM | TAR_SELF_NONO | TAR_VIOLENT | TAR_FIGHT_VICT , SYMBOL_STRESS_0, "", "", "", "", START_70, LEARN_8, START_DO_40, LEARN_DO_5, START_DO_NO, LEARN_DO_NO, LEARN_DIFF_SPELLS, 0.04, COMP_GESTURAL | COMP_VERBAL | COMP_MATERIAL  | COMP_MATERIAL_END | SPELL_TASKED, 0);

  discArray[SPELL_SHIELD_OF_MISTS] = new spellInfo(SPELL_SHAMAN, DISC_SHAMAN, DISC_SHAMAN, "shield of mists", TASK_EASY, LAG_1, POSITION_CRAWLING, MANA_0, LIFEFORCE_20, PRAY_0, TAR_CHAR_ROOM | TAR_FIGHT_SELF, SYMBOL_STRESS_0, "The mist surrounding you has faded into memory.", "The mist surrounding $n fades to memory.", "The mist surrounding you starts to become transparent.", "The mist surrounding $n disperses.", START_1, LEARN_8, START_DO_50, LEARN_DO_5, START_DO_NO, LEARN_DO_NO, LEARN_DIFF_SPELLS, 0.01, COMP_GESTURAL | COMP_GESTURAL_RANDOM | COMP_VERBAL | COMP_VERBAL_RANDOM | COMP_MATERIAL | COMP_MATERIAL_END | SPELL_TASKED, 0);

  discArray[SPELL_LIFE_LEECH] = new spellInfo(SPELL_SHAMAN, DISC_SHAMAN, DISC_SHAMAN_SPIDER, "life leech", TASK_EASY, LAG_1, POSITION_SITTING, MANA_0, LIFEFORCE_20, PRAY_0, TAR_CHAR_ROOM | TAR_SELF_NONO | TAR_VIOLENT | TAR_FIGHT_VICT , SYMBOL_STRESS_0, "", "", "", "", START_3, LEARN_5, START_DO_25, LEARN_DO_1, START_DO_NO, LEARN_DO_NO, LEARN_DIFF_SPELLS, 0.04, COMP_GESTURAL | COMP_VERBAL | COMP_MATERIAL  | COMP_MATERIAL_END | SPELL_TASKED, 0);

  discArray[SPELL_ENTHRALL_SPECTRE] = new spellInfo(SPELL_SHAMAN, DISC_SHAMAN, DISC_SHAMAN_CONTROL, "enthrall spectre", TASK_NORMAL, LAG_1, POSITION_CRAWLING, MANA_0, LIFEFORCE_70, PRAY_0, TAR_IGNORE, SYMBOL_STRESS_0, "", "", "", "", START_35, LEARN_15, START_DO_50, LEARN_DO_5, START_DO_NO, LEARN_DO_NO, LEARN_DIFF_SPELLS, 0.00, COMP_GESTURAL | COMP_GESTURAL_RANDOM | COMP_VERBAL | COMP_VERBAL_RANDOM | COMP_MATERIAL | COMP_MATERIAL_END | SPELL_TASKED, 0);

  discArray[SPELL_ENTHRALL_GHAST] = new spellInfo(SPELL_SHAMAN, DISC_SHAMAN, DISC_SHAMAN_CONTROL, "enthrall ghast", TASK_NORMAL, LAG_1, POSITION_CRAWLING, MANA_0, LIFEFORCE_90, PRAY_0, TAR_IGNORE, SYMBOL_STRESS_0, "", "", "", "", START_57, LEARN_13, START_DO_50, LEARN_DO_5, START_DO_NO, LEARN_DO_NO, LEARN_DIFF_SPELLS, 0.0, COMP_GESTURAL | COMP_GESTURAL_RANDOM | COMP_VERBAL | COMP_VERBAL_RANDOM | COMP_MATERIAL | COMP_MATERIAL_END | SPELL_TASKED, 0);

  discArray[SPELL_ENTHRALL_GHOUL] = new spellInfo(SPELL_SHAMAN, DISC_SHAMAN, DISC_SHAMAN_CONTROL, "enthrall ghoul", TASK_DIFFICULT, LAG_1, POSITION_CRAWLING, MANA_0, LIFEFORCE_120, PRAY_0, TAR_IGNORE, SYMBOL_STRESS_0, "", "", "", "", START_80, LEARN_15, START_DO_50, LEARN_DO_5, START_DO_NO, LEARN_DO_NO, LEARN_DIFF_SPELLS, 0.0, COMP_GESTURAL | COMP_GESTURAL_RANDOM | COMP_VERBAL | COMP_VERBAL_RANDOM | COMP_MATERIAL | COMP_MATERIAL_END | SPELL_TASKED, 0);

  discArray[SKILL_REPAIR_SHAMAN] = new spellInfo(SKILL_SHAMAN, DISC_SHAMAN, DISC_BLACKSMITHING, "shaman repair", TASK_NORMAL, LAG_0, POSITION_SITTING, MANA_0, LIFEFORCE_0, PRAY_0, 0, SYMBOL_STRESS_0, "", "", "", "", START_50, LEARN_3, START_DO_1, LEARN_DO_1, START_DO_NO, LEARN_DO_NO, LEARN_DIFF_SKILLS, 0.0, 0, 0);

  // shaman control

  discArray[SPELL_ENTHRALL_DEMON] = new spellInfo(SPELL_SHAMAN, DISC_SHAMAN_CONTROL, DISC_SHAMAN_CONTROL, "enthrall demon", TASK_DIFFICULT, LAG_1, POSITION_CRAWLING, MANA_0, LIFEFORCE_150, PRAY_0, TAR_IGNORE, SYMBOL_STRESS_0, "", "", "", "", START_41, LEARN_10, START_DO_50, LEARN_DO_5, START_DO_NO, LEARN_DO_NO, LEARN_DIFF_SPELLS, 0.0, COMP_GESTURAL | COMP_GESTURAL_RANDOM | COMP_VERBAL | COMP_VERBAL_RANDOM | COMP_MATERIAL | COMP_MATERIAL_END | SPELL_TASKED, 0);

  discArray[SPELL_CREATE_WOOD_GOLEM] = new spellInfo(SPELL_SHAMAN, DISC_SHAMAN_CONTROL, DISC_SHAMAN_CONTROL, "create wood golem", TASK_DIFFICULT, LAG_1, POSITION_CRAWLING, MANA_0, LIFEFORCE_170, PRAY_0, TAR_IGNORE, SYMBOL_STRESS_0, "", "", "", "", START_60, LEARN_10, START_DO_50, LEARN_DO_5, START_DO_NO, LEARN_DO_NO, LEARN_DIFF_SPELLS, 0.0, COMP_GESTURAL | COMP_GESTURAL_RANDOM | COMP_VERBAL | COMP_VERBAL_RANDOM | COMP_MATERIAL | COMP_MATERIAL_END | SPELL_TASKED, 0);

  discArray[SPELL_CREATE_ROCK_GOLEM] = new spellInfo(SPELL_SHAMAN, DISC_SHAMAN_CONTROL, DISC_SHAMAN_CONTROL, "create rock golem", TASK_DANGEROUS, LAG_1, POSITION_CRAWLING, MANA_0, LIFEFORCE_190, PRAY_0, TAR_IGNORE, SYMBOL_STRESS_0, "", "", "", "", START_70, LEARN_10, START_DO_50, LEARN_DO_5, START_DO_NO, LEARN_DO_NO, LEARN_DIFF_SPELLS, 0.0, COMP_GESTURAL | COMP_GESTURAL_RANDOM | COMP_VERBAL | COMP_VERBAL_RANDOM | COMP_MATERIAL | COMP_MATERIAL_END | SPELL_TASKED, 0);

  discArray[SPELL_CREATE_IRON_GOLEM] = new spellInfo(SPELL_SHAMAN, DISC_SHAMAN_CONTROL, DISC_SHAMAN_CONTROL, "create iron golem", TASK_DANGEROUS, LAG_1, POSITION_CRAWLING, MANA_0, LIFEFORCE_190, PRAY_0, TAR_IGNORE, SYMBOL_STRESS_0, "", "", "", "", START_75, LEARN_10, START_DO_50, LEARN_DO_5, START_DO_NO, LEARN_DO_NO, LEARN_DIFF_SPELLS, 0.0, COMP_GESTURAL | COMP_GESTURAL_RANDOM | COMP_VERBAL | COMP_VERBAL_RANDOM | COMP_MATERIAL | COMP_MATERIAL_END | SPELL_TASKED, 0);

  discArray[SPELL_CREATE_DIAMOND_GOLEM] = new spellInfo(SPELL_SHAMAN, DISC_SHAMAN_CONTROL, DISC_SHAMAN_CONTROL, "create diamond golem", TASK_DANGEROUS, LAG_1, POSITION_CRAWLING, MANA_0, LIFEFORCE_210, PRAY_0, TAR_IGNORE, SYMBOL_STRESS_0, "", "", "", "", START_85, LEARN_10, START_DO_50, LEARN_DO_5, START_DO_NO, LEARN_DO_NO, LEARN_DIFF_SPELLS, 0.0, COMP_GESTURAL | COMP_GESTURAL_RANDOM | COMP_VERBAL | COMP_VERBAL_RANDOM | COMP_MATERIAL | COMP_MATERIAL_END | SPELL_TASKED, 0);

  discArray[SPELL_RESURRECTION] = new spellInfo(SPELL_SHAMAN, DISC_SHAMAN_CONTROL, DISC_SHAMAN_CONTROL, "resurrection", TASK_DANGEROUS, LAG_5, POSITION_SITTING, MANA_0, LIFEFORCE_330, PRAY_0, TAR_OBJ_ROOM | TAR_SELF_NONO | TAR_VIOLENT | TAR_FIGHT_VICT , SYMBOL_STRESS_0, "", "", "", "", START_80, LEARN_5, START_DO_30, LEARN_DO_5, START_DO_NO, LEARN_DO_NO, LEARN_DIFF_SPELLS, 0.04, COMP_GESTURAL | COMP_VERBAL | COMP_MATERIAL  | COMP_MATERIAL_END | SPELL_TASKED, 0);

  // shaman spider

  discArray[SPELL_ROOT_CONTROL] = new spellInfo(SPELL_SHAMAN, DISC_SHAMAN_SPIDER, DISC_SHAMAN_SPIDER, "root control", TASK_NORMAL, LAG_3, POSITION_SITTING, MANA_15, LIFEFORCE_0, PRAY_0, TAR_CHAR_ROOM | TAR_VIOLENT | TAR_FIGHT_VICT | TAR_SELF_NONO, SYMBOL_STRESS_0, "", "", "", "", START_1, LEARN_5, START_DO_20, LEARN_DO_5, START_DO_NO, LEARN_DO_NO, LEARN_DIFF_SPELLS, 0.04, COMP_GESTURAL | COMP_VERBAL | COMP_MATERIAL, 0);


  discArray[SKILL_TRANSFIX] = new spellInfo(SPELL_SHAMAN, DISC_SHAMAN_SPIDER, DISC_SHAMAN_SPIDER, "transfix", TASK_DIFFICULT, LAG_3, POSITION_FIGHTING, MANA_0, LIFEFORCE_0, PRAY_0, TAR_SELF_NONO | TAR_CHAR_ROOM, SYMBOL_STRESS_0, "You're not transfixed anymore.", "$n doesn't seem to be transfixed anymore.", "You seem slightly more like yourself.", "$n seems slightly more like $mself.", START_21, LEARN_5, START_DO_20, LEARN_DO_5, START_DO_NO, LEARN_DO_NO, LEARN_DIFF_SPELLS, 0.02, COMP_VERBAL | COMP_MATERIAL, 0); 

  discArray[SPELL_LIVING_VINES] = new spellInfo(SPELL_SHAMAN, DISC_SHAMAN_SPIDER, DISC_SHAMAN_SPIDER, "living vines", TASK_NORMAL, LAG_3, POSITION_SITTING, MANA_0, LIFEFORCE_0, PRAY_0, TAR_CHAR_ROOM, SYMBOL_STRESS_0, "", "", "", "", START_41, LEARN_4, START_DO_40, LEARN_DO_5, START_DO_NO, LEARN_DO_NO, LEARN_DIFF_SPELLS, 0.04, COMP_GESTURAL | COMP_VERBAL | COMP_MATERIAL, 0);



  discArray[SPELL_RAZE] = new spellInfo(SPELL_SHAMAN, DISC_SHAMAN_SPIDER, DISC_SHAMAN_SPIDER, "raze", TASK_DANGEROUS, LAG_3, POSITION_SITTING, MANA_0, LIFEFORCE_400, PRAY_0, TAR_CHAR_ROOM | TAR_SELF_NONO | TAR_VIOLENT | TAR_FIGHT_VICT , SYMBOL_STRESS_0, "", "", "", "", START_80, LEARN_5, START_DO_35, LEARN_DO_5, START_DO_NO, LEARN_DO_NO, LEARN_DIFF_SPELLS, 0.04, COMP_GESTURAL | COMP_VERBAL | COMP_MATERIAL | SPELL_TASKED | COMP_MATERIAL_INIT, 0);

  discArray[SPELL_STICKS_TO_SNAKES] = new spellInfo(SPELL_SHAMAN, DISC_SHAMAN_SPIDER, DISC_SHAMAN_SPIDER, "sticks to snakes", TASK_NORMAL, LAG_3, POSITION_SITTING, MANA_0, LIFEFORCE_130, PRAY_0, TAR_CHAR_ROOM | TAR_SELF_NONO | TAR_VIOLENT | TAR_FIGHT_VICT , SYMBOL_STRESS_0, "", "", "", "", START_1, LEARN_5, START_DO_NO, LEARN_DO_5, START_DO_NO, LEARN_DO_NO, LEARN_DIFF_SPELLS, 0.04, COMP_GESTURAL | COMP_VERBAL | COMP_MATERIAL | SPELL_TASKED | COMP_MATERIAL_INIT, 0);

  discArray[SPELL_HYPNOSIS] = new spellInfo(SPELL_SHAMAN, DISC_SHAMAN_SPIDER, DISC_SHAMAN_SPIDER, "hypnosis", TASK_DIFFICULT, LAG_1, POSITION_CRAWLING, MANA_0, LIFEFORCE_300, PRAY_0, TAR_CHAR_ROOM | TAR_SELF_NONO | TAR_FIGHT_VICT | TAR_VIOLENT, SYMBOL_STRESS_0, "You shake your head really hard and free yourself from the hypnosis.", "$n shakes $s head really hard and frees $mself from the hypnosis.", "You momentarily become cognizant of the world around you.", "You see a brief flicker of intelligence in $n's eyes.", START_50, LEARN_11, START_DO_40, LEARN_DO_5, START_DO_NO, LEARN_DO_NO, LEARN_DIFF_SPELLS, 0.04, COMP_GESTURAL | COMP_GESTURAL_RANDOM | COMP_VERBAL | COMP_VERBAL_RANDOM | COMP_MATERIAL | COMP_MATERIAL_END | SPELL_TASKED, 0);

  discArray[SPELL_CLARITY] = new spellInfo(SPELL_SHAMAN, DISC_SHAMAN_SPIDER, DISC_SHAMAN_SPIDER, "clarity", TASK_NORMAL, LAG_1, POSITION_CRAWLING, MANA_0, LIFEFORCE_150, PRAY_0, TAR_CHAR_ROOM | TAR_SELF_ONLY, SYMBOL_STRESS_0, "The green film on your eyes disolves.", "The green film on $n's eyes disolves.", "The film in your eyes becomes more transparent.", "", START_10, LEARN_8, START_DO_40, LEARN_DO_5, START_DO_NO, LEARN_DO_NO, LEARN_DIFF_SPELLS, 0.04, COMP_GESTURAL | COMP_GESTURAL_RANDOM | COMP_VERBAL | COMP_VERBAL_RANDOM | COMP_MATERIAL | COMP_MATERIAL_END | SPELL_TASKED, 0);

  discArray[SPELL_CONTROL_UNDEAD] = new spellInfo(SPELL_SHAMAN, DISC_SHAMAN_SPIDER, DISC_SHAMAN_SPIDER, "control undead", TASK_DANGEROUS, LAG_1, POSITION_CRAWLING, MANA_0, LIFEFORCE_240, PRAY_0, TAR_CHAR_ROOM | TAR_SELF_NONO, SYMBOL_STRESS_0, "", "", "", "", START_70, LEARN_5, START_DO_30, LEARN_DO_3, START_DO_NO, LEARN_DO_NO, LEARN_DIFF_SPELLS, 0.04, COMP_GESTURAL | COMP_GESTURAL_RANDOM | COMP_VERBAL | COMP_VERBAL_RANDOM | COMP_MATERIAL | COMP_MATERIAL_END | SPELL_TASKED, 0);

  // shaman frog

  discArray[SKILL_TRANSFORM_LIMB] = new spellInfo(SPELL_SHAMAN, DISC_SHAMAN_FROG, DISC_SHAMAN_FROG, "transform limb", TASK_NORMAL, LAG_3, POSITION_CRAWLING, MANA_0, LIFEFORCE_0, PRAY_0, TAR_NAME, SYMBOL_STRESS_0, "Your limbs tingles as the transforming magic starts to leave your body.", "$n'slimbs shimmer and then start to transform back into their original form.", "Your limbs start to tingle as the magic ones momentarily resemble their original form.", "$n's limbs shimmer and momentarily resemble their original form.", START_21, LEARN_4, START_DO_20, LEARN_DO_5, START_DO_NO, LEARN_DO_NO, LEARN_DIFF_SPELLS, 0.04, COMP_MATERIAL, 0);


  discArray[SPELL_CREEPING_DOOM] = new spellInfo(SPELL_SHAMAN, DISC_SHAMAN_FROG, DISC_SHAMAN_FROG, "creeping doom", TASK_NORMAL, LAG_3, POSITION_SITTING, MANA_10, LIFEFORCE_0, PRAY_0, TAR_CHAR_ROOM | TAR_SELF_NONO | TAR_FIGHT_VICT | TAR_VIOLENT, SYMBOL_STRESS_0, "", "", "", "", START_41, LEARN_4, START_DO_40, LEARN_DO_5, START_DO_NO, LEARN_DO_NO, LEARN_DIFF_SPELLS, 0.04, COMP_GESTURAL | COMP_VERBAL | COMP_MATERIAL, 0);

  discArray[SPELL_STORMY_SKIES] = new spellInfo(SPELL_SHAMAN, DISC_SHAMAN_FROG, DISC_SHAMAN_FROG, "stormy skies", TASK_DANGEROUS, LAG_4, POSITION_SITTING, MANA_0, LIFEFORCE_170, PRAY_0, TAR_CHAR_ROOM | TAR_SELF_NONO | TAR_VIOLENT | TAR_FIGHT_VICT , SYMBOL_STRESS_0, "", "", "", "", START_30, LEARN_3, START_DO_40, LEARN_DO_5, START_DO_NO, LEARN_DO_NO, LEARN_DIFF_SPELLS, 0.05, COMP_GESTURAL | COMP_GESTURAL_RANDOM | COMP_VERBAL | COMP_VERBAL_RANDOM | COMP_MATERIAL | COMP_MATERIAL_END | SPELL_TASKED, 0);

  discArray[SPELL_AQUATIC_BLAST] = new spellInfo(SPELL_SHAMAN, DISC_SHAMAN_FROG, DISC_SHAMAN_FROG, "aquatic blast", TASK_DANGEROUS, LAG_4, POSITION_SITTING, MANA_0, LIFEFORCE_230, PRAY_0, TAR_CHAR_ROOM | TAR_SELF_NONO | TAR_VIOLENT | TAR_FIGHT_VICT , SYMBOL_STRESS_0, "", "", "", "", START_50, LEARN_20, START_DO_40, LEARN_DO_5, START_DO_NO, LEARN_DO_NO, LEARN_DIFF_SPELLS, 0.05, COMP_GESTURAL | COMP_GESTURAL_RANDOM | COMP_VERBAL | COMP_VERBAL_RANDOM | COMP_MATERIAL | COMP_MATERIAL_END | SPELL_TASKED, 0);

  discArray[SPELL_DEATHWAVE] = new spellInfo(SPELL_SHAMAN, DISC_SHAMAN_FROG, DISC_SHAMAN_FROG, "death wave", TASK_DANGEROUS, LAG_4, POSITION_SITTING, MANA_0, LIFEFORCE_160, PRAY_0, TAR_CHAR_ROOM | TAR_SELF_NONO | TAR_VIOLENT | TAR_FIGHT_VICT , SYMBOL_STRESS_0, "", "", "", "", START_75, LEARN_20, START_DO_40, LEARN_DO_5, START_DO_NO, LEARN_DO_NO, LEARN_DIFF_SPELLS, 0.05, COMP_GESTURAL | COMP_GESTURAL_RANDOM | COMP_VERBAL | COMP_VERBAL_RANDOM | COMP_MATERIAL | COMP_MATERIAL_END | SPELL_TASKED, 0);

  discArray[SPELL_SHAPESHIFT] = new spellInfo(SPELL_SHAMAN, DISC_SHAMAN_FROG, DISC_SHAMAN_FROG, "shapeshift", TASK_NORMAL, LAG_2, POSITION_CRAWLING, MANA_0, LIFEFORCE_200, PRAY_0, TAR_NAME, SYMBOL_STRESS_0, "You are not able to hold this form any longer.", "", "You do not know how much longer you can hold this form.", "", START_5, LEARN_10, START_DO_50, LEARN_DO_4, START_DO_NO, LEARN_DO_NO, LEARN_DIFF_SPELLS, 0.0, COMP_GESTURAL | COMP_GESTURAL_RANDOM | COMP_VERBAL | COMP_VERBAL_RANDOM | COMP_MATERIAL | COMP_MATERIAL_END | SPELL_TASKED, 0);


  // shaman skunk

  discArray[SPELL_BLOOD_BOIL] = new spellInfo(SPELL_SHAMAN, DISC_SHAMAN_SKUNK, DISC_SHAMAN_SKUNK, "boiling blood", TASK_NORMAL, LAG_2, POSITION_SITTING, MANA_0, LIFEFORCE_150, PRAY_0, TAR_CHAR_ROOM | TAR_SELF_NONO | TAR_VIOLENT | TAR_FIGHT_VICT , SYMBOL_STRESS_0, "", "","", "", START_20, LEARN_50, START_DO_45, LEARN_DO_5, START_DO_NO, LEARN_DO_NO, LEARN_DIFF_SPELLS, 0.04, COMP_GESTURAL | COMP_GESTURAL_RANDOM | COMP_VERBAL | COMP_VERBAL_RANDOM | COMP_MATERIAL | COMP_MATERIAL_END | SPELL_TASKED, 0);

  discArray[SPELL_CLEANSE] = new spellInfo(SPELL_SHAMAN, DISC_SHAMAN_SKUNK, DISC_SHAMAN_SKUNK, "cleanse", TASK_EASY, LAG_2, POSITION_SITTING, MANA_0, LIFEFORCE_300, PRAY_0, TAR_CHAR_ROOM | TAR_FIGHT_SELF, SYMBOL_STRESS_0, "", "", "", "", START_1, LEARN_5, START_DO_40, LEARN_DO_10, START_DO_NO, LEARN_DO_NO, LEARN_DIFF_SPELLS, 0.04, COMP_GESTURAL | COMP_GESTURAL_RANDOM | COMP_VERBAL | COMP_VERBAL_RANDOM | COMP_MATERIAL | COMP_MATERIAL_END | SPELL_TASKED, 0);

  discArray[SPELL_DEATH_MIST] = new spellInfo(SPELL_SHAMAN, DISC_SHAMAN_SKUNK, DISC_SHAMAN_SKUNK, "death mist", TASK_NORMAL, LAG_3, POSITION_SITTING, MANA_0, LIFEFORCE_260, PRAY_0, TAR_AREA | TAR_FIGHT_VICT | TAR_VIOLENT | TAR_IGNORE, SYMBOL_STRESS_0, "", "", "", "", START_25, LEARN_5, START_DO_50, LEARN_DO_5, START_DO_NO, LEARN_DO_NO, LEARN_DIFF_SPELLS, 0.04, COMP_GESTURAL | COMP_GESTURAL_RANDOM | COMP_VERBAL | COMP_VERBAL_RANDOM | COMP_MATERIAL | COMP_MATERIAL_END | SPELL_TASKED, 0);

  discArray[SPELL_CARDIAC_STRESS] = new spellInfo(SPELL_SHAMAN, DISC_SHAMAN_SKUNK, DISC_SHAMAN_SKUNK, "coronary", TASK_DANGEROUS, LAG_4, POSITION_SITTING, MANA_0, LIFEFORCE_240, PRAY_0, TAR_CHAR_ROOM | TAR_SELF_NONO | TAR_VIOLENT | TAR_FIGHT_VICT , SYMBOL_STRESS_0, "", "", "", "", START_75, LEARN_20, START_DO_40, LEARN_DO_5, START_DO_NO, LEARN_DO_NO, LEARN_DIFF_SPELLS, 0.05, COMP_GESTURAL | COMP_GESTURAL_RANDOM | COMP_VERBAL | COMP_VERBAL_RANDOM | COMP_MATERIAL | COMP_MATERIAL_END | SPELL_TASKED, 0);

  discArray[SPELL_LICH_TOUCH] = new spellInfo(SPELL_SHAMAN, DISC_SHAMAN_SKUNK, DISC_SHAMAN_SKUNK, "lich touch", TASK_DANGEROUS, LAG_3, POSITION_SITTING, MANA_0, LIFEFORCE_120, PRAY_0, TAR_CHAR_ROOM | TAR_SELF_NONO | TAR_VIOLENT | TAR_FIGHT_VICT , SYMBOL_STRESS_0, "", "", "", "", START_50, LEARN_5, START_DO_50, LEARN_DO_2, START_DO_NO, LEARN_DO_NO, LEARN_DIFF_SPELLS, 0.04, COMP_GESTURAL | COMP_VERBAL | COMP_MATERIAL  | COMP_MATERIAL_END | SPELL_TASKED, 0);

  discArray[SKILL_TURN] = new spellInfo(SKILL_SHAMAN, DISC_SHAMAN_SKUNK, DISC_SHAMAN_SKUNK, "turn", TASK_NORMAL, LAG_3, POSITION_STANDING, MANA_0, LIFEFORCE_0, PRAY_0, 0, SYMBOL_STRESS_0, "", "", "", "", START_86, LEARN_5, START_DO_1, LEARN_DO_1, START_DO_NO, LEARN_DO_NO, LEARN_DIFF_SKILLS, 0.0, 0, 0);


  // shaman armadillo

  discArray[SPELL_EARTHMAW] = new spellInfo(SPELL_SHAMAN, DISC_SHAMAN_ARMADILLO, DISC_SHAMAN_ARMADILLO, "earthmaw", TASK_NORMAL, LAG_3, POSITION_SITTING, MANA_10, LIFEFORCE_0, PRAY_0, TAR_CHAR_ROOM | TAR_SELF_NONO | TAR_FIGHT_VICT | TAR_VIOLENT, SYMBOL_STRESS_0, "", "", "", "", START_41, LEARN_4, START_DO_40, LEARN_DO_5, START_DO_NO, LEARN_DO_NO, LEARN_DIFF_SPELLS, 0.04, COMP_GESTURAL | COMP_VERBAL | COMP_MATERIAL, 0);

  discArray[SPELL_CELERITE] = new spellInfo(SPELL_SHAMAN, DISC_SHAMAN_ARMADILLO, DISC_SHAMAN_ARMADILLO, "celerite", TASK_NORMAL, LAG_2, POSITION_CRAWLING, MANA_0, LIFEFORCE_260, PRAY_0, TAR_SELF_ONLY | TAR_FIGHT_SELF | TAR_CHAR_ROOM, SYMBOL_STRESS_0, "You dont seem to be as protected as you once were.", "$n seems to be slowing down.", "", "", START_30, LEARN_5, START_DO_40, LEARN_DO_5, START_DO_NO, LEARN_DO_NO, LEARN_DIFF_SPELLS, 0.04, COMP_GESTURAL | COMP_GESTURAL_RANDOM | COMP_VERBAL | COMP_VERBAL_RANDOM | COMP_MATERIAL | COMP_MATERIAL_END | SPELL_TASKED, 0);

  discArray[SPELL_AQUALUNG] = new spellInfo(SPELL_SHAMAN, DISC_SHAMAN_ARMADILLO, DISC_SHAMAN_ARMADILLO, "aqualung", TASK_EASY, LAG_2, POSITION_CRAWLING, MANA_0, LIFEFORCE_100, PRAY_0, TAR_CHAR_ROOM | TAR_FIGHT_SELF, SYMBOL_STRESS_0, "The transparent globe around your head disappears.", "The transparent globe around $n's head disappears.", "You begin to wonder how much longer you can breathe water.", "", START_1, LEARN_13, START_DO_45, LEARN_DO_5, START_DO_NO, LEARN_DO_NO, LEARN_DIFF_SPELLS, 0.02, COMP_GESTURAL | COMP_GESTURAL_RANDOM | COMP_VERBAL | COMP_VERBAL_RANDOM | COMP_MATERIAL | COMP_MATERIAL_END | SPELL_TASKED, 0);

  discArray[SPELL_THORNFLESH] = new spellInfo(SPELL_SHAMAN, DISC_SHAMAN_ARMADILLO, DISC_SHAMAN_ARMADILLO, "thornflesh", TASK_DIFFICULT, LAG_3, POSITION_CRAWLING, MANA_0, LIFEFORCE_300, PRAY_0, TAR_SELF_ONLY | TAR_FIGHT_SELF | TAR_CHAR_ROOM, SYMBOL_STRESS_0, "The thorns on your body fade away.", "The thorns on $n's body start to fade.", "The thorns on your body start to fade.", "", START_40, LEARN_4, START_DO_45, LEARN_DO_2, START_DO_NO, LEARN_DO_NO, LEARN_DIFF_SPELLS, 0.02, COMP_GESTURAL | COMP_GESTURAL_RANDOM | COMP_VERBAL | COMP_VERBAL_RANDOM | COMP_MATERIAL | COMP_MATERIAL_END | SPELL_TASKED, 0);

  discArray[SPELL_SHADOW_WALK] = new spellInfo(SPELL_SHAMAN, DISC_SHAMAN_ARMADILLO, DISC_SHAMAN_ARMADILLO, "shadow walk", TASK_DIFFICULT, LAG_2, POSITION_CRAWLING, MANA_0, LIFEFORCE_300, PRAY_0, TAR_CHAR_ROOM | TAR_SELF_ONLY, SYMBOL_STRESS_0, "You no longer walk in the shadows.", "$n blinks into view.", "", "", START_60, LEARN_8, START_DO_45, LEARN_DO_5, START_DO_NO, LEARN_DO_NO, LEARN_DIFF_SPELLS, 0.04, COMP_GESTURAL | COMP_GESTURAL_RANDOM | COMP_VERBAL | COMP_VERBAL_RANDOM | COMP_MATERIAL | COMP_MATERIAL_END | SPELL_TASKED, 0);

  // shaman alchemy

  discArray[SKILL_BREW] = new spellInfo(SPELL_SHAMAN, DISC_SHAMAN_ALCHEMY, DISC_SHAMAN_ALCHEMY, "brew", TASK_NORMAL, LAG_0, POSITION_STANDING, MANA_0, LIFEFORCE_0, PRAY_0, 0, SYMBOL_STRESS_0, "", "", "", "", START_1, LEARN_1, START_DO_1, LEARN_DO_5, START_DO_NO, LEARN_DO_NO, LEARN_DIFF_UNUSUAL, 0.0, 0, 0);

  // healing abilities
  discArray[SPELL_HEALING_GRASP] = new spellInfo(SPELL_SHAMAN, DISC_SHAMAN_HEALING, DISC_SHAMAN_HEALING, "healing grasp", TASK_DIFFICULT, LAG_2, POSITION_SITTING, MANA_0, LIFEFORCE_100, PRAY_0, TAR_CHAR_ROOM | TAR_FIGHT_SELF, SYMBOL_STRESS_0, "", "", "", "", START_1, LEARN_1, START_DO_35, LEARN_DO_5, START_DO_NO, LEARN_DO_NO, LEARN_DIFF_SPELLS, 0.02, COMP_GESTURAL | COMP_GESTURAL_RANDOM | COMP_VERBAL | COMP_VERBAL_RANDOM | COMP_MATERIAL | COMP_MATERIAL_END | SPELL_TASKED | SPELL_TASKED_EVERY, 0);

  discArray[SPELL_ENLIVEN] = new spellInfo(SPELL_SHAMAN, DISC_SHAMAN_HEALING, DISC_SHAMAN_HEALING, "enliven", TASK_DIFFICULT, LAG_2, POSITION_CRAWLING, MANA_0, LIFEFORCE_150, PRAY_0, TAR_CHAR_ROOM | TAR_FIGHT_SELF, SYMBOL_STRESS_0, "You don't seem as lively.", "$n doesn't seem as lively.", "", "", START_1, LEARN_17, START_DO_50, LEARN_DO_5, START_DO_NO, LEARN_DO_NO, LEARN_DIFF_SPELLS, 0.04, COMP_GESTURAL | COMP_GESTURAL_RANDOM | COMP_VERBAL | COMP_VERBAL_RANDOM | COMP_MATERIAL | COMP_MATERIAL_END | SPELL_TASKED, 0);


  // SHAMAN END HERE ***********************************************************************************




  // disc_piety

  discArray[SKILL_DEVOTION] = new spellInfo(SKILL_CLERIC_TYPES, DISC_FAITH, DISC_FAITH, "devotion", TASK_NORMAL, LAG_0, POSITION_RESTING, MANA_0, LIFEFORCE_0, PRAY_0, 0, SYMBOL_STRESS_0, "", "", "", "", START_1, LEARN_1, START_DO_NO, LEARN_DO_NO, START_DO_NO, LEARN_DO_NO, LEARN_DIFF_UNUSUAL, 0.0, 0, 0);

// disc_theology

  discArray[SKILL_PENANCE] = new spellInfo(SKILL_CLERIC_TYPES, DISC_THEOLOGY, DISC_THEOLOGY, "penance", TASK_NORMAL, LAG_0, POSITION_RESTING, MANA_0, LIFEFORCE_0, PRAY_0, 0, SYMBOL_STRESS_0, "", "", "", "", START_1, LEARN_1, START_DO_1, LEARN_DO_1, START_DO_NO, LEARN_DO_NO, LEARN_DIFF_UNUSUAL, 0.0, SPELL_IGNORE_POSITION, 0);


  // disc_slash

  discArray[SKILL_SLASH_SPEC] = new spellInfo(SKILL_WARRIOR, DISC_SLASH, DISC_SLASH, "slash specialization", TASK_NORMAL, LAG_0, POSITION_DEAD, MANA_0, LIFEFORCE_0, PRAY_0, 0, SYMBOL_STRESS_0, "", "", "", "", START_1, LEARN_1, START_DO_1, LEARN_DO_1, START_DO_NO, LEARN_DO_NO, LEARN_DIFF_UNUSUAL, 0.0, 0, 0);


  // disc_blunt

  discArray[SKILL_BLUNT_SPEC] = new spellInfo(SKILL_WARRIOR, DISC_BLUNT, DISC_BLUNT, "blunt specialization", TASK_NORMAL, LAG_0, POSITION_DEAD, MANA_0, LIFEFORCE_0, PRAY_0, 0, SYMBOL_STRESS_0, "", "", "", "", START_1, LEARN_1, START_DO_1, LEARN_DO_1, START_DO_NO, LEARN_DO_NO, LEARN_DIFF_UNUSUAL, 0.0, 0, 0);

  // disc_pierce

  discArray[SKILL_PIERCE_SPEC] = new spellInfo(SKILL_WARRIOR, DISC_PIERCE, DISC_PIERCE, "pierce specialization", TASK_NORMAL, LAG_0, POSITION_DEAD, MANA_0, LIFEFORCE_0, PRAY_0, 0, SYMBOL_STRESS_0, "", "", "", "", START_1, LEARN_1, START_DO_1, LEARN_DO_1, START_DO_NO, LEARN_DO_NO, LEARN_DIFF_UNUSUAL, 0.0, 0, 0);

  // disc_ranged

  discArray[SKILL_RANGED_SPEC] = new spellInfo(SKILL_WARRIOR, DISC_RANGED, DISC_RANGED, "ranged specialization", TASK_NORMAL, LAG_0, POSITION_DEAD, MANA_0, LIFEFORCE_0, PRAY_0, 0, SYMBOL_STRESS_0, "", "", "", "", START_1, LEARN_1, START_DO_1, LEARN_DO_2, START_DO_NO, LEARN_DO_NO, LEARN_DIFF_UNUSUAL, 0.0, 0, 0);

  discArray[SKILL_FAST_LOAD] = new spellInfo(SKILL_WARRIOR, DISC_RANGED, DISC_RANGED, "fast load", TASK_NORMAL, LAG_0, POSITION_DEAD, MANA_0, LIFEFORCE_0, PRAY_0, 0, SYMBOL_STRESS_0, "", "", "", "", START_1, LEARN_2, START_DO_1, LEARN_DO_4, START_DO_NO, LEARN_DO_NO, LEARN_DIFF_UNUSUAL, 0.0, 0, 0);

  // disc_barehand

  discArray[SKILL_BAREHAND_SPEC] = new spellInfo(SKILL_MONK, DISC_BAREHAND, DISC_BAREHAND, "barehand specialization", TASK_NORMAL, LAG_0, POSITION_DEAD, MANA_0, LIFEFORCE_0, PRAY_0, 0, SYMBOL_STRESS_0, "", "", "", "", START_1, LEARN_1, START_DO_1, LEARN_DO_1, START_DO_NO, LEARN_DO_NO, LEARN_DIFF_UNUSUAL, 0.0, 0, 0);

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
discArray[SKILL_PSITELEPATHY] = new spellInfo(SKILL_GENERAL, DISC_PSIONICS, DISC_PSIONICS, "psionic telepathy", TASK_EASY, LAG_0, POSITION_STANDING, MANA_10, LIFEFORCE_0, PRAY_0, 0, SYMBOL_STRESS_0, "", "", "", "", START_1, LEARN_5, START_DO_1, LEARN_DO_1, START_DO_NO, LEARN_DO_NO, LEARN_DIFF_SKILLS, 0.0, 0, 0);

discArray[SKILL_TELE_SIGHT] = new spellInfo(SKILL_GENERAL, DISC_PSIONICS, DISC_PSIONICS, "telepathic sight", TASK_NORMAL, LAG_0, POSITION_STANDING, MANA_0, LIFEFORCE_0, PRAY_0, 0, SYMBOL_STRESS_0, "", "", "", "", START_10, LEARN_100, START_DO_NO, LEARN_DO_NO, START_DO_NO, LEARN_DO_NO, LEARN_DIFF_SKILLS, 0.0, 0, 0);

discArray[SKILL_TELE_VISION] = new spellInfo(SKILL_GENERAL, DISC_PSIONICS, DISC_PSIONICS, "telepathic vision", TASK_NORMAL, LAG_3, POSITION_STANDING, MANA_40, LIFEFORCE_0, PRAY_0, 0, SYMBOL_STRESS_0, "", "", "", "", START_70, LEARN_4, START_DO_1, LEARN_DO_1, START_DO_NO, LEARN_DO_NO, LEARN_DIFF_SKILLS, 0.0, 0, 0);

discArray[SKILL_MIND_FOCUS] = new spellInfo(SKILL_GENERAL, DISC_PSIONICS, DISC_PSIONICS, "mind focus", TASK_NORMAL, LAG_1, POSITION_STANDING, MANA_50, LIFEFORCE_0, PRAY_0, 0, SYMBOL_STRESS_0, "", "", "", "", START_25, LEARN_2, START_DO_1, LEARN_DO_1, START_DO_NO, LEARN_DO_NO, LEARN_DIFF_SKILLS, 0.0, 0, 0);

discArray[SKILL_PSI_BLAST] = new spellInfo(SKILL_GENERAL, DISC_PSIONICS, DISC_PSIONICS, "psionic blast", TASK_NORMAL, LAG_3, POSITION_STANDING, MANA_20, LIFEFORCE_0, PRAY_0, 0, SYMBOL_STRESS_0, "", "", "", "", START_20, LEARN_2, START_DO_1, LEARN_DO_1, START_DO_NO, LEARN_DO_NO, LEARN_DIFF_SKILLS, 0.0, 0, 0);

discArray[SKILL_MIND_THRUST] = new spellInfo(SKILL_GENERAL, DISC_PSIONICS, DISC_PSIONICS, "mind thrust", TASK_NORMAL, LAG_3, POSITION_STANDING, MANA_25, LIFEFORCE_0, PRAY_0, 0, SYMBOL_STRESS_0, "", "", "", "", START_40, LEARN_2, START_DO_1, LEARN_DO_1, START_DO_NO, LEARN_DO_NO, LEARN_DIFF_SKILLS, 0.0, 0, 0);

discArray[SKILL_PSYCHIC_CRUSH] = new spellInfo(SKILL_GENERAL, DISC_PSIONICS, DISC_PSIONICS, "psychic crush", TASK_NORMAL, LAG_3, POSITION_STANDING, MANA_30, LIFEFORCE_0, PRAY_0, 0, SYMBOL_STRESS_0, "", "", "", "", START_60, LEARN_3, START_DO_1, LEARN_DO_1, START_DO_NO, LEARN_DO_NO, LEARN_DIFF_SKILLS, 0.0, 0, 0);

discArray[SKILL_KINETIC_WAVE] = new spellInfo(SKILL_GENERAL, DISC_PSIONICS, DISC_PSIONICS, "kinetic wave", TASK_NORMAL, LAG_3, POSITION_STANDING, MANA_40, LIFEFORCE_0, PRAY_0, 0, SYMBOL_STRESS_0, "", "", "", "", START_80, LEARN_5, START_DO_1, LEARN_DO_1, START_DO_NO, LEARN_DO_NO, LEARN_DIFF_SKILLS, 0.0, 0, 0);

discArray[SKILL_MIND_PRESERVATION] = new spellInfo(SKILL_GENERAL, DISC_PSIONICS, DISC_PSIONICS, "mind preservation", TASK_NORMAL, LAG_0, POSITION_STANDING, MANA_0, LIFEFORCE_0, PRAY_0, 0, SYMBOL_STRESS_0, "", "", "", "", START_1, LEARN_1, START_DO_NO, LEARN_DO_NO, START_DO_NO, LEARN_DO_NO, LEARN_DIFF_SKILLS, 0.0, 0, 0);

discArray[SKILL_TELEKINESIS] = new spellInfo(SKILL_GENERAL, DISC_PSIONICS, DISC_PSIONICS, "telekinesis", TASK_NORMAL, LAG_1, POSITION_STANDING, MANA_0, LIFEFORCE_0, PRAY_0, 0, SYMBOL_STRESS_0, "", "", "", "", START_1, LEARN_1, START_DO_1, LEARN_DO_1, START_DO_NO, LEARN_DO_NO, LEARN_DIFF_SKILLS, 0.0, 0, 0);

 discArray[SKILL_PSIDRAIN] = new spellInfo(SKILL_GENERAL, DISC_PSIONICS, DISC_PSIONICS, "psionic drain", TASK_DIFFICULT, LAG_1, POSITION_STANDING, MANA_0, LIFEFORCE_0, PRAY_0, 0, SYMBOL_STRESS_0, "","","","", START_1, LEARN_1, START_DO_1, LEARN_DO_1, START_DO_NO, LEARN_DO_NO, LEARN_DIFF_SKILLS, 0.0, 0, 0);


// end psionics


  // last_discipline

  discArray[MAX_SKILL] = new spellInfo(SPELL_NOCLASS, DISC_NONE, DISC_NONE, "\n", TASK_NORMAL, LAG_0, POSITION_DEAD, MANA_0, LIFEFORCE_0, PRAY_0, 0, SYMBOL_STRESS_0, "", "", "", "", START_0, LEARN_0, START_DO_NO, LEARN_DO_NO, START_DO_NO, LEARN_DO_NO, 0, 0.0, 0, 0);
}






