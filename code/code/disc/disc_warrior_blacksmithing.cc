#include "being.h"
#include "disc_warrior_blacksmithing.h"
#include "obj_tool.h"
#include "materials.h"

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
