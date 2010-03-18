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

TScheduler::TScheduler(){
  pulse.init(0);
}

// we have some legacy code here, in that many processes expect pulse
// to be mod 2400.  So we use the real pulse, but pass mod 2400.
void TScheduler::run(int pulseNum)
{
  TTiming timer;
  std::vector<TProcess *>::iterator iter;

  pulse.init(pulseNum);

  for(iter=procs.begin();iter!=procs.end();++iter){
    if((*iter)->should_run(pulseNum)){
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
