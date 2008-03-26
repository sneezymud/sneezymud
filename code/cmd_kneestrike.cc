//////////////////////////////////////////////////////////////////////////
//
// SneezyMUD - All rights reserved, SneezyMUD Coding Team
//
//////////////////////////////////////////////////////////////////////////


#include "stdsneezy.h"
#include "combat.h"

bool TBeing::canKneestrike(TBeing *victim, silentTypeT silent)
{
  if (checkBusy()) {
    return FALSE;
  }
  if (affectedBySpell(AFFECT_TRANSFORMED_LEGS)) {
    sendTo("You don't know how to knee someone with these kind of legs.\n\r");
    return FALSE;
  }
  if (getPosition() == POSITION_CRAWLING) {
    sendTo("You can't kneestrike while crawling.\n\r");
    return FALSE;
  }
  if (!doesKnowSkill(SKILL_KNEESTRIKE)) {
    sendTo("You know nothing about knee strikes.\n\r");
    return FALSE;
  }
  if (checkPeaceful("You feel too peaceful to contemplate violence.\n\r"))
    return FALSE;

  if (victim == this) {
    if (!silent)
      sendTo("You try to knee yourself in the groin, but it doesn't seem to work...\n\r");
    return FALSE;
  }
  if (riding) {
    if (!silent)
      sendTo("You would fall off your mount if you tried that!\n\r");
    return FALSE;
  }
  if (noHarmCheck(victim))
    return FALSE;
  
  if (victim->isImmortal() || IS_SET(victim->specials.act, ACT_IMMORTAL)) {
    if (!silent)
      sendTo("Attacking an immortal wouldn't be very smart.\n\r");
    return FALSE;
  }

  if (!hasLegs()) {
    if (!silent)
      sendTo("It's very hard to kneestrike without any knees!\n\r");
    return FALSE;
  }

  if (isFourLegged()) {
    if (!silent)
      sendTo("You have way too many legs to be kneestriking!\n\r");
    return FALSE;
  }

  if (eitherLegHurt()) {
    if (!silent)
      sendTo("It's very hard to kneestrike without the use of your legs!\n\r");
    return FALSE;
  }

  if (victim->getPosition() < POSITION_STANDING){
    if (!silent)
      sendTo(fmt("That might work, but your victim seems to be on the %s.\n\r") % 
		   roomp->describeGround());
    return FALSE;
  }

  if (victim->isFlying()) {
    if (!silent)
      sendTo("You can't kneestrike something that is flying.\n\r");
    return FALSE;
  }
  if (victim->getHeight() < getPartMinHeight(ITEM_WEAR_LEGS)) {  
    if (!silent)
      sendTo("Your victim is too far below you to kneestrike.\n\r");    
    return FALSE;
  }
  if (victim->getHeight() < getPartMinHeight(ITEM_WEAR_FEET)) {
    if (!silent)
      sendTo("Your victim is too far above you to kneestrike.\n\r");
    return FALSE;
  }   


  return true;
}

static int kneestrikeMiss(TBeing *c, TBeing *v, int type)
{
  switch(type){
  case 0:{ // normal
    act("$n tries to plant a knee on $N, but fails.", 
	FALSE, c, 0, v, TO_NOTVICT);
    act("$N moves out of the way and you miss your knee strike.", 
	FALSE, c, 0, v, TO_CHAR);
    act("$n tries to hit you with $s knee, but you dodge it.", 
	FALSE, c, 0, v, TO_VICT);
  } break;
  case 1:{ // monk counter move
    act("$N grabs your knee, and hits you in the side with a powerful roundhouse kick.",
        FALSE, c, 0, v, TO_CHAR);
    act("You grab $n's knee, and use the opening to hit $m in the side with a powerful roundhouse kick.",
        FALSE, c, 0, v, TO_VICT);
    act("$N grabs $n's knee, and hit $m in the side with a powerful roundhouse kick.",
        FALSE, c, 0, v, TO_NOTVICT);

    // do 1/2 the dam we would have done to caster
    // yes, I mean to use caster below since vict doesn't have this skill...
    int dam = c->getSkillDam(v, SKILL_KNEESTRIKE, c->getSkillLevel(SKILL_KNEESTRIKE), c->getAdvLearning(SKILL_KNEESTRIKE)) / 2;

    c->sendTo("Ooof! That knocked some wind out of you.\n\r");
    c->cantHit += v->loseRound(0.25);
    c->addToWait(combatRound(0.25));
    TObj *item;
    if ((item = dynamic_cast<TObj *>(v->equipment[v->getPrimaryFoot()])))
      if (item->isSpiked() || item->isObjStat(ITEM_SPIKED)) {

	act("The spikes on your $o sink into $N's side.", FALSE, v, item, c, TO_CHAR);
	act("The spikes on $n's $o sink into $N's side.", FALSE, v, item, c, TO_NOTVICT);
	act("The spikes on $n's $o sink into your side, OW!", FALSE, v, item, c, TO_VICT);

        if (v->reconcileDamage(c, (int)(dam*0.15), TYPE_STAB) == -1)
          return DELETE_THIS;
      }
    if (v->reconcileDamage(c, dam, DAMAGE_KICK_HEAD) == -1)
      return DELETE_THIS;
  } break;
  /* Old monk Kneestrike counter move
  case 1:{ // monk counter move
    act("$n tries to plant a knee on $N, but $E nimbly leaps out of the way.", 
	FALSE, c, 0, v, TO_NOTVICT);
    act("$N nimbly leaps out of the way and you miss your knee strike.", 
	FALSE, c, 0, v, TO_CHAR);
    act("$n tries to hit you with $s knee, but you nimbly leap out of the way.", 
	FALSE, c, 0, v, TO_VICT);    
  } break;
  */
  }

  c->addToWait(combatRound(0.25)); // lag extra round for miss
  c->reconcileDamage(v, 0,SKILL_KNEESTRIKE);
  return TRUE;
}

static int kneestrikeHit(TBeing *c, TBeing *victim)
{
  int rc = 0;
  TObj *item;
  int h_dam = 1;
  int caster_hgt, victim_hgt;
  int i;
  wearSlotT pos = WEAR_NOWHERE;
  wearSlotT caster_pos = WEAR_NOWHERE;
  int crit;
  //  int lev = c->getSkillLevel(SKILL_KNEESTRIKE);
  int spikeddam=0;
  int dam = c->getSkillDam(victim, SKILL_KNEESTRIKE, c->getSkillLevel(SKILL_KNEESTRIKE), c->getAdvLearning(SKILL_KNEESTRIKE));

  actToParmT msg_tgt[]={TO_CHAR, TO_VICT, TO_NOTVICT};
  const char *hit_msg[][3]={
    // shorter than victims foot
    {"You try to knee $N's foot, but $E is just too high up right now.",
     "$n tries to knee your foot, but fails.",
     "$n tries to knee $N's foot, but fails."},
    // shorter than victims legs
    {"You slam your knee into $S foot.",
     "$n slams $s head into your foot.",
     "$n slams $s knee into $N's foot."},
    // shorter than victims waist
    {"You knee $N, knocking $S shin painfully.", 
     "$n knees you, knocking your shin painfully.",
     "$n knees $N, knocking $N's shin painfully."},
    // shorter than victims body
    {"You butt knees with $N, yow that kinda hurt!", 
     "$n butts knees with you, yow that kinda hurt!",
     "$n butts knees with $N, bet that hurt!"},
    // shorter than victims arms
    {"You slam your knee into $N's thigh.  Charley horse!", 
     "$n slams $s knee into your thigh.  Charley horse!",
     "$n slams $s knee into $N's thigh.  Charley horse!"},
    // taller than victims arms and victim taller than casters arms
    {"You jerk your knee into $N's crotch.",
     "$n jerks $s knee into your crotch.",
     "$n jerks $s knee into $N's crotch."},
    // victim shorter than arms
    {"You plant your knee into $N's solar plexus.", 
     "$n plants $s knee into your solar plexus.",    
     "$n plants $s knee into $N's solar plexus."},
    // victim shorter than body
    {"$N's head jerks back sharply as you knee $S chin hard.", 
     "Your head jerks back sharply as $n knees your chin hard.",
     "$N's head jerks back sharply as $n knees $S chin hard."},
    // victim shorter than waist
    {"You smash your knee into $N's face.", 
     "$n smashes $s knee into your face.",
     "$n smashes $s knee into $N's face."},
    // victim shorter than foot
    {"You try to hit $N with your knee, but $E is out of reach.",
     "$n tries to hit you with $s knee, but you are out of reach.",
     "$n tries to hit $N with $s knee, but $N is out of reach."}

  };
  const char *crit_msg[][3]={
    {"You try to strike $N with your knee, but you slip and tumble to the ground!",
     "$n tries to hit you with $s knee, but slips and tumbles to the ground.",
     "$n tries to hit $N with $s knee, but slips and tumbles to the ground."},
    {"You feel a rush of adrenalin as you execute a near perfect knee strike!",
     "You cringe in pain as $n hits you REALLY hard with $s knee!",
     "$n grins in triumph as $e slams $N with $s knee HARD!"},
    {"You smile inwardly as you drive your knee home with extra force.",
     "You frown to yourself as $n drives $s knee home with extra force.",
     "$n nails $N with a well executed knee strike."},
    {"You slip and almost fall, but regain your footing. Whew!",
     "$n slips and almost falls as $e hits you weakly.",
     "$n almost falls and $e barely hits $n with $s knee."}
  };
  if (!victim)
    return FALSE;
    
  crit=critSuccess(c, SKILL_KNEESTRIKE);
  switch(crit){
    // would like to add broken limbs for CRIT_S_KILL eventually
    case CRIT_S_KILL:
    case CRIT_S_TRIPLE:
      dam*=2; 
      for(i=0;i<3;++i)
         act(crit_msg[1][i], FALSE, c, 0, victim, msg_tgt[i]);
      break;
    case CRIT_S_DOUBLE:
      dam=(int)(dam*1.5);
      for(i=0;i<3;++i) act(crit_msg[2][i], FALSE, c, 0, victim, msg_tgt[i]);
      break;
    case CRIT_S_NONE:
      break;
  }
#if 0
// why the fuck are we doing critFail inside the success case?
    crit=critFail(c, SKILL_KNEESTRIKE);    
    if(crit==CRIT_F_HITSELF){  // reduced damage
      dam/=2;
    for(i=0;i<3;++i) act(crit_msg[3][i], FALSE, c, 0, victim, msg_tgt[i]);
    } else if(crit==CRIT_F_HITOTHER){  // fall down
    for(i=0;i<3;++i) act(crit_msg[0][i], FALSE, c, 0, victim, msg_tgt[i]);

    rc = c->crashLanding(POSITION_SITTING);
    if (IS_SET_DELETE(rc, DELETE_THIS))
      return DELETE_THIS;
    
    rc = c->trySpringleap(victim);
    if (IS_SET_DELETE(rc, DELETE_THIS) || IS_SET_DELETE(rc, DELETE_VICT))
      return rc;
    
    c->reconcileDamage(victim, 0,SKILL_KNEESTRIKE);   
    return TRUE;
    }
#endif

  caster_hgt = c->getHeight();
  victim_hgt = victim->getHeight();
  spellNumT dam_type = SKILL_KNEESTRIKE;

  if (caster_hgt < victim->getPartMinHeight(ITEM_WEAR_FEET)) {
    // caster too short, no hit
    for(i=0;i<3;++i) act(hit_msg[0][i], FALSE, c, 0, victim, msg_tgt[i]);

    if ((rc = c->reconcileDamage(victim, 0,dam_type)) == -1)
      return DELETE_VICT;
    return TRUE;    
  } else if (caster_hgt < victim->getPartMinHeight(ITEM_WEAR_LEGS)) {
    // target foot
    pos = (::number(0,1) ? WEAR_FOOT_L : WEAR_FOOT_R);
    dam_type = DAMAGE_KNEESTRIKE_FOOT;

    for(i=0;i<3;++i) act(hit_msg[1][i], FALSE, c, 0, victim, msg_tgt[i]);
  } else if (caster_hgt < victim->getPartMinHeight(ITEM_WEAR_WAIST)) {
    // target shin
    pos = (::number(0,1) ? WEAR_LEG_L : WEAR_LEG_R);
    dam_type = DAMAGE_KNEESTRIKE_SHIN;

    for(i=0;i<3;++i) act(hit_msg[2][i], FALSE, c, 0, victim, msg_tgt[i]);
  } else if (caster_hgt < victim->getPartMinHeight(ITEM_WEAR_BODY)) {
    // target knee
    pos = (::number(0,1) ? WEAR_LEG_L : WEAR_LEG_R);
    dam_type = DAMAGE_KNEESTRIKE_KNEE;
    
    for(i=0;i<3;++i) act(hit_msg[3][i], FALSE, c, 0, victim, msg_tgt[i]);
  } else if (caster_hgt < victim->getPartMinHeight(ITEM_WEAR_ARMS)) {
    // target thigh
    pos = (::number(0,1) ? WEAR_LEG_L : WEAR_LEG_R);
    dam_type = DAMAGE_KNEESTRIKE_THIGH;

    for(i=0;i<3;++i) act(hit_msg[4][i], FALSE, c, 0, victim, msg_tgt[i]);
  } else if (caster_hgt >= victim->getPartMinHeight(ITEM_WEAR_ARMS) &&
	     victim_hgt >= c->getPartMinHeight(ITEM_WEAR_ARMS)) {
    // target crotch
    pos = WEAR_WAIST;
    dam_type = DAMAGE_KNEESTRIKE_CROTCH;

    for(i=1;i<3;++i) act(hit_msg[5][i], FALSE, c, 0, victim, msg_tgt[i]);

    if (victim->getSex() == SEX_MALE) {
      act("You jerk your knee into $N's crotch.  Felt like you got both of them.", 
	  FALSE, c, 0, victim, TO_CHAR);
      if (!victim->equipment[WEAR_WAIST] || 
	  isname("belt", victim->equipment[WEAR_WAIST]->name)) {
	// no equipment or just a belt
	victim->sendTo("Your voice just went up an octave.  Ouch!\n\r");
	dam += 10;
	victim->cantHit += victim->loseRound(0.25);
      } else {
	victim->sendTo(fmt("Good thing you were wearing your %s.\n\r") %
		       fname(victim->equipment[WEAR_WAIST]->name));
      }
    } else {
      act(hit_msg[5][0], FALSE, c, 0, victim, TO_CHAR);
    }
  } else if (victim_hgt < c->getPartMinHeight(ITEM_WEAR_LEGS)) {
    // victim too short, no hit
    for(i=0;i<3;++i) act(hit_msg[9][i], FALSE, c, 0, victim, msg_tgt[i]);

    if ((rc = c->reconcileDamage(victim, 0,dam_type)) == -1)
      return DELETE_VICT;
    return TRUE;    
  } else if (victim_hgt < c->getPartMinHeight(ITEM_WEAR_WAIST)) {
    // target skull
    pos = WEAR_HEAD;
    dam_type = DAMAGE_KNEESTRIKE_FACE;
      
    for(i=0;i<3;++i) act(hit_msg[8][i], FALSE, c, 0, victim, msg_tgt[i]);
  } else if (victim_hgt < c->getPartMinHeight(ITEM_WEAR_BODY)) {
    // target chin
    pos = WEAR_HEAD;
    dam_type = DAMAGE_KNEESTRIKE_CHIN;

    for(i=0;i<3;++i) act(hit_msg[7][i], FALSE, c, 0, victim, msg_tgt[i]);
  } else if (victim_hgt < c->getPartMinHeight(ITEM_WEAR_ARMS)) {
    // target solar plexus
    pos = WEAR_BODY;
    dam_type = DAMAGE_KNEESTRIKE_SOLAR;

    for(i=0;i<3;++i) act(hit_msg[6][i], FALSE, c, 0, victim, msg_tgt[i]);
  } else {
  // target solar plexus
    pos = WEAR_BODY;
    dam_type = DAMAGE_KNEESTRIKE_SOLAR;
    for(i=0;i<3;++i) act(hit_msg[6][i], FALSE, c, 0, victim, msg_tgt[i]);
    c->sendTo("Please tell Peel you saw this message (kneestrike)\n\r");
  }

  // advance learning gave increased success, this seems bad idea
  //  dam += c->getAdvLearning(SKILL_KNEESTRIKE)/10;
  
  // apply damage to caster if no leg eq
  caster_pos = (::number(0,1) ? WEAR_LEG_L : WEAR_LEG_R);
  if (!(item = dynamic_cast<TObj *>(c->equipment[caster_pos]))) {
    rc = c->damageLimb(c, caster_pos, 0, &h_dam);
    if (IS_SET_DELETE(rc, DELETE_VICT))
      return DELETE_THIS;
  } else if(item->isSpiked() || item->isObjStat(ITEM_SPIKED)){
    spikeddam=(int) (dam*0.15);
    
    act("The spikes on your $o sink into $N.", FALSE, c, item, victim, TO_CHAR);
    act("The spikes on $n's $o sink into $N.", FALSE, c, item, victim, TO_NOTVICT);
    act("The spikes on $n's $o sink into you.", FALSE, c, item, victim, TO_VICT);

  } else {
  // apply damage to victim if no eq on targetted spot
    if (!(item = dynamic_cast<TObj *>(victim->equipment[pos]))) {
      rc = c->damageLimb(victim, pos, 0, &h_dam);
      if (IS_SET_DELETE(rc, DELETE_VICT))
	return DELETE_VICT;
    }
  }

  if(spikeddam)
    if((rc = c->reconcileDamage(victim, spikeddam, TYPE_STAB)) == -1)
      return DELETE_VICT;
  if ((rc = c->reconcileDamage(victim, dam, dam_type)) == -1)
    return DELETE_VICT;

  return TRUE;
}

static int kneestrike(TBeing *caster, TBeing *victim)
{
  int percent;
  int i = 0;
  int rc = 0;
  const int KNEESTRIKE_MOVE   = 15;
  int adv=caster->getAdvLearning(SKILL_KNEESTRIKE);
  
  if (!caster->canKneestrike(victim, SILENT_NO))
    return FALSE;

  percent = ((10 - (victim->getArmor() / (10+adv/10))) << 1);
  percent += caster->getDexReaction() * 5;
  percent -= victim->getAgiReaction() * 5;
  
  int bKnown = caster->getSkillValue(SKILL_KNEESTRIKE);
  
  if (caster->getMove() < KNEESTRIKE_MOVE) {
    caster->sendTo("You lack the vitality.\n\r");
    return FALSE;
  }
  caster->addToMove(-(KNEESTRIKE_MOVE-(adv/20)));

  // keep bSucc at end so counters are OK
  i = caster->specialAttack(victim,SKILL_KNEESTRIKE);
  if (i &&
      i != GUARANTEED_FAILURE &&
      percent < bKnown &&
      caster->bSuccess(bKnown + percent, SKILL_KNEESTRIKE)) {

    if (victim->canCounterMove((bKnown-adv/2)/3)) {
      SV(SKILL_KNEESTRIKE);
      rc = kneestrikeMiss(caster, victim, 1);
      if (IS_SET_DELETE(rc, DELETE_THIS) || IS_SET_DELETE(rc, DELETE_VICT))
	return rc;
      return TRUE;
    }

    return (kneestrikeHit(caster, victim));
  } else {
    rc = kneestrikeMiss(caster, victim, 0);
    if (IS_SET_DELETE(rc, DELETE_THIS) || IS_SET_DELETE(rc, DELETE_VICT))
      return rc;
  }
  return TRUE;
}

int TBeing::doKneestrike(const char *argument, TBeing *vict)
{
  int rc;
  TBeing *v;
  char name_buf[256];
  
  strcpy(name_buf, argument);
  
  if (!(v = vict)) {
    if (!(v = get_char_room_vis(this, name_buf))) {
      if (!(v = fight())) {
        sendTo("Knee strike whom?\n\r");
        return FALSE;
      }
    }
  }
  if (!sameRoom(*v)) {
    sendTo("That person doesn't seem to be around.\n\r");
    return FALSE;
  }
  rc = kneestrike(this, v);
  if (rc) 
    addSkillLag(SKILL_KNEESTRIKE, rc);

  if (IS_SET_ONLY(rc, DELETE_VICT)) {
    if (vict)
      return rc;
    delete v;
    v = NULL;
    REM_DELETE(rc, DELETE_VICT);
  }
  return rc;
}







