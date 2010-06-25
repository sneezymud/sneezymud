//////////////////////////////////////////////////////////////////////////
//
// SneezyMUD - All rights reserved, SneezyMUD Coding Team
//
//      "mobact.cc" - make mobiles be mobiles
//
//////////////////////////////////////////////////////////////////////////

#include <cmath>

#include "extern.h"
#include "handler.h"
#include "room.h"
#include "low.h"
#include "monster.h"
#include "range.h"
#include "disease.h"
#include "statistics.h"
#include "shop.h"
#include "person.h"
#include "being.h"
#include "spec_mobs.h"
#include "database.h"
#include "obj_spellbag.h"
#include "obj_base_corpse.h"
#include "obj_player_corpse.h"
#include "obj_bow.h"
#include "obj_money.h"
#include "obj_treasure.h"
#include "obj_component.h"
#include "obj_arrow.h"
#include "obj_general_weapon.h"
#include "obj_base_weapon.h"
#include "obj_base_clothing.h"
#include "obj_gas.h"
#include "weather.h"
#include "configuration.h"

// returns DELETE_THIS if this has to be deleted
int TMonster::mobileGuardian()
{
  TBeing *tbeTarg = NULL;
  int rc;

  if (((in_room) < 0) || !master || !isPet(PETTYPE_PET | PETTYPE_CHARM | PETTYPE_THRALL)||
      !master->fight() || !(tbeTarg = master->findAnAttacker()) ||
      !canSee(tbeTarg) || (getPosition() < POSITION_STANDING))
    return FALSE;

  if(isFriend(*tbeTarg) && ::number(0,3))
    return FALSE;

  if (sameRoom(*tbeTarg)) {
    if (!isDumbAnimal())
      act("$n screams 'I must protect my master!'", FALSE, this, 0, 0, TO_ROOM);
    else
      aiGrowl(NULL);

    rc = hit(tbeTarg);
    if (IS_SET_DELETE(rc, DELETE_VICT)) {
      delete tbeTarg;
      tbeTarg = NULL;
    }
    if (IS_SET_DELETE(rc, DELETE_THIS))
      return DELETE_THIS;
  }
  return TRUE;
}

/* This just handles some simple stuff for mobs related to joining in
fights of same race mobs or being protected (children should get this) */
// may return DELETE_THIS
int TMonster::protectionStuff()
{
  TThing *t = NULL;
  int found = FALSE, rc = 0;

  if (fight() || desc)
    return FALSE;

  if(getPosition() <= POSITION_SLEEPING)
    return FALSE;

  if (isPet(PETTYPE_PET | PETTYPE_CHARM | PETTYPE_THRALL) || master) 
    return FALSE;

  if (GuildProcs(spec) || UtilProcs(spec))
    return FALSE;


  for(StuffIter it=roomp->stuff.begin();it!=roomp->stuff.end();){
    t=*(it++);
    TBeing *tbt = dynamic_cast<TBeing *>(t);
    if (!tbt || (this == tbt))
      continue;
    if (tbt->fight() && IS_SET(tbt->specials.act, ACT_PROTECTEE)) {
#if 0
      if ((IS_SET(specials.act, ACT_PROTECTOR) &&
            isSameFaction(tbt)) ||
          (getRace() == tbt->getRace())) {
#endif
      if (IS_SET(specials.act, ACT_PROTECTOR) &&
            isSameFaction(tbt) && canSee(tbt->fight())) {
        doSay("I will come to thy Aid!");
        if (getPosition() < POSITION_STANDING)
          doStand();
        rc = doAssist("", tbt, TRUE);
        return rc;
      } else {
        continue;
      }
    } else {
      if (!tbt->fight() || tbt->fight()->desc || tbt->fight()->isImmortal()) {
        continue;
      } else {
        if (isSameFaction(tbt->fight())) {
//        if (getRace() == tbt->fight()->getRace()) {
          found = TRUE;
        }
        continue;
      }
    }
  }
  if (found && IS_SET(specials.act, ACT_PROTECTOR)) {
    for(StuffIter it=roomp->stuff.begin();it!=roomp->stuff.end();){
      t=*(it++);
      TBeing *tbt = dynamic_cast<TBeing *>(t);
      if (!tbt || (this == tbt))
        continue;
      if (!tbt->fight()) 
        continue;
      if (tbt->fight()->isImmortal() || tbt->fight()->desc)
        continue;
      if (isSameFaction(tbt->fight()) && !tbt->fight()->desc) {
//      if (getRace() == tbt->fight()->getRace()) {
        if (isSameFaction(tbt)) {
//        if (getRace() == tbt->getRace()) {
          continue;
        } else if (canSee(tbt->fight())) {
          doSay("I will come to thy Aid!");
          if (getPosition() < POSITION_STANDING)
            doStand();
          rc = doAssist("", tbt->fight(), TRUE);
          return rc;
        }
      }
      continue;
    }

    if (found) {
      for(StuffIter it=roomp->stuff.begin();it!=roomp->stuff.end();){
        t=*(it++);
        TBeing *tbt = dynamic_cast<TBeing *>(t);
        if (!tbt || (this == tbt))
          continue;
        if (!tbt->fight())
          continue;
        if (tbt->fight()->isImmortal() || tbt->fight()->desc)
          continue;
        if ((isSameFaction(tbt->fight())) &&
              (getRace() == tbt->fight()->getRace())) {
          if (isSameFaction(tbt) &&
               (getRace() == tbt->getRace())) {
            continue;
          } else if (canSee(tbt->fight())) {
            doSay("I will come to thy Aid!");
            if (getPosition() < POSITION_STANDING)
              doStand();
            rc = doAssist("", tbt->fight(), TRUE);
            return rc;
          }
        }
      }
      act("$n watches the fight closely with some concern.", FALSE, this, NULL, NULL, TO_ROOM);
      return FALSE;
    } else {
      return FALSE;
    }
  } else {
    return FALSE;
  }
  return FALSE;
}

/* This just handles some simple stuff for mobs that are charmed  -Batopr */
// may return DELETE_THIS
int TMonster::charmeeStuff()
{
  TBeing *tmp;
  int rc;

  if(getPosition() <= POSITION_SLEEPING)
    return FALSE;
  if (isPet(PETTYPE_PET | PETTYPE_CHARM | PETTYPE_THRALL) && master) {
    if (!sameRoom(*master))
      return FALSE;
    if (fight()) {
      /* if master ain't fighting, do nothing */
      if (!(tmp = master->fight()) || !canSee(tmp))
        return FALSE;
      /* if my master is tanking.... */
      if (tmp->fight() == master) {
        if (hasClass(CLASS_WARRIOR)) {
          if (this == tmp) {    /* master is fighting charm */
            master->sendTo("Your follower no longer likes you very much.\n\r");
            stopFollower(TRUE);
            return TRUE;
          }
	  
	  if(!master->isPc() || IS_SET(specials.act, ACT_GUARDIAN)){
	    if (!isDumbAnimal())
	      act("$n hollers, \"I'll save you master!\"",
		  FALSE,this,0,0,TO_ROOM);
	    else
	      aiGrowl(tmp);

	    if (!::number(0,2)) {
	      act("$n has rescued you!",FALSE,this,0,master,TO_VICT);
	      act("$n has rescued $N.",TRUE,this,0,master,TO_NOTVICT);
	      stopFighting();
	      tmp->stopFighting();
	      master->stopFighting();
	      setCharFighting(tmp);
	      tmp->setCharFighting(this);
	      tmp->addToWait(combatRound(1));
	    } else {
	      act("$n tries to rescue you but fails.",
		  FALSE,this,0,master,TO_VICT);
	    }
	    addSkillLag(getSkillNum(SKILL_RESCUE), 0);
	  }
        }
      }
    } else {
      /* if master ain't fighting, do nothing */
      if (!(tmp = master->fight()))
        return FALSE;
      /* master is fighting and i am not */
      addToWait(combatRound(1));
      if (!::number(0,2)) {
        rc = mobileGuardian();
        if (IS_SET_DELETE(rc, DELETE_THIS))
          return DELETE_THIS;
      }
    }
    return TRUE;
  }
  return FALSE;
}

static bool legalMobMovement(TMonster * mob, dirTypeT door)
{
  roomDirData *exitp;
  TRoom *rp = NULL;
  int iHeight;

  if(door <= DIR_NONE)
    return false;

  // this means it's a portal, so it will work ok for goDirection
  // not sure exactly what to do here, so we'll take the lazy way out
  // and just allow it
  if(door >= MAX_DIR)
    return true;

  exitp=mob->exitDir(door);

  if (!exit_ok(exitp, &rp) ||
      rp->isRoomFlag(ROOM_DEATH) ||
      (rp->isUnderwaterSector() && !(mob->isAffected(AFF_WATERBREATH))) ||
      (mob->isAquatic() &&
          !(rp->isWaterSector() || rp->isUnderwaterSector())) ||
      mob->willBumpHeadDoor(exitp, &iHeight) ||
      !(mob->isHumanoid() ? mob->canGoHuman(door) : mob->canGo(door)))
    return false;

  return true;
}

// this forces the mob to pick an exit and leave through it.
int TMonster::wanderAround()
{
  dirTypeT iterDir;
  int rc;

  // First, determine the total number of exits
  unsigned int numExits = 0;
  for(iterDir=MIN_DIR; iterDir < MAX_DIR; iterDir++)
    if (exitDir(iterDir))
      numExits++;
  
  if (!numExits)
    return false;

  // choose one of the exits at random
  unsigned int iterEx = ::number(0, numExits-1); 
  for (iterDir=MIN_DIR; iterDir < MAX_DIR; iterDir++) {
    if (exitDir(iterDir)) {
      if (!iterEx)
        break;
      iterEx--;
    }
  }

  // iterDir now holds a random starting point for our exit

  bool been_here = false;

  // cycle over all exits back to beginning
  // we will use "iterDir" as the random starting point
  // we will use "i" as an iterator
  // been_here is used to keep from infinitely looping

  // note also that i is abusing the loop feature we set up in the
  // enum ++ operator for it.
  dirTypeT i;
  for(i=iterDir; ; i++) {
    if (i==MAX_DIR)
      continue;
    if (i == iterDir) {
      if (been_here)
        return FALSE;
      been_here = true;
    }

    if((rc = mobileWander(i)))
      return rc;
  }
}

// returns DELETE_THIS
// attempt to wander in a given direction
int TMonster::mobileWander(dirTypeT door)
{
  roomDirData *exitp;
  TRoom *rp = NULL, *rp2 = NULL;
  int rc;

  if ((getPosition() < POSITION_STANDING) || 
      !exit_ok(exitp = exitDir(door), &rp) ||
      (rp->isRoomFlag(ROOM_NO_MOB) && !isPolice()) ||
      !(rp2 = roomp) || master ||
      !legalMobMovement(this, door))
    return 0;

  if (rider || (riding && riding->horseMaster() != this))
    return 0;

  // let's not go through unrevealed secret doors
  if (IS_SET(exitp->condition, EX_SECRET) &&
      IS_SET(exitp->condition, EX_CLOSED))
    return 0;
  
  if ((rp->isWaterSector() &&
          !(isFlying() || isLevitating() || isAffected(AFF_SWIM))) ||
      (rp->isVertSector() &&
          !(isFlying() || raceHasNaturalClimb())) ||
      (rp->isAirSector() &&
          !(isFlying()))) {
    if (!canFly())
      return 0;
    // if it can fly, goDir will make it when appropriate
  }

  // keep mobs where they are supposed to be
  // but let them out if they're pissed - dash
  if (IS_SET(specials.act, ACT_STAY_ZONE) && !IS_SET(specials.act, ACT_HUNTING) &&
      rp->getZoneNum() != rp2->getZoneNum())
    return 0;

  if (specials.last_direction == door)
    specials.last_direction = DIR_NONE;
  else {
    specials.last_direction = door;
    rc = goDirection(door);
    if (IS_SET_DELETE(rc, DELETE_THIS))
      return DELETE_THIS;
  }
  return 1;
}

// returns DELETE_THIS
int TMonster::targetFound()
{
  TBeing *v;
  int rc;

  if (!(v = specials.hunting) || !awake()) 
    return FALSE;

  if (!canSee(v, INFRA_YES))
    return FALSE;

  if (checkPeaceful("You'd love to tear your quarry to bits, but you just CAN'T.\n\r"))
    doAction(fname(v->name),CMD_FUME);
  else {
    if (!isDumbAnimal()) {
      sstring screamBuf = "$n screams, 'Time to die, ";
      screamBuf += pers(v);
      screamBuf += "!'";
      if(strcmp(pers(v),"someone"))
	act(screamBuf, FALSE, this, 0, 0, TO_ROOM);
      // hee hee -GR
    } else 
      doAction(fname(v->name),CMD_GROWL);
    
    rc = takeFirstHit(*v);
    if (IS_SET_DELETE(rc, DELETE_VICT)) {
      delete v;
      v = NULL;
    }
    if (IS_SET_DELETE(rc, DELETE_THIS))
      return DELETE_THIS;
  }
  return TRUE;
}

// returns DELETE_THIS
int TMonster::hunt()
{
  int rc=0;
  TBeing *hunted;
  dirTypeT res;

  if (!(hunted = specials.hunting)) {
    REMOVE_BIT(specials.act, ACT_HUNTING);
    return FALSE;
  }
 
  if (fight() || (attackers >= 1))
    return FALSE;

  // keep horses from hunting
  if (rider)
    return FALSE;

  // let's try to summon or astral to them
  // Don't pick on level 15 or less
  // Peel
  if (!::number(0,4) && hasClass(CLASS_CLERIC) && persist >= 30 && !fight() &&
      !spelltask && hunted && !sameRoom(*hunted) && 
      hunted->GetMaxLevel() > 15 && !isPolice()){
    if(roomp && hunted->roomp && 
       roomp->getZoneNum()  == hunted->roomp->getZoneNum()){
      if(doesKnowSkill(SPELL_SUMMON) && getSkillValue(SPELL_SUMMON) > 33 &&
	 !isNotPowerful(hunted, getSkillLevel(SPELL_SUMMON), 
			SPELL_SUMMON, SILENT_YES)) {
	act("$n utters the words, 'Set it free and if it returns, it was meant to be.'", 
	    TRUE, this, 0, 0, TO_ROOM);
	rc = doDiscipline(SPELL_SUMMON, hunted->name);
      }
    } else {
      if(doesKnowSkill(SPELL_ASTRAL_WALK) && 
	 getSkillValue(SPELL_ASTRAL_WALK) > 33){
	act("$n utters the words, 'You can run, but you can't hide!'",
	    TRUE, this, 0, 0, TO_ROOM);
	rc = doDiscipline(SPELL_ASTRAL_WALK, hunted->name);
      }
    }
    if (IS_SET_DELETE(rc, DELETE_THIS))
      return DELETE_THIS;
  }


  if (persist <= 0) {
    if (sameRoom(*hunted)) {
      if (hunted == fight()) {
        REMOVE_BIT(specials.act, ACT_HUNTING);
        persist = 0;
        specials.hunting = 0;
        hunt_dist = 0;
        return FALSE;
      } else if ((Hates(hunted, NULL) || hunted->fight()) && canSee(hunted)) {
        rc = targetFound();
        if (IS_SET_DELETE(rc, DELETE_THIS))
          return DELETE_THIS;
        return 1;
      }
    } else if ((res = choose_exit_global(in_room, oldRoom, 2000)) > DIR_NONE) {
      rc = goDirection(res);   // walk home
      if (IS_SET_DELETE(rc, DELETE_THIS))
        return DELETE_THIS;
      return TRUE;
    } else {
      REMOVE_BIT(specials.act, ACT_HUNTING);
      specials.hunting = NULL;
      hunt_dist = 0;
      return FALSE;
    }
  } else if (hunted && hunted->roomp && canSee(hunted, INFRA_YES)) {
    // canSee needs to have hunted->roomp
    int count;

    // amt is the number of rooms we track in a single 'tick'
    // this used to be 1 + GetMaxLevel()/10
    // adjusted to be less linear 6/15/08 - Pappy
    float moves = cbrt(GetMaxLevel());
    int amt = int(moves);
    if (::number(0, 10) < int((moves-amt)*10))
      amt++;

    for (count = 0; count < amt; count++) {
      if (persist <= 0)
        return TRUE;
      persist -= 1;
      // rooms with musk are double hard for mobs to track through
      for(StuffIter it=roomp->stuff.begin();it!=roomp->stuff.end();++it)
        if (dynamic_cast<TGas*>(*it) && dynamic_cast<TGas*>(*it)->getType() == GAS_MUSK) {
          count++;
          break;
        }
      res = dirTrack(hunted);
      int next_room;
      if (spec == SPEC_ARCHER && (next_room = clearpath(in_room, res)) &&
          next_room != inRoom() && next_room == hunted->inRoom()) {
        TBow *bow = NULL;
        TArrow *ammo = NULL;
        TArrow *tempArr = NULL;
        std::vector <TBow *> bows = getBows();
        unsigned int j;
        for (j = 0; j < bows.size(); j++)
        {
          bow = bows[j];
          if (!bow->stuff.empty())
	    break;
          if (!bow) vlogf(LOG_BUG, "mobact.cc: bow is null somehow");
          if (bow && (tempArr = autoGetAmmo(bow))) {
            ammo = tempArr;
            break;
          }
        }
        if (bow && (ammo || !bow->stuff.empty())) {
          if (archer(NULL, CMD_GENERIC_PULSE, NULL, this, NULL))
            break;
        }
      }
      
      if (res != DIR_NONE) {
        if (legalMobMovement(this, res)) {
          rc = goDirection(res);
          if (IS_SET_DELETE(rc, DELETE_THIS))
            return DELETE_THIS;
        }

        if (sameRoom(*hunted)) {
          if (Hates(hunted, NULL) || hunted->fight()) {
            rc = targetFound();
            if (IS_SET_DELETE(rc, DELETE_THIS))
              return DELETE_THIS;
            return TRUE;
          }
        }
      } else {
        // no path exists to current huntee
        // If I don't hate them, stop hunting.
        // If I do hate them, and hate more than one person, hunt someone else
        // Otherwise, hold a grudge  :)
        if (!Hates(hunted, NULL)) {
          REMOVE_BIT(specials.act, ACT_HUNTING);
          persist = 0;
          specials.hunting = 0;
          hunt_dist = 0;
          return FALSE;
        }
        // find new huntee
        charList *i;
        for (i = hates.clist; i; i = i->next) {
          TBeing * tmp_ch = get_char(i->name, EXACT_YES);
          if (tmp_ch && tmp_ch != hunted) {
            // a slight problem in setHunt to new victim is that it will set
            // "oldRoom" (= my home) to where I am now.  Let's kludge our way
            // around this.
            int tmprm = oldRoom;
            setHunting(tmp_ch);
            oldRoom = tmprm;

            return true;
          }
        }

        // no path was found (concealed path, or too far away, etc)
        // prevent from searching multiple times
        return FALSE;
      }
    }
  } else {
    persist = 0;
    return FALSE;
  }

  return TRUE;
}

wearSlotT slot_from_bit(int wb)
{
  if (wb & ITEM_HOLD)
    return HOLD_RIGHT;
  else if (wb & ITEM_WEAR_FINGERS)
    return WEAR_FINGER_R;
  else if (wb & ITEM_WEAR_BODY)
    return WEAR_BODY;
  else if (wb & ITEM_WEAR_HEAD)
    return WEAR_HEAD;
  else if (wb & ITEM_WEAR_LEGS)
    return WEAR_LEG_R;
  else if (wb & ITEM_WEAR_FEET)
    return WEAR_FOOT_R;
  else if (wb & ITEM_WEAR_HANDS)
    return WEAR_HAND_R;
  else if (wb & ITEM_WEAR_ARMS)
    return WEAR_ARM_R;
  else if (wb & ITEM_WEAR_BACK)
    return WEAR_BACK;
  else if (wb & ITEM_WEAR_WAIST)
    return WEAR_WAIST;
  else if (wb & ITEM_WEAR_NECK)
    return WEAR_NECK;
  else if (wb & ITEM_WEAR_WRISTS)
    return WEAR_WRIST_R;
  else
    return WEAR_NOWHERE;
}

// DELETE_VICT = ch
int TObj::scavengeMe(TBeing *, TObj **)
{
  return FALSE;
}

int TMoney::scavengeMe(TBeing *, TObj **best_o)
{
  *best_o = this;
  return FALSE;
}

int TTreasure::scavengeMe(TBeing *, TObj **best_o)
{
  if (!objVnum() == Obj::IMMORTAL_EXCHANGE_COIN)
    *best_o = this;
  return FALSE;
}

// may return DELETE_THIS
int TMonster::superScavenger()
{
  int sl;
  TThing *t=NULL;
  TObj *best_o = NULL;
  TPCorpse *corpse = NULL;
  TBaseCorpse *bcorpse = NULL;
  int rc;
  char buf[256];

  if (!hasHands() && !isHumanoid())
    return FALSE;
  if (in_room == Room::DONATION)
    return FALSE;
  if (UtilMobProc(this))
    return FALSE;

  for(StuffIter it=stuff.begin();it!=stuff.end() && (t=*it);++it) {
    TBaseClothing *o = NULL;
    if (!(o = dynamic_cast<TBaseClothing *>(t)))
      continue;
    if (o->isMonogrammed()) // can't wear monogrammed items
      continue;
    sl = slot_from_bit(o->obj_flags.wear_flags);
    TObj *tobj = dynamic_cast<TObj *>(equipment[sl]);
    if (o->itemAC() < (tobj ?  tobj->itemAC() : 0)) {
      best_o = o;
      break;
    } else {
      rc = doDrop("", o);
      if (IS_SET_DELETE(rc, DELETE_ITEM)) {
        delete o;
        o = NULL;
      }
      if (IS_SET_DELETE(rc, DELETE_THIS)) {
        return DELETE_THIS;
      }
      return TRUE;
    }
  }
  // best_o is something I possess
  if (best_o) {
    // i found something I want, do I have something on my body
    // preventing me from wearing it?
    if ((t = equipment[slot_from_bit(best_o->obj_flags.wear_flags)])) {
      rc = doRemove("", t);
      if (IS_SET_DELETE(rc, DELETE_ITEM)) {
        delete t;
        t = NULL;
      }
      if (IS_SET_DELETE(rc, DELETE_THIS)) 
        return DELETE_THIS;

#if 0
// why do we need to rearrange inventory?
      /* Now move blocking object to end of inventory list */
      if (t && t->parent && t->nextThing) {
        t=*(it++);
        t->nextThing = NULL;
        for(StuffIter it=stuff.begin();it!=stuff.end() && (t2=*it);++it);
        t2->nextThing = t;
      }
#endif
      return TRUE;
    } else {
      if (best_o->parent) {
        // item i want is in a bag I own
        --(*best_o);
        *this += *best_o;
      }
      if (best_o->canWear(ITEM_HOLD)) {
        strcpy(buf, best_o->name);
        strcpy(buf, add_bars(buf).c_str());
        doGrab(buf);
      } else {
        strcpy(buf, best_o->name);
        strcpy(buf, add_bars(buf).c_str());
        doWear(buf);
      }
      return TRUE;
    }
  }
  
  // best_o = NULL here, check room for goodies
  for(StuffIter it=roomp->stuff.begin();it!=roomp->stuff.end() && (t=*it);++it) {
    rc = t->scavengeMe(this, &best_o);
    if (IS_SET_DELETE(rc, DELETE_VICT))
      return DELETE_THIS;
    else if (rc)
      return TRUE;
  }
  if (!best_o)
    return FALSE;

  // don't pick up entire, unlooted corpses
  bcorpse = dynamic_cast<TBaseCorpse *>(best_o);
  if (bcorpse && !bcorpse->stuff.empty())
    return FALSE;
  corpse = dynamic_cast<TPCorpse *>(best_o);
  if (corpse)
    return FALSE;

  if (best_o->parent) {
    // arbitrarily cast it to a corpse, non-corpses will be NULL
    // if we loot a corpse, we need to update save file
    // for now, mobs should not be looting PCorpses
    corpse = dynamic_cast<TPCorpse *>(best_o->parent);
    if (corpse)
      return FALSE;

    --(*best_o);
    if (corpse) {
	    vlogf(LOG_MOB_AI, format("Mob superScavenger: %s looting %s from %s in room (%d)") % this->name % best_o->name % corpse->name % (roomp ? roomp->in_room : 0));
      corpse->saveCorpseToFile();
    }
    *roomp += *best_o;
  }

  if (best_o) {
    vlogf(LOG_MOB_AI, format("Mob superScavenger: %s picking up %s in room (%d)") % this->name % best_o->name % (roomp ? roomp->in_room : 0));
    strcpy(buf, best_o->name);
    strcpy(buf, add_bars(buf).c_str());
    rc = doGet(buf);
    if (IS_SET_DELETE(rc, DELETE_THIS))
      return DELETE_THIS;
    return TRUE;
  } else
    return FALSE;
}

// may return DELETE_THIS
int TMonster::remove()
{
  wearSlotT i;
  int rc;
  TThing *t;

  if (!awake())
    return FALSE;

  if (!hasHands())
    return FALSE;

  for (i = MIN_WEAR; i < MAX_WEAR; i++) {
    if (i == HOLD_LEFT || i == HOLD_RIGHT)
      continue;
    if ((t = getStuckIn(i))) {
      rc = doRemove("", t);
      if (IS_SET_DELETE(rc, DELETE_ITEM)) {
        delete t;
        t = NULL;
      }
      if (IS_SET_DELETE(rc, DELETE_THIS))
        return DELETE_THIS;
      return TRUE;
    }
    if (i == WEAR_NECK && hasDisease(DISEASE_GARROTTE) && (t = equipment[i])) {
      rc = doRemove("", t);
      if (IS_SET_DELETE(rc, DELETE_ITEM)) {
        delete t;
        t = NULL;
      }
      if (IS_SET_DELETE(rc, DELETE_THIS))
        return DELETE_THIS;
      return TRUE;
    }
  }
  return FALSE;
}

// routine switches to wimpiest in room based on lowest "score" 
// a high "score" implies you are not a wimp 
// might also randomly switch just for the hell of it 
// score is based on current hit pts, current mana and AC 
//fudge factors are thrown in so switch preferences will be toward PC's 
// and away from newbie (level <5) PC's  - Batopr 
int TMonster::senseWimps()
{
  TBeing *tmp_victim = NULL;
  TThing *t=NULL;
  int score, smallest=100000;
  TBeing *wimp = NULL;
  
  if (!fight() || !awake())
    return FALSE;

  if (isPet(PETTYPE_PET | PETTYPE_CHARM | PETTYPE_THRALL))
    return FALSE;

  // keep horses from starting fights
  if (rider)
    return FALSE;

  // if we are being used as a cheap tank (by exploiting zombie/leprosy/cityguard), lets turn the tables
  // we don't want this to happen if you're helping defend a city though, so skip if in cities
  if (roomp && isAffected(AFF_AGGRESSOR) && !inGrimhaven() && !inAmber() && !inLogrus() && !inBrightmoon() &&
    !fight()->isPet(PETTYPE_PET|PETTYPE_CHARM|PETTYPE_THRALL) && !fight()->isPc()) {
    bool beingUsed = false;
    // if a PC using me to tank?
    for(StuffIter it=roomp->stuff.begin();!beingUsed && it!=roomp->stuff.end() && (t=*it);++it) {

      if (!(wimp = dynamic_cast<TBeing*>(t)))
        continue;
      if (wimp == this || wimp->isPc() || wimp->fight())
        continue;

      beingUsed = (wimp->fight() == this || wimp->fight() == fight());
    }

    if (beingUsed) {
      if (!isDumbAnimal()) 
        doSay("I think I'll just take care of you first!");
      act("$n <R>switches to $N<Z>.", TRUE, this, 0, wimp, TO_NOTVICT);
      act("$n <R>switches to you<Z>.", TRUE, this, 0, wimp, TO_VICT);

      if (fight()->fight() == this)
        fight()->stopFighting();
      stopFighting();
      addHated(wimp);
      setCharFighting(wimp);
      return TRUE;
    }
    wimp = NULL;
    t = NULL;
  }

  // first lets see if there are tons of attackers and break out if so */
  if (!::number(0,4) && ::number(0,attackers)) {

    /* randomly consider folks */
    for(StuffIter it=roomp->stuff.begin();it!=roomp->stuff.end() && (t=*it);++it) {
      tmp_victim = dynamic_cast<TBeing *>(t);
      if (!tmp_victim)
        continue;
      if (!tmp_victim->fight() || !canSee(t, INFRA_YES) || (tmp_victim == this))
        continue;
      if (::number(0,1) || tmp_victim->isImmortal())
        continue;
      aiWimpCheck(tmp_victim);
      return TRUE; 
    }
  }
  for(StuffIter it=roomp->stuff.begin();it!=roomp->stuff.end();){
    t=*(it++);
    tmp_victim = dynamic_cast<TBeing *>(t);
    if (!tmp_victim)
      continue;

    // random requirements
    if (tmp_victim == this || !canSee(tmp_victim, INFRA_YES))
      continue;

    if (tmp_victim->isImmortal() || !tmp_victim->awake())
      continue;

    // Don't brawl in guard proc mobiles, both annoying and exploitable.
    if (tmp_victim->spec == SPEC_CITYGUARD)
      continue;

    // protect recently dead PCs
    if ((tmp_victim->getHit() < 15) && tmp_victim->isPc())
      continue; 

    // protect the true newbie
    if ((tmp_victim->GetMaxLevel() < 3) && tmp_victim->isPc())
      continue;

    // skip cons on who I am fighting
    if (fight() == tmp_victim)
      continue;

    // skip if they are helping me
    if (fight() == tmp_victim->fight())
      continue;

    // skip group members
    if (inGroup(*tmp_victim))
      continue;

    // mostly a newbie vs super-buff protector
    if (GetMaxLevel()/tmp_victim->GetMaxLevel() >= 2)
      continue;

// COSMO MARKER right below here it does it on tmp_vict
// and the setCharFighting is the pc
    // randomly switch for the hell of it
    if (!::number(0,31)) {
      if (tmp_victim->fight()) {
        if (aiWimpSwitch(tmp_victim)) {
          stopFighting();
          setCharFighting(tmp_victim);
          return TRUE;
        } else continue;
      } else if (!tmp_victim->rider) {
        if (tmp_victim->isPc() && tmp_victim->task &&
            (tmp_victim->getStat(STAT_CURRENT, STAT_FOC) >= ::number(-10, 270))) {
          act("You feel nudged but you stay focused on your task.",
              TRUE, this, NULL, tmp_victim, TO_VICT);
          continue;
        } else if (!tmp_victim->isPc() && ::number(1, 100) < min(99, max(GetMaxLevel(), tmp_victim->GetMaxLevel()) - 1)) {
          // added to reduce chance of brawling as mob level increases
          // % chance of avoiding brawl is equal to brawler or brawlee level (whichever is higher)
          continue;
        } else if(tmp_victim->doesKnowSkill(SKILL_BRAWL_AVOIDANCE) &&
            tmp_victim->bSuccess(SKILL_BRAWL_AVOIDANCE)){
          act("You are nearly pulled into the brawl, but manage to avoid it.",
              TRUE, this, 0, tmp_victim,TO_VICT);
          continue;
        } else {
          act("Suddenly, $N finds $Mself involved in the brawl.",
                   TRUE, this, 0, tmp_victim, TO_NOTVICT);
          act("Suddenly, you find yourself involved in the brawl.",
                   TRUE, this, 0, tmp_victim, TO_VICT);
          vlogf(LOG_MOB_AI, format("Brawl! %s (%d) brawls %s (%d) in %d") % tmp_victim->name % tmp_victim->GetMaxLevel() % this->name % this->GetMaxLevel() % (roomp ? roomp->in_room : 0));
          if (getPosition() == POSITION_RESTING ||  getPosition() == POSITION_SITTING)
            doStand();

          setVictFighting(tmp_victim);
          return TRUE;
        }
      }
    }
    // end of random switch

    score = tmp_victim->getHit() + tmp_victim->hitLimit();
    score += tmp_victim->getMana();
    score += (2000 - tmp_victim->getArmor());
    score += tmp_victim->plotStat(STAT_CURRENT, STAT_KAR, 0, 2000, 1000);
  
// a lot of these adjustments are so the mob fights the person we want it
// to.  most of its arbitrary, ie. a mounted person _should_ be more powerful
// in reality, but otherwise the master (a PC) would rarely be hit 
// artificial newbie protection 
    if ((tmp_victim->GetMaxLevel() < 10) && tmp_victim->isPc())
      score += (500 - 50*tmp_victim->GetMaxLevel());   

    // a non combatant
    if (!tmp_victim->fight()) {
      // for the most part, lets ignore non combatants
      score += 1000;

      // of course someone in the group of my victim is probably up to no good
      // clerics that heal, etc
      if (tmp_victim->inGroup(*fight()))
        score -= 2000;
    }

    // someone fighting me
    if (tmp_victim->fight() == this) {
      // if person isn't grouped with my victim, lets treat them as wimpy
      // fighting me and "off on their own"
      if (!tmp_victim->inGroup(*fight()))
        score -= 150;

      // take out them nasty mages that cast on me
      if (tmp_victim->hasClass(CLASS_MAGE))
        score -= 200;

      if (tmp_victim->hasClass(CLASS_SHAMAN))
        score -= 200;

      // monks do massive damage, better kick their ass first
      if (tmp_victim->hasClass(CLASS_MONK))
        score -= 150;
    }

    score = max(1,score);
    if (score < smallest) {
      smallest = score;
      wimp = tmp_victim;
    }
    if (wimp && wimp->fight() && !::number(0,3))
      aiWimpCheck(wimp);
  }
  if (!wimp) 
    return FALSE;

  // lets give a big chance of switching to a horse's rider
  TBeing *tbt = dynamic_cast<TBeing *>(fight()->rider);
  if (tbt && (!::number(0,1))) {
    act("$n realizes that $N is just an innocent mount.",
            TRUE,this,0,fight(),TO_ROOM);
    if ((wimp = tbt)->fight())
      wimp->stopFighting();

    stopFighting();
    act("$n lashes into $N, the mount's master!",TRUE,this,0,wimp,TO_NOTVICT);
    act("$n lashes into you, the mount's master!",TRUE,this,0,wimp,TO_VICT);
    setCharFighting(wimp);
    return TRUE;
  }
  // Yet another way to hit horse's rider rather than horse
  tbt = dynamic_cast<TBeing *>(wimp->rider);
  if (tbt && ::number(0,6))
    wimp = tbt;

  if (wimp->fight()) {
    if (fight() != wimp) {
      if (!::number(0, 2) && aiWimpSwitch(wimp)) {
        stopFighting();
        setCharFighting(wimp);
        return TRUE;
      } else return FALSE;
    } else   // we are fighting the wimp already 
      return FALSE;
  } else {
    if (wimp->isPc() && wimp->task &&
        (wimp->getStat(STAT_CURRENT, STAT_FOC) >= ::number(-10, 270))) {
      act("You feel nudged but you stay focused on your task.",
          TRUE, this, NULL, wimp, TO_VICT);
      return FALSE;
    } else if (!wimp->isPc() && ::number(1, 65) < max(int(GetMaxLevel()), wimp->GetMaxLevel() - 1)) {
        // added to reduce chance of brawling as mob level increases
        // % chance of avoiding brawl is equal to brawler or brawlee level (whichever is higher)
        return FALSE;
    } else if (wimp->doesKnowSkill(SKILL_BRAWL_AVOIDANCE) &&
        wimp->bSuccess(SKILL_BRAWL_AVOIDANCE)){
      act("You are nearly pulled into the brawl, but manage to avoid it.",
          TRUE, this, 0, wimp, TO_VICT);
      return FALSE;
    } else {
      act("Suddenly, $N finds $Mself involved in the brawl.", TRUE, this, 0, wimp, TO_NOTVICT);
      act("Suddenly, you find yourself involved in the brawl.", TRUE, this, 0, wimp, TO_VICT);
      vlogf(LOG_MOB_AI, format("Brawl! %s (%d) lured to %s (%d) in (%d)") % wimp->name % wimp->GetMaxLevel() % this->name % this->GetMaxLevel() % (roomp ? roomp->in_room : 0));
      if (getPosition() == POSITION_RESTING ||  getPosition() == POSITION_SITTING)
        doStand();

      setVictFighting(wimp);
      return TRUE;
    }
  }
}

static bool mobBSlamCheck(TMonster &mob, TBeing &vict)
{
  // slam tends to have some builtin failures, so make the mobs smart here...
  // weight
  return ((compareWeights(vict.getTotalWeight(TRUE), mob.carryWeightLimit()) != -1) &&
  // dex
          (mob.getDexReaction() > vict.getAgiReaction()));
}

int TMonster::fighterMove(TBeing &vict)
{
  followData *f;
  TBeing *k, *t=NULL;
  bool offensive=FALSE;
  bool badspell=FALSE;
  spellTaskData *ts=NULL;

  if (!awake())
    return FALSE;

  // BEYOND HERE IS COMBAT ONLY STUFF
  if (!sameRoom(vict))
    return FALSE;

  // let's bash clerics/mages instead of the person we're fighting
  // we'll bash if they are an casting offensive spell on me or the room, or
  // if they are casting non-off on their group members, or sometimes just if
  // they are a mage or cleric.
  if(!::number(0,1) && vict.isAffected(AFF_GROUP)){
    if (!(k = vict.master))
      k = &vict;
    for (f = k->followers; f; f = f->next){
      if((t=f->follower) && t->spelltask && (ts = t->spelltask)){
	      offensive=discArray[ts->spell]->targets & TAR_VIOLENT;

	      if(((ts->victim == this || ts->room == this->roomp) && offensive) ||
	          (t->inGroup(vict) && !offensive))
	        badspell = TRUE;
      }
      if (!canSee(t)) {
	      continue;
      }
      if (t && t->isAffected(AFF_GROUP) && 
	        t->hasClass(CLASS_MAGE | CLASS_CLERIC | CLASS_SHAMAN) &&
	        (badspell || !::number(0,4)) &&
	        canBash(t, SILENT_YES) && getPosition() > POSITION_SITTING){
	      return doBash("", t);
      }
    }
  }
  
  //distinguish between fighting a caster and a non-caster
  if (vict.hasClass(CLASS_MAGE | CLASS_CLERIC | CLASS_SHAMAN) &&
      vict.getPosition() > POSITION_SITTING) {
    // caster
    // knock uhm over or we are toast!
    // bash, grapple, spin or bodyslam

    // use the best option
    if (getSkillValue(SKILL_BODYSLAM) > 33 &&
        getPosition() > POSITION_SITTING &&
        mobBSlamCheck(*this, vict) &&
        canBodyslam(&vict, SILENT_YES)) {
      return doBodyslam("", &vict);
    } else if (getSkillValue(SKILL_SPIN) > 33 &&
        getPosition() > POSITION_SITTING &&
        canSpin(&vict, SILENT_YES)) {
      return doSpin("", &vict);
#if 0
    } else if (getSkillValue(SKILL_GRAPPLE) > 33 &&
        getPosition() > POSITION_SITTING &&
        canGrapple(&vict)) {
#endif
    } else if (getSkillValue(SKILL_BASH) > 33 &&
        getPosition() > POSITION_SITTING &&
        canBash(&vict, SILENT_YES)) {
      return doBash("", &vict);
    } else if (getSkillValue(SKILL_TRIP) > 33 &&
        getPosition() > POSITION_SITTING &&
        canTrip(&vict, SILENT_YES)) {
      return doTrip("", &vict);
    }
    // in the event we get here, just fall through and do a damage attack
  }

  // use the best skill...  There's no sense in randomness IMO
  if (getSkillValue(SKILL_STOMP) > 33 &&
             canStomp(&vict, SILENT_YES) &&
      // stomp only good if very tall or vic on ground
             (vict.getPosition() < POSITION_STANDING ||
              getHeight() >= 5 * vict.getPosHeight()) &&
             (getPosition() >= POSITION_CRAWLING)) {
    return doStomp("", &vict);
  } else if (getSkillValue(SKILL_BODYSLAM) > 33 &&
             canBodyslam(&vict, SILENT_YES) &&
             mobBSlamCheck(*this, vict) &&
             (getPosition() >= POSITION_CRAWLING)) {
    return doBodyslam("", &vict);
  } else if (getSkillValue(SKILL_SPIN) > 33 &&
             canSpin(&vict, SILENT_YES) &&
             (getPosition() >= POSITION_CRAWLING)) {
    return doSpin("", &vict);
  } else if (getSkillValue(SKILL_KNEESTRIKE) > 33 &&
      canKneestrike(&vict, SILENT_YES) &&
      (getPosition() >= POSITION_CRAWLING)) {
    return doKneestrike("", &vict);
  } else if (getSkillValue(SKILL_HEADBUTT) > 33 &&
             (vict.getPosHeight() <= getHeight()) &&
             canHeadbutt(&vict, SILENT_YES) &&
             (getPosition() >= POSITION_CRAWLING)) {
    return doHeadbutt("", &vict);
  } else if (getSkillValue(SKILL_KICK) > 33 &&
             canKick(&vict, SILENT_YES) &&
             (getPosition() > POSITION_CRAWLING)) {
    return doKick("", &vict);
  } else if (getSkillValue(SKILL_BASH) > 33 &&
      canBash(&vict, SILENT_YES) &&
      (getPosition() >= POSITION_CRAWLING)) {
    return doBash("", &vict);
  } else if (getSkillValue(SKILL_TRIP) > 33 &&
      canTrip(&vict, SILENT_YES) &&
      (getPosition() >= POSITION_CRAWLING)) {
    return doTrip("", &vict);
  } else if (getSkillValue(SKILL_DISARM) > 33 &&
            canDisarm(&vict, SILENT_YES) &&
           (getPosition() >= POSITION_CRAWLING) &&
           !vict.affectedBySpell(SPELL_FUMBLE)) {
    return doDisarm("", &vict);
  }
  return FALSE;
}

int TMonster::monkMove(TBeing &vict)
{
  int num;

  if (!awake())
    return FALSE;

  if (!sameRoom(vict))
    return FALSE;

  if (getPosition() <= POSITION_SITTING) {
    if (!doesKnowSkill(SKILL_SPRINGLEAP))
      setSkillValue(SKILL_SPRINGLEAP,min(100, 10 + GetMaxLevel() * 4));
    return doSpringleap("", false, &vict);
  } else {
#if (0)
    if (GetMaxLevel() > 30 && !::number(0, 4)) {
      if (vict.GetMaxLevel() <= GetMaxLevel()) {
        if (vict.hitLimit() < 2 * hitLimit()) {
          if (!affectedBySpell(SKILL_QUIV_PALM)) {
            if (!doesKnowSkill(SKILL_QUIV_PALM))
              setSkillValue(SKILL_QUIV_PALM,min(100, 10 + GetMaxLevel() * 2));
            return doQuiveringPalm("", &vict);
          }
        }
      }
    }
#endif
    num = ::number(1, 6);
    switch (num) {
      case 1:
        if ((this->attackers > 2) &&
           (getMove() >= 80) &&
           (getPosition() == POSITION_STANDING) &&
           (doesKnowSkill(SKILL_HURL)) &&
           (getSkillValue(SKILL_HURL) > 66)) {
          int i;

          for (i = 0; i < 20; i++) {
            dirTypeT hurlDir = dirTypeT(::number(MIN_DIR, MAX_DIR-1));

            if (this->canGo(hurlDir)) {
              vlogf(LOG_ANGUS, format("monkMove: %s hurling %s, dir= %i") %  name %
                vict.name % hurlDir);
              return aiHurl(hurlDir, &vict);
            }
          }
          return doChop("", &vict);
          break;
        }
      case 2:
        if ((GetMaxLevel() > 75) &&
           (doesKnowSkill(SKILL_BONEBREAK)) &&
           (getMove() >= 80) &&
           (!vict.isImmune(IMMUNE_BONE_COND, WEAR_BODY))) {
	  // in theory we could do a per slot isImmune check here...
          return doBoneBreak("", &vict);
          break;
        }
      case 3:
        if (!vict.hasClass(CLASS_MONK) &&
           (vict.getPosition() == POSITION_STANDING) &&
           (getPosition() == POSITION_STANDING) &&
           (doesKnowSkill(SKILL_SHOULDER_THROW)) &&
           (getMove() >= 80)) {
          return doShoulderThrow("", &vict);
          break;
        }
      case 4:
        if (getSkillValue(SKILL_DISARM_MONK) > 33 &&
           canDisarm(&vict, SILENT_YES) &&
           (getPosition() >= POSITION_CRAWLING) &&
           !vict.affectedBySpell(SPELL_FUMBLE)) {
          return doDisarm("", &vict);
          break;
        }
      case 5:
        if ((getPosition() >= POSITION_STANDING) &&
           (!vict.affectedBySpell(SKILL_CHI)) &&
           (getMana() > 200)) {
          return doChi("", &vict);
          break;
        }
      default:
        if (getMove() >= 80) {
          return doChop("", &vict);
          break;
        }
    }
    /*
    if (num <= 5 && GetMaxLevel() > 75 && doesKnowSkill(SKILL_BONEBREAK)) {
      return doBoneBreak("", &vict);
    } else if ((num <= 10) &&
              (!vict.hasClass(CLASS_MONK)) &&
              (vict.getPosition() > POSITION_SITTING) &&
              (canBash(&vict, SILENT_YES)) &&
              (getPosition() >= POSITION_CRAWLING) &&
              (doesKnowSkill(SKILL_SHOULDER_THROW)) {
      return doShoulderThrow("", &vict);
    } else if ((num <= 15) &&
              (canDisarm(&vict, SILENT_YES)) &&
              (getPosition() >= POSITION_CRAWLING) &&
              (vict.heldInPrimHand() || 
                (vict.heldInSecHand() && !secHandCloth) ||
              (secHandCloth && !secHandCloth->isShield()) ||
              // trying to account for disarm working 1/3 of time with shield
              (secHandCloth && secHandCloth->isShield() && !::number(0,2)))) {
      return doDisarm("", &vict);
    //else if (num <= 11 && (4 * getHit() < 3 * hitLimit()))
      //return doFeignDeath();
    } else if ((num <= 20) &&
              (getPosition() >= POSITION_STANDING) &&
              (!vict.affectedBySpell(SKILL_CHI))) {
      return doChi("", &vict);
    } else if ((num <= 25) &&
              (this->attackers > 2) &&
              (getPosition() == POSITION_STANDING) &&
              (doesKnowSkill(SKILL_HURL)) &&
              (getSkillValue(SKILL_HURL) > 66)) {
      int i;

      for (i = 0; i < 20; i++) {
        dirTypeT hurlDir = dirTypeT(::number(MIN_DIR, MAX_DIR-1));

        if (this->canGo(hurlDir)) {
          vlogf(LOG_ANGUS, format("monkMove: %s hurling %s, dir= %i") %  name %
                vict.name % hurlDir);
          return aiHurl(hurlDir, &vict);
        }
      }
      return doChop("", &vict);
    } else {
      return doChop("", &vict);
    }
    */
  }
  return FALSE;
}

int TMonster::thiefMove(TBeing &vict)
{
// this goes with disarm at the end
#if 0
  TBaseClothing *secHandCloth = dynamic_cast<TBaseClothing *>(vict.heldInSecHand());
#endif
  
  if (!awake())
    return FALSE;

  // BEYOND HERE IS COMBAT ONLY STUFF
  if (!sameRoom(vict))
    return FALSE;

  if (::number(0, 1)) {
    TThing * prim = heldInPrimHand();
    if (hasHands() && (getPosition() >= POSITION_CRAWLING) &&
        prim && prim->isPierceWeapon()) {
      return doStab("", &vict);
    }
  } else {
    if (getSkillValue(SKILL_DISARM_THIEF) > 33 &&
       canDisarm(&vict, SILENT_YES) &&
       (getPosition() >= POSITION_CRAWLING) &&
       !vict.affectedBySpell(SPELL_FUMBLE)) {
      return doDisarm("", &vict);
    }
  }
  return FALSE;
}

int TMonster::deikhanMove(TBeing &vict)
{
  if (!awake())
    return FALSE;

  // don't cast two spells at once
  if (spelltask)
    return FALSE;

  if (::number(0,1)) {
    TBeing *tbt = dynamic_cast<TBeing *>(riding);
    if (tbt && (tbt->getRace() == RACE_HORSE)) {
      return doCharge("", &vict);
    }
  }

  return fighterMove(vict);
}

static bool faerieFireCheck(TBeing &ch, TBeing &vict, spellNumT spell)
{
  if (!vict.affectedBySpell(spell) && !::number(0, 9) &&
       ch.doesKnowSkill(spell) && (ch.getSkillValue(spell) > 33)) {
    act("$n utters the words, 'Tingle all over!'",
             TRUE, &ch, 0, 0, TO_ROOM);
    return true;
  }
  return false;
}

static spellNumT get_mage_spell(TMonster &ch, TBeing &vict, bool &on_me)
{
  spellNumT spell = TYPE_UNDEFINED;
  int j;
  discNumT i, best_disc;
  discNumT good_discs[]={DISC_AIR, DISC_ALCHEMY, DISC_EARTH, 
		    DISC_FIRE, DISC_SORCERY, DISC_SPIRIT, DISC_WATER, DISC_NONE};

  on_me = FALSE;

  best_disc = DISC_MAGE;
  for(j=0,i=good_discs[j];i != DISC_NONE;++j,i=good_discs[j]){
    CDiscipline *cdisc = ch.getDiscipline(i);
    if (!(ch.isValidDiscClass(i, CLASS_MAGE, 0)) || !cdisc) {
      continue;
    }

    if ((cdisc->getLearnedness() >= 
	 ch.getDiscipline(best_disc)->getLearnedness()) ||
	(best_disc==DISC_MAGE && cdisc->getLearnedness()>20)){

      if((i == DISC_ALCHEMY) || (i == DISC_SPIRIT)){
	if(!::number(0,20))
	  best_disc = i;
      } else if (!::number(0,1))
        best_disc = i;

    }
  }

  if (!ch.getDiscipline(best_disc) ||
      ch.getDiscipline(best_disc)->getLearnedness() <= 0)
    return TYPE_UNDEFINED;

  // PANIC spells
  // two varieties of teleport:
  spell = SPELL_TELEPORT;
  if (ch.doesKnowSkill(spell) && 
      (ch.getSkillValue(spell) > 33)) {
    // teleport myself as a "flee"
    if (!::number(0,30) &&
        !ch.pissed() && 
        (ch.getHit() < ch.hitLimit()/8) &&
        // don't let test fight mobs teleport
        !ch.affectedBySpell(AFFECT_TEST_FIGHT_MOB)) {
      act("$n utters the words, 'Departing is such Sweet Sorrow!'",
            FALSE, &ch, 0, 0, TO_ROOM);
      on_me = TRUE;
      return spell;
    }
    // teleport tank to deal with the weak and elderly
    // make this very rare, but let it happen
    if (ch.attackers >= 2 &&
        vict.isPc() &&
        !::number(0,30)) {
      act("$n sing-songs 'Happy Trails!'", FALSE, &ch, 0, 0, TO_ROOM);
      return spell;
    }
  }


  // to help mobs pick good spells, and avoid L100 mobs casting gusher
  // use this.
  int cutoff = min((int) ch.GetMaxLevel(), 50);

  if (best_disc == DISC_MAGE) {
    // area affect
    if (ch.attackers >= 2 && ::number(0, ch.attackers - 1)) {
      spell = SPELL_ACID_BLAST;
      if (!::number(0, 6) &&
           (cutoff < discArray[spell]->start) &&
           ch.doesKnowSkill(spell) && (ch.getSkillValue(spell) > 33)) {
        act("$n utters the words, 'Burn, Acid Burn!'",
                 TRUE, &ch, 0, 0, TO_ROOM);
        return spell;
      }
      spell = SPELL_ICE_STORM;
      if (!::number(0, 6) &&
           (cutoff < discArray[spell]->start) &&
           ch.doesKnowSkill(spell) && (ch.getSkillValue(spell) > 33)) {
        act("$n utters the words, 'Frozen Death!'",
                 TRUE, &ch, 0, 0, TO_ROOM);
        return spell;
      }
      spell = SPELL_SAND_BLAST;
      if (!::number(0, 6) &&
           (cutoff < discArray[spell]->start) &&
           ch.doesKnowSkill(spell) && (ch.getSkillValue(spell) > 33)) {
        act("$n utters the words, 'Stone Wash!'",
                 TRUE, &ch, 0, 0, TO_ROOM);
        return spell;
      }
      spell = SPELL_TORNADO;
      if (!::number(0, 6) &&
           (cutoff < discArray[spell]->start) &&
           ch.doesKnowSkill(spell) && (ch.getSkillValue(spell) > 33)) {
        act("$n utters the words, 'There's no place like home!'",
                 TRUE, &ch, 0, 0, TO_ROOM);
        return spell;
      }
      spell = SPELL_FIREBALL;
      if (!::number(0, 6) &&
           (cutoff < discArray[spell]->start) &&
           ch.doesKnowSkill(spell) && (ch.getSkillValue(spell) > 33)) {
        act("$n utters the words, 'Napalm!'",
                 TRUE, &ch, 0, 0, TO_ROOM);
        return spell;
      }
      spell = SPELL_COLOR_SPRAY;
      if (!::number(0, 6) &&
           (cutoff < discArray[spell]->start) &&
           ch.doesKnowSkill(spell) && (ch.getSkillValue(spell) > 33)) {
        act("$n utters the words, 'Magic Rainbow Colors!'",
                 TRUE, &ch, 0, 0, TO_ROOM);
        return spell;
      }
      spell = SPELL_DUST_STORM;
      if (!::number(0, 6) &&
           (cutoff < discArray[spell]->start) &&
           ch.doesKnowSkill(spell) && (ch.getSkillValue(spell) > 33)) {
        act("$n utters the words, 'Mr. Clean!'",
                 TRUE, &ch, 0, 0, TO_ROOM);
        return spell;
      }
      spell = SPELL_ARCTIC_BLAST;
      if (!::number(0, 6) &&
           (cutoff < discArray[spell]->start) &&
           ch.doesKnowSkill(spell) && (ch.getSkillValue(spell) > 33)) {
        act("$n utters the words, 'York Peppermint Patty!'",
                 TRUE, &ch, 0, 0, TO_ROOM);
        return spell;
      }
      spell = SPELL_PEBBLE_SPRAY;
      if (!::number(0, 6) &&
           (cutoff < discArray[spell]->start) &&
           ch.doesKnowSkill(spell) && (ch.getSkillValue(spell) > 33)) {
        act("$n utters the words, 'Gravel Travel!'",
                 TRUE, &ch, 0, 0, TO_ROOM);
        return spell;
      }
    }

    // offensive spells
    // hit um with the long-term effect ones first
    spell = SPELL_DISPEL_MAGIC;
    if (!::number(0, 9) &&
       ch.doesKnowSkill(SPELL_DISPEL_MAGIC) &&
       (ch.getSkillValue(SPELL_DISPEL_MAGIC) > 66) &&
       (vict.affectedBySpell(SPELL_HASTE) ||
       vict.affectedBySpell(SPELL_CELERITE) ||
       vict.affectedBySpell(SPELL_PLASMA_MIRROR) ||
       vict.affectedBySpell(SPELL_THORNFLESH) ||
       vict.affectedBySpell(SPELL_GILLS_OF_FLESH) ||
       vict.affectedBySpell(SPELL_AQUALUNG)) &&
       !(vict.affectedBySpell(SPELL_FAERIE_FIRE) ||
       vict.affectedBySpell(SPELL_BIND)) &&
       (cutoff < discArray[spell]->start)) {
      act("$n utters the words, 'Back to the basics!'",
          TRUE, &ch, 0, 0, TO_ROOM);
      return spell;
    }
    spell = SPELL_FAERIE_FIRE;
    if (faerieFireCheck(ch, vict, spell))
      return spell;

    spell = SPELL_ICY_GRIP;
    if (!::number(0, 9) && 
           (cutoff < discArray[spell]->start) &&
         ch.getSkillValue(spell) > 33 ) {
      act("$n utters the words, 'Cold Shoulder!'",
               TRUE, &ch, 0, 0, TO_ROOM);
      return spell;
    }

    // just plain damage spells here on
    spell = SPELL_STUNNING_ARROW;
    if (!::number(0, 3) &&
           (cutoff < discArray[spell]->start) &&
         ch.doesKnowSkill(spell) && (ch.getSkillValue(spell) > 33)) {
      act("$n utters the words, 'Destroying Arrow!'",
               TRUE, &ch, 0, 0, TO_ROOM);
      return spell;
    }
    spell = SPELL_GRANITE_FISTS;
    if (!::number(0, 3) &&
           (cutoff < discArray[spell]->start) &&
         ch.doesKnowSkill(spell) && (ch.getSkillValue(spell) > 33)) {
      act("$n utters the words, 'Pumice Pummel!'",
               TRUE, &ch, 0, 0, TO_ROOM);
      return spell;
    }
    spell = SPELL_MYSTIC_DARTS;
    if (!::number(0, 1) && 
           (cutoff < discArray[spell]->start) &&
        ch.doesKnowSkill(spell)) {
      act("$n utters the words, 'Missiles, come forth!'",
               TRUE, &ch, 0, 0, TO_ROOM);
      return spell;
    }
    spell = SPELL_HANDS_OF_FLAME;
    if (ch.hasHands() && !::number(0, 1) && 
           (cutoff < discArray[spell]->start) &&
        ch.doesKnowSkill(spell)) {
      act("$n utters the words, 'Fire, Fire Everywhere!'",
               TRUE, &ch, 0, 0, TO_ROOM);
      return spell;
    }
    spell = SPELL_GUSHER;
    if (!::number(0, 1) && 
           (cutoff < discArray[spell]->start) &&
        ch.doesKnowSkill(spell) ) {
      act("$n utters the words, 'Jack and Jill Time!'",
               TRUE, &ch, 0, 0, TO_ROOM);
      return spell;
    }
    spell = SPELL_SLING_SHOT;
    if (!::number(0, 1) && 
           (cutoff < discArray[spell]->start) &&
        ch.doesKnowSkill(spell)) {
      act("$n utters the words, 'David and Goliath!'",
               TRUE, &ch, 0, 0, TO_ROOM);
      return spell;
    }
    spell = SPELL_GUST;
    if (!::number(0, 1) && 
           (cutoff < discArray[spell]->start) &&
        ch.doesKnowSkill(spell)) {
      act("$n utters the words, 'Summer Breeze!'",
               TRUE, &ch, 0, 0, TO_ROOM);
      return spell;
    }

  } else if (best_disc == DISC_AIR) {

    // area affect
    if (ch.attackers >= 2 && ::number(0, ch.attackers - 1)) {
      spell = SPELL_TORNADO;
      if (!::number(0, 6) &&
           (cutoff < discArray[spell]->start) &&
           ch.doesKnowSkill(spell) && (ch.getSkillValue(spell) > 33)) {
        act("$n utters the words, 'There's no place like home!'",
                 TRUE, &ch, 0, 0, TO_ROOM);
        return spell;
      }
      spell = SPELL_DUST_STORM;
      if (!::number(0, 6) &&
           (cutoff < discArray[spell]->start) &&
           ch.doesKnowSkill(spell) && (ch.getSkillValue(spell) > 33)) {
        act("$n utters the words, 'Mr. Clean!'",
                 TRUE, &ch, 0, 0, TO_ROOM);
        return spell;
      }
    }
    spell = SPELL_SUFFOCATE;
    if (!::number(0, 6) &&
           (cutoff < discArray[spell]->start) &&
         ch.doesKnowSkill(spell) && (ch.getSkillValue(spell) > 33)) {
      act("$n utters the words, 'Catch your Breath!'",
               TRUE, &ch, 0, 0, TO_ROOM);
      return spell;
    }
    spell = SPELL_IMMOBILIZE;
    if (!::number(0, 3) && !vict.affectedBySpell(SPELL_IMMOBILIZE) &&
           (cutoff < discArray[spell]->start) &&
         ch.doesKnowSkill(spell) && (ch.getSkillValue(spell) > 33)) {
      act("$n utters the words, 'Freeze!'",
               TRUE, &ch, 0, 0, TO_ROOM);
      return spell;
    }
    spell = SPELL_GUST;
    if (!::number(0, 1) &&
           (cutoff < discArray[spell]->start) &&
         ch.doesKnowSkill(spell)) {
      act("$n utters the words, 'Summer Breeze!'",
               TRUE, &ch, 0, 0, TO_ROOM);
      return spell;
    }
  } else if (best_disc == DISC_ALCHEMY) {
    spell = SPELL_SHATTER;
    if (!::number(0, 1) && 
           (cutoff < discArray[spell]->start) &&
        ch.doesKnowSkill(spell)) {
      act("$n utters the words, 'Crack and Break!'",
               TRUE, &ch, 0, 0, TO_ROOM);
      return spell;
    }
  } else if (best_disc == DISC_EARTH) {
    // area affect
    if (ch.attackers >= 2 && ::number(0, ch.attackers - 1)) {

      spell = SPELL_LAVA_STREAM;
      if (!::number(0, 6) &&
           (cutoff < discArray[spell]->start) &&
           ch.doesKnowSkill(spell) && (ch.getSkillValue(spell) > 33)) {
        act("$n utters the words, 'Krakatoa!'",
                 TRUE, &ch, 0, 0, TO_ROOM);
        return spell;
      }
      spell = SPELL_SAND_BLAST;
      if (!::number(0, 6) &&
           (cutoff < discArray[spell]->start) &&
           ch.doesKnowSkill(spell) && (ch.getSkillValue(spell) > 33)) {
        act("$n utters the words, 'Stone Wash!'",
                 TRUE, &ch, 0, 0, TO_ROOM);
        return spell;
      }
      spell = SPELL_PEBBLE_SPRAY;
      if (!::number(0, 6) &&
           (cutoff < discArray[spell]->start) &&
           ch.doesKnowSkill(spell) && (ch.getSkillValue(spell) > 33)) {
        act("$n utters the words, 'Gravel Travel!'",
                 TRUE, &ch, 0, 0, TO_ROOM);
        return spell;
      }
    }
    // offensive spells
    spell = SPELL_METEOR_SWARM;
    if (!::number(0, 6) &&
           (cutoff < discArray[spell]->start) &&
         ch.doesKnowSkill(spell) && (ch.getSkillValue(spell) > 33)) {
      act("$n utters the words, 'Flocks of Rocks!'",
               TRUE, &ch, 0, 0, TO_ROOM);
      return spell;
    }
    spell = SPELL_GRANITE_FISTS;
    if (!::number(0, 3) &&
           (cutoff < discArray[spell]->start) &&
         ch.doesKnowSkill(spell) && (ch.getSkillValue(spell) > 33)) {
      act("$n utters the words, 'Pumice Pummel!'",
               TRUE, &ch, 0, 0, TO_ROOM);
      return spell;
    }
    spell = SPELL_SLING_SHOT;
    if (!::number(0, 1) && 
           (cutoff < discArray[spell]->start) &&
          ch.doesKnowSkill(spell)) {
      act("$n utters the words, 'David and Goliath!'",
               TRUE, &ch, 0, 0, TO_ROOM);
      return spell;
    }
  } else if (best_disc == DISC_FIRE) {
    // area affects
    if (ch.attackers >= 2 && ::number(0, ch.attackers - 1)) {

      spell = SPELL_HELLFIRE;
      if (!::number(0, 6) &&
           (cutoff < discArray[spell]->start) &&
           ch.doesKnowSkill(spell) && (ch.getSkillValue(spell) > 33)) {
        act("$n utters the words, 'Snowball's chance!'",
                 TRUE, &ch, 0, 0, TO_ROOM);
        return spell;
      }
      spell = SPELL_FIREBALL;
      if (!::number(0, 6) &&
           (cutoff < discArray[spell]->start) &&
           ch.doesKnowSkill(spell) && (ch.getSkillValue(spell) > 33)) {
        act("$n utters the words, 'Napalm!'",
                 TRUE, &ch, 0, 0, TO_ROOM);
        return spell;
      }
    }
    // offensive spells
    spell = SPELL_FAERIE_FIRE;
    if (faerieFireCheck(ch, vict, spell))
      return spell;

    spell = SPELL_INFERNO;
    if (!::number(0, 6) &&
           (cutoff < discArray[spell]->start) &&
         ch.doesKnowSkill(spell) && (ch.getSkillValue(spell) > 33)) {
      act("$n utters the words, 'Pillar of Flame!'",
               TRUE, &ch, 0, 0, TO_ROOM);
      return spell;
    }
    spell = SPELL_FLAMING_SWORD;
    if (!::number(0, 3) &&
           (cutoff < discArray[spell]->start) &&
         ch.doesKnowSkill(spell) && (ch.getSkillValue(spell) > 33)) {
      act("$n utters the words, 'Blade of Fire!'",
               TRUE, &ch, 0, 0, TO_ROOM);
      return spell;
    }
    spell = SPELL_HANDS_OF_FLAME;
    if (!::number(0, 1) && 
           (cutoff < discArray[spell]->start) &&
        ch.doesKnowSkill(spell)) {
      act("$n utters the words, 'Fire, Fire Everywhere!'",
               TRUE, &ch, 0, 0, TO_ROOM);
      return spell;
    }
  } else if (best_disc == DISC_SORCERY) {
    // defensive spells
#if 0
    if ((ch.getHit() > ch.hitLimit()/2) && !ch.pissed() &&
        (vict.GetMaxLevel() < ch.GetMaxLevel()) && ::number(0,1)) {

      spell = SPELL_BIND;
      if (!vict.affectedBySpell(spell) && !::number(0, 6) &&
           (cutoff < discArray[spell]->start) &&
           ch.doesKnowSkill(spell) && (ch.getSkillValue(spell) > 33)) {
        act("$n utters the words, 'Spiders, spiders everywhere!'",
                 TRUE, &ch, 0, 0, TO_ROOM);
        return spell;
      }
    } 
#endif
    // area affect
    if (ch.attackers >= 2 && ::number(0, ch.attackers - 1)) {

      spell = SPELL_ACID_BLAST;
      if (!::number(0, 6) &&
           (cutoff < discArray[spell]->start) &&
           ch.doesKnowSkill(spell) && (ch.getSkillValue(spell) > 33)) {
        act("$n utters the words, 'Burn, Acid Burn!'",
                 TRUE, &ch, 0, 0, TO_ROOM);
        return spell;
      }
      spell = SPELL_COLOR_SPRAY;
      if (!::number(0, 6) &&
           (cutoff < discArray[spell]->start) &&
           ch.doesKnowSkill(spell) && (ch.getSkillValue(spell) > 33)) {
        act("$n utters the words, 'Magic Rainbow Colors!'",
                 TRUE, &ch, 0, 0, TO_ROOM);
        return spell;
      }
    }
    // offesive spells
    spell = SPELL_BLAST_OF_FURY;
    if (!::number(0, 6) &&
           (cutoff < discArray[spell]->start) &&
      ch.doesKnowSkill(spell) && (ch.getSkillValue(spell) > 33)) {
      act("$n utters the words, 'Feel my WRATH!'",
          TRUE, &ch, 0, 0, TO_ROOM);
      return spell;
    }

    spell = SPELL_ATOMIZE;
    if (!::number(0, 6) &&
           (cutoff < discArray[spell]->start) &&
         ch.doesKnowSkill(spell) && (ch.getSkillValue(spell) > 33)) {
      act("$n utters the words, 'Jankity JANK!'",
               TRUE, &ch, 0, 0, TO_ROOM);
      return spell;
    }
    spell = SPELL_BLAST_OF_FURY;
    if (!::number(0, 6) &&
           (cutoff < discArray[spell]->start) &&
         ch.doesKnowSkill(spell) && (ch.getSkillValue(spell) > 33)) {
      act("$n utters the words, 'Feel my WRATH!'",
               TRUE, &ch, 0, 0, TO_ROOM);
      return spell;
    }

    spell = SPELL_STUNNING_ARROW;
    if (!::number(0, 3) &&
           (cutoff < discArray[spell]->start) &&
         ch.doesKnowSkill(spell) && (ch.getSkillValue(spell) > 33)) {
      act("$n utters the words, 'Destroying Arrow!'",
               TRUE, &ch, 0, 0, TO_ROOM);
      return spell;
    }

    spell = SPELL_MYSTIC_DARTS;
    if (!::number(0, 1) && 
           (cutoff < discArray[spell]->start) &&
        ch.doesKnowSkill(spell)) {
      act("$n utters the words, 'Missiles, come forth!'",
               TRUE, &ch, 0, 0, TO_ROOM);
      return spell;
    }

  } else if (best_disc == DISC_SPIRIT) {
    spell = SPELL_FUMBLE;
    if ((cutoff < discArray[spell]->start) &&
         ch.doesKnowSkill(spell) &&
         (vict.heldInPrimHand() || vict.heldInSecHand())) {
      act("$n utters the words, 'Butter Fingers!'",
               TRUE, &ch, 0, 0, TO_ROOM);
      return spell;
    }
  } else if (best_disc == DISC_WATER) {
    // area affect
    if (ch.attackers >= 2 && ::number(0, ch.attackers - 1)) {
      spell = SPELL_TSUNAMI;
      if (!::number(0, 6) &&
           (cutoff < discArray[spell]->start) &&
           ch.doesKnowSkill(spell) && (ch.getSkillValue(spell) > 33)) {
        act("$n utters the words, 'Surf's up!'",
                 TRUE, &ch, 0, 0, TO_ROOM);
        return spell;
      }
      spell = SPELL_ICE_STORM;
      if (!::number(0, 6) &&
           (cutoff < discArray[spell]->start) &&
           ch.doesKnowSkill(spell) && (ch.getSkillValue(spell) > 33)) {
        act("$n utters the words, 'Frozen Death!'",
                 TRUE, &ch, 0, 0, TO_ROOM);
        return spell;
      }
      spell = SPELL_ARCTIC_BLAST;
      if (!::number(0, 6) &&
           (cutoff < discArray[spell]->start) &&
           ch.doesKnowSkill(spell) && (ch.getSkillValue(spell) > 33)) {
        act("$n utters the words, 'York Peppermint Patty!'",
                 TRUE, &ch, 0, 0, TO_ROOM);
        return spell;
      }
    }
    spell = SPELL_PLASMA_MIRROR;
    if (!::number(0, 8) &&
           (cutoff < discArray[spell]->start) &&
         ch.doesKnowSkill(spell) && (ch.getSkillValue(spell) > 33)) {
      act("$n utters the words, 'Mirror Mirror!'",
               TRUE, &ch, 0, 0, TO_ROOM);
      on_me = TRUE;
      return spell;
    }
    spell = SPELL_WATERY_GRAVE;
    if (!::number(0, 6) &&
           (cutoff < discArray[spell]->start) &&
         ch.doesKnowSkill(spell) && (ch.getSkillValue(spell) > 33)) {
      act("$n utters the words, 'Sleep with the Fishies!'",
               TRUE, &ch, 0, 0, TO_ROOM);
      return spell;
    }
    spell = SPELL_ICY_GRIP;
    if (!::number(0, 3) && 
           (cutoff < discArray[spell]->start) &&
        ch.getSkillValue(spell) > 33 ) {
      act("$n utters the words, 'Cold Shoulder!'",
               TRUE, &ch, 0, 0, TO_ROOM);
      return spell;
    }
    spell = SPELL_GUSHER;
    if (!::number(0, 1) && 
           (cutoff < discArray[spell]->start) &&
        ch.doesKnowSkill(spell) ) {
      act("$n utters the words, 'Jack and Jill Time!'",
               TRUE, &ch, 0, 0, TO_ROOM);
      return spell;
    }

  }
  return TYPE_UNDEFINED;
}

static bool stupidityCheck(TBeing &ch, TBeing &vict, spellNumT spell)
{
  if (!vict.affectedBySpell(spell) && !::number(0, 6) &&
       ch.doesKnowSkill(spell) && (ch.getSkillValue(spell) > 33)) {
    act("$n utters the words, 'DUUHHHHHHHHHHH!!!!!!!!!'",
             TRUE, &ch, 0, 0, TO_ROOM);
    return true;
  }
  return false;
}

// SHAMAN
static bool deathMistCheck(TBeing &ch, TBeing &vict, spellNumT spell)
{
  if (!vict.affectedBySpell(SPELL_DEATH_MIST) && !::number(0, 6) &&
       ch.doesKnowSkill(spell) && (ch.getSkillValue(spell) > 33)) {
    act("$n utters the invokation, 'Chowe Kondiz Bub!'",
             TRUE, &ch, 0, 0, TO_ROOM);
    return true;
  }
  return false;
}

static spellNumT get_shaman_spell(TMonster &ch, TBeing &vict, bool &on_me)
{
  spellNumT spell = TYPE_UNDEFINED;
  int j;
  discNumT i, best_disc;
  discNumT good_discs[]={DISC_SHAMAN, DISC_SHAMAN_ARMADILLO, DISC_SHAMAN_FROG, DISC_SHAMAN_HEALING, DISC_SHAMAN_SPIDER, DISC_SHAMAN_SKUNK, DISC_SHAMAN_CONTROL, DISC_NONE};

  on_me = FALSE;

  best_disc = DISC_SHAMAN;
  for(j=0,i=good_discs[j];i != DISC_NONE;++j,i=good_discs[j]){
    CDiscipline *cdisc = ch.getDiscipline(i);
    if (!(ch.isValidDiscClass(i, CLASS_SHAMAN, 0)) || !cdisc) {
      continue;
    }

    if ((cdisc->getLearnedness() >= 
	 ch.getDiscipline(best_disc)->getLearnedness()) ||
	(best_disc==DISC_SHAMAN && cdisc->getLearnedness()>20)){
      if (!::number(0,1))
        best_disc = i;
    }
  }

  if (!ch.getDiscipline(best_disc) ||
      ch.getDiscipline(best_disc)->getLearnedness() <= 0)
    return TYPE_UNDEFINED;

  int cutoff = min((int) ch.GetMaxLevel(), 50);

  spell = SPELL_CHASE_SPIRIT;
  if (!::number(0, 3) &&
     ch.doesKnowSkill(SPELL_CHASE_SPIRIT) &&
     (ch.getSkillValue(SPELL_CHASE_SPIRIT) > 66) &&
     (vict.affectedBySpell(SPELL_HASTE) ||
     vict.affectedBySpell(SPELL_CELERITE) ||
     vict.affectedBySpell(SPELL_PLASMA_MIRROR) ||
     vict.affectedBySpell(SPELL_THORNFLESH) ||
     vict.affectedBySpell(SPELL_GILLS_OF_FLESH) ||
     vict.affectedBySpell(SPELL_AQUALUNG)) &&
     !(vict.affectedBySpell(SPELL_FAERIE_FIRE) ||
     vict.affectedBySpell(SPELL_BIND)) &&
     (cutoff < discArray[spell]->start)) {
    act("$n utters the words, 'Spirits be gone from this pathetic one!'",
        TRUE, &ch, 0, 0, TO_ROOM);
    return spell;
  }

  // PANIC spells
  spell = SPELL_INTIMIDATE;
  if (ch.doesKnowSkill(spell) && 
      (ch.getSkillValue(spell) > 33)) {
    if (!::number(0,30) &&
        !ch.pissed() && 
        (ch.getHit() < ch.hitLimit()/8) &&
        !ch.affectedBySpell(AFFECT_TEST_FIGHT_MOB)) {
      act("$n utters the invokation, 'Go Away! Leave me the Hell Alone!'", FALSE, &ch, 0, 0, TO_ROOM);
      on_me = FALSE;
      return spell;
    }
  }
  if (best_disc == DISC_SHAMAN) {
    // AREA AFFECT HERE
    if (ch.attackers >= 2 && ::number(0, ch.attackers - 1)) {
      spell = SPELL_FLATULENCE;
      if (!::number(0, 3) &&
	  (cutoff < discArray[spell]->start) &&
	  ch.doesKnowSkill(spell) && (ch.getSkillValue(spell) > 33)) {
	act("$n utters the invokation, 'He who smelt it, dealt it!'",
	    TRUE, &ch, 0, 0, TO_ROOM);
	return spell;
      }
      spell = SPELL_FLATULENCE;
      if (!::number(0, 3) &&
	  (cutoff < discArray[spell]->start) &&
	  ch.doesKnowSkill(spell) && (ch.getSkillValue(spell) > 33)) {
	act("$n utters the invokation, 'He who denied it, supplied it!'",
	    TRUE, &ch, 0, 0, TO_ROOM);
	return spell;
      }
    }

    // STANDARD OFFENSE
    // hit um with the long-term effect ones first
    // just plain damage spells here on
    spell = SPELL_STUPIDITY;
    if (stupidityCheck(ch, vict, spell))
      return spell;
    spell = SPELL_DISTORT;
    if (!::number(0, 3) &&
           (cutoff < discArray[spell]->start) &&
         ch.doesKnowSkill(spell) && (ch.getSkillValue(spell) > 33)) {
      act("$n utters the invokation, 'Houngan's Delight!'",
               TRUE, &ch, 0, 0, TO_ROOM);
      return spell;
    }
    spell = SPELL_DISTORT;
    if (!::number(0, 3) &&
           (cutoff < discArray[spell]->start) &&
         ch.doesKnowSkill(spell) && (ch.getSkillValue(spell) > 33)) {
      act("$n utters the invokation, 'Houngan's Delight!'",
               TRUE, &ch, 0, 0, TO_ROOM);
      return spell;
    }
    spell = SPELL_SOUL_TWIST;
    if (!::number(0, 3) &&
           (cutoff < discArray[spell]->start) &&
         ch.doesKnowSkill(spell) && (ch.getSkillValue(spell) > 33)) {
      act("$n utters the invokation, 'Internal Pretzel!'",
               TRUE, &ch, 0, 0, TO_ROOM);
      return spell;
    }
    spell = SPELL_SOUL_TWIST;
    if (!::number(0, 3) &&
           (cutoff < discArray[spell]->start) &&
         ch.doesKnowSkill(spell) && (ch.getSkillValue(spell) > 33)) {
      act("$n utters the invokation, 'Internal Pretzel!'",
               TRUE, &ch, 0, 0, TO_ROOM);
      return spell;
    }
    spell = SPELL_SQUISH;
    if (!::number(0, 3) &&
           (cutoff < discArray[spell]->start) &&
         ch.doesKnowSkill(spell) && (ch.getSkillValue(spell) > 33)) {
      act("$n utters the invokation, 'Firsta you takka da dough like-a dis...'",
               TRUE, &ch, 0, 0, TO_ROOM);
      return spell;
    }
    spell = SPELL_SQUISH;
    if (!::number(0, 3) &&
           (cutoff < discArray[spell]->start) &&
         ch.doesKnowSkill(spell) && (ch.getSkillValue(spell) > 33)) {
      act("$n utters the invokation, 'Firsta you takka da dough like-a dis...'",
               TRUE, &ch, 0, 0, TO_ROOM);
      return spell;
    }
    spell = SPELL_LIFE_LEECH;
    if (!::number(0, 3) && 
           (cutoff < discArray[spell]->start) &&
        ch.doesKnowSkill(spell)) {
      act("$n utters the invokation, 'I'm gonna suck you dry!!!'",
               TRUE, &ch, 0, 0, TO_ROOM);
      return spell;
    }
    spell = SPELL_VAMPIRIC_TOUCH;
    if (!::number(0, 3) &&
           (cutoff < discArray[spell]->start) &&
         ch.doesKnowSkill(spell) && (ch.getSkillValue(spell) > 33)) {
      act("$n utters the invokation, 'Ahh!! The BLUUD!!!!!'",
               TRUE, &ch, 0, 0, TO_ROOM);
      return spell;
    }
    spell = SPELL_LIFE_LEECH;
    if (!::number(0, 3) && 
           (cutoff < discArray[spell]->start) &&
        ch.doesKnowSkill(spell)) {
      act("$n utters the invokation, 'I'm gonna suck you dry!!!'",
               TRUE, &ch, 0, 0, TO_ROOM);
      return spell;
    }
    spell = SPELL_VAMPIRIC_TOUCH;
    if (!::number(0, 3) &&
           (cutoff < discArray[spell]->start) &&
         ch.doesKnowSkill(spell) && (ch.getSkillValue(spell) > 33)) {
      act("$n utters the invokation, 'Ahh!! The BLUUD!!!!!'",
               TRUE, &ch, 0, 0, TO_ROOM);
      return spell;
    }
    spell = SPELL_LIFE_LEECH;
    if (!::number(0, 3) && 
           (cutoff < discArray[spell]->start) &&
        ch.doesKnowSkill(spell)) {
      act("$n utters the invokation, 'I'm gonna suck you dry!!!'",
               TRUE, &ch, 0, 0, TO_ROOM);
      return spell;
    }
    spell = SPELL_LIFE_LEECH;
    if (!::number(0, 3) && 
           (cutoff < discArray[spell]->start) &&
        ch.doesKnowSkill(spell)) {
      act("$n utters the invokation, 'I'm gonna suck you dry!!!'",
               TRUE, &ch, 0, 0, TO_ROOM);
      return spell;
    }

  } else if (best_disc == DISC_SHAMAN_SPIDER) {

    // area affect
    if (ch.attackers >= 2 && ::number(0, ch.attackers - 1)) {
    }
    spell = SPELL_STICKS_TO_SNAKES;
    if (!::number(0, 6) && 
           (cutoff < discArray[spell]->start) &&
        ch.doesKnowSkill(spell) && (ch.getSkillValue(spell) > 33)) {
      act("$n utters the invokation, 'I got a woody and I'm gonna use it!'",
               TRUE, &ch, 0, 0, TO_ROOM);
      return spell;
    }
    spell = SPELL_RAZE;
    if (!::number(0, 6) &&
           (cutoff < discArray[spell]->start) &&
         ch.doesKnowSkill(spell) && (ch.getSkillValue(spell) > 33)) {
      act("$n utters the invokation, 'Rubbem Ow!'",
               TRUE, &ch, 0, 0, TO_ROOM);
      return spell;
    }
    spell = SPELL_STICKS_TO_SNAKES;
    if (!::number(0, 6) && 
           (cutoff < discArray[spell]->start) &&
        ch.doesKnowSkill(spell) && (ch.getSkillValue(spell) > 33)) {
      act("$n utters the invokation, 'I got a woody and I'm gonna use it!'",
               TRUE, &ch, 0, 0, TO_ROOM);
      return spell;
    }
    spell = SPELL_RAZE;
    if (!::number(0, 6) &&
           (cutoff < discArray[spell]->start) &&
         ch.doesKnowSkill(spell) && (ch.getSkillValue(spell) > 33)) {
      act("$n utters the invokation, 'Rubbem Ow!'",
               TRUE, &ch, 0, 0, TO_ROOM);
      return spell;
    }
    spell = SPELL_RAZE;
    if (!::number(0, 6) &&
           (cutoff < discArray[spell]->start) &&
         ch.doesKnowSkill(spell) && (ch.getSkillValue(spell) > 33)) {
      act("$n utters the invokation, 'Rubbem Ow!'",
               TRUE, &ch, 0, 0, TO_ROOM);
      return spell;
    }

  } else if (best_disc == DISC_SHAMAN_FROG) {
    spell = SPELL_STORMY_SKIES;
    if (!::number(0, 3) &&
           (cutoff < discArray[spell]->start) &&
         ch.doesKnowSkill(spell) && (ch.getSkillValue(spell) > 33)) {
      act("$n utters the invokation, 'Weather! Do my deed!'",
               TRUE, &ch, 0, 0, TO_ROOM);
      return spell;
    }
    spell = SPELL_DEATHWAVE;
    if (!::number(0, 3) &&
           (cutoff < discArray[spell]->start) &&
         ch.doesKnowSkill(spell) && (ch.getSkillValue(spell) > 33)) {
      act("$n utters the invokation, 'Deadly Blackness!'",
               TRUE, &ch, 0, 0, TO_ROOM);
      return spell;
    }
    spell = SPELL_AQUATIC_BLAST;
    if (!::number(0, 3) &&
           (cutoff < discArray[spell]->start) &&
         ch.doesKnowSkill(spell) && (ch.getSkillValue(spell) > 33)) {
      act("$n utters the invokation, 'River run DEEEEEEEEEEP!!!!'",
               TRUE, &ch, 0, 0, TO_ROOM);
      return spell;
    }
    spell = SPELL_STORMY_SKIES;
    if (!::number(0, 3) &&
           (cutoff < discArray[spell]->start) &&
         ch.doesKnowSkill(spell) && (ch.getSkillValue(spell) > 33)) {
      act("$n utters the invokation, 'Weather! Do my deed!'",
               TRUE, &ch, 0, 0, TO_ROOM);
      return spell;
    }
    spell = SPELL_AQUATIC_BLAST;
    if (!::number(0, 3) &&
           (cutoff < discArray[spell]->start) &&
         ch.doesKnowSkill(spell) && (ch.getSkillValue(spell) > 33)) {
      act("$n utters the invokation, 'River run DEEEEEEEEEEP!!!!'",
               TRUE, &ch, 0, 0, TO_ROOM);
      return spell;
    }
    spell = SPELL_DEATHWAVE;
    if (!::number(0, 3) &&
           (cutoff < discArray[spell]->start) &&
         ch.doesKnowSkill(spell) && (ch.getSkillValue(spell) > 33)) {
      act("$n utters the invokation, 'Deadly Blackness!'",
               TRUE, &ch, 0, 0, TO_ROOM);
      return spell;
    }
  } else if (best_disc == DISC_SHAMAN_SKUNK) {
    // AREA AFFECT
    if (ch.attackers >= 2 && ::number(0, ch.attackers - 1)) {
      spell = SPELL_DEATH_MIST;
      if (deathMistCheck(ch, vict, spell))
        return spell;
    }
    // REGULAR
    spell = SPELL_BLOOD_BOIL;
    if (!::number(0, 6) &&
           (cutoff < discArray[spell]->start) &&
         ch.doesKnowSkill(spell) && (ch.getSkillValue(spell) > 33)) {
      act("$n utters the invokation, 'Bubble, bubble. BOILING BLOOD!!'",
               TRUE, &ch, 0, 0, TO_ROOM);
      return spell;
    }
    spell = SPELL_CARDIAC_STRESS;
    if (!::number(0, 3) && 
           (cutoff < discArray[spell]->start) &&
        ch.doesKnowSkill(spell) ) {
      act("$n utters the invokation, 'Don't go breakin' my heart!'",
               TRUE, &ch, 0, 0, TO_ROOM);
      return spell;
    }
    spell = SPELL_LICH_TOUCH;
    if (!::number(0, 3) &&
           (cutoff < discArray[spell]->start) &&
         ch.doesKnowSkill(spell) && (ch.getSkillValue(spell) > 33)) {
      act("$n utters the invokation, 'Lich me, SUCKAH!!!'",
               TRUE, &ch, 0, 0, TO_ROOM);
      return spell;
    }
    spell = SPELL_LICH_TOUCH;
    if (!::number(0, 3) &&
           (cutoff < discArray[spell]->start) &&
         ch.doesKnowSkill(spell) && (ch.getSkillValue(spell) > 33)) {
      act("$n utters the invokation, 'Lich me, SUCKAH!!!'",
               TRUE, &ch, 0, 0, TO_ROOM);
      return spell;
    }
    spell = SPELL_BLOOD_BOIL;
    if (!::number(0, 6) &&
           (cutoff < discArray[spell]->start) &&
         ch.doesKnowSkill(spell) && (ch.getSkillValue(spell) > 33)) {
      act("$n utters the invokation, 'Bubble, bubble. BOILING BLOOD!!'",
               TRUE, &ch, 0, 0, TO_ROOM);
      return spell;
    }
    spell = SPELL_LICH_TOUCH;
    if (!::number(0, 3) &&
           (cutoff < discArray[spell]->start) &&
         ch.doesKnowSkill(spell) && (ch.getSkillValue(spell) > 33)) {
      act("$n utters the invokation, 'Lich me, SUCKAH!!!'",
               TRUE, &ch, 0, 0, TO_ROOM);
      return spell;
    }
  } else if (best_disc == DISC_SHAMAN_ARMADILLO) {
    // area affect
    if (ch.attackers >= 2 && ::number(0, ch.attackers - 1)) {
    }
  } else if (best_disc == DISC_SHAMAN_HEALING) {
    // area affect
    if (ch.attackers >= 2 && ::number(0, ch.attackers - 1)) {
    }
    spell = SPELL_HEALING_GRASP;
    if (!::number(0, 3) &&
           (cutoff < discArray[spell]->start) &&
         ch.doesKnowSkill(spell) && (ch.getSkillValue(spell) > 33)) {
      act("$n utters the invokation, 'Ahhh...that's better...'",
               TRUE, &ch, 0, 0, TO_ROOM);
      return spell;
    }
  } else if (best_disc == DISC_SHAMAN_CONTROL) {
    // area affect
    if (ch.attackers >= 2 && ::number(0, ch.attackers - 1)) {
    }
  }
  return TYPE_UNDEFINED;
}
// END SHAMAN
// try to keep this generic for mages AND rangers
// loadrate is the chance out of 100 of loading a new comp if there isn't one
// Peel
int TMonster::dynamicComponentLoader(spellNumT spell, int loadrate)
{
  TComponent *tcomp;
  unsigned int comp;

  for (comp = 0; (comp < CompInfo.size()) &&
    (spell != CompInfo[comp].spell_num); comp++);
  if (comp == CompInfo.size() || CompInfo[comp].comp_num < 0) 
    return FALSE;

  if (!(tcomp = findComponent(spell))) {
    if (::number(1,100) <= loadrate) {
      tcomp = dynamic_cast<TComponent *>(read_object(CompInfo[comp].comp_num, VIRTUAL));
      if (!tcomp)
        return FALSE;
      tcomp->setComponentCharges(::number(0, tcomp->getComponentCharges() - 1));
      *this += *tcomp;
    } else
      return FALSE;
  } else {
    tcomp->addToComponentCharges(-1);
  }
  if(tcomp && tcomp->getComponentCharges() <= 0)
    delete tcomp;

  return TRUE;
}

int TMonster::mageMove(TBeing &vict)
{
  bool on_me;
  spellNumT spell = TYPE_UNDEFINED;
  int i;

  if (!awake() || isPc() || cantHit > 0)
    return FALSE;
// COSMO DISC MARKER
// check wizardry

  if (!canSpeak())
    return FALSE;
  if (eitherArmHurt())
    return FALSE;
  if (checkSoundproof() || nomagic("No-magic room prevents spell cast."))
    return FALSE;

  // don't cast twice
  if (spelltask)
    return FALSE;

  // ONLY COMBAT STUFF BEYOND HERE
  if (!sameRoom(vict))
    return FALSE;

  if (getPosition() < POSITION_STANDING) {
    vlogf(LOG_BUG, format("%s was not standing in mageMove vs %s.") %  getName() % vict.getName());
    return FALSE;
  }

  for (i=0;i < 10;i++) {
    spell = get_mage_spell(*this, vict, on_me);
    if (spell != TYPE_UNDEFINED)
      break;
  }
  if (i>= 10)
    return FALSE;

  if (spell == TYPE_UNDEFINED)
    return FALSE;

  if (spell >= MAX_SKILL) {
    vlogf(LOG_BUG, "Something returned bad spell number in mageMove");
    return FALSE;
  }
// COSMO MAGE MARKER - check for tasking
  if (!(IS_SET(discArray[spell]->comp_types, SPELL_TASKED)) &&
        discArray[spell]->lag > 0) {
    addSkillLag(spell, 0);
  } else {
    addToWait(combatRound(2));
  }
  
  // if the target is not me but is a mob, don't load comp
  // prevents comp farming by using brawling or pets
  if(on_me || (!on_me && vict.isPc()))
    dynamicComponentLoader(spell, 100);

  if (on_me) {
    if (IS_SET(discArray[spell]->targets, TAR_SELF_ONLY | TAR_IGNORE | TAR_FIGHT_SELF)) {
      return doDiscipline(spell, "");
    }
    vlogf(LOG_BUG, format("Mob casting (1) spell %d on self with possibly bad target flags for spell") %  spell);
    return doDiscipline(spell, name);
  } else {
    return doDiscipline(spell, vict.name);
  }
}

int TMonster::shamanMove(TBeing &vict)
{
  bool on_me;
  spellNumT spell = TYPE_UNDEFINED;
  int i;

  if (!awake() || isPc() || cantHit > 0)
    return FALSE;
// COSMO DISC MARKER
// check wizardry

  if (!canSpeak())
    return FALSE;
  if (eitherArmHurt())
    return FALSE;
  if (checkSoundproof() || nomagic("No-magic room prevents spell cast."))
    return FALSE;

  // don't cast twice
  if (spelltask)
    return FALSE;

  // ONLY COMBAT STUFF BEYOND HERE
  if (!sameRoom(vict))
    return FALSE;

  if (getPosition() < POSITION_STANDING) {
    vlogf(LOG_BUG, format("%s was not standing in shamanMove vs %s.") %  getName() % vict.getName());
    return FALSE;
  }

  for (i=0;i < 10;i++) {
    spell = get_shaman_spell(*this, vict, on_me);
    if (spell != TYPE_UNDEFINED)
      break;
  }
  if (i>= 10)
    return FALSE;

  if (spell == TYPE_UNDEFINED)
    return FALSE;

  if (spell >= MAX_SKILL) {
    vlogf(LOG_BUG, "Something returned bad spell number in shamanMove");
    return FALSE;
  }
// COSMO MAGE MARKER - check for tasking
  if (!(IS_SET(discArray[spell]->comp_types, SPELL_TASKED)) &&
        discArray[spell]->lag > 0) {
    addSkillLag(spell, 0);
  } else {
    addToWait(combatRound(2));
  }
  
  // if the target is not me but is a mob, don't load comp
  // prevents comp farming by using brawling or pets
  if(on_me || (!on_me && vict.isPc()))
    dynamicComponentLoader(spell, 100);

  if (on_me) {
    if (IS_SET(discArray[spell]->targets, TAR_SELF_ONLY | TAR_IGNORE | TAR_FIGHT_SELF)) {
      return doDiscipline(spell, "");
    }
    vlogf(LOG_BUG, format("Shaman Mob casting (1) spell %d on self with possibly bad target flags for spell") %  spell);
    return doDiscipline(spell, name);
  } else {
    return doDiscipline(spell, vict.name);
  }
}

int TMonster::rangMove(TBeing &vict)
{
  if (!awake())
    return FALSE;

  // don't cast two spells at once
  if (spelltask)
    return FALSE;

  return fighterMove(vict);
}

// find the best healing that ch can give targ
// Peel
static spellNumT get_cleric_heal_spell(TMonster &ch, TBeing &targ)
{
  // best to worst
  spellNumT heals[]={SPELL_HEAL_FULL, SPELL_HEAL_CRITICAL,
	     SPELL_HEAL_SERIOUS, SPELL_HEAL_LIGHT};
  int count=4, i;
  wearSlotT slot;

  if(((targ.getHit()*100)/targ.hitLimit())<=75){
    for(i=0;i<count;++i){
      if (ch.doesKnowSkill(heals[i]) && (ch.getSkillValue(heals[i]) > 33)){
	return heals[i];
      }
    }
  }


#if 1
  // check for limbs that need to be restored
  if(ch.doesKnowSkill(SPELL_RESTORE_LIMB) &&
     ch.getSkillValue(SPELL_RESTORE_LIMB) > 33){
    for (slot = MIN_WEAR; slot < MAX_WEAR; slot++) {
      if (!targ.slotChance(slot) ||
	  !targ.isLimbFlags(slot, PART_USELESS | PART_PARALYZED))
	continue;
      break;
    }
    if (slot < MAX_WEAR) {
      return SPELL_RESTORE_LIMB;
    }
  }

  // check for broken bones for knit bone
  if(ch.doesKnowSkill(SPELL_KNIT_BONE) &&
     ch.getSkillValue(SPELL_KNIT_BONE) > 33){
    for (slot = MIN_WEAR; slot < MAX_WEAR; slot++) {
      if (targ.slotChance(slot) && targ.isLimbFlags(slot, PART_BROKEN))
	break;
    }
    if (slot < MAX_WEAR)
      return SPELL_KNIT_BONE;
  }

  // cure blindness
  if(ch.doesKnowSkill(SPELL_CURE_BLINDNESS) && 
     ch.getSkillValue(SPELL_CURE_BLINDNESS) > 33 &&
#if 0
     targ.isAffected(AFF_BLIND)) {
#else
     targ.affectedBySpell(SPELL_BLINDNESS)) {
#endif
    return SPELL_CURE_BLINDNESS;
  }

  // cure poison
  if(ch.doesKnowSkill(SPELL_CURE_POISON) &&
     ch.getSkillValue(SPELL_CURE_POISON) > 33){
    if (targ.isAffected(AFF_POISON) || 
        targ.affectedBySpell(SPELL_POISON) ||
        targ.affectedBySpell(SPELL_POISON_DEIKHAN) ||
        targ.hasDisease(DISEASE_FOODPOISON))
      return SPELL_CURE_POISON;
  }

  // cure disease
  if(ch.doesKnowSkill(SPELL_CURE_DISEASE) &&
     ch.getSkillValue(SPELL_CURE_DISEASE) > 33){
    if(targ.hasDisease(DISEASE_COLD) ||
       targ.hasDisease(DISEASE_FLU) ||
       targ.hasDisease(DISEASE_FROSTBITE) ||
       targ.hasDisease(DISEASE_LEPROSY) ||
       targ.hasDisease(DISEASE_PLAGUE) ||
       targ.hasDisease(DISEASE_PNEUMONIA) ||
       targ.hasDisease(DISEASE_DYSENTERY) ||
       targ.hasDisease(DISEASE_GANGRENE) ||
       targ.hasDisease(DISEASE_SCURVY) ||
       targ.hasDisease(DISEASE_SYPHILIS)){
      return SPELL_CURE_DISEASE;
    }
  }


  // clot
  if(ch.doesKnowSkill(SPELL_CLOT) &&
     ch.getSkillValue(SPELL_CLOT) > 33){
    for (slot = MIN_WEAR; slot < MAX_WEAR; slot++) {
      if (!targ.slotChance(slot))
	continue;
      if (targ.isLimbFlags(slot, PART_BLEEDING))
	break;
    }
    if(slot < MAX_WEAR)
      return SPELL_CLOT;
  }
#endif

  return TYPE_UNDEFINED;
}

static spellNumT get_cleric_spell(TMonster &ch, TBeing &vict, bool &on_me)
{
  spellNumT spell;
  discNumT best_disc = DISC_WRATH;
  discNumT i = DISC_NONE;
  int j;

  on_me = FALSE;

  for (j = 0;j < 6; j++) {
    if (j == 0) 
      i = DISC_WRATH;
    else if (j == 1) 
      i = DISC_AFFLICTIONS;
    else if (j == 2) 
      i = DISC_CURES;
    else if (j == 3) 
      i = DISC_HAND_OF_GOD;
    else if (j == 4) 
      i = DISC_AEGIS;
    else if (j == 5)
      i = DISC_CLERIC;

    if ((i == DISC_FAITH) || (i == DISC_THEOLOGY))
      continue;   // no useful spells
    if (!ch.getDiscipline(best_disc))
      return TYPE_UNDEFINED;
    if (ch.getDiscipline(i) && (ch.getDiscipline(i)->getLearnedness() > ch.getDiscipline(best_disc)->getLearnedness())) 
      best_disc = i;
  }
  if (ch.getDiscipline(best_disc)->getLearnedness() <= 0)
    return TYPE_UNDEFINED;

  int cutoff = min((int)ch.GetMaxLevel(), 50);

  // PANIC spells
  spell = SPELL_CURE_BLINDNESS;
  if (ch.doesKnowSkill(spell) && (ch.getSkillValue(spell) > 33) && ch.isAffected(AFF_BLIND)) {
    act("$n utters the words, 'Sight, come to me!'", FALSE, &ch, 0, 0, TO_ROOM);
    on_me = TRUE;
    return spell;
  }

  // two varieties of recall:
  // modeled after teleport logic
  spell = SPELL_WORD_OF_RECALL;
  if (ch.doesKnowSkill(spell) && 
      (ch.getSkillValue(spell) > 33)) {
    // recall myself as a "flee"
    if (!::number(0,30) &&
        !ch.pissed() && 
        (ch.getHit() < ch.hitLimit()/8) &&

        // extra check for recall.  avoid recalling if already home
        ch.oldRoom != ch.inRoom() &&

        // don't let test fight mobs recall
        !ch.affectedBySpell(AFFECT_TEST_FIGHT_MOB)) {
      act("$n sing-songs 'Na na na na, na na na na, hey hey hey, goodbye!'", FALSE, &ch, 0, 0, TO_ROOM);
      on_me = TRUE;
      return spell;
    }
    // recall tank to deal with the weak and elderly
    // make this very rare, but let it happen
    if (ch.attackers >= 2 &&
        vict.isPc() &&
        !::number(0,30)) {
      act("$n sing-songs 'Na na na na, na na na na, hey hey hey, goodbye!'", FALSE, &ch, 0, 0, TO_ROOM);
      return spell;
    }
  }

  if (best_disc == DISC_CLERIC) {
    spell = SPELL_PARALYZE_LIMB;
    if (!::number(0, 6) &&
        ch.doesKnowSkill(spell) && 
        (cutoff < discArray[spell]->start) &&
        (ch.getSkillValue(spell) > 33) &&
        ch.canParalyzeLimb(&vict, SILENT_YES)) {
      act("$n utters the words, 'Be Still!'", TRUE, &ch, 0, 0, TO_ROOM);
      return spell;
    }
    spell = SPELL_BLINDNESS;
    if (!::number(0, 6) &&
         !vict.affectedBySpell(spell) &&
         ch.doesKnowSkill(spell) && 
           (cutoff < discArray[spell]->start) &&
         (ch.getSkillValue(spell) > 33)) {
      act("$n utters, \"Darkness...take over the land!!!\"", TRUE, &ch, 0, 0, TO_ROOM);
      return spell;
    }
    spell = ch.getSkillNum(SPELL_HARM);
    if (!::number(0, 6) &&
         ch.doesKnowSkill(spell) && 
           (cutoff < discArray[spell]->start) &&
         (ch.getSkillValue(spell) > 33)) {
      act("$n utters the words, 'Ichor and Gore'", TRUE, &ch, 0, 0, TO_ROOM);
      return spell;
    }
    spell = SPELL_HEAL;
    if (!::number(0, 6) &&
         (ch.getHit() < ch.hitLimit()) &&
         ch.doesKnowSkill(spell) && 
           (cutoff < discArray[spell]->start) &&
         (ch.getSkillValue(spell) > 33)) {
      act("$n utters the words, 'Kjila'", TRUE, &ch, 0, 0, TO_ROOM);
      on_me = TRUE;
      return spell;
    }
    spell = ch.getSkillNum(SPELL_INFECT);
    if (!::number(0, 6) &&
         ch.doesKnowSkill(spell) && 
           (cutoff < discArray[spell]->start) &&
         (ch.getSkillValue(spell) > 33)) {
      act("$n utters the words, 'Pox and Smegma!'", TRUE, &ch, 0, 0, TO_ROOM);
      return spell;
    }
    spell = ch.getSkillNum(SPELL_POISON);
    if (!::number(0, 6) &&
         !vict.affectedBySpell(spell) &&
         ch.doesKnowSkill(spell) && 
           (cutoff < discArray[spell]->start) &&
         (ch.getSkillValue(spell) > 33)) {
      act("$n utters, \"Mold and Mushrooms!\"", TRUE, &ch, 0, 0, TO_ROOM);
      return spell;
    }
    spell = SPELL_DISEASE;
    if (!::number(0, 6) &&
         ch.doesKnowSkill(spell) && 
           (cutoff < discArray[spell]->start) &&
         (ch.getSkillValue(spell) > 33)) {
      act("$n beseeches, \"Oh lord, unleash pestilence upon this unrighteous one!\"", TRUE, &ch, 0, 0, TO_ROOM);
      return spell;
    }
    spell = ch.getSkillNum(SPELL_HARM_CRITICAL);
    if (!::number(0, 6) &&
         ch.doesKnowSkill(spell) && 
           (cutoff < discArray[spell]->start) &&
         (ch.getSkillValue(spell) > 33)) {
      act("$n utters the words, 'Pain and Suffering'", TRUE, &ch, 0, 0, TO_ROOM);
      return spell;
    }
    spell = SPELL_FLAMESTRIKE;
    if (!::number(0, 4) &&
         ch.doesKnowSkill(spell) && 
           (cutoff < discArray[spell]->start) &&
         (ch.getSkillValue(spell) > 33)) {
      act("$n biblethumps, \"Hellfire and Lava unleashed!\"", TRUE, &ch, 0, 0, TO_ROOM);
      return spell;
    }
    spell = ch.getSkillNum(SPELL_CURSE);
    if (!::number(0, 4) && !vict.affectedBySpell(spell) &&
         ch.doesKnowSkill(spell) && 
           (cutoff < discArray[spell]->start) &&
         (ch.getSkillValue(spell) > 33)) {
      act("$n utters, \"!&*!*#@(*&\"", TRUE, &ch, 0, 0, TO_ROOM);
      return spell;
    }
    spell = ch.getSkillNum(SPELL_HARM_SERIOUS);
    if (!::number(0, 6) &&
         ch.doesKnowSkill(spell) && 
           (cutoff < discArray[spell]->start) &&
         (ch.getSkillValue(spell) > 33)) {
      act("$n utters the words, 'Cuts and Bruises'", TRUE, &ch, 0, 0, TO_ROOM);
      return spell;
    }
    spell = SPELL_HEAL_CRITICAL;
    if (!::number(0, 6) &&
         (ch.getHit() < ch.hitLimit()) &&
         ch.doesKnowSkill(spell) && 
           (cutoff < discArray[spell]->start) &&
         (ch.getSkillValue(spell) > 33)) {
      act("$n utters the words, 'Comfort and Joy'", TRUE, &ch, 0, 0, TO_ROOM);
      on_me = TRUE;
      return spell;
    }
    spell = ch.getSkillNum(SPELL_HEAL_SERIOUS);
    if (!::number(0, 6) &&
         (ch.getHit() < ch.hitLimit()) &&
         ch.doesKnowSkill(spell) && 
           (cutoff < discArray[spell]->start) &&
         (ch.getSkillValue(spell) > 33)) {
      act("$n utters the words, 'I feel much better now'", TRUE, &ch, 0, 0, TO_ROOM);
      on_me = TRUE;
      return spell;
    }
    spell = ch.getSkillNum(SPELL_RAIN_BRIMSTONE);
    if (!::number(0, 1) &&
         ch.doesKnowSkill(spell) && 
           (cutoff < discArray[spell]->start) &&
         (ch.getSkillValue(spell) > 33)) {
      act("$n utters, \"Let's see how this smells!\"", TRUE, &ch, 0, 0, TO_ROOM);
      return spell;
    }
    spell = ch.getSkillNum(SPELL_BLESS);
    if (!::number(0, 6) &&
         !ch.affectedBySpell(SPELL_BLESS) &&
         ch.doesKnowSkill(spell) && 
           (cutoff < discArray[spell]->start) &&
         (ch.getSkillValue(spell) > 33)) {
      act("$n utters, \"What a Rush!\"", TRUE, &ch, 0, 0, TO_ROOM);
      on_me = TRUE;
      return spell;
    }
    spell = ch.getSkillNum(SPELL_ARMOR);
    if (!::number(0, 6) &&
         !ch.affectedBySpell(SPELL_ARMOR) &&
         ch.doesKnowSkill(spell) && 
           (cutoff < discArray[spell]->start) &&
         (ch.getSkillValue(spell) > 33)) {
      act("$n utters the words, 'Abrazak'",  TRUE, &ch, 0, 0, TO_ROOM);
      on_me = TRUE;
      return spell;
    }
    spell = ch.getSkillNum(SPELL_HARM_LIGHT);
    if (!::number(0, 6) &&
         ch.doesKnowSkill(spell) && 
           (cutoff < discArray[spell]->start) &&
         (ch.getSkillValue(spell) > 33)) {
      act("$n utters the words, 'This will only hurt a little...'",
               TRUE, &ch, 0, 0, TO_ROOM);
      return spell;
    }

    spell = ch.getSkillNum(SPELL_HEAL_LIGHT);
    if (!::number(0, 6) &&
         (ch.getHit() < ch.hitLimit()) &&
         ch.doesKnowSkill(spell) && 
           (cutoff < discArray[spell]->start) &&
         (ch.getSkillValue(spell) > 33)) {
      act("$n utters the words, 'I feel better now'",
               TRUE, &ch, 0, 0, TO_ROOM);
      on_me = TRUE;
      return spell;
    }
  } else if (best_disc == DISC_WRATH) {
    // area affect
    if (ch.attackers >= 2 && ::number(0, ch.attackers - 1)) {
      spell = ch.getSkillNum(SPELL_EARTHQUAKE);
      if (!::number(0, 6) &&
           !ch.roomp->isWaterSector() &&
           !ch.roomp->isUnderwaterSector() &&
           ch.doesKnowSkill(spell) && 
           (cutoff < discArray[spell]->start) &&
         (ch.getSkillValue(spell) > 33)) {
        act("$n utters the words, 'Crumble and die!'",  TRUE, &ch, 0, 0, TO_ROOM);
        return spell;
      }
    }
    spell = SPELL_PILLAR_SALT;
    if (!::number(0, 6) &&
         ch.doesKnowSkill(spell) && 
           (cutoff < discArray[spell]->start) &&
         (ch.getSkillValue(spell) > 33)) {
      act("$n utters, \"How about a little salt for your wounds!\"", TRUE, &ch, 0, 0, TO_ROOM);
      return spell;
    }
    spell = ch.getSkillNum(SPELL_CALL_LIGHTNING);
    if (!::number(0, 6) && 
         vict.outside() &&
         ((Weather::getWeather(*vict.roomp) == Weather::RAINY) ||
	  (Weather::getWeather(*vict.roomp) == Weather::LIGHTNING)) &&
         ch.doesKnowSkill(spell) && 
           (cutoff < discArray[spell]->start) &&
         (ch.getSkillValue(spell) > 33)) {
      act("$n utters the words, 'Come forth lightning!'", TRUE, &ch, 0, 0, TO_ROOM);
      return spell;
    }
    spell = SPELL_SPONTANEOUS_COMBUST;
    if (!::number(0, 6) &&
         ch.doesKnowSkill(spell) && 
           (cutoff < discArray[spell]->start) &&
         (ch.getSkillValue(spell) > 33)) {
      act("$n utters, \"Burn for your sins!\"", TRUE, &ch, 0, 0, TO_ROOM);
      return spell;
    }
    spell = SPELL_PILLAR_SALT;
    if (!::number(0, 6) &&
         ch.doesKnowSkill(spell) && 
           (cutoff < discArray[spell]->start) &&
         (ch.getSkillValue(spell) > 33)) {
      act("$n utters, \"How about a little salt for your wounds!\"", TRUE, &ch, 0, 0, TO_ROOM);
      return spell;
    }
    spell = SPELL_FLAMESTRIKE;
    if (!::number(0, 4) &&
         ch.doesKnowSkill(spell) && 
           (cutoff < discArray[spell]->start) &&
         (ch.getSkillValue(spell) > 33)) {
      act("$n biblethumps, \"Hellfire and Lava unleashed!\"", TRUE, &ch, 0, 0, TO_ROOM);
      return spell;
    }
    spell = ch.getSkillNum(SPELL_CURSE);
    if (!::number(0, 4) && !vict.affectedBySpell(spell) &&
         ch.doesKnowSkill(spell) && 
           (cutoff < discArray[spell]->start) &&
         (ch.getSkillValue(spell) > 33)) {
      act("$n utters, \"!&*!*#@(*&\"", TRUE, &ch, 0, 0, TO_ROOM);
      return spell;
    }
    spell = ch.getSkillNum(SPELL_RAIN_BRIMSTONE);
    if (!::number(0, 1) &&
         ch.doesKnowSkill(spell) && 
           (cutoff < discArray[spell]->start) &&
         (ch.getSkillValue(spell) > 33)) {
      act("$n utters, \"Let's see how this smells!\"", TRUE, &ch, 0, 0, TO_ROOM);
      return spell;
    }
  } else if (best_disc == DISC_AFFLICTIONS) {

#if 0
    spell = SPELL_HARM_FULL;
    if (!::number(0, 6) &&
         ch.doesKnowSkill(spell) && 
           (cutoff < discArray[spell]->start) &&
         (ch.getSkillValue(spell) > 33)) {
      act("$n utters the words, 'Hurts, doesn't it?\?'",
               TRUE, &ch, 0, 0, TO_ROOM);
      return spell;
    }
#endif
    spell = SPELL_WITHER_LIMB;
    if (!::number(0, 6) &&
        ch.doesKnowSkill(spell) && 
        (cutoff < discArray[spell]->start) &&
        (ch.getSkillValue(spell) > 33) &&
        ch.canWither(&vict, SILENT_YES)) {
      act("$n utters the words, 'Wither away!'", TRUE, &ch, 0, 0, TO_ROOM);
      return spell;
    }
    spell = ch.getSkillNum(SPELL_HARM);
    if (!::number(0, 6) &&
         ch.doesKnowSkill(spell) && 
           (cutoff < discArray[spell]->start) &&
         (ch.getSkillValue(spell) > 33)) {
      act("$n utters the words, 'Ichor and Gore'", TRUE, &ch, 0, 0, TO_ROOM);
      return spell;
    }
    spell = SPELL_PARALYZE;
    if (!::number(0, 6) &&
         !vict.affectedBySpell(spell) &&
         ch.doesKnowSkill(spell) && 
           (cutoff < discArray[spell]->start) &&
         (ch.getSkillValue(spell) > 33)) {
      act("$n utters the words, 'Stay put!'", TRUE, &ch, 0, 0, TO_ROOM);
      return spell;
    }
    spell = SPELL_BONE_BREAKER;
    if (!::number(0, 6) &&
        ch.doesKnowSkill(spell) && 
        (cutoff < discArray[spell]->start) &&
        (ch.getSkillValue(spell) > 33) &&
        ch.canBoneBreak(&vict, SILENT_YES)) {
      act("$n utters the words, 'Sticks and Stones!'", TRUE, &ch, 0, 0, TO_ROOM);
      return spell;
    }
    spell = ch.getSkillNum(SPELL_HARM_CRITICAL);
    if (!::number(0, 6) &&
         ch.doesKnowSkill(spell) && 
           (cutoff < discArray[spell]->start) &&
         (ch.getSkillValue(spell) > 33)) {
      act("$n utters the words, 'Pain and Suffering'", TRUE, &ch, 0, 0, TO_ROOM);
      return spell;
    }
    spell = ch.getSkillNum(SPELL_POISON);
    if (!::number(0, 6) &&
         !vict.affectedBySpell(spell) &&
         ch.doesKnowSkill(spell) && 
           (cutoff < discArray[spell]->start) &&
         (ch.getSkillValue(spell) > 33)) {
      act("$n utters, \"Mold and Mushrooms!\"", TRUE, &ch, 0, 0, TO_ROOM);
      return spell;
    }
    spell = SPELL_BLINDNESS;
    if (!::number(0, 6) &&
         !vict.affectedBySpell(spell) &&
         ch.doesKnowSkill(spell) && 
           (cutoff < discArray[spell]->start) &&
         (ch.getSkillValue(spell) > 33)) {
      act("$n utters, \"Darkness...COME TO ME!!!\"", TRUE, &ch, 0, 0, TO_ROOM);
      return spell;
    }
    spell = SPELL_DISEASE;
    if (!::number(0, 6) &&
         ch.doesKnowSkill(spell) && 
           (cutoff < discArray[spell]->start) &&
         (ch.getSkillValue(spell) > 33)) {
      act("$n beseeches, \"Oh lord, unleash pestilence upon this unrighteous one!\"", TRUE, &ch, 0, 0, TO_ROOM);
      return spell;
    }
    spell = ch.getSkillNum(SPELL_INFECT);
    if (!::number(0, 6) &&
         ch.doesKnowSkill(spell) && 
           (cutoff < discArray[spell]->start) &&
         (ch.getSkillValue(spell) > 33)) {
      act("$n utters the words, 'Pox and Smegma!'", TRUE, &ch, 0, 0, TO_ROOM);
      return spell;
    }
    spell = ch.getSkillNum(SPELL_HARM_SERIOUS);
    if (!::number(0, 6) &&
         ch.doesKnowSkill(spell) && 
           (cutoff < discArray[spell]->start) &&
         (ch.getSkillValue(spell) > 33)) {
      act("$n utters the words, 'Cuts and Bruises'", TRUE, &ch, 0, 0, TO_ROOM);
      return spell;
    }
    spell = SPELL_PARALYZE_LIMB;
    if (!::number(0, 6) &&
        ch.doesKnowSkill(spell) && 
        (cutoff < discArray[spell]->start) &&
        (ch.getSkillValue(spell) > 33) &&
        ch.canParalyzeLimb(&vict, SILENT_YES)) {
      act("$n utters the words, 'Be Still!'",
               TRUE, &ch, 0, 0, TO_ROOM);
      return spell;
    }
    spell = SPELL_BLEED;
    if (!::number(0, 6) &&
         !vict.affectedBySpell(spell) &&
         ch.doesKnowSkill(spell) && 
           (cutoff < discArray[spell]->start) &&
         (ch.getSkillValue(spell) > 33)) {
      act("$n utters the words, 'Blood Drive!'", TRUE, &ch, 0, 0, TO_ROOM);
      return spell;
    }
    spell = ch.getSkillNum(SPELL_HARM_LIGHT);
    if (!::number(0, 6) &&
         ch.doesKnowSkill(spell) && 
           (cutoff < discArray[spell]->start) &&
         (ch.getSkillValue(spell) > 33)) {
      act("$n utters the words, 'This will only hurt a little...'", TRUE, &ch, 0, 0, TO_ROOM);
      return spell;
    }
  } else if (best_disc == DISC_CURES) {
    spell = SPELL_HEAL_FULL;
    if (!::number(0, 6) &&
         (ch.getHit() < ch.hitLimit()) &&
         ch.doesKnowSkill(spell) && 
           (cutoff < discArray[spell]->start) &&
         (ch.getSkillValue(spell) > 33)) {
      act("$n utters the words, 'I feel REAL good'",
               TRUE, &ch, 0, 0, TO_ROOM);
      on_me = TRUE;
      return spell;
    }
    spell = SPELL_HEAL;
    if (!::number(0, 6) &&
         (ch.getHit() < ch.hitLimit()) &&
         ch.doesKnowSkill(spell) && 
           (cutoff < discArray[spell]->start) &&
         (ch.getSkillValue(spell) > 33)) {
      act("$n utters the words, 'JAMES BROWN'",
               TRUE, &ch, 0, 0, TO_ROOM);
      on_me = TRUE;
      return spell;
    }
    spell = SPELL_HEAL_CRITICAL;
    if (!::number(0, 6) &&
         (ch.getHit() < ch.hitLimit()) &&
         ch.doesKnowSkill(spell) && 
           (cutoff < discArray[spell]->start) &&
         (ch.getSkillValue(spell) > 33)) {
      act("$n utters the words, 'Comfort and Joy'",
               TRUE, &ch, 0, 0, TO_ROOM);
      on_me = TRUE;
      return spell;
    }
    spell = ch.getSkillNum(SPELL_HEAL_SERIOUS);
    if (!::number(0, 6) &&
         (ch.getHit() < ch.hitLimit()) &&
         ch.doesKnowSkill(spell) && 
           (cutoff < discArray[spell]->start) &&
         (ch.getSkillValue(spell) > 33)) {
      act("$n utters the words, 'I feel much better now'",
               TRUE, &ch, 0, 0, TO_ROOM);
      on_me = TRUE;
      return spell;
    }
    spell = ch.getSkillNum(SPELL_HEAL_LIGHT);
    if (!::number(0, 6) &&
         (ch.getHit() < ch.hitLimit()) &&
         ch.doesKnowSkill(spell) && 
           (cutoff < discArray[spell]->start) &&
         (ch.getSkillValue(spell) > 33)) {
      act("$n utters the words, 'I feel better now'",
               TRUE, &ch, 0, 0, TO_ROOM);
      on_me = TRUE;
      return spell;
    }
  } else if (best_disc == DISC_HAND_OF_GOD) {
  } else if (best_disc == DISC_AEGIS) {
    spell = SPELL_SANCTUARY;
    if (!::number(0, 6) &&
         !ch.affectedBySpell(spell) && !ch.isAffected(AFF_SANCTUARY) &&
         ch.doesKnowSkill(spell) && 
           (cutoff < discArray[spell]->start) &&
         (ch.getSkillValue(spell) > 33)) {
      
      switch (::number(1,3)) {
        case 1:
          act("$n utters, \"Safe Haven!\"", 
               TRUE, &ch, 0, 0, TO_ROOM);
        case 2:
        case 3:
        default:
          act("$n utters, \"White aura, surround me!\"", 
               TRUE, &ch, 0, 0, TO_ROOM);
          break;
      }

      on_me = TRUE;
      return spell;
    }
    spell = ch.getSkillNum(SPELL_BLESS);
    if (!::number(0, 6) &&
         !ch.affectedBySpell(SPELL_BLESS) &&
         ch.doesKnowSkill(spell) && 
           (cutoff < discArray[spell]->start) &&
         (ch.getSkillValue(spell) > 33)) {
      act("$n utters, \"What a Rush!\"", 
               TRUE, &ch, 0, 0, TO_ROOM);
      on_me = TRUE;
      return spell;
    }
    spell = ch.getSkillNum(SPELL_ARMOR);
    if (!::number(0, 6) &&
         !ch.affectedBySpell(SPELL_ARMOR) &&
         ch.doesKnowSkill(spell) && 
           (cutoff < discArray[spell]->start) &&
         (ch.getSkillValue(spell) > 33)) {
      act("$n utters the words, 'Abrazak'",
               TRUE, &ch, 0, 0, TO_ROOM);
      on_me = TRUE;
      return spell;
    }
  }
  return TYPE_UNDEFINED;
}


int TMonster::clerMove(TBeing &vict)
{
  bool on_me;
  spellNumT spell=TYPE_UNDEFINED;
  TBeing *tank=NULL, *k, *t=NULL;
  followData *f;

  if (!awake() || cantHit > 0)
    return FALSE;

  if (!canSpeak())
    return FALSE;

  if (eitherArmHurt() ||
      checkSoundproof() || nomagic("No-magic room prevents spell cast."))
    return FALSE;

  if(spelltask && spelltask->rounds==0 && (t=spelltask->victim) &&
     (((t->getHit()*100)/t->hitLimit())<=75) && ::number(0,10)){
    switch(spelltask->spell){
      case SPELL_HEAL_FULL: 
      case SPELL_HEAL_CRITICAL:
      case SPELL_HEAL_SERIOUS:
      case SPELL_HEAL_LIGHT:
	++spelltask->rounds;
      default:
        break;
    }
  }

  // don't cast two spells at once
  if (spelltask)
    return FALSE;

  // ONLY COMBAT STUFF BEYOND HERE
  if (!sameRoom(vict))
    return FALSE;

  // do some healing
  // modified this 3/25/02 so charms only heal thier master - dash
  if(!isAffected(AFF_CHARM) && (tank=vict.fight()) && tank != this && isFriend(*tank)){
    spell = get_cleric_heal_spell(*this, *(t=tank));
    if (spell == TYPE_UNDEFINED) {
      // tank is in ok shape, let's see if other group members need help
      if (!(k = master))
	k = &vict;
      for (f = k->followers; f; f = f->next){
	spell=get_cleric_heal_spell(*this, *(t=f->follower));
	if (spell != TYPE_UNDEFINED)
	  break;
      }
    }
    
    if (spell != TYPE_UNDEFINED && t) {
      //      vlogf(LOG_MISC, format("%s HEALING %s, spell %i") %  name % t->name % spell);
      return doDiscipline(spell, t->name);
    }
  } else if (isAffected(AFF_CHARM) && (tank=vict.fight()) && tank != this && tank == master) {
    spell = get_cleric_heal_spell(*this, *(t=tank));
    if (spell != TYPE_UNDEFINED && t) {
      //      vlogf(LOG_MISC, format("%s HEALING %s, spell %i") %  name % t->name % spell);
      return doDiscipline(spell, t->name);
    }
  }


  spell = get_cleric_spell(*this, vict, on_me);
  if (spell == TYPE_UNDEFINED)
    return FALSE;

  addSkillLag(spell, 0);

  if (on_me) {
    if (IS_SET(discArray[spell]->targets, TAR_SELF_ONLY | TAR_IGNORE | TAR_FIGHT_SELF)) {
      return doDiscipline(spell, "");
    }
    vlogf(LOG_BUG, format("Mob invoking (1) prayer %d on self with possibly bad target flags for spell") %  spell);
    return doDiscipline(spell, name);
  } else {
    return doDiscipline(spell, vict.name);
  }
}

// DELETE_VICT means vict should be deleted
// DELETE_THIS means delete this
int TMonster::takeFirstHit(TBeing &vict)
{
  TThing *stabber;
  int rc;
  TBeing *v2;

  if (checkPeaceful("Peaceful rooms are such a pain...\n\r"))
    return FALSE;

  if (fight())
    return FALSE;   // already fighting

  if (cantHit > 0)
    return FALSE;

  if (!vict.isAffected(AFF_AGGRESSOR))
    SET_BIT(specials.affectedBy, AFF_AGGRESSOR);

  if (hasClass(CLASS_THIEF) && !riding) {
    if ((stabber = heldInPrimHand())) {
      if (dynamic_cast<TBaseWeapon *>(stabber)) {
        if (stabber->isPierceWeapon()) {
          v2 = dynamic_cast<TBeing *>(vict.riding);
	  // basically if victim is < 35 we want backstab to fire
	  // otherwise lets slit their throat
	  // I would be in favor of discussing a better condition
	  // but until then lets go with this
	  if (GetMaxLevel() < 35) {
	    if (v2) {
            // can't backstab them (mounted), scrag the horse instead
	    //            rc = backstabHit(v2, stabber);
	    //            addSkillLag(SKILL_BACKSTAB, 0);

	    // lets not have automatic success
	      rc=doBackstab("", v2);


	      if (IS_SET_DELETE(rc, DELETE_VICT)) {
		if (vict.riding) {
		  vlogf(LOG_MISC, "is this called (ping)?");
		  vict.dismount(POSITION_SITTING);
		}
		delete v2;
		v2 = NULL;
	      }
	      return TRUE;
	    } else {
	    //            rc = backstabHit(&vict, stabber);
	    //            addSkillLag(SKILL_BACKSTAB, 0);
	      rc=doBackstab("", &vict);
	      if (IS_SET_DELETE(rc, DELETE_VICT)) {
		return DELETE_VICT;
	      }
	      return TRUE;
	    }
	  } else {
	  // Thieves should have slit as well
	    if (v2) {
	    // lets not have automatic success
	      rc=doThroatSlit("", v2);

	      if (IS_SET_DELETE(rc, DELETE_VICT)) {
		if (vict.riding) {
		// not sure why were loging here with backstab
		// but may as well do the same for slit
		  vlogf(LOG_MISC, "is this called? (slit-mobact.cc:3183)");
		  vict.dismount(POSITION_SITTING);
		}
		delete v2;
		v2 = NULL;
	      }
	      return TRUE;
	    } else {
	      rc=doThroatSlit("", &vict);
	      if (IS_SET_DELETE(rc, DELETE_VICT)) {
		return DELETE_VICT;
	      }
	      return TRUE;
	    }
          }
        }
      }
    }
  }
  
  // fuzzy logic to figure out how to open fight best.
  if (getMult() > 2.0) {
    // lots of attacks translates into small damage per hit.  (I only get 1
    // hit on opening round.  use a special instead  
    rc = classStuff(vict);
    if (IS_SET_DELETE(rc, DELETE_VICT) || IS_SET_DELETE(rc, DELETE_THIS)) {
      return rc;
    }
    if (!fight()) {
      // special chosen was not offensive
      return hit(&vict);
    }
    return FALSE;
  } else {
    // otherwise, use a normal swing
    return hit(&vict);
  }
}

/* Hopefully this will be robust enough to let the class of the monster */
/* actually mean something. - Russ                                      */
// pre-assumes that mob is standing and fighting
// returns DELETE_THIS if this needs to die
int TMonster::classStuff(TBeing &vict)
{
  if (hasClass(CLASS_WARRIOR)) 
    return fighterMove(vict);
  else if (hasClass(CLASS_THIEF))
    return thiefMove(vict);
  else if (hasClass(CLASS_MONK))
    return monkMove(vict);
  else if (hasClass(CLASS_SHAMAN))
    return shamanMove(vict);
  else if (hasClass(CLASS_DEIKHAN))
    return deikhanMove(vict);
  else if (hasClass(CLASS_RANGER))
    return rangMove(vict);
  else if (hasClass(CLASS_MAGE))
    return mageMove(vict);
  else if (hasClass(CLASS_CLERIC))
    return clerMove(vict);

  return FALSE;
}

// might return DELETE_THIS
int TMonster::scavenge()
{
  TObj *best_obj = 0, *obj = 0;
  int iMax;
  int rc;
  TBaseCorpse *corpse;
  
  if (in_room == Room::DONATION)
    return FALSE;
  if (UtilMobProc(this))
    return FALSE;
  if (!hasHands() && !isHumanoid())
    return FALSE;
  
  if (!roomp->stuff.empty() && !::number(0, 5)) {
    best_obj = NULL;
    iMax=1;
    for(StuffIter it=roomp->stuff.begin();it!=roomp->stuff.end();++it){
      if (!(obj = dynamic_cast<TObj *>(*it)))
        continue;
      
      // skip over unlooted corpses
      corpse = dynamic_cast<TBaseCorpse *>(*it);
      if (corpse && !corpse->stuff.empty())
        continue;
      
      if (obj->canWear(ITEM_TAKE) && canCarry(obj, SILENT_YES) &&
          canSee(obj) && 
          !obj->action_description &&   // checks for personalized
          obj->stuff.empty()) {
        if (obj->obj_flags.cost > iMax) {
          best_obj = obj;
          iMax = obj->obj_flags.cost;
        }
      }
    }
    if (best_obj) {
      rc = checkForAnyTrap(best_obj);
      if (IS_SET_DELETE(rc, DELETE_THIS))
        return DELETE_THIS;
      else if (rc)
        return FALSE;

      --(*best_obj);
      *this += *best_obj;
      vlogf(LOG_MOB_AI, format("Mob scavenge: %s picking up %s in room (%d)") % this->name % best_obj->name % (roomp ? roomp->in_room : 0));
      act("$n gets $p.", FALSE, this, best_obj, 0, TO_ROOM);
      act("You get $p.", FALSE, this, best_obj, 0, TO_CHAR);
      return TRUE;
    }
  }
  return FALSE;
}

// returns DELETE_THIS
int TMonster::notFightingMove(int pulse)
{
  TThing *t=NULL;
  TPerson *cons = NULL;
  int rc;
  TBeing *tmp_ch=NULL;

  if ((spec == SPEC_ATTUNER) && !fight()) {
    static attune_struct *job;
    job = (struct attune_struct *) act_ptr;
    if (job && job->hasJob) {
      return FALSE;
    }
  } 

  if (!(pulse%(7*PULSE_MOBACT))) {
    rc = lookForHorse();
    if (IS_SET_DELETE(rc, DELETE_THIS))
      return DELETE_THIS;
    else if (rc)
      return TRUE;

    for(StuffIter it=roomp->stuff.begin();it!=roomp->stuff.end();){
      t=*(it++);
      if(t == this || !(tmp_ch = dynamic_cast<TBeing *>(t)))
	continue;

      // assist group members faster
      if(isAffected(AFF_GROUP) && inGroup(*tmp_ch) && tmp_ch->fight()){
	rc = assistFriend();
	if (IS_SET_DELETE(rc, DELETE_THIS))
	  return DELETE_THIS;
	else if (rc)
	  return TRUE;
	break;
      } 

      // wimpy mobs don't like standing around fights
      if (!isAffected(AFF_GROUP) && IS_SET(specials.act, ACT_WIMPY) &&
	 !IS_SET(specials.act, ACT_SENTINEL) && tmp_ch->fight() &&
	 ::number(0,1)) {
        act("$n looks distressed.", false, this, 0, 0, TO_ROOM);
        rc = wanderAround();
	if (IS_SET_DELETE(rc, DELETE_THIS))
	  return DELETE_THIS;
	else if (rc)
	  return TRUE;
      }
    }

    if (pissed() || isPolice() || !::number(0,4)) {
      rc = assistFriend();
      if (IS_SET_DELETE(rc, DELETE_THIS))
        return DELETE_THIS;
      else if (rc)
        return TRUE;
    }
  }

  if (!(pulse%(11*PULSE_MOBACT))) {
    if (greed() > 90) {
      if (isSmartMob(10) && !hasClass(CLASS_MONK)) {
        rc = superScavenger();
        if (IS_SET_DELETE(rc, DELETE_THIS))
          return DELETE_THIS;
        else if (rc)
          return TRUE;
      } else {
        rc = scavenge();
        if (IS_SET_DELETE(rc, DELETE_THIS))
          return DELETE_THIS;
        else if (rc)
          return TRUE;
      }
    }
  }

  if (hasClass(CLASS_THIEF) && !(pulse%(13*PULSE_MOBACT))) {
    for(StuffIter it=roomp->stuff.begin();it!=roomp->stuff.end() && (t=*it);++it) {
      cons = dynamic_cast<TPerson *>(t);
      if (!cons)
        continue;
      if (!cons->isImmortal() && !::number(0, 2)) {
        rc = npcSteal(cons);
        if (IS_SET_DELETE(rc, DELETE_THIS) || IS_SET_DELETE(rc, DELETE_VICT))
          return rc;
        return TRUE;
      }
    }
  }

  if (IS_SET(specials.act, ACT_HUNTING) && !(pulse%(4*PULSE_MOBACT))) {
    // this was pulse/5 but was too slow
    rc = hunt();
    if (IS_SET_DELETE(rc, DELETE_THIS))
      return DELETE_THIS;
    else if (rc)
      return TRUE;
    if (fight())
      return TRUE;
  }

  // if I'm in a group and I've been seperated from my leader, track them
  if(isAffected(AFF_GROUP) && master && !master->isPc() &&
     roomp->number!=master->roomp->number)
    setHunting(master);

  if (!IS_SET(specials.act, ACT_SENTINEL)) {
    // this is examined every pulse (no pulse check), i.e. once every 3 secs
    
    // Change this to increase or decrease the chance of mobs moving about
    // each check.
    const int DEF_MOBILE_WANDER_CHANCE = 40;
    
    if (!::number(0,DEF_MOBILE_WANDER_CHANCE)) {
      rc = wanderAround();
      if (IS_SET_DELETE(rc, DELETE_THIS))
        return DELETE_THIS;
      else if (rc)
        return TRUE;
    }
  }

  if (!(pulse%(9*PULSE_MOBACT)) &&
      !spelltask) {
    rc = defendSelf(pulse);
    if (IS_SET_DELETE(rc, DELETE_THIS)) {
      return DELETE_THIS;
    } else if (rc)
      return TRUE;

    for(StuffIter it=roomp->stuff.begin();it!=roomp->stuff.end();){
      t=*(it++);
      tmp_ch = dynamic_cast<TBeing *>(t);
      
      // spellup friendly people
      if (t != this && tmp_ch && 
	  !tmp_ch->isPc() &&
	  canSee(tmp_ch) && isFriend(*tmp_ch) && !tmp_ch->isDumbAnimal() ){
	rc = defendOther(*tmp_ch);
	if (IS_SET_DELETE(rc, DELETE_THIS)) {
	  return DELETE_THIS;
	} else if (rc)
	  return TRUE;
      } 
    }   
  }

  // if I need to preen to be able to fly, let's keep ourselves flight-ready
  if (!(pulse%(9*PULSE_MOBACT)) && !spelltask && !task && canFly() &&
    race->isWinged() && race->isFeathered() && !isAffected(AFF_FLIGHTWORTHY)) {
    sstring self = "";
    doPreen(self);
  }

  return FALSE;
}

// this function is called once per mob every PULSE_MOBACT
// The best way to use "pulse" is to make it refer to PULSE_MOBACT so that
// events occur every call, every xth call, etc.
// Also, since this routine is VERY-HIGH USE, this function is *CRUCIAL*!!!
// to the overall mud performance.  Code extremely tightly anything inside
// this routine (or sub-routine).
// returns DELETE_THIS if this dead
int TMonster::mobileActivity(int pulse)
{
  TBeing *tmp_ch = NULL, *vict=NULL;
  affectedData *af, *af2;
  int  iHeight;
  int rc;
  TThing *t=NULL;

#if 0
  // this doesn't work right
  // these check if the mob is diurnal/nocturnal and its night/day
  // meaning we have mob in the world who aren't doing mobileActivity
  if (isDiurnal() || isNocturnal())
    return 0;
#else
  // this is better
  if(inRoom() == Room::NOCTURNAL_STORAGE)
    return 0;
#endif

  if(parent || !roomp)
    return 0;
  
  if (isAquatic() && !IS_SET(specials.act, ACT_IMMORTAL) && (specials.zone != 1) && (mobVnum() >= 0) &&
     !roomp->isWaterSector() && !roomp->isUnderwaterSector() &&
     !inImperia() && getRace() != RACE_FISHMAN) {
    vlogf(LOG_MISC, format("Fish (%s), found out of water in %s (%d)! Destroying!") %  
           getName() % roomp->getName() % in_room);
    return DELETE_THIS;
  }
  // assign old room if not set
  if (oldRoom == Room::NOWHERE)
    oldRoom = inRoom();

#if 0 
  if (isPet(PETTYPE_PET) && !isElemental()) {
    if (getCond(FULL) < 0) {
      setCond(FULL, 20);
    } else if (getCond(FULL) > 0) {
      gainCondition(FULL, -1);
    }
    if (getCond(THIRST) < 0) {
      setCond(THIRST, 20);
    } else if (getCond(THIRST) > 0) {
      gainCondition(THIRST, -1);
    }
    if (!fight() && !spelltask && awake()) {
      if (master && sameRoom(*master)) {
         if (getCond(FULL) < 3)
           act("$n looks at you with hunger in its eyes.", FALSE, master, 0, this, TO_CHAR);
         if (getCond(THIRST) < 3)
           act("$n looks at you with thirst in its eyes.", FALSE, master, 0, this, TO_CHAR);
      }
    }
  } 
#endif

  // we could use affectedBySpell() but those will scroll through affects
  // each call.  Doing it once here is a lot faster.
  for (af = affected; af; af = af2) {
    af2 = af->next;
    if (af->type == SPELL_PLAGUE_LOCUSTS) {
      if (!master || !fight() || !sameRoom(*master)) {
        act("$n circles the area once and then dissipates.",
               TRUE, this, 0, 0, TO_ROOM);
        return DELETE_THIS;
      }
    }
    if (af->type == SPELL_STICKS_TO_SNAKES) {
      if (!master || !master->fight() || !fight() || !sameRoom(*master)) {
        act("$n goes away.", TRUE, this, 0, 0, TO_ROOM);
        return DELETE_THIS;
      }
    }  
    if (af->type == SKILL_TRANSFIX) {
      if (!::number(0, 5)) {
        act("$n looks glazed-eyed and content.", TRUE, this, NULL, NULL, TO_ROOM);
        return TRUE;
      }
    }
  }
  // handle NPCs skill/spell lag
  if (getWait() > 0) {
    wait -= min(wait, (short int) PULSE_MOBACT);
    return FALSE;
  }

  if (isAffected(AFF_SLEEP) && getPosition() == POSITION_SLEEPING)
    return FALSE;

  // if not in a fighting stance, and in a fight...
  // stand up and fight like a mob
  if ((getPosition() < POSITION_CRAWLING) && 
          (fight() || (default_pos > POSITION_SITTING))) {
    if ((spec == SPEC_ATTUNER) && !fight()) {
      static attune_struct *job;
      job = (struct attune_struct *) act_ptr;
      if (!job || (!(job->hasJob) && !(pulse%(1 * PULSE_MOBACT)))) {
        if (!isPet(PETTYPE_PET | PETTYPE_CHARM | PETTYPE_THRALL) && ::number(0, 1)) {
          rc = standUp();
          if (rc)
            return FALSE;
        }
      }
    } else if (!(pulse%(1 * PULSE_MOBACT))) {
      if (!isPet(PETTYPE_PET | PETTYPE_CHARM | PETTYPE_THRALL) && ::number(0, 1)) {
        rc = standUp(); 
        if (rc)
          return FALSE;
      }
    }
    // if we didn't stand up, fall through
  } else if (riding && !dynamic_cast<TBeing *> (riding) &&
             (fight() || (default_pos > POSITION_SITTING))) {
    // on a chair, stand up and fight like a mob
    if ((spec == SPEC_ATTUNER) && !fight()) {
      static attune_struct *job;
      job = (struct attune_struct *) act_ptr;
      if (!job || (!(job->hasJob) && !(pulse%(1 * PULSE_MOBACT)))) {
        if (!isPet(PETTYPE_PET | PETTYPE_CHARM | PETTYPE_THRALL) && ::number(0, 1)) {
          rc = standUp();/* stand up and fight like a mob */
          if (rc)
            return FALSE;
        }
      }
    } else if (!(pulse%(1 * PULSE_MOBACT))) {
      if (!isPet(PETTYPE_PET | PETTYPE_CHARM | PETTYPE_THRALL) && ::number(0, 1)) {
        rc = standUp();            /* stand up and fight like a mob */
        if (rc)
          return FALSE;
      }
    }
  } else if (getPosition() <= POSITION_SITTING) {
    // if we are hunting from sitting position, go get the bastard!
    if (IS_SET(specials.act, ACT_HUNTING)) {
      rc = standUp(); 
      if (rc)
        return FALSE;
    } 
    if (!(pulse%(36*PULSE_MOBACT)) &&
        !riding &&
        !fight() &&
        (getHit() >= hitLimit()) &&
        !isPet(PETTYPE_PET | PETTYPE_CHARM | PETTYPE_THRALL)) {
      // anything more comfortable around?
      std::map <sstring, short int> bedsChecked;
      for(StuffIter it=roomp->stuff.begin();it!=roomp->stuff.end() && (t=*it);++it) {
      // added the hash to keep track of 1.chair, 2.chair, etc
        if (bedsChecked.find(fname(t->name)) == bedsChecked.end()) {
          bedsChecked[fname(t->name)] = 1;
        } else { bedsChecked[fname(t->name)] += 1; }
        
        if (t->mobPulseBed(this, bedsChecked[fname(t->name)]))
          return TRUE;
      }
    }
  } else if ((default_pos <= POSITION_CRAWLING) && 
              !fight() &&
              (getPosition() > POSITION_CRAWLING) && 
              !IS_SET(specials.act, ACT_HUNTING) &&
              (getHit() >= hitLimit()) &&
              (spec != SPEC_BOUNTY_HUNTER)) {
    if (!(pulse%(30 * PULSE_MOBACT))) {
      if (default_pos == POSITION_SITTING)
        doSit("");
      else if (default_pos == POSITION_RESTING)
        doRest("");
      else if (default_pos == POSITION_SLEEPING)
        doSleep("");
      else if (default_pos == POSITION_CRAWLING)
        doCrawl();
      return TRUE;
    }
  }

  if (!(pulse%(2 * PULSE_MOBACT))) {
    // rescue group members if needed
    // this needs high priority so we put it here instead of fighterMove
    if(hasClass(CLASS_WARRIOR) && isAffected(AFF_GROUP) &&
       (vict=fight()) && (tmp_ch=vict->fight()) && 
       tmp_ch != this && inGroup(*tmp_ch) &&
       (tmp_ch->hasClass(CLASS_MAGE | CLASS_CLERIC | CLASS_THIEF) ||
	((tmp_ch->getHit()*100)/tmp_ch->hitLimit())<50)){
      if (!::number(0,2)) {
	act("$n has rescued you!",FALSE,this,0,tmp_ch,TO_VICT);
	act("$n has rescued $N.",TRUE,this,0,tmp_ch,TO_NOTVICT);
	stopFighting();
	vict->stopFighting();
        if (tmp_ch->fight())
          tmp_ch->stopFighting();
	setCharFighting(vict);
	vict->setCharFighting(this);
	vict->addToWait(combatRound(1));
      } else {
	act("$n tries to rescue you but fails.",FALSE,this,0,tmp_ch,TO_VICT);
	act("$n tries to rescue $N but fails.",TRUE,this,0,tmp_ch,TO_NOTVICT);
      }
      addSkillLag(getSkillNum(SKILL_RESCUE), 0);
      return TRUE;
    }
  }

  if (!(pulse%(5 * PULSE_MOBACT))) {
    rc = charmeeStuff();
    if (IS_SET_DELETE(rc, DELETE_THIS))
      return DELETE_THIS;
    else if (rc)
      return TRUE;

    rc = protectionStuff();
    if (IS_SET_DELETE(rc, DELETE_THIS))
      return DELETE_THIS;
    else if (rc)
      return TRUE;

    if (!::number(0,4))
      bumpHead(&iHeight);
  }

  if (isShopkeeper() && !(pulse %(200*PULSE_MOBACT)) && !Config::NoSpecials()){
    unsigned int shop_nr;
    
    for (shop_nr = 0; (shop_nr < shop_index.size()) && (shop_index[shop_nr].keeper != number); shop_nr++);
    
    if (shop_nr >= shop_index.size()) {
      vlogf(LOG_BUG, format("Warning... shop # for mobile %d (real nr) not found.") %  mob_index[number].virt);
      return FALSE;
    }
    
    if(shop_index[shop_nr].isOwned()){
      TDatabase db(DB_SNEEZY);
#if 0

      int salary=1000;
      TObj *o;
      int value=0;
      sstring tname="";
      bool found=true;

      if(!(pulse%1200)){
	// basically we just toss items in the dump until we can afford our
	// salary
	while(getMoney() < salary && found){
	  found=false;
	  for(TThing *t=getStuff();t;t=t->nextThing){
	    if((o=dynamic_cast<TObj *>(t))){
	      tname=o->getName();
	      o->thingDumped(this, &value);
	      shoplog(shop_nr, this, this, tname, value, "dumped");
	      setMoney(getMoney()+value);
	      found=true;
	      break;
	    }
	  }
	}
	
	setMoney(getMoney()-salary);
	shoplog(shop_nr, this, this, "talens", -salary, "salary");
      }
#endif

    }
  }

  if (spec && !(pulse %(50*PULSE_MOBACT)) && !::number(0, 1) && 
      !Config::NoSpecials()) {
    rc = checkSpec(this, CMD_MOB_ALIGN_PULSE, "", NULL);
    if (IS_SET_DELETE(rc, DELETE_THIS) || IS_SET_DELETE(rc, DELETE_VICT))
      return DELETE_THIS;
    else if (rc)
      return TRUE;
  }
  // trigger spec_procs

  // should do this in socket.cc
  if (spec && !(pulse %(2 * PULSE_MOBACT)) && !Config::NoSpecials()) {
    rc = checkSpec(this, CMD_GENERIC_PULSE, "", NULL);
    if (IS_SET_DELETE(rc, DELETE_VICT) || IS_SET_DELETE(rc, DELETE_THIS))
      return DELETE_THIS;
    else if (rc)
      return TRUE;
  }


  if ( !awake() || desc || IS_SET(specials.act, ACT_POLYSELF) || 
      spec == SPEC_HERO_FAERIE )
    return FALSE;

  // yank out stuck-in
  if (!(pulse%(16 * PULSE_MOBACT))) {
    rc = remove();
    if (IS_SET_DELETE(rc, DELETE_THIS))
      return DELETE_THIS;
    else if (rc)
      return TRUE;
  }

  //  beyond here, require mob be standing
  if (getPosition() <= POSITION_SITTING)
    return FALSE;

  if (!(pulse%(5 * PULSE_MOBACT))) {
    if (isSmartMob(15))
      if (senseWimps())
        return TRUE;
    if (isSmartMob(20) && !::number(0,5)) {
      rc = findABetterWeapon();
      if (IS_SET_DELETE(rc, DELETE_THIS))
        return DELETE_THIS;
      else if (rc)
        return TRUE;
    }
  }

  tmp_ch = fight();
  if (tmp_ch) {
    if (IS_SET(specials.act, ACT_AFRAID)) {
      rc = fearCheck(NULL, true);
      if (IS_SET_DELETE(rc, DELETE_THIS))
        return DELETE_THIS;
      if (rc)
        return TRUE;
    }

    // a lot of specs are combat-related, trigger these
    if (spec) {
      rc = checkSpec(this, (::number(0,2)) ? CMD_MOB_COMBAT : CMD_MOB_COMBAT2, "", NULL);
      if (IS_SET_DELETE(rc, DELETE_VICT) || IS_SET_DELETE(rc, DELETE_THIS))
        return DELETE_THIS;
      else if (rc)
        return TRUE;
    }

    // Do the vampire/lycanthrope stuff
    if (!::number(0, 74) && !tmp_ch->isLucky(levelLuckModifier(GetMaxLevel()))) {
      if ((isVampire() && (tmp_ch->hitLimit() < hitLimit()) &&
         (GetMaxLevel() > tmp_ch->GetMaxLevel() + 10) &&
         hits(tmp_ch, attackRound(tmp_ch) - tmp_ch->defendRound(this))) ||
         ((getMyRace()->isLycanthrope()) &&
         (!isPc() || hasQuestBit(TOG_TRANSFORMED_LYCANTHROPE)))) {
        rc = doBite(tmp_ch->name);
        if (IS_SET_DELETE(rc, DELETE_VICT) || IS_SET_DELETE(rc, DELETE_THIS))
		{
			if (IS_SET_DELETE(rc, DELETE_VICT))
				vlogf(LOG_BUG, "PAPPY: Why return DELETE_THIS if DELETE_VICT?");
			return DELETE_THIS;
		}
        else if (rc)
          return TRUE;
      }
    }

    // either no spec or decided against it
    // go into normal class-related attacks


    rc = classStuff(*tmp_ch);

    if (IS_SET_DELETE(rc, DELETE_VICT)) {
      delete tmp_ch;
      tmp_ch = NULL;
    }
    if (IS_SET_DELETE(rc, DELETE_THIS))
      return DELETE_THIS;
    return TRUE;
  } else {
    rc = notFightingMove(pulse);
    if (IS_SET_DELETE(rc, DELETE_THIS))
      return DELETE_THIS;
    else if (rc)
      return TRUE;

    if (getHit() > (hitLimit() / 2)) {
      if (IS_SET(specials.act, ACT_HATEFUL)) {
        rc = doHatefulStuff();
        if (IS_SET_DELETE(rc, DELETE_THIS))
          return DELETE_THIS;
        else if (rc)
          return TRUE;
      }
      if (IS_SET(specials.act, ACT_AFRAID)) {
        rc = fearCheck(NULL, true);
        if (IS_SET_DELETE(rc, DELETE_THIS))
          return DELETE_THIS;
        if (rc)
          return TRUE;

        // if they heal back to max, make them track down the bad guys
        // don't do it if we have busted limbs
        // we also wait for the effects of a "fear" spell to decay
        if (getHit() == (hitLimit()*4)/5 &&
            !eitherLegHurt() && !eitherArmHurt() &&
            !affectedBySpell(SPELL_FEAR) &&
            !affectedBySpell(SPELL_FEAR)) {
	  if(fears.clist) {
	    int safety = 0;
	    while(fears.clist && safety < 10) {
	      if (fears.clist && 
		  (tmp_ch = get_char(fears.clist->name, EXACT_YES)) &&
		  !IS_SET(specials.act, ACT_HUNTING)) {
		remFeared(tmp_ch, NULL);
		setHunting(tmp_ch);
		addHated(tmp_ch);
	      }
	      safety++; // this is just to make sure we dont go into an infinite loops here
	    }
	    return TRUE;
          }
        }
      }
    } else if (getHit() <= (hitLimit()/4)) {
      // hp <= 1/2 max
      rc = fearCheck(NULL, true);
      if (IS_SET_DELETE(rc, DELETE_THIS))
        return DELETE_THIS;
      if (rc)
        return TRUE;

      if (IS_SET(specials.act, ACT_HATEFUL)) {
        rc = doHatefulStuff();
        if (IS_SET_DELETE(rc, DELETE_THIS))
          return DELETE_THIS;
        else if (rc)
          return TRUE;
      }
    } else if (IS_SET(specials.act, ACT_HATEFUL)) {
      rc = doHatefulStuff();
      if (IS_SET_DELETE(rc, DELETE_THIS))
	return DELETE_THIS;
      else if (rc)
	return TRUE;
    }
    rc = aggroCheck(true);
    if (IS_SET_DELETE(rc, DELETE_THIS))
      return DELETE_THIS;
  }
  return FALSE;
}

// ch will be NULL for mobpulse=true
// otherwise, it is basically a roomenter case
int TMonster::fearCheck(const TBeing *ch, bool mobpulse)
{
  // this function causes the mobs to flee
  // since this is semi-annoying for players, lets have it "miss" flees
  // a lot of the time.
  if (::number(0,3))
    return false;

  if (getHit() > hitLimit()/4 && (!eitherLegHurt() && !eitherArmHurt()) &&
      !affectedBySpell(SPELL_FEAR) && !affectedBySpell(SPELL_FEAR))
    return false;


  if (IS_SET(specials.act, ACT_AFRAID)) {
    if (!ch) {
      // see if anyone nearby is someone I fear
      TThing *t=NULL;
      for(StuffIter it=roomp->stuff.begin();it!=roomp->stuff.end();++it) {
	t=*it;
        if (t == this)
          continue;
        TBeing *tmpch = dynamic_cast<TBeing *>(t);
        if (!tmpch)
          continue;
        if (!canSee(tmpch))
          continue;
        if (Fears(tmpch, NULL))
          break;
      }
      if (!t)
        return FALSE;
      // I only get here if someone in the room I fear
    } else {
      // see if person that entered is someone I fear
      if (!Fears(ch, NULL) || !canSee(ch)) {
        // person entered is not someone I fear
        return false;
      }
    }

    // mobpulse is per call to mobileActivity()
    // the other case is mostlikely on roomenter
    if (mobpulse || ::number(0,199) < min(max(10, (int) GetMaxLevel()), 70)) {
      int rc = doFlee("");
      if (IS_SET_DELETE(rc, DELETE_THIS))
        return DELETE_THIS;
      return TRUE;
    }
  }
  return FALSE;
}


int TMonster::factionAggroCheck()
{
  TThing *t;
  TPerson *tmp_ch;
  int rc;

  // if the mob is neither cult or brother, or is a cult or brother
  // but not in their hometown, then don't aggro
  if((!isCult() && !isBrother()) ||
     (isCult() && !inLogrus()) ||
     (isBrother() && !inBrightmoon())){
    return FALSE;
  }

  if (fight())
    return FALSE;

  if (desc)
    return FALSE;

    // Scroll through stuff list to get count of valid things to hit, 

  for(StuffIter it=roomp->stuff.begin();it!=roomp->stuff.end();){
    t=*(it++);
    // cast to Person, rather than isPc(), so poly'd chars are safe 
    tmp_ch = dynamic_cast<TPerson *>(t);
    if (!tmp_ch)
      continue;

    if (canSee(tmp_ch) && (getPosition() >= POSITION_STANDING)) {
      if (tmp_ch->isImmortal() && tmp_ch->isPlayerAction(PLR_NOHASSLE)) {
	continue;
      }

      if (checkPeaceful("You can't seem to exercise your violent tendencies.\n\r")) {
	if (!::number(0, 4)) {
	  aiGrowl(NULL);
	}
	return TRUE;
      }

      if(((isCult() && (tmp_ch->isBrother() || tmp_ch->isSnake())) ||
	  (isBrother() && tmp_ch->isCult())) &&
	 !::number(0,3)){
	
	if(isCult() && tmp_ch->isBrother()){
	  switch(::number(0,3)){
	    case 0:
	      doAction(fname(tmp_ch->name), CMD_FLIPOFF);
	      doAction("", CMD_SCREAM);	  
	      break;
	    case 1:
	      doSay("Religious psycho-babbeling weirdos are not welcome here!");
	      break;
	    case 2:
	      doSay("Your stench makes me ill, Galekian.");
	      break;
	    case 3:
	      doSay("Now is your chance to meet Galek, face to face!");
	      doAction("", CMD_CACKLE);
	      break;
	  }
	} else if(isCult() && tmp_ch->isSnake()){
	  // check for trade pass that prevents aggro first
	  TObj *pass;
	  for(StuffIter it=tmp_ch->stuff.begin();it!=tmp_ch->stuff.end();++it){
	    if((pass=dynamic_cast<TObj *>(*it)) && pass->objVnum()==8879){
	      if(!::number(0,4))
		doEmote("checks your papers carefully and then lets you pass.");
	      return FALSE;
	    }
	  }

	  switch(::number(0,3)){
	    case 0:
	      doSay("You are not welcome, manipulator.");
	      break;
	    case 1:
	      doSay("You are either with us or against us.  You will take your choice to the grave!");
	      break;
	    case 2:
	      doSay("The streets of Logrus will run red with the blood of our enemies!");
	      doAction("", CMD_SCREAM);
	      break;
	    case 3:
	      doSay("I will eat you in a sandwich.");
	      doAction("", CMD_DROOL);
	      break;
	  }
	} else if(isBrother()){
	  switch(::number(0,3)){
	    case 0:
	      doSay("Get thee back to the festering hole whence thou came, Logrite!");
	      doEmote("screams with rage as he charges into battle.");
	      break;
	    case 1:
	      doSay("I shall vanquish thee!");
	      doEmote("rushes to the defense of the city.");
	      break;
	    case 2:
	      doSay("Silly Logrite, Brightmoon is for Brothers!");
	      doEmote("leaps into battle.");
	      break;
	    case 3:
	      doSay("Rally the guard!  There are Logrites about!");
	      doEmote("charges into the fray.");
	      break;
	  }	      
	} else {
	  act("$n snarls angrily and attacks!",TRUE,this,0,0,TO_ROOM);
	  vlogf(LOG_BUG, "faction confusion in factionAggroCheck()");
	}
	

	rc = takeFirstHit(*tmp_ch);
	if (IS_SET_DELETE(rc, DELETE_VICT)) {
	  delete tmp_ch;
	  tmp_ch = NULL;
	  return DELETE_VICT;
	} else if (IS_SET_DELETE(rc, DELETE_THIS)) 
	  return DELETE_THIS;
	
	return TRUE;
      }
    }
  }
  return FALSE;
}



int TMonster::aggroCheck(bool mobpulse)
{
  TThing *t;
  TBeing *tmp_ch;
  int rc, numtargets, whichtarget;
  numtargets = 0;
  bool hasWandered = FALSE;

  rc = factionAggroCheck();
  if (IS_SET_DELETE(rc, DELETE_VICT)) {
    return DELETE_VICT;
  } else if (IS_SET_DELETE(rc, DELETE_THIS)) {
    return DELETE_THIS;
  } else if (rc) {
    return TRUE;
  }

  if (fight())
    return FALSE;

  if (desc && !hasQuestBit(TOG_TRANSFORMED_LYCANTHROPE))
    return FALSE;

  // OK check this out.  I want to make sentenial aggro mobs not aggro
  // unless they are in their starting room, and stay_zone aggro mobs
  // not aggro unless they are in the zone that include their starting
  // room.  This works in conjunction with the free tracking code i
  // set up to make sure high level aggro mobs don't go and fuck
  // lowbies up, but should also make mobs act like they should under
  // normal circumstances ok, lets do it. -Dash 4/7/01

  TRoom *rp1 = NULL, *rp2 = NULL;

  if (!(rp1 = roomp) || !(rp2 = real_roomp(brtRoom)))
    return FALSE;
  if (IS_SET(specials.act, ACT_STAY_ZONE) && rp1->getZoneNum() != rp2->getZoneNum())
    hasWandered = TRUE;
  if (IS_SET(specials.act, ACT_SENTINEL) && rp1 != rp2)
    hasWandered = TRUE;

  // ok now we know if theyre out of their home area, now we just tack
  // on a level check later on to protect lowbies


  // Some randomization of who gets attacked added - Brutius 1/28/97
  if ((aggro() && (getHit() >= (hitLimit()/2)))){
    // Scroll through stuff list to get count of valid things to hit, 

    for(StuffIter it=roomp->stuff.begin();it!=roomp->stuff.end();){
      t=*(it++);
      tmp_ch = dynamic_cast<TBeing *>(t);
      if (!tmp_ch || !tmp_ch->isPc())
        continue;

      if (canSee(tmp_ch) && (getPosition() >= POSITION_STANDING)) {
        if (tmp_ch->isImmortal() && tmp_ch->isPlayerAction(PLR_NOHASSLE)) {
          continue;
        }
	numtargets++; // we found a valid target
      }
    }

    whichtarget = 0;
    if (numtargets) whichtarget = ::number(1,numtargets);
    // randomly choose one of the targets we found
    numtargets = 0;
    if(!mobpulse) whichtarget = -1;  // no randomization since we're attacking on roomenter
    for(StuffIter it=roomp->stuff.begin();it!=roomp->stuff.end();){
      t=*(it++);
      tmp_ch = dynamic_cast<TBeing *>(t);
      if (!tmp_ch || !tmp_ch->isPc())
        continue;

      if (canSee(tmp_ch) && (getPosition() >= POSITION_STANDING)) {
        if (tmp_ch->isImmortal() && tmp_ch->isPlayerAction(PLR_NOHASSLE)) {
          continue;
        }
	numtargets++;
	if(numtargets != whichtarget && whichtarget != -1) continue;
	stats.aggro_attempts++;

	if (mobpulse && !::number(0,4))
	  continue;

        if (checkPeaceful("You can't seem to exercise your violent tendencies.\n\r")) {
          if (!::number(0, 4)) {
            aiGrowl(NULL);
          }
          return TRUE;
        }
        if (((tmp_ch->plotStat(STAT_CURRENT, STAT_KAR, 0, 100, 50)) <=
	     (plotStat(STAT_CURRENT, STAT_INT, 0, 200, 100) + anger())) ||
	    IS_SET(specials.act, ACT_AGGRESSIVE)){

          if (!isDumbAnimal() && ::number(0,9)) {
            // This should basically prevent mobs from attacking when they
            // are grossly out-armed and they know it.  But it also gives
            // mobs a varience so a level 40 mob will be willing to attack
            // a level 48 PC.
            if ((double)(tmp_ch->GetMaxLevel()) > ((double)GetMaxLevel() * 1.5)) {
	      if(!::number(0,6) || !mobpulse) {
		act("$n considers killing $N, but thinks better of it.",TRUE,this,NULL,tmp_ch,TO_NOTVICT,NULL);
		act("$n considers killing you, but thinks better of it.",TRUE,this,NULL,tmp_ch,TO_VICT,NULL);
		act("You consider killing $N, but thinks better of it.",TRUE,this,NULL,tmp_ch,TO_CHAR,NULL);
	      }
	      
	      return FALSE;
	    }
	  }
	  if (hasWandered && ::number(0,29)) {
	    // don't attack people less than 2/3rds my level if i'm somewhere i'm not 'expected' to be
	    if ((double)(tmp_ch->GetMaxLevel()) < ((double)GetMaxLevel() * 0.67)) {
	      if(!::number(0,4) || !mobpulse) {
		act("$n considers killing $N, but decides not to waste $s time.",TRUE,this,NULL,tmp_ch,TO_NOTVICT,NULL);
		act("$n considers killing you, but decides not to waste $s time.",TRUE,this,NULL,tmp_ch,TO_VICT,NULL);
		act("You consider killing $N, but decides not to waste your time.",TRUE,this,NULL,tmp_ch,TO_CHAR,NULL);
	      }
	      return FALSE;
	    }
	  }
	  if (specials.act & ACT_WIMPY){
	    if (!tmp_ch->awake()) {
              stats.aggro_successes++;
              rc = takeFirstHit(*tmp_ch);
              if (IS_SET_DELETE(rc, DELETE_VICT)) {
                delete tmp_ch;
                tmp_ch = NULL;
                return DELETE_VICT;
              } else if (IS_SET_DELETE(rc, DELETE_THIS)) {
                return DELETE_THIS;
              }
              return TRUE;
            }
            continue;
          } else {
            stats.aggro_successes++;
            act("$n snarls angrily and attacks!",TRUE,this,0,0,TO_ROOM);
            rc = takeFirstHit(*tmp_ch);
            if (IS_SET_DELETE(rc, DELETE_VICT)) {
              delete tmp_ch;
              tmp_ch = NULL;
              return DELETE_VICT;
            } else if (IS_SET_DELETE(rc, DELETE_THIS)) {
              return DELETE_THIS;
            }
            return TRUE;
          }
        }
      }
    }
  }
  return FALSE;
}

/* I redid this function to help NPC's if at all possible. */
/* It was way too easy to 'cheat' a mob into attacking who */
/* you were attacking and get easy exp/equipment - Russ    */
// may return DELETE_THIS
int TMonster::assistFriend()
{
  TBeing *tbeTarg = NULL, *tmp_ch = NULL;
  TBeing *final = NULL;
  int rc;

  if (checkPeaceful(""))
    return FALSE;

  if (UtilMobProc(this))
    return FALSE;

  if (GuildProcs(spec))
    return FALSE;

  if (!roomp) {
    thing_to_room(this, Room::VOID);
    return FALSE;
  }
  TThing *t;
  for(StuffIter it=roomp->stuff.begin();it!=roomp->stuff.end();){
    t=*(it++);
    if (t == this)
      continue;
    tmp_ch = dynamic_cast<TBeing *>(t);
    if (!tmp_ch)
      continue;
    if (!tmp_ch->fight() || !canSee(tmp_ch))
      continue;

    if(isFriend(*tmp_ch)){
      // don't assist if he's flipping out
      if (!tmp_ch->isPc() && (dynamic_cast<TMonster *>(tmp_ch)->anger() > 40))
        continue;
      
      if (isPolice() && !fight()) {
        if (!tmp_ch->fight()->isAnimal()) {
          if (tmp_ch->fight()->isPolice()) {
            doSay("Stop!  Murderer!");
            final = tmp_ch;
            break;
          } else if (tmp_ch->isPolice()) {
            doSay("Stop!  Murderer!");
            final = tmp_ch->fight();
            break;
          } else {
            doSay("Break this up at once!");
            doSay("There is no fighting within the city limits!");
            act("$n breaks up the fight.", FALSE, this, 0, 0, TO_ROOM);

            TThing *toto=NULL;
            // stop all fights
            for(StuffIter it=roomp->stuff.begin();it!=roomp->stuff.end() && (toto=*it);++it) {
              TBeing *tbto = dynamic_cast<TBeing *>(toto);
              if (!tbto)
                continue;
              if (!tbto->fight())
                continue;
              TMonster *tbtm = dynamic_cast<TMonster *>(tbto);
              if (tbtm && !tbtm->isPc()) {
                if (tbtm->Hates(tbtm->fight(), NULL))
                  tbtm->remHated(tbtm->fight(), NULL);
                tbtm->DA(10);
              }
              tbto->stopFighting();
            }
            return TRUE;
          }
        }
      }
      
      if ((tbeTarg = tmp_ch->findAnAttacker())) {
        if (tbeTarg == master || inGroup(*tbeTarg) || !canSee(tbeTarg))
          continue;

        if (!tbeTarg->isPc() && ::number(1, 100) < min(99, max(GetMaxLevel(), tbeTarg->GetMaxLevel()) - 1)) {
          // added to reduce chance of brawling as mob level increases
          // % chance of avoiding brawl is equal to brawler or brawlee level (whichever is higher)
          continue;
        }
        
        if (tbeTarg->isPc()) {
          final = tbeTarg;
          break;
        }
        if (!final)
          final = tbeTarg;
        else if (!final->isPc() && tbeTarg->isPc())
          final = tbeTarg;
      }
    }
  }
  if (final) {
    act("$n attacks $N.", FALSE, this, 0, final, TO_NOTVICT);
    act("$n attacks you!", FALSE, this, 0, final, TO_VICT, ANSI_ORANGE);
    vlogf(LOG_MOB_AI, format("Brawl! %s (%d) attacks %s (%d) in %d") % this->name % this->GetMaxLevel() % final->name % final->GetMaxLevel() % (roomp ? roomp->in_room : 0));
    rc = hit(final);
    if (IS_SET_DELETE(rc, DELETE_VICT)) {
      delete final;
      final = NULL;
    }
    if (IS_SET_DELETE(rc, DELETE_THIS))
      return DELETE_THIS;

    return TRUE;
  }
  return FALSE;
}

// might return DELETE_THIS
int TMonster::findABetterWeapon()
{
  TThing *t=NULL;
  TObj *o = NULL, *best = NULL;
  int rc =0;
  
  if (!hasHands())
    return FALSE;

  if (UtilMobProc(this))
    return FALSE;

  for(StuffIter it=roomp->stuff.begin();it!=roomp->stuff.end() && (t=*it);++it) {
    if (!(dynamic_cast<TBaseWeapon *>(t)))
      continue;
    if (!(o = dynamic_cast<TObj *>(t)))
      continue;
    if (dynamic_cast<TBow *>(o) || dynamic_cast<TArrow *>(o))
      continue;
    if (!canSee(o))
      continue;
    if (o->action_description)
      continue;
    if (!canUseEquipment(o, SILENT_YES))
      continue;
    if ((!best || (o->baseDamage() >= best->baseDamage()))) 
      best = o;
  }
  for(StuffIter it=stuff.begin();it!=stuff.end() && (t=*it);++it) {
    if (!(o = dynamic_cast<TObj *>(t)))
      continue;
    if (dynamic_cast<TBow *>(o) || dynamic_cast<TArrow *>(o))
      continue;
    if (!canSee(o))
      continue;
    if (!canUseEquipment(o, SILENT_YES) || o->isMonogrammed())
      continue;
    if (!best || (o->baseDamage() >= best->baseDamage()))
      best = o;
  }
  TGenWeapon *tgw = dynamic_cast<TGenWeapon *>(heldInPrimHand());
  if (tgw && canSee(tgw)) {
    if (!best || (tgw->baseDamage() >= best->baseDamage()))
      return FALSE;
  }

#if 0
  // getWeaponDamage for mobs will add wielded-weapon damage to its
  // bare hand damage, so no need to remove "bad" weapons

  if (!best || (baseDamage() > best->baseDamage())) {
    TObj *tobj = dynamic_cast<TObj *>(heldInPrimHand());
    if (tobj && canSee(tobj)) {
      if (!dynamic_cast<TBow *>(tobj)){
        doSay("What a piece of junk.");
        rc = doRemove("", tobj);
        if (IS_SET_DELETE(rc, DELETE_ITEM)) {
          delete tobj;
          tobj = NULL;
        }
        if (IS_SET_DELETE(rc, DELETE_THIS))
          return DELETE_THIS;

        // beyond here is just for logging
        if (master)   // skip charms
          return FALSE;
        // skip training gear
        if (tobj && tobj->objVnum() == Obj::WEAPON_T_DAGGER)
          return FALSE;
        vlogf(LOG_LOW,format("%s (%d) removed %s (%d : base=%.2f) as hands are better.") % 
                  getName() % mobVnum() % tobj->getName() % tobj->objVnum() % tobj->baseDamage());

        return TRUE;
      }
    }
    return FALSE;
  }
#else
  if (!best)
    return FALSE;
#endif

  // pick it up if we don't possess it
  if ((best->equippedBy != this) && (best->parent != this)) { 
    vlogf(LOG_MOB_AI, format("Mob weapon grab: %s picking up %s in room (%d)") % this->name % best->name % (roomp ? roomp->in_room : 0));
    rc = doGet(fname(best->name).c_str());
    if (IS_SET_DELETE(rc, DELETE_THIS))
      return DELETE_THIS;
  }

  // remove "bad" stuff, and wield the best stuff
  if (best->parent == this) {
    if ((t = heldInPrimHand()) && canSee(t) && (mobVnum()!=18576 && mobVnum()!=18577)) {
      //brightmoon archers tend to do this to their bows, which is bad.
      doSay("What a hunk of junk.");
      rc = doRemove("", t);
      if (IS_SET_DELETE(rc, DELETE_ITEM)) {
        delete t;
        t = NULL;
      }
      if (IS_SET_DELETE(rc, DELETE_THIS))
        return DELETE_THIS;

      return TRUE;
    }
    doWield(fname(best->name).c_str());
    return TRUE;
  }
  return FALSE;
}

bool TMonster::isFriend(TBeing &myfriend)
{
  if (inGroup(myfriend))
    return TRUE;

  TMonster *fm = dynamic_cast<TMonster *>(&myfriend);
  if (fm && mob_index[getMobIndex()].virt == mob_index[fm->getMobIndex()].virt)
    return TRUE;

  // only if they are no more than 5 levels less than me
  // or we're both over level 50
  if (!myfriend.isPc() &&
      isSameRace(&myfriend) && isSameFaction(&myfriend) &&
      (((myfriend.GetMaxLevel()+5) > GetMaxLevel()) ||
       (GetMaxLevel() > 50 && myfriend.GetMaxLevel() > 50)))
    return TRUE;

  return FALSE;
}

int TMonster::defendOther(TBeing &targ)
{
  spellNumT spell = TYPE_UNDEFINED;
  int found = FALSE;
  int rc = 0;

  // this is called by notFightingMove, so we have already made checks for
  // standing, awake, !fight(), etc.  Don't bother to repeat them

  if (!awake() || isPc() || cantHit > 0 || !canSpeak() || eitherArmHurt() ||
      checkSoundproof() || nomagic("No-magic room prevents spell cast."))
    return FALSE;

  // don't cast twice
  if (spelltask)
    return FALSE;

  if (hasClass(CLASS_SHAMAN)) {
    if (!found) {
      spell = SPELL_SHIELD_OF_MISTS;
      if (!targ.affectedBySpell(spell) && 
           doesKnowSkill(spell) && (getSkillValue(spell) > 33)) {
        act("$n sings the words, 'Eluagga Xypomine!'",
                 TRUE, this, 0, 0, TO_ROOM);
        found = TRUE;
      }
    }
    if (found) {
      if (IS_SET(discArray[spell]->targets, TAR_CHAR_ROOM)) {
        rc = doDiscipline(spell, targ.name);
      } else {
vlogf(LOG_BUG, format("Shaman Mob casting (2) spell %d on other with possibly bad target flags for spell") %  spell);
        rc = doDiscipline(spell, "");
      }
      if (IS_SET_DELETE(rc, DELETE_THIS))
        return DELETE_THIS;
      return TRUE;
    }
  }

  if (hasClass(CLASS_MAGE)) {
    if (!found) {
      spell = SPELL_SORCERERS_GLOBE;
      if (!targ.affectedBySpell(spell) && 
           doesKnowSkill(spell) && (getSkillValue(spell) > 33)) {
        act("$n utters the words, 'Protection Correction!'",
                 TRUE, this, 0, 0, TO_ROOM);
        found = TRUE;
      }
    }

    if (found) {
      if (IS_SET(discArray[spell]->targets, TAR_CHAR_ROOM)) {
        rc = doDiscipline(spell, targ.name);
      } else {
vlogf(LOG_BUG, format("Mob casting (2) spell %d on other with possibly bad target flags for spell") %  spell);
        rc = doDiscipline(spell, "");
      }
      if (IS_SET_DELETE(rc, DELETE_THIS))
        return DELETE_THIS;
      return TRUE;
    }
  } else if (hasClass(CLASS_CLERIC)) {
    if (!found) {
      spell = getSkillNum(SPELL_BLESS);
      if (!targ.affectedBySpell(SPELL_BLESS) &&
           doesKnowSkill(spell) && (getSkillValue(spell) > 33)) {
        act("$n utters, \"What a Rush!\"", 
                 TRUE, this, 0, 0, TO_ROOM);
        found = TRUE;
      }
    }
    if (!found) {
      spell = getSkillNum(SPELL_ARMOR);
      if ( !targ.affectedBySpell(SPELL_ARMOR) &&
           doesKnowSkill(spell) && (getSkillValue(spell) > 33)) {
        act("$n utters the words, 'Abrazak'",
                 TRUE, this, 0, 0, TO_ROOM);
        found = TRUE;
      }
    }
#if 0
    // sanc is kind of buff to be casting on others at random
    if (!found) {
      spell = SPELL_SANCTUARY;
      if ( !targ.affectedBySpell(spell) && !targ.isAffected(AFF_SANCTUARY) &&
           doesKnowSkill(spell) && (getSkillValue(spell) > 33)) {
        act("$n utters, \"White aura, surround me!\"", 
                 TRUE, this, 0, 0, TO_ROOM);
        found = TRUE;
      }
    }
#endif

    if(!found) {
      spell = get_cleric_heal_spell(*this, targ);
      if(spell != TYPE_UNDEFINED)
	found=TRUE;
    }


    if (found) {
      if (IS_SET(discArray[spell]->targets, TAR_CHAR_ROOM)) {
        rc = doDiscipline(spell, targ.name);
      } else {
vlogf(LOG_BUG, format("Mob invoking (2) prayer %d on other with possibly bad target flags for spell") %  spell);
        rc = doDiscipline(spell, "");
      }
      if (IS_SET_DELETE(rc, DELETE_THIS))
        return DELETE_THIS;
      return TRUE;
    }
  } else if (hasClass(CLASS_DEIKHAN)) {
    if (!found) {
      spell = getSkillNum(SPELL_BLESS);
      if (!targ.affectedBySpell(SPELL_BLESS) &&
           doesKnowSkill(spell) && (getSkillValue(spell) > 33)) {
        act("$n utters, \"What a Rush!\"", 
                 TRUE, this, 0, 0, TO_ROOM);
        found = TRUE;
      }
    }
    if (!found) {
      spell = getSkillNum(SPELL_ARMOR);
      if ( !targ.affectedBySpell(SPELL_ARMOR) &&
           doesKnowSkill(spell) && (getSkillValue(spell) > 33)) {
        act("$n utters the words, 'Abrazak'",
                 TRUE, this, 0, 0, TO_ROOM);
        found = TRUE;
      }
    }

    if (found) {
      if (IS_SET(discArray[spell]->targets, TAR_CHAR_ROOM)) {
        rc = doDiscipline(spell, targ.name);
      } else {
vlogf(LOG_BUG, format("Mob invoking (3) prayer %d on other with possibly bad target flags for spell") %  spell);
        rc = doDiscipline(spell, "");
      }
      if (IS_SET_DELETE(rc, DELETE_THIS))
        return DELETE_THIS;
      return TRUE;
    }
  } else if (hasClass(CLASS_RANGER)) {
    if (!found) {
      spell = SKILL_BARKSKIN;
      if (!targ.affectedBySpell(spell) &&
           doesKnowSkill(spell) && (getSkillValue(spell) > 33)) {
        act("$n utters, \"I'm a lumberjack, and I'm OK...\"", 
                 TRUE, this, 0, 0, TO_ROOM);
        found = TRUE;
      }
    }

    if (found) {
      if (IS_SET(discArray[spell]->targets, TAR_CHAR_ROOM)) {
        if (spell == SKILL_BARKSKIN)
          rc = doBarkskin(targ.name);
        else
          rc = doDiscipline(spell, targ.name);
      } else {
vlogf(LOG_BUG, format("Mob casting (3) spell %d on other with possibly bad target flags for spell") %  spell);
        if (spell == SKILL_BARKSKIN)
          rc = doBarkskin(targ.name);
        else
          rc = doDiscipline(spell, "");
      }
      if (IS_SET_DELETE(rc, DELETE_THIS))
        return DELETE_THIS;
      return TRUE;
    }
  }
  
  return FALSE;
}

int TMonster::defendSelf(int)
{
  spellNumT spell = TYPE_UNDEFINED;
  bool found = FALSE;
  int rc = 0;
  int usecomp=1;
  TBeing *k, *t=NULL;
  followData *f;

  // this is called by notFightingMove, so we have already made checks for
  // standing, awake, !fight(), etc.  Don't bother to repeat them
  if (!awake() || isPc() || cantHit > 0)
    return FALSE;

  if (!canSpeak())
    return FALSE;

  if (checkSoundproof() || nomagic("No-magic room prevents spell cast."))
    return FALSE;

  // don't cast twice
  if (spelltask)
    return FALSE;

  // the purpose of this function is to have mob improve their defenses
 
  if (hasClass(CLASS_SHAMAN)) {
    if (!found) {
      spell = SPELL_SHIELD_OF_MISTS;
      if (!affectedBySpell(spell) && 
           doesKnowSkill(spell) && (getSkillValue(spell) > 33)) {
        act("$n sings the words, 'Eluagga Xypomine!'",
                 TRUE, this, 0, 0, TO_ROOM);
        found = TRUE;
      }
    }
    if (!found) {
      spell = SPELL_CELERITE;
      if (!affectedBySpell(spell) && 
           doesKnowSkill(spell) && (getSkillValue(spell) > 33)) {
        act("$n sings the words, 'Kick up your heels! Stick out your toosh!'",
                 TRUE, this, 0, 0, TO_ROOM);
        found = TRUE;
      }
    }
    if (!found) {
      spell = SPELL_THORNFLESH;
      if (!affectedBySpell(spell) && 
           doesKnowSkill(spell) && (getSkillValue(spell) > 33)) {
        act("$n utters the words, 'Big Prick!'",
                 TRUE, this, 0, 0, TO_ROOM);
        found = TRUE;
      }
    }
    if (!found) {
      spell = SPELL_SHADOW_WALK;
      if (!affectedBySpell(spell) && 
           doesKnowSkill(spell) && (getSkillValue(spell) > 33)) {
        act("$n utters the invokation, 'You Sneeggy Bastidge!'",
                 TRUE, this, 0, 0, TO_ROOM);
        found = TRUE;
      }
    }
    if (!found) {
      spell = SPELL_CLARITY;
      if (!affectedBySpell(spell) && 
           doesKnowSkill(spell) && (getSkillValue(spell) > 33)) {
        act("$n utters the invokation, 'I can see clearly now the rain is gone!'",
                 TRUE, this, 0, 0, TO_ROOM);
        found = TRUE;
      }
    }
#ifdef NPCTHRALLUSE
    //////// enthralls
    if (!found) {
      int foundmental=0;
      followData *k, *k2;
      TBeing *ch=NULL;

      for (k = followers;k; k = k2) {
	k2 = k->next;
	if (!(ch = k->follower) || !dynamic_cast<TBeing *>(ch)) {
	  vlogf(LOG_BUG, format("Non-TBeing in followers of %s") %  getName());
	  break;
	}
	if (ch->isAffected(AFF_CHARM) && sameRoom(*ch)) {
	  foundmental=1;
	  break;
	}
      }
      if(!foundmental) {
	int spellnum = ::number(1,8);
	switch(spellnum) {
	  case 1:
	    spell = SPELL_ENTHRALL_SPECTRE;
	    if (doesKnowSkill(spell) && (getSkillValue(spell) > 35)) {
	      act("$n sings the rada, 'Xebec Kamala!'",
		  TRUE, this, 0, 0, TO_ROOM);
	      found = TRUE;
	    }
	    break;
	  case 2:
	    spell = SPELL_ENTHRALL_GHAST;
	    if (doesKnowSkill(spell) && (getSkillValue(spell) > 40)) {
	      act("$n sings the rada, 'Xebec Romula!'",
		  TRUE, this, 0, 0, TO_ROOM);
	      found = TRUE;
	    }
	    break;
	  case 3:
	    spell = SPELL_ENTHRALL_GHOUL;
	    if (doesKnowSkill(spell) && (getSkillValue(spell) > 45)) {
	      act("$n sings the rada, 'Xebec Pumula!'",
		  TRUE, this, 0, 0, TO_ROOM);
	      found = TRUE;
	    }
	    break;
	  case 4:
	    spell = SPELL_ENTHRALL_DEMON;
	    if (doesKnowSkill(spell) && (getSkillValue(spell) > 50)) {
	      act("$n sings the rada, 'Xebec Tamala!'",
		  TRUE, this, 0, 0, TO_ROOM);
	      found = TRUE;
	    }
	    break;
	  case 5:
	    spell = SPELL_CREATE_WOOD_GOLEM;
	    if (doesKnowSkill(spell) && (getSkillValue(spell) > 55)) {
	      act("$n sings the rada, 'Xebec Oakala!'",
		  TRUE, this, 0, 0, TO_ROOM);
	      found = TRUE;
	    }
	    break;
	  case 6:
	    spell = SPELL_CREATE_ROCK_GOLEM;
	    if (doesKnowSkill(spell) && (getSkillValue(spell) > 60)) {
	      act("$n sings the rada, 'Xebec Rokala!'",
		  TRUE, this, 0, 0, TO_ROOM);
	      found = TRUE;
	    }
	    break;
	  case 7:
	    spell = SPELL_CREATE_IRON_GOLEM;
	    if (doesKnowSkill(spell) && (getSkillValue(spell) > 65)) {
	      act("$n sings the rada, 'Xebec Metala!'",
		  TRUE, this, 0, 0, TO_ROOM);
	      found = TRUE;
	    }
	    break;
	  case 8:
	  default:
	    spell = SPELL_CREATE_DIAMOND_GOLEM;
	    if (doesKnowSkill(spell) && (getSkillValue(spell) > 70)) {
	      act("$n sings the rada, 'Xebec Diala!'",
		  TRUE, this, 0, 0, TO_ROOM);
	      found = TRUE;
	    }
	    break;
	}
	usecomp=0;  // too easy to farm comps with this one
      }
    }
    //////// end enthralls
#endif
    if (!found) {
      spell = SPELL_CHEVAL;
      if (!affectedBySpell(spell) && 
           doesKnowSkill(spell) && (getSkillValue(spell) > 33)) {
        act("$n utters the invokation, 'Legba de' Cheval!'",
                 TRUE, this, 0, 0, TO_ROOM);
        found = TRUE;
      }
    }
    if (found) {
      if(usecomp)
	dynamicComponentLoader(spell, 10);
      if (IS_SET(discArray[spell]->targets, TAR_SELF_ONLY | TAR_IGNORE | TAR_FIGHT_SELF)) {
        rc = doDiscipline(spell, "");
      } else {
	vlogf(LOG_BUG, format("Shaman Mob casting (4) spell %d on self with possibly bad target flags for spell") %  spell);
        rc = doDiscipline(spell, name);
      }
      if (IS_SET_DELETE(rc, DELETE_THIS))
        return DELETE_THIS;
      return TRUE;
    }
  }
  if (hasClass(CLASS_MAGE)) {
    if (!found) {
      spell = SPELL_SORCERERS_GLOBE;
      if (!affectedBySpell(spell) && 
           doesKnowSkill(spell) && (getSkillValue(spell) > 33)) {
        act("$n utters the words, 'Protection Correction!'",
                 TRUE, this, 0, 0, TO_ROOM);
        found = TRUE;
      }
    }
    if (!found) {
      spell = ::number(0,1) ? SPELL_STONE_SKIN : SPELL_FLAMING_FLESH;
      if (!affectedBySpell(SPELL_STONE_SKIN) &&
          !affectedBySpell(SPELL_FLAMING_FLESH) &&
           doesKnowSkill(spell) && (getSkillValue(spell) > 33)) {
        if (spell == SPELL_STONE_SKIN) {
          act("$n utters the words, 'I Rock!'",
                 TRUE, this, 0, 0, TO_ROOM);
        } else {
          act("$n utters the words, 'HOT DAMN!  I'm on fire!'",
                 TRUE, this, 0, 0, TO_ROOM);
        }
        found = TRUE;
      }
    }
    if (!found) {
      spell = SPELL_PLASMA_MIRROR;
      if (!affectedBySpell(spell) && 
           doesKnowSkill(spell) && (getSkillValue(spell) > 33)) {
        act("$n utters the words, 'Mirror Mirror!'",
                 TRUE, this, 0, 0, TO_ROOM);
        found = TRUE;
      }
    }
    if (!found && specials.hunting && Hates(specials.hunting, NULL)) {
      // we're hunting down some bastard!  Let's be real nasty about it
      if (!found) {
        spell = SPELL_DISPEL_MAGIC;
        if (affectedBySpell(SPELL_CALM) &&
	    doesKnowSkill(spell) &&
            (getSkillValue(spell) > 66)) {
          act("$n utters the words, 'Back to the basics!'",
                   TRUE, this, 0, 0, TO_ROOM);
          found = TRUE;
        }
      }
      if (!found) {
        spell = SPELL_FLY;
        if (!isAffected(AFF_FLYING) &&
	     doesKnowSkill(spell) && (getSkillValue(spell) > 60)) {
          act("$n utters the words, 'Whoa, I feel light headed!'",
                   TRUE, this, 0, 0, TO_ROOM);
          found = TRUE;
        }
      }
      if (!found) {
        spell = SPELL_HASTE;
        if (!affectedBySpell(spell) &&
	     doesKnowSkill(spell) && (getSkillValue(spell) > 60)) {
          act("$n utters the words, 'Time to get moving!'",
                   TRUE, this, 0, 0, TO_ROOM);
          found = TRUE;
        }
      }
      if (!found) {
        spell = SPELL_DETECT_INVISIBLE;
        if (!isAffected(AFF_DETECT_INVISIBLE) &&
             doesKnowSkill(spell) && (getSkillValue(spell) > 60)) {
          act("$n utters the words, 'I can smell 'em.  Now I can see 'em!'",
                   TRUE, this, 0, 0, TO_ROOM);
          found = TRUE;
        }
      }
      if (!found) {
        spell = SPELL_INVISIBILITY;
        if (!isAffected(AFF_INVISIBLE) &&
             doesKnowSkill(spell) && (getSkillValue(spell) > 33)) {
          act("$n utters the words, 'All my troubles will disappear!'",
                   TRUE, this, 0, 0, TO_ROOM);
          found = TRUE;
        }
      }
    }

#if 0
    if ((susp()-defsusp()) > 9){
      if (!found){
	int mentals[]={SPELL_CONJURE_AIR, SPELL_CONJURE_FIRE, 
		       SPELL_CONJURE_EARTH, SPELL_CONJURE_WATER, 
		       SPELL_ENTHRALL_SPECTRE, SPELL_ENTHRALL_GHAST,
		       SPELL_ENTHRALL_GHOUL, SPELL_ENTHRALL_DEMON, 
		       SPELL_CREATE_WOOD_GOLEM, SPELL_CREATE_ROCK_GOLEM,
		       SPELL_CREATE_IRON_GOLEM, SPELL_CREATE_DIAMOND_GOLEM};
	int count=12, i, foundmental=0;
	followData *k, *k2;
	TBeing *ch=NULL;

	// pick a conjure spell
	for(i=::number(0,11),count+=i;i<count;++i)
	  if (doesKnowSkill(mentals[i%4]) && 
	      (getSkillValue(mentals[i%4]) > 33))
	    spell=mentals[i%4];

	// we only want 1 'mental at a time
	for (k = followers;k; k = k2) {
	  k2 = k->next;
	  if (!(ch = k->follower) || !dynamic_cast<TBeing *>(ch)) {
	    vlogf(LOG_BUG, format("Non-TBeing in followers of %s") %  getName());
	    break;
	  }
	  if (ch->isCharm() && sameRoom(*ch)){
	    foundmental=1;
	    break;
	  }
	}
	if(!foundmental){
	  switch(spell){
	    case SPELL_CONJURE_AIR:
	      act("$n utters the words, 'Atmospheric Aid!'",
		  TRUE, this, 0, 0, TO_ROOM);	      
	      break;
	    case SPELL_CONJURE_FIRE:
	      act("$n utters the words, 'Fiery Friend!'",
		  TRUE, this, 0, 0, TO_ROOM);	      
	      break;
	    case SPELL_CONJURE_EARTH:
	      act("$n utters the words, 'Earthly Endorsement!'",
		  TRUE, this, 0, 0, TO_ROOM);	      
	      break;
	    case SPELL_CONJURE_WATER:
	      act("$n utters the words, 'Watery Waiter!'",
		  TRUE, this, 0, 0, TO_ROOM);	      
	      break;
	    case SPELL_ENTHRALL_SPECTRE:
	      act("$n sings the rada, 'Xebec Kamala!'",
		  TRUE, this, 0, 0, TO_ROOM);	      
	      break;
	    case SPELL_ENTHRALL_GHAST:
	      act("$n sings the rada, 'Xebec Romula!'",
		  TRUE, this, 0, 0, TO_ROOM);	      
	      break;
	    case SPELL_ENTHRALL_GHOUL:
	      act("$n sings the rada, 'Xebec Pumula!'",
		  TRUE, this, 0, 0, TO_ROOM);	      
	      break;
	    case SPELL_ENTHRALL_DEMON:
	      act("$n sings the rada, 'Xebec Tamala!'",
		  TRUE, this, 0, 0, TO_ROOM);	      
	      break;
	    case SPELL_CREATE_WOOD_GOLEM:
	      act("$n sings the rada, 'Xebec Oakala!'",
		  TRUE, this, 0, 0, TO_ROOM);	      
	      break;
	    case SPELL_CREATE_ROCK_GOLEM:
	      act("$n sings the rada, 'Xebec Rokala!'",
		  TRUE, this, 0, 0, TO_ROOM);	      
	      break;
	    case SPELL_CREATE_IRON_GOLEM:
	      act("$n sings the rada, 'Xebec Metala!'",
		  TRUE, this, 0, 0, TO_ROOM);	      
	      break;
	    case SPELL_CREATE_DIAMOND_GOLEM:
	      act("$n sings the rada, 'Xebec Diala!'",
		  TRUE, this, 0, 0, TO_ROOM);	      
	      break;
	  }
	  found = TRUE;
	  usecomp=0;  // too easy to farm comps with this one
	}
      }
    }
#endif

    if (found) {
      if(usecomp)
	dynamicComponentLoader(spell, 10);
      if (IS_SET(discArray[spell]->targets, TAR_SELF_ONLY | TAR_IGNORE | TAR_FIGHT_SELF)) {
        rc = doDiscipline(spell, "");
      } else {
vlogf(LOG_BUG, format("Mob casting (4) spell %d on self with possibly bad target flags for spell") %  spell);
        rc = doDiscipline(spell, name);
      }
      if (IS_SET_DELETE(rc, DELETE_THIS))
        return DELETE_THIS;
      return TRUE;
    }
  } else if (hasClass(CLASS_CLERIC)) {
    if (!found) {
      spell = getSkillNum(SPELL_BLESS);
      if (!affectedBySpell(SPELL_BLESS) &&
           doesKnowSkill(spell) && (getSkillValue(spell) > 33)) {
        act("$n utters, \"What a Rush!\"", 
                 TRUE, this, 0, 0, TO_ROOM);
        found = TRUE;
      }
    }
    if (!found) {
      spell = getSkillNum(SPELL_ARMOR);
      if ( !affectedBySpell(SPELL_ARMOR) &&
           doesKnowSkill(spell) && (getSkillValue(spell) > 33)) {
        act("$n utters the words, 'Abrazak'",
                 TRUE, this, 0, 0, TO_ROOM);
        found = TRUE;
      }
    }
    if (!found) {
      spell = SPELL_SANCTUARY;
      if ( !affectedBySpell(spell) && !isAffected(AFF_SANCTUARY) &&
           doesKnowSkill(spell) && (getSkillValue(spell) > 33)) {
        act("$n utters, \"White aura, surround me!\"", 
                 TRUE, this, 0, 0, TO_ROOM);
        found = TRUE;
      }
    }
    // Let's heal ourselves...
    if (!found) {
      spell = get_cleric_heal_spell(*this, *this);
      if (spell == TYPE_UNDEFINED){
	// we don't need healing so check the group
	if (!(k = master))
	  k = this;
	for (f = k->followers; f; f = f->next){
	  spell=get_cleric_heal_spell(*this, *(t=f->follower));
	  if (spell != TYPE_UNDEFINED)
	    break;
	}
      }
      if (spell != TYPE_UNDEFINED) {
	act("$n utters the words, 'I feel better now'",
	    TRUE, this, 0, 0, TO_ROOM);
	found = TRUE;
      }
    }

    if (found) {
      if (IS_SET(discArray[spell]->targets, TAR_SELF_ONLY | TAR_IGNORE | TAR_FIGHT_SELF)) {
        rc = doDiscipline(spell, "");
      } else {
vlogf(LOG_BUG, format("Mob invoking (4) prayer %d on self with possibly bad target flags for spell") %  spell);
        rc = doDiscipline(spell, name);
      }
      if (IS_SET_DELETE(rc, DELETE_THIS))
        return DELETE_THIS;
      return TRUE;
    }
  } else if (hasClass(CLASS_DEIKHAN)) {
    if (!found) {
      spell = getSkillNum(SPELL_BLESS);
      if (!affectedBySpell(SPELL_BLESS) &&
           doesKnowSkill(spell) && (getSkillValue(spell) > 33)) {
        act("$n utters, \"What a Rush!\"", 
                 TRUE, this, 0, 0, TO_ROOM);
        found = TRUE;
      }
    }
    if (!found) {
      spell = getSkillNum(SPELL_ARMOR);
      if ( !affectedBySpell(SPELL_ARMOR) &&
           doesKnowSkill(spell) && (getSkillValue(spell) > 33)) {
        act("$n utters the words, 'Abrazak'",
                 TRUE, this, 0, 0, TO_ROOM);
        found = TRUE;
      }
    }

    if (found) {
      if (IS_SET(discArray[spell]->targets, TAR_SELF_ONLY | TAR_IGNORE | TAR_FIGHT_SELF)) {
        rc = doDiscipline(spell, "");
      } else {
vlogf(LOG_BUG, format("Mob invoking (5) prayer %d on self with possibly bad target flags for spell") %  spell);
        rc = doDiscipline(spell, name);
      }
      if (IS_SET_DELETE(rc, DELETE_THIS))
        return DELETE_THIS;
      return TRUE;
    }
  } else if (hasClass(CLASS_RANGER)) {
    if (!found) {
      spell = SKILL_BARKSKIN;
      if (!affectedBySpell(spell) &&
           doesKnowSkill(spell) && (getSkillValue(spell) > 33)) {
        act("$n utters, \"I'm a lumberjack, and I'm OK...\"", 
                 TRUE, this, 0, 0, TO_ROOM);
        found = TRUE;
      }
    }

    if((susp()-defsusp()) > 5){
      if(!found){
	spell = SKILL_TRANSFORM_LIMB;
	if (doesKnowSkill(spell) && (getSkillValue(spell) > 33) &&
	    isTransformableLimb(WEAR_HAND_R, TRUE) &&
            roomp && !roomp->notRangerLandSector() &&
	    !equipment[WEAR_HAND_R] && !equipment[WEAR_HAND_L] &&
            !equipment[HOLD_RIGHT] && !equipment[HOLD_LEFT]) {
	  act("$n utters, 'Shred!'",
	      TRUE, this, 0, 0, TO_ROOM);
	  found = TRUE;	  
	}
      }
    }

    if (found) {
      dynamicComponentLoader(spell, 10);
      if (IS_SET(discArray[spell]->targets, TAR_SELF_ONLY | TAR_IGNORE | TAR_FIGHT_SELF | TAR_NAME)) {
        if (spell == SKILL_BARKSKIN)
          rc = doBarkskin(name);
        else if(spell == SKILL_TRANSFORM_LIMB)
	  rc = doTransform("hands");
	else
          rc = doDiscipline(spell, "");
      } else {
vlogf(LOG_BUG, format("Mob casting (5) spell %d on self with possibly bad target flags for spell") %  spell);
        if (spell == SKILL_BARKSKIN)
          rc = doBarkskin(name);
	else if(spell == SKILL_TRANSFORM_LIMB)
	  rc = doTransform("hands");
        else
          rc = doDiscipline(spell, name);
      }
      if (IS_SET_DELETE(rc, DELETE_THIS))
        return DELETE_THIS;
      return TRUE;
    }
  }
  
  return FALSE;
}

int TMonster::doHatefulStuff()
{
  TBeing *tmp_ch;
  int rc;

  if ((tmp_ch = findAHatee()) && canSee(tmp_ch)) {
    // look in room to see if anyone I hate is here
    if (tmp_ch->isPc() &&
         !(tmp_ch->isImmortal() &&
           tmp_ch->isPlayerAction(PLR_NOHASSLE))) {
      if (checkPeaceful("You ask your mortal enemy to step outside to settle matters.\n\r")) {
        if (!::number(0, 4)) {
          if (canSpeak())
            act("$n growls '$N, would you care to step outside where we can settle this?'", TRUE, this, 0, tmp_ch, TO_ROOM);
          else
            act("$n points and motions to the nearest exit.", TRUE, this, 0, tmp_ch, TO_ROOM);
          TThing *t;
          for(StuffIter it=roomp->stuff.begin();it!=roomp->stuff.end();){
            t=*(it++);
            TBeing *tbt = dynamic_cast<TBeing *>(t);
            if (!tbt || tbt == this || tbt == tmp_ch)
              continue;
            rc = tbt->checkSpec(this, CMD_MOB_VIOLENCE_PEACEFUL, "", tmp_ch);
            if (IS_SET_DELETE(rc, DELETE_ITEM)) {
              delete tmp_ch;
              tmp_ch = NULL;
            }
            if (IS_SET_DELETE(rc, DELETE_THIS)) {
              delete tbt;
              tbt = NULL;
            }
            if (IS_SET_DELETE(rc, DELETE_VICT))
              return DELETE_THIS;
            if (rc)
              return TRUE;
          }
          return TRUE;
        }
      } else {
        if (canSpeak()) {
          if (hates.sex == SEX_MALE && tmp_ch->getSex() == SEX_MALE)
            act("$n screams, 'I HATE MEN!!!!'", TRUE, this, 0, 0, TO_ROOM);
          else if (hates.race == RACE_DWARF && tmp_ch->getRace() == RACE_DWARF)
            act("$n screams, 'Die, dwarven scum!!!!'", TRUE, this, 0, 0, TO_ROOM);
          else if (hates.race == RACE_ELVEN && tmp_ch->getRace() == RACE_ELVEN)
            act("$n screams, 'Death to all elves!!!!'", TRUE, this, 0, 0, TO_ROOM);
          else if (!isDumbAnimal())
            act("$n screams 'I'm gonna kill you!'", TRUE, this, 0, 0, TO_ROOM);
          else
            aiGrowl(NULL);
        } else
          aiGrowl(tmp_ch);

        rc = takeFirstHit(*tmp_ch);
        if (IS_SET_DELETE(rc, DELETE_VICT)) {
          delete tmp_ch;
          tmp_ch = NULL;
        }
        if (IS_SET_DELETE(rc, DELETE_THIS)) {
          return DELETE_THIS;
        }
      }
      return TRUE;
    }
  } else if (!IS_SET(specials.act, ACT_HUNTING)) {
    // no hated in room, lets go a'hunting
    int percent = (int) (100 * getHit() / hitLimit());
    // we'll be at least 1/2 strength before this kicks in
    if (::number(50, 101) < percent && ::number(0,1)) {
      // we can only hunt one person at a time
      charList *i;
      for (i = hates.clist; i; i = i->next) {
        tmp_ch = get_char(i->name, EXACT_YES);
        if (tmp_ch) {
          setHunting(tmp_ch);
          return true;
        }
      }
    }
  }
  return FALSE;
}

void TMonster::quickieDefend()
{
  unsigned int iter;
  iter = 10;
  while (iter--) {
    int rc = defendSelf(0);
    if (IS_SET_DELETE(rc, DELETE_THIS))
      vlogf(LOG_BUG, "Error: uncaught DELETE (1) in quickieDefend");
    unsigned int rnds = 7;
    while (rnds--) {
      if (spelltask) {
        rc = cast_spell(this, CMD_TASK_CONTINUE, 0);
        if (IS_SET_DELETE(rc, DELETE_THIS | DELETE_VICT))
          vlogf(LOG_BUG, "Error: uncaught DELETE (2) in quickieDefend");
      }
    }
  }
}




