#include "stdsneezy.h"
#include "games.h"

HoldemGame gHoldem;



bool TBeing::checkHoldem(bool inGame = false) const
{
  gHoldem.linkPlayers();

  if (in_room == ROOM_HOLDEM && (inGame || gHoldem.isPlaying(this)))
    return true;
  else
    return false;
}

void HoldemGame::nextRound(TBeing *ch)
{
  switch(state){
    case STATE_NONE:
      break;
    case STATE_DEAL:
      flop(ch);
      break;
    case STATE_FLOP:
      turn(ch);
     break;
    case STATE_TURN:
      river(ch);
      break;
    case STATE_RIVER:
      showdown(ch);
      break;
    }
}



int HoldemGame::handValue(HoldemPlayer *hp){
  // this code jacked and modified from fish.c by Roy T. Hashimoto
  int i;/* loop variable */
  int s_len = 0;/* straight length */
  int sf_accum = 0;/* straight flush status */
  int f_accum = 0x3333;/* flush status */
  int state = 0;/* match state */
  int rval = 0;/* return value */
  int rank[13];
  int c;
  int t[4] = { 0x1, 0x2, 0x4, 0x8 };

  /* spreads four bits into four nybbles for adding up flushes */
  unsigned int f_table[] = {
    0x0000, 0x0001, 0x0010, 0x0011,
    0x0100, 0x0101, 0x0110, 0x0111,
    0x1000, 0x1001, 0x1010, 0x1011,
    0x1100, 0x1101, 0x1110, 0x1111,
  };
  
  /*
    finite state machine for matches (pairs, trips, etc.)
    
    m_table[current state][rank multiplicity] := next state
    
    There are six achieveable states (rags, one pair, two pair, trips,
    full house, four of a kind) and 0-4 cards of any particular rank.
    All hand ranks are included for a small time-for space tradeoff
    in making the state value equal the return value.
  */
  int m_table[9][16] = {
    {0, 0, 0, 1, 0, 1, 1, 3, 0, 1, 1, 3, 1, 3, 3, 7},
    {1, 1, 1, 2, 1, 2, 2, 6, 1, 2, 2, 6, 2, 6, 6, 7},
    {2, 2, 2, 2, 2, 2, 2, 6, 2, 2, 2, 6, 2, 6, 6, 7},
    {3, 3, 3, 6, 3, 6, 6, 6, 3, 6, 6, 6, 6, 6, 6, 7},
    {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
    {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
    {6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 7},
    {7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7},
    {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
  };


  // initialize rank
  for(i=0;i<13;++i) rank[i]=0;
  for(i=0;i<5;++i){
    c=((community[i]->getValAceHi()-2)*4) + community[i]->getSuit();

    if(!(rank[c >> 2] & t[c & 3]))
      rank[c >> 2] |= t[c & 3];
  }
  for(i=0;i<2;++i){
    c=((hp->hand[i]->getValAceHi()-2)*4) + hp->hand[i]->getSuit();
    
    if(!(rank[c >> 2] & t[c & 3]))
      rank[c >> 2] |= t[c & 3];
  }


  i = 13;
  while ( i-- )
    {
      if ( !rank[i] )/* loop again if no instances */
	{
	  s_len = 0; /* the next card will not be contiguous */
	  continue;
	}


      if ( ++s_len >= 5 )/* made a straight */
	{
	  int tmp;

	  /* see if the suit matches the last four */
	  if ( (tmp = rank[i] & sf_accum) &&
	       (tmp &= sf_accum >> 4) &&
	       (tmp &= sf_accum >> 8) &&
	       (tmp & sf_accum >> 12) )
	    {
	      rval = 8;
	      break;/* why look further? */
	    }
	  else if ( rval < 4 )
	    rval = 4;
	}
      
      /* remember the suit(s) of this rank */
      sf_accum = (sf_accum << 4) | rank[i];

      /*
	The number of cards in a suit is incremented in
	parallel here.  The result is held in 4 bits.  The
	count starts at 3 so the test for the flush can be
	made against the fourth bit (3+5=8, 1000b).
      */
      if ( ((f_accum += f_table[rank[i]]) & 0x8888) && rval < 5 )
	{
	  f_accum = 0;/* to avoid further success */
	  rval = 5;
	}

      /* Run the state machine for the other hand rankings */
      state = m_table[state][rank[i]];
    }

  /* special case aces (can be used high or low in straights) */
  if ( rank[12] && ++s_len >= 5 )
    {
      int tmp;

      if ( (tmp = rank[12] & sf_accum) &&
	   (tmp &= sf_accum >> 4) &&
	   (tmp &= sf_accum >> 8) &&
	   (tmp & sf_accum >> 12) )
	rval = 8;
      else if ( rval < 4 )
	rval = 4;
    }

  /* see if the state machine result is better than the other results */
  if ( rval < state )
    rval = state;

  return rval;
}


void HoldemGame::showdown(TBeing *ch)
{
  int hands[MAX_HOLDEM_PLAYERS], highest=0;
  vector <int> winners;
  int i;
  sstring buf, msg;

  if (!ch->checkHoldem())
    return;

  if(playerCount() < 2){
    act("$n wins by default.", TRUE, ch, 0, 0, TO_ROOM);
    act("You win by default.", TRUE, ch, 0, 0, TO_CHAR);
    payout(ch, bet);
  } else {
    for(i=0;i<MAX_HOLDEM_PLAYERS;++i){
      hands[i]=-1;
      if(players[i]){
	hands[i]=handValue(players[i]);
      }
    }
    for(i=0;i<MAX_HOLDEM_PLAYERS;++i){
      if(hands[i] > hands[highest])
	highest=i;
    }
    for(i=0;i<MAX_HOLDEM_PLAYERS;++i){
      if(hands[i] == hands[highest])
	winners.push_back(i);
    }
    
    switch(hands[highest]){
      case 0:
	msg="high card";
	break;
      case 1:
	msg="a pair";
	break;
      case 2:
	msg="two pair";
	break;
      case 3:
	msg="three of a kind";
	break;
      case 4:
	msg="a straight";
	break;
      case 5:
	msg="a flush";
	break;
      case 6:
	msg="full house";
	break;
      case 7:
	msg="four of a kind";
	break;
      case 8:
	msg="a straight flush";
	break;
      case 9:
	msg="royal flush";
	break;
      default:
	ssprintf(msg, "unknown: %i", hands[highest]);
    }
    
    for(unsigned int p=0;p<winners.size();++p){
      ssprintf(buf, "$n %s with %s!", 
	       winners.size()>1?"ties":"wins", msg.c_str());
      act(buf.c_str(), TRUE, players[winners[p]]->ch, 0, 0, TO_ROOM);
      ssprintf(buf, "You %s with %s!", 
	       winners.size()>1?"tie":"win", msg.c_str());
      act(buf, TRUE, players[winners[p]]->ch, 0, 0, TO_CHAR);
      payout(players[winners[p]]->ch, (int)(bet/winners.size()));
    }
  }


  for(i=0;i<MAX_HOLDEM_PLAYERS;++i){
    if(players[i]){
      players[i]->hand[0]=NULL;
      players[i]->hand[1]=NULL;
    }
  }


  better=0;
  bet=0;
  last_bet=0;
  state=STATE_NONE;

  for(int i=0;i<5;++i){
    community[i]=NULL;
  }
}


void HoldemGame::linkPlayers()
{
  for(int i=0;i<MAX_HOLDEM_PLAYERS;++i){
    if(players[i]){
      if(!(players[i]->ch=get_char_room(players[i]->name, ROOM_HOLDEM))){
	delete players[i];
	players[i]=NULL;
      }
    }
  }
}

int HoldemGame::nextPlayer(int b)
{
  for(int i=b+1;i<MAX_HOLDEM_PLAYERS;++i){
    if(players[i] && players[i]->hand[0])
      return i;
  }
   
  return b;
}

int HoldemGame::lastPlayer()
{
  int p=0;

  for(int i=0;i<MAX_HOLDEM_PLAYERS;++i){
    if(players[i] && players[i]->hand[0])
      p=i;
  }

  return p;
}

int HoldemGame::firstPlayer()
{
  int p=0;

  for(int i=0;i<MAX_HOLDEM_PLAYERS;++i){
    if(players[i] && players[i]->hand[0])
      return i;
  }

  return p;
}



int HoldemGame::playerCount()
{
  int count=0;
  
  for(int i=0;i<MAX_HOLDEM_PLAYERS;++i){
    if(players[i])
      ++count;
  }
  return count;
}

int HoldemGame::playerHandCount()
{
  int count=0;
  
  for(int i=0;i<MAX_HOLDEM_PLAYERS;++i){
    if(players[i] && players[i]->hand[0])
      ++count;
  }
  return count;
}


int HoldemGame::exitGame(const TBeing *ch)
{
  if (!ch->checkHoldem())
    return false;

  for(int i=0;i<MAX_HOLDEM_PLAYERS;++i){
    if(players[i] && players[i]->name == ch->name){
      ch->sendTo("You leave the hold'em table.\n\r");
      delete players[i];
      players[i]=NULL;
      break;
    }
  }

  if(playerCount() < 2){
    showdown(players[firstPlayer()]->ch);
  }
  
  return false;
}


bool HoldemGame::enter(const TBeing *ch)
{
  for(int i=0;i<MAX_HOLDEM_PLAYERS;++i){
    if(players[i]==NULL){
      players[i]=new HoldemPlayer(ch);
      ch->sendTo("You move up to the hold'em table.\n\r");
      return true;
    }
  }

  ch->sendTo("This table is full.\n\r");
  return false;
}


bool HoldemGame::isPlaying(const TBeing *ch) const
{
  return getPlayer(ch->name);
}

HoldemPlayer *HoldemGame::getPlayer(const sstring &name) const
{
  for(int i=0;i<MAX_HOLDEM_PLAYERS;++i){
    if(players[i] && players[i]->name == name)
      return players[i];
  }

  return NULL;
}


void HoldemGame::peek(const TBeing *ch) const
{
  sstring log_msg;

  if (!ch->checkHoldem())
    return;

  if(!isPlaying(ch)){
    ch->sendTo("You are not sitting at the table yet.\n\r");
    return;
  }

  if (!bet) {
    ch->sendTo("You are not playing a game.\n\r");
    return;
  }

  HoldemPlayer *tmp = getPlayer(ch->name);

  ch->sendTo(COLOR_BASIC, "You peek at your hand:\n\r");
  ch->sendTo(COLOR_BASIC, "%s\n\r", tmp->hand[0]->getName());
  ch->sendTo(COLOR_BASIC, "%s\n\r", tmp->hand[1]->getName());
  
  if(community[0]){
    ch->sendTo(COLOR_BASIC, "\n\rYou peek at the community cards:\n\r");

    for(int i=0;i<5;++i){
      if(community[i]){
	ch->sendTo(COLOR_BASIC, "%s\n\r", community[i]->getName());
      }
    }
  }
}


void HoldemGame::call(TBeing *ch)
{
  sstring buf;
  TObj *chip;
  vector <TObj *> chipl;

  if (!ch->checkHoldem())
    return;

  if (!isPlaying(ch)) {
    ch->sendTo("You are not sitting at the table yet.\n\r");
    return;
  }
  if(state==STATE_NONE){
    ch->sendTo("Betting hasn't started.\n\r");
    return;
  }

  if(playerCount() < 2){
    ch->sendTo("You need at least two players.\n\r");
    return;
  }

  if(ch->name != players[better]->name){
    ch->sendTo("It's not your turn.\n\r");
    return;
  }
  
  if(!(chip=find_chip(ch, last_bet))){
    ch->sendTo("You don't have the required chip!\n\r");
    return;
  }

  for(int i=0;i<nraises;++i){
    if(!(chip=find_chip(ch, last_bet))){
      ch->sendTo("You don't have the required chip!\n\r");
      for(unsigned int i=0;i<chipl.size();++i){
	*ch += *chipl[i];
      }
      return;
    }

    (*chip)--;
    chipl.push_back(chip);
  }

  for(unsigned int i=0;i<chipl.size();++i){
    ssprintf(buf, "$n calls with %s.", chipl[i]->getName());
    act(buf, TRUE, ch, 0, 0, TO_ROOM);
    ssprintf(buf, "You call with %s.", chipl[i]->getName());
    act(buf, TRUE, ch, 0, 0, TO_CHAR);

    bet += chip->obj_flags.cost;

    delete chipl[i];
  }


  ch->doSave(SILENT_YES);
   
  if(players[better]->name!=players[lastPlayer()]->name){
    better=nextPlayer(better);
    act("The bet moves to $n.", TRUE, players[better]->ch, 0, 0, TO_ROOM);
    players[better]->ch->sendTo(COLOR_BASIC, "You can <c>raise<1>, <c>fold<1>, or <c>call<1>.\n\r");
  } else {
    nextRound(players[better]->ch);
  }
}

void HoldemGame::raise(TBeing *ch)
{
  sstring buf;
  TObj *chip;
  vector <TObj *> chipl;

  if (!ch->checkHoldem())
    return;

  if (!isPlaying(ch)) {
    ch->sendTo("You are not sitting at the table yet.\n\r");
    return;
  }
  if(state==STATE_NONE){
    ch->sendTo("Betting hasn't started.\n\r");
    return;
  }

  if(playerCount() < 2){
    ch->sendTo("You need at least two players.\n\r");
    return;
  }

  if(ch->name != players[better]->name){
    ch->sendTo("It's not your turn.\n\r");
    return;
  }

  nraises++;

  for(int i=0;i<nraises;++i){
    if(!(chip=find_chip(ch, last_bet))){
      ch->sendTo("You don't have the required chip!\n\r");
      for(unsigned int i=0;i<chipl.size();++i){
	*ch += *chipl[i];
      }
      return;
    }
    bet += chip->obj_flags.cost;

    (*chip)--;
    chipl.push_back(chip);
  }

  for(unsigned int i=0;i<chipl.size();++i){
    ssprintf(buf, "$n raises with %s.", chipl[i]->getName());
    act(buf, TRUE, ch, 0, 0, TO_ROOM);
    ssprintf(buf, "You raises with %s.", chipl[i]->getName());
    act(buf, TRUE, ch, 0, 0, TO_CHAR);
    bet += chip->obj_flags.cost;
  
    delete chipl[i];
  }

  
  ch->doSave(SILENT_YES);
  
  if(players[better]->name!=players[lastPlayer()]->name){
    better=nextPlayer(better);
    act("The bet moves to $n.", TRUE, players[better]->ch, 0, 0, TO_ROOM);
    players[better]->ch->sendTo(COLOR_BASIC, "You can <c>raise<1>, <c>fold<1>, or <c>call<1>.\n\r");
  } else {
    nextRound(players[better]->ch);
  }
}



void HoldemGame::flop(TBeing *ch)
{
  sstring buf;

  if (!ch->checkHoldem())
    return;

  act("The flop is:", TRUE, ch, 0, 0, TO_ROOM);
  act("The flop is:", TRUE, ch, 0, 0, TO_CHAR);
  
  community[0]=deck.draw();
  ssprintf(buf, "%s", community[0]->getName());
  act(buf, TRUE, ch, 0, 0, TO_ROOM);
  act(buf, TRUE, ch, 0, 0, TO_CHAR);
  
  community[1]=deck.draw();
  ssprintf(buf, "%s", community[1]->getName());
  act(buf, TRUE, ch, 0, 0, TO_ROOM);
  act(buf, TRUE, ch, 0, 0, TO_CHAR);
  
  community[2]=deck.draw();
  ssprintf(buf, "%s", community[2]->getName());
  act(buf, TRUE, ch, 0, 0, TO_ROOM);
  act(buf, TRUE, ch, 0, 0, TO_CHAR);
  
  better=firstPlayer();

  act("The bet moves to $n.",
      TRUE, players[better]->ch, 0, 0, TO_ROOM);
  players[better]->ch->sendTo(COLOR_BASIC, "You can <c>raise<1>, <c>fold<1>, or <c>call<1>.\n\r");
  
  state=STATE_FLOP;
}

void HoldemGame::turn(TBeing *ch)
{
  sstring buf;

  if (!ch->checkHoldem())
    return;

  act("The turn is:", TRUE, ch, 0, 0, TO_ROOM);
  act("The turn is:", TRUE, ch, 0, 0, TO_CHAR);
  
  community[3]=deck.draw();
  ssprintf(buf, "%s", community[3]->getName());
  act(buf, TRUE, ch, 0, 0, TO_ROOM);
  act(buf, TRUE, ch, 0, 0, TO_CHAR);
  
  better=firstPlayer();

  act("The bet moves to $n.",
      TRUE, players[better]->ch, 0, 0, TO_ROOM);
  players[better]->ch->sendTo(COLOR_BASIC, "You can <c>raise<1>, <c>fold<1>, or <c>call<1>.\n\r");
  
  state=STATE_TURN;
}

void HoldemGame::river(TBeing *ch)
{
  sstring buf;

  if (!ch->checkHoldem())
    return;

  act("The river is:", TRUE, ch, 0, 0, TO_ROOM);
  act("The river is:", TRUE, ch, 0, 0, TO_CHAR);
  
  community[4]=deck.draw();
  ssprintf(buf, "%s", community[4]->getName());
  act(buf, TRUE, ch, 0, 0, TO_ROOM);
  act(buf, TRUE, ch, 0, 0, TO_CHAR);
  
  better=firstPlayer();

  act("The bet moves to $n.",
      TRUE, players[better]->ch, 0, 0, TO_ROOM);
  players[better]->ch->sendTo(COLOR_BASIC, "You can <c>raise<1>, <c>fold<1>, or <c>call<1>.\n\r");
  
  state=STATE_RIVER;
}


void HoldemGame::fold(TBeing *ch)
{
  if (!ch->checkHoldem())
    return;

  if (!isPlaying(ch)) {
    ch->sendTo("You are not sitting at the table yet.\n\r");
    return;
  }
  if(state==STATE_NONE){
    ch->sendTo("Betting hasn't started.\n\r");
    return;
  }

  if(playerCount() < 2){
    ch->sendTo("You need at least two players.\n\r");
    return;
  }

  if(ch->name != players[better]->name){
    ch->sendTo("It's not your turn.\n\r");
    return;
  }

  act("$n folds.", TRUE, players[better]->ch, 0, 0, TO_ROOM);
  act("You fold.", TRUE, players[better]->ch, 0, 0, TO_CHAR);

  if(playerHandCount() == 2){
    players[better]->hand[0]=NULL;
    players[better]->hand[1]=NULL;
    showdown(players[firstPlayer()]->ch);
  } else if(players[better]->name!=players[lastPlayer()]->name){
    players[better]->hand[0]=NULL;
    players[better]->hand[1]=NULL;
    better=nextPlayer(better);
    act("The bet moves to $n.", TRUE, players[better]->ch, 0, 0, TO_ROOM);
    players[better]->ch->sendTo(COLOR_BASIC, "You can <c>raise<1>, <c>fold<1>, or <c>call<1>.\n\r");
  } else {
    players[better]->hand[0]=NULL;
    players[better]->hand[1]=NULL;
    nextRound(players[better]->ch);
  }
}

void HoldemGame::Bet(TBeing *ch, const sstring &arg)
{
  const Card *card;
  sstring coin_str;
  TObj *chip;
  int i;

  if (!ch->checkHoldem())
    return;

  if (!isPlaying(ch)) {
    ch->sendTo("You are not sitting at the table.\n\r");
    return;
  }
  if(state!=STATE_NONE){
    ch->sendTo("Betting has already started.\n\r");
    return;
  }

  if(playerCount() < 2){
    ch->sendTo("You need at least two players.\n\r");
    return;
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
  last_bet = chip->objVnum();
  nraises=1;
  ch->doSave(SILENT_YES);
  
  sstring buf;
  ssprintf(buf, "$n bets %s.", chip->getName());
  act(buf, TRUE, ch, 0, 0, TO_ROOM);
  ssprintf(buf, "You bet %s.", chip->getName());
  act(buf, TRUE, ch, 0, 0, TO_CHAR);
  
  (*chip)--;
  delete chip;
  
  deck.shuffle();
  act("The dealer shuffles the deck.",FALSE, ch, 0, 0, TO_CHAR);
  act("The dealer shuffles the deck.",FALSE, ch, 0, 0, TO_ROOM);

  // find this better in the list
  for(i=0;i<MAX_HOLDEM_PLAYERS;++i){
    if(players[i] && players[i]->name == ch->name)
      break;
  }

  // swap with first player
  HoldemPlayer *tmp=players[0];
  players[0]=players[i];
  players[i]=tmp;
  better=0;
  
  // deal cards to everyone
  for(int i=0;i<MAX_HOLDEM_PLAYERS;++i){    
    if(!players[i])
      continue;

    players[i]->ch->sendTo(COLOR_BASIC, "You are dealt:\n\r");
    act("$n is dealt two cards facedown.\n\r", 
	TRUE, players[i]->ch, 0, 0, TO_ROOM);
    
    card=deck.draw();
    players[i]->hand[0]=card;
    players[i]->ch->sendTo(COLOR_BASIC, "%s\n\r", card->getName());
    
    card=deck.draw();
    players[i]->hand[1]=card;
    players[i]->ch->sendTo(COLOR_BASIC, "%s\n\r", card->getName());
  }
    
  // move the bet to the next person
  better=nextPlayer(better);
  act("The bet moves to $n.", TRUE, players[better]->ch, 0, 0, TO_ROOM);
  players[better]->ch->sendTo(COLOR_BASIC, "You can <c>raise<1>, <c>fold<1>, or <c>call<1>.\n\r");
  
  state=STATE_DEAL;
}



/*

deal 2 cards
bet
deal 3 community cards
bet
deal 1
bet
deal 1
bet

first player: bet fold or check
after that:   raise fold or call




*/

