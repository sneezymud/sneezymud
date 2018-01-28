#include "process.h"
#include "timing.h"
#include "database.h"
#include "shop.h"
#include "parse.h"
#include "faction.h"
#include "extern.h"
#include "toggle.h"
#include "guild.h"
#include "being.h"
#include <sys/shm.h>
#include <sys/ipc.h>


///////////

// procFactoryProduction
procFactoryProduction::procFactoryProduction(const int &p)
{
  trigger_pulse=p;
  name="procFactoryProduction";
}

void procFactoryProduction::run(const TPulse &) const
{
  TDatabase db(DB_SNEEZY);

  db.query("select distinct shop_nr from factoryproducing");

  while(db.fetchRow()){
    factoryProduction(convertTo<int>(db["shop_nr"]));
  }
}

// procSaveFactions
procSaveFactions::procSaveFactions(const int &p)
{
  trigger_pulse=p;
  name="procSaveFactions";
}

void procSaveFactions::run(const TPulse &) const
{
  save_factions();
}

// procSaveNewFactions
procSaveNewFactions::procSaveNewFactions(const int &p)
{
  trigger_pulse=p;
  name="procSaveNewFactions";
}

void procSaveNewFactions::run(const TPulse &) const
{
  save_guilds();
}

// procDoComponents
procDoComponents::procDoComponents(const int &p)
{
  trigger_pulse=p;
  name="procDoComponents";
}

void procDoComponents::run(const TPulse &) const
{
  do_components(-1);
}


// procPerformViolence
procPerformViolence::procPerformViolence(const int &p)
{
  trigger_pulse=p;
  name="procPerformViolence";
}

void procPerformViolence::run(const TPulse &pl) const
{
  perform_violence(pl.pulse);
}

///////

bool TBaseProcess::should_run(int p) const
{
  if(!(p % trigger_pulse))
    return true;
  else
    return false;
}

void TScheduler::add(TProcess *p)
{
  procs.push_back(p);
}

void TScheduler::add(TObjProcess *p)
{
  obj_procs.push_back(p);
}

void TScheduler::add(TCharProcess *p)
{
  char_procs.push_back(p);
}

TScheduler::TScheduler(){
  pulse.init(0);
  placeholder=read_object(42, VIRTUAL);
  // don't think we can recover from this
  mud_assert(placeholder!=NULL, "couldn't load placeholder object");
  mud_assert(real_roomp(0) != NULL, "couldn't load room 0");
  *(real_roomp(0)) += *placeholder;
  objIter=find(object_list.begin(), object_list.end(), placeholder);
  tmp_ch=NULL;
}

void TScheduler::runObj(int pulseNum)
{
  int count;
  TObj *obj;

  // we want to go through 1/12th of the object list every pulse
  // obviously the object count will change, so this is approximate.
  count=(int)((float)objCount/11.5);

  while(count--){
    // remove placeholder from object list and increment iterator
    object_list.erase(objIter++);

    // set object to be processed
    obj=(*objIter);

    // move to front of list if we reach the end
    // otherwise just stick the placeholder in
    if(++objIter == object_list.end()){
      object_list.push_front(placeholder);
      objIter=object_list.begin();
    } else {
      object_list.insert(objIter, placeholder);
      --objIter;
    }

    bool invalidated = false;
    for(std::vector<TObjProcess *>::iterator iter=obj_procs.begin();
        iter!=obj_procs.end();++iter){
      if((*iter)->should_run(pulse.pulse)){

        if((*iter)->run(pulse, obj)){
          delete obj;
          invalidated = true;
          break;
        }
      }
    }
    if (invalidated) {
      auto newObjIter = find(object_list.begin(), object_list.end(), placeholder);
      if (objIter != newObjIter)
        objIter = newObjIter;
    }
  }
}

void TScheduler::runChar(int pulseNum)
{
  TBeing *temp;
  int count;

  // we've already finished going through the character list, so start over
  if(!tmp_ch)
    tmp_ch=character_list;

  count=max((int)((float)mobCount/11.5), 1);

  for (; tmp_ch; tmp_ch = temp) {
    temp = tmp_ch->next;  // just for safety

    if (tmp_ch->roomp == NULL || tmp_ch->getName().empty())
    {
      vlogf(LOG_BUG, "Error: character_list contains a bogus item, removing.");
      tmp_ch = temp;
      temp = temp->next;
    }

    if(!count--)
      break;

    if (tmp_ch->getPosition() == POSITION_DEAD) {
      assert(tmp_ch); // I suppose this guy exists if we have already dereferenced it
      // even if shortDescr is NULL.
      vlogf(LOG_BUG, format("Error: dead creature (%s at %d) in character_list, removing.") % 
          tmp_ch->getName() % tmp_ch->in_room);
      delete tmp_ch;
      continue;
    }
    if ((tmp_ch->getPosition() < POSITION_STUNNED) &&
        (tmp_ch->getHit() > 0)) {
      vlogf(LOG_BUG, format("Error: creature (%s) with hit > 0 found with position < stunned") % 
          tmp_ch->getName());
      vlogf(LOG_BUG, "Setting player to POSITION_STANDING");
      tmp_ch->setPosition(POSITION_STANDING);
    }


    for(TCharProcess *char_proc : char_procs) {
      if(char_proc->should_run(pulse.pulse)){
        if(char_proc->run(pulse, tmp_ch)){
          delete tmp_ch;
          break;
        }
      }
      if (!tmp_ch->roomp || tmp_ch->getName().empty()) {
        vlogf(LOG_BUG, format("Error: char %s roomp %p in proc %s") %
            tmp_ch->getName() % tmp_ch ->roomp % char_proc->name);
        tmp_ch = temp;
        temp = temp->next;
      }
    }
  }
}

void TScheduler::run(int pulseNum)
{
  pulse.init(pulseNum);

  // run general processes
  for(TProcess* proc : procs) {
    if(proc->should_run(pulse.pulse)){
      proc->run(pulse);
    }
  }

  pulse.init12(pulseNum);

  // run object processes
  runObj(pulseNum);

  // run character processes
  runChar(pulseNum);

  pulse.init(pulseNum);
}

procSeedRandom::procSeedRandom(const int &p)
{
  trigger_pulse=p;
  name="procSeedRandom";
}

void procSeedRandom::run(const TPulse &) const
{
  srand(time(0));
  vlogf(LOG_SILENT, "procSeedRandom: Generated new seed.");
}
