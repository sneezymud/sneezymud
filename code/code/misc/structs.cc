//////////////////////////////////////////////////////////////////////////
//
//      SneezyMUD - All rights reserved, SneezyMUD Coding Team
//      "structs.cc" - Various class destructors, constructors and operators
//
//      Last major revision : November 1996
//
//////////////////////////////////////////////////////////////////////////

#include <cstdio>
#include "extern.h"
#include "room.h"
#include "handler.h"
#include "being.h"
#include "low.h"
#include "monster.h"
#include "charfile.h"
#include "account.h"
#include "combat.h"
#include "obj_board.h"
#include "person.h"
#include "statistics.h"
#include "games.h"
#include "obj_base_corpse.h"
#include "obj_table.h"
#include "obj_money.h"
#include "materials.h"
#include "obj_seethru.h"
#include "cmd_trophy.h"
#include "obj_open_container.h"
#include "obj_component.h"
#include "obj_tooth_necklace.h"
#include "obj_potion.h"
#include "obj_base_cup.h"
//#include "liquids.h"
#include "socket.h"
#include "timing.h"

TBeing::TBeing() :
  TThing(),
  race(NULL),
  points(),
  chosenStats(), 
  curStats(),
  multAtt(1.0),
  heroNum(0),
  m_craps(NULL),
  invisLevel(0),
  my_protection(0),
  combatMode(ATTACK_NORMAL),
  my_garbleFlags(0),
  faction(),
  discs(NULL),
  inPraying(0), 
  inQuaffUse(0),
  attackers(0),
  visionBonus(0),
  age_mod(0),
  cantHit(0),
  wait(0),
  polyed(POLY_TYPE_NONE),
  hunt_dist(0),
  wimpy(0),
  delaySave(FALSE),
  immunities(),
  player(),
  specials(),
  practices(),
  affected(NULL),
  master(NULL),
  orig(NULL),
  next(NULL),
  next_fighting(NULL),
  next_caster(NULL),
  followers(NULL),
  spelltask(NULL),
  task(NULL),
  skillApplys(NULL),
  trophy(NULL)
{
  // change the default value here
  number = -1;
  
  for (int i = 1; i< MAX_TOG_INDEX; i++) {
    toggles[i] = 0;
  }

  mobCount++;
  setRace(RACE_NORACE);
  trophy = new TTrophy(this);
}

TBeing::~TBeing() 
{
  affectedData *af = NULL, *af2 = NULL;
  skillApplyData *tempApply = NULL, *temp2Apply = NULL;
  TThing *i = NULL;
  TBeing *k = NULL, *next_char = NULL;
  TRoom *rp = NULL;
  int rc = 0;

  if ((!roomp || in_room == Room::NOWHERE) &&
      (!desc || !desc->connected || desc->connected >= CON_REDITING)) {
    if (in_room != Room::NOWHERE || parent)
      --(*this);
    else if (stuckIn) {
      if (eq_stuck > WEAR_NOWHERE) {
        stuckIn->setStuckIn(eq_stuck, NULL);
      } else {
        vlogf(LOG_BUG, format("Extract on stuck in items %s in slot -1 on %s") %  name % 
               stuckIn->name);
        return;
      }
    } else if (equippedBy) {
      if (eq_pos > WEAR_NOWHERE) {
        dynamic_cast<TBeing *>(equippedBy)->unequip(eq_pos);
      } else {
        vlogf(LOG_BUG, format("Extract on equipped item %s in slot -1 on %s") %  name % 
                equippedBy->name);
        return;
      }
    } else if (riding) 
      dismount(getPosition());

    // put um somewhere, we use to warn here but I see no good reason for
    // a warning about extracting from NOWHERE
    thing_to_room(this, Room::VOID);
  }

  // yank um out of casino situations
  removeAllCasinoGames();
  delete m_craps;
  m_craps = NULL;

  if (task) 
    stopTask();

  delete task;

  if (spelltask)
    stopCast(STOP_CAST_DEATH);

  delete spelltask;

  // must be done before name gets deleted
  if (followers || master)
    dieFollower();

  // stopFighting sometimes does stuff (quests, bezerk), do before name delete
  // Call AUTO_LIMBS stuff to show followers tanks limbs - Brutius 11/10/98
  if (fight()) 
    stopFighting();

  removeFromPeelPk(this);

  affectFrom(SKILL_BERSERK);
  
  for (k = gCombatList; k; k = next_char) {
    next_char = k->next_fighting;
    if (k->fight() == this)
      k->stopFighting();
  }

  if (desc) {
    if (desc->snoop.snooping && desc->snoop.snooping->desc)
      desc->snoop.snooping->desc->snoop.snoop_by = 0;

    if (desc->snoop.snoop_by) {
      desc->snoop.snoop_by->sendTo("Your victim is no longer among us.\n\r");
      if (desc->snoop.snoop_by->desc)
        desc->snoop.snoop_by->desc->snoop.snooping = 0;
    }
    desc->snoop.snooping = desc->snoop.snoop_by = 0;
  }
  wearSlotT j;
  if (!stuff.empty()) {
    // non-immortal pcs have a separate handling for this in ~TPerson
    sendTo("Here, you dropped some stuff, let me help you get rid of that.\n\r");
    for(StuffIter it=stuff.begin();it!=stuff.end();){
      i=*(it++);
      --(*i);
      delete i;
      i = NULL;
    }
    for (j = MIN_WEAR; j < MAX_WEAR; j++) {
      if (equipment[j]) {
        i = unequip(j);
        delete i;
        i = NULL;
      }
    }
  } else {
    if (GetMaxLevel() > MAX_MORT) {
      for (j = MIN_WEAR; j < MAX_WEAR; j++) {
        if (equipment[j]) {
          i = unequip(j);
          delete i;
          i = NULL;
        }
      }
    } else {
    }
  }

  for (k = character_list; k; k = k->next) {
    if (k->specials.hunting) {
      if (k->specials.hunting == this) {
        k->specials.hunting = NULL;
        REMOVE_BIT(k->specials.act, ACT_HUNTING);

        if (k->affectedBySpell(SKILL_TRACK)) {
          k->sendTo(COLOR_MOBS, format("You stop tracking %s.\n\r") % getName());
          k->affectFrom(SKILL_TRACK);
          k->stopTask();
        }
      }
    }
    TMonster *tmons = dynamic_cast<TMonster *>(k);
    if (tmons) {
#if 0
// don't do this.
// killing someone will clear hates/fears
// but renting out should preserve hatreds toward me

      // hates/fears on others need to be handled BEFORE my name is deleted
      if (tmons->Hates(this, NULL))
        tmons->remHated(this, NULL);
 
      if (tmons->Fears(this, NULL))
        tmons->remFeared(this, NULL);
#endif
 
      if (tmons->targ() == this)
        tmons->setTarg(NULL);

      if (tmons->opinion.random == this)
        tmons->opinion.random = NULL;
    }
  }

  while (rider)
    rider->dismount(POSITION_STANDING);

  if (getCaptiveOf())
    (getCaptiveOf())->remCaptive(this);

  while(getCaptive()) 
    remCaptive(getCaptive());

  if (riding) 
    dismount(POSITION_STANDING);

  if (desc) {
    if (desc->original) {
      remQuestBit(TOG_TRANSFORMED_LYCANTHROPE);
      doReturn("", WEAR_NOWHERE, true);
    }
  }

  // Must remove from room before removing the equipment!
  rp = roomp;
  --(*this);

  wearSlotT l;
  for (l = MIN_WEAR; l < MAX_WEAR; l++) {
    if (equipment[l])
      *rp += *unequip(l);
  }
  int res;
  for (l = MIN_WEAR; l < MAX_WEAR; l++) {
    if (getStuckIn(l))
      *rp += *pulloutObj(l, TRUE, &res);
  }

  if (!desc || !desc->connected || desc->connected >= CON_REDITING) {
    if (this == character_list)
      character_list = next;
    else {
      for (k = character_list; (k) && (k->next != this); k = k->next);

      // this isn't a critical problem, but using it to figure out how it happens
      mud_assert(k != NULL, "Character not found in character_list");
      if (k != NULL)
        k->next = next;
    }
  } else { // has to have both a desc and a desc->connected
    for (k = character_list; (k); k = k->next) {
      if (k == this)
        vlogf(LOG_BUG, format("%s (being) deleted without removal from character_list connected = (%d)") %  getName() % desc->connected);
    }
  }

  //  setArmor(1000);

  // OK, technically, this is all Descriptor maintained stuff...
  // but, has to be here for this situation:
  // PC editing (has desc->edit_str) when gets killed, nanny will use
  // presence of str to place person into proper routine
  // would be very bad to leave them with a str and return them to nanny
  if (desc) {
    desc->cleanUpEditStr();
  }

  if (desc) {
    if (desc->account)
      desc->account->status = TRUE;
    desc->connected = CON_CONN;
    desc->character = NULL;
    rc = desc->doAccountMenu("");
    if (IS_SET_DELETE(rc, DELETE_THIS)) {
      delete desc; 
      desc = NULL;
    }
  }
  if (rp)

    rp->initLight();
  else
    vlogf(LOG_BUG, "NULL rp in TBeing destructor at initLight call.");

  // get rid of discipline stuff
  delete discs;
  discs = NULL;

  if (getCaster()) {
    remCastingList(this);
    setCaster(NULL);
  }
  for (af = affected; af; af = af2) {
    af2 = af->next;
    affectRemove(af);
  }

  // remove the skillApplies AFTER the affects
  for (tempApply = skillApplys;tempApply; tempApply = temp2Apply) {
    temp2Apply = tempApply->nextApply;
    delete tempApply;
  }
  skillApplys = NULL;

  setRace(RACE_NORACE);

  delete trophy;
}

TObj::TObj() :
  TThing(),
  obj_flags(), 
  action_description(NULL),
  owners(NULL),
  isTasked(false),
  isLocked(false)
{
  // change the default value here
  number = -1;

  objCount++;
  object_list.push_front(this);
}

TObj::~TObj() 
{
  TThing *t = NULL;

  if (spec)
    checkSpec(NULL, CMD_GENERIC_DESTROYED, "", NULL);

  if (getCaster()) {
    remCastingList(this);
    setCaster(NULL);
  }

  // corpses have had items removed from them in ~TBaseCorpse()
  for(StuffIter it=stuff.begin();it!=stuff.end();){
    t=*(it++);
    --(*t);
    if (t) {
      delete t;
      t = NULL;
    }
  }

  if (in_room != Room::NOWHERE || parent)
    --(*this);
  else if (stuckIn) {
    if (eq_stuck > WEAR_NOWHERE) {
      stuckIn->setStuckIn(eq_stuck, NULL);
    } else {
      vlogf(LOG_BUG, format("Extract on stuck in items %s in slot -1 on %s") %  name % 
             stuckIn->name);
      return;
    }
  } else if (equippedBy) {
    if (eq_pos > WEAR_NOWHERE) {
      dynamic_cast<TBeing *>(equippedBy)->unequip(eq_pos);
    } else {
      vlogf(LOG_BUG, format("Extract on equipped item %s in slot -1 on %s") %  name % 
              equippedBy->name);
      return;
    }
  } else if (riding) {
    positionTypeT new_pos = POSITION_DEAD;
    TBeing *tbt = dynamic_cast<TBeing *>(this);
    if (tbt)
      new_pos = tbt->getPosition();
    dismount(new_pos);
  }


  while (rider) {
    positionTypeT new_pos = POSITION_DEAD;
    TBeing *tbt = dynamic_cast<TBeing *>(rider);
    if (tbt)
      new_pos = tbt->getPosition();
    rider->dismount(new_pos);
  }

  // we must cancel any tasks which are running using this object
  // this may be super-slow if we are destroying alot of objects
  // so hide it behind a flag
  if (isTaskObj())
  {
    for(TBeing *owner = character_list; owner; owner = owner->next)
    {
      if (!owner->task || owner->task->obj != this)
        continue;

      vlogf(LOG_MISC, format("Cancelling task from ~TObj, object %s is being destroyed and is tasked by %s") %  getName() % owner->getName());
      owner->stopTask();
    }
  }

  object_list.remove(this);

  //  object_list.erase(find(object_list.begin(), object_list.end(), this));
  //  object_list.remove(this);

  if (number >= 0) {
    mud_assert(number < (signed int) obj_index.size(), "~TObj: range (%d) beyond obj_index size (%d).  obj=[%s]", number, obj_index.size(), name.c_str());
    obj_index[number].addToNumber(-1);
  }
  
  objCount--;


  // if thing is using shared sstrings, temporarily assign it new sstrings
  // so that TThing delete can clean up without problem
  // Also, must use number and not objVnum
  if (!isObjStat(ITEM_STRUNG) && (number != -1)) {
    name = name;
    setDescr(getDescr());
    shortDescr = shortDescr;
    action_description = action_description;

    if (ex_description)
      ex_description = new extraDescription(*ex_description);
    else
      ex_description = NULL;
  }
  action_description = "";

  delete [] owners;
  owners = NULL;


}

TRoom::TRoom(int r) :
  TThing(),
  sectorType(MIN_SECTOR_TYPE), 
  riverDir(DIR_NONE),
  riverSpeed(0),
  hasWindow(FALSE), 
  teleLook(0),
  zone(NULL),
  teleTime(0),
  teleTarg(0),
  moblim(0),
  roomHeight(-1),
  roomFlags(0),
  descPos(-1),
  fished(0),
  logsHarvested(0),
  treetype(0),
  tBornInsideMe(NULL)
{
  spec=0;
  number = in_room = r;

  for (dirTypeT i = MIN_DIR; i < MAX_DIR; i++)
    dir_option[i] = NULL;
}

TRoom::~TRoom()
{
  TThing *t;

  // Burn the born list.
  for (t = tBornInsideMe; t; t = t->nextBorn) {
    TMonster *tMonster = dynamic_cast<TMonster *>(t);

    if (tMonster)
      tMonster->brtRoom = Room::NOWHERE;
  }

  for(StuffIter it=stuff.begin();it!=stuff.end();){
    t=*(it++);
    if (t->isPc()) {
      vlogf(LOG_BUG, "~TRoom() with room occupied by PC()");
      continue;
    }
    delete t;
    t = NULL;
  }

  // remove room from specials vector
  unsigned int tri;
  for (tri = 0; tri < roomspec_db.size(); tri++) {
    if (roomspec_db[tri] == this) {
      roomspec_db.erase(roomspec_db.begin() + tri);
      tri--; // backup so we don't skip any
    }
  }

  // A whacky what-if contingency thing
  // save rooms 100 200
  // in clean state, goto 100 (creates room 100, puts in db)
  // rload 150 200
  // RoomLoad sees room 100, and to advance file pointer, reads the data, then
  // deletes room.  We don't want the delete of that dummy room to remove
  // the real (goto 100) room we created.
  // I cleaned up RoomLoad to fix this problem, but I figure this check is
  // a what-if for ther future.  Bat 5/30/99
  if (room_db[in_room] != this)
    vlogf(LOG_BUG, "TRoom dtor removing room from database that isn't in the database");
  else
    room_db[in_room] = NULL;

  dirTypeT i;
  for (i = MIN_DIR; i < MAX_DIR; i++) {
    delete dir_option[i];
    dir_option[i] = NULL;
  }
}

TThing& TObj::operator += (TThing& t) 
{
  TThing::operator += (t);

  // Thing being put in is a TObj
  stuff.push_front(&t);
  t.parent = this;

  return *this;
}


// tables dont hold stuff, they just 'mount' it
TThing& TTable::operator += (TThing& t) 
{
  if (t.parent)
    --t;
  t.mount(this);

  vlogf(LOG_LOW, format("Object (%s) put onto table (%s) using += and not putSomethingIntoTable") % t.getName() % getName());

  return *this;
}


// if tPreserve is set true we don't vlogf nor do we add to the 'owners'
// list.  This is primarly used to catch portal cheaters.
bool TObj::checkOwnersList(const TPerson *ch, bool tPreserve)
{

  const char * tmpbuf = owners;
  char indiv[256];
  bool iHaveOwned = false,
       isCheat = false;

  while (tmpbuf && *tmpbuf) {
    tmpbuf = one_argument(tmpbuf, indiv, cElements(indiv));
    if (!*indiv)
      continue;
    
    // don't bother to check if it got given to myself
    if (!strcmp(indiv, ch->getName().c_str())) {
      iHaveOwned = true;
      continue;
    }

    if (ch->hasWizPower(POWER_WIZARD))
      continue;
    
    charFile st;
    if (!load_char(indiv, &st))
      continue;
    
    if (ch->desc && ch->desc->account && 
	!strcmp(ch->desc->account->name.c_str(), st.aname)) {
      isCheat = true;      
    }
  }

  if (!tPreserve && !iHaveOwned) {
    sstring tmp("");

    if (owners) {
      tmp  = owners;
      tmp += " ";
      delete [] owners;
    }

    tmp += ch->getName();
    owners = mud_str_dup(tmp);
  }

  // check contents too
  TThing *t=NULL;
  for(StuffIter it=stuff.begin();it!=stuff.end() && (t=*it);++it) {
    TObj * obj = dynamic_cast<TObj *>(t);
    if (obj)
      obj->checkOwnersList(ch, tPreserve);
  }

  return isCheat;
}

TThing& TPerson::operator += (TThing& t) 
{
  // make recursive
  TBeing::operator += (t);

  TObj *obj=dynamic_cast<TObj *>(&t);
  if(obj)
    obj->checkOwnersList(this);

  return *this;
}

TThing& TBeing::operator += (TThing& t) 
{
  // make recursive
  TThing::operator += (t);

#if 0
  // Thing being put into is a TBeing
  if (dynamic_cast<TBeing *>(&t)) {
    // Thing being put in is a TBeing - Russ 07/01/96
    vlogf(LOG_BUG, format("warning, Being put into Being :%s into %s.") %  t.getName() % getName());
  }
#endif

  stuff.push_front(&t);
  t.parent = this;

  return *this;
}

TThing& TThing::operator += (TThing& t) 
{
  // I commented these out. We can log later if we want, but there is really
  // no need to assert and abort the MUD when these things happen.
  // I put these back in.  The only place they ever happen is 1.) mud memory is
  // totally corrupt (reboot needed), or 2.) some badly designed piece of new
  // code which needs to be trapped and fixed (hopefully while still on beta)
  // basic security checks - verify out of everything

  TRoom *rp2 = dynamic_cast<TRoom *>(&t);
  if (rp2)
    vlogf(LOG_BUG, "Operator += trying to put a room somewhere");

  mud_assert(t.parent == NULL, ((sstring)(format("TThing += : t.parent existed: %s") % (!t.name.empty() ? t.name.c_str() : "null"))).c_str());
  mud_assert(t.equippedBy == NULL, "TThing += : t.equippedBy existed");
  mud_assert(t.stuckIn == NULL, "TThing += : t.stuckIn existed");
  mud_assert(t.roomp == NULL, "TThing += : t.roomp existed");
  //
  mud_assert(((t.inRoom() == Room::VOID) || (t.inRoom() == Room::NOWHERE) ||
              (t.inRoom() == Room::AUTO_RENT)),
      "TThing += with t.inRoom()");

  TMergeable *tm=dynamic_cast<TMergeable *>(&t);
  if(tm){
    for(StuffIter it=stuff.begin();it!=stuff.end();++it){
      TMergeable *tMerge=dynamic_cast<TMergeable *>(*it);
      
      if(tMerge && tm!=tMerge && tm->willMerge(tMerge)){
	tm->doMerge(tMerge);
	break;
      }
    }
  }

  return *this;
}

TThing& TRoom::operator += (TThing& t) 
{
  // make recursive
  TThing::operator += (t);

  // Thing put into is a TRoom
  // jesus
  // obj to room
  // char to room
  stuff.push_front(&t);
  t.in_room = in_room;
  t.roomp = this;
    
  addToLight(t.getLight());

  TSeeThru *tst = dynamic_cast<TSeeThru *>(&t);
  if (tst && tst->givesOutsideLight()) {
    int best=0, curr = 0;
    TThing *i=NULL;
    for(StuffIter it=stuff.begin();it!=stuff.end() && (i=*it);++it) {
      TSeeThru *tst2 = dynamic_cast<TSeeThru *>(i);
      if (tst2 && (tst2 != tst) && tst2->givesOutsideLight()) {
        curr = tst2->getLightFromOutside();
        if (curr > best)
          best = curr;
      }
    }
    curr = tst->getLightFromOutside();
    if (curr > best) {
      // light must increase by curr-best 
      addToLight(curr-best);
    }
    incrementWindow();
  }

  if (dynamic_cast<TObj *>(&t) && !dynamic_cast<TObj *>(&t)->isObjStat(ITEM_NORENT) && isRoomFlag(ROOM_SAVE_ROOM))
    saveItems("");

  if (t.isPc() && ( getZoneNum() >= 0 && getZoneNum() < ((signed int) zone_table.size()) )) {
    zoneData &zd = zone_table[getZoneNum()];
    if ((zd.zone_value == 0) &&
         zd.enabled) {
      zd.zone_value = ZONE_MAX_TIME;
      // init to non-zero before resetting so mobs load
      zone_table[getZoneNum()].resetZone(FALSE);
    }
    if (zd.zone_value >= 0)
      zd.zone_value = ZONE_MAX_TIME;
  } else if (getZoneNum() >= 0 && getZoneNum() < ((signed int) zone_table.size())) {
#if 1
    if (zone_table[getZoneNum()].enabled) {
      TThing *tThing   = NULL,
	*tObj     = NULL;
      TBeing *tBeing   = NULL;
      TObj   *tObjTemp, *tObjTemp2 = NULL;

      if ((tBeing = dynamic_cast<TBeing *>((tThing = &t)))){
        for (wearSlotT wearIndex = MIN_WEAR; wearIndex < MAX_WEAR; wearIndex++){
          if ((tObjTemp = dynamic_cast<TObj *>(tBeing->equipment[wearIndex]))) {
            if (tObjTemp->isObjStat(ITEM_PROTOTYPE)) {
              tBeing->unequip(wearIndex);
              delete tObjTemp;
              tObjTemp = NULL;
            }

            if (!tObjTemp)
              continue;

            for(StuffIter it=tObjTemp->stuff.begin();it!=tObjTemp->stuff.end();){
              tObj=*(it++);

              if ((tObjTemp2 = dynamic_cast<TObj *>(tObj)) &&
                  tObjTemp2->isObjStat(ITEM_PROTOTYPE)) {
                --(*tObj);
                delete tObj;
                tObj = NULL;
              }
            }
          }
	}
      }
      
      for(StuffIter it=t.stuff.begin();it!=t.stuff.end();){
        tObj=*(it++);
	
        if ((tObjTemp = dynamic_cast<TObj *>(tObj)) &&
            tObjTemp->isObjStat(ITEM_PROTOTYPE)) {
          --(*tObj);
          delete tObj;
          tObj = NULL;
        }

        if (!tObjTemp)
          continue;

        for(StuffIter it=tObjTemp->stuff.begin();it!=tObjTemp->stuff.end();){
          tThing=*(it++);

          if ((tObjTemp2 = dynamic_cast<TObj *>(tThing)) &&
              tObjTemp2->isObjStat(ITEM_PROTOTYPE)) {
            --(*tThing);
            delete tThing;
            tThing = NULL;
          }
        }
      }
    }
#endif
  }
  return *this;
}

TThing& TBeing::operator -- ()
{
  // need to find out how critters leave the BJ table without freeing it
  if (checkBlackjack()) {
    extern BjGame gBj;
    if (gBj.index(this) >= 0)
      vlogf(LOG_BUG, "forced crash: blackjack");
  }

  return TThing::operator-- ();
}

TThing& TThing::operator -- ()
{
  TThing *tmp = NULL, *t_in = NULL;
  TRoom *rp = NULL;
  int light_mod = 0;

  mud_assert(equippedBy == NULL, "TThing -- : equippedBy existed");
  mud_assert(stuckIn == NULL, "TThing -- : stuckIn existed");

  if ((t_in = parent)) {
    // obj from char
    // obj from obj
    mud_assert(!t_in->stuff.empty(), "TThing -- : parent had no stuff");
    mud_assert(roomp == NULL, "TThing -- : had roomp and parent simultaneously");
    mud_assert(inRoom() == Room::NOWHERE || inRoom() == Room::AUTO_RENT, 
            "TThing -- : had parent and in room simultaneously");

    if (t_in->stuff.front() == this)       
      t_in->stuff.pop_front();
    else {
      t_in->stuff.remove(this);
    }
    for (tmp = t_in, light_mod = 0; tmp; tmp = tmp->parent) {
      tmp->addToLight(-light_mod);

      // subtract light from corpses and notify things above to also reduce
      if (dynamic_cast<TBaseCorpse *>(tmp)) 
        light_mod += getLight();

      tmp->addToLight(-light_mod);
    }

    if(dynamic_cast<TToothNecklace *>(t_in)){
      dynamic_cast<TToothNecklace *>(t_in)->updateDesc();
    }

    if (t_in->roomp &&
	t_in->roomp->isRoomFlag(ROOM_SAVE_ROOM)){
      roomsave_db.push_back(t_in->roomp);
    }

  } else if ((rp = dynamic_cast<TRoom *> (roomp))) {
    // obj from room
    // char from room

    if (this == rp->stuff.front())   // head of list 
      rp->stuff.pop_front();
    else {
      rp->stuff.remove(this);
    }

    if(tied_to){
      tied_to->tied_to=NULL;
      tied_to=NULL;
    }

    // adjust room light levels 
    rp->addToLight(-getLight());

    // if this object was the last 'window' in the room providing outside
    // light, be sure we decrement the rooms light value accordingly.     
    // these should only apply to objs, beings don't give light from outside
    TSeeThru *tst = dynamic_cast<TSeeThru *>(this);
    if (tst && tst->givesOutsideLight()) {
      int best=0, curr = 0;
      for(StuffIter it=rp->stuff.begin();it!=rp->stuff.end() && (tmp=*it);++it) {
        TSeeThru *tst2 = dynamic_cast<TSeeThru *>(tmp);
        if (tst2 && tst2->givesOutsideLight()) {
          curr = tst2->getLightFromOutside();
          if (curr > best)
            best = curr;
        }
      }
      curr = tst->getLightFromOutside();

      // light must dim by curr-best 
      if (curr > best) 
        rp->addToLight(-(curr-best));
      
      rp->decrementWindow();
    }
  
    if (dynamic_cast<TObj *>(this) &&
        !dynamic_cast<TObj *>(this)->isObjStat(ITEM_NORENT) &&
        rp->isRoomFlag(ROOM_SAVE_ROOM))
      rp->saveItems("");

  } else {
    // guaranteed crash
    vlogf_trace(LOG_BUG, format("Bogus call to TThing operator--, %s") %  getName());
  }

  // set the obj 
  equippedBy = NULL;
  stuckIn = NULL;
  parent = NULL;
  in_room = Room::NOWHERE;
  roomp = NULL;

  return *this;
}

TPerson::TPerson(Descriptor *thedesc) :
  TBeing(),
  base_age(0),
  tLogFile(NULL),
  title(NULL), 
  timer(0)
{
  *lastHost = '\0';
  memset(toggles, 0, sizeof(toggles));
  memset(wizPowers, 0, sizeof(wizPowers));
  memset(wizPowersOriginal, 0, sizeof(wizPowersOriginal));

  desc = thedesc;

  // default case for TBeing is to add to mobCount, don't count pc's as mobs
  mobCount--;

  // this resets some values
  // rememebr that it could init some desc stuff, then have new char
  // come in causing bad settings.
  desc->session.setToZero();
  desc->prompt_d.xptnl = 0;

  AccountStats::player_num++;
  AccountStats::max_player_since_reboot = max(AccountStats::max_player_since_reboot, AccountStats::player_num);
}

TPerson::TPerson(const TPerson &a) :
  TBeing(a),
  base_age(a.base_age),
  tLogFile(a.tLogFile),
  timer(a.timer)
{
  title = mud_str_dup(a.title);
  strcpy(lastHost, a.lastHost);
  memcpy(toggles, a.toggles, sizeof(toggles));
  memcpy(wizPowers, a.wizPowers, sizeof(wizPowers));
  memcpy(wizPowersOriginal, a.wizPowers, sizeof(wizPowersOriginal));

  AccountStats::player_num++;
  AccountStats::max_player_since_reboot = max(AccountStats::max_player_since_reboot, AccountStats::player_num);
}

TPerson & TPerson::operator=(const TPerson &a)
{
  if (this == &a) return *this;  
  TBeing::operator=(a);
  base_age = a.base_age;
  timer = a.timer;

  delete [] title;
  title = mud_str_dup(a.title);

  strcpy(lastHost, a.lastHost);
  memcpy(toggles, a.toggles, sizeof(toggles));
  memcpy(wizPowers, a.wizPowers, sizeof(wizPowers));
  memcpy(wizPowersOriginal, a.wizPowers, sizeof(wizPowersOriginal));
  return *this;
}

TPerson::~TPerson()
{
  Descriptor *t_desc;

  if (!desc) {
    for (t_desc = descriptor_list; t_desc; t_desc = t_desc->next) {
      if (t_desc->original && t_desc->original == this) {
        t_desc->character->remQuestBit(TOG_TRANSFORMED_LYCANTHROPE);
        t_desc->character->doReturn("", WEAR_NOWHERE, true);
      }
    }
  }

  setInvisLevel(MAX_IMMORT+1);
  fixClientPlayerLists(TRUE);

  // We use to let this be a handler for quit
  // however, if we accidentally delete a player (bad return code?)
  // this gets called and duplicates items
  // quit should now have similar code to what was here, so regard 
  // getting here as an error.
  dropItemsToRoom(SAFE_NO, DROP_IN_ROOM);

  AccountStats::player_num--;

  delete [] title;
  title = NULL;

  if (tLogFile) {
    logf("Logging out...");
    fclose(tLogFile);
    tLogFile = NULL;
  }
}

TThing::~TThing() 
{
  extraDescription *exd = NULL, *next_one = NULL;

  for (exd = ex_description; exd; exd = next_one) {
    next_one = exd->next;
    delete exd; // extraDesc desctructor takes care of freeing everything
  }
  ex_description = NULL;

  if (getCaster()) {
    remCastingList(this);
    setCaster(NULL);
  }

  if(tied_to){
    tied_to->tied_to=NULL;
    tied_to=NULL;
  }

  if (act_ptr) {
    vlogf(LOG_BUG, format("Memory leaked: act_ptr on %s") %  getName()); 
#if 0
    delete act_ptr;
    act_ptr = NULL;
#endif
  }
  name = "";
  shortDescr = "";;
  descr = "";
}

TThing::TThing() :
  weight(0.0),
  light(0),
  material_type(MAT_UNDEFINED),
  carried_weight(0.0),
  carried_volume(0), 
  the_caster(NULL),
  descr(NULL),
  stuckIn(NULL),
  equippedBy(NULL),
  tied_to(NULL),
  eq_pos(WEAR_NOWHERE),
  eq_stuck(WEAR_NOWHERE),
  act_ptr(NULL),
  max_exist(9999),
  in_room(Room::NOWHERE),
  spec(0),
  snum(-1),
  number(0), 
  height(0),
  canBeSeen(0),
  name(NULL),
  shortDescr(NULL),
  parent(NULL),
  nextBorn(NULL),
  roomp(NULL),
  desc(NULL), 
  ex_description(NULL),
  rider(NULL),
  riding(NULL),
  nextRider(NULL)
{
}

/*
A NOTE ON compareWeights

  In converting a number from decimal into its binary notation, there is an
  inherent imprecision.  For variables of type float, this imprecision is
  typically on the order of 1.27E-7.  For instance 0.1 might actually be
  stored as 0.0999997, or as 0.100003

  Since we sometimes want to compare two floats and don't want the imprecision
  of a float to affect the comparison, we do this.

  In essence, we first add a small amount (.2) beyond the precision we care
  about so that if the imprecision is low, we round the number up.  eg,
  if we want precisoion out to 0.01, we add .002 to all values.
  
  Next, multiply the result by a power of 10 sufficient to move the precision
  to the LHS of the decimal point.  eg, precision to the hundredths place means
  multiply by 100.

  convert to an int.  This truncates everything out of range of the
  precision we care about.

  Do the comparison.  Since we are comparing 2 integers, everything should be
  straight forward now that we've massaged the data.
*/

// returns -1 if x1 > x2
// returns 0 if x1 == x2
// returns 1 if x1 < x2
int compareWeights(const float x1, const float x2)
{
  float a, b;

  a = 1000.0 * (x1 + .0002);
  b = 1000.0 * (x2 + .0002);
  // int x = (int) a;
  // int y = (int) b;
  
  if (a > b)
    return -1;
  else if (a < b)
    return 1;
  return 0;
}

specialData::specialData() :
  fighting(NULL),
  hunting(NULL),
  affectedBy(0),
  position(POSITION_STANDING),
  last_direction(DIR_NONE),
  edit(MAIN_MENU),
  editFriend(0),
  act(0),
  was_in_room(Room::NOWHERE),
  zone(-1)
{
  for(int i=0;i<MAX_COND_TYPE;++i)
    conditions[i]=0;
}

specialData::specialData(const specialData &a) :
  fighting(a.fighting),
  hunting(a.hunting),
  affectedBy(a.affectedBy),
  position(a.position),
  last_direction(a.last_direction),
  edit(a.edit),
  editFriend(a.editFriend),
  act(a.act),
  was_in_room(a.was_in_room),
  zone(a.zone)
{
  for(int i=0;i<MAX_COND_TYPE;++i)
    conditions[i]=a.conditions[i];
}

specialData & specialData::operator=(const specialData &a)
{
  if (this == &a) return *this;

  fighting = a.fighting; 
  hunting = a.hunting;
  was_in_room = a.was_in_room;
  position = a.position;
  edit = a.edit;
  editFriend = a.editFriend;
  last_direction = a.last_direction;
  for(int i=0;i<MAX_COND_TYPE;++i)
    conditions[i]=a.conditions[i];
  act = a.act;
  zone = a.zone;
  affectedBy = a.affectedBy;
  return *this;
}

specialData::~specialData()
{
}

void TThing::mount(TThing *ch)
{
  if (riding) {
    vlogf(LOG_BUG, format("%s already riding in call to mount()") %  getName());
    return;
  }
  // update linked list info
  nextRider = ch->rider;
  ch->rider = this;
  riding = ch;

  // lamp on a table ought to contribute to room's light
  TTable *ttab = dynamic_cast<TTable *>(ch);
  if (ttab) {
    if (ttab->roomp)
      ttab->roomp->addToLight(getLight());
    else if (ttab->parent && dynamic_cast<TBeing *>(ttab->parent)) {
      // damn gods screwing around!
      // light on a table held by a person.  Do nothing for this case
      vlogf(LOG_BUG, format("Possible lighting error due to table being mounted in bad state.  (Room=%d, heldBy=%s)") %  
              ttab->parent->inRoom() % ttab->parent->getName());
    } else 
      vlogf(LOG_BUG, "Potential lighting screw up involving tables.");
  }
}

TBeing::TBeing(const TBeing &a) :
  TThing(a),
  race(a.race),
  points(a.points),
  chosenStats(a.chosenStats), 
  curStats(a.curStats),
  multAtt(a.multAtt),
  heroNum(a.heroNum),
  m_craps(a.m_craps),
  invisLevel(a.invisLevel),
  my_protection(a.my_protection),
  combatMode(a.combatMode),
  my_garbleFlags(a.my_garbleFlags),
  faction(a.faction),
  inPraying(a.inPraying),
  inQuaffUse(a.inQuaffUse),
  attackers(a.attackers),
  visionBonus(a.visionBonus),
  age_mod(a.age_mod),
  cantHit(a.cantHit),
  wait(a.wait),
  polyed(a.polyed),
  hunt_dist(a.hunt_dist),
  wimpy(a.wimpy),
  delaySave(a.delaySave),
  immunities(a.immunities),
  player(a.player),
  specials(a.specials),
  practices(a.practices),
  equipment(a.equipment),
  master(a.master),
  orig(a.orig),
  next_fighting(a.next_fighting),
  next_caster(a.next_caster)
{
  int i;

  // this is not heavily tested
  // I didn't see how it would be used, so had trouble deciding how some things
  // would get set
  // pay attention to next, next_fighting, next_caster, etc

  race = a.race;

  for (i = 1; i< MAX_TOG_INDEX; i++) {
    toggles[i] = a.toggles[i];
  }

  for (i = MIN_WEAR; i < MAX_WEAR; i++) {
    body_parts[i] = a.body_parts[i];
  }

  if (a.discs)
    discs = new CMasterDiscipline(*a.discs);
  else
    discs = NULL;

  if (a.affected)
    affected = new affectedData(*a.affected);
  else
    affected = NULL;

  if (a.followers)
    followers = new followData(*a.followers);
  else
    followers = NULL;

  if (a.task)
    task = new taskData(*a.task);
  else
    task = NULL;

  if (a.spelltask)
    spelltask = new spellTaskData(*a.spelltask);
  else
    spelltask = NULL;


  if (IS_SET(a.specials.act,ACT_STRINGS_CHANGED)) {
    name = a.name;
    shortDescr = a.shortDescr;
    player.longDescr = a.getLongDesc();
    setDescr(a.getDescr());

    if (ex_description)
      ex_description = new extraDescription(*a.ex_description);
    else
      ex_description = NULL;
  }
  mobCount++;

  trophy = new TTrophy(this);
}

TBeing & TBeing::operator=(const TBeing &a)
{
  if (this == &a) return *this;

  TThing::operator=(a);

  int i;

  race = a.race;

  points = a.points;
  chosenStats = a.chosenStats;
  curStats = a.curStats;

  multAtt = a.multAtt; 
  faction = a.faction;
  heroNum = a.heroNum;
  m_craps = a.m_craps;
  invisLevel = a.invisLevel;
  my_protection = a.my_protection;
  my_garbleFlags = a.my_garbleFlags;
  equipment = a.equipment;

  for (i = 1; i< MAX_TOG_INDEX; i++) {
    toggles[i] = a.toggles[i];
  }

  for (i = MIN_WEAR; i < MAX_WEAR; i++) {
    body_parts[i] = a.body_parts[i];
  }

  if (a.discs)
    discs = new CMasterDiscipline(*a.discs);
  else
    discs = NULL;

  inPraying = a.inPraying;
  inQuaffUse = a.inQuaffUse;
  attackers = a.attackers;
  visionBonus = a.visionBonus;
  age_mod = a.age_mod;
  cantHit = a.cantHit;
  wait = a.wait;
  combatMode = a.combatMode;
  polyed = a.polyed;
  hunt_dist = a.hunt_dist;
  wimpy = a.wimpy;
  delaySave = a.delaySave;
  immunities = a.immunities;

  // if player.longDescr is not shared, it has to be deleted
  // player operator= will member copy, not allocate
  if (IS_SET(specials.act,ACT_STRINGS_CHANGED)) {
    player.longDescr = "";
  }
  player = a.player;

  specials = a.specials;
  practices = a.practices;
  if (a.affected)
    affected = new affectedData(*a.affected);
  else
    affected = NULL;

  master = a.master; 
  orig = a.orig;
  next = a.next;
  next_fighting = a.next_fighting;
  next_caster = a.next_caster;

  if (a.followers)
    followers = new followData(*a.followers);
  else
    followers = NULL;

  if (a.task)
    task = new taskData(*a.task);
  else
    task = NULL;

  if (a.spelltask)
    spelltask = new spellTaskData(*a.spelltask);
  else
    spelltask = NULL;

  if (IS_SET(a.specials.act,ACT_STRINGS_CHANGED)) {
    name = a.name;
    shortDescr = a.shortDescr;
    player.longDescr = a.player.longDescr;
    setDescr(a.getDescr());

    if (a.ex_description)
      ex_description = new extraDescription(*a.ex_description);
    else
      ex_description = NULL;
  }

  return *this;
}

TThing::TThing(const TThing &a) :
  weight(a.weight), light(a.light), material_type(a.material_type),
    carried_weight(a.carried_weight), carried_volume(a.carried_volume),
    the_caster(a.the_caster),
    stuff(a.stuff),
    descr(a.descr),
    stuckIn(a.stuckIn),
    equippedBy(a.equippedBy),
    tied_to(a.tied_to),
    eq_pos(a.eq_pos), 
    eq_stuck(a.eq_stuck), act_ptr(a.act_ptr),
    max_exist(a.max_exist), in_room(a.in_room), spec(a.spec),
    number(a.number), height(a.height),
    canBeSeen(a.canBeSeen), name(a.name), shortDescr(a.shortDescr),
    parent(a.parent), 
    nextBorn(a.nextBorn),
    roomp(a.roomp), desc(a.desc), 
    ex_description(a.ex_description),
    rider(a.rider), riding(a.riding),
    nextRider(a.nextRider)
{
  // default will be to member copy the text fields
  // will have to reallocate where appropriate
}

TThing & TThing::operator=(const TThing &a)
{
  if (this == &a) return *this;

  max_exist = a.max_exist;
  light = a.light;
  material_type = a.material_type;
  spec = a.spec;
  // default will be to member copy the text fields
  // will have to reallocate where appropriate
  name = a.name;
  shortDescr = a.shortDescr;
  descr = a.descr;
  ex_description = a.ex_description;
  eq_pos = a.eq_pos;
  eq_stuck = a.eq_stuck;
  number = a.number;
  height = a.height;
  weight = a.weight;
  canBeSeen = a.canBeSeen;
  the_caster = a.the_caster;
  stuckIn = a.stuckIn;
  equippedBy = a.equippedBy;
  tied_to = a.tied_to;
  nextBorn = a.nextBorn;
  stuff = a.stuff;
  parent = a.parent;
  desc = a.desc;
  roomp = a.roomp;
  riding = a.riding;
  rider = a.rider;
  nextRider = a.nextRider;
  carried_weight = a.carried_weight;
  carried_volume = a.carried_volume;
  act_ptr = a.act_ptr;
  in_room = a.in_room;
  return *this;
}

TObj::TObj(const TObj &a) :
  TThing(a),
  obj_flags(a.obj_flags),
  isTasked(a.isTasked),
  isLocked(a.isLocked)
{
  int i;

  for (i = 0; i < MAX_OBJ_AFFECT; i++) 
    affected[i] = a.affected[i];
  
  if (a.isObjStat(ITEM_STRUNG) || (a.number == -1)) {
    name = a.name;
    shortDescr = a.shortDescr;
    setDescr(a.getDescr());
    action_description = a.action_description;

    if (a.ex_description)
      ex_description = new extraDescription(*a.ex_description);
    else
      ex_description = NULL;
  } else 
    action_description = a.action_description;

  objCount++;
  object_list.push_front(this);

  owners = mud_str_dup(a.owners);
}

TObj & TObj::operator= (const TObj &a)
{
  if (this == &a) return *this;

  TThing::operator=(a);

  int i;

  obj_flags = a.obj_flags;

  for (i = 0; i < MAX_OBJ_AFFECT; i++) {
    affected[i] = a.affected[i];
  }

  // duplicate necessary sstrings
  if (a.isObjStat(ITEM_STRUNG) || (a.number == -1)) {
    name = a.name;
    shortDescr = a.shortDescr;
    setDescr(a.getDescr());
    action_description = a.action_description;

    if (a.ex_description)
      ex_description = new extraDescription(*a.ex_description);
    else
      ex_description = NULL;
  } else {
    // this is only sstring that is obj specific, others got assigned by TThing
    action_description = a.action_description;
  }

  // note, we do NOT assign next pointer since this thing should already be
  // in the object list

  owners = mud_str_dup(a.owners);
  isTasked = a.isTasked;
  isLocked = a.isLocked;

  return *this;
}

extraDescription::extraDescription() :
  keyword(NULL),
  description(NULL),
  next(NULL)
{
}

extraDescription::~extraDescription()
{
}

extraDescription & extraDescription::operator= (const extraDescription &a)
{
  if (this == &a)
    return *this;

  keyword = a.keyword;
  description = a.description;

  extraDescription *ad, *ad2;
  for (ad = next; ad; ad = ad2) {
    ad2 = ad->next;
    delete ad;
  }
  if (a.next)
    next = new extraDescription(*a.next);
  else
    next = NULL;

  return *this;
}

extraDescription::extraDescription(const extraDescription &a)
{
  keyword = a.keyword;
  description = a.description;
  if (a.next)
    next = new extraDescription(*a.next);
  else
    next = NULL;
}

objFlagData::objFlagData() :
  extra_flags(0),
  depreciation(0),
  wear_flags(0),
  cost(0),
  bitvector(0),
  decay_time(0),
  struct_points(0),
  max_struct_points(0),
  volume(0)
{
}

objFlagData::objFlagData(const objFlagData &a) :
  extra_flags(a.extra_flags),
  depreciation(a.depreciation),
  wear_flags(a.wear_flags),
  cost(a.cost),
  bitvector(a.bitvector),
  decay_time(a.decay_time),
  struct_points(a.struct_points),
  max_struct_points(a.max_struct_points),
  volume(a.volume)
{
}

objFlagData & objFlagData::operator=(const objFlagData &a)
{
  if (this == &a) return *this;

  wear_flags = a.wear_flags;
  cost = a.cost;
  volume = a.volume;
  extra_flags = a.extra_flags;
  bitvector = a.bitvector;
  decay_time = a.decay_time;
  struct_points = a.struct_points;
  max_struct_points = a.max_struct_points;
  depreciation = a.depreciation;

  return *this;
}

objFlagData::~objFlagData()
{
}


snoopData::snoopData()
  : snooping(NULL), snoop_by(NULL)
{
}

snoopData::snoopData(const snoopData &a)
  : snooping(a.snooping), snoop_by(a.snoop_by)
{
}

snoopData & snoopData::operator=(const snoopData &a)
{
  if (this == &a) return *this;
  snooping = a.snooping;
  snoop_by = a.snoop_by;
  return *this;
}

snoopData::~snoopData()
{
}

betData::betData()
  : come(0), crap(0), slot(0),
    eleven(0), twelve(0), two(0),
    three(0), horn_bet(0), field_bet(0),
    hard_eight(0), hard_six(0), hard_ten(0),
    hard_four(0), seven(0), one_craps(0)
{
}

betData::betData(const betData &a)
  : come(a.come), crap(a.crap), slot(a.slot),
    eleven(a.eleven), twelve(a.twelve), two(a.two),
    three(a.three), horn_bet(a.horn_bet), field_bet(a.field_bet),
    hard_eight(a.hard_eight), hard_six(a.hard_six), hard_ten(a.hard_ten),
    hard_four(a.hard_four), seven(a.seven), one_craps(a.one_craps)
{
}

betData & betData::operator=(const betData &a)
{
  if (this == &a) return *this;
  come = a.come;
  crap = a.crap;
  slot = a.slot;
  eleven = a.eleven;
  twelve = a.twelve;
  two = a.two;
  three = a.three;
  horn_bet = a.horn_bet;
  field_bet = a.field_bet;
  hard_eight = a.hard_eight;
  hard_six = a.hard_six;
  hard_ten = a.hard_ten;
  hard_four = a.hard_four;
  seven = a.seven;
  one_craps = a.one_craps;
  return *this;
}

betData::~betData()
{
}

cBetData::cBetData()
  : crapsOptions(0), oneRoll(0), roulOptions(0)
{
}

cBetData::cBetData(const cBetData &a)
  : crapsOptions(a.crapsOptions), 
    oneRoll(a.oneRoll), 
    roulOptions(a.roulOptions)
{
}

cBetData & cBetData::operator=(const cBetData &a)
{
  if (this == &a) return *this;
  crapsOptions = a.crapsOptions;
  oneRoll = a.oneRoll;
  roulOptions = a.roulOptions;
  return *this;
}

cBetData::~cBetData()
{
}

lastChangeData::lastChangeData()
  : hit(0), mana(0), move(0),
    piety(0.0), money(0), exp(0),
    room(0), perc(-1.0), mudtime(-1),
    fighting(0), full(0), thirst(0), pos(0)
{
  time_t t1;
  t1 = time(0);
  minute = localtime(&t1)->tm_min;
}

lastChangeData::lastChangeData(const lastChangeData &a)
  : hit(a.hit), mana(a.mana), move(a.move),
    piety(a.piety), money(a.money), exp(a.exp),
    room(a.room), perc(a.perc), mudtime(a.mudtime),
    minute(a.minute),
    fighting(a.fighting), full(a.full), thirst(a.thirst), pos(a.pos)
{
}

lastChangeData::~lastChangeData()
{
}

objAffData::objAffData() : 
  type(TYPE_UNDEFINED),
  level(0),
  duration(0),
  renew(0),
  location(APPLY_NONE),
  modifier(0),
  modifier2(0),
  bitvector(0)
{
}

objAffData::objAffData(const objAffData &a) :
  type(a.type),
  level(a.level),
  duration(a.duration),
  renew(a.renew),
  location(a.location),
  modifier(a.modifier),
  modifier2(a.modifier2),
  bitvector(a.bitvector)
{
}

objAffData & objAffData::operator=(const objAffData &a)
{
  if (this == &a) return *this;
  type = a.type;
  level = a.level;
  duration = a.duration;
  renew = a.renew;
  location = a.location;
  modifier = a.modifier;
  modifier2 = a.modifier2;
  bitvector = a.bitvector;
  return *this;
}

objAffData::~objAffData()
{
}

roomDirData::roomDirData() :
  description(NULL),
  keyword(NULL),
  door_type(DOOR_NONE),
  condition(0),
  lock_difficulty(-1),
  weight(-1),
  trap_info(0),
  trap_dam(0),
  key(0),
  to_room(0)
{
}

roomDirData::roomDirData(const roomDirData &a) :
  description(a.description),
  keyword(a.keyword),
  door_type(a.door_type),
  condition(a.condition),
  lock_difficulty(a.lock_difficulty),
  weight(a.weight),
  trap_info(a.trap_info),
  trap_dam(a.trap_dam),
  key(a.key),
  to_room(a.to_room)
{
}

roomDirData & roomDirData::operator=(const roomDirData &a)
{
  if (this == &a) return *this;
  door_type = a.door_type;
  condition = a.condition;
  lock_difficulty = a.lock_difficulty;
  weight = a.weight;
  trap_info = a.trap_info;
  trap_dam = a.trap_dam;
  key = a.key;
  to_room = a.to_room;
  description = a.description;
  keyword = a.keyword;
  return *this;
}

roomDirData::~roomDirData()
{
}

wizListInfo::wizListInfo()
{
  buf1 = new char[1];
  *buf1 = '\0';
  buf2 = new char[2];
  *buf2 = '\0';
  buf3 = new char[2];
  *buf3 = '\0';
}

wizListInfo::~wizListInfo()
{
  delete [] buf1;
  delete [] buf2;
  delete [] buf3;
}

saveAffectedData::saveAffectedData() :
  type(-1),
  level(0),
  duration(0),
  renew(0),
  modifier(0),
  modifier2(0),
  location(0),
  bitvector(0),
  unused2(NULL)
{
}

saveAffectedData & saveAffectedData::operator=(const affectedData &a)
{
  type = mapSpellnumToFile(a.type);
  level = a.level;
  duration = a.duration;
  renew = a.renew;
  location = mapApplyToFile(a.location);

  if (applyTypeShouldBeSpellnum(a.location))
    modifier = mapSpellnumToFile(spellNumT(a.modifier));
  else
    modifier = a.modifier;

  modifier2 = a.modifier2;
  bitvector = a.bitvector;
  unused2 = NULL;

  return *this;
}

affectedData::affectedData() :
  type(TYPE_UNDEFINED),
  level(0),
  duration(0),
  renew(0),
  modifier(0),
  modifier2(0),
  location(APPLY_NONE),
  bitvector(0),
  be(NULL),
  next(NULL)
{
}

affectedData::affectedData(const affectedData &a) :
  type(a.type),
  level(a.level),
  duration(a.duration),
  renew(a.renew),
  modifier(a.modifier),
  modifier2(a.modifier2),
  location(a.location),
  bitvector(a.bitvector),
  be(a.be)
{
  if (a.next)
    next = new affectedData(*a.next);
  else
    next = NULL;

  if ((type == AFFECT_PET) || 
      (type == AFFECT_CHARM) ||
      (type == AFFECT_THRALL) ||
      (type == AFFECT_ORPHAN_PET) ||
      (type == AFFECT_COMBAT && modifier == COMBAT_RESTRICT_XP)) {
    // this affect has reinterpreted "be" to be a char *
    // and has allocated memory to it.  Member copying is
    // inappropriate for this cast, so...
    be = reinterpret_cast<TThing *>(mud_str_dup((char *) a.be));
  }
}

affectedData::affectedData(const saveAffectedData &a) :
  level(a.level),
  duration(a.duration),
  renew(a.renew),
  modifier2(a.modifier2),
  location(APPLY_NONE),
  bitvector(a.bitvector),
  be(NULL),
  next(NULL)
{
  location = mapFileToApply(a.location);
  if (applyTypeShouldBeSpellnum(location))
    modifier = mapFileToSpellnum(a.modifier);
  else
    modifier = a.modifier;

  type = mapFileToSpellnum(a.type);

  // for AFFECT_PET types, TThing * be should get the owner as a sstring
  // but we didn't save that info, and we lack the info here, we will
  // have to recreate this info elsewhere (pet rentin)
  // Ditto, AFFECT_ORPHAN_PET, AFFECT_CHARM and AFFECT_THRALL
}

affectedData & affectedData::operator=(const affectedData &a)
{
  if (this == &a) return *this;
  type = a.type;
  level = a.level;
  duration = a.duration;
  renew = a.renew;
  modifier = a.modifier;
  modifier2 = a.modifier2;
  location = a.location;
  bitvector = a.bitvector;
  be = a.be;

  if (a.next)
    next = new affectedData(*a.next);
  else
    next = NULL;

  if ((type == AFFECT_PET) || 
      (type == AFFECT_CHARM) ||
      (type == AFFECT_THRALL) ||
      (type == AFFECT_ORPHAN_PET) ||
      (type == AFFECT_COMBAT && modifier == COMBAT_RESTRICT_XP)) {
    // this affect has reinterpreted "be" to be a char *
    // and has allocated memory to it.  Member copying is
    // inappropriate for this cast, so...
    be = reinterpret_cast<TThing *>(mud_str_dup((char *) a.be));
  }

  return *this;
}

affectedData::~affectedData()
{
  // we redefined "TThing * be" to be a "char *" for AFFECT_PET
  // clean up our memory we allocated when we did this
  if ((type == AFFECT_PET) || 
      (type == AFFECT_CHARM) ||
      (type == AFFECT_THRALL) ||
      (type == AFFECT_ORPHAN_PET) ||
      (type == AFFECT_COMBAT && modifier == COMBAT_RESTRICT_XP)) {
    char * tmp = (char *) be;
    be = NULL;
    delete [] tmp;
    tmp = NULL;
  }
}

bool affectedData::canBeRenewed() const
{
  return ((renew >= 0) && (duration <= renew));
}

bool affectedData::shouldGenerateText() const
{
  // since some things set two or more affects with same type
  // and we would only want one "message" (decay/renew) for all of them
  // this decides when it is OK to skip text
  if (next && next->type == type) {
    if (type == AFFECT_DRUG) {
      return (modifier2 != next->modifier2);
    }
    return false;
  }
  return true;
}

const char * extraDescription::findExtraDesc(const char *word)
{
  extraDescription *i;

  for (i = this; i; i = i->next) {
    if (!i->keyword.empty() && isname(word, i->keyword))
      return (i->description.c_str());
  }
  return NULL;
}

