//////////////////////////////////////////////////////////////////////////
//
// SneezyMUD - All rights reserved, SneezyMUD Coding Team
//
//////////////////////////////////////////////////////////////////////////


#include "stdsneezy.h"
#include "disease.h"
#include "combat.h"
#include "disc_draining.h"

void lifeLeech(TBeing * caster)
{
  int hitp;
  int level;
  TThing *t, *n;

  if (!bPassClericChecks(caster, SPELL_LIFE_LEECH))
    return;

  level = caster->getClassLevel(CLASS_SHAMAN);

  for (t = caster->roomp->stuff; t; t = n) {
    n = t->nextThing;
    TBeing *tbt = dynamic_cast<TBeing *>(t);
    if (tbt && (caster != tbt) && !tbt->isImmortal()) {
      if (!caster->inGroup(*tbt)) {
        caster->reconcileHurt(tbt, 0.04);

        hitp = min(dice(level, 2), (tbt->getHit() - 1));

        if (caster->isNotPowerful(tbt, level, SPELL_LIFE_LEECH, SILENT_NO)) {
          continue;
        }

        if (bSuccess(caster, caster->getSkillValue(SPELL_LIFE_LEECH), caster->getPerc(), SPELL_LIFE_LEECH) ||
           (tbt->getPosition() > POSITION_STUNNED)) {
          act("$n drains $N's blood!", FALSE, caster, NULL, tbt, TO_NOTVICT);
          act("You drain $N's blood!", FALSE, caster, NULL, tbt, TO_CHAR);
          act("$n drains your blood!", FALSE, caster, NULL, tbt, TO_VICT);
          tbt->addToHit(-hitp);
          if (tbt->getHit() <= 0)
            tbt->setHit(1);

          caster->addToHit(hitp);

          if (caster->getHit() >= caster->hitLimit())
            caster->setHit(caster->hitLimit());

          tbt->updatePos();
        } else {
          act("You attempt to drain $N's blood but fail miserably!", FALSE, caster, NULL, tbt, TO_CHAR);
          act("$n attempts to drain your blood but fails miserably!", FALSE, caster, NULL, tbt, TO_VICT);
        }
      }
    }
  }
}

