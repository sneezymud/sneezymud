#include "stdsneezy.h"
#include "process.h"
#include "timing.h"

///////////

// procSaveFactions
procSaveFactions::procSaveFactions(const int &p)
{
  trigger_pulse=p;
  name="procSaveFactions";
}

void procSaveFactions::run(int pulse) const
{
  save_factions();
}

// procSaveNewFactions
procSaveNewFactions::procSaveNewFactions(const int &p)
{
  trigger_pulse=p;
  name="procSaveNewFactions";
}

void procSaveNewFactions::run(int pulse) const
{
  save_newfactions();
}

// procDoComponents
procDoComponents::procDoComponents(const int &p)
{
  trigger_pulse=p;
  name="procDoComponents";
}

void procDoComponents::run(int pulse) const
{
  do_components(-1);
}


// procPerformViolence
procPerformViolence::procPerformViolence(const int &p)
{
  trigger_pulse=p;
  name="procPerformViolence";
}

void procPerformViolence::run(int pulse) const
{
  perform_violence(pulse);
}

///////

bool TProcess::should_run(int p) const
{
  if(!(p % trigger_pulse))
    return true;
  else
    return false;
}

void TProcessList::add(TProcess *p)
{
  procs.push_back(p);
}


void TProcessList::run(int pulse)
{
  TTiming timer;

  for(unsigned int i=0;i<procs.size();++i){
    if(procs[i]->should_run(pulse)){
      if(gameLoopTiming)
	timer.start();

      procs[i]->run(pulse);
      
      if(gameLoopTiming){
	timer.end();
	vlogf(LOG_MISC, fmt("%i %i) %s: %i") % 
	      pulse % (pulse%12) % procs[i]->name % 
	      (int)(timer.getElapsed()*1000000));
      }
    }
  }
}
