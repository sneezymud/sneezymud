//////////////////////////////////////////////////////////////////////////
//
// SneezyMUD - All rights reserved, SneezyMUD Coding Team
//
//////////////////////////////////////////////////////////////////////////


// thing.h

#include "stdsneezy.h"
#include "obj_base_container.h"
#include "obj_open_container.h"
#include "obj_component.h"
#include "obj_tool.h"

const char * TThing::objs(const TThing *t) const
{
  return (canSee(t) ? t->getName() : "something");
}

const sstring TThing::objn(const TThing *t) const
{
  return (canSee(t) ? fname(t->name) : "something");
}

const char * TThing::ana() const
{
  return (strchr("aeiouyAEIOUY", *name) ? "An" : "A");
}

const char * TThing::sana() const
{
  return (strchr("aeiouyAEIOUY", *name) ? "an" : "a");
}

const char * TThing::pers(const TThing *t) const
{
  return (canSee(t) ? t->getName() : "someone");
}

const sstring TThing::persfname(const TThing *t) const
{
  return (canSee(t) ? fname(t->name) : "someone");
}

bool TThing::sameRoom(const TThing &ch) const
{
  return (inRoom() == ch.inRoom());
}

bool TThing::inImperia() const
{
  return (inRoom() >= 0 && inRoom() < 100);
}

bool TThing::inGrimhaven() const
{
  return ((inRoom() >= 100 && inRoom() < 950) ||
          (inRoom() >= 25400 && inRoom() <= 25499) ||
	  (inRoom() >= 4400 && inRoom() <= 4699));
}

bool TThing::inAmber() const
{
  return ((inRoom() >= 2850 && inRoom() <= 3014) ||
	  (inRoom() >= 8700 && inRoom() <= 8899) ||
	  (inRoom() >= 16200 && inRoom() <= 16249) ||
	  (inRoom() >= 27800 && inRoom() <= 27899));
}

bool TThing::inLogrus() const
{
  return ((inRoom() >= 3700 && inRoom() <= 3899) ||
	  (inRoom() >= 26650 && inRoom() <= 26699));
}

bool TThing::inBrightmoon() const
{ 
  return ((inRoom() >= 1200 && inRoom() <= 1399) ||
	  (inRoom() >= 16450 && inRoom() <= 16499));
}

bool TThing::inLethargica() const
{
  return (inRoom() >= 23400 && inRoom() <= 23599);
}

bool TThing::isSpiked() const
{
  if (isname("spiked", name))
    return TRUE;
  return FALSE;
}

int TThing::swungObjectDamage(const TBeing *, const TBeing *) const
{
  int dam = 0;

  dam = (int) getWeight() / 5;
  dam = min(15, dam);
  return dam;
}

int TThing::useMe(TBeing *ch, const char *)
{
  ch->sendTo("Use is normally only for wands and magic staves.\n\r");
  return FALSE;
}

// Weight of all things that I am carrying, or that I contain
// or things that are riding me
float TThing::getCarriedWeight() const
{
  TThing *t;
  float total=0;

  for(t=rider;t;t=t->nextRider){
    total+=t->getTotalWeight(true);
  }

  for(t=getStuff();t;t=t->nextThing){
    if(dynamic_cast<TComponent *>(t))
      total+=(t->getTotalWeight(true)*0.10);
    else 
      total+=t->getTotalWeight(true);
  }

  return total;
}

int TThing::getCarriedVolume() const
{
  TThing *t;
  int total=0;

  for(t=rider;t;t=t->nextRider){
    total+=t->getTotalVolume();
  }

  for(t=getStuff();t;t=t->nextThing){
    if(dynamic_cast<TComponent *>(t))
      total+=(int)(t->getTotalVolume()*0.10);
    else
      total+=t->getTotalVolume();
  }

  return total;
}

void TThing::setCarriedWeight(float num)
{
  carried_weight = num;
}

void TThing::setCarriedVolume(int num)
{
  carried_volume = num;
}

float TThing::getWeight() const
{
  return weight;
}

void TThing::setWeight(const float w)
{
  weight = w;
}

void TThing::addToWeight(const float w)
{
  weight += w;
}

void TThing::findComp(TComponent **, spellNumT)
{
}

void TThing::nukeFood()
{
  if (getMaterial() == MAT_FOODSTUFF)
    delete this;
}

int TThing::inRoom() const
{
  return in_room;
}

int TThing::getLight() const
{
  return light;
}

void TThing::setLight(int num)
{
  light = num;
}

void TThing::addToLight(int num)
{
  light += num;
}

void TThing::setRoom(int room)
{
  in_room = room;
}

void TThing::setMaterial(ubyte num)
{
  material_type = num;
}

ubyte TThing::getMaterial() const
{
  return material_type;
}

int TThing::getReducedVolume(const TThing *o) const
{
  // we use o if we want to do a check BEFORE the volume changes.
  // otherwise, (o == NULL) we fall into the parent case (normally done in += )
  // note: item might be in a bag and also have o == NULL
  // also: if item is in a bag, reduce volume by 5%

  int num = getTotalVolume();

  if ((o && dynamic_cast<const TBaseContainer *>(o)) ||
      (parent && dynamic_cast<const TBaseContainer *>(parent))) {
    // do material type reduction
    num /= material_nums[getMaterial()].vol_mult;

    // we know we are in a bag here, reduce by 5%
    num *= 95;
    num /= 100;
  }

  return num;
}

int TThing::getTotalVolume() const
{
  return getVolume();
}

float TThing::getTotalWeight(bool pweight) const
{
  const TOpenContainer *toc;
  float calc=0;

  if((toc=dynamic_cast<const TOpenContainer *>(this)) &&
     toc->isContainerFlag(CONT_WEIGHTLESS))
    calc = 0;
  else
    calc = getCarriedWeight();

  if (pweight)
    calc += getWeight();

  return calc;
}

void TThing::peeOnMe(const TBeing *ch)
{
  ch->sendTo("Piss on what?!\n\r");
}

void TThing::lightMe(TBeing *ch, silentTypeT)
{
  TObj *tObj;

  if(!material_nums[getMaterial()].flammability){
    act("You can't light $p, it's not flammable!", FALSE, ch, this, 0, TO_CHAR);
    return;
  } else if ((tObj = dynamic_cast<TObj *>(this)) && tObj->isObjStat(ITEM_MAGIC)) {
    act("$p resists your attempt at burning it...",
        FALSE, ch, this, NULL, TO_CHAR);
    return;
  } else {
    TThing *t;
    TTool *flintsteel;

    if (!(t = get_thing_char_using(ch, "flintsteel", 0, FALSE, FALSE)) ||
        !(flintsteel=dynamic_cast<TTool *>(t))){
      ch->sendTo("You need to own some flint and steel to light that.\n\r");
      return;
    }

    ch->sendTo("You begin to start a fire.\n\r");
    start_task(ch, this, NULL, TASK_LIGHTFIRE, "", 2, ch->inRoom(), 0, 0, 5);
  }
}

int TThing::chiMe(TBeing *tLunatic)
{
  int tMana = ::number(10, 30);

  if (tLunatic->getMana() <= tMana) {
    tLunatic->sendTo("You don't have the mana to do that!\n\r");
    return RET_STOP_PARSING;
  }

  act("$p resists your attempts to chi it!",
      FALSE, tLunatic, this, NULL, TO_CHAR);
  tLunatic->reconcileMana(TYPE_UNDEFINED, 0, tMana);

  return true;
}
