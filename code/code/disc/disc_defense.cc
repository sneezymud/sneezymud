//////////////////////////////////////////////////////////////////////////
//
// SneezyMUD - All rights reserved, SneezyMUD Coding Team
//
//////////////////////////////////////////////////////////////////////////


#include "disc_defense.h"
#include "being.h"


CDDefense::CDDefense() :
  CDiscipline(),
  skAdvancedDefense(),
  skFocusedAvoidance()
{
}

CDDefense::CDDefense(const CDDefense &a) :
  CDiscipline(a),
  skAdvancedDefense(a.skAdvancedDefense),
  skFocusedAvoidance(a.skFocusedAvoidance)
{
}

CDDefense & CDDefense::operator=(const CDDefense &a)
{
  if (this == &a) return *this;
  CDiscipline::operator=(a);
  skAdvancedDefense = a.skAdvancedDefense;
  skFocusedAvoidance = a.skFocusedAvoidance;
  return *this;
}

CDDefense::~CDDefense()
{
}

// The larger perc is passed in the harder it is to avoid
bool TBeing::canFocusedAvoidance(int perc) {

  if (!doesKnowSkill(SKILL_FOCUSED_AVOIDANCE))
    return FALSE;

  if (!awake() || getPosition() < POSITION_CRAWLING)
    return FALSE;

  int skill = getSkillValue(SKILL_FOCUSED_AVOIDANCE);

  // Agi seems to be the defense stat so this makes sense
  skill *= getAgiMod();

  if (eitherLegHurt())
    skill = (int) (skill * 0.75);

  skill -= perc;

  // bSuccess is modified by focus
  if (!bSuccess(skill, SKILL_FOCUSED_AVOIDANCE))
    return FALSE;

  return TRUE;
}