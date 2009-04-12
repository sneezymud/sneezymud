//////////////////////////////////////////////////////////////////////////
//
// SneezyMUD - All rights reserved, SneezyMUD Coding Team
//
//
//////////////////////////////////////////////////////////////////////////


#ifndef __DISC_IRON_BODY_H
#define __DISC_IRON_BODY_H

#include "discipline.h"
#include "skills.h"

class CDIronBody : public CDiscipline
{
public:
  CSkill skIronFist;
  CSkill skIronFlesh;
  CSkill skIronSkin;
  CSkill skIronBones;
  CSkill skIronMuscles;
  CSkill skIronLegs;
  CSkill skIronWill;

    CDIronBody()
      : CDiscipline(),
      skIronFist(),
      skIronFlesh(),
      skIronSkin(),
      skIronBones(),
      skIronMuscles(),
      skIronLegs(),
      skIronWill(){
    }
    CDIronBody(const CDIronBody &a)
      : CDiscipline(a),
      skIronFist(a.skIronFist),
      skIronFlesh(a.skIronFlesh),
      skIronSkin(a.skIronSkin),
      skIronBones(a.skIronBones),
      skIronMuscles(a.skIronMuscles),
      skIronLegs(a.skIronLegs),
      skIronWill(a.skIronWill){
    }
    CDIronBody & operator=(const CDIronBody &a) {
      if (this == &a) return *this;
      CDiscipline::operator=(a);
      skIronFist = a.skIronFist;
      skIronFlesh = a.skIronFlesh;
      skIronSkin = a.skIronSkin;
      skIronBones = a.skIronBones;
      skIronMuscles = a.skIronMuscles;
      skIronLegs = a.skIronLegs;
      skIronWill = a.skIronWill;

      return *this;
    }
    virtual ~CDIronBody() {}
    virtual CDIronBody * cloneMe() { return new CDIronBody(*this); }

private:
};

#endif
