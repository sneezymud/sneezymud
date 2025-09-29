
#include "being.h"
#include "enum.h"
#include "monster.h"
#include "disc_deikhan_mounted.h"
#include "spells.h"
#include "combat.h"

// return 0-100, which is an average of advanced riding and ride xxx
// does learning
int TBeing::advancedRidingBonus(TMonster* mount) {
  int skillTotal = 0;

  if (!mount)
    return 0;

  if (doesKnowSkill(SKILL_ADVANCED_RIDING)) {
    if (bSuccess(SKILL_ADVANCED_RIDING))
      skillTotal += getSkillValue(SKILL_ADVANCED_RIDING);
  }
  if (doesKnowSkill(mount->mountSkillType())) {
    if (bSuccess(mount->mountSkillType()))
      skillTotal += getSkillValue(mount->mountSkillType());
  }

  return (skillTotal / 2);
}

void TBeing::calmMount(TBeing* m) {
  TMonster* mount = NULL;
  int skillTotal = 0, amt;

  if (!m || !doesKnowSkill(SKILL_CALM_MOUNT) ||
      !(mount = dynamic_cast<TMonster*>(m)) || !bSuccess(SKILL_CALM_MOUNT))
    return;

  skillTotal += getSkillValue(SKILL_CALM_MOUNT);  // 1/2 calm mount
  skillTotal += advancedRidingBonus(mount);  // 1/2 advanced riding/ride xxx

  amt = ::number(0, skillTotal / 30);  // 0-6
  if ((mount->anger() + 20) > mount->defanger())
    mount->DA(amt);
  if ((mount->malice() + 20) > mount->defmalice())
    mount->DMal(amt);
  if ((mount->susp() + 20) > mount->defsusp())
    mount->DS(amt);
}

int TBeing::tryVault(TBeing* deikhan, TBeing* victim, TBeing* mount) {
  if (!deikhan || !victim || !mount)
    return FALSE;
  if (deikhan->doesKnowSkill(SKILL_VAULTING)) {
    doVault(deikhan, victim, mount);
  }
  return TRUE;
}

int TBeing::doVault(TBeing* deikhan, TBeing* victim, TBeing* mount) {
  if (!deikhan || !victim || !mount)
    return FALSE;
  TObj* saddle = dynamic_cast<TObj*>(mount->equipment[WEAR_BACK]);
  if (!saddle) {
    deikhan->sendTo("You reach out for a pommel as you fall, but your hand grasps at nothing.\n\r");
    return FALSE;
  }

  if (deikhan->getCond(DRUNK) > 9 && !deikhan->bSuccess(SKILL_ALCOHOLISM)) {
    deikhan->sendTo("You are too drunk and your vault is a total mess.\n\r");
    return FALSE;
  }

  int mod = 0;

  if (deikhan->getCond(DRUNK) && !deikhan->bSuccess(SKILL_ALCOHOLISM)) {
    mod -= deikhan->getCond(DRUNK); 
  }

  if (deikhan->isAgile(0)) {
    mod += 5;
  }

  if (deikhan->isBrawny()) {
    mod += 5;
  }

  if (deikhan->doesKnowSkill(SKILL_ADVANCED_RIDING) && deikhan->bSuccess(SKILL_ADVANCED_RIDING)) {
    mod += 5;
  }

  if (deikhan->doesKnowSkill(mount->mountSkillType()) && deikhan->bSuccess(mount->mountSkillType())) {
    mod += 5;
  }

  if (deikhan->bSuccess(mod, SKILL_VAULTING)) {
    // SKILL SUCCESS - Mount the deikhan
    act("You swing yourself up into the saddle atop $N.", FALSE, deikhan, 0, mount, TO_CHAR);
    act("$n swings $mself up into the saddle atop $N.", FALSE, deikhan, 0, mount, TO_ROOM);
    
    // Mount the deikhan on the mount
    deikhan->mount(mount);
    deikhan->setPosition(POSITION_MOUNTED);
    
    // Now attempt special attack on victim
    int attackResult = deikhan->specialAttack(victim, SKILL_VAULTING, mod, STAT_AGI, STAT_BRA, STAT_AGI, STAT_CON, true);
    
    if (attackResult == COMPLETE_SUCCESS || attackResult == GUARANTEED_SUCCESS) {
      // SPECIAL ATTACK HIT - Calculate damage and effects
      int dam = deikhan->getSkillDam(victim, SKILL_VAULTING, deikhan->getSkillLevel(SKILL_VAULTING), deikhan->getAdvLearning(SKILL_VAULTING));
      
      if (deikhan->willKill(victim, dam, SKILL_VAULTING, TRUE)) {
        // VICTIM DEATH - Special death messages
        act("Your mighty kick devastates $N completely as you vault into the saddle!", FALSE, deikhan, 0, victim, TO_CHAR);
        act("$n's mounted charge devastates you completely as $E vaults into the saddle!", FALSE, deikhan, 0, victim, TO_VICT);
        act("$n's mounted charge devastates $N completely as $E vaults into the saddle!", FALSE, deikhan, 0, victim, TO_NOTVICT);
      } else {
        // VICTIM SURVIVES - Normal hit messages
        act("As you swing into the saddle, you strike $N with a hefty kick!", FALSE, deikhan, 0, victim, TO_CHAR);
        act("$n lands a hefty kick on you as $E vaults into the saddle.", FALSE, deikhan, 0, victim, TO_VICT);
        act("$n lands a hefty kick on $N as $E vaults into the saddle.", FALSE, deikhan, 0, victim, TO_NOTVICT);
      }
      
      if (deikhan->reconcileDamage(victim, dam, SKILL_VAULTING) == -1)
        return DELETE_VICT;
      victim->addToWait(combatRound(1)); 
    } else {
      // SPECIAL ATTACK MISS - Miss messages
      act("Your mounted charge misses $N completely!", FALSE, deikhan, 0, victim, TO_CHAR);
      act("$n's mounted charge misses you completely!", FALSE, deikhan, 0, victim, TO_VICT);
      act("$n's mounted charge misses $N completely!", FALSE, deikhan, 0, victim, TO_NOTVICT);
      deikhan->reconcileDamage(victim, 0, SKILL_VAULTING);
    }
    
    return TRUE;
    
  } else {
    // SKILL FAILURE - Effects on deikhan
    act("You fail to vault properly and fall awkwardly!", FALSE, deikhan, 0, 0, TO_CHAR);
    act("$n fails to vault properly and falls awkwardly!", FALSE, deikhan, 0, 0, TO_ROOM);

    int rc = deikhan->crashLanding(0, false);
    if (IS_SET_DELETE(rc, DELETE_THIS))
      return DELETE_THIS;
    
    return TRUE;
  }
}

int TBeing:: doSaddlePosture(TBeing* deikhan, TBeing* mount) {
  if (!deikhan || !mount)
    return FALSE;

  int skillLevel = deikhan->getSkillLevel(SKILL_SADDLE_POSTURE);  
  int mod = 0;

  if (!deikhan->doesKnowSkill(SKILL_SADDLE_POSTURE))
    return FALSE;

  if (deikhan->riding != mount) {
    deikhan->sendTo("You must be mounted atop a steed to assume a saddle posture.\n\r");
    return FALSE;
  }

  TObj* saddle = dynamic_cast<TObj*>(mount->equipment[WEAR_BACK]);
  
  if (!saddle && skillLevel < 50) {
    deikhan->sendTo("You are not skilled enough to achieve proper posture without a saddle.\n\r");
    return FALSE;
  }

  if (deikhan->isAgile(0)) {
    mod += 5;
  }

  if (deikhan->isBrawny()) {
    mod += 5;
  }

  if (deikhan->doesKnowSkill(SKILL_ADVANCED_RIDING) && deikhan->bSuccess(SKILL_ADVANCED_RIDING)) {
    mod += 5;
  }

  if (deikhan->doesKnowSkill(mount->mountSkillType()) && deikhan->bSuccess(mount->mountSkillType())) {
    mod += 5;
  }

  if (deikhan->fight()) {
    
  }

  if (deikhan->bSuccess(skillLevel + mod, SKILL_SADDLE_POSTURE)) {
    act("You assume a proper saddle posture and remain seated.", FALSE, deikhan, 0, 0, TO_CHAR);
    act("$n assumes a proper saddle posture and remains seated.", FALSE, deikhan, 0, 0, TO_ROOM);
    return TRUE;
  } else {
    act("You attempt to assume a proper saddle posture, but fail.", FALSE, deikhan, 0, 0, TO_CHAR);
    act("$n attempts to assume a proper saddle posture, but fails.", FALSE, deikhan, 0, 0, TO_ROOM);
    return FALSE;
  }
}
