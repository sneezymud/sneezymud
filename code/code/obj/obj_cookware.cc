//////////////////////////////////////////////////////////////////////////
//
// SneezyMUD - All rights reserved, SneezyMUD Coding Team
//
// chest.cc
//
//////////////////////////////////////////////////////////////////////////


#include "low.h"
#include "monster.h"
#include "obj_cookware.h"
#include "obj_open_container.h"
#include "obj_pool.h"
#include "liquids.h"

TCookware::TCookware() :
  TOpenContainer()
{
}

TCookware::TCookware(const TCookware &a) :
  TOpenContainer(a)
{
}

TCookware & TCookware::operator=(const TCookware &a)
{
  if (this == &a) return *this;
  TOpenContainer::operator=(a);
  return *this;
}

TCookware::~TCookware()
{
}



void TCookware::assignFourValues(int x1, int x2, int x3, int x4)
{
  TOpenContainer::assignFourValues(x1, x2, x3, x4);
}

void TCookware::getFourValues(int *x1, int *x2, int *x3, int *x4) const
{
  TOpenContainer::getFourValues(x1, x2, x3, x4);
}

sstring TCookware::statObjInfo() const
{
  return TOpenContainer::statObjInfo();
}

bool TCookware::objectRepair(TBeing *ch, TMonster *repair, silentTypeT silent)
{
  if (!silent) {
    repair->doTell(fname(ch->name), "Does this look like a cookware repair shop to you?");
  }
  return TRUE;
}



void TCookware::pourMeOut(TBeing *ch)
{
  act("You can't pour from that!", FALSE, ch, 0, 0, TO_CHAR);
  return;
}

void TCookware::pourMeIntoDrink1(TBeing *ch, TObj *)
{
  act("You can't pour from $p!", FALSE, ch, this, 0, TO_CHAR);
  return;
}

void TCookware::pourMeIntoDrink2(TBeing *ch, TBaseCup *from)
{
  TPool *pool;
  TObj *obj;

  if(from->getDrinkUnits()<=0){
    act("$p is empty.", FALSE, ch, from, 0, TO_CHAR);
    return;
  }

  // get pool object
  int robj = real_object(Obj::GENERIC_POOL);
  if (robj < 0 || robj >= (signed int) obj_index.size()) {
    vlogf(LOG_BUG, format("TCookware::pourMeIntoDrink2(): No object (%d) in database!") %  Obj::GENERIC_POOL);
    return;
  }
  
  obj = read_object(robj, REAL);
  
  pool = dynamic_cast<TPool *>(obj);
  if (!pool)
    return;


  // let them know
  sstring buf;
  buf = format("You pour %s into %s.\n\r") %
    liquidInfo[from->getDrinkType()]->name % ch->objs(this);
  act(buf, FALSE, ch, 0, 0, TO_CHAR);

  buf = format("$n pours %s into %s.\n\r") %
    liquidInfo[from->getDrinkType()]->name % ch->objs(this);
  act(buf, TRUE, ch, 0, 0, TO_ROOM);

  
  // init the pool
  pool->initPool(from->getDrinkUnits(), from->getDrinkType());
  from->genericEmpty();

  // put pool object in me
  *this += *pool;


}
