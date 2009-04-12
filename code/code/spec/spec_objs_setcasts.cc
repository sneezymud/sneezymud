#include "comm.h"
#include "obj_base_clothing.h"
#include "being.h"

// constants that define the combo items
// combo 1
const int handItem = 36371;
const int wristItem = 36373;
const int armItem = 36370;
const int neckItem = 36366;

int doFeralEQCast(TBeing *ch, TObj *o) {
  affectedData aff2;
  TObj *hand1 = NULL;
  TObj *wrist1 = NULL;
  TObj *arm1 = NULL;
  TObj *hand2 = NULL;
  TObj *wrist2 = NULL;
  TObj *arm2 = NULL;

  if (!o || !(ch = dynamic_cast<TBeing *>(o->equippedBy)))
    return false;

  // make sure the proc is attached to only one item
  if (o->objVnum() != neckItem)
    return false;

  int modifier = 25 * ::number(80,125)/100;
  int which = ::number(1,3);

  if (ch->affectedBySpell(SPELL_FERAL_WRATH)) {
    act("$n's $o glows but then quickly fades.<1>", 0, ch, o, 0, TO_ROOM);
    act("Your $o glows but then quickly fades.<1>", 0, ch, o, 0, TO_CHAR);
    return false;
  } else {
    act("$n's $o glows with power.<1>", 0, ch, o, 0, TO_ROOM);
    act("Your $o glows with power.<1>", 0, ch, o, 0, TO_CHAR);
  }

  hand1 = dynamic_cast<TObj *>(ch->equipment[WEAR_HAND_L]);
  hand2 = dynamic_cast<TObj *>(ch->equipment[WEAR_HAND_R]);
  wrist1 = dynamic_cast<TObj *>(ch->equipment[WEAR_WRIST_L]);
  wrist2 = dynamic_cast<TObj *>(ch->equipment[WEAR_WRIST_R]);
  arm1 = dynamic_cast<TObj *>(ch->equipment[WEAR_ARM_L]);
  arm2 = dynamic_cast<TObj *>(ch->equipment[WEAR_ARM_R]);

  if ((!hand1) || (!hand2) || (!wrist1) || (!wrist2) || (!arm1) || (!arm2)) {
    act("$n's $o sparks a bit but then fizzles.<1>", 0, ch, o, 0, TO_ROOM);
    act("Your $o sparks a bit but then fizzles.<1>", 0, ch, o, 0, TO_CHAR);
    return false;
  }
  if (((hand1->objVnum() != handItem) || (hand2->objVnum() != handItem) ||
      (wrist1->objVnum() != wristItem) || (wrist2->objVnum() != wristItem) ||
      (arm1->objVnum() != armItem) || (arm2->objVnum() != armItem))) {
    act("Your $o sparks a bit and then loses it's glow.<1>", 0, ch, o, 0, TO_CHAR);
    act("$n's $o sparks a bit and then loses it's glow.<1>", 0, ch, o, 0, TO_ROOM);
    return false;
  }

  switch (which) {
    case 1:
      aff2.location = APPLY_STR;
      aff2.modifier = modifier;
      break;
    case 2:
      aff2.location = APPLY_DEX;
      aff2.modifier = modifier;
      break;
    case 3:
      aff2.location = APPLY_SPE;
      aff2.modifier = modifier;
      break;
  }
  aff2.type = SPELL_FERAL_WRATH;
  aff2.duration = 2*UPDATES_PER_MUDHOUR;
  aff2.bitvector = 0;

  if (!ch->affectJoin(ch, &aff2, AVG_DUR_NO, AVG_EFF_YES)) {
    vlogf(LOG_JESUS, "Failed to join aff2 in castingEQ proc.");
    act("Your $o sparks a bit and and then loses it's glow.<1>", 0, ch, o, 0, TO_CHAR);
  } else {
    act("$n's $o crackles with energy sending sparks into $s left wrist.<1>", 0, ch, arm1, 0, TO_ROOM);
    act("Your $o crackles with energy sending sparks into your left wrist.<1>", 0, ch, arm1, 0, TO_CHAR);
    act("$n's $o crackles with energy sending sparks into $s right wrist.<1>", 0, ch, arm2, 0, TO_ROOM);
    act("Your $o crackles with energy sending sparks into your right wrist.<1>", 0, ch, arm2, 0, TO_CHAR);
    act("$n's $o crackles with energy sending sparks into $s left hand.<1>", 0, ch, wrist1, 0, TO_ROOM);
    act("Your $o crackles with energy sending sparks into your left hand.<1>", 0, ch, wrist1, 0, TO_CHAR);
    act("$n's $o crackles with energy sending sparks into $s right hand.<1>", 0, ch, wrist2, 0, TO_ROOM);
    act("Your $o crackles with energy sending sparks into your right hand.<1>", 0, ch, wrist2, 0, TO_CHAR);
    act("$n's $o crackles with energy sending sparks back into $s whistle.<1>", 0, ch, hand1, 0, TO_ROOM);
    act("Your $o crackles with energy sending sparks back into your whistle.<1>", 0, ch, hand1, 0, TO_CHAR);
    act("$n's $o crackles with energy sending sparks back into $s whistle.<1>", 0, ch, hand2, 0, TO_ROOM);
    act("Your $o crackles with energy sending sparks back into your whistle.<1>", 0, ch, hand2, 0, TO_CHAR);
    if (aff2.location == APPLY_STR) {
      act("Your $o has given you greater strength.<1>", 0, ch, o, 0, TO_CHAR);
    } else if (aff2.location == APPLY_DEX) {
      act("Your $o has made you more nimble.<1>", 0, ch, o, 0, TO_CHAR);
    } else if (aff2.location == APPLY_SPE) {
      act("Your $o has given you the gift of speed.<1>", 0, ch, o, 0, TO_CHAR);
    }
  }
  return true;
}

// combo 2
const int waistItem = 36369;
const int legItem = 36374;
const int footItem = 36372;
int doFlyingEQCast(TBeing *ch, TObj *o) {
  affectedData aff2;
  TObj *girth = NULL;
  TObj *leg1 = NULL;
  TObj *leg2 = NULL;
  TObj *foot1 = NULL;
  TObj *foot2 = NULL;

  if (!o || !(ch = dynamic_cast<TBeing *>(o->equippedBy)))
    return false;
  // make sure the proc is attached to only one item
  if (o->objVnum() != waistItem)
    return false;

  int which = ::number(1,2);

  if (ch->affectedBySpell(SPELL_LEVITATE) || ch->affectedBySpell(SPELL_FLY)) {
    act("$n's $o glows but then quickly fades.<1>", 0, ch, o, 0, TO_ROOM);
    act("Your $o glows but then quickly fades.<1>", 0, ch, o, 0, TO_CHAR);
    return false;
  } else {
    act("$n's $o glows with power.<1>", 0, ch, o, 0, TO_ROOM);
    act("Your $o glows with power.<1>", 0, ch, o, 0, TO_CHAR);
  }

  leg1 = dynamic_cast<TObj *>(ch->equipment[WEAR_LEG_L]);
  leg2 = dynamic_cast<TObj *>(ch->equipment[WEAR_LEG_R]);
  foot1 = dynamic_cast<TObj *>(ch->equipment[WEAR_FOOT_L]);
  foot2 = dynamic_cast<TObj *>(ch->equipment[WEAR_FOOT_R]);
  girth = dynamic_cast<TObj *>(ch->equipment[WEAR_WAIST]);

  if ((!leg1) || (!leg2) || (!foot1) || (!foot2) || (!girth)) {
    act("$n's $o sparks a bit but then fizzles.<1>", 0, ch, o, 0, TO_ROOM);
    act("Your $o sparks a bit but then fizzles.<1>", 0, ch, o, 0, TO_CHAR);
    return false;
  }
  if (((leg1->objVnum() != legItem) || (leg2->objVnum() != legItem) ||
      (foot1->objVnum() != footItem) || (foot2->objVnum() != footItem) ||
      (girth->objVnum() != waistItem))) {
    act("Your $o sparks a bit and then loses it's glow.<1>", 0, ch, o, 0, TO_CHAR);
    act("$n's $o sparks a bit and then loses it's glow.<1>", 0, ch, o, 0, TO_ROOM);
    return false;
  }

  switch (which) {
    case 1:
      aff2.type = SPELL_FLY;
      aff2.bitvector = AFF_FLYING;
      aff2.duration = 2 * UPDATES_PER_MUDHOUR;
      break;
    case 2:
      aff2.type = SPELL_LEVITATE;
      aff2.bitvector = AFF_LEVITATING;
      aff2.duration = 3 * UPDATES_PER_MUDHOUR;
      break;
  }
  aff2.location = APPLY_NONE;
  aff2.modifier = 0;

  if (!ch->affectJoin(ch, &aff2, AVG_DUR_NO, AVG_EFF_YES)) {
    vlogf(LOG_JESUS, "Failed to join aff2 in castingEQ proc.");
    act("Your $o sparks a bit and and then loses it's glow.<1>", 0, ch, o, 0, TO_CHAR);
  } else {
    act("$n's $o crackles with energy sending sparks into $s left foot.<1>", 0, ch, leg1, 0, TO_ROOM);
    act("Your $o crackles with energy sending sparks into your left foot.<1>", 0, ch, leg1, 0, TO_CHAR);
    act("$n's $o crackles with energy sending sparks into $s right foot.<1>", 0, ch, leg2, 0, TO_ROOM);
    act("Your $o crackles with energy sending sparks into your right foot.<1>", 0, ch, leg2, 0, TO_CHAR);
    act("$n's $o crackles with energy sending sparks back into $s waist.<1>", 0, ch, foot1, 0, TO_ROOM);
    act("Your $o crackles with energy sending sparks back into your waist.<1>", 0, ch, foot1, 0, TO_CHAR);
    act("$n's $o crackles with energy sending sparks back into $s waist.<1>", 0, ch, foot2, 0, TO_ROOM);
    act("Your $o crackles with energy sending sparks back into your waist.<1>", 0, ch, foot2, 0, TO_CHAR);
    if (aff2.type == SPELL_FLY) {
      act("Your $o has given you flight.<1>", 0, ch, o, 0, TO_CHAR);
    } else if (aff2.type == SPELL_LEVITATE) {
      act("Your $o has made you levitate.<1>", 0, ch, o, 0, TO_CHAR);
    }
  }
  return true;
}

// combo 3
const int bodyItem = 36367;
const int backItem = 36375;
const int headItem = 36365;
int doACEQCast(TBeing *ch, TObj *o) {
  affectedData aff2;
  TObj *body = NULL;
  TObj *back = NULL;
  TObj *head = NULL;

  if (!o || !(ch = dynamic_cast<TBeing *>(o->equippedBy)))
    return false;
  // make sure the proc is attached to only one item
  if (o->objVnum() != bodyItem)
    return false;

  int which = ::number(1,2);

  if (ch->affectedBySpell(SPELL_ARMOR) || ch->affectedBySpell(SPELL_SANCTUARY)) {
    act("$n's $o glows but then quickly fades.<1>", 0, ch, o, 0, TO_ROOM);
    act("Your $o glows but then quickly fades.<1>", 0, ch, o, 0, TO_CHAR);
    return false;
  } else {
    act("$n's $o glows with power.<1>", 0, ch, o, 0, TO_ROOM);
    act("Your $o glows with power.<1>", 0, ch, o, 0, TO_CHAR);
  }

  body = dynamic_cast<TObj *>(ch->equipment[WEAR_BODY]);
  back = dynamic_cast<TObj *>(ch->equipment[WEAR_BACK]);
  head = dynamic_cast<TObj *>(ch->equipment[WEAR_HEAD]);

  if ((!body) || (!back) || (!head)) {
    act("$n's $o sparks a bit but then fizzles.<1>", 0, ch, o, 0, TO_ROOM);
    act("Your $o sparks a bit but then fizzles.<1>", 0, ch, o, 0, TO_CHAR);
    return false;
  }
  if (((body->objVnum() != bodyItem) || (back->objVnum() != backItem) ||
      (head->objVnum() != headItem))) {
    act("Your $o sparks a bit and then loses it's glow.<1>", 0, ch, o, 0, TO_CHAR);
    act("$n's $o sparks a bit and then loses it's glow.<1>", 0, ch, o, 0, TO_ROOM);
    return false;
  }

  switch (which) {
    case 1:
      aff2.type = SPELL_ARMOR;
      aff2.level = 35;
      aff2.bitvector = 0;
      aff2.duration = 3 * UPDATES_PER_MUDHOUR;
      aff2.location = APPLY_ARMOR;
      aff2.modifier = -75; // deikhan armor, not cleric
      break;
    case 2:
      aff2.type = SPELL_SANCTUARY;
      aff2.level = 35;
      aff2.bitvector = AFF_SANCTUARY;
      aff2.duration = 3 * UPDATES_PER_MUDHOUR;
      aff2.location = APPLY_PROTECTION;
      aff2.modifier = 50;
      break;
  }

  if (!ch->affectJoin(ch, &aff2, AVG_DUR_NO, AVG_EFF_YES)) {
    vlogf(LOG_JESUS, "Failed to join aff2 in castingEQ proc.");
    act("Your $o sparks a bit and and then loses it's glow.<1>", 0, ch, o, 0, TO_CHAR);
  } else {
    act("$n's $o crackles with energy sending sparks into $s back.<1>", 0, ch, body, 0, TO_ROOM);
    act("Your $o crackles with energy sending sparks into your back.<1>", 0, ch, body, 0, TO_CHAR);
    act("$n's $o crackles with energy sending sparks into $s head.<1>", 0, ch, back, 0, TO_ROOM);
    act("Your $o crackles with energy sending sparks into your head.<1>", 0, ch, back, 0, TO_CHAR);
    act("$n's $o crackles with energy sending sparks back into $s body.<1>", 0, ch, head, 0, TO_ROOM);
    act("Your $o crackles with energy sending sparks back into your body.<1>", 0, ch, head, 0, TO_CHAR);
    if (aff2.type == SPELL_ARMOR) {
      act("Your $o has given you armor.<1>", 0, ch, o, 0, TO_CHAR);
    } else if (aff2.type == SPELL_SANCTUARY) {
      act("Your $o has given you sanctuary.<1>", 0, ch, o, 0, TO_CHAR);
    }
  }
  return true;
}


int comboEQCast(TBeing *vict, cmdTypeT cmd, const char *arg, TObj *o, TObj *) {
  TBeing *ch;
  int chance = 0;

  if (!o)
    return false;
  if (!(ch = dynamic_cast<TBeing *>(o->equippedBy)))
    return false;
  if (::number(0,chance))
    return false;

  if (cmd == CMD_GENERIC_PULSE) {
    // not too often...
    if (!::number(0,39)) {
      int which = ::number(1,3);
      switch (which) {
	case 1:
	  if (!ch->affectedBySpell(SPELL_FERAL_WRATH)) {
	    doFeralEQCast(ch, o);
	  }
	  break;
	case 2:
	  if (!ch->affectedBySpell(SPELL_LEVITATE) && !ch->affectedBySpell(SPELL_FLY)) {
	    doFlyingEQCast(ch, o);
	  }	  
	  break;
	case 3:
	  if (!ch->affectedBySpell(SPELL_ARMOR) && !ch->affectedBySpell(SPELL_SANCTUARY)) {
	    doACEQCast(ch, o);
	  }
	  break;
      }
      return true;
    }
    return true;
  }  
  return false;
}

