//////////////////////////////////////////////////////////////////////////
//
// SneezyMUD - All rights reserved, SneezyMUD Coding Team
//
// $Log: damage.cc,v $
// Revision 1.1  1999/09/12 17:24:04  sneezy
// Initial revision
//
//
//////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////
//
//      SneezyMUD++ - All rights reserved, SneezyMUD Coding Team
//
//      "damage.cc" - functions for doing damage
//
//////////////////////////////////////////////////////////////////////////

#include "stdsneezy.h"
#include "combat.h"
#include "statistics.h"

// there is another one of these defines in combat.cc
#define DAMAGE_DEFINE 0

// -1 = v is dead, needs to go bye-bye
int TBeing::reconcileDamage(TBeing *v, int dam, spellNumT how)
{
  int rc;
  spellNumT how2;

  if (desc && !fight()) {
    SET_BIT(specials.affectedBy, AFF_AGGRESSOR);
    if (checkEngagementStatus()) 
      SET_BIT(specials.affectedBy, AFF_ENGAGER);
  }

  dam = getActualDamage(v, NULL, dam, how);

#if 0
  // Do more damage if you're much larger. Doesn't apply to mobs less than 5th.
  if (!isPc() && (getLevel(bestClass()) < 5)) {
    if (dam)
      dam += (int)((getWeight() / v->getWeight())/2);
  }
#endif
#if 0
  // hey cool, my spells do more damage!!!!!!  - Bad to do this here
  // FYI use getStrDamModifier instead too
  // Modify by strength.
  if (dam)
    dam += plotStat(STAT_CURRENT, STAT_STR, 1, 7, 4);
#endif

  // make um fly if appropriate
  if (!v->isPc() && v->canFly() && !v->isFlying())  {
    // in general, we did text before we applied the dam
    // if we incapacitated um, keep um on the ground.
    // if we knocked um over, also reconsider flying
    TMonster *tv = dynamic_cast<TMonster *>(v);
    if ((v->getHit() - dam > 0) &&
        (v->getPosition() == tv->default_pos))
      v->doFly();
  }

  switch (how) {
    case DAMAGE_KICK_HEAD:
    case DAMAGE_KICK_SIDE:
    case DAMAGE_KICK_SHIN:
    case DAMAGE_KICK_SOLAR:
      how2 = getSkillNum(SKILL_KICK);
      break;
    case DAMAGE_HEADBUTT_THROAT:
    case DAMAGE_HEADBUTT_BODY:
    case DAMAGE_HEADBUTT_CROTCH:
    case DAMAGE_HEADBUTT_LEG:
    case DAMAGE_HEADBUTT_FOOT:
    case DAMAGE_HEADBUTT_JAW:
    case DAMAGE_HEADBUTT_SKULL:
      how2 = SKILL_HEADBUTT;
      break;
    case DAMAGE_KNEESTRIKE_FOOT:
    case DAMAGE_KNEESTRIKE_SHIN:
    case DAMAGE_KNEESTRIKE_KNEE:
    case DAMAGE_KNEESTRIKE_THIGH:
    case DAMAGE_KNEESTRIKE_CROTCH:
    case DAMAGE_KNEESTRIKE_SOLAR:
    case DAMAGE_KNEESTRIKE_CHIN:
    case DAMAGE_KNEESTRIKE_FACE:
      how2 = SKILL_KNEESTRIKE;
      break;
    default:
      how2 = how;
  }

  // some places we want to insure a kill so we send HUGE damage through
  // account for this by just reducing it to smallest required amount
  // dam2 is strictly for logging, this adjustment is done again
  // later on, but with some other quirks
  int dam2 = min(dam, 11 + v->getHit());

  if (how2 >= MIN_SPELL && how2 < MAX_SKILL && this != v) 
    LogDam(this, how2, dam2);

  rc = applyDamage(v, dam, how);
  if (IS_SET_DELETE(rc, DELETE_VICT)) {
    if (desc);
      v->reformGroup();
    return -1;
  }
  return rc;
}

// returns DELETE_VICT if v died
int TBeing::applyDamage(TBeing *v, int dam, spellNumT dmg_type)
{
  double percent = 0;
  int learn = 0;
  int rc = 0;
  int questmob;
  bool found = FALSE;

  // ranged damage comes through here via reconcileDamage
  // lets not set them fighting unless we need to
  if (sameRoom(v)) {
    if (setCharFighting(v, 0) == -1)    // immortal being attacked
      return 0;

    setVictFighting(v, dam);
  }

  // account for protection (sanct, etc)
  int protection = v->getProtection();
  dam *= 100 - protection; 
  dam = (dam + 50) / 100;  // the 50 is here to round appropriately

  if (this != v) {
    if (v->getHit() <= -11) {
      vlogf(10,"Uh Oh, %s had %d hp and wasn't dead.  Was fighting %s",v->getName(),v->getHit(),getName());
      stopFighting();
      act("Something bogus about this fight.  Tell a god!", TRUE, this, 0, v, TO_CHAR);
      return 0;
    }
    if (dam <= 0) 
      return FALSE;

    // special code for quest mobs
    questmob = v->mobVnum();
    switch (questmob) {
      case MOB_TROLL_GIANT:
      case MOB_CAPTAIN_RYOKEN:
      case MOB_TREE_SPIRIT:
      case MOB_JOHN_RUSTLER:
      case MOB_ORC_MAGI:
        found = TRUE;
        break;
      default:
        found = FALSE;
        break;
    }

    if (found) {
      affectedData *af, *af2;
      for (af = v->affected; af; af = af2) {
        af2 = af->next;
        if (af->type == AFFECT_COMBAT && af->modifier == COMBAT_SOLO_KILL) {
          TBeing *tbt = dynamic_cast<TBeing *>(af->be);
          if (tbt && tbt != this) {
            // Receiving help
//            vlogf(3, "%s received help from %s killing %s", tbt->getName(), getName(), v->getName());
            switch (questmob) {
              case MOB_TROLL_GIANT:
                tbt->setQuestBit(TOG_AVENGER_CHEAT);
                tbt->remQuestBit(TOG_AVENGER_HUNTING);
                break;
              case MOB_CAPTAIN_RYOKEN:
                tbt->remQuestBit(TOG_VINDICATOR_HUNTING_1);
                tbt->setQuestBit(TOG_VINDICATOR_CHEAT);
                break;
              case MOB_TREE_SPIRIT:
                tbt->remQuestBit(TOG_VINDICATOR_HUNTING_2);
                tbt->setQuestBit(TOG_VINDICATOR_CHEAT_2);
                break;
  	      case MOB_JOHN_RUSTLER:
		sendTo("You have failed to kill the rustler without aid, you must try again, but without assistance.\n\r");
		break;
	      case MOB_ORC_MAGI:
		sendTo("<c>You realize you did not follow the guidelines of your quest, so this fight will be for naught.<1>\n\r");
		tbt->setQuestBit(TOG_FAILED_TO_KILL_MAGI);
		break;
              default:
                break;
            }
//            vlogf(3, "Removing combat bit (%d) from: %s", af->level, v->getName());
            v->affectRemove(af);
          }
        }
      }
    }

    if (willKill(v, dam, dmg_type, TRUE))
      dam = 11 + v->getHit();

// kludge to make sure newbie pc's will kill mobs after they get them down
// without a ton of hits
    if (!v->isPc() && (v->getHit() <= -2))
      dam = 11 + v->getHit();

    percent = ((double) dam / (double) (v->getHit() + 11));

    if (percent < 0)
      vlogf(9, "Error: %% < 0 in applyDamage() : %s fighting %s.", getName(),v->getName());
    else 
      gainExpPerHit(v, percent);
  } else {
    // this == v

    if (willKill(v, dam, dmg_type, TRUE)) {
      // Mages can no longer kill themselves with crit failures...Russ 09/28/98
      if (hasClass(CLASS_MAGIC_USER) && (dmg_type >= 0) && (dmg_type <= 125)) 
        dam = v->getHit() - ::number(1, 10);  
      else
        dam = 11 + v->getHit();
    }
  }

  // doDamage erases flight, needed later to do crash landings by tellStatus
  bool flying = v->isFlying();

  if (dmg_type == DAMAGE_NORMAL || 
     (dmg_type >= TYPE_MIN_HIT && dmg_type <= TYPE_MAX_HIT)) {
    stats.combat_damage[v->isPc() ? PC_STAT : MOB_STAT] += dam;
  }
  if (desc) {
    desc->career.dam_done[getCombatMode()] += dam;
    if (dmg_type == DAMAGE_NORMAL || 
       (dmg_type >= TYPE_MIN_HIT && dmg_type <= TYPE_MAX_HIT))
      desc->session.combat_dam_done[getCombatMode()] += dam;
    else
      desc->session.skill_dam_done[getCombatMode()] += dam;
  }
#if DAMAGE_DEBUG
  vlogf(5,"DAMAGE APPLIED (%d) %s (victim = %s) Damage Type = %d .", dam, getName(), v->getName(), dmg_type);
#endif

  v->doDamage(dam, dmg_type);

  // award leftover xp from any roundoff problems
  if ((v->getPosition() == POSITION_DEAD) && !v->isPc())
    gain_exp(this, v->getExp());

  if (v->getPosition() == POSITION_DEAD && isPc() && (v->GetMaxLevel() >= GetMaxLevel())) {
    if (v->isAnimal() && doesKnowSkill(SKILL_CONS_ANIMAL)) {
      learn = getSkillValue(SKILL_CONS_ANIMAL);
      setSkillValue(SKILL_CONS_ANIMAL,learn++);
    }
    if (v->isVeggie() && doesKnowSkill(SKILL_CONS_VEGGIE)) {
      learn = getSkillValue(SKILL_CONS_VEGGIE);
      setSkillValue(SKILL_CONS_VEGGIE,learn++);
    }
    if (v->isDiabolic() && doesKnowSkill(SKILL_CONS_DEMON)) {
      learn = getSkillValue(SKILL_CONS_DEMON);
      setSkillValue(SKILL_CONS_DEMON,learn++);
    }
    if (v->isUndead() && doesKnowSkill(SKILL_CONS_UNDEAD)) {
      learn = getSkillValue(SKILL_CONS_UNDEAD);
      setSkillValue(SKILL_CONS_UNDEAD,learn++);
    }
    if (v->isReptile() && doesKnowSkill(SKILL_CONS_REPTILE)) {
      learn = getSkillValue(SKILL_CONS_REPTILE);
      setSkillValue(SKILL_CONS_REPTILE,learn++);
    }
    if (v->isGiantish() && doesKnowSkill(SKILL_CONS_GIANT)) {
      learn = getSkillValue(SKILL_CONS_GIANT);
      setSkillValue(SKILL_CONS_GIANT,learn++);
    }
    if (v->isPeople() && doesKnowSkill(SKILL_CONS_PEOPLE)) {
      learn = getSkillValue(SKILL_CONS_PEOPLE);
      setSkillValue(SKILL_CONS_PEOPLE,learn++);
    }
    if (v->isOther() && doesKnowSkill(SKILL_CONS_OTHER)) {
      learn = getSkillValue(SKILL_CONS_OTHER);
      learn = min(learn, 50);
      setSkillValue(SKILL_CONS_OTHER,learn++);
    }
  }

  if ((v->getPosition() == POSITION_DEAD) && v->isPc() && !isPc()) {
    // correct some AI stuff.
    // additional correction is done inside DeleteHatreds
    // ok, I won, make me not as mad
    TMonster *tmons = dynamic_cast<TMonster *>(this);
    tmons->DA(16);
    tmons->DMal(9);
    // I was attacked, anybody else looking at me funny?
    tmons->DS(8);
    // hey, look at all this cool gear in the corpse!
    tmons->UG(60);   // spiked high, but will drop off quickly

    if (tmons->inGrimhaven()) {
      // complete newbie protection
      tmons->setAnger(0);
      tmons->setMalice(0);
    }
  }
  rc = v->tellStatus(dam, (this == v), flying);
  if (IS_SET_DELETE(rc, DELETE_THIS))
    return DELETE_VICT;
  rc = damageEpilog(v, dmg_type);
  if (IS_SET_DELETE(rc, DELETE_VICT))
    return DELETE_VICT;

  // we use to return dam here - bat 12/22/97
  return TRUE;
}

// DELETE_VICT or FALSE
int TBeing::damageEpilog(TBeing *v, int dmg_type)
{
  char buf[256], buf2[256];
  int rc, questmob;
  TBeing *k, *ThTank=v->fight();
  followData *f;
  TThing *t, *t2;
  affectedData *af, *af2;
  TPerson *tp=NULL;;

  if ((v->master == this) && dynamic_cast<TMonster *>(v))
    v->stopFollower(TRUE);

  if (v->affected) {
    for (af = v->affected; af; af = af2) {
      af2 = af->next;
      if (af->type == SKILL_TRANSFIX)
        v->affectRemove(af);
    }
  }
  // should only appear if *I* hit, not get hit
  if (isAffected(AFF_INVISIBLE) && this != v)
    appear();

  if (v->riding && dynamic_cast<TBeing *> (v->riding)) {
    if (!v->rideCheck(-3)) {
      rc = v->fallOffMount(v->riding, POSITION_SITTING);
      v->addToWait(combatRound(2));
      if (IS_SET_DELETE(rc, DELETE_THIS))
        return DELETE_VICT;
    }
  } else if (v->riding && dynamic_cast<TMonster *> (v) && !v->desc) {
    if (::number(0,1)) {
      dynamic_cast<TMonster *> (v)->standUp();
    }
  }

  for (t = v->rider; t; t = t2) {
    t2 = t->nextRider;
    TBeing *tb = dynamic_cast<TBeing *>(t);
    // force a doublly failed rideCheck for riders of victim
    if (tb &&
        !tb->rideCheck(-3) && 
        !tb->rideCheck(-3)) {
      rc = tb->fallOffMount(v, POSITION_SITTING);
      tb->addToWait(combatRound(2));
      if (IS_SET_DELETE(rc, DELETE_THIS)) {
        delete tb;
        tb = NULL;
      }
    }
  }

  if (v->isPc() && !v->desc) {
    if (this != v) {
      catchLostLink(v);
      return FALSE;
    }
    if (v->getPosition() != POSITION_DEAD)
      return FALSE;
  }
  if (isPc() && !desc) {
    if (this != v) {
      v->catchLostLink(this);
      return FALSE;
    }
  }

  // this save was moved from gain_exp()
  doSave(SILENT_YES);

  // special code for quest mobs
  questmob = v->mobVnum();
  af2 = NULL;
  for (af = v->affected; af; af = af2) {
    af2 = af->next;
    if (af->type == AFFECT_COMBAT && af->modifier == COMBAT_SOLO_KILL) {
      if (af->be == this && v->getPosition() == POSITION_DEAD) {
        // successfully solo killed
        vlogf(3, "%s successfully killed %s", getName(), v->getName());
        vlogf(3, "Removing combat bit (%d) from: %s", af->level, v->getName());
        v->affectRemove(af);
        switch (questmob) {
          case MOB_TROLL_GIANT:
            setQuestBit(TOG_AVENGER_SOLO);
            remQuestBit(TOG_AVENGER_HUNTING);
            break;
          case MOB_CAPTAIN_RYOKEN:
            remQuestBit(TOG_VINDICATOR_HUNTING_1);
            setQuestBit(TOG_VINDICATOR_SOLO_1);
            *v += *read_object(OBJ_QUEST_ORE, VIRTUAL);
            break;
          case MOB_TREE_SPIRIT:
            remQuestBit(TOG_VINDICATOR_HUNTING_2);
            setQuestBit(TOG_VINDICATOR_SOLO_2);
            break;
  	  case MOB_JOHN_RUSTLER:
	    setQuestBit(TOG_RANGER_FIRST_KILLED_OK);
	    remQuestBit(TOG_RANGER_FIRST_FARMHAND);
	    break;
    	  case MOB_ORC_MAGI:
	    setQuestBit(TOG_KILLED_ORC_MAGI);
	    remQuestBit(TOG_SEEKING_ORC_MAGI);
	    break;
          default:
            break;
        }
      }
    }
  }

  // PC died while fighting quest mob..with checks.
  if (!isPc() && v->getPosition() == POSITION_DEAD) {
    questmob = mobVnum();
    switch (questmob) {
      case MOB_TROLL_GIANT:
        if (v->hasQuestBit(TOG_AVENGER_HUNTING)) {
          vlogf(4, "%s died in quest and flags being reset/removed", v->getName());
          v->remQuestBit(TOG_AVENGER_HUNTING);
          v->setQuestBit(TOG_AVENGER_RULES);
          v->sendTo("You have failed in your quest.\n\r");
          v->sendTo("You must return to the Bishop and let him know when you are ready.\n\r");
          v->remPlayerAction(PLR_SOLOQUEST);
        }
        break;
      case MOB_CAPTAIN_RYOKEN:
        if (v->hasQuestBit(TOG_VINDICATOR_HUNTING_1)) {
          vlogf(4, "%s died in quest and flags being reset/removed", v->getName());
          v->remQuestBit(TOG_VINDICATOR_HUNTING_1);
          v->setQuestBit(TOG_VINDICATOR_FOUND_BLACKSMITH);
          v->sendTo("You have failed in your quest.\n\r");
          v->sendTo("You must return to Fistlandantilus and let him know when you are ready.\n\r");
          v->remPlayerAction(PLR_SOLOQUEST);
        }
        break;
      case MOB_TREE_SPIRIT:
        if (v->hasQuestBit(TOG_VINDICATOR_HUNTING_2)) {
          vlogf(4, "%s died in quest and flags being reset/removed", v->getName());
          v->remQuestBit(TOG_VINDICATOR_HUNTING_2);
          v->setQuestBit(TOG_VINDICATOR_RULES_2);
          v->sendTo("You have failed in your quest.\n\r");
          v->sendTo("You must return to the phoenix and let him know you are ready.\n\r");
          v->remPlayerAction(PLR_SOLOQUEST);
        }
        break;
      case MOB_ORC_MAGI:
	if (hasQuestBit(TOG_SEEKING_ORC_MAGI) &&
	    !hasQuestBit(TOG_FAILED_TO_KILL_MAGI) &&
	    !hasQuestBit(TOG_PROVING_SELF)){
	  sendTo("<c>You realize you did not follow the guidelines of your quest, so this fight will be for naught.<1>\n\r");
	  setQuestBit(TOG_FAILED_TO_KILL_MAGI);
	}
	break;
      default:
        break;
    }
  }

  if ((v->getPosition() < POSITION_STUNNED) && !v->isPc()) {
    if (specials.fighting == v)
      if (desc && !(desc->autobits & AUTO_KILL)) 
        stopFighting();
  }

  if (v->getPosition() == POSITION_DEAD) {
    if (specials.fighting == v) 
      stopFighting();

    reconcileHurt(v, 0.03);

    if (dmg_type >= 0 && (dmg_type < MAX_SKILL) && discArray[dmg_type]) {
      strcpy(buf2, discArray[dmg_type]->name);
    } else if (dmg_type >= TYPE_MIN_HIT && dmg_type <= TYPE_MAX_HIT) {
      strcpy(buf2, attack_hit_text[(dmg_type - TYPE_MIN_HIT)].singular);
    } else {
      switch (dmg_type) {
        case DAMAGE_KICK_HEAD:
        case DAMAGE_KICK_SIDE:
        case DAMAGE_KICK_SHIN:
        case DAMAGE_KICK_SOLAR:
          strcpy(buf2, "kick");
          break;
        case DAMAGE_COLLISION:
          strcpy(buf2, "collision");
          break;
        case DAMAGE_FALL:
          strcpy(buf2, "falling");
          break;
        case DAMAGE_NORMAL:
          strcpy(buf2, "normal damage");
          break;
        case DAMAGE_STARVATION:
          strcpy(buf2, "starvation");
          break;
        case DAMAGE_HEADBUTT_THROAT:
        case DAMAGE_HEADBUTT_BODY:
        case DAMAGE_HEADBUTT_LEG:
        case DAMAGE_HEADBUTT_JAW:
        case DAMAGE_HEADBUTT_FOOT:
        case DAMAGE_HEADBUTT_CROTCH:
        case DAMAGE_HEADBUTT_SKULL:
          strcpy(buf2, "headbutt");
          break;
        case DAMAGE_KNEESTRIKE_FOOT:
        case DAMAGE_KNEESTRIKE_SHIN:
        case DAMAGE_KNEESTRIKE_KNEE:
        case DAMAGE_KNEESTRIKE_THIGH:
        case DAMAGE_KNEESTRIKE_CROTCH:
        case DAMAGE_KNEESTRIKE_SOLAR:
        case DAMAGE_KNEESTRIKE_CHIN:
        case DAMAGE_KNEESTRIKE_FACE:
          strcpy(buf2, "kneestrike");
	  break;
        case DAMAGE_CAVED_SKULL:
          strcpy(buf2, "caved-in skull");
          break;
        case DAMAGE_BEHEADED:
          strcpy(buf2, "behead");
          break;
        case SPELL_FIRE_BREATH:
          strcpy(buf2, "fire breath");
          break;
        case SPELL_CHLORINE_BREATH:
          strcpy(buf2, "chlorine breath");
          break;
        case SPELL_FROST_BREATH:
          strcpy(buf2, "frost breath");
          break;
        case SPELL_ACID_BREATH:
          strcpy(buf2, "acid breath");
          break;
        case SPELL_LIGHTNING_BREATH:
          strcpy(buf2, "lightning breath");
          break;
        default:
          sprintf(buf2, "damage type: %d", dmg_type);
          break;
      }
    }
    if (dynamic_cast<TPerson *>(v)) {
      if (v->inRoom() > -1) {
        if (!isPc()) {
          // killed by a mob
          // for sanity, check if the killer mob was a pet, etc
          if (master)
            vlogf(5, "%s killed by %s at %s (%d).  Method: %s -- master was %s", 
                 v->getName(), getName(), v->roomp->name, v->in_room, buf2, master->getName());
          else if (rider)
            vlogf(5, "%s killed by %s at %s (%d).  Method: %s -- rider was %s", 
                 v->getName(), getName(), v->roomp->name, v->in_room, buf2, rider->getName());
          else
            vlogf(5, "%s killed by %s at %s (%d).  Method: %s", 
                 v->getName(), getName(), v->roomp->name, v->in_room, buf2);
          
        } else {
          vlogf(5, "%s killed by %s at %s (%d) Method: %s -- <%sPlayer kill>",
                v->getName(), getName(), v->roomp->name, v->in_room,
                buf2,
                ((v->GetMaxLevel() <= 5 && v != this) ? "NEWBIE " : ""));
          total_player_kills++;

          affectedData aff;
          aff.type = AFFECT_PLAYERKILL;
          aff.duration = 10 * ONE_SECOND * SECS_PER_REAL_MIN;
        }

        // create grave marker
        if (!v->inGrimhaven()) {
          TObj * grave = read_object(OBJ_GENERIC_GRAVE, VIRTUAL);
          if (grave) {
            string graveDesc = "Here lies ";
            graveDesc += v->getName();
            graveDesc += ".\n\rKilled ";
  
            graveDesc += "by ";
            graveDesc += getName();
            char buf[256];
            sprintf(buf, " this %s day of %s, Year %d P.S.\n\r", 
                   numberAsString(time_info.day+1).c_str(),
                   month_name[time_info.month], time_info.year);

            graveDesc += buf;

            grave->swapToStrung();
            extraDescription *ed;
            ed = new extraDescription();
            ed->next = grave->ex_description;
            grave->ex_description = ed;
            ed->keyword = mud_str_dup(grave->name);
            ed->description = mud_str_dup(graveDesc.c_str());

            *v->roomp += *grave;
          }
        }
      } else 
        vlogf(5, "%s killed by %s at Nowhere  Method: %s.", v->getName(), getName(), buf2);
    }
    // Mark an actual kill for the person giving the final blow
    if (desc) {
      if (roomp->isRoomFlag(ROOM_ARENA)) 
        desc->career.arena_victs++;
      else {
        desc->session.kills++;
        desc->career.kills++;
      }
    }
    // track the death for the victim
    if (v->desc) {
      if (roomp->isRoomFlag(ROOM_ARENA)) {
        v->desc->career.arena_loss++;
      } else {
        v->desc->career.deaths++;
      }
    }

    // Mark a groupKill for all in the group

    if (isAffected(AFF_GROUP) && (master || followers)) {
      if (master)
        k = master;
      else
        k = this;

      if (k->isAffected(AFF_GROUP)) {
        if (sameRoom(k)) {
          if (k->desc) {
            k->desc->session.groupKills++;
            k->desc->career.group_kills++;
          }
        }
      }
      for (f = k->followers; f; f = f->next) {
        if (f->follower->isAffected(AFF_GROUP) && canSee(f->follower)) {
          if (sameRoom(f->follower)) { 
            if (f->follower->desc) {
              f->follower->desc->session.groupKills++;
              f->follower->desc->career.group_kills++;
            }
          }
        }
      }
    }
    strcpy(buf2, v->name);
    add_bars(buf2);

    rc = v->peelPkRespawn(this, dmg_type);
    // theoretically possible, but I don't believe situation will
    // have it occur realistically, JIC though
    if (IS_SET_DELETE(rc, DELETE_THIS)) {
      forceCrash("Bad result from PkRespawn (%s at %d).  Multiple deaths??",
          v->getName(), v->inRoom());
    }

    if (rc)
      return FALSE;

    rc = v->die(dmg_type);
    if (!IS_SET_DELETE(rc, DELETE_THIS))
      return FALSE;

    for (t = roomp->stuff; t; t = t2) {
      t2 = t->nextThing;
      TBeing *tbt = dynamic_cast<TBeing *>(t);
      if (!tbt || tbt == this || tbt == v)
        continue;
      if (!tbt->isPc() && tbt->spec) {
        rc = tbt->checkSpec(this, CMD_MOB_KILLED_NEARBY, "", v);
        if (IS_SET_DELETE(rc, DELETE_THIS)) {
          delete tbt;
          tbt = NULL;
        }
        if (IS_SET_DELETE(rc, DELETE_ITEM)) {
          // of course v is dead, duh!
          vlogf(10, "this return should probably not be invoked (1)");
          return DELETE_VICT;
        }
        if (IS_SET_DELETE(rc, DELETE_VICT)) {
          // we're not set up to handle this return from damageEpilog
          vlogf(10, "this return should probably not be invoked (2)");
          return DELETE_THIS;
        }
      }
    }
    if (sameRoom(v)) {
      // use auto-bit settings to do appropriate looting
      // let masters loot the kills of a follower
      if (desc && (desc->autobits & AUTO_LOOT_MONEY) && 
          !(desc->autobits & AUTO_LOOT_NOTMONEY)) {
        sprintf(buf, "get all.talen %s-autoloot", buf2);
        addCommandToQue(buf);
      } else if ((dynamic_cast<TMonster *>(this) && !v->isPc() && this==ThTank)|| 
		 (desc && (desc->autobits & AUTO_LOOT_NOTMONEY))) {
        sprintf(buf, "get all %s-autoloot", buf2);
        addCommandToQue(buf);
      }

      // more quest stuff
      tp=dynamic_cast<TPerson *>(this);
      if (tp && v->mobVnum()==MOB_LEPER) {
	if(tp->hasQuestBit(TOG_MONK_PURPLE_LEPER_KILLED4)) {
	  tp->remQuestBit(TOG_MONK_PURPLE_LEPER_KILLED4);
	  tp->setQuestBit(TOG_MONK_PURPLE_FINISHED);
	} else if(tp->hasQuestBit(TOG_MONK_PURPLE_LEPER_KILLED3)) {
	  tp->remQuestBit(TOG_MONK_PURPLE_LEPER_KILLED3);
	  tp->setQuestBit(TOG_MONK_PURPLE_LEPER_KILLED4);
	} else if(tp->hasQuestBit(TOG_MONK_PURPLE_LEPER_KILLED2)) {
	  tp->remQuestBit(TOG_MONK_PURPLE_LEPER_KILLED2);
	  tp->setQuestBit(TOG_MONK_PURPLE_LEPER_KILLED3);
	} else if(tp->hasQuestBit(TOG_MONK_PURPLE_LEPER_KILLED1)) {
	  tp->remQuestBit(TOG_MONK_PURPLE_LEPER_KILLED1);
	  tp->setQuestBit(TOG_MONK_PURPLE_LEPER_KILLED2);
	} else {
	  tp->setQuestBit(TOG_MONK_PURPLE_LEPER_KILLED1);
	  tp->remQuestBit(TOG_MONK_PURPLE_STARTED);
	}
      }
      //
      if ((getPosition() != POSITION_MOUNTED) &&
          desc && (desc->autobits & AUTO_DISSECT)) {
        char msg[256], gl[256];
        int comp, amt;
        TBaseCorpse *corpse = NULL;

        if ((t2 = searchLinkedListVis(this, buf2, roomp->stuff)) &&
            (corpse = dynamic_cast<TBaseCorpse *>(t2))) {
          if (doesKnowSkill(SKILL_DISSECT)) {
            comp = determineDissectionItem(corpse, &amt, msg, gl, this);
            if (comp != -1) {
              sprintf(buf, "dissect %s", buf2);
              addCommandToQue(buf);
            }
          }
          if (doesKnowSkill(SKILL_SKIN)) {
            comp = determineSkinningItem(corpse, &amt, msg, gl);
            if (comp != -1) {
              // skin is tasked and requires tools, just inform, don't do
              TObj *obj;
              obj = read_object(comp, VIRTUAL);
              act("You should be able to skin $p from $N.", FALSE, this, obj, corpse, TO_CHAR);
              delete obj;
            }
          }
        }
      }
    }
    return DELETE_VICT;
  } else
    return FALSE;
}

void TBeing::doDamage(int dam, int dmg_type)
{
//  dam = modifyForProtections(dam);
  points.hit -= dam;
  stats.damage[isPc() ? PC_STAT : MOB_STAT] += dam;

 if (desc) {
    desc->career.dam_received[getCombatMode()] += dam;
    if (dmg_type == DAMAGE_NORMAL || 
       (dmg_type >= TYPE_MIN_HIT && dmg_type <= TYPE_MAX_HIT))
      desc->session.combat_dam_received[getCombatMode()] += dam;
    else
      desc->session.skill_dam_received[getCombatMode()] += dam;
  }

  updatePos();
}

int TBeing::getActualDamage(TBeing *v, TThing *o, int dam, spellNumT attacktype)
{
  // checks for peaceful rooms, etc
  if (damCheckDeny(v, attacktype))
    return 0;

  dam = v->skipImmortals(dam);
  if (dam == -1)
    return 0;

  // insures both in same room
  bool rc = damDetailsOk(v, dam, FALSE);
  if (!rc)
    return FALSE;

  dam = damageTrivia(v, o, dam, attacktype);
  return dam;
}

int TBeing::damageEm(int dam, string log, int dmg_type)
{
  int rc;
  int flying;

  if (isImmortal())
    return FALSE;

  // doDamage erases flight, needed later to do crash landings by tellStatus
  flying = isFlying();
  doDamage(dam, dmg_type);

  rc = tellStatus(dam, TRUE, flying);
  if (IS_SET_DELETE(rc, DELETE_THIS))
    return DELETE_THIS;

  if (getPosition() == POSITION_DEAD) {
    if (!log.empty())
      vlogf(5, "[%s] - %s", getName(), log.c_str());
    if (specials.fighting)
      stopFighting();

    return die(dmg_type);
  }
  return FALSE;
}
