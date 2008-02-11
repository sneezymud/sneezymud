#include "stdsneezy.h"
#include "obj_tool.h"
#include "process.h"
#include "database.h"
#include "obj_food.h"

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
    if (!getMyRace()->hasTalent(TALENT_FISHEATER)) {
      sendTo("You can't fish underwater!\n\r");
      return;
    }
  } else if(!rp->isWaterSector()){
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
//  f.push_back(13811); duplicate flounder! see 13829
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


  if(::number(0, 24)){
    num=::number(0, fishlist.size()-1);
    fish=read_object(fishlist[num], VIRTUAL);
    
    if(num != 12445){ // don't do this for seaweed
      fish->setWeight(fish->getWeight()*weightmod);
      fish->setVolume((int)(fish->getWeight()*200));
      if (dynamic_cast<TFood*>(fish))
        dynamic_cast<TFood*>(fish)->setFoodFlags(FOOD_FISHED);
    }
    
    rp->setFished(rp->getFished()+1);
    
    if (mRoomsFished.find(rp->number) == mRoomsFished.end())
      mRoomsFished[rp->number] = true;
  } else {
    // grab a random item from room 19024
    TRoom *briny_deep=real_roomp(19024);
    int count=0;

    for(TThing *t=briny_deep->getStuff();t;t=t->nextThing)
      ++count;

    count=::number(0,count-1);

    for(TThing *t=briny_deep->getStuff();t;t=t->nextThing){
      if(!count-- && (fish=dynamic_cast<TObj *>(t))){
	sendrpf(briny_deep, "A fishing line with a hook attached descends from above and pulls up %s!\n\r", fish->getName());
	--(*fish);
	break;
      }
    }
  }

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
  bool awesomeFisher = ch->getMyRace()->hasTalent(TALENT_FISHEATER);
  wearSlotT fishHand = ch->getPrimaryHold();

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

  if(!bait && !awesomeFisher){
    ch->sendTo("You need to have some bait to fish.\n\r");
    ch->stopTask();
    return FALSE;
  }
  
  // find our pole here
  if(!awesomeFisher && ((!(tpole=ch->heldInPrimHand()) && !(tpole = ch->heldInSecHand())) ||
     !isname("fishingpole", tpole->name))){
    ch->sendTo("You need to hold a fishing pole to fish!\n\r");
    ch->stopTask();
    return FALSE;
  }
  if(!awesomeFisher && !(pole=dynamic_cast<TObj *>(tpole))){
    vlogf(LOG_BUG, "Hmm got a fishing pole that isn't a TObj");
    ch->sendTo("You need to hold a fishing pole to fish!\n\r");
    ch->stopTask();
    return FALSE;
  }
  if (awesomeFisher && NULL == pole && 
    !ch->canUseHand(true) && (!ch->isAmbidextrous() || ch->bothHandsHurt()))
  {
    ch->sendTo(fmt("Fish with what?  Your %s is too damaged.\n\r") % ch->describeBodySlot(ch->getPrimaryHand()));
    ch->stopTask();
    return FALSE;
  }
  if (awesomeFisher && NULL == pole)
  {
    if (ch->equipment[ch->getPrimaryHold()] && ch->isAmbidextrous())
      fishHand = ch->getSecondaryHold();

    if (ch->equipment[fishHand])
    {
      ch->sendTo("Your primary hand must be free to fish!\n\r");
      ch->stopTask();
      return FALSE;
    }
  }

  /*
    do generic checks here
   */

  if (rp && rp->isUnderwaterSector()) {
    if (!awesomeFisher) {
      ch->sendTo("You can't fish underwater!\n\r");
      ch->stopTask();
      return FALSE;
    }
  } else if(rp && !rp->isWaterSector()){
    ch->sendTo("You can't fish on land!\n\r");
    ch->stopTask();
    return FALSE;
  }

  if (ch->task && ch->task->timeLeft < 0){
    if (awesomeFisher)
      ch->sendTo("You give up and stop fishing.\n\r");
    else
      ch->sendTo("You pack up and stop fishing.\n\r");
    ch->stopTask();
    return FALSE;
  }


  switch (cmd) {
    case CMD_TASK_CONTINUE:
      ch->task->calcNextUpdate(pulse, PULSE_MOBACT * 5);

      switch (ch->task->timeLeft) {
	case 2:
    {
      // check for out of bait
      if (!awesomeFisher)
      {
        bait->addToToolUses(-1);
        if (bait->getToolUses() <= 0) {
          act("Oops, you're out of bait.", FALSE, ch, NULL, 0, TO_CHAR);
          act("$n looks startled as $e realizes that $e is out of bait.", FALSE, ch, NULL, 0, TO_ROOM);
          ch->stopTask();
          delete bait;
          return FALSE;
        }
      }

      ch->task->timeLeft--;

      sstring toChar, toRoom;
      bool fishlore = ch->bSuccess(SKILL_FISHLORE) && ::number(0,99)<20;
      if (ch->isPlayerAction(PLR_BRIEF))
        break;

      if (!awesomeFisher && fishlore) {
        toChar = fmt("You <c>smoothly<1> bait %s with $p in one fluid motion.") % pole->shortDescr;
        toRoom = fmt("$n <c>smoothly<1> baits %s with $p in one fluid motion.") % pole->shortDescr;
      } else if (awesomeFisher) {
        toChar = "You scan the surrounding water for prey.";
        toRoom = "$n scans the water looking for prey.";
      } else {
        toChar = fmt("You bait %s with $p.") % pole->shortDescr;
        toRoom = fmt("$n baits %s with $p.") % pole->shortDescr;
      }

      act(toChar, FALSE, ch, bait, 0, TO_CHAR);
      act(toRoom, TRUE, ch, bait, 0, TO_ROOM);
      break;
    }
	case 1:
    {
      ch->task->timeLeft--;

      sstring toChar, toRoom;
      bool fishlore = ch->bSuccess(SKILL_FISHLORE) && ::number(0,99)<20;
      if (ch->isPlayerAction(PLR_BRIEF))
        break;

      if (!awesomeFisher && fishlore) {
        toChar = "You spot a <c>ripple in the water<1> and cast your line right at it.";
        toRoom = "$n casts $s line out right at a <c>ripple in the water<1>.";
      } else if (awesomeFisher && fishlore) {
        toChar = "You spot a <c>ripple in the water<1> and reach towards it.";
        toRoom = "$n reaches toward a <c>ripple in the water<1>.";
      } else if (awesomeFisher) {
        toChar = fmt("You search around the water with your %s.") % ch->describeBodySlot(fishHand);
        toRoom = fmt("$n gropes around the water with $s %s.") % ch->describeBodySlot(fishHand);
      } else {
        toChar = "You cast your line out.";
        toRoom = "$n casts $s line out.";
      }

      act(toChar, FALSE, ch, NULL, 0, TO_CHAR);
      act(toRoom, TRUE, ch, NULL, 0, TO_ROOM);
      break;
    }
	case 0:
    if (awesomeFisher) {
      baitchance= ch->plotStat(STAT_CURRENT, STAT_PER, 0, 5, 2) + ch->plotStat(STAT_CURRENT, STAT_KAR, 0, 5, 3);
      polechance= ch->GetMaxLevel();
    } else {
      baitchance=(int)(((float)((float)(bait->obj_flags.cost*2)/(float)baitmax))*25);
      polechance=(int)(((float)((float)(pole->obj_flags.cost*2)/(float)polemax))*25);
    }
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

      if (awesomeFisher)
      {
        act("You snatch up $p!", FALSE, ch, fish, 0, TO_CHAR);
        act("$n snatches $p!", TRUE, ch, fish, 0, TO_ROOM);
      }
      else
      {
        act("You reel in $p!", FALSE, ch, fish, 0, TO_CHAR);
        act("$n reels in $p!", TRUE, ch, fish, 0, TO_ROOM);
      }

	  } else {
	    if(fish)
	      delete fish;

	    act("You didn't catch anything.", FALSE, ch, NULL, 0, TO_CHAR);
	    act("$n doesn't catch anything.", TRUE, ch, NULL, 0, TO_ROOM);

	    if(rp->getFished()>10 && 
	       ch->bSuccess(SKILL_FISHLORE) && ::number(0,99)<20){
	      act("<c>This place seems all fished out.<1>",
		  FALSE, ch, NULL, 0, TO_CHAR);
	    }
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
      if(ch->bSuccess(SKILL_FISHLORE) && ::number(0,99)<20){
	ch->sendTo(COLOR_BASIC, "You <c>focus your fishlore<1> and maintain your concentration.\n\r");
	ch->sendTo("You continue fishing while fighting.\n\r");
      } else {
	ch->sendTo("You have not yet mastered the art of fighting while fishing.\n\r");
	ch->stopTask();
      }
      break;
    default:
      if (cmd < MAX_CMD_LIST)
        warn_busy(ch);
      break;                    // eat the command
  }
  return TRUE;
}

void initialize_fish_records()
{
  // put a row into the fishlargest table for any new, uncaught fish
  TDatabase db(DB_SNEEZY);
  unsigned int step;
  vector <int> fishious;
  
  fishious = freshfishes();
  for (step = 0; step < fishious.size(); step++) {
    db.query("select vnum from fishlargest where vnum = %i", fishious[step]);
    if (!db.isResults())
      db.query("insert into fishlargest (name, vnum, weight) select 'no one', %i, 0.0", fishious[step]);
  }
  fishious.clear();
  
  fishious = marinefishes();
  for (step = 0; step < fishious.size(); step++) {
    db.query("select vnum from fishlargest where vnum = %i", fishious[step]);
    if (!db.isResults())
      db.query("insert into fishlargest (name, vnum, weight) select 'no one', %i, 0.0", fishious[step]);
  }
  fishious.clear();
  
  fishious = icefishes();
  for (step = 0; step < fishious.size(); step++) {
    db.query("select vnum from fishlargest where vnum = %i", fishious[step]);
    if (!db.isResults())
      db.query("insert into fishlargest (name, vnum, weight) select 'no one', %i, 0.0", fishious[step]);
  }
  fishious.clear();
  
  fishious = fishworldfishes();
  for (step = 0; step < fishious.size(); step++) {
    db.query("select vnum from fishlargest where vnum = %i", fishious[step]);
    if (!db.isResults())
      db.query("insert into fishlargest (name, vnum, weight) select 'no one', %i, 0.0", fishious[step]);
  }
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


