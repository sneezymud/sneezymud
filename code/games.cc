#include "stdsneezy.h"
#include "games.h"
#include "obj_casino_chip.h"

void payout(TBeing *ch, int talens)
{
  TObj *chip;
  sstring buf;

  while(talens>0){
    if(talens >= 1000000)
      chip=read_object(CHIP_1000000, VIRTUAL);
    else if(talens >= 500000)
      chip=read_object(CHIP_500000, VIRTUAL);
    else if(talens >= 100000)
      chip=read_object(CHIP_100000, VIRTUAL);
    else if(talens >= 50000)
      chip=read_object(CHIP_50000, VIRTUAL);
    else if(talens >= 10000)
      chip=read_object(CHIP_10000, VIRTUAL);
    else if(talens >= 5000)
      chip=read_object(CHIP_5000, VIRTUAL);
    else if(talens >= 1000)
      chip=read_object(CHIP_1000, VIRTUAL);
    else if(talens >= 500)
      chip=read_object(CHIP_500, VIRTUAL);
    else if(talens >= 100)
      chip=read_object(CHIP_100, VIRTUAL);
    else {
      ssprintf(buf, "You receive %i talens.", talens);
      act(buf.c_str(), TRUE, ch, 0, 0, TO_CHAR);
      ssprintf(buf, "$n receives %i talens.", talens);
      act(buf.c_str(), TRUE, ch, 0, 0, TO_ROOM);
      ch->addToMoney(talens, GOLD_GAMBLE);
      return;
    }

    if(!chip){
      vlogf(LOG_BUG, "couldn't load chip in payout");
      return;
    }

    talens -= chip->obj_flags.cost;

    ssprintf(buf, "You receive %s.", chip->getName());
    act(buf.c_str(), TRUE, ch, 0, 0, TO_CHAR);
    ssprintf(buf, "$n receives %s.", chip->getName());
    act(buf.c_str(), TRUE, ch, 0, 0, TO_ROOM);

    *ch += *chip;
  }
}


TObj *find_chip(TBeing *ch, const int &chip)
{
  TObj *o;

  for(TThing *tt=ch->getStuff();tt;tt=tt->nextThing)
    if((o=dynamic_cast<TObj *>(tt)) && dynamic_cast<TCasinoChip *>(o) &&
       o->objVnum()==chip)
      return o;
  
  return NULL;
}


TObj *find_chip(TBeing *ch, const sstring &coin_str)
{
  TObj *chip;

  if(!(chip=generic_find_obj(coin_str, FIND_OBJ_INV, ch)) ||
     !(dynamic_cast<TCasinoChip *>(chip))){
    return NULL;
  }
  
  return chip;
}


