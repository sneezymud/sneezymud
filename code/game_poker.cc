#include "stdsneezy.h"
#include "games.h"

PokerGame gPoker;


int cardCompare(const void *card1, const void *card2)
{
  int temp = CARD_NUM_ACEHI(*(const int *)card1) - CARD_NUM_ACEHI(*(const int *)card2);
  if (temp > 0)
    return 1;
  else if (temp < 0)
    return -1;
  else
    return 0;
}

bool PokerGame::enter(const TBeing *ch)
{
  if(inuse){
    ch->sendTo("This table is already in use.\n\r");
    return false;
  }

  inuse = true;
  poker_shuffle(ch);
  bet = 0;
  name=ch->name;

  return true;
}

void PokerGame::poker_shuffle(const TBeing *ch)
{
  act("The dealer shuffles the deck.",FALSE, ch, 0, 0, TO_CHAR);
  act("The dealer shuffles the deck.",FALSE, ch, 0, 0, TO_ROOM);

  shuffle();
  deck_inx = 0;
}

bool TBeing::checkPoker(bool inGame) const
{
  if (in_room == ROOM_POKER && (inGame || (gPoker.index(this) > -1)))
    return true;
  else
    return false;
}

void PokerGame::stay(TBeing *ch)
{
  sstring log_msg;

  log_msg="You are dealt:\n\r";
  ch->sendTo(COLOR_BASIC, log_msg);
  act("$n is dealt", TRUE, ch, 0, 0, TO_ROOM);

  for(int i=0;i<5;++i){
    if(!card[i])
      card[i] = deck[deck_inx++];

      
    log_msg = fmt("%i) %s\n\r") % (i+1) % pretty_card_printout(ch, card[i]);
    ch->sendTo(COLOR_BASIC, log_msg);
    
    log_msg = fmt("%s") %pretty_card_printout(ch, card[i]);
    act(log_msg, TRUE, ch, 0, 0, TO_ROOM);
  }


  // determine win/loss
  float mult=0;

  if(isRoyalFlush()){
    mult=800;
    ch->sendTo("You win with a royal flush!\n\r");
    act("$n wins with a royal flush!", TRUE, ch, 0, 0, TO_ROOM);
  } else if(isStraightFlush()){
    mult=50;
    ch->sendTo("You win with a straight flush!\n\r");
    act("$n wins with a straight flush!", TRUE, ch, 0, 0, TO_ROOM);
  } else if(isFourOfAKind()){
    mult=25;
    ch->sendTo("You win with four of a kind!\n\r");
    act("$n wins with four of a kind!", TRUE, ch, 0, 0, TO_ROOM);
  } else if(isFullHouse()){
    mult=9;
    ch->sendTo("You win with a full house!\n\r");
    act("$n wins with a full house!", TRUE, ch, 0, 0, TO_ROOM);
  } else if(isFlush()){
    mult=6;
    ch->sendTo("You win with a flush!\n\r");
    act("$n wins with a flush!", TRUE, ch, 0, 0, TO_ROOM);
  } else if(isStraight()){
    mult=4;
    ch->sendTo("You win with a straight!\n\r");
    act("$n wins with a straight!", TRUE, ch, 0, 0, TO_ROOM);
  } else if(isThreeOfAKind()){
    mult=3;
    ch->sendTo("You win with three of a kind!\n\r");
    act("$n wins with three of a kind!", TRUE, ch, 0, 0, TO_ROOM);
  } else if(isTwoPair()){
    mult=2;
    ch->sendTo("You win with two pair!\n\r");
    act("$n wins with two pair!", TRUE, ch, 0, 0, TO_ROOM);
  } else if(isPair()){
    mult=1;
    ch->sendTo("You win with a pair!\n\r");
    act("$n wins with a pair!", TRUE, ch, 0, 0, TO_ROOM);
  }

  if(mult>0){
    payout(ch, (int)((double)bet * mult));
    observerReaction(ch, GAMBLER_WON);
  } else {
    ch->sendTo("You lose.\n\r");
    act("$n loses.", TRUE, ch, 0, 0, TO_ROOM);
    observerReaction(ch, GAMBLER_LOST);
  }

  bet = 0;
}

bool PokerGame::isRoyalFlush()
{
  if(!isStraightFlush())
    return false;

  qsort(card, 5, sizeof(int), cardCompare);

  if(CARD_NUM_ACEHI(card[4])!=14)
    return false;
  

  return true;
}

bool PokerGame::isStraightFlush()
{
  if(isFlush() && isStraight())
    return true;

  return false;
}

bool PokerGame::isFourOfAKind()
{
  int matches=0;

  qsort(card, 5, sizeof(int), cardCompare);

  for(int i=1;i<5;++i){
    if(CARD_NUM_ACEHI(card[i]) == CARD_NUM_ACEHI(card[i-1]))
      ++matches;
    else
      matches=0;

    if(matches==3)
      return true;
  }

  return false;
}

bool PokerGame::isFullHouse()
{
  qsort(card, 5, sizeof(int), cardCompare);

  if(CARD_NUM_ACEHI(card[0])!=CARD_NUM_ACEHI(card[1]) ||
     CARD_NUM_ACEHI(card[3])!=CARD_NUM_ACEHI(card[4]))
    return false;

  if(CARD_NUM_ACEHI(card[0])!=CARD_NUM_ACEHI(card[2]) &&
     CARD_NUM_ACEHI(card[4])!=CARD_NUM_ACEHI(card[2]))
    return false;

  return true;
}


bool PokerGame::isFlush()
{
  for(int i=1;i<5;++i){
    if(!same_suit(card[0], card[i]))
      return false;
  }

  return true;
}

bool PokerGame::isStraight()
{
  qsort(card, 5, sizeof(int), cardCompare);

  for(int i=1;i<5;++i){
    if(CARD_NUM_ACEHI(card[i]) != (CARD_NUM_ACEHI(card[i-1])+1))
      return false;
  }

  return true;
}

bool PokerGame::isThreeOfAKind()
{
  int matches=0;

  qsort(card, 5, sizeof(int), cardCompare);

  for(int i=1;i<5;++i){
    if(CARD_NUM_ACEHI(card[i]) == CARD_NUM_ACEHI(card[i-1]))
      ++matches;
    else
      matches=0;

    if(matches==2)
      return true;
  }

  return false;
}

bool PokerGame::isTwoPair()
{
  int matches=0;

  qsort(card, 5, sizeof(int), cardCompare);

  for(int i=1;i<5;++i){
    if(CARD_NUM_ACEHI(card[i]) == CARD_NUM_ACEHI(card[i-1])){
      ++matches;
      ++i;
    }
  }
  
  if(matches==2)
    return true;

  return false;
}

bool PokerGame::isPair()
{
  int matches=0;

  qsort(card, 5, sizeof(int), cardCompare);

  for(int i=1;i<5;++i){
    if(CARD_NUM_ACEHI(card[i]) == CARD_NUM_ACEHI(card[i-1]) &&
       CARD_NUM_ACEHI(card[i]) > 10) // jacks or higher
      ++matches;
  }

  if(matches==1)
    return true;

  return false;
}


void PokerGame::Bet(TBeing *ch, const sstring &arg)
{
  int inx;
  sstring coin_str;
  sstring log_msg;
  sstring buf;
  TObj *chip;

  if (ch->checkPoker()) {
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

    if (deck_inx > 10)
      poker_shuffle(ch);

    log_msg="You are dealt:\n\r";
    ch->sendTo(COLOR_BASIC, log_msg);
    act("$n is dealt:", TRUE, ch, 0, 0, TO_ROOM);

    for(int i=0;i<5;++i){
      card[i] = deck[deck_inx++];

      log_msg = fmt("%i) %s\n\r") % (i+1) % pretty_card_printout(ch, card[i]);
      ch->sendTo(COLOR_BASIC, log_msg);
      

      log_msg = fmt("%s") %pretty_card_printout(ch, card[i]);
      act(log_msg, TRUE, ch, 0, 0, TO_ROOM);
    }    

    observerReaction(ch, GAMBLER_BET);
  }
}

void PokerGame::discard(TBeing *ch, sstring arg)
{
  sstring buf;
  int i;

  for(int j=0;!arg.word(j).empty();++j){
    i=convertTo<int>(arg.word(j));

    if(!i || i<1 || i>5)
      continue;
    
    
    if(!card[--i]){
      ch->sendTo("You've already discarded that card.\n\r");
      return;
    }
    
    
    ch->sendTo(COLOR_BASIC, fmt("You discard %s.\n\r") %
	       pretty_card_printout(ch, card[i]));
    buf = fmt("$n discards %s.") %
      pretty_card_printout(ch, card[i]);
    act(buf, TRUE, ch, 0, 0, TO_ROOM);

    card[i]=0;
  }
  return;
}



void PokerGame::peek(const TBeing *ch)
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

  for(int i=0;i<5;++i){
    if(card[i]){
      log_msg = fmt("%i) %s\n\r") % (i+1) %
	pretty_card_printout(ch, card[i]);
      ch->sendTo(COLOR_BASIC, log_msg);
    }
  }    

}


int PokerGame::exitGame(const TBeing *ch)
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
  for(int i=0;i<5;++i)
    card[i]=0;
  setup_deck();
  ch->sendTo("You leave the poker table.\n\r");
  return TRUE;
}


int PokerGame::index(const TBeing *ch) const
{
  if(ch->name == name)
    return 0;

  return -1;
}
