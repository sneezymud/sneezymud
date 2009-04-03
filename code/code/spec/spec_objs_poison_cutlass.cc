#include "stdsneezy.h"
#include "obj_base_weapon.h"


int poisonCutlass(TBeing *vict, cmdTypeT cmd, const char *arg, TObj *o, TObj *){
  TBaseWeapon *cutlass;
  TBeing *ch;

  if(cmd != CMD_GENERIC_PULSE)
    return FALSE;

  if(::number(0,19))
    return FALSE;

  if(!(cutlass=dynamic_cast<TBaseWeapon *>(o)))
    return FALSE;

  if (!(ch = dynamic_cast<TBeing *>(o->equippedBy)))
    return FALSE;       // weapon not equipped (carried or on ground)

  cutlass->setPoison(LIQ_POISON_STANDARD);


  act("<g>Poison oozes from $n's $o and drips to the $g.<1>",
      0, ch, o, 0, TO_ROOM);
  act("<g>Your $o oozes poison, which runs down the length of the blade and drips to the $g.<1>",
      0, ch, o, 0, TO_CHAR);


  return TRUE;
}


