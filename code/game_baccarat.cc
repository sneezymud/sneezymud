#include "stdsneezy.h"
#include "games.h"

BaccaratGame gBaccarat;

bool BaccaratGame::enter(const TBeing *ch)
{
  if(inuse){
    ch->sendTo("This table is already in use.\n\r");
    return false;
  }

  inuse = true;
  baccarat_shuffle(ch);
  bet = 0;
  name=ch->name;

  return true;
}

void BaccaratGame::baccarat_shuffle(const TBeing *ch)
{
  act("The dealer shuffles the deck.",FALSE, ch, 0, 0, TO_CHAR);
  act("The dealer shuffles the deck.",FALSE, ch, 0, 0, TO_ROOM);

  shuffle();
  deck_inx = 0;
}

bool TBeing::checkBaccarat(bool inGame) const
{
  if (in_room == ROOM_BACCARAT && (inGame || (gBaccarat.index(this) > -1)))
    return true;
  else
    return false;
}

int handValue(int hand[3])
{
  int total=0;

  for(int i=0;i<3;++i){
    if(CARD_NUM(hand[i]) < 11)
      total+=CARD_NUM(hand[i]);
  }

  return total%10;
}

void BaccaratGame::stay(TBeing *ch)
{
  sstring log_msg;

  // dealer plays
  if((bet_type==0 && handValue(dealer)<=5) || 
     (bet_type==1 && handValue(dealer)>5)){
    dealer[2]=deck[deck_inx++];

    log_msg = fmt("The dealer is dealt %s.") %pretty_card_printout(ch, dealer[2]);
    ch->sendTo(COLOR_BASIC, log_msg);
    act(log_msg, TRUE, ch, 0, 0, TO_ROOM);
  }

  // show final hands
  ch->sendTo("\n\rYour final hand:\n\r");
  act("\n\r$n's final hand:", TRUE, ch, 0, 0, TO_ROOM);
  for(int i=0;i<3;++i){
    if(player[i]){
      ch->sendTo(COLOR_BASIC, fmt("%s\n\r") % pretty_card_printout(ch, player[i]));
      log_msg = fmt("%s") % pretty_card_printout(ch, player[i]);
      act(log_msg, TRUE, ch, 0, 0, TO_ROOM);
    }
  }
  
  ch->sendTo("\n\rThe dealer's final hand:\n\r");
  act("\n\rThe dealer's final hand:",TRUE, ch, 0, 0, TO_ROOM);

  for(int i=0;i<3;++i){
    if(dealer[i]){
      ch->sendTo(COLOR_BASIC, fmt("%s\n\r") % pretty_card_printout(ch, dealer[i]));
      log_msg = fmt("%s") % pretty_card_printout(ch, dealer[i]);
      act(log_msg, TRUE, ch, 0, 0, TO_ROOM);
    }
  }

  ch->sendTo("\n\r");
  act("\n\r", TRUE, ch, 0, 0, TO_ROOM);

  // determine win/loss
  float mult=0;

  if(handValue(dealer) < handValue(player)){
    ch->sendTo("Your hand wins.\n\r");
    act("$n's hand wins.", TRUE, ch, 0, 0, TO_ROOM);

    if(bet_type==0){
      ch->sendTo("You win your bet!\n\r");
      act("$n's wins $s bet!", TRUE, ch, 0, 0, TO_ROOM);
      mult=2;
    }
  } else if(handValue(dealer) > handValue(player)){
    ch->sendTo("The dealer's hand wins.\n\r");
    act("The dealer's hand wins.", TRUE, ch, 0, 0, TO_ROOM);

    if(bet_type==1){
      ch->sendTo("You win your bet!\n\r");
      act("$n's wins $s bet!", TRUE, ch, 0, 0, TO_ROOM);
      mult=2;
    }
  } else if(handValue(dealer)==handValue(player)){
    ch->sendTo("It's a tie.\n\r");
    act("It's a tie.", TRUE, ch, 0, 0, TO_ROOM);

    if(bet_type==2){
      ch->sendTo("You win your bet!\n\r");
      act("$n's wins $s bet!", TRUE, ch, 0, 0, TO_ROOM);
      mult=8;
    }
  }
  
  if(mult>0){
    payout(ch, (int)((double)bet * mult));
    observerReaction(ch, GAMBLER_WON);
  } else {
    ch->sendTo("You lose your bet.\n\r");
    act("$n loses $s bet.", TRUE, ch, 0, 0, TO_ROOM);
    observerReaction(ch, GAMBLER_LOST);
  }

  bet = 0;
}

void BaccaratGame::Hit(TBeing *ch)
{
  sstring log_msg;

  if(player[2]){
    ch->sendTo("You've already taken a hit.  You must stay now.\n\r");
    return;
  }

  player[2]=deck[deck_inx++];
  
  log_msg = fmt("You are dealt %s.\n\r") % pretty_card_printout(ch, player[2]);
  ch->sendTo(COLOR_BASIC, log_msg);
    
  log_msg = fmt("$n is dealt %s.") %pretty_card_printout(ch, player[2]);
  act(log_msg, TRUE, ch, 0, 0, TO_ROOM);
}

void BaccaratGame::Bet(TBeing *ch, const sstring &arg)
{
  int inx;
  sstring coin_str, log_msg, buf, bet_str;
  TObj *chip;

  if (ch->checkBaccarat()) {
    inx = index(ch);
    if (inx < 0) {
      ch->sendTo("You are not sitting at the table yet.\n\r");
      return;
    }

    if (bet > 0){
      ch->sendTo("You're already playing a hand.\n\r");
      return;
    }

    coin_str=arg.word(0);
    bet_str=arg.word(1);

    if (coin_str.empty()){
      ch->sendTo("Bet which chip?\n\r");
      return;
    }

    if(!(chip=find_chip(ch, coin_str))){
      ch->sendTo("You don't have that chip!\n\r");
      return;
    }

    if(is_abbrev(bet_str, "player")){
      bet_type=0;
      ch->sendTo("You place your bet on your hand winning.\n\r");
      act("$n places $s bet on your hand winning.",
	  TRUE, ch, 0, 0, TO_ROOM);
    } else if(is_abbrev(bet_str,"dealer")){
      bet_type=1;
      ch->sendTo("You place your bet on the dealer's hand winning.\n\r");
      act("$n places $s bet on the dealer's hand winning.",
	  TRUE, ch, 0, 0, TO_ROOM);
    } else if(is_abbrev(bet_str,"tie")){
      bet_type=2;
      ch->sendTo("You place your bet on the hands coming out in a tie.\n\r");
      act("$n places $s bet on the hands coming out in a tie.",
	  TRUE, ch, 0, 0, TO_ROOM);
    } else {
      ch->sendTo("Bet on player, dealer or tie?\n\r");
      return;
    }

    bet = chip->obj_flags.cost;

    sstring buf;
    buf = fmt("$n bets %s.") % chip->getName();
    act(buf, TRUE, ch, 0, 0, TO_ROOM);
    buf = fmt("You bet %s.") % chip->getName();
    act(buf, TRUE, ch, 0, 0, TO_CHAR);

    (*chip)--;
    delete chip;
    ch->doSave(SILENT_YES);

    if (deck_inx > 10)
      baccarat_shuffle(ch);

    for(int i=0;i<3;++i)
      player[i]=dealer[i]=0;

    log_msg="You are dealt:\n\r";
    ch->sendTo(COLOR_BASIC, log_msg);
    act("$n is dealt:", TRUE, ch, 0, 0, TO_ROOM);

    for(int i=0;i<2;++i){
      player[i] = deck[deck_inx++];
      dealer[i] = deck[deck_inx++];

      log_msg = fmt("%s (down)\n\r") % pretty_card_printout(ch, player[i]);
      ch->sendTo(COLOR_BASIC, log_msg);
      
      log_msg = fmt("%s (down)") %pretty_card_printout(ch, player[i]);
      act(log_msg, TRUE, ch, 0, 0, TO_ROOM);
    }    


    // check for natural hands
    if((handValue(player)==8 && handValue(dealer)!=8 && handValue(dealer)!=9)||
       (handValue(player)==9 && handValue(dealer)!=9)){
      // player wins
      ch->sendTo("\n\rYour final hand:\n\r");
      act("\n\r$n's final hand:", TRUE, ch, 0, 0, TO_ROOM);
      for(int i=0;i<3;++i){
	if(player[i]){
	  ch->sendTo(COLOR_BASIC, fmt("%s\n\r") % pretty_card_printout(ch, player[i]));
	  log_msg = fmt("%s") % pretty_card_printout(ch, player[i]);
	  act(log_msg, TRUE, ch, 0, 0, TO_ROOM);
	}
      }
      
      ch->sendTo("\n\rThe dealer's final hand:\n\r");
      act("\n\rThe dealer's final hand:",TRUE, ch, 0, 0, TO_ROOM);
      
      for(int i=0;i<3;++i){
	if(dealer[i]){
	  ch->sendTo(COLOR_BASIC, fmt("%s\n\r") % pretty_card_printout(ch, dealer[i]));
	  log_msg = fmt("%s") % pretty_card_printout(ch, dealer[i]);
	  act(log_msg, TRUE, ch, 0, 0, TO_ROOM);
	}
      }
      
      ch->sendTo("\n\r");
      act("\n\r", TRUE, ch, 0, 0, TO_ROOM);

      ch->sendTo(fmt("You win with a natural %i!\n\r") % handValue(player));
      log_msg = fmt("$n wins with a natural %i!") % handValue(player);
      act(log_msg, TRUE, ch, 0, 0, TO_ROOM);
      payout(ch, 2 * bet);
      bet=0;
      observerReaction(ch, GAMBLER_WON);
    } else if((handValue(dealer)==8 && handValue(player)!=8 && handValue(player)!=9)||
       (handValue(dealer)==9 && handValue(player)!=9)){
      // dealer wins
      ch->sendTo("\n\rYour final hand:\n\r");
      act("\n\r$n's final hand:", TRUE, ch, 0, 0, TO_ROOM);
      for(int i=0;i<3;++i){
	if(player[i]){
	  ch->sendTo(COLOR_BASIC, fmt("%s\n\r") % pretty_card_printout(ch, player[i]));
	  log_msg = fmt("%s") % pretty_card_printout(ch, player[i]);
	  act(log_msg, TRUE, ch, 0, 0, TO_ROOM);
	}
      }
      
      ch->sendTo("\n\rThe dealer's final hand:\n\r");
      act("\n\rThe dealer's final hand:",TRUE, ch, 0, 0, TO_ROOM);
      
      for(int i=0;i<3;++i){
	if(dealer[i]){
	  ch->sendTo(COLOR_BASIC, fmt("%s\n\r") % pretty_card_printout(ch, dealer[i]));
	  log_msg = fmt("%s") % pretty_card_printout(ch, dealer[i]);
	  act(log_msg, TRUE, ch, 0, 0, TO_ROOM);
	}
      }
      
      ch->sendTo("\n\r");
      act("\n\r", TRUE, ch, 0, 0, TO_ROOM);

      ch->sendTo(fmt("The dealer wins with a natural %i!\n\r") % handValue(dealer));
      log_msg = fmt("The dealer wins with a natural %i!") % handValue(dealer);
      act(log_msg, TRUE, ch, 0, 0, TO_ROOM);
      bet=0;
      observerReaction(ch, GAMBLER_LOST);
    } else if((handValue(player)==8 && handValue(dealer)==8) ||
	      (handValue(player)==9 && handValue(dealer)==9)){
      ch->sendTo("\n\rYour final hand:\n\r");
      act("\n\r$n's final hand:", TRUE, ch, 0, 0, TO_ROOM);
      for(int i=0;i<3;++i){
	if(player[i]){
	  ch->sendTo(COLOR_BASIC, fmt("%s\n\r") % pretty_card_printout(ch, player[i]));
	  log_msg = fmt("%s") % pretty_card_printout(ch, player[i]);
	  act(log_msg, TRUE, ch, 0, 0, TO_ROOM);
	}
      }
      
      ch->sendTo("\n\rThe dealer's final hand:\n\r");
      act("\n\rThe dealer's final hand:",TRUE, ch, 0, 0, TO_ROOM);
      
      for(int i=0;i<3;++i){
	if(dealer[i]){
	  ch->sendTo(COLOR_BASIC, fmt("%s\n\r") % pretty_card_printout(ch, dealer[i]));
	  log_msg = fmt("%s") % pretty_card_printout(ch, dealer[i]);
	  act(log_msg, TRUE, ch, 0, 0, TO_ROOM);
	}
      }
      
      ch->sendTo("\n\r");
      act("\n\r", TRUE, ch, 0, 0, TO_ROOM);

      ch->sendTo("It's a tie, you push.\n\r");
      log_msg="It's a tie, $n pushes.";
      act(log_msg, TRUE, ch, 0, 0, TO_ROOM);      
      payout(ch, bet);
    } else
      observerReaction(ch, GAMBLER_BET);
  }
}


void BaccaratGame::peek(const TBeing *ch)
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
  log_msg="You peek at your hand:\n\r";
  ch->sendTo(COLOR_BASIC, log_msg);

  for(int i=0;i<3;++i){
    if(player[i]){
      log_msg = fmt("%s (down)\n\r") %
	       pretty_card_printout(ch, player[i]);
      ch->sendTo(COLOR_BASIC, log_msg);
    }
  }    

}


int BaccaratGame::exitGame(const TBeing *ch)
{
  int inx;

  if ((inx = index(ch)) < 0) {
    vlogf(LOG_BUG, fmt("%s left a table he was not at!") %  ch->name);
    return FALSE;
  }
  inuse = FALSE;
  name="";
  deck_inx = 0;
  bet = 0;
  for(int i=0;i<3;++i)
    player[i]=0;
  setup_deck();
  ch->sendTo("You leave the baccarat table.\n\r");
  return TRUE;
}


int BaccaratGame::index(const TBeing *ch) const
{
  if(ch->name == name)
    return 0;

  return -1;
}
