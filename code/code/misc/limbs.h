#pragma once

#include "discipline.h"
#include "spells.h"

class TBeing;

enum wearSlotT {
  WEAR_NOWHERE,
  WEAR_HEAD,
  WEAR_NECK,
  WEAR_BODY,
  WEAR_BACK,
  WEAR_ARM_R,
  WEAR_ARM_L,
  WEAR_WRIST_R,
  WEAR_WRIST_L,
  WEAR_HAND_R,
  WEAR_HAND_L,
  WEAR_FINGER_R,
  WEAR_FINGER_L,
  WEAR_WAIST,
  WEAR_LEG_R,
  WEAR_LEG_L,
  WEAR_FOOT_R,
  WEAR_FOOT_L,
  HOLD_RIGHT,
  HOLD_LEFT,
  WEAR_EX_LEG_R,
  WEAR_EX_LEG_L,
  WEAR_EX_FOOT_R,
  WEAR_EX_FOOT_L,
  MAX_WEAR,
  MIN_WEAR = WEAR_HEAD,
  MAX_HUMAN_WEAR = HOLD_LEFT + 1,
};

extern wearSlotT& operator++(wearSlotT&, int);
extern wearSlotT pickRandomLimb(bool = false);

struct TransformLimbType {
    char name[20];
    int level;
    int learning;
    char newName[20];
    wearSlotT limb;
    spellNumT spell;
    discNumT discipline;
};

extern bool has_healthy_body(TBeing*);
extern void break_bone(TBeing*, wearSlotT which);
extern bool canDestroyLimbViolently(const TBeing* victim, wearSlotT limb);
