#include <boost/format.hpp>
#include <memory>

#include "ansi.h"
#include "being.h"
#include "colorstring.h"
#include "comm.h"
#include "db.h"
#include "defs.h"
#include "extern.h"
#include "limbs.h"
#include "obj_arrow.h"
#include "obj_gun.h"
#include "obj_handgonne.h"
#include "obj_tool.h"
#include "parse.h"
#include "range.h"
#include "room.h"
#include "spells.h"
#include "sstring.h"
#include "structs.h"
#include "task.h"
#include "thing.h"
#include "weather.h"

// this is a hand held single-shot cannon-lock firearm
// it takes a long time to load and is virtually unaimable

int THandgonne::shootMeBow(TBeing* ch, TBeing* targ, unsigned int count,
  dirTypeT dir, int shoot_dist) {
  TAmmo* ammo;
  TObj* bullet;

  if (targ &&
      ch->checkPeacefulVictim(
        "They are in a peaceful room. You can't seem to fire the gun.\n\r",
        targ))
    return false;

  if (targ && ch->noHarmCheck(targ))
    return false;

  // find flint
  TTool* flint;
  TThing* t;

  t = findFlint(ch->stuff);

  int m = WEAR_NOWHERE;
  while (!t && m < MAX_WEAR) {
    ++m;
    t = findFlint(ch->equipment[m]->stuff);
  }

  ammo = dynamic_cast<TAmmo*>(getAmmo());
  flint = dynamic_cast<TTool*>(t);

  if (!flint) {
    ch->sendTo("You need to have some flint and steel to light it.\n\r");
    return false;
  }

  flint->addToToolUses(-1);
  if (flint->getToolUses() <= 0) {
    act("You use the last of your flint and steel.", false, ch, nullptr, 0,
      TO_CHAR);
    delete flint;
  }

  act("You light a fuse and bring it to the touchhole of $N!", true, ch, nullptr,
    this, TO_CHAR);
  act("$n lights a fuse and brings it to the touchhole of $N!", true, ch, nullptr,
    this, TO_ROOM);

  if (!ammo) {
    act("Nothing happens.  $N is not loaded.", true, ch, nullptr, this, TO_CHAR);
    return false;
  }

  // 1% backfire rate
  if (!::number(0, 99)) {
    act("<r>Something goes wrong and <1>$N<r> explodes in your face!<1>", true,
      ch, nullptr, this, TO_CHAR);
    act("<r>Something goes wrong and <1>$N<r> expodes in $n's face!<1>", true,
      ch, nullptr, this, TO_ROOM);

    addToFlags(GUN_FLAG_FOULED);

    int rc = ch->objDamage(DAMAGE_TRAP_TNT, ::number(25, 100), this);
    if (IS_SET_DELETE(rc, DELETE_THIS))
      return DELETE_THIS;

    return false;
  }

  // 10% failure rate
  if (!::number(0, 9))
    addToFlags(GUN_FLAG_FOULED);

  if (isFouled()) {
    act("Nothing happens, $N has been fouled and won't fire!", true, ch, nullptr,
      this, TO_CHAR);
    return false;
  }

  if ((!ch->roomp->isIndoorSector() &&
        Weather::getSky() == Weather::SKY_RAINING) ||
      ch->roomp->isUnderwaterSector()) {
    act("Nothing happens, $N has been fouled by wet weather!", true, ch, nullptr,
      this, TO_CHAR);

    addToFlags(GUN_FLAG_FOULED);
    return false;
  }

  --(*ammo);
  delete ammo;

  // grab a bullet object and adjust for damage
  bullet = read_object(31869, VIRTUAL);
  TArrow* tmp = dynamic_cast<TArrow*>(bullet);
  if (tmp) {
    tmp->setWeapDamLvl(getWeapDamLvl());
    tmp->setWeapDamDev(getWeapDamDev());
  }

  // send messages
  sstring lc_bullet =
    colorString(ch, ch->desc, bullet->getName(), nullptr, COLOR_OBJECTS, true)
      .uncap();
  sstring lc_gun =
    colorString(ch, ch->desc, getName(), nullptr, COLOR_OBJECTS, true).uncap();

  if (targ) {
    ch->sendTo(COLOR_BASIC,
      format("<Y>BANG!<1>  A loud blast sounds as you ignite %s.\n\r") %
        shortDescr);
    ch->sendTo(COLOR_MOBS, format("You shoot %s out of %s at %s.\n\r") %
                             lc_bullet % lc_gun % targ->getName());
  } else {
    ch->sendTo(COLOR_BASIC,
      format("<Y>BANG!<1>  A loud blast sounds as you ignite %s.\n\r") %
        shortDescr);
    ch->sendTo(format("You shoot %s out of %s.\n\r") % lc_bullet % lc_gun);
  }

  act("<Y>BANG!<1>  A loud blast sounds as $n ignites $p.", false, ch, this,
    bullet, TO_ROOM);
  act(format("$n points $p %swards, and shoots $N out of it.") % dirs[dir],
    false, ch, this, bullet, TO_ROOM);

  ch->dropGas(::number(1, 5), GAS_SMOKE);

  // put the bullet in the room and then "throw" it
  *ch->roomp += *bullet;

  int rc = throwThing(bullet, dir, ch->in_room, &targ, shoot_dist, 1, ch);

  if (!isSilenced())
    ch->roomp->getZone()->sendTo("A gunshot echoes in the distance.\n\r",
      ch->in_room);

  // delete the bullet afterwards, arbitrary decision
  // since they are arrow type and you usually don't find spent lead anyway
  delete bullet;
  bullet = nullptr;

  if (IS_SET_DELETE(rc, DELETE_VICT)) {
    delete targ;
    targ = nullptr;
    return false;
  }

  ch->addToWait(combatRound(1));

  return false;
}

int task_handgonne_load(TBeing* ch, cmdTypeT cmd, const char*, int pulse,
  TRoom* rp, TObj* o) {
  THandgonne* handgonne = dynamic_cast<THandgonne*>(o);
  TAmmo* shot;
  TTool* powder;
  TThing* t;

  if (ch->utilityTaskCommand(cmd) || ch->nobrainerTaskCommand(cmd))
    return false;

  // basic tasky safechecking
  if (ch->isLinkdead() || (ch->in_room != ch->task->wasInRoom)) {
    act("You cease loading.", false, ch, 0, 0, TO_CHAR);
    act("$n stops loading.", true, ch, 0, 0, TO_ROOM);
    ch->stopTask();
    return false;  // returning false lets command be interpreted
  }

  // find powder
  t = findPowder(ch->stuff, 1);

  int m = WEAR_NOWHERE;
  while (!t && m < MAX_WEAR) {
    ++m;
    t = findPowder(ch->equipment[m]->stuff, 1);
  }

  powder = dynamic_cast<TTool*>(t);

  if (!powder) {
    ch->sendTo("You need to have some black powder.\n\r");
    ch->stopTask();
    return false;
  }

  // find shot
  t = findShot(ch->stuff, AMMO_LEAD_SHOT);

  m = WEAR_NOWHERE;
  while (!t && m < MAX_WEAR) {
    ++m;
    t = findShot(ch->equipment[m]->stuff, AMMO_LEAD_SHOT);
  }

  shot = dynamic_cast<TAmmo*>(t);

  if (!shot) {
    ch->sendTo("You need to have some shot.\n\r");
    ch->stopTask();
    return false;
  }

  if (ch->task && ch->task->timeLeft < 0) {
    ch->sendTo("You stop loading.\n\r");
    ch->stopTask();
    return false;
  }

  switch (cmd) {
    case CMD_TASK_CONTINUE:
      ch->task->calcNextUpdate(pulse, Pulse::MOBACT * 5);

      switch (ch->task->timeLeft) {
        case 3:
          // powder
          handgonne->addToFlags(GUN_FLAG_FOULED);

          act("You pack some powder from $p into %N.", false, ch, powder, 0,
            TO_CHAR);
          act("$n packs some powder from $p into %N.", true, ch, powder, 0,
            TO_ROOM);

          powder->addToToolUses(-1);
          if (powder->getToolUses() <= 0) {
            act("You use the last of your powder.", false, ch, nullptr, 0,
              TO_CHAR);
            delete powder;
          }

          ch->task->timeLeft--;
          break;
        case 2:
          // plug
          act("You push a <o>wooden plug<1> down the barrel of $N.", true, ch,
            shot, handgonne, TO_CHAR);
          act("$n pushes a <o>wooden plug<1> down the barrel of $N.", true, ch,
            shot, handgonne, TO_ROOM);
          ch->task->timeLeft--;
          break;
        case 1:
          // shot
          --(*shot);
          handgonne->setAmmo(shot);

          act("You load $p into $N.", true, ch, shot, handgonne, TO_CHAR);
          act("$n loads $p into $N.", true, ch, shot, handgonne, TO_ROOM);

          ch->task->timeLeft--;
          break;
        case 0:
          // primer
          act("You pour priming powder into the touchhole of $N.", true, ch,
            shot, handgonne, TO_CHAR);
          act("$n pours priming powder into the touchhole of $N.", true, ch,
            shot, handgonne, TO_ROOM);

          ch->sendTo(COLOR_BASIC, format("You have finished loading %s.\n\r") %
                                    handgonne->shortDescr);
          handgonne->remFromFlags(GUN_FLAG_FOULED);
          ch->stopTask();
          break;
      }
      break;
    case CMD_ABORT:
    case CMD_STOP:
      act("You cease loading.", false, ch, 0, 0, TO_CHAR);
      act("$n stops loading.", true, ch, 0, 0, TO_ROOM);
      ch->stopTask();
      break;
    case CMD_TASK_FIGHTING:
      ch->sendTo("You cannot fight while loading.\n\r");
      ch->stopTask();
      break;
    default:
      if (cmd < MAX_CMD_LIST)
        warn_busy(ch);
      break;  // eat the command
  }
  return true;
}

void THandgonne::loadMe(TBeing* ch, TAmmo* ammo) {
  // find black powder
  // check for flint and steel

  if (isFouled()) {
    ch->sendTo(
      format("%s is fouled and must be unloaded first\n\r") % shortDescr);
    return;
  }

  ch->sendTo(COLOR_BASIC, format("You start loading %s.\n\r") % shortDescr);

  start_task(ch, this, ch->roomp, TASK_HANDGONNE_LOAD, "", 3, ch->inRoom(), 0,
    0, 5);
}

void THandgonne::unloadMe(TBeing* ch, TAmmo* ammo) {
  TThing* arrow = dynamic_cast<TThing*>(ammo);

  if (arrow) {
    --(*arrow);
    *ch += *arrow;

    act("You unload $N.", true, ch, ammo, this, TO_CHAR);
    act("$n unloads $N.", true, ch, ammo, this, TO_ROOM);
  }

  act("You clear out $N.", true, ch, ammo, this, TO_CHAR);
  act("$n clears out $N.", true, ch, ammo, this, TO_ROOM);

  remFromFlags(GUN_FLAG_FOULED);

  ch->addToWait(combatRound(1));
}

THandgonne::THandgonne() : TGun() {}

THandgonne::THandgonne(const THandgonne& a) : TGun(a) {}

THandgonne& THandgonne::operator=(const THandgonne& a) {
  if (this == &a)
    return *this;
  TGun::operator=(a);
  return *this;
}

THandgonne::~THandgonne() {}
