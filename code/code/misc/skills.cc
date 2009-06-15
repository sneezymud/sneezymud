#if 0
 Some quickie Documentation on the different functions.
 CSkill holds two values, natural and actual.  Natural reflects a straight-up
 value based on practicing and training.  actual is modified by equipment..
 Obviously, only the natural needs to be saved.

 getRawNaturalSkillValue and setNaturalSkillValue access natural
 getRawSkillValue and setSkillValue access actual

  on top of that, are two utility functions...
  getSkillValue() and getNaturalSkillValue() which impose some limits
  on the range returned.  For instance, they keep it less than max and
  forced unlearned to return 0.  These two functions manipulate the above
  "Raw" functions.
#endif

#include "being.h"
#include "disc_mage.h"
#include "disc_cleric.h"
#include "extern.h"
#include "disc_soldiering.h"
#include "disc_blacksmithing.h"
#include "disc_deikhan_fight.h"
#include "disc_deikhan_aegis.h"
#include "disc_deikhan_cures.h"
#include "disc_deikhan_wrath.h"
#include "disc_defense.h"
#include "disc_mounted.h"
#include "disc_monk.h"
#include "disc_iron_body.h"
#include "disc_meditation.h"
#include "disc_leverage.h"
#include "disc_mindbody.h"
#include "disc_fattacks.h"
#include "disc_plants.h"
#include "disc_shaman_frog.h"
#include "disc_shaman_healing.h"
#include "disc_shaman_alchemy.h"
#include "disc_shaman_control.h"
#include "disc_shaman_spider.h"
#include "disc_shaman_skunk.h"
#include "disc_ritualism.h"
#include "disc_thief.h"
#include "disc_thief_fight.h"
#include "disc_poisons.h"
#include "disc_traps.h"
#include "disc_stealth.h"
#include "disc_air.h"
#include "disc_alchemy.h"
#include "disc_earth.h"
#include "disc_fire.h"
#include "disc_sorcery.h"
#include "disc_spirit.h"
#include "disc_water.h"
#include "disc_wrath.h"
#include "disc_aegis.h"
#include "disc_shaman.h"
#include "disc_afflictions.h"
#include "disc_cures.h"
#include "disc_hand_of_god.h"
#include "disc_deikhan.h"
#include "disc_ranger.h"
#include "disc_looting.h"
#include "disc_murder.h"
#include "disc_dueling.h"
#include "disc_combat.h"
#include "disc_adventuring.h"
#include "disc_adv_adventuring.h"
#if 0
#include "disc_jumando.h"
#include "disc_kararki.h"
#include "disc_zinra.h"
#include "disc_yofu.h"
#include "disc_akodi.h"
#endif
#include "disc_wizardry.h"
#include "disc_lore.h"
#include "disc_theology.h"
#include "disc_faith.h"
#include "disc_slash.h"
#include "disc_blunt.h"
#include "disc_pierce.h"
#include "disc_ranged.h"
#include "disc_barehand.h"
#include "disc_brawling.h"
#include "disc_warrior.h"
#include "disc_animal.h"
#include "disc_shaman_armadillo.h"
#include "disc_nature.h"
#include "disc_psionics.h"
#include "disc_commoner.h"

static bool doesKnow(byte know)
{
  if (know <= 0)
    return FALSE;
  else
    return TRUE;
}

CSkill *TBeing::getSkill(spellNumT skill) const
{
  discNumT which = getDisciplineNumber(skill, FALSE);
  if (which == DISC_NONE) {
    // silly core-generator, but helps to track down the item that is bad
    vlogf(LOG_BUG, format("Bad discipline for skill %d in getSkill()") %  skill);
    return NULL;
  }

  mud_assert(skill > TYPE_UNDEFINED && skill < MAX_SKILL, "Bad skill in getSkill()");

  CDiscipline *cd = getDiscipline(which);
  if (!cd)
    return NULL;

  switch (skill) {
#if 0
    case SKILL_MASS_FORAGE:  //               392
      return &((CDSurvival *) cd)->skMassForage;
    case SKILL_TAN:  //                       394
      return &((CDSurvival *) cd)->skTan;
    case SKILL_HOLY_WEAPONS:  //              454             // not coded
      return &((CDDeikhanFight *) cd)->skHolyWeapons;
    case SPELL_HOLY_LIGHT:  //                495         // not coded
      return &((CDDeikhanWrath *) cd)->skHolyLight;
    case SKILL_BUTCHER:  //                    342  // not coded
      return &((CDRanger *) cd)->skButcher;
    case SPELL_FIND_FAMILIAR: // 35
      return &((CDMage *) cd)->skFindFamiliar;
    case SPELL_CHAIN_LIGHTNING:  //            123 // not coded
      return &((CDSorcery *) cd)->skChainLightning;
    case SPELL_DETECT_POISON:  //              174 // NOT CODED
      return &((CDCleric *) cd)->skDetectPoison;
    case SPELL_DETECT_POISON_DEIKHAN:  //     417     // not coded
      return &((CDDeikhan *) cd)->skDetectPoisonDeikhan;
    case SKILL_CASTING: // 961
      return &((CDWizardry *) cd)->skCasting;
    case SKILL_PRAYING: // 961
      return &((CDFaith *) cd)->skPraying;
#endif

//  MAGE CLASS

// disc_mage
    case SPELL_GUST: // 0 
      return &((CDMage *) cd)->skGust;
    case SPELL_SLING_SHOT: // 1 
      return &((CDMage *) cd)->skSlingShot;
    case SPELL_GUSHER: // 2
      return &((CDMage *) cd)->skGusher;
    case SPELL_HANDS_OF_FLAME: // 3
      return &((CDMage *) cd)->skHandsOfFlame;
    case SPELL_MYSTIC_DARTS: // 4
      return &((CDMage *) cd)->skMysticDarts;
    case SPELL_FLARE: //  5
      return &((CDMage *) cd)->skFlare;
    case SPELL_SORCERERS_GLOBE: // 6
      return &((CDMage *) cd)->skSorcerersGlobe;
    case SPELL_FAERIE_FIRE:  // 7
      return &((CDMage *) cd)->skFaerieFire;
    case SPELL_ILLUMINATE:   // 8
      return &((CDMage *) cd)->skIlluminate;
    case SPELL_DETECT_MAGIC: // 9
      return &((CDMage *) cd)->skDetectMagic;
    case SPELL_STUNNING_ARROW:  // 10
      return &((CDMage *) cd)->skStunningArrow;
    case SPELL_MATERIALIZE:  // 11  
      return &((CDMage *) cd)->skMaterialize;
    case SPELL_PROTECTION_FROM_EARTH: // 12    
      return &((CDMage *) cd)->skProtectionFromEarth;
    case SPELL_PROTECTION_FROM_AIR:  // 13     
      return &((CDMage *) cd)->skProtectionFromAir;
    case SPELL_PROTECTION_FROM_FIRE:  //14   
      return &((CDMage *) cd)->skProtectionFromFire;
    case SPELL_PROTECTION_FROM_WATER: // 15 
      return &((CDMage *) cd)->skProtectionFromWater;
    case SPELL_PROTECTION_FROM_ELEMENTS: //16
      return &((CDMage *) cd)->skProtectionFromElements;
    case SPELL_PEBBLE_SPRAY: // 17
      return &((CDMage *) cd)->skPebbleSpray;
    case SPELL_ARCTIC_BLAST: // 18
      return &((CDMage *) cd)->skArcticBlast;
    case SPELL_COLOR_SPRAY: // 19  
      return &((CDMage *) cd)->skColorSpray;
    case SPELL_INFRAVISION: // 20 
      return &((CDMage *) cd)->skInfravision;
    case SPELL_IDENTIFY:  // 21   
      return &((CDMage *) cd)->skIdentify;
    case SPELL_POWERSTONE:  // 22
      return &((CDMage *) cd)->skPowerstone;
    case SPELL_FAERIE_FOG:  // 23
      return &((CDMage *) cd)->skFaerieFog;
    case SPELL_TELEPORT: // 24
      return &((CDMage *) cd)->skTeleport;
    case SPELL_SENSE_LIFE: // 25
      return &((CDMage *) cd)->skSenseLife;
    case SPELL_CALM:  // 26
      return &((CDMage *) cd)->skCalm;
    case SPELL_ACCELERATE: // 27
      return &((CDMage *) cd)->skAccelerate;
    case SPELL_DUST_STORM: // 28
      return &((CDMage *) cd)->skDustStorm;
    case SPELL_LEVITATE: // 29 
      return &((CDMage *) cd)->skLevitate;
    case SPELL_FEATHERY_DESCENT: // 30
      return &((CDMage *) cd)->skFeatheryDescent;
    case SPELL_STEALTH: // 31
      return &((CDMage *) cd)->skStealth;
    case SPELL_GRANITE_FISTS: //32
      return &((CDMage *) cd)->skGraniteFists;
    case SPELL_ICY_GRIP:  // 33
      return &((CDMage *) cd)->skIcyGrip;
    case SPELL_GILLS_OF_FLESH:  // 34
      return &((CDMage *) cd)->skGillsOfFlesh;
    case SPELL_TELEPATHY:  // 36
      return &((CDMage *) cd)->skTelepathy;
    case SPELL_FEAR:    // 37
      return &((CDMage *) cd)->skFear;
    case SPELL_SLUMBER: // 38
      return &((CDMage *) cd)->skSlumber;
    case SPELL_CONJURE_EARTH: // 39
      return &((CDMage *) cd)->skConjureElemEarth;
    case SPELL_CONJURE_AIR: // 40 
      return &((CDMage *) cd)->skConjureElemAir;
    case SPELL_CONJURE_FIRE: // 41
      return &((CDMage *) cd)->skConjureElemFire;
    case SPELL_CONJURE_WATER: // 42
      return &((CDMage *) cd)->skConjureElemWater;
    case SPELL_DISPEL_MAGIC: // 43
      return &((CDMage *) cd)->skDispelMagic;
    case SPELL_ENHANCE_WEAPON: // 44
      return &((CDMage *) cd)->skEnhanceWeapon;
    case SPELL_GALVANIZE:
      return &((CDMage *) cd)->skGalvanize;
    case SPELL_DETECT_INVISIBLE:
      return &((CDMage *) cd)->skDetectInvisible;
    case SPELL_DISPEL_INVISIBLE:
      return &((CDMage *) cd)->skDispelInvisible;
    case SPELL_TORNADO:
      return &((CDMage *) cd)->skTornado;
    case SPELL_SAND_BLAST:
      return &((CDMage *) cd)->skSandBlast;
    case SPELL_ICE_STORM:
      return &((CDMage *) cd)->skIceStorm;
    case SPELL_FLAMING_SWORD:
      return &((CDMage *) cd)->skFlamingSword;
    case SPELL_ACID_BLAST:
      return &((CDMage *) cd)->skAcidBlast;
    case SPELL_FIREBALL:
      return &((CDMage *) cd)->skFireball;
    case SPELL_FARLOOK:
      return &((CDMage *) cd)->skFarlook;
    case SPELL_FALCON_WINGS:
      return &((CDMage *) cd)->skFalconWings;
    case SPELL_INVISIBILITY:
      return &((CDMage *) cd)->skInvisibility;
    case SPELL_ENSORCER:
      return &((CDMage *) cd)->skEnsorcer;
    case SPELL_EYES_OF_FERTUMAN:
      return &((CDMage *) cd)->skEyesOfFertuman;
    case SPELL_COPY:
      return &((CDMage *) cd)->skCopy;
    case SPELL_HASTE:
      return &((CDMage *) cd)->skHaste;
    case SKILL_REPAIR_MAGE:
      return &((CDMage *) cd)->skRepairMage;

// disc_air

    case SPELL_IMMOBILIZE:
      return &((CDAir *) cd)->skImmobilize;
    case SPELL_SUFFOCATE:
      return &((CDAir *) cd)->skSuffocate;
    case SPELL_FLY:
      return &((CDAir *) cd)->skFly;
    case SPELL_ANTIGRAVITY:
      return &((CDAir *) cd)->skAntigravity;

// disc_alchemy
 
    case SPELL_DIVINATION:
      return &((CDAlchemy *) cd)->skDivination;
    case SPELL_SHATTER:
      return &((CDAlchemy *) cd)->skShatter;
    case SKILL_SCRIBE:
      return &((CDAlchemy *) cd)->skScribe;
    case SPELL_ETHER_GATE:
      return &((CDAlchemy *) cd)->skEtherGate;
    case SPELL_SPONTANEOUS_GENERATION:
      return &((CDAlchemy *) cd)->skSpontaneousGeneration;
    case SKILL_STAVECHARGE:
      return &((CDAlchemy *) cd)->skStaveCharge;

// disc_earth

    case SPELL_METEOR_SWARM:
      return &((CDEarth *) cd)->skMeteorSwarm;
    case SPELL_LAVA_STREAM:
      return &((CDEarth *) cd)->skLavaStream;
    case SPELL_STONE_SKIN:
      return &((CDEarth *) cd)->skStoneSkin;
    case SPELL_TRAIL_SEEK:
      return &((CDEarth *) cd)->skTrailSeek;

// disc_fire

    case SPELL_INFERNO:
      return &((CDFire *) cd)->skInferno;
    case SPELL_HELLFIRE:
      return &((CDFire *) cd)->skHellFire;
    case SPELL_FLAMING_FLESH:
      return &((CDFire *) cd)->skFlamingFlesh;

// disc_sorcery

    case SPELL_BLAST_OF_FURY:
      return &((CDSorcery *) cd)->skBlastOfFury;
    case SPELL_ENERGY_DRAIN:
      return &((CDSorcery *) cd)->skEnergyDrain;
    case SPELL_ATOMIZE:
      return &((CDSorcery *) cd)->skAtomize;
    case SPELL_ANIMATE:
      return &((CDSorcery *) cd)->skAnimate;
    case SPELL_BIND:
      return &((CDSorcery *) cd)->skBind;

// disc_spirit

    case SPELL_FUMBLE:
      return &((CDSpirit *) cd)->skFumble;
    case SPELL_TRUE_SIGHT:
      return &((CDSpirit *) cd)->skTrueSight;
    case SPELL_CLOUD_OF_CONCEALMENT:
      return &((CDSpirit *) cd)->skCloudOfConcealment;
    case SPELL_POLYMORPH:
      return &((CDSpirit *) cd)->skPolymorph;
    case SPELL_SILENCE:
      return &((CDSpirit *) cd)->skSilence;
    case SPELL_KNOT:
      return &((CDSpirit *) cd)->skKnot;
 
// disc_water

    case SPELL_WATERY_GRAVE:
      return &((CDWater *) cd)->skWateryGrave;
    case SPELL_TSUNAMI:
      return &((CDWater *) cd)->skTsunami;
    case SPELL_BREATH_OF_SARAHAGE:
      return &((CDWater *) cd)->skBreathOfSarahage;
    case SPELL_PLASMA_MIRROR:
      return &((CDWater *) cd)->skPlasmaMirror;
    case SPELL_GARMULS_TAIL:
      return &((CDWater *) cd)->skGarmulsTail;

// CLASS CLERIC

// disc_cleric

    case SPELL_HEAL_LIGHT:
      return &((CDCleric *) cd)->skHealLight;
    case SPELL_HARM_LIGHT:
      return &((CDCleric *) cd)->skHarmLight;
    case SPELL_CREATE_FOOD:
      return &((CDCleric *) cd)->skCreateFood;
    case SPELL_CREATE_WATER:
      return &((CDCleric *) cd)->skCreateWater;
    case SPELL_ARMOR:
      return &((CDCleric *) cd)->skArmor;
    case SPELL_BLESS:
      return &((CDCleric *) cd)->skBless;
    case SPELL_CLOT:
      return &((CDCleric *) cd)->skClot;
    case SPELL_RAIN_BRIMSTONE:
      return &((CDCleric *) cd)->skRainBrimstone;
    case SPELL_HEAL_SERIOUS:
      return &((CDCleric *) cd)->skHealSerious;
    case SPELL_HARM_SERIOUS:
      return &((CDCleric *) cd)->skHarmSerious;
    case SPELL_STERILIZE:
      return &((CDCleric *) cd)->skSterilize;
    case SPELL_EXPEL:
      return &((CDCleric *) cd)->skExpel;
    case SPELL_CURE_DISEASE:
      return &((CDCleric *) cd)->skCureDisease;
    case SPELL_CURSE:
      return &((CDCleric *) cd)->skCurse;
    case SPELL_REMOVE_CURSE:
      return &((CDCleric *) cd)->skRemoveCurse;
    case SPELL_CURE_POISON:
      return &((CDCleric *) cd)->skCurePoison;
    case SPELL_HEAL_CRITICAL:
      return &((CDCleric *) cd)->skHealCritical;
    case SPELL_SALVE:
      return &((CDCleric *) cd)->skSalve;
    case SPELL_POISON:
      return &((CDCleric *) cd)->skPoison;
    case SPELL_HARM_CRITICAL:
      return &((CDCleric *) cd)->skHarmCritical;
    case SPELL_INFECT:
      return &((CDCleric *) cd)->skInfect;
    case SPELL_REFRESH:
      return &((CDCleric *) cd)->skRefresh;
    case SPELL_NUMB:
      return &((CDCleric *) cd)->skNumb;
    case SPELL_DISEASE:
      return &((CDCleric *) cd)->skDisease;
    case SPELL_FLAMESTRIKE:
      return &((CDCleric *) cd)->skFlamestrike;
    case SPELL_PLAGUE_LOCUSTS:
      return &((CDCleric *) cd)->skPlagueOfLocusts;
    case SPELL_CURE_BLINDNESS:
      return &((CDCleric *) cd)->skCureBlindness;
    case SPELL_SUMMON:
      return &((CDCleric *) cd)->skSummon;
    case SPELL_HEAL:
      return &((CDCleric *) cd)->skHeal;
    case SPELL_PARALYZE_LIMB:
      return &((CDCleric *) cd)->skParalyzeLimb;
    case SPELL_WORD_OF_RECALL:
      return &((CDCleric *) cd)->skWordOfRecall;
    case SPELL_HARM:
      return &((CDCleric *) cd)->skHarm;
    case SPELL_KNIT_BONE:
      return &((CDCleric *) cd)->skKnitBone;
    case SPELL_BLINDNESS:
      return &((CDCleric *) cd)->skBlindness;
    case SKILL_REPAIR_CLERIC:
      return &((CDCleric *) cd)->skRepairCleric;


//disc_wrath.h
    case SPELL_PILLAR_SALT:
      return &((CDWrath *) cd)->skPillarOfSalt;
    case SPELL_EARTHQUAKE:
      return &((CDWrath *) cd)->skEarthquake;
    case SPELL_CALL_LIGHTNING:
      return &((CDWrath *) cd)->skCallLightning;
    case SPELL_SPONTANEOUS_COMBUST:
      return &((CDWrath *) cd)->skSpontaneousCombust;

// disc_afflictions

    case SPELL_BLEED:
      return &((CDAfflict *) cd)->skBleed;
    case SPELL_PARALYZE:
      return &((CDAfflict *) cd)->skParalyze;
    case SPELL_BONE_BREAKER:
      return &((CDAfflict *) cd)->skBoneBreaker;
    case SPELL_WITHER_LIMB:
      return &((CDAfflict *) cd)->skWitherLimb;


// disc_aegis

    case SPELL_SANCTUARY:
      return &((CDAegis *) cd)->skSanctuary;
    case SPELL_CURE_PARALYSIS:
      return &((CDAegis *) cd)->skCureParalyze;
    case SPELL_SECOND_WIND:
      return &((CDAegis *) cd)->skSecondWind;
    case SPELL_RELIVE:
      return &((CDAegis *) cd)->skRelive;

// disc_hand_of_god

    case SPELL_HEROES_FEAST:
      return &((CDHand *) cd)->skHeroesFeast;
    case SPELL_ASTRAL_WALK:
      return &((CDHand *) cd)->skAstralWalk;
    case SPELL_PORTAL:
      return &((CDHand *) cd)->skPortal;

// disc_cures

    case SPELL_HEAL_FULL:
      return &((CDCures *) cd)->skHealFull;
    case SPELL_HEAL_CRITICAL_SPRAY:
      return &((CDCures *) cd)->skHealCritSpray;
    case SPELL_HEAL_SPRAY:
      return &((CDCures *) cd)->skHealSpray;
    case SPELL_HEAL_FULL_SPRAY:
      return &((CDCures *) cd)->skHealFullSpray;
    case SPELL_RESTORE_LIMB:
      return &((CDCures *) cd)->skRestoreLimb;

// CLASS WARRIOR

// disc_warrior

    case SKILL_KICK:
      return &((CDWarrior *) cd)->skKick;
    case SKILL_BASH:
      return &((CDWarrior *) cd)->skBash;
    case SKILL_HEADBUTT:
      return &((CDWarrior *) cd)->skHeadbutt;
    case SKILL_RESCUE:
      return &((CDWarrior *) cd)->skRescue;
    case SKILL_BLACKSMITHING:
      return &((CDWarrior *) cd)->skBlacksmithing;
    case SKILL_DISARM:
      return &((CDWarrior *) cd)->skDisarm;
    case SKILL_BERSERK:
      return &((CDWarrior *) cd)->skBerserk;
    case SKILL_SWITCH_OPP:
      return &((CDWarrior *) cd)->skSwitch;
    case SKILL_KNEESTRIKE:
      return &((CDWarrior *) cd)->skKneestrike;
    case SKILL_TRIP:
      return &((CDWarrior *) cd)->skTrip;

//disc_dueling

    case SKILL_SHOVE:
      return &((CDDueling *) cd)->skShove;
    case SKILL_RETREAT:
      return &((CDDueling *) cd)->skRetreat;
    case SKILL_PARRY_WARRIOR:  //            664
      return &((CDDueling *) cd)->skParryWarrior;
    case SKILL_TRANCE_OF_BLADES:
      return &((CDDueling *) cd)->skTranceOfBlades;
    case SKILL_WEAPON_RETENTION:
      return &((CDDueling *) cd)->skWeaponRetention;
    case SKILL_RIPOSTE:
      return &((CDDueling *) cd)->skRiposte;
//disc_brawling

    case SKILL_GRAPPLE:
      return &((CDBrawling *) cd)->skGrapple;
    case SKILL_STOMP:
      return &((CDBrawling *) cd)->skStomp;
    case SKILL_BRAWL_AVOIDANCE:
      return &((CDBrawling *) cd)->skBrawlAvoidance;
    case SKILL_BODYSLAM:
      return &((CDBrawling *) cd)->skBodyslam;
    case SKILL_SPIN:
      return &((CDBrawling *) cd)->skSpin;
    case SKILL_CLOSE_QUARTERS_FIGHTING:
      return &((CDBrawling *) cd)->skCloseQuartersFighting;
    case SKILL_TAUNT:
      return &((CDBrawling *) cd)->skTaunt;

// disc_soldiering

    case SKILL_DUAL_WIELD:  // needs to be moved   666
      return &((CDSoldiering *) cd)->skDualWieldWarrior;
    case SKILL_DOORBASH:
      return &((CDSoldiering *) cd)->skDoorbash;
    case SKILL_DEATHSTROKE:
      return &((CDSoldiering *) cd)->skDeathstroke;
    case SKILL_POWERMOVE:
      return &((CDSoldiering *) cd)->skPowerMove;

// disc_blacksmithing
    case SKILL_BLACKSMITHING_ADVANCED:
      return &((CDBlacksmithing *) cd)->skBlacksmithingAdvanced;


// No skills currently


// CLASS RANGER

    case SKILL_BEAST_SOOTHER:  //              338
      return &((CDRanger *) cd)->skBeastSoother;
    case SKILL_BEFRIEND_BEAST:  //             344
      return &((CDRanger *) cd)->skBefriendBeast;
    case SKILL_BEAST_SUMMON:  //               349
      return &((CDRanger *) cd)->skBeastSummon;
    case SKILL_BARKSKIN:  //                   350
      return &((CDRanger *) cd)->skBarkskin;


// disc_fight_ranger

// disc_armadillo

// disc_animal

    case SKILL_BEAST_CHARM:  //                371
      return &((CDAnimal *) cd)->skBeastCharm;
#if 1
    case SPELL_FERAL_WRATH:
      return &((CDAnimal *) cd)->skFeralWrath;
    case SPELL_SKY_SPIRIT:
      return &((CDAnimal *) cd)->skSkySpirit;
#endif
// disc_plants

    case SKILL_APPLY_HERBS:  //                382
      return &((CDPlants *) cd)->skApplyHerbs;


// DEIKHAN CLASS

// disc_deikhan

    case SKILL_KICK_DEIKHAN:  //              411
      return &((CDDeikhan *) cd)->skKickDeikhan;
    case SPELL_HEAL_LIGHT_DEIKHAN:  //        412
      return &((CDDeikhan *) cd)->skHealLightDeikhan;
    case SPELL_HARM_LIGHT_DEIKHAN:  //        412
      return &((CDDeikhan *) cd)->skHarmLightDeikhan;
    case SKILL_CHIVALRY:  //                  413
      return &((CDDeikhan *) cd)->skChivalry;
    case SPELL_ARMOR_DEIKHAN:  //             414
      return &((CDDeikhan *) cd)->skArmorDeikhan;
    case SPELL_BLESS_DEIKHAN:  //             415
      return &((CDDeikhan *) cd)->skBlessDeikhan;
    case SKILL_BASH_DEIKHAN:  //              416
      return &((CDDeikhan *) cd)->skBashDeikhan;
    case SPELL_EXPEL_DEIKHAN:  //             418
      return &((CDDeikhan *) cd)->skExpelDeikhan;
    case SPELL_CLOT_DEIKHAN:  //              419
      return &((CDDeikhan *) cd)->skClotDeikhan;
    case SPELL_RAIN_BRIMSTONE_DEIKHAN:  //    420
      return &((CDDeikhan *) cd)->skRainBrimstoneDeikhan;
    case SPELL_STERILIZE_DEIKHAN:  //         421
      return &((CDDeikhan *) cd)->skSterilizeDeikhan;
    case SPELL_REMOVE_CURSE_DEIKHAN:  //      422
      return &((CDDeikhan *) cd)->skRemoveCurseDeikhan;
    case SPELL_CURSE_DEIKHAN:  //             423
      return &((CDDeikhan *) cd)->skCurseDeikhan;
    case SKILL_RESCUE_DEIKHAN:  //            424
      return &((CDDeikhan *) cd)->skRescueDeikhan;
    case SKILL_SMITE:  //                    425
      return &((CDDeikhan *) cd)->skSmite;
    case SPELL_INFECT_DEIKHAN:  //            426
      return &((CDDeikhan *) cd)->skInfectDeikhan;
    case SPELL_CURE_DISEASE_DEIKHAN:  //      427
      return &((CDDeikhan *) cd)->skCureDiseaseDeikhan;
    case SPELL_CREATE_FOOD_DEIKHAN:  //       428
      return &((CDDeikhan *) cd)->skCreateFoodDeikhan;
    case SPELL_CREATE_WATER_DEIKHAN:  //      429
      return &((CDDeikhan *) cd)->skCreateWaterDeikhan;
    case SPELL_HEAL_SERIOUS_DEIKHAN:  //      430
      return &((CDDeikhan *) cd)->skHealSeriousDeikhan;
    case SPELL_CURE_POISON_DEIKHAN:  //       431
      return &((CDDeikhan *) cd)->skCurePoisonDeikhan;
    case SKILL_CHARGE:  //                    432
      return &((CDDeikhan *) cd)->skCharge;
    case SPELL_HARM_SERIOUS_DEIKHAN:  //      433
      return &((CDDeikhan *) cd)->skHarmSeriousDeikhan;
    case SPELL_POISON_DEIKHAN:  //            434
      return &((CDDeikhan *) cd)->skPoisonDeikhan;
    case SKILL_DISARM_DEIKHAN:  //            435
      return &((CDDeikhan *) cd)->skDisarmDeikhan;
    case SPELL_HEAL_CRITICAL_DEIKHAN:  //     436
      return &((CDDeikhan *) cd)->skHealCriticalDeikhan;
    case SPELL_HARM_CRITICAL_DEIKHAN:  //     437
      return &((CDDeikhan *) cd)->skHarmCriticalDeikhan;
    case SKILL_REPAIR_DEIKHAN:
      return &((CDDeikhan *) cd)->skRepairDeikhan;


// disc_deikhan_fight

    case SKILL_SWITCH_DEIKHAN:  //            451
      return &((CDDeikhanFight *) cd)->skSwitchDeikhan;
    case SKILL_RETREAT_DEIKHAN:  //           452
      return &((CDDeikhanFight *) cd)->skRetreatDeikhan;
    case SKILL_SHOVE_DEIKHAN:  //             453
      return &((CDDeikhanFight *) cd)->skShoveDeikhan;

// disc_mount

    case SKILL_CALM_MOUNT:
      return &((CDMounted *) cd)->skCalmMount;
    case SKILL_TRAIN_MOUNT:
      return &((CDMounted *) cd)->skTrainMount;
    case SKILL_ADVANCED_RIDING:
      return &((CDMounted *) cd)->skAdvancedRiding;
    case SKILL_RIDE_DOMESTIC:
      return &((CDMounted *) cd)->skRideDomestic;
    case SKILL_RIDE_NONDOMESTIC:
      return &((CDMounted *) cd)->skRideNonDomestic;
    case SKILL_RIDE_WINGED:
      return &((CDMounted *) cd)->skRideWinged;
    case SKILL_RIDE_EXOTIC:
      return &((CDMounted *) cd)->skRideExotic;

// disc_deikhan_aegis

    case SPELL_HEROES_FEAST_DEIKHAN:  //      471
      return &((CDDeikhanAegis *) cd)->skHeroesFeastDeikhan;

    case SPELL_REFRESH_DEIKHAN:  //           472
      return &((CDDeikhanAegis *) cd)->skRefreshDeikhan;

    case SPELL_SYNOSTODWEOMER:  //            473
      return &((CDDeikhanAegis *) cd)->skSynostodweomer;



// disc_deikhan_cures

    case SPELL_SALVE_DEIKHAN:  //             481
      return &((CDDeikhanCures *) cd)->skSalveDeikhan;
    case SKILL_LAY_HANDS:  //                 482
      return &((CDDeikhanCures *) cd)->skLayHands;


// disc_deikhan_wrath

    case SPELL_HARM_DEIKHAN:  //              491
      return &((CDDeikhanWrath *) cd)->skHarmDeikhan;
    case SPELL_NUMB_DEIKHAN:  //              492
      return &((CDDeikhanWrath *) cd)->skNumbDeikhan;
    case SPELL_EARTHQUAKE_DEIKHAN:  //        493
      return &((CDDeikhanWrath *) cd)->skEarthquakeDeikhan;
    case SPELL_CALL_LIGHTNING_DEIKHAN:  //    494
      return &((CDDeikhanWrath *) cd)->skCallLightningDeikhan;

// MONK CLASS

// disc_monk

     case SKILL_YOGINSA: 
      return &((CDMonk *) cd)->skYoginsa;
     case SKILL_GROUNDFIGHTING:
      return &((CDMonk *) cd)->skGroundfighting;
     case SKILL_CINTAI:  
      return &((CDMonk *) cd)->skCintai;
     case SKILL_OOMLAT:  
      return &((CDMonk *) cd)->skOomlat;
     case SKILL_KICK_MONK:
      return &((CDMonk *) cd)->skKickMonk;
     case SKILL_ADVANCED_KICKING:
      return &((CDMonk *) cd)->skAdvancedKicking;
     case SKILL_SPRINGLEAP:      
      return &((CDMonk *) cd)->skSpringleap;
     case SKILL_SNOFALTE:     
      return &((CDMonk *) cd)->skSnofalte;
     case SKILL_COUNTER_MOVE: 
      return &((CDMonk  *) cd)->skCounterMove;
     case SKILL_SWITCH_MONK:  
      return &((CDMonk *) cd)->skSwitchMonk;
     case SKILL_JIRIN:        
      return &((CDMonk *) cd)->skJirin;
     case SKILL_KUBO:         
      return &((CDMonk *) cd)->skKubo;
     case SKILL_DUFALI:       
      return &((CDMonk *) cd)->skDufali;
    case SKILL_RETREAT_MONK:
      return &((CDMonk *) cd)->skRetreatMonk;
    case SKILL_CHOP:  
      return &((CDMonk *) cd)->skChop;
    case SKILL_DISARM_MONK:
      return &((CDMonk *) cd)->skDisarmMonk;
    case SKILL_CHI:
      return &((CDMonk *) cd)->skChi;
    case SKILL_CATFALL:
      return &((CDMonk *) cd)->skCatfall;
    case SKILL_REPAIR_MONK:
      return &((CDMonk *) cd)->skRepairMonk;
    case SKILL_CATLEAP:
      return &((CDMonk *) cd)->skCatleap;

// disc_meditation_monk
    case SKILL_WOHLIN:
      return &((CDMeditationMonk *) cd)->skWohlin;
    case SKILL_VOPLAT:
      return &((CDMeditationMonk *) cd)->skVoplat;
    case SKILL_BLINDFIGHTING:
      return &((CDMeditationMonk *) cd)->skBlindfighting;

// disc_iron_body
    case SKILL_IRON_FIST:
      return &((CDIronBody *) cd)->skIronFist;
    case SKILL_IRON_FLESH:
      return &((CDIronBody *) cd)->skIronFlesh;
    case SKILL_IRON_SKIN:
      return &((CDIronBody *) cd)->skIronSkin;
    case SKILL_IRON_BONES:
      return &((CDIronBody *) cd)->skIronBones;
    case SKILL_IRON_MUSCLES:
      return &((CDIronBody *) cd)->skIronMuscles;
    case SKILL_IRON_LEGS:
      return &((CDIronBody *) cd)->skIronLegs;
    case SKILL_IRON_WILL:
      return &((CDIronBody *) cd)->skIronWill;

// disc_karoki
    case SKILL_SHOULDER_THROW:
      return &((CDLeverage *) cd)->skShoulderThrow;
    case SKILL_HURL:
      return &((CDLeverage *) cd)->skHurl;
    case SKILL_CHAIN_ATTACK:
      return &((CDLeverage *) cd)->skChainAttack;
    case SKILL_DEFENESTRATE:
      return &((CDLeverage *) cd)->skDefenestrate;
    case SKILL_BONEBREAK:
      return &((CDLeverage *) cd)->skBoneBreak;

// disc_Taedoki

    case SKILL_FEIGN_DEATH:
      return &((CDMindBody *) cd)->skFeignDeath;
    case SKILL_BLUR:
      return &((CDMindBody *) cd)->skBlur;

// disc_kendoki
    case SKILL_QUIV_PALM:
      return &((CDFAttacks *) cd)->skQuiveringPalm;
    case SKILL_CRIT_HIT:
      return &((CDFAttacks *) cd)->skCriticalHitting;


// THIEF CLASS

// disc_thief

    case SKILL_SWINDLE:
      return &((CDThief *) cd)->skSwindle;
    case SKILL_SNEAK: //                     483
      return &((CDThief *) cd)->skSneak;
    case SKILL_STABBING: //                  484
      return &((CDThief *) cd)->skStabbing;
    case SKILL_RETREAT_THIEF: //             485
      return &((CDThief *) cd)->skRetreatThief;
    case SKILL_KICK_THIEF:  //                486
      return &((CDThief *) cd)->skKickThief;
    case SKILL_PICK_LOCK:   //                 487
      return &((CDThief *) cd)->skPickLock;
    case SKILL_BACKSTAB:  //                  487
      return &((CDThief *) cd)->skBackstab;
    case SKILL_SEARCH:  //                    489
      return &((CDThief *) cd)->skSearch;
    case SKILL_SPY:  //                       490
      return &((CDThief *) cd)->skSpy;
    case SKILL_SWITCH_THIEF:  //              491
      return &((CDThief *) cd)->skSwitchThief;
    case SKILL_STEAL:  //                     492
      return &((CDThief *) cd)->skSteal;
    case SKILL_DETECT_TRAP: //               493
      return &((CDThief *) cd)->skDetectTraps;
    case SKILL_SUBTERFUGE:  //                494
      return &((CDThief *) cd)->skSubterfuge;
    case SKILL_DISARM_TRAP:  //               495
      return &((CDThief *) cd)->skDisarmTraps;
    case SKILL_CUDGEL: //                    496
      return &((CDThief *) cd)->skCudgel;
    case SKILL_HIDE:  //                      497
      return &((CDThief *) cd)->skHide;
    case SKILL_DISARM_THIEF:
      return &((CDThief *) cd)->skDisarmThief;
    case SKILL_REPAIR_THIEF:
      return &((CDThief *) cd)->skRepairThief;
    case SKILL_TRACK:  //                      339
      return &((CDThief *) cd)->skTrack;


// disc_fighting_thief

    case SKILL_DODGE_THIEF:  //            498
      return &((CDThiefFight *) cd)->skDodgeThief;
    case SKILL_DUAL_WIELD_THIEF: //          503
      return &((CDThiefFight *) cd)->skDualWieldThief;


// disc_looting
    case SKILL_COUNTER_STEAL:
      return &((CDLooting *) cd)->skCounterSteal;
    case SKILL_PLANT:
      return &((CDLooting *) cd)->skPlant;

// disc_murder

    case SKILL_THROATSLIT:
      return &((CDMurder *) cd)->skThroatSlit;
    case SKILL_GARROTTE:  //                  501
      return &((CDMurder *) cd)->skGarrotte;

// disc_poisons

    case SKILL_POISON_WEAPON:  //            498
      return &((CDPoisons *) cd)->skPoisonWeapons;

// disc_stealth

    case SKILL_CONCEALMENT:  //                381
      return &((CDStealth *) cd)->skConcealment;
    case SKILL_DISGUISE:  //                  499
      return &((CDStealth *) cd)->skDisguise;

// disc_traps
    case SKILL_SET_TRAP_CONT:
      return &((CDTraps *) cd)->skSetTrapsCont;
    case SKILL_SET_TRAP_DOOR:
      return &((CDTraps *) cd)->skSetTrapsDoor;
    case SKILL_SET_TRAP_MINE:
      return &((CDTraps *) cd)->skSetTrapsMine;
    case SKILL_SET_TRAP_GREN:
      return &((CDTraps *) cd)->skSetTrapsGren;
    case SKILL_SET_TRAP_ARROW:
      return &((CDTraps *) cd)->skSetTrapsArrow;

    case SPELL_TREE_WALK:
      return &((CDNature *) cd)->skTreeWalk;

      // SHAMAN **********************************************************

      // basic shaman----------------------

    case SPELL_FLATULENCE:
      return &((CDShaman *) cd)->skFlatulence;
    case SPELL_SQUISH:
      return &((CDShaman *) cd)->skSquish;
    case SPELL_STUPIDITY:
      return &((CDShaman *) cd)->skStupidity;
    case SPELL_DISTORT:
      return &((CDShaman *) cd)->skDistort;
    case SPELL_SOUL_TWIST:
      return &((CDShaman *) cd)->skSoulTwist;
    case SPELL_LIFE_LEECH:
      return &((CDShaman *) cd)->skLifeLeech;
    case SPELL_INTIMIDATE:
      return &((CDShaman *) cd)->skIntimidate;
    case SPELL_DJALLA:
      return &((CDShaman *) cd)->skDjallasProtection;
    case SPELL_LEGBA:
      return &((CDShaman *) cd)->skLegbasGuidance;
    case SPELL_SENSE_LIFE_SHAMAN:
      return &((CDShaman *) cd)->skSenseLifeShaman;
    case SPELL_DETECT_SHADOW:
      return &((CDShaman *) cd)->skDetectShadow;
    case SPELL_ROMBLER:
      return &((CDShaman *) cd)->skRombler;
    case SPELL_CHEVAL:
      return &((CDShaman *) cd)->skCheval;
    case SPELL_CHASE_SPIRIT:
      return &((CDShaman *) cd)->skChaseSpirits;
    case SPELL_CHRISM:
      return &((CDShaman *) cd)->skChrism;
    case SPELL_VAMPIRIC_TOUCH: // 480
      return &((CDShaman *) cd)->skVampiricTouch;
    case SPELL_SHIELD_OF_MISTS: 
      return &((CDShaman *) cd)->skShieldOfMists;
    case SKILL_SACRIFICE: 
      return &((CDShaman *) cd)->skSacrifice;
    case SPELL_ENTHRALL_SPECTRE:
      return &((CDShaman *) cd)->skEnthrallSpectre;
    case SPELL_ENTHRALL_GHAST:
      return &((CDShaman *) cd)->skEnthrallGhast;
    case SPELL_DANCING_BONES: // 402
      return &((CDShaman *) cd)->skDancingBones;
    case SPELL_VOODOO: // 405
      return &((CDShaman *) cd)->skVoodoo;
    case SPELL_ENTHRALL_GHOUL:
      return &((CDShaman *) cd)->skEnthrallGhoul;
    case SKILL_REPAIR_SHAMAN:
      return &((CDShaman *) cd)->skRepairShaman;
    case SPELL_EMBALM:
      return &((CDShaman *) cd)->skEmbalm;

      // spider abilities----------------------

    case SPELL_RAZE: 
      return &((CDShamanSpider *) cd)->skRaze;
    case SPELL_STICKS_TO_SNAKES:  //           361
      return &((CDShamanSpider *) cd)->skSticksToSnakes;
    case SPELL_CLARITY:  // 546
      return &((CDShamanSpider *) cd)->skClarity;
    case SPELL_HYPNOSIS:
      return &((CDShamanSpider *) cd)->skHypnosis;
    case SPELL_CONTROL_UNDEAD: // 403
      return &((CDShamanSpider *) cd)->skControlUndead;
    case SKILL_TRANSFIX:  //                   345
      return &((CDShamanSpider *) cd)->skTransfix;
    case SPELL_ROOT_CONTROL:  //               340
      return &((CDShamanSpider *) cd)->skRootControl;
    case SPELL_LIVING_VINES:  //               348
      return &((CDShamanSpider *) cd)->skLivingVines;


      // skunk abilities------------------------

    case SPELL_BLOOD_BOIL:
      return &((CDShamanSkunk *) cd)->skBloodBoil;
    case SPELL_LICH_TOUCH: // 543
      return &((CDShamanSkunk *) cd)->skLichTouch;
    case SPELL_DEATH_MIST: // 544
      return &((CDShamanSkunk *) cd)->skDeathMist;
    case SKILL_TURN:
      return &((CDShamanSkunk *) cd)->skTurnSkill;
    case SPELL_CARDIAC_STRESS:
      return &((CDShamanSkunk *) cd)->skCardiacStress;
    case SPELL_CLEANSE: 
      return &((CDShamanSkunk *) cd)->skCleanse;

      // frog abilities--------------------------

    case SPELL_STORMY_SKIES:  //               362
      return &((CDShamanFrog *) cd)->skStormySkies;
    case SPELL_SHAPESHIFT:  //                 372
      return &((CDShamanFrog *) cd)->skShapeShift;
    case SPELL_AQUATIC_BLAST: 
      return &((CDShamanFrog *) cd)->skAquaticBlast;
    case SPELL_DEATHWAVE: 
      return &((CDShamanFrog *) cd)->skDeathWave;
    case SKILL_TRANSFORM_LIMB:  //             337
      return &((CDShamanFrog *) cd)->skTransformLimb;
    case SPELL_CREEPING_DOOM:
      return &((CDShamanFrog *) cd)->skCreepingDoom;

      // armadillo abilities----------------------

    case SPELL_CELERITE:
      return &((CDShamanArmadillo *) cd)->skCelerite;
    case SPELL_SHADOW_WALK:  // 545
      return &((CDShamanArmadillo *) cd)->skShadowWalk;
    case SPELL_AQUALUNG: 
      return &((CDShamanArmadillo *) cd)->skAqualung;
    case SPELL_THORNFLESH:
      return &((CDShamanArmadillo *) cd)->skThornflesh;
    case SPELL_EARTHMAW:
      return &((CDShamanArmadillo *) cd)->skEarthmaw;

      // control abilities--------------------------

    case SPELL_ENTHRALL_DEMON:
      return &((CDShamanControl *) cd)->skEnthrallDemon;
    case SPELL_CREATE_WOOD_GOLEM:
      return &((CDShamanControl *) cd)->skCreateWoodGolem;
    case SPELL_CREATE_ROCK_GOLEM:
      return &((CDShamanControl *) cd)->skCreateRockGolem;
    case SPELL_CREATE_IRON_GOLEM:
      return &((CDShamanControl *) cd)->skCreateIronGolem;
    case SPELL_CREATE_DIAMOND_GOLEM:
      return &((CDShamanControl *) cd)->skCreateDiamondGolem;
    case SPELL_RESURRECTION: // 404
      return &((CDShamanControl *) cd)->skResurrection;

      // shaman alchemy------------------------------

    case SKILL_BREW: // 405
      return &((CDShamanAlchemy *) cd)->skBrew;

      // healing abilities---------------------------

    case SPELL_HEALING_GRASP:
      return &((CDShamanHealing *) cd)->skHealingGrasp;
    case SPELL_ENLIVEN:
      return &((CDShamanHealing *) cd)->skEnliven;

      // ritualism

    case SKILL_RITUALISM:
      return &((CDRitualism *) cd)->skRitualism;

      // SHAMAN END *****************************************************


// GENERAL DISCIPLINES

// advanced adventuring
    case SKILL_HIKING: //                      331
      return &((CDAdvAdventuring *) cd)->skHiking;
    case SKILL_FORAGE:  //                     335
      return &((CDAdvAdventuring *) cd)->skForage;
    case SKILL_SEEKWATER:  //                  336
      return &((CDAdvAdventuring *) cd)->skSeekWater;
    case SKILL_SKIN:  //                       346
      return &((CDAdvAdventuring *) cd)->skSkin;
    case SKILL_DIVINATION:  //                391
      return &((CDAdvAdventuring *) cd)->skDivination;
    case SKILL_ENCAMP:  //                    393
      return &((CDAdvAdventuring *) cd)->skEncamp;
    case SKILL_FISHLORE:
      return &((CDAdvAdventuring *) cd)->skFishlore;
    case SKILL_TROLLISH:
      return &((CDAdvAdventuring *) cd)->skTrollish;
    case SKILL_BULLYWUGCROAK:
      return &((CDAdvAdventuring *) cd)->skBullywug;
    case SKILL_AVIAN:
      return &((CDAdvAdventuring *) cd)->skAvian;
    case SKILL_FISHBURBLE:
      return &((CDAdvAdventuring *) cd)->skKalysian;

      // adventuring
    case SKILL_ALCOHOLISM:// 668
      return &((CDAdventuring *) cd)->skAlcoholism;
    case SKILL_FISHING: // 669
      return &((CDAdventuring *) cd)->skFishing;
    case SKILL_RIDE: // 760
      return &((CDAdventuring *) cd)->skRide;
    case SKILL_SIGN: // 761
      return &((CDAdventuring *) cd)->skSign;
    case SKILL_SWIM: // 762
      return &((CDAdventuring *) cd)->skSwim;
    case SKILL_CONS_UNDEAD: // 763
      return &((CDAdventuring *) cd)->skKnowUndead;
    case SKILL_CONS_VEGGIE: // 764
      return &((CDAdventuring *) cd)->skKnowVeggie;
    case SKILL_CONS_DEMON: // 765
      return &((CDAdventuring *) cd)->skKnowDemon;
    case SKILL_CONS_ANIMAL: // 766
      return &((CDAdventuring *) cd)->skKnowAnimal;
    case SKILL_CONS_REPTILE: // 767
      return &((CDAdventuring *) cd)->skKnowReptile;
    case SKILL_CONS_PEOPLE: // 768
      return &((CDAdventuring *) cd)->skKnowPeople;
    case SKILL_CONS_GIANT: // 769
      return &((CDAdventuring *) cd)->skKnowGiant;
    case SKILL_CONS_OTHER: // 770
      return &((CDAdventuring *) cd)->skKnowOther;
    case SKILL_READ_MAGIC: // 771
      return &((CDAdventuring *) cd)->skReadMagic;
    case SKILL_BANDAGE: // 772
      return &((CDAdventuring *) cd)->skBandage;
    case SKILL_CLIMB: // 773
      return &((CDAdventuring *) cd)->skClimb;
    case SKILL_FAST_HEAL: // 774
      return &((CDAdventuring *) cd)->skFastHeal;
    case SKILL_EVALUATE: // 780
      return &((CDAdventuring *) cd)->skEvaluate;
    case SKILL_TACTICS: // 781
      return &((CDAdventuring *) cd)->skTactics;
    case SKILL_DISSECT: // 783
      return &((CDAdventuring *) cd)->skDissect;
    case SKILL_DEFENSE: // 593 
      return &((CDAdventuring *) cd)->skDefense;
    case SKILL_OFFENSE: // 594
      return &((CDAdventuring *) cd)->skOffense;
    case SKILL_WHITTLE: // 663
      return &((CDAdventuring *) cd)->skWhittle;
    case SKILL_MEND:
      return &((CDAdventuring *) cd)->skMend;
    case SKILL_BUTCHER:
      return &((CDAdventuring *) cd)->skButcher;
    case SKILL_LOGGING:
      return &((CDAdventuring *) cd)->skLogging;
    case SKILL_GUTTER_CANT:
      return &((CDAdventuring *) cd)->skGutterCant;
    case SKILL_GNOLL_JARGON:
      return &((CDAdventuring *) cd)->skGnollJargon;
    case SKILL_TROGLODYTE_PIDGIN:
      return &((CDAdventuring *) cd)->skTroglodytePidgin;

  // disc_wizardry
    case SKILL_WIZARDRY: // 960
      return &((CDWizardry *) cd)->skWizardry;

// disc_lore
    case SKILL_MEDITATE: // 961
      return &((CDLore *) cd)->skMeditate;
    case SKILL_MANA: // 962
      return &((CDLore *) cd)->skMana;

  // disc_faith
    case SKILL_DEVOTION: // 1000
      return &((CDFaith *) cd)->skDevotion;

// disc_theology

    case SKILL_PENANCE: // 1001
      return &((CDTheology *) cd)->skPenance;
    case SKILL_ATTUNE: // 1001
      return &((CDTheology *) cd)->skAttune;



  // disc_slash
    case SKILL_SLASH_SPEC:  // 1044
      return &((CDSlash *) cd)->skSlashSpec;
  // disc_bash
    case SKILL_BLUNT_SPEC:  // 1096
      return &((CDBash *) cd)->skBluntSpec;
 // disc_pierce
    case SKILL_PIERCE_SPEC:  // 1126
      return &((CDPierce *) cd)->skPierceSpec;

  // disc_ranged
    case SKILL_RANGED_SPEC:  // 1164
      return &((CDRanged *) cd)->skRangedSpec;
    case SKILL_FAST_LOAD:  // 1165
      return &((CDRanged *) cd)->skFastLoad;

 // disc_barehand
    case SKILL_BAREHAND_SPEC:
      return &((CDBarehand *) cd)->skBarehandSpec;   

// disc_defense
    case SKILL_ADVANCED_DEFENSE: // 674
      return &((CDDefense *) cd)->skAdvancedDefense;

// disc_psionics
    case SKILL_PSITELEPATHY:
      return &((CDPsionics *) cd)->skTelepathy;
    case SKILL_TELE_SIGHT:
      return &((CDPsionics *) cd)->skTeleSight;
    case SKILL_TELE_VISION:
      return &((CDPsionics *) cd)->skTeleVision;
    case SKILL_MIND_FOCUS:
      return &((CDPsionics *) cd)->skMindFocus;
    case SKILL_PSI_BLAST:
      return &((CDPsionics *) cd)->skPsiBlast;
    case SKILL_MIND_THRUST:
      return &((CDPsionics *) cd)->skMindThrust;
    case SKILL_PSYCHIC_CRUSH:
      return &((CDPsionics *) cd)->skPsyCrush;
    case SKILL_KINETIC_WAVE:
      return &((CDPsionics *) cd)->skKineticWave;
    case SKILL_MIND_PRESERVATION:
      return &((CDPsionics *) cd)->skMindPreservation;
    case SKILL_TELEKINESIS:
      return &((CDPsionics *) cd)->skTelekinesis;
    case SKILL_PSIDRAIN:
      return &((CDPsionics *) cd)->skPsiDrain;




  // disc_combat
    case SKILL_SLASH_PROF: // 1521
      return &((CDCombat *) cd)->skSlash;
    case SKILL_RANGED_PROF: // 1522
      return &((CDCombat *) cd)->skBow;
    case SKILL_PIERCE_PROF: // 1523
      return &((CDCombat *) cd)->skPierce;
    case SKILL_BLUNT_PROF: // 1524
      return &((CDCombat *) cd)->skBlunt;
    case SKILL_SHARPEN:
      return &((CDCombat *) cd)->skSharpen;
    case SKILL_DULL:
      return &((CDCombat *) cd)->skDull;
    case SKILL_BAREHAND_PROF:
      return &((CDCombat *) cd)->skBarehand;
    case TYPE_UNDEFINED:
    case MAX_SKILL:
    case DAMAGE_NORMAL:
    case DAMAGE_RIPPED_OUT_HEART:
    case DAMAGE_CAVED_SKULL:
    case DAMAGE_BEHEADED:
    case DAMAGE_DISEMBOWLED_HR:
    case DAMAGE_DISEMBOWLED_VR:
    case DAMAGE_STOMACH_WOUND:
    case DAMAGE_HACKED:
    case DAMAGE_IMPALE:
    case DAMAGE_STARVATION:
    case DAMAGE_FALL:
    case DAMAGE_HEMORRAGE:
    case DAMAGE_DROWN:
    case DAMAGE_DRAIN:
    case DAMAGE_DISRUPTION:
    case DAMAGE_SUFFOCATION:
    case DAMAGE_RAMMED:
    case DAMAGE_WHIRLPOOL:
    case DAMAGE_ELECTRIC:
    case DAMAGE_ACID:
    case DAMAGE_GUST:
    case DAMAGE_EATTEN:
    case DAMAGE_KICK_HEAD:
    case DAMAGE_KICK_SOLAR:
    case DAMAGE_HEADBUTT_THROAT:
    case DAMAGE_HEADBUTT_BODY:
    case DAMAGE_HEADBUTT_CROTCH:
    case DAMAGE_HEADBUTT_LEG:
    case DAMAGE_HEADBUTT_FOOT:
    case DAMAGE_HEADBUTT_JAW:
    case DAMAGE_TRAP_SLEEP:
    case DAMAGE_TRAP_TELEPORT:
    case DAMAGE_TRAP_FIRE:
    case DAMAGE_TRAP_POISON:
    case DAMAGE_TRAP_ACID:
    case DAMAGE_TRAP_TNT:
    case DAMAGE_TRAP_ENERGY:
    case DAMAGE_TRAP_BLUNT:
    case DAMAGE_TRAP_PIERCE:
    case DAMAGE_TRAP_SLASH:
    case DAMAGE_TRAP_FROST:
    case DAMAGE_TRAP_DISEASE:
    case DAMAGE_ARROWS:
    case DAMAGE_FIRE:
    case DAMAGE_FROST:
    case DAMAGE_HEADBUTT_SKULL:
    case DAMAGE_COLLISION:
    case DAMAGE_KICK_SHIN:
    case DAMAGE_KNEESTRIKE_FOOT:
    case DAMAGE_KNEESTRIKE_SHIN:
    case DAMAGE_KNEESTRIKE_KNEE:
    case DAMAGE_KNEESTRIKE_THIGH:
    case DAMAGE_KNEESTRIKE_CROTCH:
    case DAMAGE_KNEESTRIKE_SOLAR:
    case DAMAGE_KNEESTRIKE_CHIN:
    case DAMAGE_KNEESTRIKE_FACE:
    case DAMAGE_KICK_SIDE:
    case TYPE_HIT:
    case TYPE_BLUDGEON:
    case TYPE_WHIP:
    case TYPE_CRUSH:
    case TYPE_SMASH:
    case TYPE_SMITE:
    case TYPE_PUMMEL:
    case TYPE_FLAIL:
    case TYPE_BEAT:
    case TYPE_THRASH:
    case TYPE_THUMP:
    case TYPE_WALLOP:
    case TYPE_BATTER:
    case TYPE_STRIKE:
    case TYPE_CLUB:
    case TYPE_POUND:
    case TYPE_PIERCE:
    case TYPE_BITE:
    case TYPE_STING:
    case TYPE_STAB:
    case TYPE_THRUST:
    case TYPE_SPEAR:
    case TYPE_BEAK:
    case TYPE_SLASH:
    case TYPE_CLAW:
    case TYPE_CLEAVE:
    case TYPE_SLICE:
    case TYPE_AIR:
    case TYPE_EARTH:
    case TYPE_FIRE:
    case TYPE_WATER:
    case TYPE_BEAR_CLAW:
    case TYPE_KICK:
    case TYPE_MAUL:
    case TYPE_SHOOT:
    case TYPE_CANNON:
    case TYPE_SHRED:
    case TYPE_MAX_HIT:
    case AFFECT_DISEASE:
    case AFFECT_COMBAT:
    case AFFECT_TRANSFORMED_HANDS:
    case AFFECT_TRANSFORMED_ARMS:
    case AFFECT_TRANSFORMED_LEGS:
    case AFFECT_TRANSFORMED_HEAD:
    case AFFECT_TRANSFORMED_NECK:
    case LAST_TRANSFORMED_LIMB:
    case AFFECT_DUMMY:
    case AFFECT_WAS_INDOORS:
    case AFFECT_DRUNK:
    case AFFECT_NEWBIE:
    case AFFECT_SKILL_ATTEMPT:
    case AFFECT_FREE_DEATHS:
    case AFFECT_TEST_FIGHT_MOB:
    case AFFECT_DRUG:
    case SPELL_FIRE_BREATH:
    case SPELL_CHLORINE_BREATH:
    case SPELL_FROST_BREATH:
    case SPELL_ACID_BREATH:
    case SPELL_LIGHTNING_BREATH:
    case SPELL_DUST_BREATH:
    case LAST_BREATH_WEAPON:
    case AFFECT_PET:
    case AFFECT_CHARM:
    case AFFECT_THRALL:
    case AFFECT_ORPHAN_PET:
    case AFFECT_PLAYERKILL:
    case AFFECT_PLAYERLOOT:
    case AFFECT_HORSEOWNED:
    case AFFECT_GROWTH_POTION:
    case AFFECT_WARY:
    case AFFECT_DEFECTED:
    case AFFECT_OFFER:
    case AFFECT_OBJECT_USED:
    case LAST_ODDBALL_AFFECT:
    case AFFECT_BITTEN_BY_VAMPIRE:
    case AFFECT_IMMORTAL_BLESSING:
    case AFFECT_PEEL_BLESSING:
    case AFFECT_ANGUS_BLESSING:
    case AFFECT_DAMESCENA_BLESSING:
    case AFFECT_JESUS_BLESSING:
    case AFFECT_VASCO_BLESSING:
    case AFFECT_CORAL_BLESSING:
    case AFFECT_BUMP_BLESSING:
    case AFFECT_MAROR_BLESSING:
    case AFFECT_DASH_BLESSING:
    case AFFECT_DEIRDRE_BLESSING:
    case AFFECT_GARTHAGK_BLESSING:
    case AFFECT_MERCURY_BLESSING:
    case AFFECT_METROHEP_BLESSING:
    case AFFECT_MAGDALENA_BLESSING:
    case AFFECT_MACROSS_BLESSING:
    case AFFECT_PAPPY_BLESSING:
    case AFFECT_STAFFA_BLESSING:
    case AFFECT_PREENED:
    case AFFECT_WET:
    case ABSOLUTE_MAX_SKILL:
      break;
  }


  if (discArray[skill] == NULL) {
    vlogf(LOG_BUG,format("Bad skill number: %d in getSkill (NADA)") % skill);
  } else if (*discArray[skill]->name) {
    if (strcmp(discArray[skill]->name,"\n"))
      vlogf(LOG_BUG,format("Bad skill number: %d in getSkill (%s)") % skill % discArray[skill]->name);
  }
  return (NULL);
}

bool TBeing::doesKnowSkill(spellNumT skill) const
{
  CSkill *sk = NULL;
  if (!(sk = getSkill(skill)))
    return FALSE;

  return doesKnow(getMaxSkillValue(skill));
}

short TBeing::getRawSkillValue(spellNumT skill) const
{
  CSkill *sk;

  if (!(sk = getSkill(skill)))
    return SKILL_MIN;

  return sk->getLearnedness();
}

short TBeing::getSkillValue(spellNumT skill) const
{
  int value;
  int iMax;

  CSkill *sk;

  if (!(sk = getSkill(skill)))
    return SKILL_MIN;

  iMax = getMaxSkillValue(skill);
  value = getRawSkillValue(skill);
//  value = sk->getLearnedness();
  if ((value == 255) || (value == SKILL_MIN))
    return SKILL_MIN;
  value = min(value, iMax);
#if 1 
  int applyAmt = 0;
  skillApplyData *temp = NULL;

  for (temp = skillApplys; temp; temp = temp->nextApply) {
    if (temp->skillNum == skill) {
      applyAmt = temp->amount;
      break;
    }
  }
  value += applyAmt;
//  value += getSkillApply(skill);
  value = min(value, (int) MAX_SKILL_LEARNEDNESS);
#endif
  value = max(value, 0);
  return value;
}

void TBeing::setSkillValue(spellNumT skill, int lValue)
{
  byte newValue;
  CSkill *sk;

  if (lValue > 120)
    newValue = 120;
  else if (lValue < 0)
    newValue = -1;
  else
    newValue = (byte) lValue;

  if (!(sk = getSkill(skill)))
    return;

  sk->setLearnedness(newValue);
}

short TBeing::getRawNatSkillValue(spellNumT skill) const
{
  CSkill *sk;

  if (!(sk = getSkill(skill)))
    return SKILL_MIN;

  return sk->getNatLearnedness();
}

int TBeing::getAdvDoLearning(spellNumT skill) const
{
  CSkill *sk;
  int ret = -1;
  CDiscipline *assDisc = NULL;

  skill = getSkillNum(skill);

  if (!(sk = getSkill(skill)))
    return 0;

  if (discArray[skill] && *discArray[skill]->name) {
    if ((assDisc = getDiscipline(discArray[skill]->assDisc))) {
      ret = assDisc->getDoLearnedness();
      return ret;
    } else {
      vlogf(LOG_BUG, format("Someone (%s) doesnt have a valid associated discipline for skill (%d)") %  getName() % skill);
      return 0;
    }
  }
  return 0;
}


int TBeing::getAdvLearning(spellNumT skill) const
{
  CSkill *sk;
  CDiscipline *assDisc = NULL;
  int ret = -1;

  skill = getSkillNum(skill);

  if (!(sk = getSkill(skill)))
    return 0;

  if (discArray[skill] && *discArray[skill]->name) {
    if ((assDisc = getDiscipline(discArray[skill]->assDisc))){
      ret = assDisc->getLearnedness();
      return ret;
    } else {
      vlogf(LOG_BUG,format("Someone (%s) doesnt have a valid associated discipline for skill (%d)") %  getName() % skill);
      return 0;
    }
  }
  return 0;
}

short TBeing::getNatSkillValue(spellNumT skill) const
{
  int value;
  int iMax;

  CSkill *sk;

  if (!(sk = getSkill(skill)))
    return SKILL_MIN;
  
  iMax = getMaxSkillValue(skill);
//  value = sk->getNatLearnedness();
  value = getRawNatSkillValue(skill);
  value = min(value, iMax);

  if (value < 0) {
    return SKILL_MIN;
  } else if (value == 0) {
    vlogf(LOG_BUG, format("%s showed up with 0 learning in a skill (%d)") %  getName() % skill);
    return SKILL_MIN;
  } else {
    value = min((int) MAX_SKILL_LEARNEDNESS, value);
    return value;
  }
//  return (min(sk->getNatLearnedness(), getMaxSkillValue(skill)));
}

void TBeing::setNatSkillValue(spellNumT skill, int lValue)
{
  byte newValue;
  CSkill *sk;

  if (lValue > 120)
    newValue = 120;
  else if (lValue < 0)
    newValue = -1;
  else
    newValue = (byte) lValue;

  if (!(sk = getSkill(skill)))
    return;

  sk->setNatLearnedness(newValue);
}

CSkill::CSkill()
  : lastUsed(time(0))
{
  setLearnedness(SKILL_MIN);
  setNatLearnedness(SKILL_MIN);
}

CSkill::CSkill(const CSkill &a)
  : value(a.value),
    lastUsed(a.lastUsed)
{
}

CSkill & CSkill::operator=(const CSkill &a)
{
  if (this == &a) return *this;
  value = a.value;
  lastUsed = a.lastUsed;
  return *this;
}

CSkill::~CSkill()
{
}

// this is a global function that  was originally developed to allow
// QuestCode toggling of spells, e.g. enable a spell at the end of a 
// a quest.
// it is intended to sit just inside a for(...i < MAX_SKILL; ...)
// It seems like a good place to check for the global obvious conditions
// too.
bool hideThisSpell(spellNumT spell)
{
  if (!discArray[spell] || !*discArray[spell]->name)
    return true;

  if (spell == SPELL_ENSORCER)
    return true;

  if (spell == SPELL_GARMULS_TAIL)
    return true;

  if (spell == SPELL_ETHER_GATE)
    return true;

  return false;
}
