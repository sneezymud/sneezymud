#include "stdsneezy.h"
#include "process.h"
#include "timing.h"
#include "database.h"
#include "shop.h"

///////////

// procFactoryProduction
procFactoryProduction::procFactoryProduction(const int &p)
{
  trigger_pulse=p;
  name="procFactoryProduction";
}

void procFactoryProduction::run(int pulse) const
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
  save_guilds();
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

void TScheduler::add(TProcess *p)
{
  procs.push_back(p);
}


// we have some legacy code here, in that many processes expect pulse
// to be mod 2400.  So we use the real pulse, but pass mod 2400.
void TScheduler::run(int pulse)
{
  TTiming timer;
  vector<TProcess *>::iterator iter;

  for(iter=procs.begin();iter!=procs.end();++iter){
    if((*iter)->should_run(pulse)){
      if(toggleInfo[TOG_GAMELOOP]->toggle)
	timer.start();

      (*iter)->run(pulse % 2400);
      
      if(toggleInfo[TOG_GAMELOOP]->toggle){
	timer.end();
	vlogf(LOG_MISC, fmt("%i %i) %s: %i") % 
	      (pulse % 2400) % (pulse%12) % (*iter)->name % 
	      (int)(timer.getElapsed()*1000000));
      }
    }
  }
}

procSeedRandom::procSeedRandom(const int &p)
{
  trigger_pulse=p;
  name="procSeedRandom";
}

void procSeedRandom::run(int) const
{
  srand(time(0));
  vlogf(LOG_SILENT, fmt("procSeedRandom: Generated new seed."));
}
