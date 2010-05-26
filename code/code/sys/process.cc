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

TProcTop::TProcTop(){
  if((shmid=shmget(gamePort, shm_size, IPC_CREAT | 0666)) < 0){
    vlogf(LOG_BUG, "failed to get shared memory segment in TScheduler()");
    shm=NULL;
  } else if((shm = (char *)shmat(shmid, NULL, 0)) == (char *) -1){
    vlogf(LOG_BUG, "failed to attach shared memory segment in TScheduler()");
    shm=NULL;
  }  
}

void TProcTop::clear(){
  if(shm){
    memset(shm, 0, shm_size);
    shm_ptr=shm;
    added.clear();
  }
}

void TProcTop::add(const sstring &s){
  if(shm && 
     ((unsigned int)(shm_size-(shm_ptr-shm)) > (s.length()+1)) &&
     (added.find(s)==added.end())){
    strcpy(shm_ptr, s.c_str());
    shm_ptr+=s.length()+1;
    added[s]=true;
  }
}

TScheduler::TScheduler(){
  pulse.init(0);
  placeholder=read_object(42, VIRTUAL);
  // don't think we can recover from this
  mud_assert(placeholder!=NULL, "couldn't load placeholder object");
  *(real_roomp(0)) += *placeholder;
  objIter=find(object_list.begin(), object_list.end(), placeholder);
  tmp_ch=NULL;
}

void TScheduler::runObj(int pulseNum)
{
  int count;
  TObj *obj;
  TTiming timer;

  // we want to go through 1/12th of the object list every pulse
  // obviously the object count will change, so this is approximate.
  count=(int)((float)objCount/11.5);

  if(toggleInfo[TOG_GAMELOOP]->toggle){
    for(std::vector<TObjProcess *>::iterator iter=obj_procs.begin();
	iter!=obj_procs.end();++iter)
      (*iter)->timing=0;
  }

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

    for(std::vector<TObjProcess *>::iterator iter=obj_procs.begin();
	iter!=obj_procs.end();++iter){
      if((*iter)->should_run(pulse.pulse)){
	if(toggleInfo[TOG_GAMELOOP]->toggle)
	  timer.start();
	top.add((*iter)->name);
      
	if((*iter)->run(pulse, obj)){
	  delete obj;

	  if(toggleInfo[TOG_GAMELOOP]->toggle)
	    (*iter)->timing+=timer.getElapsed();

	  break;
	}
	
	if(toggleInfo[TOG_GAMELOOP]->toggle)
	  (*iter)->timing+=timer.getElapsed();
      }
    }
  }

  if(toggleInfo[TOG_GAMELOOP]->toggle){
    for(std::vector<TObjProcess *>::iterator iter=obj_procs.begin();
	iter!=obj_procs.end();++iter){
      if((*iter)->should_run(pulse.pulse)){
	vlogf(LOG_MISC, format("%i %i) %s: %i") % 
	      (pulseNum % 2400) % (pulseNum%12) % (*iter)->name % 
	      (int)((*iter)->timing*1000000));
	(*iter)->timing=0;
      }
    }
  }
}

void TScheduler::runChar(int pulseNum)
{
  TBeing *temp;
  int count;
  TTiming timer;

  // we've already finished going through the character list, so start over
  if(!tmp_ch)
    tmp_ch=character_list;

  count=max((int)((float)mobCount/11.5), 1);

  if(toggleInfo[TOG_GAMELOOP]->toggle)
    for(std::vector<TCharProcess *>::iterator iter=char_procs.begin();
	iter!=char_procs.end();++iter)
      (*iter)->timing=0;
    

  for (; tmp_ch; tmp_ch = temp) {
    temp = tmp_ch->next;  // just for safety

    if(!count--)
      break;

    if (tmp_ch->getPosition() == POSITION_DEAD) {
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

    
    for(std::vector<TCharProcess *>::iterator iter=char_procs.begin();
	iter!=char_procs.end();++iter){
      if((*iter)->should_run(pulse.pulse)){
	if(toggleInfo[TOG_GAMELOOP]->toggle)
	  timer.start();
	top.add((*iter)->name);
	      
	if((*iter)->run(pulse, tmp_ch)){
	  delete tmp_ch;

	  if(toggleInfo[TOG_GAMELOOP]->toggle)
	    (*iter)->timing+=timer.getElapsed();

	  break;
	}
	
	if(toggleInfo[TOG_GAMELOOP]->toggle)
	  (*iter)->timing+=timer.getElapsed();
      }
    }
    temp = tmp_ch->next;  // just for safety
  }

  if(toggleInfo[TOG_GAMELOOP]->toggle){
    for(std::vector<TCharProcess *>::iterator iter=char_procs.begin();
	iter!=char_procs.end();++iter){
      if((*iter)->should_run(pulse.pulse)){
	vlogf(LOG_MISC, format("%i %i) %s: %i") % 
	      (pulseNum % 2400) % (pulseNum%12) % (*iter)->name % 
	      (int)((*iter)->timing*1000000));
      }
    }
  }

}

void TScheduler::run(int pulseNum)
{
  TTiming timer;

  pulse.init(pulseNum);
  top.clear();

  if(toggleInfo[TOG_GAMELOOP]->toggle)
    vlogf(LOG_MISC, format("%i %i) pulses: %s") % 
	  pulse.pulse % (pulse.pulse%12) % pulse.showPulses());


  // run general processes
  for(std::vector<TProcess *>::iterator iter=procs.begin();
      iter!=procs.end();++iter){
    if((*iter)->should_run(pulse.pulse)){
      if(toggleInfo[TOG_GAMELOOP]->toggle)
	timer.start();
	top.add((*iter)->name);

      (*iter)->run(pulse);

      if(toggleInfo[TOG_GAMELOOP]->toggle){
	timer.end();
	vlogf(LOG_MISC, format("%i %i) %s: %i") % 
	      (pulseNum % 2400) % (pulseNum%12) % (*iter)->name % 
	      (int)(timer.getElapsed()*1000000));
      }
    }
  }

  pulse.init12(pulseNum);

  if(toggleInfo[TOG_GAMELOOP]->toggle)
    vlogf(LOG_MISC, format("%i %i) distributed pulses: %s") % 
	  pulse.pulse % (pulse.pulse%12) % pulse.showPulses());

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
