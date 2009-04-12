//////////////////////////////////////////////////////////////////////////
//
// SneezyMUD - All rights reserved, SneezyMUD Coding Team
//
//////////////////////////////////////////////////////////////////////////

#include "extern.h"
#include "being.h"
#include "disc_theology.h"
#include "obj_vial.h"
#include "obj_symbol.h"

void TBeing::doAttune(const char *argument)
{
  char name_buf[256];
  TThing *obj;

  strcpy(name_buf, argument);

  if (!*name_buf) {
    sendTo("What is it you intend to attune?\n\r");
    return;
  }

  if (!doesKnowSkill(SKILL_ATTUNE)) {
    sendTo("You know nothing about attuning symbols.\n\r");
    return;
  }
  if (!(obj = equipment[getPrimaryHold()]) || !isname(name_buf, obj->name)) {
    sendTo("You'll have to be holding that in your primary hand to attune it.\n\r");
    return;
  }

  if (getPosition() > POSITION_SITTING) {
    sendTo("You can not attune symbols unless you are sitting comfortably.\n\r");
    return;
  }
  attune(this, obj);
}

void TThing::attuneMe(TBeing *caster, TVial *)
{
  caster->sendTo("You can only attune symbols.\n\r");
}

void TSymbol::attuneMe(TBeing *caster, TVial *obj)
{
  if (getSymbolFaction() != FACT_UNDEFINED) {
    caster->sendTo(COLOR_OBJECTS, format("But %s has already been attuned!\n\r") % getName());
    return;
  }

  int reqUses = 0;
  int uses = 0;
 
  if (!checkAttuneUsage(caster, &uses, &reqUses, &obj, this))
    return;

  if (caster->getMove() < 10) {
    act("You are much too tired to attune $p.", FALSE, caster, this, NULL, TO_CHAR);
    return;
  }

  act("You place $p carefully on your lap.",FALSE, caster, this, NULL, TO_CHAR);
  act("$n places $p carefully on $s lap.",FALSE, caster, this, NULL, TO_ROOM);
  act("You say a small prayer as you sprinkle some drops from $P on $p and begin attuning it.", FALSE, caster, this, obj, TO_CHAR);
  act("$n sprinkles some drops from $P on $p and begins to pray.", FALSE, caster, this, obj, TO_ROOM);
  // terminate sitting tasks
  if ((caster->task) && caster->getPosition() <= POSITION_SITTING)
    caster->stopTask();

  ubyte rounds =  (ubyte) (getSymbolLevel() / 2 + 1);
  start_task(caster, (TObj *) this, NULL, TASK_ATTUNE, NULL, 0, (ushort) caster->in_room, rounds, 0, 0);
}


void attune(TBeing * caster, TThing * sym) 
{
  TVial *best = NULL;
  TThing *obj = NULL;

  if (caster->fight()) {
    caster->sendTo("Not while fighting..\n\r");
    return;
  }
  
  for(StuffIter it=caster->stuff.begin();it!=caster->stuff.end() && (obj=*it);++it) 
    obj->getBestVial(&best);
  
  if (!best) {
    caster->sendTo("You need more holy water to attempt this attune.\n\r");
    //caster->sendTo("You need to own some holy water.\n\r");
    return;
  }
  sym->attuneMe(caster, best);
}


