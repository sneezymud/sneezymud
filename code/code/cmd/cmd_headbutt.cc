//////////////////////////////////////////////////////////////////////////
//
// SneezyMUD - All rights reserved, SneezyMUD Coding Team
//
// cmd_headbutt.cc : The headbutt command
//
//////////////////////////////////////////////////////////////////////////

#include "handler.h"
#include "extern.h"
#include "room.h"
#include "being.h"
#include "combat.h"
#include "enum.h"
#include "skill_handler.h"
#include "obj_general_weapon.h"

bool TBeing::canHeadbutt(TBeing* victim, const silentTypeT silent) const {
  // Define static vector of tests that need to pass before headbutt can
  // execute
  static const SkillHandler::Tests tests = []() -> SkillHandler::Tests {
    using namespace SkillHandler;

    // Custom test needed by headbutt
    static const auto ch_valid_height = SkillHandler::custom(
      [](const SkillHandler::Config& config) {
        return config.ch->getPosHeight() * 0.75 <=
               config.target->getPosHeight();
      },
      make_simple_act("Your head is much higher than theirs."));

    return {
      ch_not_busy(),
      ch_knows_skill("You know nothing about headbutting."),
      ch_has_valid_body_type(
        {BODY_HUMANOID, BODY_OWLBEAR, BODY_MINOTAUR, BODY_ORB},
        "You lack the proper body form to headbutt."),
      not_peaceful_room(),
      target_not_on_furniture("You can't headbutt $N while $E is on $p!"),
      target_not_self("Aren't we funny today..."),
      ch_not_mounted("You can't butt heads while mounted!"),
      ch_can_harm_target(),
      target_not_immortal("You can't butt an immortal."),
      target_not_flying("") ||
        ch_is_flying("You can't headbutt something that is flying."),
      ch_valid_height(),
    };
  }();

  SkillHandler::Config config{SKILL_HEADBUTT, this, victim, silent};
  return SkillHandler::run_tests(config, tests);
}

int TBeing::headbuttMiss(TBeing* v) {
  if (v->doesKnowSkill(SKILL_COUNTER_MOVE) || isCombatMode(ATTACK_BERSERK)) {
    // I don't understand this logic
    act("$N deftly avoids $n's headbutt.", FALSE, this, 0, v, TO_NOTVICT);
    act("$N deftly avoids your headbutt.", FALSE, this, 0, v, TO_CHAR);
    act("You deftly avoid $n's headbutt.", FALSE, this, 0, v, TO_VICT);
  } else {
    act("$N moves $S head out of the way, causing $n to miss.", false, this, 0,
      v, TO_NOTVICT);
    act("$N moves $S head out of the way, causing you to miss.", false, this, 0,
      v, TO_CHAR);
    act("You move your head out of the way, causing $n to miss.", false, this,
      0, v, TO_VICT);

    int rc = stumble();
    if (IS_SET_DELETE(rc, DELETE_THIS))
      return rc;
  }
  reconcileDamage(v, 0, SKILL_HEADBUTT);

  return TRUE;
}

int TBeing::headbuttHit(TBeing* victim) {
  int rc;
  int hgt;
  spellNumT dam_type = SKILL_HEADBUTT;
  wearSlotT pos;

  int dam = getSkillDam(victim, SKILL_HEADBUTT, getSkillLevel(SKILL_HEADBUTT),
    getAdvLearning(SKILL_HEADBUTT));

  hgt = getHeight();

  if (hgt < victim->getPartMinHeight(ITEM_WEAR_FEET)) {
    dam_type = DAMAGE_HEADBUTT_FOOT;
    act("You try to headbutt $N's foot, but $E is just too high up right now.",
      FALSE, this, 0, victim, TO_CHAR);
    act("$n tries to headbutt your foot, but fails.", FALSE, this, 0, victim,
      TO_VICT);
    act("$n tries to headbutt $N's foot, but fails.", FALSE, this, 0, victim,
      TO_NOTVICT);

    if ((rc = reconcileDamage(victim, 0, dam_type)) == -1)
      return DELETE_VICT;

    return TRUE;
  } else {
    static constexpr const char* headbutt_msg =
      "%s headbutt %s, slamming %s head into %s %s.";

    const char* target_area;
    bool causes_wait = false;

    if (hgt < victim->getPartMinHeight(ITEM_WEAR_LEGS)) {
      pos = (::number(0, 1) ? WEAR_FOOT_L : WEAR_FOOT_R);
      dam_type = DAMAGE_HEADBUTT_FOOT;
      target_area = "foot";
    } else if (hgt < victim->getPartMinHeight(ITEM_WEAR_WAIST)) {
      pos = (::number(0, 1) ? WEAR_LEG_L : WEAR_LEG_R);
      dam_type = DAMAGE_HEADBUTT_LEG;
      target_area = "leg";
    } else if (hgt < victim->getPartMinHeight(ITEM_WEAR_BODY)) {
      pos = WEAR_WAIST;
      dam_type = DAMAGE_HEADBUTT_CROTCH;
      target_area = "crotch";
      causes_wait = true;
    } else if (hgt < victim->getPartMinHeight(ITEM_WEAR_ARMS)) {
      pos = WEAR_BODY;
      dam_type = DAMAGE_HEADBUTT_BODY;
      target_area = "solar plexus";
      causes_wait = true;
    } else if (hgt < victim->getPartMinHeight(ITEM_WEAR_NECK)) {
      pos = WEAR_NECK;
      dam_type = DAMAGE_HEADBUTT_THROAT;
      target_area = "throat";
      causes_wait = true;
    } else if (hgt < victim->getPartMinHeight(ITEM_WEAR_HEAD)) {
      pos = WEAR_HEAD;
      dam_type = DAMAGE_HEADBUTT_JAW;
      target_area = "jaw";
      causes_wait = true;
    } else {
      pos = WEAR_HEAD;
      dam_type = DAMAGE_HEADBUTT_SKULL;
      target_area = "skull";
      causes_wait = true;
    }

    act(format(headbutt_msg) % "$n" % "$N" % "$s" % "$N's" % target_area, FALSE,
      this, 0, victim, TO_NOTVICT);
    act(format(headbutt_msg) % "You" % "$N" % "your" % "$S" % target_area,
      FALSE, this, 0, victim, TO_CHAR);
    act(format(headbutt_msg) % "$n" % "you" % "$s" % "your" % target_area,
      FALSE, this, 0, victim, TO_VICT);

    if (causes_wait) {
      victim->addToWait(combatRound(0.25));
    }
  }

  // Use impactSpec to handle all impact effects (spikes, thorns, hardness)
  dam += impactSpec(this, victim, WEAR_HEAD, pos);
  if ((rc = reconcileDamage(victim, dam, dam_type)) == -1)
    return DELETE_VICT;

  return TRUE;
}

int TBeing::headbutt(TBeing* victim) {
  int rc;
  const int HEADBUTT_MOVE = 6;

  if (!canHeadbutt(victim, SILENT_NO))
    return FALSE;

  if (getMove() < HEADBUTT_MOVE) {
    sendTo("You lack the vitality.\n\r");
    return FALSE;
  }
  addToMove(-HEADBUTT_MOVE);

  int bKnown = getSkillValue(SKILL_HEADBUTT);
  int successfulHit = specialAttack(victim, SKILL_HEADBUTT);
  int successfulSkill = bSuccess(bKnown, SKILL_HEADBUTT);

  // keep bSucc at end so counters are OK
  if (!victim->awake() || (successfulSkill && successfulHit &&
                            successfulHit != GUARANTEED_FAILURE &&
                            !victim->canCounterMove(bKnown * 2 / 5) &&
                            !victim->canFocusedAvoidance(bKnown * 2 / 5))) {
    return (headbuttHit(victim));
  } else {
    rc = headbuttMiss(victim);
    if (IS_SET_DELETE(rc, DELETE_THIS) || IS_SET_DELETE(rc, DELETE_VICT))
      return rc;
  }
  return TRUE;
}

int TBeing::doHeadbutt(const char* argument, TBeing* vict) {
  int rc;
  TBeing* v;
  char name_buf[256];

  strcpy(name_buf, argument);

  if (!(v = vict)) {
    if (!(v = get_char_room_vis(this, name_buf))) {
      if (!(v = fight())) {
        sendTo("Butt whose head?\n\r");
        return FALSE;
      }
    }
  }
  if (!sameRoom(*v)) {
    sendTo("That person isn't around.\n\r");
    return FALSE;
  }
  rc = headbutt(v);
  if (rc)
    addSkillLag(SKILL_HEADBUTT, rc);

  if (IS_SET_DELETE(rc, DELETE_VICT)) {
    if (vict)
      return rc;
    delete v;
    v = NULL;
    REM_DELETE(rc, DELETE_VICT);
  }
  return rc;
}
