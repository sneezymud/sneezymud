//////////////////////////////////////////////////////////////////////////
//
// SneezyMUD - All rights reserved, SneezyMUD Coding Team
//
// "makecorpse.cc" - All functions and routines related to corpse creation
//
//////////////////////////////////////////////////////////////////////////

#include "room.h"
#include "being.h"
#include "handler.h"
#include "low.h"
#include "extern.h"
#include "monster.h"
#include "configuration.h"
#include "combat.h"
#include "obj_component.h"
#include "obj_base_corpse.h"
#include "obj_player_corpse.h"
#include "obj_corpse.h"
#include "obj_money.h"
#include "materials.h"

TThing * TBeing::makeCorpse(spellNumT dmg_type, TBeing * tKiller, float exp_lost)
{
  TMoney *money;
  TThing *o;
  TRoom *rp;
  char buf[MAX_INPUT_LENGTH], tmpbuf[MAX_NAME_LENGTH], namebuf[MAX_NAME_LENGTH];
  bool specialCorpse = FALSE;
  int j;
  TCorpse *corpse = NULL;
  TPCorpse *pcorpse = NULL;
  TBaseCorpse *gen_corpse;

  if (specials.was_in_room != Room::NOWHERE) {
    // character was in void
    // bleeding, infected, etc can cause this

    vlogf(LOG_BUG, format("Character: %s in void when corpse made.  Being moved to room #%d.") %  getName() % specials.was_in_room);
    --(*this);
    rp = real_roomp(specials.was_in_room);
    *rp += *this;
  }

  if (isPc() && !desc && !affectedBySpell(AFFECT_PLAYERKILL)) {
    vlogf(LOG_BUG, format("Character: %s with no link when creating corpse in rm %d") %  getName() % in_room);
    vlogf(LOG_BUG, format("%s corpse generated in room %d to avoid possible duplication") %  getName() % Room::STORAGE);
    --(*this);
    rp = real_roomp(Room::STORAGE);
    *rp += *this;
  }
  sprintf(tmpbuf, "%s", getName());
  strcpy(namebuf, sstring(tmpbuf).cap().c_str());

  if ((desc || isPc()) && (GetMaxLevel() <= MAX_MORT)) {
    pcorpse = race->makePCorpse();
    gen_corpse = pcorpse;
    pcorpse->setCorpseVnum(-2);  // flag for pc
    pcorpse->addCorpseFlag(CORPSE_NO_REGEN);

    if(isPlayerAction(PLR_DENY_LOOT))
      pcorpse->addCorpseFlag(CORPSE_DENY_LOOT);
    else
      pcorpse->remCorpseFlag(CORPSE_DENY_LOOT);

  } else {
    corpse = race->makeCorpse();
    gen_corpse = corpse;
    corpse->setCorpseVnum(mobVnum());

    if (mobVnum() < 0)
      corpse->addCorpseFlag(CORPSE_NO_REGEN);
  }
  
  gen_corpse->setCorpseFlags(0);
  gen_corpse->setCorpseLevel(GetMaxLevel());
  gen_corpse->addObjStat(ITEM_STRUNG);
  
  gen_corpse->ex_description = NULL;
  gen_corpse->action_description = NULL;

  // Lets make some customized "corpses" for elementals - Brutius 07/26/1999
  if (getMyRace()->getBodyType() == BODY_ELEMENTAL) {
    if (isname("[fire]", name)) {
      sprintf(buf, "cinders pile %s",name);
      gen_corpse->name = mud_str_dup(buf);
      gen_corpse->setDescr(mud_str_dup("A <r>smoldering pile of cinders<1> is here."));
      gen_corpse->shortDescr = mud_str_dup("a <r>smoldering pile of cinders<1>");
      gen_corpse->addCorpseFlag(CORPSE_NO_REGEN);
      gen_corpse->setMaterial(MAT_POWDER);
    } else if (isname("[water]", name)) {
      sprintf(buf, "water puddle %s",name);
      gen_corpse->name = mud_str_dup(buf);
      gen_corpse->setDescr(mud_str_dup("A <b>stagnant puddle of water<1> is here."));
      gen_corpse->shortDescr = mud_str_dup("a <b>stagnant puddle of water<1>");
      gen_corpse->addCorpseFlag(CORPSE_NO_REGEN);
      gen_corpse->setMaterial(MAT_WATER);
    } else if (isname("[earth]", name)) {
      sprintf(buf, "pile dirt %s",name);
      gen_corpse->name = mud_str_dup(buf);
      gen_corpse->setDescr(mud_str_dup("A lifeless pile of dirt is here."));
      gen_corpse->shortDescr = mud_str_dup("a lifeless pile of dirt");
      gen_corpse->addCorpseFlag(CORPSE_NO_REGEN);
      gen_corpse->setMaterial(MAT_POWDER);
    } else if (isname("[air]", name)) {
      sprintf(buf, "tornado natural small %s",name);
      gen_corpse->name = mud_str_dup(buf);
      gen_corpse->setDescr(mud_str_dup("A small uncontrolled tornado is here."));
      gen_corpse->shortDescr = mud_str_dup("a small uncontrolled tornado");
      gen_corpse->addCorpseFlag(CORPSE_NO_REGEN);
      gen_corpse->setMaterial(MAT_POWDER);
    } else {
      // non-standard elementals
      sprintf(buf, "corpse %s",name);
      gen_corpse->name = mud_str_dup(buf);
      gen_corpse->setDescr(mud_str_dup("The corpse of some sort of elemental is here."));
      gen_corpse->shortDescr = mud_str_dup("the corpse of an elemental");
      gen_corpse->addCorpseFlag(CORPSE_NO_REGEN);
    }
    specialCorpse = TRUE;
  } else if (dmg_type == SPELL_RAZE) {
    sprintf(buf, "green powder pile strange %s",name);
    gen_corpse->name = mud_str_dup(buf);
    gen_corpse->setDescr(mud_str_dup("A strange looking pile of green powder lies here."));
    gen_corpse->shortDescr = mud_str_dup("a pile of strange green powder");
    gen_corpse->addCorpseFlag(CORPSE_NO_REGEN);
    gen_corpse->setMaterial(MAT_POWDER);
    specialCorpse = TRUE;
  } else if (isUndead() || (dmg_type == SKILL_TURN)) {
    sprintf(buf, "dust pile %s",name);
    gen_corpse->name = mud_str_dup(buf);
    gen_corpse->setDescr(mud_str_dup("A pile of dust is here."));
    gen_corpse->shortDescr = mud_str_dup("a pile of dust");
    gen_corpse->addCorpseFlag(CORPSE_NO_REGEN);
    gen_corpse->setMaterial(MAT_POWDER);
    specialCorpse = TRUE;
  } else if (dmg_type == SPELL_ATOMIZE) {
    sprintf(buf, "ash pile %s",name);
    gen_corpse->name = mud_str_dup(buf);
    gen_corpse->setDescr(mud_str_dup("A pile of ash is here."));
    gen_corpse->shortDescr = mud_str_dup("a pile of ash");
    gen_corpse->addCorpseFlag(CORPSE_NO_REGEN);
    gen_corpse->setMaterial(MAT_POWDER);
    specialCorpse = TRUE;
  } else {
    sprintf(buf, "corpse %s%s", name, pcorpse ? " pcorpse" : "");
    gen_corpse->name = mud_str_dup(buf);
    // shortDescr and Desc are set below
    gen_corpse->setMaterial(getMaterial(WEAR_BODY));
  }

  if(!isname("corpse", gen_corpse->name)){
    sprintf(buf, "%s corpse", gen_corpse->name);
    delete gen_corpse->name;
    gen_corpse->name=mud_str_dup(buf);
  }


  if (!specialCorpse && roomp && 

      (roomp->isUnderwaterSector() || roomp->isWaterSector())) {
    sprintf(buf, "The bloated, water-filled corpse of %s is floating here.", getName());
  } else if (!specialCorpse) {
    switch (dmg_type) {
      case SPELL_GUST:
      case DAMAGE_GUST:
      case SPELL_DUST_STORM:
      case SPELL_TORNADO:
      case TYPE_AIR:
        sprintf(buf,
          "The skin on %s's corpse has been torn to shreds by a raging wind.",
          getName());
        break;
      case DAMAGE_TRAP_TNT:
        sprintf(buf,
          "The skin on %s's corpse has been torn to shreds by something explosive.",
          getName());
        break;
      case SPELL_DEATHWAVE:
      case SPELL_DISTORT:
      case SKILL_QUIV_PALM:
        sprintf(buf,
          "%s's corpse has been shattered from within by a powerful shockwave.",
          namebuf);
        break;
      case SKILL_SHOULDER_THROW:
         sprintf(buf, "%s's corpse lies shattered here, having been thrown hard to the ground.", namebuf); 
        break;
      case DAMAGE_ELECTRIC:
      case SPELL_CALL_LIGHTNING_DEIKHAN:
      case SPELL_CALL_LIGHTNING:
      case SPELL_LIGHTNING_BREATH:
        sprintf(buf, "The charred and burnt corpse of %s smolders here.",getName());
        break;
      case SPELL_GUSHER:
      case SPELL_AQUATIC_BLAST:
        sprintf(buf, "The dripping wet corpse of %s is here.",getName());
        break;
      case SPELL_ICY_GRIP:
      case SPELL_ARCTIC_BLAST:
      case SPELL_ICE_STORM:
      case SPELL_FROST_BREATH:
      case DAMAGE_TRAP_FROST:
      case DAMAGE_FROST:
        sprintf(buf, "The frozen solid corpse of %s is embedded in some ice here.",getName());
        break;
      case DAMAGE_DROWN:
      case SPELL_WATERY_GRAVE:
      case DAMAGE_WHIRLPOOL:
      case TYPE_WATER:
        sprintf(buf, "%s's corpse's face is blue, and the smell of brine comes from %s mouth.",namebuf, hshr());
        break;
      case SPELL_TSUNAMI:
        sprintf(buf, "The bloated, water-filled corpse of %s is here.",getName());
        break;
      case SPELL_CARDIAC_STRESS:
      case DAMAGE_HEMORRAGE:
        sprintf(buf, "The skin of %s's corpse is bright red, but no wound presents itself.",getName());
        break;
      case DAMAGE_IMPALE:
        sprintf(buf, "The torso of %s's corpse has a hole going all the way through.  Something must have impaled %s.",getName(),hmhr());
        break;
      case SPELL_CHLORINE_BREATH:
        sprintf(buf, "%s's corpse is here, melting with a wisp of caustic smoke.  The face looks strangled.",namebuf);
        break;
      case SPELL_DUST_BREATH:
        sprintf(buf, "%s's corpse has been shredded by some unseen force.", namebuf);
        break;
      case SPELL_POISON_DEIKHAN:
      case SPELL_POISON:
      case SPELL_DEATH_MIST:
      case DAMAGE_TRAP_POISON:
        sprintf(buf, "The sickly, green poisoned corpse of %s is here.",getName());
        break;
      case SPELL_ACID_BREATH:
      case DAMAGE_ACID:
      case SPELL_ACID_BLAST:
      case DAMAGE_TRAP_ACID:
        sprintf(buf, "The badly scalded corpse of %s sits in a puddle of acid.",getName());
        break;
      case SKILL_BODYSLAM:
        sprintf(buf, "The body of %s looks as if a great weight has squished %s.",getName(),hmhr());
        break;
      case SKILL_SPIN:
	sprintf(buf, "%s's corpse lies here with a dazed look.", getName());
	break;
      case DAMAGE_COLLISION:
        sprintf(buf, "%s's corpse lies shattered here, having collided with something massive.", namebuf);
        break;
      case DAMAGE_FALL:
        sprintf(buf, "%s's corpse lies shattered here, having fallen from a great height.", namebuf);
        break;
      case SKILL_CHARGE:
        sprintf(buf, "It appears %s was trampled to death as %s corpse is here.",getName(), hshr());
        break;
      case SKILL_SMITE:
        sprintf(buf, "Some powerful deity has smitten %s and %s shattered corpse lies here.", getName(), hshr());
        break;
      case SPELL_METEOR_SWARM:
      case SPELL_EARTHQUAKE_DEIKHAN:
      case SPELL_EARTHQUAKE:
      case DAMAGE_TRAP_BLUNT:
      case TYPE_EARTH:
        sprintf(buf, "The corpse of %s has been virtually crushed flat.",getName());
        break;
      case SPELL_PILLAR_SALT:
        sprintf(buf, "The crusty remains of %s has a salty sheen.",getName());
        break;
      case SPELL_FIREBALL:
      case SPELL_HANDS_OF_FLAME:
      case SPELL_INFERNO:
      case SPELL_HELLFIRE:
      case SPELL_RAIN_BRIMSTONE_DEIKHAN:
      case SPELL_RAIN_BRIMSTONE:
      case SPELL_FLAMESTRIKE:
      case SPELL_FIRE_BREATH:
      case SPELL_SPONTANEOUS_COMBUST:
      case SPELL_FLAMING_SWORD:
      case SPELL_BLOOD_BOIL:
      case DAMAGE_TRAP_FIRE:
      case TYPE_FIRE:
      case DAMAGE_FIRE:
      case SPELL_FLARE:
        sprintf(buf, "The charred remains of %s lies surrounded by soot.",getName());
        break;
      case DAMAGE_DISRUPTION:
      case SPELL_MYSTIC_DARTS:
      case SPELL_STUNNING_ARROW:
      case SPELL_COLOR_SPRAY:
       sprintf(buf, "Eldritch magic has coursed through %s's corpse here.",getName());
        break;
      case SPELL_STORMY_SKIES:
       sprintf(buf, "The weather eroded corpse of %s is here, charred and dripping wet.",getName());
        break;
      case SPELL_SAND_BLAST:
        sprintf(buf, "%s's corpse has thousands of tiny pinprick wounds on the exposed skin.", namebuf);
        break;
      case SPELL_PEBBLE_SPRAY:
        sprintf(buf, "%s's corpse has many small gory wounds on the exposed skin.", namebuf);
        break;
      case SPELL_LAVA_STREAM:
        sprintf(buf, "The charred remains of %s lies here, seared by lava.",
            getName());
        break;
      case SPELL_SLING_SHOT:
        sprintf(buf, "%s's forehead has been caved in by some massive force.", namebuf);
        break;
      case SPELL_GRANITE_FISTS:
        sprintf(buf, "It appears %s was punched incredibly hard by someone.", getName());
        break;
      case SPELL_ENERGY_DRAIN:
      case SPELL_LICH_TOUCH:
      case SPELL_VAMPIRIC_TOUCH:
      case SPELL_LIFE_LEECH:
      case SPELL_SYNOSTODWEOMER:
      case DAMAGE_DRAIN:
      case SPELL_HARM_DEIKHAN:
      case SPELL_HARM:
      case SPELL_HARM_LIGHT_DEIKHAN:
      case SPELL_HARM_SERIOUS_DEIKHAN:
      case SPELL_HARM_CRITICAL_DEIKHAN:
      case SPELL_HARM_LIGHT:
      case SPELL_HARM_SERIOUS:
      case SPELL_SOUL_TWIST:
      case SPELL_HARM_CRITICAL:
      case SPELL_WITHER_LIMB:
      case DAMAGE_TRAP_ENERGY:
        sprintf(buf, "The withered form of what used to be %s is here looking quite pale.",getName());
        break;
      case SPELL_BLEED:
        sprintf(buf, "The corpse of %s lies in a pool of blood.",getName());
        break;
      case SPELL_SQUISH:
        sprintf(buf, "The corpse of %s has been squished into a ball.", getName());
        break;
      case DAMAGE_KICK_HEAD:
        sprintf(buf, "The body of %s has a footprint on its forehead.", getName());
        break;
      case DAMAGE_KICK_SHIN:
        sprintf(buf, "The body of %s has a footprint near its shin.", getName());
        break;
      case DAMAGE_KICK_SIDE:
        sprintf(buf, "The body of %s has a footprint in its side.",getName());
        break;
      case DAMAGE_KICK_SOLAR:
      case SKILL_KICK_DEIKHAN:
      case SKILL_KICK_THIEF:
      case SKILL_KICK_MONK:
      case SKILL_KICK:
      case SKILL_SPRINGLEAP:
      case TYPE_KICK:
        sprintf(buf, "The body of %s has a footprint in its solar plexus.",getName());
        break;
      case SKILL_DEATHSTROKE:
        sprintf(buf, "The body of %s has had many vital organs disembowled from it.",getName());
        break;
      case SKILL_BASH_DEIKHAN:
      case SKILL_BASH:
      case SKILL_TRIP:
        sprintf(buf, "The corpse of %s has a broken nose from falling down too hard.",getName());
        break;
      case SPELL_BONE_BREAKER:
      case SKILL_BONEBREAK:
        sprintf(buf, "The twisted and mangled corpse of %s lies in a heap.",getName());
        break;
      case SPELL_PARALYZE:
      case SPELL_PARALYZE_LIMB:
        sprintf(buf, "The corpse of %s looks stiff from paralysis even before rigor-mortis has begun to set in.",getName());
        break;
      case SPELL_INFECT_DEIKHAN:
      case SPELL_INFECT:
        sprintf(buf, "A strange infection oozes out of the wounds in %s's corpse.",getName());
        break;
      case SKILL_CHOP:
        sprintf(buf, "It appears someone beat %s to death with their bare hands.", getName());
        break;
      case SPELL_DISEASE:
      case DAMAGE_TRAP_DISEASE:
        sprintf(buf, "%s's corpse is here.  It appears %s was consumed by disease.",namebuf, hssh());
        break;
      case SPELL_FLATULENCE:
      case DAMAGE_SUFFOCATION:
      case SPELL_SUFFOCATE:
      case SKILL_GARROTTE:
        sprintf(buf, "The remains of %s appear blue.  They must have suffocated.",getName());
        break;
      case TYPE_CLAW:
      case TYPE_SLASH:
      case TYPE_CLEAVE:
      case TYPE_SLICE:
      case DAMAGE_TRAP_SLASH:
      case TYPE_SHRED:
        sprintf(buf, "%s's corpse lies here, slashed in many places.",getName());
         break;
      case TYPE_BEAR_CLAW:
        sprintf(buf, "%s's corpse looks to have been mauled by a bear.",getName());
        break;
      case TYPE_MAUL:
	sprintf(buf, "The corpse of %s lying on the ground looks smashed and mauled.", getName());
        break;
      case TYPE_SMASH:
      case TYPE_WHIP:
      case TYPE_CRUSH:
      case TYPE_BLUDGEON:
      case TYPE_SMITE:
      case TYPE_HIT:
      case TYPE_FLAIL:
      case TYPE_PUMMEL:
      case TYPE_THRASH:
      case TYPE_THUMP:
      case TYPE_WALLOP:
      case TYPE_BATTER:
      case TYPE_BEAT:
      case TYPE_STRIKE:
      case TYPE_POUND:
      case TYPE_CLUB:
        sprintf(buf, "The corpse of %s lying on the ground looks bashed and bludgeoned.",getName());
        break;
      case SKILL_STABBING:
        sprintf(buf, "%s's corpse has a large, gaping stab wound.",namebuf);
        break;
      case DAMAGE_ARROWS:
        sprintf(buf, "Arrows decorate %s's corpse, making %s look like a pincushion.", getName(), hmhr());
        break;
      case TYPE_PIERCE:
      case TYPE_STAB:
      case TYPE_STING:
      case TYPE_THRUST:
      case TYPE_SPEAR:
        sprintf(buf, "Many stab wounds cover %s's dead form here.",getName());
        break;
      case DAMAGE_TRAP_PIERCE:
        sprintf(buf, "%s's corpse has been skewered by hundreds of tiny bolts.",namebuf);
        break;
      case TYPE_BEAK:
        sprintf(buf, "Many tiny peck wounds cover %s's dead form.",getName());
        break;
      case TYPE_CANNON:
	sprintf(buf, "A hole the size of a <k>cannonball<1> has been punched through %s's corpse's chest.", getName());
	break;
      case TYPE_SHOOT:
        sprintf(buf, "%s's corpse lies in a pool of blood, riddled with bullet holes.", namebuf);
	break;
      case TYPE_BITE:
        sprintf(buf, "%s's corpse looks like %s was bitten to death.",namebuf, hssh());
        break;
      case SKILL_BACKSTAB:
        sprintf(buf, "A big gaping wound is in the center of %s's back.", getName());
        break;
      case SKILL_THROATSLIT:
        sprintf(buf, "The throat of %s's corpse has been sliced from ear to ear.", getName());
        break;
      case DAMAGE_DISEMBOWLED_HR:
        sprintf(buf, "The corpse of %s has been disembowled with a slash from gullet to groin!",getName());
        break;
      case DAMAGE_DISEMBOWLED_VR:
        sprintf(buf, "The corpse of %s has a large slash from hip to hip!", getName());
        break;
      case DAMAGE_EATTEN:
        sprintf(buf,"%s's corpse has been mawed and chewed upon by something VERY large!",namebuf);
        break;
      case DAMAGE_HACKED:
        sprintf(buf,"%s's corpse has been hacked in two!",namebuf);
        break;
      case DAMAGE_KNEESTRIKE_FOOT:
      case DAMAGE_HEADBUTT_FOOT:
        sprintf(buf,"%s's corpse has a swollen toe.",
             namebuf);
        break;
      case DAMAGE_KNEESTRIKE_SHIN:
      case DAMAGE_KNEESTRIKE_KNEE:
      case DAMAGE_KNEESTRIKE_THIGH:
      case DAMAGE_HEADBUTT_LEG:
        sprintf(buf,"%s's corpse has a shattered kneecap.",
             namebuf);
        break;
      case DAMAGE_KNEESTRIKE_SOLAR:
      case DAMAGE_HEADBUTT_BODY:
        sprintf(buf,"%s's corpse lies here with a crushed rib cage.",
             namebuf);
        break;
      case DAMAGE_KNEESTRIKE_CROTCH:      
      case DAMAGE_HEADBUTT_CROTCH:
        sprintf(buf,"%s's corpse seems folded at the waist, protecting its privates.",
              namebuf);
         break;
      case DAMAGE_HEADBUTT_THROAT:
        sprintf(buf,"%s's corpse displays a broken collar bone.",
             namebuf);
        break;
      case DAMAGE_KNEESTRIKE_CHIN:
      case DAMAGE_HEADBUTT_JAW:
        sprintf(buf,"%s's corpse has a crushed skull and a shattered jaw.",
             namebuf);
        break;
      case DAMAGE_KNEESTRIKE_FACE:
      case SKILL_HEADBUTT:
      case DAMAGE_CAVED_SKULL:
      case DAMAGE_HEADBUTT_SKULL:
        sprintf(buf,"Brains ooze out of the crushed skull of %s's corpse.",getName());
        break;
      case DAMAGE_RIPPED_OUT_HEART:
	sprintf(buf,"%s's corpse has a gaping bloody hole where the heart once was.",getName());
	break;
      case SKILL_STOMP:
        sprintf(buf,"Stomped to death, %s's corpse is here.",getName());
        break;
      case DAMAGE_STARVATION:
        sprintf(buf, "The emaciated corpse of %s is lying here.", getName());
        break;
      case DAMAGE_STOMACH_WOUND:
        sprintf(buf,"%s's intestines protrude through a wound in %s corpse's stomach.",namebuf,hshr());
        break;
      case DAMAGE_RAMMED:
         sprintf(buf, "%s's corpse has a huge indention right in the midriff.", namebuf);
         break;
      case DAMAGE_BEHEADED:
         sprintf(buf, "%s's corpse is headless, and congealed blood lies around the neck.",namebuf);
         break;
      case SPELL_BLAST_OF_FURY:
        sprintf(buf, "%s was torn asunder by powerful magic!", namebuf);
        break;
      case SKILL_CHI:
	sprintf(buf, "%s's corpse wears a deep grimace of agony.", getName());
	break;
      case SKILL_KINETIC_WAVE:
	sprintf(buf, "Blood trickles out of the eyes, ears and nose of %s's corpse.", getName());
	break;
      case SKILL_PSI_BLAST:
	sprintf(buf, "Blank eyes stare out of the slackjawed face of %s's corpse.", getName());
	break;
      case SKILL_PSYCHIC_CRUSH:
	sprintf(buf, "%s's corpse wears a death mask of pure terror.", getName());
	break;
      case SKILL_PSIDRAIN:
	sprintf(buf, "The head of %s's corpse is blackened and bears a look of grim terror.", getName());
	break;

#if 1
      case SPELL_EARTHMAW:
	sprintf(buf, "The corpse of %s lies here half buried in the earth.", getName());
	break;
      case SPELL_CREEPING_DOOM:
        sprintf(buf, "A light coat of pollen covers the wide-eyed corpse of %s.", getName());
        break;
      case SPELL_FERAL_WRATH:
        sprintf(buf, "%s's corpse wears a fierce snarl.", getName());
        break;
      case SPELL_SKY_SPIRIT:
        sprintf(buf, "The corpse of %s looks torn apart by some terrible beast.", getName());
        break;
#endif
      case DAMAGE_NORMAL:
      case SPELL_FUMBLE:
      case SPELL_BLINDNESS:
      case SPELL_GARMULS_TAIL:
        sprintf(buf, "The corpse of %s is lying here.", getName());
        break;
      default:
        break;
    }
  }
  if (!specialCorpse) {
    gen_corpse->setDescr(mud_str_dup(buf));
    sprintf(buf, "the corpse of %s", getName());
    gen_corpse->shortDescr = mud_str_dup(buf);
  }

  gen_corpse->setWeight(specialCorpse ? 1.0 : getWeight());
  gen_corpse->obj_flags.wear_flags = ITEM_TAKE;
  gen_corpse->setVolume(getVolume());
  gen_corpse->setMaxStructPoints(50);
  gen_corpse->setStructPoints(50);

  // whacky problems from shadowy/glowing EQ and such
  // No reason to set the cbs based on creature.
#if 1
  if (isPc())
    gen_corpse->canBeSeen = 0;
  else
    gen_corpse->canBeSeen = canBeSeen;
#else
  gen_corpse->canBeSeen = canBeSeen;
#endif

  // generate an empty corpse if in a pk zone
  if(pcorpse && inPkZone()){
    pcorpse->obj_flags.decay_time = MAX_NPC_CORPSE_TIME;
    *roomp+=*pcorpse;

    return pcorpse;
  }

  if (getMoney() > 0) {
    if (pcorpse){
      money = create_money(getMoney() + ::number(GetMaxLevel(), 2*GetMaxLevel()));
    } else {
      money = create_money(getMoney(), getFaction());
    }

    addToMoney(-getMoney(), GOLD_INCOME);
    *gen_corpse += *money;
  }
  
#if 0
  // this is designed to move the pc's light cuz of items to the corpse
  // this seems to be silly.  makes the corpse glow or shadowy artificially
  gen_corpse->setLight(getLight());
#endif

  wearSlotT i;
  for (i = MIN_WEAR; i < MAX_WEAR; i++) {
    TThing *obo;
    if (equipment[i]) {
      obo = unequip(i);
      *gen_corpse += *obo;
      logItem(obo, CMD_SOUTH);  //log fact putting into corpse;
    }
    if (getStuckIn(i)) {
      obo = pulloutObj(i, TRUE, &j);
      *gen_corpse += *obo;
      logItem(obo, CMD_SOUTH);
    }
  }

  //load a herald and put on corpse - Russ 010298
  // Grimhaven Newsboy, chance to put herald in corpse
  if (mobVnum() == Mob::NEWSBOY) { 
    if (!::number(0, 10)) 
      *gen_corpse += *(read_object(Obj::HERALD, VIRTUAL));
  } 

  // banshee loading banshee larynx (shatter comp)
  // would be better as dissect, but banshee=undead=dustpile=no-dissect
  if (mobVnum() == Mob::BANSHEE) { 
    if (!::number(0, 1)) 
      *gen_corpse += *(read_object(COMP_SHATTER, VIRTUAL));
  } 

  for(StuffIter it=stuff.begin();it!=stuff.end();){
    TThing *obo=*(it++);
    *gen_corpse += (*obo)--;
    logItem(obo, CMD_SOUTH);
  }

  if (dynamic_cast<TMonster *>(this)) 
    gen_corpse->obj_flags.decay_time = MAX_NPC_CORPSE_TIME;
  else {
    if (!gen_corpse->stuff.empty()) 
      gen_corpse->obj_flags.decay_time = MAX_PC_CORPSE_EQUIPPED_TIME;
    else 
      gen_corpse->obj_flags.decay_time = MAX_PC_CORPSE_EMPTY_TIME;
  }

  if (roomp) {
//    *roomp += *gen_corpse;
    rp = roomp;
  } else {
    vlogf(LOG_BUG, format("%s had NULL roomp pointer, moved to room %d.") %  getName() % Room::STORAGE);

    rp = real_roomp(Room::STORAGE);
//    *rp += *gen_corpse;
  }

  // make sure we don't have any "corpses in a corpse" 
  for(StuffIter it=gen_corpse->stuff.begin();it!=gen_corpse->stuff.end();){
    o=*(it++);
    TBaseCorpse *tbc = dynamic_cast<TBaseCorpse *>(o);
    if (tbc) {
      --(*tbc);
      *rp += *tbc;
      TPCorpse * tmpcorpse = dynamic_cast<TPCorpse *>(o);
      if (tmpcorpse) {
        tmpcorpse->setRoomNum(in_room);
        tmpcorpse->removeGlobalNext();
        tmpcorpse->addCorpseToLists();
        tmpcorpse->saveCorpseToFile();
      }
    }
  }

  // Add the corpse to the room here, in case we dumped another corpse above.
  *rp += *gen_corpse;

  if (pcorpse && (GetMaxLevel() <= MAX_MORT)) {
    pcorpse->setRoomNum(in_room);
    pcorpse->removeGlobalNext();
    pcorpse->setOwner(sstring(tmpbuf).lower());
    pcorpse->addCorpseToLists();
    pcorpse->saveCorpseToFile();
    if (hasQuestBit(TOG_PERMA_DEATH_CHAR)) {
      pcorpse->setExpLost(0);
    } else {
      pcorpse->setExpLost(exp_lost);
    }
  }

  if (gamePort != Config::Port::PROD)
    gen_corpse->setupDissectionObjects();


  return gen_corpse;
}

void TBaseCorpse::setupDissectionObjects()
{
  int tPossible = (Races[getCorpseRace()]->tDissectItem[0].loadItem ?
                   (Races[getCorpseRace()]->tDissectItem[1].loadItem ? 1 : 0) : -1);

  if (tPossible > -1) {
    vlogf(LOG_LAPSOS, format("setupDissectionObjects: tPossible(%d)") %  tPossible);

    tPossible = ::number(0, tPossible);

    tDissections = new dissectInfo;

    tDissections->loadItem = Races[getCorpseRace()]->tDissectItem[tPossible].loadItem;
    tDissections->amount   = Races[getCorpseRace()]->tDissectItem[tPossible].amount;
    tDissections->count    = Races[getCorpseRace()]->tDissectItem[tPossible].count;
    tDissections->message_to_self =
      Races[getCorpseRace()]->tDissectItem[tPossible].message_to_self;
    tDissections->message_to_others =
      Races[getCorpseRace()]->tDissectItem[tPossible].message_to_others;
    tDissections->tNext = NULL;
  }

  std::map<unsigned short int, dissectInfo>::const_iterator tMarker;

  if ((tMarker = dissect_array.find(getCorpseVnum())) != dissect_array.end()) {
    dissectInfo *tDissect;

    if (tDissections)
      tDissect = tDissections->tNext = new dissectInfo;
    else
      tDissect = tDissections = new dissectInfo;

    tDissect->loadItem          = tMarker->second.loadItem;
    tDissect->amount            = tMarker->second.amount;
    tDissect->count             = tMarker->second.count;
    tDissect->message_to_self   = tMarker->second.message_to_self;
    tDissect->message_to_others = tMarker->second.message_to_others;
    tDissect->tNext = NULL;
  }
}
