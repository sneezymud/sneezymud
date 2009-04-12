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

bool TBeing::canHeadbutt(TBeing *victim, silentTypeT silent)
{
  if (checkBusy()) {
    return FALSE;
  }
  if (!doesKnowSkill(SKILL_HEADBUTT)) {
    if (!silent)
      sendTo("You know nothing about headbutting.\n\r");
    return FALSE;
  }
  switch (race->getBodyType()) {
    case BODY_HUMANOID:
    case BODY_OWLBEAR:
    case BODY_MINOTAUR:
    case BODY_ORB:
      break;
    case BODY_KUOTOA:  // thick necks
    case BODY_OTYUGH:  // no real head
    default:
      if (!silent)
        sendTo("You lack the proper body form to headbutt.\n\r");
      return FALSE;
  }
  if (checkPeaceful("You feel too peaceful to contemplate violence.\n\r"))
    return FALSE;

  if (getCombatMode() == ATTACK_BERSERK) {
    if (!silent)
      sendTo("You are berserking! You can't focus enough to headbutt anyone!\n\r ");
    return FALSE;
  }
  if (victim->riding && !dynamic_cast<TBeing *> (victim->riding)) {
    act("You can't headbutt $N while $E is on $p!", TRUE, this, victim->riding, victim, TO_CHAR);
    return FALSE;
  }

  if (victim == this) {
    if (!silent)
      sendTo("Aren't we funny today...\n\r");
    return FALSE;
  }
  if (riding) {
    if (!silent)
      sendTo("You can't butt heads while mounted!\n\r");
    return FALSE;
  }
  if (noHarmCheck(victim))
    return FALSE;

  if (victim->isImmortal() || IS_SET(victim->specials.act, ACT_IMMORTAL)) {
    if (!silent)
      sendTo("You can't successfully butt an immortal.\n\r");
    return FALSE;
  }

  if ((int) (getPosHeight() * 0.9) > victim->getPosHeight()) {
    if (victim->getPosition() < POSITION_STANDING) {
      if (!silent)

        sendTo(format("That might work, but your victim seems to be on the %s.\n\r") % roomp->describeGround());
    } else {
      if (!silent)
        sendTo("Your head is much higher than theirs.\n\r");
    }
    return FALSE;
  }
  if (victim->isFlying()) {
    if (!silent)
      sendTo("You can't headbutt something that is flying.\n\r");
    return FALSE;
  }
  if (getPosition() == POSITION_CRAWLING) {
    if (!silent)
      sendTo("You can't headbutt while crawling.\n\r");
    return FALSE;
  }

  return true;
}

static int headbuttMiss(TBeing *c, TBeing *v)
{
  int rc;

  if (v->doesKnowSkill(SKILL_COUNTER_MOVE)) {
    // I don't understand this logic
    act("$N deftly avoids $n's headbutt.", FALSE, c, 0, v, TO_NOTVICT); 
    act("$N deftly avoids your headbutt.", FALSE, c, 0, v, TO_CHAR);
    act("You deftly avoid $n's headbutt.", FALSE, c, 0, v, TO_VICT);
  } else {
    act("$N avoids $n's headbutt.", FALSE, c, 0, v, TO_NOTVICT);
    act("$N moves $S head, and you fall down as you miss your headbutt.", FALSE, c, 0, v, TO_CHAR);
    act("$n tries to headbutt you, but you dodge it.", FALSE, c, 0, v, TO_VICT);
   
    rc = c->crashLanding(POSITION_SITTING);
    if (IS_SET_DELETE(rc, DELETE_THIS))
      return DELETE_THIS;

    rc = c->trySpringleap(v);
    if (IS_SET_DELETE(rc, DELETE_THIS) || IS_SET_DELETE(rc, DELETE_VICT))
      return rc;
  }
  c->reconcileDamage(v, 0,SKILL_HEADBUTT);

  return TRUE;
}

static int headbuttHit(TBeing *c, TBeing *victim)
{
  int rc;
  int h_dam = 1, spikeddam=0;
  int hgt;
  spellNumT dam_type = SKILL_HEADBUTT;
  wearSlotT pos;

  int dam = c->getSkillDam(victim, SKILL_HEADBUTT, c->getSkillLevel(SKILL_HEADBUTT), c->getAdvLearning(SKILL_HEADBUTT));

  hgt = c->getHeight();

  if (hgt < victim->getPartMinHeight(ITEM_WEAR_FEET)) {
    dam_type = DAMAGE_HEADBUTT_FOOT;
    act("You try to headbutt $N's foot, but $E is just too high up right now.",
          FALSE, c, 0, victim, TO_CHAR);
    act("$n tries to headbutt your foot, but fails.",
          FALSE, c, 0, victim, TO_VICT);
    act("$n tries to headbutt $N's foot, but fails.",
          FALSE, c, 0, victim, TO_NOTVICT);

    if ((rc = c->reconcileDamage(victim, 0,dam_type)) == -1)
      return DELETE_VICT;

    return TRUE;
  } else if (hgt < victim->getPartMinHeight(ITEM_WEAR_LEGS)) {
    pos = (::number(0,1) ? WEAR_FOOT_L : WEAR_FOOT_R);
    dam_type = DAMAGE_HEADBUTT_FOOT;
    act("$n headbutts $N, slamming $s head into $N's foot.", FALSE, c,
                  0, victim, TO_NOTVICT);
    act("You headbutt $N, slamming your head into $S foot.", FALSE, c,
                  0, victim, TO_CHAR);
    act("$n headbutts you, slamming $s head into your foot.", FALSE, c,
                  0, victim, TO_VICT);
  } else if (hgt < victim->getPartMinHeight(ITEM_WEAR_WAIST)) {
    pos = (::number(0,1) ? WEAR_LEG_L : WEAR_LEG_R);
    dam_type = DAMAGE_HEADBUTT_LEG;
    act("$n headbutts $N, slamming $s head into $N's leg.", FALSE, c,
                  0, victim, TO_NOTVICT);
    act("You headbutt $N, slamming your head into $S leg.", FALSE, c,
                  0, victim, TO_CHAR);
    act("$n headbutts you, slamming $s head into your leg.", FALSE, c,
                  0, victim, TO_VICT);
  } else if (hgt < victim->getPartMinHeight(ITEM_WEAR_BODY)) {
    pos = WEAR_WAIST;
    dam_type = DAMAGE_HEADBUTT_CROTCH;
    act("$n headbutts $N, slamming $s head into $N's crotch.", FALSE, c, 
                  0, victim, TO_NOTVICT);
    act("You headbutt $N, slamming your head into $S crotch.", FALSE, c, 
                  0, victim, TO_CHAR);
    act("$n headbutts you, slamming $s head into your crotch.", FALSE, c, 
                  0, victim, TO_VICT);
    victim->addToWait(combatRound(0.25));
  } else if (hgt < victim->getPartMinHeight(ITEM_WEAR_ARMS)) {
    pos = WEAR_BODY;
    dam_type = DAMAGE_HEADBUTT_BODY;
    act("$n headbutts $N, slamming $s head into $N's solar plexus.", FALSE, c, 
                  0, victim, TO_NOTVICT);
    act("You headbutt $N, slamming your head into $S solar plexus.", FALSE, c, 
                  0, victim, TO_CHAR);
    act("$n headbutts you, slamming $s head into your solar plexus.", FALSE, c, 
                  0, victim, TO_VICT);
    victim->addToWait(combatRound(0.25));
  } else if (hgt < victim->getPartMinHeight(ITEM_WEAR_NECK)) {
    pos = WEAR_NECK;
    dam_type = DAMAGE_HEADBUTT_THROAT;
    act("$n headbutts $N, slamming $s head into $N's throat.", FALSE, c, 
                  0, victim, TO_NOTVICT);
    act("You headbutt $N, slamming your head into $S throat.", FALSE, c, 
                  0, victim, TO_CHAR);
    act("$n headbutts you, slamming $s head into your throat.", FALSE, c, 
                  0, victim, TO_VICT);
    victim->addToWait(combatRound(0.25));
  } else if (hgt < victim->getPartMinHeight(ITEM_WEAR_HEAD)) {
    pos = WEAR_HEAD;
    dam_type = DAMAGE_HEADBUTT_JAW;
    act("$n headbutts $N, slamming $s head into $N's jaw.", FALSE, c, 
                  0, victim, TO_NOTVICT);
    act("You headbutt $N, slamming your head into $S jaw.", FALSE, c, 
                  0, victim, TO_CHAR);
    act("$n headbutts you, slamming $s head into your jaw.", FALSE, c, 
                  0, victim, TO_VICT);
    victim->addToWait(combatRound(0.25));
  } else {
    pos = WEAR_HEAD;
    dam_type = DAMAGE_HEADBUTT_SKULL;
    act("$n headbutts $N, slamming $s head into $N's skull.", FALSE, c, 
                  0, victim, TO_NOTVICT);
    act("You headbutt $N, slamming your head into $S skull.", FALSE, c, 
                  0, victim, TO_CHAR);
    act("$n headbutts you, slamming $s head into your skull.", FALSE, c, 
                  0, victim, TO_VICT);
    victim->addToWait(combatRound(0.25));
  }

  TObj *item = dynamic_cast<TObj *>(victim->equipment[pos]);
  if (!item) {
    rc = c->damageLimb(victim, pos, 0, &h_dam);
    if (IS_SET_DELETE(rc, DELETE_VICT))
      return DELETE_VICT;
  } else if (c->dentItem(victim, item, 1, WEAR_HEAD) == DELETE_ITEM) {
    delete item;
    item = NULL;
  }

  item = dynamic_cast<TObj *>(c->equipment[WEAR_HEAD]);
  if (!item) {
    rc = c->damageLimb(c, WEAR_HEAD, 0, &h_dam);
    if (IS_SET_DELETE(rc, DELETE_VICT))
      return DELETE_THIS;
  } else {
    if (item->isSpiked() || item->isObjStat(ITEM_SPIKED))
      spikeddam=(int) (dam*0.15);
    if (c->dentItem(victim, item, 1, WEAR_HEAD) == DELETE_ITEM) {
      delete item;
      item = NULL;
    }
  }

  if(spikeddam) {
    act("The spikes on your $o sink into $N.", FALSE, c, item, victim, TO_CHAR);
    act("The spikes on $n's $o sink into $N.", FALSE, c, item, victim, TO_NOTVICT);
    act("The spikes on $n's $o sink into you.", FALSE, c, item, victim, TO_VICT);
    if ((rc = c->reconcileDamage(victim, spikeddam,TYPE_STAB)) == -1)
      return DELETE_VICT;
  }
  if ((rc = c->reconcileDamage(victim, dam,dam_type)) == -1)
    return DELETE_VICT;

  return TRUE;
}

static int headbutt(TBeing *caster, TBeing *victim)
{
  int percent;
  int i;
  int rc;
  const int HEADBUTT_MOVE   = 15;

  if (!caster->canHeadbutt(victim, SILENT_NO))
    return FALSE;

  percent = ((10 - (victim->getArmor() / 100)) << 1);
  percent += caster->getDexReaction() * 5;
  percent -= victim->getAgiReaction() * 5;
  
  int bKnown = caster->getSkillValue(SKILL_HEADBUTT);

  if (caster->getMove() < HEADBUTT_MOVE) {
    caster->sendTo("You lack the vitality.\n\r");
    return FALSE;
  }
  caster->addToMove(-HEADBUTT_MOVE);

  if (caster->bSuccess(bKnown + percent, SKILL_HEADBUTT) &&
         (i = caster->specialAttack(victim,SKILL_HEADBUTT)) &&
         i != GUARANTEED_FAILURE &&
         percent < bKnown&&
         !victim->canCounterMove(bKnown*2/5)) {
    return (headbuttHit(caster, victim));
  } else {
    rc = headbuttMiss(caster, victim);
    if (IS_SET_DELETE(rc, DELETE_THIS) || IS_SET_DELETE(rc, DELETE_VICT))
      return rc;
  }
  return TRUE;
}

int TBeing::doHeadbutt(const char *argument, TBeing *vict)
{
  int rc;
  TBeing *v;
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
  rc = headbutt(this, v);
  if (rc)
    addSkillLag(SKILL_HEADBUTT, rc);

  if (IS_SET_ONLY(rc, DELETE_VICT)) {
    if (vict)
      return rc;
    delete v;
    v = NULL;
    REM_DELETE(rc, DELETE_VICT);
  }
  return rc;
}

