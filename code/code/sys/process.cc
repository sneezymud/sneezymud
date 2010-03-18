#include "process.h"
#include "timing.h"
#include "database.h"
#include "shop.h"
#include "parse.h"
#include "faction.h"
#include "extern.h"
#include "toggle.h"
#include "guild.h"

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

TScheduler::TScheduler(){
  pulse.init(0);
  placeholder=read_object(42, VIRTUAL);
  // don't think we can recover from this
  mud_assert(placeholder!=NULL, "couldn't load placeholder object");
  *(real_roomp(0)) += *placeholder;
  objIter=find(object_list.begin(), object_list.end(), placeholder);
}

void TScheduler::run(int pulseNum)
{
  TTiming timer;

  pulse.init(pulseNum);

  for(std::vector<TProcess *>::iterator iter=procs.begin();
      iter!=procs.end();++iter){
    if((*iter)->should_run(pulse.pulse)){
      if(toggleInfo[TOG_GAMELOOP]->toggle)
	timer.start();

      if((*iter)->trigger_pulse == PULSE_EVERY_DISTRIBUTED)
	pulse.init12(pulseNum);

      (*iter)->run(pulse);

      if((*iter)->trigger_pulse == PULSE_EVERY_DISTRIBUTED)
	pulse.init(pulseNum);

      if(toggleInfo[TOG_GAMELOOP]->toggle){
	timer.end();
	vlogf(LOG_MISC, format("%i %i) %s: %i") % 
	      (pulseNum % 2400) % (pulseNum%12) % (*iter)->name % 
	      (int)(timer.getElapsed()*1000000));
      }
    }
  }

  pulse.init12(pulseNum);
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
    
    
    if (!dynamic_cast<TObj *>(obj)) {
      vlogf(LOG_BUG, format("Object_list produced a non-obj().  rm: %d") %
	    obj->in_room);
      vlogf(LOG_BUG, format("roomp %s, parent %s") %  
	    (obj->roomp ? "true" : "false") %
	    (obj->parent ? "true" : "false"));
      // bogus objects tend to have garbage in obj->next
      // it would be dangerous to continue with this loop
      // this is called often enough that one skipped iteration should
      // not be noticed.  Therefore, break out.
      break;
    }


    for(std::vector<TObjProcess *>::iterator iter=obj_procs.begin();
	iter!=obj_procs.end();++iter){
      if((*iter)->should_run(pulse.pulse)){
	if(toggleInfo[TOG_GAMELOOP]->toggle)
	  (*iter)->timer.start();
	      
	if((*iter)->run(pulse, obj)){
	  delete obj;

	  if(toggleInfo[TOG_GAMELOOP]->toggle)
	    (*iter)->timer.end();

	  break;
	}
	
	if(toggleInfo[TOG_GAMELOOP]->toggle)
	  (*iter)->timer.end();
      }
    }
  }

  if(toggleInfo[TOG_GAMELOOP]->toggle){
    for(std::vector<TObjProcess *>::iterator iter=obj_procs.begin();
	iter!=obj_procs.end();++iter){
      if((*iter)->should_run(pulse.pulse)){
	vlogf(LOG_MISC, format("%i %i) %s: %i") % 
	      (pulseNum % 2400) % (pulseNum%12) % (*iter)->name % 
	      (int)((*iter)->timer.getElapsedReset()*1000000));
      }
    }
  }
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
