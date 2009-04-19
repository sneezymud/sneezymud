//////////////////////////////////////////////////////////////////////////
//
// SneezyMUD - All rights reserved, SneezyMUD Coding Team
//
//////////////////////////////////////////////////////////////////////////


// flame.cc

#include "handler.h"
#include "extern.h"
#include "room.h"
#include "being.h"
#include "obj_fuel.h"
#include "obj_organic.h"
#include "obj_trash.h"
#include "obj_tool.h"
#include "obj_flame.h"
#include "obj_light.h"
#include "obj_drinkcon.h"
#include "liquids.h"
#include "materials.h"
#include "weather.h"

TFFlame::TFFlame() :
  TBaseLight(),
  magBV(0)
{
}

TFFlame::TFFlame(const TFFlame &a) :
  TBaseLight(a),
  magBV(a.magBV)
{
}

TFFlame & TFFlame::operator=(const TFFlame &a)
{
  if (this == &a) return *this;
  TBaseLight::operator=(a);
  magBV = a.magBV;
  return *this;
}

TFFlame::~TFFlame()
{
}

void TFFlame::assignFourValues(int x1, int x2, int x3, int x4)
{
  TBaseLight::assignFourValues(x1, x2, x3, x4);

  setMagBV(x4);
}

void TFFlame::getFourValues(int *x1, int *x2, int *x3, int *x4) const
{
  TBaseLight::getFourValues(x1, x2, x3, x4);

  *x4 = getMagBV();
}

void TFFlame::setMagBV(int x3)
{
  magBV = x3;
}

int TFFlame::getMagBV() const
{
  return magBV;
}

bool TFFlame::hasMagBV(int cBV) const
{
  return (magBV & cBV);
}

void TFFlame::addMagBV(int cBV)
{
  magBV |= cBV;
}

void TFFlame::remMagBV(int cBV)
{
  magBV &= ~cBV;
}

sstring TFFlame::statObjInfo() const
{
  char Buf[2][256]={"\0", "\0"};

  // This is where we find out which various magic properites
  // have been applied to this flame:
  //   1 = [Inverse Heat]  = light is Negative
  //   2 = [Inverse Light] = heat is Negative
  //   4 = [Magic Heat]    = Fire produces *2 heat.
  //   8 = [Magic Light]   = Fire produces *2 light.
  //  16 = [Immortal]      = Never decays, cannot be added to.

  if (hasMagBV(TFFLAME_INVHEAT))
    sprintf(Buf[1], " [Inverse Heat]");
  else if (hasMagBV(TFFLAME_INVLIGHT))
    sprintf(Buf[1] + strlen(Buf[1]), " [Inverse Light]");
  else if (hasMagBV(TFFLAME_MAGHEAT))
    sprintf(Buf[1] + strlen(Buf[1]), " [Magic Heat]");
  else if (hasMagBV(TFFLAME_MAGLIGHT))
    sprintf(Buf[1] + strlen(Buf[1]), " [Magic Light]");
  else if (hasMagBV(TFFLAME_IMMORTAL))
    sprintf(Buf[1] + strlen(Buf[1]), " [Immortal]");

  sprintf(Buf[0], "Light: %s (%d) Heat: %d Magics:\n\r\t%s",
          describe_light(getLightAmt()),
          getLightAmt(),
          getMaxBurn(),
          (strlen(Buf[1]) > 0 ? Buf[1] : " --none--"));

  sstring a(Buf[0]);
  return a;
}

// Class specific functions:
/*******************************************************************************
 * Notes:
 *   Right now the fire and trash objects are created on the fly, need to get
 * some proper ones made up and added in and just load from the db.  Then just
 * modify the weight and stuff.  Might want to keep a strung fire, because it
 * will end up having a Lot of different descrs later on and will be going from
 * one to another pretty often.
 *   Go over all the messages again, some are still pretty basic and can be
 * better than they are.
 *   Modify addFlameToMe so decay_time =/+= takes the fireItem into account
 * instead of just player level.
 *   Make sure to remove the "Lapsos" check in doIgnite when this goes in offical
 *******************************************************************************/

// called by 'doPour substance TFFlame_item'
// if return == false then doPour continues execution
// if return == true then doPour will stop execution
int TFFlame::pourWaterOnMe(TBeing *ch, TObj *sObj)
{
  TDrinkCon *dContainer = NULL;
  TFuel *fContainer = NULL;
  char Buf[2][256];
  int drunk=0;
  liqTypeT type=LIQ_WATER;

  if (!(dContainer = dynamic_cast<TDrinkCon *>(sObj)) &&
      !(fContainer = dynamic_cast<TFuel *>(sObj))) {
    ch->sendTo("Might help to try something that has some liquid in it.\n\r");
    return false;
  }
  if ((dContainer && dContainer->getDrinkUnits() <= 0) ||
      (fContainer && fContainer->getCurFuel() <= 0)) {
    ch->sendTo("I'm afraid that container is empty.\n\r");
    return true;
  }
  // Does the substance were pouring have a 'drunk' factor?  If so then this
  // substance is most likely liquor, so we flare up and add to the decay
  // instead of removing from it.
  // Also.  Are we adding lantern fuel to this?  If so, *3 the strength.
  if (fContainer || (drunk = liquidInfo[(type = dContainer->getDrinkType())]->drunk) > 0) {
    char lName[256] = "<r>lantern fuel<z>";
    if (dContainer) {
      strcpy(lName, liquidInfo[type]->name);
      dContainer->setDrinkUnits(0);
    } else {
      drunk = fContainer->getCurFuel()*3;
      fContainer->setCurFuel(0);
    }
    if (!hasMagBV(TFFLAME_IMMORTAL))
      obj_flags.decay_time = (short int) min(200, (int) (obj_flags.decay_time+drunk*2));

    // message depending on how Strong the substance is
    if (drunk < 5) {
      sprintf(Buf[0], "%s suddenly flares up as you pour %s on it.\n\r",
              sstring(shortDescr).cap().c_str(), lName);
      sprintf(Buf[1], "%s flares up as $n pours %s over it.",
              sstring(shortDescr).cap().c_str(), lName);
    } else if (drunk < 10) {
      sprintf(Buf[0], "%s bursts outwards as you pour %s on it.\n\r",
              sstring(shortDescr).cap().c_str(), lName);
      sprintf(Buf[1], "%s bursts outwards as $n pours %s over it.",
              sstring(shortDescr).cap().c_str(), lName);
    } else {
      sprintf(Buf[0], "%s nearly explodes as you pour %s on it.\n\r",
              sstring(shortDescr).cap().c_str(), lName);
      sprintf(Buf[1], "%s nearly explodes as $n pours %s over it.",
              sstring(shortDescr).cap().c_str(), lName);
    }

    ch->sendTo(COLOR_OBJECTS, Buf[0]);
    act(Buf[1], TRUE, ch, NULL, NULL, TO_ROOM);
    // Hit decay.  Updates the messages in a clean fashion.
    decayMe();
  } else {
    // Wasn't liquor or fuel, prolly water or something.  So we remove from the decay_time
    if (!hasMagBV(TFFLAME_IMMORTAL))
      obj_flags.decay_time = max(0, (int)(obj_flags.decay_time-(dContainer->getDrinkUnits()/2)));
    dContainer->setDrinkUnits(0);
    // If object is left, then we 'crack and pop'
    if (obj_flags.decay_time > 0) {
      ch->sendTo(COLOR_OBJECTS, format("%s lets off a large crack and pop as you pour some %s on it.\n\r") %
                 sstring(shortDescr).cap() % liquidInfo[dContainer->getDrinkType()]->name);
      sprintf(Buf[0], "%s dies down a little as $n pours %s over it.",
              sstring(shortDescr).cap().c_str(), liquidInfo[dContainer->getDrinkType()]->name);
      act(Buf[0], TRUE, ch, NULL, NULL, TO_ROOM);
      decayMe();
    } else {
      // The liquid killed the fire, lets decay it and drop some ash.
      objectDecay();
      delete this;
    }
  }
  return true;
}

// Technically this should never be called for a mortal, but if a builder
// creates a TFFlame object and forgets to put !TAKE, lets prevent it
// here.  If it's an immortal, then just let them get it.
int TFFlame::getMe(TBeing *ch, TThing *)
{
  if (!ch->isImmortal()) {
    ch->sendTo(COLOR_OBJECTS,
               format("As you grab ahold of %s % you feel your fingers begin to burn and drop it.\n\r") %
               shortDescr);
    ch->doDrop("", this, true);
    if (ch->reconcileDamage(ch, ::number(1,2), DAMAGE_FIRE) == -1)
      return DELETE_VICT; // This means we killed the player.
    return TRUE;
  }

  putLightOut();
  return FALSE;
}

bool TFFlame::isLit() const
{
  return true;
}

void TFFlame::lightMe(TBeing *ch, silentTypeT)
{
  act("$p is already burning, what else do you want?", false, ch, this, 0, TO_CHAR);
}

void TFFlame::extinguishMe(TBeing *ch)
{
  ch->sendTo("I'm afraid it can't be put out in that fashion.\n\r");
}

void TFFlame::peeOnMe(const TBeing *ch)
{
  ch->sendTo(COLOR_OBJECTS, format("Steam rises from %s as you pee on it.\n\r") % getName());
}

void TFFlame::refuelMeLight(TBeing *ch, TThing *)
{
  ch->sendTo("I'm afraid it doesn't work that way, maybe add some more fire wood?\n\r");
}

void TFFlame::describeObjectSpecifics(const TBeing *ch) const
{
  ch->sendTo(COLOR_OBJECTS, format("%s seems to be %s.") % getName() %
    (obj_flags.decay_time > 150 ? "burning wildly" :
    (obj_flags.decay_time > 100 ? "burning fiercly" :
    (obj_flags.decay_time >  50 ? "burning barely out of control" :
    (obj_flags.decay_time >  25 ? "barely burning" : "slightly burning" )))));
}

// This is called when decay_time <= 0, thus the item is lost.
int TFFlame::objectDecay()
{
  if (!roomp || hasMagBV(TFFLAME_IMMORTAL)) return FALSE;

  sendrpf(COLOR_OBJECTS, roomp,
          "You hear a final crack and pop as the %s dies down.\n\r", shortDescr);
  putLightOut();
  TTrash *fireAsh;
  fireAsh = new TTrash();
  fireAsh->swapToStrung();
  delete [] fireAsh->name;
  delete [] fireAsh->shortDescr;
  delete [] fireAsh->descr;
  delete [] fireAsh->ex_description;
  delete [] fireAsh->action_description;
  fireAsh->name = mud_str_dup("ash soot debris pit fire");
  fireAsh->shortDescr = mud_str_dup("<k>a pile of ashes<z>");
  fireAsh->descr = mud_str_dup("<k>A pile of soot and ash is here, <z><r>smoldering<k> away.<z>");
  setMaterial(MAT_POWDER);
  fireAsh->obj_flags.decay_time = 10;
  fireAsh->setWeight((int)(getWeight()*.9));
  fireAsh->setVolume((int)(getVolume()*.9));
  *roomp += *fireAsh;

  // We want to delete the TFFlame object, so tell it so.
  return DELETE_THIS;
}

// Called every tick to knock some decay_time off of us.
void TFFlame::decayMe()
{
  if (!roomp || hasMagBV(TFFLAME_IMMORTAL)) return;

  // Current weather affects decay_time loss.

  if (Weather::getWeather(*roomp) == Weather::SNOWY) {
    sendrpf(roomp, "The current snowstorm kills the fire a little more.\n\r");
    obj_flags.decay_time -= 5;
  } else if (Weather::getWeather(*roomp) == Weather::LIGHTNING) {
    sendrpf(roomp, "The great downpour of rain kills the fire and puts it out.\n\r");
    obj_flags.decay_time = 0;
  } else if (Weather::getWeather(*roomp) == Weather::RAINY) {
    sendrpf(roomp, "The rain effects the fire and slowly kills it.\n\r");
    obj_flags.decay_time -= 20;
  } else obj_flags.decay_time--;

  // If item still has some decay_time left, give it new 'size' messages.
  // Also update the light and such.  Also drop some smoke into the room.
  if (obj_flags.decay_time > 0) {
    addFlameMessages();
  //dropSmoke(max(1, obj_flags.decay_time/25));
  } else
    obj_flags.decay_time = 0;
  updateFlameInfo();
}

// Sets new light/burn(heat) values and also changes the resulting light.
void TFFlame::updateFlameInfo()
{
  if (!roomp) return;

  if (obj_flags.decay_time > 0) {
    setLightAmt(max(5, (int)(obj_flags.decay_time/10*
                (hasMagBV(TFFLAME_MAGLIGHT) ? 2 : 1))));
    if (hasMagBV(TFFLAME_INVLIGHT))
      setLightAmt(-getLightAmt());
    setMaxBurn(max(1, (int)(obj_flags.decay_time/20*
               (hasMagBV(TFFLAME_MAGHEAT) ? 2 : 1))));
    if (hasMagBV(TFFLAME_INVHEAT))
      setMaxBurn(-getMaxBurn());
    setCurBurn(getMaxBurn());
  } else {
    setLightAmt(0);
    setMaxBurn(0);
    setCurBurn(0);
  }
  bool lFound = false;
  // See if we've already been lit before, and just modify it.
  for (int i = 0; (!lFound && i < MAX_OBJ_AFFECT); i++)
    if (affected[i].location == APPLY_LIGHT) {
      addToLight(getLightAmt()-affected[i].modifier);
      //roomp->addToLight(getLightAmt()-affected[i].modifier);
      affected[i].modifier = getLightAmt();
      lFound = true;
    }
  // See if we were Not lit before, if not lets light up.
  for (int i = 0; (!lFound && i < MAX_OBJ_AFFECT); i++)
    if (affected[i].location == APPLY_NONE) {
      affected[i].location = APPLY_LIGHT;
      affected[i].modifier = getLightAmt();
      addToLight(affected[i].modifier);
      //roomp->addToLight(affected[i].modifier);
      lFound = true;
    }
  if (!lFound)
    vlogf(LOG_BUG, format("TFFlame object with No extra slots for lighting [%s].") % 
          (shortDescr ? shortDescr : "BAD OBJECT!"));
}

// rename the item depending on it's size[determined by decay_time].
void TFFlame::addFlameMessages()
{
  if (hasMagBV(TFFLAME_IMMORTAL)) return;

  swapToStrung();

  delete [] name;
  delete [] shortDescr;
  delete [] descr;
  if (obj_flags.decay_time <= 25) {
    name       = mud_str_dup("fire flame wood tiny");
    shortDescr = mud_str_dup("<r>a tiny fire<z>");
    descr      = mud_str_dup("<r>A tiny fire is here.<z>");
  } else if (obj_flags.decay_time <= 50) {
    name       = mud_str_dup("fire flame wood small");
    shortDescr = mud_str_dup("<r>a small fire<z>");
    descr      = mud_str_dup("<r>A small fire is here.<z>");
  } else if (obj_flags.decay_time <= 100) {
    name       = mud_str_dup("fire flame wood large");
    shortDescr = mud_str_dup("<r>a large fire<z>");
    descr      = mud_str_dup("<r>A large fire is here.<z>");
  } else if (obj_flags.decay_time <= 150) {
    name       = mud_str_dup("fire flame wood huge");
    shortDescr = mud_str_dup("<r>a huge fire<z>");
    descr      = mud_str_dup("<r>A huge fire is here.<z>");
  } else {
    name       = mud_str_dup("fire flame wood inferno huge");
    shortDescr = mud_str_dup("<r>a huge inferno<z>");
    descr      = mud_str_dup("<r>A huge inferno is here, blazing away.<z>");
  }
  setMaterial(MAT_FIRE);
}

// Used when a person 'looks' and sees the object in a room and such.
// TLight does a (lit) and I prefer not to have that here.
sstring TFFlame::showModifier(showModeT, const TBeing *) const
{
  sstring a("");
  return a;
}

// What the person sees when they 'ignite' a TFFlame object.
int TFFlame::igniteMessage(TBeing *ch) const
{
  char Buf[2][256];
  bool sFound=true;
  int ePower=1,
      mana=0;

  // Basically we want to give messages depending on the person who is doing
  // the igniting.
  // Cost/Mage:
  //  13/Flaming Sword
  //   8/Fireball
  //   2/Hands of Flame
  // Cost/Cleric/Deikhan:
  //  13/Flamestrike (Cleric only)
  //   2/Rain Brimstone
  // Else...
  if (ch->isImmortal()) {
    sprintf(Buf[0], "You stare at some fire wood, causing it to ignite!\n\r");
    sprintf(Buf[1], "$n stares at some fire wood, causing it to burst into flames!\n\r");
  } else if (ch->hasClass(CLASS_MAGE)) {
    if (ch->doesKnowSkill(SPELL_FLAMING_SWORD) && ch->getMana() > 13) {
      sprintf(Buf[0], "You create a minor sword of flame and slash some fire wood with it.\n\r");
      sprintf(Buf[1], "$n creates a minor sword of flame and slashes some fire wood with it.");
      ePower = 4;
      mana = 13;
    } else if (ch->doesKnowSkill(SPELL_FIREBALL) && ch->getMana() > 8) {
      sprintf(Buf[0], "You launch a minor fireball into some fire wood.\n\r");
      sprintf(Buf[1], "$n launches a minor fireball into some fire wood.");
      ePower = 3;
      mana = 8;
    } else if (ch->doesKnowSkill(SPELL_HANDS_OF_FLAME) && ch->getMana() > 2) {
      sprintf(Buf[0], "You embue your hands with flame and touch the fire wood, lighting it.\n\r");
      sprintf(Buf[1], "$n embues $s hands with flames and touches the fire wood, lighting it.");
      ePower = 2;
      mana = 2;
    } else sFound = false;
  } else if (ch->hasClass(CLASS_CLERIC) || ch->hasClass(CLASS_DEIKHAN)) {
    if (ch->doesKnowSkill(SPELL_FLAMESTRIKE) && ch->getPiety() > 13) {
      sprintf(Buf[0], "You call a minor flamestrike down onto the fire wood.\n\r");
      sprintf(Buf[1], "$n calls a minor flamestrike down onto the fire wood.");
      ePower = 3;
      mana = 13;
    } else if ((ch->doesKnowSkill(SPELL_RAIN_BRIMSTONE) ||
               ch->doesKnowSkill(SPELL_RAIN_BRIMSTONE_DEIKHAN)) && ch->getPiety() > 2) {
      sprintf(Buf[0], "You call down some brimstone upon some fire wood.\n\r");
      sprintf(Buf[1], "$n calles down some brimstone upon some fire wood.");
      ePower = 2;
      mana = 2;
    } else sFound = false;
  } else sFound = false;
  // Rangers get a bonus regardless...
  if (ch->hasClass(CLASS_RANGER))
    ePower = 3;
  // Either wasn't a mage/cleric/deikhan or didn't have an appropriate skill,
  // so we revert to the old Flint&Steel tool...But not for immortals.
  if (!sFound) {
    TTool *cTool = NULL;
    for(StuffIter it=ch->stuff.begin();it!=ch->stuff.end();++it){
      if((cTool=dynamic_cast<TTool *>(*it)) &&
	 (cTool->getToolType() != TOOL_FLINTSTEEL))
	break;
    }
	 
    if ((!cTool || !cTool->getToolType())) {
      ch->sendTo("I'm afraid you need some flint and steel for this.\n\r");
      return -1;
    }
    sprintf(Buf[0], "You spark your flint and steel against some fire wood, igniting it.\n\r");
    sprintf(Buf[1], "$n sparks some flint and steel against some fire wood, igniting it.\n\r");
  } else {
    if (ch->hasClass(CLASS_MAGE))
      ch->addToMana(-mana);
    else {
      double piety = mana;
      ch->addToPiety(-piety);
    }
  }

  ch->sendTo(Buf[0]);
  act(Buf[1], TRUE, ch, NULL, NULL, TO_ROOM);
  return ePower;
}

// Basically this creates the actual TFFlame object for us, and removes the
// TOrganic type 3 'woodItem'.
// Doing it this way we can either let it be called through the lightMe->igniteObject
// or let them call it directly.
void TFFlame::addFlameToMe(TBeing *ch, const char *argument, TThing *fObj, bool isFirst)
{
  int count = 0, ePower=1;
  TThing *woodItem;
  TOrganic *fireItem;

  if (!fObj) {
    for (; isspace(*argument); argument++);

    if (!*argument) {
      ch->sendTo("It helps to specify Something...\n\r");
      if (isFirst) delete this;
      return;
    }
    if (!(woodItem = searchLinkedListVis(ch, argument, ch->stuff, &count)) &&
        !(woodItem = searchLinkedListVis(ch, argument, ch->roomp->stuff, &count))) {
      ch->sendTo(COLOR_OBJECTS, format("You can not seem to find the '%s'.\n\r") % argument);
      if (isFirst) delete this;
      return;
    }
  } else
    woodItem = fObj;
  
  // Make sure we got a TOrganic type 3, It's not a Fire, and not a
  // PC or NPC.
  if (dynamic_cast<TBeing *>(woodItem)) {
    ch->sendTo("I'm sure they would Really love that one...\n\r");
    if (isFirst) delete this;
    return;
  } else if (dynamic_cast<TFFlame *>(woodItem)) {
    ch->sendTo("That fire is already burning.\n\r");
    if (isFirst) delete this;
    return;
  } else if (!(fireItem = dynamic_cast <TOrganic *>(woodItem)) ||
             fireItem->getOType() != 3) {
    ch->sendTo("I'm afraid only fire wood can be used for this.\n\r");
    if (isFirst) delete this;
    return;
  }

  // Messages depending on if this is the First TFFlame in the room or not.
  if (!isFirst) {
    ch->sendTo("You throw some fire wood into the pile.\n\r");
    obj_flags.decay_time = (short int) min(200, (int) (obj_flags.decay_time+max(1,
                            (int) (fireItem->getVolume()/300+ch->GetMaxLevel()/10))));
  } else {
    delete [] ex_description;
    delete [] action_description;
    if ((ePower = igniteMessage(ch)) == -1) {
      delete this;
      return;
    }
    if (ePower > 1) {
      if (ch->hasClass(CLASS_MAGE))
        setMagBV(TFFLAME_MAGHEAT);
      else if (ch->hasClass(CLASS_CLERIC) || ch->hasClass(CLASS_DEIKHAN))
        setMagBV(TFFLAME_MAGLIGHT);
    }
    obj_flags.decay_time = (short int) max(1,
                           (int) ((fireItem->getVolume()/300+ch->GetMaxLevel()/10)*ePower));
  }

  addFlameMessages();

  obj_flags.wear_flags = 0; // Make it no-take, safty first.
  obj_flags.cost       = 0;
  setVolume(min(150000, (int) ((isFirst ? 0 : getVolume()) +
                   (fireItem->getVolume()*(.9+ch->GetMaxLevel()/50)))));
  setWeight(min(2000, (int) ((isFirst ? 0 : getWeight()) +
                   (fireItem->getWeight()*(.9+ch->GetMaxLevel()/50)))));

  // We use the TOrganic type 3 wood to create the fire, so lets get rid of it.
  delete fireItem;
  if (isFirst) *ch->roomp += *dynamic_cast<TThing *>(this);
  updateFlameInfo();
}

// Non-Class Specific functions
void TBeing::igniteObject(const char *argument, TThing *fObj)
{

  TFFlame *newFlame = NULL;

  if (!roomp) return;
  if (roomp->isWaterSector()) {
    sendTo("Unfortunatly, it doesn't work to start a fire in the water.\n\r");
    return;
  }
  if (Weather::getWeather(*roomp) == Weather::LIGHTNING ||
      Weather::getWeather(*roomp) == Weather::RAINY) {
    sendTo("I'm afraid it doesn't work to ignite a fire in the rain.\n\r");
    return;
  }
  // See if were doing a new flame or adding to an existing one.
    for(StuffIter it=roomp->stuff.begin();it!=roomp->stuff.end();++it){
      if((newFlame=dynamic_cast<TFFlame *>(*it)))
	break;
    }
  if (newFlame) {
    if (!newFlame->isObjStat(ITEM_STRUNG)) newFlame->swapToStrung();
    newFlame->addFlameToMe(this, argument, fObj, false);
    return;
  }

  // must be new, so lets create it and make sure it got created.
  if (!(newFlame = new TFFlame())) {
    vlogf(LOG_BUG, format("Was unable to allocate for new Flame item.  User[%s]") %  getName());
    sendTo("Something bad occured, tell a god.\n\r");
    return;
  }
  newFlame->swapToStrung();
  newFlame->addFlameToMe(this, argument, fObj, true);
}

// Non-Class partly related functions:
int TBeing::pourWaterOnMe(TBeing *ch, TObj *sObj)
{
  liqTypeT type  = LIQ_WATER;
  int drunk = 0;
  int size  = 0,
    rc    = 1;
  char Buf[256];
  TBaseCup *dContainer;

  if (!(dContainer = dynamic_cast<TBaseCup *>(sObj)))
    return false; // let doPour continue its run, we don't do this.

  if ((size = dContainer->getDrinkUnits()) <= 0) {
    ch->sendTo("I'm afraid that container is empty.\n\r");
    return true; // stop doPour, if we had units we would have done something.
  }

  if (ch->checkPeaceful("Vicious things cannot be done here, and that is awfully vicious...\n\r")
      || noHarmCheck(this))
    return true;

  if (ch->fight() || ch->task) {
    ch->sendTo("Come now...  You are a bit busy for that, don't you think??\n\r");
    return true;
  }

  drunk = liquidInfo[(type = dContainer->getDrinkType())]->drunk;

  if(getPosition() <= POSITION_STUNNED){
    // unconscious person, let's pour it in their mouth instead
    sprintf(Buf, "You carefully pour %s into $N's mouth!", 
	    liquidInfo[type]->name);
    act(Buf, TRUE, ch, 0, this, TO_CHAR);
    sprintf(Buf, "$n just poured %s into your mouth.", liquidInfo[type]->name);
    act(Buf, TRUE, ch, 0, this, TO_VICT);
    sprintf(Buf, "$n just poured %s into $N's mouth.", liquidInfo[type]->name);
    act(Buf, TRUE, ch, 0, this, TO_NOTVICT);
    
    setQuaffUse(TRUE);
    dContainer->sipMe(this);
    setQuaffUse(FALSE);

    return true;
  }

  sprintf(Buf, "You pour %s all over $N!", liquidInfo[type]->name);
  act(Buf, TRUE, ch, 0, this, TO_CHAR);
  sprintf(Buf, "$n just poured %s all over you!", liquidInfo[type]->name);
  act(Buf, TRUE, ch, 0, this, TO_VICT);
  sprintf(Buf, "$n just poured %s all over $N!", liquidInfo[type]->name);
  act(Buf, TRUE, ch, 0, this, TO_NOTVICT);

  // prevent people from using this at real low level to gain xp then suicide or something.
  if ((ch->GetMaxLevel()+3) < GetMaxLevel()) {
    ch->addToWait(combatRound(2));
    if (!isPc()) {
      ch->setCharFighting(this);
      ch->setVictFighting(this);
    }
    return true;
  }

  if (getMaterial(WEAR_BODY) == MAT_FIRE) {
    int drunk;
    if ((drunk = liquidInfo[type]->drunk) > 0) {
      act("$N suddenly flares up, but $E doesn't really look happy now.",
        TRUE, ch, 0, this, TO_CHAR);
      act("Yow!  You suddenly flare up, but that wasn't nice of them...",
        TRUE, ch, 0, this, TO_VICT);
      act("$N suddenly flares up, but $E doesn't really look all that happy...",
        TRUE, ch, 0, this, TO_NOTVICT);

      rc = reconcileDamage(this, -(::number(5, max(6, min(25, 15+drunk)))), DAMAGE_FIRE);
      size = 0;
    } else {
      act("$N dies down a little, $E does Not look happy!",
        TRUE, ch, 0, this, TO_CHAR);
      act("That HURT!",
        TRUE, ch, 0, this, TO_VICT);
      act("$N dies down a little and begins fuming at $n.",
        TRUE, ch, 0, this, TO_NOTVICT);

      rc = reconcileDamage(this, ::number(5, max(2, min(5, (int) (size/20)))), DAMAGE_FROST);
    }
  } else if (type == LIQ_HOLYWATER && (isDiabolic() || isUndead() || isLycanthrope())) {
    act("$N looks like $E is in pain!  And they seem pretty mad at YOU!",
        TRUE, ch, 0, this, TO_CHAR);
    act("That was <b>HOLYWATER<z>!!!  That <r>HURTS<z>!!!",
        TRUE, ch, 0, this, TO_VICT);
    act("$N looks like $E is in pain!  And they seem pretty mad at $n.",
        TRUE, ch, 0, this, TO_NOTVICT);
    
    rc = reconcileDamage(this, ::number(5, max(6, min(15, (int) (size/20)))), DAMAGE_DISRUPTION);
  } else if (roomp && roomp->isArcticSector() && type != LIQ_WARM_MEAD &&
      getMaterial(WEAR_BODY) != MAT_ICE) {

    act("$N looks very cold now, I think your going to have a bad day...",
        TRUE, ch, 0, this, TO_CHAR);
    act("BRRRRR!  That doesn't help much, now your REALLY cold!",
        TRUE, ch, 0, this, TO_VICT);
    act("$N looks very cold now.  $n is probably in a lot of trouble.",
        TRUE, ch, 0, this, TO_NOTVICT);

    rc = reconcileDamage(this, ::number(5, max(2, min(5, (int) (size/20)))), DAMAGE_FROST);
  } else if ((type != LIQ_WHISKY) && (type != LIQ_FIREBRT) &&
	     (type != LIQ_VODKA) && (type != LIQ_RUM) &&
	     (type != LIQ_BRANDY)){
    TThing *t;
    TObj *obj = NULL;
    int i;

    for (i = MIN_WEAR;i < MAX_WEAR;i++) {
      if (!(t = equipment[i]) || !(obj = dynamic_cast<TObj *>(t)) ||
	  !obj->isObjStat(ITEM_BURNING) || ::number(0,3))
	continue;
      obj->remBurning(ch);
      act("Your $p is extinguished.", FALSE, this, obj, 0, TO_CHAR);
      act("$n's $p is extinguished.", FALSE, this, obj, 0, TO_ROOM);
    }
  }

  // add wetness affect
  if (size > 0)
  {
    Weather::addWetness(this, size); // we never expect this to return 0
    sendTo(format("You feel %s.\n\r") % Weather::describeWet(this));
  }

  dContainer->setDrinkUnits(0);  

  // This prevents mass use from one person:
  ch->addToWait(combatRound(2));
  if (!isPc()) {
    ch->setCharFighting(this);
    ch->setVictFighting(this);
  }
  return rc;
}

// this is probably something that should *NOT* be called
void TFFlame::putLightOut()
{
  // You are correct.  This really messes up the flame and will
  // cause it to go -light.
  //  TBaseLight::putLightOut();
}

int TFFlame::chiMe(TBeing *tLunatic)
{
  int tMana  = ::number(10, 30),
      bKnown = tLunatic->getSkillLevel(SKILL_CHI);

  if (tLunatic->getMana() < tMana) {
    tLunatic->sendTo("You lack the chi to do this.\n\r");
    return RET_STOP_PARSING;
  } else
    tLunatic->reconcileMana(TYPE_UNDEFINED, 0, tMana);

  if (!tLunatic->bSuccess(bKnown, SKILL_CHI)) {
    act("You fail to affect $p in any way.",
        FALSE, tLunatic, this, NULL, TO_CHAR);
    return true;
  }

  act("You focus your chi, causing $p to burst momentarily!",
      FALSE, tLunatic, this, NULL, TO_CHAR);
  act("$n stares at $p, causing it to burst momentarily",
      TRUE, tLunatic, this, NULL, TO_ROOM);

  obj_flags.decay_time += 10;

  return true;
}
