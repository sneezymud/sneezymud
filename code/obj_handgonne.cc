#include "stdsneezy.h"
#include "obj_handgonne.h"
#include "obj_general_weapon.h"
#include "obj_base_weapon.h"
#include "obj_gun.h"
#include "range.h"
#include "obj_arrow.h"
#include "obj_tool.h"

// this is a hand held cannon-lock firearm - essentially a small cannon
// it takes a long time to load and is virtually unaimable



int THandgonne::shootMeBow(TBeing *ch, TBeing *targ, unsigned int count, dirTypeT dir, int shoot_dist)
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

  if((!ch->roomp->isIndoorSector() && weather_info.sky==SKY_RAINING) ||
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
    bullet=read_object(31869, VIRTUAL);
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
      ch->sendTo(COLOR_BASIC, fmt("<Y>BANG!<1>  A loud blast sounds as you ignite %s.\n\r") % shortDescr);
      ch->sendTo(COLOR_MOBS, fmt("You shoot %s out of %s at %s.\n\r") %
		 capbuf.uncap() % capbuf2.uncap() %
		 targ->getName());
    } else {
      ch->sendTo(COLOR_BASIC, fmt("<Y>BANG!<1>  A loud blast sounds as you ignite %s.\n\r") % shortDescr);
      ch->sendTo(fmt("You shoot %s out of %s.\n\r") %
		 capbuf.uncap() % 
		 capbuf2.uncap());
    }    

    act("<Y>BANG!<1>  A loud blast sounds as $n ignites $p.",
	FALSE, ch, this, bullet, TO_ROOM);
    sprintf(buf, "$n points $p %swards, and shoots $N out of it.",
	    dirs[dir]);
    act(buf, FALSE, ch, this, bullet, TO_ROOM);

    ch->dropSmoke(::number(1,5));
    
    // put the bullet in the room and then "throw" it
    *ch->roomp += *bullet;    
    
    int rc = throwThing(bullet, dir, ch->in_room, &targ, shoot_dist, 1, ch);

    if(!isSilenced())
      ch->roomp->getZone()->sendTo("A gunshot echoes in the distance.\n\r",
				   ch->in_room);

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





int task_handgonne_load(TBeing *ch, cmdTypeT cmd, const char *, int pulse, TRoom *rp, TObj *o)
{
  THandgonne *handgonne=dynamic_cast<THandgonne *>(o);
  TAmmo *shot;
  TTool *powder;
  TThing *t;
  sstring buf;

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

  // find powder
  TThing *ss=ch->getStuff();

  t=findPowder(ss, 1);
  
  int m=WEAR_NOWHERE;
  while(!t && m<MAX_WEAR){
    ++m;
    t=findPowder(ch->equipment[m], 1);
  }

  powder=dynamic_cast<TTool *>(t);

  if(!powder){
    ch->sendTo("You need to have some black powder.\n\r");
    ch->stopTask();
    return FALSE;
  }

  // find shot
  ss=ch->getStuff();

  t=findShot(ss, AMMO_LEAD_SHOT);
  
  m=WEAR_NOWHERE;
  while(!t && m<MAX_WEAR){
    ++m;
    t=findShot(ch->equipment[m], AMMO_LEAD_SHOT);
  }

  shot=dynamic_cast<TAmmo *>(t);

  if(!shot && !handgonne->getAmmo()){
    ch->sendTo("You need to have some shot.\n\r");
    ch->stopTask();
    return FALSE;
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
	case 3:
	  // powder
	  handgonne->addToFlags(GUN_FLAG_FOULED);

	  powder->addToToolUses(-1);
	  if (powder->getToolUses() <= 0) {
	    act("You use the last of your powder.",
		FALSE, ch, NULL, 0, TO_CHAR);
	    delete powder;
	  }

	  buf = fmt("You pack some powder from $p into %s.") %
		   handgonne->shortDescr;
	  act(buf, FALSE, ch, powder, 0, TO_CHAR);

	  buf = fmt("$n packs some powder from $p into %s.") %
		   handgonne->shortDescr;
          act(buf, TRUE, ch, powder, 0, TO_ROOM);
          ch->task->timeLeft--;
          break;
	case 2:
	  // plug
	  act("You push a <o>wooden plug<1> down the barrel of $N.",
	      TRUE, ch, shot, handgonne, TO_CHAR);
	  act("$n pushes a <o>wooden plug<1> down the barrel of $N.",
	      TRUE, ch, shot, handgonne, TO_ROOM);
	  ch->task->timeLeft--;
	  break;
	case 1:
	  // shot
	  --(*shot);
	  handgonne->setAmmo(shot);

	  act("You load $p into $N.", TRUE, ch, shot, handgonne, TO_CHAR);
	  act("$n loads $p into $N.", TRUE, ch, shot, handgonne, TO_ROOM);
	  
          ch->task->timeLeft--;
          break;
	case 0:
	  // primer
	  act("You pour priming powder into the touchhole of $N.",
	      TRUE, ch, shot, handgonne, TO_CHAR);
	  act("$n pours priming powder into the touchhole of $N.",
	      TRUE, ch, shot, handgonne, TO_ROOM);

	  ch->sendTo(COLOR_BASIC, fmt("You have finished loading %s.\n\r") % handgonne->shortDescr);
	  handgonne->remFromFlags(GUN_FLAG_FOULED);
	  ch->stopTask();
	  break;
      }
      break;
    case CMD_ABORT:
    case CMD_STOP:
      act("You cease loading.",
          FALSE, ch, 0, 0, TO_CHAR);
      act("$n stops loading.",
          TRUE, ch, 0, 0, TO_ROOM);
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

void THandgonne::loadMe(TBeing *ch, TAmmo *ammo)
{

  // find black powder
  // check for flint and steel

  ch->sendTo(COLOR_BASIC, fmt("You start loading %s.\n\r") % shortDescr);

  start_task(ch, this, ch->roomp, TASK_HANDGONNE_LOAD, "", 3, ch->inRoom(), 0, 0, 5);

}

void THandgonne::unloadMe(TBeing *ch, TAmmo *ammo)
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



THandgonne::THandgonne() :
  TGun()
{
}

THandgonne::THandgonne(const THandgonne &a) :
  TGun(a)
{
}

THandgonne & THandgonne::operator=(const THandgonne &a)
{
  if (this == &a) return *this;
  TGun::operator=(a);
  return *this;
}

THandgonne::~THandgonne()
{
}

