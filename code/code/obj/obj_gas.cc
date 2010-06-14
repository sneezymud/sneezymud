#include "comm.h"
#include "room.h"
#include "low.h"
#include "extern.h"
#include "obj_gas.h"
#include "pathfinder.h"
#include "obj_portal.h"
#include "being.h"
#include "obj_plant.h"
#include "materials.h"

// TGas uses function pointers to get a virtual-class like affect,
// without having to have multiple objects and object types for each
// gas type created in makeNewObj it also makes gasses able to change
// their type and behavior on the fly, which can be useful if decaying
typedef void (*specialsFunction)(TGas *myself);
typedef const char * (*nameFunction)(const TGas *myself);

int getSmokeIndex(int volume)
{
  if(volume<=0){
    return 0;
  } else if(volume<=5){
    return 1;
  } else if(volume<=10){
    return 2;
  } else if(volume<=25){
    return 3;
  } else if(volume<=50){ 
    return 4;
  } else if(volume<=100){
    return 5;
  } else if(volume<=250){
    return 6;
  } else if(volume<=1000){
    return 7;
  } else if(volume<=10000){
    return 8;
  } else if(volume<=25000){
    return 9;
  } else {
    return 10;
  }
}

void doNothing(TGas *myself)
{
}

void doChoke(TGas *myself)
{
  if(!myself->roomp || getSmokeIndex(myself->getVolume()) < 7)
    return;

  for(StuffIter it=myself->roomp->stuff.begin();it!=myself->roomp->stuff.end();){
    TBeing *tb;
    if((tb=dynamic_cast<TBeing *>(*(it++))) && !::number(0,4)){
      tb->sendTo(COLOR_BASIC, "<r>The large amount of smoke in the room causes you to choke and cough!<1>\n\r");
      int rc=tb->reconcileDamage(tb, ::number(3,11), DAMAGE_SUFFOCATION);
      
      if (IS_SET_DELETE(rc, DELETE_VICT)) {
        delete tb;
        continue;
      }
    }
  }
}

#define NUM_SCENTS 3
void doScent(TGas *myself)
{
  if(!myself->roomp)
    return;

  static const sstring self_scents[NUM_SCENTS] = {
    "Is that your own musk you smell?  It must be.",
    "This place smells alot like you.  You've really left your mark here.",
    "This smells like an interesting place full of danger or excitement!"
  };
  static const sstring opp_scents[NUM_SCENTS] = {
    "You smell an attractive troglodyte nearby.",
    "Someone was here recently who smells GOOD!  Yowza, what a troglodyte!",
    "You smell an alluring troglodyte nearby who was in danger!",
  };
  static const sstring same_scents[NUM_SCENTS] = {
    "The scent of some nearby showoff troglodyte fills your nostrils.",
    "You smell some troglodyge's musk nearby.  What a loser.",
    "It smells like troglodyte has stunk up the place here with their own musk.",
  };

  TBeing *createdBy = myself->getCreatedBy();


  for(StuffIter it= myself->roomp->stuff.begin();it!= myself->roomp->stuff.end();++it)
  {
    // only 25% chance of action per tick
    if (::number(0,3))
      continue;

    TPlant *plant = dynamic_cast<TPlant *>(*it);
    if (plant)
    {
      plant->updateAge();
      act("$n wilts slightly from exposure to $N!", FALSE, plant, NULL, myself, TO_ROOM);
      continue;
    }

    TBeing *being = dynamic_cast<TBeing *>(*it);
    if (!being)
      continue;

    if (/*!being->isImmortal() && */!being->isImmune(IMMUNE_SUFFOCATION, WEAR_BODY) &&
      !being->getMyRace()->hasTalent(TALENT_MUSK))
    {
      act("The smell of $N in this room makes you gag!", FALSE, being, NULL, myself, TO_CHAR);
      act("$n coughs and gags on $N in the room!", FALSE, being, NULL, myself, TO_ROOM);
      if (!being->isTough())
      {
        act("You feel really queasy.", FALSE, being, NULL, myself, TO_CHAR);
        act("$n turns a sickly pale color.", FALSE, being, NULL, myself, TO_ROOM);
        being->doAction("", CMD_PUKE);
      }
      continue;
    }
    const sstring *scents = NULL;
    if (myself->hasCreator(being->name))
      scents = self_scents;
    else if (createdBy && being->getSex() == createdBy->getSex())
      scents = same_scents;
    else
      scents = opp_scents;

    act(scents[::number(0, NUM_SCENTS-1)], FALSE, being, NULL, myself, TO_CHAR);
  }
}

const char * getNameNothing(const TGas *myself)
{
  return "smoke wisps generic";
}

const char * getDescNothing(const TGas *myself)
{
  return "an unidentified gas cloud is here.";
}

const char * getShortNameNothing(const TGas *myself)
{
  return "smoke";
}

const char * getNameSmoke(const TGas *myself)
{
  static const char *smokename [] =
  {
    "<k>a few wisps of smoke<1>", 
    "<k>a tiny cloud of smoke<1>", 
    "<k>a small cloud of smoke<1>", 
    "<k>a cloud of smoke<1>", 
    "<k>a fair sized cloud of smoke<1>", 
    "<k>a big cloud of smoke<1>", 
    "<k>a large cloud of smoke<1>", 
    "<k>a huge cloud of smoke<1>",
    "<k>a massive cloud of smoke<1>",
    "<k>a tremendously huge cloud of smoke<1>",
    "<k>a room full of smoke<1>"
  };
  return smokename[getSmokeIndex(myself->getVolume())];
}

const char * getDescSmoke(const TGas *myself)
{
  static const char *smokedesc [] =
  {
    "<k>A few wisps of smoke are fading fast.<1>",
    "<k>A tiny cloud of smoke has gathered here.<1>",
    "<k>A small cloud of smoke is here.<1>",
    "<k>A cloud of smoke is here.<1>",
    "<k>A fair sized cloud of smoke is here.<1>",
    "<k>A big cloud of smoke is here.<1>",
    "<k>A large cloud of smoke is here.<1>",
    "<k>A huge cloud of smoke is here.<1>",
    "<k>A massive cloud of smoke is here.<1>",
    "<k>A tremendously huge cloud of smoke dominates the area.<1>",
    "<k>The whole area is filled with smoke.<1>"
  };
  return smokedesc[getSmokeIndex(myself->getVolume())];
}

const char * getShortNameSmoke(const TGas *myself)
{
  return "smoke";
}

const char * getNameMusk(const TGas *myself)
{
  return "a cloud of musk";
}

const char * getDescMusk(const TGas *myself)
{
  return "a <Y>yellowish musk cloud<1> is here.";
}

const char * getShortNameMusk(const TGas *myself)
{
  return "musk cloud";
}

// specials for a gas - choking, stinking, etc
void TGas::doSpecials()
{
  static const specialsFunction specials[GAS_MAX] = { doNothing, doChoke, doScent };
  return specials[getType()](this);
}

// merge with other gas clouds in the room
void TGas::doMerge(TMergeable *tm)
{
  TGas *tGas;

  if(!(tGas=dynamic_cast<TGas *>(tm)))
    return;

  addCreator(tGas->creator.c_str());
  addToVolume(tGas->getVolume());

  --(*tGas);
  delete tGas;
}

bool TGas::willMerge(TMergeable *tm)
{
  TGas *tGas;

  if(!(tGas=dynamic_cast<TGas *>(tm)) ||
     tGas==this || tGas->type != type)
    return false;

  return true;
}


// drift upwards, or towards the closest outdoor room
void TGas::doDrift()
{
  roomDirData *exitp;
  TRoom *rp=roomp;
  TThing *t;
  TPortal *tp;

  if(!roomp)
    return;

  // move up if possible
  if((exitp=roomp->exitDir(DIR_UP)) &&
     !IS_SET(exitp->condition, EX_CLOSED) &&
     (rp=real_roomp(exitp->to_room))){
    act("$n drifts upwards.",FALSE, this, 0, 0, TO_ROOM); 
    --(*this);
    *rp += *this;
    act("$n drifts in from below.",FALSE, this, 0, 0, TO_ROOM); 
  } else {
    dirTypeT dir;

    if(!driftPath){
      driftPath=new TPathFinder();
      driftPath->setUsePortals(true);
      driftPath->setNoMob(false);
      driftPath->setThruDoors(false);
      driftPath->setRange(25);
      driftPath->setUseCached(true);
    }

    dir=driftPath->findPath(inRoom(), findOutdoors());

    if(dir >= MAX_DIR){
      dir=dirTypeT(dir-MAX_DIR+1);
      int seen = 0;

      for(StuffIter it=roomp->stuff.begin();it!=roomp->stuff.end();){
        if (!(t=*(it++)))
          continue;

        if ((tp=dynamic_cast<TPortal *>(t)) && dir == seen && (rp=real_roomp(tp->getTarget()))){
          act(format("$n drifts into %s.") % tp->getName(), FALSE, this, 0, 0, TO_ROOM);
          --(*this);
          *rp += *this;
          act(format("$n drifts in from %s.") % tp->getName(), FALSE, this, 0, 0, TO_ROOM);
          break;
        }
      }
    } else if (dir >= MIN_DIR && dir != DIR_DOWN && 
               (exitp=roomp->exitDir(dir)) &&
               (rp=real_roomp(exitp->to_room))){
      act(format("$n drifts %s.") % dirs_to_blank[dir], FALSE, this, 0, 0, TO_ROOM);
      --(*this);
      *rp += *this;
      act(format("$n drifts in from the %s.") % dirs[rev_dir[dir]], FALSE, this, 0, 0, TO_ROOM); 
    }
  }
}

TGas::~TGas(){
  delete driftPath;
}

TGas::TGas(gasTypeT gasType) :
  TObj()
{
  type = gasType;
  driftPath=NULL;
}

TGas::TGas(const TGas &a) :
  TObj(a)
{
  type = a.type;
  driftPath=NULL;
}

void TGas::setVolume(int n)
{
  TObj::setVolume(n);
  updateDesc();

  if(roomp && roomp->isIndoorSector()){
    obj_flags.decay_time=getVolume()/2;
  } else {
    obj_flags.decay_time=0;
  }
}

void TGas::addToVolume(int n)
{
  TObj::addToVolume(n);
  updateDesc();
  if(roomp && roomp->isIndoorSector()){
    obj_flags.decay_time=getVolume()/2;
  } else {
    obj_flags.decay_time=0;
  }
}

const char * TGas::getName() const
{
  static const nameFunction names[GAS_MAX] = { getNameNothing, getNameSmoke, getNameMusk };
  return names[getType()](this);
}

const char * TGas::getDesc() const
{
  static const nameFunction descs[GAS_MAX] = { getDescNothing, getDescSmoke, getDescMusk };
  return descs[getType()](this);
}

const char * TGas::getShortName() const
{
  static const nameFunction shortnames[GAS_MAX] = { getShortNameNothing, getShortNameSmoke, getShortNameMusk };
  return shortnames[getType()](this);
}

void TGas::updateDesc()
{
  if (isObjStat(ITEM_STRUNG)) {
    delete [] shortDescr;
    delete [] descr;

    extraDescription *exd;
    while ((exd = ex_description)) {
      ex_description = exd->next;
      delete exd;
    }
    ex_description = NULL;
    delete [] action_description;
    action_description = NULL;
  } else {
    addObjStat(ITEM_STRUNG);
    name = mud_str_dup(obj_index[getItemIndex()].name);
    ex_description = NULL;
    action_description = NULL;
  }

  shortDescr = mud_str_dup(getName());
  setDescr(mud_str_dup(getDesc()));
}

bool TGas::isPluralItem() const
{
  // a few drops of smoke
  if (getType() == GAS_SMOKE && getSmokeIndex(getVolume()) == 0)
    return true;

  // otherwise, make recursive
  return TObj::isPluralItem();
}

int TThing::dropGas(int amt, gasTypeT type)
{
  TGas *gas = NULL;

  if(amt == 0 || !roomp || type >= GAS_MAX)
    return FALSE;

  // look for preexisting smoke
  for(StuffIter it= roomp->stuff.begin();it!= roomp->stuff.end();++it)
  {
    gas = dynamic_cast<TGas *>(*it);
    if (gas && gas->getType() == type)
      break;
    gas = NULL;
  }

  // create new gas
  if (!gas)
  {
    TObj *obj = NULL;
    int robj = real_object(Obj::GENERIC_GAS);
    if (robj < 0 || robj >= (signed int) obj_index.size())
    {
      vlogf(LOG_BUG, format("dropGas(): No object (%d) in database!") %  robj);
      return FALSE;
    }
    if (!(obj = read_object(robj, REAL)))
    {
      vlogf(LOG_LOW, format("Error, No gas object created  (%d)") %  robj);
      return FALSE;
    }
    if (!(gas = dynamic_cast<TGas*>(obj)))
    {
      vlogf(LOG_LOW, "Error, Could not cast gas to TGas");
      return FALSE;
    }

    gas->setType(type);
    gas->swapToStrung();
    REMOVE_BIT(gas->obj_flags.wear_flags, ITEM_TAKE);
    gas->canBeSeen = 1;
    gas->setMaterial(MAT_GHOSTLY);
    gas->setObjStat(ITEM_NORENT);
    delete [] gas->name;
    gas->name = mud_str_dup(gas->getShortName());

    *roomp += *gas;
  }

  if (!gas->hasCreator(name))
    gas->addCreator(name);
  gas->addToVolume(amt);

  return TRUE;
}

void TGas::decayMe()
{
  int volume = getVolume();

  if (!roomp) {
    vlogf(LOG_BUG, "TGas::decayMe() called while TGas not in room!");
    setVolume(0);
    return;
  }

  if (volume <= 0)
    setVolume(0);
  else if (volume < 25)
    addToVolume((roomp->isIndoorSector() ? -5 : -8));
  else // large smokes evaporate faster
    addToVolume((roomp->isIndoorSector() ? -(volume/15) : -(volume/8))); 

  if (getVolume() < 0)
    setVolume(0);
}

sstring TGas::statObjInfo() const
{
  return "";
}

void TGas::getFourValues(int *x1, int *x2, int *x3, int *x4) const
{
  *x1 = int(type);
  *x2 = 0;
  *x3 = 0;
  *x4 = 0;
}

void TGas::assignFourValues(int t, int, int, int)
{
  type = gasTypeT(t);
}

