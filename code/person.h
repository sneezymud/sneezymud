//////////////////////////////////////////////////////////////////////////
//
// SneezyMUD - All rights reserved, SneezyMUD Coding Team
//
//////////////////////////////////////////////////////////////////////////


#ifndef __PERSON_H
#define __PERSON_H

#include "being.h"

extern Descriptor *descriptor_list;
extern Descriptor *next_to_process;

class TPerson : public TBeing {
  private:
    ush_int base_age;
    TPerson();  // made private to make it uncallable

  public:
    FILE *tLogFile;
    char lastHost[40];
    char *title; 
    int last_rent;
    bool toggles[MAX_TOG_INDEX];
    bool wizPowers[MAX_POWER_INDEX];
    byte timer;

    TPerson(Descriptor *);
    TPerson(const TPerson &);
    TPerson & operator=(const TPerson &);
    virtual ~TPerson();

    virtual const char *getName() const { return name; }

    void setBaseAge(ush_int num) {
      base_age = num;
    }
    virtual ush_int getBaseAge() const {
      return base_age;
    }
    virtual int getWait() const {
      if (desc && !isImmortal())
        return desc->wait;
      return 0;
    }
    virtual void setWait(int num) { 
      if (desc)
        desc->wait = num;
    }
    virtual byte getTimer() const {
      return timer;
    }
    virtual void setTimer(byte num) { 
      timer = num;
    }
    virtual void addToTimer(byte num) { 
      timer += num;
    }

    void storeToSt(charFile *);
    void autoDeath();
    void resetChar();
    int genericLoadPC();
    void loadFromSt(charFile *);
    void initChar();
    void doStart();
    void setDimensions();
    void setTitle(bool);
    void rentAffectTo(saveAffectedData *);
    void setSelectToggles(TBeing *, classIndT, silentTypeT);
    void advanceSelectDisciplines(TBeing *, classIndT, int, silentTypeT);
    void raiseLevel(classIndT, TMonster *);
    void doLevelSkillsLearn(TBeing *, discNumT, int, int);
    void setBaseAge();   // sets new base-age
    void saveRent(objCost *, bool, int);
    void loadRent();

    virtual void doUsers(const char *);
    virtual void doToggle(const char *);
    virtual void doInvis(const char *);
    virtual void doVisible(const char *, bool);
    virtual void doMedit(const char *);
    virtual void doSEdit(const char *);
    virtual void doOEdit(const char *);
    virtual void doEdit(const char *);
    virtual void doRload(const char *);
    virtual void doRsave(const char *);
    virtual void doRedit(const char *);
    virtual void doTrans(const char *);
    virtual int doAt(const char *, bool);
    virtual void doAccess(const char *);
    virtual void doSet(const char *);
    virtual void doLow(const char *);
    virtual void doShutdow();
    virtual void doShutdown(const char *);
    virtual void doSnoop(const char *);
    virtual void doSwitch(const char *);
    virtual void doForce(const char *);
    virtual void doLoad(const char *);
    virtual void doCutlink(const char *);
    virtual void doPurge(const char *);
    virtual void doNohassle(const char *);
    virtual void doStealth(const char *);
    virtual short int manaLimit() const;
    virtual short int hitLimit() const;
    virtual void setMaxHit(int);
    virtual int getMaxMove() const;
    virtual void reconcileHelp(TBeing *, double);
    virtual void reconcileHurt(TBeing *, double);
    virtual void doTitle(const char *);
    virtual void parseTitle(char *, Descriptor *);
    virtual void addToWait(int);
    virtual void doBug(const char *);
    virtual void doIdea(const char *);
    virtual void doTypo(const char *);
    virtual bool hasQuestBit(int) const;
    virtual void setQuestBit(int);
    virtual void remQuestBit(int);
    void saveToggles();
    void loadToggles();
    virtual bool hasWizPower(wizPowerT) const;
    virtual void setWizPower(wizPowerT);
    virtual void remWizPower(wizPowerT);
    void saveWizPowers();
    void loadWizPowers();
    virtual void doColor(const char *);
    virtual void doChange(const char *);
    virtual void doStat(const char *);
    virtual void doShow(const char *);
    virtual bool isPc() const { return TRUE; }
    virtual void logf(const char *, ...);
    virtual int manaGain();
    virtual int hitGain();
    virtual int doQuit2();
    virtual void wizFileSave();
    virtual bool isDragonRideable() const { return false; }
    virtual void failCharm(TBeing *);
    virtual int learnFromDoingUnusual(learnUnusualTypeT, spellNumT, int);
    virtual int learnFromDoing(spellNumT, silentTypeT, unsigned int);
    virtual immortalTypeT isImmortal(int level = GOD_LEVEL1) const;
    void dropItemsToRoom(safeTypeT, dropNukeT);
    virtual TThing& operator+= (TThing& t);
    virtual void doPowers(const char *) const;
    void startLevels();
    void advanceLevel(classIndT, TMonster *);
    void doHPGainForLev(classIndT);
};

#endif
