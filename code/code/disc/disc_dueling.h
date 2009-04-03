//////////////////////////////////////////////////////////////////////////
//
// SneezyMUD - All rights reserved, SneezyMUD Coding Team
//
//////////////////////////////////////////////////////////////////////////


#ifndef __DISC_DUELING_H
#define __DISC_DUELING_H

// This is the dueling discipline.

class CDDueling : public CDiscipline
{
public:
    CSkill skShove;
    CSkill skRetreat;
    CSkill skParryWarrior;
    CSkill skTranceOfBlades;
    CSkill skWeaponRetention;
    CSkill skRiposte;

    CDDueling()
      : CDiscipline(),
      skShove(),
      skRetreat(),
      skParryWarrior(),
      skTranceOfBlades(),
      skWeaponRetention(),
      skRiposte(){
    }      
    CDDueling(const CDDueling &a)
      : CDiscipline(a),
      skShove(a.skShove),
      skRetreat(a.skRetreat),
      skParryWarrior(a.skParryWarrior),
      skTranceOfBlades(a.skTranceOfBlades),
      skWeaponRetention(a.skWeaponRetention),
      skRiposte(a.skRiposte){
    }
    CDDueling & operator=(const CDDueling &a) {
      if (this == &a) return *this;
      CDiscipline::operator=(a);
      skShove = a.skShove;
      skRetreat = a.skRetreat;
      skParryWarrior = a.skParryWarrior;
      skTranceOfBlades = a.skTranceOfBlades;
      skWeaponRetention = a.skWeaponRetention;
      skRiposte = a.skRiposte;
      return *this;
    } 
    //    CDDueling();
    //    CDDueling(const CDDueling &a);
    //    CDDueling & operator=(const CDDueling &a);
    virtual ~CDDueling() {}
    virtual CDDueling * cloneMe() { return new CDDueling(*this); }
private:
};

    int shove(TBeing *, TBeing *, char *, spellNumT);

#endif



