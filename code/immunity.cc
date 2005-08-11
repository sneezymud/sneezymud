// Immunities.cc
//
// Class for Immunity Data.

#include "stdsneezy.h"
#include "immunity.h"

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

  vlogf(LOG_BUG, fmt("Unknown immunity '%s', in convert()") %  immunity);
  return IMMUNE_NONE;
}

// setImmunity() assigns a percentage to a particular immunity.
void Immunities::setImmunity(const sstring &whichImmunity, byte percent)
{
  immuneTypeT itt = convert(whichImmunity);
  ImmunityArray[itt] = percent;
}

// getImmunity() returns the value of the particular immunity.
byte Immunities::getImmunity(immuneTypeT whichImmunity) const
{
  return ImmunityArray[whichImmunity];
}

byte TBeing::getImmunity(immuneTypeT type) const
{
  int amount, imm=0;

  if(doesKnowSkill(SKILL_DUFALI)) {
    amount = getSkillValue(SKILL_DUFALI);
    amount = max(amount, 0);
    switch(type){
      case IMMUNE_PARALYSIS: 
	imm=immunities.immune_arr[type]+(amount/3);
	break;
      case IMMUNE_CHARM:
	imm=immunities.immune_arr[type]+amount;
	break;
      case IMMUNE_POISON:
	imm=immunities.immune_arr[type]+(amount/2);
	break;
      default:
	break;
    }
    
    if(imm){
      if (imm < -100)
	imm=-100;
      if (imm > 100)
	imm=100;
      
      return imm;
    }
  }
  
  if(doesKnowSkill(SKILL_IRON_SKIN)){
    amount = getSkillValue(SKILL_IRON_SKIN);
    amount = max(amount, 0);
    
    switch(type){
      case IMMUNE_SKIN_COND:
	imm=immunities.immune_arr[type]+(amount/2);
	break;
      case IMMUNE_BLEED:
	imm=immunities.immune_arr[type]+(int)(amount/1.5);
	break;
      default:
	break;
    }
    
    if(imm){
      if (imm < -100)
	imm=-100;
      if (imm > 100)
	imm=100;
      
      return imm;
    }
  }
  if(doesKnowSkill(SKILL_IRON_BONES)){
    amount = getSkillValue(SKILL_IRON_BONES);
    amount = max(amount, 0);
    
    switch(type){
      case IMMUNE_BONE_COND:
	imm=immunities.immune_arr[type]+(amount/2);
	break;
      default:
	break;
    }
    
    if(imm){
      if (imm < -100)
	imm=-100;
      if (imm > 100)
	imm=100;
      
      return imm;
    }
  }
  if(doesKnowSkill(SKILL_IRON_WILL)){
    amount = getSkillValue(SKILL_IRON_WILL);
    amount = max(amount, 0);
    
    switch(type){
      case IMMUNE_NONMAGIC:
	imm=immunities.immune_arr[type]+(amount/30);
	break;
      default:
	break;
    }
    
    if(imm){
      if (imm < -100)
	imm=-100;
      if (imm > 100)
	imm=100;
      
      return imm;
    }
  }
  if(hasQuestBit(TOG_IS_HEALTHY) && type==IMMUNE_DISEASE){
    imm=immunities.immune_arr[type]+75;
    if(imm){
      if (imm < -100)
	imm=-100;
      if (imm > 100)
	imm=100;
      
      return imm;
    }
  }

  return immunities.immune_arr[type];
}

void TBeing::setImmunity(immuneTypeT type, byte amt)
{
  immunities.immune_arr[type] = amt;
}

void TBeing::addToImmunity(immuneTypeT type, byte amt)
{
  immunities.immune_arr[type] = min(max(immunities.immune_arr[type] + amt, -100), 100);
}

bool TBeing::isImmune(immuneTypeT bit, int modifier) const
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
      bit = IMMUNE_BLUNT;
      break;
    case SPELL_POISON_DEIKHAN:
    case SPELL_POISON:
    case SPELL_DEATH_MIST:
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
    case DAMAGE_TRAP_DISEASE:
      bit = IMMUNE_DISEASE;
      break;
    case -1:
      bit = IMMUNE_SKIN_COND;
      break;
    case SKILL_POWERMOVE:
    case SKILL_PARRY_WARRIOR:
    case SKILL_QUIV_PALM:
    case SKILL_SPRINGLEAP:
    case SKILL_SMITE:
    case SPELL_FUMBLE:
    case SKILL_HEADBUTT:
    case SKILL_KNEESTRIKE:
    case SKILL_BRAWL_AVOIDANCE:
    case SKILL_STOMP:
    case SPELL_BLINDNESS:
    case SPELL_CURSE_DEIKHAN:
    case SPELL_CURSE:
    case SPELL_PORTAL:
    case SPELL_RESURRECTION:
    case SPELL_ENHANCE_WEAPON:
    case SPELL_TELEPORT:
    case SPELL_KNOT:
    case DAMAGE_TRAP_TELEPORT:
    case SPELL_FEATHERY_DESCENT:
    case SPELL_FLY:
    case SPELL_ANTIGRAVITY:
    case SPELL_FALCON_WINGS:
    case SPELL_LEVITATE:
    case SPELL_IDENTIFY:
    case SPELL_DIVINATION:
    case SPELL_POWERSTONE:
    case SPELL_SHATTER:
    case SPELL_EYES_OF_FERTUMAN:
    case SPELL_FARLOOK:
    case SPELL_ILLUMINATE:
    case SPELL_DETECT_MAGIC:
    case SPELL_DISPEL_MAGIC:
    case SPELL_CHASE_SPIRIT:
    case SPELL_CARDIAC_STRESS:
    case SPELL_COPY:
    case SPELL_CHRISM:
    case SPELL_DETECT_SHADOW:
    case SPELL_MATERIALIZE:
    case SPELL_SPONTANEOUS_GENERATION:
    case SPELL_GALVANIZE:
    case SPELL_STONE_SKIN:
    case SPELL_TRAIL_SEEK:
    case SPELL_FAERIE_FIRE:
    case SPELL_STUPIDITY:
    case SPELL_FLAMING_FLESH:
    case SPELL_DJALLA: // shaman
    case SPELL_LEGBA: // shaman
    case SPELL_PROTECTION_FROM_FIRE:
    case SPELL_PROTECTION_FROM_ELEMENTS:
    case SPELL_PROTECTION_FROM_EARTH:
    case SPELL_PROTECTION_FROM_AIR:
    case SPELL_PROTECTION_FROM_WATER:  
    case SPELL_FLARE:
    case SPELL_INFRAVISION:
    case SPELL_SENSE_LIFE:
    case SPELL_SENSE_LIFE_SHAMAN:
    case SPELL_SILENCE:
    case SPELL_STEALTH:
    case SPELL_CALM:
    case SPELL_CLOUD_OF_CONCEALMENT:
    case SPELL_DETECT_INVISIBLE:
    case SPELL_DISPEL_INVISIBLE:
    case SPELL_TELEPATHY:
    case SPELL_ROMBLER:
    case SPELL_TRUE_SIGHT:
    case SPELL_POLYMORPH:
    case SPELL_ACCELERATE:
    case SPELL_CELERITE:
    case SPELL_ENLIVEN:
    case SPELL_CHEVAL: // shaman
    case SPELL_HASTE:
    case SPELL_FAERIE_FOG:
    case SPELL_GILLS_OF_FLESH:
    case SPELL_AQUALUNG:
    case SPELL_BREATH_OF_SARAHAGE:
    case SPELL_CREATE_FOOD_DEIKHAN:
    case SPELL_CREATE_WATER_DEIKHAN:
    case SPELL_CREATE_FOOD:
    case SPELL_CREATE_WATER:
    case SPELL_BLESS_DEIKHAN:
    case SPELL_BLESS:
    case SPELL_HEROES_FEAST_DEIKHAN:
    case SPELL_HEROES_FEAST:
    case SPELL_ARMOR_DEIKHAN:
    case SPELL_ARMOR:
    case SPELL_ASTRAL_WALK:
    case SPELL_SANCTUARY:
    case SPELL_RELIVE:
    case SPELL_WORD_OF_RECALL:
    case SPELL_SUMMON:
    case SPELL_REMOVE_CURSE_DEIKHAN:
    case SPELL_REMOVE_CURSE:
    case SKILL_BEFRIEND_BEAST:
    case SKILL_BEAST_CHARM:
    case SPELL_SHAPESHIFT:
    case SPELL_PLAGUE_LOCUSTS:
    case SPELL_ROOT_CONTROL:
    case SPELL_STICKS_TO_SNAKES:
    case SPELL_LIVING_VINES:
    case SPELL_STORMY_SKIES:
    case SPELL_TREE_WALK:
    case SKILL_BARKSKIN:
    case SPELL_HEALING_GRASP:
    case SPELL_HEAL_LIGHT_DEIKHAN:
    case SPELL_HEAL_SERIOUS_DEIKHAN:
    case SPELL_HEAL_CRITICAL_DEIKHAN:
    case SPELL_HEAL_LIGHT:
    case SPELL_HEAL_SERIOUS:
    case SPELL_HEAL_CRITICAL:
    case SPELL_HEAL_CRITICAL_SPRAY:
    case SPELL_HEAL:
    case SPELL_HEAL_SPRAY:
    case SPELL_HEAL_FULL:
    case SPELL_HEAL_FULL_SPRAY:
    case SPELL_CURE_POISON_DEIKHAN:
    case SPELL_CURE_POISON:
    case SPELL_SALVE_DEIKHAN:
    case SPELL_SALVE:
    case SPELL_CLEANSE:
    case SPELL_RESTORE_LIMB:
    case SPELL_KNIT_BONE:
    case SPELL_CURE_PARALYSIS:
    case SPELL_CLOT_DEIKHAN:
    case SPELL_CLOT:
    case SPELL_STERILIZE_DEIKHAN:
    case SPELL_REFRESH_DEIKHAN:
    case SPELL_STERILIZE:
    case SPELL_CURE_BLINDNESS:
    case SPELL_REFRESH:
    case SPELL_SECOND_WIND:
    case SPELL_EXPEL_DEIKHAN:
    case SPELL_EXPEL:
    case SKILL_DEATHSTROKE:
    case SPELL_SORCERERS_GLOBE:
    case SKILL_CHARGE:
    case SPELL_SYNOSTODWEOMER:
    case SPELL_INVISIBILITY:
    case SKILL_BERSERK:
    case SKILL_SHOULDER_THROW:
    case SKILL_DISARM_DEIKHAN:
    case SKILL_DISARM_THIEF:
    case SKILL_DISARM_MONK:
    case SKILL_DISARM:
    case SKILL_TRANSFORM_LIMB:
    case SPELL_CURE_DISEASE:
    case SPELL_CURE_DISEASE_DEIKHAN:
    case SPELL_PLASMA_MIRROR:
    case SPELL_THORNFLESH:
    case SPELL_CLARITY:
    case SPELL_GARMULS_TAIL:
    case SPELL_ETHER_GATE:
    case SPELL_SHADOW_WALK:
    case SKILL_SKIN:
    case SKILL_BUTCHER:
    case SKILL_WHITTLE:
    case SKILL_STAVECHARGE:
    case SKILL_RANGED_PROF: // was skill_bow... wierd error
    case SKILL_PIERCE_PROF:
    case SKILL_BLUNT_PROF:
    case SKILL_SHARPEN:
    case SKILL_DULL:
    case SKILL_BAREHAND_PROF:
    case SKILL_ATTUNE:
    case SKILL_RANGED_SPEC:
    case SKILL_FAST_LOAD:
    case SKILL_SLASH_PROF:
    case SKILL_PIERCE_SPEC:
    case SKILL_SLASH_SPEC:
    case SKILL_SCRIBE:
    case SKILL_RESCUE:
    case SKILL_BLACKSMITHING:
    case SKILL_REPAIR_MAGE:
    case SKILL_REPAIR_MONK:
    case SKILL_REPAIR_CLERIC:
    case SKILL_REPAIR_DEIKHAN:
    case SKILL_REPAIR_SHAMAN:
    case SKILL_REPAIR_THIEF:
    case SKILL_BLACKSMITHING_ADVANCED:
    case SKILL_MEND:
    case SKILL_SACRIFICE:
    case SKILL_SWITCH_OPP:
    case SKILL_RETREAT:
    case SKILL_GRAPPLE:
    case SKILL_TRANCE_OF_BLADES:
    case SKILL_WEAPON_RETENTION:
    case SKILL_CLOSE_QUARTERS_FIGHTING:
    case SKILL_HIKING:
    case SKILL_FORAGE:
    case SKILL_SEEKWATER:
    case SKILL_TRACK:
    case SKILL_CONCEALMENT:
    case SKILL_DIVINATION:
    case SKILL_APPLY_HERBS:
    case SKILL_ENCAMP:
    case SKILL_CHIVALRY:
    case SKILL_RESCUE_DEIKHAN:
    case SKILL_SWITCH_DEIKHAN:
    case SKILL_RETREAT_DEIKHAN:
    case SKILL_RIDE:
    case SKILL_CALM_MOUNT:
    case SKILL_TRAIN_MOUNT:
    case SKILL_ADVANCED_RIDING:
    case SKILL_RIDE_DOMESTIC:
    case SKILL_RIDE_NONDOMESTIC:
    case SKILL_RIDE_WINGED:
    case SKILL_RIDE_EXOTIC:
    case SKILL_LAY_HANDS:
    case SKILL_YOGINSA:
    case SKILL_CINTAI:
    case SKILL_OOMLAT:
    case SKILL_ADVANCED_KICKING:
    case SKILL_GROUNDFIGHTING:
    case SKILL_DUFALI:
    case SKILL_RETREAT_MONK:
    case SKILL_SNOFALTE:
    case SKILL_COUNTER_MOVE:
    case SKILL_SWITCH_MONK:
    case SKILL_JIRIN:
    case SKILL_KUBO:
    case SKILL_CATLEAP:
    case SKILL_CATFALL:
    case SKILL_WOHLIN:
    case SKILL_VOPLAT:
    case SKILL_BLINDFIGHTING:
    case SKILL_CRIT_HIT:
    case SKILL_FEIGN_DEATH:
    case SKILL_BLUR:
    case SKILL_CHAIN_ATTACK:
    case SKILL_HURL:
    case SKILL_SWINDLE:
    case SKILL_SNEAK:
    case SKILL_RETREAT_THIEF:
    case SKILL_PICK_LOCK:
    case SKILL_SEARCH:
    case SKILL_SPY:
    case SKILL_SWITCH_THIEF:
    case SKILL_STEAL:
    case SKILL_DETECT_TRAP:
    case SKILL_SUBTERFUGE:
    case SKILL_DISARM_TRAP:
    case SKILL_HIDE:
    case SKILL_POISON_WEAPON:
    case SKILL_DISGUISE:
    case SKILL_DODGE_THIEF:
    case SKILL_SET_TRAP_CONT:
    case SKILL_SET_TRAP_DOOR:
    case SKILL_SET_TRAP_MINE:
    case SKILL_SET_TRAP_GREN:
    case SKILL_SET_TRAP_ARROW:
    case SKILL_DUAL_WIELD_THIEF:
    case SKILL_COUNTER_STEAL:
    case SKILL_BREW:
    case SKILL_TURN:
    case SKILL_SIGN:
    case SKILL_SWIM:
    case SKILL_CONS_UNDEAD:
    case SKILL_CONS_VEGGIE:
    case SKILL_CONS_DEMON:
    case SKILL_CONS_ANIMAL:
    case SKILL_CONS_REPTILE:
    case SKILL_CONS_PEOPLE:
    case SKILL_CONS_GIANT:
    case SKILL_CONS_OTHER:
    case SKILL_READ_MAGIC:
    case SKILL_BANDAGE:
    case SKILL_CLIMB:
    case SKILL_FAST_HEAL:
    case SKILL_EVALUATE:
    case SKILL_TACTICS:
    case SKILL_DISSECT:
    case SKILL_DEFENSE:
    case SKILL_ADVANCED_DEFENSE:
    case SKILL_OFFENSE:
    case SKILL_WIZARDRY:
    case SKILL_RITUALISM:
    case SKILL_MEDITATE:
    case SKILL_DEVOTION:
    case SKILL_PENANCE:
    case SKILL_BLUNT_SPEC:
    case SKILL_BAREHAND_SPEC:
    case SKILL_DUAL_WIELD:
    case SPELL_SHIELD_OF_MISTS:
    case SPELL_ENTHRALL_SPECTRE:
    case SPELL_ENTHRALL_GHAST:
    case SPELL_ENTHRALL_GHOUL:
    case SPELL_ENTHRALL_DEMON:
    case SPELL_CREATE_WOOD_GOLEM:
    case SPELL_CREATE_ROCK_GOLEM:
    case SPELL_CREATE_IRON_GOLEM:
    case SPELL_CREATE_DIAMOND_GOLEM:
    case DAMAGE_NORMAL:
    case DAMAGE_BEHEADED:
    case DAMAGE_RAMMED:
    case DAMAGE_HACKED:
    case DAMAGE_RIPPED_OUT_HEART:
    case DAMAGE_CAVED_SKULL:
    case DAMAGE_HEADBUTT_SKULL:
    case DAMAGE_KNEESTRIKE_FOOT:
    case DAMAGE_KNEESTRIKE_SHIN:
    case DAMAGE_KNEESTRIKE_KNEE:
    case DAMAGE_KNEESTRIKE_THIGH:
    case DAMAGE_KNEESTRIKE_CROTCH:
    case DAMAGE_KNEESTRIKE_SOLAR:
    case DAMAGE_KNEESTRIKE_CHIN:
    case DAMAGE_KNEESTRIKE_FACE:
    case DAMAGE_HEADBUTT_JAW:
    case DAMAGE_HEADBUTT_THROAT:
    case DAMAGE_HEADBUTT_BODY:
    case DAMAGE_HEADBUTT_CROTCH:
    case DAMAGE_HEADBUTT_LEG:
    case DAMAGE_HEADBUTT_FOOT:
    case DAMAGE_DISEMBOWLED_HR:
    case DAMAGE_DISEMBOWLED_VR:
    case DAMAGE_STOMACH_WOUND:
    case DAMAGE_IMPALE:
    case DAMAGE_STARVATION:
    case DAMAGE_EATTEN:
    case TYPE_MAX_HIT:
    case MAX_SKILL:
    case AFFECT_TRANSFORMED_HANDS:
    case AFFECT_TRANSFORMED_ARMS:
    case AFFECT_TRANSFORMED_LEGS:
    case AFFECT_TRANSFORMED_HEAD:
    case AFFECT_TRANSFORMED_NECK:
    case LAST_TRANSFORMED_LIMB:
    case LAST_BREATH_WEAPON:
    case AFFECT_DUMMY:
    case AFFECT_WAS_INDOORS:
    case AFFECT_DRUNK:
    case AFFECT_NEWBIE:
    case AFFECT_SKILL_ATTEMPT:
    case AFFECT_FREE_DEATHS:
    case AFFECT_TEST_FIGHT_MOB:
    case AFFECT_DRUG:
    case AFFECT_ORPHAN_PET:
    case AFFECT_DISEASE:
    case AFFECT_COMBAT:
    case AFFECT_PET:
    case AFFECT_CHARM:
    case AFFECT_THRALL:
    case AFFECT_PLAYERKILL:
    case AFFECT_PLAYERLOOT:
    case AFFECT_HORSEOWNED:
    case AFFECT_GROWTH_POTION:
    case AFFECT_WARY:
    case AFFECT_DEFECTED:
    case AFFECT_OFFER:
    case AFFECT_OBJECT_USED:
    case AFFECT_BITTEN_BY_VAMPIRE:
    case LAST_ODDBALL_AFFECT:
    case SKILL_ALCOHOLISM:
    case SKILL_FISHING:
    case SKILL_LOGGING:
    case SKILL_PSITELEPATHY:
    case SKILL_TELE_SIGHT:
    case SKILL_TELE_VISION:
    case SKILL_MIND_FOCUS:
    case SKILL_PSI_BLAST:
    case SKILL_MIND_THRUST:
    case SKILL_PSYCHIC_CRUSH:
    case SKILL_KINETIC_WAVE:
    case SKILL_MIND_PRESERVATION:
    case SKILL_TELEKINESIS:
    case SKILL_PSIDRAIN:
    case SKILL_MANA:
    case SKILL_IRON_FIST:
    case SKILL_IRON_FLESH:
    case SKILL_IRON_SKIN:
    case SKILL_IRON_BONES:
    case SKILL_IRON_MUSCLES:
    case SKILL_IRON_LEGS:
    case SKILL_IRON_WILL:
    case SKILL_PLANT:
    case SPELL_EMBALM:
    case ABSOLUTE_MAX_SKILL:
#if 1
    case SPELL_EARTHMAW:
    case SPELL_CREEPING_DOOM:
    case SPELL_FERAL_WRATH:
    case SPELL_SKY_SPIRIT:
#endif
      bit = IMMUNE_NONE;
      break;
  }
  return bit;
}


