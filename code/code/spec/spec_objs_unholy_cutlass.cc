#include "stdsneezy.h"
#include "obj_base_weapon.h"


int doHurt(TBeing *ch, TBeing *vict, TObj *o)
{
  int rc=0;

  act("$N <r>recoils in pain as $n's $o burns $m with unholy vengeance.<1>",
      0, ch, o, vict, TO_ROOM);
  act("$N <r>recoils in pain as your $o burns $m with unholy vengeance.<1>",
      0, ch, o, vict, TO_CHAR);

  rc = ch->reconcileDamage(vict, ::number(6,10), DAMAGE_DRAIN);
  if (rc == -1)
    return DELETE_VICT;
  return TRUE;
}


void doCurse(TBeing *ch, TBeing *vict, TObj *o)
{
  TBaseWeapon *tWeap;

  if (!(tWeap = dynamic_cast<TBaseWeapon *>(o)) || !vict)
    return;

  if (vict->affectedBySpell(SPELL_CURSE))
    return;

      act("<r>$N glows evilly with a dark forboding red as $n's $p <1><r>strikes $m!<1>",
      0, ch, o, vict, TO_ROOM);
      act("<r>You glow evilly with a dark forboding red as $n's $p <1><r>strikes you!<1>",
      0, ch, o, vict, TO_VICT);
  

  genericCurse(ch, vict, 50, SPELL_CURSE);
}


int unholyCutlass(TBeing *vict, cmdTypeT cmd, const char *arg, TObj *o, TObj *)
{
  TBeing *ch;

  if(cmd == CMD_GENERIC_PULSE){
    if(!::number(0,29)){
      if (!(ch = dynamic_cast<TBeing *>(o->equippedBy)))
	return FALSE;       // weapon not equipped (carried or on ground)

      act("<r>Blood oozes from $n's $o and drips to the $g.<1>",
	  0, ch, o, 0, TO_ROOM);
      act("<r>Your $o oozes blood, which runs down the length of the blade and drips to the $g.<1>",
	  0, ch, o, 0, TO_CHAR);
      ch->dropPool(1, LIQ_BLOOD);
    }
  }  

  if(!(ch=genericWeaponProcCheck(vict, cmd, o, 5)))
     return FALSE;
  
  if(!::number(0,3)){
    doCurse(ch, vict, o);      
    return TRUE;
  }
  
  return doHurt(ch, vict, o);
}
