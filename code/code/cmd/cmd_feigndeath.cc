//////////////////////////////////////////////////////////////////////////
//
// SneezyMUD - All rights reserved, SneezyMUD Coding Team
//
//////////////////////////////////////////////////////////////////////////

#include <list>

#include "being.h"
#include "comm.h"
#include "defs.h"
#include "enum.h"
#include "monster.h"
#include "room.h"
#include "skills.h"
#include "spells.h"
#include "thing.h"

static int feignDeath(TBeing* caster) {
  TRoom* rp;
  TThing* t = nullptr;

  if (!caster->fight()) {
    caster->sendTo("But you are not fighting anything...\n\r");
    return false;
  }
  if (!caster->doesKnowSkill(SKILL_FEIGN_DEATH) && !caster->isImmortal()) {
    caster->sendTo("You don't know how to do that!\n\r");
    return false;
  }
  if (caster->riding) {
    caster->sendTo("Yeah... right... while mounted.\n\r");
    return false;
  }
  if (caster->isFlying()) {
    caster->sendTo("Yeah... right... while flying.\n\r");
    return false;
  }
  if (!(rp = caster->roomp))
    return false;

  caster->sendTo("You try to fake your own demise.\n\r");

  int bKnown = caster->getSkillValue(SKILL_FEIGN_DEATH);
  if (caster->bSuccess(bKnown, SKILL_FEIGN_DEATH)) {
    caster->deathCry();
    act("$n is dead! R.I.P.", false, caster, 0, 0, TO_ROOM);

    caster->stopFighting();
    caster->setPosition(POSITION_SLEEPING);

    for (StuffIter it = rp->stuff.begin(); it != rp->stuff.end() && (t = *it);
         ++it) {
      TBeing* tc = dynamic_cast<TBeing*>(t);
      if (!tc)
        continue;
      if (tc->fight() == caster) {
        tc->stopFighting();
        if (::number(1, 101) < bKnown / 2) {
          if (!tc->isPc()) {
            TMonster* tmons = dynamic_cast<TMonster*>(tc);
            if (tmons->Hates(caster, nullptr))
              tmons->remHated(caster, nullptr);
          }
        }
      }
    }
  } else {
    caster->deathCry();
    act("$n makes a lousy attempt at playing possum.", false, caster, 0, 0,
      TO_ROOM);

    switch (critFail(caster, SKILL_FEIGN_DEATH)) {
      case CRIT_F_HITOTHER:
        caster->sendTo(
          "You stop your heart for too long and kill yourself!\n\r");
        caster->applyDamage(caster, (20 * caster->hitLimit()),
          DAMAGE_SUFFOCATION);
        return DELETE_THIS;
      case CRIT_F_HITSELF:
      case CRIT_F_NONE:
        caster->setPosition(POSITION_SLEEPING);
    }
  }
  return true;
}

int TBeing::doFeignDeath() {
  int rc;

  if (!doesKnowSkill(SKILL_FEIGN_DEATH)) {
    sendTo("You aren't a very good actor.\n\r");
    return false;
  }

  rc = feignDeath(this);
  if (rc)
    addSkillLag(SKILL_FEIGN_DEATH, rc);

  return rc;
}
