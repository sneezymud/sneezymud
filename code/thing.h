//////////////////////////////////////////////////////////////////////////
//
// SneezyMUD - All rights reserved, SneezyMUD Coding Team
//
//////////////////////////////////////////////////////////////////////////


#ifndef __THING_H
#define __THING_H

#ifndef _STRING_H
#include <string.h>
#endif

enum thingTypeT {
  TYPETHING,    // = 0;
  TYPEOBJ,    //   = 1;
  TYPEMOB,    //   = 2;
  TYPEPC,    //    = 3;
  TYPEROOM,    //  = 4;
  TYPEBEING,    // = 5;
};

extern bool isname(const sstring &str, const sstring &namelist);
extern const sstring fname(const char *namelist);

class extraDescription {
  public:
    const char *keyword;
    const char *description;
    extraDescription *next;
    extraDescription();
    ~extraDescription();
    extraDescription & operator= (const extraDescription &a);
    extraDescription(const extraDescription &a);

    const char *findExtraDesc(const char *word);
};

class TThing {
  private:
    float weight;              // Weight in pounds
    sh_int light;
    ubyte material_type;
    float carried_weight;
    int carried_volume;
    TBeing * the_caster;
    TThing *stuff;           // The stuff inside me
  public:
    const char *descr;             // Description of thing
    const char *real_descr;        // used with disguise/polymorph
    TBeing *stuckIn;
    TThing *equippedBy;
    wearSlotT eq_pos;                 // what is the equip. pos?
    wearSlotT eq_stuck;
    void *act_ptr;

    TThing *getStuff() const { return stuff; }
    void setStuff(TThing *t){ stuff=t; }


    int max_exist;
    int in_room;
    int spec;
    int snum;                // Special variable used in builder manipulation
    int number;              // Number of thing
    int height;              // Height in centimeters
    byte canBeSeen;          
    const char *name;              // Name of thing
    const char *real_name;         // used with disguise/polymorph
    const char *shortDescr;
    const char *real_shortDescr;   // used with disguise/polymorph
    bool is_disguised;
    int disguise_level;
    int disguise_zone;
    TThing *parent;          // Room, Obj, Being etc. that I am inside of.
    TThing *nextThing;       // The next thing in the list I am inside of
    TThing *nextBorn;        // The next thing born in my room(mobiles)
    TRoom *roomp;
    Descriptor *desc;
    extraDescription *ex_description;  // extra descriptions 
    TThing *rider;          // thing on me
    TThing *riding;         // thing I am on
    TThing *nextRider;
    
  protected:
    TThing();
  public:
    TThing(const TThing &);
    virtual ~TThing();

    // Overloaded operators to take care of moving things to and from each other
    TThing& operator= (const TThing& t);
    virtual TThing& operator+= (TThing& t);
    virtual TThing& operator-- ();
    TThing& operator-- (int) {
      // postfix operator
      return --(*this);
    }
    
    // VIRTUAL FUNCTIONS
    virtual int editAverageMe(TBeing *, const char *);
    virtual int chiMe(TBeing *);
    virtual void eatMe(TBeing *);
    virtual const char *hshr() const { return "its"; }
    virtual const char *hssh() const { return "it"; }
    virtual const char *hmhr() const { return "it"; }
    virtual int checkSpec(TBeing *, cmdTypeT, const char *, TThing *) { return 0; }
    virtual const char *getName() const { return name; }
    virtual bool shouldntBeShown(wearSlotT) const;
    virtual int checkFalling() { return 0;}
    virtual float carryWeightLimit() const { return 0.0; }
    virtual int carryVolumeLimit() const { return 0; }
    virtual roomDirData *exitDir(dirTypeT) const = 0;
    virtual int fallOffMount(TThing *, positionTypeT, bool death = FALSE);
    virtual void addToCarriedVolume(int num);
    virtual int getReducedVolume(const TThing *) const;
    virtual float getTotalWeight(bool) const;
    virtual int getTotalVolume() const;
    virtual void setVolume(int) {}
    virtual void addToVolume(int) {}
    virtual int getVolume() const { return 0; }
    virtual bool canDrop() const { return TRUE; }
    virtual sstring yourDeity(spellNumT, personTypeT, const TBeing *who = NULL) const { return ""; }
    virtual void remCastingList(TThing *);
    virtual bool isSimilar(const TThing *) const { return false; }
    virtual bool canSee(const TThing *, infraTypeT = INFRA_NO) const { return false; }
    virtual int  getSnum() const { return snum; };

    // ch can not be const due to showTo()
    virtual void lookAtObj(TBeing *, const char *, showModeT) const;

    // this can not be const, due to show_me_to_char
    virtual void showTo(const TThing *, showModeT) {}
    virtual void showMultTo(const TThing *, showModeT, unsigned int) {}

    virtual bool isPc() const { return false; }
    virtual void sendTo(colorTypeT, const sstring &) const;
    virtual void sendTo(const sstring &) const;
    virtual void update(int) {}

    virtual int  pourWaterOnMe(TBeing *, TObj *) { return false; }
    virtual bool splitMe(TBeing *, const sstring &) { return false; }
    virtual int dropSmoke(int);

    virtual sstring compareMeAgainst(TBeing *, TObj *);
    // END VIRTUAL FUNCTIONS

    // INLINE FUNCTIONS
    const char * getDescr() const { return descr; }
    void setDescr(const char *s) { descr = s; }
    int getHeight() const{ return height; }
    void setHeight(int h) { height = h; }
    void setCaster(TBeing *c) { the_caster = c; }
    TBeing * getCaster() const { return the_caster; }
    // END INLINE FUNCTIONS

    int inRoom() const;
    float getWeight() const;
    int getLight() const;
    void setLight(int num);
    void addToLight(int num);
    void setRoom(int room);
    void setMaterial(ubyte num);
    ubyte getMaterial() const;
    void setWeight(const float w);
    void addToWeight(const float w);
    float getCarriedWeight() const;
    int getCarriedVolume() const;
    void setCarriedWeight(float num);
    void setCarriedVolume(int num);
    bool isSpiked() const;
    bool canGo(dirTypeT) const;
    bool canGoHuman(dirTypeT) const;
    void addToCarriedWeight(float num);

    bool isMetal() const;
    bool isMineral() const;

    void newOwner(TThing *);
    const char *objs(const TThing *t) const;
    const sstring objn(const TThing *t) const;
    const char *ana() const;
    const char *sana() const;
    const char *pers(const TThing *t) const;
    const sstring persfname(const TThing *t) const;
    TThing * dismount(positionTypeT);
    void mount(TThing *);
    bool sameRoom(const TThing &ch) const;
    bool inImperia() const;
    bool inGrimhaven() const;
    bool inLogrus() const;
    bool inAmber() const;
    bool inBrightmoon() const;
    bool inLethargica() const;
    int getDrechels(int) const;
    int checkSoundproof() const;
    int visibility() const;
    bool outside() const;
    TThing *horseMaster() const;
    bool isMount() const { return (rider != NULL); }
    virtual int getNumRiders(TThing *) const { return 0; }
    virtual int getMaxRiders() const { return 0; }
    virtual int getRiderHeight() const { return 0; }
    virtual int bumpHeadDoor(roomDirData *, int *) {return FALSE; }
    TThing *thingHolding() const;
    virtual bool isRideable() const { return FALSE; }
    virtual int genericMovedIntoRoom(TRoom *, int, checkFallingT = CHECK_FALL_YES)
    { return FALSE; }
    int genericTeleport(silentTypeT, bool keepZone = FALSE, bool unsafe=false);
    virtual bool isFlying() const { return FALSE; }
    virtual bool isLevitating() const { return FALSE; }
    virtual int trapSleep(int) { return FALSE; }
    virtual void trapPoison(int) {}
    virtual void trapDisease(int) {}
    virtual int trapTeleport(int) { return FALSE; }
    virtual bool canGet(const TThing *, silentTypeT) const { return FALSE; }
    virtual bool canGetMe(const TBeing *, silentTypeT) const;
    virtual TThing *heldInPrimHand() const { return NULL; }
    virtual TThing *heldInSecHand() const { return NULL; }
    virtual int swungObjectDamage(const TBeing *, const TBeing *) const;
    virtual double baseDamage() const { return 0; }
    virtual int detonateGrenade() { return FALSE; }
    virtual int grenadeHit(TTrap *) { return FALSE; } 
    virtual bool hasHands() const { return FALSE; }

    virtual bool poisonObject() { return FALSE; }
    virtual void findSomeFood(TFood **, TBaseContainer **, TBaseContainer *) {}
    virtual void findSomeDrink(TDrinkCon **, TBaseContainer **, TBaseContainer *) {}
    virtual void describeContains(const TBeing *) const;
    virtual void describeObjectSpecifics(const TBeing *) const {}
    virtual void nukeFood();
    virtual void evaporate(TBeing *, silentTypeT) {}
    virtual int quaffMe(TBeing *);
    virtual bool waterSource() { return FALSE; }
    virtual void spill(const TBeing *) {}
    virtual void adjustLight() {}
    virtual void lampLightStuff(TMonster *) {}
    virtual void lightDecay() {}
    virtual void extinguishWater(TBeing *) {}
    virtual void extinguishWater() {}
    virtual int getMe(TBeing *, TThing *);
    virtual void getMeFrom(TBeing *, TThing *);
    virtual void lightMe(TBeing *, silentTypeT silent);
    virtual void extinguishMe(TBeing *);
    virtual void refuelMeDrug(TBeing *, TDrugContainer *);
    virtual void refuelMeFuel(TBeing *, TLight *);
    virtual void refuelMeLight(TBeing *, TThing *);
    virtual void junkMe(TBeing *) {}
    virtual void thingDumped(TBeing *, int *) {}
    virtual void writeMeNote(TBeing *, TPen *);
    virtual void writeMePen(TBeing *, TThing *);
    virtual void powerstoneCheck(TOpal **) {}
    virtual void powerstoneCheckCharged(TOpal **) {}
    virtual void powerstoneMostMana(int *) {}
    virtual void audioCheck(int) const {}
    virtual void usingBoat(int *) {}
    virtual int scavengeMe(TBeing *, TObj **) { return FALSE; }
    virtual int moneyMeBeing(TThing *, TThing *) { return FALSE; }
    virtual int moneyMeMoney(TBeing *, TThing *) { return FALSE; }
    virtual void logMe(const TBeing *, const char *) const;
    virtual int powerstoneMe(TBeing *, int, byte);
    virtual int divineMe(TBeing *, int, byte);
    virtual void postMe(TBeing *, const char *, boardStruct *);
    virtual void giveToRepairNote(TMonster *, TBeing *, int *) {}
    virtual void describeMe(TBeing *) const {}

    // ch can not be const, due to windowLook
    virtual void showMe(TBeing *) const;

    // ch can not be const due to use of bSuccess
    virtual void listMe(TBeing *, unsigned int) const;
    virtual void listMeExcessive(TBeing *) const;

    virtual void dropMe(TBeing *, showMeT, showRoomT);
    virtual int throwMe(TBeing *, dirTypeT, const char *);
    virtual int moveTrapCheck(TBeing *, dirTypeT) { return FALSE; }
    virtual int insideTrapCheck(TBeing *, TThing *) { return FALSE; }
    virtual int anyTrapCheck(TBeing *) { return FALSE; }
    virtual int getTrapCheck(TBeing *) { return FALSE; }
    virtual int detectMe(TBeing *) const;
    virtual int stealModifier();
    virtual void purgeMe(TBeing *);
    virtual void evaluateMe(TBeing *) const;
    virtual void findSomeComponent(TComponent **, TComponent **, TComponent **, spellNumT, int) {}
    virtual bool allowsCast() { return false; }
    virtual int putMeInto(TBeing *, TOpenContainer *);
    virtual void findComp(TComponent **, spellNumT);
    virtual int componentSell(TBeing *, TMonster *, int, TThing *);
    virtual int componentValue(TBeing *, TMonster *, int, TThing *);
    virtual int removeMe(TBeing *, wearSlotT);
    virtual void scrapMe(TBeing *) {}
    virtual void findBandage(int *);
    virtual void destroyBandage(int *) {}
    virtual void sitMe(TBeing *);
    virtual void restMe(TBeing *);
    virtual void sleepMe(TBeing *);
    virtual int mobPulseBed(TMonster *, short int) { return FALSE; }
    virtual void attuneMe(TBeing *, TVial *);
    virtual void sharpenMeStone(TBeing *, TThing *);
    virtual void dullMeFile(TBeing *, TThing *);
    virtual int poisonMePoison(TBeing *, TBaseWeapon *);
    virtual int garotteMe(TBeing *, TBeing *);
    virtual void sstringMeBow(TBeing *, TThing *);
    virtual void sstringMeString(TBeing *, TBow *);
    virtual void skinMe(TBeing *, const char *);
    virtual void butcherMe(TBeing *, const char *);
    virtual void sacrificeMe(TBeing *, const char *);
    virtual int pickWithMe(TBeing *, const char *, const char *, const char *);
    virtual void repairMeHammer(TBeing *, TObj *);
    virtual int garottePulse(TBeing *, affectedData *);
    virtual int ChargePulse(TBeing *);
    virtual void unequipMe(TBeing *) {}
    virtual void findBlacksmithingTools(TTool **, TTool **) {}
    virtual void blacksmithingPulse(TBeing *, TObj *);
    virtual void attunePulse(TBeing *);
    virtual void pickPulse(TBeing *);
    virtual int  skinPulse(TBeing *, TBaseCorpse *);
    virtual int  butcherPulse(TBeing *, TBaseCorpse *);
    virtual void sharpenPulse(TBeing *, TThing *);
    virtual void dullPulse(TBeing *, TThing *);
    virtual void findSym(TSymbol **) {}
    const sstring getNameNOC(const TBeing *) const;
    virtual int sellCommod(TBeing *, TMonster *, int, TThing *);
    virtual void makeScraps() {}
    virtual void attunerValue(TBeing *, TMonster *);
    virtual void attunerGiven(TBeing *, TMonster *);
    virtual int reciteMe(TBeing *, const char *);
    virtual int useMe(TBeing *, const char *);
    virtual void findVialAttune(TVial **, int *) {}
    virtual void getBestVial(TVial **) {}
    virtual int damageMe(TBeing *, TBeing *, wearSlotT) { return FALSE; }
    virtual int sharpenerValueMe(const TBeing *, TMonster *) const;
    virtual int sharpenerGiveMe(TBeing *, TMonster *);
    virtual void sharpenMe(TBeing *, TTool *);
    virtual void dullMe(TBeing *, TTool *);
    virtual int sellHidenSkin(TBeing *, TMonster *, int, TThing *);

    virtual sstring describeMySharp(const TBeing *) const;
    virtual void sharpenMeStoneWeap(TBeing *, TTool *);
    virtual void dullMeFileWeap(TBeing *, TTool *);
    virtual int catchSmack(TBeing *, TBeing **, TRoom *, int, int);
    virtual void specializationCheck(TBeing *, float *) {}
    virtual int expelPrice(const TBeing *, int) const;
    virtual int wieldMe(TBeing *, char *);
    virtual void curseMe() {}
    virtual int poisonWeaponWeapon(TBeing *, TThing *);
    virtual int smiteWithMe(TBeing *, TBeing *);
    virtual int enhanceMe(TBeing *, int, byte);
    virtual void moneyMove(TBeing *);
    virtual bool isRentable() const { return FALSE; }
    virtual float blowCountSplitter(const TBeing *, bool) const;
    virtual void bloadBowArrow(TBeing *, TThing *);
    virtual void bloadArrowBow(TBeing *, TArrow *);
    virtual int shootMeBow(TBeing *ch, TBeing *, unsigned int, dirTypeT, int);
    virtual int openMe(TBeing *);
    virtual int putSomethingInto(TBeing *, TThing *);
    virtual int putSomethingIntoTable(TBeing *, TTable *);
    virtual int putSomethingIntoContainer(TBeing *, TOpenContainer *);
    virtual void peeOnMe(const TBeing *);
    virtual bool listThingRoomMe(const TBeing *) const;
    virtual bool canSeeMe(const TBeing *, infraTypeT) const;
    virtual bool getObjFromMeCheck(TBeing *);
    virtual void getObjFromMeText(TBeing *, TThing *, getTypeT, bool);

    // ch can not be const, due to showMe
    virtual void show_me_mult_to_char(TBeing *, showModeT, unsigned int) const {}
    virtual void show_me_to_char(TBeing *, showModeT) const {}

    virtual spellNumT getWtype() const;
    virtual bool isBluntWeapon() const { return TRUE; }   // generic things = blunt?
    virtual bool isSlashWeapon() const { return FALSE; }
    virtual bool isPierceWeapon() const { return FALSE; }
    virtual bool canCarryMe(const TBeing *, silentTypeT) const;
};

#endif
