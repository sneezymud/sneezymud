//////////////////////////////////////////////////////////////////////////
//
// SneezyMUD - All rights reserved, SneezyMUD Coding Team
//
//////////////////////////////////////////////////////////////////////////

#include "extern.h"
#include "being.h"
#include "statistics.h"

#define SKILL_DAMAGE_DEBUG 0

double getSkillDiffModifier(spellNumT skill) {
  int amt = 0;
  switch (discArray[skill]->task) {
    case TASK_TRIVIAL:
      amt = 110;
      break;
    case TASK_EASY:
      amt = 100;
      break;
    case TASK_NORMAL:
      amt = 90;
      break;
    case TASK_DIFFICULT:
      amt = 80;
      break;
    case TASK_DANGEROUS:
      amt = 70;
      break;
    case TASK_HOPELESS:
      amt = 50;
      break;
    case TASK_IMPOSSIBLE:
      amt = 35;
      break;
  }
  return amt;
}

void getSkillLevelRange(spellNumT skill, int& min_lev, int& max_lev,
  int adv_learn) {
  if (discArray[skill]->disc != discArray[skill]->assDisc) {
    // skill is in a basic disc
    max_lev =
      ((100 / discArray[skill]->learn) + (discArray[skill]->start - 1)) * 30 /
      100;
    min_lev = discArray[skill]->start * 30 / 100;
  } else {
    max_lev =
      30 + (((100 / discArray[skill]->learn) + (discArray[skill]->start - 1)) *
             20 / 100);
    min_lev = 30 + (discArray[skill]->start * 20 / 100);
  }
  min_lev = max(min_lev, 1);
  max_lev = max(max_lev, 1);

  // a multiplier to boost the min/max lev based on specialization in skill
  float bump_lev = 1.0;
  if (discArray[skill]->disc != discArray[skill]->assDisc)
    bump_lev = 1.0 + (0.20 * adv_learn / 100);
  min_lev = (int)(min_lev * bump_lev);
  max_lev = (int)(max_lev * bump_lev);
}

enum reduceTypeT {
  REDUCE_NO = false,
  REDUCE_YES = true
};

enum trimTypeT {
  TRIM_NO = false,
  TRIM_YES = true
};

// victim = NULL for area effects
// class_amt is dam appropriate for the class on a per level, per round
// reduce indicates if a check on hit() has been done
//    physical attacks are probably !reduce, spells are reduce
// npc tells whether the caster is an NPC or not
// trim is for spells that are cast on PCs rather than NPCs in general
//    e.g. heals.
static int genericDam(const TBeing* victim, const TBeing* caster,
  spellNumT skill, discNumT basic_disc, int level, int adv_learn,
  float class_amt, reduceTypeT reduce, bool npc, trimTypeT trim) {
  int dam;

  // Note on level: set the max level for any skill equiv to where the skill
  // reaches max learnedness.  We will assume that 100% in basic disc is
  // reached at L30, and 100% in special disc is L50.  That is, if a skill
  // is start=0, learn=2 (maxing at 50%) we would set the max level either
  // to L15 or L40 depending on whether it was a basic or a special.
  //
  // However, if they have specialized, we will assume the min level is 30
  // and scales up to 50 based on learning in the specialized disc.
  int max_lev;
  int min_lev;

  // sanity test
  if (discArray[skill]->disc == discArray[skill]->assDisc) {
    // specialized disc
    //  if (discArray[skill]->disc == basic_disc)
    //    vlogf(LOG_BUG, format("bad setup for skill %d wrt disc arrangement
    //    (1)") %  skill);
  } else {
    // basic disc
    if (discArray[skill]->disc != basic_disc)
      vlogf(LOG_BUG,
        format("bad setup for skill %d wrt disc arrangement (2)") % skill);
  }

  getSkillLevelRange(skill, min_lev, max_lev, adv_learn);

  level = max(min(level, max_lev), min_lev);

  int lagamt = discArray[skill]->lag;
  if (discArray[skill]->comp_types & SPELL_TASKED) {
    // tasked spells take an extra round, so account for this
    lagamt++;

    // tasked spells also do damage at back end of cast generally
    // this is sort of a penalty, so lets boost the effects because
    // of it
    class_amt *= 1.1;
  }

  float fixed_amt = (class_amt * lagamt * level);

  // MOBs:
  // PCs are doing 1.75 dam per lev per round, but mobs are only doing 0.9
  // If the dam from the skills are the same for each, this would be a
  // much bigger percentage for the mobs (bad!).
  // so, for mobs, throw in a scale factor
  // totally arbitrary, and unrealistic, but necessary for balance

  // Adjust NPC damage/healing
  if (npc && ((victim && victim->isPc()) || !victim)) {
    fixed_amt *= 0.5195;
    if (reduce == REDUCE_YES)
      fixed_amt *= 0.5;
  }

  // Adjust healing for PCs
  if (!npc && trim)
    fixed_amt *= 0.5195;

  // Obviously, we should tweak dam up/down based on how successful the
  // skill is. We increase damage for skills as the failure rate increases
  // to ensure those skills are worth using.
  // factoring in once evenly weights easy vs hard skills in terms of
  // resulting damage.
  // -- Adjusting to increase damage to difficulty ration
  fixed_amt *= (100.0 / (getSkillDiffModifier(skill) - 15));

  // cut area effects in half
  if (IS_SET(discArray[skill]->targets, TAR_AREA))
    fixed_amt *= 0.75;

  // only for PC hitting NPC case
  if (!npc && victim && !victim->isPc())
    fixed_amt *= balanceCorrectionForLevel(level);

  // use a randomizer that avgs to L/4
  fixed_amt -= level / 4.0;
  dam = (int)(fixed_amt + ::number(1, level / 2));

  // adjust for stats
  if (discArray[skill]->modifierStat <= MAX_STATS_USED) {
    dam *= caster->plotStat(STAT_CURRENT, discArray[skill]->modifierStat, 0.8,
      1.25, 1.0);
  }

  // adjust for global values
  if (npc)
    dam = (int)(dam * stats.npc_skill_damage_mod);
  else
    dam = (int)(dam * stats.skill_damage_mod);

  dam = max(1, dam);

  if (!npc && ((!trim && ((victim && !victim->isPc()) || !victim)) || trim)) {
    discArray[skill]->pot_victims++;
    discArray[skill]->pot_damage += dam;
    discArray[skill]->pot_level += level;
  }

  return dam;
}

// area effect will send victim as NULL
int TBeing::getSkillDam(const TBeing* victim, spellNumT skill, int level,
  int adv_learn) const {
  // this is a centralized repository of damage formulas.
  // Yes, they could be in each of the skill/spells, but putting them here
  // gives us some ability to see the big picture...
  int dam;

  // look at balance notes to see how these numbers were arrived at
  // realize they are NOT arbitrary, and should not be toyed with
  // mage    = 2.050
  // cleric  = 1.667
  // warrior = 0.200
  // thief   = 1.033
  // deikhan = 0.639
  // monk    = 0.683
  // ranger  = 0.529
  // shaman  = 2.150

  // some global modifications
  // saving throw: c.f. balance.
  // a 50% chance of taking half damage (save).
  // raise damage by 4/3 to bring damage up to par.
  const double HAS_SAVING_THROW = 4.0 / 2.0;
  // component
  // components that load naturally, or via dissect, are harder to find
  // then the more generic ones.  Make the use of these spells worthwhile
  const double HARD_TO_FIND_COMPONENT = 1.5;  // arbitrary

  // multiplier for spells that must be used outdoors
  const double OUTDOOR_ONLY = 1.5;

  // multiplier for spells that need weather condition
  // since these mostly have the outdoor-only too, don't make these obscene
  const double NEED_RAIN_SNOW_LIGHTNING = 1.5;
  const double NEED_RAIN_LIGHTNING = 1.5;

  switch (skill) {
    case SKILL_SLAM:
    case SKILL_BASH:
    case SKILL_HEADBUTT:
    case SKILL_KNEESTRIKE:
    case SKILL_STOMP:
    case SKILL_BODYSLAM:
    case SKILL_SPIN:
      // other: bash lag is handled based on this is bash
      dam = genericDam(victim, this, skill, DISC_WARRIOR, level, adv_learn,
        0.50, REDUCE_NO, !isPc(), TRIM_NO);
      break;
    case SKILL_DEATHSTROKE:
      // damage scaling higher the lower the victim's hp %
      dam = genericDam(victim, this, skill, DISC_WARRIOR, level, adv_learn,
        2.5 - 2.0 * ((double)victim->getHit() / (double)victim->hitLimit()),
        REDUCE_NO, !isPc(), TRIM_NO);
      break;
    case SKILL_WHIRLWIND:
      dam = genericDam(victim, this, skill, DISC_WARRIOR, level, adv_learn, 4.0,
        REDUCE_NO, !isPc(), TRIM_NO);
      break;
    case SPELL_SAND_BLAST:
    case SPELL_HELLFIRE:
    case SPELL_ENERGY_DRAIN:
      dam = genericDam(victim, this, skill, DISC_MAGE, level, adv_learn,
        2 * HARD_TO_FIND_COMPONENT, REDUCE_YES, !isPc(), TRIM_NO);
      break;
    case SPELL_DUST_STORM:
    case SPELL_PEBBLE_SPRAY:
    case SPELL_LAVA_STREAM:
      dam = genericDam(victim, this, skill, DISC_MAGE, level, adv_learn, 2.5,
        REDUCE_YES, !isPc(), TRIM_NO);
      break;
    case SPELL_TORNADO:
      dam = genericDam(victim, this, skill, DISC_MAGE, level, adv_learn,
        2.5 * OUTDOOR_ONLY, REDUCE_YES, !isPc(), TRIM_NO);
      break;
    case SPELL_COLOR_SPRAY:
    case SPELL_ACID_BLAST:
    case SPELL_FLAMING_SWORD:
    case SPELL_STUNNING_ARROW:
      // for normal success, these spells provide a "save" that cuts dam in
      // half.  That is, 50% chance of dam in half.  The average would be 75%
      // hence we multiply by 4/3 to get the desired result
      // damage also increased due to difficulty in obtaining component
      dam = genericDam(victim, this, skill, DISC_MAGE, level, adv_learn,
        2.05 * HARD_TO_FIND_COMPONENT * HAS_SAVING_THROW, REDUCE_YES, !isPc(),
        TRIM_NO);
      break;
    case SPELL_ATOMIZE:
    case SPELL_BLAST_OF_FURY:
    case SPELL_MYSTIC_DARTS:
    case SPELL_GUSHER:
    case SPELL_TSUNAMI:
    case SPELL_ICE_STORM:
    case SPELL_GUST:
    case SPELL_ARCTIC_BLAST:
    case SPELL_ICY_GRIP:
    case SPELL_FIREBALL:
    case SPELL_LAVA_LANCE:
    case SPELL_INFERNO:
    case SPELL_HANDS_OF_FLAME:
    case SPELL_SLING_SHOT:
    case SPELL_GRANITE_FISTS:
      dam = genericDam(victim, this, skill, DISC_MAGE, level, adv_learn,
        2.0 * HAS_SAVING_THROW, REDUCE_YES, !isPc(), TRIM_NO);
      break;
    case SPELL_METEOR_SWARM:
      dam = genericDam(victim, this, skill, DISC_MAGE, level, adv_learn,
        2.0 * HAS_SAVING_THROW * OUTDOOR_ONLY, REDUCE_YES, !isPc(), TRIM_NO);
      break;
    case SPELL_HARM:
    case SPELL_PILLAR_SALT:
    case SPELL_RAIN_BRIMSTONE:
    case SPELL_EARTHQUAKE:
      dam = genericDam(victim, this, skill, DISC_CLERIC, level, adv_learn, 3.0,
        REDUCE_YES, !isPc(), TRIM_NO);
      dam = (int)(dam * percModifier());
      break;
    case SPELL_SPONTANEOUS_COMBUST:
    case SPELL_FLAMESTRIKE:
      dam = genericDam(victim, this, skill, DISC_CLERIC, level, adv_learn,
        2.5 * HAS_SAVING_THROW, REDUCE_YES, !isPc(), TRIM_NO);

      // additionally, do faction percent modification for clerics
      dam = (int)(dam * percModifier());
      break;
    case SPELL_CALL_LIGHTNING:
      dam = genericDam(victim, this, skill, DISC_CLERIC, level, adv_learn,
        3.0 * HAS_SAVING_THROW * OUTDOOR_ONLY * NEED_RAIN_LIGHTNING, REDUCE_YES,
        !isPc(), TRIM_NO);
      // additionally, do faction percent modification for clerics
      dam = (int)(dam * percModifier());
      break;
      ////////////////////
      // SHAMAN STUFF
      ////////////////////
    case SPELL_STORMY_SKIES:
      dam = genericDam(victim, this, skill, DISC_SHAMAN, level, adv_learn,
        3.0 * HARD_TO_FIND_COMPONENT * NEED_RAIN_SNOW_LIGHTNING, REDUCE_YES,
        !isPc(), TRIM_NO);
      break;
    case SPELL_CARDIAC_STRESS:
    case SPELL_AQUATIC_BLAST:
    case SPELL_BLOOD_BOIL:
    case SPELL_DEATHWAVE:
    case SPELL_RAZE:
      dam = genericDam(victim, this, skill, DISC_SHAMAN, level, adv_learn,
        1.5 * HARD_TO_FIND_COMPONENT, REDUCE_YES, !isPc(), TRIM_NO);
      break;
    case SPELL_DISTORT:
    case SPELL_STICKS_TO_SNAKES:
    case SPELL_SOUL_TWIST:
    case SPELL_SQUISH:
    case SPELL_FLATULENCE:
    case SPELL_LIFE_LEECH:
      dam = genericDam(victim, this, skill, DISC_SHAMAN, level, adv_learn,
        2.150 * HAS_SAVING_THROW, REDUCE_YES, !isPc(), TRIM_NO);
      break;
    case SPELL_LICH_TOUCH:
    case SPELL_VAMPIRIC_TOUCH:
      dam = genericDam(victim, this, skill, DISC_SHAMAN, level, adv_learn,
        1.9 * HAS_SAVING_THROW, REDUCE_YES, !isPc(), TRIM_NO);
      break;
      ///////////////////////
      // END SHAMAN STUFF
      ///////////////////////
    case SPELL_HARM_LIGHT:
    case SPELL_HARM_SERIOUS:
    case SPELL_HARM_CRITICAL:
      // other: paralyze lag is based on this logic manually in paralyze
      dam = genericDam(victim, this, skill, DISC_CLERIC, level, adv_learn, 2.5,
        REDUCE_YES, !isPc(), TRIM_NO);
      // additionally, do faction percent modification for clerics
      dam = (int)(dam * percModifier());
      break;
    case SPELL_BONE_BREAKER:
    case SPELL_PARALYZE_LIMB:
    case SPELL_NUMB:
    case SPELL_WITHER_LIMB:
      // these are torments, this gets called for anti-salve stuff
      // divide by scale factor to keep under control as castable multiple times
      dam = genericDam(victim, this, skill, DISC_CLERIC, level, adv_learn,
        1.667, REDUCE_YES, !isPc(), TRIM_NO);
      break;
    case SPELL_HEAL_LIGHT:
    case SPELL_HEAL_SERIOUS:
    case SPELL_HEAL_CRITICAL:
    case SPELL_HEAL:
    case SPELL_HEAL_FULL:
    case SPELL_HEAL_CRITICAL_SPRAY:
    case SPELL_HEAL_SPRAY:
    case SPELL_HEAL_FULL_SPRAY:
      // heal spells should NOT be reduced for casting over leve
      // also, lets let the modifier be 1.5* what it is for damage
      // however, in PC v PC case, do decrease the amount being healed
      dam = genericDam(victim, this, skill, DISC_CLERIC, level, adv_learn, 2.50,
        REDUCE_NO, !isPc(), TRIM_YES);

      // additionally, do faction percent modification for clerics
      dam = (int)(dam * percModifier());
      break;
    case SPELL_HEALING_GRASP:
      dam = genericDam(victim, this, skill, DISC_CLERIC, level, adv_learn, 2.50,
        REDUCE_NO, !isPc(), TRIM_YES);
      break;
    case SKILL_KICK:
    case SKILL_GARROTTE:
    case SKILL_STABBING:
      dam = genericDam(victim, this, skill, DISC_THIEF, level, adv_learn, 1.033,
        REDUCE_NO, !isPc(), TRIM_NO);
      break;
    // backstab has some limitations (sneak, opening only), so we allow it to
    // violate the rules slightly (arbitrary)
    case SKILL_BACKSTAB:
      dam = genericDam(victim, this, skill, DISC_THIEF, level, adv_learn, 2.00,
        REDUCE_NO, !isPc(), TRIM_NO);
      break;
      // made this slightly higher than backstab since it is in an advanced
      // discipline
    case SKILL_THROATSLIT:
      dam = genericDam(victim, this, skill, DISC_THIEF, level, adv_learn, 2.01,
        REDUCE_NO, !isPc(), TRIM_NO);
      break;
    case SKILL_BASH_DEIKHAN:
      dam = genericDam(victim, this, skill, DISC_DEIKHAN, level, adv_learn,
        0.639, REDUCE_NO, !isPc(), TRIM_NO);
      dam = (int)(dam * percModifier());
      break;
    case SKILL_CHARGE:
      // limited to mounted and has other penalties  (3*normal dam)
      dam = genericDam(victim, this, skill, DISC_DEIKHAN, level, adv_learn,
        0.9 * 3, REDUCE_NO, !isPc(), TRIM_NO);
      // additionally, do faction percent modification for deikhan
      dam = (int)(dam * percModifier());
      break;
    case SKILL_SMITE:
      // this is limited to once a day, and has limits from weapon-use to
      // so lets let it do a LOT of damage
      dam = genericDam(victim, this, skill, DISC_DEIKHAN, level, adv_learn,
        0.639 * 8, REDUCE_YES, !isPc(), TRIM_NO);
      // additionally, do faction percent modification for deikhan
      dam = (int)(dam * percModifier());
      break;
    case SPELL_HARM_DEIKHAN:
      // a 4/3 factor added for save cutting into overall damage
      dam = genericDam(victim, this, skill, DISC_DEIKHAN, level, adv_learn,
        0.639 * HAS_SAVING_THROW, REDUCE_YES, !isPc(), TRIM_NO);
      // additionally, do faction percent modification for clerics
      dam = (int)(dam * percModifier());
      break;
    case SPELL_HARM_LIGHT_DEIKHAN:
    case SPELL_HARM_SERIOUS_DEIKHAN:
    case SPELL_HARM_CRITICAL_DEIKHAN:
      dam = genericDam(victim, this, skill, DISC_DEIKHAN, level, adv_learn,
        0.639, REDUCE_YES, !isPc(), TRIM_NO);
      // additionally, do faction percent modification for clerics
      dam = (int)(dam * percModifier());
      break;
    case SPELL_NUMB_DEIKHAN:
      // these are torments, this gets called for anti-salve stuff
      // divide by scale factor to keep under control as castable multiple times
      dam = genericDam(victim, this, skill, DISC_DEIKHAN, level, adv_learn,
        0.639 / 5.0, REDUCE_YES, !isPc(), TRIM_NO);
      break;
    case SPELL_HEAL_LIGHT_DEIKHAN:
    case SPELL_HEAL_SERIOUS_DEIKHAN:
    case SPELL_HEAL_CRITICAL_DEIKHAN:
      // heal spells should NOT be reduced for casting over leve
      // also, lets let the modifier be 1.5* what it is for damage
      dam = genericDam(victim, this, skill, DISC_DEIKHAN, level, adv_learn,
        0.959, REDUCE_NO, !isPc(), TRIM_YES);
      // additionally, do faction percent modification for clerics
      dam = (int)(dam * percModifier());
      break;
    case SPELL_ROOT_CONTROL:
      // 4/3 factor added here due to save cutting into avg damage
      dam = genericDam(victim, this, skill, DISC_RANGER, level, adv_learn,
        0.529 * HAS_SAVING_THROW, REDUCE_YES, !isPc(), TRIM_NO);
      break;
    case SKILL_CHOP:
      dam = genericDam(victim, this, skill, DISC_MONK, level, adv_learn, 0.4,
        REDUCE_NO, !isPc(), TRIM_NO);
      break;
    case SKILL_KICK_MONK:
    case SKILL_HURL:
    case SKILL_BONEBREAK:
    case SKILL_DEFENESTRATE:
    case SKILL_SHOULDER_THROW:
      dam = genericDam(victim, this, skill, DISC_MONK, level, adv_learn, 0.233,
        REDUCE_NO, !isPc(), TRIM_NO);
      break;
    case SKILL_CHI:
      // there is no hits() check on this, so treat like a spell
      dam = genericDam(victim, this, skill, DISC_MONK, level, adv_learn, 0.45,
        REDUCE_YES, !isPc(), TRIM_NO);
      break;
    case SKILL_PSI_BLAST:
    case SKILL_MIND_THRUST:
    case SKILL_PSYCHIC_CRUSH:
    case SKILL_KINETIC_WAVE:
      dam = genericDam(victim, this, skill, DISC_PSIONICS, level, adv_learn,
        1.2, REDUCE_YES, !isPc(), TRIM_NO);
      break;
    case SPELL_SKY_SPIRIT:
    case SPELL_EARTHMAW:
      dam = genericDam(victim, this, skill, DISC_ANIMAL, level, adv_learn,
        2.5 * OUTDOOR_ONLY, REDUCE_YES, !isPc(), TRIM_NO);
      break;
    default:
      vlogf(LOG_BUG, format("Unknown skill %d in call to getSkillDam") % skill);
      dam = 0;
  }

#if SKILL_DAMAGE_DEBUG
  vlogf(LOG_BUG, format("SKILL DAMAGE CALCULATED: %s -> (%d) %s") %
                   discArray[skill]->name % dam % getName());
#endif

  return dam;
}
