#include "stdsneezy.h"
#include "games.h"

HiLoGame gHiLo;

const float WIN_INIT=0.05;

bool HiLoGame::enter(const TBeing *ch)
{
  if(inuse){
    ch->sendTo("This table is already in use.\n\r");
    return false;
  }

  inuse = true;
  bet = 0;
  name=ch->name;

  return true;
}

bool TBeing::checkHiLo(bool inGame) const
{
  if (in_room == ROOM_HILO && (inGame || (gHiLo.index(this) > -1)))
    return true;
  else
    return false;
}

void HiLoGame::BetHi(TBeing *ch, const Card *new_card)
{
  sstring buf;

  if(new_card->getValAceHi() > card->getValAceHi()){
    win_perc*=2;
    ch->sendTo(fmt("You win!  Your winnings are now at %i talens.\n\r") %
	       (int)((float)bet * (1.0 + win_perc)));
    buf = fmt("$n wins!  $n's winnings are now at %i talens.") %
	       (int)((float)bet * (1.0 + win_perc));
    act(buf, TRUE, ch, 0, 0, TO_ROOM);    
    observerReaction(ch, GAMBLER_HILO_BET);

    if(win_perc > 10){
      ch->sendTo("You've reach the win limit.\n\r");
      stay(ch);
    }

  } else {
    ch->sendTo("You lose!\n\r");
    act("$n loses!", TRUE, ch, 0, 0, TO_ROOM);
    observerReaction(ch, GAMBLER_LOST);
    bet = 0;
    card = 0;
  }
}

void HiLoGame::BetLo(TBeing *ch, const Card *new_card)
{
  sstring buf;

  if(new_card->getValAceHi() < card->getValAceHi()){
    win_perc*=2;
    ch->sendTo(fmt("You win!  Your winnings are now at %i talens.\n\r") %
	       (int)((float)bet * (1.0 + win_perc)));
    buf = fmt("$n wins!  $n's winnings are now at %i talens.") %
	       (int)((float)bet * (1.0 + win_perc));
    act(buf, TRUE, ch, 0, 0, TO_ROOM);    
    observerReaction(ch, GAMBLER_HILO_BET);

    if(win_perc > 10){
      ch->sendTo("You've reach the win limit.\n\r");
      stay(ch);
    }

  } else {
    ch->sendTo("You lose!\n\r");
    act("$n loses!", TRUE, ch, 0, 0, TO_ROOM);
    observerReaction(ch, GAMBLER_LOST);
    bet = 0;
    card = 0;
  }
}

void HiLoGame::stay(TBeing *ch)
{
  if(win_perc==WIN_INIT){
    ch->sendTo("You just started, you can't quit now!\n\r");
    return;
  }

  ch->sendTo("You give up and cash out your winnings.\n\r");
  act("$n gives up and cashes out $s winnings.",
      TRUE, ch, 0, 0, TO_ROOM);

  const Card *next_card=deck.draw();
  //  vlogf(LOG_PEEL, fmt("drew %s") %  next_card->getName());

  sstring buf;
  ch->sendTo(COLOR_BASIC,fmt("The next card was the %s.\n\r") %next_card->getName());

  buf = fmt("The next card was the %s.") % next_card->getName();
  act(buf, TRUE, ch, 0, 0, TO_ROOM);

  payout(ch, (int)((double)bet * (1.0 + win_perc)));
  bet = 0;
  card = 0;
  observerReaction(ch, GAMBLER_WON);
}


void HiLoGame::Bet(TBeing *ch, const sstring &arg)
{
  const Card *new_card;
  sstring coin_str;
  sstring log_msg;
  sstring buf;
  TObj *chip;

  if (ch->checkHiLo()) {
    if (index(ch) < 0) {
      ch->sendTo("You are not sitting at the table yet.\n\r");
      return;
    }
    if (bet > 0) {
      if(arg=="hi" || arg=="lo"){
	new_card=deck.draw();
	//	vlogf(LOG_PEEL, fmt("drew %s") %  new_card->getName());
	
	buf = fmt("$n bets %s.") % arg;
	act(buf, TRUE, ch, 0, 0, TO_ROOM);
	
	ch->sendTo(COLOR_BASIC,fmt("You are dealt:\n\r%s\n\r") %new_card->getName());
	
	log_msg = fmt("$n is dealt:\n\r%s") % new_card->getName();
	act(log_msg, TRUE, ch, 0, 0, TO_ROOM);
	
	if(arg=="hi"){
	  BetHi(ch, new_card);
	} else if(arg=="lo"){
	  BetLo(ch, new_card);
	}
	card=new_card;
	return;
      } else {
	ch->sendTo("You can't change your bet now.\n\r");
	ch->sendTo("You must either bet 'hi' or 'lo' for the next card.\n\r");
	return;
      }
    }
    coin_str=arg.word(0);

    if (coin_str.empty()){
      ch->sendTo("Bet which chip?\n\r");
      return;
    }

    if(!(chip=find_chip(ch, coin_str))){
      ch->sendTo("You don't have that chip!\n\r");
      return;
    }

    bet = chip->obj_flags.cost;
    ch->doSave(SILENT_YES);

    sstring buf;
    buf = fmt("$n bets %s.") % chip->getName();
    act(buf, TRUE, ch, 0, 0, TO_ROOM);
    buf = fmt("You bet %s.") % chip->getName();
    act(buf, TRUE, ch, 0, 0, TO_CHAR);

    (*chip)--;
    delete chip;

    win_perc=WIN_INIT;
    card=NULL;

    deck.shuffle();
    act("The dealer shuffles the deck.",FALSE, ch, 0, 0, TO_CHAR);
    act("The dealer shuffles the deck.",FALSE, ch, 0, 0, TO_ROOM);

    card=deck.draw();
    //    vlogf(LOG_PEEL, fmt("drew %s") %  card->getName());

    ch->sendTo(COLOR_BASIC, fmt("You are dealt:\n\r%s\n\r") % card->getName());

    log_msg = fmt("$n is dealt:\n\r%s\n\r") % card->getName();
    act(log_msg, TRUE, ch, 0, 0, TO_ROOM);

    observerReaction(ch, GAMBLER_HILO_BET);
  }
}


void HiLoGame::peek(const TBeing *ch)
{
  sstring log_msg;

  if (index(ch) < 0) {
    ch->sendTo("You are not sitting at the table yet.\n\r");
    return;
  }
  if (!bet) {
    ch->sendTo("You are not playing a game.\n\r");
    return;
  }
  log_msg = fmt("You peek at your hand:\n\r%s\n\r") % card->getName();
  ch->sendTo(COLOR_BASIC, log_msg);
}


int HiLoGame::exitGame(const TBeing *ch)
{
  int inx;

  if ((inx = index(ch)) < 0) {
    vlogf(LOG_BUG, fmt("%s left a table he was not at!") %  ch->name);
    return FALSE;
  }
  inuse = FALSE;
  name="";
  bet = 0;
  card = 0;
  win_perc=0;
  setup_deck();
  ch->sendTo("You leave the hi-lo table.\n\r");
  return TRUE;
}


int HiLoGame::index(const TBeing *ch) const
{
  if(ch->name == name)
    return 0;

  return -1;
}
