#include "stdsneezy.h"
#include "obj_base_weapon.h"


int poisonCutlass(TBeing *vict, cmdTypeT cmd, const char *arg, TObj *o, TObj *){
  int j;
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

  for (j = 0; j < MAX_SWING_AFFECT; j++) {
    if (cutlass->oneSwing[j].type == SPELL_POISON)
      return FALSE;
    
    if (cutlass->oneSwing[j].type == TYPE_UNDEFINED) {
      cutlass->oneSwing[j].type = SPELL_POISON;
      cutlass->oneSwing[j].bitvector = AFF_POISON;
      cutlass->oneSwing[j].location = APPLY_STR;
      cutlass->oneSwing[j].modifier = -20;
      cutlass->oneSwing[j].duration = 50;
      cutlass->oneSwing[j].level = 50;
      cutlass->oneSwing[j].renew = -1;
      break;
    }
  }
  for (; j < MAX_SWING_AFFECT; j++) {
    if (cutlass->oneSwing[j].type == TYPE_UNDEFINED) {
      cutlass->oneSwing[j].type = AFFECT_DISEASE;
      cutlass->oneSwing[j].level = 0;
      cutlass->oneSwing[j].duration = 50;
      cutlass->oneSwing[j].modifier = DISEASE_POISON;
      cutlass->oneSwing[j].location = APPLY_NONE;
      cutlass->oneSwing[j].bitvector = AFF_POISON;
      cutlass->oneSwing[j].renew = -1;
      break;
    }
  }

  act("<g>Poison oozes from $n's $o and drips to the $g.<1>",
      0, ch, o, 0, TO_ROOM);
  act("<g>Your $o oozes poison, which runs down the length of the blade and drips to the $g.<1>",
      0, ch, o, 0, TO_CHAR);


  return TRUE;
}


