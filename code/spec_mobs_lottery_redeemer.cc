#include "stdsneezy.h"
#include "spec_objs_lottery_ticket.h"


int lotteryRedeemer(TBeing *ch, cmdTypeT cmd, const char *, TMonster *myself, TObj *o)
{
  sstring buf;

  if(cmd != CMD_MOB_GIVEN_ITEM)
    return false;

  if(!ch || !o) // something weird going on if this happens
    return false;
  
  if(o->objVnum() != OBJ_LOTTERY_TICKET){
    myself->doSay("What the hell is this?");
    myself->doDrop("", o);
    return false;
  }

  for(int i=0;i<NUM_LOTTERY_PRIZES;++i){
    if(isname(prizes[i].name, o->name)){
      TObj *op=read_object(prizes[i].vnum, VIRTUAL);
      *ch += *op;
      myself->doSay("Ah yes, here you are.");
      act("$n gives you $p.", 0, myself, op, ch, TO_VICT);
      act("$n gives $N $p.", 0, myself, op, ch, TO_NOTVICT);

      delete o;
      return true;
    }
  }


  return false;
}
