//////////////////////////////////////////////////////////////////////////
//
// SneezyMUD - All rights reserved, SneezyMUD Coding Team
//
//////////////////////////////////////////////////////////////////////////
// Peel

#include "stdsneezy.h"
#include "obj_tooth_necklace.h"

TToothNecklace::TToothNecklace() :
  TExpandableContainer()
{
}

TToothNecklace::TToothNecklace(const TToothNecklace &a) :
  TExpandableContainer(a)
{
}

TToothNecklace & TToothNecklace::operator=(const TToothNecklace &a)
{
  if (this == &a) return *this;
  TExpandableContainer::operator=(a);
  return *this;
}

TToothNecklace::~TToothNecklace()
{
}




sstring teeth_count_desc(int count)
{
  int activemobcount=0;
  for (unsigned int mobnum = 0; mobnum < mob_index.size(); mobnum++) {
    for (unsigned int zone = 0; zone < zone_table.size(); zone++) {
      if(mob_index[mobnum].virt <= zone_table[zone].top){
	if(zone_table[zone].enabled)
	  activemobcount++;
	break;
      }
    }
  }

  activemobcount /= 18;

  if(count==0){
    return "no";
  } else if(count < activemobcount*1){
    return "a pitiful amount of";
  } else if(count < activemobcount*2){
    return "not many";
  } else if(count < activemobcount*3){
    return "a few";
  } else if(count < activemobcount*4){
    return "some";
  } else if(count < activemobcount*5){
    return "a handful of";
  } else if(count < activemobcount*6){
    return "quite a few";
  } else if(count < activemobcount*7){
    return "several crocodiles worth of";
  } else if(count < activemobcount*8){
    return "a large amount of";
  } else if(count < activemobcount*9){
    return "several";
  } else if(count < activemobcount*10){
    return "plenty of";
  } else if(count < activemobcount*11){
    return "many";
  } else if(count < activemobcount*12){
    return "an abundance of";
  } else if(count < activemobcount*13){
    return "an impressive array of";
  } else if(count < activemobcount*14){
    return "a myriad of";
  } else if(count < activemobcount*15){
    return "a great many";
  } else if(count < activemobcount*16){
    return "a huge amount of";
  } else if(count < activemobcount*17){
    return "an unbelievable amount of";
  } else {
    return "bristling with";
  }
}

void TToothNecklace::updateDesc()
{
  int count=0;
  for(TThing *t=getStuff();t;t=t->nextThing){
    count++;
  }

  if (!isObjStat(ITEM_STRUNG))
    swapToStrung();

  delete [] shortDescr;
  shortDescr=mud_str_dup(fmt("<W>a necklace with %s <1><r>bloody<1><W> teeth<1>")%
			 teeth_count_desc(count));
}

void TToothNecklace::describeObjectSpecifics(const TBeing *ch) const
{
  int count=0;
  for(TThing *t=getStuff();t;t=t->nextThing){
    count++;
  }

  ch->sendTo(fmt("It has %i teeth on it.\n\r") % count);
}


void TToothNecklace::assignFourValues(int x1, int x2, int x3, int x4)
{
  TExpandableContainer::assignFourValues(x1, x2, x3, x4);
}

void TToothNecklace::getFourValues(int *x1, int *x2, int *x3, int *x4) const
{
  TExpandableContainer::getFourValues(x1, x2, x3, x4);
}

sstring TToothNecklace::statObjInfo() const
{
  return TExpandableContainer::statObjInfo();
}

bool TToothNecklace::objectRepair(TBeing *ch, TMonster *repair, silentTypeT silent)
{
  if (!silent) {
    repair->doTell(fname(ch->name), "I can't repair that.");
  }
  return TRUE;
}
