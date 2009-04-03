//////////////////////////////////////////////////////////////////////////
//
// SneezyMUD - All rights reserved, SneezyMUD Coding Team
//
//////////////////////////////////////////////////////////////////////////


#include "stdsneezy.h"
#include "disease.h"
#include "combat.h"
#include "obj_component.h"
#include "disc_combat.h"
#include "obj_tool.h"

void TBeing::doSharpen(const char *argument)
{
  char name_buf[256];
  TThing *obj;

  strcpy(name_buf, argument);

  if (!doesKnowSkill(SKILL_SHARPEN)) {
    sendTo("You know nothing about sharpening weapons.\n\r");
    return;
  }

  if (!*name_buf) {
    if (!(obj = equipment[getPrimaryHold()])) {
      sendTo("What is it you intend to sharpen?\n\r");
      return;
    }
  } else {
    if (!(obj = equipment[getPrimaryHold()]) || !isname(name_buf, obj->name)) {
      sendTo("You have to be holding that in your primary hand to sharpen it.\n\r");
      return;
    }
  }

  sharpen(this, obj);
}

void TThing::sharpenMeStone(TBeing *caster, TThing *)
{
  caster->sendTo("You need to own a whetstone.\n\r");
}

void TTool::sharpenMeStone(TBeing *caster, TThing *obj)
{
  if (getToolType() != TOOL_WHETSTONE) {
    caster->sendTo("You need to own a whetstone.\n\r");
    return;
  }
  obj->sharpenMeStoneWeap(caster, this);
}

void TThing::sharpenMeStoneWeap(TBeing *caster, TTool *)
{
  caster->sendTo("Sorry.  You only know how to sharpen weapons.\n\r");
  return;
}

void sharpen(TBeing * caster, TThing * obj)
{
  TThing *stone;

  if (caster->fight()) {
    caster->sendTo("Not while fighting..\n\r");
    return;
  }

  if (!(stone = get_thing_char_using(caster, "whetstone", 0, FALSE, FALSE))) {
    caster->sendTo("You need to own a whetstone.\n\r");
    return;
  }
  stone->sharpenMeStone(caster, obj);
}

void TBeing::doDull(const char *argument)
{
  char name_buf[256];
  TThing *obj;
 
  strcpy(name_buf, argument);
 
  if (!doesKnowSkill(SKILL_DULL)) {
    sendTo("You know nothing about dulling weapons.\n\r");
    return;
  }

  if (!*name_buf) {
    if (!(obj = equipment[getPrimaryHold()])) {
      sendTo("What is it you intend to dull?\n\r");
      return;
    }
  } else {
    if (!(obj = equipment[getPrimaryHold()]) || !isname(name_buf, obj->name)) {
      sendTo("You have to be holding that in your primary hand to dull it.\n\r");
      return;
    }
  }
 
  dull(this, obj);
}
 
void TThing::dullMeFile(TBeing *caster, TThing *)
{
  caster->sendTo("You need to own a file.\n\r");
}

void TTool::dullMeFile(TBeing *caster, TThing *obj)
{
  if (getToolType() != TOOL_FILE) {
    caster->sendTo("You need to own a file.\n\r");
    return;
  }
  obj->dullMeFileWeap(caster, this);
}

void TThing::dullMeFileWeap(TBeing *caster, TTool *)
{
  caster->sendTo("Sorry.  You only know how to dull weapons.\n\r");
  return;
}
 
void dull(TBeing * caster, TThing * obj)
{
  TThing *file;
 
  if (!(file = get_thing_char_using(caster, "file", 0, FALSE, FALSE))) {
    caster->sendTo("You need to own a file.\n\r");
    return;
  }
  file->dullMeFile(caster, obj);
}

CDCombat::CDCombat()
  : CDiscipline(),
    skBarehand(),
    skArmorUse(),
    skSlash(),
    skBow(),
    skPierce(),
    skBlunt(),
    skSharpen(),
    skDull()
{
}

CDCombat::CDCombat(const CDCombat &a)
  : CDiscipline(a),
    skBarehand(a.skBarehand),
    skArmorUse(a.skArmorUse),
    skSlash(a.skSlash),
    skBow(a.skBow),
    skPierce(a.skPierce),
    skBlunt(a.skBlunt),
    skSharpen(a.skSharpen),
    skDull(a.skDull)
{
}

CDCombat & CDCombat::operator=(const CDCombat &a)
{
  if (this == &a) return *this;
  CDiscipline::operator=(a);
  skBarehand = a.skBarehand;
  skArmorUse = a.skArmorUse;
  skSlash = a.skSlash;
  skBow = a.skBow;
  skPierce = a.skPierce;
  skBlunt = a.skBlunt;
  skSharpen = a.skSharpen;
  skDull = a.skDull;
  return *this;
}

CDCombat::~CDCombat()
{
}
