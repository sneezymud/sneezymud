#include "stdsneezy.h"
#include "combat.h"

extern void startChargeTask(TBeing *, const char *);

static int charge(TBeing *ch, TBeing *vict)
{
  TThing *c;
  TBeing *tb;
  int rc;

  byte bKnown = ch->getSkillValue(SKILL_CHARGE);

  TMonster * mount=dynamic_cast<TMonster *>(ch->riding);
  if (!mount || (ch->getPosition() != POSITION_MOUNTED)) {
    ch->sendTo("You must be mounted to charge!\n\r");
    return FALSE;
  }
  if(ch->doesKnowSkill(mount->mountSkillType()) &&
     ch->advancedRidingBonus(mount)<50
     && mount->getRace() != RACE_HORSE) {
    ch->sendTo("You lack the skill to charge with this mount.\n\r");
    return FALSE;
  }
  if (ch == vict) {
    ch->sendTo("It is impossible to charge yourself!\n\r");
    return FALSE;
  }
  if (vict == mount) {
    ch->sendTo("You order your mount to charge into itself.\n\r");
    return FALSE;
  }
  if (vict->riding == mount) {
    // we are both on same horse
    act("Now how is your $o going to charge someone that is riding it?",
        FALSE, ch, mount, NULL, TO_CHAR);
    return FALSE;
  }
  if (mount->horseMaster() != ch) {
    act("You are not in control of $p and can't order it to charge.",
          FALSE, ch, mount, NULL, TO_CHAR);
    return FALSE;
  }
  if (!mount->hasLegs()) {
    act("You can't charge on a legless $o!", 
         false, ch, mount, NULL, TO_CHAR);
    return FALSE;
  }
  if (mount->eitherLegHurt()) {
    act("Your $o's injury prevents you from charging!", 
         false, ch, mount, NULL, TO_CHAR);
    return FALSE;
  }
  if (!mount->isFlying() && vict->isFlying()) {
    act("That would be hard, considering $N is flying, and your $o is not.", 
	FALSE,ch,mount,vict, TO_CHAR);
    return FALSE;
  }
  if (ch->checkPeaceful("This room is too peaceful to contemplate violence in.\n\r"))
    return FALSE;

  // if there are a lot of attackers, just plain deny
  if (vict->attackers > 4) {
    act("Too many people are fighting $N.  Charging is prohibited.",
          FALSE, ch, 0, vict, TO_CHAR);
    return FALSE;
  }
  // otherwise, allow the charge provided all the attackers are working together
  for (c = vict->roomp->getStuff(); c; c = c->nextThing) {
    TBeing *tbt = dynamic_cast<TBeing *>(c);
    if (!tbt)
      continue;
    if (tbt == ch || tbt == vict)
      continue;

    if (tbt->fight() == vict) {
      if (!tbt->inGroup(*ch)) {
        act("An innocent $o in the vicinity of $N prevents you from charging!",
            FALSE, ch, tbt, vict, TO_CHAR);
        return FALSE;
      }
    }
  }
  
  // very hard to tank and charge. Charge should be opening move
  if ((vict->getPosition() > POSITION_SITTING) &&
      ::number(0,2) &&
      ch->isTanking()) {
    ch->sendTo(COLOR_MOBS, fmt("You try to get in position to take a charge at %s.\n\r") %
vict->getName());
    ch->sendTo(COLOR_MOBS, fmt("However, %s stays close to you.\n\rYou can't get the space needed to charge!\n\r") % vict->getName());
    ch->cantHit += ch->loseRound(1);
    return FALSE;
  }

  if (ch->fight()) 
    ch->cantHit += ch->loseRound(5);
  else 
    ch->cantHit += ch->loseRound(3);

  soundNumT snd = pickRandSound(SOUND_HORSE_1, SOUND_HORSE_2);
  ch->roomp->playsound(snd, SOUND_TYPE_NOISE);

  if (vict->awake() &&
      (!ch->specialAttack(vict,SKILL_CHARGE) ||
       !ch->bSuccess(bKnown, ch->getPerc(), SKILL_CHARGE))) {
    act("You charge $N, but $E dodges to the side at the last moment.",
          TRUE, ch, 0, vict, TO_CHAR);
    act("$n and $s mount come charging at you.\n\rFortunately you were able to dodge them.",
          TRUE, ch, 0, vict, TO_VICT);
    act("$n and $s mount charge down upon $N.\n\rBut $E was able to dodge them.",
          TRUE, ch, 0, vict, TO_NOTVICT);

    for (c = vict->roomp->getStuff(); c; c = c->nextThing) {
      if (c == vict || c == ch)
        continue;

      tb = dynamic_cast<TBeing *>(c);
      if (!tb)
        continue;
      // don't have mount scatter
      if (ch->riding == tb)
        continue;
      if ((tb->fight() == vict) && (tb != mount)) {
        // we have already validated that all attackers are in ch's group
        act("You scatter as $N charges!",
           FALSE, tb, 0, ch, TO_CHAR);
        act("$n scatters as you charge!",
           FALSE, tb, 0, ch, TO_VICT);
        act("$n scatters as $N charges!",
           FALSE, tb, 0, ch, TO_NOTVICT);
        tb->loseRound(2);
      }
    }

    ch->reconcileDamage(vict, 0, SKILL_CHARGE);
    return TRUE;
  }

  int dam = ch->getSkillDam(vict, SKILL_CHARGE, ch->getSkillLevel(SKILL_CHARGE), ch->getAdvLearning(SKILL_CHARGE));

#if 1  
  // added charge crit per popular request
  // this is not the right way to do this.  - bat
  // New Damage Formula for Charge
  //if (gamePort != PROD_GAMEPORT) 
    //float newDam        = (100 + ((float) mount->GetMaxLevel() -
  // (float) ch->GetMaxLevel())) / 100;
    //float crossValue    = (float) dam * newDam;
    //int   initialDamage = dam;
    //bool  didCrit       = false;

    //    dam = (int) crossValue;
  
    TThing *prim = ch->heldInPrimHand();
    if (prim && !(::number(0, 25))) {
      act("A split second before the charge you brace your $o to strike.",
          TRUE, ch, prim, vict, TO_CHAR);
      act("$n braces $s $o in preparation for the strike.",
          TRUE, ch, prim, vict, TO_VICT);
      act("$n braces $s $o in preperation for $s charge at $N.",
          TRUE, ch, prim, vict, TO_NOTVICT);

      dam += ::number(5, ch->GetMaxLevel() / 2);
      //   didCrit = true;
    }
	
    // vlogf(LOG_MISC, fmt("Charge Damage Formula [%s][%.2f / %.2f|%d / %s]") %  ch->getName() %
    //      newDam % crossValue % initialDamage % (didCrit ? "Critical" : "Normal"));
    //  }
#endif

  act("You charge $N, striking $M with a mighty blow.",  
         TRUE, ch, 0, vict, TO_CHAR);
  act("$n and $s mount come charging at you.",
         TRUE, ch, 0, vict, TO_VICT);
  act("$n and $s mount charge down upon $N.",
         TRUE, ch, 0, vict, TO_NOTVICT);

  for (c = vict->roomp->getStuff(); c; c = c->nextThing) {
    if (c == vict || c == ch)
      continue;

    tb = dynamic_cast<TBeing *>(c);
    if (!tb)
      continue;
    if ((tb->fight() == vict) && (tb != mount)) {
      // we have already validated that all attackers are in ch's group
      act("You scatter as $N charges!", FALSE, tb, 0, ch, TO_CHAR);
      act("$n scatters as you charge!", FALSE, tb, 0, ch, TO_VICT);
      act("$n scatters as $N charges!",  FALSE, tb, 0, ch, TO_NOTVICT);
      tb->loseRound(2);
    }
  }

  if (vict->riding && dynamic_cast<TBeing *>(vict->riding)) {
    act("$n is heaved from $s mount and falls to the $g.",
          TRUE, vict, 0, 0, TO_ROOM);
    act("You are knocked from your mount and dashed to the $g!",
          TRUE, vict, 0,0, TO_CHAR);
    rc = vict->fallOffMount(vict->riding, POSITION_SITTING);
    if (IS_SET_DELETE(rc, DELETE_THIS)) 
      return DELETE_VICT;

    vict->addToWait(combatRound(1));
  } else if (vict->riding) {
    act("$n is heaved from $p and falls to the $g.",
          TRUE, vict, vict->riding, 0, TO_ROOM);
    act("You are knocked from $p and dashed to the $g!",
          TRUE, vict, vict->riding,0, TO_CHAR);
    rc = vict->fallOffMount(vict->riding, POSITION_SITTING);
    if (IS_SET_DELETE(rc, DELETE_THIS)) {
      return DELETE_VICT;
    }
  } else if ((c = vict->rider)) {
    act("$n is knocked from under you and you fall to the $g.",
          TRUE, vict, 0, c, TO_VICT);
    act("You are battered by the blow and $N falls off you!",
          TRUE, vict, 0, c, TO_CHAR);
    act("$n is stricken by the blow and $N falls off $m!",    
          TRUE, vict, 0, c, TO_NOTVICT);
    rc = c->fallOffMount(vict, POSITION_SITTING);
    vict->addToWait(combatRound(1));
    if (IS_SET_DELETE(rc, DELETE_THIS)) {
      delete c;
      c = NULL;
    }
  } else {
    act("You are battered by the blow and trampled to the $g!",
          TRUE, vict, 0, 0, TO_CHAR);
    act("$n is battered by the blow and trampled to the $g!",
          TRUE, vict, 0, 0, TO_ROOM);
    vict->setPosition(POSITION_SITTING);
    vict->addToWait(combatRound(1));
  }

  vict->cantHit += vict->loseRound(2);
  if (ch->reconcileDamage(vict, dam, SKILL_CHARGE) == -1)
    return DELETE_VICT;

  return TRUE;
}

int TBeing::doCharge(const char *arg, TBeing *victim)
{
  TBeing   *vict;
  char      tmp[80],
            tString[256];
  int       rc        = 0;
  dirTypeT  Direction = DIR_NONE;

  if (doesKnowSkill(SKILL_STAVECHARGE)) {
    doChargeStave(arg);
    return FALSE;
  }

  if (checkBusy()) {
    return FALSE;
  }
  if (!doesKnowSkill(SKILL_CHARGE)) {
    sendTo("You know nothing about charging.\n\r");
    return FALSE;
  }
  half_chop(arg, tmp, tString);
  if (!victim) {
    Direction = getDirFromChar(tmp);

    if (Direction > DIR_NONE && Direction < MAX_DIR) {
      startChargeTask(this, arg);
      return FALSE;
    }
  }
  if (!(vict = victim)) {
    if (!(vict = get_char_room_vis(this,tmp))) {
      if (!(vict = fight())) {
        sendTo("Charge whom?\n\r");
        return FALSE;
      }
    }
  }
  if (noHarmCheck(vict))
    return FALSE;

  if (!sameRoom(*vict)) {
    sendTo("That person isn't around.\n\r");
    return FALSE;
  }

  rc = charge(this, vict);
  if (rc)
    addSkillLag(SKILL_CHARGE, rc);
  if (IS_SET_DELETE(rc, DELETE_VICT)) {
    if (victim)
      return rc;
    delete vict;
    vict = NULL;
    REM_DELETE(rc, DELETE_VICT);
  }
  return rc;
}

