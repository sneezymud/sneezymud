#include "stdsneezy.h"
#include "disease.h"
#include "combat.h"
#include "spelltask.h"
#include "disc_alchemy.h"
#include "obj_component.h"
#include "obj_potion.h"

void TBeing::doBrew(const char *arg)
{
  char buf[256];
  TComponent *invalid, *comp_spell, *comp_brew;
  TPotion *comp_gen;
  TThing *t;
  spellNumT which_spell = TYPE_UNDEFINED;
  liqTypeT which_liq = LIQ_WATER;
  int i;


  for (;arg && *arg && isspace(*arg); arg++);

  if (!*arg || !arg) {
    sendTo("You need to specify a potion type to brew!\n\r");
    return;
  }
  if (((which_spell = searchForSpellNum(arg, EXACT_YES)) < MIN_SPELL) &&
      ((which_spell = searchForSpellNum(arg, EXACT_NO)) < MIN_SPELL) &&
      ((which_liq = spell_to_liq(which_spell)) != LIQ_WATER)) {
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
    invalid=NULL;

  for (i = MIN_WEAR; i < MAX_WEAR; i++) {
    if ((t = equipment[i])) {
      t->findSomeComponent(&invalid, &comp_spell, &comp_brew, which_spell, 1);

      if((comp_gen=dynamic_cast<TPotion *>(t)) && 
	 comp_gen->getDrinkType() != LIQ_MAGICAL_ELIXIR){
	comp_gen=NULL;
      }
    }
  }
  for (t = getStuff(); t; t = t->nextThing) {
    t->findSomeComponent(&invalid, &comp_spell, &comp_brew, which_spell, 1);
    
    if((comp_gen=dynamic_cast<TPotion *>(t)) && 
       comp_gen->getDrinkType() != LIQ_MAGICAL_ELIXIR){
      comp_gen=NULL;
    }
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
  int how_many = comp_gen->getDrinkUnits();

  sprintf(buf, "You begin to brew %d ounces of %s.", 
         how_many, discArray[which_spell]->name);
  act(buf, FALSE, this, 0, 0, TO_CHAR);
  sprintf(buf, "$n begins to brew a potion.");
  act(buf, FALSE, this, 0, 0, TO_ROOM);

  comp_gen->setDrinkUnits(0);

  comp_brew->addToComponentCharges(-1);
  if(comp_brew->getComponentCharges() <= 0) {
    sprintf(buf, "$p is consumed in the process.");
    act(buf, FALSE, this, comp_brew, 0, TO_CHAR);
    delete comp_brew;
    comp_brew = NULL;
  }

  sprintf(buf, "You use up one charge of $p.");
  act(buf, FALSE, this, comp_spell, 0, TO_CHAR);
  comp_spell->addToComponentCharges(-1);
  if (comp_spell->getComponentCharges() <= 0) {
    sprintf(buf, "$p is consumed in the process.");
    act(buf, FALSE, this, comp_spell, 0, TO_CHAR);
    delete comp_spell;
    comp_spell = NULL;
  }

  start_task(this, NULL, NULL, TASK_BREWING, "", 0, in_room, how_many, which_spell, 0);

  return;
}













