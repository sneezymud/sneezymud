//////////////////////////////////////////////////////////////////////////
//
// SneezyMUD - All rights reserved, SneezyMUD Coding Team
//
//      "movement.cc" - All functions and routines related to movement
//         or changing position
//
///////////////////////////////////////////////////////////////////////////

// NOTE ON MOVEMENT:

// rawMove() actually handles moving the tbeing from room to room.
// It will show messages to the char, but NOT to everyone else.
// At the end of rawMove, the being is actually in the new room.

// displayMove() handles echoing the movement to everyone else around.
// It keeps track of where being was and temporarily moves them back and
// forth in order to echo things properly.

#include "stdsneezy.h"
#include "combat.h"
#include "games.h"
#include "client.h"
#include "disc_looting.h"
#include "room.h"
#include "obj_base_corpse.h"
#include "obj_keyring.h"
#include "obj_key.h"
#include "obj_portal.h"
#include "pathfinder.h"

void TBeing::goThroughPortalMsg(const TPortal *o) const
{
  char buf[256];
  int type;

  type = o->getPortalType();
  if (type > MAX_PORTAL_TYPE) {
    vlogf(LOG_BUG, fmt("%s has illegal portal type of %d.") %  o->shortDescr % type);
    type = 0;
  }
  if (riding) {
    sprintf(buf, "You urge your %s to %s $p.", 
        fname(riding->name).c_str(), portal_self_enter_mess[type]);
    act(buf, TRUE, this, o, NULL, TO_CHAR);
    sprintf(buf, "$n and $s %s disappear as they %s $p.",
        fname(riding->name).c_str(), portal_other_enter_mess[type]);
    act(buf, TRUE, this, o, NULL, TO_ROOM);
  } else if (!rider) {
    sprintf(buf, "You %s $p.", portal_self_enter_mess[type]);
    act(buf, TRUE, this, o, NULL, TO_CHAR);
    sprintf(buf, "$n disappears as $e %s $p.", portal_other_enter_mess[type]);
    act(buf, TRUE, this, o, NULL, TO_ROOM);
  }
}

void TBeing::exitFromPortalMsg(const TPortal *o) const
{
  char buf[256];
  int type;
  TPortal *o2;

  if (!(o2 = o->findMatchingPortal())) {
    act("$n appears in the room suddenly.", TRUE, this, NULL, NULL, TO_ROOM);
    return;
  }
  type = o2->getPortalType();
  if (type > MAX_PORTAL_TYPE) {
    vlogf(LOG_BUG, fmt("%s has illegal portal type of %d.") %  o2->shortDescr % type);
    type = 0;
  }
  if (riding) {
    sprintf(buf, "$n and $s %s %s $p.",
          fname(riding->name).c_str(), portal_self_exit_mess[type]);
    act(buf, TRUE, this, o2, NULL, TO_ROOM);
  } else if (!rider) {
    sprintf(buf, "$n %s $p.", portal_self_exit_mess[type]);
    act(buf, TRUE, this, o2, NULL, TO_ROOM);
  }
}

void portal_flag_change(TPortal *o, unsigned int bit_to_change, const char *message, setRemT set)
{
  TPortal *o2;
  char buf[256], capbuf[256];

  if (set)
    o->addPortalFlag(bit_to_change);
  else
    o->remPortalFlag(bit_to_change);

  if (!(o2 = o->findMatchingPortal()))
    return;

  if (set)
    o2->addPortalFlag(bit_to_change);
  else
    o2->remPortalFlag(bit_to_change);

  strcpy(capbuf, o2->shortDescr);
  sprintf(buf, message, sstring(capbuf).cap().c_str());
  sendToRoom(buf, o2->in_room);
}

void TBeing::notLegalMove() const
{
  sendTo("Alas, you cannot go that way...\n\r");
}

bool TBeing::validMove(dirTypeT cmd)
{
  roomDirData *exitp;
  TRoom *rp;
  int iHeight;
  int pass;
  TBeing *tbt;
  TMonster *tmon;

  exitp = exitDir(cmd);

  affectedData *aff;
  for (aff = affected; aff; aff = aff->next) {
    if (aff->type == SPELL_BIND) {
      if (!isLucky(levelLuckModifier(aff->level))) {
        sendTo("You are entrapped in sticky webs!\n\r");
        sendTo("Your struggles only entrap you further!\n\r");
        addToWait(combatRound(3));
        if (!isPc())
          setMove(0);
        return FALSE;
      } else {
        addToWait(combatRound(1));
        addToMove(-50);
        sendTo("You briefly pull free from the sticky webbing!\n\r");
      }
    }
  }
  if (riding) {
    tbt = dynamic_cast<TBeing *>(riding);
    if (tbt && tbt->fight()) {
      sendTo("Your mount is fighting!\n\r");
      return FALSE;
    }
    if (tbt && tbt->getPosition() < POSITION_FIGHTING) {
      sendTo("Your mount must be standing!\n\r");
      return FALSE;
    }
    if (!sameRoom(*riding) && (riding->horseMaster() == this)) {
      vlogf(LOG_BUG, fmt("mount/rider in different rooms: (%s, %s) %d %d") %  
            getName() % riding->getName() % inRoom() % riding->inRoom());
      dismount(POSITION_STANDING);
    }
    if (tbt && !tbt->master) {
      vlogf(LOG_BUG, fmt("Bogus riding situation (no master for %s).  setting to %s") % 
          tbt->getName() % getName());
      tbt->master = this;
    }
  }
  if (!exit_ok(exitp, NULL)) {
    notLegalMove();
    return FALSE;
  } 
  if (IS_SET(exitp->condition, EX_CAVED_IN)) {
    if (isImmortal() || IS_SET(specials.act, ACT_GHOST)) {
      act( "$n's body splits into a cloud of atoms before your eyes!", 
          TRUE, this, 0, NULL, TO_ROOM, NULL, (isPlayerAction(PLR_STEALTH) ? MAX_MORT : 0));
      sendTo("You make yourself ethereal to pass through the cave in.\n\r");
      return TRUE;
    }
    sendTo("A cave in blocks the way.\n\r");
    return FALSE;
  } 
  if (IS_SET(exitp->condition, EX_CLOSED)) {
  // jesus
    if (isAffected(AFF_SHADOW_WALK) && !riding) {
      int perc = getSkillValue(SPELL_SHADOW_WALK);
      int diff = exitp->lock_difficulty;
      if (perc > diff) {
	act( "$n's transparent body passes through the barrier!",
	     TRUE, this, 0, NULL, TO_ROOM, NULL, (isPlayerAction(PLR_STEALTH) ? MAX_MORT : 0));
	sendTo("You walk directly through the barrier!\n\r");
	return TRUE;
      } else {
	sendTo("You attempt to walk through a solid barrier and fail.\n\r");
        notLegalMove();
        return FALSE;
      }
    }
    if (isImmortal() || IS_SET(specials.act, ACT_GHOST)) {
      act( "$n's body splits into a cloud of atoms before your eyes!",
          TRUE, this, 0, NULL, TO_ROOM, NULL, (isPlayerAction(PLR_STEALTH) ? MAX_MORT : 0));
      sendTo("You make yourself ethereal to pass through the barrier.\n\r");
      return TRUE;
    }
    if (exitp->keyword) {
      if (!IS_SET(exitp->condition, EX_SECRET)) {
        char doorbuf[64];
        strcpy(doorbuf, fname(exitp->keyword).c_str());
        sendTo(fmt("The %s %s %s.\n\r") % 
               doorbuf %
               (doorbuf[strlen(doorbuf) -1] == 's' ? "are" : "is") %
               exitp->closed());
        return FALSE;
      } else {
        notLegalMove();
        return FALSE;
      }
    } else {
      vlogf(LOG_LOW, fmt("Problematic door: rm %d dir %d (closed with no-name?)") % 
            inRoom() % cmd);
      notLegalMove();
      return FALSE;
    }
  } 
  if (IS_SET(exitp->condition, EX_WARDED)) {
    if (isImmortal() || IS_SET(specials.act, ACT_GHOST)) {
      act( "$n's body splits into a cloud of atoms before your eyes!",
          TRUE, this, 0, NULL, TO_ROOM, NULL,
          (isPlayerAction(PLR_STEALTH) ? MAX_MORT : 0));
      sendTo("You make yourself ethereal to pass through the ward.\n\r");
      return TRUE;
    }
    pass = checkPassWard(cmd);   // centralized messages
    if (pass) {
      return TRUE;
    } else {
      return FALSE;
    }
  }

  rp = real_roomp(exitp->to_room);
  if (rp->getMoblim() && !isImmortal() &&
      (MobCountInRoom(rp->getStuff()) >= rp->getMoblim())) {
    sendTo("Sorry, there is no room to get in there.\n\r");
    return FALSE;
  }
  if (!isPc() && rp->isRoomFlag(ROOM_NO_MOB)) {
    if (!master && !(specials.act & ACT_HUNTING) && !rider && 
        !riding && 
        (spec != SPEC_BOUNTY_HUNTER) &&
        (spec != SPEC_LAMPBOY) &&
        (spec != SPEC_JANITOR) &&
        (spec != SPEC_CARAVAN)
        ) {
      sendTo("Sorry, seems to be a no-mob room that-a-way.\n\r");
      return FALSE;
    }
  }

  if (rp->isRoomFlag(ROOM_PRIVATE)) {
    if (MobCountInRoom(rp->getStuff()) > 2) {
      sendTo("Sorry, that room is private.\n\r");
      return FALSE;
    }
  }
  tbt = dynamic_cast<TBeing *>(riding);
  tmon=dynamic_cast<TMonster *>(riding);
  if(!tmon ||
     tmon->mobVnum()!=MOB_ELEPHANT ||
     !hasQuestBit(TOG_MONK_GREEN_STARTED)){
    if (tbt && tbt->willBumpHeadDoor(exitp, &iHeight)) {
      sendTo("Your mount refuses to go that way.\n\r");
      return FALSE;
    }
    if (tbt &&
        (tbt->getRace() == RACE_FISH) &&
        !rp->isUnderwaterSector() &&
        !rp->isWaterSector()) {
      sendTo("Your mount refuses to go that way.\n\r");
      return FALSE;
    }
    if (rp->isRoomFlag(ROOM_DEATH)) {
      if (riding) {
        sendTo("Your mount refuses to go that way.\n\r");
        return FALSE;
      }
    }
  }
  if (rp->isRoomFlag(ROOM_PEACEFUL)) {
    if (isCombatMode(ATTACK_BERSERK)) {
      act("You can't go into that place of tranquility while berserking.",TRUE,this,0,0,TO_CHAR);
      return FALSE;
    }
  }
  return TRUE;
}

int TBeing::checkPassWard(dirTypeT cmd) const
{
  int rp = 0;
  TObj *tmp = NULL;
  char buf[512];

  rp = in_room;

   if (!rp || !(in_room == rp)) {
     vlogf(LOG_BUG,fmt("Bad room in checkPassWard: %s in %d") %  getName() % rp);
     sendTo("Please bug what you just tried to do.\n\r");
     return FALSE;
   }

   switch (rp) {
     case (ROOM_WARD_1):
     case (ROOM_WARD_2):
       tmp = dynamic_cast<TObj *>(equipment[WEAR_NECK]);
       if (tmp) {
         if (tmp->objVnum() == OBJ_TALISMAN) {
           act("There is a disturbance in the air around you.",
               TRUE, this, 0, 0, TO_CHAR);
           act("There is a disturbance in the air around $n.",
               TRUE, this, 0, 0, TO_ROOM);
           act("$n's talisman flares as it encounters the disturbance.",
               TRUE, this, 0, 0, TO_ROOM);
           act("Your talisman flares as it encounters the disturbance.",
               TRUE, this, 0, 0, TO_CHAR);
           act("Your talisman protects you from the magic and allows you to move forward.", TRUE, this, 0, 0, TO_CHAR);
           return TRUE;
         } else {
           act("There is a disturbance in the air around you.", 
               TRUE, this, 0, 0, TO_CHAR);
           act("There is a disturbance in the air around $n.", 
               TRUE, this, 0, 0, TO_ROOM);
           sendTo("You are prevented from moving forward by a magical force.\n\r");
           sprintf(buf,"$n is stopped by a magical force as $e tries to go %s",
                   dirs[cmd]);
           act(buf, TRUE, this, 0, 0, TO_ROOM);
           return FALSE;
         }
       } else {
           act("There is a disturbance in the air around you.", 
               TRUE, this, 0, 0, TO_CHAR);
           act("There is a disturbance in the air around $n.", 
               TRUE, this, 0, 0, TO_ROOM);
           sendTo("You are prevented from moving forward by a magical force.\n\r");
           sprintf(buf,"$n is stopped by a magical force as $e tries to go %s",
                   dirs[cmd]);
           act(buf, TRUE, this, 0, 0, TO_ROOM);
           return FALSE;

       }
       break;
     default:
       sendTo("**SMACK**  You seem to have slammed into a magical ward.\n\r");
       sprintf(buf,"$n slams into a magical ward as $e tries to go %s",
          dirs[cmd]);
       act(buf, TRUE, this, 0, 0, TO_ROOM);
       return FALSE;
   }
   return FALSE;
}

void TBeing::putOutLightsInWater()
{
  int pos;
  TThing *t;

  for (pos = HOLD_RIGHT; pos <= HOLD_LEFT; pos++) {
    if (!(t = equipment[pos]))
      continue;
    t->extinguishWater(this);
  }
}

// returns DELETE_THIS
// A note on mounts:
// we will never be a horse in this function.
// if the horse itself moved, doMove converted this to the rider moving
// This function moves a rider's horse with them, and because of this
// moveGroup skips the horse (as follower) when moving critters
int TBeing::rawMove(dirTypeT dir)
{
  int need_movement = 0, new_r = 0, rc = 0;
  bool has_boat = FALSE;
  TRoom *from_here = NULL, *to_here = NULL;
  int iHeight = 0;
  char tmp[256];
  TBeing *tbt;

  if (!validMove(dir))
    return FALSE;

  if (isAffected(AFF_CHARM) && master && sameRoom(*master)) {
    if (!::number(0, 5))
      act("$n bursts into tears.", TRUE, this, 0, 0, TO_ROOM);
    act("You burst into tears at the thought of leaving $N.", FALSE, this, 0, master, TO_CHAR);
    return FALSE;
  }
  if (affectedBySpell(SKILL_ENCAMP)) {
    act("You break camp.", FALSE, this, 0, 0, TO_CHAR);
    act("$n breaks camp.", FALSE, this, 0, 0, TO_ROOM);
    affectFrom(SKILL_ENCAMP);
  }
  from_here = roomp;
  to_here = real_roomp(from_here->dir_option[dir]->to_room);
  new_r = from_here->dir_option[dir]->to_room;

  // See if char is carrying a boat */
  has_boat = hasBoat();

  if (!(roomp->isUnderwaterSector() || roomp->isWaterSector())) {
    if (isLimbFlags(WEAR_LEGS_R, PART_TRANSFORMED)) {
      sendTo("It's hard to walk without legs.\n\rMaybe crawling would help.\n\r");
      return FALSE;
    }
  }
  if(affectedBySpell(AFFECT_PLAYERKILL) && 
     to_here && to_here->isRoomFlag(ROOM_PEACEFUL) &&
     !isImmortal()){
    sendTo("Player killers can't enter peaceful rooms.\n\r");
    return FALSE;
  }
  if (willBumpHeadDoor(from_here->dir_option[dir], &iHeight) &&
        (getPosHeight() *4/5 > iHeight) &&
      !hasQuestBit(TOG_MONK_GREEN_FALLING)) {
    // make them crawl
    // the above height check permits them to stoop to get in
    sendTo("It's a pretty low ceiling.  Maybe crawling would help.\n\r");
    return FALSE;
  }
  if (!to_here) {
    --(*this);
    thing_to_room(this, ROOM_VOID);
    sendTo(fmt("Uh-oh.  The %s melts beneath you as you fall into the swirling chaos.\n\r") % roomp->describeGround());
    doLook("room", CMD_LOOK);
    return TRUE;
  }

  sprintf(tmp, "%i", dir);
  rc = from_here->checkSpec(this, CMD_ROOM_ATTEMPTED_EXIT, tmp, from_here);
  if (IS_SET_DELETE(rc, DELETE_THIS))
    return DELETE_THIS;
  if(rc==TRUE) // not allowed to move
    return FALSE;

  need_movement = (TerrainInfo[from_here->getSectorType()]->movement + 
                   TerrainInfo[to_here->getSectorType()]->movement) / 2;
  if ((TerrainInfo[from_here->getSectorType()]->movement%2) && 
         !(TerrainInfo[to_here->getSectorType()]->movement%2))
    need_movement++;

  if (affectedBySpell(SKILL_SNEAK) || checkForSkillAttempt(SKILL_SNEAK))
    need_movement += 2;


  if (getRace() == RACE_DWARF &&
      (roomp->isWaterSector() || roomp->isUnderwaterSector())) {
    need_movement += 20;
  }

  if (!riding && !isFlying()) {
    if (bothLegsHurt()) {
      if (!bothArmsHurt() && getPosition() == POSITION_CRAWLING) {
        sendTo(fmt("You drag yourself along the %s slowly and painfully.\n\r") % roomp->describeGround());
        need_movement += 20;
        if (eitherArmHurt()) {
          sendTo("And to make matters worse, you only have one arm to drag yourself with.\n\r");
          need_movement += 20;
        }
      } else if (getPosition() == POSITION_CRAWLING) {
        sendTo("All your appendages are busted, you can't even crawl.\n\r");
        return FALSE;
      } else {
        sendTo("You can't walk because your legs are inoperable!\n\r");
        sendTo("Perhaps you could crawl though.\n\r");
        return FALSE;
      }
    }
    TThing *stick;
    if ((stick = getStuckIn(WEAR_FOOT_R)) || (stick = getStuckIn(WEAR_FOOT_L))) {
      act("The $o in your foot causes you great pain!", false, this, stick, 0, TO_CHAR);
      need_movement += 5;
      if (!::number(0,4)) {
        sendTo("Your bad foot causes you to fall over!\n\r");
        doSit("");
        return FALSE;
      }
    }
    if (!canUseLimb(WEAR_FOOT_L) || !canUseLimb(WEAR_FOOT_R)) {
      sendTo("Your foot is badly wounded.  It pains you to move!\n\r");
      need_movement += 5;
      if (!::number(0,4)) {
        sendTo("Your bad foot causes you to fall over!\n\r");
        doSit("");
        return FALSE;
      }
    } else if (eitherLegHurt() && !isLevitating() && !isFlying()) {
      sendTo("Your leg is badly wounded.  It pains you to move!\n\r");
      need_movement += 10;
      if (!::number(0,5)) {
        sendTo("Your bad leg causes you to fall over!\n\r");
        doSit("");
        if (reconcileDamage(this, ::number(2,5), DAMAGE_NORMAL) == -1)
          return DELETE_THIS;
        return FALSE;
      }
    }
    if (getCond(DRUNK) > 9) {
      if (isSwimming()) {
        sendTo("You weave a bit as you swim along.\n\r");
        need_movement += 1;
        if (!::number(0,4)) {
          sendTo("HEY! One of those purple elephants you keep seeing pushed your head underwater.\n\r");
          return FALSE;
        }
      } else {
        sendTo("You stagger a bit as you move along.\n\r");
        need_movement += 1;
        if (!::number(0,4)) {
          sendTo("Ooops, one of those purple elephants you keep seeing must have tripped you.\n\r");
          doSit("");
          return FALSE;
        }
      }
    }      
    if (getPosition() == POSITION_CRAWLING) {
      if ((dir == DIR_UP) || (dir == DIR_DOWN))
        need_movement += 16;
      else
        need_movement += 8;
    }
    // old people move harder  (kicks in around age 50 (human))
    // disabled
#if 0
    if (age()->year - getBaseAge() >= 35)
      need_movement += (age()->year - getBaseAge() - 30)/5;
#endif 

    if (isFlying())
      need_movement = min(1, need_movement/4);

    if (isLevitating())
      need_movement = min(5, need_movement/4);

    if (affectedBySpell(SPELL_HASTE) || affectedBySpell(SPELL_ACCELERATE) || affectedBySpell(SPELL_CHEVAL) || affectedBySpell(SPELL_CELERITE))
      need_movement = max(0, need_movement/2);

    if (inLethargica()) {
      need_movement = 0;
    }
    if (getMove() < need_movement && !isImmortal()) {
      sendTo("You are too exhausted.\n\r");
      return FALSE;
    }
  } else if (riding) {
    if (bothLegsHurt()) {
      sendTo(COLOR_MOBS, fmt("Riding %s without working legs is painful!\n\r") % riding->getName());
      addToMove(-5);
      if (!::number(0,2)) {
        rc = fallOffMount(riding, POSITION_SITTING);
        if (IS_SET_DELETE(rc, DELETE_THIS))
          return DELETE_THIS;
        return FALSE;
      }
      if (reconcileDamage(this,::number(0,2),DAMAGE_NORMAL) == -1)
        return DELETE_THIS;
    }
    if (!riding) {
      // probably the reconcileDamage roll made me fall off
      return FALSE;
    }
    tbt = dynamic_cast<TBeing *>(riding);
    if (tbt && tbt->bothLegsHurt()) {
      act("$N has no working legs to transport you.",
              FALSE, this, 0, tbt, TO_CHAR);
      return FALSE;
    }
    if (eitherLegHurt()) {
      sendTo("A damaged leg or foot makes it tough to ride!\n\r");
      addToMove(-2);
      if (reconcileDamage(this,::number(0,1),DAMAGE_NORMAL) == -1)
        return DELETE_THIS;
      if (!riding) {
        // probably the reconcileDamage roll made me fall off
        return FALSE;
      }
      if (!::number(0,5)) {
        rc = fallOffMount(riding, POSITION_SITTING);
        if (IS_SET_DELETE(rc, DELETE_THIS))
          return DELETE_THIS;
        return FALSE;
      }
    }
    if (!riding) {
      // probably the reconcileDamage roll made me fall off
      return FALSE;
    }
    tbt = dynamic_cast<TBeing *>(riding);
    if (tbt && tbt->eitherLegHurt() && !::number(0,3)) {
      act("$N stumbles due to $S injuries and throws you.",
              FALSE, this, 0, tbt, TO_CHAR);
      if (reconcileDamage(this,::number(0,3),DAMAGE_NORMAL) == -1)
        return DELETE_THIS;
      if (!riding) {
        // probably the reconcileDamage roll made me fall off
        return FALSE;
      }
      rc = fallOffMount(tbt, POSITION_SITTING);
      if (IS_SET_DELETE(rc, DELETE_THIS))
        return DELETE_THIS;
      return FALSE;
    }
    // old forula was > (riding->weight/2)) changed for practical - Cos
        // riding-weight < total weight
    if (riding && 
        (compareWeights(riding->getWeight(), getTotalWeight(TRUE)) == 1)) {
      act("$N collapses beneath your weight.", 
               FALSE, this, 0, riding, TO_CHAR);
      act("$N collapses beneath $n's weight.", 
               FALSE, this, 0, riding, TO_NOTVICT);
      act("You collapse beneath $n's weight.", 
               FALSE, this, 0, riding, TO_VICT);
      TBeing *tbr = dynamic_cast<TBeing *>(riding);
      if (tbr) {
        tbr->setMove(0);
      }
      addToMove(-2);
      rc = fallOffMount(riding, POSITION_SITTING);
      if (IS_SET_DELETE(rc, DELETE_THIS))
        return DELETE_THIS;
      if (reconcileDamage(this,::number(0,1),DAMAGE_NORMAL) == -1)
        return DELETE_THIS;
      return FALSE;
    }
    if (riding && getCond(DRUNK) > 9) {
      sendTo("You wobble drunkenly as your mount moves along.\n\r");
      if (!::number(0,4)) {
        sendTo("Ooops, one of those purple elephants you keep seeing must have pushed you off.\n\r");
        rc = fallOffMount(riding, POSITION_SITTING);
        if (IS_SET_DELETE(rc, DELETE_THIS))
          return DELETE_THIS;
        return FALSE;
      }
    }
    if (riding) {
      if (riding->isLevitating())
        need_movement /= 4;
      if (riding->isFlying())
        need_movement = 1;
      TBeing *tbr = dynamic_cast<TBeing *>(riding);
      if (tbr) {
        if (tbr->getMove() < need_movement) {
          sendTo("Your mount is too exhausted.\n\r");
          return FALSE;
        }
      }
    }
    if (riding && getMove() < 1) {
      act("You're too tired to stay on your $o.", false, this, riding, 0, TO_CHAR);
      rc = fallOffMount(riding, POSITION_SITTING);
      if (IS_SET_DELETE(rc, DELETE_THIS))
        return DELETE_THIS;
      return FALSE;
    }
  } else { 
  // this is basically the case where they are flying
    if (getCond(DRUNK) > 9) {
      sendTo("Your drunken flight pattern is fairly erratic.\n\r");
      need_movement++;
      if (!::number(0,4)) {
        sendTo("Oops, you must have crashed into one of those purple elephants you keep seeing.\n\r");
        rc = crashLanding(POSITION_SITTING);
        if (IS_SET_DELETE(rc, DELETE_THIS))
          return DELETE_THIS;
        return FALSE;
      }
    }
    if (isLevitating())
      need_movement /= 4;
    if (isFlying())
      need_movement = 1;    

    // old people move harder
/*    if (age()->year - getBaseAge() >= 50) {
      need_movement += (age()->year - getBaseAge() - 50)/10;
      if (((age()->year - getBaseAge() - 50)%10) >= ::number(1,10))
        need_movement++;
    }
*/ 
    if (affectedBySpell(SPELL_HASTE) || affectedBySpell(SPELL_ACCELERATE) || affectedBySpell(SPELL_CHEVAL) || affectedBySpell(SPELL_CELERITE))
      need_movement = max(0, need_movement/2);

    if ((getMove() < need_movement) && !isImmortal()) {
      sendTo("You are too exhausted.\n\r");
      return FALSE;
    }
  }
  if (to_here->isFlyingSector()) {
    TBeing *tbr = dynamic_cast<TBeing *>(riding);
    if (tbr && !tbr->isFlying()) {
      tbr->setPosition(POSITION_FLYING);
      sendTo("Without effort, your mount starts to fly around.\n\r");
    } else if (riding && !tbr) {
      dismount(POSITION_FLYING);
      sendTo("Without effort, you start to fly around.\n\r");
    } else if (riding && riding->isFlying()) {
    } else if (!isFlying()) {
      sendTo("Without effort, you start to fly around.\n\r");
      setPosition(POSITION_FLYING);
    }
  }
  if (from_here->isFlyingSector() && !to_here->isFlyingSector()) {
    TBeing *tbr = dynamic_cast<TBeing *>(riding);
    if (to_here->isAirSector() || to_here->isVertSector()) {
      if (tbr && !tbr->canFly()) {
        sendTo("Your mount would need fly to go there.\n\r");
        return FALSE;
      }
      if (!riding && !affectedBySpell(SPELL_FLY) && !isAffected(AFF_FLYING)) {
        sendTo("You would need to control your flying to go there.\n\r");
        return FALSE;
      } 
    } else if (to_here->isWaterSector() || to_here->isUnderwaterSector()) {
      if (!riding) 
        setPosition(POSITION_STANDING);
    } else if (tbr) {
      if  (tbr->canFly()) {
      } else {
        tbr->setPosition(POSITION_STANDING);
        sendTo("Your mount stops flying around.\n\r");
      }
    } else if (getPosition() == POSITION_FLYING) {
      if (!affectedBySpell(SPELL_FLY) && !isAffected(AFF_FLYING)) {
        setPosition(POSITION_STANDING);
        sendTo("The magic leaves you and you are set on your feet.\n\r");
      }
    }
  }
  if (to_here->isWaterSector()) {
#if 0
    if (getPosition() == POSITION_CRAWLING) {
      sendTo("You can't crawl into the water.\n\r");
      return FALSE;
    } else 
#endif
    if (from_here->isAirSector()) {
    } else if (from_here->isUnderwaterSector()) {
      sendTo("Your head breaks the surface, and you take a big refreshing gulp of air.\n\r");
    } else if (!from_here->isWaterSector()) {
      if (riding) 
        sendTo("You urge your mount out into the water.\n\r");
      else if (isLevitating() || isFlying())
         sendTo("If it weren't for your magic, your feet would be wet now.\n\r");
      else if (!has_boat)
          sendTo("You wade out into the water.  I hope you can swim...\n\r");
      else
        sendTo("You push your boat out into the water.\n\r");
    } else {
      if (!isLevitating() && !isFlying()) {
        tbt = dynamic_cast<TBeing *>(riding);
        if (tbt) {
          if (!tbt->isLevitating() && !tbt->isFlying()) {
            if (tbt->canSwim(dir) > 0) {
              sendTo("Your mount swims through the waters.\n\r");
            } else {
              sendTo("Your mount is unable to swim through the water.\n\r");
              tbt->addToMove(-2);
              return FALSE;
            }
          }
        } else {
          if (!has_boat) {
            if ((rc = canSwim(dir)) > 0) {
              sendTo("You swim valiantly through the water.\n\r");
            } else {
              sendTo("You try to swim, but fail to make any headway.\n\r");
              if (rc == -1)
                sendTo("The weight of your equipment and inventory makes it too difficult.\n\r");
              addToMove(-::number(1,7));
              if (reconcileDamage(this, ::number(1,7),DAMAGE_DROWN) == -1)
                return DELETE_THIS;

              return FALSE;
            }
          }
          if (has_boat)
            need_movement = max(2, (::number(2, need_movement)));
          else if (affectedBySpell(AFFECT_TRANSFORMED_LEGS))
            need_movement /= 2;
        }
      }
    }
  }
  // Movement in SECT_AIR 
  if (from_here->isAirSector() || to_here->isAirSector()) {
    if (getPosition() == POSITION_CRAWLING) {
      sendTo("I'm afraid crawling through the air is not possible.\n\r");
      return FALSE;
    }
    if (!isFlying()) {
      if ((!riding || !riding->isFlying()) && (dir == DIR_UP)) {
        sendTo("You would have to fly to go there!\n\r");
        return FALSE;
      }
    }
  }
  // Movement in SECT_UNDERWATER 
  if (to_here->isUnderwaterSector()) {
#if 0
    if (getPosition() == POSITION_CRAWLING) {
      sendTo("You can't crawl into the water.\n\r");
      return FALSE;
    } else 
#endif
    if (from_here->isUnderwaterSector()) {
      if (!isAffected(AFF_WATERBREATH)) {
        if ((rc = canSwim(dir)) > 0) {
          sendTo("You swim valiantly under the water.\n\r");
        } else {
          sendTo("You try to swim under the water, but fail to make any headway.\n\r");
          if (rc == -1)
            sendTo("Perhaps you are over-encumbered.\n\r");
          addToMove(-::number(4,10));
          if (reconcileDamage(this, ::number(4,10),DAMAGE_DROWN) == -1)
            return DELETE_THIS;

          return FALSE;
        }
      }
#if 0
    // these cases are all from non-underwater to underwater bwlow here
    } else if (isLevitating() || isFlying()) {
      sendTo("The fact that you are flying prevents you from going underwater.\n\r");
      return FALSE;
    } else if (has_boat) {
      sendTo("You can't go underwater while in a boat.\n\r");
      return FALSE;
#endif
    } else if (riding) {
      TBeing *tbt = dynamic_cast<TBeing *>(riding);
      if (tbt && tbt->isAffected(AFF_SWIM)) {
        sendTo("Your mount dives beneath the surface.\n\r");
      } else {
        sendTo("Your mount refuses to go underwater.\n\r");
        return FALSE;
      }
    } else if (isAffected(AFF_WATERBREATH)) {
      sendTo("You dive beneath the surface.\n\r");
    } else {
      sendTo("You dive beneath the surface and try to hold your breath.\n\r");
    }
  }
  if (to_here->isVertSector() || from_here->isVertSector()) {
    // in general, don't let them be crawling in a climb zone
    // I don't recall why crawling in climb is bad, but assume its valid
    // but since the positions are sort of similar, just switch their position
    // and allow it
    if (getPosition() == POSITION_CRAWLING) {
#if 0
      sendTo("I'm afraid crawling while trying to climb is not possible.\n\r");
      return FALSE;
#else
      sendTo("You stop crawling and start climbing.\n\r");
      setPosition(POSITION_STANDING);
#endif
    }
    if (!riding && isFlying()) {
    } else if (riding && riding->isFlying()) {
    } else if (riding) {
      sendTo("Your mount refuses to climb for you.\n\r");
      return FALSE;
    } else if (dir == DIR_UP) 
      sendTo("You climb upward.\n\r");
    else if (dir == DIR_DOWN) 
      sendTo("You rappel downward.\n\r");
    else { // lateral movement
    }
  }
  if (getPosition() == POSITION_CRAWLING) 
    sendTo(fmt("You crawl %s.\n\r") %dirs[dir]);
  else if (isAffected(AFF_BLIND) && !isImmortal() && !isAffected(AFF_TRUE_SIGHT)) {
    if (dir == DIR_UP || dir == DIR_DOWN) {
      // say nothing on these
    } else if (isSwimming()) {
      sendTo(fmt("You blindly paddle %s.\n\r") %dirs[dir]);
    } else {
      sendTo(fmt("You blindly stumble %s.\n\r") %dirs[dir]);
    }
  }

  
  foodNDrink(from_here->getSectorType(),1);


  wearNTear();

  if (!isImmortal() || riding) {
    if (doesKnowSkill(SKILL_HIKING) &&
        (from_here->isMountainSector() ||
         from_here->isSwampSector()    ||
         from_here->isForestSector())) {
      need_movement *= (200 - getSkillValue(SKILL_HIKING));
      need_movement /= 200;
    }
    /*
    if (doesKnowSkill(SKILL_MOUNTAIN_WALK) && from_here->isMountainSector()) {
      need_movement *= (200 - getSkillValue(SKILL_MOUNTAIN_WALK));
      need_movement /= 200;
    }
    if (doesKnowSkill(SKILL_SWAMP_WALK) && from_here->isSwampSector()) {
      need_movement *= (200 - getSkillValue(SKILL_SWAMP_WALK));
      need_movement /= 200;
    }
    if (doesKnowSkill(SKILL_FOREST_WALK) && from_here->isForestSector()) {
      need_movement *= (200 - getSkillValue(SKILL_FOREST_WALK));
      need_movement /= 200;
    }
    */
    if (inLethargica()) 
      need_movement = 0;

    if (riding) {
      TBeing *tbr = dynamic_cast<TBeing *>(riding);
      if (tbr)
        tbr->addToMove(-need_movement);
      addToMove(-::number(0,need_movement)/3);
    } else
      addToMove(-need_movement);
  }
  // Check for some traps that would hinder movement
  TBeing *tbng = dynamic_cast<TBeing *>(riding);
  if (tbng) {
    rc = tbng->checkForMoveTrap(dir);
    if (IS_SET_DELETE(rc, DELETE_THIS))
      return DELETE_THIS;
    else if (rc)
      return FALSE;
  } else {
    rc = checkForMoveTrap(dir);
    if (IS_SET_DELETE(rc, DELETE_THIS))
      return DELETE_THIS;
    else if (rc)
      return FALSE;
  }

  if (riding) {
    // this is here because if mount moves by itself, rider needs to know
    sprintf(tmp, "You ride %s.", dirs[dir]);
    act(tmp, 0, this, 0, 0, TO_CHAR);

    sprintf(tmp, "$n and you ride %s.", dirs[dir]);
    act(tmp, FALSE, this, 0, riding, TO_VICT);

    --(*riding);
    thing_to_room(riding, new_r);

    TBeing *tbt = dynamic_cast<TBeing *>(riding);
    if (tbt) {
      rc = tbt->bumpHeadDoor(from_here->dir_option[dir], &iHeight);
      if (IS_SET_DELETE(rc, DELETE_THIS)) {
        delete tbt;
        tbt = NULL;
      }
      if (tbt)
        tbt->doLook("", CMD_LOOK);
    }
  }

  if(isPc())
    specials.last_direction=dir;

  --(*this);
  thing_to_room(this, new_r);
  rc = bumpHeadDoor(from_here->dir_option[dir], &iHeight);
  if (IS_SET_DELETE(rc, DELETE_THIS))
    return DELETE_THIS;
  doLook("", CMD_LOOK);

  // I am in new room and have looked at it
  // but people have not seen me leave or enter yet (displayMove())

  if (to_here->isUnderwaterSector())
    putOutLightsInWater();

#if 0
  // useful for creating paths
  if (!strcmp(name, "Batopr")) {
    FILE *fp;
    fp = fopen("/tmp/paths", "a+");
    fprintf(fp, "{%d, %d},\n", dir, in_room);
    fclose(fp);
  }
#endif

  return TRUE;
}

// returns DELETE_THIS
int TBeing::moveOne(dirTypeT dir)
{
  int was_in, result;
  int rc;

  was_in = in_room;
  if ((result = rawMove(dir))) {
    if (IS_SET_DELETE(result, DELETE_THIS))
      return DELETE_THIS;

    rc = displayOneMove(dir, was_in);
    if (IS_SET_DELETE(rc, DELETE_THIS))
      return DELETE_THIS;

    return TRUE;
  } else
    return FALSE;
}

// returns DELETE_THIS
int TBeing::moveGroup(dirTypeT dir)
{
  int was_in, heap_top, result, rc;
  followData *k, *n;

  was_in = in_room;
  result = rawMove(dir);
  if (result) {
    if (IS_SET_DELETE(result, DELETE_THIS))
      return DELETE_THIS;

    rc = displayOneMove(dir, was_in);
    if (IS_SET_DELETE(rc, DELETE_THIS))
      return DELETE_THIS;
    if (!rc)
      return FALSE;   // could not display move
    if (rc == 2)
      return FALSE;  // move got aborted, no follower motion
    if (followers) {
      heap_top = 0;
      for (k = followers; k; k = n) {
        n = k->next;
        TBeing *tft = k->follower;

        // sanity check, necessary cuz things going whacky occasionally
        // bat - 11/19/99
        if (tft->master != this) {
          // this happens, but I guess it's safe to just ignore
          vlogf(LOG_BUG, fmt("ERROR: Bad critter looping through moveGroup()! (this=[%s], badguy=[%s], master=[%s])") %  getName() % 
		(tft->getName() ? tft->getName() : "NoName") % 
		(tft->master ? tft->master->getName() : "NoMaster"));
          continue;
        }

        // my mount moved with me in rawMove
        // my mount is one of my followers, but note that while mount
        // has moved with me, other riders have not.
        if (tft->rider) {
          // we must do 2 things here
          // move all my OTHER riders (my horseMaster has already moved)
          // move anyone following me
          TThing *t, *t2;
          TBeing *tb;
          for (t = tft->rider; t; t = t2) {
            t2 = t->nextRider;
            if (t == tft->horseMaster())
              continue;
            tb = dynamic_cast<TBeing *>(t);
            if (!tb)
              continue;
            act("You follow $N.", FALSE, tb, 0, this, TO_CHAR);
	    if (hasClass(CLASS_SHAMAN)) {
	      addToLifeforce(-1);
	    }
            if (tb->followers) {
              // move followers of other riders
              rc = tb->moveGroup(dir);
              if (IS_SET_DELETE(rc, DELETE_THIS)) {
                delete tb;
                tb = NULL;
                continue;
              }
              // if this moveGroupFailed (movement points or something)
              // then horse and horsemaster moved, but I did not...
              // it is possible I was already dismounted, in which case we can skip
              if (!rc && tb->riding) {
                positionTypeT post = tb->getPosition();
                tb->dismount(post <= POSITION_STANDING ? post : POSITION_STANDING);
              }
            } else {
              rc = tb->rawMove(dir);
              if (IS_SET_DELETE(rc, DELETE_THIS)) {
                delete tb;
                tb = NULL;
                continue;
              }
              if (rc) {
                rc = tb->displayOneMove(dir, was_in);
                if (IS_SET_DELETE(rc, DELETE_THIS)) {
                  delete tb;
                  tb = NULL;
                }
              }
              // if this moveGroupFailed (movement points or something)
              // then horse and horsemaster moved, but I did not...
              if (!rc && tb->riding) {
                positionTypeT post = tb->getPosition();

#if 1
// multiple cores generated here, riding (above) yet !riding in dismount()
// this is temporary. bat 7/9/99
                if (tb->riding)
                  tb->dismount(post <= POSITION_STANDING ? post : POSITION_STANDING);
                else
                  vlogf(LOG_BUG, "riding error in moveGroup");
#else
                tb->dismount(post <= POSITION_STANDING ? post : POSITION_STANDING);
#endif
              }
            }
          }  // handler for extra riders

          // is someone following the horse?
          if (followers) {
            // this gets ugly, so I won't do it.
          } // handler for followers of horse

          continue;
        }

        if ((was_in == tft->inRoom()) && !tft->fight() &&
            (tft->getPosition() >= POSITION_CRAWLING) && result) {
          if (tft->riding && 
              (tft != tft->riding->horseMaster())) {
            // not in control of horse, so can't follow
            continue;
          }
          act("You follow $N.", FALSE, tft, 0, this, TO_CHAR);
	  if (hasClass(CLASS_SHAMAN)) {
	    addToLifeforce(-1);
	  }
          if (tft->followers) {
            rc = tft->moveGroup(dir);
            if (IS_SET_DELETE(rc, DELETE_THIS)) {
              delete tft;
              tft = NULL;
              continue;
            }
          } else {
            rc = tft->rawMove(dir);
            if (IS_SET_DELETE(rc, DELETE_THIS)) {
              delete tft;
              tft = NULL;
              continue;
            }
            if (rc) {
              rc = tft->displayOneMove(dir, was_in);
              if (IS_SET_DELETE(rc, DELETE_THIS)) {
                delete tft;
                tft = NULL;
              }
            }
          }
        }
      }
    }
  }
  return FALSE;
}

// may return DELETE_THIS
int TBeing::displayOneMove(dirTypeT dir, int was_in)
{
  return displayMove(dir, was_in, 1);
}

// may return DELETE_THIS
int TBeing::displayGroupMove(dirTypeT dir, int was_in, int total)
{
  return displayMove(dir, was_in, total);
}


// returns DELETE_THIS if this should be deleted
// returns 0 if movement attempt was failed
// returns 1 if movement attempt succeeded
int TBeing::doMove(cmdTypeT cmd)
{
  dirTypeT dir = getDirFromCmd(cmd);
  return doMove(dir);
}

int TBeing::doMove(dirTypeT cmd)
{
  int rc = 0;

  if (riding && (this != riding->horseMaster())) {
    act("You are not the master of $N, and can't control where $E goes.",
         FALSE, this, 0, riding, TO_CHAR);
    return FALSE;
  }

  if (isAffected(AFF_SHOCKED)) {
    sendTo("You are still recovering from something and cannot bring yourself to move.\n\r");
    return FALSE;
  }

  // If I am a mount, make my master move
  // this causes me to be moved WITH him, so don't do me separately
  TBeing *tb = dynamic_cast<TBeing *>(rider);
  if (tb) {
    if (tb->rideCheck(0)) {
      rc = tb->doMove(cmd);
      if (IS_SET_DELETE(rc, DELETE_THIS)) {
        // death room, may as well kill me too.
        delete tb;
        tb = NULL;
        return DELETE_THIS;
      }
      return 1;
    } else {
      rc = tb->fallOffMount(this, POSITION_SITTING);
      if (IS_SET_DELETE(rc, DELETE_THIS)) {
        delete tb;
        tb = NULL;
      }
      return 1;
    }
  }
  if (attackers > 1) {
    sendTo("There's too many people around, no place to flee!\n\r");
    return 0;
  }

  // This is needed.  If the move was typed in directly, it would get caught
  // by the argument parser.  But anywhere doMove called indirectly
  // like "order horse east", we need an extra check for it here...
  if (fight()) {
    sendTo("You can't concentrate enough while fighting!\n\r");
    return FALSE;
  }

  if (!followers && !master) {
    // PS. this precludes mounted people since horse follows rider...
    rc = moveOne(cmd);
    if (IS_SET_DELETE(rc, DELETE_THIS))
      return DELETE_THIS;
    if (!rc)
      return FALSE;
  } else {
    if (!followers) {
      // PS. this precludes mounted people since horse follows rider...
      rc = moveOne(cmd);
      if (IS_SET_DELETE(rc, DELETE_THIS))
        return DELETE_THIS;
      if (!rc)
        return FALSE;
    } else {
      rc = moveGroup(cmd);
      if (IS_SET_DELETE(rc, DELETE_THIS))
        return DELETE_THIS;
      if (!rc)
        return FALSE;
    }
  }
  goBerserk(NULL);

  return TRUE;
}


// returns DELETE_THIS
// retuns FALSE on error
// returns TRUE if successfully displayed
// displays 2 if displayed, but move got aborted by special in next room
int TBeing::displayMove(dirTypeT dir, int was_in, int total)
{
  // due to how act() handles hide_invis flag, have to have the
  // act() do a for-loop to VICT

  TThing *t, *t2;
  char tmp[256], how[128];
  TRoom *rp1, *rp2;
  int rc;

  rp1 = real_roomp(was_in);
  rp2 = roomp;

  if (!rp1 || !rp2) {
    if (!getName()) {
      vlogf(LOG_BUG, "NULL getName in NULL rp in displayMove()");
      return FALSE;
    }
    vlogf(LOG_BUG, fmt("NULL rp in displayMove!  (%s)(%d)") %  getName() % was_in);
    return FALSE;
  }
  strcpy(how, movementType(FALSE).c_str());

  if (total > 1)
    sprintf(tmp, "$n %s %s. [%d]", how, dirs[dir], total);
  else {
    if (riding) 
      sprintf(tmp, "$n leaves %s, riding on $p.", dirs[dir]);
    else if (eitherLegHurt() && !isFlying()) 
      sprintf(tmp, "$n hobbles on one leg as $e leaves %s.", dirs[dir]);
    else 
      sprintf(tmp, "$n %s %s.", how, dirs[dir]);
  }
  // at time func called, this is actually in new room
  // temporarily put him back so canSee works properly
  --(*this);
  *rp1 += *this;
  for (t = rp1->getStuff(); t; t = t->nextThing) {
    TBeing *ch = dynamic_cast<TBeing *>(t);
    if (!ch)
      continue;
    if (this == ch)
      continue;
    if (!ch->awake())
      continue;

    // ways to detect it:  be immortal, be able to see it, be able to hear it,
    if (ch->isImmortal() || ch->canSee(this, INFRA_YES) || makesNoise()) {
      if (isImmortal())
        act(msgVariables(MSG_MOVE_IN, (TThing *)NULL, dirs[dir]),
            FALSE, this, riding, ch, TO_VICT);
      else {
#if 1
        if (ex_description && ex_description->findExtraDesc("moveout"))
          strcpy(tmp, ex_description->findExtraDesc("moveout"));
#endif

        act(tmp, FALSE, this, riding, ch, TO_VICT);
      }
    }
  }

  // not totallys ure why this is needed, but had a core report of
  // still being in casino game and moving. 3/6/99
  removeAllCasinoGames();

  --(*this);
  *rp2 += *this;

  strcpy(how, movementType(TRUE).c_str());

  if ((dir < 4) || (dir > 5)) {
    if (total == 1) {
      if (riding) {
        sprintf(tmp, "$n has arrived from the %s, riding on $p.", dirs[rev_dir[dir]]);
      } else if (eitherLegHurt()) {
        sprintf(tmp, "$n has arrived from the %s, hobbling on one leg.",
                     dirs[rev_dir[dir]]);
      } else {
        sprintf(tmp, "$n %s from the %s.", how, dirs[rev_dir[dir]]);
      }
    } else
      sprintf(tmp, "$n %s from the %s.", how, dirs[rev_dir[dir]]);
  } else if (dir == 4) {
    if (total == 1) {
      if (riding) {
        sprintf(tmp, "$n has arrived from below, riding on $p.");
      } else if (eitherLegHurt()) {
        sprintf(tmp, "$n has arrived from below, hobbling on one leg.");
      } else {
        sprintf(tmp, "$n %s from below.", how);
      }
    } else
      sprintf(tmp, "$n %s from below.", how);
  } else if (dir == 5) {
    if (total == 1) {
      if (riding) {
        sprintf(tmp, "$n has arrived from above, riding on $p.");
      } else if (eitherLegHurt()) {
        sprintf(tmp, "$n has arrived from above, hobbling on one leg.");
      } else {
        sprintf(tmp, "$n %s from above.", how);
      }
    } else
      sprintf(tmp, "$n %s from above.", how);
  } else
    sprintf(tmp, "$n has arrived from somewhere.");

  if (total > 1)
    sprintf(tmp + strlen(tmp), " [%d]", total);

  for (t = rp2->getStuff(); t; t = t->nextThing) {
    TBeing *tbt = dynamic_cast<TBeing *>(t);
    if (!tbt)
      continue;
    if (this == tbt || tbt == riding)
      continue;
    if (!tbt->awake())
      continue;

    // ways to detect it:  be immortal, be able to see it, be able to hear it,
    // just dumb luck

    if (tbt->isImmortal() ||
        (tbt->canSee(this, INFRA_YES)) ||
        (makesNoise())) {
      if (isImmortal()) {
        char dirText[256];

        if ((dir < 4) || (dir > 5))
          sprintf(dirText, "the %s", dirs[rev_dir[dir]]);
        else if (dir == 4)
          strcpy(dirText, "below");
        else
          strcpy(dirText, "above");

        act(msgVariables(MSG_MOVE_OUT, (TThing *)NULL, dirText),
            FALSE, this, riding, tbt, TO_VICT);
      } else {
#if 1
        if (ex_description && ex_description->findExtraDesc("movein"))
          strcpy(tmp, ex_description->findExtraDesc("movein"));
#endif

        act(tmp, FALSE, this, riding, tbt, TO_VICT);
      }
    }
  }

  // everybody in room has now seen me come into room and leave old room

  if (riding) {
    rc = riding->genericMovedIntoRoom(rp2, was_in);
    if (IS_SET_DELETE(rc, DELETE_THIS)) {
      delete riding;
      riding = NULL;
      REM_DELETE(rc, DELETE_THIS);
    }
    if (rc == 2) 
      return FALSE;
  } else {
    rc = genericMovedIntoRoom(rp2, was_in);
    if (IS_SET_DELETE(rc, DELETE_THIS))
      return DELETE_THIS;
  }

  // check for items we want to drag with us...
  for (t = rp1->getStuff(); t; t = t2) {
    t2 = t->nextThing;
    
    TObj *obj = dynamic_cast<TObj *>(t);
    if (!obj)
      continue;

    rc = obj->checkSpec(this, CMD_OBJ_MOVEMENT, (char *) dir, NULL);
    if (IS_SET_DELETE(rc, DELETE_THIS)) {
      delete obj;
      obj = NULL;
    }
    if (IS_SET_DELETE(rc, DELETE_VICT))
      return DELETE_THIS;
  }

  if (rc)   // for rc == 2 case
    return rc;

  return TRUE;
}

// returns DELETE_THIS
// may return 2 if a spec proc prevented motion (was_in != -1)
// was_in should equal -1 if you don't want player being returned to original rm
// gets called recursively for all riders, so initial call should be for
// lowest level of riding
int TBeing::genericMovedIntoRoom(TRoom *rp, int was_in, 
     checkFallingT checkFall)
{
  TThing *t, *t2, *t3;
  int rc;
  TMonster *mob;
  int groupcount=0;// used to make mobs not go superaggro on groups - dash

  
  for (t3 = roomp->getStuff(); t3; t3 = t3->nextThing) {
    TBeing *tbt = dynamic_cast<TBeing *>(t3);
    if (tbt && inGroup(*tbt))
      groupcount++;
  }
  if (was_in != -1) {
    for (t3 = real_roomp(was_in)->getStuff(); t3; t3 = t3->nextThing) {
      TBeing *tbt = dynamic_cast<TBeing *>(t3);
      if (tbt && inGroup(*tbt))
	groupcount++;
    }
  }

  // ok, now we have the number of people in my group who are (supposedly) travelling with me
  // we'll use it later to modify how often we call the roomenter mobaggro code -dash

  for (t = rider; t; t = t2) {
    t2 = t->nextRider;
    rc = t->genericMovedIntoRoom(rp, was_in);
    if (IS_SET_DELETE(rc, DELETE_THIS)) {
      delete t;
      t = NULL;
    }
  }    
  if (rp->isRoomFlag(ROOM_DEATH) && !isImmortal()) {
    vlogf(LOG_MISC, fmt("%s killed by DEATHTRAP at %s (%d)") % 
          getName() % roomp->getName() % inRoom());
    die(DAMAGE_NORMAL);
    return DELETE_THIS;
  }

  rc = rp->checkSpec(this, CMD_ROOM_ENTERED, NULL, NULL);
  if (IS_SET_DELETE(rc, DELETE_THIS))
    return DELETE_THIS;
 
  for (t = rp->getStuff(); t; t = t2) {
    t2 = t->nextThing;
    TMonster *tmons = dynamic_cast<TMonster *>(t);
    if (!tmons)
      continue;
    if (this == tmons)
      continue;

    if (was_in != -1) {
      rc = tmons->checkSpec(this, CMD_MOB_MOVED_INTO_ROOM, "", (TObj *) was_in);
      if (rc) {
        // if TRUE, was prevented from entering.  
        // will have been put back into original room
        if (!rider)
          act("$n returns immediately.", TRUE, this, 0, 0, TO_ROOM);

        doLook("", CMD_LOOK);
        return 2;
      }
    }
    char buf[32];
    sprintf(buf, "%d", was_in);
    rc = tmons->checkResponses(this, 0, buf, CMD_RESP_ROOM_ENTER);
    if (IS_SET_DELETE(rc, DELETE_THIS)) {
      delete tmons;
      tmons = NULL;
    }
    if (IS_SET_DELETE(rc, DELETE_VICT)) 
      return DELETE_THIS;

    rc = tmons->fearCheck(this, false);
    if (IS_SET_DELETE(rc, DELETE_THIS)) {
      delete tmons;
      tmons = NULL;
    }
    if (rc)
      continue;
    
    if((groupcount == 1 && (::number(1,100)<85)) || 
       (groupcount != 1 && !::number(0,groupcount-1) && (::number(1,100)<66))) {
      // people walking alone, 85%
      // people in groups get 66% / num of people in group (66% chance of aggroing the group)
      //ok here's what were doing - we basically saying he'll only try and aggro once
      //per group. roughly, sort of. its a hack, so sue me.
      rc = tmons->aggroCheck(false);
      if (IS_SET_DELETE(rc, DELETE_THIS)) {
	delete tmons;
	tmons = NULL;
      }
      if (rc)
	continue;
    }
  }
  // special stuff for quest bits
  if (inRoom() == AVENGER_ROOM && hasQuestBit(TOG_AVENGER_HUNTING)) {
    // avenger quest, hunting troll set and got to right room

    if (mob_index[real_mobile(MOB_TROLL_GIANT)].getNumber()) {
      // don't load unless unique
      return TRUE;
    }
    if (!(mob = read_mobile(MOB_TROLL_GIANT, VIRTUAL))) {
      vlogf(LOG_BUG, "Problem loading mob for quest.");
      return TRUE;
    }
    *rp += *mob;
    act("$n appears from a swirling mist.", FALSE, mob, 0, 0, TO_ROOM);
    act("$n says, \"So, $N, you think you can defeat me?\"", FALSE, mob, 0, this, TO_ROOM);
    act("$n says, \"We shall soon see about that.\"", FALSE, mob, 0, this, TO_ROOM);
  }

  if (in_room == ROOM_VINDICATOR_2 && hasQuestBit(TOG_VINDICATOR_HUNTING_2)) {
    // vindicator quest, hunting tree spirit set and got to right room
    if (mob_index[real_mobile(MOB_TREE_SPIRIT)].getNumber()) {
      // don't load unless unique
      return TRUE;
    }
    if (!(mob = read_mobile(MOB_TREE_SPIRIT, VIRTUAL))) {
      vlogf(LOG_BUG, "Problem loading mob for quest.");
      return TRUE;
    }
    *rp += *mob;
    act("In front of you, the limbs of a tree start to bend and smoke rises from the $g.", FALSE, mob, 0, 0, TO_ROOM);
    act("With a flash of a blinding light, $n forms from the smoke.", FALSE, mob, 0, 0, TO_ROOM);
    act("$n says, \"So, $N, you think you can defeat me?\"", FALSE, mob, 0, this, TO_ROOM);
    act("$n says, \"We shall soon see about that.\"", FALSE, mob, 0, this, TO_ROOM);
  }

  if (checkFall) {
    // since falling causes this function to be called (with -1 arg)
    // we don't want to go into endless loop.  Since checkFalling will
    // keep looping until splat, we avoid this routine.

    rc = checkFalling();
    if (IS_SET_DELETE(rc, DELETE_THIS))
      return DELETE_THIS;
  }

  return TRUE;
}

int AddToCharHeap(TBeing *heap[50], int *top, int total[50], TBeing *k)
{
  int found, i;

  if (*top > 50)
    return FALSE;
  else {
    found = FALSE;
    for (i = 0; ((i < *top) && !found); i++) {
      if (*top > 0) {
        if (dynamic_cast<TMonster *>(k) && (k->number == heap[i]->number) && heap[i]->shortDescr &&
            !strcmp(k->getName(), heap[i]->getName())) {
          total[i] += 1;
          found = TRUE;
        }
      }
    }
    if (!found) {
      heap[*top] = k;
      total[*top] = 1;
      *top += 1;
    }
  }
  return TRUE;
}

// returns DELETE_THIS or false
int TBeing::doOpen(const char *argument)
{
  dirTypeT door;
  char type[MAX_INPUT_LENGTH], dir[MAX_INPUT_LENGTH];
  char buf[256];
  const char *tmpdesc;
  roomDirData *exitp;
  TObj *obj;
  int rc;

  argument_interpreter(argument, type, dir);

  if (!hasHands() || bothHandsHurt()) {
    sendTo("Pretty tough to do without working hands.\n\r");
    return FALSE;
  }

  if (isDumbAnimal()) {
    sendTo("You are a dumb animal, you don't understand things like this!\n\r");
    return FALSE;
  }

  if (!*type) {
    sendTo("Open what?\n\r");
    return FALSE;
  }
  if (strlen(type) < 3) {  
    sendTo("You need to be more specific about what you wish to open.\n\r");
    return FALSE;
  } 

  if ((findDoor(type, dir, DOOR_INTENT_OPEN, SILENT_YES) == DIR_NONE) &&
             (tmpdesc = roomp->ex_description->findExtraDesc(argument))) {
    sendTo(fmt("%s: Your attempt to open it seems to have no effect.\n\r") % type);
    return FALSE;
  } 

  obj = get_obj_vis_accessible(this, argument);
  if (obj) {
    if ((getPosition() < POSITION_CRAWLING) && obj->parent != this) {
      sendTo("You need to stand up to reach it.\n\r");
      doStand();
    }
    rc = rawOpen(obj);
    if (IS_SET_DELETE(rc, DELETE_ITEM)) {
      delete obj;
      obj = NULL;
    }
    if (IS_SET_DELETE(rc, DELETE_THIS))
      return DELETE_THIS;

    return TRUE;
  } 

  // it is plausible that a door in the room has the same keyword as an object
  // we own (ex. steel).  Natural reaction is to "open 2.steel".   The best
  // handling would be to find out how many "steels" were found above, and
  // fix things up here, but since findDoor doesn't know "1.stee" syntax
  // anyway, let's just truncate it here
  char * citer = strchr(type, '.');
  if (citer && *(citer+1))
    strcpy(type, ++citer);

  door = findDoor(type, dir, DOOR_INTENT_OPEN, SILENT_NO);
  if (door >= MIN_DIR) {
    // perhaps it is a door
    if (getPosition() < POSITION_CRAWLING) {
      sendTo("You need to stand up to reach it.\n\r");
      doStand();
    }
    exitp = exitDir(door);
    if (exitp->door_type == DOOR_NONE) {
      sendTo("That's impossible, I'm afraid.\n\r");
      return FALSE;
    }
    if ((exitp->door_type == DOOR_PORTCULLIS) ||
        (exitp->door_type == DOOR_DRAWBRIDGE)) {
      sendTo("You should use raise or lower to manipulate that exit.\n\r");
      return FALSE;
    }
    // Manipulation Examples:
    //    0 Ride: 1.50
    //   10 Ride: 1.45
    //   50 Ride: 1.25
    //  100 Ride: 1.00
    float tRidingManip = ((getPosition() == POSITION_MOUNTED) ?
                          (1.5 - (((float)getSkillValue(SKILL_RIDE) / 2) / 100)) : 1.0);
    if ((exitp->weight * tRidingManip) > maxWieldWeight(NULL, HAND_TYPE_PRIM)) {
      sendTo(fmt("The %s is too large and heavy for you to budge it.\n\r") %
         exitp->getName());
      sprintf(buf, "$n throws $mself at a %s, but $e can't budge it.",
         exitp->getName().c_str());
      act(buf, TRUE, this, 0, 0, TO_ROOM);
      return FALSE;
    }
    if (IS_SET(exitp->condition, EX_DESTROYED)) {
      sendTo(fmt("The %s has been destroyed, it stands wide open.\n\r") %
         exitp->getName());
      return FALSE;
    } else if (IS_SET(exitp->condition, EX_CAVED_IN)) {
      sendTo("A cave-in blocks the way.  It would take hours to dig your way through.\n\r");
      return FALSE;
    } else if (!IS_SET(exitp->condition, EX_CLOSED))
      sendTo("It's already open!\n\r");
    else if (IS_SET(exitp->condition, EX_LOCKED))
      sendTo("It seems to be locked.\n\r");
    else if (IS_SET(exitp->condition, EX_TRAPPED)) {
      if (doesKnowSkill(SKILL_DETECT_TRAP)) {
        if (detectTrapDoor(this, door)) {
          sendTo(fmt("You start to open the %s, but then notice an insidious %s trap...\n\r") %
               sstring(exitp->getName()) % sstring(trap_types[exitp->trap_info]).uncap());
          return FALSE;
        }
      }
      rc = triggerDoorTrap(door);
      if (IS_SET_DELETE(rc, DELETE_THIS))
        return DELETE_THIS;
    } else 
      rawOpenDoor(door);
  }
  return FALSE;
}

int TBeing::doRaise(const char *argument, cmdTypeT cmd)
{
  int rc;
  dirTypeT door;
  char type[MAX_INPUT_LENGTH], dir[MAX_INPUT_LENGTH];
  char buf[256];
  const char *tmpdesc;
  roomDirData *exitp;
  argument_interpreter(argument, type, dir);

  if (checkHoldem()) {
    gHoldem.raise(this, argument);
    return true;
  }
 
  if (isDumbAnimal()) {
    sendTo("You are a dumb animal, you don't understand things like this!\n\r");
    return FALSE;
  }

  if (!*type) {
      if (cmd == CMD_LIFT)
        sendTo("Lift what?\n\r");
      else
        sendTo("Raise what?\n\r");
  } else if ((findDoor(type, dir, DOOR_INTENT_RAISE, SILENT_YES) == DIR_NONE) &&
             (tmpdesc = roomp->ex_description->findExtraDesc(argument))) {
    sendTo(fmt("%s: Your attempt to raise it seems to have no effect.\n\r") % type);
    return FALSE;
  } else if ((door = findDoor(type, dir, DOOR_INTENT_RAISE, SILENT_NO)) >= MIN_DIR) {
    // perhaps it is a door
    exitp = exitDir(door);
    if (exitp->door_type == DOOR_NONE) {
      sendTo("That's impossible, I'm afraid.\n\r");
      return FALSE;
    }
    if ((exitp->door_type != DOOR_PORTCULLIS) && 
        (exitp->door_type != DOOR_DRAWBRIDGE)) {
      sendTo("You should use open or close to manipulate that exit.\n\r");
      return FALSE;
    }
    // Manipulation Examples:
    //    0 Ride: 1.50
    //   10 Ride: 1.45
    //   50 Ride: 1.25
    //  100 Ride: 1.00
    float tRidingManip = ((getPosition() == POSITION_MOUNTED) ?
                          (1.5 - (((float)getSkillValue(SKILL_RIDE) / 2) / 100)) : 1.0);
    if ((exitp->weight * tRidingManip) > maxWieldWeight(NULL, HAND_TYPE_PRIM)) {
      sendTo(fmt("The %s is too large and heavy for you to budge it.\n\r") %
         exitp->getName());
      sprintf(buf, "$n throws $mself at a %s, but $e can't budge it.",
         exitp->getName().c_str());
      act(buf, TRUE, this, 0, 0, TO_ROOM);
      return FALSE;
    }
    if (IS_SET(exitp->condition, EX_DESTROYED)) {
      sendTo(fmt("The %s has been destroyed, it stands wide open.\n\r") %
         exitp->getName());
      return FALSE;
    } else if (IS_SET(exitp->condition, EX_CAVED_IN)) {
      sendTo("A cave-in blocks the way.  It would take hours to dig your way through.\n\r");
      return FALSE;
    } else if (exitp->door_type == DOOR_DRAWBRIDGE) {
      // raiseing drawbridge closes it
      if (IS_SET(exitp->condition, EX_CLOSED))
        sendTo("It's already raised!\n\r");
      else 
        rawCloseDoor(door);
    } else if (exitp->door_type == DOOR_PORTCULLIS) {
      // raiseing portcullis opens it
      if (!IS_SET(exitp->condition, EX_CLOSED))
        sendTo("It's already raised!\n\r");
      else if (IS_SET(exitp->condition, EX_LOCKED))
        sendTo("It seems to be locked.\n\r");
      else if (IS_SET(exitp->condition, EX_TRAPPED)) {
        if (doesKnowSkill(SKILL_DETECT_TRAP)) {
          if (detectTrapDoor(this, door)) {
            sendTo(fmt("You start to raise the %s, but then notice an insidious %s trap...\n\r") %                 exitp->getName() % sstring(trap_types[exitp->trap_info]).uncap());
            return FALSE;
          }
        }
        rc = triggerDoorTrap(door);
        if (IS_SET_DELETE(rc, DELETE_THIS))
          return DELETE_THIS;
      } else 
        rawOpenDoor(door);
    }
  }
  return FALSE;
}

void TBeing::doClose(const char *argument)
{
  dirTypeT door;
  char type[MAX_INPUT_LENGTH], dir[MAX_INPUT_LENGTH];
  char buf[256];
  const char *tmpdesc;
  roomDirData *exitp;
  TObj *obj;

  argument_interpreter(argument, type, dir);

  if (!*type)
    sendTo("Close what?\n\r");
  else if ((obj = get_obj_vis_accessible(this, argument)) && 
           !dynamic_cast<TBaseCorpse *>(obj)) {
    if ((getPosition() < POSITION_CRAWLING) && obj->parent != this) {
      sendTo("You need to stand up to reach it.\n\r");
      doStand();
    }
    // this is an object */
    obj->closeMe(this);
  } else if ((findDoor(type, dir, DOOR_INTENT_CLOSE, SILENT_YES) == DIR_NONE) &&
             (tmpdesc = roomp->ex_description->findExtraDesc(argument))) {
    sendTo(fmt("%s: Your attempt to close it seems to have no effect.\n\r") % type);
    return;
  } else if ((door = findDoor(type, dir, DOOR_INTENT_CLOSE, SILENT_NO)) >= MIN_DIR) {
    if (getPosition() < POSITION_CRAWLING){
      sendTo("You need to stand up to reach it.\n\r");
      doStand();
    }
    exitp = exitDir(door);
    if (exitp->door_type == DOOR_NONE) {
      sendTo("That's impossible, I'm afraid.\n\r");
      return;
    }
    if ((exitp->door_type == DOOR_PORTCULLIS) ||
        (exitp->door_type == DOOR_DRAWBRIDGE)) {
      sendTo("You should use raise or lower to manipulate that exit.\n\r");
      return;
    }
    // Manipulation Examples:
    //    0 Ride: 1.50
    //   10 Ride: 1.45
    //   50 Ride: 1.25
    //  100 Ride: 1.00
    float tRidingManip = ((getPosition() == POSITION_MOUNTED) ?
                          (1.5 - (((float)getSkillValue(SKILL_RIDE) / 2) / 100)) : 1.0);
    if ((exitp->weight * tRidingManip) > maxWieldWeight(NULL, HAND_TYPE_PRIM)) {
      sendTo(fmt("The %s is too large and heavy for you to budge it.\n\r") %
         exitp->getName());
      sprintf(buf, "$n throws $mself at a %s, but $e can't budge it.",
         exitp->getName().c_str());
      act(buf, TRUE, this, 0, 0, TO_ROOM);
      return;
    }
    if (IS_SET(exitp->condition, EX_DESTROYED)) {
      sendTo(fmt("The %s has been destroyed, it can't be closed anymore.\n\r") %
         exitp->getName());
      return;
    } else if (IS_SET(exitp->condition, EX_CAVED_IN)) {
      sendTo("It's been caved in.  You can't get much more closed than that...\n\r");
      return;
    } else if (IS_SET(exitp->condition, EX_CLOSED))
      sendTo("It's already closed!\n\r");
    else {
      rawCloseDoor(door);
    }
  }
}

int TBeing::doLower(const char *argument)
{
  dirTypeT door;
  int rc;
  char type[MAX_INPUT_LENGTH], dir[MAX_INPUT_LENGTH];
  char buf[256];
  const char *tmpdesc;
  roomDirData *exitp;
 
  argument_interpreter(argument, type, dir);
 
  if (isDumbAnimal()) {
    sendTo("You are a dumb animal, you don't understand things like this!\n\r");
    return FALSE;
  }

  if (!*type) {
    sendTo("Lower what?\n\r");
  } else if ((findDoor(type, dir, DOOR_INTENT_LOWER, SILENT_YES) == DIR_NONE) &&
             (tmpdesc = roomp->ex_description->findExtraDesc(argument))) {
    sendTo(fmt("%s: Your attempt to lower it seems to have no effect.\n\r") % type);
    return FALSE;
  } else if ((door = findDoor(type, dir, DOOR_INTENT_LOWER, SILENT_NO)) >= MIN_DIR) {
    exitp = exitDir(door);
    if (exitp->door_type == DOOR_NONE) {
      sendTo("That's impossible, I'm afraid.\n\r");
      return FALSE;
    }
    if ((exitp->door_type != DOOR_PORTCULLIS) &&
        (exitp->door_type != DOOR_DRAWBRIDGE)) {
      sendTo("You should use open or close to manipulate that exit.\n\r");
      return FALSE;
    }
    // Manipulation Examples:
    //    0 Ride: 1.50
    //   10 Ride: 1.45
    //   50 Ride: 1.25
    //  100 Ride: 1.00
    float tRidingManip = ((getPosition() == POSITION_MOUNTED) ?
                          (1.5 - (((float)getSkillValue(SKILL_RIDE) / 2) / 100)) : 1.0);
    if ((exitp->weight * tRidingManip) > maxWieldWeight(NULL, HAND_TYPE_PRIM)) {
      sendTo(fmt("The %s is too large and heavy for you to budge it.\n\r") %
         exitp->getName());
      sprintf(buf, "$n throws $mself at a %s, but $e can't budge it.",
         exitp->getName().c_str());
      act(buf, TRUE, this, 0, 0, TO_ROOM);
      return FALSE;
    }
    if (IS_SET(exitp->condition, EX_DESTROYED)) {
      sendTo(fmt("The %s has been destroyed.\n\r") %
         exitp->getName());
      return FALSE;
    } else if (IS_SET(exitp->condition, EX_CAVED_IN)) {
      sendTo("It's been caved in.  You can't get much more lowered than that...\n\r");
      return FALSE;
    } else if (exitp->door_type == DOOR_DRAWBRIDGE) {
      // lowering drawbridge opens it
      if (!IS_SET(exitp->condition, EX_CLOSED))
        sendTo("It's already lowered!\n\r");
      else if (IS_SET(exitp->condition, EX_LOCKED))
        sendTo("It seems to be locked.\n\r");
      else if (IS_SET(exitp->condition, EX_TRAPPED)) {
        if (doesKnowSkill(SKILL_DETECT_TRAP)) {
          if (detectTrapDoor(this, door)) {
            sendTo(fmt("You start to lower the %s, but then notice an insidious %s trap...\n\r") %                 exitp->getName() %  sstring(trap_types[exitp->trap_info]).uncap());
            return FALSE;
          }
        }
        rc = triggerDoorTrap(door);
        if (IS_SET_DELETE(rc, DELETE_THIS))
          return DELETE_THIS;
      } else
        rawOpenDoor(door);
    } else if (exitp->door_type == DOOR_PORTCULLIS) {
      // lowering portcullis closes it
      if (IS_SET(exitp->condition, EX_CLOSED))
        sendTo("It's already lowered!\n\r");
      else
        rawCloseDoor(door);
    }
  }
  return FALSE;
}

static bool keyCheck(const TObj *obj, int key)
{
  return (obj_index[obj->getItemIndex()].virt == key ||
      (obj->objVnum() == -1 && obj->getSnum() == key));
}

bool has_key(TBeing *ch, int key)
{
  TObj *o = NULL;
  TThing *t, *t2;
  TKeyring *ring;

  for (t = ch->getStuff(); t; t = t->nextThing) {
    o = dynamic_cast<TObj *>(t);
    if (!o)
      continue;
    if (keyCheck(o, key))
      return (1);

    ring = dynamic_cast<TKeyring *>(t);
    if (!ring)
      continue;
    for (t2 = ring->getStuff(); t2; t2 = t2->nextThing) {
      o = dynamic_cast<TObj *>(t2);
      if (!o)
	continue;
      if (keyCheck(o, key))
	return (1);
    }
  }

  if ((t = ch->heldInPrimHand())) {
    o = dynamic_cast<TObj *>(t);
    if (keyCheck(o, key))
      return (1);
  }

  for(wearSlotT i=WEAR_HEAD;i<MAX_WEAR;i++){
    if(ch->equipment[i]){
      o = dynamic_cast<TObj *>(ch->equipment[i]);
      if(keyCheck(o, key))
	return (1);
    }
  }
  
  return (0);
}

void TBeing::doLock(const char *argument)
{
  dirTypeT door;
  char type[MAX_INPUT_LENGTH], dir[MAX_INPUT_LENGTH];
  const char *tmpdesc;
  roomDirData *back, *exitp;
  TObj *obj;
  TRoom *rp;

  argument_interpreter(argument, type, dir);

  if (!*type)
    sendTo("Lock what?\n\r");
  else if ((obj = get_obj_vis_accessible(this, argument)) && 
           !dynamic_cast<TBaseCorpse *>(obj) &&
           !dynamic_cast<TKey *>(obj)) {
    // this is an object */
    obj->lockMe(this);
  } else if ((findDoor(type, dir, DOOR_INTENT_LOCK, SILENT_YES) == DIR_NONE) &&
             (tmpdesc = roomp->ex_description->findExtraDesc(argument))) {
    sendTo(fmt("%s: Your attempt to lock it seems to have no effect.\n\r") % type);
    return;
  } else if ((door = findDoor(type, dir, DOOR_INTENT_LOCK, SILENT_NO)) >= MIN_DIR) {
    exitp = exitDir(door);

    if (exitp->door_type == DOOR_NONE) {
      sendTo("That's impossible, I'm afraid.\n\r");
      return;
    }
    if (IS_SET(exitp->condition, EX_DESTROYED)) {
      sendTo(fmt("The %s has been destroyed; locking it now is a bit silly.\n\r") %

         exitp->getName().c_str());
      return;
    } else if (IS_SET(exitp->condition, EX_CAVED_IN)) {
      sendTo("How do you expect to find the lock when it's buried under that cave in?\n\r");
      return;
    } else if (!IS_SET(exitp->condition, EX_CLOSED))
      sendTo("You have to close it first, I'm afraid.\n\r");
    else if (exitp->key < 0)
      sendTo("There does not seem to be any keyholes.\n\r");
    else if (!has_key(this, exitp->key))
      sendTo("You don't have the proper key.\n\r");
    else if (IS_SET(exitp->condition, EX_LOCKED))
      sendTo("It's already locked!\n\r");
    else {
      SET_BIT(exitp->condition, EX_LOCKED);
      act("$n locks the $T.", TRUE, this, 0, (const TThing *) (exitp->getName().c_str()), TO_ROOM);

      sendTo("*Click*\n\r");

      rp = real_roomp(exitp->to_room);
      if (rp && (back = rp->dir_option[rev_dir[door]]) && back->to_room == in_room)
        SET_BIT(back->condition, EX_LOCKED);
    }
  }
}

void TBeing::doUnlock(const char *argument)
{
  dirTypeT door;
  char type[MAX_INPUT_LENGTH], dir[MAX_INPUT_LENGTH];
  const char *tmpdesc;
  roomDirData *back, *exitp;
  TObj *obj;
  TRoom *rp;

  argument_interpreter(argument, type, dir);

  if (!*type)
    sendTo("Unlock what?\n\r");
  else if ((obj = get_obj_vis_accessible(this, argument)) && 
           !dynamic_cast<TBaseCorpse *>(obj) &&
           !dynamic_cast<TKey *>(obj)) {
    // this is an object */
    obj->unlockMe(this);
  } else if ((findDoor(type, dir, DOOR_INTENT_UNLOCK, SILENT_YES) == DIR_NONE) &&
             (tmpdesc = roomp->ex_description->findExtraDesc(argument))) {
    sendTo(fmt("%s: Your attempt to unlock it seems to have no effect.\n\r") % type);
    return;
  } else if ((door = findDoor(type, dir, DOOR_INTENT_UNLOCK, SILENT_NO)) >= MIN_DIR) {
    exitp = exitDir(door);

    if (exitp->door_type == DOOR_NONE) {
      sendTo("That's impossible, I'm afraid.\n\r");
      return;
    }
    if (IS_SET(exitp->condition, EX_DESTROYED)) {
      sendTo(fmt("The %s has been destroyed and stands wide open.\n\rWhy do you need to unlock it?\n\r") %
         exitp->getName());
      return;
    } else if (IS_SET(exitp->condition, EX_CAVED_IN)) {
      sendTo("It's not locked....  it's caved in!\n\r");
      return;
    } else if (!IS_SET(exitp->condition, EX_CLOSED))
      sendTo("Heck.. it ain't even closed!\n\r");
    else if (exitp->key < 0)
      sendTo("You can't seem to spot any keyholes.\n\r");
    else if (!has_key(this, exitp->key))
      sendTo("You do not have the proper key for that.\n\r");
    else if (!IS_SET(exitp->condition, EX_LOCKED))
      sendTo("It's already unlocked, it seems.\n\r");
    else {
      if (fight()) {
        act("You are forced to sacrifice a few rounds to unlock the $T.",
            TRUE, this, 0, (const TThing *)(exitp->getName().c_str()), TO_CHAR);
        cantHit += loseRound(5);
      }
      char buf[256];
      sprintf(buf, "$n unlocks the $T with %s.", obj_index[real_object(exitp->key)].short_desc);
      act(buf, TRUE, this, 0, (const TThing *) (exitp->getName().c_str()), TO_ROOM);
      sprintf(buf, "You unlock the $T with %s.", obj_index[real_object(exitp->key)].short_desc);
      act(buf, TRUE, this, 0, (const TThing *) (exitp->getName().c_str()), TO_CHAR);
     
      REMOVE_BIT(exitp->condition, EX_LOCKED);

      sendTo("*Click*\n\r");
      rp = real_roomp(exitp->to_room);
      if (rp && (back = rp->dir_option[rev_dir[door]]) && back->to_room == in_room)
        REMOVE_BIT(back->condition, EX_LOCKED);
    }
  }
}

// returns DELETE_THIS, DELETE_ITEM(por)
int TBeing::doEnter(const char *argument, TPortal *por)
{
  int rc;
  char buf[MAX_INPUT_LENGTH];
  TObj *o;
  int dummy = 0;

  if (por) {
    rc = por->enterMe(this);
    if (IS_SET_DELETE(rc, DELETE_VICT | DELETE_THIS)) {
      return DELETE_THIS | DELETE_ITEM;
    }
    if (IS_SET_DELETE(rc, DELETE_VICT)) {
      return DELETE_THIS;
    }
    if (IS_SET_DELETE(rc, DELETE_THIS)) {
      return DELETE_ITEM;
    }
    return FALSE;
  }

  one_argument(argument, buf);

  if (*buf) {
    TThing *tto = searchLinkedListVis(this, buf, roomp->getStuff(), &dummy, TYPEOBJ);
    o = dynamic_cast<TObj *>(tto);
    if (!o) {
      sendTo("Enter what?\n\r");
      return FALSE;
    }
    if (getPosition() <= POSITION_SITTING) {
      sendTo("You need to be mobile to do that.\n\r");
      return FALSE;
    }
    rc = o->enterMe(this);
    if (IS_SET_DELETE(rc, DELETE_THIS)) {
      delete o;
      o = NULL;
    }
    if (IS_SET_DELETE(rc, DELETE_VICT)) {
      return DELETE_THIS;
    }
    return FALSE;
  } else if (roomp->isRoomFlag(ROOM_INDOORS))
    sendTo("You are already indoors.\n\r");
  else {
   /* try to locate an entrance */
    dirTypeT door;
    for (door = MIN_DIR; door < MAX_DIR; door++) {
      if (canGo(door) && real_roomp(exitDir(door)->to_room)->isRoomFlag(ROOM_INDOORS)) {
        rc = doMove(door);
        if (IS_SET_DELETE(rc, DELETE_THIS))
          return DELETE_THIS;
        return FALSE;
      }
    }
    sendTo("You can't seem to find anything to enter.\n\r");
  }
  return FALSE;
}

// returns DELETE_THIS
// FLASE: no portal found, true otherwise
int TBeing::portalLeaveCheck(char *argum, cmdTypeT cmd)
{
  TThing *t;
  TPortal *o = NULL;
  char arg[80];

  one_argument(argum, arg);
  for (t = roomp->getStuff(); t; t = t->nextThing) {
    o = dynamic_cast<TPortal *>(t);
    if (o &&
           (((cmd == CMD_LEAVE) && (!arg || !*arg || isname(arg,o->name))) || 
            ((cmd == CMD_EXITS) && *arg && isname(arg, o->name))) ) {
      break;
    }
  }
  if (!o)
    return FALSE;

  int rc = o->enterMe(this);
  if (IS_SET_DELETE(rc, DELETE_THIS)) {
    delete o;
    o = NULL;
  }
  if (IS_SET_DELETE(rc, DELETE_VICT)) {
    return DELETE_THIS;
  }

  return TRUE;
}

// returns DELETE_THIS
int TBeing::doLeave(const char *argument)
{
  int rc;
  roomDirData *exitp;
  TRoom *rp;
  char arg[80];

  *arg = '\0';
  one_argument(argument, arg);
  if (arg || *arg) {
    rc = portalLeaveCheck(arg, CMD_LEAVE);
    if (IS_SET_DELETE(rc, DELETE_THIS))
      return DELETE_THIS;
    if (rc)
      return FALSE;
  }

  if (!roomp->isRoomFlag(ROOM_INDOORS))
    sendTo("You are outside.. where do you want to go?\n\r");
  else {
    dirTypeT door;
    for (door = MIN_DIR; door < MAX_DIR; door++) {
      if (exit_ok(exitp = exitDir(door), &rp) &&
          !IS_SET(exitp->condition, EX_CLOSED) &&
          !rp->isRoomFlag(ROOM_INDOORS)) {
        rc = doMove(door);
        if (IS_SET_DELETE(rc, DELETE_THIS))
          return DELETE_THIS;
        return FALSE;
      }
    }
    rc = portalLeaveCheck(arg, CMD_LEAVE);
    if (IS_SET_DELETE(rc, DELETE_THIS))
      return DELETE_THIS;
    if (rc)
      return FALSE;

    sendTo("You see no obvious exits to the outside.\n\r");
  }
  return FALSE;
}


void TBeing::doStand()
{
  if (isFlying()) {
    sendTo("Why don't we land first...\n\r");
    return;
  }
  if (bothLegsHurt()) {
    sendTo("You can't stand up.  You have no working legs.\n\r");
    return;
  }

  switch (getPosition()) {
    case POSITION_STANDING:
      act("You are already standing.", FALSE, this, 0, 0, TO_CHAR);
      break;
    case POSITION_SITTING:
    case POSITION_CRAWLING:
      if (removeAllCasinoGames())
        return;

      act("You stand up.", FALSE, this, 0, 0, TO_CHAR);
      act("$n clambers to $s feet.", TRUE, this, 0, 0, TO_ROOM);
      if (riding)
        dismount(POSITION_STANDING);
      setPosition(POSITION_STANDING);
      break;
    case POSITION_RESTING:
      act("You stop resting and stand up.", FALSE, this, 0, 0, TO_CHAR);
      act("$n stops resting and clambers to $s feet.", TRUE, this, 0, 0, TO_ROOM);
      if (riding)
        dismount(POSITION_STANDING);
      setPosition(POSITION_STANDING);
      break;
    case POSITION_SLEEPING:
      act("You have to wake up first!", FALSE, this, 0, 0, TO_CHAR);
      break;
    case POSITION_MOUNTED:
      sendTo("Not while riding you don't!\n\r");
      break;
    default:
      act("You stop floating around and put your feet on the $g.", FALSE, this, 0, 0, TO_CHAR);
      act("$n stops floating around and puts $s feet on the $g.", TRUE, this, 0, 0, TO_ROOM);
      break;
  }
}

void TThing::sitMe(TBeing *ch)
{
  ch->sendTo("You can't sit in that.\n\r");
  return;
}

static bool sitCasinoEnter(const TBeing *ch)
{
  if (gGin.check(ch)) {
    if (!gGin.enter(ch))
      return true;
  }
  if (ch->checkBlackjack(true)) {
    if (!gBj.enter(ch))
      return true;
  }
  if (ch->checkHoldem(true)) {
    if (!gHoldem.enter(ch))
      return true;
  }
  if (ch->checkHiLo(true)) {
    if (!gHiLo.enter(ch))
      return true;
  }
  if (ch->checkPoker(true)) {
    if (!gPoker.enter(ch))
      return true;
  }
  if (ch->checkHearts(true)) {
    if (!gHearts.enter(ch))
      return true;
  }
  if (ch->checkCrazyEights(true)) {
    if (!gEights.enter(ch))
      return true;
  }
  if (ch->checkDrawPoker(true)) {
    if (!gDrawPoker.enter(ch))
      return true;
  }
  if (ch->checkBaccarat(true)){
    if (!gBaccarat.enter(ch))
      return true;
  }
  if (ch->checkSlots()) {
    if (ch->checkSlotPlayer()) {
      ch->sendTo("Someone is already at this machine.\n\r");
      return true;
    }
  }

  return false;
}

void TBeing::doSit(const sstring & argument)
{
  if (isFlying()) {
    sendTo("Why don't we land first...\n\r");
    return;
  }

  if (roomp->isUnderwaterSector()) {
    sendTo("You can't sit down in the water.\n\r");
    return;
  }

  if (fight()) {
    sendTo("You can't sit and fight at the same time!!\n\r");
    return;
  }
  if (isCombatMode(ATTACK_BERSERK)) {
    sendTo("Your berserker rage prevents you from resting at this time.\n\r");
    return;
  }
#if 0
  int rc = triggerSpecial(this, CMD_OBJ_SATON, arg);
  if (IS_SET_DELETE(rc, DELETE_THIS)) 
    return DELETE_THIS;

  if (rc)
    return FALSE;
#endif

  sstring arg = argument;
  trimString(arg);

  if (arg.empty() && (!riding || dynamic_cast<TBeing *>(riding))) {
    loseSneak();
    switch (getPosition()) {
      case POSITION_STANDING:
      case POSITION_CRAWLING:
        if (sitCasinoEnter(this))
          return;
        act("You sit down.", FALSE, this, 0, 0, TO_CHAR);
        act("$n sits down.", TRUE, this, 0, 0, TO_ROOM);
        setPosition(POSITION_SITTING);
        if (isPc())
          start_task(this, 0, 0, TASK_SIT, "", 350, 0, 1, 0, 4 * regenTime());
        break;
      case POSITION_SITTING:
        sendTo("You're sitting already.\n\r");
        break;
      case POSITION_RESTING:
        if (sitCasinoEnter(this))
          return;

        act("You stop resting, and sit up.", FALSE, this, 0, 0, TO_CHAR);
        act("$n stops resting.", TRUE, this, 0, 0, TO_ROOM);
        setPosition(POSITION_SITTING);
        if (isPc())
          start_task(this, 0, 0, TASK_SIT, "", 350, 0, 1, 0, 4 * regenTime());
        break;
      case POSITION_SLEEPING:
        act("You have to wake up first.", FALSE, this, 0, 0, TO_CHAR);
        break;
      case POSITION_MOUNTED:
        sendTo("Not while riding you don't!\n\r");
        break;
      default:
        if (sitCasinoEnter(this))
          return;

        act("You stop floating around, and sit down.", FALSE, this, 0, 0, TO_CHAR);
        act("$n stops floating around, and sits down.", TRUE, this, 0, 0, TO_ROOM);
        setPosition(POSITION_SITTING);
        if (isPc())
          start_task(this, 0, 0, TASK_SIT, "", 350, 0, 1, 0, 4 * regenTime());
        break;
    }
  } else {
    sstring str;

    if (sstringncmp(arg, "on ", 3) == 0)
      str = string(arg, 3);
    else if (sstringncmp(arg, "in ", 3) == 0)
      str = string(arg, 3);
    else
      str = arg;

    TThing *t;
    if (!riding) {
      TBeing *dummy;
      TObj *obj;
      if(!generic_find(str.c_str(), FIND_OBJ_ROOM, this, &dummy, &obj) ||
	 !obj){
        sendTo("You don't see that here.\n\r");
        return;
      }
      t = obj;
    } else 
      t = riding;
    
    if (sitCasinoEnter(this))
      return;

    t->sitMe(this);
  }
}

void TThing::restMe(TBeing *ch)
{
  ch->sendTo("You can't rest in that.\n\r");
  return;
}

void TBeing::doRest(const sstring & argument)
{
  if (isFlying()) {
    sendTo("Why don't we land first...\n\r");
    return;
  }
  if (!hasBoat() && roomp->isWaterSector() || roomp->isUnderwaterSector()) {
    sendTo("You can't rest in the water.\n\r");
    return;
  }
  if (fight()) {
    sendTo("You can't rest and fight at the same time!!\n\r");
    return;
  }
  if (isCombatMode(ATTACK_BERSERK)) {
    sendTo("Your berserker rage prevents you from resting at this time.\n\r");
    return;
  }
  sstring arg = argument;
  trimString(arg);

  if (arg.empty() && (!riding || dynamic_cast<TBeing *>(riding))) {
    loseSneak();
    switch (getPosition()) {
      case POSITION_STANDING:
      case POSITION_CRAWLING:
        act("You sit down, lean back and rest your tired bones.", FALSE, this, 0, 0, TO_CHAR);
        act("$n sits down, leans back and rests.", TRUE, this, 0, 0, TO_ROOM);
        setPosition(POSITION_RESTING);
        if (isPc())
          start_task(this, 0, 0, TASK_REST, "", 350, 0, 1, 0, 2 * regenTime());
        break;
      case POSITION_SITTING:
        if (removeAllCasinoGames())
          break;

        act("You lean back and rest your tired bones.", FALSE, this, 0, 0, TO_CHAR);
        act("$n leans back and rests.", TRUE, this, 0, 0, TO_ROOM);
        setPosition(POSITION_RESTING);
        if (isPc())
          start_task(this, 0, 0, TASK_REST, "", 350, 0, 1, 0, 2 * regenTime());
        break;
      case POSITION_RESTING:
        act("You are already resting.", FALSE, this, 0, 0, TO_CHAR);
        break;
      case POSITION_SLEEPING:
        act("You have to wake up first.", FALSE, this, 0, 0, TO_CHAR);
        break;
      case POSITION_MOUNTED:
        sendTo("Not while riding you don't!\n\r");
        break;
      default:
        act("You stop floating around, and stop to rest your tired bones.",FALSE, this, 0, 0, TO_CHAR);
        act("$n stops floating around, and rests.", FALSE, this, 0, 0, TO_ROOM);
        setPosition(POSITION_SITTING);
        if (isPc())
          start_task(this, 0, 0, TASK_REST, "", 350, 0, 1, 0, 2 * regenTime());
        break;
    }
  } else {
    sstring str;

    if (sstringncmp(arg, "on ", 3) == 0)
      str = string(arg, 3);
    else if (sstringncmp(arg, "in ", 3) == 0)
      str = string(arg, 3);
    else
      str = arg;

    TThing *t;
    if (!riding) {
      TBeing *dummy = NULL;
      TObj *obj = NULL;
      if(!generic_find(str.c_str(), FIND_OBJ_ROOM, this, &dummy, &obj) ||
	 !obj) {
        sendTo("You don't see that here.\n\r");
        return;
      }
      t = obj;
    } else {
      t = riding;
    }
    if (t)
      t->restMe(this);
    else
      sendTo("You can't rest on that.\n\r");
  }
}

void TThing::sleepMe(TBeing *ch)
{
  ch->sendTo("You can't go to sleep in that.\n\r");
  return;
}

void TBeing::doSleep(const sstring & argument)
{
  TThing *t = NULL;

  if (isFlying()) {
    sendTo("Why don't we land first...\n\r");
    return;
  }
  if ((roomp->isWaterSector() || roomp->isUnderwaterSector()) && !isAquatic()) {
    sendTo("You can't sleep in the water.\n\r");
    return;
  }
  if (fight()) {
    sendTo("You can't sleep and fight at the same time!!\n\r");
    return;
  }
  if (isCombatMode(ATTACK_BERSERK)) {
    sendTo("Your berserker rage prevents you from resting at this time.\n\r");
    return;
  }
  sstring arg = argument;
  trimString(arg);

  if (arg.empty() && (!riding || dynamic_cast<TBeing *>(riding))) {
    loseSneak();
    switch (getPosition()) {
      case POSITION_STANDING:
      case POSITION_RESTING:
        sendTo("You go to sleep.\n\r");
        act("$n lies down and falls asleep.", TRUE, this, 0, 0, TO_ROOM);
        setPosition(POSITION_SLEEPING);
        if (isPc())
          start_task(this, 0, 0, TASK_SLEEP, "", 350, 0, 1, 0, regenTime());
        break;
      case POSITION_CRAWLING:
      case POSITION_SITTING:
        sendTo("You go to sleep.\n\r");
        act("$n lies down and falls asleep.", TRUE, this, 0, 0, TO_ROOM);
        setPosition(POSITION_SLEEPING);
        if (isPc())
          start_task(this, 0, 0, TASK_SLEEP, "", 350, 0, 1, 0, regenTime());
        if (removeAllCasinoGames())
          break;
        break;
      case POSITION_SLEEPING:
        sendTo("You are already sound asleep.\n\r");
        break;
      case POSITION_MOUNTED:
        sendTo("Not while riding you don't!\n\r");
        break;
      default:
        act("You stop floating around, and lie down to sleep.", FALSE, this, 0, 0, TO_CHAR);
        act("$n stops floating around, and lie down to sleep.", TRUE, this, 0, 0, TO_ROOM);
        setPosition(POSITION_SLEEPING);
        if (isPc())
          start_task(this, 0, 0, TASK_SLEEP, "", 350, 0, 1, 0, regenTime());
        break;
    }
  } else {
    sstring str;

    if (sstringncmp(arg, "on ", 3) == 0)
      str = string(arg, 3);
    else if (sstringncmp(arg, "in ", 3) == 0)
      str = string(arg, 3);
    else
      str = arg;

    if (!riding) {
      TBeing *dummy;
      TObj *obj = NULL;
      if(!generic_find(str.c_str(), FIND_OBJ_ROOM, this, &dummy, &obj) ||
	 !obj) {
        sendTo("You don't see that here.\n\r");
        return;
      }
      t = obj;
    } else
      t = riding;

    t->sleepMe(this);
  }
}

void TBeing::doWake(const char *argument)
{
  TBeing *tmp_char;
  char arg[MAX_INPUT_LENGTH];

  if (isFlying()) {
    sendTo("Why don't we land first...\n\r");
    return;
  }
  one_argument(argument, arg);

  if (*arg) {
    if (getPosition() == POSITION_SLEEPING)
      act("You can't wake people up if you are asleep yourself!", 
           FALSE, this, 0, 0, TO_CHAR);
    else {
      tmp_char = get_char_room_vis(this, arg);
      if (tmp_char) {
        if (tmp_char == this)
          act("If you want to wake yourself up, just type 'wake'", 
              FALSE, this, 0, 0, TO_CHAR);
        else {
          if (tmp_char->getPosition() == POSITION_SLEEPING) {
            if (tmp_char->isAffected(AFF_SLEEP))
              act("You can not wake $M up!", FALSE, this, 0, tmp_char, TO_CHAR);
            else {
              if (tmp_char->checkBlackjack()) {
                // setting sitting in BJ room causes problems....

                removeAllCasinoGames();
                act("You wake $M up and drag $m to $M feet.", FALSE, this, 0,
                    tmp_char, TO_CHAR);
                tmp_char->setPosition(POSITION_STANDING);
                act("You are awakened and drug to your feet by $n.", TRUE,
                    this, 0, tmp_char, TO_VICT);
              } else {
                act("You wake $M up.", FALSE, this, 0, tmp_char, TO_CHAR);
                tmp_char->setPosition(POSITION_SITTING);
                act("You are awakened by $n.", TRUE, this,0, tmp_char, TO_VICT);
                tmp_char->stopsound();
              }
            }
          } else
            act("$N is already awake.", FALSE, this, 0, tmp_char, TO_CHAR);
        }
      } else
        sendTo("You do not see that person here.\n\r");
    }
  } else {
    if (isAffected(AFF_SLEEP))
      sendTo("You can't wake up!\n\r");
    else {
      if (getPosition() > POSITION_SLEEPING)
        sendTo("You are already awake...\n\r");
      else {
        if (checkBlackjack()) {
          // to be set sitting in BJ room would be bad

          removeAllCasinoGames();
          sendTo("You wake and decide to stand to see the games.\n\r");
          act("$n awakens and clambers to $s feet.", TRUE, this, 0, 0, TO_ROOM);
          setPosition(POSITION_STANDING);
        } else if (riding) {
          sendTo("You wake, and stand up.\n\r");
          act("You wake, climb out of the $o and stand up.",
                      TRUE,this,riding,0,TO_CHAR);
          act("$n awakens, climbs out of the $o and stands up.", 
                      TRUE, this, riding, 0, TO_ROOM);
          dismount(POSITION_STANDING);
          setPosition(POSITION_STANDING);
        } else {
          sendTo("You wake, and stand up.\n\r");
          act("$n awakens and stands up.", TRUE, this, 0, 0, TO_ROOM);
          setPosition(POSITION_STANDING);
        }
        stopsound();
      }
    }
  }
}

void TBeing::doFollow(const char *argument)
{
  char caName[160];
  TBeing *leader;

  strcpy(caName, argument);

  if (rider)
    return;

  if (*caName) {
    if (!strcmp(caName,"me") || !strcmp(caName, "self")) {
      leader = this;
    } else if (!(leader = get_best_char_room(this, caName))) {
      sendTo("I see no person by that name here!\n\r");
      return;
    }
  } else {
    sendTo("Whom do you wish to follow?\n\r");
    return;
  }

  if ((this != leader) && !isImmortal()){
    if (isPlayerAction(PLR_SOLOQUEST)) {
      sendTo("You can't follow with your solo quest flag set.\n\r");
      return;
    }
    if (leader->isPlayerAction(PLR_SOLOQUEST)) {
      sendTo("That player has a quest flag set, you can't follow!\n\r");
      return;
    }
    if (leader->isPlayerAction(PLR_GRPQUEST) && !isPlayerAction(PLR_GRPQUEST)) {
      sendTo(fmt("COLOR_MOBS, %s has a group quest flag. You can't follow without one yourself.\n\r") % leader->getName());
      return;
    }
    if (isPlayerAction(PLR_GRPQUEST) && !leader->isPlayerAction(PLR_GRPQUEST)) {
      sendTo("You can't follow someone without a group quest flag, while you have that flag.\n\r");
      return;
    }
  }
  if (isAffected(AFF_CHARM) && master) 
    act("But you only feel like following $N!", FALSE, this, 0, master, TO_CHAR);
  else {
    if (leader == this) {
      if (!master) {
        sendTo("You are already following yourself.\n\r");
        return;
      }
      stopFollower(TRUE);
    } else {
      if (circleFollow(leader)) {
        act("Sorry, but following in 'loops' is not allowed.", FALSE, this, 0, 0, TO_CHAR);
        return;
      }
      if (master)
        stopFollower(TRUE);
      if (isAffected(AFF_GROUP))
        REMOVE_BIT(specials.affectedBy, AFF_GROUP);
      leader->addFollower(this);
    }
  }
}

void TBeing::setPosition(positionTypeT pos)
{
  // Take care of possible riders.
  if (rider && pos < POSITION_RESTING) {
    act("Your mounts change in position forces you off.",
        FALSE, rider, NULL, this, TO_CHAR);
    act("$n is forced off $N.",
        FALSE, rider, NULL, this, TO_ROOM);

    rider->dismount((!::number(0, 9) ? POSITION_STANDING : POSITION_SITTING));
  }

  if (dynamic_cast<TBeing *>(riding) && 
      (pos != POSITION_FIGHTING) && (pos != POSITION_MOUNTED)) {
    // for debug
    vlogf(LOG_BUG, fmt("Mounted person (%s) set to new position (%s:%d).") %  getName() % position_types[pos] % pos);
    dismount(POSITION_STANDING);
  }
  if (!hasLegs()) {
    if ((pos == POSITION_SLEEPING) ||
        (pos == POSITION_RESTING) ||
        (pos == POSITION_SITTING) ||
        (pos == POSITION_CRAWLING))
      return;
  }
  if (task)
    stopTask();

  if (specials.position == POSITION_SLEEPING && pos != POSITION_SLEEPING) {
    playsound(SOUND_SNORE, SOUND_TYPE_SOCIAL, 100, 50, -1);
  }
  specials.position = pos;
  if (pos == POSITION_SLEEPING) {
    playsound(SOUND_SNORE, SOUND_TYPE_SOCIAL, 100, 50, -1);
  } 
}

void TBeing::doCrawl()
{

  if (isFlying()) {
    sendTo("Why don't we land first...\n\r");
    return;
  }
  if (riding || rider) {
    sendTo("You'd have to dismount before crawling.\n\r");
    return;
  }
  if (isFlying()) {
    sendTo("Sorry, you can't crawl and fly at the same time.\n\r");
    return;
  }
#if 0
  if (roomp->isWaterSector()) {
    sendTo("You can't crawl on the water.\n\r");
    return;
  }
  if (roomp->isUnderwaterSector()) {
    sendTo("You can't crawl underwater.\n\r");
    return;
  }
#endif
  if (roomp->isVertSector()) {
    sendTo("You can't crawl while climbing.\n\r");
    return;
  }
  if (roomp->isAirSector()) {
    sendTo("You can't crawl while falling.\n\r");
    return;
  }
#if 0
  // we have to allow this.  wimpy sets them standing sometimes
  if (fight()) {
    sendTo("You can't crawl and fight at the same time!!\n\r");
    return;
  }
#endif
  if (isCombatMode(ATTACK_BERSERK)) {
    sendTo("Your berserker rage prevents you from crawling at this time.\n\r");
    return;
  }
  if (riding)
    doStand();

  if (!riding) {
    switch (getPosition()) {
      case POSITION_CRAWLING:
        sendTo("You are already crawling.\n\r");
        break;
      case POSITION_STANDING:
      case POSITION_RESTING:
      case POSITION_SITTING:
        sendTo("You start to crawl.\n\r");
        act("$n gets down on $s knees and starts to crawl.", 
                TRUE, this, 0, 0, TO_ROOM);
        setPosition(POSITION_CRAWLING);
        removeAllCasinoGames();
        break;
      case POSITION_SLEEPING:
        sendTo("You are already sound asleep.\n\r");
        break;
      case POSITION_MOUNTED:
        sendTo("Not while riding you don't!\n\r");
        break;
      default:
        act("You stop floating around, and lie down to sleep.", 
           FALSE, this, 0, 0, TO_CHAR);
        act("$n stops floating around, and lie down to sleep.", 
           TRUE, this, 0, 0, TO_ROOM);
        setPosition(POSITION_CRAWLING);
        break;
    }
  }
}

// returns DELETE_THIS
// returns FALSE if can not go that way
int TBeing::goDirection(dirTypeT dir)
{
  int rc = 0;
  TRoom *rp;
  roomDirData *exitp;
  char buf[256];
  TThing *t, *t2;

  if (fight())
    return FALSE;

  if (isAffected(AFF_SHOCKED)) {
    sendTo("You are still recovering from something and cannot bring yourself to move.\n\r");
    return FALSE;
  }
 
  if (dir < MAX_DIR) {
    if (!(exitp = exitDir(dir)))
      return FALSE;

    if (!IS_SET(exitp->condition, EX_CLOSED)) {
      if (!(rp = real_roomp(exitp->to_room))) {
        return FALSE;
      }
      if (rp->isRoomFlag(ROOM_INDOORS)) {
        if (riding)
          doMount("", CMD_DISMOUNT, NULL);
      }
      if (!isPc() && !isFlying() && canFly()) {
        // on ground, but could fly, see if we need to
        if (rp->isWaterSector() || rp->isVertSector() || rp->isAirSector()) {
          doFly();
        }
      }
      if (getPosition() <= POSITION_CRAWLING)
        doStand();
 
      rc = doMove(dir);
      if (IS_SET_DELETE(rc, DELETE_THIS))
        return DELETE_THIS;
      return TRUE;
    } else if (isHumanoid() && !IS_SET(exitp->condition, EX_SECRET) &&
        !IS_SET(exitp->condition, EX_LOCKED)) {

      // Manipulation Examples:
      //    0 Ride: 1.50
      //   10 Ride: 1.45
      //   50 Ride: 1.25
      //  100 Ride: 1.00
      float tRidingManip = ((getPosition() == POSITION_MOUNTED) ?
                            (1.5 - (((float)getSkillValue(SKILL_RIDE) / 2) / 100)) : 1.0);
      if ((exitp->weight * tRidingManip) > maxWieldWeight(NULL, HAND_TYPE_PRIM)) {
        sendTo(fmt("The %s is too large and heavy for you to budge it.\n\r") %
           exitp->getName());
        sprintf(buf, "$n throws $mself at a %s, but $e can't budge it.",
           exitp->getName().c_str());
        act(buf, TRUE, this, 0, 0, TO_ROOM);
        return FALSE;
      }
      rawOpenDoor(dir);
      return 0;
    } else if (!IS_SET(exitp->condition, EX_SECRET) &&
               IS_SET(exitp->condition, EX_LOCKED)) {
      int origroom = in_room;
      if (doesKnowSkill(SKILL_DOORBASH))
        rc = doDoorbash(fname(exitp->keyword));
  
      if (IS_SET_DELETE(rc, DELETE_THIS))
        return DELETE_THIS;
    
      // doorbash might have taken them to new room
      return (origroom != in_room);
    } else if (IS_SET(exitp->condition, EX_SECRET)) {
      // a secret door bars the way
      // morts abuse this by hiding on one side of secret, popping door,
      // shoot an arrow, close the door
      // in general, we don't want to reveal secrets to morts though...
      if (real_roomp(exitp->to_room) && real_roomp(exitp->to_room)->roomIsEmpty(true)) {
        rawOpenDoor(dir);
        return 0;
      }
    }
  } else {
    // portals will never be closed, see logic in find_path

    dirTypeT count = dirTypeT(dir-MAX_DIR+1);
    int seen = 0;
    for (t = roomp->getStuff(); t; t = t2) {
      t2 = t->nextThing;
      TPortal *tp = dynamic_cast<TPortal *>(t);
      if (tp) {
        seen++;
        if (count == seen) {
          rc = doEnter(NULL, tp);
          if (IS_SET_DELETE(rc, DELETE_ITEM)) {
            delete tp;
            tp = NULL;
          }
          if (IS_SET_DELETE(rc, DELETE_THIS))
            return DELETE_THIS;
          return TRUE;
        }
      }
    }
    if (!t) {
      sendTo("Error finding path target!  Tell a god.\n\r");
      vlogf(LOG_BUG, "Error finding path (goDirection)");
      return FALSE;
    }
  }
  return 0;
}
 
bool TThing::canGoHuman(dirTypeT door) const
{
  return (exitDir(door) && real_roomp(exitDir(door)->to_room) &&
            !(exitDir(door)->condition & EX_LOCKED));
}

bool TThing::canGo(dirTypeT door) const
{
  return (exitDir(door) && real_roomp(exitDir(door)->to_room) &&
            !(exitDir(door)->condition & EX_CLOSED));
}

void TBeing::doFly()
{
  if (isFlying()) {
    sendTo("You are already flying.\n\r");
    return;
  }
  if (!canFly()) {
    sendTo("You flap and flap, but seem to be less than flightworthy.\n\r");
    return;
  }
  if (riding) {
    sendTo("You can't fly while riding something else.\n\r");
    return;
  }
  if (roomp && roomp->isUnderwaterSector()) {
    sendTo("It hurts your brain too much even contemplating how to fly underwater?!?\n\r");
    return;
  }
  act("You take to the air and start flying about.", TRUE, this, 0, 0, TO_CHAR);
  act("$n takes to the air and starts flying about.", TRUE, this, 0, 0, TO_ROOM);

  setPosition(POSITION_FLYING);
  return;
}

void TBeing::doLand()
{
 roomDirData *exitp = NULL;

  if (!isFlying()) {
    sendTo("It seems you have already landed.\n\r");
    return;
  }
#if 0
  if (roomp->isAirSector()) {
    sendTo("You have to descend before you can land.\n\r");
    return;
  }
#endif
  if (roomp->isFlyingSector()) {
    sendTo("The magic in the air prevents you from landing.\n\r");
    return;
  }
  if ((roomp->isAirSector() || roomp->isVertSector()) &&
      (exitp = roomp->exitDir(DIR_DOWN))) {
    if (exit_ok(exitp, NULL)) {
      sendTo("You have to descend before you can land.\n\r");
      return;
    }
  }

  act("You come in for a landing and stop flying.", TRUE, this, 0, 0, TO_CHAR);
  act("$n comes in for a landing and stops flying.", TRUE, this, 0, 0, TO_ROOM);
  
  setPosition(POSITION_STANDING);
  return;
}

int TBeing::crashLanding(positionTypeT pos, bool force, bool dam)
{
  int rc = FALSE;

  if (force) {
// option to force this
    setPosition(pos);
    sendTo(fmt("You smash into the %s hard!\n\r") % roomp->describeGround());
    act("$n tumbles end over end as $e crash lands!",
        TRUE, this, 0, 0, TO_ROOM);
  } else if (!isFlying()) {
    setPosition(pos);
    if ((getPosition() >= POSITION_RESTING) && 
          ((pos == POSITION_FLYING) || roomp->isFlyingSector())) {
      sendTo("You start flying around.\n\r");
      act("$n starts to fly up in the air.",
           TRUE, this, 0, 0, TO_ROOM);
      setPosition(POSITION_FLYING);
      return FALSE;
    } else if (roomp->isFallSector()) {
      rc = checkFalling();
      if (IS_SET_DELETE(rc, DELETE_THIS))
        return DELETE_THIS;
      return TRUE;
    }
    return FALSE;
  } else if (roomp->isFlyingSector()) {
    if (pos == POSITION_FLYING) {
      return FALSE;
    } else if (getPosition() >= POSITION_RESTING) {
      sendTo("You start flying around.\n\r");
      act("$n starts to fly up in the air.",
           TRUE, this, 0, 0, TO_ROOM);
      setPosition(POSITION_FLYING);
      return FALSE;
    } else {
      setPosition(pos);
      return FALSE;
    }
  } else if (roomp->isFallSector()) {
    setPosition(pos);
    rc = checkFalling();
    if (IS_SET_DELETE(rc, DELETE_THIS))
      return DELETE_THIS;
    return TRUE;
  } else if (doesKnowSkill(SKILL_CATFALL) && bSuccess(SKILL_CATFALL)){
    setPosition(POSITION_STANDING);
    act("$n drops gracefully onto the $g.", FALSE, this, 0, 0, TO_ROOM);
    sendTo(fmt("You drop gracefully to the %s.\n\r") % roomp->describeGround());
    dam=false;
  } else {
// Flying person
    setPosition(pos);
    sendTo(fmt("You smash into the %s hard!\n\r") % roomp->describeGround());
    act("$n tumbles end over end as $e crash lands!",
        TRUE, this, 0, 0, TO_ROOM);
  }

  if (dam) {
    // some things skip damage.  (e.g. already dead when crashed)
    if (reconcileDamage(this, ::number(2,8), DAMAGE_FALL) == -1)
      return DELETE_THIS;
  }

  return TRUE;
}

int TBeing::doMortalGoto(const sstring & argument)
{
  int targ_rm = 0;
  int targ_ch = 0;
  dirTypeT dir;
  TRoom *rp;
  TBeing *ch;
  sstring arg;

  one_argument(argument, arg);

  if (!inGrimhaven()) {
    sendTo("You aren't familiar with this area and have no idea how to get there.\n\r");
    return FALSE;
  }
  if (getPosition() < POSITION_STANDING) {
    sendTo("You need to be standing in order to get your bearings.\n\r");
    return FALSE;
  }
  if (fight()) {
    sendTo("The ensuing battle makes it difficult to get your bearings.\n\r");
    return FALSE;
  }
  if (roomp && 
      (roomp->getLight() + visionBonus <= 0) &&
      !roomp->isRoomFlag(ROOM_ALWAYS_LIT) &&
      !isAffected(AFF_TRUE_SIGHT)) {
    sendTo("You can't see well enough to get your bearings.\n\r");
    return FALSE;
  }
  if (is_abbrev(arg, "trainer")) {
    if (hasClass(CLASS_WARRIOR)) {
      arg = "warrior_trainer";
    } else if (hasClass(CLASS_MAGE)) {
      arg = "mage_trainer";
    } else if (hasClass(CLASS_CLERIC)) {
      arg = "cleric_trainer";
    } else if (hasClass(CLASS_RANGER)) {
      arg = "ranger_trainer";
    } else if (hasClass(CLASS_THIEF)) {
      arg = "thief_trainer";
    } else if (hasClass(CLASS_DEIKHAN)) {
      arg = "deikhan_trainer";
    } else if (hasClass(CLASS_MONK)) {
      arg = "monk_trainer";
    } else if (hasClass(CLASS_SHAMAN)) {
      arg = "shaman_trainer";
    } 
  }
  if (is_abbrev(arg, "guildmaster")) {
    if (hasClass(CLASS_WARRIOR)) {
      arg = "warrior";
    } else if (hasClass(CLASS_MAGE)) {
      arg = "mage";
    } else if (hasClass(CLASS_CLERIC)) {
      arg = "cleric";
    } else if (hasClass(CLASS_RANGER)) {
      arg = "ranger";
    } else if (hasClass(CLASS_THIEF)) {
      arg = "thief";
    } else if (hasClass(CLASS_DEIKHAN)) {
      arg = "deikhan";
    } else if (hasClass(CLASS_MONK)) {
      arg = "monk";
    } else if (hasClass(CLASS_SHAMAN)) {
      arg = "shaman";
    }
  }

  if (is_abbrev(arg, "cs") || is_abbrev(arg, "center")) {
    targ_rm = ROOM_CS;
  } else if (is_abbrev(arg, "mail") || is_abbrev(arg, "postoffice")) {
    targ_rm = 406;
  } else if (is_abbrev(arg, "sharpener") || is_abbrev(arg, "beavis")) {
    targ_rm = 408;
  } else if (is_abbrev(arg, "repair") || is_abbrev(arg, "piggot")) {
    targ_rm = 409;
  } else if (is_abbrev(arg, "bank")) {
    targ_rm = 410;
  } else if (is_abbrev(arg, "doctor") || is_abbrev(arg, "hospital")) {
    targ_rm = 419;
  } else if (is_abbrev(arg, "scribe") || is_abbrev(arg, "scrolls")) {
    targ_rm = 501;
  } else if (is_abbrev(arg, "alchemist") || is_abbrev(arg, "potions")) {
    targ_rm = 503;
  } else if (is_abbrev(arg, "light") || is_abbrev(arg, "lamp")) {
    targ_rm = 550;
  } else if (is_abbrev(arg, "symbols") || is_abbrev(arg, "pious")) {
    targ_rm = 551;
  } else if (is_abbrev(arg, "supply") || is_abbrev(arg, "taloc")) {
    targ_rm = 552;
  } else if (is_abbrev(arg, "drink")) {
    targ_rm = 553;
  } else if (is_abbrev(arg, "components") || is_abbrev(arg, "camron")) {
    targ_rm = 554;
  } else if (is_abbrev(arg, "armor")) {
    targ_rm = 555;
  } else if (is_abbrev(arg, "pub")) {
    targ_rm = 556;
  } else if (is_abbrev(arg, "innkeeper") || is_abbrev(arg, "rent") || is_abbrev(arg, "receptionist")) {
    targ_rm = 557;
  } else if (is_abbrev(arg, "commodity") || is_abbrev(arg, "prigon")) {
    targ_rm = 558;
  } else if (is_abbrev(arg, "weapon")) {
    targ_rm = 559;
  } else if (is_abbrev(arg, "food") || is_abbrev(arg, "froddors")) {
    targ_rm = 560;
  } else if (is_abbrev(arg, "magicitem") || is_abbrev(arg, "quazars")) {
    targ_rm = 561;
  } else if (is_abbrev(arg, "pawn") || is_abbrev(arg, "curio")) {
    targ_rm = 562;
  } else if (is_abbrev(arg, "petguy")) {
    targ_rm = 564;
  } else if (is_abbrev(arg, "board")) {
    targ_rm = 567;
  } else if (is_abbrev(arg, "tanner") || is_abbrev(arg, "clothing")) {
    targ_rm = 568;
  } else if (is_abbrev(arg, "monogramming") || is_abbrev(arg, "hornsby")) {
    targ_rm = 569;
  } else if (is_abbrev(arg, "donation") || is_abbrev(arg, "surplus")) {
    targ_rm = ROOM_DONATION;
  } else if (is_abbrev(arg, "dump")) {
    targ_rm = 600;
  } else if (is_abbrev(arg, "newbie") || is_abbrev(arg, "park")) {
    targ_rm = 25400;  // entrance to the park
  } else if (is_abbrev(arg, "attuner") || is_abbrev(arg, "karalina")) {
    targ_ch = 343;
  } else if (is_abbrev(arg, "mage")) {
    targ_ch = 200;
  } else if (is_abbrev(arg, "cleric")) {
    targ_ch = 201;
  } else if (is_abbrev(arg, "warrior") || is_abbrev(arg, "fighter")) {
    targ_ch = 202;
  } else if (is_abbrev(arg, "thief") || is_abbrev(arg, "thieving")) {
    targ_ch = 203;
  } else if (is_abbrev(arg, "ranger")) {
    targ_ch = 204;
  } else if (is_abbrev(arg, "deikhan")) {
    targ_ch = 205;
  } else if (is_abbrev(arg, "shaman")) {
    targ_ch = 206;
  } else if (is_abbrev(arg, "monk")) {
    targ_ch = 207;
  } else if (is_abbrev(arg, "lore_trainer") ||
             is_abbrev(arg, "lore-trainer")) {
    targ_ch = 333;
  } else if (is_abbrev(arg, "theology_trainer") || 
             is_abbrev(arg, "theology-trainer")) {
    targ_ch = 335;
  } else if (is_abbrev(arg, "mage_trainer") ||
             is_abbrev(arg, "mage-trainer")) {
    targ_ch = 502;
  } else if (is_abbrev(arg, "cleric_trainer") ||
             is_abbrev(arg, "cleric-trainer")) {
    targ_ch = 259;
  } else if (is_abbrev(arg, "thief_trainer") ||
             is_abbrev(arg, "thief-trainer")) {
    targ_ch = 262;
  } else if (is_abbrev(arg, "warrior_trainer") ||
             is_abbrev(arg, "warrior-trainer")) {
    targ_ch = 355;
  } else if (is_abbrev(arg, "shaman_trainer") ||
             is_abbrev(arg, "shaman-trainer")) {
    targ_ch = 247;
  } else if (is_abbrev(arg, "deikhan_trainer") ||
             is_abbrev(arg, "deikhan-trainer")) {
    targ_ch = 278;
  } else if (is_abbrev(arg, "ranger_trainer") ||
             is_abbrev(arg, "ranger-trainer")) {
    targ_ch = 261;
  } else if (is_abbrev(arg, "monk_trainer") ||
             is_abbrev(arg, "monk-trainer")) {
    targ_ch = 285;
  } else if (is_abbrev(arg, "adventuring_trainer") ||
             is_abbrev(arg, "adventuring-trainer")) {
    targ_ch = 290;
  } else if (is_abbrev(arg, "combat_trainer") ||
             is_abbrev(arg, "combat-trainer")) {
    targ_ch = MOB_COMBAT_TRAINER;
  } else if (is_abbrev(arg, "thief_entrance") ||
	     is_abbrev(arg, "thief-entrance")) {
    targ_rm = 634;
  } else if (is_abbrev(arg, "air_trainer") ||
             is_abbrev(arg, "air-trainer") ||
             is_abbrev(arg, "alchemy_trainer") ||
             is_abbrev(arg, "alchemy-trainer") ||
             is_abbrev(arg, "earth_trainer") ||
             is_abbrev(arg, "earth-trainer") ||
             is_abbrev(arg, "fire_trainer") ||
             is_abbrev(arg, "fire-trainer") ||
             is_abbrev(arg, "sorcery_trainer") ||
             is_abbrev(arg, "sorcery-trainer") ||
             is_abbrev(arg, "spirit_trainer") ||
             is_abbrev(arg, "spirit-trainer") ||
             is_abbrev(arg, "water_trainer") ||
             is_abbrev(arg, "water-trainer") ||
             is_abbrev(arg, "wrath_trainer") ||
             is_abbrev(arg, "afflictions_trainer") ||
             is_abbrev(arg, "cures_trainer") ||
             is_abbrev(arg, "hand_trainer") ||
             is_abbrev(arg, "looting_trainer") ||
             is_abbrev(arg, "murder_trainer") ||
             is_abbrev(arg, "dueling_trainer") ||
             is_abbrev(arg, "wizardry_trainer") ||
             is_abbrev(arg, "faith_trainer") ||
             is_abbrev(arg, "slash_trainer") ||
             is_abbrev(arg, "blunt_trainer") ||
             is_abbrev(arg, "pierce_trainer") ||
             is_abbrev(arg, "ranged_trainer") ||
             is_abbrev(arg, "meditation_trainer") ||
             is_abbrev(arg, "survival_trainer") ||
             is_abbrev(arg, "nature_trainer") ||
             is_abbrev(arg, "animal_trainer") ||
             is_abbrev(arg, "aegis_trainer") ||
             is_abbrev(arg, "plants_trainer") ||
             is_abbrev(arg, "soldiering_trainer") ||
             is_abbrev(arg, "blacksmithing_trainer") ||
             is_abbrev(arg, "fighting_trainer") ||
             is_abbrev(arg, "mounted_trainer") ||
             is_abbrev(arg, "leverage_trainer") ||
             is_abbrev(arg, "mindbody_trainer") ||
             is_abbrev(arg, "focused_trainer") ||
             is_abbrev(arg, "barehand_trainer") ||
             is_abbrev(arg, "poisons_trainer") ||
             is_abbrev(arg, "healing_trainer") ||
             is_abbrev(arg, "undead_trainer") ||
             is_abbrev(arg, "totemism_trainer") ||
             is_abbrev(arg, "stealth_trainer") ||
             is_abbrev(arg, "traps_trainer") ||
             is_abbrev(arg, "wrath-trainer") ||
             is_abbrev(arg, "afflictions-trainer") ||
             is_abbrev(arg, "cures-trainer") ||
             is_abbrev(arg, "hand-trainer") ||
             is_abbrev(arg, "looting-trainer") ||
             is_abbrev(arg, "murder-trainer") ||
             is_abbrev(arg, "dueling-trainer") ||
             is_abbrev(arg, "wizardry-trainer") ||
             is_abbrev(arg, "faith-trainer") ||
             is_abbrev(arg, "slash-trainer") ||
             is_abbrev(arg, "blunt-trainer") ||
             is_abbrev(arg, "pierce-trainer") ||
             is_abbrev(arg, "ranged-trainer") ||
             is_abbrev(arg, "meditation-trainer") ||
             is_abbrev(arg, "survival-trainer") ||
             is_abbrev(arg, "nature-trainer") ||
             is_abbrev(arg, "animal-trainer") ||
             is_abbrev(arg, "aegis-trainer") ||
             is_abbrev(arg, "plants-trainer") ||
             is_abbrev(arg, "soldiering-trainer") ||
             is_abbrev(arg, "blacksmithing-trainer") ||
             is_abbrev(arg, "fighting-trainer") ||
             is_abbrev(arg, "mounted-trainer") ||
             is_abbrev(arg, "leverage-trainer") ||
             is_abbrev(arg, "mindbody-trainer") ||
             is_abbrev(arg, "focused-trainer") ||
             is_abbrev(arg, "barehand-trainer") ||
             is_abbrev(arg, "poisons-trainer") ||
             is_abbrev(arg, "frog-trainer") ||
             is_abbrev(arg, "spider-trainer") ||
             is_abbrev(arg, "totemism-trainer") ||
             is_abbrev(arg, "stealth-trainer") ||
             is_abbrev(arg, "traps-trainer")) {
    sendTo("That discipline is fairly advanced.\n\r");
    sendTo("You will need to explore in order to locate the trainer for that discipline.\n\r");
    return FALSE;
  } else {
    sendTo(fmt("You can't seem to locate '%s'.\n\r") % arg);
    return FALSE;
  }

  if (targ_rm) {
    if (inRoom() == targ_rm) {
      sendTo("Uhm, not for nothing, but I think you are already there...\n\r");
      return FALSE;
    }
    rp = real_roomp(targ_rm);

    TPathFinder path;
    path.setNoMob(false);
    dir=path.findPath(in_room, findRoom(targ_rm));

    if (dir < DIR_NORTH || dir > DIR_SOUTHWEST) {
      sendTo("Strangely, you can't quite figure out how to get there from here.\n\r");
      return FALSE;
    }
    sendTo(COLOR_ROOMS, fmt("You can get to %s by going %s.\n\r") % rp->getName() % dirs[dir]); 
  } else {
    int rn = real_mobile(targ_ch);
    if (rn < 0) {
      vlogf(LOG_BUG, fmt("Error in goto for mob %s") %  arg);
      return FALSE;
    }
    ch = get_char_num(rn);
    if (!ch) {
      sendTo("You have no idea where they might be at present.\n\r");
      return FALSE;
    }
    targ_rm = ch->inRoom();

    if (inRoom() == targ_rm) {
      sendTo("Uhm, not for nothing, but I think you are already there...\n\r");
      return FALSE;
    }

    TPathFinder path;
    path.setNoMob(false);
    dir=path.findPath(in_room, findRoom(targ_rm));
    

    if (dir < DIR_NORTH || dir > DIR_SOUTHWEST) {
      // thieves guild is behind a secret door
      // if not with jennica track to jennica
      // if with jennica, tell them to open the door
      if(in_room == 634){
	sendTo("The entrance to the Thieves Guild is secret.\n\r");
	sendTo("You must carefully examine this room to find it the entrance.\n\r");
	sendTo("Once you have entered the Guild, you may use goto to continue.\n\r");
	return FALSE;
      } else if(targ_ch == 262 || targ_ch == 203){
	// this better not match the if above or we're in trouble
	return doMortalGoto("thief-entrance");
      }


      sendTo("Strangely, you can't quite figure out how to get there from here.\n\r");
      return FALSE;
    }
    sendTo(COLOR_MOBS, fmt("You can get to %s by going %s.\n\r") % ch->getName() % dirs[dir]); 
  }

  return FALSE;
}

int TObj::getNumRiders(TThing *ch) const
{
  TThing *t;
  int num = 0;

  for (t = rider; t; t = t->nextRider) {
    TBeing * tbt = dynamic_cast<TBeing *>(t);
    if (t == ch)
      continue;
    if (tbt && tbt->getPosition() == POSITION_SLEEPING)
      num += 3;
    else if (tbt && tbt->getPosition() == POSITION_RESTING)
      num += 2;
    else
      num++;
  }
  return num;
}

void TObj::closeMe(TBeing *ch)
{
  ch->sendTo("That's not a container.\n\r");
}

void TObj::lockMe(TBeing *ch)
{
  ch->sendTo("That's not a container.\n\r");
}

void TObj::unlockMe(TBeing *ch)
{
  ch->sendTo("That's not a container.\n\r");
}

int TObj::enterMe(TBeing *ch)
{
  ch->sendTo("You can't seem to find a way to enter that.\n\r");
  return FALSE;
}

bool TBeing::removeAllCasinoGames() const
{
  if (checkBlackjack())
    if (gBj.index(this) >= 0)
      gBj.exitGame(this);

  if (checkHoldem())
    if (gHoldem.index(this) >= 0)
      gHoldem.exitGame(this);

  if (checkHiLo())
    if (gHiLo.index(this) >= 0)
      gHiLo.exitGame(this);

  if (checkPoker())
    if (gPoker.index(this) >= 0)
      gPoker.exitGame(this);

  if (checkBaccarat())
    if (gBaccarat.index(this) >= 0)
      gBaccarat.exitGame(this);

  if (gGin.check(this))
    if (gGin.index(this) >= 0)
      if (!gGin.exitGame(this))
        return true;

  if (checkHearts()) {
    if (gHearts.index(this) >= 0)
      if (!gHearts.exitGame(this))
        return true;
  }

  if (checkCrazyEights()) {
    if (gEights.index(this) >= 0)
      if (!gEights.exitGame(this))
        return true;
  }

  if (checkDrawPoker()) {
    if (gDrawPoker.index(this) >= 0)
      if (!gDrawPoker.exitGame(this))
        return true;
  }

  return false;
}

const sstring roomDirData::getName() const
{
  return (keyword ? fname(keyword) : "door");
}

const sstring roomDirData::closed() const
{
  switch (door_type) {
    case DOOR_DRAWBRIDGE:
      return "raised";
    case DOOR_PORTCULLIS:
      return "lowered";
    case DOOR_PANEL:
    case DOOR_SCREEN:
    case DOOR_RUBBLE:
    case DOOR_GRATE:
    case DOOR_GATE:
    case DOOR_TRAPDOOR:	
    case DOOR_HATCH:
    case DOOR_DOOR:
    case DOOR_NONE:
    case MAX_DOOR_TYPES:
      break;
  }
  return "closed";
}
