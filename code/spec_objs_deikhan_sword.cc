#include "stdsneezy.h"
#include "obj_base_weapon.h"

const int AVENGER=319;
const int VINDICATOR=326;
const int DEVASTATOR=320;


// heal ser for avenger, heal crit for vindicator, heal for devastator
void doHeal(TBeing *ch, TObj *o){
  int hp;

  act("$n's $o emanates a gentle <r>w<R>a<Y>rm<R>t<1><r>h.<1>", 
      0, ch, o, 0, TO_ROOM);
  act("Your $o emanates a gentle <r>w<R>a<Y>rm<R>t<1><r>h.<1>",
      0, ch, o, 0, TO_CHAR);
  

  switch(o->objVnum()){
    case AVENGER:
      colorAct(COLOR_SPELLS, "$n glows briefly with a <b>blue hue<z>.", 
	       FALSE, ch, NULL, 0, TO_ROOM);
      colorAct(COLOR_SPELLS, "You glow briefly with a <b>blue hue<z>.", 
	       FALSE, ch, NULL, 0, TO_CHAR);
      
      hp = ch->getSkillDam(ch, SPELL_HEAL_SERIOUS, 50, 100);
      break;
    case VINDICATOR:
      colorAct(COLOR_SPELLS, "$n glows briefly with an <p>indigo hue<1>.",FALSE, ch, NULL, 0, TO_ROOM);
      colorAct(COLOR_SPELLS, "You glow briefly with an <p>indigo hue<1>.",FALSE, ch, NULL, 0, TO_CHAR);
      
      hp = ch->getSkillDam(ch, SPELL_HEAL_CRITICAL, 50, 100);
      break;
    case DEVASTATOR:
    default:
      act("$n glows briefly with an <b>ultramarine hue<1>.", 
	  FALSE, ch, NULL, 0, TO_ROOM);
      act("You glow briefly with an <b>ultramarine hue<1>.", 
	  FALSE, ch, NULL, 0, TO_CHAR);
      
      hp = ch->getSkillDam(ch, SPELL_HEAL, 50, 100);
  }

  
  ch->addToHit(hp);
  ch->updatePos();
}

void doBlind(TBeing *ch, TBeing *vict, TObj *o)
{
  TBaseWeapon *tWeap;

  if (!(tWeap = dynamic_cast<TBaseWeapon *>(o)) || !vict)
    return;

  if (vict->affectedBySpell(SPELL_BLINDNESS) ||
      vict->isAffected(AFF_TRUE_SIGHT) ||
      ch->isNotPowerful(vict, (int)tWeap->weaponLevel(), SPELL_BLINDNESS, SILENT_YES))
    return;

  if (!::number(0, max(10, (int)(tWeap->weaponLevel() +
                                 (vict->GetMaxLevel() -
                                  ch->GetMaxLevel()))))) {
    act("A searing light shines from $p, blinding $N.",
        FALSE, ch, o, vict, TO_CHAR);
    act("$n shields $s eyes as a searing light shines from $p, blinding $N.",
        FALSE, ch, o, vict, TO_NOTVICT);
    act("The world goes white then black as a searing light shines from $n's $p.",
        FALSE, ch, o, vict, TO_VICT);

    int       tDuration = (int)(tWeap->weaponLevel() * UPDATES_PER_MUDHOUR);
    saveTypeT tSave     = SAVE_NO;

    vict->rawBlind((int)tWeap->weaponLevel(), tDuration, tSave);
  }
}

void doSanc(TBeing *ch, TObj *o)
{
  affectedData aff;
  int level=30;

  aff.type = SPELL_SANCTUARY;
  aff.level = level;
  aff.duration = UPDATES_PER_MUDHOUR/4;
  aff.location = APPLY_PROTECTION;
  aff.modifier = level;
  aff.bitvector = AFF_SANCTUARY;


  act("$n's $o <W>flashes brightly<1>!", 
      0, ch, o, 0, TO_ROOM);
  act("Your $o <W>flashes brightly<1>!", 
      0, ch, o, 0, TO_CHAR);
  
  ch->affectJoin(ch, &aff, AVG_DUR_NO, AVG_EFF_YES);
}


int doHarm(TBeing *ch, TBeing *vict, TObj *o)
{
  int dam=0, rc=0;

  switch(o->objVnum()){
    case AVENGER:
      dam+=::number(1,3);
    case VINDICATOR:
      dam+=::number(2,4);
    case DEVASTATOR:
      dam+=::number(3,5);
  }


  act("$n's $o projects righteous <Y>fury<1> into $N.", 
      0, ch, o, vict, TO_ROOM);
  act("Your $o projects righteous <Y>fury<1> into $N.", 
      0, ch, o, vict, TO_CHAR);


  rc = ch->reconcileDamage(vict, dam, DAMAGE_DRAIN);
  if (rc == -1)
    return DELETE_VICT;
  return TRUE;
}



int deikhanSword(TBeing *vict, cmdTypeT cmd, const char *arg, TObj *o, TObj *)
{
  TBeing *ch;
  int chance=0;

  if (!o)
    return FALSE;
  if (!(ch = dynamic_cast<TBeing *>(o->equippedBy)))
    return FALSE;       // weapon not equipped (carried or on ground)
  if(::number(0,chance))
    return FALSE;

  if(cmd == CMD_GENERIC_PULSE){
    if(!::number(0,99) && o->objVnum()==DEVASTATOR){
      doSanc(ch, o);
      return TRUE;
    }
    
    if(!::number(0,49)){
      doHeal(ch, o);
      return TRUE;
    }
    return TRUE;
  }
  
  if (cmd == CMD_OBJ_HIT && vict){
    if(!::number(0,9) && 
       (o->objVnum()==DEVASTATOR || o->objVnum()==VINDICATOR)){
      doBlind(ch, vict, o);      
      return TRUE;
    }

    if(!::number(0,3)){
      return doHarm(ch, vict, o);
    }
    return TRUE;
  }
  
  return FALSE;
}


