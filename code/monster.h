//////////////////////////////////////////////////////////////////////////
//
// SneezyMUD - All rights reserved, SneezyMUD Coding Team
//
//////////////////////////////////////////////////////////////////////////


#ifndef __MONSTER_H
#define __MONSTER_H

#include "being.h"
#include "paths.h"

enum zoneHateT {
     OP_SEX      = 1,
     OP_RACE,
     OP_CHAR,
     OP_CLASS,
     OP_UNUSED,
     OP_UNUSED2,
     OP_VNUM,
     MAX_HATE
};
const zoneHateT MIN_HATE = OP_SEX;

class charList {
  public:
  const char *name;
  long iHateStrength;
  int account_id;
  int player_id;
  
  charList *next;

  charList();
  charList(const charList &a);
  charList & operator=(const charList &a);
  ~charList();
};

class opinionData {
  public:
    charList *clist;
    sexTypeT sex;
    race_t race;
    byte Class;
    short vnum;

  opinionData();
  opinionData(const opinionData &a);
  opinionData & operator=(const opinionData &a);
  ~opinionData();
};

class Mobile_Attitude {
  friend class TMonster;

  private:
    ubyte suspicion;          /* current suspicion value */
    ubyte greed;              /* current greed value */
    ubyte malice;             /* current malice value */
    ubyte anger;              /* current anger value */
  
    ubyte def_suspicion;      /* the various default values */
    ubyte def_greed;
    ubyte def_malice;
    ubyte def_anger;
  
    TBeing *target;             /* the target of the mobs opinions */
  public:
    TBeing *random;             // used to store a tbeing for random things
    int last_cmd;
  
    Mobile_Attitude();
    Mobile_Attitude(const Mobile_Attitude &a);
    Mobile_Attitude & operator=(const Mobile_Attitude &a);
    ~Mobile_Attitude();
};

class TMonster : public TBeing {
  public:
    Responses *resps;
    Mobile_Attitude opinion; 
    opinionData hates;
    opinionData fears;
    sh_int persist;
    int oldRoom;
    int brtRoom;
    unsigned short hatefield;
    unsigned short fearfield;
    ubyte moneyConst;

    const char *sounds;
    const char *distantSnds;
    float hpLevel;
    float damLevel;
    ubyte damPrecision;
    float acLevel;
    sstring procData;

    positionTypeT default_pos;

    TMonster();
    TMonster(const TMonster &);
    TMonster & operator=(const TMonster &);
    virtual ~TMonster();

    // VIRTUAL FUNCTIONS
    virtual double baseDamage() const;
    virtual int grenadeHit(TTrap *);
    virtual int checkSpec(TBeing *, cmdTypeT, const char *, TThing *);
    virtual int mobVnum() const;

    virtual const char *getName() const { return (shortDescr); }
    // END VIRTUAL FUNCTIONS

    // pets.cc
    virtual bool restorePetToPc(TBeing *);

    int greed() const {
      return opinion.greed;
    }
    int anger() const {
      return opinion.anger;
    }
    int malice() const {
      return opinion.malice;
    }
    int susp() const {
      return opinion.suspicion;
    }
    void setGreed(int res) {
      opinion.greed = res;
    }
    void setAnger(int res) {
      opinion.anger = res;
    }
    void setMalice(int res) {
      opinion.malice = res;
    }
    void setSusp(int res) {
      opinion.suspicion = res;
    }
    int defgreed() const {
      return opinion.def_greed;
    }
    int defanger() const {
      return opinion.def_anger;
    }
    int defmalice() const {
      return opinion.def_malice;
    }
    int defsusp() const {
      return opinion.def_suspicion;
    }
    void setDefGreed(int res) {
      opinion.def_greed = res;
    }
    void setDefAnger(int res) {
      opinion.def_anger = res;
    }
    void setDefMalice(int res) {
      opinion.def_malice = res;
    }
    void setDefSusp(int res) {
      opinion.def_suspicion = res;
    }
    TBeing * targ() const {
      return opinion.target;
    }
    void setTarg(TBeing *ch) {
      opinion.target = ch;
    }
    bool isGreedy() const {
      return (::number(0,101) < greed());
    }
    bool isAngry() const {
      return (::number(0,101) < anger());
    }
    bool isMalice() const {
      return (::number(0,101) < malice());
    }
    bool isSusp() const {
      return (::number(0,101) < susp());
    }
    void US(int num) {
      setSusp(susp() + min(::number(0,2*num),100 - susp()));
    }
    void DS(int num) {
      setSusp(susp() - min(::number(0,2*num),susp()));
    }
    void UG(int num) {
      setGreed(greed() + min(::number(0,2*num),100 - greed()));
    }
    void DG(int num) {
      setGreed(greed() - min(::number(0,2*num),greed()));
    }
    void UA(int num) {
      setAnger(anger() + min(::number(0,2*num),100 - anger()));
    }
    void DA(int num) {
      setAnger(anger() - min(::number(0,2*num),anger()));
    }
    void UM(int num) {
      setMalice(malice() + min(::number(0,2*num),100 - malice()));
    }
    void DMal(int num) {
      setMalice(malice() - min(::number(0,2*num),malice()));
    }

    void setACFromACLevel();
    void setHPFromHPLevel();
    virtual void raiseLevel(classIndT);

    float getACLevel() const;
    void setACLevel(float);
    float getHPLevel() const;
    void setHPLevel(float);
    float getDamLevel() const;
    void setDamLevel(float);
    ubyte getDamPrecision() const;
    void setDamPrecision(ubyte);
    int getMobDamage() const;

    int aiSocialSwitch(TBeing *, TBeing *, cmdTypeT, aiTarg);
    int npcSteal(TPerson *);
    void checkMobStats(tinyfileTypeT);
    int mobileActivity(int);
    int notFightingMove(int);
    int defendOther(TBeing &);
    int defendSelf(int);
    int pissed();
    int aggro();
    int aggroCheck(bool);
    int factionAggroCheck();
    int fearCheck(const TBeing *, bool);
    int aiUglyMug(TBeing *);
    void aiTarget(TBeing *);
    int aiShutUp(TBeing *);
    int aiOtherInsulted(TBeing *, TBeing *);
    int aiLoveSelf(TBeing *);
    int aiAccuseAvoid(TBeing *);
    int aiGrinnedAt(TBeing *);
    int aiStrangeThings(TBeing *);
    int aiInsultDoer(TBeing *);
    int aiRudeNoises(TBeing *);
    int aiBadManners(TBeing *);
    int aiUpset(TBeing *);
    int aiRudeNoise(TBeing *);
    void aiHorse(TBeing *);
    void aiMobCreation();
    void aiMaintainCalm();
    int aiMobShock(TBeing *);
    int aiMobHappy(TBeing *);
    int aiMudSexRepulsed(TBeing *);
    int aiMudSex(TBeing *);
    int aiMudSexOther(TBeing *, TBeing *);
    int aiFag(TBeing *, int);
    int aiSay(TBeing *, char*);
    void aiLook(TBeing *);
    int aiWimpCheck(TBeing *);
    int aiWimpSwitch(TBeing *);
    int aiSteal(TBeing *);
    int aiToastedAt(TBeing *);
    int aiBounce(TBeing *, TBeing *, aiTarg);
    int aiBlink(TBeing *, TBeing *, aiTarg);
    int aiDance(TBeing *, TBeing *, aiTarg);
    int aiSmile(TBeing *, TBeing *, aiTarg);
    int aiCackle(TBeing *, TBeing *, aiTarg);
    int aiLaugh(TBeing *, TBeing *, aiTarg);
    int aiGiggle(TBeing *, TBeing *, aiTarg);
    int aiShake(TBeing *, TBeing *, aiTarg);
    int aiPuke(TBeing *, TBeing *, aiTarg);
    int aiGrowl(TBeing *, TBeing *, aiTarg);
    int aiScream(TBeing *, TBeing *, aiTarg);
    int aiComfort(TBeing *, TBeing *, aiTarg);
    int aiNod(TBeing *, TBeing *, aiTarg);
    int aiSigh(TBeing *, TBeing *, aiTarg);
    int aiSulk(TBeing *, TBeing *, aiTarg);
    int aiHug(TBeing *, TBeing *, aiTarg);
    int aiSnuggle(TBeing *, TBeing *, aiTarg);
    int aiCuddle(TBeing *, TBeing *, aiTarg);
    int aiNuzzle(TBeing *, TBeing *, aiTarg);
    int aiCry(TBeing *, TBeing *, aiTarg);
    int aiPoke(TBeing *, TBeing *, aiTarg);
    int aiAccuse(TBeing *, TBeing *, aiTarg);
    int aiGrin(TBeing *, TBeing *, aiTarg);
    int aiBow(TBeing *, TBeing *, aiTarg);
    int aiApplaud(TBeing *, TBeing *, aiTarg);
    int aiBlush(TBeing *, TBeing *, aiTarg);
    int aiBurp(TBeing *, TBeing *, aiTarg);
    int aiChuckle(TBeing *, TBeing *, aiTarg);
    int aiClapReact(TBeing *, TBeing *);
    int aiClap(TBeing *, TBeing *, aiTarg);
    int aiCough(TBeing *, TBeing *, aiTarg);
    int aiCurtsey(TBeing *, TBeing *, aiTarg);
    int aiFlip(TBeing *, TBeing *, aiTarg);
    int aiFart(TBeing *, TBeing *, aiTarg);
    int aiFondle(TBeing *, TBeing *, aiTarg);
    int aiFrown(TBeing *, TBeing *, aiTarg);
    int aiGasp(TBeing *, TBeing *, aiTarg);
    int aiGlare(TBeing *, TBeing *, aiTarg);
    int aiGroan(TBeing *, TBeing *, aiTarg);
    int aiHiccup(TBeing *, TBeing *, aiTarg);
    int aiLove(TBeing *, TBeing *, aiTarg);
    int aiLick(TBeing *, TBeing *, aiTarg);
    int aiGrope(TBeing *, TBeing *, aiTarg);
    int aiMoan(TBeing *, TBeing *, aiTarg);
    int aiNibble(TBeing *, TBeing *, aiTarg);
    int aiPout(TBeing *, TBeing *, aiTarg);
    int aiPurr(TBeing *, TBeing *, aiTarg);
    int aiRuffle(TBeing *, TBeing *, aiTarg);
    int aiShiver(TBeing *, TBeing *, aiTarg);
    int aiShrug(TBeing *, TBeing *, aiTarg);
    int aiSing(TBeing *, TBeing *, aiTarg);
    int aiSlap(TBeing *, TBeing *, aiTarg);
    int aiSmirk(TBeing *, TBeing *, aiTarg);
    int aiSnap(TBeing *, TBeing *, aiTarg);
    int aiSneeze(TBeing *, TBeing *, aiTarg);
    int aiSnicker(TBeing *, TBeing *, aiTarg);
    int aiSniff(TBeing *, TBeing *, aiTarg);
    int aiSnore(TBeing *, TBeing *, aiTarg);
    int aiSpit(TBeing *, TBeing *, aiTarg);
    int aiSqueeze(TBeing *, TBeing *, aiTarg);
    int aiStare(TBeing *, TBeing *, aiTarg);
    int aiStrut(TBeing *, TBeing *, aiTarg);
    int aiThank(TBeing *, TBeing *, aiTarg);
    int aiTwiddle(TBeing *, TBeing *, aiTarg);
    int aiWave(TBeing *, TBeing *, aiTarg);
    int aiWhistle(TBeing *, TBeing *, aiTarg);
    int aiWiggle(TBeing *, TBeing *, aiTarg);
    int aiWink(TBeing *, TBeing *, aiTarg);
    int aiYawn(TBeing *, TBeing *, aiTarg);
    int aiSnowball(TBeing *, TBeing *, aiTarg);
    int aiFrench(TBeing *, TBeing *, aiTarg);
    int aiComb(TBeing *, TBeing *, aiTarg);
    int aiMassage(TBeing *, TBeing *, aiTarg);
    int aiTickle(TBeing *, TBeing *, aiTarg);
    int aiPat(TBeing *, TBeing *, aiTarg);
    int aiCurse(TBeing *, TBeing *, aiTarg);
    int aiBeg(TBeing *, TBeing *, aiTarg);
    int aiBleed(TBeing *, TBeing *, aiTarg);
    int aiCringe(TBeing *, TBeing *, aiTarg);
    int aiDaydream(TBeing *, TBeing *, aiTarg);
    int aiFume(TBeing *, TBeing *, aiTarg);
    int aiGrovel(TBeing *, TBeing *, aiTarg);
    int aiHop(TBeing *, TBeing *, aiTarg);
    int aiNudge(TBeing *, TBeing *, aiTarg);
    int aiPeer(TBeing *, TBeing *, aiTarg);
    int aiPoint(TBeing *, TBeing *, aiTarg);
    int aiPonder(TBeing *, TBeing *, aiTarg);
    int aiPunch(TBeing *, TBeing *, aiTarg);
    int aiSnarl(TBeing *, TBeing *, aiTarg);
    int aiSpank(TBeing *, TBeing *, aiTarg);
    int aiSteam(TBeing *, TBeing *, aiTarg);
    int aiTackle(TBeing *, TBeing *, aiTarg);
    int aiTaunt(TBeing *, TBeing *, aiTarg);
    int aiWhine(TBeing *, TBeing *, aiTarg);
    int aiWorship(TBeing *, TBeing *, aiTarg);
    int aiWhap(TBeing *, TBeing *, aiTarg);
    int aiYodel(TBeing *, TBeing *, aiTarg);
    int aiThink(TBeing *, TBeing *, aiTarg);
    int aiBeam(TBeing *, TBeing *, aiTarg);
    int aiChortle(TBeing *, TBeing *, aiTarg);
    int aiBonk(TBeing *, TBeing *, aiTarg);
    int aiScold(TBeing *, TBeing *, aiTarg);
    int aiDrool(TBeing *, TBeing *, aiTarg);
    int aiRip(TBeing *, TBeing *, aiTarg);
    int aiStretch(TBeing *, TBeing *, aiTarg);
    int aiPimp(TBeing *, TBeing *, aiTarg);
    int aiBelittle(TBeing *, TBeing *, aiTarg);
    int aiPiledrive(TBeing *, TBeing *, aiTarg);
    int aiTap(TBeing *, TBeing *, aiTarg);
    int aiFlipoff(TBeing *, TBeing *, aiTarg);
    int aiMoon(TBeing *, TBeing *, aiTarg);
    int aiPinch(TBeing *, TBeing *, aiTarg);
    int aiBite(TBeing *, TBeing *, aiTarg);
    int aiKiss(TBeing *, TBeing *, aiTarg);
    int aiCheer(TBeing *, TBeing *, aiTarg);
    int aiWoo(TBeing *, TBeing *, aiTarg);
    int aiGrumble(TBeing *, TBeing *, aiTarg);
    int aiApologize(TBeing *, TBeing *, aiTarg);
    int aiAgree(TBeing *, TBeing *, aiTarg);
    int aiDisagree(TBeing *, TBeing *, aiTarg);
    int aiSpam(TBeing *, TBeing *, aiTarg);
    int aiRaise(TBeing *, TBeing *, aiTarg);
    int aiGreet(TBeing *, TBeing *, aiTarg);
    int aiTip(TBeing *, TBeing *, aiTarg);
    int aiRoll(TBeing *, TBeing *, aiTarg);
    int aiBop(TBeing *, TBeing *, aiTarg);
    int aiJump(TBeing *, TBeing *, aiTarg);
    int aiWhimper(TBeing *, TBeing *, aiTarg);
    int aiSneer(TBeing *, TBeing *, aiTarg);
    int aiMoo(TBeing *, TBeing *, aiTarg);
    int aiBoggle(TBeing *, TBeing *, aiTarg);
    int aiSnort(TBeing *, TBeing *, aiTarg);
    int aiTango(TBeing *, TBeing *, aiTarg);
    int aiRoar(TBeing *, TBeing *, aiTarg);
    int aiFlex(TBeing *, TBeing *, aiTarg);
    int aiTug(TBeing *, TBeing *, aiTarg);
    int aiCross(TBeing *, TBeing *, aiTarg);
    int aiHowl(TBeing *, TBeing *, aiTarg);
    int aiGrunt(TBeing *, TBeing *, aiTarg);
    int aiWedgie(TBeing *, TBeing *, aiTarg);
    int aiScuff(TBeing *, TBeing *, aiTarg);
    int aiNoogie(TBeing *, TBeing *, aiTarg);
    int aiBrandish(TBeing *, TBeing *, aiTarg);
    int aiTrip(TBeing *, TBeing *, aiTarg);
    int aiDuck(TBeing *, TBeing *, aiTarg);
    int aiBeckon(TBeing *, TBeing *, aiTarg);
    int aiWince(TBeing *, TBeing *, aiTarg);
    int aiFaint(TBeing *, TBeing *, aiTarg);
    int aiHum(TBeing *, TBeing *, aiTarg);
    int aiRazz(TBeing *, TBeing *, aiTarg);
    int aiGag(TBeing *, TBeing *, aiTarg);
    int aiAvert(TBeing *, TBeing *, aiTarg);
    int aiSalute(TBeing *, TBeing *, aiTarg);
    int aiPet(TBeing *, TBeing *, aiTarg);
    int aiGrimace(TBeing *, TBeing *, aiTarg);
    int aiScruff(TBeing *, TBeing *, aiTarg);
    int aiToast(TBeing *, TBeing *, aiTarg);
    int aiShoveReact(TBeing *, bool, dirTypeT);
    int findABetterWeapon();
    bool isRetrainable();
    bool isFriend(TBeing &);
    void mobAI();
    int standUp();
    void elementalFix(TBeing *, spellNumT, bool);
    void genericCharmFix();
    void genericPetFix();
    int findMyHorse();
    int classStuff(TBeing &);
    int hunt();
    int assistFriend();
    int mobileGuardian();
    int takeFirstHit(TBeing &);
    int targetFound();
    int mobileWander(dirTypeT);
    int superScavenger();
    int remove();
    int senseWimps();
    int monkMove(TBeing &);
    int shamanMove(TBeing &);
    int thiefMove(TBeing &);
    int deikhanMove(TBeing &);
    int mageMove(TBeing &);
    int rangMove(TBeing &);
    int clerMove(TBeing &);
    int fighterMove(TBeing &);
    int scavenge();
    int charmeeStuff();
    int protectionStuff();
    int randomHunt();
    int modifiedDoCommand(cmdTypeT, const sstring &, TBeing *, const resp *);
    sstring parseResponse(TBeing *, const char *);
    int remHated(const TBeing *, const char *);
    int remFeared(const TBeing *, const char *);
    int remHatred(unsigned short);
	bool multiHates(const TBeing *, bool);
    bool Hates(const TBeing *, const char *) const;
    bool Fears(const TBeing *, const char *) const;
    TBeing *findAHatee();
    TBeing *findAFearee();
    virtual void setHunting(TBeing *);
    virtual bool addHated(TBeing *);
    void developHatred(TBeing *);
    int addHatred(zoneHateT, int);
    int addFeared(TBeing *);
    int addFears(zoneHateT, int);
    void autoCreateShop(int);
    void makeNoise();
    virtual bool isRideable() const;
    virtual bool isNocturnal() const;
    virtual bool isDiurnal() const;
    virtual bool isPolice() const;
    virtual bool isShopkeeper() const;
    walkPathT walk_path(const path_struct *, int &);

    virtual int getWait() const {
      return wait;
    }
    virtual void setWait(int amt) {
      wait = amt;
    }
    virtual void addToWait(int amt);
    int readMobFromDB(int, bool, TBeing * = NULL);
    int getMobIndex() const { return (number < 0 ? 0 : number); }
    bool isUnique() {
      return (!mob_index[getMobIndex()].getNumber());
    }
    virtual void purgeMe(TBeing *);
    void loadResponses(int, const sstring & = "");
    bool checkResponsesPossible(cmdTypeT, const sstring &, TBeing *);
    int  checkResponses(TBeing*, TThing *, const sstring &, cmdTypeT);
    int  checkResponsesReal(TBeing*, TThing *, const sstring &, cmdTypeT);
    resp *readCommand(istringstream &);
    void createWealth();
    int dynamicComponentLoader(spellNumT, int);
    void thiefLootLoader();
    void mageComponentLoader();
    void rangerComponentLoader();
    void shamanComponentLoader();
    void clericSymbolLoader();
    void clericHolyWaterLoader();
    void buffMobLoader();
    void genericMobLoader(TOpenContainer **);
    virtual int hitGain();
    virtual int manaGain();
    virtual int rawKill(spellNumT, TBeing * = NULL, float = 0);
    virtual int doQuit2();
    virtual void wizFileSave();
    virtual bool isDragonRideable() const;
    virtual void failCharm(TBeing *);
    virtual int learnFromDoingUnusual(learnUnusualTypeT, spellNumT, int);
    virtual int learnFromDoing(spellNumT, silentTypeT, unsigned int);
    int doHatefulStuff();
    virtual unsigned int getTimer() const;
    virtual void setTimer(unsigned int);
    virtual void addToTimer(unsigned int);
    int petPrice() const;
    void aiGrowl(const TBeing *) const;
    void swapToStrung();
    double determineExp();
    int calculateGoldFromConstant();
    bool isTestmob() const;
    int wanderAround();
    void quickieDefend();
    void saveItems(const sstring &);
    void saveItems(int);
    int saveItem(int, TObj *, int=0);
    void loadItems(const sstring &);
    TObj *loadItem(int, int);
    void deleteItem(int, int);
    bool isSmartMob(int) const;
    bool aiLoveNonHumanoid(TBeing *, aiTarg);
    double getRealLevel() const;
    int lookForHorse();
    void balanceMakeNPCLikePC();
    int lookForEngaged(const TBeing *);
    bool isAttackerMultiplay(TBeing *aggressor);
};

#endif  // inclusion sandwich
