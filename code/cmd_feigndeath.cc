//////////////////////////////////////////////////////////////////////////
//
// SneezyMUD - All rights reserved, SneezyMUD Coding Team
//
// $Log: cmd_feigndeath.cc,v $
// Revision 5.1  1999/10/16 04:31:17  batopr
// new branch
//
// Revision 1.1  1999/09/12 17:24:04  sneezy
// Initial revision
//
//
//////////////////////////////////////////////////////////////////////////


#include "stdsneezy.h"
#include "combat.h"
#include "disc_mindbody.h"

static int feignDeath(TBeing * caster)
{
  TRoom *rp;
  TThing *t;

  if (!caster->fight()) {
    caster->sendTo("But you are not fighting anything...\n\r");
    return FALSE;
  }
  if (!caster->hasClass(CLASS_MONK) && !caster->isImmortal()) {
    caster->sendTo("You're no monk!\n\r");
    return FALSE;
  }
  if (caster->riding) {
    caster->sendTo("Yeah... right... while mounted.\n\r");
    return FALSE;
  }
  if (caster->isFlying()) {
    caster->sendTo("Yeah... right... while flying.\n\r");
    return FALSE;
  }
  if (!(rp = caster->roomp))
    return FALSE;

  caster->sendTo("You try to fake your own demise.\n\r");

  int bKnown = caster->getSkillValue(SKILL_FEIGN_DEATH);
  if (bSuccess(caster, bKnown, SKILL_FEIGN_DEATH)) {
    caster->deathCry();
    act("$n is dead! R.I.P.", FALSE, caster, 0, 0, TO_ROOM);

    caster->stopFighting();
    caster->setPosition(POSITION_SLEEPING);

    for (t = rp->stuff; t; t = t->nextThing) {
      TBeing *tc = dynamic_cast<TBeing *>(t);
      if (!tc) continue;
      if (tc->fight() == caster) {
	tc->stopFighting();
        if (::number(1, 101) < bKnown/ 2) {
          if (!tc->isPc()){
            TMonster *tmons = dynamic_cast<TMonster *>(tc);
            if (tmons->Hates(caster, NULL)) 
              tmons->remHated(caster, NULL);
          }
        }
      }
    }
  } else {
    caster->deathCry();
    act("$n makes a lousy attempt at playing possum.", FALSE, caster, 0, 0, TO_ROOM);

    switch (critFail(caster, SKILL_FEIGN_DEATH)) {
      case CRIT_F_HITOTHER:
        caster->sendTo("You stop your heart for too long and kill yourself!\n\r");
        caster->applyDamage(caster, (20 * caster->hitLimit()), DAMAGE_SUFFOCATION);
        return DELETE_THIS;
      case CRIT_F_HITSELF:
      case CRIT_F_NONE:
        caster->setPosition(POSITION_SLEEPING);
    }
  } 
  return TRUE;
}

int TBeing::doFeignDeath()
{
  int rc;

  if (!doesKnowSkill(SKILL_FEIGN_DEATH)) {
    sendTo("You aren't a very good actor.\n\r");
    return FALSE;
  }

  rc = feignDeath(this);
  if (rc)
    addSkillLag(SKILL_FEIGN_DEATH);

  return rc;
}

