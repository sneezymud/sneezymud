//////////////////////////////////////////////////////////////////////////
//
// SneezyMUD - All rights reserved, SneezyMUD Coding Team
//
//////////////////////////////////////////////////////////////////////////


#ifndef __EXTERN_H
#define __EXTERN_H

#ifndef __BEING_H
#error Please include BEING.H prior to inclusion of EXTERN.H
#endif

extern "C" {
#if 0
// disabled functions
extern int max_stat(race_t, statTypeT);
extern int min_stat(race_t, statTypeT);
#endif
extern wearSlotT slot_from_bit(int);
extern void cleanCharBuf(char *);
extern int split_string(const sstring &str, const sstring &sep, vector<sstring> &argv);
extern void sendrp_exceptf(TRoom *, TBeing *, const char *,...);
extern const char *how_good(int);
extern sstring sprintbit(unsigned long, const char * const []);
extern bool is_exact_spellname(const sstring &, const sstring &);
extern bool is_exact_name(const sstring &, const sstring &);
extern int get_number(char **);
extern void printLimitedInRent(void);
extern void updateRentFiles(void);
extern bool noteLimitedItems(FILE *, const char *, unsigned char, bool);
extern const int thaco[128];
extern const byte ac_percent_pos[MAX_WEAR];
extern int AddToCharHeap(TBeing *heap[50], int *, int total[50], TBeing *);
extern int MobCountInRoom(const TThing *);
extern bool WizLock;
extern bool WizGoto;
extern bool WizShout;
extern bool WizInvis;
extern bool Twink;
extern bool Sleep;
extern bool NewbiePK;
extern bool AllowPcMobs;
extern bool TestCode1;
extern bool TestCode2;
extern bool TestCode3;
extern bool TestCode4;
extern bool TestCode5;
extern bool TestCode6;  
extern bool timeQueries;
extern bool gameLoopTiming;
extern bool Silence;
extern bool WizBuild;
extern bool Gravity;
extern bool QuestCode;
extern bool QuestCode2;
extern bool QuestCode3;
extern bool QuestCode4;
extern bool Shutdown;
extern long timeTill;
extern time_t Uptime;
extern bool fireInGrimhaven;
extern int clearpath(int room, dirTypeT direc);
extern const char *DescDamage(double dam);
extern const char *DescMoves(double a);
extern const char *ac_for_score(int a);
extern const char *DescRatio(double f);
extern const char *DescAttacks(double a);
extern int MountEgoCheck(TBeing *, TBeing *);
extern bool getall(const char *, char *);
extern int getabunch(const char *, char *);
extern bool is_number(const sstring &);
extern void bisect_arg(const char *, int *, char *, const char * const array[]);

  extern const char * heraldcolors[];
  extern const char * heraldcodes[];
extern const char * const card_names[14];
extern const char * const scandirs[];
extern const char * const home_terrains[];
extern const char * const editor_types_oedit[];
extern const char * const editor_types_medit[];
extern const char * const immunity_names[MAX_IMMUNES];
extern const char * const extra_bits[];
extern const char * const affected_bits[];
extern const char * const dirs[];
extern const char * const dirs_to_leading[];
extern const char * const dirs_to_blank[];
extern const char * const connected_types[MAX_CON_STATUS];
extern const char * const body_flags[];
extern const char * const fullness[];
extern const char * const attack_modes[];
extern const char * const attr_player_bits[];
extern const char * const wear_bits[MAX_ITEM_WEARS];
extern const char * const room_bits[MAX_ROOM_BITS];
extern const char * const exit_bits[MAX_DOOR_CONDITIONS];
extern const char * const sector_types[];
extern const char * const action_bits[];
extern const char * const player_bits[];
extern const sstring position_types[];
extern const char * const material_groups[];
extern const char * const portal_self_enter_mess[MAX_PORTAL_TYPE];
extern const char * const portal_other_enter_mess[MAX_PORTAL_TYPE];
extern const char * const portal_self_exit_mess[MAX_PORTAL_TYPE];
extern const char * const deities[MAX_DEITIES];
extern const char * const bodyParts[MAX_WEAR+1];
extern const char * const trap_effects[MAX_TRAP_EFF];
extern const char * const chest_bits[];
extern const char * const door_types[];
extern const char * const color_options[10];
extern const char * const disc_names[];
extern const char * const auto_name[MAX_AUTO];
extern const char * const exits[];
extern const char * const corpse_flags[MAX_CORPSE_FLAGS];
extern const char * const illegalnames[];
extern const char * const month_name[12];
extern const char * const weekdays[7];
extern int numberhosts;
extern sstring lockmess;
extern const byte sharpness[];
extern const byte attack_mode_bonus[];
extern const int corpse_volume[];
extern const struct race_perc race_size[];
extern const double race_vol_constants[MAX_WEAR];
extern const int TrapDir[];
extern float power_level_number(int);
extern int personalize_object(TBeing *deity, TBeing *, int virt, int decay);
extern int resize_personalize_object(TBeing *deity, TBeing *, int virt, int decay);
extern bool safe_to_save_shop_stuff(TMonster *);
extern bool safe_to_be_in_system(const sstring &);
extern void sprinttype(int type, const sstring names[], char *result);
extern int vsystem(const sstring &);
extern bool load_char(const sstring &name, charFile *);
extern int game_loop(int s);
extern bool safe_to_be_in_system_filepath(const sstring &);
extern void RoomLoad(TBeing *, int, int, bool);
extern int noise(const TBeing *);
extern void change_hands(TBeing *, const char *);
extern void appendPlayerName(TBeing *, TBeing *);
extern void setCombatStats(TBeing *, TBeing *, PolyType, spellNumT);
extern void SwitchStuff(TBeing *, TBeing *, bool setStats = TRUE);
extern int lycanthropeTransform(TBeing *);
extern void DisguiseStuff(TBeing *, TBeing *);
extern void CreateOneRoom(int);
extern void gain_exp(TBeing *, double gain, int rawdamage);
extern int check_sinking_obj(TObj *obj, int room);
extern int ctoi(char c);
extern void update_time(void);
extern void do_check_mail();
extern void do_components(int pulse);
extern void extract_edit_char(TMonster *);
extern void obj_edit(TBeing *, const char *arg);
extern void room_edit(TBeing *, const char *arg);
extern void mob_edit(TBeing *, const char *);
extern void seditCore(TBeing *, char *);
extern void perform_violence(int pulse);
extern void help_edit(TBeing *, char *arg);
extern void call_room_specials(void);
extern int DetermineExp(TBeing *mob);
extern void load_one_room(FILE * fl, TRoom *rp);
extern const dirTypeT rev_dir[];
extern void allocate_room(int);
extern void buildCommandArray();
extern void assign_objects(void);
extern void assign_rooms(void);
extern void buildSpellArray();
extern void buildSpellDamArray();
extern void buildTerrainDamMap();
extern void buildWeatherDamMap();
extern void buildComponentArray();
extern int init_game_stats();
extern void save_game_stats();
extern void fixup_players(void);
extern void bootSocialMessages(void);
extern void bootTheShops();
extern void processShopFiles(void);
extern void processCorpseFiles(void);
extern void processRepairFiles(void);
extern void updateSavedRoomItems(void);
extern void updateSavedRoom(char *fname);
extern TObj *get_object_in_equip(TBeing *, char *, TObj *[], int *, int);
extern char *fold(char *);
extern bool has_prereqs(TBeing *, int);
extern bool should_be_logged(const TBeing *);
extern void stripFrontBytes(char *, int);
extern void BatoprsResetCharFlags (TBeing *);
extern bool trade_with(const TObj *, int);
extern void order_high_to_low(int *num1, int *num2, int *num3);
extern bool isanumber(const char *);
// extern void roll_abilities(TBeing *);
extern void zero_stats(TBeing *);
extern int CheckStorageChar (TBeing *, TBeing *);
extern bool raw_save_char(const char *, charFile *);
extern void set_killer_flag(TBeing *, TBeing *);
extern int SpaceForSkills(TBeing *);
extern sstring add_bars(const sstring &sstring);
extern dirTypeT can_see_linear(const TBeing *, const TBeing *targ, int *rng, dirTypeT *dr);
extern dirTypeT choose_exit_in_zone(int in_room, int tgt_room, int depth);
extern dirTypeT choose_exit_global(int in_room, int tgt_room, int depth);
extern void SetupCreateEngineData();
extern sstring nextToken(char, unsigned int, char *);
extern void MakeRoomNoise(TMonster *, int room, const char *local_snd, const char *distant_snd);
extern void MakeNoise(int room, const char *local_snd, const char *distant_snd);
extern bool has_healthy_body(TBeing *);
extern int RecGetObjRoom(const TThing *);
extern int ObjFromCorpse(TObj *c);
extern void break_bone(TBeing *, wearSlotT which);
extern void dirwalk(const sstring &dir, void (*fcn) (const char *));
extern void dirwalk_fullname(const char *dir, void (*fcn) (const char *));
extern void dirwalk_subs_fullname(const char *dir, void (*fcn) (const char *));
extern void argument_split_2(const char *, char *, char *);
extern int RecCompObjNum(const TObj *o, int obj_num);
extern char * dsearch(const char *);
extern TOpal *find_biggest_powerstone(const TBeing *);
extern void wipeCorpseFile(const char *);
extern void wipeRentFile(const char *);
extern void wipeFollowersFile(const char *);
extern void wipePlayerFile(const char *);
extern void handleCorrupted(const char *, char *);
extern void store_mail(const char *, const char *, const char *);
extern void setup_dir(FILE * fl, int room, dirTypeT dir, TRoom * = NULL);
extern char hostLogList[MAX_BAN_HOSTS][40];
extern int numberLogHosts;
extern long roomCount;
extern int script_on_command(TBeing *, char *, int);
extern void initWhittle();
extern vector<zoneData>zone_table;
extern void DeleteHatreds(const TBeing *, const char *);
extern void DeleteFears(const TBeing *, const char *);
extern int SHARP_PRICE(TObj *);
extern bool UtilProcs(int);
extern bool GuildProcs(int);
extern void deityCheck(int);
extern void apocCheck();

extern void list_char_in_room(TThing *list, TBeing *ch);

// ch can not be const due to listMe()
extern void list_thing_in_room(const TThing *list, TBeing *ch);
extern void list_thing_on_heap(const TThing *, TBeing *ch, bool);

// ch can not be const, due to showTo
extern void list_in_heap(const TThing *list, TBeing *ch, bool show_all, int perc);

extern void update_world_weather();
extern bool pierceType(spellNumT);
extern bool bluntType(spellNumT);
extern bool slashType(spellNumT);
extern int generic_dispel_magic(TBeing *, TBeing *, int, immortalTypeT, safeTypeT = SAFE_NO);
extern int genericChaseSpirits(TBeing *, TBeing *, int, immortalTypeT, safeTypeT = SAFE_NO);
extern bool file_to_sstring(const char *name, sstring &buf, concatT concat = CONCAT_NO);
extern const char *skill_diff(byte);
extern immuneTypeT getTypeImmunity(spellNumT type);
extern TPCorpse *pc_corpse_list;
#ifdef SUN
extern void bzero(char *, int);
extern int mkdir(const char *, short unsigned int);
extern int strcasecmp(char *, char *);
extern int setitimer(int, struct itimerval *, struct itimerval *);
extern int gettimeofday(struct timeval *, struct timezone *);
extern int sigsetmask(int);
extern long lrand48(void);
extern int strncasecmp(char *, char *, int);
#endif

#ifndef _CRYPT_H
#if defined(LINUX) || defined(SUN) || defined(SOLARIS)
extern char *crypt(const char *, const char *);
#endif
#endif

#if defined(SUN)
extern int bind(int, const struct sockaddr *, int namelen);
extern int gethostname(char *, int);
extern int accept(int, struct sockaddr *, int *);
extern int getpeername(int, struct sockaddr *, int *);
extern int getsockname(int, struct sockaddr *, int *);
extern int listen(int, int);
extern int socket(int, int, int);
#endif

#if defined(SUN)
extern int setsockopt(int, int, int, const char *, int);
extern int getsockopt(int, int, int, char *buf, int *);
#endif

void   doGlobalRoomStuff(void);
extern const int spec_skill_array[50];
unsigned int CountBits(unsigned int);
extern bool exit_ok(roomDirData *, TRoom **);
extern spellNumT searchForSpellNum(const sstring &arg, exactTypeT exact);
extern bool thingsInRoomVis(TThing *, TRoom *);
extern int get(TBeing *, TThing *, TThing *, getTypeT, bool);
extern void portal_flag_change(TPortal *, unsigned int, const char *, setRemT); 
extern const sstring numberAsString(int);
extern void readStringNoAlloc(FILE *);
extern void reset_zone(int, bool);
extern void loadsetCheck(TBeing *, int, int, wearSlotT, const sstring &);
extern void room_iterate(TRoom *[], void (*func) (int, TRoom *, sstring &, struct show_room_zone_struct *), sstring &, void *);
extern void do_where_thing(const TBeing *, const TThing *, bool, sstring &);
extern bool canSeeThruDoor(const roomDirData *);
extern bool hasDigit(char *);
extern void countAccounts(const char *arg);
extern int repair_number;
extern int tics;
extern void count_repair_items(const char *name);
extern int roomOfObject(const TThing *t);
extern char lcb[256];
extern int getWeather(int);
extern const sstring describeTime();
extern void mudTimePassed(time_t t2, time_t t1, time_info_data *);
extern void realTimePassed(time_t t2, time_t t1, time_info_data *);
extern void assign_item_info();
extern void assignTerrainInfo();
extern int gamePort;   // the port we are running on
extern void mud_assert(int, const char *,...);
extern int determineDissectionItem(TBaseCorpse *, int *, char *, char *, TBeing *);
extern int determineSkinningItem(TBaseCorpse *, int *, char *, char *);
extern struct attack_hit_type attack_hit_text[];
extern struct attack_hit_type attack_hit_text_twink[];
extern void processAllInput();
extern void setPrompts(fd_set);
extern void afterPromptProcessing(fd_set);
extern bool is_daytime();
extern bool is_nighttime();
extern wearSlotT mapFileToSlot(int);
extern int mapSlotToFile(wearSlotT);
extern positionTypeT mapFileToPos(int);
extern int mapPosToFile(positionTypeT);
extern itemTypeT mapFileToItemType(int);
extern int mapItemTypeToFile(itemTypeT);
extern bool notBreakSlot(wearSlotT, bool);
extern bool notBleedSlot(wearSlotT);
extern bool illegalEmail(char *, Descriptor *, silentTypeT);
extern const struct class_info classInfo[MAX_CLASSES];
extern const struct disc_names_data discNames[MAX_DISCS];
extern const struct racial_health_type racial_health[MAX_RACIAL_TYPES];
extern const ubyte slot_chance[MAX_BODY_TYPES][MAX_WEAR];
extern TBeing *FindTBeingDiffZoneSameRace(TBeing *ch);
extern int check_size_restrictions(const TBeing *ch, const TObj *o, wearSlotT slot, const TBeing *);
extern bool hitInnocent(const TBeing *, const TThing *, const TThing *);
extern double get_doubling_level(float);
extern double mob_exp(float);
extern int kills_to_level(int);
extern int secs_to_level(int);
extern double getExpClassLevel(classIndT, int);
extern int total_ac_obj(TObj *o);
extern char LOWER(char c);
extern char UPPER(char c);
extern char ISNEWL(char ch) ;
extern int combatRound(double);
extern int compareWeights(const float, const float);
extern void pissOff(TMonster *irritated, TBeing *reason);
extern int get_range_actual_damage(TBeing *ch, TBeing *victim, TObj *o, int dam, spellNumT attacktype);
extern void generic_sell(TBeing *ch, TMonster *keeper, TObj *obj, int shop_nr);
extern void generic_num_sell(TBeing *ch, TMonster *keeper, TObj *obj, int shop_nr, int num);
extern TRoom *room_find_or_create(int);
class TNote;
extern TNote *createNote(char *);
extern sstring secsToString(time_t num);
extern sstring talenDisplay(int);
extern sstring volumeDisplay(int);
extern TThing *unequip_char_for_save(TBeing *ch, wearSlotT pos);
extern bool isCritPart(wearSlotT);
extern bool hideThisSpell(spellNumT);
extern void test_fight_death(TBeing *, TBeing *, int);
extern sstring shutdown_or_reboot();
extern void raw_write_out_object(const TObj *o, FILE *fp, unsigned int vnum);
extern void bootPulse(const char *, bool = true);
extern void readDissectionFile();
extern void sendAutoTips();
extern int sstringncmp(const sstring, const sstring, unsigned int);
extern void generate_obj_index();
extern void generate_mob_index();
extern void generic_cleanup();
extern int listAccount(sstring, sstring &);
extern bool genericBless(TBeing *, TBeing *, int, bool);
extern bool genericDisease(TBeing *, int);
extern void genericCurse(TBeing *, TBeing *, int, spellNumT);
extern sstring displayDifficulty(spellNumT skill);
extern void generic_dirlist(const char *, const TBeing *);
extern int doLiqSpell(TBeing *, TBeing *, liqTypeT, int);
extern int doObjSpell(TBeing *, TBeing *, TMagicItem *, TObj *, const char *, spellNumT);
extern void checkGoldStats();
extern double getSkillDiffModifier(spellNumT);
extern void getSkillLevelRange(spellNumT, int &, int &, int);
extern int getSpellCost(spellNumT spell, int lev, int learn);
extern int getSpellCasttime(spellNumT spell);
extern void nukeLdead(TBeing *);
extern dirTypeT getDirFromChar(const sstring);
extern dirTypeT getDirFromCmd(cmdTypeT);
extern dirTypeT mapFileToDir(int);
extern int mapDirToFile(dirTypeT);
extern sectorTypeT mapFileToSector(int);
extern int mapSectorToFile(sectorTypeT);
extern discNumT mapFileToDisc(int);
extern int mapDiscToFile(discNumT);
extern applyTypeT mapFileToApply(int);
extern int mapApplyToFile(applyTypeT);
extern void processStringForClient(sstring &);
extern void repoCheck(TMonster *mob, int rnum);
extern void repoCheckForRent(TBeing *ch, TObj *obj, bool corpse);
extern double balanceCorrectionForLevel(double);
extern double getLevMod(ush_int, unsigned int);
extern unsigned int rent_credit(ush_int, unsigned int, unsigned int);
extern int levelLuckModifier(float);
extern bool isDissectComponent(int);
extern bool isInkComponent(int);
extern bool isBrewComponent(int);
extern wizPowerT mapFileToWizPower(int);
extern int mapWizPowerToFile(wizPowerT);
extern bool checkAttuneUsage(TBeing *, int *, int *, TVial **, TSymbol *);
extern const sstring getWizPowerName(wizPowerT); 
extern void setWizPowers(const TBeing *, TBeing *, const sstring &);
extern void remWizPowers(const TBeing *, TBeing *, const char *);
extern void assign_drink_types();
extern void assign_drug_info();
extern drugTypeT mapFileToDrug(int);
extern int mapDrugToFile(drugTypeT);
extern int mapSpellnumToFile(spellNumT);
extern spellNumT mapFileToSpellnum(int);
extern bool applyTypeShouldBeSpellnum(applyTypeT);
extern void stSpaceOut(sstring &);
extern bool has_key(TBeing *ch, int key);
extern int bogusAccountName(const char *arg);
extern const char *LimbHealth(double a);
extern doorTrapT mapFileToDoorTrap(int);
extern int mapDoorTrapToFile(doorTrapT);
extern int age_mod_for_stat(int age_num, statTypeT whichStat);
extern sstring describeDuration(const TBeing *, int);
extern bool is_ok(TMonster *, TBeing *, int);
extern int compareDetermineMessage(const int tDrift, const int tValue);
extern bool in_range(int, int, int);
extern int openQueue();
extern void closeQueue();
extern void mudSendMessage(int, int, const char *);
extern void recvTextHandler(const char *);
extern void mudRecvMessage();
extern const sstring RandomWord();
extern void calcBankInterest();
}

// these needs C++ linkage to avoid conflict with functions in stdlib
extern int remove(TBeing *, TThing *);
extern int atoi(const sstring &);
extern int atoi_safe(const sstring);
extern double atof_safe(const sstring);
extern int GetApprox(int, int);
extern double GetApprox(double, int);

#endif

