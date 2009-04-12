//////////////////////////////////////////////////////////////////////////
//
// SneezyMUD - All rights reserved, SneezyMUD Coding Team
//
//////////////////////////////////////////////////////////////////////////


#include "being.h"
#include "monster.h"
#include "disc_mounted.h"

// return 0-100, which is an average of advanced riding and ride xxx
// does learning
// Peel
int TBeing::advancedRidingBonus(TMonster *mount){
  int skillTotal=0;

  if(!mount)
    return 0;

  if(doesKnowSkill(SKILL_ADVANCED_RIDING)){
    if(bSuccess(SKILL_ADVANCED_RIDING))
      skillTotal+=getSkillValue(SKILL_ADVANCED_RIDING);
  }
  if(doesKnowSkill(mount->mountSkillType())){
    if(bSuccess(mount->mountSkillType()))
      skillTotal+=getSkillValue(mount->mountSkillType());
  }

  return(skillTotal/2);
}

// Peel
void TBeing::calmMount(TBeing *m){
  TMonster *mount=NULL;
  int skillTotal=0, amt;

  if(!m || !doesKnowSkill(SKILL_CALM_MOUNT) || 
     !(mount=dynamic_cast<TMonster *>(m)) ||
     !bSuccess(SKILL_CALM_MOUNT))
    return;

  skillTotal+=getSkillValue(SKILL_CALM_MOUNT); // 1/2 calm mount
  skillTotal+=advancedRidingBonus(mount);      // 1/2 advanced riding/ride xxx
 
  amt=::number(0, skillTotal/30);  // 0-6
  if((mount->anger()+20) > mount->defanger())
     mount->DA(amt);
  if((mount->malice()+20) > mount->defmalice())
    mount->DMal(amt);
  if((mount->susp()+20) > mount->defsusp())
    mount->DS(amt);
}
