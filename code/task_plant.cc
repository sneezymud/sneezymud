#include "stdsneezy.h"
#include "obj_tool.h"
#include "obj_plant.h"

int TBeing::doPlant(sstring arg)
{
  sstring obj_arg, vict_arg, orig=arg;

  arg=one_argument(arg, obj_arg);
  arg=one_argument(arg, vict_arg);

  if(!obj_arg.empty() && vict_arg.empty())
    return doSeedPlant(orig);
  else
    return doThiefPlant(orig);
}



static bool genericCanPlantThief(TBeing *thief, TBeing *victim)
{
  bool is_imp = thief->hasWizPower(POWER_WIZARD);

  if ((thief->equipment[HOLD_LEFT] || thief->equipment[HOLD_RIGHT]) && 
      !is_imp) {
    thief->sendTo("It is impossible to plant something with your hand(s) already full!\n\r");
    return FALSE;
  }
  if (victim->isImmortal()) {
    thief->sendTo("You can't plant on an immortal.\n\r");
    return FALSE;
  }
  if (!thief->doesKnowSkill(SKILL_STEAL)) {
    thief->sendTo("You know nothing about planting.\n\r");
    return FALSE;
  }
  if (!is_imp) { 
    if (thief->checkPeaceful("What if they caught you?\n\r"))
      return FALSE;
    if (thief->roomp->isRoomFlag(ROOM_NO_STEAL)) {
      thief->sendTo("Such actions are prevented here.\n\r");
      return FALSE;
    }
  }

  if (victim == thief) {
    thief->sendTo("Come on now, that's rather stupid!\n\r");
    return FALSE;
  }

  if (thief->riding) {
    thief->sendTo("Yeah... right... while mounted.\n\r");
    return FALSE;
  }

  if (thief->isFlying()) {
    thief->sendTo("The fact that you are flying makes you a bit too conspicuous to steal.\n\r");
    return FALSE;
  }

  if (victim->isShopkeeper() && !is_imp) {
    thief->sendTo("Oh, Bad Move.  Bad Move.\n\r");
    vlogf(LOG_CHEAT, fmt("%s just tried to plant on a shopkeeper! [%s]") % 
          thief->getName() % victim->getName());
    return FALSE;
  }

  return true;
}


static int getPlantThiefChance(TBeing *thief, TBeing *victim)
{
  int vict_lev = victim->GetMaxLevel();
  int level = thief->getSkillLevel(SKILL_PLANT);
  int modifier = (level - vict_lev)/3;

  modifier += thief->plotStat(STAT_CURRENT, STAT_DEX, -70, 15, 0);

  if (!victim->awake())
    modifier += 50;

  if ((vict_lev > level) ||
      victim->isLucky(thief->spellLuckModifier(SKILL_PLANT)))
    modifier -= 45;

  modifier += victim->getCond(DRUNK)/4;

  if (!victim->isPc())
    modifier -= dynamic_cast<TMonster *>(victim)->susp()/2;

  int bKnown = thief->getSkillValue(SKILL_PLANT);

  modifier = max(min(modifier, 100 - bKnown), -100);

  return bKnown+modifier;
}


int TBeing::doThiefPlant(sstring arg)
{
  sstring obj_arg, vict_arg;
  TObj *obj;
  TBeing *vict;

  obj_arg=arg.word(0);
  vict_arg=arg.word(1);

  if(obj_arg.empty() || vict_arg.empty()){
    sendTo("Plant what on whom?\n\r");
    return FALSE;
  }
  
  if(!(obj=generic_find_obj(obj_arg, FIND_OBJ_INV|FIND_OBJ_EQUIP, this))){
    sendTo("You don't have that object.\n\r");
    return FALSE;
  }
  
  if(!(vict=generic_find_being(vict_arg, FIND_CHAR_ROOM, this))){
    sendTo("You don't see that person.\n\r");
    return FALSE;
  }

  if(!genericCanPlantThief(this, vict))
    return FALSE;

  if(this->bSuccess(getPlantThiefChance(this, vict), SKILL_PLANT)){
    return doGive(vict, obj, GIVE_FLAG_SILENT_VICT);
    sendTo("You were not detected.\n\r");
  } else {
    int rc=doGive(vict, obj);
    vict->sendTo("That seemed suspicious.\n\r");
    sendTo("You were detected.\n\r");
    return rc;
  }
}

int TBeing::doSeedPlant(sstring arg){
  TThing *t;
  TTool *seeds;
  int found=0, count;  

  if ((t = searchLinkedListVis(this, arg, getStuff(), NULL))){
    if((seeds=dynamic_cast<TTool *>(t))){
      if(seeds->getToolType() == TOOL_SEED){
	found=1;
      }
    }
  }
  if(!found){
    sendTo("You need to specify some seeds to plant.\n\r");
    return FALSE;
  }

  if(roomp->isFallSector() || roomp->isWaterSector() || 
     roomp->isIndoorSector()){
    sendTo("You can't plant anything here.\n\r");
    return FALSE;
  }

  TThing *tcount;
  for(tcount=roomp->getStuff(),count=0;tcount;tcount=tcount->nextThing){
    if(dynamic_cast<TPlant *>(tcount))
      ++count;
  }
  if(count>=8){
    sendTo("There isn't any room for more plants in here.\n\r");
    return FALSE;
  }


  sendTo("You begin to plant some seeds.\n\r");
  start_task(this, t, NULL, TASK_PLANT, "", 2, inRoom(), 0, 0, 5);
  return TRUE;
}


int task_plant(TBeing *ch, cmdTypeT cmd, const char *, int pulse, TRoom *, TObj *obj)
{
  TTool *tt;

  if (ch->utilityTaskCommand(cmd) ||
      ch->nobrainerTaskCommand(cmd))
    return FALSE;

  
  // basic tasky safechecking
  if (ch->isLinkdead() || (ch->in_room != ch->task->wasInRoom) || !obj){
    act("You stop planting your seeds.",
        FALSE, ch, 0, 0, TO_CHAR);
    act("$n stops planting seeds.",
        TRUE, ch, 0, 0, TO_ROOM);
    ch->stopTask();
    return FALSE; // returning FALSE lets command be interpreted
  }

  tt=dynamic_cast<TTool *>(obj);

  if (ch->task->timeLeft < 0){
    act("You finish planting $p.",
	FALSE, ch, obj, 0, TO_CHAR);
    act("$n finishes planting $p.",
	TRUE, ch, obj, 0, TO_ROOM);
    ch->stopTask();

    TObj *tp;
    TPlant *tplant;
    tp = read_object(OBJ_GENERIC_PLANT, VIRTUAL);
    if((tplant=dynamic_cast<TPlant *>(tp))){
      tplant->setType(seed_to_plant(tt->objVnum()));
      tplant->updateDesc();
    }
    
    *ch->roomp += *tp;

    if (tt->getToolUses() <= 0) {
      act("You discard $p because it is empty.",
	  FALSE, ch, tt, 0, TO_CHAR);
      delete tt;
    }

    return FALSE;
  }

  switch (cmd) {
    case CMD_TASK_CONTINUE:
      ch->task->calcNextUpdate(pulse, PULSE_MOBACT * 3);

      switch (ch->task->timeLeft) {
	case 2:
          act("You dig a little hole for some seeds from $p.",
              FALSE, ch, obj, 0, TO_CHAR);
          act("$n digs a little hole.",
              TRUE, ch, obj, 0, TO_ROOM);
          ch->task->timeLeft--;
          break;
	case 1:
          act("You put some seeds from $p into your hole.",
              FALSE, ch, tt, 0, TO_CHAR);
          act("$n puts some seeds from $p into the hole.",
              TRUE, ch, tt, 0, TO_ROOM);
          ch->task->timeLeft--;

	  tt->addToToolUses(-1);

          break;
	case 0:
	  act("You cover up the hole.",
              FALSE, ch, obj, 0, TO_CHAR);
	  act("$n covers up the hole.",
	      TRUE, ch, obj, 0, TO_ROOM);
	  ch->task->timeLeft--;
	  break;
      }
      break;
    case CMD_ABORT:
    case CMD_STOP:
      act("You stop planting seeds.",
          FALSE, ch, 0, 0, TO_CHAR);
      act("$n stops planting seeds.",
          TRUE, ch, 0, 0, TO_ROOM);
      ch->stopTask();
      break;
    case CMD_TASK_FIGHTING:
      ch->sendTo("You can't properly plant seeds while under attack.\n\r");
      ch->stopTask();
      break;
  default:
      if (cmd < MAX_CMD_LIST)
        warn_busy(ch);
      break;                    // eat the command
  }
  return TRUE;


}

