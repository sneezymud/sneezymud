//////////////////////////////////////////////////////////////////////////
//
// SneezyMUD - All rights reserved, SneezyMUD Coding Team
//
// $Log: disc_mounted.cc,v $
// Revision 1.1  1999/09/12 17:24:04  peel
// Initial revision
//
//
//////////////////////////////////////////////////////////////////////////


#include "stdsneezy.h"
#include "disc_mounted.h"

// return 0-100, which is an average of advanced riding and ride xxx
// does learning
// Peel
int TBeing::advancedRidingBonus(TMonster *mount){
  int skillTotal=0;

  if(!mount)
    return 0;

  if(doesKnowSkill(SKILL_ADVANCED_RIDING)){
    if(bSuccess(this, getSkillValue(SKILL_ADVANCED_RIDING),
		SKILL_ADVANCED_RIDING))
      skillTotal+=getSkillValue(SKILL_ADVANCED_RIDING);
  }
  if(doesKnowSkill(mount->mountSkillType())){
    if(bSuccess(this, getSkillValue(mount->mountSkillType()),
		mount->mountSkillType()))
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
     !bSuccess(this, getSkillValue(SKILL_CALM_MOUNT), SKILL_CALM_MOUNT))
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
