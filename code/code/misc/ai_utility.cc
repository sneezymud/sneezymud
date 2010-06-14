//////////////////////////////////////////////////////////////////////////
//
// SneezyMUD - All rights reserved, SneezyMUD Coding Team
//
//      "ai_utility.cc" - handles utility routines needed by the mob AI
//        code.
//
//////////////////////////////////////////////////////////////////////////

// A general note on AI:  This is designed so that NPC's interact with PC's
//   hence at this time we are NOT allowing (except accidentally) for
//   opinions.target to be an NPC.  It might be interesting later to allow
//   target to be an NPC (mobs interacting with mobs) but my feeling is
//   that this will lead to endless loop (A's action triggering B's triggering 
//   A's triggering B's...).  The problem with the present system is that
//   a mob could piss off another mob, and the pissed off mob "think"
//   it was some random PC that did nothing. */

#include "being.h"
#include "room.h"
#include "low.h"
#include "extern.h"
#include "monster.h"
#include "combat.h"
#include "spec_mobs.h"

// This function should be used if you want to see if the mob is "pissed" 
int TMonster::pissed(void)
{
  if (UtilMobProc(this)) 
    return FALSE;

  if (isAngry() && isMalice()) 
    return TRUE;

  return FALSE;
}

// This function should be used if you want to see if ch is REALLY pissed 
// aggro should be called for most things leading to fights 
// the 4*anger+5*malice thing allows for "love/hate" fights 
int TMonster::aggro(void)
{
  if (UtilMobProc(this))
    return FALSE;

  if (GuildProcs(spec))
    return FALSE;

  if(isPet(PETTYPE_PET | PETTYPE_CHARM | PETTYPE_THRALL))
    return FALSE;

  if (isAngry() && isMalice()) {
    if ((4*anger() + 5*malice()) >=450 )
      return TRUE;
  }

  if(IS_SET(specials.act, ACT_AGGRESSIVE))
    return TRUE;

  return FALSE;
}

// function designed for use in conjunction with aggro 
// it returns TRUE if the char is real ugly and the mob is sufficient
// low level as to be little challenge for the char.  A pissed roll also
// has to be passed. 
int TMonster::aiUglyMug(TBeing *tmp_ch)
{
  int lmob = GetMaxLevel(), ltmp = tmp_ch->GetMaxLevel();
 
  if (lmob < 7)
    return FALSE;
  if (ltmp < 10)
    return FALSE;
  if(isAffected(AFF_CHARM))
    return FALSE;
  if (!tmp_ch->isRealUgly())
    return FALSE;
  if ((lmob * (tmp_ch->getStat(STAT_CURRENT, STAT_CHA) - 1)/10) > ltmp)
    return FALSE;
  if (!pissed() || ::number(0,4))
    return FALSE;
  if (tmp_ch->getHit() < 11)
    return FALSE;
  if (UtilMobProc(this))
    return FALSE;
  if (!isHumanoid())
    return FALSE;

  switch (::number(1,3)) {
    case 1:
      act("$N finds you repulsive!",TRUE,tmp_ch,0,this,TO_CHAR);
      act("$N finds $n utterly repulsive.",TRUE,tmp_ch,0,this,TO_ROOM);
      break;
    case 2:
      act("$N doesn't like your attitude!",TRUE,tmp_ch,0,this,TO_CHAR);
      act("$N finds $n's attitude offensive.",TRUE,tmp_ch,0,this,TO_ROOM);
      break;
    case 3:
    default:
      act("$N can't stand to be around you any longer!",TRUE,tmp_ch,0,this,TO_CHAR);
      act("$N can't stand the likes of $n any longer.",TRUE,tmp_ch,0,this,TO_ROOM);
      break;
  }
  return TRUE;
}

void TMonster::aiTarget(TBeing *vict)
{
  if (!canSee(vict) || !awake() || UtilMobProc(this) ||
      !sameRoom(*vict))
    return;

  if (vict->isPc())
    setTarg(vict);
  else {
    if (vict->master && vict->master->isPc())
      setTarg(vict->master);
    else  // mob acting on its own 
      setTarg(NULL);
  }
  TBeing * targy = targ();
  if (!targy)
    return;

  if (!targy->isPc()) {
    vlogf(LOG_MOB_AI, "Bug in aiTarget()");
    setTarg(NULL);
  }
}

// this functon checks conditions of limbs and disallows some socials
// it should return FALSE if you want the standard text that social will show 
// and TRUE otherwise (which returns out of the doAction loop 
int TBeing::socialLimbBad(TBeing *mob, cmdTypeT cmd)
{
  if (!hasHands() || bothHandsHurt()) {   // polymorphed into something handless? 
    switch (cmd) {
      case CMD_DANCE:
      case CMD_SHAKE:
      case CMD_HUG:
      case CMD_POKE:
      case CMD_FONDLE:
      case CMD_GROPE:
      case CMD_MASSAGE:
        act("Gee, hands would be nice...",TRUE,this,0,mob,TO_CHAR);
        return TRUE;
        break;
      default:
        break;
    }
  }
  if (eitherArmHurt()) {
    switch (cmd) {
      case CMD_DANCE:
      case CMD_SHAKE:
      case CMD_HUG:
      case CMD_POKE:
      case CMD_FONDLE:
      case CMD_GROPE:
      case CMD_MASSAGE:
      case CMD_SQUEEZE:
      case CMD_CUDDLE:
        act("Generally, you need working arms to do that...",TRUE,this,0,mob,TO_CHAR);
        return TRUE;
        break;
      default:
        break;
    }
  } 
  if (riding) {
    switch (cmd) {
      case CMD_HOP:
        act("You bounce up and down on $N, how thrilling.",
              FALSE, this, 0, riding, TO_CHAR);
        act("$n bounces up and down on you, ouch, your back hurts.",
              FALSE, this, 0, riding, TO_VICT);
        act("$n bounces up and down on $N, $e must be excited.",
              FALSE, this, 0, riding, TO_NOTVICT);
        return TRUE;
      default:
        break;
    }
  }
  if (bothLegsHurt()) {
    switch (cmd) {
      case CMD_HOP:
        act("Your busted legs makes that rather impossible I'm afraid.",TRUE,this,0,mob,TO_CHAR);
        return TRUE;
      default:
        break;
    }
  }
  if (eitherLegHurt()) {
    switch (cmd) {
      case CMD_DANCE:
        act("Your busted legs makes that rather impossible I'm afraid.",TRUE,this,0,mob,TO_CHAR);
        return TRUE;
      case CMD_HOP:
        act("Seeing you have a busted leg, that's about all you CAN do.",TRUE,this,0,mob,TO_CHAR);
        return TRUE;
      default:
        break;
    }
  }
  if (!mob->hasHands()) {
    switch (cmd) {
      case -1:
        act("$N doesn't have any hands so you can't do that to $M.",TRUE,this,0,mob,TO_CHAR);
        return TRUE;
        break;
      default:
        break;
    }
  }
  if (!mob->canUseLimb(WEAR_HAND_L) || !mob->canUseLimb(WEAR_HAND_R) || !mob->canUseLimb(WEAR_ARM_L) || !mob->canUseLimb(WEAR_ARM_R)) {
    switch (cmd) {
      case CMD_DANCE:
      case CMD_SHAKE: 
        act("$N has a busted arm, so you can't do that to $M.",TRUE,this,0,mob,TO_CHAR);
        return TRUE;
        break;
      default:
        break;
    }
  }
  if (mob->eitherLegHurt()) {
    switch (cmd) {
      case CMD_DANCE:
        act("$E has a problem with $S legs so you can't do that to them.",TRUE,this,0,mob,TO_CHAR);
        return TRUE;
      default:
        break;
    }
  }
  if (mob->getSex() == SEX_NEUTER) {
    switch (cmd) {
      case CMD_FONDLE:
      case CMD_GROPE:
        act("$N lacks the genetalia for that to be practical.",
            TRUE,this,0,mob,TO_CHAR);
        return TRUE;
      default:
        break;
    }
  } else if (mob->getSex() == SEX_FEMALE) {
    switch (cmd) {
      case CMD_RIP:
        act("$N lacks the genetalia for that to be practical.",
            TRUE,this,0,mob,TO_CHAR);
        return TRUE;
      default:
        break;
    }
  }

  // a large number of functions call doAction without checking min_pos first
  // lets add some (redundant) code to verify proper position.
  if (getPosition() < commandArray[cmd]->minPosition) {
    sendTo("You aren't in the proper position to do that.\n\r");
    return TRUE;
  }
  return FALSE;
}

// called when ch falls off horse 
void TMonster::aiHorse(TBeing *ch)
{
  aiTarget(ch);
  UA(3);
  UM(1);
  US(4);
}


void TMonster::mobAI()
{
  TThing *t=NULL;
  int aggro_state_1 = aggro();
  int aggro_state_2;

  if (in_room == Room::NOWHERE)
    return;

  if (UtilMobProc(this))
    return;

  if (!awake())
    return;

  // if an opinion is very far away from its default-attitude, shift the
  // def-attitude up slightly.  if its different from default, move opinion
  // back toward default BUT give a 1 in 5 chance of moving away from default.
  // This should have the effect of a slow fluxuating decay back towards its
  // default value.  Let's also decay them at varying rates.

  // suspicion is whether the mob has seen "wierd things" or thinks that
  // it is about to be attacked.  look at, and con affect it a lot.
  // It's basically how "wary and alert" it is.  

  if (!::number(0,2)) {
    if (susp() > defsusp()) {
      if (((susp() - 20) >= defsusp()) && !::number(0,4)) {
        setDefSusp(defsusp() + min(1, 100 - defsusp()));
        setSusp(defsusp());
      }
    } else if (susp() < defsusp()) {
      if (((susp() + 20) <= defsusp()) && !::number(0,4)) {
        setDefSusp(defsusp() - min(1, defsusp()));
        setSusp(defsusp());
      }
    } else {
      if (!::number(0,4)) 
        if (::number(0,1)) 
          US(2);
        else
          DS(2);
    } 
  }
  // Greed should be fairly obvious.  it starts at 50%, very high greed implies
  // it wants lots of valuables, while low implies it feels cheritable.  this
  // should only be affected in small increments.

  if (!::number(0,4)) {
    if (greed() > defgreed()) {
      if (((greed() - 20) >= defgreed()) && !::number(0,4)) {
        setDefGreed(defgreed() + min(1, 100 - defgreed()));
        setGreed(defgreed());
      }
    } else if (greed() < defgreed()) {
      if (((greed() + 20) <= defgreed()) && !::number(0,4)) {
        setDefGreed(defgreed() - min(1, defgreed()));
        setGreed(defgreed());
      }
    } else {
      if (!::number(0,4))
        if (::number(0,1))
          UG(2);
        else
          DG(2);
    }
  }
  // Anger is a "how pissed am I?" ::number.  It should go up and down in "big"
  // chunks and basically represents a mobs mood at the moment in question.

  if (!::number(0,1)) {
    if (anger() > defanger()) {
      if (((anger() - 20) >= defanger()) && !::number(0,4)) {
        setDefAnger(defanger() + min(1, 100 - defanger()));
        setAnger(defanger());
      }
    } else if (anger() < defanger()) {
      if (((anger() + 20) <= defanger()) && !::number(0,4)) {
        setDefAnger(defanger() - min(1, defanger()));
        setAnger(defanger());
      }
    } else {
      if (!::number(0,4))
        if (::number(0,1))
          UA(2);
        else
          DA(2);
    }
  }
  // Malice is a mobs disposition.  50% base (higher for aggros) so things
  // shouldn't make it jump up or down too fast.  It basically is whether the
  // mob will act nice or mean.  A "love/hate conflict" would be characterizerd
  // by a low malice(love) and a high anger(the hate).  Also, a real low malice
  // allows a mob to wait longer to go aggro.

  if (!::number(0,4)) {
    if (malice() > defmalice()) {
      if (((malice() - 20) >= defmalice()) && !::number(0,4)) {
        setDefMalice(defmalice() + min(1, 100 - defmalice()));
        setMalice(defmalice());
      }
    } else if (malice() < defmalice()) {
      if (((malice() + 20) <= defmalice()) && !::number(0,4)) {
        setDefMalice(defmalice() - min(1, defmalice()));
        setMalice(defmalice());
      }
    } else {
      if (!::number(0,4))
        if (::number(0,1))
          UM(2);
        else
          DMal(2);
    }
  }
  // turn off targets if set by mistake 
  if (targ()) 
    if (!targ()->isPc()) {
      vlogf(LOG_MOB_AI, format("Ooops, target for %s got set to a mob: %s.") % getName() % targ()->getName());
      setTarg(NULL);
    }
  
  // Let's set target for a couple of things. 
  // Obviously opinion should be of who we are fighting 
  if (fight()) {
    if (fight()->isPc()) 
      aiTarget(fight());
    else if (fight()->master)
      aiTarget(fight()->master); 
    else
      setTarg(NULL);

    // fighting don't make me happy
    if (::number(0,1))
      UA(1);
    if (!::number(0,5))
      UM(1);
    if (!::number(0,3))
      US(1);
  }

  aggro_state_2 = aggro();
  if (inGrimhaven() && !IS_SET(specials.act, ACT_AGGRESSIVE)) {
    // keep newbiedom peaceful and happy
    DA(2);
    DMal(1);
  } 

  if (!aggro_state_1 && aggro_state_2) {
    // flipped down
    act("$n goes mad with rage!", TRUE, this, 0, 0, TO_ROOM, ANSI_RED);
    act("You go mad with rage!", TRUE, this, 0, 0, TO_CHAR, ANSI_RED);

    // force it a little higher so that it is well over limit
    UA(7);
    UM(3);
  } else if (aggro_state_1 && !aggro_state_2) {
    // calmed down
    act("$n seems more in control of $s anger!",
             TRUE, this, 0, 0, TO_ROOM, ANSI_GREEN);
    act("You seem more in control of your anger!",
             TRUE, this, 0, 0, TO_CHAR, ANSI_GREEN);

    // force it a little lower so that it is well under limit
    DA(7);
    DMal(3);
  }

  // OK, if we don't have a target, lets see who's in the room 
  // set it to a random PC in the room that we can see 
  if (!targ()) {
    for(StuffIter it=roomp->stuff.begin();it!=roomp->stuff.end() && (t=*it);++it) {
      if (t->isPc() && canSee(t) && (t != this)) {
        aiTarget(dynamic_cast<TBeing *>(t));
        break;
      }
    }
    if (!targ())
      return;
  }
  // we should probably call some independant-reaction functions here 
  // like have the mob demand money if greedy, or leave if suspicious

  return;
}

// DELETE_VICT delete doer
// DELETE_THIS delete this
int TMonster::aiSocialSwitch (TBeing *doer,TBeing *other, cmdTypeT cmd, aiTarg cond)
{
  int rc = 0;

   // added pc check so that poly and switched into mobs dont ai -Cos
  if (isPc() || desc || isname("[clone]", name))
    return FALSE;

  if (!canSee(doer))
    return FALSE;
  if (fight())
    return FALSE;
  if (!awake())
    return FALSE;
  if (doer == this)
    return FALSE;

  // one of these checkResponses will activate
  switch (cond) {
    case TARGET_NONE:
      rc = checkResponses(doer, NULL, NULL, cmd);
      break;
    case TARGET_SELF:
      rc = checkResponses(doer, doer, NULL, cmd);
      break;
    case TARGET_MOB:
      rc = checkResponses(doer, this, NULL, cmd);
      break;
    case TARGET_OTHER:
      rc = checkResponses(doer, other, NULL, cmd);
      break;
  }
  if (IS_SET_DELETE(rc, DELETE_THIS) || IS_SET_DELETE(rc, DELETE_VICT))
    return rc;
  else if (rc)    // if response triggered, skip ai
    return FALSE;

  rc = 0;

  if ((cond != TARGET_MOB) && ::number(0,2))
    return FALSE;
  if ((cond == TARGET_MOB) && !::number(0,7))
    return FALSE;

  // mobs reacting to mobs is bad idea.
  // also allows you to get mount to tank by ordering puke or whatever
  if (!doer->isPc())
    return FALSE;

  aiTarget(doer);
  if (!awake())
    return FALSE;
  if (!canSee(doer))
    return FALSE;
  if (UtilMobProc(this))
    return FALSE;
  //if (GetMaxLevel() <= 5)    // lessen grimhaven spam
    //return FALSE;    
  
  // have it skip if someone is spamming with same command over and over
  if (cmd == opinion.last_cmd) {
    US(1);
    return FALSE;
  }
  opinion.last_cmd = cmd;

  // bigass switch to hand off to appropriate ai function.
  switch (cmd) {
    case CMD_BOUNCE:
      rc = aiBounce(doer,other,cond);
      break;
    case CMD_SMILE:
      rc = aiSmile(doer,other,cond);
      break;
    case CMD_DANCE:
      rc = aiDance(doer,other,cond);
      break;
    case CMD_CACKLE:
      rc = aiCackle(doer,other,cond);
      break;
    case CMD_LAUGH:
      rc = aiLaugh(doer,other,cond);
      break;
    case CMD_GIGGLE:
      rc = aiGiggle(doer,other,cond);
      break;
    case CMD_SHAKE:
      rc = aiShake(doer,other,cond);
      break;
    case CMD_PUKE:
      rc = aiPuke(doer,other,cond);
      break;
    case CMD_GROWL:
      rc = aiGrowl(doer,other,cond);
      break;
    case CMD_SCREAM:
      rc = aiScream(doer,other,cond);
      break;
    case CMD_COMFORT:
      rc = aiComfort(doer,other,cond);
      break;
    case CMD_NOD:
      rc = aiNod(doer,other,cond);
      break;
    case CMD_SIGH:
      rc = aiSigh(doer,other,cond);
      break;
    case CMD_SULK:
      rc = aiSulk(doer,other,cond);
      break;
    case CMD_HUG:
      rc = aiHug(doer,other,cond);
      break;
    case CMD_SNUGGLE:
      rc = aiSnuggle(doer,other,cond);
      break;
    case CMD_CUDDLE:
      rc = aiCuddle(doer,other,cond);
      break;
    case CMD_NUZZLE:
      rc = aiNuzzle(doer,other,cond);
      break;
    case CMD_CRY:
      rc = aiCry(doer,other,cond);
      break;
    case CMD_POKE:
      rc = aiPoke(doer,other,cond);
      break;
    case CMD_ACCUSE:
      rc = aiAccuse(doer,other,cond);
      break;
    case CMD_GRIN:
      rc = aiGrin(doer,other,cond);
      break;
    case CMD_BOW:
      rc = aiBow(doer,other,cond);
      break;
    case CMD_APPLAUD:
      rc = aiApplaud(doer,other,cond);
      break;
    case CMD_BLUSH:
      rc = aiBlush(doer,other,cond);
      break;
    case CMD_BURP:
      rc = aiBurp(doer,other,cond);
      break;
    case CMD_CHUCKLE:
      rc = aiChuckle(doer,other,cond);
      break;
    case CMD_CLAP:
      rc = aiClap(doer,other,cond);
      break;
    case CMD_COUGH:
      rc = aiCough(doer,other,cond);
      break;
    case CMD_CURTSEY:
      rc = aiCurtsey(doer,other,cond);
      break;
    case CMD_FART:
      rc = aiFart(doer,other,cond);
      break;
    case CMD_FLIP:
      rc = aiFlip(doer,other,cond);
      break;
    case CMD_FONDLE:
      rc = aiFondle(doer,other,cond);
      break;
    case CMD_FROWN:
      rc = aiFrown(doer,other,cond);
      break;
    case CMD_GASP:
      rc = aiGasp(doer,other,cond);
      break;
    case CMD_GLARE:
      rc = aiGlare(doer,other,cond);
      break;
    case CMD_GROAN:
      rc = aiGroan(doer,other,cond);
      break;
    case CMD_GROPE:
      rc = aiGrope(doer,other,cond);
      break;
    case CMD_HICCUP:
      rc = aiHiccup(doer,other,cond);
      break;
    case CMD_LICK:
      rc = aiLick(doer,other,cond);
      break;
    case CMD_LOVE:
      rc = aiLove(doer,other,cond);
      break;
    case CMD_MOAN:
      rc = aiMoan(doer,other,cond);
      break;
    case CMD_NIBBLE:
      rc = aiNibble(doer,other,cond);
      break;
    case CMD_POUT:
      rc = aiPout(doer,other,cond);
      break;
    case CMD_PURR:
      rc = aiPurr(doer,other,cond);
      break;
    case CMD_RUFFLE:
      rc = aiRuffle(doer,other,cond);
      break;
    case CMD_SHIVER:
      rc = aiShiver(doer,other,cond);
      break;
    case CMD_SHRUG:
      rc = aiShrug(doer,other,cond);
      break;
    case CMD_SING:
      rc = aiSing(doer,other,cond);
      break;
    case CMD_SLAP:
      rc = aiSlap(doer,other,cond);
      break;
    case CMD_SMIRK:
      rc = aiSmirk(doer,other,cond);
      break;
    case CMD_SNAP:
      rc = aiSnap(doer,other,cond);
      break;
    case CMD_SNEEZE:
      rc = aiSneeze(doer,other,cond);
      break;
    case CMD_SNICKER:
      rc = aiSnicker(doer,other,cond);
      break;
    case CMD_SNIFF:
      rc = aiSniff(doer,other,cond);
      break;
    case CMD_SNORE:
      rc = aiSnore(doer,other,cond);
      break;
    case CMD_SPIT:
      rc = aiSpit(doer,other,cond);
      break;
    case CMD_SQUEEZE:
      rc = aiSqueeze(doer,other,cond);
      break;
    case CMD_STARE:
      rc = aiStare(doer,other,cond);
      break;
    case CMD_STRUT:
      rc = aiStrut(doer,other,cond);
      break;
    case CMD_THANK:
      rc = aiThank(doer,other,cond);
      break;
    case CMD_TWIDDLE:
      rc = aiTwiddle(doer,other,cond);
      break;
    case CMD_WAVE:
      rc = aiWave(doer,other,cond);
      break;
    case CMD_WHISTLE:
      rc = aiWhistle(doer,other,cond);
      break;
    case CMD_WIGGLE:
      rc = aiWiggle(doer,other,cond);
      break;
    case CMD_WINK:
      rc = aiWink(doer,other,cond);
      break;
    case CMD_YAWN:
      rc = aiYawn(doer,other,cond);
      break;
    case CMD_SNOWBALL:
      rc = aiSnowball(doer,other,cond);
      break;
    case CMD_FRENCH:
      rc = aiFrench(doer,other,cond);
      break;
    case CMD_COMB:
      rc = aiComb(doer,other,cond);
      break;
    case CMD_MASSAGE:
      rc = aiMassage(doer,other,cond);
      break;
    case CMD_TICKLE:
      rc = aiTickle(doer,other,cond);
      break;
    case CMD_PAT:
      rc = aiPat(doer,other,cond);
      break;
    case CMD_CURSE:
      rc = aiCurse(doer,other,cond);
      break;
    case CMD_BEG:
      rc = aiBeg(doer,other,cond);
      break;
    case CMD_BLEED:
      rc = aiBleed(doer,other,cond);
      break;
    case CMD_CRINGE:
      rc = aiCringe(doer,other,cond);
      break;
    case CMD_DAYDREAM:
      rc = aiDaydream(doer,other,cond);
      break;
    case CMD_FUME:
      rc = aiFume(doer,other,cond);
      break;
    case CMD_GROVEL:
      rc = aiGrovel(doer,other,cond);
      break;
    case CMD_HOP:
      rc = aiHop(doer,other,cond);
      break;
    case CMD_NUDGE:
      rc = aiNudge(doer,other,cond);
      break;
    case CMD_PEER:
      rc = aiPeer(doer,other,cond);
      break;
    case CMD_POINT:
      rc = aiPoint(doer,other,cond);
      break;
    case CMD_PONDER:
      rc = aiPonder(doer,other,cond);
      break;
    case CMD_PUNCH:
      rc = aiPunch(doer,other,cond);
      break;
    case CMD_SNARL:
      rc = aiSnarl(doer,other,cond);
      break;
    case CMD_SPANK:
      rc = aiSpank(doer,other,cond);
      break;
    case CMD_STEAM:
      rc = aiSteam(doer,other,cond);
      break;
    case CMD_TACKLE:
      rc = aiTackle(doer,other,cond);
      break;
    case CMD_TAUNT:
      rc = aiTaunt(doer,other,cond);
      break;
    case CMD_WHINE:
      rc = aiWhine(doer,other,cond);
      break;
    case CMD_WORSHIP:
      rc = aiWorship(doer,other,cond);
      break;
    case CMD_YODEL:
      rc = aiYodel(doer,other,cond);
      break;
    case CMD_THINK:
      rc = aiThink(doer,other,cond);
      break;
    case CMD_WHAP:
      rc = aiWhap(doer,other,cond);
      break;
    case CMD_BEAM:
      rc = aiBeam(doer,other,cond);
      break;
    case CMD_CHORTLE:
      rc = aiChortle(doer,other,cond);
      break;
    case CMD_BONK:
      rc = aiBonk(doer,other,cond);
      break;
    case CMD_SCOLD:
      rc = aiScold(doer,other,cond);
      break;
    case CMD_DROOL:
      rc = aiDrool(doer,other,cond);
      break;
    case CMD_RIP:
      rc = aiRip(doer,other,cond);
      break;
    case CMD_STRETCH:
      rc = aiStretch(doer,other,cond);
      break;
    case CMD_PIMP:
      rc = aiPimp(doer,other,cond);
      break;
    case CMD_BELITTLE:
      rc = aiBelittle(doer,other,cond);
      break;
    case CMD_PILEDRIVE:
      rc = aiPiledrive(doer,other,cond);
      break;
    case CMD_TAP:
      rc = aiTap(doer,other,cond);
      break;
    case CMD_FLIPOFF:
      rc = aiFlipoff(doer,other,cond);
      break;
    case CMD_MOON:
      rc = aiMoon(doer,other,cond);
      break;
    case CMD_PINCH:
      rc = aiPinch(doer,other,cond);
      break;
    case CMD_BITE:
      rc = aiBite(doer,other,cond);
      break;
    case CMD_KISS:
      rc = aiKiss(doer,other,cond);
      break;
    case CMD_CHEER:
      rc = aiCheer(doer,other,cond);
      break;
    case CMD_WOO:
      rc = aiWoo(doer,other,cond);
      break;
    case CMD_GRUMBLE:
      rc = aiGrumble(doer,other,cond);
      break;
    case CMD_APOLOGIZE:
      rc = aiApologize(doer,other,cond);
      break;
    case CMD_AGREE:
      rc = aiAgree(doer,other,cond);
      break;
    case CMD_DISAGREE:
      rc = aiDisagree(doer,other,cond);
      break;
    case CMD_SPAM:
      rc = aiSpam(doer,other,cond);
      break;
    case CMD_ARCH:
      rc = aiRaise(doer,other,cond);
      break;
    case CMD_ROLL:
      rc = aiRoll(doer,other,cond);
      break;
    case CMD_BLINK:
      rc = aiBlink(doer,other,cond);
      break;
    case CMD_GREET:
      rc = aiGreet(doer,other,cond);
      break;
    case CMD_TIP:
      rc = aiTip(doer,other,cond);
      break;
    case CMD_BOP:
      rc = aiBop(doer,other,cond);
      break;
    case CMD_JUMP:
      rc = aiJump(doer,other,cond);
      break;
    case CMD_WHIMPER:
      rc = aiWhimper(doer,other,cond);
      break;
    case CMD_SNEER:
      rc = aiSneer(doer,other,cond);
      break;
    case CMD_MOO:
      rc = aiMoo(doer,other,cond);
      break;
    case CMD_BOGGLE:
      rc = aiBoggle(doer,other,cond);
      break;
    case CMD_SNORT:
      rc = aiSnort(doer,other,cond);
      break;
    case CMD_TANGO:
      rc = aiTango(doer,other,cond);
      break;
    case CMD_ROAR:
      rc = aiRoar(doer,other,cond);
      break;
    case CMD_FLEX:
      rc = aiFlex(doer,other,cond);
      break;
    case CMD_TUG:
      rc = aiTug(doer,other,cond);
      break;
    case CMD_CROSS:
      rc = aiCross(doer,other,cond);
      break;
    case CMD_HOWL:
      rc = aiHowl(doer,other,cond);
      break;
    case CMD_GRUNT:
      rc = aiGrunt(doer,other,cond);
      break;
    case CMD_WEDGIE:
      rc = aiWedgie(doer,other,cond);
      break;
    case CMD_SCUFF:
      rc = aiScuff(doer,other,cond);
      break;
    case CMD_NOOGIE:
      rc = aiNoogie(doer,other,cond);
      break;
    case CMD_BRANDISH:
      rc = aiBrandish(doer,other,cond);
      break;
    case CMD_TRIP:
      rc = aiTrip(doer,other,cond);
      break;
    case CMD_DUCK:
      rc = aiDuck(doer,other,cond);
      break;
    case CMD_BECKON:
      rc = aiBeckon(doer,other,cond);
      break;
    case CMD_WINCE:
      rc = aiWince(doer,other,cond);
      break;
    case CMD_FAINT:
      rc = aiFaint(doer,other,cond);
      break;
    case CMD_HUM:
      rc = aiHum(doer,other,cond);
      break;
    case CMD_RAZZ:
      rc = aiRazz(doer,other,cond);
      break;
    case CMD_GAG:
      rc = aiGag(doer,other,cond);
      break;
    case CMD_AVERT:
      rc = aiAvert(doer,other,cond);
      break;
    case CMD_SALUTE:
      rc = aiSalute(doer,other,cond);
      break;
    case CMD_PET:
      rc = aiPet(doer,other,cond);
      break;
    case CMD_GRIMACE:
      rc = aiGrimace(doer,other,cond);
      break;
    case CMD_TOAST:
      rc = aiToast(doer,other,cond);
      break;
    default:
      vlogf(LOG_MOB_AI, format("doAction mob_AI loop called with cmd of %d and type %d") % cmd %cond);
      US(1);
      aiStrangeThings(doer);
      break;
  }
  return rc;
}

void TMonster::aiMobCreation()
{
  setDefSusp(max(1,GetMaxLevel()/5));
  setDefGreed(50);
  setDefAnger(GetMaxLevel()/10);
  setDefMalice(50);

  if (specials.act & ACT_NICE_THIEF) {
    setDefGreed(defgreed() - min(25,defgreed()));
    setDefSusp(defsusp() - min(15,defsusp()));
  }
  if (specials.act & ACT_AGGRESSIVE) {
    setDefAnger(defanger() + min(50,100-defanger()));
    setDefMalice(defmalice() + min(25,100-defmalice()));
  }
  if (specials.act & ACT_SCAVENGER)
    setDefGreed(defgreed() + min(25,100-defgreed()));

  setTarg(NULL);

  setSusp(defsusp());
  setMalice(defmalice());
  setAnger(defanger());
  setGreed(defgreed());

  return;
}

void TMonster::aiMaintainCalm()
{
  if (defanger() - anger() < -10)
    DA(1);
  if (defmalice() - malice() < -5)
    DMal(1);
}
