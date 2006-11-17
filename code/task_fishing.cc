#include "stdsneezy.h"
#include "obj_tool.h"
#include "process.h"

map <int, bool> mRoomsFished;

void TBeing::doFish(sstring direction){
  TRoom *rp;
  roomDirData *exitp;
  const int ROOM_FISHING_SHACK=31818;

  if(!(exitp=exitDir(getDirFromChar(direction))) || direction.empty()){
    rp=roomp;
  } else {
    if(!exitp->to_room || !(rp = real_roomp(exitp->to_room))){
      rp=roomp;      
    }
  }

  if(rp->isUnderwaterSector()){
    sendTo("You can't fish underwater!\n\r");
    return;
  }
  if(!rp->isWaterSector()){
    sendTo("You can't fish on land!\n\r");
    return;
  }


  if(task){
    stopTask();
  }

  sendTo("You start fishing.\n\r");

  if(getCond(DRUNK) > 10 && !::number(0,3) &&
     (inRoom() < 31800 || inRoom() > 31899)){

    sendTo("All of this drunken fishing has caused you to pass out.\n\r");
    sendTo("Strange things begin running through your mind...\n\r");

    setPosition(POSITION_SLEEPING);

    TRoom *room = real_roomp(ROOM_FISHING_SHACK);
    --(*this);
    *room += *this;    
  }

  start_task(this, NULL, rp, TASK_FISHING, "", 2, inRoom(), 0, 0, 5);
}

vector <int> freshfishes()
{
  vector <int> f;

  f.push_back(13800);
  f.push_back(13801);
  f.push_back(13802);
  f.push_back(13803);
  f.push_back(13804);
  f.push_back(13805);
  f.push_back(13806);
  f.push_back(13807);
  f.push_back(13816);
  f.push_back(13817);
  f.push_back(13818);
  f.push_back(13819);
  f.push_back(13820);
  f.push_back(13821);
  f.push_back(13822);
  f.push_back(13823);
  f.push_back(13824);
  f.push_back(13896);
  f.push_back(617);
  f.push_back(620);
  f.push_back(621);
  f.push_back(622);
  f.push_back(13814);

  return f;
}

vector <int> marinefishes()
{
  vector <int> f;

  f.push_back(12445);
  f.push_back(13808);
  f.push_back(13809);
  f.push_back(13810);
  f.push_back(13811);
  f.push_back(13812);
  f.push_back(13813);
  f.push_back(13815);
  f.push_back(13825);
  f.push_back(13826);
  f.push_back(13827);
  f.push_back(13828);
  f.push_back(13829);
  f.push_back(13830);
  f.push_back(13831);
  f.push_back(13832);
  f.push_back(13833);
  f.push_back(13834);
  f.push_back(13835);
  f.push_back(13836);
  f.push_back(13837);
  f.push_back(13838);
  f.push_back(13839);
  f.push_back(13840);
  f.push_back(13897);
  f.push_back(607);
  f.push_back(608);
  f.push_back(609);
  f.push_back(610);
  f.push_back(611);
  f.push_back(612);
  f.push_back(613);
  f.push_back(614);
  f.push_back(615);
  f.push_back(616);

  return f;
}

vector <int> icefishes()
{
  vector <int> f;
  f.push_back(13875);
  f.push_back(13876);
  f.push_back(13877);
  f.push_back(13878);
  f.push_back(13879);
  f.push_back(618);
  f.push_back(619);

  return f;
}


vector <int> fishworldfishes()
{
  vector <int> f;
  f.push_back(31870);
  return f;
}


TObj *catch_a_fish(TRoom *rp)
{
  TObj *fish=NULL;
  unsigned int num=0;
  vector <int> freshfish=freshfishes();
  vector <int> marinefish=marinefishes();
  vector <int> icefish=icefishes();
  vector <int> fishworld=fishworldfishes();
  float weightmod=(((float)(::number(0,100))-50.0)/100.0)+1.0;  // plus or minus 30%

  //  vlogf(LOG_PEEL, fmt("weightmod=%f") %  weightmod);

  if(!::number(0,99)){  // 1 in 100
    // big one
    weightmod = 2 + ((float)::number(0,100)/100.0); // 2-3
    
    if(!::number(0,99)){ // 1 in 10000
      // real big one
      weightmod = 3 + ((float)::number(0,100)/100.0); // 3-4

      if(!::number(0,99)){ // 1 in 1000000
	// REAL big one
	weightmod = 4 + ((float)::number(0,100)/100.0); // 4-5

	if(!::number(0,99)){ // 1 in 100000000
	  // freak of nature
	  weightmod = 5 + ((float)::number(0,500)/100.0); // 5-10
	}
      }
    }
  }

  vector <int> fishlist;

  if(rp->number >= 31800 && rp->number <= 31899){ // fish world
    for(unsigned int i=0;i<fishworld.size();++i)
      fishlist.push_back(fishworld[i]);
  }

  if(rp->getSectorType() == SECT_ICEFLOW){
    for(unsigned int i=0;i<icefish.size();++i)
      fishlist.push_back(icefish[i]);    
    for(unsigned int i=0;i<marinefish.size();++i)
      fishlist.push_back(marinefish[i]);    
  } else if(rp->isOceanSector()){
    for(unsigned int i=0;i<marinefish.size();++i)
      fishlist.push_back(marinefish[i]);    
  } else {
    for(unsigned int i=0;i<freshfish.size();++i)
      fishlist.push_back(freshfish[i]);    
  }

  num=::number(0, fishlist.size()-1);
  fish=read_object(fishlist[num], VIRTUAL);

  if(num != 12445){ // don't do this for seaweed
    fish->setWeight(fish->getWeight()*weightmod);
    fish->setVolume((int)(fish->getWeight()*200));
  }

  rp->setFished(rp->getFished()+1);

  if (mRoomsFished.find(rp->number) == mRoomsFished.end())
    mRoomsFished[rp->number] = true;

  return fish;
}


TThing *findBait(TThing *stuff){
  TThing *tt;
  TTool *bait;
  TThing *ret;

  if(!stuff) 
    return NULL;

  for(tt=stuff;tt;tt=tt->nextThing){
    if(tt && (bait=dynamic_cast<TTool *>(tt)) &&
       (bait->getToolType() == TOOL_FISHINGBAIT))
      return tt;

    if(tt && tt->getStuff() && (ret=findBait(tt->getStuff())))
      return ret;
  }

  return NULL;
}




int task_fishing(TBeing *ch, cmdTypeT cmd, const char *, int pulse, TRoom *rp, TObj *)
{
  TTool *bait=NULL;
  TThing *t=NULL, *tpole=NULL;
  sstring buf;
  TObj *fish=NULL, *pole=NULL;
  int baitmax=1000, baitchance=0;
  int polemax=5000, polechance=0;
  int catchchance=0;

  if(ch->utilityTaskCommand(cmd) || ch->nobrainerTaskCommand(cmd))
    return FALSE;

  // basic tasky safechecking
  if (ch->isLinkdead() || (ch->in_room != ch->task->wasInRoom)){
    act("You cease fishing.",
        FALSE, ch, 0, 0, TO_CHAR);
    act("$n stops fishing.",
        TRUE, ch, 0, 0, TO_ROOM);
    ch->stopTask();
    return FALSE; // returning FALSE lets command be interpreted
  }

  TThing *ss=ch->getStuff();

  // find our bait here
  t=findBait(ss);
  
  int m=WEAR_NOWHERE;
  while(!t && m<MAX_WEAR){
    ++m;
    t=findBait(ch->equipment[m]);
  }


  bait=dynamic_cast<TTool *>(t);

  if(!bait){
    ch->sendTo("You need to have some bait to fish.\n\r");
    ch->stopTask();
    return FALSE;
  }
  
  // find our pole here
  if((!(tpole=ch->heldInPrimHand()) && !(tpole = ch->heldInSecHand())) ||
     !isname("fishingpole", tpole->name)){
    ch->sendTo("You need to hold a fishing pole to fish!\n\r");
    ch->stopTask();
    return FALSE;
  }
  if(!(pole=dynamic_cast<TObj *>(tpole))){
    vlogf(LOG_BUG, "Hmm got a fishing pole that isn't a TObj");
    ch->sendTo("You need to hold a fishing pole to fish!\n\r");
    ch->stopTask();
    return FALSE;
  }



  /*
    do generic checks here
   */


  if(rp && !rp->isWaterSector()){
    ch->sendTo("You can't fish on land!\n\r");
    ch->stopTask();
    return FALSE;
  }

  if (ch->task && ch->task->timeLeft < 0){
    ch->sendTo("You pack up and stop fishing.\n\r");
    ch->stopTask();
    return FALSE;
  }


  switch (cmd) {
    case CMD_TASK_CONTINUE:
      ch->task->calcNextUpdate(pulse, PULSE_MOBACT * 5);

      switch (ch->task->timeLeft) {
	case 2:
	  // check for out of bait
	  bait->addToToolUses(-1);
	  if (bait->getToolUses() <= 0) {
	    act("Oops, you're out of bait.",
		FALSE, ch, NULL, 0, TO_CHAR);
	    act("$n looks startled as $e realizes that $e is out of bait.",
		FALSE, ch, NULL, 0, TO_ROOM);
	    ch->stopTask();
	    delete bait;
	    return FALSE;
	  }

	  if(!ch->isPlayerAction(PLR_BRIEF)){
	    buf = fmt("You bait %s with $p.") % pole->shortDescr;
	    act(buf, FALSE, ch, bait, 0, TO_CHAR);
	    
	    buf = fmt("$n baits %s with $p.") % pole->shortDescr;
	    act(buf, TRUE, ch, bait, 0, TO_ROOM);
	  }
          ch->task->timeLeft--;
          break;
	case 1:
	  if(!ch->isPlayerAction(PLR_BRIEF)){
	    act("You cast your line out.",
		FALSE, ch, NULL, 0, TO_CHAR);
	    act("$n casts $s line out.",
		TRUE, ch, NULL, 0, TO_ROOM);
	  }
          ch->task->timeLeft--;
          break;
	case 0:
	  baitchance=(int)(((float)((float)(bait->obj_flags.cost*2)/(float)baitmax))*25);
	  polechance=(int)(((float)((float)(pole->obj_flags.cost*2)/(float)polemax))*25);
	  catchchance=::number(1,100);
	  

	  //	  vlogf(LOG_PEEL, fmt("fishing: baitcost=%i, bait=%i, pole=%i, catch=%i") % 
	  //	bait->obj_flags.cost % baitchance % polechance % catchchance);
  
	  if((ch->bSuccess(SKILL_FISHING) ||
	      (!ch->doesKnowSkill(SKILL_FISHING) && !::number(0,99))) &&
	     (catchchance<(baitchance+polechance)) &&
	     (fish=catch_a_fish(rp)) &&
	     (::number(5,10) > rp->getFished())){
            *ch += *fish;

	    //	    gain_exp(ch, fish->getWeight() * 10, -1);
	    int lvl=ch->GetMaxLevel();
	    if(lvl>15)
	      lvl-=15;
	    else
	      lvl=1;

	    // 10% exp variance
	    double exp=mob_exp(lvl);
	    exp *= (1.0+((::number(0,20)-10)/100.0));
	    
	    gain_exp(ch, exp/50, -1);

	    ch->doSave(SILENT_YES);

	    act("You reel in $p!",
		FALSE, ch, fish, 0, TO_CHAR);
	    act("$n reels in $p!",
		TRUE, ch, fish, 0, TO_ROOM);
	  } else {
	    if(fish)
	      delete fish;

	    act("You didn't catch anything.",
		FALSE, ch, NULL, 0, TO_CHAR);
	    act("$n doesn't catch anything.",
		TRUE, ch, NULL, 0, TO_ROOM);

            // Don't reveal to the fisherman that there isn't any more fish
            // in the room.
            /*
	    if(rp->getFished()>10){
	      act("This place seems all fished out.",
		  FALSE, ch, NULL, 0, TO_CHAR);
	    }
            */
	  }
	  ch->stopTask();
          break;
      }
      break;
    case CMD_ABORT:
    case CMD_STOP:
      act("You cease fishing.",
          FALSE, ch, 0, 0, TO_CHAR);
      act("$n stops fishing.",
          TRUE, ch, 0, 0, TO_ROOM);
      ch->stopTask();
      break;
    case CMD_TASK_FIGHTING:
      ch->sendTo("You have not yet mastered the art of fighting while fishing.\n\r");
      ch->stopTask();
      break;
    default:
      if (cmd < MAX_CMD_LIST)
        warn_busy(ch);
      break;                    // eat the command
  }
  return TRUE;
}

// procFishRespawning
procFishRespawning::procFishRespawning(const int &p)
{
  trigger_pulse=p;
  name="procFishRespawning";
}

void procFishRespawning::run(int pulse) const
{
  map <int, bool> ::iterator tIter     = mRoomsFished.begin(),
                             tLastGood = mRoomsFished.begin();

  while (tIter != mRoomsFished.end()) {
    TRoom * tRoom = real_roomp((*tIter).first);

    if (!tRoom) {
      vlogf(LOG_BUG, fmt("handleFishRespawning() handling non-existent room! (%d)") % (*tIter).first);
      continue;
    }

    // Make it only a chance.
    if ((tRoom->getFished() > 0) && !::number(0, 24))
      tRoom->setFished(tRoom->getFished() - 1);

    if (tRoom->getFished() < 1) {
      if (tIter == tLastGood) {
        mRoomsFished.erase(tIter);
        tIter = tLastGood = mRoomsFished.begin();
      } else {
        mRoomsFished.erase(tIter);
        tIter = tLastGood;
      }
    } else
      tLastGood = tIter;

    if (tIter == mRoomsFished.end())
      break;

    ++tIter;
  }
}
