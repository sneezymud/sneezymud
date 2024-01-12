#include <boost/format.hpp>
#include <initializer_list>
#include <list>
#include <memory>

#include "ansi.h"
#include "being.h"
#include "combat.h"
#include "comm.h"
#include "defs.h"
#include "enum.h"
#include "extern.h"
#include "handler.h"
#include "monster.h"
#include "parse.h"
#include "race.h"
#include "room.h"
#include "sound.h"
#include "spells.h"
#include "sstring.h"
#include "structs.h"
#include "thing.h"

extern void startChargeTask(TBeing*, const char*);

static int charge(TBeing* ch, TBeing* vict) {
  TThing* c = nullptr;
  TBeing* tb;
  int rc;

  TMonster* mount = dynamic_cast<TMonster*>(ch->riding);
  if (!mount || (ch->getPosition() != POSITION_MOUNTED)) {
    ch->sendTo("You must be mounted to charge!\n\r");
    return false;
  }

  if (mount->getRace() != RACE_HORSE &&
      (!ch->doesKnowSkill(mount->mountSkillType()) ||
        ch->advancedRidingBonus(mount) < 50)) {
    ch->sendTo("You lack the skill to charge with this mount.\n\r");
    return false;
  }
  if (ch == vict) {
    ch->sendTo("It is impossible to charge yourself!\n\r");
    return false;
  }
  if (vict == mount) {
    ch->sendTo("You order your mount to charge into itself.\n\r");
    return false;
  }
  if (vict->riding == mount) {
    // we are both on same horse
    act("Now how is your $o going to charge someone that is riding it?", false,
      ch, mount, nullptr, TO_CHAR);
    return false;
  }
  if (mount->horseMaster() != ch) {
    act("You are not in control of $p and can't order it to charge.", false, ch,
      mount, nullptr, TO_CHAR);
    return false;
  }
  if (!mount->hasLegs()) {
    act("You can't charge on a legless $o!", false, ch, mount, nullptr, TO_CHAR);
    return false;
  }
  if (mount->eitherLegHurt()) {
    act("Your $o's injury prevents you from charging!", false, ch, mount, nullptr,
      TO_CHAR);
    return false;
  }
  if (!mount->isFlying() && vict->isFlying()) {
    act("That would be hard, considering $N is flying, and your $o is not.",
      false, ch, mount, vict, TO_CHAR);
    return false;
  }
  if (ch->checkPeaceful(
        "This room is too peaceful to contemplate violence in.\n\r"))
    return false;

  // if there are a lot of attackers, just plain deny
  if (vict->attackers > 4) {
    act("Too many people are fighting $N.  Charging is prohibited.", false, ch,
      0, vict, TO_CHAR);
    return false;
  }
  // otherwise, allow the charge provided all the attackers are working together
  for (StuffIter it = vict->roomp->stuff.begin();
       it != vict->roomp->stuff.end(); ++it) {
    c = *it;
    TBeing* tbt = dynamic_cast<TBeing*>(c);
    if (!tbt)
      continue;
    if (tbt == ch || tbt == vict)
      continue;

    if (tbt->fight() == vict) {
      if (!tbt->inGroup(*ch)) {
        act("An innocent $o in the vicinity of $N prevents you from charging!",
          false, ch, tbt, vict, TO_CHAR);
        return false;
      }
    }
  }

  // very hard to tank and charge. Charge should be opening move
  if ((vict->getPosition() > POSITION_SITTING) && ::number(0, 2) &&
      ch->isTanking()) {
    ch->sendTo(COLOR_MOBS,
      format("You try to get in position to take a charge at %s.\n\r") %
        vict->getName());
    ch->sendTo(COLOR_MOBS, format("However, %s stays close to you.\n\rYou "
                                  "can't get the space needed to charge!\n\r") %
                             vict->getName());
    ch->cantHit += ch->loseRound(1);
    return false;
  }

  if (ch->fight())
    ch->cantHit += ch->loseRound(5);
  else
    ch->cantHit += ch->loseRound(3);

  soundNumT snd = pickRandSound(SOUND_HORSE_1, SOUND_HORSE_2);
  ch->roomp->playsound(snd, SOUND_TYPE_NOISE);

  auto chOriented = ch->isAffected(AFF_ORIENT);
  int bKnown = ch->getSkillValue(SKILL_CHARGE);
  int successfulSkill = ch->bSuccess(bKnown, SKILL_CHARGE);
  // Guaranteed hit if affected by orient
  int successfulHit = chOriented ? true : ch->specialAttack(vict, SKILL_CHARGE);

  // Failure case
  if (vict->awake() &&
      (!successfulHit || successfulSkill == GUARANTEED_FAILURE ||
        successfulSkill == FAILURE)) {
    act("You charge $N, but $E dodges to the side at the last moment.", true,
      ch, 0, vict, TO_CHAR);
    act(
      "$n and $s mount come charging at you.\n\rFortunately you were able to "
      "dodge them.",
      true, ch, 0, vict, TO_VICT);
    act(
      "$n and $s mount charge down upon $N.\n\rBut $E was able to dodge them.",
      true, ch, 0, vict, TO_NOTVICT);

    for (StuffIter it = vict->roomp->stuff.begin();
         it != vict->roomp->stuff.end() && (c = *it); ++it) {
      if (c == vict || c == ch)
        continue;

      tb = dynamic_cast<TBeing*>(c);
      if (!tb)
        continue;
      // don't have mount scatter
      if (ch->riding == tb)
        continue;
      if ((tb->fight() == vict) && (tb != mount)) {
        // we have already validated that all attackers are in ch's group
        act("You scatter as $N charges!", false, tb, 0, ch, TO_CHAR);
        act("$n scatters as you charge!", false, tb, 0, ch, TO_VICT);
        act("$n scatters as $N charges!", false, tb, 0, ch, TO_NOTVICT);
        tb->loseRound(2);
      }
    }

    ch->reconcileDamage(vict, 0, SKILL_CHARGE);
    return true;
  }

  // Success case
  int dam = ch->getSkillDam(vict, SKILL_CHARGE, ch->getSkillLevel(SKILL_CHARGE),
    ch->getAdvLearning(SKILL_CHARGE));

  dam /= (successfulSkill == COMPLETE_SUCCESS) ? 1 : 2;

  TThing* prim = ch->heldInPrimHand();
  if (prim && (!(::number(0, 25)) || chOriented)) {
    act("A split second before the charge you brace your $o to strike.", true,
      ch, prim, vict, TO_CHAR);
    act("$n braces $s $o in preparation for the strike.", true, ch, prim, vict,
      TO_VICT);
    act("$n braces $s $o in preperation for $s charge at $N.", true, ch, prim,
      vict, TO_NOTVICT);

    if (chOriented)
      REMOVE_BIT(ch->specials.affectedBy, AFF_ORIENT);

    dam += ::number(5, ch->GetMaxLevel() + bKnown);
  }

  const int WEIGHT_SCALING_CONSTANT = 4000;
  int shockKnown = ch->getSkillValue(SKILL_SHOCK_CAVALRY);
  int successfulShockSkill = ch->bSuccess(shockKnown, SKILL_SHOCK_CAVALRY);
  int successfulShockHit = ch->specialAttack(vict, SKILL_SHOCK_CAVALRY);
  int ridingSkillBonus = 60;

  // Set the scaling bonus from base of 60% up to 110%, based on advanced riding
  // skill
  for (const auto skill : {SKILL_RIDE_WINGED, SKILL_RIDE_DOMESTIC,
         SKILL_RIDE_NONDOMESTIC, SKILL_RIDE_EXOTIC})
    if (mount->mountSkillType() == skill && ch->doesKnowSkill(skill) &&
        ch->bSuccess(skill))
      ridingSkillBonus += ch->getSkillValue(skill) / 2;

  if (shockKnown && successfulShockSkill && successfulShockHit) {
    dam += (dam * (mount->getWeight() + mount->getCarriedWeight()) /
             WEIGHT_SCALING_CONSTANT) *
           ridingSkillBonus / 100;

    act("You charge $N, trampling $M with an especially mighty blow!", true, ch,
      0, vict, TO_CHAR);
    act("$n and $s mount trample you with a mighty charge!", true, ch, 0, vict,
      TO_VICT);
    act("$n and $s mount trample $N with a mighty charge!", true, ch, 0, vict,
      TO_NOTVICT);
  } else {
    act("You charge $N, striking $M with a mighty blow.", true, ch, 0, vict,
      TO_CHAR);
    act("$n and $s mount come charging at you.", true, ch, 0, vict, TO_VICT);
    act("$n and $s mount charge down upon $N.", true, ch, 0, vict, TO_NOTVICT);
  }

  for (StuffIter it = vict->roomp->stuff.begin();
       it != vict->roomp->stuff.end() && (c = *it); ++it) {
    if (c == vict || c == ch)
      continue;

    tb = dynamic_cast<TBeing*>(c);
    if (!tb)
      continue;
    if ((tb->fight() == vict) && (tb != mount)) {
      // we have already validated that all attackers are in ch's group
      act("You scatter as $N charges!", false, tb, 0, ch, TO_CHAR);
      act("$n scatters as you charge!", false, tb, 0, ch, TO_VICT);
      act("$n scatters as $N charges!", false, tb, 0, ch, TO_NOTVICT);
      tb->loseRound(2);
    }
  }

  if (vict->riding && dynamic_cast<TBeing*>(vict->riding)) {
    act("$n is heaved from $s mount and falls to the $g.", true, vict, 0, 0,
      TO_ROOM);
    act("You are knocked from your mount and dashed to the $g!", true, vict, 0,
      0, TO_CHAR);
    rc = vict->fallOffMount(vict->riding, POSITION_SITTING);
    if (IS_SET_DELETE(rc, DELETE_THIS))
      return DELETE_VICT;

    vict->addToWait(combatRound(1));
  } else if (vict->riding) {
    act("$n is heaved from $p and falls to the $g.", true, vict, vict->riding,
      0, TO_ROOM);
    act("You are knocked from $p and dashed to the $g!", true, vict,
      vict->riding, 0, TO_CHAR);
    rc = vict->fallOffMount(vict->riding, POSITION_SITTING);
    if (IS_SET_DELETE(rc, DELETE_THIS)) {
      return DELETE_VICT;
    }
  } else if ((c = vict->rider)) {
    act("$n is knocked from under you and you fall to the $g.", true, vict, 0,
      c, TO_VICT);
    act("You are battered by the blow and $N falls off you!", true, vict, 0, c,
      TO_CHAR);
    act("$n is stricken by the blow and $N falls off $m!", true, vict, 0, c,
      TO_NOTVICT);
    rc = c->fallOffMount(vict, POSITION_SITTING);
    vict->addToWait(combatRound(1));
    if (IS_SET_DELETE(rc, DELETE_THIS)) {
      delete c;
      c = nullptr;
    }
  } else {
    act("You are battered by the blow and trampled to the $g!", true, vict, 0,
      0, TO_CHAR);
    act("$n is battered by the blow and trampled to the $g!", true, vict, 0, 0,
      TO_ROOM);
    vict->setPosition(POSITION_SITTING);
    vict->addToWait(combatRound(1));
  }

  vict->cantHit += vict->loseRound(2);
  if (ch->reconcileDamage(vict, dam, SKILL_CHARGE) == -1)
    return DELETE_VICT;

  return true;
}

int TBeing::doCharge(const char* arg, TBeing* victim) {
  TBeing* vict;
  char tmp[80], tString[256];
  int rc = 0;
  dirTypeT Direction = DIR_NONE;

  if (checkBusy()) {
    return false;
  }
  if (!doesKnowSkill(SKILL_CHARGE)) {
    sendTo("You know nothing about charging.\n\r");
    return false;
  }
  half_chop(arg, tmp, tString);
  if (!victim) {
    Direction = getDirFromChar(tmp);

    if (Direction > DIR_NONE && Direction < MAX_DIR) {
      startChargeTask(this, arg);
      return false;
    }
  }
  if (!(vict = victim)) {
    if (!(vict = get_char_room_vis(this, tmp))) {
      if (!(vict = fight())) {
        sendTo("Charge whom?\n\r");
        return false;
      }
    }
  }
  if (noHarmCheck(vict))
    return false;

  if (!sameRoom(*vict)) {
    sendTo("That person isn't around.\n\r");
    return false;
  }

  rc = charge(this, vict);
  if (rc)
    addSkillLag(SKILL_CHARGE, rc);
  if (IS_SET_DELETE(rc, DELETE_VICT)) {
    if (victim)
      return rc;
    delete vict;
    vict = nullptr;
    REM_DELETE(rc, DELETE_VICT);
  }
  return rc;
}
