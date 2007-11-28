///////////////////////////////////////////////////////////////////////////
//
//      SneezyMUD++ - All rights reserved, SneezyMUD Coding Team
//      "craps.cc" - All functions and routines related to the craps table 
//      
//      Craps table coded by Russ Russell, January 1993. Last revision
//      December 12, 1997.
//
///////////////////////////////////////////////////////////////////////////

#include "stdsneezy.h"
#include "games.h"

Craps::Craps() :
  m_ch(NULL)
{
}

Craps::Craps(TBeing *ch) :
  m_ch(ch)
{
}

class Ccraps {
  public:
    int pos;
    Ccraps() : pos(0) {}
    ~Ccraps() {}
};

const char DICE_ONE[] =
" #########\n\r\
 #       #\n\r\
 #   *   #\n\r\
 #       #\n\r\
 #########\n\r\n\r";

const char DICE_TWO[] =
" #########\n\r\
 #     * #\n\r\
 #       #\n\r\
 # *     #\n\r\
 #########\n\r\n\r";

const char DICE_THREE[] = 
" #########\n\r\
 #     * #\n\r\
 #   *   #\n\r\
 # *     #\n\r\
 #########\n\r\n\r";

const char DICE_FOUR[] =
" #########\n\r\
 # *   * #\n\r\
 #       #\n\r\
 # *   * #\n\r\
 #########\n\r\n\r";

const char DICE_FIVE[] =
" #########\n\r\
 # *   * #\n\r\
 #   *   #\n\r\
 # *   * #\n\r\
 #########\n\r\n\r";

const char DICE_SIX[] =
" #########\n\r\
 # *   * #\n\r\
 # *   * #\n\r\
 # *   * #\n\r\
 #########\n\r\n\r";

void Craps::loseDice()
{
  TObj *the_dice;
  TMonster *crap_man;

  the_dice = dynamic_cast<TObj *>(m_ch->heldInPrimHand());
  if (!the_dice || the_dice->objVnum() != CRAPS_DICE) {
    the_dice = dynamic_cast<TObj *>(m_ch->heldInSecHand());
    if (!the_dice || the_dice->objVnum() != CRAPS_DICE) {
      vlogf(LOG_BUG, "loseDice called without dice held???");
    }
  }
  

  crap_man = FindMobInRoomWithProcNum(m_ch->in_room, SPEC_CRAPSGUY);

  if (crap_man && the_dice)
    *crap_man += *(m_ch->unequip(the_dice->eq_pos));

  crap_man->doSay("Next roller please?");
}

void Craps::getDice()
{
}

bool TBeing::checkForDiceHeld() const
{
  TThing *t;
  TObj *obj;

  t = heldInPrimHand();
  obj = dynamic_cast<TObj *>(t);
  if (obj) {
    if (obj->objVnum() == CRAPS_DICE)
      return TRUE;
    for (t = obj->getStuff(); t; t = t->nextThing) {
      obj = dynamic_cast<TObj *>(t);
      if (obj && obj->objVnum() == CRAPS_DICE)
        return TRUE;
    }
  }
  t = heldInSecHand();
  obj = dynamic_cast<TObj *>(t);
  if (obj) {
    if (obj->objVnum() == CRAPS_DICE)
      return TRUE;
    for (t = obj->getStuff(); t; t = t->nextThing) {
      obj = dynamic_cast<TObj *>(t);
      if (obj && obj->objVnum() == CRAPS_DICE)
        return TRUE;
    }
  }
  return FALSE;
}

TObj *TBeing::checkForDiceInInv() const
{
  TThing *t, *t2;
  TObj *obj, *obj2;

  for (t = getStuff(); t; t = t->nextThing) {
    obj = dynamic_cast<TObj *>(t);
    if (obj) {
      if (obj->objVnum() == CRAPS_DICE)
        return obj;
      for (t2 = obj->getStuff(); t2; t2 = t2->nextThing) {
        obj2 = dynamic_cast<TObj *>(t2);
        if (obj2 && obj2->objVnum() == CRAPS_DICE)
          return obj2;
      }
    }
  }

  return NULL;
}

int TRoom::checkPointroll()
{
  TThing *c;

  for (c = getStuff(); c; c = c->nextThing) {
    if (c->desc && c->desc->point_roll)
      return TRUE;
  }
  return FALSE;
}

void TBeing::doBet(const char *arg)
{
  Descriptor *d;
  char amount[15], craps[256];
  TObj *chip;

  if (!(d = desc))
    return;

  half_chop(arg, amount, craps);

  if (checkHoldem()) {
    gHoldem.Bet(this, arg);
    return;
  }
  if (checkBlackjack()) {
    gBj.Bet(this, arg);
    return;
  }
  if (checkDrawPoker()) {
    gDrawPoker.bet(this, arg);
    return;
  }
  if (checkHiLo()){
    gHiLo.Bet(this, arg);
    return;
  }
  if (checkPoker()){
    gPoker.Bet(this, arg);
    return;
  }
  if (checkBaccarat()){
    gBaccarat.Bet(this, arg);
    return;
  }

  
  if (!FindMobInRoomWithProcNum(in_room, SPEC_CRAPSGUY)) {
    sendTo("The bet command is used for casino games.\n\r");
    return;
  }

  if (!*amount) {
    sendTo("\n\rSyntax :  bet <option> <amount> \n\r\n\r");
    sendTo("Options :\n\r\n\r");
    sendTo("1)  The craps table :\n\r");
    sendTo("    a) come : Bet an amount on the come out roll of the roller. \n\r");
    sendTo("    b) crap : Bet on the no pass.\n\r");
    sendTo("    c) OneRolls :\n\r");
    sendTo("       Type bet one to see the various one roll bets available to you.\n\r");
    sendTo("2)  The slot machines :\n\r");
    sendTo("      The correct syntax for the slots machines is play slots <option>\n\r");
    sendTo("      Type play slots to see the different options.\n\r");
    return;
  } else {
    if (is_abbrev(amount, "onerolls")) {
      if (!*craps) {
        sendTo("\n\rOne Roll bets : \n\r");
        sendTo("Two   : Bet on a one roll snake eyes (2). Pays 30 to 1.\n\r");
        sendTo("Three : Bet on a one roll acey deucy (3). Pays 15 to 1.\n\r");
        sendTo("Eleven: Bet on a one roll eleven (11). Pays 15 to 1.\n\r");
        sendTo("Twelve: Bet on a one roll box-cars (12). Pay 30 to 1.\n\r");
        sendTo("Craps : Bet on a one roll craps roll(2,3, or 12). Pays 7 to 1.\n\r");
        sendTo("Seven : Bet on a one roll seven roll. Pays 4 to 1.\n\r");
        sendTo("Horn  : Bet on a one roll horn bet. Type help horn for help on the horn bet.\n\r");
        sendTo("Field : Bet on a one roll field bet. Type help field for help on this bet.\n\r");
	return;
      }
    }
    if (*craps) {
      if(!(chip=find_chip(this, craps))){
	sendTo("You don't have that chip!\n\r");
	return;
      }

      if (is_abbrev(amount, "crap")) {
	if (!d->bet.crap) {
	  d->bet_opt.crapsOptions += CRAP_OUT;
	  sendTo(COLOR_OBJECTS, fmt("You place %s for your bet on the crap roll.\n\r") % 
		 chip->getName());

	  d->bet.crap = chip->obj_flags.cost;

	  (*chip)--;
	  delete chip;
	} else {
	  sendTo("You've already betted on craps.\n\r");
	}
      } else if (is_abbrev(amount, "come")) {
	if (!checkForDiceHeld() && !checkForDiceInInv()) {
	  TObj *the_dice;
	  TMonster *crap_man;
	  char buf[80];

	  if (!(crap_man = FindMobInRoomWithProcNum(in_room, SPEC_CRAPSGUY))) {
	  } else {
	    // no dice in world
	    if (!obj_index[real_object(CRAPS_DICE)].getNumber()) {
	      the_dice = read_object(CRAPS_DICE, VIRTUAL);
	      *crap_man += *the_dice;
	    }
	    if (!(the_dice = crap_man->checkForDiceInInv())) {
	      // player doesn't have dice, craps doesn't either
	      // someone else is the roller
	    } else {
	      sprintf(buf, "Fine %s, here are the dice!", getName());
	      crap_man->doSay(buf);
	      crap_man->doSay("Hold the dice, and throw them when I say it's ok!");
	      --(*the_dice);
	      *this += *the_dice;
  
	      if (!crap_man->act_ptr)
		crap_man->act_ptr = new Ccraps();
  
	      Ccraps *cr = (Ccraps *) crap_man->act_ptr;
	      cr->pos = 4;
	      m_craps = new Craps(this);
	    }
	  }
	}
	if (!d->bet.come) {
	  d->bet_opt.crapsOptions += COME_OUT;
	  sendTo(COLOR_OBJECTS, fmt("You just placed %s down on the come out roll.\n\r") %
		 chip->getName());

	  d->bet.come = chip->obj_flags.cost;

	  (*chip)--;
          delete chip;
	} else {
	  sendTo("You've already betted on the come out roll.\n\r");
	}
      } else if (is_abbrev(amount, "three")) {
	if (!d->bet.three) {
	  d->bet_opt.oneRoll += THREE3;
	  sendTo(COLOR_OBJECTS, fmt("You just placed %s down on a oneroll three (3) roll.\n\r") %
		 chip->getName());

	  d->bet.three = chip->obj_flags.cost;

	  (*chip)--;
          delete chip;
	} else {
	  sendTo("You've already betted on a oneroll three (3) roll.\n\r");
	}
      } else if (is_abbrev(amount, "two")) {
	if (!d->bet.two) {
	  d->bet_opt.oneRoll += TWO2;
	  sendTo(COLOR_OBJECTS, fmt("You just placed %s down on a oneroll two (2) roll.\n\r") %
		 chip->getName());

	  d->bet.two = chip->obj_flags.cost;

	  (*chip)--;
          delete chip;
	} else {
	  sendTo("You've already betted on a oneroll two (2) roll.\n\r");
	}
      } else if (is_abbrev(amount, "eleven")) {
	if (!d->bet.eleven) {
	  d->bet_opt.oneRoll += ELEVEN;
	  sendTo(COLOR_OBJECTS, fmt("You just placed %s down on a oneroll eleven (11) roll.\n\r") %
		 chip->getName());

	  d->bet.eleven = chip->obj_flags.cost;

	  (*chip)--;
          delete chip;
	} else {
	  sendTo("You've already betted on a oneroll eleven (11) roll.\n\r");
	}
      } else if (is_abbrev(amount, "twelve")) {
	if (!d->bet.twelve) {
	  d->bet_opt.oneRoll += TWELVE;
	  sendTo(COLOR_OBJECTS, fmt("You just placed %s down on a oneroll twelve (12) roll.\n\r") %
		 chip->getName());

	  d->bet.twelve = chip->obj_flags.cost;

	  (*chip)--;
          delete chip;
	} else {
	  sendTo("You've already betted on a oneroll twelve (12) roll.\n\r");
	}
      } else if (is_abbrev(amount, "horn")) {
	if (!d->bet.horn_bet) {
	  d->bet_opt.oneRoll += HORN_BET;
	  sendTo(COLOR_OBJECTS, fmt("You just placed %s down on a oneroll horn roll.\n\r") %
		 chip->getName());

	  d->bet.horn_bet = chip->obj_flags.cost;

	  (*chip)--;
          delete chip;
	} else {
	  sendTo("You've already betted on a oneroll horn roll.\n\r");
	}
      } else if (is_abbrev(amount, "field")) {
	if (!d->bet.field_bet) {
	  d->bet_opt.oneRoll += FIELD_BET;
	  sendTo(COLOR_OBJECTS, fmt("You just placed %s down on a oneroll field roll.\n\r") %
		 chip->getName());

	  d->bet.field_bet = chip->obj_flags.cost;

	  (*chip)--;
          delete chip;
	} else {
	  sendTo("You've already betted on a oneroll field roll.\n\r");
	}
      } else if (is_abbrev(amount, "seven")) {
	if (!d->bet.seven) {
	  d->bet_opt.oneRoll += SEVEN;
	  sendTo(COLOR_OBJECTS, fmt("You just placed %s down on a oneroll seven roll.\n\r") %
		 chip->getName());

	  d->bet.seven = chip->obj_flags.cost;

	  (*chip)--;
          delete chip;
	} else {
	  sendTo("You've already betted on a oneroll seven roll.\n\r");
	}
      } else {
	sendTo("Wrong option.\n\r");
	return;
      }
      observerReaction(this, GAMBLER_BET);
    }
  }
}

void Craps::clearBets()
{
}

int can_bet_craps(TBeing *ch)
{
  TMonster *crap_man;

  if (!(crap_man = FindMobInRoomWithProcNum(ch->in_room, SPEC_CRAPSGUY)))
    return FALSE;

  if (!crap_man->act_ptr)
    crap_man->act_ptr = new Ccraps();

  Ccraps *cr = (Ccraps *) crap_man->act_ptr;
  if (!cr->pos)
    return TRUE;
  else
    return FALSE;
}

int Craps::checkCraps(int diceroll)
{
  TThing *t;
  Descriptor *d;
  int newRoll = FALSE;

  if (!m_ch->roomp) {
    vlogf(LOG_BUG, "checkCraps() called with NULL roomp!");
    return FALSE;
  }

  if (m_ch->desc->point_roll)
    return FALSE;

  for (t = m_ch->roomp->getStuff(); t; t = t->nextThing) {
    if (!(d = t->desc))
      continue;

    if ((diceroll != 2) && (diceroll != 3) && (diceroll != 12)){
      t->sendTo(fmt("No Craps....You Lose your bet (%d)!\n\r") % d->bet.crap);
      REMOVE_BIT(d->bet_opt.crapsOptions, CRAP_OUT);
      d->bet.crap=0;
      continue;
    }

    if (IS_SET(d->bet_opt.crapsOptions, COME_OUT)) {
      t->sendTo(fmt("Craps....You Lose your bet (%d)!\n\r") % d->bet.come);
      REMOVE_BIT(d->bet_opt.crapsOptions, COME_OUT);
      d->bet.come = 0;
      if (t == m_ch) {
	loseDice();
        newRoll = TRUE;
      }
      observerReaction(dynamic_cast<TBeing *>(t), GAMBLER_LOST);
    }
    if (IS_SET(d->bet_opt.crapsOptions, CRAP_OUT)) {
      t->sendTo(fmt("Craps....You Win your bet (%d)!\n\r") % d->bet.crap);
      REMOVE_BIT(d->bet_opt.crapsOptions, CRAP_OUT);
      payout(dynamic_cast<TBeing *>(t), 2 * d->bet.crap);
      observerReaction(dynamic_cast<TBeing *>(t), GAMBLER_WON);

      d->bet.crap = 0;
      if (t == m_ch) {
        if (!newRoll)
          loseDice();
        newRoll = TRUE;
      }
    }
  }
  if (newRoll) 
    return TRUE;
  return FALSE;
}


int Craps::checkSeven(int diceroll)
{
  TThing *t;
  Descriptor *d;
  int pointRoll = FALSE, newRoll = FALSE;

  if (diceroll != 7)
    return FALSE;

  if (m_ch->desc) {
    pointRoll = m_ch->desc->point_roll;
    m_ch->desc->point_roll = 0;
  } else {
    vlogf(LOG_BUG, "Somehow m_ch without a desc got to checkSeven");
  }

  for (t = m_ch->roomp->getStuff(); t; t = t->nextThing) {
    TBeing *tbt = dynamic_cast<TBeing *>(t);
    if (!tbt)
      continue;
    if (!(d = tbt->desc))
      continue;

    if (!pointRoll) {
      if (IS_SET(d->bet_opt.crapsOptions, COME_OUT)) {
	tbt->sendTo(fmt("Seven! You win your come out bet (%d)!\n\r") % d->bet.come);
	payout(tbt, 2 * d->bet.come);
	observerReaction(dynamic_cast<TBeing *>(t), GAMBLER_WON);
	REMOVE_BIT(d->bet_opt.crapsOptions, COME_OUT);
	d->bet.come = 0;
      }
      if (IS_SET(d->bet_opt.crapsOptions, CRAP_OUT)) {
	tbt->sendTo(fmt("Seven! You lose your crap out bet (%d)!\n\r") % d->bet.crap);
	REMOVE_BIT(d->bet_opt.crapsOptions, CRAP_OUT);
	d->bet.crap = 0;
	observerReaction(dynamic_cast<TBeing *>(t), GAMBLER_LOST);
      
#if 0
// unless we want them to start with a crapout and get the dice
	if (tbt == m_ch) {
	  loseDice();
          newRoll = TRUE;
        }
#endif
      }
    } else {
      if (IS_SET(d->bet_opt.crapsOptions, COME_OUT)) {
	tbt->sendTo(fmt("Seven! You lose your come out bet (%d)!\n\r") % d->bet.come);
	REMOVE_BIT(d->bet_opt.crapsOptions, COME_OUT);
	observerReaction(dynamic_cast<TBeing *>(t), GAMBLER_LOST);
	d->bet.come = 0;
	if (tbt == m_ch) {
	  loseDice();
          newRoll = TRUE;
	}
      }
      if (IS_SET(d->bet_opt.crapsOptions, CRAP_OUT)) {
	tbt->sendTo(fmt("Seven! You win your crap out bet (%d)!\n\r") % d->bet.crap);
	payout(tbt, 2 * d->bet.crap);
	observerReaction(dynamic_cast<TBeing *>(t), GAMBLER_WON);

	REMOVE_BIT(d->bet_opt.crapsOptions, CRAP_OUT);
	d->bet.crap = 0;
        if (tbt == m_ch) {
          if (!newRoll)
            loseDice();
          newRoll = TRUE;
        }
      }
    }
  }

  if (newRoll)
    return TRUE;
  return FALSE;
}

int Craps::checkEleven(int diceroll)
{
  TThing *t;
  Descriptor *d;
//  int newRoll = FALSE;

  if ((diceroll != 11) || m_ch->desc->point_roll)
    return FALSE;

  for (t = m_ch->roomp->getStuff(); t; t = t->nextThing) {
    TBeing *tbt = dynamic_cast<TBeing *>(t);
    if (!tbt)
      continue;
    if (!(d = tbt->desc))
      continue;

    if (IS_SET(d->bet_opt.crapsOptions, COME_OUT)) {
      tbt->sendTo("Seven come ELEVEN! You hit your come out bet!\n\r");
      payout(tbt, 2 * d->bet.come);
      observerReaction(dynamic_cast<TBeing *>(t), GAMBLER_WON);

      REMOVE_BIT(d->bet_opt.crapsOptions, COME_OUT);
      d->bet.come = 0;
    }
    if (IS_SET(d->bet_opt.crapsOptions, CRAP_OUT)) {
      tbt->sendTo("Seven come ELEVEN! You hit the come out bet. You lose!\n\r");
      REMOVE_BIT(d->bet_opt.crapsOptions, CRAP_OUT);
      observerReaction(dynamic_cast<TBeing *>(t), GAMBLER_LOST);
      d->bet.crap = 0;
    }
  }
  return FALSE;
}

void Craps::setPoint(int diceroll)
{
  char buf[256];

  if (!m_ch->desc)
    return;

  if ((diceroll == 2) || (diceroll == 3) || (diceroll == 7) ||
      (diceroll == 11) || (diceroll == 12))
    return;

  m_ch->desc->point_roll = diceroll;

  sprintf(buf, "%d is the point. The point is %d.\n\r", diceroll, diceroll);
  sendToRoom(buf, m_ch->in_room);
}

void Craps::checkOnerollSeven(int diceroll, TBeing *ch)
{
  Descriptor *d;

  if (!(d = ch->desc)) {
    vlogf(LOG_BUG, "No better desc in checkOnerollSeven()");
    return;
  }
  if (diceroll == 7) {
    ch->sendTo(fmt("You win your oneroll seven bet (%d)!\n\r") % (3 * d->bet.seven));
    payout(ch, 4 * d->bet.seven);
    observerReaction(ch, GAMBLER_WON);
  } else {
    ch->sendTo(fmt("You lose your oneroll seven bet (%d).\n\r") % d->bet.seven);
    observerReaction(ch, GAMBLER_LOST);  
  }

  REMOVE_BIT(d->bet_opt.oneRoll, SEVEN);
  d->bet.seven = 0;
}

void Craps::checkHorn(int diceroll, TBeing *ch)
{
  Descriptor *d;

  if (!(d = ch->desc)) {
    vlogf(LOG_BUG, "No better desc in checkHorn()");
    return;
  }
  if ((diceroll > 3) || (diceroll < 11)){
    ch->sendTo(fmt("You lose your horn bet (%d).\n\r") % d->bet.horn_bet);
    observerReaction(ch, GAMBLER_LOST);
  } else {
    if ((diceroll == 2) || (diceroll == 12)) {
      ch->sendTo(fmt("The roll is a [%d]. You win your horn bet (%d)!\n\r") % diceroll % (6 * d->bet.horn_bet));
      payout(ch, 7 * d->bet.horn_bet);
      observerReaction(ch, GAMBLER_WON);
    } else {
      ch->sendTo(fmt("The roll is a [%d]. You win your horn bet (%d)!\n\r") % diceroll % (((int) (3.5 * d->bet.horn_bet)) - d->bet.horn_bet));
      payout(ch, (int)(3.5 * d->bet.horn_bet));
      observerReaction(ch, GAMBLER_WON);
    }
  }
  REMOVE_BIT(d->bet_opt.oneRoll, HORN_BET);
  d->bet.horn_bet = 0;
}

void Craps::checkField(int diceroll, TBeing *ch)
{
  TMonster *crap_man;
  sstring buf;

  Descriptor *d;

  if (!(d = ch->desc)) {
    vlogf(LOG_BUG, "No better desc in checkField()");
    return;
  }
  crap_man = FindMobInRoomWithProcNum(ch->in_room, SPEC_CRAPSGUY);

  if ((diceroll >= 5) && (diceroll <= 8)) {
    if (crap_man) {
      crap_man->doTell(ch->getName(), fmt("The roll is %d. You lose your bet on the field (%d).") % diceroll % d->bet.field_bet);
      observerReaction(ch, GAMBLER_LOST);
    }
  } else {
    if ((diceroll < 12) && (diceroll > 2)) {
      buf=fmt("The roll is %d. You win your bet on the field (%d)!") % diceroll % d->bet.field_bet;
      payout(ch, 2 * d->bet.field_bet);
      observerReaction(ch, GAMBLER_WON);
    } else {
      buf=fmt("%s The roll is %d. You get your field bet back (%d)!") % diceroll % d->bet.field_bet;
      payout(ch, d->bet.field_bet);
      observerReaction(ch, GAMBLER_WON);
    }
    if (crap_man) {
      crap_man->doTell(ch->getName(), buf);
    }

  }
  d->bet.field_bet = 0;
  REMOVE_BIT(d->bet_opt.oneRoll, FIELD_BET);
}


void Craps::checkHardFour(int)
{
}

void Craps::checkHardSix(int)
{
}

void Craps::checkHardEight(int)
{
}

void Craps::checkHardTen(int)
{
}

void Craps::checkHardrolls(int)
{
}


void Craps::checkTwo(int diceroll, TBeing *ch)
{
  Descriptor *d;

  if (!(d = ch->desc)) {
    vlogf(LOG_BUG, "No better desc in check_two()");
    return;
  }
  if (diceroll == 2) {
    ch->sendTo(fmt("Two hit!  Nice bet! You Win your bet (%d)!\n\r") % (30 *
d->bet.two));
    payout(ch, 31 * d->bet.two);
    observerReaction(ch, GAMBLER_WON);
    REMOVE_BIT(d->bet_opt.oneRoll, TWO2);
    d->bet.two = 0;
  } else {
    ch->sendTo(fmt("No two....You lose your two bet (%d)!\n\r") % d->bet.two);
    REMOVE_BIT(d->bet_opt.oneRoll, TWO2);
    d->bet.two = 0;
    observerReaction(ch, GAMBLER_LOST);
  }
}

void Craps::checkThree(int diceroll, TBeing *ch)
{
  Descriptor *d;

  if (!(d = ch->desc)) {
    vlogf(LOG_BUG, "No better desc in check_three()");
    return;
  }
  if (diceroll == 3) {
    ch->sendTo(fmt("Three hit! Nice bet! You win your bet (%d)\n\r") % (15 *
d->bet.three));
    payout(ch, 16 * d->bet.three);
    observerReaction(ch, GAMBLER_WON);
    REMOVE_BIT(d->bet_opt.oneRoll, THREE3);
    d->bet.three = 0;
  } else {
    ch->sendTo(fmt("No three....You lose your three bet (%d)!\n\r") % d->bet.three);
    REMOVE_BIT(d->bet_opt.oneRoll, THREE3);
    d->bet.three = 0;
    observerReaction(ch, GAMBLER_LOST);
  }
}

void Craps::checkOnerollEleven(int diceroll, TBeing *ch)
{
  Descriptor *d;

  if (!(d = ch->desc)) {
    vlogf(LOG_BUG, "No better desc in check_oneroll_eleven()");
    return;
  }
  if (diceroll == 11) {
    ch->sendTo(fmt("Eleven hit! Nice bet! You win your bet (%d)!\n\r") % (15 *
d->bet.eleven));
    payout(ch, 16 * d->bet.eleven);
    observerReaction(ch, GAMBLER_WON);
    REMOVE_BIT(d->bet_opt.oneRoll, ELEVEN);
    d->bet.eleven = 0;
  } else {
    ch->sendTo(fmt("No eleven....You lose your eleven bet (%d)!\n\r") % d->bet.eleven);
    REMOVE_BIT(d->bet_opt.oneRoll, ELEVEN);
    d->bet.eleven = 0;
    observerReaction(ch, GAMBLER_LOST);
  }
}

void Craps::checkTwelve(int diceroll, TBeing *ch)
{
  Descriptor *d;

  if (!(d = ch->desc)) {
    vlogf(LOG_BUG, "No better desc in check_twelve()");
    return;
  }
  if (diceroll == 12) {
    ch->sendTo(fmt("Twelve hit! Nice bet You win your twelve bet (%d)!\n\r") % (30 * d->bet.twelve));
    payout(ch, 31 * d->bet.twelve);
    observerReaction(ch, GAMBLER_WON);
  } else{
    ch->sendTo(fmt("No twelve....You lose your twelve bet (%d)!\n\r") % d->bet.twelve);
    observerReaction(ch, GAMBLER_LOST);
  }

  REMOVE_BIT(d->bet_opt.oneRoll, TWELVE);
  d->bet.twelve = 0;
}

void Craps::checkOnerollCraps(int diceroll, TBeing *ch)
{
  Descriptor *d;

  if (!(d = ch->desc)) {
    vlogf(LOG_BUG, "No better desc in check_oneroll_craps()");
    return;
  }
  if (diceroll == 3) {
    ch->sendTo(fmt("Three hit! Your bet on craps hit! You win (%d)!\n\r") % (6 * d->bet.one_craps));
    REMOVE_BIT(d->bet_opt.oneRoll, CRAPS);
    payout(ch, 7 * d->bet.one_craps);
    observerReaction(ch, GAMBLER_WON);
  } else if (diceroll == 2) {
    ch->sendTo(fmt("Two hit! Your bet on craps hit! You win (%d)!\n\r") % (6 * d->bet.one_craps));
    REMOVE_BIT(d->bet_opt.oneRoll, CRAPS);
    payout(ch, 7 * d->bet.one_craps);
    observerReaction(ch, GAMBLER_WON);
  } else if (diceroll == 12) {
    ch->sendTo(fmt("Twelve hit. Your bet on craps hit! You win (%d)!\n\r") % (6 * d->bet.one_craps));
    REMOVE_BIT(d->bet_opt.oneRoll, CRAPS);
    payout(ch, 7 * d->bet.one_craps);
    observerReaction(ch, GAMBLER_WON);
  } else {
    ch->sendTo(fmt("No craps....You lose your bet on craps (%d)!\n\r") % d->bet.one_craps);
    REMOVE_BIT(d->bet_opt.oneRoll, CRAPS);
    observerReaction(ch, GAMBLER_LOST);
  }
  d->bet.one_craps = 0;
}

void Craps::checkOnerolls(int diceroll)
{
  TThing *t;

  // m_ch is the roller

  for (t = m_ch->roomp->getStuff(); t; t = t->nextThing) {
    Descriptor *d = t->desc;
    if (!d)
      continue;
    TBeing *ch = dynamic_cast<TBeing *>(t);
    if (!ch)
      continue;

    // check people in room that may have bets

    if (IS_SET(d->bet_opt.oneRoll, TWO2))
      checkTwo(diceroll, ch);
    if (IS_SET(d->bet_opt.oneRoll, THREE3))
      checkThree(diceroll, ch);
    if (IS_SET(d->bet_opt.oneRoll, ELEVEN))
      checkOnerollEleven(diceroll, ch);
    if (IS_SET(d->bet_opt.oneRoll, TWELVE))
      checkTwelve(diceroll, ch);
    if (IS_SET(d->bet_opt.oneRoll, CRAPS))
      checkOnerollCraps(diceroll, ch);
    if (IS_SET(d->bet_opt.oneRoll, SEVEN))
      checkOnerollSeven(diceroll, ch);
    if (IS_SET(d->bet_opt.oneRoll, FIELD_BET))
      checkField(diceroll, ch);
    if (IS_SET(d->bet_opt.oneRoll, HORN_BET))
      checkHorn(diceroll, ch);
  }
}

void WinLoseCraps(TBeing *ch, int diceroll)
{
  TThing *t;
  Descriptor *d;

  for (t = ch->roomp->getStuff(); t; t = t->nextThing) {
    TBeing *tbt = dynamic_cast<TBeing *>(t);
    if (!tbt)
      continue;
    if (!(d = tbt->desc))
      continue;

    if (IS_SET(d->bet_opt.crapsOptions, COME_OUT)) {
      tbt->sendTo(fmt("The point [%d] was hit. You win your bet (%d)!\n\r") % diceroll % d->bet.come);
      payout(tbt, 2 * d->bet.come);
      observerReaction(ch, GAMBLER_WON);
      REMOVE_BIT(d->bet_opt.crapsOptions, COME_OUT);
      d->bet.come = 0;
    }
    if (IS_SET(d->bet_opt.crapsOptions, CRAP_OUT)) {
      tbt->sendTo(fmt("The point [%d] was hit. You lose your bet (%d).\n\r") % diceroll % d->bet.crap);
      observerReaction(ch, GAMBLER_LOST);
      REMOVE_BIT(d->bet_opt.crapsOptions, CRAP_OUT);
      d->bet.crap = 0;
      if (tbt == ch)
	ch->m_craps->loseDice();
    }
  }
  ch->desc->point_roll = 0;
}

int Craps::rollDice()
{
  int die_one, die_two, dice_roll;
  sstring buf, buf2;
  TMonster *table_man;

  if (!m_ch || !m_ch->desc)
    return TRUE;

  if (!m_ch->checkForDiceHeld()) {
    m_ch->sendTo("You need to hold the dice.\n\r");
    return FALSE;
  }

  if ((table_man = FindMobInRoomWithProcNum(m_ch->in_room, SPEC_CRAPSGUY))) {
    if (!can_bet_craps(m_ch)) {
      table_man->doTell(m_ch->getName(), "You can't roll until I say so!");
      return FALSE;
    }
    if (!m_ch->desc->bet.come) {
      table_man->doTell(m_ch->getName(), "Sorry to keep the table, you need to place a come bet.");
      return FALSE;
    }
  }
  die_one = dice(1, 6);
  die_two = dice(1, 6);
  dice_roll = (die_one + die_two);

  if (die_one == 1)
    sendToRoom(COLOR_NONE, DICE_ONE, m_ch->in_room);
  else if (die_one == 2)
    sendToRoom(COLOR_NONE, DICE_TWO, m_ch->in_room);
  else if (die_one == 3)
    sendToRoom(COLOR_NONE, DICE_THREE, m_ch->in_room);
  else if (die_one == 4)
    sendToRoom(COLOR_NONE, DICE_FOUR, m_ch->in_room);
  else if (die_one == 5)
    sendToRoom(COLOR_NONE, DICE_FIVE, m_ch->in_room);
  else if (die_one == 6)
    sendToRoom(COLOR_NONE, DICE_SIX, m_ch->in_room);

  if (die_two == 1)
    sendToRoom(DICE_ONE, m_ch->in_room);
  else if (die_two == 2)
    sendToRoom(DICE_TWO, m_ch->in_room);
  else if (die_two == 3)
    sendToRoom(DICE_THREE, m_ch->in_room);
  else if (die_two == 4)
    sendToRoom(DICE_FOUR, m_ch->in_room);
  else if (die_two == 5)
    sendToRoom(DICE_FIVE, m_ch->in_room);
  else if (die_two == 6)
    sendToRoom(DICE_SIX, m_ch->in_room);

  if (m_ch->desc->point_roll) {
    buf=fmt("You rolled a %d and a %d. Total = %d    Point = %d\n\r") %
      die_one % die_two % dice_roll % m_ch->desc->point_roll;
    buf2=fmt("%s rolled a %d and a %d. Total = %d    Point = %d\n\r") %
      m_ch->getName() % die_one % die_two % dice_roll % m_ch->desc->point_roll;
  } else {
    buf=fmt("You rolled a %d and a %d. Total = %d\n\r") %
      die_one % die_two % dice_roll;
    buf2=fmt("%s rolled a %d and a %d. Total = %d\n\r") %
      m_ch->getName() % die_one % die_two % dice_roll;
  }
  act(buf,TRUE, m_ch, NULL, NULL, TO_CHAR);
  act(buf2,TRUE, m_ch, NULL, NULL, TO_ROOM);
//  sendToRoom(buf, m_ch->in_room);

  if (checkCraps(dice_roll)) {
    return TRUE;
  } 
  if (checkSeven(dice_roll)) {
    return TRUE;
  }
  if (checkEleven(dice_roll)) {
    return TRUE;
  }
  checkOnerolls(dice_roll);

  if (!m_ch->desc->point_roll) {
    setPoint(dice_roll);
  } else if (m_ch->desc->point_roll == dice_roll) {
    WinLoseCraps(m_ch, dice_roll);
  }
  return FALSE;
}

int craps_table_man(TBeing *ch, cmdTypeT cmd, const char *arg, TMonster *myself, TObj *)
{
  char buf[256], amount[256], options[256];
  char dice_buf[256];

  if (cmd == CMD_GENERIC_DESTROYED) {
    delete static_cast<Ccraps *>(myself->act_ptr);
    myself->act_ptr = NULL;
    return FALSE;
  }

  if (cmd != CMD_GENERIC_PULSE) {
    if ((cmd == CMD_GIVE) ||
        (cmd == CMD_JUNK) ||
        (cmd == CMD_DONATE) ||
        (cmd == CMD_DROP)) {
      if (!*arg)
        return FALSE;
      one_argument(arg, dice_buf, cElements(dice_buf));
      if (is_abbrev(dice_buf, "dice")) {
        ch->sendTo("Doing that to the dice is not permitted!\n\r")
;
        return TRUE;
      }
      return FALSE;
    } else if (cmd == CMD_BET) {
      if (!*arg)
	return FALSE;
      half_chop(arg, options, amount);
      if (ch->roomp->checkPointroll()) {
	if (is_abbrev(options, "come")) {
	  sprintf(buf, "Sorry %s, no bets can be placed on the come", ch->getName());
	  myself->doSay(buf);
	  myself->doSay("or no-pass after a pointroll has been called.");
	  return TRUE;
	} else
	  return FALSE;
      } else
	return FALSE;
    } else if (cmd == CMD_THROW || cmd == CMD_ROLL) {
      if (!*arg)
	return FALSE;
      one_argument(arg, dice_buf, cElements(dice_buf));
      if (is_abbrev(dice_buf, "dice")) {
        if (ch->m_craps) {
          if (ch->m_craps->rollDice()) {
            delete ch->m_craps;
            ch->m_craps = NULL;
          }
	  return TRUE;
        }
	ch->sendTo("You need to be shooting in the game to throw the dice!\n\r");
        return TRUE;
      }
      return FALSE;
    } else if ((cmd == CMD_NORTH) || 
        (cmd == CMD_SOUTH) ||
        (cmd == CMD_EAST) ||
        (cmd == CMD_WEST) ||
        (cmd == CMD_UP) ||
        (cmd == CMD_DOWN) ||
        (cmd == CMD_NE) ||
        (cmd == CMD_NW) ||
        (cmd == CMD_SE) ||
        (cmd == CMD_FLEE) ||
        (cmd == CMD_SW)) {
      if (ch->checkForDiceHeld()) {
	ch->sendTo("You can't leave the table with the dice!\n\r");
	return TRUE;
      }
      if (ch->checkForDiceInInv()) {
	ch->sendTo("You can't leave the table with the dice!\n\r");
	return TRUE;
      }
      Descriptor *d;
      if ((d = ch->desc)) {
        d->point_roll = 0;
        d->bet_opt.roulOptions = 0;
        d->bet_opt.crapsOptions = 0;
        d->bet_opt.oneRoll = 0;
        d->bet.come = 0;
        d->bet.crap = 0;
        d->bet.eleven = 0;
        d->bet.horn_bet = 0;
        d->bet.slot = 0;
        d->bet.twelve = 0;
        d->bet.two = 0;
        d->bet.three = 0;
        d->bet.field_bet = 0;
        d->bet.hard_four = 0;
        d->bet.hard_eight = 0;
        d->bet.hard_six = 0;
        d->bet.hard_ten = 0;
        d->bet.seven = 0;
        d->bet.one_craps = 0;
      }
      return FALSE;
    }

    return FALSE;
  } else if (myself->checkForDiceInInv()) {
    if (!number(0, 8)) {
      myself->doSay("Who wants to roll the dice next?");
      return TRUE;
    }
  } else {
    if (!myself->act_ptr) {
      myself->act_ptr = new Ccraps();
    }
    Ccraps *cr = (Ccraps *) myself->act_ptr;

    switch (cr->pos) {
      case 4:
	myself->doSay("Place all bets now!");
	cr->pos--;
	return TRUE;
      case 3:
	cr->pos--;
	return TRUE;
      case 2:
	myself->doSay("Last call for bets!");
	cr->pos--;
	return TRUE;
      case 1:
	myself->doSay("Ok roller, roll at your will");
	cr->pos--;
	return TRUE;
      default:
	return FALSE;
    }
  }
  return FALSE;
}
