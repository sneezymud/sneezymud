#include "stdsneezy.h"
#include "disease.h"
#include "combat.h"
#include "spelltask.h"
#include "disc_alchemy.h"
#include "obj_component.h"



void TBeing::doBrew(const char *arg)
{
  char buf[256];
  TComponent *comp_gen, *comp_spell, *comp_brew;
  TThing *t;
  spellNumT which = TYPE_UNDEFINED;
  int i;

  for (;arg && *arg && isspace(*arg); arg++);

  if (!*arg || !arg) {
    sendTo("You need to specify a potion type to brew!\n\r");
    return;
  }
  if (((which = searchForSpellNum(arg, EXACT_YES)) < MIN_SPELL) &&
      ((which = searchForSpellNum(arg, EXACT_NO)) < MIN_SPELL)) {
    sendTo("You can't mix a potion of that type.\n\r");
    return;
  }

  // find the 3 necessary pieces
    // generic component (spell == -1, type = brew)
    comp_gen = NULL;
    // spell comp (spell = which, type = spell)
    comp_spell = NULL;
    // brew comp (spell = which, type = brew)
    comp_brew = NULL;

  for (i = MIN_WEAR; i < MAX_WEAR; i++) {
    if ((t = equipment[i])) {
      t->findSomeComponent(&comp_gen, &comp_spell, &comp_brew, which, 1);
    }
  }
  for (t = getStuff(); t; t = t->nextThing) {
    t->findSomeComponent(&comp_gen, &comp_spell, &comp_brew, which, 1);
  }

  if (!comp_gen) {
    sendTo("You seem to be lacking a flask.\n\r");
    return;
  }
  if (!comp_spell) {
    sendTo("You seem to be lacking the spell component.\n\r");
    return;
  }
  if (!comp_brew) {
    sendTo("You seem to be lacking the brew component.\n\r");
    return;
  }
  if (!doesKnowSkill(SKILL_BREW)) {
    sendTo("You lack any knowledge of how to brew potions.\n\r");
    return;
  }
  if (checkBusy(NULL)) {
    return;
  }
  if (isSwimming()) {
    sendTo("You can't brew while swimming.\n\r");
    return;
  }
  if (riding) {
    sendTo("You can't brew while riding.\n\r");
    return;
  }

  // trash all items first
  int how_many = comp_brew->getComponentCharges();

  sprintf(buf, "You begin to brew %d potions of %s.", 
         how_many, discArray[which]->name);
  act(buf, FALSE, this, 0, 0, TO_CHAR);
  sprintf(buf, "$n begins to brew a potion.");
  act(buf, FALSE, this, 0, 0, TO_ROOM);

  delete comp_gen;
  comp_gen = NULL;

  sprintf(buf, "$p is consumed in the process.");
  act(buf, FALSE, this, comp_brew, 0, TO_CHAR);
  delete comp_brew;
  comp_brew = NULL;
  
  sprintf(buf, "You use up one charge of $p.");
  act(buf, FALSE, this, comp_spell, 0, TO_CHAR);
  comp_spell->addToComponentCharges(-1);
  if (comp_spell->getComponentCharges() <= 0) {
    sprintf(buf, "$p is consumed in the process.");
    act(buf, FALSE, this, comp_spell, 0, TO_CHAR);
    delete comp_spell;
    comp_spell = NULL;
  }

  start_task(this, NULL, NULL, TASK_BREWING, "", 0, in_room, how_many, which, 0);

  return;
}













