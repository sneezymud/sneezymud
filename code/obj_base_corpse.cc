//////////////////////////////////////////////////////////////////////////
//
// SneezyMUD - All rights reserved, SneezyMUD Coding Team
//
// base_corpse.cc
//
//////////////////////////////////////////////////////////////////////////


#include "stdsneezy.h"
#include "obj_base_corpse.h"
#include "obj_player_corpse.h"
#include "obj_base_corpse.h"
#include "obj_base_clothing.h"
#include "obj_tooth_necklace.h"

TBaseCorpse::TBaseCorpse() :
  TBaseContainer(),
  corpse_flags(0),
  corpse_race(RACE_NORACE),
  corpse_level(0),
  corpse_vnum(-1),
  tDissections(NULL)
{
}

TBaseCorpse::TBaseCorpse(const TBaseCorpse &a) :
  TBaseContainer(a),
  corpse_flags(a.corpse_flags),
  corpse_race(a.corpse_race),
  corpse_level(a.corpse_level),
  corpse_vnum(a.corpse_vnum),
  tDissections(a.tDissections)
{
}

TBaseCorpse & TBaseCorpse::operator=(const TBaseCorpse &a)
{
  if (this == &a) return *this;
  TBaseContainer::operator=(a);
  corpse_flags = a.corpse_flags;
  corpse_race = a.corpse_race;
  corpse_level = a.corpse_level;
  corpse_vnum = a.corpse_vnum;
  tDissections = a.tDissections;
  return *this;
}

TBaseCorpse::~TBaseCorpse()
{
  for (; getStuff(); ) {
    // assumption that corpse is not in another container
    // assumption that corpse is also not stuck in someone, or equipped
    TThing * t = getStuff();
    --(*t);
    if (parent)
      *parent += *t;
    else if (riding)
      t->mount(riding);
    else if (roomp)
      *roomp += *t;
    else {
      // the corpse wasn't owned by anything
      // this can happen if an immortal holds a corpse and quits
      //    as in: "Here, let me help you get rid of this stuff"
      // seems best solution (given how this can happen), is to junk stuff
      delete t;
    }
  }

  if (tDissections) {
    dissectInfo *tNextDissect = tDissections;

    for (; tNextDissect; tNextDissect = tDissections) {
      tDissections = tNextDissect->tNext;
      delete tNextDissect;
      tNextDissect = NULL;
    }

    tDissections = NULL;
  }
}

int TBaseCorpse::putMeInto(TBeing *ch, TOpenContainer *container)
{
  TObj *o;
  TThing *t;
  
  for(t=container->getStuff(); t; t=t->nextThing){
    o = dynamic_cast<TObj *>(t);

    if (!o)
      continue;

    if (dynamic_cast<TToothNecklace *>(container) &&
	dynamic_cast<TBaseCorpse *>(o) &&
	getCorpseVnum() == dynamic_cast<TBaseCorpse *>(o)->getCorpseVnum()){
      ch->sendTo(fmt("You already have one of those teeth in your %s.\n\r") %
		 fname(container->name).c_str());
      return TRUE;
    }
  }
  return FALSE;

}


void TBaseCorpse::assignFourValues(int x1, int x2, int x3, int x4)
{
  setCorpseFlags(x1);
  setCorpseRace((race_t) x2);
  setCorpseLevel(x3);
  setCorpseVnum(x4);
}

void TBaseCorpse::getFourValues(int *x1, int *x2, int *x3, int *x4) const
{
  *x1 = getCorpseFlags();
  *x2 = getCorpseRace();
  *x3 = getCorpseLevel();
  *x4 = getCorpseVnum();
}

void TBaseCorpse::setCorpseFlags(unsigned int f)
{
  corpse_flags = f;
}

unsigned int TBaseCorpse::getCorpseFlags() const
{
  return corpse_flags;
}

void TBaseCorpse::addCorpseFlag(unsigned int r)
{
  corpse_flags |= r;
}

void TBaseCorpse::remCorpseFlag(unsigned int r)
{
  corpse_flags &= ~r;
}

bool TBaseCorpse::isCorpseFlag(unsigned int r) const
{
  return ((corpse_flags & r) != 0);
}

void TBaseCorpse::setCorpseRace(race_t r)
{
  corpse_race = r;
}

race_t TBaseCorpse::getCorpseRace() const
{
  return corpse_race;
}

void TBaseCorpse::setCorpseLevel(unsigned int r)
{
  corpse_level = r;
}

unsigned int TBaseCorpse::getCorpseLevel() const
{
  return corpse_level;
}

void TBaseCorpse::setCorpseVnum(int r)
{
  corpse_vnum = r;
}

int TBaseCorpse::getCorpseVnum() const
{
  return corpse_vnum;
}

void TBaseCorpse::peeOnMe(const TBeing *ch)
{
  act("With a scream of victory, $n pisses on $p.",
             TRUE, ch, this, NULL, TO_ROOM);
  act("You scream victoriously as you piss on $p.",
             TRUE, ch, this, NULL, TO_CHAR);
}

int TBaseCorpse::dissectMe(TBeing *caster)
{
  int num = -1, rnum;
  int amount = 100;
  TObj *obj;
  char msg[256], gl_msg[256];
  
  if (isCorpseFlag(CORPSE_NO_REGEN)) {
    // a body part or something
    act("$p: You aren't able to dissect that.",
	FALSE, caster, this, 0, TO_CHAR);
    return FALSE;
  }
  if (isCorpseFlag(CORPSE_NO_DISSECT)) {
    // dissection already occurred
    act("Nothing more of any use can be taken from $p.",
	FALSE, caster, this, 0, TO_CHAR);
    return FALSE;
  }
  num = determineDissectionItem(this, &amount, msg, gl_msg, caster);
  if (num == -1 || !(rnum = real_object(num))) {
    // no item
    act("You aren't aware of any useful dissections that come from $p.",
	FALSE, caster, this, 0, TO_CHAR);
    return TRUE;
  }
  
  act("You dissect $p.", FALSE, caster, this, 0, TO_CHAR);
  act("$n dissects $p.", FALSE, caster, this, 0, TO_ROOM);
  addCorpseFlag(CORPSE_NO_DISSECT);
  
  if (obj_index[rnum].getNumber() >= obj_index[rnum].max_exist) {
    // item at max
    act("You find nothing useful in $p.",
	FALSE, caster, this, 0, TO_CHAR);
    return TRUE;
  }
  if (!(obj = read_object(num, VIRTUAL))) {
    caster->sendTo("Serious problem in dissect.\n\r");
    vlogf(LOG_OBJ, fmt("Bad call to read_object in dissect, num %d") %  num);
    return FALSE;
  }
  int bKnown = caster->getSkillValue(SKILL_DISSECT);
  
  if (!caster->bSuccess(bKnown, SKILL_DISSECT) &&
      !caster->hasQuestBit(TOG_STARTED_MONK_BLUE)) {
    // dissection failed
    act("You find nothing useful in $p.",
	FALSE, caster, this, 0, TO_CHAR);
    delete obj;
    obj = NULL;
    return TRUE;
  } else {
    if (::number(0,99) >= amount) {
      // failed to pass amount
      CF(SKILL_DISSECT);
      act("You find nothing useful in $p.",
	  FALSE, caster, this, 0, TO_CHAR);
      delete obj;
      obj = NULL;
      return TRUE;
    }
    
    if(caster->hasQuestBit(TOG_STARTED_MONK_BLUE)){
      caster->setQuestBit(TOG_MONK_KILLED_SHARK);
    }
    
    *caster += *obj;
    act(   msg, FALSE, caster, obj, this, TO_CHAR);
    act(gl_msg, FALSE, caster, obj, this, TO_ROOM);
    return TRUE;
  }

  return TRUE;
}

void TBaseCorpse::update(int use)
{
  if (getStuff())
    getStuff()->update(use);

  if (nextThing) {
    if (nextThing != this)
      nextThing->update(use);
  }
}

void TBaseCorpse::lookObj(TBeing *ch, int) const
{
  act("$p contains:",TRUE, ch, this, 0, TO_CHAR);
  list_in_heap(getStuff(), ch, 0, 100);
  return;
}

int TBaseCorpse::putSomethingInto(TBeing *ch, TThing *)
{
  // technically, would be OK since is a container, but prevent them anyhow
  act("Unfortunately, you can't put things into $p.", 
             FALSE, ch, this, 0, TO_CHAR);
  return 2;
}

int TBaseCorpse::scavengeMe(TBeing *ch, TObj **)
{
  TThing *t;
  TObj *obj;
  wearSlotT sl;
  char buf[256], buf2[256], buf3[256];
  int rc;

  if (!::number(0, 5) && getStuff()) {
    for (t = getStuff(); t; t = t->nextThing) {
      if (!t->getStuff() && (obj = dynamic_cast<TObj *>(t))) {
        sl = slot_from_bit(obj->obj_flags.wear_flags);
        if (dynamic_cast<TBaseClothing *>(obj)) {
          TObj *tobj = dynamic_cast<TObj *>(ch->equipment[sl]);
          if (obj->itemAC() < (tobj ?  tobj->itemAC() : 0)) {
            strcpy(buf2, obj->name);
            strcpy(buf3, name);
            strcpy(buf2, add_bars(buf2).c_str());
            strcpy(buf3, add_bars(buf3).c_str());
            sprintf(buf, "%s from %s", buf2, buf3);
            rc = ch->doGet(buf);
            if (IS_SET_DELETE(rc, DELETE_THIS))
              return DELETE_VICT;
            return TRUE;
          }
        }
      }
    }
  }
  return FALSE;
}

void TBaseCorpse::decayMe()
{
  if(obj_flags.decay_time>0){
    // corpses decay regardless of where they are
    obj_flags.decay_time--;
    canBeSeen++;
  }
}

int TBaseCorpse::objectDecay()
{
  if (parent)
    act("$p disintegrates in your hands.", FALSE, parent, this, 0, TO_CHAR);
  else if (roomp && roomp->getStuff()) {
    if (getMaterial() == MAT_POWDER) {
      sendrpf(COLOR_OBJECTS, roomp, "A gust of wind scatters %s.\n\r",
getName());
//      act("A gust of wind scatters $n.", TRUE, this, 0, 0, TO_ROOM);
    } else {
      sendrpf(COLOR_OBJECTS, roomp, "Flesh-eaters dissolve %s.\n\r", getName());
//      act("Flesh-eaters dissolve $n.", TRUE, this, 0, 0, TO_ROOM);
    }
  }
  ObjFromCorpse(this);
  return DELETE_THIS;
}

int TBaseCorpse::chiMe(TBeing *tLunatic)
{
  int tMana  = ::number(10, 30),
      bKnown = tLunatic->getSkillLevel(SKILL_CHI);
  //  TThing *tThing;

  if (tLunatic->getMana() < tMana) {
    tLunatic->sendTo("You lack the chi to do this.\n\r");
    return RET_STOP_PARSING;
  } else
    tLunatic->reconcileMana(TYPE_UNDEFINED, 0, tMana);

  if (dynamic_cast<TPCorpse *>(this) ||
      !tLunatic->bSuccess(bKnown, SKILL_CHI) ||
      getCorpseLevel() > tLunatic->GetMaxLevel()) {
    act("You attempt to nuke $p, but fail to control the chi.",
        FALSE, tLunatic, this, NULL, TO_CHAR);
    return true;
  }

  if (isObjStat(ITEM_BURNING)) {
    act("You focus your chi and cause $p to flair up violently, if only for a second.", FALSE, tLunatic, this, NULL, TO_CHAR);
    act("$n stares at $p causing it to flair up violently!", TRUE, tLunatic, this, NULL, TO_ROOM);
  } else {
    act("You focus your chi and set $p ablaze!", FALSE, tLunatic, this, NULL, TO_CHAR);
    act("$n stares at $p which suddenly bursts into flames!", TRUE, tLunatic, this, NULL, TO_ROOM);
  }

  if(material_nums[getMaterial()].flammability){
    setBurning(tLunatic);}

    return true;
   
}
