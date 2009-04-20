//////////////////////////////////////////////////////////////////////////
//
// SneezyMUD - All rights reserved, SneezyMUD Coding Team
//
//////////////////////////////////////////////////////////////////////////


#ifndef __PERSON_H
#define __PERSON_H

#include "being.h"
#include "wiz_powers.h"
#include "connect.h"

extern Descriptor *descriptor_list;
extern Descriptor *next_to_process;

class TPerson : public TBeing {
  private:
    unsigned short base_age;
    TPerson();  // made private to make it uncallable

  public:
    FILE *tLogFile;
    char lastHost[40];
    char *title; 
    int last_rent;
    //    bool toggles[MAX_TOG_INDEX];
    bool wizPowers[MAX_POWER_INDEX];
    bool wizPowersOriginal[MAX_POWER_INDEX];
    unsigned int timer;

    TPerson(Descriptor *);
    TPerson(const TPerson &);
    TPerson & operator=(const TPerson &);
    virtual ~TPerson();

    virtual const char *getName() const { return name; }

    void setBaseAge(unsigned short num) {
      base_age = num;
    }
    virtual unsigned short getBaseAge() const {
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
    virtual unsigned int getTimer() const {
      return timer;
    }
    virtual void setTimer(unsigned int num) { 
      timer = num;
    }
    virtual void addToTimer(unsigned int num) { 
      timer += num;
    }

    void storeToSt(charFile *);
    void autoDeath();
    void resetChar();
    int genericLoadPC();
    void loadFromSt(charFile *);
    void fixPracs();
    void initChar();
    void doStart();
    void setDimensions();
    void setTitle(bool);
    void rentAffectTo(saveAffectedData *);
    void setSelectToggles(TBeing *, classIndT, silentTypeT);
    void advanceSelectDisciplines(classIndT, int, silentTypeT);
    void doLevelSkillsLearn(discNumT, int, int);
    void setBaseAge();   // sets new base-age
    void saveRent(objCost *, bool, int);
    void loadRent();

    virtual void raiseLevel(classIndT);
    virtual void doUsers(const sstring &);
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
    virtual void doAccess(const sstring &);
    virtual void doSet(const char *);
    virtual void doLow(const sstring &);
    virtual void doShutdow();
    virtual void doShutdown(const char *);
    virtual void doSnoop(const char *);
    virtual void doSwitch(const char *);
    virtual void doForce(const char *);
    virtual void doLoad(const char *);
    virtual int doRent(const sstring &);
    virtual void doCutlink(const char *);
    virtual void doPurge(const char *);
    virtual short int manaLimit() const;
    virtual short int hitLimit() const;
    virtual void setMaxHit(int);
    virtual int getMaxMove() const;
    virtual void reconcileHelp(TBeing *, double);
    virtual void reconcileHurt(TBeing *, double);
    virtual void doTitle(const char *);
    virtual sstring parseTitle(Descriptor *);
    virtual void addToWait(int);
    virtual void doFeedback(const sstring &type, int clientCmd, const sstring &arg);
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
    virtual void doStat(const sstring &argument);
    virtual void doShow(const sstring &argument);
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
    virtual void doPowers(const sstring &) const;
    void startLevels();
    void advanceLevel(classIndT);
    void doHPGainForLev(classIndT);
    virtual void doBestow(const sstring &);
};

#endif
