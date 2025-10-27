#include "being.h"
#include "disc_warrior_blacksmithing.h"
#include "obj_tool.h"
#include "materials.h"
#include "obj.h"

void TBeing::doRepair(const char* arg) {
  char v_name[MAX_INPUT_LENGTH];
  TThing* obj = NULL;

  strcpy(v_name, arg);

  if (!*v_name) {
    sendTo("What is it you intend to repair?\n\r");
    return;
  }

  for (StuffIter it = stuff.begin(); it != stuff.end(); ++it) {
    obj = *it;
    if (isname(v_name, obj->name))
      break;
  }
  if (!obj) {
    sendTo("You'll have to have that item in your inventory to repair it.\n\r");
    return;
  }

#if 0
  if (!(obj = heldInSecHand()) || !isname(v_name, obj->name)) {
    sendTo("You'll have to be holding that in your secondary hand to repair it.\n\r");
    return;
  }
#endif

  TObj* item = dynamic_cast<TObj*>(obj);
  if (!item) {
    sendTo("You can only repair objects.\n\r");
    return;
  }

  if (material_nums[min(max((int)obj->getMaterial(), 0), 200)].repair_proc)
    (*(material_nums[min(max((int)obj->getMaterial(), 0), 200)].repair_proc))(
      this, item);
  else
    sendTo("You have no idea how to repair something like that.\n\r");

  //  repair(this,item);
}

void TThing::repairMeHammer(TBeing* caster, TObj* obj) {
  act("You need to hold a hammer in your primary hand in order to repair $p",
    TRUE, caster, obj, NULL, TO_CHAR);
}

void TTool::repairMeHammer(TBeing* caster, TObj* obj) {
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
    caster->sendTo(
      "You are much too tired to repair things right now.  Take a nap or "
      "something.\n\r");
    return;
  }
  if (material_nums[min(max((int)obj->getMaterial(), 0), 200)].repair_proc)
    (*(material_nums[min(max((int)obj->getMaterial(), 0), 200)].repair_proc))(
      caster, obj);
  else
    caster->sendTo("You have no idea how to repair something like that.\n\r");
}

void repair(TBeing* caster, TObj* obj) {
  TThing* hammer;

  if (!(hammer = caster->heldInPrimHand())) {
    act("You need to hold a hammer in your primary hand in order to repair $p",
      TRUE, caster, obj, NULL, TO_CHAR);
    return;
  }
  hammer->repairMeHammer(caster, obj);
}

void TBeing::doDebride(const char* arg) {
  char v_name[MAX_INPUT_LENGTH];
  TThing* obj = NULL;

  strncpy(v_name, arg, MAX_INPUT_LENGTH - 1);
  v_name[MAX_INPUT_LENGTH - 1] = '\0';

  if (!*v_name) {
    act("What is it you intend to debride?", FALSE, this, 0, 0, TO_CHAR);
    return;
  }

  for (StuffIter it = stuff.begin(); it != stuff.end(); ++it) {
    obj = *it;
    if (isname(v_name, obj->name))
      break;
  }
  if (!obj) {
    act("You'll have to have that item in your inventory to debride it.", FALSE, this, 0, 0, TO_CHAR);
    return;
  }

  TObj* item = dynamic_cast<TObj*>(obj);
  if (!item) {
    act("You can't debride that...", FALSE, this, item, 0, TO_CHAR);
    return;
  }

  if (!item->isObjStat(ITEM_RUSTY)) {
    act("You don't need to debride that. It's not even rusty.", FALSE, this, item, 0, TO_CHAR);
    return;
  }

  if (item->getMaterial() != (MAT_STEEL) && item->getMaterial() != (MAT_IRON)) {
      act("You have no idea how that object even got rusty in the first place.", FALSE, this, item, 0, TO_CHAR);
      return;
  }

  TTool* tool = getToolSlot(getPrimaryHold(), TOOL_FILE);
  if (!tool) {
    act("You need to hold a file in your primary hand in order to debride $p.", FALSE, this, item, 0, TO_CHAR);
    return;
  }

  if (getMove() < 10) {
    act("You are much too tired to debride things right now.  Take a nap or something.", FALSE, this, item, 0, TO_CHAR);
    return;
  }

  if (getPosition() < POSITION_CRAWLING) {
    act("You need to be crawling or better to debride something.", FALSE, this, item, 0, TO_CHAR);
    return;
  }

  learnFromDoingUnusual(LEARN_UNUSUAL_NORM_LEARN, SKILL_BLACKSMITHING, 4);
  act("You start to debride $p with $P.", FALSE, this, item, tool, TO_CHAR);
  act("$n begins debriding $p with $P.", FALSE, this, item, tool, TO_ROOM);

  // Calculate task duration based on item volume (each 100 volume = 1 unit of time)
  int taskDuration = max(1, item->getVolume() / 100);
  start_task(this, NULL, NULL, TASK_DEBRIDE, item->name.c_str(), taskDuration,
    (ushort)in_room, 0, 0, 0);
}


