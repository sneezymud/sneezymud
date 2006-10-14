#include "stdsneezy.h"
#include "obj_cannon.h"
#include "obj_general_weapon.h"
#include "obj_base_weapon.h"
#include "obj_gun.h"
#include "range.h"
#include "obj_arrow.h"
#include "obj_tool.h"



int TCannon::shootMeBow(TBeing *ch, TBeing *targ, unsigned int count, dirTypeT dir, int shoot_dist)
{
  TAmmo *ammo;
  TObj *bullet;
  char  buf[256];
  
  if (targ &&
      ch->checkPeacefulVictim("They are in a peaceful room. You can't seem to fire the gun.\n\r", targ))
    return FALSE;

  if (targ && ch->noHarmCheck(targ))
    return FALSE;

  sstring capbuf, capbuf2;
  
  int rof=getROF();


  // find flint
  TTool *flint;
  TThing *t;
  TThing *ss=ch->getStuff();

  t=findFlint(ss);
  
  int m=WEAR_NOWHERE;
  while(!t && m<MAX_WEAR){
    ++m;
    t=findFlint(ch->equipment[m]);
  }

  flint=dynamic_cast<TTool *>(t);

  if(!flint){
    ch->sendTo("You need to have some flint and steel to light it.\n\r");
    return FALSE;
  }
  flint->addToToolUses(-1);
  if (flint->getToolUses() <= 0) {
    act("You use the last of your flint and steel.",
	FALSE, ch, NULL, 0, TO_CHAR);
    delete flint;
  }


  act("You light a fuse and bring it to the touchhole of $N!",
      TRUE, ch, NULL, this, TO_CHAR);
  act("$n lights a fuse and brings it to the touchhole of $N!",
      TRUE, ch, NULL, this, TO_ROOM);

  // 1% backfire rate
  if(!::number(0,99)){
    act("<r>Something goes wrong and <1>$N<r> explodes in your face!<1>",
	TRUE, ch, NULL, this, TO_CHAR);
    act("<r>Something goes wrong and <1>$N<r> expodes in $n's face!<1>",
	TRUE, ch, NULL, this, TO_ROOM);

    addToFlags(GUN_FLAG_FOULED);
    
    ammo=dynamic_cast<TAmmo *>(getAmmo());

    if(ammo){
      --(*ammo);
      delete ammo;
    }
      
    int rc = ch->objDamage(DAMAGE_TRAP_TNT, ::number(25,100), this);
    if (IS_SET_DELETE(rc, DELETE_THIS))
      return DELETE_THIS;

    return FALSE;
  }


  // 10% failure rate
  if(!::number(0,9))
    addToFlags(GUN_FLAG_FOULED);
  

  if(isFouled()){
    act("Nothing happens, $N has been fouled and won't fire!",
	TRUE, ch, NULL, this, TO_CHAR);
    return FALSE;
  }

  if((!ch->roomp->isIndoorSector() && weather_info.sky==SKY_RAINING &&
      !::number(0,3)) ||
     ch->roomp->isUnderwaterSector()){
    act("Nothing happens, $N has been fouled by wet weather!",
	TRUE, ch, NULL, this, TO_CHAR);
    
    addToFlags(GUN_FLAG_FOULED);
    return false;
  }

  while(rof--){
    if(!(ammo=dynamic_cast<TAmmo *>(getAmmo())) || ammo->getRounds()<=0){
      act("Nothing happens.  $N is not loaded.",
	  TRUE, ch, NULL, this, TO_CHAR);
      break;
    }

    // grab a bullet object and adjust for damage
    bullet=read_object(19090, VIRTUAL);
    TArrow *tmp=dynamic_cast<TArrow *>(bullet);
    if(tmp){
      tmp->setWeapDamLvl(getWeapDamLvl());
      tmp->setWeapDamDev(getWeapDamDev());
    }

    // decrement ammo and drop a casing
    if(!isCaseless())
      dropSpentCasing(ch->roomp);
    setRounds(getRounds()-1);
    
    // send messages
    capbuf = colorString(ch, ch->desc, bullet->getName(), NULL, COLOR_OBJECTS, TRUE);
    capbuf2 = colorString(ch, ch->desc, getName(), NULL, COLOR_OBJECTS, TRUE);
    
    if (targ){
      ch->sendTo(COLOR_BASIC, fmt("<Y>BANG!<1>  A deafening blast sounds as you ignite %s.\n\r") % shortDescr);
      ch->sendTo(COLOR_MOBS, fmt("You shoot %s out of %s at %s.\n\r") %
		 capbuf.uncap() % capbuf2.uncap() %
		 targ->getName());
    } else {
      ch->sendTo(COLOR_BASIC, fmt("<Y>BANG!<1>  A deafening blast sounds as you ignite %s.\n\r") % shortDescr);
      ch->sendTo(fmt("You shoot %s out of %s.\n\r") %
		 capbuf.uncap() % 
		 capbuf2.uncap());
    }    

    act("<Y>BANG!<1>  A deafening blast sounds as $n ignites $p.",
	FALSE, ch, this, bullet, TO_ROOM);
    sprintf(buf, "$n points $p %swards, and shoots $N out of it.",
	    dirs[dir]);
    act(buf, FALSE, ch, this, bullet, TO_ROOM);

    ch->dropSmoke(::number(5,15));
    
    // put the bullet in the room and then "throw" it
    *ch->roomp += *bullet;    
    
    int rc = throwThing(bullet, dir, ch->in_room, &targ, shoot_dist, 1, ch);

    if(!isSilenced())
      ch->roomp->getZone()->sendTo(fmt("<R>BOOM!<1>  A loud cannon shot echoes around.\n\r"));

    // delete the bullet afterwards, arbitrary decision
    // since they are arrow type and you usually don't find spent lead anyway
    delete bullet;
    bullet = NULL;

    if (IS_SET_DELETE(rc, DELETE_VICT)) {
      delete targ;
      targ = NULL;
      return FALSE;
    }
  }

  ch->addToWait(combatRound(1));

  return FALSE;
}




int task_cannon_load(TBeing *ch, cmdTypeT cmd, const char *, int pulse, TRoom *rp, TObj *o)
{
  TCannon *cannon=dynamic_cast<TCannon *>(o);
  TAmmo *shot;
  TTool *powder;
  TThing *t, *ss;
  sstring buf;
  int m;

  if(ch->utilityTaskCommand(cmd) || ch->nobrainerTaskCommand(cmd))
    return FALSE;

  // basic tasky safechecking
  if (ch->isLinkdead() || (ch->in_room != ch->task->wasInRoom)){
    act("You cease loading.",
        FALSE, ch, 0, 0, TO_CHAR);
    act("$n stops loading.",
        TRUE, ch, 0, 0, TO_ROOM);
    ch->stopTask();
    return FALSE; // returning FALSE lets command be interpreted
  }



  if (ch->task && ch->task->timeLeft < 0){
    ch->sendTo("You stop loading.\n\r");
    ch->stopTask();
    return FALSE;
  }

  switch (cmd) {
    case CMD_TASK_CONTINUE:
      ch->task->calcNextUpdate(pulse, PULSE_MOBACT * 5);
      
      switch (ch->task->timeLeft) {
	case 5:
	  buf = fmt("You clear out the barrel of %s.") %
		   cannon->shortDescr;
	  act(buf, FALSE, ch, NULL, cannon, TO_CHAR);
	  buf = fmt("$n clears out the barrel of %s.") %
		   cannon->shortDescr;
	  act(buf, FALSE, ch, NULL, cannon, TO_ROOM);

	  ch->task->timeLeft--;
	  break;
	case 4:
	  // powder
	  cannon->addToFlags(GUN_FLAG_FOULED);

	  // find powder
	 ss=ch->getStuff();
	  
	  t=findPowder(ss, 12);
	  
	 m=WEAR_NOWHERE;
	  while(!t && m<MAX_WEAR){
	    ++m;
	    t=findPowder(ch->equipment[m], 12);
	  }
	  
	  powder=dynamic_cast<TTool *>(t);
	  
	  if(!powder){
	    ch->sendTo("You need to have a full flask of black powder.\n\r");
	    ch->stopTask();
	    return FALSE;
	  }

	  buf = fmt("You pour some powder from $p into %s.") %
		   cannon->shortDescr;
	  act(buf, FALSE, ch, powder, 0, TO_CHAR);

	  buf = fmt("$n pours some powder from $p into %s.") %
		   cannon->shortDescr;
          act(buf, FALSE, ch, powder, 0, TO_ROOM);
          ch->task->timeLeft--;

	  powder->addToToolUses(-12);
	  if (powder->getToolUses() <= 0) {
	    act("You use the last of your powder.",
		FALSE, ch, NULL, 0, TO_CHAR);
	    delete powder;
	  }


          break;
	case 3:
	  act("You pack down the charge in $N with a <o>ramrod<1>.",
	      FALSE, ch, NULL, cannon, TO_CHAR);
	  act("$n packs down the charge in $N with a <o>ramrod<1>.",
	      FALSE, ch, NULL, cannon, TO_ROOM);
	  ch->task->timeLeft--;
	  break;
	case 2:
	  // plug
	  act("You shove a <W>ball of wadding<1> down the barrel of $N.",
	      FALSE, ch, NULL, cannon, TO_CHAR);
	  act("$n shoves a <W>ball of wadding<1> down the barrel of $N.",
	      FALSE, ch, NULL, cannon, TO_ROOM);
	  ch->task->timeLeft--;
	  break;
	case 1:
	  // shot

	  // find shot
	  ss=ch->getStuff();
	  
	  t=findShot(ss, AMMO_CANNON_BALL);
	  
	  m=WEAR_NOWHERE;
	  while(!t && m<MAX_WEAR){
	    ++m;
	    t=findShot(ch->equipment[m], AMMO_CANNON_BALL);
	  }
	  
	  shot=dynamic_cast<TAmmo *>(t);
	  
	  if(!shot && !cannon->getAmmo()){
	    ch->sendTo("You need to have a cannon ball.\n\r");
	    ch->stopTask();
	    return FALSE;
	  }
  

	  --(*shot);
	  cannon->setAmmo(shot);

	  act("You load $p into $N.", FALSE, ch, shot, cannon, TO_CHAR);
	  act("$n loads $p into $N.", FALSE, ch, shot, cannon, TO_ROOM);
	  
          ch->task->timeLeft--;
          break;
	case 0:
	  // primer
	  act("You pour priming powder into the touchhole of $N.",
	      FALSE, ch, NULL, cannon, TO_CHAR);
	  act("$n pours priming powder into the touchhole of $N.",
	      FALSE, ch, NULL, cannon, TO_ROOM);

	  ch->sendTo(COLOR_BASIC, fmt("You have finished loading %s.\n\r") % cannon->shortDescr);
	  cannon->remFromFlags(GUN_FLAG_FOULED);
	  ch->stopTask();
	  break;
      }
      break;
    case CMD_ABORT:
    case CMD_STOP:
      act("You cease loading.",
          FALSE, ch, 0, 0, TO_CHAR);
      act("$n stops loading.",
          FALSE, ch, 0, 0, TO_ROOM);
      ch->stopTask();
      break;
    case CMD_TASK_FIGHTING:
      ch->sendTo("You cannot fight while loading.\n\r");
      ch->stopTask();
      break;
    default:
      if (cmd < MAX_CMD_LIST)
        warn_busy(ch);
      break;                    // eat the command
  }
  return TRUE;
}

void TCannon::loadMe(TBeing *ch, TAmmo *ammo)
{

  // find black powder
  // check for flint and steel

  ch->sendTo(COLOR_BASIC, fmt("You start loading %s.\n\r") % shortDescr);

  start_task(ch, this, ch->roomp, TASK_CANNON_LOAD, "", 5, ch->inRoom(), 0, 0, 5);

}

void TCannon::unloadMe(TBeing *ch, TAmmo *ammo)
{
  TThing *arrow=dynamic_cast<TThing *>(ammo);

  if(arrow){
    --(*arrow);
    *ch += *arrow;
    
    act("You unload $N.", TRUE, ch, ammo, this, TO_CHAR);
    act("$n unloads $N.", TRUE, ch, ammo, this, TO_ROOM);
  }

  act("You clear out $N.", TRUE, ch, ammo, this, TO_CHAR);
  act("$n clears out $N.", TRUE, ch, ammo, this, TO_ROOM);
  
  remFromFlags(GUN_FLAG_FOULED);

  ch->addToWait(combatRound(1));    
}



TCannon::TCannon() :
  TGun()
{
}

TCannon::TCannon(const TCannon &a) :
  TGun(a)
{
}

TCannon & TCannon::operator=(const TCannon &a)
{
  if (this == &a) return *this;
  TGun::operator=(a);
  return *this;
}

TCannon::~TCannon()
{
}

