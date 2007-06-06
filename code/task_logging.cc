#include "stdsneezy.h"
#include "process.h"
#include "obj_base_weapon.h"

#define LAST_WOOD_TYPE 0
#define LOGS_PER_ROOM 5

map <int, bool> mRoomsLogsHarvested;
    
void TBeing::doLogging(){

  vector<int>treetypes;
  const int woodtypes[] = { 75, 76, 77, 78, 79, 80, 81, 82, 83, 84, 85,
    86, 87, 88, 0 };
  int i = 0;
  

  if(!roomp->isForestSector()){
    sendTo("Forests are said to be a good place to find trees.  Maybe you should find one?\n\r");
    return;
  }

  if(task){
    stopTask();
  }

  if (!roomp->getTreetype()) {
    while (woodtypes[i] != LAST_WOOD_TYPE) {
      treetypes.push_back(woodtypes[i]);
      i++;
    }
    std::random_shuffle(treetypes.begin(), treetypes.end());
    roomp->setTreetype(treetypes[0]);
  }

  sendTo("You start looking for a tree to chop down.\n\r");

  start_task(this, NULL, roomp, TASK_LOGGING, "", 3, inRoom(), 0, 0, 5);
}


TObj *harvest_a_log(TRoom *rp){
  TObj *log=NULL;
  int log_vnum = rp->getTreetype();
  
  if (rp->getLogsHarvested() > LOGS_PER_ROOM) {
    vlogf(LOG_BUG, "Lumberjack in a treeless room.");
    rp->setLogsHarvested(LOGS_PER_ROOM-1);
  }
  log = read_object(log_vnum, VIRTUAL);

  rp->setLogsHarvested(rp->getLogsHarvested()+1);
  if (mRoomsLogsHarvested.find(rp->number) == mRoomsLogsHarvested.end())
    mRoomsLogsHarvested[rp->number] = true;

  return log;
}


int task_logging(TBeing *ch, cmdTypeT cmd, const char *, int pulse, TRoom *rp, TObj *)
{
  TObj *log=NULL;
  TBaseWeapon *tool=NULL;
  int learning = ch->getSkillValue(SKILL_LOGGING);
  int dam, critnum, log_vnum;
  wearSlotT part_hit;

  if(ch->utilityTaskCommand(cmd) || ch->nobrainerTaskCommand(cmd))
    return FALSE;

  // basic tasky safechecking
  // adding the learning check - we had an arithmetic exception trying to divide...
  if (ch->isLinkdead() || (ch->in_room != ch->task->wasInRoom) || learning == 0){
    act("You cease your deforestation activities.",
        FALSE, ch, 0, 0, TO_CHAR);
    act("$n stops $s deforestation activities.",
        TRUE, ch, 0, 0, TO_ROOM);
    ch->stopTask();
    return FALSE; // returning FALSE lets command be interpreted
  }

  // find our axe or whatever here
  tool = dynamic_cast<TBaseWeapon *>(ch->heldInPrimHand());
  if((!tool || !tool->isSlashWeapon()))
  {
    ch->sendTo("You need to hold a slash weapon in your primary hand to engage in deforestation!\n\r");
    ch->stopTask();
    return TRUE;
  }

  /*
    do generic checks here
   */


  if(rp && !rp->isForestSector()){
    vlogf(LOG_BUG, "Somehow chopping wood in an unforested area.");
    ch->sendTo("Forests are said to be a good place to find trees.  Maybe you should find one?\n\r");
    ch->stopTask();
    return TRUE;
  }

  if (ch->task && ch->task->timeLeft < 0){
    ch->sendTo("You give up on your deforestation activities.\n\r");
    ch->stopTask();
    return TRUE;
  }

  switch (cmd) {
    case CMD_TASK_CONTINUE:
      ch->task->calcNextUpdate(pulse, PULSE_MOBACT * 5);

      switch (ch->task->timeLeft)
      {
        case 3:
          // looking for a tree
          act("You look around for a tree to chop down with your $o.",
            FALSE, ch, tool, 0, TO_CHAR);
          act("$n looks around for something.",
            TRUE, ch, NULL, 0, TO_ROOM);
          if (!::number(0,100/learning))
            ch->task->timeLeft--;
          if(!(ch->bSuccess(SKILL_LOGGING) || 
                (!ch->doesKnowSkill(SKILL_LOGGING) && !::number(0,99))) ||
                !(::number(1,LOGS_PER_ROOM*3/2) > rp->getLogsHarvested())) {
            act("You don't find any promising trees.",
              FALSE, ch, NULL, 0, TO_CHAR);
            act("$n doesn't seem to have found whatever $e was looking for.",
              TRUE, ch, NULL, 0, TO_ROOM);
            ch->stopTask();
            return TRUE;
          }
          break;
        case 2:
          // find a tree
          log_vnum = rp->getTreetype();
          log = read_object(log_vnum, VIRTUAL);
          if(log && (ch->bSuccess(SKILL_LOGGING) || 
                (!ch->doesKnowSkill(SKILL_LOGGING) && !::number(0,99))) &&
                (::number(1,LOGS_PER_ROOM*3/2) > rp->getLogsHarvested())) {
            if (!(ch->canSee(log)))
            {
              act("You decide that swinging $p around in the dark at trees you can't see is a bad idea.",
                FALSE, ch, tool, 0, TO_CHAR);
              act("$n doesn't seem to have found whatever $e was looking for.",
                TRUE, ch, tool, 0, TO_ROOM);
              ch->stopTask();
              return TRUE;
            }
            ch->sendTo(fmt("You've found a %s tree!\n\r") 
                % sstring(log->getName()).word(1));
            act("You begin to chop at the tree.",
              FALSE, ch, NULL, 0, TO_CHAR);
            act("$n begins to chop at a tree.",
              TRUE, ch, NULL, 0, TO_ROOM);
            delete log;
            log = NULL;
          } else {
            if (!log)
              vlogf(LOG_BUG, fmt("Error loading log %d in lumberjack task.")
                % log_vnum);
            act("You don't find any promising trees.",
              FALSE, ch, NULL, 0, TO_CHAR);
            act("$n doesn't seem to have found whatever $e was looking for.",
              TRUE, ch, NULL, 0, TO_ROOM);
            ch->stopTask();
            return TRUE;
          }
          ch->task->timeLeft--;
          break;
        case 1:
          ch->addToMove(::number(-1,-10+ch->getSkillValue(SKILL_LOGGING)/10));
          if (ch->getMove() < 5) {
            act("You are much too tired to continue chopping.", 
                FALSE, ch, tool, 0, TO_CHAR);
            act("$n stops chopping, and wipes sweat from $s brow.", 
                TRUE, ch, tool, 0, TO_ROOM);
            ch->stopTask();
            return TRUE;
          }
          
          act("You chop at the tree with your $o.",
                    FALSE, ch, tool, 0, TO_CHAR);
          act("$n chops at a tree with $s $o.",
                    TRUE, ch, tool, 0, TO_ROOM);
          if (!::number(0,3)) {
            if (tool->getCurSharp() > 1)
              tool->addToCurSharp(-1);
          }
          if (critFail(ch, SKILL_BUTCHER)) {
            dam = (5+min(20, tool->getCurSharp()/2));
            act("You chop at the tree with your $o, but you miss terribly and cut yourself!",
                      FALSE, ch, tool, 0, TO_CHAR);
            act("$n chops at a tree with $s $o, but misses terribly and cuts $sself!",
                      TRUE, ch, NULL, 0, TO_ROOM);
            if (critFail(ch, SKILL_BUTCHER)) { // double crit
              critnum = ::number(67,82);
              ch->critSlash(ch, tool, &part_hit, TYPE_SLASH, &dam, critnum);
            }
            if (ch->reconcileDamage(ch, dam, SKILL_LOGGING) == -1) {
              ch->stopTask();
              ch->doSave(SILENT_YES);
              return DELETE_THIS;
            }
            act("You stop your lumberjack activities.",
                FALSE, ch, 0, 0, TO_CHAR);
            act("$n stops $s lumberjack activities.",
                TRUE, ch, 0, 0, TO_ROOM);
            ch->stopTask();
            break;
          }
          if (!::number(0,200/learning))
            ch->task->timeLeft--;
          break;
        case 0:
          if((ch->bSuccess(SKILL_LOGGING) ||
              (!ch->doesKnowSkill(SKILL_LOGGING) && !::number(0,99))) &&
             (log=harvest_a_log(rp))) 
          {
            
            *rp += *log;

            int lvl=ch->GetMaxLevel();
            if(lvl>15)
              lvl-=15;
            else
              lvl=1;

            // 10% exp variance
            double exp=mob_exp(lvl);
            exp *= 0.1*(1.0+((::number(0,20)-10)/100.0)); // 0.1 x fishing xp
            
            gain_exp(ch, exp, -1);

            ch->doSave(SILENT_YES);

            act("You harvest $p.",
              FALSE, ch, log, 0, TO_CHAR);
            act("$n harvests $p.",
              TRUE, ch, log, 0, TO_ROOM);
          } else {
            if(log) {
              delete log;
              log = NULL;
            }
            act("You don't manage to harvest any useable logs.",
              FALSE, ch, NULL, 0, TO_CHAR);
            act("$n doesn't seem to have harvested anything useful from the tree.",
              TRUE, ch, NULL, 0, TO_ROOM);
          }
          ch->stopTask();
          break;
        }
      break;
    case CMD_ABORT:
    case CMD_STOP:
      act("You stop your lumberjack activities.",
          FALSE, ch, 0, 0, TO_CHAR);
      act("$n stops $s lumberjack activities.",
          TRUE, ch, 0, 0, TO_ROOM);
      ch->stopTask();
      break;
    case CMD_TASK_FIGHTING:
      ch->sendTo("Logging and fighting don't mix, grasshopper!\n\r");
      ch->stopTask();
      break;
    default:
      if (cmd < MAX_CMD_LIST)
        warn_busy(ch);
      break;                    // eat the command
  }
  return TRUE;
}

// procReforestation
procReforestation::procReforestation(const int &p)
{
  trigger_pulse=p;
  name="procReforestation";
}

void procReforestation::run(int pulse) const
{
  map <int, bool> ::iterator tIter     = mRoomsLogsHarvested.begin(),
                             tLastGood = mRoomsLogsHarvested.begin();

  while (tIter != mRoomsLogsHarvested.end()) {
    TRoom * tRoom = real_roomp((*tIter).first);

    if (!tRoom) {
      vlogf(LOG_BUG, fmt("procReforestation() handling non-existent room! (%d)") % (*tIter).first);
      continue;
    }

    // Make it only a chance.
    if ((tRoom->getLogsHarvested() >= LOGS_PER_ROOM) && !::number(0, 24))
      tRoom->setLogsHarvested(tRoom->getLogsHarvested()-1);

    if (tRoom->getLogsHarvested() <= 0) {
      if (tIter == tLastGood) {
        mRoomsLogsHarvested.erase(tIter);
        tIter = tLastGood = mRoomsLogsHarvested.begin();
      } else {
        mRoomsLogsHarvested.erase(tIter);
        tIter = tLastGood;
      }
    } else
      tLastGood = tIter;

    if (tIter == mRoomsLogsHarvested.end())
      break;

    ++tIter;
  }
}
