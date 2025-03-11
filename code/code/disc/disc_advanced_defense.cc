#include "disc_advanced_defense.h"
#include "being.h"

// The larger perc is passed in the harder it is to avoid
bool TBeing::canFocusedAvoidance(int perc) {
  if (!doesKnowSkill(SKILL_FOCUSED_AVOIDANCE))
    return FALSE;

  if (!awake() || getPosition() < POSITION_CRAWLING)
    return FALSE;

  int skill = getSkillValue(SKILL_FOCUSED_AVOIDANCE);

  // Agi seems to be the defense stat so this makes sense
  skill *= getStatMod(STAT_AGI);

  if (eitherLegHurt())
    skill = (skill * 0.75);

  skill -= perc;

  // bSuccess is modified by focus
  if (!bSuccess(skill, SKILL_FOCUSED_AVOIDANCE))
    return FALSE;

  return TRUE;
}
