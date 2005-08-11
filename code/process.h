#ifndef __PROCESS_H
#define __PROCESS_H


// This is the template of a process.
class TProcess {
 public:
  int trigger_pulse;
  sstring name;

  // in general, you shouldn't have to override should_run()
  virtual bool should_run(int) const;

  virtual void run(int) const = 0;

  virtual ~TProcess(){}
};


//// processes

class procTrophyDecay: public TProcess {
 public:
  void run(int) const;
  procTrophyDecay(const int &);
};

class procTweakLoadRate : public TProcess {
 public:
  void run(int) const;
  procTweakLoadRate(const int &);
};

class procGlobalRoomStuff : public TProcess {
 public:
  void run(int) const;
  procGlobalRoomStuff(const int &);
};

class procDoRoomSaves : public TProcess {
 public:
  void run(int) const;
  procDoRoomSaves(const int &);
};


class procDeityCheck : public TProcess {
 public:
  void run(int) const;
  procDeityCheck(const int &);
};


class procApocCheck : public TProcess {
 public:
  void run(int) const;
  procApocCheck(const int &);
};

class procSaveFactions : public TProcess {
 public:
  void run(int) const;
  procSaveFactions(const int &);
};

class procSaveNewFactions : public TProcess {
 public:
  void run(int) const;
  procSaveNewFactions(const int &);
};

class procWeatherAndTime : public TProcess {
 public:
  void run(int) const;
  procWeatherAndTime(const int &);
};

class procWholistAndUsageLogs : public TProcess {
 public:
  void run(int) const;
  procWholistAndUsageLogs(const int &);
};

class procCheckForRepo : public TProcess {
 public:
  void run(int) const;
  procCheckForRepo(const int &);
};

class procCheckMail : public TProcess {
 public:
  void run(int) const;
  procCheckMail(const int &);
};

class procPerformViolence : public TProcess {
 public:
  void run(int) const;
  procPerformViolence(const int &);
};

class procFishRespawning : public TProcess {
 public:
  void run(int) const;
  procFishRespawning(const int &);
};

class procReforestation : public TProcess {
 public:
  void run(int) const;
  procReforestation(const int &);
};

class procZoneUpdate : public TProcess {
 public:
  void run(int) const;
  procZoneUpdate(const int &);
};

class procLaunchCaravans : public TProcess {
 public:
  void run(int) const;
  procLaunchCaravans(const int &);
};

class procUpdateAvgPlayers : public TProcess {
 public:
  void run(int) const;
  procUpdateAvgPlayers(const int &);
};

class procCheckGoldStats : public TProcess {
 public:
  void run(int) const;
  procCheckGoldStats(const int &);
};

class procAutoTips : public TProcess {
 public:
  void run(int) const;
  procAutoTips(const int &);
};

class procPingData : public TProcess {
 public:
  void run(int) const;
  procPingData(const int &);
};

class procUpdateAuction : public TProcess {
 public:
  void run(int) const;
  procUpdateAuction(const int &);
};

class procBankInterest : public TProcess {
 public:
  void run(int) const;
  procBankInterest(const int &);
};

class procRecalcFactionPower : public TProcess {
 public:
  void run(int) const;
  procRecalcFactionPower(const int &);
};

class procNukeInactiveMobs : public TProcess {
 public:
  void run(int) const;
  procNukeInactiveMobs(const int &);
};

class procUpdateTime : public TProcess {
 public:
  void run(int) const;
  procUpdateTime(const int &);
};

class procSetZoneEmpty : public TProcess {
 public:
  void run(int) const;
  procSetZoneEmpty(const int &);
};

class procCallRoomSpec : public TProcess {
 public:
  void run(int) const;
  procCallRoomSpec(const int &);
};

class procMobHate : public TProcess {
 public:
  void run(int) const;
  procMobHate(const int &);
};

class procDoComponents : public TProcess {
 public:
  void run(int) const;
  procDoComponents(const int &);
};

/////////////////////

class TScheduler {
  vector<TProcess *>procs;

 public:
  void add(TProcess *);
  void run(int);
};


#endif
