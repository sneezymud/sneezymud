// Immunities.cc
//
// Class for Immunity Data.

#include "being.h"
#include "immunity.h"
#include "extern.h"

using std::min;
using std::max;

// Constructor.  Zero out everything.
Immunities::Immunities()
{
  for (immuneTypeT i=MIN_IMMUNE; i < MAX_IMMUNES; i++)
    ImmunityArray[i] = 0;
}

// convert() is a utility function to switch from const char *
// to immune_t so other functions can access the ImmunityArray.
immuneTypeT Immunities::convert(const sstring & immunity) const
{
  if (!immunity.compare("IMMUNE_HEAT"))
    return IMMUNE_HEAT;
  if (!immunity.compare("IMMUNE_COLD"))
    return IMMUNE_COLD;
  if (!immunity.compare("IMMUNE_ACID"))
    return IMMUNE_ACID;
  if (!immunity.compare("IMMUNE_POISON"))
    return IMMUNE_POISON;
  if (!immunity.compare("IMMUNE_SLEEP"))
    return IMMUNE_SLEEP;
  if (!immunity.compare("IMMUNE_PARALYSIS"))
    return IMMUNE_PARALYSIS;
  if (!immunity.compare("IMMUNE_CHARM"))
    return IMMUNE_CHARM;
  if (!immunity.compare("IMMUNE_PIERCE"))
    return IMMUNE_PIERCE;
  if (!immunity.compare("IMMUNE_SLASH"))
    return IMMUNE_SLASH;
  if (!immunity.compare("IMMUNE_BLUNT"))
    return IMMUNE_BLUNT;
  if (!immunity.compare("IMMUNE_NONMAGIC"))
    return IMMUNE_NONMAGIC;
  if (!immunity.compare("IMMUNE_PLUS1"))
    return IMMUNE_PLUS1;
  if (!immunity.compare("IMMUNE_PLUS2"))
    return IMMUNE_PLUS2;
  if (!immunity.compare("IMMUNE_PLUS3"))
    return IMMUNE_PLUS3;
  if (!immunity.compare("IMMUNE_AIR"))
    return IMMUNE_AIR;
  if (!immunity.compare("IMMUNE_ENERGY"))
    return IMMUNE_ENERGY;
  if (!immunity.compare("IMMUNE_ELECTRICITY"))
    return IMMUNE_ELECTRICITY;
  if (!immunity.compare("IMMUNE_DISEASE"))
    return IMMUNE_DISEASE;
  if (!immunity.compare("IMMUNE_SUFFOCATION"))
    return IMMUNE_SUFFOCATION;
  if (!immunity.compare("IMMUNE_SKIN_COND"))
    return IMMUNE_SKIN_COND;
  if (!immunity.compare("IMMUNE_BONE_COND"))
    return IMMUNE_BONE_COND;
  if (!immunity.compare("IMMUNE_BLEED"))
    return IMMUNE_BLEED;
  if (!immunity.compare("IMMUNE_WATER"))
    return IMMUNE_WATER;
  if (!immunity.compare("IMMUNE_DRAIN"))
    return IMMUNE_DRAIN;
  if (!immunity.compare("IMMUNE_FEAR"))
    return IMMUNE_FEAR;
  if (!immunity.compare("IMMUNE_EARTH"))
    return IMMUNE_EARTH;
  if (!immunity.compare("IMMUNE_SUMMON"))
    return IMMUNE_SUMMON;
  if (!immunity.compare("IMMUNE_UNUSED2"))
    return IMMUNE_UNUSED2;

  vlogf(LOG_BUG, format("Unknown immunity '%s', in convert()") %  immunity);
  return IMMUNE_NONE;
}

// setImmunity() assigns a percentage to a particular immunity.
void Immunities::setImmunity(const sstring &whichImmunity, short percent)
{
  immuneTypeT itt = convert(whichImmunity);
  ImmunityArray[itt] = percent;
}

// getImmunity() returns the value of the particular immunity.
short Immunities::getImmunity(immuneTypeT whichImmunity) const
{
  return ImmunityArray[whichImmunity];
}

short TBeing::getImmunity(immuneTypeT type) const
{
  int amount, imm;

  imm = immunities.immune_arr[type];

  if(doesKnowSkill(SKILL_DUFALI)) {
    amount = max((int)getSkillValue(SKILL_DUFALI), 0);
    switch(type){
      case IMMUNE_PARALYSIS: 
        imm += (amount/3);
        break;
      case IMMUNE_CHARM:
        imm += amount;
        break;
      case IMMUNE_POISON:
        imm += (amount/2);
        break;
      default:
        break;
    }
  }
  
  if(doesKnowSkill(SKILL_IRON_SKIN)){
    amount = max((int)getSkillValue(SKILL_IRON_SKIN), 0);
    switch(type){
      case IMMUNE_SKIN_COND:
        imm += (amount/2);
        break;
      case IMMUNE_BLEED:
        imm += (int)(amount/1.5);
        break;
      default:
        break;
    }
  }

  if(doesKnowSkill(SKILL_IRON_BONES) && type == IMMUNE_BONE_COND){
    amount = max((int)getSkillValue(SKILL_IRON_BONES), 0);
    imm += (amount/2);
  }

  if(doesKnowSkill(SKILL_IRON_WILL) && type == IMMUNE_NONMAGIC){
    amount = max((int)getSkillValue(SKILL_IRON_WILL), 0);
    imm += (amount/30);
  }

  if(hasQuestBit(TOG_IS_HEALTHY) && type == IMMUNE_DISEASE){
    imm += 75;
  }

  if(affectedBySpell(AFFECT_WET)) {
    amount = 0;
    for (affectedData *wetAffect = affected; wetAffect; wetAffect = wetAffect->next)
      if (wetAffect->type == AFFECT_WET) {
        amount = wetAffect->level;
        break;
      }
    switch(type){
      case IMMUNE_HEAT: 
        imm += (amount/3);
        break;
      case IMMUNE_ELECTRICITY:
        imm += -(amount/5);
        break;
      case IMMUNE_COLD:
        imm += -(amount/5);
        break;
      default:
        break;
    }
  }

  imm = max(-100, min(100, imm));
  return imm;
}

void TBeing::setImmunity(immuneTypeT type, short amt)
{
  immunities.immune_arr[type] = amt;
}

void TBeing::addToImmunity(immuneTypeT type, short amt)
{
  immunities.immune_arr[type] = min(max(immunities.immune_arr[type] + amt, -100), 100);
}

bool TBeing::isImmune(immuneTypeT bit, wearSlotT pos, int modifier) const
{
  // 'modifier' is not required and defaults to 0
  // 'modifier' is subtracted from any resistance less than 100%
  // this function history subtracted (modifier + 50) from the resistance
  //   this made absolutely no sense
  //   - Maror 02/2004
  int gi = getImmunity(bit);
  if (gi >= 100)
   return TRUE;
  if (gi <= -100)
    return FALSE;

  return (gi - modifier) > ::number(1,100);
}


immuneTypeT getTypeImmunity(spellNumT type)
{
  immuneTypeT bit = IMMUNE_NONE;

  switch (type) {
    case SPELL_BLOOD_BOIL:
    case DAMAGE_FIRE:
    case SPELL_FIREBALL:
    case SPELL_HANDS_OF_FLAME:
    case SPELL_FLAMESTRIKE:
    case SPELL_FIRE_BREATH:
    case SPELL_RAIN_BRIMSTONE:
    case SPELL_RAIN_BRIMSTONE_DEIKHAN:
    case SPELL_INFERNO:
    case SPELL_HELLFIRE:
    case SPELL_FLAMING_SWORD:
    case SPELL_SPONTANEOUS_COMBUST:
    case DAMAGE_TRAP_FIRE:
    case DAMAGE_TRAP_TNT:
    case TYPE_FIRE:
      bit = IMMUNE_HEAT;
      break;
//    case SPELL_LIGHTNING_BOLT:
    case SPELL_CALL_LIGHTNING_DEIKHAN:
    case SPELL_CALL_LIGHTNING:
    case SPELL_LIGHTNING_BREATH:
    case DAMAGE_ELECTRIC:
      bit = IMMUNE_ELECTRICITY;
      break;
    case DAMAGE_FROST:
    case SPELL_ICY_GRIP:
    case SPELL_ARCTIC_BLAST:
    case SPELL_ICE_STORM:
    case SPELL_FROST_BREATH:
    case DAMAGE_TRAP_FROST:
      bit = IMMUNE_COLD;
      break;
    case SKILL_CHI:
    case DAMAGE_DISRUPTION:
    case SPELL_MYSTIC_DARTS:
    case SPELL_STUNNING_ARROW:
    case SPELL_COLOR_SPRAY:
    case SPELL_BLAST_OF_FURY:
    case SPELL_ATOMIZE:
    case SPELL_RAZE:
    case SPELL_DISTORT:
    case SPELL_DEATHWAVE:
    case DAMAGE_TRAP_ENERGY:
      bit = IMMUNE_ENERGY;
      break;
    case SPELL_METEOR_SWARM:
    case SPELL_EARTHQUAKE_DEIKHAN:
    case SPELL_EARTHQUAKE:
    case SPELL_PEBBLE_SPRAY:
    case SPELL_SAND_BLAST:
    case SPELL_LAVA_STREAM:
    case SPELL_SLING_SHOT:
    case SPELL_GRANITE_FISTS:
    case SPELL_PILLAR_SALT:
    case DAMAGE_COLLISION:
    case DAMAGE_FALL:
    case TYPE_EARTH:
      bit = IMMUNE_EARTH;
      break;
    case DAMAGE_GUST:
    case SPELL_GUST:
    case SPELL_DUST_STORM:
    case SPELL_TORNADO:
    case TYPE_AIR:
    case SPELL_DUST_BREATH:
      bit = IMMUNE_AIR;
      break;
    case SPELL_VAMPIRIC_TOUCH:
    case SPELL_LIFE_LEECH:
    case SPELL_LICH_TOUCH:
    case SPELL_ENERGY_DRAIN:
    case DAMAGE_DRAIN:
    case SPELL_HARM_DEIKHAN:
    case SPELL_SOUL_TWIST:
    case SPELL_HARM:
    case SPELL_HARM_LIGHT_DEIKHAN:
    case SPELL_HARM_SERIOUS_DEIKHAN:
    case SPELL_HARM_CRITICAL_DEIKHAN:
    case SPELL_HARM_LIGHT:
    case SPELL_HARM_SERIOUS:
    case SPELL_HARM_CRITICAL:
    case SPELL_WITHER_LIMB:
      bit = IMMUNE_DRAIN;
      break;
    case DAMAGE_ACID:
    case SPELL_ACID_BREATH:
    case SPELL_ACID_BLAST:
    case DAMAGE_TRAP_ACID:
      bit = IMMUNE_ACID;
      break;
    case SKILL_BACKSTAB:
    case SKILL_THROATSLIT:
    case SKILL_STABBING:
    case TYPE_PIERCE:
    case TYPE_STING:
    case TYPE_STAB:
    case TYPE_THRUST:
    case TYPE_SPEAR:
    case TYPE_BEAK:
    case TYPE_SHOOT:
    case DAMAGE_TRAP_PIERCE:
    case DAMAGE_ARROWS:
      bit = IMMUNE_PIERCE;
      break;
    case TYPE_SLASH:
    case TYPE_SLICE:
    case TYPE_CLEAVE:
    case TYPE_CLAW:
    case TYPE_BEAR_CLAW:
    case DAMAGE_TRAP_SLASH:
    case TYPE_SHRED:
      bit = IMMUNE_SLASH;
      break;
    case TYPE_BLUDGEON:
    case TYPE_WHIP:
    case SKILL_CUDGEL:
    case SKILL_DOORBASH:
    case SKILL_SHOVE_DEIKHAN:
    case SKILL_SHOVE:
    case TYPE_HIT:
    case DAMAGE_KICK_HEAD:
    case DAMAGE_KICK_SIDE:
    case DAMAGE_KICK_SHIN:
    case DAMAGE_KICK_SOLAR:
    case SKILL_KICK_DEIKHAN:
    case SKILL_KICK_THIEF:
    case SKILL_KICK_MONK:
    case SKILL_KICK:
    case TYPE_KICK:
    case SKILL_CHOP:
    case TYPE_CRUSH:
    case TYPE_BITE:
    case TYPE_SMASH:
    case TYPE_FLAIL:
    case TYPE_PUMMEL:
    case TYPE_POUND:
    case TYPE_THRASH:
    case TYPE_THUMP:
    case TYPE_WALLOP:
    case TYPE_BATTER:
    case TYPE_BEAT:
    case TYPE_STRIKE:
    case TYPE_CLUB:
    case TYPE_SMITE:
    case TYPE_MAUL:
    case SKILL_BASH_DEIKHAN:
    case SKILL_BASH:
    case SKILL_BODYSLAM:
    case SKILL_SPIN:
    case DAMAGE_TRAP_BLUNT:
    case SKILL_TRIP:
    case TYPE_CANNON:
      bit = IMMUNE_BLUNT;
      break;
    case SPELL_POISON_DEIKHAN:
    case SPELL_POISON:
    case SPELL_CHLORINE_BREATH:
    case DAMAGE_TRAP_POISON:
      bit = IMMUNE_POISON;
      break;
    case SPELL_SLUMBER:
    case DAMAGE_TRAP_SLEEP:
      bit = IMMUNE_SLEEP;
      break;
    case SPELL_PARALYZE:
    case SPELL_PARALYZE_LIMB:
    case SPELL_BIND:
    case SPELL_IMMOBILIZE:
    case SPELL_NUMB_DEIKHAN:
    case SPELL_NUMB:
      bit = IMMUNE_PARALYSIS;
      break;
    case SPELL_ENSORCER:
    case SPELL_HYPNOSIS:
    case SPELL_CONTROL_UNDEAD:
    case SPELL_ANIMATE:
    case SPELL_VOODOO:
    case SPELL_DANCING_BONES:
    case SPELL_CONJURE_AIR:
    case SPELL_CONJURE_EARTH:
    case SPELL_CONJURE_WATER:
    case SPELL_CONJURE_FIRE:
    case SKILL_BEAST_SUMMON:
    case SKILL_BEAST_SOOTHER:
    case SKILL_TRANSFIX:
      bit = IMMUNE_CHARM;
      break;
    case SPELL_FLATULENCE:
    case DAMAGE_SUFFOCATION:
    case DAMAGE_DROWN:
    case SPELL_SUFFOCATE:
    case SKILL_GARROTTE:
      bit = IMMUNE_SUFFOCATION;
      break;
    case SPELL_SQUISH:
    case SPELL_BONE_BREAKER:
      bit = IMMUNE_BONE_COND;
      break;
    case SPELL_BLEED:
    case DAMAGE_HEMORRAGE:
      bit = IMMUNE_BLEED;
      break;
    case SPELL_TSUNAMI:
    case SPELL_WATERY_GRAVE:
    case DAMAGE_WHIRLPOOL:
    case SPELL_GUSHER:
    case SPELL_AQUATIC_BLAST:
    case TYPE_WATER:
      bit = IMMUNE_WATER;
      break;
    case SPELL_FEAR:
    case SPELL_INTIMIDATE:
      bit = IMMUNE_FEAR;
      break; 
    case SPELL_DISEASE:
    case SPELL_INFECT_DEIKHAN:
    case SPELL_INFECT:
    case SPELL_DEATH_MIST:
    case DAMAGE_TRAP_DISEASE:
      bit = IMMUNE_DISEASE;
      break;
    case -1:
      bit = IMMUNE_SKIN_COND;
      break;

    // check: should any of these skills have immunity types?
    case SPELL_CARDIAC_STRESS:
    case SPELL_FUMBLE:
    case SPELL_FAERIE_FIRE:
    case SPELL_STUPIDITY:
    case SPELL_FLAMING_FLESH:
    case SPELL_FLARE:
    case SPELL_SILENCE:
    case SKILL_BEFRIEND_BEAST:
    case SKILL_BEAST_CHARM:
    case SPELL_SHAPESHIFT:
    case SPELL_PLAGUE_LOCUSTS:
    case SPELL_ROOT_CONTROL:
    case SPELL_STICKS_TO_SNAKES:
    case SPELL_LIVING_VINES:
    case SPELL_STORMY_SKIES:
    case SPELL_PLASMA_MIRROR:
    case SPELL_THORNFLESH:
    case SKILL_SUBTERFUGE:
#if 1
    case SPELL_EARTHMAW:
    case SPELL_CREEPING_DOOM:
    case SPELL_FERAL_WRATH:
    case SPELL_SKY_SPIRIT:
#endif
    default:
      bit = IMMUNE_NONE;
      break;
  }
  return bit;
}


