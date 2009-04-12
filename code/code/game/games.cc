#include "room.h"
#include "monster.h"
#include "games.h"
#include "obj_casino_chip.h"
#include "handler.h"

void payout(TBeing *ch, int talens, int chip_vnum)
{
  TObj *chip;
  sstring buf;
  std::map <sstring, int> chipout;
  std::map <sstring, int>::iterator iter;

  while(talens>0){
    if(!chip_vnum){
      if(talens >= 1000000){
	chip=read_object(CHIP_1000000, VIRTUAL);
      } else if(talens >= 500000){
	chip=read_object(CHIP_500000, VIRTUAL);
      } else if(talens >= 100000){
	chip=read_object(CHIP_100000, VIRTUAL);
      } else if(talens >= 50000){
	chip=read_object(CHIP_50000, VIRTUAL);
      } else if(talens >= 10000){
	chip=read_object(CHIP_10000, VIRTUAL);
      } else if(talens >= 5000){
	chip=read_object(CHIP_5000, VIRTUAL);
      } else if(talens >= 1000){
	chip=read_object(CHIP_1000, VIRTUAL);
      } else if(talens >= 500){
	chip=read_object(CHIP_500, VIRTUAL);
      } else if(talens >= 100){
	chip=read_object(CHIP_100, VIRTUAL);
      } else {
	buf = format("You receive %i talens.") % talens;
	act(buf, TRUE, ch, 0, 0, TO_CHAR);
	buf = format("$n receives %i talens.") % talens;
	act(buf, TRUE, ch, 0, 0, TO_ROOM);
	ch->addToMoney(talens, GOLD_GAMBLE);
	break;
      }
    } else {
      chip=read_object(chip_vnum, VIRTUAL);

      if((talens-chip->obj_flags.cost) < 0){
	delete chip;
	chip_vnum=0;
	continue;
      }
    }

    if(!chip){
      vlogf(LOG_BUG, "couldn't load chip in payout");
      return;
    }

    talens -= chip->obj_flags.cost;

    chipout[chip->getName()]++;

    *ch += *chip;
  }


  for(iter=chipout.begin();iter!=chipout.end();++iter){
    if((*iter).second > 1){
      buf = format("You receive %s. [%i]") %
	       (*iter).first % (*iter).second;
      act(buf, FALSE, ch, 0, 0, TO_CHAR);
      buf = format("$n receives %s. [%i]") %
	       (*iter).first % (*iter).second;
      act(buf, FALSE, ch, 0, 0, TO_ROOM);
    } else {
      buf = format("You receive %s.") % (*iter).first;
      act(buf, FALSE, ch, 0, 0, TO_CHAR);
      buf = format("$n receives %s.") % (*iter).first;
      act(buf, FALSE, ch, 0, 0, TO_ROOM);
    }
  }
}


TObj *find_chip(TBeing *ch, const int &chip)
{
  TObj *o;

  for(StuffIter it=ch->stuff.begin();it!=ch->stuff.end();++it)
    if((o=dynamic_cast<TObj *>(*it)) && dynamic_cast<TCasinoChip *>(o) &&
       o->objVnum()==chip)
      return o;
  
  return NULL;
}


TObj *find_chip(TBeing *ch, const sstring &coin_str)
{
  TObj *chip;

  if(!(chip=generic_find_obj(coin_str, FIND_OBJ_INV, ch)) ||
     !(dynamic_cast<TCasinoChip *>(chip))){
    return NULL;
  }
  
  return chip;
}

void react_gambler_won(TBeing *ch, TMonster *tm)
{
  switch(::number(0,3)){
    case 0:
      tm->doSay("Keep it going!");
      break;
    case 1:
      tm->doAction("", CMD_APPLAUD);
      break;
    case 2:
      tm->doSay("Nice win!");
      break;
    case 3:
      tm->doSay("Hey can I borrow some of those chips?");
      tm->doAction(ch->name, CMD_GRIN);
      break;
  }
}


void react_gambler_lost(TBeing *ch, TMonster *tm)
{
  switch(tm->mobVnum()){
    case 2364: // pit boss
      tm->doSay("Please enjoy your stay at the casino.");
      payout(ch, 100);
      return;
    case 2365: // waitress
      tm->doSay("Please enjoy your stay at the casino.");
      *ch += *(read_object(3526, VIRTUAL)); // margarita
      return;
    case 2366: // bartender
      tm->doSay("Please enjoy your stay at the casino.");
      *ch += *(read_object(3503, VIRTUAL)); // chips
      return;
  }      


  switch(::number(0,5)){
    case 0:
      tm->doSay("Ouch!  Blood on the floor!");
      break;
    case 1:
      tm->doAction("", CMD_WINCE);
      break;
    case 2:
      tm->doSay("Small setback!  Keep it going!");
      break;
    case 3:
      tm->doSay("I think the table is rigged.");
      break;
    case 4:
      tm->doAction("", CMD_FROWN);
      break;
    case 5:
      tm->doAction(ch->name, CMD_COMFORT);
      break;
  }
}

void react_gambler_bet(TBeing *ch, TMonster *tm)
{
  switch(::number(0,4)){
    case 0:
      tm->doSay("Go for it!");
      break;
    case 1:
      tm->doSay("Come on, don't be cheap.  Bet the farm on the next one!");
      break;
    case 2:
      tm->doSay("Maybe someone should cut you off.");
      break;
    case 3:
      tm->doSay("I've got a good feeling about this one.");
      break;
    case 4:
      tm->doSay("You're just throwing money away.  There's no way you'll win.");
      break;

  }
}

void react_gambler_hilo_bet(TBeing *ch, TMonster *tm)
{
  switch(::number(0,5)){
    case 0:
      tm->doSay("Bet hi!");
      break;
    case 1:
      tm->doSay("Bet lo!");
      break;
    case 2:
      tm->doSay("Cash out!  Cash out!");
      break;
    case 3:
    case 4:
    case 5:
      react_gambler_bet(ch, tm);
      break;
  }
}

void react_gambler_blackjack_bet(TBeing *ch, TMonster *tm)
{
  switch(::number(0,5)){
    case 0:
      tm->doSay("Hit!");
      break;
    case 1:
      tm->doSay("Stay!");
      break;
    case 2:
    case 3:
    case 4:
    case 5:
      react_gambler_bet(ch, tm);
      break;
  }
}

void observerReaction(TBeing *ch, int what)
{
  TMonster *tm;



  for(StuffIter it=ch->roomp->stuff.begin();it!=ch->roomp->stuff.end();++it){
    if((tm=dynamic_cast<TMonster *>(*it)) &&
       isname("gambler", tm->name) && !::number(0,9)){
      switch(what){
	case GAMBLER_WON:
	  react_gambler_won(ch, tm);
	  break;
	case GAMBLER_LOST:
	  react_gambler_lost(ch, tm);
	  break;
	case GAMBLER_BET:
	  react_gambler_bet(ch, tm);
	  break;
	case GAMBLER_HILO_BET:
	  react_gambler_hilo_bet(ch, tm);
	  break;
	case GAMBLER_BLACKJACK_BET:
	  react_gambler_blackjack_bet(ch, tm);
	  break;
      }
    }
  }
}









