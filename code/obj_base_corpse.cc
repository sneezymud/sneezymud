//////////////////////////////////////////////////////////////////////////
//
// SneezyMUD - All rights reserved, SneezyMUD Coding Team
//
// $Log: obj_base_corpse.cc,v $
// Revision 5.1  1999/10/16 04:31:17  batopr
// new branch
//
// Revision 1.1  1999/09/12 17:24:04  sneezy
// Initial revision
//
//
//////////////////////////////////////////////////////////////////////////


// base_corpse.cc
//

#include "stdsneezy.h"

TBaseCorpse::TBaseCorpse() :
  TContainer(),
  corpse_flags(0),
  corpse_race(RACE_NORACE),
  corpse_level(0),
  corpse_vnum(-1),
  tDissections(NULL)
{
}

TBaseCorpse::TBaseCorpse(const TBaseCorpse &a) :
  TContainer(a),
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
  TContainer::operator=(a);
  corpse_flags = a.corpse_flags;
  corpse_race = a.corpse_race;
  corpse_level = a.corpse_level;
  corpse_vnum = a.corpse_vnum;
  tDissections = a.tDissections;
  return *this;
}

TBaseCorpse::~TBaseCorpse()
{
  for (; stuff; ) {
    // assumption that corpse is not in another container
    // assumption that corpse is also not stuck in someone, or equipped
    TThing * t = stuff;
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
  if (gamePort != PROD_GAMEPORT) {
    int          tValue,
                 tWhich = 0;
    TObj        *tObj;
    dissectInfo *tDissect;

    if (isCorpseFlag(CORPSE_NO_REGEN)) {
      act("$p: I am afraid that can not be dissected.",
          FALSE, caster, this, 0, TO_CHAR);
      return FALSE;
    }

    if (!tDissections) {
      act("That has nothing of value anymore.",
          FALSE, caster, this, 0, TO_CHAR);
      return FALSE;
    }

    for (tDissect = tDissections; tDissect; tDissect = tDissect->tNext)
      tWhich++;

    vlogf(1, "dissectMe: tWhich(%d)", tWhich);

    tWhich = ::number(0, tWhich);

    for (tDissect = tDissections; (tDissect && tWhich--); tDissect = tDissect->tNext);

    if (!tDissect && !(tDissect = tDissections)) {
      vlogf(1, "dissectMe Error.  tDissect ended up NULL.");
      return FALSE;
    }

    if (!(tValue = real_object(tDissect->loadItem))) {
      act("That has nothing of value anymore.",
          FALSE, caster, this, 0, TO_CHAR);
      return FALSE;
    }

    act("You dissect $p.", FALSE, caster, this, 0, TO_CHAR);
    act("$n dissects $p.", FALSE, caster, this, 0, TO_ROOM);

    if (obj_index[tValue].number >= obj_index[tValue].max_exist) {
      // item at max
      act("You find nothing useful in $p.",
            FALSE, caster, this, 0, TO_CHAR);
      return TRUE;
    }

    if (!(tObj = read_object(tValue, REAL))) {
      caster->sendTo("Serious problem in dissect.\n\r");
      vlogf(8, "Bad call to read_object in dissect, num %d", tValue);
      return FALSE;
    }

    int bKnown = caster->getSkillValue(SKILL_DISSECT);

    if (!bSuccess(caster, bKnown, SKILL_DISSECT) &&
        !caster->hasQuestBit(TOG_STARTED_MONK_BLUE)) {
      // dissection failed
      act("You find nothing useful in $p.",
            FALSE, caster, this, 0, TO_CHAR);
      delete tObj;
      tObj = NULL;
      return TRUE;
    } else {
      if (::number(0,99) >= (signed) tDissect->amount) {
        // failed to pass amount
        CF(SKILL_DISSECT);
        act("You find nothing useful in $p.",
              FALSE, caster, this, 0, TO_CHAR);
        delete tObj;
        tObj = NULL;
        return TRUE;
      }

      if (caster->hasQuestBit(TOG_STARTED_MONK_BLUE))
        caster->setQuestBit(TOG_MONK_KILLED_SHARK);

      *caster += *tObj;
      act(tDissect->message_to_self.c_str(),
          FALSE, caster, tObj, this, TO_CHAR);
      act(tDissect->message_to_others.c_str(),
          FALSE, caster, tObj, this, TO_ROOM);

      if (--tDissect->count <= 0) {
        if (tDissections == tDissect)
          tDissections = tDissect->tNext;
        else {
          dissectInfo *tDissectNext;

          for (tDissectNext = tDissections; tDissectNext->tNext != tDissect;
               tDissectNext = tDissectNext->tNext);

          tDissectNext->tNext = tDissect->tNext;
        }

        delete tDissect;
        tDissect = NULL;
      }

      return TRUE;
    }
  } else {
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

    if (obj_index[rnum].number >= obj_index[rnum].max_exist) {
      // item at max
      act("You find nothing useful in $p.",
            FALSE, caster, this, 0, TO_CHAR);
      return TRUE;
    }
    if (!(obj = read_object(num, VIRTUAL))) {
      caster->sendTo("Serious problem in dissect.\n\r");
      vlogf(8, "Bad call to read_object in dissect, num %d", num);
      return FALSE;
    }
    int bKnown = caster->getSkillValue(SKILL_DISSECT);

    if (!bSuccess(caster, bKnown, SKILL_DISSECT) &&
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
  }

  return TRUE;
}

void TBaseCorpse::update(int use)
{
  if (stuff)
    stuff->update(use);

  if (nextThing) {
    if (nextThing != this)
      nextThing->update(use);
  }
}

void TBaseCorpse::lookObj(TBeing *ch, int) const
{
  act("$p contains:",TRUE, ch, this, 0, TO_CHAR);
  list_in_heap(stuff, ch, 0, 100);
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

  if (!::number(0, 5) && stuff) {
    for (t = stuff; t; t = t->nextThing) {
      if (!t->stuff && (obj = dynamic_cast<TObj *>(t))) {
        sl = slot_from_bit(obj->obj_flags.wear_flags);
        if (dynamic_cast<TBaseClothing *>(obj)) {
          TObj *tobj = dynamic_cast<TObj *>(ch->equipment[sl]);
          if (obj->itemAC() < (tobj ?  tobj->itemAC() : 0)) {
            strcpy(buf2, obj->name);
            strcpy(buf3, name);
            add_bars(buf2);
            add_bars(buf3);
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
    act("$p biodegrades in your hands.", FALSE, parent, this, 0, TO_CHAR);
  else if (roomp && roomp->stuff) {
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

