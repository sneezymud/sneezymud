#pragma once

#include "discipline.h"
#include "sstring.h"
#include "spells.h"

const int LIMB_NONE = -1;

const int LIMB_HEAD = 0;
const int LIMB_NECK = 1;
const int LIMB_BACK = 2;
const int LIMB_ARM = 3;
const int LIMB_WAIST = 4;
const int LIMB_LEG = 5;
const int LIMB_WINGS = 6;
const int LIMB_TAIL = 7;

const int LIMB_BODY = 17;
const int LIMB_FOREARM = 18;
const int LIMB_HAND = 19;
const int LIMB_FOOT = 20;

const int MAX_MAIN_LIMBS = 8;
const int MAX_LIMB_TYPES = 21;

const int LIMB_USELESS = (1 << 0);
const int LIMB_BROKEN = (1 << 1);
const int LIMB_PARALYZED = (1 << 2);
const int LIMB_CUT = (1 << 3);
const int LIMB_WITHERED = (1 << 4);
const int LIMB_NOSUBLIMB = (1 << 5);

const int MAX_STATUS = 6;

const int LEAF_NODE = (1 << 0);
const int CAN_HOLD = (1 << 1);
const int ATTACK = (1 << 2);

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
  MAX_WEAR
};

const wearSlotT MIN_WEAR = WEAR_HEAD;
const wearSlotT MAX_HUMAN_WEAR = wearSlotT(HOLD_LEFT + 1);
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
