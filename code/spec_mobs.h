//////////////////////////////////////////////////////////////////////////
//
// SneezyMUD - All rights reserved, SneezyMUD Coding Team
//
//////////////////////////////////////////////////////////////////////////


/************************************************************************

      SneezyMUD - All rights reserved, SneezyMUD Coding Team
      spec_mobs.h : interface for calling mobiles special procedures

  ----------------------------------------------------------------------

  Special procedures for mobiles may be called under a number of different
  conditions.  These conditions are defined in this file with two
  exceptions:  when cmd >= 0, a command is triggering the spec_proc, and
  when cmd == -1, the spec_proc is being triggered by a PULSE.

  A return value of zero (FALSE) indicates that nothing exceptional
  happened.  A non-zero return value (TRUE) indicates that some action
  needs to be taken (such as ignoring the players command).  When in doubt,
  return FALSE.

*************************************************************************/

#ifndef __SPEC_MOBS_H
#define __SPEC_MOBS_H

const int SPEC_JANITOR           =5;
const int SPEC_TORMENTOR         =6;
const int SPEC_TRAINER_AIR       =7;
const int SPEC_CHICKEN           =8;
const int SPEC_BOUNTY_HUNTER   =11;
const int SPEC_ALIGN_DEITY     =12;
const int SPEC_ENGRAVER        =13;
const int SPEC_CITYGUARD       =14;
const int SPEC_HORSE           =16;
const int SPEC_PET_KEEPER      =18;
const int SPEC_STABLE_MAN      =19 ;
const int SPEC_HORSE_FAMINE    =20 ;
const int SPEC_HORSE_WAR       =21 ;
const int SPEC_CRAPSGUY        =24;
const int SPEC_SHOPKEEPER      =25;
const int SPEC_POSTMASTER      =26;
const int SPEC_RECEPTIONIST    =27;
const int SPEC_GM_MAGE         =29;
const int SPEC_GM_DEIKHAN      =30;
const int SPEC_GM_MONK         =31;
const int SPEC_GM_WARRIOR      =32;
const int SPEC_GM_THIEF        =33;
const int SPEC_GM_CLERIC       =34;
const int SPEC_GM_RANGER       =35;
const int SPEC_TRAINER_ALCHEMY =37;
const int SPEC_TRAINER_EARTH   =38;
const int SPEC_REPAIRMAN       =39;
const int SPEC_SHARPENER       =40 ;   
const int SPEC_CARAVAN         =43;
const int SPEC_HORSE_DEATH     =44;
const int SPEC_HORSE_PESTILENCE=45;
const int SPEC_DOCTOR          =48;
const int SPEC_TRAINER_FIRE    =49;
const int SPEC_TRAINER_SORCERY =63;
const int SPEC_TRAINER_SPIRIT  =64;
const int SPEC_TRAINER_WATER   =65;
const int SPEC_TRAINER_WRATH   =66;
const int SPEC_TRAINER_AFFLICTIONS=67;
const int SPEC_TRAINER_CURE     =68;
const int SPEC_TRAINER_HAND_OF_GOD =69;
const int SPEC_TRAINER_RANGER  =70;
const int SPEC_TRAINER_LOOTING =72;
const int SPEC_TRAINER_MURDER  =73;
const int SPEC_TRAINER_DUELING= 74;
const int SPEC_TRAINER_ADVENTURING=75;
const int SPEC_TRAINER_WARRIOR  =76;
const int SPEC_TOLL_TAKER       =79;
const int SPEC_TRAINER_WIZARDRY =80;
const int SPEC_TRAINER_FAITH    =81;
const int SPEC_TRAINER_SLASH    =82;
const int SPEC_TRAINER_BLUNT    =83;
const int SPEC_TRAINER_PIERCE   =84;
const int SPEC_TRAINER_RANGED   =85;
const int SPEC_QUAKETUNNELER    =86;
const int SPEC_TRAINER_DEIKHAN  =87;
const int SPEC_LAMPBOY          =96;
const int SPEC_LEPER            =97;
const int SPEC_AIR_MAGI         =98;
const int SPEC_WATER_MAGI       =99;
const int SPEC_EARTH_MAGI      =100;
const int SPEC_FIRE_MAGI       =101;
const int SPEC_SORCERER        =102;
const int SPEC_FACTION_FAERY   =103;
const int SPEC_TRAINER_BRAWLING=104;
const int SPEC_TRAINER_SHAMAN_HEALING   =105;
const int SPEC_TRAINER_MEDITATION_MONK    =106;
const int SPEC_TRAINER_SURVIVAL=107;
const int SPEC_TRAINER_SHAMAN_ARMADILLO  =108;
const int SPEC_TRAINER_ANIMAL         =109;
const int SPEC_TRAINER_AEGIS          =110;
const int SPEC_TRAINER_SHAMAN         =111;
const int SPEC_RUMORMONGER            =114;
const int SPEC_TRAINER_MAGE           =115;
const int SPEC_TRAINER_MONK           =116;
const int SPEC_TRAINER_CLERIC         =117;
const int SPEC_TRAINER_THIEF          =118;
const int SPEC_TRAINER_PLANTS         =119;
const int SPEC_TRAINER_SOLDIERING     =120;
const int SPEC_TRAINER_BLACKSMITHING  =121;
const int SPEC_TRAINER_DEIKHAN_FIGHT  =122;
const int SPEC_TRAINER_MOUNTED        =123;
const int SPEC_TRAINER_DEIKHAN_AEGIS  =124;
const int SPEC_TRAINER_DEIKHAN_CURES  =125;
const int SPEC_TRAINER_DEIKHAN_WRATH  =126;
const int SPEC_TRAINER_LEVERAGE       =127;
const int SPEC_TRAINER_MINDBODY       =128;
const int SPEC_TRAINER_UNUSED6        =129;
const int SPEC_TRAINER_UNUSED7        =130;
const int SPEC_TRAINER_FOCUSED_ATTACKS=131;
const int SPEC_TRAINER_UNUSED8        =132;
const int SPEC_TRAINER_BAREHAND       =133;
const int SPEC_TRAINER_THIEF_FIGHT    =134;
const int SPEC_TRAINER_POISONS        =135;
const int SPEC_TRAINER_SHAMAN_FROG    =136;
const int SPEC_TRAINER_SHAMAN_ALCHEMY =137;
const int SPEC_TRAINER_SHAMAN_SKUNK   =138;
const int SPEC_TRAINER_SHAMAN_SPIDER  =139;
const int SPEC_TRAINER_SHAMAN_CONTROL =140;
const int SPEC_TRAINER_RITUALISM      =141;
const int SPEC_PALADIN_PATROL         =142;
const int SPEC_GM_SHAMAN              =143;
const int SPEC_TRAINER_COMBAT         =144;
const int SPEC_TRAINER_STEALTH        =145;
const int SPEC_TRAINER_TRAPS      =146;
const int SPEC_NEWBIE_EQUIPPER    =147;
const int SPEC_TRAINER_LORE       =148;
const int SPEC_TRAINER_THEOLOGY   =149;
const int SPEC_ATTUNER            =150;
const int SPEC_DOPPLEGANGER       =152;
const int SPEC_TUSKGORE           =153;
const int SPEC_FISHTRACKER        =154;
const int SPEC_BANK_GUARD         =155;
const int SPEC_REAL_ESTATE_AGENT  =156;
const int SPEC_CORONER            =157;
const int SPEC_FACTON_REGISTRAR   =158;
const int SPEC_TRAINER_DEFENSE    =159;
const int SPEC_SCARED_KID         =160;
const int SPEC_CORPORATE_ASSISTANT=161;
const int SPEC_TRAINER_PSIONICS   =162;
const int SPEC_DIVMAN             =163;
const int SPEC_GM_MAGE_THIEF      =164; // reuse this for something else
const int SPEC_PLANTER            =165;
const int SPEC_BMARCHER           =166;
const int SPEC_MONEY_TRAIN        =167;
const int SPEC_ADVENTURER         =168;
const int SPEC_TRAINER_IRON_BODY  =169;
const int SPEC_TUDY               =170;
const int SPEC_KAVOD_BARMAID      =171;
const int SPEC_TATTOO_ARTIST      =172;
const int SPEC_ELEVATOR_OP        =173;
const int SPEC_ELEVATOR_GUARD     =174;
const int SPEC_COMMOD_MAKER       =175;
const int SPEC_LOTTERY_REDEEMER   =176;
const int SPEC_KONASTIS_GUARD     =177;
const int SPEC_HOLDEM_PLAYER      =178;
const int SPEC_POSTMAN            =179;
const int SPEC_POISON_BITE        =180;
const int SPEC_RIDDLING_TREE      =181;
const int SPEC_FIREMAN            =182;
const int SPEC_MIMIC              =183;
const int SPEC_ARCHER             =184;
const int SPEC_FLASK_PEDDLER      =185;
const int SPEC_LIMB_DISPO         =186;
const int SPEC_STAT_SURG          =187;
const int SPEC_SHIPPING_OFFICIAL  =188;
const int SPEC_LOAN_SHARK         =189;
const int SPEC_TROLLEY_DRIVER     =190;
const int SPEC_PROPERTY_CLERK     =191;
const int SPEC_BANKER             =192;
const int SPEC_PRISON_JANITOR     =193;
const int SPEC_CAT                =194;
const int SPEC_TAXMAN             =195;
const int SPEC_TRAINER_ADV_ADVENTURING=196;
const int SPEC_AMBER_JANITOR      =197;
const int SPEC_BRIGHTMOON_JANITOR =198;
const int SPEC_GARBAGE_CONVOY     =199;
const int SPEC_SIGNMAKER          =200;
const int SPEC_BUTLER             =201;
const int SPEC_LEPER_HUNTER       =202;
const int SPEC_AUCTIONEER         =203;
const int SPEC_LOAN_MANAGER       =204;
const int SPEC_BEE_DEATH          =205;
const int SPEC_HERO_FAERIE        =206;
const int SPEC_BRICK_COLLECTOR    =207;
const int SPEC_CARETAKER          =208;
const int SPEC_SHIP_CAPTAIN       =209;
const int SPEC_AGGRO_FOLLOWER     =210;
const int SPEC_CENTRAL_BANKER     =211;
const int SPEC_CANNON_LOADER      =212;
const int SPEC_ID_CARD_PROVIDER   =213;
const int SPEC_TARGET_DUMMY       =214;
const int SPEC_FRUIT_SCAVENGER    =215;
const int SPEC_COMMOD_TRADER      =216;
const int SPEC_RATION_FACTORY     =217;
const int NUM_MOB_SPECIALS        =217;

extern const int GET_MOB_SPE_INDEX(int d);

struct TMobSpecs {
  bool assignable;
  const char *name;
  int (*proc) (TBeing *, cmdTypeT, const char *, TMonster *, TObj *);
};

extern TMobSpecs mob_specials[NUM_MOB_SPECIALS + 1];
extern bool UtilMobProc(TBeing *);
extern bool GuildMobProc(TBeing *);
extern TMonster *pawnman;
extern void CallForGuard(TBeing *ch, TBeing *vict, int lev);
extern TMonster *FindMobInRoomWithProcNum(int room, int num);
extern bool okForJanitor(TMonster *, TObj *);

extern int tattooArtist(TBeing *, cmdTypeT, const char *, TMonster *, TObj *);
extern int payToll(TBeing *, cmdTypeT, const char *, TMonster *, TObj *);
extern int meeting_organizer(TBeing *, cmdTypeT, const char *, TMonster *, TObj *);
extern int fighter(TBeing *, cmdTypeT, const char *, TMonster *, TObj *);
extern int doctor(TBeing *, cmdTypeT, const char *, TMonster *, TObj *);
extern int craps_table_man(TBeing *, cmdTypeT, const char *, TMonster *, TObj *);
extern int shop_keeper(TBeing *, cmdTypeT, const char *, TMonster *, TObj *);
extern int receptionist(TBeing *, cmdTypeT, const char *, TMonster *, TObj *);
extern int postmaster(TBeing *, cmdTypeT, const char *, TMonster *, TObj *);
extern int ShamanGuildMaster(TBeing *, cmdTypeT, const char *, TMonster *, TObj *);
extern int MageGuildMaster(TBeing *, cmdTypeT, const char *, TMonster *, TObj *);
extern int MageThiefGuildMaster(TBeing *, cmdTypeT, const char *, TMonster *, TObj *);
extern int DeikhanGuildMaster(TBeing *, cmdTypeT, const char *, TMonster *, TObj *o);
extern int MonkGuildMaster(TBeing *, cmdTypeT, const char *, TMonster *, TObj *);
extern int WarriorGuildMaster(TBeing *, cmdTypeT, const char *, TMonster *, TObj *);
extern int ThiefGuildMaster(TBeing *, cmdTypeT, const char *, TMonster *, TObj *);
extern int ClericGuildMaster(TBeing *, cmdTypeT, const char *, TMonster *, TObj *);
extern int RangerGuildMaster(TBeing *, cmdTypeT, const char *, TMonster *, TObj *);
extern int repairman(TBeing *, cmdTypeT, const char *, TMonster *, TObj *);
extern int alignment_deity(TBeing *, cmdTypeT, const char *, TMonster *, TObj *);
extern int CDGenericTrainer(TBeing *, cmdTypeT, const char *, TMonster *, TObj *);
extern int bounty_hunter(TBeing *, cmdTypeT, const char *, TMonster *, TObj *);
extern int DragonBreath(TBeing *, cmdTypeT, const char *, TMonster *, TObj *);
extern int doNewbieEquipLoad(int, int, int);
extern int doppleganger(TBeing *, cmdTypeT, const char *, TMonster *, TObj *);
extern int tunnelerEarthquake(TBeing *, cmdTypeT, const char *, TMonster *, TObj *);
extern int tuskGoring(TBeing *, cmdTypeT, const char *, TMonster *, TObj *);
extern int fishTracker(TBeing *, cmdTypeT, const char *, TMonster *, TObj *);
extern int bankGuard(TBeing *, cmdTypeT, const char *, TMonster *, TObj *);
extern int realEstateAgent(TBeing *, cmdTypeT, const char *, TMonster *, TObj *);
extern int shopWhisper(TBeing *, TMonster *, int, const char *);
extern int archer(TBeing *, cmdTypeT, const char *, TMonster *, TObj *);
extern int heroFaerie(TBeing *ch, cmdTypeT cmd, const char *arg, TMonster *myself, TObj *, bool login=false);
extern int heroFaerie(TBeing *ch, cmdTypeT cmd, const char *arg, TMonster *myself, TObj *);
extern int brickCollector(TBeing *, cmdTypeT, const char *, TMonster *, TObj *);
extern int targetDummy(TBeing *, cmdTypeT, const char *, TMonster *, TObj *);

class attune_struct {
  public:
    int wait;
    int cost;
    factionTypeT faction;
    bool hasJob;
    TBeing *pc;
    TSymbol *sym;

    attune_struct() :
      wait(0),
      cost(0),
      faction(FACT_UNDEFINED),
      hasJob(false),
      pc(NULL),
      sym(NULL) {
    }
    ~attune_struct() {
    }
    void clearAttuneData();
};


class bounty_hunt_struct
{
  public:
  char *hunted_item;
  char *hunted_victim;
  char *last_targ;
  int num_chances;
  int level_command;
  int num_retrieved;

  // this guy serves to identify a spot in the object_list after which
  // we know there are no valid hunted_items.  This will truncate the time
  // spent in findHuntedItem().
  TObj *noneBeyond;

  // improved intelligence
  bool warned;
  bool singletarg;
  bool missionaccomplished;

  private:
    bounty_hunt_struct() {} // prevent use
  public:
    bounty_hunt_struct(char *hi, char *hv, int chan, int lev);
    ~bounty_hunt_struct();
    void reset();
};

class sharp_struct {
  public:
    int wait;
    int cost;
    char *char_name;
    char *obj_name;
    byte isBlunt;
    sharp_struct() :
      wait(0),
      cost(0),
      char_name(NULL),
      obj_name(NULL),
      isBlunt(FALSE) {
    }
    ~sharp_struct() {
      delete [] char_name;
      delete [] obj_name;
    }
};

#endif
