/* *******************************************************************
    disc_adventuring.cc : all procedures related to the adventuring discipline

    Copyright 1999 SneezyMUD development team
    All rights reserved.
******************************************************************** */

#include "stdsneezy.h"
#include "disease.h"
#include "combat.h"
#include "disc_adventuring.h"

void TThing::findBandage(int *)
{
}

int findBandages(TThing *t) 
{
  int count = 0;

  for (; t; t = t->nextThing) {
    t->findBandage(&count);
    if (t->getStuff())
      count += findBandages(t->getStuff());
  }
  return(count);
}

int destroy_bandages(TThing *t, int band)
{
  int count = 0;
  TThing *t2;

  for (; t && (count < band); t = t2) {
    t2 = t->nextThing;
    if (t->getStuff()) {
      count += destroy_bandages(t->getStuff(), band-count);
    }
    if (count >= band)
      break;
    t->destroyBandage(&count);
  }
  return(count);
}

void TBeing::bandage(TBeing *victim,wearSlotT slot)
{
  TObj *bandaid;
  char buf[256], limb[256];
  int r_num;

  if (!victim) {
    vlogf(LOG_BUG, "Null critters passed into bandage_victim");
    return;
  }
  if ((r_num = real_object(OBJ_BANDAGE)) >= 0) {
    bandaid = read_object(r_num, REAL);
  } else {
    vlogf(LOG_BUG, "bogus bandaid!");
    return;
  }
#if 0
  if (!(bandaid = read_object(OBJ_BANDAGE, VIRTUAL))) {
    vlogf(LOG_BUG, "bogus bandaid!");
    return;
  }
#endif
  // we know the slot the bandaid is for is clear from doBandage 
  if (!victim->validEquipSlot(slot)) {
    sendTo("They don't seem to have that body location.\n\r");
    return;
  } 
  victim->equipChar(bandaid, slot);
  victim->addToLimbFlags(slot, PART_BANDAGED);
  if (this != victim) {
    act("You bind $N's wounds and put a $o on $M.",TRUE,this,bandaid,victim,TO_CHAR);
    act("$n binds your wounds and puts a $o on you.",TRUE,this,bandaid,victim,TO_VICT);
    act("$n binds $N's wounds and puts a $o on $M.",TRUE,this,bandaid,victim,TO_NOTVICT);
  } else {
    act("You bind your wounds with a $o.",TRUE,this,bandaid,0,TO_CHAR);
    act("$n binds $s wounds with a $o.",TRUE,this,bandaid,0,TO_ROOM);
  } 
  if (victim->isLimbFlags(slot, PART_BLEEDING)) {
    sprintf(limb, "%s", victim->describeBodySlot(slot).c_str());
    victim->remLimbFlags(slot, PART_BLEEDING);
    sprintf(buf, "The gash on your %s slowly stops bleeding and the flesh closes up!", limb);
    act(buf, FALSE, victim, NULL, NULL, TO_CHAR);
    sprintf(buf, "The gash on $n's %s slowly stops bleeding and the flesh closes up!", limb);
    act(buf, FALSE, victim, NULL, NULL, TO_ROOM);
    reconcileHelp(victim, discArray[SKILL_BANDAGE]->alignMod);
  }
}

void TBeing::doBandage(const string &arg)
{
  TBeing *vict;
  wearSlotT slot;
  int band_num, count;
  string buf, buf2;

  argument_parser(arg, buf, buf2);

  if (!hasHands() || affectedBySpell(AFFECT_TRANSFORMED_ARMS) ||
                     affectedBySpell(AFFECT_TRANSFORMED_HANDS)) {
    sendTo("You need a good hand to bandange!\n\r");
    return;
  }

  if (!doesKnowSkill(SKILL_BANDAGE)) {
    sendTo("You aren't skilled enough to bandage someone!\n\r");
    return;
  }
  // Auto-Target the bandager.
  if(buf.empty()){
    vict = this;
  } else if (!(vict = get_char_room_vis(this, buf.c_str()))) {
    sendTo("Syntax : Bandage <person> <location>\n\r");
    return;
  }

  if (!vict->isPc()){
    sendTo("Bandage non-players has been temporarily disabled.\n\r");
    return;
  }

  // Auto-Find bleeding limbs.
  int tmpi;
  if(buf2.empty()){
    for (tmpi = 0; (tmpi < MAX_WEAR) &&
                   !vict->isLimbFlags(wearSlotT(tmpi), PART_BLEEDING); tmpi++);

    if (tmpi > MAX_WEAR) {
      sendTo("No bleeding body part found.\n\r");
      sendTo("Syntax : Bandage <person> <location>\n\r");
      return;
    }
    slot = wearSlotT(tmpi);
  } else {
    int tmpi;
    if ((tmpi = old_search_block(buf2.c_str(), 0, buf2.length(), bodyParts, 0)) <= 0) {
      sendTo("You must pick a correct body part to bandage!\n\r");
      return;
    }
    slot = wearSlotT(tmpi-1);
  }

  if (vict->equipment[slot]) {
    sendTo("It is impossible to bandage a body part that is equipped!\n\r");
    return;
  }
  if (vict->isLimbFlags(slot, PART_BANDAGED)) {
    ssprintf(buf, "$N's %s is already bandaged!", vict->describeBodySlot(slot).c_str());
    act(buf.c_str(), FALSE, this, NULL, vict, TO_CHAR);
    return;
  }
  switch (slot) {
    case WEAR_FINGER_L:
    case WEAR_FINGER_R:
    case WEAR_WRIST_L:
    case WEAR_WRIST_R:
      band_num = 1;
      break;
    case WEAR_ARM_L:
    case WEAR_ARM_R:
    case WEAR_FOOT_L:
    case WEAR_FOOT_R:
    case WEAR_EX_FOOT_L:
    case WEAR_EX_FOOT_R:
    case WEAR_NECK:
    case WEAR_HAND_R:
    case WEAR_HAND_L:
      band_num = 2;
      break;
    case WEAR_LEGS_L:
    case WEAR_LEGS_R:
    case WEAR_EX_LEG_L:
    case WEAR_EX_LEG_R:
      band_num = 4;
    case WEAR_HEAD:
    case WEAR_WAISTE:
      band_num = 5;
      break;
    case WEAR_BODY:
    case WEAR_BACK:
      band_num = 6;
      break;
    default:
      sendTo("Syntax : Bandage <player> <body part>\n\r");
      return;
  }
  
  // adjust for race
  band_num = max(1,(int) (band_num * ((double) getHeight()/(double) 70)));

  count = findBandages(getStuff());
  if (count >= band_num) {
    if (band_num != destroy_bandages(getStuff(), band_num)) {
      vlogf(LOG_BUG, "error in destroy bandage routine!");
      sendTo("code error - tell a god\n\r");
      return;
    } else {
      if (bSuccess(this,  getSkillValue(SKILL_BANDAGE), SKILL_BANDAGE)) {
        if (band_num > 1) {
          ssprintf(buf,"You quickly combine %d bandages into one big enough to bandage that part.\n\r",band_num);
          sendTo(buf.c_str());
        }
        bandage(vict,slot);
      } else {
        sendTo("Your lack of skill causes you to screw up the bandage.\n\r");
        sendTo("The bandage must be thrown out as it is worthless now.\n\r");
      }
      addSkillLag(SKILL_BANDAGE, 0);
    }
  } else
    sendTo("You need %d total bandages to cover that area.  You have %d.\n\r", band_num, count);
}
