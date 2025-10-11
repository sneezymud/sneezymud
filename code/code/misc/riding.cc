#include "being.h"
#include "combat.h"
#include "extern.h"
#include "handler.h"
#include "low.h"
#include "monster.h"
#include "obj_base_clothing.h"
#include "obj_base_container.h"
#include "obj_table.h"
#include "room.h"
#include "spec_mobs.h"
#include "spells.h"

namespace {
// Mount size compatibility ratios
constexpr int MIN_MOUNT_HEIGHT_PERCENT =
    60; // mount must be at least 60% of rider height
constexpr int MAX_MOUNT_HEIGHT_PERCENT =
    250; // mount can be at most 250% of rider height

// Riding skill bonuses
constexpr int SECONDARY_RIDER_PENALTY = 5;

// Combat mounting difficulty divisors
constexpr int TANKING_DIVISOR_NORMAL = 4;
constexpr int TANKING_DIVISOR_DEIKHAN = 3;
constexpr int FIGHTING_DIVISOR = 2;

// Winged mount skill threshold
constexpr int MIN_WINGED_SKILL_FOR_COAX = 70;

// Rider capacity and size calculations
constexpr int MAX_RIDERS = 4;
constexpr int LARGE_RIDER_THRESHOLD_PERCENT =
    67; // riders over 2/3 mount height count as 2 slots
constexpr int RIDER_HEIGHT_PERCENT = 82;
} // namespace

// Peel
spellNumT TBeing::mountSkillType() const {
  switch (getRace()) {
  case RACE_HORSE:
  case RACE_BOVINE:
  case RACE_OX:
  case RACE_PIG:
  case RACE_SHEEP:
  case RACE_BAANTA:
  case RACE_CANINE:
  case RACE_GOAT:
    return SKILL_RIDE_DOMESTIC;
  case RACE_RHINO:
  case RACE_TIGER:
  case RACE_GIRAFFE:
  case RACE_BEAR:
  case RACE_BOAR:
  case RACE_ELEPHANT:
  case RACE_DEER:
    return SKILL_RIDE_NONDOMESTIC;
  case RACE_GRIFFON:
  case RACE_HIPPOGRIFF:
  case RACE_WYVERN:
  case RACE_DRAGON:
  case RACE_DRAGONNE:
  case RACE_LAMMASU:
  case RACE_SHEDU:
  case RACE_SPHINX:
    return SKILL_RIDE_WINGED;
  case RACE_FELINE:
  case RACE_BASILISK:
  case RACE_CENTAUR:
  case RACE_CHIMERA:
  case RACE_FROG:
  case RACE_LAMIA:
  case RACE_MANTICORE:
  case RACE_TURTLE:
  case RACE_LION:
  case RACE_LEOPARD:
  case RACE_COUGAR:
  case RACE_WYVELIN:
    return SKILL_RIDE_EXOTIC;
  default:
    return SKILL_RIDE_EXOTIC;
  }
}

bool TMonster::isDragonRideable() const {
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

bool TMonster::isRideable() const {
  if (spec == SPEC_HORSE)
    return TRUE;

  if (race->isRidable())
    return TRUE;
  return FALSE;
}

bool TBeing::canRide(const TBeing *horse) const {
  if (!horse->isRideable())
    return FALSE;

  // horse riding horse.  bad.
  if (isRideable())
    return FALSE;

  // this is checked for in doMount
  if (horse->mobVnum() == Mob::ELEPHANT && hasQuestBit(TOG_MONK_GREEN_STARTED))
    return TRUE;
  if (horse->getHeight() <= (MIN_MOUNT_HEIGHT_PERCENT * getHeight() / 100)){
    act("$N is too small for you to ride.", FALSE, this, 0, horse, TO_CHAR);
    return FALSE;
  }
  
  if (horse->getHeight() >= (MAX_MOUNT_HEIGHT_PERCENT * getHeight() / 100)){
    act("$N is too large for you to ride.", FALSE, this, 0, horse, TO_CHAR);
    return FALSE;
  }

  return TRUE;
}

bool TBeing::hasSaddle() const {
  if (!isRideable())
    return false;

  TThing *obj = equipment[WEAR_BACK];
  if (!obj)
    return false;

  // Check if it's a clothing saddle (TSaddle)
  if (auto *clothing = dynamic_cast<TBaseClothing *>(obj)) {
    return clothing->isSaddle();
  }

  // Check if it's a container saddle (saddlebag)
  if (auto *container = dynamic_cast<TBaseContainer *>(obj)) {
    return container->isSaddle() > 0;
  }

  return false;
}

// returns DELETE_THIS
int TMonster::lookForHorse() {
  int rc;
  sstring buf;
  TThing *t = NULL;
  TBeing *horse = NULL;

  if (!isHumanoid() || UtilMobProc(this) || GuildMobProc(this) ||
      IS_SET(specials.act, ACT_SENTINEL) ||

      roomp->isRoomFlag(ROOM_PEACEFUL))
    return FALSE;

  if (5 * getHit() < 4 * hitLimit())
    return FALSE;

  if (isShopkeeper())
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
      buf = format("order %s wake") % fname(tbt->name);
      rc = addCommandToQue(buf);
      if (IS_SET_DELETE(rc, DELETE_THIS))
        return DELETE_THIS;
      return TRUE;
    } else if (tbt->getPosition() <= POSITION_SITTING) {
      buf = format("order %s stand") % fname(tbt->name);
      rc = addCommandToQue(buf);
      if (IS_SET_DELETE(rc, DELETE_THIS))
        return DELETE_THIS;
      return TRUE;
    }

    /* don't look for another horse, but make mount assist me */
    if (fight() && !tbt->fight()) {
      buf = format("order %s hit ") % fname(tbt->name);
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

  for (StuffIter it = roomp->stuff.begin();
       it != roomp->stuff.end() && (t = *it); ++it) {
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

TThing *TThing::dismount(positionTypeT pos) {
  TThing *t;

  if (!riding) {
    // use this to find out where this is called from
    vlogf(LOG_BUG, format("%s not riding in call to dismount().") % getName());
    return NULL;
  }
  if (riding->rider == this)
    riding->rider = nextRider;
  else {
    // find previous
    for (t = riding->rider; t && t->nextRider != this; t = t->nextRider)
      ;
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
    aff.duration = 1 * Pulse::UPDATES_PER_MUDHOUR;

    tmons->affectTo(&aff);
  }

  if (tbt && tbt->master == this) {
    // stop follower unless they are following for other reasons
    if (!tbt->isPet(PETTYPE_PET | PETTYPE_CHARM | PETTYPE_THRALL)) {
      // skill based check to let mount continue to follow, even when dismounted
      if (!ch->doesKnowSkill(SKILL_TRAIN_MOUNT) ||
          (tmons && ch && ch->doesKnowSkill(SKILL_TRAIN_MOUNT) &&
           !ch->bSuccess(ch->getSkillValue(SKILL_TRAIN_MOUNT) / 2,
                         SKILL_TRAIN_MOUNT))) {
        tbt->stopFollower(TRUE);

        // locate new master
        t = tbt->horseMaster();
        TBeing *tb3 = dynamic_cast<TBeing *>(t);
        if (tb3) {
          tb3->addFollower(tbt);
          if (tbt->hasSaddle() == 1) {
            act("You hop up into the now vacant saddle.", TRUE, tb3, 0, 0,
                TO_CHAR);
            act("$n hops up into the now vacant saddle.", TRUE, tb3, 0, 0,
                TO_ROOM);
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
      vlogf(LOG_BUG,
            "Potential lighting screw up involving tables (dismount).");
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
int TBeing::doMount(const char *arg, cmdTypeT cmd, TBeing *h,
                    silentTypeT silent) {
  char caName[112];
  int /*, rc = 0*/ fightCheck = 0, learn = 0;
  TBeing *horse;

  if (cmd == CMD_RIDE || cmd == CMD_MOUNT) {
    if (!task && riding && (getDirFromChar(arg) != DIR_NONE)) {
      sendTo("You urge your mount forward.\n\r");
      start_task(this, NULL, NULL, TASK_RIDE, arg, 2, inRoom(), 0, 0, 5);
      return TRUE;
    }
    if (!(horse = h)) {
      strcpy(caName, arg);
      if (!(horse =
                get_char_room_vis(this, caName, NULL, EXACT_NO, INFRA_YES))) {
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
    if (isCombatMode(ATTACK_BERSERK)) {
      sendTo("Your berserker rage scares the mount.\n\r");
      return FALSE;
    }
    if (horse->hasSaddle()) {
      TBaseContainer *tbc3 =
          dynamic_cast<TBaseContainer *>(horse->equipment[WEAR_BACK]);
      if (tbc3 && tbc3->isSaddle() == 2) {
        act("You cannot ride $N when it is saddled with a pack.", FALSE, this,
            0, horse, TO_CHAR);
        return FALSE;
      }
    }
    if (!isImmortal() &&
        (horse->isTanking() || (horse->fight() && !hasClass(CLASS_DEIKHAN)))) {
      sendTo(
          "You do not have the skill to mount something that is fighting!\n\r");
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
    if (horse->getNumRiders(this) >= horse->getMaxRiders()) {
      sendTo(COLOR_MOBS,
             format("The maximum number of riders are already riding %s.\n\r") %
                 horse->getName());
      return FALSE;
    }

    if (!isPc() && horse->master) {
      sendTo("They are currently following someone else, my dear, and will not "
             "follow you right now.");
      return FALSE;
    }

    // weight > free horse carry weight
    if (compareWeights(
            getTotalWeight(TRUE),
            (horse->carryWeightLimit() - horse->getCarriedWeight())) == -1) {
      act("$N can't carry all your weight.", 0, this, 0, horse, TO_CHAR);
      act("$n starts to hop up onto you, but stops when $e sees you can't "
          "carry "
          "$m.",
          TRUE, this, 0, horse, TO_VICT);
      act("$n starts to hop up onto $N, but stops when $e sees $E can't carry "
          "$m.",
          TRUE, this, 0, horse, TO_NOTVICT);
      return FALSE;
    }

    //    if (isPlayerAction(PLR_SOLOQUEST) &&
    // !(hasQuestBit(TOG_MONK_GREEN_STARTED) &&
    //  horse->mobVnum()==Mob::ELEPHANT)){
    //  sendTo("You are on a solo-quest!  No use of mounts allowed!\n\r");
    //  return FALSE;
    // }

    // I commented out the above to allow use of mounts on solo quests.
    // Deikhan skills depend on mounts and whats fair is fair for all classes
    // --jh

    // keep these two checks identical to whats in canRide
    if (!(horse->mobVnum() == Mob::ELEPHANT &&
          hasQuestBit(TOG_MONK_GREEN_STARTED))) {
      if (horse->getHeight() <=
          (MIN_MOUNT_HEIGHT_PERCENT * getHeight() / 100)) {
        act("$N is too small for you to ride.", FALSE, this, 0, horse, TO_CHAR);
        return FALSE;
      }
      if (horse->getHeight() >=
          (MAX_MOUNT_HEIGHT_PERCENT * getHeight() / 100) && !this->bSuccess(SKILL_VAULTING)) {
        act("$N is too large for you to ride.", FALSE, this, 0, horse, TO_CHAR);
        return FALSE;
      }
    }
    if (roomp && !roomp->isFlyingSector()) {
      if (horse->isFlying() && !isFlying()) {
        // Non-deikhans without flying attempting to mount a flying mount
        if (!hasClass(CLASS_DEIKHAN)) {
          sendTo("You can't mount something that is flying.\n\r");
          return FALSE;
        }
        // Deikhans without flying attempting to mount a flying mount
        else {
          // Deikhan is not skilled enough attempt coaxing the mount down
          if (getSkillValue(SKILL_RIDE_WINGED) < MIN_WINGED_SKILL_FOR_COAX) {
            sendTo(
                "I am afraid you don't know enough about winged creatures to "
                "mount one while it is flying.\n\r");
            return FALSE;
            // Deikhan is skilled enough to attempt and succeeds
          } else if (::number(-10, getSkillValue(SKILL_RIDE_WINGED)) > 0) {
            if (!silent)
              act("You coax $N to land so you can mount.", TRUE, this, NULL,
                  horse, TO_CHAR);
            if (!silent)
              act("$n coaxes you into landing, you feel charmed and comply.",
                  TRUE, this, NULL, horse, TO_VICT);
            if (!silent)
              act("$n coaxes $N into landing.", TRUE, this, NULL, horse,
                  TO_NOTVICT);
            horse->doLand();

            if (horse->getPosition() != POSITION_STANDING) {
              sendTo("Oddly enough you still failed.\n\r");
              horse->sendTo("Oddly enough they still failed.\n\r");
              act("Oddly enough $n still failed.", TRUE, this, NULL, horse,
                  TO_NOTVICT);
              return FALSE;
            }
            // Deikhan is skilled enough to attempt but fails
          } else {
            act(format("You attempt to coax $N into landing but %s seems to "
                       "ignore you.") %
                    horse->thirdPerson(POS_SUBJECT),
                TRUE, this, NULL, horse, TO_CHAR);
            act("The Nerve!  $n just tried to make you land.", TRUE, this, NULL,
                horse, TO_VICT);
            act(format("$n attempts to coax $N into landing, who promptly "
                       "ignores %s.") %
                    thirdPerson(POS_OBJECT),
                TRUE, this, NULL, horse, TO_NOTVICT);
            return FALSE;
          }
        }
      }
    }
    if (!isImmortal() && (fight() || horse->fight())) {
      learn = getSkillValue(SKILL_RIDE) +
              advancedRidingBonus(dynamic_cast<TMonster *>(horse));
      if (isTanking() || horse->isTanking()) {
        if (!hasClass(CLASS_DEIKHAN)) {
          learn /= TANKING_DIVISOR_NORMAL;
        } else {
          learn /= TANKING_DIVISOR_DEIKHAN;
        }
        fightCheck = 1;
      } else if (!isAffected(AFF_ENGAGER)) {
        learn /= FIGHTING_DIVISOR;
        fightCheck = 2;
      }

      if (!bSuccess(learn, SKILL_RIDE)) {
        if (fightCheck == 1) {
          if (horse->isTanking()) {
            sendTo("You find it extremely difficult to mount something that is "
                   "tanking.\n\r");
          } else {
            sendTo(
                "You find it extremely difficult to mount while tanking.\n\r");
          }
        } else if (fightCheck == 2) {
          if (horse->fight()) {
            sendTo("You find it difficult to mount something that is "
                   "fighting.\n\r");
          } else {
            sendTo("You find it difficult to mount while fighting.\n\r");
          }
        }
        act("You try to ride $N, but fail and fall on your butt.", FALSE, this,
            0, horse, TO_CHAR);
        act("$n tries to ride $N, but fails and falls on $s butt.", FALSE, this,
            0, horse, TO_NOTVICT);
        act("$n tries to ride you, but fails and falls on $s butt.", FALSE,
            this, 0, horse, TO_VICT);
        setPosition(POSITION_SITTING);
        addToWait(combatRound(2));
        if (!horse->isPc())
          dynamic_cast<TMonster *>(horse)->aiHorse(this);
        return FALSE;
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
    if (!(horse->mobVnum() == Mob::ELEPHANT &&
          hasQuestBit(TOG_MONK_GREEN_STARTED)) &&
        horse->GetMaxLevel() > GetMaxLevel()) {
      switch (::number(0, 3)) {
      case 0:
        act("$N bucks you off, you fall on your butt.", FALSE, this, 0, horse,
            TO_CHAR);
        act("As $n tries to mount $N, $N bucks $m off.", FALSE, this, 0, horse,
            TO_NOTVICT);
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
        act("You attempt to mount $N who turns and knocks you down.", FALSE,
            this, 0, horse, TO_CHAR);
        act("$n attempts to mount $N who turns and knocks $m down.", FALSE,
            this, 0, horse, TO_NOTVICT);
        break;
      }

      addToWait(combatRound(1));
      setPosition(POSITION_SITTING);

      if (!horse->isPc())
        dynamic_cast<TMonster *>(horse)->aiHorse(this);

      return TRUE;
    }
#endif
    if (bSuccess(getRideMod(this, dynamic_cast<TBeing *>(horse)), SKILL_RIDE)) {
      if (!silent) {
        if (horse->hasSaddle() == 1 && !horse->rider) {
          act("You hop into the saddle and start riding $N.", FALSE, this, 0,
              horse, TO_CHAR);
          act("$n hops into the saddle and starts riding $N.", FALSE, this, 0,
              horse, TO_NOTVICT);
          act("$n hops on your back!", FALSE, this, 0, horse, TO_VICT);
        } else if (!horse->rider) {
          act("You start riding $N.", FALSE, this, 0, horse, TO_CHAR);
          act("$n starts riding $N.", FALSE, this, 0, horse, TO_NOTVICT);
          act("$n hops on your back!", FALSE, this, 0, horse, TO_VICT);
        } else {
          act("You start riding $N's $o.", FALSE, this, horse,
              horse->horseMaster(), TO_CHAR);
          act("$n starts riding $N's $o.", FALSE, this, horse,
              horse->horseMaster(), TO_NOTVICT);
          act("$n hops on your $o's back!", FALSE, this, horse,
              horse->horseMaster(), TO_VICT);
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
        dynamic_cast<TMonster *>(horse)->setTarg(NULL);
      dynamic_cast<TMonster *>(horse)->hates.clist = NULL;
      dynamic_cast<TMonster *>(horse)->fears.clist = NULL;

    } else {
      act("You try to ride $N, but fail and fall on your butt.", FALSE, this, 0,
          horse, TO_CHAR);
      act("$n tries to ride $N, but fails and falls on $s butt.", FALSE, this,
          0, horse, TO_NOTVICT);
      act("$n tries to ride you, but fails and falls on $s butt.", FALSE, this,
          0, horse, TO_VICT);
      setPosition(POSITION_SITTING);
      addToWait(combatRound(2));
      if (!horse->isPc())
        dynamic_cast<TMonster *>(horse)->aiHorse(this);
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
        (MobCountInRoom(roomp->stuff) >= roomp->getMoblim())) {
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
        sendTo("It would be a poor idea to leave your flying mount while in "
               "mid-air, without the ability to fly yourself.\n\r");
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
      } else if (::number(-10, getSkillValue(SKILL_RIDE_WINGED)) > 0) {
        if (!silent) {
          act("You coax $N to land so you can dismount.", TRUE, this, NULL,
              horse, TO_CHAR);
          act("$n coaxes you into landing, you feel charmed and comply.", TRUE,
              this, NULL, horse, TO_VICT);
          act("$n coaxes $N into landing.", TRUE, this, NULL, horse,
              TO_NOTVICT);
        }
        horse->doLand();

        if (!silent) {
          act("You dismount from $N.", FALSE, this, 0, horse, TO_CHAR);
          act("$n dismounts from $N.", FALSE, this, 0, horse, TO_NOTVICT);
          act("$n dismounts from you.", FALSE, this, 0, horse, TO_VICT);
        }
        dismount(POSITION_STANDING);
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
  vlogf(LOG_BUG, format("Undefined call to doMount.  cmd = %d") % cmd);
  return TRUE;
}

int TBeing::getRideMod(TBeing *rider, TBeing *mount) {
  int mod = 0;

  // Deikhan class bonuses for rider
  if (rider->hasClass(CLASS_DEIKHAN)) {
    mod += 6;
    if (rider->bSuccess(SKILL_ADVANCED_RIDING)) {
      mod += rider->getClassLevel(CLASS_DEIKHAN) / 2;
    }
  }



  // Saddle bonuses
  if (mount && mount->hasSaddle()) {
    mod += 10;
    if (rider->bSuccess(SKILL_ADVANCED_RIDING)) {
      mod += 10;
        if (rider->bSuccess(SKILL_SADDLE_POSTURE)) {
          mod += rider->getSkillValue(SKILL_SADDLE_POSTURE) / 5;
        }
    }
  }
  
  // Combat state modifiers for the rider
  if (rider->isTanking()) {
    mod -= 4;
    if (rider->hasClass(CLASS_DEIKHAN)) {
      mod += 5;  // net +1 for tanking deikhans
    }
  }
  
  if (rider->isAffected(AFF_ENGAGER)) {
    mod -= 3;
  }
  
  if (rider->fight()) {
    mod -= 3;
    if (rider->hasClass(CLASS_DEIKHAN)) {
      mod += 4;  // net +1 for fighting deikhans
    }
  }
  
  if (rider->doesKnowSkill(mount->mountSkillType()) && rider->bSuccess(mount->mountSkillType())) {
  mod += getSkillValue(mount->mountSkillType()) / 5;  
  }
 

  // Stat reaction bonuses for rider
  mod += rider->getAgiReaction();
  if (rider->getAgiReaction() > 0 && rider->bSuccess(SKILL_ADVANCED_RIDING)) {
    mod += rider->getAgiReaction();  // double bonus if advanced riding
  }
  
  mod += rider->getBraReaction();
  if (rider->getBraReaction() > 0 && rider->bSuccess(SKILL_ADVANCED_RIDING)) {
    mod += rider->getBraReaction();  // double bonus if advanced riding
  }

  if (rider->getCond(DRUNK) > 5) {
    mod -= 5;
    if (!rider->bSuccess(SKILL_ALCOHOLISM)) {
      mod -= 10;
    }
  }

  // Rider leg injuries
  if (rider->eitherLegHurt()) {
    mod -= 10;
    if (rider->bothLegsHurt()) {
      mod -= 10; // total -20 for both legs
    }
  }

  // Rider exhaustion
  if (rider->getMove() <= 0) { // exhausted
    mod -= 20;
  }

  // Mount condition checks
  if (mount) {
    if (mount->eitherLegHurt()) {
      mod -= 20;
      if (mount->bothLegsHurt()) {
        mod -= 20; // total -40 for mount both legs
      }
    }

    if (mount->getMove() <= 0) { // mount exhausted
      mod -= 60;
    }
  }


  return mod;
}

int TBeing::rideCheck(int mod) {
  if (isImmortal())
    return TRUE;

  TBeing *rider = this;
  TBeing *mount = dynamic_cast<TBeing *>(riding);

  mod += getRideMod(rider, mount); // Get base modifiers for rider

  // Riding skill check to trigger learning
  bSuccess(mod, SKILL_RIDE);

  // Movement-specific riding checks with messages
  if (rider->bothLegsHurt()) {
    rider->sendTo(COLOR_MOBS,
                  format("Riding %s without working legs is painful!\n\r") %
                      riding->getName());
  }

  if (mount && mount->bothLegsHurt()) {
    act("$N has no working legs to transport you.", FALSE, rider, 0, mount,
        TO_CHAR);
    return FALSE;
  }

  if (rider->eitherLegHurt()) {
    rider->sendTo("A damaged leg or foot makes it tough to ride!\n\r");
  }

  // Weight check - mount collapses if overloaded
  if (riding && !mount->isStrong() &&
      (compareWeights(riding->getWeight(), rider->getTotalWeight(TRUE)) == 1)) {
    act("$N collapses beneath your weight.", FALSE, rider, 0, riding, TO_CHAR);
    act("$N collapses beneath $n's weight.", FALSE, rider, 0, riding,
        TO_NOTVICT);
    act("You collapse beneath $n's weight.", FALSE, rider, 0, riding, TO_VICT);
    TBeing *tbr = dynamic_cast<TBeing *>(riding);
    if (tbr) {
      tbr->setMove(0);
    }
    rider->dismount(POSITION_SITTING);
    if (rider->reconcileDamage(rider, ::number(0, 1), DAMAGE_NORMAL) == -1)
      return FALSE; // rider died
    return FALSE;
  }

  // Drunk riding check
  if (rider->getCond(DRUNK) > 9) {
    rider->sendTo("You wobble drunkenly as your mount moves along.\n\r");
    if (!rider->bSuccess(SKILL_ALCOHOLISM)) {
      rider->sendTo(
          "Ooops, one of those purple elephants you keep seeing must have "
          "pushed you off.\n\r");
      int crashDam = fallOffMount(riding, false);
          if (crashDam == -1) {
            return DELETE_THIS;
        }

        if (crashDam > 0) {
          if (reconcileDamage(this, crashDam, DAMAGE_FALL) == -1) {
            return DELETE_THIS;
          }
        }
      return FALSE;
    }
  }

  // Rider exhaustion check
  if (rider->getMove() < 1 && !rider->isBrawny()) {
    act("You're too tired to stay on your $o.", false, rider, riding, 0,
        TO_CHAR);
    int crashDam = fallOffMount(riding, false);
          if (crashDam == -1) {
            return DELETE_THIS;
        }

        if (crashDam > 0) {
          if (reconcileDamage(this, crashDam, DAMAGE_FALL) == -1) {
            return DELETE_THIS;
          }
        }
    return FALSE;
  }

  // Incapacitated states - automatic dismount
  if (rider->getPosition() <= POSITION_SLEEPING ||
      rider->isAffected(AFF_PARALYSIS)) {
    rider->dismount(POSITION_SITTING);
    return FALSE;
  }

  // Final riding skill checks
  // If rider and mount are not tanking, allow check with advantage
  if (!rider->isTanking() && !mount->isTanking()) {
    if (!rider->bSuccess(mod, SKILL_RIDE) &&
        !rider->bSuccess(mod, SKILL_RIDE)) {
      int crashDam = rider->fallOffMount(mount, false);
      if (crashDam == -1) {
        return DELETE_THIS;
      }
      if (crashDam > 0) {
        if (rider->reconcileDamage(rider, crashDam, DAMAGE_FALL) == -1) {
          return DELETE_THIS;
        }
      }
      return FALSE;
    }
  } else {
    if (!rider->bSuccess(mod, SKILL_RIDE)) {
      int crashDam = rider->fallOffMount(mount, false);
      if (crashDam == -1) {
        return DELETE_THIS;
      }
      if (crashDam > 0) {
        if (rider->reconcileDamage(rider, crashDam, DAMAGE_FALL) == -1) {
          return DELETE_THIS;
        }
      }
      return FALSE;
    }
  }
  return TRUE;
}

int TThing::fallOffMount(TThing *, bool) {
  dismount(POSITION_SITTING);
  return 0;
}

// returns damage amount or -1 for DELETE_THIS
int TBeing::fallOffMount(TThing *h, bool force) {
  TBeing *horse = dynamic_cast<TBeing *>(h);
  TObj *chair = dynamic_cast<TObj *>(h);

  // Check if character is incapacitated (can't use skills)
  bool outCold = isAffected(AFF_SLEEP) || isAffected(AFF_PARALYSIS);

  if (!outCold && !this->isTanking() && this->bSuccess(SKILL_RIDE)) {
    return 0; // No fall, no damage
  }

  if (!outCold && horse && this->doSaddlePosture(this, horse)) {
    return 0; // No fall, no damage
  }
  if (horse) {
    act("$n loses control and falls off of $N.", FALSE, this, 0, horse,
        TO_NOTVICT, ANSI_RED);
    act("$n loses control and falls off of you.", FALSE, this, 0, horse,
        TO_VICT, ANSI_RED);
    act("You lose control and fall off of $N.", FALSE, this, 0, horse, TO_CHAR,
        ANSI_RED);

    if (!horse->isPc())
      dynamic_cast<TMonster *>(horse)->aiHorse(this);

    // Calculate height modifier before dismounting
    int heightMod = 0;
    if (horse) {
      heightMod = (horse->getHeight() * 3) / 4;  // 3/4 of mount's height
      if (horse->isFlying()) {
        heightMod += 100; // Flying mount bonus
      }
    }

    // Dismount from the mount
    dismount(POSITION_STANDING);

    // If the character can fly independently, give them a chance to recover
    if (!outCold && canFly() && isAgile(0)) {
      setPosition(POSITION_FLYING);
      sendTo("You recover your flight as you fall from your mount!\n\r");
      act("$n recovers $s flight as $e falls from $s mount!", TRUE, this, 0, 0, TO_ROOM);
      return 0; // No crash damage
    }

    // Use crash landing with mount's height (it will handle fall sectors internally)
    int crashDam = crashLanding(heightMod, force);
    if (crashDam == -1)
      return -1; // Signal deletion
    return crashDam;
  } else if (chair) {
    // Standard fall off chair messages
    act("$n loses $s balance and falls off of $p.", FALSE, this, chair, 0,
        TO_ROOM, ANSI_RED);
    act("You lose your balance and fall off of $p.", FALSE, this, chair, 0,
        TO_CHAR, ANSI_RED);

    // Dismount from chair (let crashLanding determine final position)
    dismount(POSITION_STANDING);

    // Chairs/objects don't have height, so minimal fall damage
    int crashDam = crashLanding(0, force);
    if (crashDam == -1)
      return -1; // Signal deletion
    return crashDam;
  }

  if (spelltask)
    setDistracted(-1, FALSE);

  return 0; // No mount found, no damage
}

// ego is a number representing how "ballsy" the mount is
// > 5  == mount will attack
// 0 - 4  == will buck off
// negative == accepts rider
int MountEgoCheck(TBeing *ch, TBeing *horse) {
  int check;

  if (horse->getPosition() <= POSITION_STUNNED)
    return -10;

  if (horse->isDragonRideable()) {
    check = horse->GetMaxLevel();
    check += dynamic_cast<TMonster *>(horse)->anger() / 10;
    check -= ch->GetMaxLevel();

    if (ch->doesKnowSkill(SKILL_RIDE))
      check -= ch->getSkillValue(SKILL_RIDE) / 10;
    // Bonus for proficiency in advanced riding disc.
    if (ch->doesKnowSkill(SKILL_ADVANCED_RIDING))
      check -= ch->getSkillValue(SKILL_ADVANCED_RIDING) / 8;

    // Bonus for proficiency in winged riding disc.
    // Safe to assume dragons are winged (?)
    if (ch->doesKnowSkill(SKILL_RIDE_WINGED) &&
        horse->mountSkillType() == SKILL_RIDE_WINGED)
      check -= ch->getSkillValue(SKILL_RIDE_WINGED) / 6;

    check +=
        number((int)horse->plotStat(STAT_CURRENT, STAT_PER, 1.5, 9.0, 5.0),
               (int)horse->plotStat(STAT_CURRENT, STAT_PER, 4.5, 27.0, 15.0));
    check +=
        number((int)horse->plotStat(STAT_CURRENT, STAT_FOC, 1.5, 9.0, 5.0),
               (int)horse->plotStat(STAT_CURRENT, STAT_FOC, 4.5, 27.0, 15.0));
    check -= number((int)ch->plotStat(STAT_CURRENT, STAT_CHA, 1.5, 9.0, 5.0),
                    (int)ch->plotStat(STAT_CURRENT, STAT_CHA, 4.5, 27.0, 15.0));
    if (horse->getPosition() <= POSITION_SLEEPING)
      check -= 2;
#if 0
    if (ch->isSameAlign(horse))
      check -= 2;
    else
      check += 2;
#endif
    check *= max(0, horse->getHit());
    check /= max(1, (int)horse->hitLimit());
    check *= ch->hitLimit();
    check /= max(1, ch->getHit());
    return (check);
  } else {
    check = horse->GetMaxLevel();
    check += dynamic_cast<TMonster *>(horse)->anger() / 30;
    check -= ch->GetMaxLevel();

    if (ch->doesKnowSkill(SKILL_RIDE))
      check -= ch->getSkillValue(SKILL_RIDE) / 10;

    // Bonus for proficiency in advanced riding disc.
    if (ch->doesKnowSkill(SKILL_ADVANCED_RIDING))
      check -= ch->getSkillValue(SKILL_ADVANCED_RIDING) / 8;

    // Bonus for proficiency in type-specific mount ability
    for (const auto skill : {SKILL_RIDE_WINGED, SKILL_RIDE_DOMESTIC,
                             SKILL_RIDE_NONDOMESTIC, SKILL_RIDE_EXOTIC}) {
      if (horse->mountSkillType() == skill && ch->doesKnowSkill(skill)) {
        check -= ch->getSkillValue(skill) / 6;
        break;
      }
    }

    check +=
        number((int)horse->plotStat(STAT_CURRENT, STAT_PER, 1.5, 9.0, 5.0),
               (int)horse->plotStat(STAT_CURRENT, STAT_PER, 4.5, 27.0, 15.0));
    check +=
        number((int)horse->plotStat(STAT_CURRENT, STAT_FOC, 1.5, 9.0, 5.0),
               (int)horse->plotStat(STAT_CURRENT, STAT_FOC, 4.5, 27.0, 15.0));
    check -= number((int)ch->plotStat(STAT_CURRENT, STAT_CHA, 1.5, 9.0, 5.0),
                    (int)ch->plotStat(STAT_CURRENT, STAT_CHA, 4.5, 27.0, 15.0));
    if (horse->getPosition() <= POSITION_SLEEPING)
      check -= 2;
    check *= max(0, horse->getHit());
    check /= max(1, (int)horse->hitLimit());
    check *= ch->hitLimit();
    check /= max(1, ch->getHit());
    return (check);
  }
}

TThing *TThing::horseMaster(void) const {
  TThing *t;

  // locate the last valid "rider"
  for (t = rider; t && t->nextRider; t = t->nextRider)
    ;

  return t;
}

int TBeing::getNumRiders(TThing *ch) const {
  TThing *t;
  int num = 0;

  for (t = rider; t; t = t->nextRider) {
    if (t == ch)
      continue;
    if (t->getHeight() > getHeight() * LARGE_RIDER_THRESHOLD_PERCENT / 100)
      num += 2;
    else
      num++;
  }
  return num;
}

int TBeing::getMaxRiders() const {
  // leave this fixed.
  // we can put up to 4 very small (kids) people on the horse
  // a kid essentially counts as 1 slot, a man as 2
  return MAX_RIDERS;
}

int TBeing::getRiderHeight() const {
  return (RIDER_HEIGHT_PERCENT * getHeight() / 100);
}
