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
  hilo_shuffle(ch);
  bet = 0;
  name=ch->name;

  return true;
}

void HiLoGame::hilo_shuffle(const TBeing *ch)
{
  act("The dealer shuffles the deck.",FALSE, ch, 0, 0, TO_CHAR);
  act("The dealer shuffles the deck.",FALSE, ch, 0, 0, TO_ROOM);

  shuffle();
  deck_inx = 0;
}

bool TBeing::checkHiLo(bool inGame = false) const
{
  if (in_room == ROOM_HILO && (inGame || (gHiLo.index(this) > -1)))
    return true;
  else
    return false;
}

void HiLoGame::BetHi(TBeing *ch, int new_card)
{
  sstring buf;

  if(CARD_NUM_ACEHI(new_card) > CARD_NUM_ACEHI(card)){
    win_perc*=2;
    ch->sendTo("You win!  Your winnings are now at %i talens.\n\r",
	       (int)((float)bet * (1.0 + win_perc)));
    ssprintf(buf, "$n wins!  $n's winnings are now at %i talens.",
	       (int)((float)bet * (1.0 + win_perc)));
    act(buf.c_str(), TRUE, ch, 0, 0, TO_ROOM);    
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

void HiLoGame::BetLo(TBeing *ch, int new_card)
{
  sstring buf;

  if(CARD_NUM_ACEHI(new_card) < CARD_NUM_ACEHI(card)){
    win_perc*=2;
    ch->sendTo("You win!  Your winnings are now at %i talens.\n\r",
	       (int)((float)bet * (1.0 + win_perc)));
    ssprintf(buf, "$n wins!  $n's winnings are now at %i talens.",
	       (int)((float)bet * (1.0 + win_perc)));
    act(buf.c_str(), TRUE, ch, 0, 0, TO_ROOM);    
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

  int next_card=deck[deck_inx++];
  sstring buf;
  ch->sendTo(COLOR_BASIC, "The next card was the %s.\n\r", pretty_card_printout(ch, next_card).c_str());
  ssprintf(buf, "The next card was the %s.", pretty_card_printout(ch, next_card).c_str());
  act(buf.c_str(), TRUE, ch, 0, 0, TO_ROOM);

  payout(ch, (int)((double)bet * (1.0 + win_perc)));
  bet = 0;
  card = 0;
  observerReaction(ch, GAMBLER_WON);
}


void HiLoGame::Bet(TBeing *ch, const sstring &arg)
{
  int inx, new_card;
  sstring coin_str;
  sstring log_msg;
  sstring buf;
  TObj *chip;

  if (ch->checkHiLo()) {
    inx = index(ch);
    if (inx < 0) {
      ch->sendTo("You are not sitting at the table yet.\n\r");
      return;
    }
    if (bet > 0) {
      if(arg=="hi" || arg=="lo"){
	if (deck_inx > 10)
	  hilo_shuffle(ch);
	
	new_card=deck[deck_inx++];
	
	ssprintf(buf, "$n bets %s.", arg.c_str());
	act(buf.c_str(), TRUE, ch, 0, 0, TO_ROOM);
	
	ssprintf(log_msg, "You are dealt:\n\r%s\n\r", pretty_card_printout(ch, new_card).c_str());
	ch->sendTo(COLOR_BASIC, log_msg.c_str());
	
	ssprintf(log_msg, "$n is dealt:\n\r%s", pretty_card_printout(ch, new_card).c_str());
	act(log_msg.c_str(), TRUE, ch, 0, 0, TO_ROOM);
	
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
    argument_parser(arg, coin_str);
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
    ssprintf(buf, "$n bets %s.", chip->getName());
    act(buf.c_str(), TRUE, ch, 0, 0, TO_ROOM);
    ssprintf(buf, "You bet %s.", chip->getName());
    act(buf.c_str(), TRUE, ch, 0, 0, TO_CHAR);

    (*chip)--;
    delete chip;

    win_perc=WIN_INIT;
    card=0;
    if (deck_inx > 10)
      hilo_shuffle(ch);

    card = deck[deck_inx++];

    ssprintf(log_msg, "You are dealt:\n\r%s\n\r", pretty_card_printout(ch, card).c_str());
    ch->sendTo(COLOR_BASIC, log_msg.c_str());

    ssprintf(log_msg, "$n is dealt:\n\r%s\n\r", pretty_card_printout(ch, card).c_str());
    act(log_msg.c_str(), TRUE, ch, 0, 0, TO_ROOM);

    observerReaction(ch, GAMBLER_HILO_BET);
  }
}


void HiLoGame::peek(const TBeing *ch) const
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
  ssprintf(log_msg, "You peek at your hand:\n\r%s\n\r", pretty_card_printout(ch, card).c_str());
  ch->sendTo(COLOR_BASIC, log_msg.c_str());
}


int HiLoGame::exitGame(const TBeing *ch)
{
  int inx;

  if ((inx = index(ch)) < 0) {
    forceCrash("%s left a table he was not at!", ch->name);
    return FALSE;
  }
  inuse = FALSE;
  name="";
  deck_inx = 0;
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
