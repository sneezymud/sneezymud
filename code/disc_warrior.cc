//////////////////////////////////////////////////////////////////////////
//
// SneezyMUD - All rights reserved, SneezyMUD Coding Team
//
//////////////////////////////////////////////////////////////////////////


#include "stdsneezy.h"
#include "disease.h"
#include "combat.h"
#include "disc_warrior.h"
#include "obj_tool.h"

int TBeing::doBerserk() 
{
  int rc;

  if (!doesKnowSkill(SKILL_BERSERK)) {
    sendTo("You lack the bloodlust.\n\r");
    return FALSE;
  }
  if (checkBusy()) {
    return FALSE;
  }
  if (affectedBySpell(SKILL_BERSERK)) {
    sendTo("You are unable to work up the bloodlust at this time.\n\r");
    return FALSE;
  }
  if (affectedBySpell(SKILL_DISGUISE)){
    sendTo("You can't work up the bloodlust while pretending to be someone else.\n\r");
    return FALSE;
  }

  rc = berserk(this);
  if (IS_SET_DELETE(rc, DELETE_THIS))
    return DELETE_THIS;
  return FALSE;
}

int berserk(TBeing * caster)
{
  int level;
  affectedData af;

  if (caster->riding) {
    act("Not while riding.",TRUE,caster,0,0,TO_CHAR);
    return FALSE;
  }
  if (caster->getCombatMode() == ATTACK_BERSERK) {
    act("You are already berserking!",TRUE,caster,0,0,TO_CHAR);
    return FALSE;
  }

  if (caster->checkPeaceful("This room is too tranquil to go berserk in.\n\r"))
    return FALSE;    

  if (!caster->isPc())
    return FALSE;

  level = caster->getSkillLevel(SKILL_BERSERK);
  int bKnown = caster->getSkillValue(SKILL_BERSERK);
  if (caster->bSuccess(bKnown, SKILL_BERSERK)) {
    caster->setCombatMode(ATTACK_BERSERK);
    act("You go berserk!",TRUE,caster,0,0,TO_CHAR);
    act("$n goes berserk!", TRUE, caster,0,0,TO_ROOM);

    if(caster->getHit() > (caster->hitLimit()/2)){
      af.type = SKILL_BERSERK;
      af.modifier = ::number(caster->getSkillValue(SKILL_BERSERK),
			     caster->getSkillValue(SKILL_BERSERK)*2);
      af.level = level;
      //      af.duration = caster->getSkillValue(SKILL_BERSERK);;
      af.duration = PERMANENT_DURATION;
      af.location = APPLY_HIT;
      af.bitvector = 0;
      caster->affectTo(&af, -1);

      af.location = APPLY_CURRENT_HIT;
      caster->affectTo(&af, -1);

      caster->sendTo("Berserking increases your ability to withstand damage!\n\r");
    }

    if (!caster->fight())
      caster->goBerserk(NULL);
  } else {
    act("You try to go berserk and bite yourself in the tongue!", TRUE, caster, 0, 0, TO_CHAR);
    act("$n bites $mself in the tongue while trying to go berserk!", TRUE, caster, 0, 0, TO_ROOM);
    if (caster->reconcileDamage(caster, 1, SKILL_BERSERK) == -1)
      return DELETE_THIS;

    af.type = SKILL_BERSERK;
    af.level = level;
    af.duration = 6 * UPDATES_PER_MUDHOUR;
    af.location = APPLY_NONE;
    af.modifier = 0;
    af.bitvector = 0;
    caster->affectTo(&af, -1);
  }

  return TRUE;
}

void TBeing::doRepair(const char *arg)
{
  char v_name[MAX_INPUT_LENGTH];
  TThing *obj;

  strcpy(v_name, arg);

  if (!*v_name) {
    sendTo("What is it you intend to repair?\n\r");
    return;
  }

  for(obj=getStuff();obj;obj=obj->nextThing){
    if(isname(v_name, obj->name))
      break;
  }
  if(!obj){
    sendTo("You'll have to have that item in your inventory to repair it.\n\r");
    return;
  }

#if 0
  if (!(obj = heldInSecHand()) || !isname(v_name, obj->name)) {
    sendTo("You'll have to be holding that in your secondary hand to repair it.\n\r");
    return;
  }
#endif

  TObj *item = dynamic_cast<TObj *>(obj);
  if (!item) {
    sendTo("You can only repair objects.\n\r");
    return;
  }

  if (material_nums[min(max((int) obj->getMaterial(), 0), 200)].repair_proc)
    (*(material_nums[min(max((int) obj->getMaterial(), 0), 200)].repair_proc)) (this, item);
  else
    sendTo("You have no idea how to repair something like that.\n\r");


  //  repair(this,item);
}

void TThing::repairMeHammer(TBeing *caster, TObj *obj)
{
  act("You need to hold a hammer in your primary hand in order to repair $p",
           TRUE, caster, obj, NULL, TO_CHAR);
  return;
}

void TTool::repairMeHammer(TBeing *caster, TObj *obj)
{
  if (getToolType() != TOOL_HAMMER) {
   act("You need to hold a hammer in your primary hand in order to repair $p",
           TRUE, caster, obj, NULL, TO_CHAR);
    return;
  }

  if (obj->getMaxStructPoints() <= obj->getStructPoints()) {
    caster->sendTo("But it looks as good as good as its going to get!\n\r");
    return;
  }
  if (caster->getMove() < 10) {
    caster->sendTo("You are much too tired to repair things right now.  Take a nap or something.\n\r");
    return;
  }
  if (material_nums[min(max((int) obj->getMaterial(), 0), 200)].repair_proc)
    (*(material_nums[min(max((int) obj->getMaterial(), 0), 200)].repair_proc)) (caster, obj);
  else
    caster->sendTo("You have no idea how to repair something like that.\n\r");
}

void repair(TBeing * caster, TObj *obj)
{
  TThing *hammer;

  if (!(hammer = caster->heldInPrimHand())) {
    act("You need to hold a hammer in your primary hand in order to repair $p",
           TRUE, caster, obj, NULL, TO_CHAR);
    return;
  }
  hammer->repairMeHammer(caster, obj);
}








