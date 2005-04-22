//////////////////////////////////////////////////////////////////////////
//
// SneezyMUD - All rights reserved, SneezyMUD Coding Team
//
//////////////////////////////////////////////////////////////////////////


// container.cc
//

#include "stdsneezy.h"
#include "obj_base_container.h"
#include "obj_player_corpse.h"
#include "obj_open_container.h"
#include "obj_food.h"
#include "obj_opal.h"

TBaseContainer::TBaseContainer() :
  TObj()
{
}

TBaseContainer::TBaseContainer(const TBaseContainer &a) :
  TObj(a)
{
}

TBaseContainer & TBaseContainer::operator=(const TBaseContainer &a)
{
  if (this == &a) return *this;
  TObj::operator=(a);
  return *this;
}

TBaseContainer::~TBaseContainer()
{
}

bool TBaseContainer::engraveMe(TBeing *ch, TMonster *me, bool give)
{
  char buf[256];

  // engraved bags would protect too many things
  me->doTell(ch->getName(), "The powers that be say I can't do that anymore.");

  if (give) {
    strcpy(buf, name);
    strcpy(buf, add_bars(buf).c_str());
    sprintf(buf + strlen(buf), " %s", fname(ch->name).c_str());
    me->doGive(buf);
  }
  return TRUE;
}

int TBaseContainer::stealModifier()
{
  return 50;   // make bags tough to steal
}

int TBaseContainer::getReducedVolume(const TThing *) const
{
  return getTotalVolume();
}

sstring TBaseContainer::showModifier(showModeT tMode, const TBeing *tBeing) const
{
  sstring tString("");

  // Take 1 higher than the current used and minus 1 from it to get All of the
  // bits set.  From there remove the hold/thrown/take items as we only care
  // about those worn containers.  Ex: Mage Belt
  int    tCanWear = canWear((1 << MAX_ITEM_WEARS) - 1 - ITEM_HOLD - ITEM_THROW - ITEM_TAKE);

  if ((tMode == SHOW_MODE_SHORT_PLUS ||
       tMode == SHOW_MODE_SHORT_PLUS_INV ||
       tMode == SHOW_MODE_SHORT) && tCanWear) {
    tString += " (";
    tString += equip_condition(-1);
    tString += ")";
  }

  return tString;
}

void TBaseContainer::purchaseMe(TBeing *ch, TMonster *tKeeper, int tCost, int tShop)
{
  TObj::purchaseMe(ch, tKeeper, tCost, tShop);
}

void TBaseContainer::examineObj(TBeing *ch) const
{
  int bits = FALSE;

  if (parent && (ch == parent)) {
    bits = FIND_OBJ_INV;
  } else if (equippedBy && (ch == equippedBy)) {
    bits = FIND_OBJ_EQUIP;
  } else if (parent && (ch->roomp == parent)) {
    bits = FIND_OBJ_ROOM;
  }

  ch->sendTo("When you look inside, you see:\n\r");
  lookObj(ch, bits);
}

void TBaseContainer::logMe(const TBeing *ch, const char *cmdbuf) const
{
  TObj::logMe(ch, cmdbuf);

  const char *last = NULL;
  if(getStuff())
    last=getStuff()->getName();
  int runcount=1;
  TThing *t;
  for (t = getStuff(); t; t = t->nextThing, ++runcount) {
    if(!t->nextThing || strcmp(last, t->nextThing->getName())){
      if(runcount>1){
        vlogf(LOG_SILENT, fmt("%s%s%s %s containing %s [%i].") %     
              (ch ? ch->getName() : "") %                      
              (ch ? " " : "") %                                
              cmdbuf % getName() % t->getName() % runcount);     
      } else                                                  
        vlogf(LOG_SILENT, fmt("%s%s%s %s containing %s.") %          
          (ch ? ch->getName() : "") %                          
          (ch ? " " : "") %                                    
          cmdbuf % getName() % t->getName());                   
      runcount=0;                                             
      if(t->nextThing)
        last=t->nextThing->getName();
      else
        last=t->getName();
    } else
      last=t->getName();
  }
}

int TBaseContainer::getAllFrom(TBeing *ch, const char *argument)
{
  int rc;
  TPCorpse * tCorpse;
  TBaseCorpse *corpse;


  if((tCorpse=dynamic_cast<TPCorpse *>(this)) &&
     ((sstring)ch->getName()).lower() == tCorpse->getOwner()){
    // allow loot
  } else if((corpse=dynamic_cast<TBaseCorpse *>(this)) &&
	    corpse->isCorpseFlag(CORPSE_DENY_LOOT) &&
	    !ch->isImmortal()){
    act("Looting $p isn't allowed.",
	TRUE, ch, this, NULL, TO_CHAR);
    return TRUE;
  }


  act("You start getting items from $p.", TRUE, ch, this, NULL, TO_CHAR);
  act("$n starts getting items from $p.", TRUE, ch, this, NULL, TO_ROOM);
  start_task(ch, ch->roomp->getStuff(), ch->roomp, TASK_GET_ALL, argument, 
            350, ch->in_room, 0, 0, 0);



  /*
  if ((tCorpse = dynamic_cast<TPCorpse *>(this)) &&
      !ch->isImmortal() &&
      ch->getName(.lower()) != tCorpse->getOwner()) {
    affectedData tAff;

    tAff.type     = AFFECT_PLAYERLOOT;
    tAff.duration = (24 * UPDATES_PER_MUDHOUR);
    ch->affectJoin(ch, &tAff, AVG_DUR_NO, AVG_EFF_NO);
    vlogf(LOG_CHEAT, fmt("Adding PLoot Flag To: %s (1)") %  ch->getName());
  }
  */

  // this is a kludge, task_get still has a tiny delay on it
  // this dumps around it and goes right to the guts
  rc = (*(tasks[TASK_GET_ALL].taskf))
        (ch, CMD_TASK_CONTINUE, "", 0, ch->roomp, 0);
  if (IS_SET_DELETE(rc, DELETE_THIS))                         
    return DELETE_VICT;                                       
  return FALSE;                                               
}                                                             

int TBaseContainer::getObjFrom(TBeing *ch, const char *arg1, const char *arg2)
{                                                             
  char newarg[100], capbuf[256];
  int rc;
  int p;
  TPCorpse * tCorpse;
  TBaseCorpse *corpse;

  if (getall(arg1, newarg)) {                                 
    if (!searchLinkedListVis(ch, newarg, getStuff())) {            
      ch->sendTo(COLOR_OBJECTS, fmt("There are no \"%s\"'s visible in %s.\n\r") %
               newarg % getName());
      return TRUE;                                            
    }                                                         
    if (ch->getPosition() <= POSITION_SITTING) {              
      ch->sendTo("You need to be standing to do that.\n\r");  
      if (!ch->awake())                                       
        return TRUE;   // sleeping                            
      ch->doStand();                                          
                                                              
      if (ch->fight())                                        
        return TRUE;  // don't fall through                   
    }                                                         
    if (dynamic_cast<TBeing *>(ch->riding) && (in_room != ROOM_NOWHERE)) {  
      act("You can't get things from $p while mounted!",      
             FALSE, ch, this, 0, TO_CHAR);
      return TRUE;
    }

    if((tCorpse=dynamic_cast<TPCorpse *>(this)) &&
       ((sstring)ch->getName()).lower() == tCorpse->getOwner()){
      // allow loot
    } else if((corpse=dynamic_cast<TBaseCorpse *>(this)) &&
       corpse->isCorpseFlag(CORPSE_DENY_LOOT) &&
       !ch->isImmortal()){
      act("Looting $p isn't allowed.",
	  TRUE, ch, this, NULL, TO_CHAR);
      return TRUE;
    }


    sprintf(capbuf, "%s %s", newarg, arg2);
    act("You start getting items from $p.", TRUE, ch, this, NULL, TO_CHAR);
    act("$n starts getting items from $p.", TRUE, ch, this, NULL, TO_ROOM);


    /*
    if ((tCorpse = dynamic_cast<TPCorpse *>(this)) &&
        !ch->isImmortal() &&
        ch->getName(.lower()) != tCorpse->getOwner()) {
      affectedData tAff;

      tAff.type     = AFFECT_PLAYERLOOT;
      tAff.duration = (24 * UPDATES_PER_MUDHOUR);
      ch->affectJoin(ch, &tAff, AVG_DUR_NO, AVG_EFF_NO);
      vlogf(LOG_CHEAT, fmt("Adding PLoot Flag To: %s (2)") %  ch->getName());
    }
    */

    start_task(ch, ch->roomp->getStuff(), ch->roomp, TASK_GET_ALL, capbuf, 
            350, ch->in_room, 1, 0, 0);
    // this is a kludge, task_get still has a tiny delay on it
    // this dumps around it and goes right to the guts
    rc = (*(tasks[TASK_GET_ALL].taskf))
          (ch, CMD_TASK_CONTINUE, "", 0, ch->roomp, 0);
    if (IS_SET_DELETE(rc, DELETE_THIS)) {
      return DELETE_VICT;
    }
    return TRUE;
  } else if ((p = getabunch(arg1, newarg))) {
    if (!searchLinkedListVis(ch, newarg, getStuff())) {
      ch->sendTo(COLOR_OBJECTS, fmt("There are no \"%s\"'s visible in %s.\n\r") %
              newarg % getName());
      return TRUE;
    }
    if (ch->getPosition() <= POSITION_SITTING) {
      ch->sendTo("You need to be standing to do that.\n\r");
      if (!ch->awake())
        return TRUE;   // sleeping
      ch->doStand();

      if (ch->fight())
        return TRUE;  // don't fall through
    }
    if (dynamic_cast<TBeing *>(ch->riding) && (ch->in_room != ROOM_NOWHERE)) {
      act("You can't get things from $p while mounted!",
           FALSE, ch, this, 0, TO_CHAR);
      return TRUE;
    }
    if((tCorpse=dynamic_cast<TPCorpse *>(this)) &&
       ((sstring)ch->getName()).lower() == tCorpse->getOwner()){
      // allow loot
    } else if((corpse=dynamic_cast<TBaseCorpse *>(this)) &&
       corpse->isCorpseFlag(CORPSE_DENY_LOOT) &&
       !ch->isImmortal()){
      act("Looting $p isn't allowed.",
	  TRUE, ch, this, NULL, TO_CHAR);
      return TRUE;
    }



    sprintf(capbuf, "%s %s", newarg, arg2);
    act("You start getting items from $p.", TRUE, ch, this, NULL, TO_CHAR);
    act("$n starts getting items from $p.", TRUE, ch, this, NULL, TO_ROOM);
    start_task(ch, ch->roomp->getStuff(), ch->roomp, TASK_GET_ALL, capbuf,
            350, ch->in_room, 0, p + 1, 0);

    /*
    if ((tCorpse = dynamic_cast<TPCorpse *>(this)) &&
        !ch->isImmortal() &&
        ch->getName(.lower()) != tCorpse->getOwner()) {
      affectedData tAff;

      tAff.type     = AFFECT_PLAYERLOOT;
      tAff.duration = (24 * UPDATES_PER_MUDHOUR);
      ch->affectJoin(ch, &tAff, AVG_DUR_NO, AVG_EFF_NO);
      vlogf(LOG_CHEAT, fmt("Adding PLoot Flag To: %s (3)") %  ch->getName());
    }
    */

    // this is a kludge, task_get still has a tiny delay on it
    // this dumps around it and goes right to the guts
    rc = (*(tasks[TASK_GET_ALL].taskf))
        (ch, CMD_TASK_CONTINUE, "", 0, ch->roomp, 0);
    if (IS_SET_DELETE(rc, DELETE_THIS)) {
      return DELETE_VICT;
    }
    return TRUE;
  }
  return FALSE;
}

int TBaseContainer::putSomethingIntoContainer(TBeing *ch, TOpenContainer *cont)
{
  if(getStuff()){
    act("Containers can't hold other containers unless they're empty.", FALSE, ch, cont,this, TO_CHAR);
    return FALSE;
  }

  return TThing::putSomethingIntoContainer(ch, cont);
}

void TBaseContainer::findSomeDrink(TDrinkCon **last_good, TBaseContainer **last_cont, TBaseContainer *)
{
  TThing *t;

  for (t = getStuff(); t; t = t->nextThing) {
    t->findSomeDrink(last_good, last_cont, this);

    if (last_good)
      break;
  }
}

void TBaseContainer::findSomeFood(TFood **last_good, TBaseContainer **last_cont, TBaseContainer *)
{
  TThing *t;
                                                              
  for (t = getStuff(); t; t = t->nextThing)                        
    t->findSomeFood(last_good, last_cont, this);              
}                                                             

void TBaseContainer::powerstoneCheck(TOpal **topMax)
{
  TThing *t;

  for (t = getStuff(); t; t = t->nextThing) {
    t->powerstoneCheck(topMax);
  }
}

void TBaseContainer::powerstoneCheckCharged(TOpal **topMax)
{
  TThing *t;
                                                              
  for (t = getStuff(); t; t = t->nextThing) {                      
    t->powerstoneCheckCharged(topMax);                        
  }                                                           
}                                                             
                                                              
void TBaseContainer::powerstoneMostMana(int *topMax)
{
  TThing *t;

  for (t = getStuff(); t; t = t->nextThing) {
    t->powerstoneMostMana(topMax);
  }
}

bool TBaseContainer::fitsSellType(tObjectManipT tObjectManip,
                              TBeing *ch, TMonster *tKeeper,
                              sstring tStString, itemTypeT tItemType,
                              int & tCount, int tShop)
{
  TThing         *tThing,
                 *tThingTemp;
  TObj           *tObj;
  TOpenContainer *tContainer = dynamic_cast<TOpenContainer *>(this);

  if ((tObjectManip == OBJMAN_FIT ||
       tObjectManip == OBJMAN_NOFIT ||
       tObjectManip == OBJMAN_TYPE) &&
      (!tContainer || !tContainer->isClosed())) {
    for (tThing = getStuff(); tThing; tThing = tThingTemp) {
      tThingTemp = tThing->nextThing;

      if (!ch->sameRoom(*tKeeper) || !ch->awake())
        break;

      if (!(tObj = dynamic_cast<TObj *>(tThing)))
        continue;
      if (tObj->fitsSellType(tObjectManip, ch, tKeeper, tStString, tItemType, tCount, tShop)) {
        --(*tObj);
        *ch += *tObj;
        generic_sell(ch, tKeeper, tObj, tShop);
      }
    }
  }

  // This is sort of a kludge but we don't really want to weight this. It is
  // better to not try and sell bags/containers with 'all' instead of picking
  // out all the ones that we should/shouln't.
  if (tObjectManip == OBJMAN_FIT)
    return false;
  else
    return TObj::fitsSellType(tObjectManip, ch, tKeeper, tStString, tItemType, tCount, tShop);
}

int TBaseContainer::isSaddle() const
{
  if (isname("saddle",name) && (isname("panier",name) || isname("pack",name) || isname("bag",name))) {
    if (isname ("[CAN_RIDE]",name))
      return 1;
    else return 2;
  }
  return FALSE;
}
