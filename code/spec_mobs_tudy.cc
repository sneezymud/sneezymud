#include "stdsneezy.h"

const int OBJ_TUDY_BILGE        = 9635;
const int OBJ_TUDY_SHACKLE_KEY1 = 9636;
const int OBJ_TUDY_SHACKLE_KEY2 = 9637;
const int MOB_DEMI_ANGEL_TUDY   = 9637;

// 1) handle toggle for giving key 1
// 2) handle toggle for giving key 2
// 3) switch to demi-angel tudy when both keys given
// when anything given check inventory for both keys
int tudy(TBeing *, cmdTypeT cmd, const char *, TMonster *tudy, TObj *)
{
  TThing *t;
  TObj *o;
  bool haskey1=false, haskey2=false;
  TMonster *newtudy;
  TPerson *ch;

  if(cmd != CMD_GIVE && cmd != CMD_GENERIC_PULSE)
    return FALSE;
  
  // loop through inventory
  for(t=tudy->getStuff();t;t=t->nextThing){
    if((o=dynamic_cast<TObj *>(t))){
      if(o->objVnum() == OBJ_TUDY_SHACKLE_KEY1)
	haskey1=true;
      if(o->objVnum() == OBJ_TUDY_SHACKLE_KEY2)
	haskey2=true;
    }
  }
  
  
  if(haskey1 && haskey2){
    act("$n quickly unlocks his shackles.",
	0, tudy, 0, 0, TO_ROOM);	
    act("$n looks to the heavens and as large wings sprout from his back.",
	0, tudy, 0, 0, TO_ROOM);
    tudy->doAction("", CMD_SCREAM);

    newtudy=read_mobile(MOB_DEMI_ANGEL_TUDY, VIRTUAL);
    o=read_object(OBJ_TUDY_BILGE, VIRTUAL);
    *newtudy += *o;
    *tudy->roomp += *newtudy;


    act("$n turns to you with glowing <r>red<1> eyes and a face full of anger.",
	0, newtudy, 0, 0, TO_ROOM);

    newtudy->doSay("No mortal shall ever possess the heart of an angel!");
    newtudy->doSay("You will perish for your arrogance!");

    for(t=newtudy->roomp->getStuff();t;t=t->nextThing){
      if((ch=dynamic_cast<TPerson *>(t))){
	newtudy->takeFirstHit(*ch);
	break;
      }
    }

    return DELETE_THIS;
  }

  return FALSE;
}
