#include "monster.h"
#include "obj_casino_chip.h"
#include "games.h"

void deleteChips(TMonster *me)
{
  std::vector <TThing *> chipl;
  TCasinoChip *chip;

  for(StuffIter it=me->stuff.begin();it!=me->stuff.end();++it){
    if((chip=dynamic_cast<TCasinoChip *>(*it))){
      chipl.push_back(chip);
    }
  }

  for(unsigned int i=0;i<chipl.size();++i){
    (*chipl[i])--;
    delete chipl[i];
  }
}


struct holdemPlayerInfo {
  sstring name;
  bool enabled;
  int chip;
  bool free_chips;
};

void holdemStatus(TMonster *me, holdemPlayerInfo *hpi)
{
  me->doEmote("suddenly snaps to rigid attention.");
  me->doSay("Name        : %s", hpi->name.c_str());
  me->doSay("Enabled     : %s", hpi->enabled?"<G>YES<1>":"<R>NO<1>");
  me->doSay("Chip        : %s", obj_index[real_object(hpi->chip)].short_desc);
  me->doSay("All systems : <G>NOMINAL<1>");
  me->doEmote("resumes a relaxed pose.");
}

int holdemPlayer(TBeing *ch, cmdTypeT cmd, const char *argument, TMonster *me, TObj *)
{
  holdemPlayerInfo *hpi;
  sstring arg=sstring(argument).lower(), buf;
  TObj *obj;

  if(!me->act_ptr) {
    if(!(me->act_ptr = new holdemPlayerInfo)) {
     vlogf(LOG_BUG, "failed new of holdem player.");
     return false;
    }
    hpi = static_cast<holdemPlayerInfo *>(me->act_ptr);    
    hpi->name="gambler2000";
    hpi->enabled=false;
    hpi->chip=CHIP_100;
    hpi->free_chips=true;
  } else {
    hpi = static_cast<holdemPlayerInfo *>(me->act_ptr);
  }
  
  arg=one_argument(arg, buf);
  
  if(cmd == CMD_SAY2 && ch->isImmortal() && buf==hpi->name){
    if(arg=="power up"){
      hpi->enabled=true;
      me->doSay("Gambler 2000 powering up!");
      holdemStatus(me, hpi);
    } else if(arg=="status"){
      holdemStatus(me, hpi);
    } else if(arg=="power down"){
      me->doSay("Gambler 2000 powering down!");
      delete static_cast<holdemPlayerInfo *>(me->act_ptr);
      me->act_ptr = NULL;
    } else if(arg.find("name ", 0)!=sstring::npos){
      arg=one_argument(arg, buf);

      buf=me->name;
      if(buf.find(hpi->name)!=sstring::npos){
	buf.replace(buf.find(hpi->name), hpi->name.length(), arg);
      } else {
	buf += " ";
	buf += arg;
      }

      me->swapToStrung();
      delete me->name;
      me->name=mud_str_dup(buf);


      hpi->name=arg;
      me->doSay("Gambler2000 will now bust a dope rhyme.");
      me->doSay("My name is... huh?");
      me->doSay("My name is... what?");
      me->doSay("My name is %s.", hpi->name.c_str());
      

      
    } else if(arg.find("chip ", 0)!=sstring::npos){
      arg=one_argument(arg, buf);
      if(convertTo<int>(arg)==0)
	return false;
      hpi->chip=convertTo<int>(arg);

      if(real_object(hpi->chip)==-1){
	me->doSay("I can't find that chip.");
	hpi->chip=CHIP_100;
      } else {
	me->doSay("All your %s are belong to Gambler2000.",
		  obj_index[real_object(hpi->chip)].short_desc);
      }
    } else if(arg=="freechips"){
      if(hpi->free_chips){
	hpi->free_chips=false;
	me->doSay("There ain't no such thing as a free chip.");

	int tcount=0;
	for(StuffIter it=me->stuff.begin();it!=me->stuff.end();++it){
	  if((obj=dynamic_cast<TObj *>(*it))){
	    if(obj->objVnum() == hpi->chip)
	      tcount++;
	  }
	}

	me->doSay("I have %s [%i].",
		  obj_index[real_object(hpi->chip)].short_desc, tcount);
      } else {
	hpi->free_chips=true;
	me->doSay("I will now pull chips out of my ass.");
      }
    }
    return true;
  }

  if (cmd == CMD_GENERIC_DESTROYED) {
    delete static_cast<holdemPlayerInfo *>(me->act_ptr);
    me->act_ptr = NULL;
    return FALSE;
  }

  if(cmd != CMD_GENERIC_QUICK_PULSE || !hpi->enabled)
    return false;

  if(hpi->free_chips)
    deleteChips(me);

  if (!me->checkHoldem(true))
    return false;

  if(!gHoldem.isPlaying(me)){
    gHoldem.enter(me);
    return false;
  }


  HoldemPlayer *hp=gHoldem.getPlayer(me->name);

  if(gHoldem.getBetter() != hp)
    return false;



  int handval=gHoldem.handValue(hp);
  TObj *chip;
  std::vector <TObj *> chipl;

  if(gHoldem.getLastBet()){
    if(gHoldem.getLastBet() != hpi->chip){
      me->doSay("Sorry, I only play with '%s' right now.", 
		obj_index[real_object(hpi->chip)].short_desc);
      gHoldem.fold(me);
      return false;
    }

    if(hpi->free_chips){
      for(int i=0;i<gHoldem.getNRaises()+1;++i){
	chip=read_object(gHoldem.getLastBet(), VIRTUAL);
	*me += *chip;
	chipl.push_back(chip);
      }
    }
  } else {
    return false;
  }


  switch(gHoldem.getState()){
    case STATE_NONE:
      break;
    case STATE_DEAL:
      // if someone raised before the flop
      if(gHoldem.getNRaises() > 1){
	// if we have a couple of high cards, we'll call once in awhile
	if((hp->hand[0]->getValAceHi() >= 10) &&
	   (hp->hand[1]->getValAceHi() >= 10) && !::number(0,2)){
	  gHoldem.call(me);
	  return true;
	} else if(handval > 15){
	  // or if we have a pair we'll call
	  gHoldem.call(me);
	  return true;
	} else {
	  // but otherwise fold
	  gHoldem.fold(me);
	}
      }
      
      // by default we call to see the flop
      gHoldem.call(me);
      return true;
      break;
    case STATE_FLOP:
      if(handval > 30 && ::number(0,1)){
	gHoldem.raise(me, "");
	gHoldem.call(me); // sometimes raise fails, if he's broke etc
	return true;
      } else if(handval > 15 || !::number(0,3)){
	gHoldem.call(me);
	return true;
      }
      
      // no hand, may as well fold
      gHoldem.fold(me);
      return true;
      break;
    case STATE_TURN:
    case STATE_RIVER:
      if(handval > 45){
	gHoldem.raise(me, "");
	gHoldem.call(me); // sometimes raise fails, if he's broke etc
      } else if(handval > 15){
	gHoldem.call(me);
      } else {
	gHoldem.fold(me);
      }

      if(hpi->free_chips)
	deleteChips(me);

      return true;
      break;
  }

  if(hpi->free_chips){
    for(unsigned int i=0;i<chipl.size();++i){
      (*chipl[i])--;
      delete chipl[i];
    }
  }
  
  gHoldem.fold(me);
  return true;
}


