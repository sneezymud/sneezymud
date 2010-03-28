#include "comm.h"
#include "obj_base_weapon.h"
#include "extern.h"
#include "being.h"
#include "liquids.h"

int ofManyPotions(TBeing *vict, cmdTypeT cmd, const char *arg, TObj *o, TObj *){
  TBaseWeapon *weapon;
  TBeing *ch;

  if(cmd != CMD_GENERIC_PULSE)
    return FALSE;

  if(::number(0,19))
    return FALSE;

  if(!(weapon=dynamic_cast<TBaseWeapon *>(o)))
    return FALSE;

  if (!(ch = dynamic_cast<TBeing *>(o->equippedBy)))
    return FALSE;       // weapon not equipped (carried or on ground)

  int which=::number(0,99);
  liqTypeT liq=LIQ_CHAMPAGNE; // default in case of accidental fall through

  if(which < 65){ // 0-64, 65% of the time
    switch(::number(0,2)){
      case 0: liq=LIQ_POISON_STANDARD; break;
      case 1: liq=LIQ_TEQUILA; break;
      case 2: liq=LIQ_HOLYWATER;  break;
    }
  } else if(which < 95){ // 65-94, 30% of the time
    switch(::number(0,2)){
      case 0: liq=LIQ_BRANDY; break;
      case 1: liq=LIQ_CHAMPAGNE; break;
      case 2: liq=LIQ_COFFEE;  break;
    }
  } else if(which < 99){ // 95-98, 4% of the time
    switch(::number(0,2)){
      case 0: liq=LIQ_POT_CELERITE; break;
      case 1: liq=LIQ_POT_BONE_BREAKER; break;
      case 2: liq=LIQ_POT_DETECT_MAGIC;  break;
    }
  } else { // 99, 1% of the time
    switch(::number(0,2)){
      case 0: liq=LIQ_POISON_VIOLET_FUNGUS; break;
      case 1: liq=LIQ_POT_QUICKSILVER; break;
      case 2: liq=LIQ_POT_YOUTH;  break;
    }
  }


  act(format("A %s liquid oozes from $n's $o and drips to the $g.") % 
      liquidInfo[liq]->color, 0, ch, o, 0, TO_ROOM);
  act(format("Your $o oozes a %s liquid, which drips to the $g.") % 
      liquidInfo[liq]->color, 0, ch, o, 0, TO_CHAR);

  ch->dropPool(1, liq);

  return TRUE;
}


