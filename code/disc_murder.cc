#include "stdsneezy.h"
#include "disease.h"
#include "combat.h"
#include "spelltask.h"
#include "disc_murder.h"
#include "obj_tool.h"
#include "obj_general_weapon.h"
#include "obj_base_weapon.h"
#include "obj_base_cup.h"
#include "obj_arrow.h"
#include "liquids.h"

static void playBackstab(const TRoom *rp)
{
  soundNumT snd = pickRandSound(SOUND_BACKSTAB_01, SOUND_BACKSTAB_02);
  rp->playsound(snd, SOUND_TYPE_COMBAT);
}

const int BS_MSG_DEATH_MAX = 7; // (tMessagesDeath / 2) - 1
const int BS_MSG_NONDT_MAX = 7; // (tMessagesNonDeath / 2) - 1

// returns DELETE_VICT
int TBeing::backstabHit(TBeing *victim, TThing *obj)
{
  int i, d;

  const char *tMessagesDeath[] =
  {
    "Your $o plunges deep into $N's upper back, killing $M.",
    "$n plunges $s $o deep into $N's upper back, killing $M.",

    "Your $o sinks deep into $N's lower back, killing $M.",
    "$n sinks $s $o deep into $N's lower back, killing $M.",

    "$N screams in extreme agony and then falls lifeless as you quickly place your $p into $S back.",
    "$N screams in extreme agony and then falls lifeless as $n quickly places $s $p into $N's back.",

    "Blood splatters you, as you thrust $p into $N's back, killing $M.",
    "Blood splatters as $n thrusts $p into $N's back, killing $M.",

    "Blood spurts from $N's mouth as you plunge $p in $S back, killing $M.",
    "Blood spurts from $N's mouth as $n plunges $p in $S back, killing $M.",

    "$N enters into convulsions as you slip $p into $S spine, killing $M.",
    "$N enters into convulsions as $n slips $p into $S spine, killing $M.",

    "$N coughs up some blood, and then falls dead, as you place $p in $S back.",
    "$n sticks $p in $N's back; $N coughs up some blood, and then falls dead.",

    "$N gets a blank look on $S face, then collapses, as you place your $o in $S back.",
    "$N gets a black look on $S face, then collapses, as $n places $s $o in $S back.",

    "$N collapses and begins to twitch as you place your $o in $S back, killing $M.",
    "$N collapses and begins to twitch as $n places $s $o in $S back, killing $M."
  };
  const char *tMessagesNonDeath[] =
  {
    "Your $o plunges deep into $N's upper back.",
    "$n plunges $s $o deep into $N's upper back.",

    "Your $o sinks deep into $N's lower back.",
    "$n sinks $s $o deep into $N's lower back.",

    "Blood splatters you, as you thrust $p into $N's back.",
    "Blood splatters as $n thrusts $p into $N's back.",

    "Blood spurts from $N's mouth as you plunge $p in $S back.",
    "Blood spurts from $N's mouth as $n plunges $p in $S back.",

    "$N enters into convulsions as you slip $p into $S spine.",
    "$N enters into convulsions as $n slips $p into $S spine.",

    "$N coughs up some blood, as you place $p in $S back.",
    "$n places $p in the back of $N, resulting in some strange noises and blood.",

    "$N gets a blank look on $S face, then dances around madly, as you place your $o in $S back.",
    "$N gets a blank look on $S face, then dances around madly, as $n places $s $o in $S back.",

    "$N twitches wildly for a moment as you place your $o in $S back.",
    "$N twitches wildly for a moment as $n places $s $o in $S back."
  };

  d = getSkillDam(victim, SKILL_BACKSTAB, getSkillLevel(SKILL_BACKSTAB), getAdvLearning(SKILL_BACKSTAB));

  if ((i = specialAttack(victim, SKILL_BACKSTAB)) || (i == GUARANTEED_SUCCESS)) {
    if (victim->getPosition() > POSITION_DEAD) {
      if (!(d = getActualDamage(victim, obj, d, SKILL_BACKSTAB))) {
        act("You try to backstab $N, but you can't penetrate $S skin!",  FALSE, this, obj, victim, TO_CHAR);
        act("$n tries to backstab you, but you feel no pain.", FALSE, this, obj, victim, TO_VICT);
        act("$n tries to backstab $N, but $N seems unaffected.",  FALSE, this, obj, victim, TO_NOTVICT);
      } else if (willKill(victim, d, SKILL_BACKSTAB, FALSE)) {
        playBackstab(roomp);

        if (victim->isUndead()) {
          act("$N coughs, shivers, then collapses as you place $p in $S back.",
              FALSE, this, obj, victim, TO_CHAR);
          act("$N coughs, shivers, then collapses as $n places $p in $S back.",
              FALSE, this, obj, victim, TO_NOTVICT);
        } else {
          int tMessageChoice = (::number(0, BS_MSG_DEATH_MAX) * 2);

          if (!tMessagesDeath[tMessageChoice])
            tMessageChoice = 0;

          act(tMessagesDeath[tMessageChoice],
              FALSE, this, obj, victim, TO_CHAR);
          act(tMessagesDeath[tMessageChoice + 1],
              FALSE, this, obj, victim, TO_NOTVICT);
        }

        act("Suddenly, $n stabs you in the back!  R.I.P...",
            FALSE, this, obj, victim, TO_VICT);
      } else {
        playBackstab(roomp);

        if (victim->isUndead()) {
          act("$N coughs and shivers as you place $p in $S back.",
              FALSE, this, obj, victim, TO_CHAR);
          act("$n places $p in the back of $N; $N coughs and shivers...",
              FALSE, this, obj, victim, TO_NOTVICT);
        } else {
          int tMessageChoice = (::number(0, BS_MSG_NONDT_MAX) * 2);

          if (!tMessagesNonDeath[tMessageChoice])
            tMessageChoice = 0;

          act(tMessagesNonDeath[tMessageChoice],
              FALSE, this, obj, victim, TO_CHAR);
          act(tMessagesNonDeath[tMessageChoice + 1],
              FALSE, this, obj, victim, TO_NOTVICT);
        }

        act("Suddenly, $n stabs you in the back!",
            FALSE, this, obj, victim, TO_VICT);

	// poison
	TBaseWeapon *tow;
	if(obj && (tow = dynamic_cast<TBaseWeapon *>(obj)) && 
	   tow->isPoisoned())
	  tow->applyPoison(victim);

      }
    }
  } else {
    act("$N quickly avoids your backstab, and you nearly cut your finger.",
        FALSE, this, obj, victim, TO_CHAR);
    act("$n tried to backstab you, but you avoid $m.",
        FALSE, this, obj, victim, TO_VICT);
    act("$n tried to backstab $N, but nearly cut $s own finger.",
        FALSE, this, obj, victim, TO_NOTVICT);

    d = 0;
  }

  if (reconcileDamage(victim, d, SKILL_BACKSTAB) == -1)
    return DELETE_VICT;

  victim->addHated(this);


  return 0;
}

int TBeing::doBackstab(const char *argument, TBeing *vict)
{
  TBeing *victim=NULL, *GLeader=NULL;
  followData *FDt;
  char namebuf[256] = "\0";
  int rc;

  if (!doesKnowSkill(SKILL_BACKSTAB)) {
    sendTo("You know nothing about backstabbing.\n\r");
    return FALSE;
  }
  if (checkBusy()) {
    return FALSE;
  }
  // *** Start of Test Tank-Target Backstab Code  -Lapsos
  if (!*argument && isAffected(AFF_GROUP)) {
    if (master)
      GLeader = master;
    else
      GLeader = this;

    if (GLeader != this && sameRoom(*GLeader) && GLeader->fight())
      victim = GLeader->fight();
    else {
      for (FDt = GLeader->followers; FDt; FDt = FDt->next) {
        if (!sameRoom(*FDt->follower))
          continue;

        if (FDt->follower == this)
	  continue;

        if (!FDt->follower->isAffected(AFF_GROUP))
          continue;

        if ((victim = FDt->follower->fight())) 
          break;
      }

      if (victim && !sameRoom(*victim))
        victim = NULL;
    }
  }

  if (!vict && !victim) {
    if (*argument)
      strcpy(namebuf, argument);

    if (!(victim = get_char_room_vis(this, namebuf))) {
      sendTo("Backstab whom?\n\r");
      return FALSE;
    }
  }

  if (vict && !victim)
    victim = vict;
  // *** End of Test Tank-Target Backstab Code  -Lapsos

  if (!sameRoom(*victim)) {
    sendTo("That person isn't around.\n\r");
    return FALSE;
  }

  if ((rc = backstab(this, victim))) {
    if (!victim->isPc())
      dynamic_cast<TMonster *>(victim)->US(25);
    addSkillLag(SKILL_BACKSTAB, rc);
  }

  if (IS_SET_DELETE(rc, DELETE_VICT)) {
    if (vict)
      return rc;

    delete victim;
    victim = NULL;
    REM_DELETE(rc, DELETE_VICT);
  }

  TGenWeapon * obj = dynamic_cast<TGenWeapon *>(heldInPrimHand());

  if (obj &&
      obj->checkSpec(victim, CMD_BACKSTAB, "-special-", this) == DELETE_VICT) {
    delete victim;
    victim = NULL;
  } 

  return rc;
}

int backstab(TBeing *thief, TBeing * victim)
{
  int base = 0;
  int level;
  int rc = 0;

  if (thief->checkPeaceful("You cannot backstab in a peaceful room!\n\t"))
    return FALSE;

  if (victim == thief) {
    thief->sendTo("How can you sneak up on yourself?\n\r");
    return FALSE;
  }
  TGenWeapon * obj = dynamic_cast<TGenWeapon *>(thief->heldInPrimHand());
  if (!obj) {
    thief->sendTo("You need to wield a weapon, to make it a success.\n\r");
    return FALSE;
  }
  if (thief->riding) {
    thief->sendTo("You cannot backstab while mounted!\n\r");
    return FALSE;
  }
  if (dynamic_cast<TBeing *>(victim->riding)) {
    thief->sendTo("You cannot backstab while that person is mounted!\n\r");
    return FALSE;
  }
  if (dynamic_cast<TBeing *>(victim->rider)) {
    act("Unfortunately, $N's back is covered by $p riding upon $M.",
           false, thief, victim->rider, victim, TO_CHAR);
    return FALSE;
  }
  if (thief->noHarmCheck(victim))
    return FALSE;

  if (thief->attackers) {
    thief->sendTo("There's no way to reach that back while you're fighting!\n\r");
    return FALSE;
  }
  if (!obj->canBackstab()) {
    act("You can't use $p to backstab.", false, thief, obj, NULL, TO_CHAR);
    return FALSE;
  }

  if (thief->fight()) {
    thief->sendTo("You're too busy to backstab!\n\r");
    return FALSE;
  }
  thief->reconcileHurt(victim, 0.04);

  if (thief->makesNoise() && victim->awake()) {
    act("$n's armor makes too much noise, and $N is able to avoid $s backstab.",
        FALSE, thief, 0, victim, TO_NOTVICT);
    act("You make too much noise, and $N hears you and avoids your backstab.",
        FALSE, thief, 0, victim, TO_CHAR);
    act("You hear $n's armor, and quickly dodge $s attempt to backstab you.",
        FALSE, thief, 0, victim, TO_VICT);
    thief->reconcileDamage(victim, 0,SKILL_BACKSTAB);
    victim->addHated(thief);
    return TRUE;
  }
  if (victim->awake() && victim->canSee(thief) &&
      !victim->isPc() && dynamic_cast<TMonster *>(victim)->isSusp()) {
    act("You almost succeed, but $E senses you coming at the last moment.",
        FALSE, thief, 0, victim, TO_CHAR);
    act("$n attempts to backstab you, but you sense $m coming.",
        FALSE, thief, 0, victim, TO_VICT);
    act("$n attempts to backstab $N, but $N senses $m coming.",
        FALSE, thief, 0, victim, TO_NOTVICT);
    thief->reconcileDamage(victim, 0, SKILL_BACKSTAB);
    victim->addHated(thief);
    return TRUE;
  }
  if ((!thief->isAffected(AFF_INVISIBLE) ||
       victim->isAffected(AFF_DETECT_INVISIBLE)) &&
      victim->canSee(thief) &&
      !thief->isAffected(AFF_SNEAK) &&
      !thief->isAffected(AFF_HIDE) &&
      victim->awake()) {
    act("$N notices you walking up behind $M, apparently you were not sneaking and visible...",FALSE,thief,0,victim,TO_CHAR);
    act("$n makes a pathetic attempt at backstabbing $N.", FALSE, thief, 0, victim, TO_NOTVICT);
    act("You nearly cut yourself as you try to backstab $N.", FALSE, thief, 0, victim, TO_CHAR);
    act("You quickly dodge $n's pathetic attempt to backstab you.", FALSE, thief, 0, victim, TO_VICT);

    thief->reconcileDamage(victim, 0,SKILL_BACKSTAB);
    victim->addHated(thief);
    return TRUE;
  }
  if (victim->fight())
    base = 0;
  else
    base = 4;

  level = thief->getSkillLevel(SKILL_BACKSTAB);
  int bKnown = thief->getSkillValue(SKILL_BACKSTAB);

  if ((thief->bSuccess(bKnown, SKILL_BACKSTAB) || !victim->awake())) {
    thief->setSpellHitroll(thief->getSpellHitroll() + base);
    rc = thief->backstabHit(victim, obj);
    thief->setSpellHitroll(thief->getSpellHitroll() - base);
    if (IS_SET_DELETE(rc, DELETE_VICT)) 
      return DELETE_VICT;
    
    victim->addHated(thief);
  } else {
    act("$n makes a pathetic attempt at backstabbing $N.", FALSE, thief, 0, victim, TO_NOTVICT);
    act("You nearly cut yourself as you try to backstab $N.", FALSE, thief, 0, victim, TO_CHAR);
    act("You quickly dodge $n's pathetic attempt to backstab you.", FALSE, thief, 0, victim, TO_VICT);
    if (thief->reconcileDamage(victim, 0,SKILL_BACKSTAB) == -1)
      return DELETE_VICT;

    victim->addHated(thief);
  }
  return TRUE;
}

//////////////////////////////////////////////////////////////////
// Throat slitting: Meant to be an advanced form of backstabbing
/////////////////// not to replace backstab but to enhance the
/////////////////// flavor of the thief class by giving them 
/////////////////// something else that looks different without
/////////////////// making the class overpowered skillwise
/////////////////////////////////////////////////////////////////

static void playThroatSlit(const TRoom *rp)
{
  // using backstab sounds since they seem appropriate to this skill too
  soundNumT snd = pickRandSound(SOUND_BACKSTAB_01, SOUND_BACKSTAB_02);
  rp->playsound(snd, SOUND_TYPE_COMBAT);
}

const int TS_MSG_DEATH_MAX = 4; // (tMessagesDeath / 2) - 1
const int TS_MSG_NONDT_MAX = 4; // (tMessagesNonDeath / 2) - 1

int TBeing::throatSlitHit(TBeing *victim, TThing *obj)
{
  int i, d;

  const char *tMessagesDeath[] =
  {
    "You slice $N's throat with your $o, instantly killing $M.",
    "$n slices $N's throat with $s $o, instantly killing $M.",

    "You slit $N's throat and $E falls lifeless as blood sprays everywhere!",
    "Blood sprays everywhere as $n slices into $N's throat, killing $M!",

    "$N's eyes roll back into $S head as you murder $M with your $o!",
    "$N's eyes roll back into $S head as $n murders $M with $s $o!",

    "Your $o is stained with blood as you easily slice into $N's throat, killing $M.",
    "$n's $o is stained with blood as $e effortlessly slices into $N's throat, killing $M.",

    "Blood sprays from $N's neck as you slice into $S throat, killing $M.",
    "Blood sprays from $N's neck as $n slices into $N's throat, killing $M.",

    "$N slowly collapses into a puddle of $S own blood.",
    "$N slowly collapses into a puddle of $S own blood as $n slits $N's throat, killing $M."
  };
  const char *tMessagesNonDeath[] =
  {
    "You slit $N's throat, surprisingly, $N isn't dead!",
    "$n expertly uses $s $o to slit $N's throat!",

    "Blood stains your hand as you slice into $N's throat.",
    "Blood splatters the ground as $n slices into $N's throat.",

    "$N's eyes widen in rage, as $E feels your $o slicing $S throat!",
    "$N's eyes widen in rage, as $E feels $n's $o slicing $S throat!",

    "Blood sprays from $N's neck as you slice into $S throat.",
    "Blood spurts from $N's neck as $n slices into $S throat.",

    "$N gurgles and chokes as you slice into $S throat!",
    "$N gurgles and chokes as $n slices into $S throat!",

    "$N chokes on $S own blood as you slit $S throat!",
    "$N chokes on $S own blood as $n slits $S throat!",
  };

  d = getSkillDam(victim, SKILL_THROATSLIT, getSkillLevel(SKILL_THROATSLIT), getAdvLearning(SKILL_THROATSLIT));

  if ((i = specialAttack(victim, SKILL_THROATSLIT)) || (i == GUARANTEED_SUCCESS)) {
    if (victim->getPosition() > POSITION_DEAD) {
      if (!(d = getActualDamage(victim, obj, d, SKILL_THROATSLIT))) {
        act("You try to slit $N's throat, but you can't penetrate $S thick skin!",  FALSE, this, obj, victim, TO_CHAR);
        act("$n tries to slit your throat, but your neck is too strong.", FALSE, this, obj, victim, TO_VICT);
        act("$n tries to slit $N's throat, but $N seems unaffected.",  FALSE, this, obj, victim, TO_NOTVICT);
      } else if (willKill(victim, d, SKILL_THROATSLIT, FALSE)) {
        playThroatSlit(roomp);
	victim->dropPool(20, LIQ_BLOOD);
        if (victim->isUndead()) {
          act("$N shakes and then collapses as you slit $S throat.",
              FALSE, this, obj, victim, TO_CHAR);
          act("$N shakes and then collapses as $n slits $N's throat.",
              FALSE, this, obj, victim, TO_NOTVICT);
        } else {
          int tMessageChoice = (::number(0, TS_MSG_DEATH_MAX) * 2);

          if (!tMessagesDeath[tMessageChoice])
            tMessageChoice = 0;

          act(tMessagesDeath[tMessageChoice],
              FALSE, this, obj, victim, TO_CHAR);
          act(tMessagesDeath[tMessageChoice + 1],
              FALSE, this, obj, victim, TO_NOTVICT);
        }

        act("You suddenly fall over dead as $n slices your throat!  R.I.P...",
            FALSE, this, obj, victim, TO_VICT);
      } else {
        playThroatSlit(roomp);
	victim->dropPool(20, LIQ_BLOOD);

        if (victim->isUndead()) {
          act("$N coughs and shivers as you slit $S throat.",
              FALSE, this, obj, victim, TO_CHAR);
          act("$n slits $N's throat, making $M cough and shiver...",
              FALSE, this, obj, victim, TO_NOTVICT);
        } else {
          int tMessageChoice = (::number(0, TS_MSG_NONDT_MAX) * 2);

          if (!tMessagesNonDeath[tMessageChoice])
            tMessageChoice = 0;

          act(tMessagesNonDeath[tMessageChoice],
              FALSE, this, obj, victim, TO_CHAR);
          act(tMessagesNonDeath[tMessageChoice + 1],
              FALSE, this, obj, victim, TO_NOTVICT);
        }

        act("$n sneaks up behind you and cuts your throat!",
            FALSE, this, obj, victim, TO_VICT);

      // poison
	TBaseWeapon *tow;
	if(obj && (tow = dynamic_cast<TBaseWeapon *>(obj)) && 
	   tow->isPoisoned())
	  tow->applyPoison(victim);
	
      }
    }
  } else {
    act("$N quickly avoids your feeble murder attempt.",
        FALSE, this, obj, victim, TO_CHAR);
    act("$n tried to slice your throat, but you were too quick for $m.",
        FALSE, this, obj, victim, TO_VICT);
    act("$n tried to slit $N's throat, but has failed miserably.",
        FALSE, this, obj, victim, TO_NOTVICT);

    d = 0;
  }

  if (reconcileDamage(victim, d, SKILL_THROATSLIT) == -1)
    return DELETE_VICT;

  victim->addHated(this);


  return 0;
}

int TBeing::doThroatSlit(const char *argument, TBeing *vict)
{
  TBeing *victim=NULL, *GLeader=NULL;
  followData *FDt;
  char namebuf[256] = "\0";
  int rc;

  if (!doesKnowSkill(SKILL_THROATSLIT)) {
    sendTo("You know nothing about murder.\n\r");
    return FALSE;
  }
  if (checkBusy()) {
    return FALSE;
  }
  // *** Start of Test Tank-Target Backstab Code  -Lapsos
  if (!*argument && isAffected(AFF_GROUP)) {
    if (master)
      GLeader = master;
    else
      GLeader = this;

    if (GLeader != this && sameRoom(*GLeader) && GLeader->fight())
      victim = GLeader->fight();
    else {
      for (FDt = GLeader->followers; FDt; FDt = FDt->next) {
        if (!sameRoom(*FDt->follower))
          continue;

        if (FDt->follower == this)
	  continue;

        if (!FDt->follower->isAffected(AFF_GROUP))
          continue;

        if ((victim = FDt->follower->fight())) 
          break;
      }

      if (victim && !sameRoom(*victim))
        victim = NULL;
    }
  }

  if (!vict && !victim) {
    if (*argument)
      strcpy(namebuf, argument);

    if (!(victim = get_char_room_vis(this, namebuf))) {
      sendTo("Slit who's throat?\n\r");
      return FALSE;
    }
  }

  if (vict && !victim)
    victim = vict;
  // *** End of Test Tank-Target Backstab Code  -Lapsos

  if (!sameRoom(*victim)) {
    sendTo("That person isn't around.\n\r");
    return FALSE;
  }

  if ((rc = throatSlit(this, victim))) {
    if (!victim->isPc())
      dynamic_cast<TMonster *>(victim)->US(25);
    addSkillLag(SKILL_THROATSLIT, rc);
  }

  if (IS_SET_DELETE(rc, DELETE_VICT)) {
    if (vict)
      return rc;

    delete victim;
    victim = NULL;
    REM_DELETE(rc, DELETE_VICT);
  }

  TGenWeapon * obj = dynamic_cast<TGenWeapon *>(heldInPrimHand());

  if (obj &&
      obj->checkSpec(victim, CMD_SLIT, "-special-", this) == DELETE_VICT) {
    delete victim;
    victim = NULL;
  } 

  return rc;
}

int throatSlit(TBeing *thief, TBeing * victim)
{
  int base = 0;
  int level;
  int rc = 0;

  if (thief->checkPeaceful("You cannot murder in a peaceful room!\n\t"))
    return FALSE;

  if (victim == thief) {
    thief->sendTo("How can you sneak up on yourself?\n\r");
    return FALSE;
  }
  TGenWeapon * obj = dynamic_cast<TGenWeapon *>(thief->heldInPrimHand());
  if (!obj) {
    thief->sendTo("You need to wield a weapon, to make it a success.\n\r");
    return FALSE;
  }
  if (6*thief->getHeight() < 3*victim->getHeight() && !(thief->isFlying())) {
    thief->sendTo("You don't stand a chance, that creature is too tall.\n\r");
    return FALSE;
  }

  if (thief->riding) {
    thief->sendTo("You cannot attempt murder in that way while mounted!\n\r");
    return FALSE;
  }
  if (dynamic_cast<TBeing *>(victim->riding)) {
    thief->sendTo("You can't reach their throat from here!\n\r");
    return FALSE;
  }
  if (dynamic_cast<TBeing *>(victim->rider)) {
    act("Unfortunately, $N's throat is not accessable.",
           false, thief, victim->rider, victim, TO_CHAR);
    return FALSE;
  }
  if (thief->noHarmCheck(victim))
    return FALSE;

  if (thief->attackers) {
    thief->sendTo("There's no way to reach their neck while you're fighting!\n\r");
    return FALSE;
  }
  if (!obj->canBackstab()) {
    act("You can't use $p to slice anyone's throat.", false, thief, obj, NULL, TO_CHAR);
    act("You should try a weapon that is used for backstabbing.", false, thief, NULL, NULL, TO_CHAR);
    return FALSE;
  }

  if (thief->fight()) {
    thief->sendTo("You're too busy!\n\r");
    return FALSE;
  }
  thief->reconcileHurt(victim, 0.04);

  if (thief->makesNoise() && victim->awake()) {
    act("$n's armor makes too much noise, and $N is able to avoid $n's murder attempt.",
        FALSE, thief, 0, victim, TO_NOTVICT);
    act("You make too much noise, and $N hears you and avoids you totally.",
        FALSE, thief, 0, victim, TO_CHAR);
    act("You hear $n's armor, and quickly dodge $s attempt to murder you.",
        FALSE, thief, 0, victim, TO_VICT);
    thief->reconcileDamage(victim, 0,SKILL_THROATSLIT);
    victim->addHated(thief);
    return TRUE;
  }
  if (victim->awake() && victim->canSee(thief) &&
      !victim->isPc() && dynamic_cast<TMonster *>(victim)->isSusp()) {
    act("You almost succeed, but $E senses you coming at the last moment.",
        FALSE, thief, 0, victim, TO_CHAR);
    act("$n attempts to murder you, but you sense $m coming.",
        FALSE, thief, 0, victim, TO_VICT);
    act("$n attempts to murder $N, but $N senses $m coming.",
        FALSE, thief, 0, victim, TO_NOTVICT);
    thief->reconcileDamage(victim, 0, SKILL_THROATSLIT);
    victim->addHated(thief);
    return TRUE;
  }
  if ((!thief->isAffected(AFF_INVISIBLE) ||
       victim->isAffected(AFF_DETECT_INVISIBLE)) &&
      victim->canSee(thief) &&
      !thief->isAffected(AFF_SNEAK) &&
      !thief->isAffected(AFF_HIDE) &&
      victim->awake()) {
    act("$N notices you walking up behind $M, apparently you were not sneaking and visible...",FALSE,thief,0,victim,TO_CHAR);
    act("$n makes a pathetic attempt at $N's life.", FALSE, thief, 0, victim, TO_NOTVICT);
    act("You totally fail your attempt at $N's life.", FALSE, thief, 0, victim, TO_CHAR);
    act("You quickly dodge $n's pathetic attempt to kill you.", FALSE, thief, 0, victim, TO_VICT);

    thief->reconcileDamage(victim, 0,SKILL_THROATSLIT);
    victim->addHated(thief);
    return TRUE;
  }
  if (victim->fight())
    base = 0;
  else
    base = 4;

  level = thief->getSkillLevel(SKILL_THROATSLIT);
  int bKnown = thief->getSkillValue(SKILL_THROATSLIT);

  if ((thief->bSuccess(bKnown, SKILL_THROATSLIT) || !victim->awake())) {
    thief->setSpellHitroll(thief->getSpellHitroll() + base);
    rc = thief->throatSlitHit(victim, obj);
    thief->setSpellHitroll(thief->getSpellHitroll() - base);
    if (IS_SET_DELETE(rc, DELETE_VICT)) 
      return DELETE_VICT;
    
    victim->addHated(thief);
  } else {
    act("$n makes a pathetic attempt at $N's life.", FALSE, thief, 0, victim, TO_NOTVICT);
    act("You pathetically fail to slit $N's throat.", FALSE, thief, 0, victim, TO_CHAR);
    act("You quickly dodge $n's pathetic attempt to slit your throat.", FALSE, thief, 0, victim, TO_VICT);
    if (thief->reconcileDamage(victim, 0,SKILL_THROATSLIT) == -1)
      return DELETE_VICT;

    victim->addHated(thief);
  }
  return TRUE;
}

//////////

int TBeing::doPoisonWeapon(sstring arg)
{
  TObj *obj, *poison;
  sstring namebuf;
  int rc;

  if (!doesKnowSkill(SKILL_POISON_WEAPON)){
    sendTo("You know nothing about poisoning weapons.\n\r");
    return FALSE;
  }
  if(checkBusy())
    return FALSE;
  
  arg = one_argument(arg, namebuf);

  if(arg.empty() ||
     !(obj=generic_find_obj(namebuf, FIND_OBJ_INV|FIND_OBJ_EQUIP, this))){
    sendTo("Poison what?\n\r");
    return FALSE;
  }

  if(!doesKnowSkill(SKILL_POISON_WEAPON) &&
     !(dynamic_cast<TArrow *>(obj))){
    sendTo("You are only skilled at poisoning arrows.\n\r");
    return FALSE;
  }


  arg = one_argument(arg, namebuf);

  if(!namebuf.empty()){
    if(!(poison=generic_find_obj(namebuf, FIND_OBJ_INV|FIND_OBJ_EQUIP, this))){
      sendTo("You can't find that poison.\n\r");
      return FALSE;
    }
  } else {
    if(!(poison=generic_find_obj("poison",FIND_OBJ_INV|FIND_OBJ_EQUIP, this))){
      sendTo("You can't find any poison.\n\r");
      return FALSE;
    }
  }

  if (fight()) {
    sendTo("You're a little too busy at the moment to try that.\n\r");
    return FALSE;
  }

  rc = poisonWeapon(this, obj, poison);
  if (rc)
    addSkillLag(SKILL_POISON_WEAPON, rc);
  if (IS_SET_DELETE(rc, DELETE_THIS))
    return DELETE_THIS;
  return rc;
}

int TThing::poisonMePoison(TBeing *ch, TBaseWeapon *)
{
  act("$p isn't the proper kind of poison for this.", 
            FALSE, ch, this, 0, TO_CHAR);
  return FALSE;
}


void addPoisonDefaults(affectedData *aff, int level, int duration)
{
  aff->type=SPELL_POISON;
  aff->bitvector=AFF_POISON;
  aff->renew=-1;
  aff->level=level;
  aff->duration=duration;  
}

// this is ugly as hell
bool addPoison(affectedData aff[5], 
	       liqTypeT liq, int level, int duration){
  addPoisonDefaults(&aff[4], level, duration);
  aff[4].type = AFFECT_DISEASE;
  aff[4].modifier = DISEASE_POISON;
  aff[4].location = APPLY_NONE;

  switch(liq){
    case LIQ_POISON_CAMAS:
      addPoisonDefaults(&aff[0], level, duration);
      aff[0].location=APPLY_DEX;
      aff[0].modifier=-20;
      addPoisonDefaults(&aff[1], level, duration);
      aff[1].location=APPLY_AGI;
      aff[1].modifier=-20;
      addPoisonDefaults(&aff[2], level, duration);
      aff[2].location=APPLY_STR;
      aff[2].modifier=-20;
      addPoisonDefaults(&aff[3], level, duration);
      aff[3].location=APPLY_SPE;
      aff[3].modifier=-20;
      break;
    case LIQ_POISON_ANGEL:
      addPoisonDefaults(&aff[0], level, duration);
      aff[0].location=APPLY_VISION;
      aff[0].modifier=-5;
      addPoisonDefaults(&aff[1], level, duration);
      aff[1].location=APPLY_FOC;
      aff[1].modifier=-20;
      addPoisonDefaults(&aff[2], level, duration);
      aff[2].location=APPLY_WIS;
      aff[2].modifier=-20;
      break;
    case LIQ_POISON_JIMSON:
      addPoisonDefaults(&aff[0], level, duration);
      aff[0].location=APPLY_VISION;
      aff[0].modifier=-10;
      addPoisonDefaults(&aff[1], level, duration);
      aff[1].location=APPLY_FOC;
      aff[1].modifier=-20;
      break;
    case LIQ_POISON_HEMLOCK:      
      addPoisonDefaults(&aff[0], level, duration);
      aff[0].location=APPLY_STR;
      aff[0].modifier=-20;
      addPoisonDefaults(&aff[1], level, duration);
      aff[1].location=APPLY_INT;
      aff[1].modifier=-20;
      addPoisonDefaults(&aff[2], level, duration);
      aff[2].location=APPLY_FOC;
      aff[2].modifier=-20;
      break;
    case LIQ_POISON_MONKSHOOD:
      addPoisonDefaults(&aff[0], level, duration);
      aff[0].location=APPLY_STR;
      aff[0].modifier=-20;
      break;
    case LIQ_POISON_GLOW_FISH:
      addPoisonDefaults(&aff[0], level, duration);
      aff[0].location=APPLY_CAN_BE_SEEN;
      aff[0].modifier=-10;
      break;
    case LIQ_POISON_SCORPION:
      addPoisonDefaults(&aff[0], level, duration);
      aff[0].location=APPLY_INT;
      aff[0].modifier=-20;
      addPoisonDefaults(&aff[1], level, duration);
      aff[1].location=APPLY_SPE;
      aff[1].modifier=-40;
      break;
    case LIQ_POISON_VIOLET_FUNGUS:
      addPoisonDefaults(&aff[0], level, duration);
      aff[0].location=APPLY_IMMUNITY;
      aff[0].modifier=IMMUNE_SLEEP;
      aff[0].modifier2=-30;
      break;
    case LIQ_POISON_DEVIL_ICE:
      addPoisonDefaults(&aff[0], level, duration);
      aff[0].location=APPLY_IMMUNITY;
      aff[0].modifier=IMMUNE_HEAT;
      aff[0].modifier2=-20;
      break;
    case LIQ_POISON_FIREDRAKE:
      addPoisonDefaults(&aff[0], level, duration);
      aff[0].location=APPLY_IMMUNITY;
      aff[0].modifier=IMMUNE_COLD;
      aff[0].modifier2=-20;
      break;
    case LIQ_POISON_INFANT:
      addPoisonDefaults(&aff[0], level, duration);
      aff[0].location=APPLY_IMMUNITY;
      aff[0].modifier=IMMUNE_DRAIN;
      aff[0].modifier2=-20;
      break;
    case LIQ_POISON_PEA_SEED:
      addPoisonDefaults(&aff[0], level, duration);
      aff[0].location=APPLY_SPE;
      aff[0].modifier=-20;
      break;
    case LIQ_POISON_ACACIA:
      addPoisonDefaults(&aff[0], level, duration);
      aff[0].location=APPLY_STR;
      aff[0].modifier=-40;
      break;
    case LIQ_POISON_STANDARD:
      addPoisonDefaults(&aff[0], level, duration);
      aff[0].location=APPLY_STR;
      aff[0].modifier=-20;
      break;
    default:
      return false;
      break;
  }
  return true;
}

int TBaseCup::poisonMePoison(TBeing *ch, TBaseWeapon *weapon)
{
  int j;
  int level;
  int duration;
  sstring s;
  spellNumT skill=SKILL_POISON_WEAPON;
    
  if (getDrinkUnits() <= 0) {
    act("$p seems not to have anything in it.",
       TRUE, ch, this, 0, TO_CHAR);
    return FALSE;
  }
  level = ch->getSkillValue (skill) / 2;
  int bKnown = ch->getSkillValue (skill);

  duration = (level << 2) * UPDATES_PER_MUDHOUR;
  if (ch->bSuccess(bKnown, skill)) {
    for (j = 0; j < MAX_SWING_AFFECT; j++) {
      if (weapon->isPoisoned()) {
        ch->sendTo("That weapon is already affected by poison!\n\r");
        return FALSE;
      }
    }

    weapon->setPoison(getDrinkType());
    
    s = fmt("You coat $p with %s.") % liquidInfo[getDrinkType()]->name;
    act(s, FALSE, ch, weapon, NULL, TO_CHAR);
    s = fmt("$n coats $p with %s.") % liquidInfo[getDrinkType()]->name;
    act(s, FALSE, ch, weapon, NULL, TO_ROOM);
  } else {
    if(critFail(ch, skill) != CRIT_F_NONE){
      act("You slip up and cut yourself with $p!", 
	  FALSE, ch, weapon, NULL, TO_CHAR);
      act("$n slips up and cuts $mself with $p!",
	  FALSE, ch, weapon, NULL, TO_ROOM);

      act("There was something nasty on that $o!",
	  FALSE, ch, weapon, ch, TO_VICT, ANSI_RED);
      act("You inflict something nasty on yourself!",
	  FALSE, ch, weapon, ch, TO_CHAR, ANSI_RED);
      act("There was something nasty on that $o!",
	  FALSE, ch, weapon, ch, TO_NOTVICT, ANSI_RED);


      doLiqSpell(ch, ch, getDrinkType(), 1);
    } else {
      weapon->setPoison(LIQ_WATER);
      
      s = fmt("You coat $p with %s.") % liquidInfo[getDrinkType()]->name;
      act(s, FALSE, ch, weapon, NULL, TO_CHAR);
      s = fmt("$n coats $p with %s.") % liquidInfo[getDrinkType()]->name;
      act(s, FALSE, ch, weapon, NULL, TO_ROOM);
    }
  }

  addToDrinkUnits(-1);
  return TRUE;
}

int poisonWeapon(TBeing *ch, TThing * weapon, TThing *poison)
{
  return weapon->poisonWeaponWeapon(ch, poison);
}

int TThing::poisonWeaponWeapon(TBeing *ch, TThing *)
{
  ch->sendTo("You can't poison that!  It's not a weapon!\n\r");
  return FALSE;
}

int TBeing::doGarrotte(const char * argument, TBeing *vict)
{
  TBeing *victim;
  char namebuf[256];
  int rc;

  if (!doesKnowSkill(SKILL_GARROTTE)) {
    sendTo("You know nothing about garrotting.\n\r");
    return FALSE;
  }
  if (checkBusy()) {
    return FALSE;
  }
  strcpy(namebuf, argument);

  if (!(victim = vict)) {
    if (!(victim = get_char_room_vis(this, namebuf))) {
      sendTo("Strangle whom?\n\r");
      return FALSE;
    }
  }
  if (!sameRoom(*victim)) {
    sendTo("That person isn't around.\n\r");
    return FALSE;
  }
  if (victim->isImmortal()) {
    sendTo("Sorry....that shit don't fly anymore.\n\r");
    return FALSE;
  }
  if (victim->isUndead()) {
    sendTo("You can't garrotte the undead.\n\r");
    return FALSE;
  }
  rc = garrotte(this, victim);
  if (rc)
    addSkillLag(SKILL_GARROTTE, rc);
  if (IS_SET_DELETE(rc, DELETE_VICT)) {
    if (vict)
      return rc;
    delete victim;
    victim = NULL;
    REM_DELETE(rc, DELETE_VICT);
  }
  return rc;
}

int TThing::garotteMe(TBeing *thief, TBeing *)
{
  thief->sendTo("Only a specialized garrotte can be used for this.\n\r");
  return FALSE;
}

int TTool::garotteMe(TBeing *thief, TBeing *victim)
{
  int level;

  if (getToolType() != TOOL_GARROTTE) {
    thief->sendTo("Only a specialized garrotte can be used for this.\n\r");
    return FALSE;
  }
  if (victim->equipment[WEAR_NECK]) {
    act("$N's $o prevents you from garrotting $M.",
        FALSE, thief, victim->equipment[WEAR_NECK], victim, TO_CHAR);
    return FALSE;
  }
  if (victim->isImmune(IMMUNE_SUFFOCATION)) {
    act("You can't garrotte $N, $E doesn't seem to need to breathe.",
        false, thief, 0, victim, TO_CHAR);
    return FALSE;
  }

  level = thief->getSkillLevel(SKILL_GARROTTE);
  int bKnown = thief->getSkillValue(SKILL_GARROTTE);

  thief->reconcileHurt(victim,0.05);

  int dam = thief->getSkillDam(victim, SKILL_GARROTTE, level, thief->getAdvLearning(SKILL_GARROTTE));

  if (this != (thief->unequip(thief->getPrimaryHold()))) {
    vlogf(LOG_BUG, "Error in garotte");
  }
  if (thief->bSuccess(bKnown, SKILL_GARROTTE) || !victim->awake()) {
    affectedData aff;

    victim->equipChar(this, WEAR_NECK);
    act("You throw $p around $N's neck.", FALSE, thief, this, victim, TO_CHAR);
    act("$n throws $p around $N's neck.", FALSE, thief, this, victim, TO_NOTVICT);
    act("$n throws $p around your neck.", FALSE, thief, this, victim, TO_VICT);

    aff.type = AFFECT_DISEASE;
    aff.level = thief->getSkillLevel(SKILL_GARROTTE);
    aff.duration = PERMANENT_DURATION;
    aff.modifier = DISEASE_GARROTTE;
    aff.location = APPLY_NONE;
    aff.bitvector = AFF_SILENT;

    // we've made raw immunity check, but allow it to reduce effects too
    aff.duration *= (100 - victim->getImmunity(IMMUNE_SUFFOCATION));
    aff.duration /= 100;

    victim->affectTo(&aff);

  } else {
    dam = 0;
    *thief->roomp += *this;
    act("You miss your attempt to throw $p around $N's neck.", FALSE, thief, this, victim, TO_CHAR);
    act("$n misses $s attempt to throw $p around $N's neck.", FALSE, thief, this, victim, TO_NOTVICT);
    act("$n misses $s attempt to throw $p around your neck.", FALSE, thief, this, victim, TO_VICT);
  }
  addToToolUses(-1);
  if (getToolUses() <= 0) {
    act("Your $o snaps!!",TRUE,thief,this,0,TO_CHAR);
    act("$n's $o snaps.",TRUE,thief,this,0,TO_ROOM);
    return DELETE_THIS;
  }
  if (thief->reconcileDamage(victim, dam, SKILL_GARROTTE) == -1)
    return DELETE_VICT;
  victim->addHated(thief);
  return TRUE;
}

int garrotte(TBeing *thief, TBeing * victim)
{
  TThing * obj;
  int rc;

  if (thief->checkPeaceful("Naughty, naughty.  None of that here.\n\r"))
    return FALSE;

  if (victim == thief) {
    thief->sendTo("How can you sneak up on yourself?\n\r");
    return FALSE;
  }
  if (thief->riding) {
    thief->sendTo("Not while mounted!\n\r");
    return FALSE;
  }
  if (dynamic_cast<TBeing *> (victim->riding)) {
    thief->sendTo("Not while that person is mounted!\n\r");
    return FALSE;
  }
  if (thief->noHarmCheck(victim))
    return FALSE;
  if (thief->attackers) {
    thief->sendTo("There's no way to do that while you're fighting!\n\r");
    return FALSE;
  }
  if (!(obj = thief->heldInPrimHand())) {
    thief->sendTo("You need to be holding the garrotte to make it a success.\n\r");
    return FALSE;
  }
  rc = obj->garotteMe(thief, victim);
  if (IS_SET_DELETE(rc, DELETE_THIS)) {
    delete obj;
    return TRUE;
  } else if (IS_SET_DELETE(rc, DELETE_VICT)) {
    return DELETE_VICT;
  } else if (rc)
    return TRUE;
  
  return FALSE;
}

int TBeing::doCudgel(const char * argument, TBeing *vict)
{
  TBeing *victim;
  char namebuf[256];
  int rc;

  if (!doesKnowSkill(SKILL_CUDGEL)) {
    sendTo("You know nothing about cudgeling.\n\r");
    return FALSE;
  }

  if (checkBusy()) {
    return FALSE;
  }
  strcpy(namebuf, argument);

  if (!(victim = vict)) {
    if (!(victim = get_char_room_vis(this, namebuf))) {
      sendTo("Cudgel whom?\n\r");
      return FALSE;
    }
  }
  if (!sameRoom(*victim)) {
    sendTo("That person isn't around.\n\r");
    return FALSE;
  }
  rc = cudgel(this, victim);
  if (rc)
    addSkillLag(SKILL_CUDGEL, rc);
  if (IS_SET_DELETE(rc, DELETE_VICT)) {
    if (vict)
      return rc;
    delete victim;
    victim = NULL;
    REM_DELETE(rc, DELETE_VICT);
  }
  return rc;
}

int cudgel(TBeing *thief, TBeing *victim)
{
  int level = thief->getSkillLevel(SKILL_CUDGEL);
  int bKnown = thief->getSkillValue(SKILL_CUDGEL);
  int i;

  if (thief->checkPeaceful("Naughty, naughty.  None of that here.\n\r"))
    return FALSE;

  TGenWeapon * obj = dynamic_cast<TGenWeapon *>(thief->heldInPrimHand());

  if (!obj || !obj->canCudgel()) {
    // Allow high learndness to use offhand, for a cost.
    if (bKnown >= 80) {
      obj    = dynamic_cast<TGenWeapon *>(thief->heldInSecHand());
      level  = max(1, (level - 10));
      bKnown = max(1, (bKnown - 20));
    }

    if (!obj) {
      thief->sendTo("You need to wield a weapon, to make it a success.\n\r");
      return FALSE;
    }
  }

  if (thief->riding) {
    thief->sendTo("Not while mounted!\n\r");
    return FALSE;
  }

  if (dynamic_cast<TBeing *> (victim->riding)) {
    thief->sendTo("Not while that person is mounted!\n\r");
    return FALSE;
  }

  if (thief->noHarmCheck(victim))
    return FALSE;

  if (!obj->canCudgel()) {
    act("You can't use $o to cudgel.", false, thief, obj, NULL, TO_CHAR);
    return FALSE;
  }

  // Jesus's fix for cudgel...cheesy as hell but oh well
  if (!victim->isHumanoid()) {
    thief->sendTo("You cannot cudgel that creature.\n\r");
    return FALSE;
  }

  // Jesus's fix for cudgel...cheesy as hell but oh well
  if (victim->isUndead()) {
    thief->sendTo("You cannot cudgel an undead creature.\n\r");
    return FALSE;
  }
  // Jesus's fix for cudgel...cheesy as hell but oh well
  if (victim->isFlying() && !(thief->isFlying())) {
    thief->sendTo("You cannot cudgel them while they are flying.\n\r");
    return FALSE;
  }
  // Jesus's fix for cudgel...cheesy as hell but oh well
  if (6*thief->GetMaxLevel() < 5*victim->GetMaxLevel()) {
    act("You miss your attempt to knock $N unconscious.", FALSE, thief, obj, victim, TO_CHAR);
    act("$n misses $s attempt to knock $N unconscious.", FALSE, thief, obj, victim, TO_NOTVICT);
    act("$n misses $s attempt to knock you unconscious.", FALSE, thief, obj, victim, TO_VICT);
    thief->reconcileDamage(victim, 0, SKILL_CUDGEL);
    victim->addHated(thief);
    return TRUE;
  }
  // Jesus's fix for cudgel...cheesy as hell but oh well
  if (3*thief->getHeight() < 2*victim->getHeight() && !(thief->isFlying())) {
    thief->sendTo("You don't stand a chance at cudgeling a creature that tall.\n\r");
    return FALSE;
  }

  if (victim->equipment[WEAR_HEAD]) {
    act("$N's $o prevents you from cudgeling $M.",
        FALSE, thief, victim->equipment[WEAR_HEAD], victim, TO_CHAR);
    return FALSE;
  }

  if (victim->fight())
    bKnown /= 2;
  thief->reconcileHurt(victim,0.06);
  if ((thief->bSuccess(bKnown, SKILL_CUDGEL) && 
       !thief->isNotPowerful(victim, level, SKILL_CUDGEL, SILENT_YES)) || !victim->awake()) {
    if ((i = thief->specialAttack(victim, SKILL_CUDGEL)) ||
	(i == GUARANTEED_SUCCESS)) {
      act("You knock $N on the noggin, knocking $M unconscious.", FALSE, thief, obj, victim, TO_CHAR);
      act("$n knocks $N on the noggin, knocking $M unconscious.", FALSE, thief, obj, victim, TO_NOTVICT);
      act("WHAM!  Something smacks into your skull HARD!", FALSE, thief, obj, victim, TO_VICT, ANSI_RED);
      act("All you can see are stars.", FALSE, thief, obj, victim, TO_VICT);
      victim->setPosition(POSITION_STUNNED);
      if (victim->task) {
	victim->stopTask();
      }
      if (victim->spelltask) {
	victim->stopCast(STOP_CAST_NONE);
      }

      // erm, they snuck up on them, so no idea who did it
      // victim->addHated(thief);

      affectedData aff;
      aff.type = SKILL_CUDGEL;
      aff.duration = UPDATES_PER_MUDHOUR / 3;
      aff.bitvector = AFF_STUNNED;

      victim->affectTo(&aff, -1);
      return TRUE;
    }
  }
  act("You miss your attempt to knock $N unconscious.", FALSE, thief, obj, victim, TO_CHAR);
  act("$n misses $s attempt to knock $N unconscious.", FALSE, thief, obj, victim, TO_NOTVICT);
  act("$n misses $s attempt to knock you unconscious.", FALSE, thief, obj, victim, TO_VICT);

  thief->reconcileDamage(victim, 0, SKILL_CUDGEL);
  victim->addHated(thief);

  return TRUE;
}

