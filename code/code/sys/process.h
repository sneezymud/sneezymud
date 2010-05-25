#ifndef __PROCESS_H
#define __PROCESS_H

#include "sstring.h"
#include "comm.h"
#include "obj.h"
#include "timing.h"

class TPulse {
public:  
  int pulse;
  bool every, teleport, combat, drowning, special_procs, update_stuff;
  bool pulse_mudhour, mobstuff, pulse_tick, wayslowpulse;

  sstring showPulses(){
    sstring buf;

    buf += (every?"every ":"");
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
    every = !(pulse % PULSE_EVERY);
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
    every=a.every;
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
class TBaseProcess {
 public:
  int trigger_pulse;
  sstring name;

  // in general, you shouldn't have to override should_run()
  virtual bool should_run(int) const;
  
  virtual ~TBaseProcess(){}
};

class TProcess : public TBaseProcess {
 public:
  virtual void run(const TPulse &) const = 0;

};

class TObjProcess : public TBaseProcess {
  double timing;
  
 public:
  friend class TScheduler;

  // returns true if object is to be deleted
  virtual bool run(const TPulse &, TObj *) const = 0;
};

class TCharProcess : public TBaseProcess {
  double timing;

 public:
  friend class TScheduler;

  // returns true if character is to be deleted
  virtual bool run(const TPulse &, TBeing *) const = 0;
};

//// processes
class procCharSpecProcs : public TCharProcess {
 public:
  bool run(const TPulse &, TBeing *) const;
  procCharSpecProcs(const int &);
};

class procCharDrowning : public TCharProcess {
 public:
  bool run(const TPulse &, TBeing *) const;
  procCharDrowning(const int &);
};

class procCharResponses : public TCharProcess {
 public:
  bool run(const TPulse &, TBeing *) const;
  procCharResponses(const int &);
};

class procCharSinking : public TCharProcess {
 public:
  bool run(const TPulse &, TBeing *) const;
  procCharSinking(const int &);
};

class procCharFalling : public TCharProcess {
 public:
  bool run(const TPulse &, TBeing *) const;
  procCharFalling(const int &);
};

class procCharMobileActivity : public TCharProcess {
 public:
  bool run(const TPulse &, TBeing *) const;
  procCharMobileActivity(const int &);
};

class procCharTasks : public TCharProcess {
 public:
  bool run(const TPulse &, TBeing *) const;
  procCharTasks(const int &);
};

class procCharImmLeash : public TCharProcess {
 public:
  bool run(const TPulse &, TBeing *) const;
  procCharImmLeash(const int &);
};

class procCharSpellTask : public TCharProcess {
 public:
  bool run(const TPulse &, TBeing *) const;
  procCharSpellTask(const int &);
};

class procCharAffects : public TCharProcess {
 public:
  bool run(const TPulse &, TBeing *) const;
  procCharAffects(const int &);
};

class procCharRegen : public TCharProcess {
 public:
  bool run(const TPulse &, TBeing *) const;
  procCharRegen(const int &);
};

class procCharCantHit : public TCharProcess {
 public:
  bool run(const TPulse &, TBeing *) const;
  procCharCantHit(const int &);
};

class procCharRiverFlow : public TCharProcess {
 public:
  bool run(const TPulse &, TBeing *) const;
  procCharRiverFlow(const int &);
};

class procCharTeleportRoom : public TCharProcess {
 public:
  bool run(const TPulse &, TBeing *) const;
  procCharTeleportRoom(const int &);
};

class procCharNoise : public TCharProcess {
 public:
  bool run(const TPulse &, TBeing *) const;
  procCharNoise(const int &);
};

class procCharHalfTickUpdate : public TCharProcess {
 public:
  bool run(const TPulse &, TBeing *) const;
  procCharHalfTickUpdate(const int &);
};

class procCharTickUpdate : public TCharProcess {
 public:
  bool run(const TPulse &, TBeing *) const;
  procCharTickUpdate(const int &);
};

class procCharThaw : public TCharProcess {
 public:
  bool run(const TPulse &, TBeing *) const;
  procCharThaw(const int &);
};

class procCharLightning : public TCharProcess {
 public:
  bool run(const TPulse &, TBeing *) const;
  procCharLightning(const int &);
};

class procCharNutrition : public TCharProcess {
 public:
  bool run(const TPulse &, TBeing *) const;
  procCharNutrition(const int &);
};

class procCharLycanthropy : public TCharProcess {
 public:
  bool run(const TPulse &, TBeing *) const;
  procCharLycanthropy(const int &);
};

class procCharSpecProcsQuick : public TCharProcess {
 public:
  bool run(const TPulse &, TBeing *) const;
  procCharSpecProcsQuick(const int &);
};

class procCharScreenUpdate : public TCharProcess {
 public:
  bool run(const TPulse &, TBeing *) const;
  procCharScreenUpdate(const int &);
};

class procCharVampireBurn : public TCharProcess {
 public:
  bool run(const TPulse &, TBeing *) const;
  procCharVampireBurn(const int &);
};

class procObjVehicle : public TObjProcess {
 public:
  bool run(const TPulse &, TObj *) const;
  procObjVehicle(const int &);
};

class procObjDetonateGrenades : public TObjProcess {
 public:
  bool run(const TPulse &, TObj *) const;
  procObjDetonateGrenades(const int &);
};

class procObjFalling : public TObjProcess {
 public:
  bool run(const TPulse &, TObj *) const;
  procObjFalling(const int &);
};

class procObjRiverFlow : public TObjProcess {
 public:
  bool run(const TPulse &, TObj *) const;
  procObjRiverFlow(const int &);
};

class procObjTeleportRoom : public TObjProcess {
 public:
  bool run(const TPulse &, TObj *) const;
  procObjTeleportRoom(const int &);
};

class procObjSpecProcsQuick : public TObjProcess {
 public:
  bool run(const TPulse &, TObj *) const;
  procObjSpecProcsQuick(const int &);
};


class procObjTickUpdate : public TObjProcess {
 public:
  bool run(const TPulse &, TObj *) const;
  procObjTickUpdate(const int &);
};

class procObjBurning : public TObjProcess {
 public:
  bool run(const TPulse &, TObj *) const;
  procObjBurning(const int &);
};

class procObjSinking : public TObjProcess {
 public:
  bool run(const TPulse &, TObj *) const;
  procObjSinking(const int &);
};

class procObjSpecProcs : public TObjProcess {
 public:
  bool run(const TPulse &, TObj *) const;
  procObjSpecProcs(const int &);
};

class procObjSmoke : public TObjProcess {
 public:
  bool run(const TPulse &, TObj *) const;
  procObjSmoke(const int &);
};

class procObjPools : public TObjProcess {
 public:
  bool run(const TPulse &, TObj *) const;
  procObjPools(const int &);
};

class procObjTrash : public TObjProcess {
 public:
  bool run(const TPulse &, TObj *) const;
  procObjTrash(const int &);
};

class procObjRust : public TObjProcess {
 public:
  bool run(const TPulse &, TObj *) const;
  procObjRust(const int &);
};

class procObjFreezing : public TObjProcess {
 public:
  bool run(const TPulse &, TObj *) const;
  procObjFreezing(const int &);
};

class procObjAutoPlant : public TObjProcess {
 public:
  bool run(const TPulse &, TObj *) const;
  procObjAutoPlant(const int &);
};

class procDoubleXP : public TProcess {
 public:
  void run(const TPulse &) const;
  procDoubleXP(const int &);
};

class procIdle : public TProcess {
 public:
  void run(const TPulse &) const;
  procIdle(const int &);
};

class procHandleTimeAndSockets : public TProcess {
 public:
  void run(const TPulse &) const;
  procHandleTimeAndSockets(const int &);
};

class procLagInfo : public TProcess {
 public:
  void run(const TPulse &) const;
  procLagInfo(const int &);
};

class procCheckTask : public TProcess {
 public:
  void run(const TPulse &) const;
  procCheckTask(const int &);
};

class procRoomPulse : public TProcess {
 public:
  void run(const TPulse &) const;
  procRoomPulse(const int &);
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


class TProcTop {
  std::map<sstring,bool>added;

  char *shm;
  char *shm_ptr;
  int shmid;

 public:
  const static int shm_size=1024;

  void clear();
  void add(const sstring &);

  TProcTop();
};


class TScheduler {
  // process lists
  std::vector<TProcess *>procs;
  std::vector<TObjProcess *>obj_procs;
  std::vector<TCharProcess *>char_procs;

  // object process data
  TObj *placeholder;
  TObjIter objIter;

  // char process data
  TBeing *tmp_ch;

  TProcTop top;

  void runObj(int);
  void runChar(int);

 public:
  TPulse pulse;

  void add(TProcess *);
  void add(TObjProcess *);
  void add(TCharProcess *);
  void run(int);

  TScheduler();
};


#endif
