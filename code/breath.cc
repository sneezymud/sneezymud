///////////////////////////////////////////////////////////////////////////
//
//      SneezyMUD - All rights reserved, SneezyMUD Coding Team
//      "breath.cc" - Things to do with dragon breath
//
///////////////////////////////////////////////////////////////////////////

#include "stdsneezy.h"
#include "disease.h"

static int dragonBreathDam(byte level, int lag)
{
  // This is pretty arbitrary, but keep an eye toward matching what
  // skillDam is going to do as this is effectively just a very special attack
  // let damage be 0.5 * lev * rnds of lag

  int avg = (int) (0.5 * level * lag);

  // slight randomization
  int random = (int) (0.20 * avg);
  int rndamt = ::number(-random, random);

  avg += rndamt;

  return avg;
}

// returns DELETE_VICT for victim
static int spell_frost_breath(byte level, TBeing *ch, TBeing *victim, int lag)
{
  int rc;
  int dam = dragonBreathDam(level, lag);

  act("$N is bathed in an icy jet of frost breath from $n.",TRUE,ch,0,victim,TO_NOTVICT, ANSI_WHITE_BOLD);
  act("You deep-freeze $N.",TRUE,ch,0,victim,TO_CHAR,ANSI_WHITE_BOLD);
  act("$n's breath is FREEZING!!!",TRUE,ch,0,victim,TO_VICT,ANSI_WHITE_BOLD);

  dam=victim->shieldAbsorbDamage(dam);
  
  if (victim->isLucky(levelLuckModifier(ch->GetMaxLevel()))) {
    act("You dodge out of the way, thankfully, avoiding most of the damage.",TRUE,ch, 0, victim, TO_VICT, ANSI_WHITE_BOLD);
    dam >>= 1;
  }
  ch->reconcileHurt(victim,0.5);
  rc = victim->frostEngulfed();
  if (IS_SET_DELETE(rc, DELETE_THIS))
    return DELETE_VICT;

  if (ch->reconcileDamage(victim, dam, SPELL_FROST_BREATH) == -1)
    return DELETE_VICT;

  return TRUE;
}

// returns DELETE_VICT
static int spell_fire_breath(byte level, TBeing *ch, TBeing *victim, int lag)
{
  int rc;
  int dam = dragonBreathDam(level, lag);

  act("$n blows an immense breath of fire towards $N!", FALSE, ch, NULL, victim, TO_NOTVICT);
  act("You blow an immense breath of fire towards $N!", FALSE, ch, NULL, victim, TO_CHAR);
  act("$n blows an immense breath of fire towards you! <R>FIRE!!!<z>\a", FALSE, ch, NULL, victim, TO_VICT);
  if(!(dam=victim->shieldAbsorbDamage(dam))){
    act("You dodge out of the way, thankfully, avoiding all of the damage.",TRUE,ch, 0, victim, TO_VICT);
    return 1;
  }

#if 0
  if (victim->isLucky(levelLuckModifier(ch->GetMaxLevel()))) {
    act("You dodge out of the way, thankfully, avoiding most of the damage.",TRUE,ch, 0, victim, TO_VICT);
    dam >>= 1;
  }
#endif

  ch->reconcileHurt(victim, 0.1);
  rc = victim->flameEngulfed();
  if (IS_SET_DELETE(rc, DELETE_THIS))
    return DELETE_VICT;

  if (ch->reconcileDamage(victim, dam, SPELL_FIRE_BREATH) == -1)
    return DELETE_VICT;

  return 1;
}

// DELETE_VICT retursn
static int spell_acid_breath(byte level, TBeing *ch, TBeing *victim, int lag)
{
  int rc;
  int dam = dragonBreathDam(level, lag);


  act("$N is coated in noxious acidic fumes from $n's breath.",TRUE,ch,0,victim,TO_NOTVICT);
  act("You do a litmus test on $N.",TRUE,ch,0,victim,TO_CHAR);
  act("Noxious acidic fumes from $n's breath surround you!!!",TRUE,ch,0,victim,TO_VICT);

  dam=victim->shieldAbsorbDamage(dam);

  if (victim->isLucky(levelLuckModifier(ch->GetMaxLevel()))) {
    act("You dodge out of the way, thankfully, avoiding most of the damage.",TRUE,ch, 0, victim, TO_VICT);
    dam >>= 1;
  }
  ch->reconcileHurt(victim, 0.1);
  rc = victim->acidEngulfed();
  if (IS_SET_DELETE(rc, DELETE_THIS))
    return DELETE_VICT;

  if (ch->reconcileDamage(victim, dam, SPELL_ACID_BREATH) == -1)
    return DELETE_VICT;

  return 1;
}

// returns DELETE_VICT
static int spell_chlorine_breath(byte level, TBeing *ch, TBeing *victim, int lag)
{
  int rc;
  int dam = dragonBreathDam(level, lag);

  act("$N is immersed in a cloud of chlorine gas from $n's breath.",TRUE,ch,0,victim,TO_NOTVICT);
  act("You burp a smelly one at $N.",TRUE,ch,0,victim,TO_CHAR);
  act("$n's breath is TOXIC!!!",TRUE,ch,0,victim,TO_VICT);
  dam=victim->shieldAbsorbDamage(dam);

  if (victim->isLucky(levelLuckModifier(ch->GetMaxLevel()))) {
    act("You dodge out of the way, thankfully, avoiding most of the gas.",TRUE,ch, 0, victim, TO_VICT);
    dam >>= 1;
  }
  ch->reconcileHurt(victim, 0.1);
  rc = victim->chlorineEngulfed();
  if (IS_SET_DELETE(rc, DELETE_THIS))
    return DELETE_VICT;

  if (ch->reconcileDamage(victim, dam, SPELL_CHLORINE_BREATH) == -1)
    return DELETE_VICT;

  return 1;
}

static int spell_lightning_breath(byte level, TBeing *ch, TBeing *victim, int lag)
{
  int dam = dragonBreathDam(level, lag);
  int rc;

  act("$N is struck by $n's electric breath.",TRUE,ch,0,victim,TO_NOTVICT);
  act("You electrify $N.",TRUE,ch,0,victim,TO_CHAR);
  act("$n's breath is SHOCKING!!!",TRUE,ch,0,victim,TO_VICT);

  dam=victim->shieldAbsorbDamage(dam);
  
  if (victim->isLucky(levelLuckModifier(ch->GetMaxLevel()))) {
    act("You dodge out of the way, thankfully, avoiding most of the damage.",TRUE,ch, 0, victim, TO_VICT);
    dam >>= 1;
  }
  ch->reconcileHurt(victim, 0.1);
  rc = victim->lightningEngulfed();
  if (IS_SET_DELETE(rc, DELETE_THIS))
    return DELETE_VICT;

  if (ch->reconcileDamage(victim, dam, SPELL_LIGHTNING_BREATH) == -1)
    return DELETE_VICT;

  return 1;
}

/*
static int spell_cloud_breath(byte level, TBeing *ch, TBeing *vict)
{
  int dam = dice(damno, damsize) + level / 4;
  int rc;

  act("$n creates a vortex of cloud stones which rush towards you.",
      TRUE, ch, 0, 0, TO_ROOM);
  act("$N is struck by $n's cloud stone storm.", TRUE, ch, 0, vict, TO_NOTVICT);
  act("You pelt $N with a storm of cloud stones.", TRUE, ch, 0, vict, TO_CHAR);
  act("$n pelts you with a storm of cloud stones.", TRUE, ch, 0, vict, TO_VICT);
  act("The vortex of cloud stones quickly evaporate.",
      TRUE, ch, 0, 0, TO_ROOM);

  dam=victim->shieldAbsorbDamage(dam);

  if (vict->isLucky(levelLuckModifier(ch->GetMaxLevel()))) {
    act("You dodge out of the way, thankfully, avoiding most of the damage.",
        TRUE, ch, 0, vict, TO_VICT);
    dam >>= 1;
  }
  ch->reconcileHurt(vict, 0.1);
  rc = victim->cloudStormEngulfed();

  if (IS_SET_DELETE(rc, DELETE_THIS))
    return DELETE_VICT;

  if (ch->reconcileDamage(vict, dam, SPELL_CLOUD_BREATH) == -1)
    return DELETE_VICT;

  return TRUE;
}
*/

// returns DELETE_VICT
static int spell_dust_breath(byte level, TBeing *ch, TBeing *victim, int lag)
{
  int dam = dragonBreathDam(level, lag);

  act("$N is pelted by a cloud of dust from $n's breath.",
      TRUE, ch, NULL, victim, TO_NOTVICT);
  act("You unleash a cloud of dust at $N.",
      TRUE, ch, NULL, victim, TO_CHAR);
  act("You almost feel like a pincushion as $N's breath showers you in dust.",
      TRUE, ch, NULL, victim, TO_VICT);

  dam=victim->shieldAbsorbDamage(dam);

  if (victim->isLucky(levelLuckModifier(ch->GetMaxLevel()))) {
    act("You dodge out of the way, thankfully, avoiding most of the dust.",
        TRUE, ch, NULL, victim, TO_VICT);
    dam >>= 1;
  }

  ch->reconcileHurt(victim, 0.1);

  if (ch->reconcileDamage(victim, dam, SPELL_DUST_BREATH) == -1)
    return DELETE_VICT;

  return 1;
}

typedef struct {
  int vnum;
  spellNumT dam_type;
  int lag;
} BREATHSTRUCT;

static BREATHSTRUCT dragons[] =
{
// best to have avg dam = 0.2*lev*lag on avg
  {2107, SPELL_FROST_BREATH, 5},
  {3416, SPELL_ACID_BREATH, 4},
  {4796, SPELL_CHLORINE_BREATH, 5},
  {4822, SPELL_LIGHTNING_BREATH, 2},
  {4858, SPELL_LIGHTNING_BREATH, 6},
  {6843, SPELL_FIRE_BREATH, 5},
  {8962, SPELL_FIRE_BREATH, 5},
  {10395, SPELL_FIRE_BREATH, 3},
  {10601, SPELL_LIGHTNING_BREATH, 6},
  {11805, SPELL_FIRE_BREATH, 3},
  {12401, SPELL_FROST_BREATH, 7},
  {12403, SPELL_FIRE_BREATH, 7},
  {12404, SPELL_ACID_BREATH, 7},
  {12405, SPELL_FROST_BREATH, 7},
  {14360, SPELL_FROST_BREATH, 5},
  {14361, SPELL_FIRE_BREATH, 3},
  {20400, SPELL_FROST_BREATH, 2},
  {20875, SPELL_DUST_BREATH, 5},
  {22517, SPELL_FROST_BREATH, 5},
  {23633, SPELL_LIGHTNING_BREATH, 5},
  {27905, SPELL_LIGHTNING_BREATH, 2},
  {-1, TYPE_UNDEFINED, -1}     // End of Struct, do not remove
};

int DragonBreath(TBeing *, cmdTypeT cmd, const char *, TMonster *myself, TObj *)
{
  TBeing *tmp = NULL;
  int i;
  int rc = 0;

  if (!myself || (cmd != CMD_MOB_COMBAT))
    return FALSE;
  if (!myself->fight() || !myself->fight()->sameRoom(*myself))
    return FALSE;
  if (!myself->awake())
    return FALSE;
  if (myself->getWait() > 0)
    return FALSE;
  
  for (i = 0; dragons[i].vnum != -1 && dragons[i].vnum != myself->mobVnum();i++);

  if (dragons[i].vnum == -1) {
    // in general, this is bad, but dumn builders often "test"
    if (myself->number == -1)
      vlogf(LOG_LOW, fmt("Dragon (%s:%d) trying to breathe in room %d and not hard coded.") % 
            myself->getName() % myself->mobVnum() % myself->inRoom());
    else
      vlogf(LOG_BUG, fmt("Dragon has no defined breath. (%d)") %  myself->mobVnum());
    return FALSE;
  }
  if (myself->hasDisease(DISEASE_DROWNING) ||
      myself->hasDisease(DISEASE_GARROTTE) ||
      myself->hasDisease(DISEASE_SUFFOCATE)) {
    myself->sendTo("ACK!!  Your present situation prevents you from breathing.\n\r");
    act("$n rears back...", 1, myself, 0, 0, TO_ROOM);
    act("Thank the deities $e is unable to breathe.", 1, myself, 0, 0, TO_ROOM);
    return FALSE;
  }
  act("$n rears back and inhales...", 1, myself, 0, 0, TO_ROOM);

  myself->addToWait(combatRound(1) * dragons[i].lag);

  // have all the mobs in the room try to run for it!
  TThing *t1, *t2;
      
  for (t1 = myself->roomp->getStuff(); t1; t1 = t2) {
    TMonster *tm;
    t2 = t1->nextThing;
    if ((tm = dynamic_cast<TMonster *>(t1))) {
      if (tm != myself && tm->canSee(myself)) {
        if (::number(0, 9)) {
          // vlogf(LOG_BUG, fmt("Calling doFlee for dragon breath on %s.") %  tm->getName());
          tm->doFlee("");
        }
      }
    }
  }

  for (t1 = myself->roomp->getStuff(); t1; t1 = t2) {
    t2 = t1->nextThing;
    tmp = dynamic_cast<TBeing *>(t1);
    if (!tmp)
      continue;
    if (tmp == myself)
      continue;
    switch (dragons[i].dam_type) {
      case SPELL_FROST_BREATH:
	rc = spell_frost_breath(myself->GetMaxLevel(),myself,tmp, dragons[i].lag);
	break;
      case SPELL_FIRE_BREATH:
	rc = spell_fire_breath(myself->GetMaxLevel(),myself,tmp, dragons[i].lag);
	break;
      case SPELL_ACID_BREATH:
	rc = spell_acid_breath(myself->GetMaxLevel(),myself,tmp, dragons[i].lag);
	break;
      case SPELL_LIGHTNING_BREATH:
	rc = spell_lightning_breath(myself->GetMaxLevel(),myself,tmp, dragons[i].lag);
	break;
      case SPELL_CHLORINE_BREATH:
	rc = spell_chlorine_breath(myself->GetMaxLevel(),myself,tmp, dragons[i].lag);
	break;
      case SPELL_DUST_BREATH:
	rc = spell_dust_breath(myself->GetMaxLevel(), myself, tmp, dragons[i].lag);
	break;
      default:
	vlogf(LOG_BUG, fmt("Bad breath for %s, buy it some Binaca") % myself->getName());
	break;
    }
    if (IS_SET_DELETE(rc, DELETE_VICT)) {
      delete tmp;
      tmp = NULL;
    }
  }

  switch (dragons[i].dam_type) {
    case SPELL_FROST_BREATH:
      myself->freezeRoom();
      break;
    case SPELL_FIRE_BREATH:
      myself->flameRoom();
      break;
    case SPELL_ACID_BREATH:
      myself->acidRoom();
      break;
    case SPELL_LIGHTNING_BREATH:
      break;
    case SPELL_CHLORINE_BREATH:
      myself->chlorineRoom();
      break;
    case SPELL_DUST_BREATH:
      break;
    default:
      break;
  }
  return TRUE;
}

void TBeing::doBreath(const char *argument)
{
  char buf[256];
  TBeing *vict;
  int breath;
  int rc;

  if (hasDisease(DISEASE_DROWNING) ||
      hasDisease(DISEASE_GARROTTE) ||
      hasDisease(DISEASE_SUFFOCATE)) {
    sendTo("ACK!!  Your present situation prevents you from breathing.\n\r");
    return;
  }
  if (!(isImmortal()) || !isPc()) {
    sendTo("You inhale and exhale deeply.  Feels good!\n\r");
    sendTo("Fortunately, your brain handles this for you automatically.\n\r");
    return;
  }

  if (!hasWizPower(POWER_BREATHE)) {
    sendTo("You lack the power to breathe dragonbreath.\n\r");
    return;
  }

  argument = one_argument(argument, buf);

  if (!*buf) {
    sendTo("Syntax: breathe <acid | fire | frost | lightning | chlorine> <victim>\n\r");
    return;
  } else if (is_abbrev(buf, "acid")) {
    breath = SPELL_ACID_BREATH;
  } else if (is_abbrev(buf, "fire")) {
    breath = SPELL_FIRE_BREATH;
  } else if (is_abbrev(buf, "frost")) {
    breath = SPELL_FROST_BREATH;
  } else if (is_abbrev(buf, "lightning")) {
    breath = SPELL_LIGHTNING_BREATH; 
  } else if (is_abbrev(buf, "chlorine")) {
    breath = SPELL_CHLORINE_BREATH;
  } else if (is_abbrev(buf, "dust")) {
    breath = SPELL_DUST_BREATH;
  } else {
    sendTo("Syntax: breathe <acid | fire | frost | lightning | chlorine> <victim>\n\r");
    return;
  }
  argument = one_argument(argument, buf);
  if (!*buf) {
    sendTo("Breathe on whom?\n\r");
    return;
  } else if ((vict = get_char_room_vis(this, buf)) != 0) {
    if (vict == this) {
      sendTo("You sure must be bored to do something that dumb.\n\r");
      return;
    }
    act("You inhale deeply and turn to face $N...",TRUE,this,0,vict,TO_CHAR);
    act("$n inhales deeply and turns in your direction...",TRUE,this,0,vict,TO_VICT);
    act("$n inhales deeply and turns to face $N...",TRUE,this,0,vict,TO_NOTVICT);

    act("You exhale forcefully...",TRUE,this,0,vict,TO_CHAR);
    act("$e exhales forcefully...",TRUE,this,0,vict,TO_ROOM);

    switch (breath) {
      case SPELL_ACID_BREATH:
        rc = spell_acid_breath(GetMaxLevel(),this,vict,1);
        if (IS_SET_ONLY(rc, DELETE_VICT)) {
          delete vict;
          vict = NULL;
        }
        acidRoom();
        break;
      case SPELL_FIRE_BREATH:
        rc = spell_fire_breath(GetMaxLevel(),this,vict,1);
        if (IS_SET_ONLY(rc, DELETE_VICT)) {
          delete vict;
          vict = NULL;
        }
        flameRoom();
        break;
      case SPELL_FROST_BREATH:
        rc = spell_frost_breath(GetMaxLevel(),this,vict,1);
        if (IS_SET_ONLY(rc, DELETE_VICT)) {
          delete vict;
          vict = NULL;
        }
        freezeRoom();
        break;
      case SPELL_LIGHTNING_BREATH:
        rc = spell_lightning_breath(GetMaxLevel(),this,vict,1);
        if (IS_SET_ONLY(rc, DELETE_VICT)) {
          delete vict;
          vict = NULL;
        }
        break;
      case SPELL_CHLORINE_BREATH:
        rc = spell_chlorine_breath(GetMaxLevel(),this,vict,1);
        if (IS_SET_ONLY(rc, DELETE_VICT)) {
          delete vict;
          vict = NULL;
        }
        chlorineRoom();
        break;
      case SPELL_DUST_BREATH:
        rc = spell_dust_breath(GetMaxLevel(), this, vict, 1);

        if (IS_SET_ONLY(rc, DELETE_VICT)) {
          delete vict;
          vict = NULL;
        }

        break;
      default:
        act("Clean, pure air comes forth.",TRUE,this,0,0,TO_ROOM);
        sendTo("oops, buggy code.\n\r");
    }
    return;
  } else
    sendTo("I don't see anyone here like that.\n\r");

  return;
}
