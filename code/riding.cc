#include "stdsneezy.h"
#include "combat.h"
#include "obj_base_container.h"
#include "obj_table.h"
#include "obj_base_clothing.h"


// Peel
spellNumT TBeing::mountSkillType() const
{
  switch (getRace()) {
    case RACE_HORSE: case RACE_BOVINE: case RACE_OX:     case RACE_PIG:
    case RACE_SHEEP: case RACE_BAANTA: case RACE_CANINE: case RACE_GOAT:
      return SKILL_RIDE_DOMESTIC;
    case RACE_RHINO: case RACE_TIGER:
    case RACE_GIRAFFE:  case RACE_BEAR:  case RACE_BOAR:
    case RACE_ELEPHANT: case RACE_DEER:
      return SKILL_RIDE_NONDOMESTIC;
    case RACE_GRIFFON: case RACE_HIPPOGRIFF: case RACE_WYVERN: 
    case RACE_DRAGON:  case RACE_DRAGONNE:   case RACE_LAMMASU: 
    case RACE_SHEDU:   case RACE_SPHINX:
      return SKILL_RIDE_WINGED;
    case RACE_FELINE: case RACE_BASILISK: case RACE_CENTAUR: case RACE_CHIMERA:
    case RACE_FROG:   case RACE_LAMIA:    case RACE_MANTICORE: 
    case RACE_TURTLE: case RACE_LION: case RACE_LEOPARD: case RACE_COUGAR:
    case RACE_WYVELIN:
      return SKILL_RIDE_EXOTIC;
    default:
      return SKILL_RIDE_EXOTIC;
  }
}

bool TMonster::isDragonRideable() const
{
  switch (getRace()) {
      case RACE_MANTICORE:
      case RACE_GRIFFON:
      case RACE_SHEDU:
      case RACE_SPHINX:
      case RACE_LAMMASU:
      case RACE_DRAGONNE:
      case RACE_WYVERN:
      case RACE_HIPPOGRIFF:
      case RACE_CHIMERA:
      case RACE_DRAGON:
      case RACE_CENTAUR:
      case RACE_LAMIA:
        return true;
    default:
    return false;
  }
}

bool TMonster::isRideable() const
{
  if (spec == SPEC_HORSE)
    return TRUE;

  if (race->isRidable())
    return TRUE;
  return FALSE;
}

bool TBeing::canRide(const TBeing *horse) const
{
  if (!horse->isRideable())
    return FALSE;

  // horse riding horse.  bad.
  if (isRideable())
    return FALSE;

  // this is checked for in doMount
  if (horse->mobVnum()==MOB_ELEPHANT &&
      hasQuestBit(TOG_MONK_GREEN_STARTED))
    return TRUE;
  if (horse->getHeight() <= (6 * getHeight() / 10))
    return FALSE;
  if (horse->getHeight() >= (5 * getHeight() / 2))
    return FALSE;


  return TRUE;
}

bool TBeing::hasSaddle() const
{
  TThing *obj;

  if (!isRideable())
    return FALSE;
  if (!(obj = equipment[WEAR_BACK]))
    return FALSE;
  TBaseClothing *tbc = dynamic_cast<TBaseClothing *>(obj);
  TBaseContainer *tbc2 = dynamic_cast<TBaseContainer *>(obj);
  if (tbc && tbc->isSaddle())
    return 1;
  if (tbc2 && tbc2->isSaddle())
    return (tbc2->isSaddle());
  return FALSE;
}

// returns DELETE_THIS
int TMonster::lookForHorse()
{
  int rc;
  sstring buf;
  TThing *t;
  TBeing *horse = NULL;

  if (!isHumanoid() || UtilMobProc(this) || GuildMobProc(this) ||
      IS_SET(specials.act, ACT_SENTINEL) ||
      roomp->isRoomFlag(ROOM_PEACEFUL))
    return FALSE;

  if (5*getHit() < 4*hitLimit())
    return FALSE;

  if(isShopkeeper())
    return FALSE;

  switch (spec) {
    // utilmobs already accounted for
    case SPEC_BOUNTY_HUNTER:
    case SPEC_JANITOR:
    case SPEC_DOCTOR:
    case SPEC_POSTMASTER:
    case SPEC_STABLE_MAN:
    case SPEC_REPAIRMAN:
    case SPEC_SHARPENER:
    case SPEC_PET_KEEPER:
    case SPEC_ENGRAVER:
    case SPEC_SHOPKEEPER:
      return FALSE;
    default:
      break;
  }

  TBeing *tbt = dynamic_cast<TBeing *>(riding);
  if (tbt) {
    if (tbt->getPosition() < POSITION_SLEEPING) {
    } else if (tbt->getPosition() == POSITION_SLEEPING) {
      buf = fmt("order %s wake") %fname(tbt->name);
      rc = addCommandToQue(buf);
      if (IS_SET_DELETE(rc, DELETE_THIS))
        return DELETE_THIS;
      return TRUE;
    } else if (tbt->getPosition() <= POSITION_SITTING) {
      buf = fmt("order %s stand") %fname(tbt->name);
      rc = addCommandToQue(buf);
      if (IS_SET_DELETE(rc, DELETE_THIS))
        return DELETE_THIS;
      return TRUE;
    }

    /* don't look for another horse, but make mount assist me */
    if (fight() && !tbt->fight()) {
      buf = fmt("order %s hit ") %fname(tbt->name);
      buf += fname(fight()->name);
      rc = addCommandToQue(buf);
      if (IS_SET_DELETE(rc, DELETE_THIS))
        return DELETE_THIS;
      return TRUE;
    }
    return TRUE;
  }
  if (rider || fight() || isRideable())
    return FALSE;

  if (getPosition() < POSITION_STANDING)
    return FALSE;

  for (t = roomp->getStuff(); t; t = t->nextThing) {
    horse = dynamic_cast<TBeing *>(t);
    if (!horse)
      continue;
    if (horse == this || horse->rider || horse->isPc() || !canRide(horse))
      continue;
    if (!canSee(horse) || horse->fight())
      continue;

    if (horse->isPet(PETTYPE_PET | PETTYPE_CHARM | PETTYPE_THRALL))
      continue;

    // only choose healthy horses
    if (horse->getHit() < horse->hitLimit())
      continue;

    // let's not suicide ourselves on powerful mounts
    if ((horse->GetMaxLevel() + 4) > GetMaxLevel())
      continue;

    // don't be a horse thief
    if (horse->affectedBySpell(AFFECT_HORSEOWNED))
      continue;

    // technically, should do an addCommandToQue here
    // but since we have tendency to hop on wrong horse with just a "name"
    // and we know we are a mob at this point (so will execute auto)
    // let's skip the middle step, and call doMount direct
    rc = doMount(NULL, CMD_MOUNT, horse);
    if (IS_SET_DELETE(rc, DELETE_THIS))
      return DELETE_THIS;
    return TRUE;
  }
  return FALSE;
}

TThing * TThing::dismount(positionTypeT pos)
{
  TThing *t;

  if (!riding) {
    // use this to find out where this is called from
    vlogf(LOG_BUG, fmt("%s not riding in call to dismount().") %  getName());
    return NULL;
  }
  if (riding->rider == this)
    riding->rider = nextRider;
  else {
    // find previous
    for (t = riding->rider; t && t->nextRider != this; t = t->nextRider);
    if (!t) {
      vlogf(LOG_BUG, "Illegal rider structure!");
      return NULL;
    }
    t->nextRider = nextRider;
  }
  TBeing *tbt = dynamic_cast<TBeing *>(riding);
  TMonster *tmons = dynamic_cast<TMonster *>(riding);
  TBeing *ch = dynamic_cast<TBeing *>(this);

  // If a PC hops off a mount, "save" the mount momentarily to avoid
  // complaints about mobs grabbing the mount
  if (isPc() && tmons) {
    affectedData aff;
    aff.type = AFFECT_HORSEOWNED;
    aff.duration = 1 * UPDATES_PER_MUDHOUR;

    tmons->affectTo(&aff);
  }

  if (tbt && tbt->master == this) {
    // stop follower unless they are following for other reasons
    if (!tbt->isPet(PETTYPE_PET | PETTYPE_CHARM | PETTYPE_THRALL)) {

      // skill based check to let mount continue to follow, even when dismounted
      if (!ch->doesKnowSkill(SKILL_TRAIN_MOUNT) || 
         (tmons && ch && ch->doesKnowSkill(SKILL_TRAIN_MOUNT) && 
	 !ch->bSuccess(ch->getSkillValue(SKILL_TRAIN_MOUNT)/2, SKILL_TRAIN_MOUNT))) {
        tbt->stopFollower(TRUE);

        // locate new master
        t = tbt->horseMaster();
        TBeing *tb3 = dynamic_cast<TBeing *>(t);
        if (tb3) {
          tb3->addFollower(tbt);
          if (tbt->hasSaddle() == 1) {
            act("You hop up into the now vacant saddle.", TRUE, tb3, 0, 0, TO_CHAR);
            act("$n hops up into the now vacant saddle.", TRUE, tb3, 0, 0, TO_ROOM);
          }
        }
      }
    }
  }

  // lamp on a table ought to contribute to room's light
  TTable *ttab = dynamic_cast<TTable *>(riding);
  if (ttab) {
    if (ttab->roomp)
      ttab->roomp->addToLight(-getLight());
    else {
      vlogf(LOG_BUG, "Potential lighting screw up involving tables (dismount).");
    }
  }

  nextRider = NULL;
  riding = NULL;
  if (ch)
    ch->setPosition(pos);

  return this;
}

// returns DELETE_THIS
// 'silent' mode cuts out success messages
int TBeing::doMount(const char *arg, cmdTypeT cmd, TBeing *h, silentTypeT silent)
{
  char caName[112];
  int check = 0/*, rc = 0*/, fightCheck = 0, learn = 0;
  TBeing *horse;

  if (cmd == CMD_RIDE || cmd == CMD_MOUNT) {
    if(!task && riding && (getDirFromChar(arg) != DIR_NONE)){
      sendTo("You urge your mount forward.\n\r");
      start_task(this, NULL, NULL, TASK_RIDE, arg, 2, inRoom(), 0, 0, 5);
      return TRUE;
    }
    if (!(horse = h)) {
      strcpy(caName, arg);
      if (!(horse = get_char_room_vis(this, caName, NULL, EXACT_NO, INFRA_YES))) {
        sendTo("Mount what?\n\r");
        return FALSE;
      }
    }
    if (checkBusy()) {
      return FALSE;
    }
    if (!isHumanoid()) {
      sendTo("You can't ride things!\n\r");
      return FALSE;
    }
    if (riding) {
      sendTo("You are already riding.\n\r");
      return FALSE;
    }
    if (isCombatMode( ATTACK_BERSERK)) {
      sendTo("Your berserker rage scares the mount.\n\r");
      return FALSE;
    }
    if (horse->hasSaddle()){
      TBaseContainer *tbc3 = dynamic_cast<TBaseContainer *>(horse->equipment[WEAR_BACK]);
      if (tbc3 && tbc3->isSaddle() == 2) {
      	act("You cannot ride $N when it is saddled with a pack.", FALSE,this, 0,horse, TO_CHAR);
      	return FALSE;     
      }
    }
    if (!isImmortal() && (horse->isTanking() || (horse->fight() && !hasClass(CLASS_DEIKHAN)))) {
      sendTo("You do not have the skill to mount something that is fighting!\n\r");
      return FALSE;
    }
    if (horse->isPet(PETTYPE_PET) && horse->master != this) {
      act("You can't ride someone else's pet.", FALSE, this, 0, 0, TO_CHAR);
      return FALSE;
    }
    if (!canRide(horse)) {
      sendTo("You can't ride that!\n\r");
      return FALSE;
    }
    if (horse->fight()) {
      sendTo("Trying to ride something involved in a life-or-death fight is not advisable.\n\r");
      return FALSE;
    }
    if (horse->getNumRiders(this) >= horse->getMaxRiders()) {
      sendTo(COLOR_MOBS, fmt("The maximum number of riders are already riding %s.\n\r") % horse->getName());
      return FALSE;
    }

    if (!isPc() && horse->master) {
      sendTo("They are currently following someone else, my dear, and will not follow you right now.");
      return FALSE;
    }

    // weight > free horse carry weight
    if (compareWeights(getTotalWeight(TRUE),
                 (horse->carryWeightLimit() - horse->getCarriedWeight())) == -1) {
      act("$N can't carry all your weight.", 0, this, 0, horse, TO_CHAR);
      act("$n starts to hop up onto you, but stops when $e sees you can't carry $m.",
           TRUE, this, 0, horse, TO_VICT);
      act("$n starts to hop up onto $N, but stops when $e sees $E can't carry $m.",
           TRUE, this, 0, horse, TO_NOTVICT);
      return FALSE;
    }

    //    if (isPlayerAction(PLR_SOLOQUEST) && 
    // !(hasQuestBit(TOG_MONK_GREEN_STARTED) && 
    //  horse->mobVnum()==MOB_ELEPHANT)){
    //  sendTo("You are on a solo-quest!  No use of mounts allowed!\n\r");
    //  return FALSE;
    // }

    // I commented out the above to allow use of mounts on solo quests.
    // Deikhan skills depend on mounts and whats fair is fair for all classes --jh

    // keep these two checks identical to whats in canRide
    if(!(horse->mobVnum()==MOB_ELEPHANT &&
	 hasQuestBit(TOG_MONK_GREEN_STARTED))){
      if (horse->getHeight() <= (6 * getHeight() / 10)) {
	act("$N is too small for you to ride.", FALSE, this, 0, horse, TO_CHAR);
	return FALSE;
      }
      if (horse->getHeight() >= (5 * getHeight() / 2)){
	act("$N is too large for you to ride.", FALSE, this, 0, horse, TO_CHAR);
	return FALSE;
      }
    }    
    if (!isImmortal() && (fight() || horse->fight())) {
      learn = getSkillValue(SKILL_RIDE) + 
	advancedRidingBonus(dynamic_cast<TMonster *>(horse));
      if (isTanking() || horse->isTanking()) {
        if (!hasClass(CLASS_DEIKHAN)) {
          learn /= 4;
        }else {
          learn /= 3;
        }
        fightCheck = 1;
      } else if (!isAffected(AFF_ENGAGER)) {
        learn /= 2;
        fightCheck = 2;
      }

      if (!bSuccess(learn, SKILL_RIDE)) {
        if (fightCheck == 1) {
          if (horse->isTanking()) {
             sendTo("You find it extremely difficult to mount something that is tanking.\n\r");
          } else {
            sendTo("You find it extremely difficult to mount while tanking.\n\r");
          }
        } else if (fightCheck == 2) {
          if (horse->fight()) {
            sendTo("You find it difficult to mount something that is fighting.\n\r");
          } else {
            sendTo("You find it difficult to mount while fighting.\n\r");
          }
        }
        act("You try to ride $N, but fail and fall on your butt.", FALSE, this, 0, horse, TO_CHAR);
        act("$n tries to ride $N, but fails and falls on $s butt.", FALSE, this,0, horse, TO_NOTVICT);
        act("$n tries to ride you, but fails and falls on $s butt.", FALSE, this
, 0, horse, TO_VICT);
        setPosition(POSITION_SITTING);
        addToWait(combatRound(2));
        if (!horse->isPc())
          dynamic_cast<TMonster *>(horse)->aiHorse(this);
        return FALSE;
      }
    }
    if (roomp && !roomp->isFlyingSector()) {
      if (isFlying()) {
        if (!horse->isFlying()) {
          sendTo("Riding a grounded mount while flying is impossible.\n\r");
          return FALSE;
        }
      }
      if (horse->isFlying() && !isFlying()) {
        if (!hasClass(CLASS_DEIKHAN)) {
          sendTo("You can't mount something that is flying.\n\r");
          return FALSE;
        } else if (getSkillValue(SKILL_RIDE_WINGED) < 70) {
          sendTo("I am afraid you don't know enough about winged creatures to mount one while it is flying.\n\r");
          return FALSE;
        } else {
          if (::number(-10, getSkillValue(SKILL_RIDE_WINGED)) > 0) {
            if (!silent) act("You coax $N to land so you can mount.",
                TRUE, this, NULL, horse, TO_CHAR);
            if (!silent) act("$n coaxes you into landing, you feel charmed and comply.",
                TRUE, this, NULL, horse, TO_VICT);
            if (!silent) act("$n coaxes $N into landing.",
                TRUE, this, NULL, horse, TO_NOTVICT);
            horse->doLand();

            if (horse->getPosition() != POSITION_STANDING) {
              sendTo("Oddly enough you still failed.\n\r");
              horse->sendTo("Oddly enough they still failed.\n\r");
              act("Oddly enough $n still failed.",
                  TRUE, this, NULL, horse, TO_NOTVICT);
              return FALSE;
            }
          } else {
            act(fmt("You attempt to coax $N into landing but %s seems to ignore you.")
                % horse->thirdPerson(POS_SUBJECT),
                TRUE, this, NULL, horse, TO_CHAR);
            act("The Nerve!  $n just tried to make you land.",
                TRUE, this, NULL, horse, TO_VICT);
            act(fmt("$n attempts to coax $N into landing, who promptly ignores %s.")
                % thirdPerson(POS_OBJECT),
                TRUE, this, NULL, horse, TO_NOTVICT);
            return FALSE;
          }
        }
      }
    }

    // This was the old 'lets beat the shit out of newbies' code, disabled.
#if 0
    check = MountEgoCheck(this, horse);
    if ((check > 5 || (horse->GetMaxLevel() > GetMaxLevel())) &&
        (roomp && !roomp->isRoomFlag(ROOM_PEACEFUL))) {
      act("$N snarls and attacks!", FALSE, this, 0, horse, TO_CHAR);
      act("As $n tries to mount $N, $N attacks $n!", FALSE, this, 0, horse, TO_NOTVICT);
      addToWait(combatRound(1));
      rc = horse->hit(this);
      if (IS_SET_DELETE(rc, DELETE_THIS)) {
        if (h)
          return DELETE_VICT;
        if (h)
          return DELETE_VICT;
        delete horse;
        horse = NULL;
      }
      if (IS_SET_DELETE(rc, DELETE_VICT))
        return DELETE_THIS;
      return TRUE;
    } else if ((check > -1) || (horse->GetMaxLevel() > GetMaxLevel())) {
      if ((horse->getPosition() > POSITION_STUNNED) && (horse->getPosition() < POSITION_STANDING)) {
        act("$n quickly stands up.",0, horse, 0, 0, TO_ROOM);

        horse->setPosition(POSITION_STANDING);
      }
      act("$N bucks you off, you fall on your butt.", FALSE, this, 0, horse, TO_CHAR);
      act("As $n tries to mount $N, $N bucks $m off.", FALSE, this, 0, horse, TO_NOTVICT);
      addToWait(combatRound(1));
      setPosition(POSITION_SITTING);
      if (!horse->isPc())
        dynamic_cast<TMonster *>(horse)->aiHorse(this);
      return TRUE;
    }
#else
    if(!(horse->mobVnum()==MOB_ELEPHANT &&
	 hasQuestBit(TOG_MONK_GREEN_STARTED)) &&
       horse->GetMaxLevel() > GetMaxLevel()){
      switch (::number(0, 3)) {
        case 0:
          act("$N bucks you off, you fall on your butt.",
              FALSE, this, 0, horse, TO_CHAR);
          act("As $n tries to mount $N, $N bucks $m off.",
              FALSE, this, 0, horse, TO_NOTVICT);
          break;
        case 1:
          act("$N quickly moves and you quickly find yourself on your face.",
                FALSE, this, 0, horse, TO_CHAR);
          act("$N quickly moves as $n tries to mount it leaving $n on $s face.",
                FALSE, this, 0, horse, TO_NOTVICT);
          break;
        case 2:
          act("You attempt to mount $N but get your foot caught up and fall.",
                FALSE, this, 0, horse, TO_CHAR);
          act("$n gets $s foot tangled up and falls as they attempt to mount $N.",
                FALSE, this, 0, horse, TO_NOTVICT);
          break;
        default:
          act("You attempt to mount $N who turns and knocks you down.",
                FALSE, this, 0, horse, TO_CHAR);
          act("$n attempts to mount $N who turns and knocks $m down.",
                FALSE, this, 0, horse, TO_NOTVICT);
          break;
      }

      addToWait(combatRound(1));
      setPosition(POSITION_SITTING);

      if (!horse->isPc())
        dynamic_cast<TMonster *>(horse)->aiHorse(this);

      return TRUE;
    }
#endif
    if (rideCheck(-check)) {
      if (!silent) {
        if (horse->hasSaddle()==1 && !horse->rider) {
          act("You hop into the saddle and start riding $N.",
                   FALSE, this, 0, horse, TO_CHAR);
          act("$n hops into the saddle and starts riding $N.", 
                   FALSE, this, 0, horse, TO_NOTVICT);
          act("$n hops on your back!", FALSE, this, 0, horse, TO_VICT);
        } else if (!horse->rider) {
          act("You start riding $N.", FALSE, this, 0, horse, TO_CHAR);
          act("$n starts riding $N.", FALSE, this, 0, horse, TO_NOTVICT);
          act("$n hops on your back!", FALSE, this, 0, horse, TO_VICT);
        } else {
          act("You start riding $N's $o.", FALSE, this, horse, horse->horseMaster(), TO_CHAR);
          act("$n starts riding $N's $o.", FALSE, this, horse, horse->horseMaster(), TO_NOTVICT);
          act("$n hops on your $o's back!", FALSE, this, horse, horse->horseMaster(), TO_VICT);
          act("$n hops on your back!", FALSE, this, 0, horse, TO_VICT);
        }
      }
      loseSneak();

      mount(horse);
      setPosition(POSITION_MOUNTED);

      // horse was following someone else when i started riding
      if (horse->master && horse->master != this && !horse->rider)
        horse->stopFollower(TRUE);

      // horse should follow someone (in general, this is horse master)
      if (!horse->master)
        addFollower(horse);
    
      horse->specials.hunting = 0;
      if (!horse->isPc())
        dynamic_cast<TMonster *>( horse)->setTarg(NULL);
      dynamic_cast<TMonster *>( horse)->hates.clist = NULL;
      dynamic_cast<TMonster *>( horse)->fears.clist = NULL;

    } else {
      act("You try to ride $N, but fail and fall on your butt.", FALSE, this, 0, horse, TO_CHAR);
      act("$n tries to ride $N, but fails and falls on $s butt.", FALSE, this, 0, horse, TO_NOTVICT);
      act("$n tries to ride you, but fails and falls on $s butt.", FALSE, this, 0, horse, TO_VICT);
      setPosition(POSITION_SITTING);
      addToWait(combatRound(2));
      if (!horse->isPc())
        dynamic_cast<TMonster *>( horse)->aiHorse(this);
    }
    return TRUE;
  } else if (cmd == CMD_DISMOUNT) {
    if (!riding) {
      sendTo("You don't seem to be riding anything.\n\r");
      return FALSE;
    }
    if (!dynamic_cast<TBeing *>(riding)) {
      doStand();
      return FALSE;
    }
    horse = dynamic_cast<TBeing *>(riding);
    if (horse->fight() && !hasClass(CLASS_DEIKHAN)) {
      sendTo("You can't dismount while your mount is fighting!\n\r");
      return FALSE;
    }
    if (isCombatMode(ATTACK_BERSERK)) {
      sendTo("Your berserker rage prevents you from dismounting.\n\r");
      return FALSE;
    }
    if (roomp->getMoblim() && !isImmortal() &&
        (MobCountInRoom(roomp->getStuff()) >= roomp->getMoblim())) {
      // movement treats horse + all riders as 1 "thing" in room
      sendTo("There isn't enough room in here to dismount.\n\r");
      return FALSE;
    }

    if (roomp && roomp->isFlyingSector()) {
      dismount(POSITION_FLYING);
      if (!silent) {
        act("You dismount from $N.", FALSE, this, 0, horse, TO_CHAR);
        act("$n dismounts from $N.", FALSE, this, 0, horse, TO_NOTVICT);
        act("$n dismounts from you.", FALSE, this, 0, horse, TO_VICT);
        sendTo("The magic in the air prevents you from falling.\n\r");
      }
    } else if (roomp->isAirSector() || roomp->isVertSector()) {
      if (canFly()) {
        if (!silent) {
          act("You dismount from $N.", FALSE, this, 0, horse, TO_CHAR);
          act("$n dismounts from $N.", FALSE, this, 0, horse, TO_NOTVICT);
          act("$n dismounts from you.", FALSE, this, 0, horse, TO_VICT);
        }
        dismount(POSITION_STANDING);
        doFly();
      } else {
        sendTo("You must order your mount to land before dismounting.\n\r");
        return FALSE;
      } 
    } else if (horse->isFlying()) {
      if (canFly()) {
        if (!silent) {
          act("You dismount from $N.", FALSE, this, 0, horse, TO_CHAR);
          act("$n dismounts from $N.", FALSE, this, 0, horse, TO_NOTVICT);
          act("$n dismounts from you.", FALSE, this, 0, horse, TO_VICT);
        }
        dismount(POSITION_STANDING);
        doFly();
      } else {
        sendTo("You must order your mount to land before dismounting.\n\r");
        return FALSE;
      }
    } else {
      if (!silent) {
        act("You dismount from $N.", FALSE, this, 0, horse, TO_CHAR);
        act("$n dismounts from $N.", FALSE, this, 0, horse, TO_NOTVICT);
        act("$n dismounts from you.", FALSE, this, 0, horse, TO_VICT);
      }
      dismount(POSITION_STANDING);
    }
    return TRUE;
  }
  vlogf(LOG_BUG, fmt("Undefined call to doMount.  cmd = %d") %  cmd);
  return TRUE;
}

// a positive mod is beneficial to the rider
int TBeing::rideCheck(int mod)
{
  int learn = 0;
  
  if (isImmortal())
    return TRUE;

  TBeing *tbt = dynamic_cast<TBeing *>(riding);
  if (tbt && tbt->hasSaddle() && tbt->horseMaster() == this)
    // only guy in saddle gets benefit
    mod += 8;

  if (tbt && hasClass(CLASS_DEIKHAN)){
    mod += 5;  // default bonus for deikhans
 
    // 0-6
    mod+=::number(0, advancedRidingBonus(dynamic_cast<TMonster *>(tbt)))/15;
  }

  if (tbt && tbt->horseMaster() != this)
    mod -= 5;  // secondary rider
  
  learn = getSkillValue(SKILL_RIDE);
  learn += (3 * mod);
  
  // rideChecks are sometimes made for people sitting on objects
  // use of bSuccess here allows them to learn "ride" while on chairs.
  // since it is assumed that rideCheck is called (for objs) only during
  // attacks (being shoved, etc), this is thought to be an OK setup.
  if (bSuccess(learn, SKILL_RIDE)) 
    return TRUE;
  return FALSE; 
}

int TThing::fallOffMount(TThing *, positionTypeT pos, bool)
{
  dismount(pos);
  return FALSE;
}

// returns DELETE_THIS
int TBeing::fallOffMount(TThing *h, positionTypeT pos, bool death)
{
  int rc = FALSE;

  if (dynamic_cast<TBeing *>(h)) {
    act("$n loses control and falls off of $N.", 
          FALSE, this, 0, h, TO_NOTVICT, ANSI_RED);
    act("$n loses control and falls off of you.", 
          FALSE, this, 0, h, TO_VICT, ANSI_RED);
    act("You lose control and fall off of $N.", 
          FALSE, this, 0, h, TO_CHAR, ANSI_RED);

    if (!h->isPc())
      dynamic_cast<TMonster *>(h)->aiHorse(this);

// change the position
     dismount(pos);

    // fall off a flying mount..
    if (h->isFlying()) {
      if (h->roomp->isFlyingSector()) {
        rc = crashLanding(POSITION_FLYING);
      } else if (h->roomp->isFallSector()) {
        rc = checkFalling();
      } else {
        if (canFly()) {
          rc = crashLanding(POSITION_FLYING);
        } else {
          rc = crashLanding(POSITION_RESTING);
        }
      }
    } else if (h->roomp->isFallSector()) {
      rc = checkFalling();

    } else {
    }
    if (IS_SET_DELETE(rc, DELETE_THIS))
      return DELETE_THIS;
    return TRUE;
  } else {
    // a chair or something
    // change the position
    if (death) {
      if (pos == POSITION_DEAD) {
        act("$n's lifeless body falls off $p onto the $g.",
            FALSE, this, h, 0, TO_ROOM, ANSI_RED);
      } else if ((pos > POSITION_DEAD) && (pos <= POSITION_STUNNED)) {
        act("$n's limp body falls off $p onto the $g.", FALSE, this, h, 0, TO_ROOM, ANSI_RED);

      } else {
        act("$n's falls off $p onto the $g.",
            FALSE, this, h, 0, TO_ROOM, ANSI_RED);
      }
      act("You lose your balance and fall off of $p onto the $g.",
            FALSE, this, h, 0, TO_CHAR, ANSI_RED);
    } else {
      act("$n loses $s balance and falls off of $p.", 
            FALSE, this, h, 0, TO_ROOM, ANSI_RED);
      act("You lose your balance and fall off of $p.", 
            FALSE, this, h, 0, TO_CHAR, ANSI_RED);
    }
    dismount(pos);
  }

  if (spelltask)
    setDistracted(-1, FALSE);

  return FALSE;
}

// ego is a number representing how "ballsy" the mount is 
// > 5  == mount will attack 
// 0 - 4  == will buck off   
// negative == accepts rider 
int MountEgoCheck(TBeing *ch, TBeing *horse)
{
  int check;

  if (horse->getPosition() <= POSITION_STUNNED)
    return -10;

  if (horse->isDragonRideable()) {
    check =  horse->GetMaxLevel();
    check += dynamic_cast<TMonster *>(horse)->anger() / 10;
    check -= ch->GetMaxLevel();
    check -= ch->getSkillValue(SKILL_RIDE)/10;
    check += number((int) horse->plotStat(STAT_CURRENT, STAT_PER, 1.5, 9.0, 5.0),
                    (int) horse->plotStat(STAT_CURRENT, STAT_PER, 4.5, 27.0, 15.0));
    check += number((int) horse->plotStat(STAT_CURRENT, STAT_FOC, 1.5, 9.0, 5.0),
                    (int) horse->plotStat(STAT_CURRENT, STAT_FOC, 4.5, 27.0, 15.0));
    check -= number((int) ch->plotStat(STAT_CURRENT, STAT_CHA, 1.5, 9.0, 5.0),
                    (int) ch->plotStat(STAT_CURRENT, STAT_CHA, 4.5, 27.0, 15.0));
    if (horse->getPosition() <= POSITION_SLEEPING)
      check -= 2;
#if 0
    if (ch->isSameAlign(horse))
      check -= 2;
    else
      check += 2;
#endif
    check *= max(0,horse->getHit());
    check /= max(1,(int) horse->hitLimit());
    check *= ch->hitLimit();
    check /= max(1,ch->getHit());
    return(check);
  } else {
    check = horse->GetMaxLevel();
    check += dynamic_cast<TMonster *>(horse)->anger() / 30;
    check -= ch->GetMaxLevel();
    if (ch->doesKnowSkill(SKILL_RIDE))
      check -= ch->getSkillValue(SKILL_RIDE)/10;
    check += number((int) horse->plotStat(STAT_CURRENT, STAT_PER, 1.5, 9.0, 5.0),
                    (int) horse->plotStat(STAT_CURRENT, STAT_PER, 4.5, 27.0, 15.0));
    check += number((int) horse->plotStat(STAT_CURRENT, STAT_FOC, 1.5, 9.0, 5.0),
                    (int) horse->plotStat(STAT_CURRENT, STAT_FOC, 4.5, 27.0, 15.0));
    check -= number((int) ch->plotStat(STAT_CURRENT, STAT_CHA, 1.5, 9.0, 5.0),
                    (int) ch->plotStat(STAT_CURRENT, STAT_CHA, 4.5, 27.0, 15.0));
    if (horse->getPosition() <= POSITION_SLEEPING)
      check -= 2;
    check *= max(0,horse->getHit());
    check /= max(1,(int) horse->hitLimit());
    check *= ch->hitLimit();
    check /= max(1,ch->getHit());
    return(check);
  }
}

TThing * TThing::horseMaster(void) const
{
  TThing *t;

  // locate the last valid "rider"
  for (t = rider; t && t->nextRider; t = t->nextRider);

  return t;
}

int TBeing::getNumRiders(TThing *ch) const
{
  TThing *t;
  int num = 0;

  for (t = rider; t; t = t->nextRider) {
    if (t == ch)
      continue;
    if (t->getHeight() > getHeight() *2/3)
      num += 2;
    else
      num++;
  }
  return num;
}

int TBeing::getMaxRiders() const
{
  // leave this fixed.
  // we can put up to 4 very small (kids) people on the horse
  // a kid essentially counts as 1 slot, a man as 2
  return 4;
}

int TBeing::getRiderHeight() const
{
  return (82 * getHeight() / 100);
}


