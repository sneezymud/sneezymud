#include "stdsneezy.h"
#include "obj_tool.h"

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


TObj *catch_a_fish(TRoom *rp){
  TObj *fish=NULL;
  int nfresh=23, nmarine=34, nice=7;
  int num=0;
  const int freshfishes[]={13800, 13801, 13802, 13803, 13804, 13805, 13806,
			   13807, 13816, 13817, 13818, 13819, 13820, 13821,
			   13822, 13823, 13824, 13896, 617, 620, 621, 622,
                           13814};
  const int marinefishes[]={13808, 13809, 13810, 13811, 13812, 13813,
			    13815, 13825, 13826, 13827, 13828, 13829, 13830,
			    13831, 13832, 13833, 13834, 13835, 13836, 13837,
			    13838, 13839, 13840, 13897, 607, 608, 609, 610,
                            611, 612, 613, 614, 615, 616};
  const int icefishes[]={13875, 13876, 13877, 13878, 13879, 618, 619};
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

  
  bool adjustsize=true;
  if(rp->getSectorType() == SECT_ICEFLOW){
    num=::number(0,nmarine+nice-1);
    if(num<nice)
      fish=read_object(icefishes[num], VIRTUAL);
    else
      fish=read_object(marinefishes[num-nice], VIRTUAL);
  } else if(rp->isOceanSector()){
    if(!::number(0,nmarine)){
      fish=read_object(12445, VIRTUAL); // some random crap item
      adjustsize=false;
    } else {
      fish=read_object(marinefishes[::number(0,nmarine-1)], VIRTUAL);
    }
  } else { // if(rp->isRiverSector()){  // river or pond or lake or whatever
    fish=read_object(freshfishes[::number(0,nfresh-1)], VIRTUAL);
  }

  if(adjustsize){
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


	  buf = fmt("You bait %s with $p.") % pole->shortDescr;
	  act(buf, FALSE, ch, bait, 0, TO_CHAR);

	  buf = fmt("$n baits %s with $p.") % pole->shortDescr;
          act(buf, TRUE, ch, bait, 0, TO_ROOM);
          ch->task->timeLeft--;
          break;
	case 1:
          act("You cast your line out.",
              FALSE, ch, NULL, 0, TO_CHAR);
	  act("$n casts $s line out.",
              TRUE, ch, NULL, 0, TO_ROOM);
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
	    
	    gain_exp(ch, exp, -1);

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

void handleFishRespawning()
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
