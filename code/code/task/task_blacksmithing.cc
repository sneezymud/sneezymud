//////////////////////////////////////////////////////////////////////////
//
// SneezyMUD - All rights reserved, SneezyMUD Coding Team
//
//////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////
//
//      SneezyMUD 4.0 - All rights reserved, SneezyMUD Coding Team
//      "task.cc" - All functions related to tasks that keep mobs/PCs busy
//
//////////////////////////////////////////////////////////////////////////

#include "comm.h"
#include "room.h"
#include "extern.h"
#include "obj_tool.h"
#include "obj_commodity.h"
#include "being.h"
#include "materials.h"


// used to plug messages and behavior to common repair functions
class BaseRepair
{
public:
  BaseRepair(TBeing *ch, spellNumT skill) { m_ch = ch; m_skill = skill; }

  // tools - override these
  virtual int GetPrimaryToolId() { return 0; }
  virtual int GetSecondaryToolId() { return 0; }
  virtual int GetRoom1ToolId() { return 0; }
  virtual int GetRoom2ToolId() { return 0; }

  // tool shortcuts
  TTool *GetPrimaryTool() { return GetTool(GetPrimaryToolId(), true); }
  TTool *GetSecondaryTool() { return GetTool(GetSecondaryToolId(), false); }
  bool HasPrimaryTool() { return !GetPrimaryToolId() || GetPrimaryTool() != NULL; }
  bool HasSecondaryTool() { return !GetSecondaryToolId() || GetSecondaryTool() != NULL; }
  bool HasRoom1Tool() { return !GetRoom1ToolId() || HasRoomTool(GetRoom1ToolId()); }
  bool HasRoom2Tool() { return !GetRoom2ToolId() || HasRoomTool(GetRoom2ToolId()); }

  // the main tool messages
  virtual const sstring NoPToolMsg() { return "BOGUS1"; }
  virtual const sstring NoSToolMsg() { return "BOGUS2"; }
  virtual const sstring NoR1ToolMsg() { return "BOGUS3"; }
  virtual const sstring NoR2ToolMsg() { return "BOGUS4"; }
  virtual const sstring DiePToolMsgC() { return "BOGUS5"; }
  virtual const sstring DiePToolMsgR() { return "BOGUS6"; }
  virtual const sstring DieSToolMsgC() { return "BOGUS7"; }
  virtual const sstring DieSToolMsgR() { return "BOGUS8"; }

  // sometimes needed, usually not
  virtual bool HasTools();

  // behavior messages (return true to stop tasking)
  virtual bool OnStop(TObj *o) { vlogf_trace(LOG_BUG, "Error: stop called on BaseRepair - needs override"); return false; }
  virtual bool OnComplete(TObj *o) { vlogf_trace(LOG_BUG, "Error: complete called on BaseRepair - needs override"); return false; }
  virtual bool OnDrain(TObj *o) { vlogf_trace(LOG_BUG, "Error: drain called on BaseRepair - needs override"); return false; }

  // more behavior, although you probably don't want to override these
  bool OnError();
  bool OnPulse(TObj *o);

  // return 1 if you want to actually repair the obj
  // return 0 for no repair
  // return -1 for stop
  virtual int OnSuccess(TObj *o) = 0;

  // main message pump
  int PumpMessage(cmdTypeT cmd, int pulse);

protected:
  TBeing *m_ch;
  spellNumT m_skill;

  // utility
  bool ConsumeRepairMats(TObj *o);
  TTool * GetTool(int vnum, bool primary);
  TTool* GetRoomTool(int vnum);
  bool HasRoomTool(int vnum) { return GetRoomTool(vnum); }
  bool DamageTool(bool primary, TObj *o, bool makeScraps);  // returns true if destroyed
};


TTool * BaseRepair::GetTool(int vnum, bool primary)
{
  if (0 == vnum)
    return NULL;

  TTool *tt = NULL;
  TTool *tool = NULL;

  if ((primary || m_ch->isAmbidextrous()) && m_ch->heldInPrimHand() &&
    (tt = dynamic_cast<TTool *>(m_ch->heldInPrimHand())) &&
    tt->getToolType() == vnum)
  {
    tool = tt;
  }

  if (!tool && (!primary || m_ch->isAmbidextrous()) && m_ch->heldInSecHand() &&
    (tt = dynamic_cast<TTool *>(m_ch->heldInSecHand())) &&
    tt->getToolType() == vnum)
  {
    tool = tt;
  }

  return tool;
}


TTool* BaseRepair::GetRoomTool(int vnum)
{
  TRoom *rp;
  if (vnum == 0)
    return NULL;

  if (!(rp = real_roomp(m_ch->in_room)))
    return NULL;


  for(StuffIter it= rp->stuff.begin();it!= rp->stuff.end();++it)
  {
    TTool *tt = dynamic_cast<TTool *>(*it);
    if (tt && tt->getToolType() == vnum)
      return tt;
  }

  return NULL;
}

bool BaseRepair::DamageTool(bool primary, TObj *o, bool makeScraps)
{
  TTool *tool = primary ? GetPrimaryTool() : GetSecondaryTool();
  if (!tool->addToToolUses(-1))
  {
    act(primary ? DiePToolMsgC() : DieSToolMsgC(), FALSE, m_ch, o, tool, TO_CHAR);
    act(primary ? DiePToolMsgR() : DieSToolMsgR(), FALSE, m_ch, o, tool, TO_ROOM);
    if (!makeScraps || !tool->makeScraps())
      delete tool;
    m_ch->stopTask();
    return true;
  }
  return false;
}

bool BaseRepair::HasTools()
{
  bool ret = true;
  if (!HasRoom1Tool()) { ret = false; m_ch->sendTo(NoR1ToolMsg()); }
  if (!HasRoom2Tool()) { ret = false; m_ch->sendTo(NoR2ToolMsg()); }
  if (!HasPrimaryTool()) { ret = false; m_ch->sendTo(NoPToolMsg()); }
  if (!HasSecondaryTool()) { ret = false; m_ch->sendTo(NoSToolMsg()); }
  return ret;
}

TCommodity *getRepairMaterial(StuffList list, ubyte mat)
{
  TCommodity *tc;

  for(StuffIter it=list.begin();it!=list.end();++it){
    if((tc=dynamic_cast<TCommodity *>(*it)) && tc->getMaterial() == mat)
      return tc;
  }
  return NULL;
}

bool BaseRepair::ConsumeRepairMats(TObj *o)
{
    int mats_needed=(int)((o->getWeight() / (float)o->getMaxStructPoints()) * 10.0);	
    mats_needed = (int)(repair_mats_ratio * mats_needed);
    // monogrammed items take 25% of normal mats to repair
    if (o->isMonogrammed())
      mats_needed = mats_needed / 4;
    if(mats_needed)
    {
      TCommodity *mat;

      if(!(mat = getRepairMaterial(m_ch->stuff, o->getMaterial())) || mat->numUnits() < mats_needed)
      {
        act(format("You don't have enough %s to continue repairing $p.") % material_nums[o->getMaterial()].mat_name, FALSE, m_ch, o, 0, TO_CHAR);
        return false;
      }
      mat->setWeight(mat->getWeight() - (mats_needed/10.0));
      if(mat->numUnits() <= 0)
        delete mat;
    }
    return true;
}

bool BaseRepair::OnError()
{
  if (m_ch->getPosition() < POSITION_SITTING)
  {
    act("You stop repairing, and look about confused.  Are you missing something?", FALSE, m_ch, 0, 0, TO_CHAR);
    act("$n stops repairing, and looks about confused and embarrassed.", FALSE, m_ch, 0, 0, TO_ROOM);
  }
  return true;
}

bool BaseRepair::OnPulse(TObj *o)
{
  if (::number(0,1))
  {
    act("$n examines $p carefully.", FALSE, m_ch, o, 0, TO_ROOM);
    act("You carefully examine $p.", FALSE, m_ch, o, 0, TO_CHAR);
  }
  return false;
}

// the main message pump for all repair - you probably don't need to override this
int BaseRepair::PumpMessage(cmdTypeT cmd, int pulse)
{
  TObj *o = NULL;
  int learning;
  int percent;
  bool didSucceed = FALSE;

  // get object
  for(StuffIter it= m_ch->stuff.begin();it!= m_ch->stuff.end();++it)
  {
    if((o = dynamic_cast<TObj *>(*it)) && isname(m_ch->task->orig_arg, o->name))
      break;
    o = NULL;
  }

  // check if linkdead etc
  if (m_ch->isLinkdead() || (m_ch->in_room < 0) || !o)
  {
    OnError();
    m_ch->stopTask();
    return 0;
  }

  // stop if stopped
  if (cmd == CMD_ABORT || cmd == CMD_STOP)
  {
    OnStop(o);
    m_ch->stopTask();
    return 1;
  }

  // stop if fighting
  if (cmd == CMD_TASK_FIGHTING)
  {
    m_ch->sendTo("You are unable to continue while under attack!\n\r");
    m_ch->stopTask();
    return 1;
  }

  // just ignore these
  if(m_ch->utilityTaskCommand(cmd) || m_ch->nobrainerTaskCommand(cmd))
    return FALSE;

  // from here on, we are continuing the task
  if (cmd != CMD_TASK_CONTINUE)
  {
    if (cmd < MAX_CMD_LIST)
      warn_busy(m_ch);
    return 1;
  }

  // check for tools
  if (!HasTools() || m_ch->getPosition() < POSITION_RESTING)
  {
    OnError();
    m_ch->stopTask();
	  return 0;
  }

  // task completed
  if (o->getMaxStructPoints() <= o->getStructPoints())
  {
    OnComplete(o);
    m_ch->stopTask();
    return 0;
  }

  learning = m_ch->getSkillValue(m_skill);
  didSucceed = m_ch->bSuccess(learning, m_skill);
  m_ch->task->calcNextUpdate(pulse, 2 * PULSE_MOBACT);

  if (m_ch->task->status && didSucceed || !m_ch->task->status) 
  {
    if (OnDrain(o))
    {
	    m_ch->stopTask();
	    return 0;
    }

    int succ = OnSuccess(o);
    if (succ > 0)
    {
      // comsume mats
      if (!ConsumeRepairMats(o))
      {
	      m_ch->stopTask();
	      return 0;
      }

      // calculate success
	    if ((percent = ::number(1, 101)) != 101)    // 101 is complete failure
	      percent -= m_ch->getDexReaction() * 3;

      // repair the object
	    if (percent < m_ch->getSkillValue(m_skill))
	      o->addToStructPoints(1);
	    else if (o->getStructPoints() > 0)
	      o->addToStructPoints(-1);
      else if (DamageTool(true, o, true))
      {
	      m_ch->stopTask();
        return 0;
      }

      if (o->getStructPoints() <= 1)
      {
        act("$n screws up repairing $p and wrecks it.", FALSE, m_ch, o, NULL, TO_ROOM);
        act("You screw up your job by dropping $p and wrecking it.", FALSE, m_ch, o, NULL, TO_CHAR);
        if (!o->makeScraps())
          delete o;

        m_ch->stopTask();
        return FALSE;
      } // (o->getStructPoints() <= 1)
    } // OnSuccess > 0
    else if (succ < 0)
    {
      m_ch->stopTask();
      return 0;
    }
  } // (m_ch->task->status && didSucceed || !m_ch->task->status) 
  if (OnPulse(o))
  {
    m_ch->stopTask();
    return 0;
  }

  return 1;
}

// Metal repair
class MetalRepair : public BaseRepair
{
public:
  MetalRepair(TBeing *ch) : BaseRepair(ch, SKILL_BLACKSMITHING) {}

  virtual int GetPrimaryToolId() { return TOOL_HAMMER; }
  virtual int GetSecondaryToolId() { return TOOL_TONGS; }
  virtual int GetRoom1ToolId() { return TOOL_FORGE; }
  virtual int GetRoom2ToolId() { return TOOL_ANVIL; }

  virtual const sstring NoPToolMsg() { return "You need to have a hammer in your primary hand.\n\r"; }
  virtual const sstring NoSToolMsg() { return "You need to have some tongs in your secondary hand.\n\r"; }
  virtual const sstring NoR1ToolMsg() { return "You need to have a forge in the room.\n\r"; }
  virtual const sstring NoR2ToolMsg() { return "You need to have a anvil in the room.\n\r"; }
  virtual const sstring DiePToolMsgC() { return "Your $O breaks due to overuse."; }
  virtual const sstring DiePToolMsgR() { return "$n looks startled as $e breaks $P while hammering."; }
  virtual const sstring DieSToolMsgC() { return "Your $O break with a loud *snap* and your grip on the $o slips!"; }
  virtual const sstring DieSToolMsgR() { return "$n's $O break with a loud *snap* and $s grip on the $o slips!"; }

  virtual bool OnStop(TObj *o);
  virtual bool OnComplete(TObj *o);
  virtual bool OnDrain(TObj *o);

  virtual int OnSuccess(TObj *o);
};

bool MetalRepair::OnStop(TObj *o)
{
  act("You stop trying to repair $p.", FALSE, m_ch, o, 0, TO_CHAR);
  m_ch->sendTo("Isn't there a professional around here somewhere?\n\r");
  act("$n stops repairing $p.", FALSE, m_ch, o, 0, TO_ROOM);
  if (m_ch->task->status > 0)
  {
    act("You let $p cool down.", FALSE, m_ch, o, 0, TO_CHAR);
    act("$n lets $p cool down.", FALSE, m_ch, o, 0, TO_ROOM);
  }
  return true; // stop
}

bool MetalRepair::OnComplete(TObj *o)
{
  act("$n finishes repairing $p and proudly smiles.", FALSE, m_ch, o, 0, TO_ROOM);
  act("You finish repairing $p and smile triumphantly.", FALSE, m_ch, o, 0, TO_CHAR);
  if (m_ch->task->status > 0)
  {
    act("You let $p cool down.", FALSE, m_ch, o, 0, TO_CHAR);
    act("$n lets $p cool down.", FALSE, m_ch, o, 0, TO_ROOM);
  }
  return true; // stop
}

bool MetalRepair::OnDrain(TObj *o)
{
  int amt = m_ch->getSkillValue(m_skill) / 10;
  int add = (m_ch->getRace() == RACE_DWARF) ? 4 : 0;

  m_ch->addToMove(min(-1, ::number(-10,-25) + ::number(1,amt) + add));

  if (m_ch->getMove() < 10)
  {
    act("You are much too tired to continue repairing $p.", FALSE, m_ch, o, 0, TO_CHAR);
    act("$n stops repairing, and wipes sweat from $s brow.", FALSE, m_ch, o, 0, TO_ROOM);
    return true; // stop
  }
  return false;
}

int MetalRepair::OnSuccess(TObj *o)
{
  const int HEATING_TIME = 3;
  
  TTool* forge = GetRoomTool(GetRoom1ToolId());
  TTool* anvil = GetRoomTool(GetRoom2ToolId());
  TTool* hammer = GetPrimaryTool();
  TTool* tongs = GetSecondaryTool();

  if (!m_ch->task->status)
  {
    act("$n allows $p to heat in $P.", FALSE, m_ch, o, forge, TO_ROOM);
    act("You allow $p to heat in $P.", FALSE, m_ch, o, forge, TO_CHAR);
  }
  else if (m_ch->task->status < HEATING_TIME)
  {
    act("$n continues to let $p heat in $P.", FALSE, m_ch, o, forge, TO_ROOM);
    act("You continue to let $p heat in $P.", FALSE, m_ch, o, forge, TO_CHAR);
  }
  else if (m_ch->task->status == HEATING_TIME)
  {
    act("$n removes $p with $P, as it glows red hot.", FALSE, m_ch, o, tongs, TO_ROOM);
    act("You remove $p with $P, as it glows red hot.", FALSE, m_ch, o, tongs, TO_CHAR);
    if (DamageTool(false, o, true))
      return -1; // stop
  }
  else
  {
    bool primary = ::number(0,3) != 0;
    act(format("$n pounds $p on %s with $s $O.") % anvil->getName(), FALSE, m_ch, o, hammer, TO_ROOM);
    act(format("You pound $p on %s with your $O.") % m_ch->objs(anvil), FALSE, m_ch, o, hammer, TO_CHAR);
    if (DamageTool(primary, o, true))
      return -1; // stop
    return 1; // repair this obj
  }
  m_ch->task->status++;
  return 0;
}


int task_blacksmithing(TBeing *ch, cmdTypeT cmd, const char *, int pulse, TRoom *, TObj *)
{
  MetalRepair rep(ch);
  return rep.PumpMessage(cmd, pulse);
}


// repair body parts: ash/powder (17), flesh (68), ogre hide (76), ivory (107), skull/bone (107), dragonbone (121)
// tools: an operating table (in room), scalpel (primary), forceps (secondary)
class DeadRepair : public BaseRepair
{
public:
  DeadRepair(TBeing *ch) : BaseRepair(ch, SKILL_REPAIR_SHAMAN) {}

  virtual int GetPrimaryToolId() { return TOOL_SCALPEL; }
  virtual int GetSecondaryToolId() { return TOOL_FORCEPS; }
  virtual int GetRoom1ToolId() { return TOOL_OPERATING_TABLE; }
  virtual const sstring NoPToolMsg() { return "You need to have a scalpel in your primary hand.\n\r"; }
  virtual const sstring NoSToolMsg() { return "You need to have some forceps in your secondary hand.\n\r"; }
  virtual const sstring NoR1ToolMsg() { return "You need to have an operating table in the room.\n\r"; }
  virtual const sstring DiePToolMsgC() { return "Your $O snaps in half as you operate on the $o!"; }
  virtual const sstring DiePToolMsgR() { return "$n's $O snaps in half as $e operates on the $o!"; }
  virtual const sstring DieSToolMsgC() { return "Your $O break with a loud *snap* as you operate on the $o!"; }
  virtual const sstring DieSToolMsgR() { return "$n's $O break with a loud *snap* as $e operates on the $o!"; }

  virtual bool OnStop(TObj *o);
  virtual bool OnComplete(TObj *o);
  virtual bool OnDrain(TObj *o);

  virtual int OnSuccess(TObj *o);
};


bool DeadRepair::OnComplete(TObj *o)
{
  TTool *operatingtable = GetRoomTool(GetRoom1ToolId());
	act("$n finishes operating on $p and smiles triumphantly.", FALSE, m_ch, o, 0, TO_ROOM);
	act("You finish operating on $p and smile triumphantly.", FALSE, m_ch, o, 0, TO_CHAR);
	act("You remove $p from $P.", FALSE, m_ch, o, operatingtable, TO_CHAR);
	act("$n removes $p from $P.", FALSE, m_ch, o, operatingtable, TO_ROOM);
  return true;
}

bool DeadRepair::OnDrain(TObj *o)
{
	m_ch->addToMove(min(-1, ::number(-10,-15) + ::number(1,((m_ch->getSkillValue(m_skill) / 20)))));
  if (m_ch->getMove() < 10)
  {
    act("You are much too tired to continue operating on $p.", FALSE, m_ch, o, 0, TO_CHAR);
    act("$n stops operating on $p, and wipes sweat from $s brow.", FALSE, m_ch, o, 0, TO_ROOM);
    return true; // stop
  }
  if (m_ch->getLifeforce() < 30)
  {
    act("You are too low on lifeforce to continue operating on $p.", FALSE, m_ch, o, 0, TO_CHAR);
    act("$n looks pale, and stops operating on $p.", FALSE, m_ch, o, 0, TO_ROOM);
    return true;
  }
  return false;
}

int DeadRepair::OnSuccess(TObj *o)
{
  TTool *operatingtable = GetRoomTool(GetRoom1ToolId());
  TTool *scalpel = GetPrimaryTool();
  TTool *forceps = GetSecondaryTool();

  if (!m_ch->task->status)
  {
    act("$n carefully places $p atop $P.", FALSE, m_ch, o, operatingtable, TO_ROOM);
    act("You carefully place $p atop $P.", FALSE, m_ch, o, operatingtable, TO_CHAR);	  
  }
  else if (::number(0,1))
  {
    if (::number(0,1))
    {
      act("$n delicately operates on $p with $P.", FALSE, m_ch, o, scalpel, TO_ROOM);
      act("You delicately operate on $p with $P.", FALSE, m_ch, o, scalpel, TO_CHAR);
      if (DamageTool(true, o, true))
        return -1;
    }
    else
    { 
      act("$n removes a damaged piece from $p with $s $O.", FALSE, m_ch, o, forceps, TO_ROOM);
      act("You remove a damaged piece from $p with your $O.", FALSE, m_ch, o, forceps, TO_CHAR);
      if (DamageTool(false, o, true))
        return -1;
    }
  } 
  else
  {
    act("$n focuses $s lifeforce, regrowing the damaged part of $p.", FALSE, m_ch, o, 0, TO_ROOM);
    act("You focus your lifeforce, regrowing the damaged part of $p.", FALSE, m_ch, o, 0, TO_CHAR);
    m_ch->addToLifeforce(-(::number(15,30)));
    return 1;
  }
  m_ch->task->status++;
  return 0;
}


bool DeadRepair::OnStop(TObj *o)
{
  act("You stop trying to operate on $p.", FALSE, m_ch, o, 0, TO_CHAR);
  m_ch->sendTo("Isn't there a professional around here somewhere?\n\r");
  act("$n stops operating on $p.", FALSE, m_ch, o, 0, TO_ROOM);
  return true;
}

int task_repair_dead(TBeing *ch, cmdTypeT cmd, const char *, int pulse, TRoom *, TObj *)
{
  DeadRepair rep(ch);
  return rep.PumpMessage(cmd, pulse);
}


// repair organic: wood (5), ebony (105) 
// tools: water (room), a ladle (primary), some rich soil (secondary)
class WoodRepair : public BaseRepair
{
public:
  WoodRepair(TBeing *ch) : BaseRepair(ch, SKILL_REPAIR_MONK) {}

  virtual int GetPrimaryToolId() { return TOOL_LADEL; }
  virtual int GetSecondaryToolId() { return TOOL_SOIL; }
  virtual const sstring NoPToolMsg() { return "You need to have a ladle in your primary hand.\n\r"; }
  virtual const sstring NoSToolMsg() { return "You need to have some fertilizer in your secondary hand.\n\r"; }
  virtual const sstring DiePToolMsgC() { return "Your $O breaks from overuse!"; }
  virtual const sstring DiePToolMsgR() { return "$n's $O breaks from overuse!"; }
  virtual const sstring DieSToolMsgC() { return "$P is all used up, and you discard it."; }
  virtual const sstring DieSToolMsgR() { return "$n uses up $P, and $e discards it."; }

  virtual bool HasTools();

  virtual bool OnStop(TObj *o);
  virtual bool OnComplete(TObj *o);
  virtual bool OnDrain(TObj *o);

  virtual int OnSuccess(TObj *o);
};

bool WoodRepair::HasTools()
{
  bool ret = true;
  if (!m_ch->roomp->isWaterSector()) { ret = false; m_ch->sendTo("You need to to be nearby a body of water to do this.\n\r"); }
  return BaseRepair::HasTools() && ret;
}


bool WoodRepair::OnComplete(TObj *o)
{
  act("$n finishes regrowing $p and proudly smiles.", FALSE, m_ch, o, 0, TO_ROOM);
  act("You finish regrowing $p and smile triumphantly.", FALSE, m_ch, o, 0, TO_CHAR);
  act("You uncover $p.", FALSE, m_ch, o, 0, TO_CHAR);
  act("$n uncovers $p.", FALSE, m_ch, o, 0, TO_ROOM);
  return true;
}


bool WoodRepair::OnDrain(TObj *o)
{
  int add = (m_ch->getRace() == RACE_ELVEN) ? 2 : 0;
	m_ch->addToMove(min(-1, ::number(-5,-15) + ::number(1,((m_ch->getSkillValue(m_skill) / 20))) + add));

  if (m_ch->getMove() < 10)
  {
	  act("You are much too tired to continue regrowing $p.", FALSE, m_ch, o, 0, TO_CHAR);
	  act("$n stops regrowing $p, and wipes sweat from $s brow.", FALSE, m_ch, o, 0, TO_ROOM);
	  return true;
	}
	if (m_ch->getMana() < 10)
  {
	  act("You are too low on mana to continue regrowing $p.", FALSE, m_ch, o, 0, TO_CHAR);
	  act("$n looks faint, and stops regrowing $p.", FALSE, m_ch, o, 0, TO_ROOM);
	  return true;
	}
  return false;
}


int WoodRepair::OnSuccess(TObj *o)
{
	if (!m_ch->task->status)
  {
	  act("$n makes a clearing on the ground large enough to hold $p.", FALSE, m_ch, o, 0, TO_ROOM);
	  act("You make a clearing on the ground large enough to hold $p.", FALSE, m_ch, o, 0, TO_CHAR);
	}
  else if (m_ch->task->status == 1)
  {
	  act("$n places $p in the clearing.", FALSE, m_ch, o, 0, TO_ROOM);
	  act("You place $p in the clearing.", FALSE, m_ch, o, 0, TO_CHAR);
	}
  else if (::number(0,1))
  {
	  if (::number(0,1))
    {
      TTool *ladle = GetPrimaryTool();
	    act("$n scoops some soil with $P and pours it on $p.", FALSE, m_ch, o, ladle, TO_ROOM);
	    act("You scoop some soil with $P and pour it on $p.", FALSE, m_ch, o, ladle, TO_CHAR);
      if (DamageTool(true, o, true))
        return -1;
	  }
    else
    {
      TTool *soil = GetSecondaryTool();
	    act("$n stirs manure from $P into the soil covering $p.", FALSE, m_ch, o, soil, TO_ROOM);
	    act("You stir manure from $P into the soil covering $p.", FALSE, m_ch, o, soil, TO_CHAR);
      if (DamageTool(false, o, false))
        return -1;
	  }
	}
  else
  {
	  act("$n places a hand over $p and concentrates, regrowing it.", FALSE, m_ch, o, 0, TO_ROOM);
	  act("You place a hand over $p and concentrate, regrowing it.", FALSE, m_ch, o, 0, TO_CHAR);
	  m_ch->addToMana(::number(-3,-8));
    return 1;
	}
  m_ch->task->status++;
  return 0;
}

bool WoodRepair::OnStop(TObj *o)
{
  act("You stop trying to regrowing $p.", FALSE, m_ch, o, 0, TO_CHAR);
  m_ch->sendTo("Isn't there a druid around here somewhere?\n\r");
  act("$n stops regrowing $p.", FALSE, m_ch, o, 0, TO_ROOM);
  if (m_ch->task->status > 0)
  {
    act("You uncover $p.", FALSE, m_ch, o, 0, TO_CHAR);
    act("$n uncovers $p.", FALSE, m_ch, o, 0, TO_ROOM);
  }
  return true;
}

int task_repair_wood(TBeing *ch, cmdTypeT cmd, const char *, int pulse, TRoom *, TObj *)
{
  WoodRepair rep(ch);
  return rep.PumpMessage(cmd, pulse);
}

// repair organic: coral (14), dragon scale (53), fish scale (75)
// tools: water (room), a ladle (primary), a vial of plant oils (secondary)
class OrganicRepair : public BaseRepair
{
public:
  OrganicRepair(TBeing *ch) : BaseRepair(ch, SKILL_REPAIR_MONK) {}

  virtual int GetPrimaryToolId() { return TOOL_LADEL; }
  virtual int GetSecondaryToolId() { return TOOL_PLANT_OIL; }
  virtual const sstring NoPToolMsg() { return "You need to have a ladle in your primary hand.\n\r"; }
  virtual const sstring NoSToolMsg() { return "You need to have some oil in your secondary hand.\n\r"; }
  virtual const sstring DiePToolMsgC() { return "Your $O breaks from overuse!"; }
  virtual const sstring DiePToolMsgR() { return "$n's $O breaks from overuse!"; }
  virtual const sstring DieSToolMsgC() { return "$P is all used up, and you discard it."; }
  virtual const sstring DieSToolMsgR() { return "$n uses up $P, and $e discards it."; }

  virtual bool HasTools();

  virtual bool OnStop(TObj *o);
  virtual bool OnComplete(TObj *o);
  virtual bool OnDrain(TObj *o);

  virtual int OnSuccess(TObj *o);
};

bool OrganicRepair::HasTools()
{
  bool ret = true;
  if (!m_ch->roomp->isWaterSector()) { ret = false; m_ch->sendTo("You need to to be nearby a body of water to do this.\n\r"); }
  return BaseRepair::HasTools() && ret;
}

bool OrganicRepair::OnComplete(TObj *o)
{
	act("$n finishes regenerating $p and proudly smiles.", FALSE, m_ch, o, 0, TO_ROOM);
	act("You finish regenerating $p and smile triumphantly.", FALSE, m_ch, o, 0, TO_CHAR);
	act("You dry $p off.", FALSE, m_ch, o, 0, TO_CHAR);
	act("$n dries $p off.", FALSE, m_ch, o, 0, TO_ROOM);
  return true;
}


bool OrganicRepair::OnDrain(TObj *o)
{
  int add = (m_ch->getRace() == RACE_ELVEN) ? 2 : 0;
	m_ch->addToMove(min(-1, ::number(-5,-15) + ::number(1,((m_ch->getSkillValue(m_skill) / 20))) + add));

	if (m_ch->getMove() < 10)
  {
	  act("You are much too tired to continue regenerating $p.", FALSE, m_ch, o, 0, TO_CHAR);
	  act("$n stops regenerating $p, and wipes sweat from $s brow.", FALSE, m_ch, o, 0, TO_ROOM);
	  return true;
	}
	if (m_ch->getMana() < 10)
  {
	  act("You are too low on mana to continue regenerating $p.", FALSE, m_ch, o, 0, TO_CHAR);
	  act("$n looks faint, and stops regenerating $p.", FALSE, m_ch, o, 0, TO_ROOM);
	  return true;
	}
  return false;
}


int OrganicRepair::OnSuccess(TObj *o)
{
	if (!m_ch->task->status)
  {
	  act("$n holds $p under the water for a moment.", FALSE, m_ch, o, 0, TO_ROOM);
	  act("You hold $p under the water for a moment.", FALSE, m_ch, o, 0, TO_CHAR);
	}
  else if (m_ch->task->status == 1)
  {
	  act("$n removes $p from the water.", FALSE, m_ch, o, 0, TO_ROOM);
	  act("You remove $p from the water.", FALSE, m_ch, o, 0, TO_CHAR);
	}
  else if (::number(0,1))
  {
	  if (::number(0,1))
    {
      TTool *ladle = GetPrimaryTool();
	    act("$n pours water over $p with $P.", FALSE, m_ch, o, ladle, TO_ROOM);
	    act("You pour water over $p with $P.", FALSE, m_ch, o, ladle, TO_CHAR);
      if (DamageTool(true, o, true))
        return -1;
	  }
    else
    {
      TTool *oils = GetSecondaryTool();
	    act("$n drops oil from $P and rubs it across $p.", FALSE, m_ch, o, oils, TO_ROOM);
	    act("You take oil from $P and rub it across $p.", FALSE, m_ch, o, oils, TO_CHAR);
      if (DamageTool(false, o, false))
        return -1;
	  }
	}
  else
  {
	  act("$n places a hand over $p and concentrates, regenerating it.", FALSE, m_ch, o, 0, TO_ROOM);
	  act("You place a hand over $p and concentrate, regenerating it.", FALSE, m_ch, o, 0, TO_CHAR);
	  m_ch->addToMana(::number(-3,-8));
    return 1;
	}
  m_ch->task->status++;
  return 0;
}
	
bool OrganicRepair::OnStop(TObj *o)
{
  act("You stop trying to regenerate $p.", FALSE, m_ch, o, 0, TO_CHAR);
  m_ch->sendTo("Isn't there a druid around here somewhere?\n\r");
  act("$n stops regenerating $p.", FALSE, m_ch, o, 0, TO_ROOM);
  if (m_ch->task->status > 0)
  {
    act("You dry $p off.", FALSE, m_ch, o, 0, TO_CHAR);
    act("$n dries $p off.", FALSE, m_ch, o, 0, TO_ROOM);
  }
  return true;
}


int task_repair_organic(TBeing *ch, cmdTypeT cmd, const char *, int pulse, TRoom *, TObj *)
{
  OrganicRepair rep(ch);
  return rep.PumpMessage(cmd, pulse);
}


// repair magical: plasma (12), water (57), fire (58), earth (59), elemental (60), ice (61), lightning (62),
//                 chaos (63), runed (102)
// tools: a pentagram (in room), some binding runes (primary), some globules of energy (secondary)
class MagicRepair : public BaseRepair
{
public:
  MagicRepair(TBeing *ch) : BaseRepair(ch, SKILL_REPAIR_MAGE) {}

  virtual int GetPrimaryToolId() { return TOOL_RUNES; }
  virtual int GetSecondaryToolId() { return TOOL_ENERGY; }
  virtual int GetRoom1ToolId() { return TOOL_PENTAGRAM; }
  virtual const sstring NoPToolMsg() { return "You need to have some runes in your primary hand.\n\r"; }
  virtual const sstring NoSToolMsg() { return "You need to have some energy in your secondary hand.\n\r"; }
  virtual const sstring NoR1ToolMsg() { return "You need to have a magical pentagram in the room.\n\r"; }
  virtual const sstring DiePToolMsgC() { return "Your $O is completely used up and dissolves rapidly!"; }
  virtual const sstring DiePToolMsgR() { return "$n's $O is completely used up and dissolves rapidly!"; }
  virtual const sstring DieSToolMsgC() { return "Your $O is fully discharged and disappears."; }
  virtual const sstring DieSToolMsgR() { return "$n's $O is fully discharged and disappears."; }

  virtual bool OnStop(TObj *o);
  virtual bool OnComplete(TObj *o);
  virtual bool OnDrain(TObj *o);

  virtual int OnSuccess(TObj *o);
};


bool MagicRepair::OnComplete(TObj* o)
{
  TTool *pentagram = GetRoomTool(GetRoom1ToolId());
  act("$n finishes refocusing the energy in $p and proudly smiles.", FALSE, m_ch, o, 0, TO_ROOM);
  act("You finish refocusing the energy in $p and smile triumphantly.", FALSE, m_ch, o, 0, TO_CHAR);
  act("You remove $p from $P.", FALSE, m_ch, o, pentagram, TO_CHAR);
  act("$n removes $p from $P.", FALSE, m_ch, o, pentagram, TO_ROOM);
  return true;
}

bool MagicRepair::OnDrain(TObj *o)
{
  int add = (m_ch->getRace() == RACE_ELVEN) ? 2 : 0;
  m_ch->addToMove(min(-1, ::number(-5,-10) + ::number(1,((m_ch->getSkillValue(m_skill) / 20))) + add));

  if (m_ch->getMove() < 10)
  {
    act("You are much too tired to continue refocusing the energy in $p.", FALSE, m_ch, o, 0, TO_CHAR);
    act("$n stops refocusing the energy in $p, and wipes sweat from $s brow.", FALSE, m_ch, o, 0, TO_ROOM);
    return true;
  }
	if (m_ch->getMana() < 10)
  {
	  act("You are too low on mana to continue refocusing the energy in $p.", FALSE, m_ch, o, 0, TO_CHAR);
	  act("$n looks faint, and stops refocusing the energy in $p.", FALSE, m_ch, o, 0, TO_ROOM);
	  return true;
	}
  return false;
}


int MagicRepair::OnSuccess(TObj *o)
{
  if (!m_ch->task->status)
  {
    act("$n concentrates intensly on $p.", FALSE, m_ch, o, 0, TO_ROOM);
    act("You concentrate intensly on $p.", FALSE, m_ch, o, 0, TO_CHAR);
  } 
  else if (m_ch->task->status == 1)
  {
    TTool *pentagram = GetRoomTool(GetRoom1ToolId());
    act("$n focuses on the $o, and places it in the center of $P.", FALSE, m_ch, o, pentagram, TO_ROOM);
    act("You focus on the $o, and place it in the center of $P.", FALSE, m_ch, o, pentagram, TO_CHAR);
  }
  else if (::number(0,1))
  {
    TTool *energy = GetSecondaryTool();
    act("$n channels some energy from the $O into $p.", FALSE, m_ch, o, energy, TO_ROOM);
    act("You channel some energy from the $O into $p.", FALSE, m_ch, o, energy, TO_CHAR);
    if (DamageTool(false, o, false))
      return -1;
    m_ch->addToMana(::number(-5,-10));
  }
  else
  {
    act("$n places a rune of binding upon the $o, which quickly fades after a moment.", FALSE, m_ch, o, 0, TO_ROOM);
    act("You place a rune of binding upon the $o, which quickly fades after a moment.", FALSE, m_ch, o, 0, TO_CHAR);
    if (DamageTool(true, o, false))
      return -1;
    return 1;
  }
  m_ch->task->status++;
  return 0;
}

bool MagicRepair::OnStop(TObj *o)
{
  act("You stop trying to refocus the energy in $p.", FALSE, m_ch, o, 0, TO_CHAR);
  m_ch->sendTo("Isn't there an artificer around here somewhere?\n\r");
  act("$n stops regenerating $p.", FALSE, m_ch, o, 0, TO_ROOM);
  return true;
}

int task_repair_magical(TBeing *ch, cmdTypeT cmd, const char *, int pulse, TRoom *, TObj *)
{
  MagicRepair rep(ch);
  return rep.PumpMessage(cmd, pulse);
}

// repair rock: pumice (18), pearl (67), obsidian (108), marble (113), stone (114), jade (116), amber (117),
//              turquoise (118), malchite (122), granite (123), jet (125)
// tools: pentagram (in mountain/cave room), a fine chisel (primary), some powdered silica
class RockRepair : public BaseRepair
{
public:
  RockRepair(TBeing *ch) : BaseRepair(ch, SKILL_REPAIR_MAGE) {}

  virtual int GetPrimaryToolId() { return TOOL_CHISEL; }
  virtual int GetSecondaryToolId() { return TOOL_SILICA; }
  virtual int GetRoom1ToolId() { return TOOL_PENTAGRAM; }
  virtual const sstring NoPToolMsg() { return "You need to have a chisel in your primary hand.\n\r"; }
  virtual const sstring NoSToolMsg() { return "You need to have some silica in your secondary hand.\n\r"; }
  virtual const sstring NoR1ToolMsg() { return "You need to have a magical pentagram in the room.\n\r"; }
  virtual const sstring DiePToolMsgC() { return "The tip of your $O shatters, rendering the tool useless!"; }
  virtual const sstring DiePToolMsgR() { return "The tip of $n's $O shatters, rendering the tool useless!"; }
  virtual const sstring DieSToolMsgC() { return "$P is empty, and you discard it."; }
  virtual const sstring DieSToolMsgR() { return "$n's empties $P, and $e discards it."; }

  virtual bool OnStop(TObj *o);
  virtual bool OnComplete(TObj *o);
  virtual bool OnDrain(TObj *o);

  virtual int OnSuccess(TObj *o);
};

bool RockRepair::OnComplete(TObj* o)
{
  TTool *pentagram = GetRoomTool(GetRoom1ToolId());
	act("$n finishes reforming the crystals in $p and proudly smiles.", FALSE, m_ch, o, 0, TO_ROOM);
	act("You finish reforming the crystals in $p and smile triumphantly.", FALSE, m_ch, o, 0, TO_CHAR);
	act("You remove $p from $P.", FALSE, m_ch, o, pentagram, TO_CHAR);
	act("$n removes $p from $P.", FALSE, m_ch, o, pentagram, TO_ROOM);
  return true;
}

bool RockRepair::OnDrain(TObj *o)
{
  m_ch->addToMove(min(-1, ::number(-10,-15) + ::number(1,((m_ch->getSkillValue(m_skill) / 20)))));

  if (m_ch->getMove() < 10)
  {
    act("You are much too tired to continue reforming the crystals in $p.", FALSE, m_ch, o, 0, TO_CHAR);
    act("$n stops reforming the crystals in $p, and wipes sweat from $s brow.", FALSE, m_ch, o, 0, TO_ROOM);
    return true;
  }
  if (m_ch->getMana() < 10)
  {
    act("You are too low on mana to continue reforming the crystals in $p.", FALSE, m_ch, o, 0, TO_CHAR);
    act("$n looks faint, and stops reforming the crystals in $p.", FALSE, m_ch, o, 0, TO_ROOM);
    return true;
  }
  return false;
}

int RockRepair::OnSuccess(TObj *o)
{
  TTool *pentagram = GetRoomTool(GetRoom1ToolId());
  TTool *chisel = GetPrimaryTool();
  TTool *silica = GetSecondaryTool();

  if (!m_ch->task->status)
  {
    act("$n concentrates intensly on $p.", FALSE, m_ch, o, 0, TO_ROOM);
    act("You concentrate intensly on $p.", FALSE, m_ch, o, 0, TO_CHAR);
  }
  else if (m_ch->task->status == 1)
  {
    act("$n focuses on the $o, and places it in the center of $P.", FALSE, m_ch, o, pentagram, TO_ROOM);
    act("You focus on the $o, and place it in the center of $P.", FALSE, m_ch, o, pentagram, TO_CHAR);
  }
  else if (::number(0,1))
  {
    act("$n chips some damaged rock away from $p.", FALSE, m_ch, o, chisel, TO_ROOM);
    act("You chip some damaged rock away from $p.", FALSE, m_ch, o, chisel, TO_CHAR);
    if (DamageTool(true, o, true))
      return -1;
  }
  else
  {
    act("$n sprinkles the $o with $P, reforming the crystal structures.", FALSE, m_ch, o, silica, TO_ROOM);
    act("You sprinkle the $o with $P, reforming the crystal structures.", FALSE, m_ch, o, silica, TO_CHAR);
    if (DamageTool(false, o, false))
      return -1;
    m_ch->addToMana(::number(-5,-10));
    return 1;
  }
  m_ch->task->status++;
  return 0;
}

bool RockRepair::OnStop(TObj *o)
{
  act("You stop trying to reforming the crystals in $p.", FALSE, m_ch, o, 0, TO_CHAR);
  m_ch->sendTo("Isn't there an geologist around here somewhere?\n\r");
  act("$n stops reforming the crystals in $p.", FALSE, m_ch, o, 0, TO_ROOM);
  return true;
}


int task_repair_rock(TBeing *ch, cmdTypeT cmd, const char *, int pulse, TRoom *, TObj *)
{
  RockRepair rep(ch);
  return rep.PumpMessage(cmd, pulse);
}



// repair gemmed: jeweled (101), crystal (103), diamond (104), emerald (106), onyx (109), opal (110), 
//                ruby (111), sapphire (112), amethyst (119), mica (120), quartz (124), corundum (126)
// tools: a workbench (room), a loupe (primary), a pair of needle nosed pliers (secondary)
class GemRepair : public BaseRepair
{
public:
  GemRepair(TBeing *ch) : BaseRepair(ch,
    ch->getSkillValue(SKILL_REPAIR_THIEF) > ch->getSkillValue(SKILL_BLACKSMITHING_ADVANCED) ? SKILL_REPAIR_THIEF : SKILL_BLACKSMITHING_ADVANCED) {}

  virtual int GetPrimaryToolId() { return TOOL_LOUPE; }
  virtual int GetSecondaryToolId() { return TOOL_PLIERS; }
  virtual int GetRoom1ToolId() { return TOOL_WORKBENCH; }
  virtual const sstring NoPToolMsg() { return "You need to have a loupe in your primary hand.\n\r"; }
  virtual const sstring NoSToolMsg() { return "You need to have some pliers in your secondary hand.\n\r"; }
  virtual const sstring NoR1ToolMsg() { return "You need to have a workbench in the room.\n\r"; }

  virtual const sstring DiePToolMsgC() { return "Your $O cracks and is rendered useless!"; }
  virtual const sstring DiePToolMsgR() { return "$n's $O cracks and is rendered useless!"; }
  virtual const sstring DieSToolMsgC() { return "Suddenly, your $O break and are rendered useless!"; }
  virtual const sstring DieSToolMsgR() { return "Suddenly, $n's $O break and are rendered useless!"; }

  virtual bool OnStop(TObj *o);
  virtual bool OnComplete(TObj *o);
  virtual bool OnDrain(TObj *o);

  virtual int OnSuccess(TObj *o);
};


bool GemRepair::OnComplete(TObj *o)
{
  TTool *workbench = GetRoomTool(GetRoom1ToolId());
  act("$n finishes rearranging the gems in $p and proudly smiles.", FALSE, m_ch, o, 0, TO_ROOM);
  act("You finish rearranging the gems in $p and smile triumphantly.", FALSE, m_ch, o, 0, TO_CHAR);
  act("You remove $p from $P.", FALSE, m_ch, o, workbench, TO_CHAR);
  act("$n removes $p from $P.", FALSE, m_ch, o, workbench, TO_ROOM);
  return true;
}

bool GemRepair::OnDrain(TObj *o)
{
  m_ch->addToMove(min(-1, ::number(-15,-25) + ::number(1,((m_ch->getSkillValue(m_skill) / 10)))));
  
  if (m_ch->getMove() < 10)
  {
    act("You are much too tired to continue repairing $p.", FALSE, m_ch, o, 0, TO_CHAR);
    act("$n stops fixing $p, and wipes sweat from $s brow.", FALSE, m_ch, o, 0, TO_ROOM);
    return true;
  }
  return false;
}

int GemRepair::OnSuccess(TObj *o)
{
  TTool *loupe = GetPrimaryTool();
  TTool *pliers = GetSecondaryTool();
  TTool *workbench = GetRoomTool(GetRoom1ToolId());

  if (!m_ch->task->status)
  {
    act("$n carefully inspects $p with $s $O.", FALSE, m_ch, o, loupe, TO_ROOM);
    act("You carefully inspect $p with your $O.", FALSE, m_ch, o, loupe, TO_CHAR);
  }
  else if (m_ch->task->status == 1)
  {
    act("$n delicately places the $o on $P.", FALSE, m_ch, o, workbench, TO_ROOM);
    act("You delicately place the $o on $P.", FALSE, m_ch, o, workbench, TO_CHAR);
  }
  else if (::number(0,1))
  {
    act("$n looks intently through $s $O at $p.", FALSE, m_ch, o, loupe, TO_ROOM);
    act("You look intently through your $O at $p.", FALSE, m_ch, o, loupe, TO_CHAR);
    if (DamageTool(true, o, true))
      return -1;
  }
  else
  {
    act("$n carefully uses $s $O to rearrange the gems in $p.", FALSE, m_ch, o, pliers, TO_ROOM);
    act("You carefully use your $O to rearrange the gems in $p.", FALSE, m_ch, o, pliers, TO_CHAR);
    if (DamageTool(false, 0, true))
      return -1;
    return 1;
  }
  m_ch->task->status++;
  return 0;
}

bool GemRepair::OnStop(TObj *o)
{
  act("You stop trying to rearranging the gems in $p.", FALSE, m_ch, o, 0, TO_CHAR);
  m_ch->sendTo("Isn't there an jeweler around here somewhere?\n\r");
  act("$n stops rearranging the gems in $p.", FALSE, m_ch, o, 0, TO_ROOM);
  return true;
}

int task_repair_gem(TBeing *ch, cmdTypeT cmd, const char *, int pulse, TRoom *, TObj *)
{
  GemRepair rep(ch);
  return rep.PumpMessage(cmd, pulse);
}


// mend hide: laminate (19), leather (51), toughened leather (52), fur (55), feathered (56), cat fur (69),
//            dog fur (70), rabbit fur (71), dwarven leather (73), soft leather (74)
// tools: a leather punch (primary), some leather cording (secondary) 
class LeatherRepair : public BaseRepair
{
public:
  LeatherRepair(TBeing *ch) : BaseRepair(ch, SKILL_REPAIR_MONK) {}

  virtual int GetPrimaryToolId() { return TOOL_PUNCH; }
  virtual int GetSecondaryToolId() { return TOOL_CORDING; }
  virtual const sstring NoPToolMsg() { return "You need to have a punch in your primary hand.\n\r"; }
  virtual const sstring NoSToolMsg() { return "You need to have some cording in your secondary hand.\n\r"; }

  virtual const sstring DiePToolMsgC() { return "The tip of your $O breaks with a *SNAP*, rendering it useless!"; }
  virtual const sstring DiePToolMsgR() { return "The tip of $n's $O breaks with a *SNAP*, rendering it useless!"; }
  virtual const sstring DieSToolMsgC() { return "$P is completely used up.  You'll need more to continue."; }
  virtual const sstring DieSToolMsgR() { return "$n uses $P up completely."; }

  virtual bool OnStop(TObj *o);
  virtual bool OnComplete(TObj *o);
  virtual bool OnDrain(TObj *o);

  virtual int OnSuccess(TObj *o);
};

bool LeatherRepair::OnComplete(TObj *o)
{
	act("$n finishes mending $p and proudly smiles.", FALSE, m_ch, o, 0, TO_ROOM);
	act("You finish mending $p and smile triumphantly.", FALSE, m_ch, o, 0, TO_CHAR);
  return true;
}


bool LeatherRepair::OnDrain(TObj *o)
{
  m_ch->addToMove(min(-1, ::number(-10,-15) + ::number(1,((m_ch->getSkillValue(m_skill) / 20)))));

  if (m_ch->getMove() < 10)
  {
    act("You are much too tired to continue mending $p.", FALSE, m_ch, o, 0, TO_CHAR);
    act("$n stops mending $p, and wipes sweat from $s brow.", FALSE, m_ch, o, 0, TO_ROOM);
    return true;
  }
  return false;
}

int LeatherRepair::OnSuccess(TObj *o)
{
  TTool *punch = GetPrimaryTool();
  TTool *cording = GetSecondaryTool();
  if (!m_ch->task->status) 
  {
    act("$n carefully inspects $p.", FALSE, m_ch, o, 0, TO_ROOM);
    act("You carefully inspect $p.", FALSE, m_ch, o, 0, TO_CHAR);
  }
  else if (1 == m_ch->task->status % 2)
  {
    act("$n makes a few holes around a tear in $p.", FALSE, m_ch, o, punch, TO_ROOM);
    act("You make a few holes around a tear in $p.", FALSE, m_ch, o, punch, TO_CHAR);
    if (DamageTool(true, o, true))
      return -1;
  }
  else
  {
    act("$n sews up a tear in the $o with $P.", FALSE, m_ch, o, cording, TO_ROOM);
    act("You sew up a tear in the $o with $P.", FALSE, m_ch, o, cording, TO_CHAR);
    if (DamageTool(false, o, false))
      return -1;
    return 1;
  }
  m_ch->task->status++;
  return 0;
}

bool LeatherRepair::OnStop(TObj *o)
{
  act("You stop trying to mend $p.", FALSE, m_ch, o, 0, TO_CHAR);
  m_ch->sendTo("Isn't there an seamstress around here somewhere?\n\r");
  act("$n stops mending $p.", FALSE, m_ch, o, 0, TO_ROOM);
  return true;
}

int task_mend_hide(TBeing *ch, cmdTypeT cmd, const char *, int pulse, TRoom *, TObj *)
{
  LeatherRepair rep(ch);
  return rep.PumpMessage(cmd, pulse);
}


// mend generic: paper (1), cardboard (10)
// tools: tape (primary)
// mend generic: wax (3), plastic (8), rubber (9)
// tools: a candle (primary)
// mend generic: sstring (11), hair (16), horsehair (15), straw (66)
// tools: a small needle (primary)
// mend generic: cloth (2), silk (6), wool (54), hemp (77)
// tools: a small needle (primary), a spool of thread (secondary)
// mend generic: porcelain (65), clay (64)
// tools: a tube of glue
int task_mend(TBeing *ch, cmdTypeT cmd, const char *, int pulse, TRoom *, TObj *)
{
  ch->sendTo("This skill is not yet implemented.\n\r");
  ch->stopTask();
  return TRUE;
}

// repair spiritual: ghostly (72), foodstuff (7)
// tools: an altar (in room), a small brush (primary), some astral resin (secondary) 
class SpiritRepair : public BaseRepair
{
public:
  SpiritRepair(TBeing *ch) : BaseRepair(ch,
    ch->getSkillValue(SKILL_REPAIR_CLERIC) > ch->getSkillValue(SKILL_REPAIR_DEIKHAN) ? SKILL_REPAIR_CLERIC : SKILL_REPAIR_DEIKHAN) {}

  virtual int GetPrimaryToolId() { return TOOL_BRUSH; }
  virtual int GetSecondaryToolId() { return TOOL_ASTRAL_RESIN; }
  virtual int GetRoom1ToolId() { return TOOL_ALTAR; }
  virtual const sstring NoPToolMsg() { return "You need to have a brush in your primary hand.\n\r"; }
  virtual const sstring NoSToolMsg() { return "You need to have some resin in your secondary hand.\n\r"; }
  virtual const sstring NoR1ToolMsg() { return "You need to have an altar in the room.\n\r"; }

  virtual const sstring DiePToolMsgC() { return "Your $O breaks with a *SNAP*, rendering it useless!"; }
  virtual const sstring DiePToolMsgR() { return "$n's $O breaks with a *SNAP*, rendering it useless!"; }
  virtual const sstring DieSToolMsgC() { return "Your $O is all used up, and you discard it as worthless."; }
  virtual const sstring DieSToolMsgR() { return "$n's $O is all used up, and $e discards it as worthless."; }

  virtual bool OnStop(TObj *o);
  virtual bool OnComplete(TObj *o);
  virtual bool OnDrain(TObj *o);

  virtual int OnSuccess(TObj *o);
};


bool SpiritRepair::OnComplete(TObj *o)
{
  TTool *altar = GetRoomTool(GetRoom1ToolId());
	act("$n finishes mending $p and proudly smiles.", FALSE, m_ch, o, 0, TO_ROOM);
	act("You finish mending $p and smile triumphantly.", FALSE, m_ch, o, 0, TO_CHAR);
	act("You remove $p from $P.", FALSE, m_ch, o, altar, TO_CHAR);
	act("$n removes $p from $P.", FALSE, m_ch, o, altar, TO_ROOM);
  return true;
}


bool SpiritRepair::OnDrain(TObj *o)
{
  m_ch->addToMove(min(-1, ::number(-10,-15) + ::number(1,((m_ch->getSkillValue(m_skill) / 20)))));
        
  if (m_ch->getMove() < 10)
  {
    act("You are much too tired to continue mending $p.", FALSE, m_ch, o, 0, TO_CHAR);
    act("$n stops mending $p, and wipes sweat from $s brow.", FALSE, m_ch, o, 0, TO_ROOM);
    return true;
  }

	if (m_ch->getPiety() < 10)
  {
    act("You are much too drained to continue mending $p.", FALSE, m_ch, o, 0, TO_CHAR);
    act("$n looks faint, and stops mending $p.", FALSE, m_ch, o, 0, TO_ROOM);
    return true;
  }
  return false;
}

int SpiritRepair::OnSuccess(TObj *o)
{
  TTool *brush = GetPrimaryTool();
  TTool *altar = GetRoomTool(GetRoom1ToolId());

  if (!m_ch->task->status)
  {
    act("$n carefully inspects $p.", FALSE, m_ch, o, 0, TO_ROOM);
    act("You carefully inspect $p.", FALSE, m_ch, o, 0, TO_CHAR);
  }
  else if (m_ch->task->status == 1)
  {
    act("$n delicately places the $o on $P.", FALSE, m_ch, o, altar, TO_ROOM);
    act("You delicately place the $o on $P.", FALSE, m_ch, o, altar, TO_CHAR);
  }
  else if (::number(0,2))
  {
    bool which = ::number(0,1) == 1;
    act("$n applies some resin to the $o using $s $O.", FALSE, m_ch, o, brush, TO_ROOM);
    act("You apply some resin to the $o using your $O.", FALSE, m_ch, o, brush, TO_CHAR);
    if (DamageTool(which, o, which))
      return -1;
  }
  else
  {
    act("$n channels the will of $s deity into $p, mending it.", FALSE, m_ch, o, 0, TO_ROOM);
    act("You channel the will of your deity into $p, mending it.", FALSE, m_ch, o, 0, TO_CHAR);
    m_ch->addToPiety(::number(-2, -7));
    return 1;
  }
  m_ch->task->status++;
  return 0;
}

bool SpiritRepair::OnStop(TObj *o)
{
  act("You stop trying to mending $p.", FALSE, m_ch, o, 0, TO_CHAR);
  m_ch->sendTo("Isn't there an priest around here somewhere?\n\r");
  act("$n stops mending $p.", FALSE, m_ch, o, 0, TO_ROOM);
  return true;
}

int task_repair_spirit(TBeing *ch, cmdTypeT cmd, const char *, int pulse, TRoom *, TObj *)
{
  SpiritRepair rep(ch);
  return rep.PumpMessage(cmd, pulse);
}



