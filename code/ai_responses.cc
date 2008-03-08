#include "stdsneezy.h"
#include "combat.h"
#include "obj_symbol.h"
#include "obj_potion.h"
#include "pathfinder.h"
#include "shop.h"
#include "database.h"

static char	responseFile[32];

static int specificCode(TMonster *, TBeing *, int, const resp *);

void TMonster::loadResponses(int virt, const sstring &immortal)
{
  resp *tmp = NULL;
  TDatabase db(DB_SNEEZY);

  if(immortal.empty()){
    db.query("select response from mobresponses where vnum=%i", virt);
    if(!db.fetchRow())
      return;  // no responses
  } else {
    db.setDB(DB_IMMORTAL);
    db.query("select response from mobresponses where vnum=%i and owner='%s'", virt, immortal.c_str());
    if(!db.fetchRow())
      return;  // no responses
  }

  mud_assert(resps == NULL, "Mob (%s) already had Responses.", getName());
    
  resps = new Responses();
  mud_assert(resps != NULL, "Mob (%s) failed initing Responses.", getName());

  //
  //    Read the response.
  //
  istringstream is(db["response"]);

  while ((tmp = readCommand(is)) != 0) {
    tmp->next = resps->respList;
    resps->respList = tmp;
  }
}

// returns RET_STOP_PARSING if no further response parsing should occur
// returns DELETE_THIS or DELETE_VICT in some situations.
int TMonster::modifiedDoCommand(cmdTypeT cmd, const sstring &arg, TBeing *mob, const resp* respo)
{
  int rc = 0;
  int value;
  TObj *obj = NULL;
  TMonster *tMonster;
  sstring arg2, buf;
  cmdTypeT cmd_val;
  TRoom *tRoom;
  dirTypeT dir=DIR_NONE;
  RespMemory *tMem, *rMem, *lMem;
  sstring tStObj(""),
         tStSucker(""),
         tStArgument(arg);
  TPathFinder path;

  if (!awake())
    return TRUE;

  // handle a few special cases
  switch (cmd) {
    case CMD_RESP_TOROOM:
      act(arg, FALSE, this, 0, 0, TO_ROOM); 
      return TRUE;
    case CMD_RESP_TONOTVICT:
      act(arg, FALSE, this, 0, mob, TO_NOTVICT); 
      return TRUE;
    case CMD_RESP_TOVICT:
      act(arg, FALSE, this, 0, mob, TO_VICT); 
      return TRUE;
    case CMD_RESP_UNFLAG:
      if (mob->isPlayerAction(PLR_SOLOQUEST)) {
        mob->remPlayerAction(PLR_SOLOQUEST);
        act("$n just removed your solo quest flag.", 
             FALSE, this, 0, mob, TO_VICT);
      }
      return TRUE;
    case CMD_FLAG:
      if (!mob->isPlayerAction(PLR_SOLOQUEST)) {
        mob->addPlayerAction(PLR_SOLOQUEST);
        act("$n just set your solo quest flag.", FALSE, this, 0, mob, TO_VICT);
        mob->dieFollower();
        if (dynamic_cast<TBeing *>(mob->riding)) {
          rc = mob->fallOffMount(mob->riding, POSITION_STANDING);
          if (IS_SET_DELETE(rc, DELETE_THIS))
            return DELETE_VICT;
        }
      }
      return TRUE;
    case CMD_LOAD:
    case CMD_RESP_CHECKLOAD:
      if (mobVnum() < 0) {
        doTell(mob->getNameNOC(this), "I would load it, but i'm a prototype.  Sorry.");
        return FALSE; // continue the script, even tho this is a 'dummy' trigger.
      }
      value = convertTo<int>(arg);

      if(value<0){
        setMoney(getMoney()+(-value));
        return TRUE;
      }

      if (value <= 0 || 
          ((rc = real_object(value)) <= 0)) {
        vlogf(LOG_MOB_RS, fmt("Problem in script (1).  Trying to load %d on %s") % value %getName());
        return FALSE;
      }
      if (!(obj = read_object(rc, REAL))) {
        vlogf(LOG_MOB_RS, fmt("Problem in script (2).  Trying to load %d on %s") % value %getName());
        return FALSE;
      }
      if (obj_index[rc].getNumber() > obj_index[rc].max_exist) {
        vlogf(LOG_MOB_RS, fmt("Quest mob (%s:%d) loading item (%s:%d) when over max_exist.") % 
              getName() % mobVnum() %
              obj->getName() % obj->objVnum());
      }
      TThing *t;
      for (t = getStuff(); t; t = t->nextThing) {
        TObj *tob = dynamic_cast<TObj *>(t);
        if (!tob)
          continue;
        if (tob->number == obj->number) {
          // the CHECKLOAD command is intentionally undocumented
          // builders should by default use "load" which will trip if they
          // screw the script up and load things twice or whatnot.
          // this is meant to catch problems like loading an item on roomenter
          // some valid quests let the quester get a key normally held by a
          // mob by asking for it, rather than killing the mob.  checkload
          // was built for this purpose (loads only if they don't have it)
          if (cmd == CMD_LOAD) {
            // I already have one of these, so don't load another
            vlogf(LOG_MOB_RS, fmt("Quest mob (%s) tried loading item (%s) that he already had.") % 
                  getName() % obj->getName());
            delete obj;
            return RET_STOP_PARSING;  // stop the script
          } else if (cmd == CMD_RESP_CHECKLOAD) {
            // I have the item, don't bother loading and just bypass this step
            delete obj;
            return FALSE;  // continue on
          }
        }
      }
      log_object(obj);
      *this += *obj;
      return FALSE;
    case CMD_RESP_LOADMOB:
      {
        if (mobVnum() < 0) {
          doTell(mob->getNameNOC(this), "I would load it, but i'm a prototype.  Sorry.");
          return FALSE; // continue the script, even tho this is a 'dummy' trigger.
        }

        arg2=arg;
        arg2=one_argument(arg2, buf);
        value = convertTo<int>(buf);
        arg2=one_argument(arg2, buf);
        int rnum = convertTo<int>(buf);

        TRoom *room = roomp;
        if (rnum > 0)
          room = real_roomp(rnum);
        if (room == NULL)
          room = roomp;

        if (value <= 0 || 
            ((rc = real_mobile(value)) <= 0)) {
          vlogf(LOG_MOB_RS, fmt("Problem in script (3).  Trying to load %d on %s") % value %getName());
          return FALSE;
        }
        if (!(tMonster = read_mobile(rc, REAL))) {
          vlogf(LOG_MOB_RS, fmt("Problem in script (4).  Trying to load %d on %s") % value %getName());
          return FALSE;
        }
        if (mob_index[rc].getNumber() > mob_index[rc].max_exist) {
          vlogf(LOG_MOB_RS, fmt("Quest mob (%s:%d) loading mob (%s:%d) when over max_exist.") % 
                getName() % mobVnum() %
                tMonster->getName() % tMonster->mobVnum());
        }
        if (mob_index[rc].spec == SPEC_SHOPKEEPER) {
          vlogf(LOG_MOB_RS, fmt("Problem in script.  %s trying to load %d which is a shopkeeper.") % 
                getName() % value);
          delete tMonster;
          return FALSE;
        }
        if (mob_index[rc].spec == SPEC_NEWBIE_EQUIPPER) {
          vlogf(LOG_MOB_RS, fmt("Problem in script.  %s trying to load %d which is a newbie helper.") % 
                getName() % value);
          delete tMonster;
          return FALSE;
        }

        if (room && tMonster)
          *room += *tMonster;
        return FALSE;
      }
    case CMD_RESP_PERSONALIZE:
      value = convertTo<int>(arg);

      if (value <= 0 || (real_object(value) < 0)) {
        vlogf(LOG_MOB_RS, fmt("Problem in script (5).  Trying to load %d on %s") %  value % getName());
        return FALSE;
      }

      personalize_object(this, mob, value, -1);
      return FALSE;
    case CMD_RESP_RESIZE:
      value = convertTo<int>(arg);

      if (value <= 0 || (real_object(value) < 0)) {
        vlogf(LOG_MOB_RS, fmt("Problem in script (6).  Trying to load %d on %s") %  value % getName(\
));
        return FALSE;
      }

      resize_personalize_object(this, mob, value, -1);
      return FALSE;
    case CMD_RESP_TOGGLE:
      value = convertTo<int>(arg);
      if (value <= 0 || value >= MAX_TOG_INDEX) {
        vlogf(LOG_MOB_RS, fmt("Bad argument to response (%s) command %d.  (%s)") % 
                name % cmd % arg);
        return FALSE;
      }
      if (TogIndex[value].togmob != MOB_ANY && TogIndex[value].togmob != mobVnum()) {
        vlogf(LOG_MOB_RS, fmt("Wrong mob (%s:%d) toggling toggle %d.") % 
                getName() % mobVnum() % value);
        mob->sendTo("Something bad happened, tell a god.\n\r");
        return FALSE;
      }
      mob->setQuestBit(value);

      return FALSE;
    case CMD_RESP_UNTOGGLE:
      value = convertTo<int>(arg);
      if (value <= 0 || value >= MAX_TOG_INDEX) {
        vlogf(LOG_MOB_RS, fmt("Bad argument to response (%s) special command %d.  (%s)") % 
                name % cmd % arg);
        return FALSE;
      }
      if (!mob->hasQuestBit(value)) {
        vlogf(LOG_MOB_RS, fmt("Response file untoggling an unset bit: %d on %s by %s '%s'") % 
              value % mob->getName() % getName() % respo->args);
      }
      mob->remQuestBit(value);
      return FALSE;
    case CMD_RESP_CHECKTOG:
      value = convertTo<int>(arg);
      if (value <= 0 || value >= MAX_TOG_INDEX) {
        vlogf(LOG_MOB_RS, fmt("Bad argument to response (%s) special command %d.  (%s)") % 
                name % cmd % arg);
        return FALSE;
      }
      if (!mob->hasQuestBit(value))
        return RET_STOP_PARSING;

      return FALSE;
    case CMD_RESP_CHECKUNTOG:
      value = convertTo<int>(arg);
      if (value <= 0 || value >= MAX_TOG_INDEX) {
        vlogf(LOG_MOB_RS, fmt("Bad argument to response (%s) special command %d.  (%s)") % 
                name % cmd % arg);
        return FALSE;
      }
      if (mob->hasQuestBit(value))
        return RET_STOP_PARSING;

      return FALSE;
    case CMD_RESP_CHECKMAX:
      value = real_object(convertTo<int>(arg));
      if (value <= 0 || value >= (signed)obj_index.size()) {
        vlogf(LOG_MOB_RS, fmt("Bad argument to response (%s) special command %s.  (%s)") % 
              name % cmd % arg);
        return FALSE;
      }
      if (obj_index[value].getNumber() >= obj_index[value].max_exist)
        return RET_STOP_PARSING;

      return FALSE;
    case CMD_RESP_LINK:
      // link this to an existing command
      // known problem: link nod me due to how "me" gets parsed in social      
      arg2=arg;
      arg2=one_argument(arg2, buf);
      if (buf.lower() == "roomenter"){
        cmd_val = CMD_RESP_ROOM_ENTER;
      } else if (buf.lower() == "package"){
        cmd_val = CMD_RESP_PACKAGE;
      } else if (buf.lower() == "pulse"){
        cmd_val = CMD_RESP_PULSE;
      } else if (buf.lower() == "created"){
        cmd_val = CMD_GENERIC_CREATED;
      } else if (buf.lower() == "killed"){
        cmd_val = CMD_RESP_KILLED;
      } else if (buf.lower() == "endmode"){
        cmd_val = CMD_RESP_ENDMODE;
      } else if (buf.lower() == "startfight"){
        cmd_val = CMD_RESP_STARTFIGHT;
      } else {
        cmd_val=searchForCommandNum(buf);
        if (cmd_val >= MAX_CMD_LIST) {
          vlogf(LOG_MOB_RS, fmt("Responses::readCommand(): Parse error in %s. link bad.") %  buf);
          return RET_STOP_PARSING;
        }
      }
      rc = checkResponses(mob, NULL, arg2, cmd_val);
      if (IS_SET_DELETE(rc, DELETE_THIS) || IS_SET_DELETE(rc, DELETE_VICT)) {
        return rc;
      }
    
      break;
    case CMD_RESP_CODE_SEGMENT:
      // special xx
      rc = specificCode(this, mob, convertTo<int>(arg), respo);
      if (IS_SET_DELETE(rc, DELETE_THIS) ||
          IS_SET_DELETE(rc, DELETE_VICT)) {
        return rc;
      }
      if (rc == RET_STOP_PARSING)
        return rc;
      break;
    case CMD_JUNK:
      // rc = doCommand(cmd, arg, NULL, FALSE);
      tStObj=tStArgument.word(0);
      tStSucker=tStArgument.word(1);

      TThing *tThing = searchLinkedList(tStObj, getStuff(), TYPEOBJ);

      if (!mob || !tThing)
        vlogf(LOG_BUG, fmt("Error in response script junk command:%s%s Mob[%s]") % 
              (mob ? "" : " !mob") %
              (tThing ? "" : " !tThing") %
              getNameNOC(this));

      if (tThing && tThing->parent == this)
        delete tThing;
      
      break;
    case CMD_GIVE:
#if 0
      tStObj=tStArgument.word(0);
      tStSucker=tStArgument.word(1);

      TThing *tThing = searchLinkedList(tStObj, stuff, TYPEOBJ);

      if (mob && tThing && canSee(mob))
        rc = doGive(mob, tThing, GIVE_FLAG_DROP_ON_FAIL);
      else if (!mob || !tThing)
        vlogf(LOG_BUG, fmt("Error in doGive:%s%s Mob[%s]") % 
              (mob ? "" : " !mob") %
              (tThing ? "" : " !tThing") %
              getNameNOC(this));

      if (tThing && tThing->parent == this && roomp) {
        doSay("Well I guess I'll just drop it here...");
        --(*tThing);
        *roomp += *tThing;
      
      }
#else
      //    Force the mob to drop if the give fails
      rc = doGive(arg, GIVE_FLAG_DROP_ON_FAIL);
      // the force drop shit is lame, so another safety check here. give SHOULD return 
      // FALSE if it failed, soooo... i'm gonna throw in a little hack -dash
      tStObj=tStArgument.word(0);
      tStSucker=tStArgument.word(1);

      if (!rc) {
	TThing *tThing = searchLinkedList(tStObj, getStuff(), TYPEOBJ);
	if (tThing && tThing->parent == this && roomp) {
	  doSay("Well I guess I'll just drop it here...");
	  --(*tThing);
	  *roomp += *tThing;
	  
	  act("You drop $p",
	      FALSE, this, tThing, NULL, TO_CHAR);
	  act("$n drops $p.",
	      FALSE, this, tThing, NULL, TO_ROOM);

	}
      }
#endif
      break;
    case CMD_RESP_CHECKROOM:
      value = convertTo<int>(arg);

      if ((in_room != value) && !inImperia())
        return RET_STOP_PARSING;

      break;
    case CMD_RESP_CHECKNROOM:
      value = convertTo<int>(arg);

      if ((in_room == value))
        return RET_STOP_PARSING;

      break;
    case CMD_RESP_CHECKZONE:
      value = convertTo<int>(arg);

      tRoom = real_roomp(value);

      if ((!roomp || !tRoom || tRoom->getZoneNum() != roomp->getZoneNum()) && !inImperia())
        return RET_STOP_PARSING;

      break;
    case CMD_RESP_CHECKNOTZONE:
      value = convertTo<int>(arg);

      tRoom = real_roomp(value);

      if ((!roomp || !tRoom || tRoom->getZoneNum() == roomp->getZoneNum()) && !inImperia())
        return RET_STOP_PARSING;

      break;
    case CMD_RESP_MOVETO:
      value=0;
      for (rMem = resps->respMemory; rMem; rMem = rMem->next) {
	if (rMem->cmd == CMD_RESP_DESTINATION){
	  if(!(value=convertTo<int>(rMem->args))){
	    // assume its a mob/pc name
	    TBeing *hunted;
	    for (hunted = character_list;hunted;hunted = hunted->next) {
	      if(isname(rMem->args, hunted->getName())){
		value=hunted->in_room;
		break;
	      }
	    }
	    
	  }
	  break;
	}
      }
      if(!value) break;

      switch((dir=path.findPath(in_room, findRoom(value)))){
	case -1: // lost or arrived at destination
	  break;
	case 0: case 1: case 2: case 3: case 4: 
	case 5: case 6: case 7: case 8: case 9:
	  rc=goDirection(dir);
	  break;
	default:
	  break;
      }
      break;
    case CMD_RESP_DESTINATION:
      // find and delete any other destination thats set
      for (rMem = resps->respMemory,
	     lMem = resps->respMemory; rMem; rMem = rMem->next) {
	if (rMem->cmd == CMD_RESP_DESTINATION){
	  if (rMem == resps->respMemory)
	    resps->respMemory = rMem->next;
	  else
	    lMem->next = rMem->next;
	  
	  delete rMem;
	  break;
	}
	lMem=rMem;
      }
      
      // add the new destination
      tMem = resps->respMemory;
      resps->respMemory = new RespMemory(CMD_RESP_DESTINATION, NULL, arg);
      resps->respMemory->next = tMem;

      break;
    case CMD_RESP_CHECKPERSON:
      TThing *tt;
      TBeing *tb;
      for(tt=roomp->getStuff();tt;tt=tt->nextThing){
      	if((tb=dynamic_cast<TBeing *>(tt)) && isname(tb->getName(), arg))
	        return FALSE;
      }
      return RET_STOP_PARSING;
      break;
    case CMD_RESP_CHECKCLASS:
      {
        if (!mob->hasClass(arg.c_str(), EXACT_YES))
          return RET_STOP_PARSING;
        return FALSE;
        break;
      }
    case CMD_RESP_CHECKNOTCLASS:
      {
        if (mob->hasClass(arg.c_str(), EXACT_YES))
          return RET_STOP_PARSING;
        return FALSE;
        break;
      }
    case CMD_RESP_SETMODE:
      {
        if (arg == "soloquest")
        {
          int myVnum = mobVnum();
          affectedData newAffect;
          affectedData *checkAffect;

          for (checkAffect = affected; checkAffect && checkAffect->type == AFFECT_COMBAT &&
            checkAffect->modifier == COMBAT_SOLO_KILL && checkAffect->level == myVnum;
            checkAffect = checkAffect->next);

          // if the mob is already fighting or it doesnt have this affect, stop
          if (fight() || getHit() < hitLimit() || checkAffect)
            return RET_STOP_PARSING;

          newAffect.type = AFFECT_COMBAT;
          newAffect.modifier = COMBAT_SOLO_KILL;
          newAffect.level = myVnum;
          newAffect.duration = PERMANENT_DURATION;
          newAffect.location = APPLY_NONE;
          newAffect.bitvector = 0;
          newAffect.be = this;
          mob->affectTo(&newAffect, -1);

          if (!mob->fight())
            mob->setFighting(this, 0, FALSE);
        }
        break;
      }
    case CMD_RESP_TRIGGER:
      {
        // find all mobs, and run the trigger command on them with the same arg
        for (TBeing * t = character_list; t; t = t->next)
        {
          TMonster * m = dynamic_cast<TMonster *>(t);
          if (!m || t->isPc() || t->polyed != POLY_TYPE_NONE)
            continue;
          rc = m->checkResponses(mob, NULL, arg, CMD_RESP_TRIGGER);
          if (IS_SET(rc, DELETE_THIS | DELETE_VICT))
            break;
        }

        break;
      }
    case CMD_RESP_DONERAND:
    case CMD_RESP_RANDOM:
    case CMD_RESP_RANDOPTION:
      return FALSE;
      break;
    default:
      mud_assert(cmd >= 0, "Unhandled special command in modifiedDoCommand array %d", cmd);
//      rc = doCommand(cmd, arg, (TThing *) mob, FALSE);
      rc = doCommand(cmd, arg, NULL, FALSE);
      break;
  }
  
  // this may be DELETE_THIS or DELETE_VICT
  return rc;
}

bool TMonster::checkResponsesPossible(cmdTypeT tCmd, const sstring &tSaid, TBeing *tBeing)
{
  if (desc || !resps || !resps->respList)
    return false;
  
  // NPCs cannot trigger response scripts?
  if (!tBeing->isPc() && tCmd != CMD_RESP_ENDMODE && tCmd != CMD_RESP_KILLED &&
    tCmd != CMD_RESP_STARTFIGHT && tCmd != CMD_GENERIC_CREATED)
    return false;

  // only some commands are allowable to be processed furing a fight
  if (fight() && tCmd != CMD_RESP_ENDMODE && tCmd != CMD_RESP_KILLED &&
    tCmd != CMD_RESP_STARTFIGHT && tCmd != CMD_RESP_TRIGGER)
    return false;

  resp *tResp;

  if (tCmd == CMD_WHISPER || tCmd == CMD_ASK || tCmd == CMD_SIGN)
    tCmd = CMD_SAY;


  // If they have a response with the same command we only want to do
  // sensative matching for say/ask/whisper else we just assume that it
  // is a good posibility that they will react to it.  The check for
  // say/ask/whisper is pretty much identical to the actual check down
  // below so we hit with a 90% possibility.  NOTE:
  // The last 10% is due to checktoggle/checkuntoggle and the like that
  // might exist in the body of the trigger.  We don't care about those
  // here, tho.  We only care if the general stuff matches up.
  for (tResp = resps->respList; tResp; tResp = tResp->next)
    if (tResp->cmd == tCmd)
      if (tCmd == CMD_SAY) {
        if (strcasestr(tSaid.c_str(), tResp->args))
          return true;
      } else if (tCmd == CMD_RESP_TRIGGER) {
        if (strcasestr(tSaid.c_str(), tResp->args))
          return true;
      } else
        return true;

  return false;
}

int handleMobileResponse(TBeing *tBeing, cmdTypeT tCmd, const sstring &tString)
{
  if (!tBeing->roomp || tCmd <= 0 || tCmd >= MAX_CMD_LIST )
    return FALSE;

  TThing   *tThing;
  TMonster *tMonster;
  int       nRc;

  for (tThing = tBeing->roomp->getStuff(); tThing; tThing = tThing->nextThing)
    if ((tMonster = dynamic_cast<TMonster *>(tThing)) &&
        !tMonster->isPc() && !tMonster->orig) {
      nRc = tMonster->checkResponses(tBeing, NULL, tString, tCmd);

      if (nRc) {
        if (IS_SET_DELETE(nRc, DELETE_THIS)) {
          delete tMonster;
          tMonster = NULL;
          REM_DELETE(nRc, DELETE_THIS);
        }

        if (IS_SET_DELETE(nRc, DELETE_VICT))
          return DELETE_THIS;

        return nRc;
      }
    }

  return FALSE;
}

int TMonster::checkResponses(TBeing *tBeing, TThing *tThing, const sstring &tSaid, cmdTypeT tCmd)
{
  if (!checkResponsesPossible(tCmd, tSaid, tBeing))
    return FALSE;

  int            nRc;
  taskData      *tTask = NULL;
  spellTaskData *tSpell = NULL;

  if ((tTask = task)) {
    act("You interrupt your task to deal with $N",
        FALSE, this, NULL, tBeing, TO_CHAR);
    act("$n interrupts $s task for a moment.",
        FALSE, this, NULL, tBeing, TO_ROOM);

    task = NULL;
  }

  if ((tSpell = spelltask)) {
    act("You interrupt your spell to deal with $N",
        FALSE, this, NULL, tBeing, TO_CHAR);
    act("$n interrupts $s task for a moment.",
        FALSE, this, NULL, tBeing, TO_ROOM);

    spelltask = NULL;
  }

  nRc = checkResponsesReal(tBeing, tThing, tSaid, tCmd);

  if (tTask) {
    act("You resume your task.",
        FALSE, this, NULL, tThing, TO_CHAR);

    task = tTask;
  }

  if (tSpell) {
    act("You resume your spell.",
        FALSE, this, NULL, tThing, TO_CHAR);

    spelltask = tSpell;
  }

  return nRc;
}

// return value is the random command number that will be executed
// returns -1 for faulty syntax / failed run / wrong option (don't execute)
// returns 0 for execute following commands / done with rand
int doRandCmd(cmdTypeT cmd, int choice, sstring parsedArgs) {

  int arg_int;
  
  switch(cmd) {
    case CMD_RESP_RANDOM:
      if(!(arg_int = convertTo<int>(parsedArgs))) {
        vlogf(LOG_MOB_RS, fmt("Bad argument to doRandCmd."));
        return 0;
      }
      arg_int = max(1,arg_int);
      choice = ::number(1,arg_int);
      return choice;
    case CMD_RESP_RANDOPTION:
      if((arg_int = convertTo<int>(parsedArgs))) {
        if (arg_int != choice) { 
          if (choice == 0) { return -1; }
          else return choice;
        }
        else if (arg_int == choice) return 0;
      }
      return choice;
    case CMD_RESP_DONERAND:
      return 0;
    default:
      return choice;
  }
}      



void responseTransaction(TBeing *speaker, TMonster *tm, int value)
{
  int shop_nr=160;
  TMonster *keeper;
  TBeing *t;

  for(t=character_list;t;t=t->next){
    if(t->number==shop_index[shop_nr].keeper)
      break;
  }

  if(t && (keeper=dynamic_cast<TMonster *>(t))){
    keeper->addToMoney(value, GOLD_SHOP);
    shoplog(shop_nr, speaker, keeper, tm->getName(), 
	    value, "giving");
    keeper->saveItems(fmt("%s/%d") % SHOPFILE_PATH % shop_nr);
  }
}


// returns DELETE_THIS if this has died as a result
// returns DELETE_VICT if speaker has died
// returns DELETE_ITEM (for give) if TARG should go away
//     NOTE, strip "this" of the item first
int TMonster::checkResponsesReal(TBeing *speaker, TThing *resp_targ, const sstring &said, cmdTypeT trig_cmd)
{
  sstring parsedArgs;
  resp *respo;
  int found = FALSE;
  int rc;
  int value;
  command *cmd;
  int said_int;
  int arg_int;
  int    storedCash = 0;
  bool   beenPassed = false;
  sstring tStString(""),
         tStBuffer(""),
         tStArg("");
  char   tString[256];
  int skip = 0; // command lines within brackets will be skipped if this is non-zero

  if (desc)
    return FALSE;

  if (!resps || !resps->respList)
    return FALSE;

  // speaker->isPc() is already checked in CheckResponsesPossible
  // fight() is already checked in CheckResponsesPossible

  // trying to drop something while casting is problematic
  if (spelltask)
    return FALSE;

  if (trig_cmd == CMD_WHISPER || trig_cmd == CMD_ASK || trig_cmd == CMD_SIGN)
    trig_cmd = CMD_SAY;

  for (respo=resps->respList; respo && !found; respo=respo->next) {
    if (respo->cmd == trig_cmd) {

    switch(trig_cmd) {
      case CMD_SAY:
        if (strcasestr(said.c_str(), respo->args)) {
          for( cmd = respo->cmds; cmd != 0; cmd=cmd->next) {
            parsedArgs = parseResponse( speaker, cmd->args);
            skip = doRandCmd(cmd->cmd, skip, parsedArgs);
            if (skip != 0) continue;
            found = TRUE;
            rc = modifiedDoCommand( cmd->cmd, parsedArgs, speaker, respo);
            if (IS_SET_DELETE(rc, DELETE_THIS) ||
                IS_SET_DELETE(rc, DELETE_VICT)) {
              // either mob or speaker has been whacked
              return rc;
            } else if (IS_SET_ONLY(rc, RET_STOP_PARSING)) {
              found = FALSE;
              break;
            } 
          }
        }
        break;
      case CMD_RESP_PACKAGE:
        // "package" commands are not accessible directly, but is a way
        // to give duplicate functionality to multiple commands.  That
        // is  Trigger#1 with checktog 1, and Trigger #2 with checktog 2
        // could both call the same package
        said_int = convertTo<int>(said);
        arg_int = convertTo<int>(respo->args);
        if (said_int != arg_int)
          break;

        for( cmd = respo->cmds; cmd != 0; cmd=cmd->next) {
          parsedArgs = parseResponse( speaker, cmd->args);
            skip = doRandCmd(cmd->cmd, skip, parsedArgs);
            if (skip != 0) continue;
          found = TRUE;
          rc = modifiedDoCommand( cmd->cmd, parsedArgs, speaker, respo);
          if (IS_SET_DELETE(rc, DELETE_THIS) ||
                IS_SET_DELETE(rc, DELETE_VICT)) {
            // either mob or speaker has been whacked
            return rc;
          } else if (IS_SET_ONLY(rc, RET_STOP_PARSING)) {
            found = FALSE;
            break;
          } 
        }
        break;
      case CMD_RESP_ROOM_ENTER:
        said_int = convertTo<int>(said);
        arg_int = convertTo<int>(respo->args);
        if (arg_int && said_int != arg_int) {
          break;
        }

        // skip wizinvis, pisses off newbie imms
        if (speaker->isImmortal() && !canSee(speaker, INFRA_YES))
          break;

        for( cmd = respo->cmds; cmd != 0; cmd=cmd->next) {
          parsedArgs = parseResponse( speaker, cmd->args);
            skip = doRandCmd(cmd->cmd, skip, parsedArgs);
            if (skip != 0) continue;
          found = TRUE;
          rc = modifiedDoCommand( cmd->cmd, parsedArgs, speaker, respo);
          if (IS_SET_DELETE(rc, DELETE_THIS) ||
                IS_SET_DELETE(rc, DELETE_VICT)) {
            // either mob or speaker has been whacked
            return rc;
          } else if (IS_SET_ONLY(rc, RET_STOP_PARSING)) {
            found = FALSE;
            break;
          } 
        }
        break;
      case CMD_GIVE:
        if (!(value = convertTo<int>(respo->args))) {
          vlogf(LOG_MOB_RS, fmt("Bad arguments for %s for Give") %  getName());
          break; 
        }
        if (value > 0) {
          TObj *to = NULL;
          if (resp_targ && (to = dynamic_cast<TObj *>(resp_targ)) &&
              to->objVnum() == value) {
            for( cmd = respo->cmds; cmd != 0; cmd=cmd->next) {
              parsedArgs = parseResponse( speaker, cmd->args);
            skip = doRandCmd(cmd->cmd, skip, parsedArgs);
            if (skip != 0) continue;
              found = TRUE;
              rc = modifiedDoCommand( cmd->cmd, parsedArgs, speaker, respo);
              if (IS_SET_DELETE(rc, DELETE_THIS) ||
                  IS_SET_DELETE(rc, DELETE_VICT)) {
                // either mob or speaker has been whacked
                return rc | DELETE_ITEM;
              } else if (IS_SET_ONLY(rc, RET_STOP_PARSING)) {
                found = FALSE;
                break;
              } 
            }
            if (found) {
              // I gave the obj to the mob, so I ought to destroy it
              // however, some quests have the item given back, so in these 
              // situations, don't delete it
              if (to->parent == this) {
                // notify for delete
                return DELETE_ITEM;
              }
            }
          }
          // not an item I need *
        } else if (value < 0) {
          if (!said.empty())
            said_int = convertTo<int>(said);
          else
            said_int = 0;

          for (RespMemory *rMem = resps->respMemory,
                          *lMem = resps->respMemory;
               rMem; rMem = rMem->next) {
            if (rMem->cmd == CMD_GIVE && rMem->name &&
                (speaker->getNameNOC(speaker) == rMem->name)){
              storedCash = convertTo<int>(rMem->args);

              if (rMem == resps->respMemory)
                resps->respMemory = rMem->next;
              else
                lMem->next = rMem->next;

              delete rMem;

              break;
            }

            lMem = rMem;
          }

          if (beenPassed)
            storedCash -= said_int;

          if ((storedCash + said_int) >= -value) {
            for (cmd = respo->cmds; cmd != 0; cmd = cmd->next) {
              parsedArgs = parseResponse( speaker, cmd->args);
              skip = doRandCmd(cmd->cmd, skip, parsedArgs);
              if (skip != 0) 
                continue;
              found = TRUE;
              rc = modifiedDoCommand( cmd->cmd, parsedArgs, speaker, respo);
              if (IS_SET_DELETE(rc, DELETE_THIS) ||
                  IS_SET_DELETE(rc, DELETE_VICT)) {
                // either mob or speaker has been whacked
                return rc;
              } else if (IS_SET_ONLY(rc, RET_STOP_PARSING)) {
                found = FALSE;
                break;
              } 
            }

            responseTransaction(speaker, this, -value);

            said_int += value;
          }

          if ((storedCash + said_int) > 0) {
            RespMemory *tMem = resps->respMemory;
            char tString[256];

            sprintf(tString, "%d", (storedCash + said_int));
            resps->respMemory = new RespMemory(CMD_GIVE, speaker, tString);
            resps->respMemory->next = tMem;
            beenPassed = true;
          }

          // not enough money given 
        }
        break;
      case CMD_LIST:
        if (strcasestr(said.c_str(), respo->args)) {
          for (cmd = respo->cmds; cmd != 0; cmd = cmd->next) {
            parsedArgs = parseResponse(speaker, cmd->args);
            skip = doRandCmd(cmd->cmd, skip, parsedArgs);
            if (skip != 0) continue;
            found = TRUE;
            rc = modifiedDoCommand(cmd->cmd, parsedArgs, speaker, respo);
            if (IS_SET_DELETE(rc, DELETE_THIS) ||
                IS_SET_DELETE(rc, DELETE_VICT)) {
              // either mob or speaker has been whacked
              return rc;
            } else if (IS_SET_ONLY(rc, RET_STOP_PARSING)) {
              found = FALSE;
              break;
            } 
          }
        }
        break;

      case CMD_GENERIC_CREATED:
      case CMD_RESP_PULSE:
      case CMD_RESP_ENDMODE:
      case CMD_RESP_KILLED:
      case CMD_RESP_STARTFIGHT:
      case CMD_RESP_TRIGGER:
        // args must match
        if (said != respo->args)
          break;
        for (cmd = respo->cmds; cmd != 0; cmd = cmd->next) {
          parsedArgs = parseResponse(speaker, cmd->args);
            skip = doRandCmd(cmd->cmd, skip, parsedArgs);
            if (skip != 0) continue;
          found = TRUE;
          rc = modifiedDoCommand(cmd->cmd, parsedArgs, speaker, respo);
          if (IS_SET_DELETE(rc, DELETE_THIS) ||
              IS_SET_DELETE(rc, DELETE_VICT)) {
            // either mob or speaker has been whacked
            return rc;
          } else if (IS_SET_ONLY(rc, RET_STOP_PARSING)) {
            found = FALSE;
            break;
          } 
        }
        break;
      case CMD_BUY:
        // Format: buy { "cost item name";
        //     Ex: buy { "1000 1 smoked-ham";
        tStString=sstring(respo->args).word(0);
        tStBuffer=sstring(respo->args).word(1);
        tStArg=sstring(respo->args).word(2);

        strcpy(tString, said.c_str());

        if ((is_number(tString) ?
             convertTo<int>(said) == convertTo<int>(tStBuffer) :
             isname(said, tStArg))) {
          value = convertTo<int>(tStString);

          if (speaker->getMoney() < value) {
            doTell(speaker->getNameNOC(this), "I'm afraid you don't have enough for that.");

            return TRUE;
          } else {
            speaker->addToMoney(-value, GOLD_SHOP);
            responseTransaction(speaker, this, -value);

            for (cmd = respo->cmds; cmd != 0; cmd = cmd->next) {
              parsedArgs = parseResponse(speaker, cmd->args);
            skip = doRandCmd(cmd->cmd, skip, parsedArgs);
            if (skip != 0) continue;
              found = TRUE;
              rc = modifiedDoCommand(cmd->cmd, parsedArgs, speaker, respo);
              if (IS_SET_DELETE(rc, DELETE_THIS) ||
                  IS_SET_DELETE(rc, DELETE_VICT)) {
                // either mob or speaker has been whacked
                return rc;
              } else if (IS_SET_ONLY(rc, RET_STOP_PARSING)) {
                found = FALSE;
                break;
              } 
            }
          }
        }
        break;

      case CMD_BOUNCE:
      case CMD_DANCE:
      case CMD_SMILE:
      case CMD_CACKLE:
      case CMD_LAUGH:
      case CMD_GIGGLE:
      case CMD_SHAKE:
      case CMD_PUKE:
      case CMD_GROWL:
      case CMD_SCREAM:
      case CMD_COMFORT:
      case CMD_NOD:
      case CMD_SIGH:
      case CMD_SULK:
      case CMD_HUG:
      case CMD_SNUGGLE:
      case CMD_CUDDLE:
      case CMD_NUZZLE:
      case CMD_CRY:
      case CMD_POKE:
      case CMD_ACCUSE:
      case CMD_GRIN:
      case CMD_BOW:
      case CMD_APPLAUD:
      case CMD_BLUSH:
      case CMD_BURP:
      case CMD_CHUCKLE:
      case CMD_CLAP:
      case CMD_COUGH:
      case CMD_CURTSEY:
      case CMD_FART:
      case CMD_FLIP:
      case CMD_FONDLE:
      case CMD_FROWN:
      case CMD_GASP:
      case CMD_GLARE:
      case CMD_GROAN:
      case CMD_GROPE:
      case CMD_HICCUP:
      case CMD_LICK:
      case CMD_LOVE:
      case CMD_MOAN:
      case CMD_NIBBLE:
      case CMD_POUT:
      case CMD_PURR:
      case CMD_RUFFLE:
      case CMD_SHIVER:
      case CMD_SHRUG:
      case CMD_SING:
      case CMD_SLAP:
      case CMD_SMIRK:
      case CMD_SNAP:
      case CMD_SNEEZE:
      case CMD_SNICKER:
      case CMD_SNIFF:
      case CMD_SNORE:
      case CMD_SPIT:
      case CMD_SQUEEZE:
      case CMD_STARE:
      case CMD_STRUT:
      case CMD_THANK:
      case CMD_TWIDDLE:
      case CMD_WAVE:
      case CMD_WHISTLE:
      case CMD_WIGGLE:
      case CMD_WINK:
      case CMD_YAWN:
      case CMD_SNOWBALL:
      case CMD_FRENCH:
      case CMD_COMB:
      case CMD_MASSAGE:
      case CMD_TICKLE:
      case CMD_PAT:
      case CMD_CURSE:
      case CMD_BEG:
      case CMD_BLEED:
      case CMD_CRINGE:
      case CMD_DAYDREAM:
      case CMD_FUME:
      case CMD_GROVEL:
      case CMD_HOP:
      case CMD_NUDGE:
      case CMD_PEER:
      case CMD_POINT:
      case CMD_PONDER:
      case CMD_PUNCH:
      case CMD_SNARL:
      case CMD_SPANK:
      case CMD_STEAM:
      case CMD_TACKLE:
      case CMD_TAUNT:
      case CMD_WHINE:
      case CMD_WORSHIP:
      case CMD_YODEL:
      case CMD_THINK:
      case CMD_WHAP:
      case CMD_BEAM:
      case CMD_CHORTLE:
      case CMD_BONK:
      case CMD_SCOLD:
      case CMD_DROOL:
      case CMD_RIP:
      case CMD_STRETCH:
      case CMD_PIMP:
      case CMD_BELITTLE:
      case CMD_TAP:
      case CMD_PILEDRIVE:
      case CMD_FLIPOFF:
      case CMD_MOON:
      case CMD_PINCH:
      case CMD_BITE:
      case CMD_KISS:
      case CMD_CHEER:
      case CMD_WOO:
      case CMD_GRUMBLE:
      case CMD_APOLOGIZE:
      case CMD_AGREE:
      case CMD_DISAGREE:
      case CMD_SPAM:
      case CMD_ARCH:
      case CMD_ROLL:
      case CMD_BLINK:
      case CMD_FAINT:
      case CMD_GREET:
      case CMD_TIP:
      case CMD_BOP:
      case CMD_JUMP:
      case CMD_WHIMPER:
      case CMD_SNEER:
      case CMD_MOO:
      case CMD_BOGGLE:
      case CMD_SNORT:
      case CMD_TANGO:
      case CMD_ROAR:
      case CMD_FLEX:
      case CMD_TUG:
      case CMD_CROSS:
      case CMD_HOWL:
      case CMD_GRUNT:
      case CMD_WEDGIE:
      case CMD_SCUFF:
      case CMD_NOOGIE:
      case CMD_BRANDISH:
      case CMD_TRIP:
      case CMD_DUCK:
      case CMD_BECKON:
      case CMD_WINCE:
      case CMD_HUM:
      case CMD_RAZZ:
      case CMD_GAG:
      case CMD_AVERT:
      case CMD_SALUTE:
      case CMD_PET:
      case CMD_GRIMACE:
       if ((isname( "none", respo->args) && (resp_targ == NULL)) ||
           (isname( "self", respo->args) && (resp_targ == speaker)) ||
           (isname( "me", respo->args) && (resp_targ == this)) ||
           (isname( "other", respo->args) &&
                     (resp_targ != this) &&
                     (resp_targ != speaker) && resp_targ)) {
          for( cmd = respo->cmds; cmd != 0; cmd=cmd->next) {
            parsedArgs = parseResponse( speaker, cmd->args);
            skip = doRandCmd(cmd->cmd, skip, parsedArgs);
            if (skip != 0) continue;
            found = TRUE;
            rc = modifiedDoCommand( cmd->cmd, parsedArgs, speaker, respo);
            if (IS_SET_DELETE(rc, DELETE_THIS) ||
                IS_SET_DELETE(rc, DELETE_VICT)) {
              // either mob or speaker has been whacked
              return rc;
            } else if (IS_SET_ONLY(rc, RET_STOP_PARSING)) {
              found = FALSE;
              break;
            } 
          }
        }
        break;
      default:
        vlogf(LOG_MOB_RS, fmt("Unknown command is checkResponse: %d") %  trig_cmd);
        break;
      } // end of switch 
    } 
  }

  return found;
}

//
//	parseResponse():	Replace the format chars with there
//				values.
//
sstring TMonster::parseResponse( TBeing *speaker, const char *sstring)
{
  char respBuf[256];
  char		*ptr=respBuf;
  int		i;

  *respBuf = 0;

  if (!sstring)
    return respBuf;

  for( i=0; sstring[i]; ++i) {
    //  Check for the format char.
    if( sstring[i] == '%') {
      //  Switch on the type of format char.
      //    %n - Name of person who said something.
      switch( sstring[++i]) {
        case 'n':
          sprintf( ptr, "%s", pers(speaker));
          ptr = strchr( ptr, 0);
          break;
      }
    //  Normal char.
    } else {
      *ptr++ = sstring[i];
    }
  }
  *ptr = 0;
  return respBuf;
}


void cleanInputBuffer(istringstream &is)
{
  int c;
  if(!is.eof()){
    while( (c=is.get()) != 0 && isspace( c));
    is.unget();
  }
}

// chr is always ' ' or ';'
// buf is empty buffer for result?
// so - read everything into buf until encountering:
// end of file, chr, or '}'

// should be able to convert this to sstring and find() etc
int readToChar(istringstream &is, char *buf, char chr)
{
  char *ptr = buf;
  int c;

  *ptr = 0;
  if (!is.eof()){
    cleanInputBuffer(is);

    while (((c=is.get()) != EOF) && c != chr && c != '}')
      *ptr++ = c;

    if (c == '}' && ptr != buf) {
      vlogf(LOG_LOW, 
	    fmt("Responses::readToChar(): Missing '%c' in %s on line '%s'") % 
             chr % responseFile % buf);
      return -1;  // return error, notifies for delete of response
    }

    if (c == EOF) {
      vlogf(LOG_LOW, fmt("Responses::readToChar(): hit EOF in %s while expecting '%c' on line '%s'") % 
             responseFile % chr % buf);
      return -1;  // return error, notifies for delete of response
    }
  }
  *ptr = 0;
  return 0;
}

void cleanString(char *sstring)
{
  char *ptr = sstring;
  int i;

  if (sstring) {
    for (i = 0; sstring[i]; ++i) {
      if (sstring[i] != '"')
        *ptr++ = sstring[i];
      /*
      else {
        if (sstring[i+1] && sstring[i+1] == '"') {
          *ptr++ = '"';
          i++;
        }
      }
      */
    }
    *ptr = 0;
  }
}
		
// takes the buf passed, and looks for certain subsstrings
// which will be replaced
// an implicit assumption is that the replacement subsstring will
// fit ok back into buf
static void sstringTranslate(char *buf)
{
  char * start_pt;

  while ((start_pt = strstr(buf, "#OBJCOST("))) {
    char * end_pt = strstr(start_pt, ")#");
    if (!end_pt) {
      vlogf(LOG_LOW, "sstringTranslate(): no terminator found");
      return;
    }
    end_pt += strlen(")#");

    char tmp[256];
    unsigned int len = end_pt - start_pt;

    strncpy(tmp, start_pt, len);
    tmp[len] = '\0';

    int obj_num, markup;
    int rc = sscanf(tmp, "#OBJCOST( %d, %d)#", &obj_num, &markup);
    if (rc != 2) {
      vlogf(LOG_LOW, fmt("sstringTranslate(): failed parse OBJCOST (%d)") %  rc);
      return;
    }
    int robj = real_object(obj_num);
    if (robj < 0) {
      vlogf(LOG_LOW, fmt("sstringTranslate(): bad vnum (%d)") %  obj_num);
      return;
    }
    
    int price;
    if (obj_index[robj].itemtype == ITEM_POTION) {
      TObj *obj = NULL;
      obj = read_object(robj, REAL);
      TPotion *pot = dynamic_cast<TPotion *>(obj);
      if (obj) {
        if (pot) {
          price = pot->getValue();
       } else {
         price = 0;
          vlogf(LOG_LOW, fmt("Error casting object as potion in response script: %s(%d)" ) % obj->getName() % robj);
       }
        delete obj;
        obj = NULL;
        pot = NULL;
      } else {
        price = 0;
        vlogf(LOG_LOW, fmt("Error loading potion object in response script: %d" ) % robj);
      }

    } else {
      price = obj_index[robj].value;
    }
    
    price += (int) (price * markup/100.0);

    char buf2[2048];
    strncpy(buf2, buf, start_pt - buf);
    buf2[start_pt - buf] = '\0';
    sprintf(buf2 + strlen(buf2), "%d%s", price, end_pt);

    strcpy(buf, buf2);
  }
}

resp * TMonster::readCommand(istringstream &is)
{
  char		*args, cmdStr[32], buf[1024];
  int		c, i;
  command	*newCmd=0, *prev = NULL;
  resp		*newResp=0;
  

  cleanInputBuffer(is);

  //
  //  Get the trigger command for the mob to look for.
  //
  readToChar( is, cmdStr, ' ');
  
  if( *cmdStr == 0)
     return 0;

  while (*cmdStr == '#') {
    // ignore this line, skip to next and continue
    while ((c = is.get()) != '\n');

    // recheck
    readToChar(is, cmdStr, ' ');
    if (*cmdStr == 0)
      return 0;
  }

  //
  //  Get the { from the file.
  //
  cleanInputBuffer( is);
  if( (c=is.get()) != '{') {
    vlogf(LOG_LOW, fmt("Responses::readCommand(): Parse error in %s. Error after '%s'. Expected '{' but found '%c'") % 
            responseFile % cmdStr % c);
    return 0;
  }

  //  If it's a say command we need to get what to look for.
  // all commands will have some sort of argment sstring
  readToChar(is, buf, ';');
  cleanString(buf);

  // this parses trigger arguments for special sstrings, etc
  sstringTranslate(buf);

  //  Convert the command sstring into an int and allocate space for the 
  //  resp structure.

  // convert cmdStr sstring into the actual command
  cmdTypeT cmd;
  if (!strcasecmp(cmdStr, "roomenter")) {
    cmd = CMD_RESP_ROOM_ENTER;
  } else if (!strcasecmp(cmdStr, "package")) {
    cmd = CMD_RESP_PACKAGE;
  } else if (!strcasecmp(cmdStr, "pulse")) {
    cmd = CMD_RESP_PULSE;
  } else if (!strcasecmp(cmdStr, "created")) {
    cmd = CMD_GENERIC_CREATED;
  } else if (!strcasecmp(cmdStr, "killed")) {
    cmd = CMD_RESP_KILLED;
  } else if (!strcasecmp(cmdStr, "endmode")) {
    cmd = CMD_RESP_ENDMODE;
  } else if (!strcasecmp(cmdStr, "startfight")) {
    cmd = CMD_RESP_STARTFIGHT;
  } else if (!strcasecmp(cmdStr, "trigger")) {
    cmd = CMD_RESP_TRIGGER;
  } else {
    cmd=searchForCommandNum(cmdStr);
    if (cmd >= MAX_CMD_LIST) {
      vlogf(LOG_LOW, fmt("Responses::readCommand(): Parse error in %s. Unknown command %s.") % 
            responseFile % cmdStr);
      return 0;
    }
  }

  // store trigger in lower case format for easy comparison
  for (i = 0;buf && buf[i]; i++)
    buf[i] = LOWER(buf[i]);

  newResp = new resp( cmd, buf);
  //
  //  Ok, time to grab the commands.
  //
  prev = NULL;
  if (readToChar(is, buf, ';') == -1) {
    delete newResp;
    newResp = NULL;
    return 0;
  }
  while( *buf != 0) {
    if (*buf == '#') {
      // skip and proceed
      if (readToChar(is, buf, ';') == -1) {
        delete newResp;
        newResp = NULL;
        return 0;
      }
      continue;
    }
    args = strchr( buf, ' '); // Point to where the args start.
    if( args != 0) {
      // Put a null in so buf just shows the command.
      *args++ = 0;

      // this parses response arguments for special sstrings, etc
      sstringTranslate(args);
    }

    // this allows us to check for special commands in the scripts
/*    cleanString( args);*/
    if (is_abbrev(buf, "toggle"))
      newCmd = new command( CMD_RESP_TOGGLE, args);
    else if (is_abbrev(buf, "untoggle"))
      newCmd = new command( CMD_RESP_UNTOGGLE, args);
    else if (is_abbrev(buf, "checktoggle"))
      newCmd = new command( CMD_RESP_CHECKTOG, args);
    else if (is_abbrev(buf, "checkuntoggle"))
      newCmd = new command( CMD_RESP_CHECKUNTOG, args);
    else if (is_abbrev(buf, "checkmax"))
      newCmd = new command( CMD_RESP_CHECKMAX, args);
    else if (is_abbrev(buf, "personalize"))
      newCmd = new command( CMD_RESP_PERSONALIZE, args);
    else if (is_abbrev(buf, "resize"))
      newCmd = new command( CMD_RESP_RESIZE, args);
    else if (is_abbrev(buf, "checkload"))
      newCmd = new command( CMD_RESP_CHECKLOAD, args);
    else if (is_abbrev(buf, "loadmob") && strlen(buf) > 4)
      newCmd = new command( CMD_RESP_LOADMOB, args);
    else if (is_abbrev(buf, "tovict"))
      newCmd = new command( CMD_RESP_TOVICT, args);
    else if (is_abbrev(buf, "tonotvict"))
      newCmd = new command( CMD_RESP_TONOTVICT, args);
    else if (is_abbrev(buf, "random"))
      newCmd = new command( CMD_RESP_RANDOM, args);
    else if (is_abbrev(buf, "randoption"))
      newCmd = new command( CMD_RESP_RANDOPTION, args);
    else if (is_abbrev(buf, "donerand"))
      newCmd = new command( CMD_RESP_DONERAND, args);
    else if (is_abbrev(buf, "toroom")) {
      if (strstr(buf, "$N")) {
        vlogf(LOG_LOW, fmt("Found '$N' on toroom command inside response file for %s") %   responseFile);
        delete newResp;
        newResp = NULL;
        return 0;
      }
      newCmd = new command( CMD_RESP_TOROOM, args);
    } else if (is_abbrev(buf, "unflag"))
      newCmd = new command( CMD_RESP_UNFLAG, args);
    else if (is_abbrev(buf, "link"))
      newCmd = new command( CMD_RESP_LINK, args);
    else if (is_abbrev(buf, "special"))
      newCmd = new command( CMD_RESP_CODE_SEGMENT, args);
    else if (is_abbrev(buf, "checkroom"))
      newCmd = new command(CMD_RESP_CHECKROOM, args);
    else if (is_abbrev(buf, "checknroom"))
      newCmd = new command(CMD_RESP_CHECKNROOM, args);
    else if (is_abbrev(buf, "checkzone"))
      newCmd = new command(CMD_RESP_CHECKZONE, args);
    else if (is_abbrev(buf, "checknotzone"))
      newCmd = new command(CMD_RESP_CHECKNOTZONE, args);
    else if (is_abbrev(buf, "moveto"))
      newCmd = new command(CMD_RESP_MOVETO, args);
    else if (is_abbrev(buf, "destination"))
      newCmd = new command(CMD_RESP_DESTINATION, args);
    else if (is_abbrev(buf, "checkperson"))
      newCmd = new command(CMD_RESP_CHECKPERSON, args);
    else if (is_abbrev(buf, "checkclass"))
      newCmd = new command( CMD_RESP_CHECKCLASS, args);
    else if (is_abbrev(buf, "checknotclass"))
      newCmd = new command( CMD_RESP_CHECKNOTCLASS, args);
    else if (is_abbrev(buf, "setmode"))
      newCmd = new command( CMD_RESP_SETMODE, args);
    else if (is_abbrev(buf, "trigger"))
      newCmd = new command( CMD_RESP_TRIGGER, args);
    else {
      if ((cmd=searchForCommandNum( buf)) >= MAX_CMD_LIST) {
        vlogf(LOG_MOB_RS,fmt("Responses::readCommand(): Parse error in %s. Unknown command %s.") % 
              responseFile % buf);
        delete newResp;
        return 0;
      }
      if ((cmd == CMD_ECHO) ||
          (cmd == CMD_EMOTE)) {
        vlogf(LOG_MOB_RS, fmt("potential bad command (%s) in %s response") %  buf % getName());
        delete newResp;
        return 0;
      }
      newCmd = new command( cmd, args);
    }
    if( newResp->cmds == 0)
      newResp->cmds = newCmd;
    if( prev != 0)
      prev->next = newCmd;
    prev = newCmd;
    if (readToChar(is, buf, ';') == -1) {
      delete newResp;
      newResp = NULL;
      return 0;
    }
  }
  return newResp;
}

// return DELETE_THIS for mob
// return DELETE_VICT for ch
int specificCode(TMonster *mob, TBeing *ch, int which, const resp * respo)
{
  sstring       tmpstr;
  TSymbol     *tSymbol;
  followData  *FDt;
  affectedData tAff;

  switch (which) {
    case 1:
      if (mob->mobVnum() != 3459) {
        vlogf(LOG_LOW, fmt("Bad mob (%s:%d) calling specificCode(%d)") % 
            mob->getName() % mob->mobVnum() % which);
        return RET_STOP_PARSING;
      }
      act("A swirling mist envelops you, as you bury your head between $N's legs.",
           FALSE, ch, 0, mob, TO_CHAR); 
      act("A swirling mist envelops $n, as $e buries $s head between your legs.",
           FALSE, ch, 0, mob, TO_VICT); 
      act("A swirling mist envelops $n, as $e buries $s head between $N's legs.",
           FALSE, ch, 0, mob, TO_NOTVICT); 
      act("$S thighs tighten around your neck, squeezing the life out of you.",
           FALSE, ch, 0, mob, TO_CHAR); 
      act("Your thighs tighten around $s neck, squeezing the life out of $m.",
           FALSE, ch, 0, mob, TO_VICT); 
      act("$S thighs tighten around $s neck, squeezing the life out of $m.",
           FALSE, ch, 0, mob, TO_NOTVICT); 
      act("You struggle frantically, but $N's powerful muscles sever your head from your shoulders!",
           FALSE, ch, 0, mob, TO_CHAR); 
      act("$n struggles frantically, but your powerful muscles sever $s head from $s shoulders!",
           FALSE, ch, 0, mob, TO_VICT); 
      act("$n struggles frantically, but $N's powerful muscles sever $s head from $s shoulders!",
           FALSE, ch, 0, mob, TO_NOTVICT); 
      ch->die(DAMAGE_BEHEADED);
      return DELETE_VICT;
    case 2:
      if (mob->mobVnum() != 1338) {
        vlogf(LOG_LOW, fmt("Bad mob (%s:%d) calling specificCode(%d)") % 
            mob->getName() % mob->mobVnum() % which);
        return RET_STOP_PARSING;
      }
      if (!ch->isUnaff()) {
        if (!ch->isBrother())
          mob->doSay("I'm afraid you can't become part of the Brotherhood when you belong to another faction already.");
        else
          mob->doSay("You are already in the Brotherhood.");
        return FALSE;
      }
      ch->remQuestBit(TOG_FACTIONS_ELIGIBLE);
      ch->setFaction(FACT_BROTHERHOOD);
      
      tmpstr = "Welcome to the ";
      tmpstr += FactionInfo[ch->getFaction()].faction_name;
      tmpstr += "!";
      mob->doSay(tmpstr);

      return FALSE;
    case 3:
      if (mob->mobVnum() != 3704) {
        vlogf(LOG_LOW, fmt("Bad mob (%s:%d) calling specificCode(%d)") % 
            mob->getName() % mob->mobVnum() % which);
        return RET_STOP_PARSING;
      }
      if (!ch->isUnaff()) {
        if (!ch->isCult())
          mob->doSay("I'm afraid you can't become part of the Sect when you belong to another faction already.");
        else
          mob->doSay("You are already in the Sect.");
        return FALSE;
      }
      ch->remQuestBit(TOG_FACTIONS_ELIGIBLE);
      ch->setFaction(FACT_CULT);
      
      tmpstr = "Welcome to the ";
      tmpstr += FactionInfo[ch->getFaction()].faction_name;
      tmpstr += "!";
      mob->doSay(tmpstr);

      return FALSE;
    case 4:
      if (mob->mobVnum() != 8835) {
        vlogf(LOG_LOW, fmt("Bad mob (%s:%d) calling specificCode(%d)") % 
            mob->getName() % mob->mobVnum() % which);
        return RET_STOP_PARSING;
      }
      if (!ch->isUnaff()) {
        if (!ch->isSnake())
          mob->doSay("I'm afraid you can't become part of the Order when you belong to another faction already.");
        else
          mob->doSay("You are already in the Order.");
        return FALSE;
      }
      ch->remQuestBit(TOG_FACTIONS_ELIGIBLE);
      ch->setFaction(FACT_SNAKE);
      
      tmpstr = "Welcome to the ";
      tmpstr += FactionInfo[ch->getFaction()].faction_name;
      tmpstr += "!";
      mob->doSay(tmpstr);

      return FALSE;
    case 5:
      if (mob->mobVnum() != 27105) {
        vlogf(LOG_LOW, fmt("Bad mob (%s:%d) calling specificCode(%d)") % 
          mob->getName() % mob->mobVnum() % which);
        return RET_STOP_PARSING;
      }

      if (!(tSymbol = dynamic_cast<TSymbol *>(read_object(504, VIRTUAL)))) {
        act("$n looks befuddled and says, \"I'm sorry...I can not seem to find a symbol for you.\"", TRUE, mob, 0, ch, TO_ROOM);
        vlogf(LOG_MOB_RS, "Was unable to load object 504.");
      } else {
        tSymbol->setSymbolFaction(ch->getFaction());
        act("$n reaches into his grey smock and pulls a symbol from beneath.",
            TRUE, mob, 0, ch, TO_ROOM);
        act("$n utters something barely audible and passes his hand above the symbol.",
            TRUE, mob, 0, ch, TO_ROOM);
        act("$n grins with delight as the symbol shines with power.",
            TRUE, mob, 0, ch, TO_ROOM);

        *mob += *tSymbol;
        mob->doGive(ch, tSymbol, GIVE_FLAG_DROP_ON_FAIL);
      }

      return FALSE;
    case 6:
      if (mob->mobVnum() != 10604) {
        vlogf(LOG_LOW, fmt("Bad mob (%s:%d) calling specificCode(%d)") % 
              mob->getName() % mob->mobVnum() % which);
        return RET_STOP_PARSING;
      }

      if (!ch)
        return RET_STOP_PARSING;

      FDt = (ch->master ? ch->master->followers : ch->followers);

      act("$n murmurs over a mermaids scale.",
          FALSE, mob, NULL, NULL, TO_ROOM);
      act("$n draws upon the primordial force of the mermaid.",
          FALSE, mob, NULL, NULL, TO_ROOM);
      act("A mermaids scale begins to spark and flash.",
          FALSE, mob, NULL, NULL, TO_ROOM);
      act("A giant mass of balled lightning forms above the mermaids scale.",
          FALSE, mob, NULL, NULL, TO_ROOM);
      act("A giant ball of blue-white lightning erupts from $n's hands!",
          FALSE, mob, NULL, NULL, TO_ROOM);

      tAff.type      = SPELL_LIGHTNING_BREATH;
      tAff.level     = mob->GetMaxLevel();
      tAff.location  = APPLY_IMMUNITY;
      tAff.modifier  = IMMUNE_ELECTRICITY;
      tAff.bitvector = 0;

      for (; FDt; FDt = FDt->next) {
        if (!mob->sameRoom(*FDt->follower))
          continue;

        // ch is taken care of below because they might be the leader.
        if (FDt->follower == ch)
          continue;

        // Only affect group members.
        if (!FDt->follower->isAffected(AFF_GROUP))
          continue;

        act("You are enveloped by a ball of pure blue-white light!",
            FALSE, FDt->follower, NULL, NULL, TO_CHAR);
        act("A blue-white ball of lightning surrounds $n!",
            TRUE, FDt->follower, NULL, NULL, TO_ROOM);

        tAff.duration  = ((ONE_SECOND * 60) * 60) * ::number(12, 24);
        tAff.modifier2 = ::number(25, 75);
        FDt->follower->affectJoin(FDt->follower, &tAff, AVG_DUR_NO, AVG_EFF_YES);
      }

      act("You are enveloped by a ball of pure blue-white light!",
          FALSE, ch, NULL, NULL, TO_CHAR);
      act("A blue-white ball of lightning surrounds $n!",
          TRUE, ch, NULL, NULL, TO_ROOM);
      tAff.duration  = ((ONE_SECOND * 60) * 60) * ::number(12, 24);
      tAff.modifier2 = ::number(25, 75);
        ch->affectJoin(ch, &tAff, AVG_DUR_NO, AVG_EFF_YES);

      if (ch->master) {
        act("You are enveloped by a ball of pure blue-white light!",
            FALSE, ch->master, NULL, NULL, TO_CHAR);
        act("A blue-white ball of lightning surrounds $n!",
            TRUE, ch->master, NULL, NULL, TO_ROOM);
        tAff.duration  = ((ONE_SECOND * 60) * 60) * ::number(12, 24);
        tAff.modifier2 = ::number(25, 75);
        ch->master->affectJoin(ch->master, &tAff, AVG_DUR_NO, AVG_EFF_YES);
      }

      return FALSE;
    case 7:
      if (mob->mobVnum() != 5131) {
        vlogf(LOG_LOW, fmt("Bad mob (%s:%d) calling specificCode(%d)") % 
              mob->getName() % mob->mobVnum() % which);
        return RET_STOP_PARSING;
      }

      if (!ch)
        return RET_STOP_PARSING;

      TRoom *rp;
      TThing *t;
      if (!(rp = real_roomp(526)))
	break;

      --(*ch);
      if (ch->riding) {
        --(*(ch->riding));
        *rp += *(ch->riding);
      }
      for (t = ch->rider; t; t = t->nextRider) {
        --(*t);
        *rp += *t;
      }
      *rp += *ch;
      act("$n arrives from a puff of smoke.", FALSE, ch, 0, 0, TO_ROOM);
      ch->doLook("", CMD_LOOK);
      break;
    case 8:
      if (mob->mobVnum() != 140) {
        vlogf(LOG_LOW, fmt("Bad mob (%s:%d) calling specificCode(%d)") % 
              mob->getName() % mob->mobVnum() % which);
        return RET_STOP_PARSING;
      }

      if (!ch)
        return RET_STOP_PARSING;

      if(ch->getSex() != SEX_NEUTER){
	mob->doSay("You have no need of this operation.");
	return RET_STOP_PARSING;
      }
      act("$n waves $s hands, utters many magic phrases and re-attaches your rod.", TRUE, mob, NULL, ch, TO_VICT);
      
      ch->setSex(SEX_MALE);

      break;
    default:
      vlogf(LOG_MOB_RS, fmt("Undefined response segment: %d") %  which);
      break;
  }

  return FALSE;
}


