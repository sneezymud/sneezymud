#include "stdsneezy.h"
#include "games.h"

// this is a good example of how not to write code
// no need to thank me - peel


HoldemGame gHoldem;

bool TBeing::checkHoldem(bool inGame) const
{
  gHoldem.linkPlayers();

  if (in_room == ROOM_HOLDEM && (inGame || gHoldem.isPlaying(this)))
    return true;
  else
    return false;
}


void HoldemGame::advanceRound(TBeing *ch)
{
  nraises_round=0;
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
  int rank[4][15];
  int i, tmp;
  int straight=0, flush[4], pairs=0, triplets=0;
  int score[10];
  
  for(i=0;i<10;++i)
    score[i]=0;

  for(i=0;i<15;++i)
    for(int j=0;j<4;++j)
      rank[j][i]=0;

  for(i=0;i<4;++i)
    flush[i]=0;

  for(i=0;i<2;++i){
    if(hp->hand[i])
      rank[hp->hand[i]->getSuit()][hp->hand[i]->getValAceHi()]=1;
  }
  
  for(i=0;i<5;++i){
    if(community[i])
      rank[community[i]->getSuit()][community[i]->getValAceHi()]=1;
  }

  for(i=0;i<15;++i){
    if(!rank[0][i] && !rank[1][i] && !rank[2][i] && !rank[3][i]){
      straight=0;
      continue;
    }
    

    for(int j=0;j<4;++j){
      // get highcard
      if(rank[j][i]){
	score[0]=i;
      }

      // check for flush
      if((flush[j]+=rank[j][i]) >= 5){
	score[5]=i;
	// flush
      }
    }

    tmp=rank[0][i]+rank[1][i]+rank[2][i]+rank[3][i];
    if(tmp == 4){
      score[7]=i;
      // four of a kind
    } else if(tmp==3){
      score[3]=i;
      // three of a kind
    } else if(tmp==2){
      score[1]=i;
      // pair
    }

    if(tmp==3 && triplets){
      pairs++;
    } else if(tmp==3)
      triplets++;
    else if(tmp==2){
      pairs++;
      if(pairs >= 2){
	// 2 pair
	score[2]=i;
      }
    }

    if(pairs && triplets){
      // full house
      score[6]=i; 
    }
    

    
    if((++straight) >= 5){
      // straight
      score[4]=i;
      
      for(int j=0;j<4;++j){
	if(rank[j][i] && rank[j][i-1] && rank[j][i-2] &&
	   rank[j][i-3] && rank[j][i-4]){
	  // straight flush
	  score[8]=i;
	  if(i==14){
	    // royal flush
	    score[9]=i;
	  }
	}
      }
    }
  }

  for(i=9;i>=0;--i){
    if(score[i])
      return score[i]+(i*15);
  }
  
  return 0;
}

const sstring cards_names[]={
  "unknown",
  "unknown",
  "deuce",
  "three",
  "four",
  "five",
  "six",
  "seven",
  "eight",
  "nine",
  "ten",
  "jack",
  "queen",
  "king",
  "ace",
};

sstring HoldemGame::handValToStr(int val){
  sstring msg;
  
  switch((int)((float)val/15.0)){
    case 0:
      msg = fmt("high card %s") % cards_names[val%15];
      break;
    case 1:
      msg = fmt("a pair of %ss") % cards_names[val%15];
      break;
    case 2:
      msg = fmt("two pair, high pair %ss") % cards_names[val%15];
      break;
    case 3:
      msg = fmt("three of a kind, %ss") % cards_names[val%15];
      break;
    case 4:
      msg = fmt("a straight, high card %s") % cards_names[val%15];
      break;
    case 5:
      msg = fmt("a flush, high card %s") % cards_names[val%15];
      break;
    case 6:
      msg = fmt("a full house, high card %s") % cards_names[val%15];
      break;
    case 7:
      msg = fmt("four of a kind, %ss") % cards_names[val%15];
      break;
    case 8:
      msg = fmt("a straight flush, high card %s") % cards_names[val%15];
      break;
    case 9:
      msg = fmt("a royal flush") % cards_names[val%15];
      break;
  }

  return msg;
}


void HoldemGame::showdown(TBeing *ch)
{
  int hands[MAX_HOLDEM_PLAYERS], highest=0;
  vector <int> winners;
  int i;
  sstring buf, msg;

  if (!ch->checkHoldem())
    return;

  if(playerHandCount() < 2){
    act("$n wins by default.", FALSE, ch, 0, 0, TO_ROOM);
    act("You win by default.", FALSE, ch, 0, 0, TO_CHAR);
    payout(ch, bet, last_bet);
  } else {
    for(i=0;i<MAX_HOLDEM_PLAYERS;++i){
      hands[i]=-1;
      if(players[i] && players[i]->hand[0]){
	hands[i]=handValue(players[i]);

	act("$n's hand was:", FALSE, players[i]->ch, 0, 0, TO_ROOM);
	buf = fmt("%s") % players[i]->hand[0]->getName();
	act(buf, FALSE, players[i]->ch, 0, 0, TO_ROOM);
	buf = fmt("%s") % players[i]->hand[1]->getName();
	act(buf, FALSE, players[i]->ch, 0, 0, TO_ROOM);
	buf = fmt("$n has %s.\n\r") % handValToStr(hands[i]);
	act(buf, FALSE, players[i]->ch, 0, 0, TO_ROOM);

	buf = fmt("You have %s.\n\r") % handValToStr(hands[i]);
	act(buf, FALSE, players[i]->ch, 0, 0, TO_CHAR);

	players[i]->allin=false;
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

    msg=handValToStr(hands[highest]);
    
    for(unsigned int p=0;p<winners.size();++p){
      buf = fmt("$n %s with %s!") %
	       (winners.size()>1?"ties":"wins") % msg;
      act(buf, FALSE, players[winners[p]]->ch, 0, 0, TO_ROOM);
      buf = fmt("You %s with %s!") %
	       (winners.size()>1?"tie":"win") % msg;
      act(buf, FALSE, players[winners[p]]->ch, 0, 0, TO_CHAR);
      payout(players[winners[p]]->ch, (int)(bet/winners.size()), last_bet);
    }
  }


  // show chip count
  int tcount=0;
  TObj *obj;
  for(i=0;i<MAX_HOLDEM_PLAYERS;++i){
    if(players[i] && players[i]->ch){
      for(TThing *t=players[i]->ch->getStuff();t;t=t->nextThing){
	if((obj=dynamic_cast<TObj *>(t))){
	  if(obj->objVnum() == last_bet)
	    tcount++;
	}
      }
      
      buf = fmt("$n has %i chips left.") % tcount;
      act(buf, FALSE, players[i]->ch, 0, 0, TO_ROOM);
      buf = fmt("You have %i chips left.") % tcount;
      act(buf, FALSE, players[i]->ch, 0, 0, TO_CHAR);
      tcount=0;
    }
  }


  // move button
  button=nextButton();

  act("The button moves to $n.",
      FALSE, players[button]->ch, 0, 0, TO_ROOM);
  act("The button moves to you.",
      FALSE, players[button]->ch, 0, 0, TO_CHAR);


  // reinitialize players
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

  int count;
  for(int i=0;i<MAX_HOLDEM_PLAYERS;++i){
    count=0;

    for(int j=0;j<MAX_HOLDEM_PLAYERS;++j){
      if(players[i] && players[j] && players[i]->name == players[j]->name){
	if((++count) > 1){
	  // this entry is in here more than once!
	  //	  vlogf(LOG_PEEL, fmt("duplicate entry, removing player %s") %  players[i]->name);
	  delete players[i];

	}
      }
    }
  }

}

int HoldemGame::nextBetter(int b)
{
  // find the next better that isn't all in
  for(int i=(b+1)%MAX_HOLDEM_PLAYERS;
      i!=firstbetter;
      i=(i+1)%MAX_HOLDEM_PLAYERS){
    if(players[i] && players[i]->ch && 
       players[i]->hand[0] && !players[i]->allin){
      return i;
    }
  }
  return -1;
}

int HoldemGame::nextPlayer(int b)
{
  // find the next better that isn't all in
  for(int i=(b+1)%MAX_HOLDEM_PLAYERS;
      i!=firstbetter;
      i=(i+1)%MAX_HOLDEM_PLAYERS){
    if(i < MAX_HOLDEM_PLAYERS && i>=0 && players[i] && players[i]->ch){
      return i;
    }
  }
  return -1;
}

int HoldemGame::nextButton()
{
  for(int i=(button+1)%MAX_HOLDEM_PLAYERS;
      i!=button;
      i=(i+1)%MAX_HOLDEM_PLAYERS){
    if(players[i] && players[i]->ch && players[i]->ch->isPc()){
      return i;
    }
  }
  return button;
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
  bool was_better=false;

  if (!ch->checkHoldem())
    return false;

  for(int i=0;i<MAX_HOLDEM_PLAYERS;++i){
    if(players[i] && players[i]->name == ch->name){
      if(players[better] && players[i]->name == players[better]->name)
	was_better=true;

      ch->sendTo("You leave the hold'em table.\n\r");
      delete players[i];
      players[i]=NULL;
      break;
    }
  }

  if(playerHandCount() == 1){
    showdown(players[firstPlayer()]->ch);
  } else if(was_better && state!=STATE_NONE){
    int tmp=-1;
    if((tmp=nextBetter(better))!=-1){
      better=tmp;
      act("The bet moves to $n.", FALSE, players[better]->ch, 0, 0, TO_ROOM);
      players[better]->ch->sendTo(COLOR_BASIC, "You can <c>raise<1>, <c>fold<1> or <c>call<1>.\n\r");
    } else {
      advanceRound(players[better]->ch);
    }
  }
  
  return false;
}


bool HoldemGame::enter(const TBeing *ch)
{
  if(getPlayer(ch->name)){
    ch->sendTo("Someone of that name is already playing.");
    return false;
  }

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
    if(players[i] && players[i]->name == name){
      return players[i];
    }
  }

  return NULL;
}

sstring HoldemGame::stateToString()
{
  sstring buf;

  switch(state){
    case STATE_DEAL:
      return "deal";
    case STATE_FLOP:
      return "flop";
    case STATE_TURN:
      return "turn";
    case STATE_RIVER:
      return "river";
    case STATE_NONE:
      return "none";
    default:
      return (fmt("unknown (%i)") % state);
  }
  return "can't get here";
}

void HoldemGame::peek(const TBeing *ch)
{
  sstring log_msg;
  HoldemPlayer *tmp;

  if (!ch->checkHoldem(true))
    return;

  if(last_bet){
    ch->sendTo(COLOR_BASIC, fmt("You peek at the pot and see: %s. [%i]\n\r\n\r") %
	       obj_index[real_object(last_bet)].short_desc % 
	       (int)(bet/obj_index[real_object(last_bet)].value));
  }


  if(ch->isImmortal() && !strcmp(ch->getName(), "Peel") && !ch->checkHoldem()){
    const Card *tc[5];


    ch->sendTo(COLOR_BASIC, fmt("Game state is <r>%s<1>\n\r\n\r") % 
	       stateToString());

    sstring buf="";
    for(int i=0;i<MAX_HOLDEM_PLAYERS;++i){
      if(players[i]){
	buf+=players[i]->name;
	buf+=" ";
      }
    }
    ch->sendTo(COLOR_BASIC, fmt("Players: %s\n\r\n\r") % 
	       (buf.empty()?"none":buf));


    for(int i=0;i<5;++i){
      tc[i]=community[i];
      if(!community[i]){
	community[i]=deck.draw();
	ch->sendTo(COLOR_BASIC, fmt("Community draw: %s\n\r") % 
		   community[i]->getName());
      }
    }

    for(int i=0;i<MAX_HOLDEM_PLAYERS;++i){
      if(players[i] && players[i]->hand[0] &&
	 players[i]->hand[1] && players[i]->name != ch->name){
	ch->sendTo(COLOR_BASIC, fmt("\n\rYou peek at %s's hand:\n\r") %
		   players[i]->name);
	ch->sendTo(COLOR_BASIC, fmt("%s\n\r") % players[i]->hand[0]->getName());
	ch->sendTo(COLOR_BASIC, fmt("%s\n\r") % players[i]->hand[1]->getName());
	ch->sendTo(COLOR_BASIC, fmt("%s has %s.\n\r") % players[i]->name %
		   handValToStr(handValue(players[i])));
      }
    }

    for(int i=0;i<5;++i){
      community[i]=tc[i];
      if(!tc[i])
	deck.undraw();
    }
  }

  tmp = getPlayer(ch->name);
  if(tmp && tmp->hand[0] && tmp->hand[1]){
    ch->sendTo(COLOR_BASIC, "You peek at your hand:\n\r");
    ch->sendTo(COLOR_BASIC, fmt("%s\n\r") % tmp->hand[0]->getName());
    ch->sendTo(COLOR_BASIC, fmt("%s\n\r") % tmp->hand[1]->getName());
  }  

  if(community[0]){
    ch->sendTo(COLOR_BASIC, "\n\rYou peek at the community cards:\n\r");

    for(int i=0;i<5;++i){
      if(community[i]){
	ch->sendTo(COLOR_BASIC, fmt("%s\n\r") % community[i]->getName());
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
  

  for(int i=0;i<nraises;++i){
    if(!(chip=find_chip(ch, last_bet))){
      players[better]->allin=true;
      break;
    }

    (*chip)--;
    chipl.push_back(chip);
  }

  if(players[better]->allin){
    if(chipl.size()==0){
      act("$n goes all in!", FALSE, ch, 0, 0, TO_ROOM);
      act("You go all in!", FALSE, ch, 0, 0, TO_CHAR);
    } else {
      buf = fmt("$n goes all in %s! [%i]") %
	chipl[0]->getName() % chipl.size();
      act(buf, FALSE, ch, 0, 0, TO_ROOM);
      buf = fmt("You go all in %s! [%i]") %
	chipl[0]->getName() % chipl.size();
      act(buf, FALSE, ch, 0, 0, TO_CHAR);
      bet += chipl[0]->obj_flags.cost * chipl.size();
    }
  } else {
    buf = fmt("$n calls with %s. [%i]") %
	     chipl[0]->getName() % chipl.size();
    act(buf, FALSE, ch, 0, 0, TO_ROOM);
    buf = fmt("You call with %s. [%i]") %
	     chipl[0]->getName() % chipl.size();
    act(buf, FALSE, ch, 0, 0, TO_CHAR);
    bet += chipl[0]->obj_flags.cost * chipl.size();
  }    

  for(unsigned int i=0;i<chipl.size();++i)
    delete chipl[i];

  ch->doSave(SILENT_YES);
  
  if(nextBetter(better)!=-1){
    better=nextBetter(better);
    act("The bet moves to $n.", FALSE, players[better]->ch, 0, 0, TO_ROOM);
    players[better]->ch->sendTo(COLOR_BASIC, "You can <c>raise<1>, <c>fold<1> or <c>call<1>.\n\r");
  } else {
    advanceRound(players[better]->ch);
  }
}

void HoldemGame::raise(TBeing *ch, const sstring &arg)
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
  
  if(nraises_round>=3){
    ch->sendTo("The maximum number of raises have already occurred.\n\r");
    return;
  }
  nraises_round++;


  if(!arg.empty()){
    int amt=convertTo<int>(arg);
    if(amt>3 || amt<1){
      ch->sendTo("Usage: raise <optional amount, 1-3>\n\r");
      return;
    } else {
      nraises+=amt;
    }
  } else {
    nraises++;
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

  buf = fmt("$n raises with %s. [%i]") %
	   chipl[0]->getName() % chipl.size();
  act(buf, FALSE, ch, 0, 0, TO_ROOM);
  buf = fmt("You raise with %s. [%i]") %
	   chipl[0]->getName() % chipl.size();
  act(buf, FALSE, ch, 0, 0, TO_CHAR);
  bet += chip->obj_flags.cost * chipl.size();

  for(unsigned int i=0;i<chipl.size();++i)
    delete chipl[i];

  ch->doSave(SILENT_YES);
  
  firstbetter=better;

  if(nextBetter(better)!=-1){
    better=nextBetter(better);
    act("The bet moves to $n.", FALSE, players[better]->ch, 0, 0, TO_ROOM);
    players[better]->ch->sendTo(COLOR_BASIC, "You can <c>raise<1>, <c>fold<1> or <c>call<1>.\n\r");
  } else {
    advanceRound(players[better]->ch);
  }
}



void HoldemGame::flop(TBeing *ch)
{
  sstring buf;

  if (!ch->checkHoldem())
    return;

  act("The flop is:", FALSE, ch, 0, 0, TO_ROOM);
  act("The flop is:", FALSE, ch, 0, 0, TO_CHAR);
  
  community[0]=deck.draw();
  buf = fmt("%s") % community[0]->getName();
  act(buf, FALSE, ch, 0, 0, TO_ROOM);
  act(buf, FALSE, ch, 0, 0, TO_CHAR);
  
  community[1]=deck.draw();
  buf = fmt("%s") % community[1]->getName();
  act(buf, FALSE, ch, 0, 0, TO_ROOM);
  act(buf, FALSE, ch, 0, 0, TO_CHAR);
  
  community[2]=deck.draw();
  buf = fmt("%s") % community[2]->getName();
  act(buf, FALSE, ch, 0, 0, TO_ROOM);
  act(buf, FALSE, ch, 0, 0, TO_CHAR);
  
  better=firstbetter;

  if(players[better]->hand[0]==NULL)
    if((better=nextBetter(better))==-1)
      advanceRound(players[better]->ch);


  act("The bet moves to $n.",
      FALSE, players[better]->ch, 0, 0, TO_ROOM);
  players[better]->ch->sendTo(COLOR_BASIC, "You can <c>raise<1>, <c>fold<1> or <c>call<1>.\n\r");
  
  state=STATE_FLOP;
}

void HoldemGame::turn(TBeing *ch)
{
  sstring buf;

  if (!ch->checkHoldem())
    return;

  act("The turn is:", FALSE, ch, 0, 0, TO_ROOM);
  act("The turn is:", FALSE, ch, 0, 0, TO_CHAR);
  
  community[3]=deck.draw();
  buf = fmt("%s") % community[3]->getName();
  act(buf, FALSE, ch, 0, 0, TO_ROOM);
  act(buf, FALSE, ch, 0, 0, TO_CHAR);

  better=firstbetter;
  if(players[better]->hand[0]==NULL)
    if((better=nextBetter(better))==-1)
      advanceRound(players[better]->ch);


  act("The bet moves to $n.",
      FALSE, players[better]->ch, 0, 0, TO_ROOM);
  players[better]->ch->sendTo(COLOR_BASIC, "You can <c>raise<1>, <c>fold<1> or <c>call<1>.\n\r");
  
  state=STATE_TURN;
}

void HoldemGame::river(TBeing *ch)
{
  sstring buf;

  if (!ch->checkHoldem())
    return;

  act("The river is:", FALSE, ch, 0, 0, TO_ROOM);
  act("The river is:", FALSE, ch, 0, 0, TO_CHAR);
  
  community[4]=deck.draw();
  buf = fmt("%s") % community[4]->getName();
  act(buf, FALSE, ch, 0, 0, TO_ROOM);
  act(buf, FALSE, ch, 0, 0, TO_CHAR);
  
  better=firstbetter;
  if(players[better]->hand[0]==NULL)
    if((better=nextBetter(better))==-1)
      advanceRound(players[better]->ch);


  act("The bet moves to $n.",
      FALSE, players[better]->ch, 0, 0, TO_ROOM);
  players[better]->ch->sendTo(COLOR_BASIC, "You can <c>raise<1>, <c>fold<1> or <c>call<1>.\n\r");
  
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

  act("$n folds.", FALSE, players[better]->ch, 0, 0, TO_ROOM);
  act("You fold.", FALSE, players[better]->ch, 0, 0, TO_CHAR);

  players[better]->hand[0]=NULL;
  players[better]->hand[1]=NULL;

  if(playerHandCount() == 1){
    // 1 player left, win by default
    showdown(players[firstPlayer()]->ch);
  } else if(nextBetter(better)!=-1){
    better=nextBetter(better);

    act("The bet moves to $n.", FALSE, players[better]->ch, 0, 0, TO_ROOM);
    players[better]->ch->sendTo(COLOR_BASIC, "You can <c>raise<1>, <c>fold<1> or <c>call<1>.\n\r");
  } else {
    advanceRound(players[better]->ch);
  }
}


void HoldemGame::Bet(TBeing *ch, const sstring &arg)
{
  const Card *card;
  sstring coin_str;
  TObj *chip;
  int i;

  if(ch->isImmortal() && arg=="reset"){
    for(i=0;i<MAX_HOLDEM_PLAYERS;++i){
      delete players[i];
      players[i]=NULL;
    }
    for(i=0;i<5;++i)
      community[i]=0;
    better=0;
    bet=0;
    last_bet=0;
    state=STATE_NONE;
    act("$n resets the game.", FALSE, ch, 0, 0, TO_ROOM);
    act("You reset the game.", FALSE, ch, 0, 0, TO_CHAR);
    return;
  }


  if (!ch->checkHoldem())
    return;

  if (!isPlaying(ch)) {
    ch->sendTo("You are not sitting at the table.\n\r");
    return;
  }
  if(playerCount() < 2){
    ch->sendTo("You need at least two players.\n\r");
    return;
  }
  if(state!=STATE_NONE){
    ch->sendTo("Betting has already started.\n\r");
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
  last_bet = chip->objVnum();
  nraises=1;
  ch->doSave(SILENT_YES);
  
  sstring buf;
  buf = fmt("$n bets %s.") % chip->getName();
  act(buf, FALSE, ch, 0, 0, TO_ROOM);
  buf = fmt("You bet %s.") % chip->getName();
  act(buf, FALSE, ch, 0, 0, TO_CHAR);
  
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

  better=firstbetter=button=i;

  // deal cards to everyone
  for(int i=better;i!=-1;i=nextPlayer(i)){
    players[i]->ch->sendTo(COLOR_BASIC, "You are dealt:\n\r");
    act("$n is dealt two cards facedown.", 
	FALSE, players[i]->ch, 0, 0, TO_ROOM);
    
    card=deck.draw();
    players[i]->hand[0]=card;
    players[i]->ch->sendTo(COLOR_BASIC, fmt("%s\n\r") % card->getName());
    
    card=deck.draw();
    players[i]->hand[1]=card;
    players[i]->ch->sendTo(COLOR_BASIC, fmt("%s\n\r") % card->getName());
  }
    
  // move the bet to the next person
  better=nextBetter(better);

  act("The bet moves to $n.", FALSE, players[better]->ch, 0, 0, TO_ROOM);
  players[better]->ch->sendTo(COLOR_BASIC, "You can <c>raise<1>, <c>fold<1> or <c>call<1>.\n\r");
  
  state=STATE_DEAL;
}


