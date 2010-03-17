#ifndef __PROCESS_H
#define __PROCESS_H

#include "sstring.h"
#include "comm.h"

class TPulse {
public:  
  int pulse;
  bool teleport, combat, drowning, special_procs, update_stuff;
  bool pulse_mudhour, mobstuff, pulse_tick, wayslowpulse;

  sstring showPulses(){
    sstring buf;

    buf += (teleport?"teleport ":"");
    buf += (combat?"combat ":"");
    buf += (drowning?"drowning ":"");
    buf += (special_procs?"special_procs ":"");
    buf += (update_stuff?"update_stuff ":"");
    buf += (pulse_mudhour?"pulse_mudhour ":"");
    buf += (mobstuff?"mobstuff ":"");
    buf += (pulse_tick?"pulse_tick ":"");
    buf += (wayslowpulse?"wayslowpulse ":"");

    return buf;
  }

  void init(int pulse){
    this->pulse=pulse;
    teleport = !(pulse % PULSE_TELEPORT);
    combat = !(pulse % PULSE_COMBAT);
    drowning = !(pulse % PULSE_DROWNING);
    special_procs = !(pulse % PULSE_SPEC_PROCS);
    update_stuff = !(pulse % PULSE_NOISES);
    pulse_mudhour = !(pulse % PULSE_MUDHOUR);
    mobstuff = !(pulse % PULSE_MOBACT);
    pulse_tick = !(pulse % PULSE_UPDATE);
    wayslowpulse = !(pulse % 2400);
  }

  // advances pulse to the next multiple of 12 and then initializes
  void init12(int pulse){
    while(pulse % 12)
      ++pulse;

    init(pulse);
  }

  TPulse & operator=(const TPulse &a){
    if (this == &a) return *this;    
    pulse=a.pulse;
    teleport=a.teleport;
    combat=a.combat;
    drowning=a.drowning;
    special_procs=a.special_procs;
    update_stuff=a.update_stuff;
    pulse_mudhour=a.pulse_mudhour;
    mobstuff=a.mobstuff;
    pulse_tick=a.pulse_tick;
    wayslowpulse=a.wayslowpulse;
    return *this;
  }
};


// This is the template of a process.
class TProcess {
 public:
  int trigger_pulse;
  sstring name;

  // in general, you shouldn't have to override should_run()
  virtual bool should_run(int) const;

  virtual void run(const TPulse &) const = 0;

  virtual ~TProcess(){}
};



//// processes

class procObjectPulse : public TProcess {
  TObj *placeholder;

 public:
  void run(const TPulse &) const;
  procObjectPulse(const int &);
};

class procWeightVolumeFumble : public TProcess {
 public:
  void run(const TPulse &) const;
  procWeightVolumeFumble(const int &);
};

class procFactoryProduction : public TProcess {
 public:
  void run(const TPulse &) const;
  procFactoryProduction(const int &);
};

class procRecordCommodPrices : public TProcess {
 public:
  void run(const TPulse &) const;
  procRecordCommodPrices(const int &);
};

class procCloseAccountingBooks : public TProcess {
 public:
  void run(const TPulse &) const;
  procCloseAccountingBooks(const int &);
};

class procCheckTriggerUsers : public TProcess {
 public:
  void run(const TPulse &) const;
  procCheckTriggerUsers(const int &);
};

class procTrophyDecay : public TProcess {
 public:
  void run(const TPulse &) const;
  procTrophyDecay(const int &);
};

class procSeedRandom : public TProcess {
 public:
  void run(const TPulse &) const;
  procSeedRandom(const int &);
};

class procTweakLoadRate : public TProcess {
 public:
  void run(const TPulse &) const;
  procTweakLoadRate(const int &);
};

class procGlobalRoomStuff : public TProcess {
 public:
  void run(const TPulse &) const;
  procGlobalRoomStuff(const int &);
};

class procQueryQueue : public TProcess {
 public:
  void run(const TPulse &) const;
  procQueryQueue(const int &);
};

class procDoPlayerSaves : public TProcess {
 public:
  void run(const TPulse &) const;
  procDoPlayerSaves(const int &);
};

class procDoRoomSaves : public TProcess {
 public:
  void run(const TPulse &) const;
  procDoRoomSaves(const int &);
};

class procDeityCheck : public TProcess {
 public:
  void run(const TPulse &) const;
  procDeityCheck(const int &);
};

class procApocCheck : public TProcess {
 public:
  void run(const TPulse &) const;
  procApocCheck(const int &);
};

class procSaveFactions : public TProcess {
 public:
  void run(const TPulse &) const;
  procSaveFactions(const int &);
};

class procSaveNewFactions : public TProcess {
 public:
  void run(const TPulse &) const;
  procSaveNewFactions(const int &);
};

class procWeatherAndTime : public TProcess {
 public:
  void run(const TPulse &) const;
  procWeatherAndTime(const int &);
};

class procWholistAndUsageLogs : public TProcess {
 public:
  void run(const TPulse &) const;
  procWholistAndUsageLogs(const int &);
};

class procCheckForRepo : public TProcess {
 public:
  void run(const TPulse &) const;
  procCheckForRepo(const int &);
};

class procCheckMail : public TProcess {
 public:
  void run(const TPulse &) const;
  procCheckMail(const int &);
};

class procPerformViolence : public TProcess {
 public:
  void run(const TPulse &) const;
  procPerformViolence(const int &);
};

class procFishRespawning : public TProcess {
 public:
  void run(const TPulse &) const;
  procFishRespawning(const int &);
};

class procReforestation : public TProcess {
 public:
  void run(const TPulse &) const;
  procReforestation(const int &);
};

class procZoneUpdate : public TProcess {
 public:
  void run(const TPulse &) const;
  procZoneUpdate(const int &);
};

class procLaunchCaravans : public TProcess {
 public:
  void run(const TPulse &) const;
  procLaunchCaravans(const int &);
};

class procUpdateAvgPlayers : public TProcess {
 public:
  void run(const TPulse &) const;
  procUpdateAvgPlayers(const int &);
};

class procCheckGoldStats : public TProcess {
 public:
  void run(const TPulse &) const;
  procCheckGoldStats(const int &);
};

class procAutoTips : public TProcess {
 public:
  void run(const TPulse &) const;
  procAutoTips(const int &);
};

class procPingData : public TProcess {
 public:
  void run(const TPulse &) const;
  procPingData(const int &);
};

class procUpdateAuction : public TProcess {
 public:
  void run(const TPulse &) const;
  procUpdateAuction(const int &);
};

class procBankInterest : public TProcess {
 public:
  void run(const TPulse &) const;
  procBankInterest(const int &);
};

class procRecalcFactionPower : public TProcess {
 public:
  void run(const TPulse &) const;
  procRecalcFactionPower(const int &);
};

class procNukeInactiveMobs : public TProcess {
 public:
  void run(const TPulse &) const;
  procNukeInactiveMobs(const int &);
};

class procUpdateTime : public TProcess {
 public:
  void run(const TPulse &) const;
  procUpdateTime(const int &);
};

class procSetZoneEmpty : public TProcess {
 public:
  void run(const TPulse &) const;
  procSetZoneEmpty(const int &);
};

class procCallRoomSpec : public TProcess {
 public:
  void run(const TPulse &) const;
  procCallRoomSpec(const int &);
};

class procMobHate : public TProcess {
 public:
  void run(const TPulse &) const;
  procMobHate(const int &);
};

class procDoComponents : public TProcess {
 public:
  void run(const TPulse &) const;
  procDoComponents(const int &);
};

/////////////////////



class TScheduler {
  std::vector<TProcess *>procs;

 public:
  TPulse pulse;

  void add(TProcess *);
  void run(int);
};


#endif
