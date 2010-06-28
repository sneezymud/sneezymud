//////////////////////////////////////////////////////////////////////////
//
// SneezyMUD - All rights reserved, SneezyMUD Coding Team
//
//    "db.cc" - All functions and routines  related to tinyworld databases
//
//////////////////////////////////////////////////////////////////////////

#include "room.h"
#include "being.h"
#include "low.h"
#include "person.h"
#include "monster.h"
#include "configuration.h"
#include "guild.h"

#include <sys/types.h>
#include <dirent.h>
#include <cmath>

#include "socket.h"
#include "colorstring.h"
#include "statistics.h"
#include "help.h"
#include "mail.h"
#include "spec_mobs.h"
#include "obj_component.h"
#include "extern.h"
#include "loadset.h"
#include "sys_loot.h"
#include "shop.h"
#include "process.h"
#include "database.h"
#include "shopowned.h"
#include "obj_spellbag.h"
#include "obj_player_corpse.h"
#include "obj_open_container.h"
#include "obj_corpse.h"
#include "obj_quiver.h"
#include "obj_keyring.h"
#include "obj_chest.h"
#include "obj_bag.h"
#include "obj_base_container.h"
#include "obj_bow.h"
#include "obj_commodity.h"
#include "obj_symbol.h"
#include "obj_table.h"
#include "obj_fuel.h"
#include "obj_audio.h"
#include "obj_boat.h"
#include "obj_drug_container.h"
#include "obj_food.h"
#include "obj_money.h"
#include "obj_opal.h"
#include "obj_organic.h"
#include "obj_pen.h"
#include "obj_gas.h"
#include "obj_statue.h"
#include "obj_trash.h"
#include "obj_treasure.h"
#include "obj_bandaid.h"
#include "obj_bed.h"
#include "obj_board.h"
#include "obj_book.h"
#include "obj_component.h"
#include "obj_gemstone.h"
#include "obj_key.h"
#include "obj_note.h"
#include "obj_tool.h"
#include "obj_trap.h"
#include "obj_tree.h"
#include "obj_window.h"
#include "obj_portal.h"
#include "obj_arrow.h"
#include "obj_general_weapon.h"
#include "obj_flame.h"
#include "obj_light.h"
#include "obj_gun.h"
#include "obj_vial.h"
#include "obj_pool.h"
#include "obj_base_cup.h"
#include "obj_drinkcon.h"
#include "obj_armor.h"
#include "obj_armor_wand.h"
#include "obj_base_clothing.h"
#include "obj_jewelry.h"
#include "obj_other_obj.h"
#include "obj_potion.h"
#include "obj_scroll.h"
#include "obj_staff.h"
#include "obj_appliedsub.h"
#include "obj_wand.h"
#include "obj_worn.h"
#include "obj_plant.h"
#include "obj_cookware.h"
#include "obj_vehicle.h"
#include "obj_casino_chip.h"
#include "obj_poison.h"
#include "obj_handgonne.h"
#include "obj_egg.h"
#include "obj_cannon.h"
#include "obj_tooth_necklace.h"
#include "obj_trash_pile.h"
#include "liquids.h"
#include "obj_card_deck.h"
#include "obj_suitcase.h"
#include "obj_saddle.h"
#include "obj_harness.h"
#include "obj_saddlebag.h"
#include "obj_wagon.h"
#include "timing.h"
#include "obj_moneypouch.h"
#include "weather.h"
#include "obj_fruit.h"

int top_of_world = 0;         // ref to the top element of world 

TRoom *room_db[WORLD_SIZE];

TObjList object_list; // the global linked list of obj's 

int commod_index[200];

TBeing *character_list = 0; // global l-list of chars          
TMonster *pawnman = NULL;
TPCorpse *pc_corpse_list = NULL;
// table of reset data 
std::vector<zoneData>zone_table(0);

liqInfoT liquidInfo;
currencyInfoT currencyInfo;

int top_of_script = 0;

// Lots o' global variables!!!!!! - Russ 
long total_bc = 0;
long roomCount = 0;
long mobCount = 0;
long objCount = 0;
unsigned int help_used_num = 0;
unsigned int news_used_num = 0;
unsigned int wiznews_used_num = 0;
unsigned int wizlist_used_num = 0;
unsigned int total_deaths = 0;
unsigned int total_player_kills = 0;
unsigned int typo_used_num = 0;
unsigned int bug_used_num = 0;
unsigned int idea_used_num = 0;
int mob_tick_count = 0;
int repair_number = 0;
unsigned int total_help_number = 0;
int faction_number = 0;
// load rate percentage, overrides rates defined in zonefiles
int fixed_chance = 1;


const char * const File::SIGN_MESS = "/mud/sign/currentMess";
const char * const File::MOB       = "tinymob.use"; /* monster prototypes*/
const char * const File::ZONE      = "tinyworld.zon"; /* zone defs & command tables */
const char * const File::CREDITS   = "txt/credits";   /* for the credits command */
const char * const File::NEWS      = "txt/news";  /* for the 'news' command */
const char * const File::STORY     = "txt/story";     /* Stargazers pimpy story     */
const char * const File::WIZNEWS   = "txt/wiznews";
const char * const File::MOTD      = "txt/motd";      /* messages of today */
const char * const File::WIZMOTD   = "txt/wizmotd";   /* MOTD for immorts */
const char * const File::TIME      ="time"; /* game calendar information  */
const char * const File::IDEA      ="txt/ideas"; /* for the 'idea'-command */
const char * const File::TYPO      ="txt/typos";     /*         'typo'     */
const char * const File::BUG       ="txt/bugs";      /*         'bug'      */
const char * const File::SOCMESS   ="actions"; /* messgs for social acts     */
const char * const File::HELP_PAGE ="help/general";  /* for HELP <CR> */
const char * const File::WIZLIST   ="txt/wizlist";   /* for WIZLIST   */

const char * const Path::DATA          = "lib";  /* default data directory     */
const char * const Path::HELP	       ="help/";   /* for HELP <keywrd>          */
const char * const Path::IMMORTAL_HELP ="help/_immortal";
const char * const Path::BUILDER_HELP  ="help/_builder";
const char * const Path::SKILL_HELP    ="help/_skills";
const char * const Path::SPELL_HELP    ="help/_spells";



std::vector<TRoom *>roomspec_db(0);
std::vector<TRoom *>roomsave_db(0);
std::queue<sstring>queryqueue;


struct cached_object { int number;std::map <sstring, sstring> s; };
struct cached_mob_extra { int number;sstring keyword; sstring description; };
struct cached_mob_imm { int number;int type; int amt; };


class TObjectCache {
public:
  std::map<int, cached_object *>cache;

  void preload(void);
  cached_object *operator[](int);

} obj_cache;

class TMobileCache {
public:
  std::map<int, cached_object *>cache;
  std::map<int, std::vector <cached_mob_extra *> >extra;
  std::map<int, std::vector <cached_mob_imm *> >imm;

  void preload(void);
  cached_object *operator[](int);
} mob_cache;


bool bootTime=false;

class lag_data lag_info;

// count of the number of mobs that can load an object
std::map<int, int> obj_load_potential;

// local procedures
static void bootZones(void);
static void bootWorld(void);

// fish init
void initialize_fish_records(void);

struct reset_q_type
{
  resetQElement *head;
  resetQElement *tail;
} r_q;

void update_commod_index()
{
  TDatabase db(DB_SNEEZY);

  for(int i=0;i<200;++i)
    commod_index[i]=0;

  db.query("select r.material as material, sum(r.weight*10)/(select count(*) from shoptype st where st.type=%i) as units from rent r, obj o where o.vnum=r.vnum and r.owner_type='shop' and o.type=%i group by material", ITEM_RAW_MATERIAL, ITEM_RAW_MATERIAL);

  while(db.fetchRow()){
    commod_index[convertTo<int>(db["material"])]+=convertTo<int>(db["units"]);
  }
}

int getObjLoadPotential(const int obj_num)
{
  std::map<int, int>::iterator tIter;
  int obj_lp;

  tIter = obj_load_potential.find(obj_num);
  if (tIter != obj_load_potential.end()) {
    obj_lp = tIter->second;
  } else {
    obj_lp = 0;
  }
  return obj_lp;
}

void tallyObjLoadPotential(const int obj_num)
{
  std::map<int, int>::iterator tIter;

  tIter = obj_load_potential.find(obj_num);
  if (tIter != obj_load_potential.end()) {
    obj_load_potential[obj_num] = (tIter->second) + 1;
  } else {
    obj_load_potential[obj_num] = 1;
  }
}

void bootPulse(const char *str, bool end_str)
{
  /*  This function gets called periodically during bootup
    It basically will do socket stuff to bind new connections
    And sends some boot info to all descriptors.
    This is good for giving feedback during boots that might be long
    But be careful not to send immortal-only info  :)
  */

  Descriptor *d;
  sstring sc;
  static sstring tLastRealMessage("");
  bool tLRMN = false;

  if (str) {
    if (strcmp(str, ".")) {
      sc = MUD_NAME_VERS;
      sc += " Boot Process: ";

      // Set the last real output.
      tLastRealMessage  = sc;
      tLastRealMessage += str;
      tLRMN = true;
    } else
      sc = "";

    sc += str;

    if (end_str)
      sc += "\n\r";
  } else {
    sc += "\n\r";
  }

  gSocket->addNewDescriptorsDuringBoot((tLRMN ? "\n\r" : tLastRealMessage));

  for (d = descriptor_list; d; d = d->next) {
    d->output.putInQ(new UncategorizedComm(colorString(NULL, d, sc, NULL, COLOR_BASIC, TRUE)));
    d->outputProcessing();
  }

  if (str && strcmp(str, "."))
    vlogf(LOG_MISC, format("%s") %  str);
}

void object_stats()
{
  int count[MAX_OBJ_TYPES], i=0, li=0, total=0;

  for(i=0;i<MAX_OBJ_TYPES;++i)
    count[i]=0;
  
  for(TObjIter iter=object_list.begin();iter!=object_list.end();++iter)
    count[(*iter)->itemType()]++;
  
  // BUBBLESORT IS L33T!!!
  while(1){
    for(i=0;i<MAX_OBJ_TYPES;++i){
      if(count[i]>count[li])
        li=i;
    }
    
    if(count[li]==-1)
      break;
    
    vlogf(LOG_MISC, format("[%6i] %-17s") % count[li] % ItemInfo[li]->name);
    total += count[li];
    count[li]=-1;
  }
  
  vlogf(LOG_MISC, format("[%6i] %-17s") % total % "Total");
}


void assign_rooms()
{
  TDatabase db(DB_SNEEZY);

  db.query("select vnum from room where spec!=0");
  
  while(db.fetchRow()){
    TRoom *rp=real_roomp(convertTo<int>(db["vnum"]));
    roomspec_db.push_back(rp);
  }
}


void bootDb(void)
{
  TTiming t;
  t.start();

  bootTime=true;
  bootPulse("Boot db -- BEGIN.");

  vlogf(LOG_MISC, "Boot timing: begin");

  bootPulse("Resetting the game time.");
  GameTime::reset_time();

  bootPulse("Initializing game statistics.");
  if (!init_game_stats()) {
    vlogf(LOG_MISC, "bad result from init_game_stats");
//    exit(0);
  }
  bootPulse("Loading global toggles.");
  toggleInfo.loadToggles();

  bootPulse("Loading Races.");
  for(race_t rindex=RACE_NORACE;rindex<MAX_RACIAL_TYPES;rindex++)
    Races[rindex] = new Race(rindex);

  bootPulse("Initializing faction data.");
  if (!load_factions()) {
    vlogf(LOG_MISC, "Bad loading of factions.");
    exit(0);
  }
#if 1
  if (!load_guilds()) {
    vlogf(LOG_MISC, "Bad loading of new factions.");
    exit(0);
  }
#endif

  lockmess.erase();

  // keep this prior to object load
  bootPulse("Initializing Spells.");
  buildSpellArray();
  buildTerrainDamMap();
  buildWeatherDamMap();
//  buildSpellDamArray();
  bootPulse("Initializing Components.");
  buildComponentArray();
  bootPulse("Initializing Terrains.");
  assignTerrainInfo();

  vlogf(LOG_MISC, format("Boot timing: misc 1: %.2f seconds") % 
      (t.getElapsedReset()));

  bootPulse("Generating index tables for mobile file.");
  generate_mob_index();

  bootPulse("Generating index tables for object file.");
  generate_obj_index();

  vlogf(LOG_MISC, format("Boot timing: mob/obj indexes: %.2f seconds") % (t.getElapsedReset()));

  bootPulse("Pre-loading object cache.");
  obj_cache.preload();

  vlogf(LOG_MISC, format("Boot timing: obj cache: %.2f seconds") % (t.getElapsedReset()));

  bootPulse("Pre-loading mobile cache.");
  mob_cache.preload();

  vlogf(LOG_MISC, format("Boot timing: mob cache: %.2f seconds") % (t.getElapsedReset()));


  bootPulse("Building suitset information.");
  suitSets.SetupLoadSetSuits();

  bootPulse("Building help tables.");
  buildHelpIndex();

  bootPulse("Loading zone table.");
  bootZones();

  // must be done before loading objects
  bootPulse("Loading drug-type information.");
  assign_drug_info();

  bootPulse("Checking for new species of fish.");
  initialize_fish_records();
  
  vlogf(LOG_MISC, format("Boot timing: misc 2: %.2f seconds") % (t.getElapsedReset()));

  unsigned int i;
  bootPulse("Loading rooms:", false);
  bootWorld();
  bootPulse(NULL, true);

  vlogf(LOG_MISC, format("Boot timing: rooms: %.2f seconds") % (t.getElapsedReset()));

  vlogf(LOG_MISC, "Assigning function pointers:");
  vlogf(LOG_MISC, "   Shopkeepers.");
  bootTheShops();

  vlogf(LOG_MISC, format("Boot timing: shops: %.2f seconds") % (t.getElapsedReset()));

  //bootPulse("Initializing boards.");
  //InitBoards();
  
  bootPulse("Initializing room specials.");
  assign_rooms();

  bootPulse("Initializing command array.");
  buildCommandArray();

  // command array must be initialized before social messages
  bootPulse("Loading social messages.");
  bootSocialMessages();

  bootPulse("Loading periodic components.");
  assign_component_placement();

  bootPulse("Building item-type information.");
  assign_item_info();

  bootPulse("Building dissection information.");
  readDissectionFile();

  bootPulse("Building creation engine information.");
  SetupCreateEngineData();

  bootPulse("Building whittle information.");
  initWhittle();

  vlogf(LOG_MISC, format("Boot timing: misc 3: %.2f seconds") % (t.getElapsedReset()));

  bootPulse("Updating characters with saved items:", false);
  updateRentFiles();
  //bootPulse("Processing shop-save files.");
  //processShopFiles();
  bootPulse("Processing repair-save files.");
  processRepairFiles();
  bootPulse("Processing saved-room files.");
  updateSavedRoomItems();
  bootPulse("Processing corpse-save files.");
  processCorpseFiles();

  vlogf(LOG_MISC, format("Boot timing: save files: %.2f seconds") % (t.getElapsedReset()));

  bootPulse("Calculating number of items in rent.");
  vlogf(LOG_MISC, "Totals on limited items:");
  printLimitedInRent();

  bootPulse("Creating Loot List.");
  sysLootBoot();

  bootPulse("Calculating object load potentials:", false);
  for (i = 0; i < zone_table.size(); i++) {
    int d, e;
    d = (i ? (zone_table[i - 1].top + 1) : 0);
    e = zone_table[i].top;

    vlogf(LOG_MISC, format("Calculating object load potentials of %s (rooms %d-%d).") % zone_table[i].name % d % e);
    zone_table[i].resetZone(true, true);

    if (i%10 == 0)
      bootPulse(".", false);
  }
  bootPulse(NULL, true);

  vlogf(LOG_MISC, "Object load potentials:");
  std::map<int, int>::iterator tIter = obj_load_potential.begin();
  while (tIter != obj_load_potential.end()) {
    vlogf(LOG_MISC, format("VNum[%d] = %d") % tIter->first % tIter->second);
    ++tIter;
  }

  vlogf(LOG_MISC, format("Boot timing: load potentials: %.2f seconds") % (t.getElapsedReset()));

  for (i = 0; i < zone_table.size(); i++) {
    int d, e;
    d = (i ? (zone_table[i - 1].top + 1) : 0);
    e = zone_table[i].top;

    if (i==0) {
      // all shopkeepers should load in zone 0
      bootPulse("Loading shops.", false);
    } else if (i==1) {
      bootPulse(NULL, true);
      bootPulse("Resetting zones:", false);
      update_commod_index();
    }


    vlogf(LOG_MISC, format("Performing boot-time reset of %s (rooms %d-%d).") % zone_table[i].name % d % e);
    zone_table[i].resetZone(TRUE);

    // stagger reset times
    zone_table[i].age = ::number(0, zone_table[i].lifespan);

    if (i%10 == 0)
      bootPulse(".", false);
  }
  bootPulse(NULL, true);

  vlogf(LOG_MISC, format("Boot timing: zones and shop rent: %.2f seconds") % (t.getElapsedReset()));

  for (unsigned int mobnum = 0; mobnum < mob_index.size(); mobnum++) {
    for (unsigned int zone = 0; zone < zone_table.size(); zone++) {
      if(mob_index[mobnum].virt <= zone_table[zone].top)
        mob_index[mobnum].setMaxNumber(mob_index[mobnum].getNumber());
    }
  }


  bootPulse("Collecting object count statistics.");
  object_stats();

  // after boot time object loading is minimal, so the cache isn't needed
  //  obj_cache.cache.clear();

  bootPulse("Performing playerfile maintenance and data extraction:",false);
  fixup_players();
  
  bootPulse("Initializing light levels.");
  Weather::sunriseAndSunset();

  r_q.head = r_q.tail = 0;

  vlogf(LOG_MISC, format("Boot timing: pfiles: %.2f seconds") % (t.getElapsedReset()));

  bootPulse("Boot -- DONE.");
  bootTime=false;
}



// procUpdateTime
procUpdateTime::procUpdateTime(const int &p)
{
  trigger_pulse=p;
  name="procUpdateTime";
}

void procUpdateTime::run(const TPulse &) const
{
  return;
#if 0
  FILE *f1;
  long current_time;

  if (GameTime::getHours() != 1)
    return;

  f1 = fopen(TIME_FILE, "w");
  if (!f1) {
    perror("update time");
    exit(0);
  }
  current_time = time(0);
  vlogf(LOG_MISC, "Time update.");

  fprintf(f1, "#\n");

  fprintf(f1, "%ld\n", current_time);
  fprintf(f1, "%d\n", GameTime::getMinutes());
  fprintf(f1, "%d\n", GameTime::getHours());
  fprintf(f1, "%d\n", GameTime::getDay());
  fprintf(f1, "%d\n", GameTime::getMonth());
  fprintf(f1, "%d\n", time_info.year);

  fclose(f1);
#endif
}

void bootWorld(void)
{
  int virtual_nr, num=0, tmp;
  TRoom *rp=NULL;
  TDatabase db(DB_SNEEZY), db_extras(DB_SNEEZY), db_exits(DB_SNEEZY);
  extraDescription *new_descr;

  memset((char *) room_db, 0, sizeof(TRoom *) * WORLD_SIZE);
  character_list = NULL;

  db.query("select * from room order by vnum asc");
  db_exits.query("select vnum, direction, name, description, type, condition_flag, lock_difficulty, weight, key_num, destination from roomexit order by vnum asc");
  db_exits.fetchRow();
  db_extras.query("select * from roomextra order by vnum asc");  
  db_extras.fetchRow();

  while(db.fetchRow()){
    virtual_nr=convertTo<int>(db["vnum"]);

    if (virtual_nr/1000 > num) {
      num = virtual_nr/1000;
      vlogf(LOG_MISC, format("Room %ld allocated") %  (num*1000));
      bootPulse(".", false);
    } 
    allocate_room(virtual_nr);
    rp = real_roomp(virtual_nr);


    rp->setXCoord(convertTo<int>(db["x"]));
    rp->setYCoord(convertTo<int>(db["y"]));
    rp->setZCoord(convertTo<int>(db["z"]));
    rp->name=mud_str_dup(db["name"]);
    rp->setDescr(mud_str_dup(db["description"]));

    if (!zone_table.empty()) {
      //      fscanf(fl, " %*d ");  // this is the "zone" value - unused?
      unsigned int z;
      for (z = 0; rp->number>zone_table[z].top && z<zone_table.size(); z++);

      if (z >= zone_table.size()) {
        vlogf(LOG_EDIT, format("Room %d is outside of any zone.\n") % rp->number);
        exit(0);
      }
      rp->setZoneNum(z);
    }
    rp->setRoomFlags(convertTo<int>(db["room_flag"]));

    rp->setSectorType(mapFileToSector(convertTo<int>(db["sector"])));

    rp->setTeleTime(convertTo<int>(db["teleTime"]));
    rp->setTeleTarg(convertTo<int>(db["teleTarg"]));
    rp->setTeleLook(convertTo<int>(db["teleLook"]));
    
    rp->setRiverSpeed(convertTo<int>(db["river_speed"]));
    rp->setRiverDir(mapFileToDir(convertTo<int>(db["river_dir"])));
    rp->setMoblim(convertTo<int>(db["capacity"]));

    rp->setRoomHeight(convertTo<int>(db["height"]));

    rp->spec = convertTo<int>(db["spec"]);
    rp->setLight(0);
    rp->setHasWindow(0);

    rp->ex_description = NULL;

    // in case there are extras with no associated room, we need this
    while(convertTo<int>(db_extras["vnum"]) < rp->number)
      if(!db_extras.fetchRow())
        break;


    while(convertTo<int>(db_extras["vnum"]) == rp->number){
      new_descr = new extraDescription();
      new_descr->keyword = mud_str_dup(db_extras["name"]);
      if (!new_descr->keyword || !*new_descr->keyword)
        vlogf(LOG_EDIT, format("No keyword in room %d\n") %  rp->number);
      
      new_descr->description = mud_str_dup(db_extras["description"]);
      if (!new_descr->description || !*new_descr->description)
        vlogf(LOG_LOW, format("No desc in room %d\n") %  rp->number);
      
      new_descr->next = rp->ex_description;
      rp->ex_description = new_descr;

      if(!db_extras.fetchRow())
        break;
    }

    dirTypeT dir;
    for (dir = MIN_DIR; dir < MAX_DIR; dir++)
      rp->dir_option[dir] = 0;

    // in case there are exits with no associated room, we need this
    while(convertTo<int>(db_exits[0]) < rp->number)
      if(!db_exits.fetchRow())
        break;


    while(convertTo<int>(db_exits[0]) == rp->number){
      dir=mapFileToDir(convertTo<int>(db_exits[1]));

      rp->dir_option[dir] = new roomDirData();

      if(!db_exits[2].empty())
        rp->dir_option[dir]->keyword = mud_str_dup(db_exits[2]);
      else
        rp->dir_option[dir]->keyword = NULL;

      if(!db_exits[3].empty())
        rp->dir_option[dir]->description = mud_str_dup(db_exits[3]);
      else
        rp->dir_option[dir]->description = NULL;

      tmp=convertTo<int>(db_exits[4]);
      if (tmp < 0 || tmp >= MAX_DOOR_TYPES) {
        vlogf(LOG_LOW,format("bogus door type (%d) in room (%d) dir %d.") % 
            tmp % rp->number % dir);
        return;
      }
      rp->dir_option[dir]->door_type = doorTypeT(tmp);
      if ((tmp == DOOR_NONE) && (rp->dir_option[dir]->keyword)){
        if (strcmp(rp->dir_option[dir]->keyword, "_unique_door_"))
          vlogf(LOG_LOW,format("non-door with name in room %d") % rp->number);
      }
      if ((tmp != DOOR_NONE) && !(rp->dir_option[dir]->keyword)){
        vlogf(LOG_LOW,format("door with no name in room %d") % rp->number);
      }

      rp->dir_option[dir]->condition = convertTo<int>(db_exits[5]);
      rp->dir_option[dir]->lock_difficulty= convertTo<int>(db_exits[6]);
      rp->dir_option[dir]->weight= convertTo<int>(db_exits[7]);
      rp->dir_option[dir]->key = convertTo<int>(db_exits[8]);

      rp->dir_option[dir]->to_room = convertTo<int>(db_exits[9]);

      if (IS_SET(rp->dir_option[dir]->condition, EX_SECRET) && 
          canSeeThruDoor(rp->dir_option[dir])) {
        if (IS_SET(rp->dir_option[dir]->condition, EX_CLOSED)){
          //vlogf(LOG_LOW, format("See thru door set secret. (%d, %d)") %  room % dir);
        } else
          vlogf(LOG_LOW, format("Secret door saved as open. (%d, %d)") % 
              rp->number % dir);
      }
      if(!db_exits.fetchRow())
        break;
    }
    
    roomCount++;

#if 0
// modified weather stuff, not used yet, BAT
    rp->initWeather();
#endif

    if (rp->isRoomFlag(ROOM_SAVE_ROOM))
      rp->loadItems();

    if ((rp->number == Room::NOCTURNAL_STORAGE))
      continue;

    if (rp->isRoomFlag(ROOM_PEACEFUL) && !rp->isRoomFlag(ROOM_NO_HEAL))
      vlogf(LOG_LOW, format("%s room %d set peaceful && !no_heal (bit: %d)") % 
                rp->name %rp->number % ROOM_NO_HEAL);
    if (rp->isRoomFlag(ROOM_PEACEFUL) && !rp->isRoomFlag(ROOM_NO_STEAL))
      vlogf(LOG_LOW, format("%s room %d set peaceful && !no_steal (bit: %d)") % 
                rp->name %rp->number % ROOM_NO_STEAL);
    if (rp->isRoomFlag(ROOM_PEACEFUL) && !rp->isRoomFlag(ROOM_NO_MAGIC))
      vlogf(LOG_LOW, format("%s room %d set PEACEFUL && !no_magic (bit: %d)") % 
                rp->name %rp->number % ROOM_NO_MAGIC);
    if (rp->isRoomFlag(ROOM_NO_HEAL) && rp->isRoomFlag(ROOM_HOSPITAL))
      vlogf(LOG_LOW, format("%s room %d set NO_HEAL(%d) and HOSPITAL(%d)") % 
                rp->name %rp->number % ROOM_NO_HEAL % ROOM_HOSPITAL);

    if (rp->isIndoorSector() && !rp->isRoomFlag(ROOM_INDOORS)) {
      // in general, this is an error
      // of course you could have a bldg whose roof has collapsed...
      if (rp->number != 27349)
        vlogf(LOG_LOW,format("%s room %d set building & !indoor") % 
                rp->name %rp->number);
    }
    if (rp->isRoomFlag(ROOM_INDOORS) && rp->getRoomHeight() <= 0)
      vlogf(LOG_LOW,format("%s indoor room %d set with unlimited height") % 
                rp->name %rp->number);
    if (!rp->isRoomFlag(ROOM_INDOORS) && rp->getRoomHeight() >= 0)
      vlogf(LOG_LOW,format("%s outdoor room %d set with limited height") % 
                rp->name %rp->number);

#if 0
    if ((rp->getRoomHeight() >= 0) && rp->isFallSector())
      vlogf(LOG_LOW,format("%s fall room %d set with limited height") % 
                rp->name %rp->number);
#endif
  }
}


void TRoom::colorRoom(int title, int full)
{
  int len, place = 0, letter;
  sstring buf, argument, buf2, buf3;

  if (title == 1) {
    argument=name;
  } else if (title == 2) {
    argument=getDescr();
  } else {
    return;
  }
// Found had to initialize with this logic and too tired to figure out why
  buf3="<z>";

  switch (getSectorType()) {
    case SECT_SUBARCTIC:
      buf2 = "<P>";
      buf3 = "<p>";
      break;
    case SECT_ARCTIC_WASTE:
      buf2 = "<w>";
      buf3 = "<W>";
      break;
    case SECT_ARCTIC_CITY:
      buf2 = "<C>";
      break;
    case SECT_ARCTIC_ROAD:
      buf2 = "<W>";
      break;
    case SECT_TUNDRA:
      buf2 = "<o>";
      buf3 = "<g>";
      break;
    case SECT_ARCTIC_MOUNTAINS:
      buf2 = "<o>";
      buf3 = "<W>";
      break;
    case SECT_ARCTIC_FOREST:
      buf2 = "<G>";
      buf3 = "<W>";
      break;
    case SECT_ARCTIC_MARSH:
      buf2 = "<B>";
      buf3 = "<p>";
      break;
    case SECT_ARCTIC_RIVER_SURFACE:
      buf2 = "<C>";
      buf3 = "<c>";
      break;
    case SECT_ICEFLOW:
      buf2 = "<C>";
      buf3 = "<W>";
      break;
    case SECT_COLD_BEACH:
      buf2 = "<p>";
      buf3 = "<P>";
      break;
    case SECT_SOLID_ICE:
      buf2 = "<c>";
      buf3 = "<C>";
      break;
    case SECT_ARCTIC_BUILDING:
      buf2 = "<p>";
      break;
    case SECT_ARCTIC_CAVE:
      buf2 = "<c>";
      buf3 = "<k>";
      break;
    case SECT_ARCTIC_ATMOSPHERE:
      buf2 = "<C>";
      buf3 = "<C>";
      break;
    case SECT_ARCTIC_CLIMBING:
    case SECT_ARCTIC_FOREST_ROAD:
      buf2 = "<p>";
    case SECT_PLAINS:
      buf2 = "<G>";
      buf3 = "<g>";
      break;
    case SECT_TEMPERATE_CITY:
    case SECT_TEMPERATE_ROAD:
      buf2 = "<p>";
      break;
    case SECT_GRASSLANDS:
      buf2 = "<G>";
      buf3 = "<g>";
      break;
    case SECT_TEMPERATE_HILLS:
      buf2 = "<o>";
      buf3 = "<g>";
      break;
    case SECT_TEMPERATE_MOUNTAINS:
      buf2 = "<G>";
      buf3 = "<o>";
      break;
    case SECT_TEMPERATE_FOREST:
      buf2 = "<G>";
      buf3 = "<g>";
      break;
    case SECT_TEMPERATE_SWAMP:
      buf2 = "<P>";
      buf3 = "<p>";
      break;
    case SECT_TEMPERATE_OCEAN:
      buf2 = "<C>";
      buf3 = "<c>";
      break;
    case SECT_TEMPERATE_RIVER_SURFACE:
      buf2 = "<B>";
      buf3 = "<b>";
      break;
    case SECT_TEMPERATE_UNDERWATER:
      buf2 = "<C>";
      buf3 = "<b>";
      break;
    case SECT_TEMPERATE_CAVE:
      buf2 = "<o>";
      buf3 = "<k>";
      break;
    case SECT_TEMPERATE_ATMOSPHERE:
      buf2 = "<G>";
      break;
    case SECT_TEMPERATE_CLIMBING:
      buf2 = "<G>";
      break;
    case SECT_TEMPERATE_FOREST_ROAD:
      buf2 = "<g>";
      break;
    case SECT_DESERT:
    case SECT_SAVANNAH:
      buf2 = "<y>";
      buf3 = "<o>";
      break;
    case SECT_VELDT:
      buf2 = "<g>";
      buf3 = "<o>";
      break;
    case SECT_TROPICAL_CITY:
      buf2 = "<G>";
      break;
    case SECT_TROPICAL_ROAD:
      buf2 = "<g>";
      break;
    case SECT_JUNGLE:
      buf2 = "<P>";
      buf3 = "<g>";
      break;
    case SECT_RAINFOREST:
      buf2 = "<G>";
      buf3 = "<g>";
      break;
    case SECT_TROPICAL_HILLS:
      buf2 = "<R>";
      buf3 = "<g>";
      break;
    case SECT_TROPICAL_MOUNTAINS:
      buf2 = "<P>";
      buf3 = "<p>";
      break;
    case SECT_VOLCANO_LAVA:
      buf2 = "<y>";
      buf3 = "<R>";
      break;
    case SECT_TROPICAL_SWAMP:
      buf2 = "<G>";
      buf3 = "<g>";
      break;
    case SECT_TROPICAL_OCEAN:
      buf2 = "<b>";
      buf3 = "<c>";
      break;
    case SECT_TROPICAL_RIVER_SURFACE:
      buf2 = "<C>";
      buf3 = "<B>";
      break;
    case SECT_TROPICAL_UNDERWATER:
      buf2 = "<B>";
      buf3 = "<b>";
      break;
    case SECT_TROPICAL_BEACH:
      buf2 = "<P>";
      buf3 = "<y>";
      break;
    case SECT_TROPICAL_BUILDING:
      buf2 = "<p>";
      break;
    case SECT_TROPICAL_CAVE:
      buf2 = "<P>";
      buf3 = "<k>";
      break;
    case SECT_TROPICAL_ATMOSPHERE:
      buf2 = "<P>";
      break;
    case SECT_TROPICAL_CLIMBING:
      buf2 = "<P>";
      break;
    case SECT_RAINFOREST_ROAD:
      buf2 = "<P>";
      break;
    case SECT_ASTRAL_ETHREAL:
      buf2 = "<C>";
      buf3 = "<c>";
      break;
    case SECT_SOLID_ROCK:
      buf2 = "<k>";
      buf3 = "<w>";
      break;
    case SECT_FIRE:
      buf2 = "<y>";
      buf3 = "<R>";
      break;
    case SECT_INSIDE_MOB:
      buf2 = "<R>";
      buf3 = "<r>";
      break;
    case SECT_FIRE_ATMOSPHERE:
      buf2 = "<y>";
      buf3 = "<R>";
      break;
    case SECT_DEAD_WOODS:
      buf2 = "<k>";
      buf3 = "<k>";
      break;
    case MAX_SECTOR_TYPES:
    case SECT_TEMPERATE_BEACH:
    case SECT_TEMPERATE_BUILDING:
    case SECT_MAKE_FLY:
      buf2 = "<p>";
      break;
  }
  buf="";
  if (title == 1) {
    if (!buf2.empty()) {
      buf = buf2;
      place = 3;
    }
  } else if (title == 2) {
    if (!buf3.empty()) {
      buf = buf3;
      place = 3;
    }
  }

  len = argument.length();
  for(letter=0; letter <= len; letter++, place++) {
    if (letter < 2) {
      buf[place] = argument[letter];
      continue; 
    }
    if ((argument[letter] == '>') && (argument[letter - 2] == '<')) {
      switch (argument[(letter - 1)]) {
        case '1':
        case 'z':
        case 'Z':
          buf[place] = argument[letter];
          if (title == 1) {
            if (!buf2.empty()) {
              buf += buf2;
              place +=3;
            }
          } else if (title == 2) {
            if (!buf3.empty()) {
              buf += buf3;
              place +=3;
            }
          } else {
          }
          break;
        default:
          buf[place] = argument[letter];
          break;
      }
    } else {
      buf[place] = argument[letter];
    }
  }


  buf += "<1>";
  if (title == 1) {
    delete [] name;
    name = mud_str_dup(buf);
  } else if (title == 2) {
    delete [] getDescr();
    setDescr(mud_str_dup(buf));
  }
  full = 1;
  return;
}

void allocate_room(int room_number)
{
  if (room_number > top_of_world)
    top_of_world = room_number;

  room_find_or_create(room_number);
}


void setup_dir(FILE * fl, int room, dirTypeT dir, TRoom *tRoom)
{
  int tmp;
  TRoom *rp;

  if (!(rp = real_roomp(room)) && !(rp = tRoom)) {
    vlogf(LOG_MISC, format("Setup_dir called with bad room number %d") %  room);
    return;
  }
  rp->dir_option[dir] = new roomDirData();

  rp->dir_option[dir]->description = fread_string(fl);
  rp->dir_option[dir]->keyword = fread_string(fl);

  fscanf(fl, " %d ", &tmp);
  if (tmp < 0 || tmp >= MAX_DOOR_TYPES) {
    vlogf(LOG_LOW,format("bogus door type (%d) in room (%d) dir %d.") % 
        tmp % room % dir);
    return;
  }
  rp->dir_option[dir]->door_type = doorTypeT(tmp);
  if ((tmp == DOOR_NONE) && (rp->dir_option[dir]->keyword)){
    if (strcmp(rp->dir_option[dir]->keyword, "_unique_door_"))
      vlogf(LOG_LOW,format("non-door with name in room %d") % room);
  }
  if ((tmp != DOOR_NONE) && !(rp->dir_option[dir]->keyword)){
    vlogf(LOG_LOW,format("door with no name in room %d") % room);
  }

  fscanf(fl, " %d ", &tmp);
  rp->dir_option[dir]->condition = tmp;
  fscanf(fl, " %d ", &tmp);
  rp->dir_option[dir]->lock_difficulty= tmp;
  fscanf(fl, " %d ", &tmp);
  rp->dir_option[dir]->weight= tmp;
  fscanf(fl, " %d ", &tmp);
  rp->dir_option[dir]->key = tmp;

  fscanf(fl, " %d ", &tmp);
  rp->dir_option[dir]->to_room = tmp;

  if (IS_SET(rp->dir_option[dir]->condition, EX_SECRET) && 
      canSeeThruDoor(rp->dir_option[dir])) {
    if (IS_SET(rp->dir_option[dir]->condition, EX_CLOSED)){
      //      vlogf(LOG_LOW, format("See thru door set secret. (%d, %d)") %  room % dir);
    } else
      vlogf(LOG_LOW, format("Secret door saved as open. (%d, %d)") %  room % dir);
  }


}


void zoneData::logError(char ch, const char *type, int cmd, int value)
{
  vlogf(LOG_LOW, format("zone %s cmd %d (%c) resolving %s number (%d)") % 
      name % cmd % ch % type % value);
}

void zoneData::renumCmd(void)
{
  int comm;
  int value;

  // init the zone_value array
  if(Config::NukeInactiveMobs())
    zone_value = ((zone_nr <= 1) ? -1 : 0);
  else
    zone_value = -1;
  
  // clear the stat reporting maps
  stat_mobs.clear();
  stat_objs.clear();
  int argbuf;
  
  for (comm = 0; cmd[comm].command != 'S'; comm++) {
    resetCom *rs = &cmd[comm];
    switch (rs->command) {
      case 'A':
        if (rs->arg1 < 0 || rs->arg1 >= WORLD_SIZE)
          logError('A', "room 1",comm, rs->arg1);
        if (rs->arg2 < 0 || rs->arg2 >= WORLD_SIZE)
          logError('A', "room 2",comm, rs->arg2);
        if (rs->arg2 <= rs->arg1)
          logError('A', "no overlap",comm, rs->arg2);
        break;
      case 'C':
        rs->arg1 = real_mobile(value = rs->arg1);
        if (rs->arg1 < 0)
          logError('C', "mobile",comm, value);
          else 
            ++stat_mobs[rs->arg1];
        if (rs->arg3 < 0 && rs->arg3 != ZONE_ROOM_RANDOM)
          logError('C', "room",comm, rs->arg3);
          
        break;
      case 'K':
        rs->arg1 = real_mobile(value = rs->arg1);
        if (rs->arg1 < 0)
          logError('K', "mobile",comm, value);
          else 
            ++stat_mobs[rs->arg1];
        if (rs->arg3 < 0 && rs->arg3 != ZONE_ROOM_RANDOM)
          logError('K', "room",comm, rs->arg3);
        break;
      case 'M':
        rs->arg1 = real_mobile(value = rs->arg1);
        if (rs->arg1 < 0)
          logError('M', "mobile",comm, value);
          else 
            ++stat_mobs[rs->arg1];
        if (rs->arg3 < 0 && rs->arg3 != ZONE_ROOM_RANDOM)
          logError('M', "room",comm, rs->arg3);
        break;
      case 'R':
        rs->arg1 = real_mobile(value = rs->arg1);
        if (rs->arg1 < 0)
          logError('R', "mobile",comm, value);
          else 
            ++stat_mobs[rs->arg1];
        if (rs->arg3 < 0 && rs->arg3 != ZONE_ROOM_RANDOM)
          logError('R', "room",comm, rs->arg3);
        break;
      case 'O':
        rs->arg1 = real_object(value = rs->arg1);
        if (rs->arg1 < 0)
          logError('O', "object",comm, value);
          else 
            ++stat_objs[rs->arg1];
        if (rs->arg3 < 0 && rs->arg3 != ZONE_ROOM_RANDOM)
          logError('O', "room",comm, rs->arg3);
        break;
      case 'G':
        rs->arg1 = real_object(value = rs->arg1);
        if (rs->arg1 < 0)
          logError('G', "object",comm, value);
          else 
            ++stat_objs[rs->arg1];
        break;
      case 'X': // X <set num> <slot> <vnum>
        if (rs->arg3 < 0 || rs->arg3 > 15)
          logError('X', "macro",comm, rs->arg2);
        rs->arg1 = mapFileToSlot(value = rs->arg1); 
        if (rs->arg1 < MIN_WEAR || rs->arg1 >= MAX_WEAR)
          logError('X', "bogus slot",comm, value);
          argbuf = real_object(value = rs->arg2);
          if (argbuf >= 0)
            ++stat_objs[argbuf];
        break;
      case 'Z': // Z <if flag> <set num> <perc chance>
        if (rs->arg1 < 0 || rs->arg1 > 15)
          logError('Z', "macro",comm, rs->arg3);
        if (rs->arg2 <= 0 || rs->arg2 > 100)
          logError('Z', "percent",comm, rs->arg2);
        break;
        // Add one for each suit load ..loadset
      case 'Y':
        if (rs->arg1 <= 0 || rs->arg1 > (signed) suitSets.suits.size())
          logError('Y', "macro",comm, rs->arg1);
        if (rs->arg2 <= 0 || rs->arg2 > 100)
          logError('Y', "percent",comm, rs->arg2);
        break;
      case 'E':
        rs->arg1 = real_object(value = rs->arg1);
        if (rs->arg1 < 0)
          logError('E', "object",comm, value);
          else 
            ++stat_objs[rs->arg1];
        rs->arg3 = mapFileToSlot(value = rs->arg3); 
        if (rs->arg3 < MIN_WEAR || rs->arg3 >= MAX_WEAR)
          logError('E', "bogus slot",comm, value);
        break;
      case 'P':
        rs->arg1 = real_object(value = rs->arg1);
        if (rs->arg1 < 0)
          logError('P', "object",comm, value);
          else 
            ++stat_objs[rs->arg1];
        rs->arg3 = real_object(rs->arg3);
        if (rs->arg3 < 0)
          logError('P', "container",comm, rs->arg3);
        break;
      case 'D':
        if (rs->arg1 < 0)
          logError('D', "room",comm, rs->arg1);
        break;
      case 'B':
        rs->arg1 = real_object(value = rs->arg1);
        if (rs->arg1 < 0)
          logError('B', "object",comm, value);
          else 
            ++stat_objs[rs->arg1];
        if (rs->arg3 < 0 && rs->arg3 != ZONE_ROOM_RANDOM)
          logError('B', "room",comm, rs->arg3);
        break;
      case 'H':
        if (rs->arg1 < MIN_HATE || rs->arg1 >= MAX_HATE) {
          logError('H', "hate",comm, rs->arg1);
          rs->arg1 = MIN_HATE;
        }
        break;
      case 'F':
        if (rs->arg1 < MIN_HATE || rs->arg1 >= MAX_HATE) {
          logError('F', "fear",comm, rs->arg1);
          rs->arg1 = MIN_HATE;
        }
        break;
    }
  }
  
  // set the object and mob tallies in zoneData
  std::map<int,int>::iterator iter;
  stat_mobs_total = 0;
  stat_mobs_unique = 0;
  for (iter = stat_mobs.begin(); iter != stat_mobs.end(); iter++ ) {
    ++stat_mobs_unique;
    stat_mobs_total += iter->second;
  }
  stat_objs_unique = 0;
  for (iter = stat_objs.begin(); iter != stat_objs.end(); iter++ ) {
    ++stat_objs_unique;
  }
}

void TBeing::doBoot(const sstring &arg)
{
  int z=0;
  TThing *t;
  TMonster *mob;
  TObj *obj;
  bool found=true;


  if(arg.word(0)=="zone")
    z=convertTo<int>(arg.word(1));

  if(!z){
    sendTo("Usage: boot zone <zone_nr>\n\r");
    return;
  }

  // reset allows global use
  if(!hasWizPower(POWER_RESET)){
    // otherwise they need zonefile power and to be resetting their zone
    if (!hasWizPower(POWER_ZONEFILE_UTILITY)) {
      sendTo("You have not been given this power yet.\n\r");
      return;
    }

    // ok, check if they're resetting their zone
    if((desc->blockastart != zone_table[z].bottom
        || desc->blockaend != zone_table[z].top)
        && (desc->blockbstart != zone_table[z].bottom
        || desc->blockbend != zone_table[z].top)){
      sendTo("You can only boot your own zones.\n\r");
      return;      
    }
  }


  sendTo(format("Rebooting zone %s (%i) (rooms %i-%i)\n\r")
      % zone_table[z].name % z % zone_table[z].bottom % zone_table[z].top);

  sendTo("Reloading zonefile.\n\r");
  zone_table[z].bootZone(zone_table[z].bottom);
  sendTo("Renumbering loads (vnum->rnum).\n\r");
  zone_table[z].renumCmd();

  sendTo("Purging mobs and objects in zone.\n\r");
  while(found){
    found=false;
    for(int r=zone_table[z].bottom;r<=zone_table[z].top;++r){
      if(real_roomp(r)){
        for(StuffIter it=real_roomp(r)->stuff.begin();it!=real_roomp(r)->stuff.end();){
          t=*(it++);
          
          if((obj=dynamic_cast<TObj *>(t))){
            if(obj->objVnum() >= zone_table[z].bottom ||
               obj->objVnum() <= zone_table[z].top){
              found=true;
              delete obj;
            }
          } else if((mob=dynamic_cast<TMonster *>(t))){
            if(mob->mobVnum() >= zone_table[z].bottom ||
               mob->mobVnum() <= zone_table[z].top){
              found=true;
              delete mob;
            }
          }
        }
      }
    }
  }

  sendTo("Boottime resetting zone.\n\r");
  bool enabled=zone_table[z].enabled;
  zone_table[z].enabled=true;
  zone_table[z].resetZone(true);
  zone_table[z].enabled=enabled;
}




bool zoneData::bootZone(int zone_nr)
{
  int tmp;
  char *check, buf[256];
  int i1 = 0, i2, i3, i4;
  int rc;
  FILE *fl=fopen(((sstring)(format("zonefiles/%i") % zone_nr)).c_str(), "r");

  if (!fl) {
    perror("bootZone");
    return false;
  }

  fscanf(fl, " #%d\n", &bottom);
  check = fread_string(fl);
  
  name = check;
  rc = fscanf(fl, " %d %d %d %d", &i1, &i2, &i3, &i4);
  if (rc == 4) {
    top = i1;
    lifespan = i2;
    reset_mode = i3;
    enabled = i4;
    age = 0;
  } else { 
    vlogf(LOG_LOW, format("Bad zone format for zone %d (%s)") % zone_nr % check);
    return false;
  }

  cmd.erase(cmd.begin(), cmd.end());

  for (;;) {
    resetCom rs;
    
    fscanf(fl, " ");                
    fscanf(fl, "%c", &rs.command);
    
    if (rs.command == 'S') {
      cmd.push_back(rs);
      break;
    }

    if (rs.command == '*' || gamePort == Config::Port::GAMMA) {
      fgets(buf, 255, fl);        
      continue;
    }


    int numc = fscanf(fl, " %d %d %d", &tmp, &rs.arg1, &rs.arg2);
    if (numc != 3)
      vlogf(LOG_LOW,format("command %u ('%c') in %s missing some of first three args [%d : %d %d %d]") % 
          cmd.size() %
          rs.command %
          name %
          numc %
          (numc >= 1 ? tmp : -99) %
          (numc >= 2 ? rs.arg1 : -99) %
          (numc >= 3 ? rs.arg2 : -99));

    if(rs.command=='X')
      rs.arg3 = tmp;
    else
      rs.if_flag = tmp;
      
    switch (rs.command) {
      case 'G':
      case 'P':
      case 'E':
        if (!rs.if_flag) {
          vlogf(LOG_LOW,format("command %u in %s has bogus if_flag") % 
          cmd.size() %name);
          continue;
        }
        break;
      default:
        break;
    }

    if (rs.command == 'M' ||
        rs.command == 'O' ||
        rs.command == 'B' ||
        rs.command == 'C' ||
        rs.command == 'K' ||
        rs.command == 'E' ||
        rs.command == 'P' ||
        (rs.command == 'T' && !rs.if_flag) ||
        rs.command == 'R' ||
        rs.command == 'D' ||
        rs.command == 'L')
      if ((rc = fscanf(fl, " %d", &rs.arg3)) != 1)
        vlogf(LOG_LOW,format("command %u ('%c') in %s missing arg3 (rc=%d)") % 
            cmd.size() %
            rs.command %
            name % rc);

    if (rs.command == '?')
      if (fscanf(fl, " %c", &rs.character) != 1)
        vlogf(LOG_LOW,format("command %u ('?') in %s missing character") % cmd.size() %name);

    if (rs.command == 'T' && !rs.if_flag) 
      if (fscanf(fl, " %d", &rs.arg4) != 1)
        vlogf(LOG_LOW,format("command %u ('T') in %s missing arg4") % 
            cmd.size() % name);

    if (rs.command == 'L')
      if (fscanf(fl, " %d", &rs.arg4) != 1)
        vlogf(LOG_LOW, format("command %u ('L') in %s missing arg4") % 
            cmd.size() % name);
    
    cmd.push_back(rs);

    fgets(buf, 255, fl);        
  }

  fclose(fl);

  return true;
}

void bootZones(void)
{
  DIR *dfd;
  struct dirent *dp;
  int zon=0, tmp;
  std::multimap <int, sstring, std::less<int> > files;
  std::multimap <int, sstring, std::less<int> >::iterator it;
  TDatabase db(DB_SNEEZY);
  
  if(!(dfd=opendir("zonefiles"))){
    vlogf(LOG_BUG, "couldn't open zonefiles directory");
    perror("bootZones");
    exit(0);
  }
  
  while ((dp = readdir(dfd))) {
    if(!strcmp(dp->d_name, ".") ||
       !strcmp(dp->d_name, ".."))
      continue;

    tmp=convertTo<int>(dp->d_name);

    // convertTo returns 0 on failure, so this means some random file
    // in the zonefiles directory.  Just ignore it.
    if(tmp==0 && strcmp(dp->d_name, "0"))
      continue;

    files.insert(std::pair<int,sstring>(tmp, dp->d_name));
  }
  
  db.query("update zone set util_flag = 0");
  for(it=files.begin();it!=files.end();++it){
    zoneData zd;
    if(zd.bootZone((*it).first)){
      zd.renumCmd();
      vlogf(LOG_MISC, format("booting zone %d") % zon);
      zd.zone_nr=zon++;
      // note that a zone's zone_nr may change over time if a new zone is inserted before it
      // so update all records in the zone table
      db.query("update zone set zone_name = '%s', zone_enabled = %i, bottom = %i, top = %i, reset_mode = %i, lifespan = %i, util_flag = 1 where zone_nr = %i", zd.name, (zd.enabled ? 1 : 0), zd.bottom, zd.top, zd.reset_mode, zd.lifespan, zd.zone_nr);
      if (db.rowCount() == 0) {
        // unsuccessful update, do an insert
        db.query("insert zone (zone_nr, zone_name, zone_enabled, bottom, top, reset_mode, lifespan, util_flag) select %i, '%s', %i, %i, %i, %i, %i, 1", zd.zone_nr, zd.name, (zd.enabled ? 1 : 0), zd.bottom, zd.top, zd.reset_mode, zd.lifespan);
      }
      zone_table.push_back(zd);
    }
  }
  // trim off any extra entries (zones have been removed since last boot, presumably)
  db.query("delete from zone where util_flag = 0");
}




TMonster *read_mobile(int nr, readFileTypeT type)
{
  int i, rc, virt=nr;
  TMonster *mob = NULL;

  i = nr;

  if (type == VIRTUAL) {
    nr = real_mobile(nr);
  } else {
    virt = mob_index[nr].virt;
  }

  if (nr < 0) {
    vlogf(LOG_FILE, format("Mobile (V) %d does not exist in database.") %  i);
    return NULL;
  }

  try {
    mob = new TMonster();
  } catch (...) {
    vlogf(LOG_BUG, "caught an exception in read_mobile");
    return NULL;
  }
  mob->number = nr;

  // do this here to avoid the 'deleted & not found in character_list' assertion 
  // in ~TMonster if readMobFromDB fails or returns delete
  mob->next = character_list;
  character_list = mob;
  
  rc = mob->readMobFromDB(virt, FALSE);
  if (IS_SET_DELETE(rc, DELETE_THIS)) {
    vlogf(LOG_BUG, format("Mobile %d returned DELETE_THIS on init.") %  virt);
    delete mob;
    return NULL;
  } else if (!rc) {
    vlogf(LOG_BUG, format("Mobile %d failed to load from database.") %  virt);
    delete mob;
    return NULL;
  }

  mob->loadResponses( mob_index[nr].virt);

  mob->setCombatMode(ATTACK_NORMAL);

  if ((mob->getRace() == RACE_HORSE) || (mob->getRace() == RACE_DRAGON)) {
    mob->setMaxMove(mob->getMaxMove() + 150);
    mob->setMove(mob->moveLimit());
  }
  mob->convertAbilities();
  statTypeT ij;
  for (ij = MIN_STAT; ij < MAX_STATS_USED; ij++)
    mob->setStat(STAT_CURRENT, ij, mob->getStat(STAT_NATURAL, ij));

  mob->checkMobStats(TINYFILE_YES);

  mob->setRacialStuff();

  wearSlotT j;
  for (j = MIN_WEAR; j < MAX_WEAR; j++) {
    mob->setLimbFlags(j, 0);
    mob->setCurLimbHealth(j, mob->getMaxLimbHealth(j));
    mob->setStuckIn(j, NULL);
  }
  mob_index[nr].addToNumber(1);

  if (mob->GetMaxLevel() >= 70) {
    if (!::number(0, 2)) {
      // Make all mobs level 70+ have TS automatically 1/3 of the time
      SET_BIT(mob->specials.affectedBy, AFF_TRUE_SIGHT);
    }
    if (!::number(0, 2)) {
      // Make all mobs level 70+ have DI automatically 1/3 of the time
      SET_BIT(mob->specials.affectedBy, AFF_DETECT_INVISIBLE);
    }
  }

  if (mob->GetMaxLevel() <= 5)
    stats.act_1_5++;
  else if (mob->GetMaxLevel() <= 10)
    stats.act_6_10++;
  else if (mob->GetMaxLevel() <= 15)
    stats.act_11_15++;
  else if (mob->GetMaxLevel() <= 20)
    stats.act_16_20++;
  else if (mob->GetMaxLevel() <= 25)
    stats.act_21_25++;
  else if (mob->GetMaxLevel() <= 30)
    stats.act_26_30++;
  else if (mob->GetMaxLevel() <= 40)
    stats.act_31_40++;
  else if (mob->GetMaxLevel() <= 50)
    stats.act_41_50++;
  else if (mob->GetMaxLevel() <= 60)
    stats.act_51_60++;
  else if (mob->GetMaxLevel() <= 70)
    stats.act_61_70++;
  else if (mob->GetMaxLevel() <= 100)
    stats.act_71_100++;
  else
    stats.act_101_127++;

  rc = mob->checkSpec(mob, CMD_GENERIC_CREATED, "", NULL);
  if (IS_SET_DELETE(rc, DELETE_THIS)) {
    delete mob;
    return NULL;
  }
  rc = mob->checkResponses(mob, 0, "", CMD_GENERIC_CREATED);
  if (IS_SET_DELETE(rc, DELETE_THIS)) {
    delete mob;
    return NULL;
  }


  return (mob);
}


cached_object *TObjectCache::operator[](int nr)
{
  std::map<int, cached_object *>::iterator tIter;
  cached_object *ret;

  tIter = cache.find(nr);
  if (tIter != cache.end()) {
    ret = tIter->second;
  } else {
    ret = NULL;
  }
  return ret;
}

cached_object *TMobileCache::operator[](int nr)
{
  std::map<int, cached_object *>::iterator tIter;
  cached_object *ret;

  tIter = cache.find(nr);
  if (tIter != cache.end()) {
    ret = tIter->second;
  } else {
    ret = NULL;
  }
  return ret;
}


void log_object(TObj *obj)
{
  // Don't log objects that are flagged as newbie
  if (obj->isObjStat(ITEM_NEWBIE)) {
    return;
  }
  // Don't log tools
  if (dynamic_cast<TTool *>(obj)) {
    return;
  }
  // Don't log commodities
  if (dynamic_cast<TCommodity *>(obj)) {
    return;
  }
  // Don't log treasures
  if (dynamic_cast<TTreasure *>(obj)) {
    return;
  }
  // Don't log food
  if (dynamic_cast<TFood *>(obj)) {
    return;
  }
  // Don't log other
  if (dynamic_cast<TOtherObj *>(obj)) {
    return;
  }
  // Don't log trash
  if (dynamic_cast<TTrash *>(obj)) {
    return;
  }
  TDatabase db(DB_SNEEZY);
  db.query("insert into objlog values (%i, now(), %i)", obj_index[obj->getItemIndex()].virt, obj_index[obj->getItemIndex()].getNumber());
}

void TObjectCache::preload()
{
  TDatabase db(DB_SNEEZY);

  db.query("select vnum, short_desc, type, action_flag, wear_flag, val0, val1, val2, val3, weight, price, can_be_seen, spec_proc, max_struct, cur_struct, decay, volume, material, max_exist from obj");

  while(db.fetchRow()){
    cached_object *c=new cached_object;
    c->number=real_object(convertTo<int>(db["vnum"]));
    c->s["type"]=db["type"];
    c->s["action_flag"]=db["action_flag"];
    c->s["wear_flag"]=db["wear_flag"];
    c->s["val0"]=db["val0"];
    c->s["val1"]=db["val1"];
    c->s["val2"]=db["val2"];
    c->s["val3"]=db["val3"];
    c->s["weight"]=db["weight"];
    c->s["price"]=db["price"];
    c->s["can_be_seen"]=db["can_be_seen"];
    c->s["spec_proc"]=db["spec_proc"];
    c->s["max_struct"]=db["max_struct"];
    c->s["cur_struct"]=db["cur_struct"];
    c->s["decay"]=db["decay"];
    c->s["volume"]=db["volume"];
    c->s["material"]=db["material"];
    c->s["max_exist"]=db["max_exist"];

    cache[c->number]=c;
  }
}

void TMobileCache::preload()
{
  TDatabase db(DB_SNEEZY);

  db.query("select vnum, name, short_desc, long_desc, description, actions, affects, faction, fact_perc, letter, attacks, class, level, tohit, ac, hpbonus, damage_level, damage_precision, gold, race, weight, height, str, bra, con, dex, agi, intel, wis, foc, per, cha, kar, spe, pos, def_position, sex, spec_proc, skin, vision, can_be_seen, max_exist, local_sound, adjacent_sound from mob");

  while(db.fetchRow()){
    cached_object *c=new cached_object;
    c->number=real_mobile(convertTo<int>(db["vnum"]));
    c->s["vnum"]=db["vnum"];
    c->s["name"]=db["name"];
    c->s["short_desc"]=db["short_desc"];
    c->s["long_desc"]=db["long_desc"];
    c->s["description"]=db["description"];
    c->s["actions"]=db["actions"];
    c->s["affects"]=db["affects"];
    c->s["faction"]=db["faction"];
    c->s["fact_perc"]=db["fact_perc"];
    c->s["letter"]=db["letter"];
    c->s["attacks"]=db["attacks"];
    c->s["class"]=db["class"];
    c->s["level"]=db["level"];
    c->s["tohit"]=db["tohit"];
    c->s["ac"]=db["ac"];
    c->s["hpbonus"]=db["hpbonus"];
    c->s["damage_level"]=db["damage_level"];
    c->s["damage_precision"]=db["damage_precision"];
    c->s["gold"]=db["gold"];
    c->s["race"]=db["race"];
    c->s["weight"]=db["weight"];
    c->s["height"]=db["height"];
    c->s["str"]=db["str"];
    c->s["bra"]=db["bra"];
    c->s["con"]=db["con"];
    c->s["dex"]=db["dex"];
    c->s["agi"]=db["agi"];
    c->s["intel"]=db["intel"];
    c->s["wis"]=db["wis"];
    c->s["foc"]=db["foc"];
    c->s["per"]=db["per"];
    c->s["cha"]=db["cha"];
    c->s["kar"]=db["kar"];
    c->s["spe"]=db["spe"];
    c->s["pos"]=db["pos"];
    c->s["def_position"]=db["def_position"];
    c->s["sex"]=db["sex"];
    c->s["spec_proc"]=db["spec_proc"];
    c->s["skin"]=db["skin"];
    c->s["vision"]=db["vision"];
    c->s["can_be_seen"]=db["can_be_seen"];
    c->s["max_exist"]=db["max_exist"];
    c->s["local_sound"]=db["local_sound"];
    c->s["adjacent_sound"]=db["adjacent_sound"];
    
    cache[c->number]=c;
  }

  db.query("select vnum, keyword, description from mob_extra");

  while(db.fetchRow()){
    cached_mob_extra *c=new cached_mob_extra;
    c->number=real_mobile(convertTo<int>(db["vnum"]));
    c->keyword=db["keyword"];
    c->description=db["description"];

    extra[c->number].push_back(c);
  }

  db.query("select vnum, type, amt from mob_imm");

  while(db.fetchRow()){
    cached_mob_imm *c=new cached_mob_imm;
    c->number=real_mobile(convertTo<int>(db["vnum"]));
    c->type=convertTo<int>(db["type"]);
    c->amt=convertTo<int>(db["amt"]);

    imm[c->number].push_back(c);
  }
}



// the idea here is to search all shops for the object we want to load
// and if we find it at a decent price, buy it.
// if we can't find it, then try to buy some commods to "make it".
// failing that, just load it.
TObj *read_object_buy_build(TBeing *buyer, int nr, readFileTypeT type)
{

  if (type == VIRTUAL)
    nr = real_object(nr);

  if(bootTime || !obj_cache[nr]){
    //    vlogf(LOG_BUG, "read_object_buy_build() called during bootTime");
    return read_object(nr, REAL);
  }

  int material=convertTo<int>(obj_cache[nr]->s["material"]);
  float weight=convertTo<float>(obj_cache[nr]->s["weight"]);
  sstring name=obj_index[nr].short_desc;
  int indexed_cost=convertTo<int>(obj_cache[nr]->s["price"]);

  int price, shop_nr, rent_id=0;
  int commod_price=0, commod_shop_nr=0, commod_rent_id=0;
  TObj *o=NULL;
  TObj *commod=NULL;

  TDatabase db(DB_SNEEZY);

  // look for the object in a shop
  db.query("select r.weight as weight, r.owner as shop_nr, r.rent_id as rent_id from rent r, shopowned so where r.owner=so.shop_nr and r.owner_type='shop' and r.vnum=%i order by profit_buy asc, (r.cur_struct/r.max_struct) asc, price asc", obj_index[nr].virt);

  if(db.fetchRow()){
    shop_nr=convertTo<int>(db["shop_nr"]);
    rent_id=convertTo<int>(db["rent_id"]);
    TShopOwned tso(shop_nr, buyer);

    if (tso.getKeeper())
      o=tso.getKeeper()->loadItem(shop_nr, convertTo<int>(db["rent_id"]));
    if (o) {
      *tso.getKeeper() += *o;
      price = o->shopPrice(1, shop_nr, -1, buyer);
    }
  }

  // ok, we now have a cheap object (o) selling for price at shop_nr
  // let's see if we can find a cheaper commod
  db.query("select r.owner as shop_nr, r.rent_id as rent_id, r.material as material, r.weight*10 as units from rent r, obj o where o.type=%i and r.vnum=o.vnum and r.material=%i and owner_type='shop' and r.weight>=%f order by units desc", ITEM_RAW_MATERIAL, material, weight);

  if(db.fetchRow()){
    commod_shop_nr=convertTo<int>(db["shop_nr"]);
    commod_rent_id=convertTo<int>(db["rent_id"]);
    TShopOwned tso(commod_shop_nr, buyer);
    TObj *obj = NULL;

    if (tso.getKeeper())
      obj=tso.getKeeper()->loadItem(commod_shop_nr, convertTo<int>(db["rent_id"]));
    if (obj) {
      if(!(commod=dynamic_cast<TCommodity *>(obj)))
        return read_object(nr, type);
      
      *tso.getKeeper() += *commod;

      commod_price = commod->shopPrice((int)(weight*10), commod_shop_nr, 
				       -1, buyer);
    }
  }


  // if we have a cheap item, and no commodity to consider, OR
  // we do have a commod and it's more expensive...
  if(o && (!commod || 
	   (price <= (commod_price+indexed_cost)))){
    TShopOwned tso(shop_nr, buyer);
    --(*o);
    buyer->addToMoney(price, GOLD_XFER); // this is to offset cost
    tso.doBuyTransaction(price, o->getName(), TX_BUYING, o);
    vlogf(LOG_PEEL, format("%s purchased %s from shop %i for %i talens.") %
	  buyer->getName() % o->getName() % shop_nr % price);

    if (tso.getKeeper())
      tso.getKeeper()->deleteItem(shop_nr, rent_id);
    delete commod;
    return o;
  // otherwise buy the commod if it is available
  } else if(commod){
    TShopOwned tso(commod_shop_nr, buyer);
    buyer->addToMoney(commod_price, GOLD_XFER); // this is to offset cost
    tso.doBuyTransaction(commod_price, commod->getName(), 
			 TX_BUYING, commod);

    commod->setWeight(commod->getWeight() - weight);
    vlogf(LOG_PEEL, format("%s purchased %s (%i) from shop %i for %i talens.") %
	  buyer->getName() %commod->getName() % (int)(weight*10) %
	  commod_shop_nr % commod_price);

    if (tso.getKeeper()) {
      tso.getKeeper()->deleteItem(commod_shop_nr, commod_rent_id);
      if(commod->getWeight() > 0)
        tso.getKeeper()->saveItem(commod_shop_nr, commod);
    }
  }

  if (commod)
    delete commod;
  if (o)
    delete o;

  // we either purchased commod, or there was nothing to purchase so
  // load the item
  return read_object(nr, REAL);
}

int TMonster::readMobFromDB(int virt, bool should_alloc, TBeing *ch)
{
  long tmp; //, tmp2;
  int calc_level; // tmp3, 
  // float att;
  int rc;
  char letter;
  TDatabase db;

  int nr = real_mobile(virt);

  if(!(ch && should_alloc) && mob_cache[nr]!=NULL){
    name = mob_index[number].name;
    shortDescr = mob_index[number].short_desc;
    player.longDescr = mob_index[number].long_desc;
    setDescr(mob_index[number].description);
    
    setMult(1.0);
    
    specials.act = convertTo<int>(mob_cache[nr]->s["actions"]);
    if (should_alloc)
      SET_BIT(specials.act, ACT_STRINGS_CHANGED);
    
    specials.affectedBy = convertTo<int>(mob_cache[nr]->s["affects"]);
    
    if (isAffected(AFF_SANCTUARY)) {
      REMOVE_BIT(this->specials.affectedBy, AFF_SANCTUARY);
      
      affectedData aff;
      
      aff.type = SPELL_SANCTUARY;
      aff.level = 50;
      aff.duration = PERMANENT_DURATION;
      aff.location = APPLY_PROTECTION;
      aff.modifier = 50;
      aff.bitvector = AFF_SANCTUARY;
      affectJoin(this, &aff, AVG_DUR_NO, AVG_EFF_YES);
      // setProtection(50);
    }
    
    tmp=convertTo<int>(mob_cache[nr]->s["faction"]);
    mud_assert(tmp >= MIN_FACTION && tmp < MAX_FACTIONS, "Bad faction value");
    setFaction(factionTypeT(tmp));
    
    setPerc((double) convertTo<double>(mob_cache[nr]->s["fact_perc"]));
    
    letter=convertTo<char>(mob_cache[nr]->s["letter"]);
    
    if ((letter == 'A') || (letter == 'L')) {
      setMult((double) convertTo<double>(mob_cache[nr]->s["attacks"]));
      
      setClass(convertTo<int>(mob_cache[nr]->s["class"]));
      fixLevels(convertTo<int>(mob_cache[nr]->s["level"]));
      // int lvl = convertTo<int>(mob_cache[nr]->s["level"]);
      
      setHitroll(convertTo<int>(mob_cache[nr]->s["tohit"]));
      
      setACLevel(convertTo<float>(mob_cache[nr]->s["ac"]));
      setACFromACLevel();
      
      setHPLevel(convertTo<float>(mob_cache[nr]->s["hpbonus"]));
      setHPFromHPLevel();
      
      setDamLevel(convertTo<float>(mob_cache[nr]->s["damage_level"]));
      setDamPrecision(convertTo<int>(mob_cache[nr]->s["damage_precision"]));
      
      calc_level = (int) (getHPLevel() + getACLevel() + getDamLevel())/3;
      
      setMana(10);
      setMaxMana(10);
      setLifeforce(9000);
      setMaxMove(50 + 10*GetMaxLevel());
      setMove(moveLimit());
      
      moneyConst = (ubyte) convertTo<int>(mob_cache[nr]->s["gold"]);
      
      setExp(0);
      
      setRace(race_t(convertTo<int>(mob_cache[nr]->s["race"])));
      setWeight(convertTo<float>(mob_cache[nr]->s["weight"]));
      setHeight(convertTo<int>(mob_cache[nr]->s["height"]));
      
      // statTypeT local_stat;
      
      setStat(STAT_CHOSEN, STAT_STR, convertTo<int>(mob_cache[nr]->s["str"]));
      setStat(STAT_CHOSEN, STAT_BRA, convertTo<int>(mob_cache[nr]->s["bra"]));
      setStat(STAT_CHOSEN, STAT_CON, convertTo<int>(mob_cache[nr]->s["con"]));
      setStat(STAT_CHOSEN, STAT_DEX, convertTo<int>(mob_cache[nr]->s["dex"]));
      setStat(STAT_CHOSEN, STAT_AGI, convertTo<int>(mob_cache[nr]->s["agi"]));
      setStat(STAT_CHOSEN, STAT_INT, convertTo<int>(mob_cache[nr]->s["intel"]));
      setStat(STAT_CHOSEN, STAT_WIS, convertTo<int>(mob_cache[nr]->s["wis"]));
      setStat(STAT_CHOSEN, STAT_FOC, convertTo<int>(mob_cache[nr]->s["foc"]));
      setStat(STAT_CHOSEN, STAT_PER, convertTo<int>(mob_cache[nr]->s["per"]));
      setStat(STAT_CHOSEN, STAT_CHA, convertTo<int>(mob_cache[nr]->s["cha"]));
      setStat(STAT_CHOSEN, STAT_KAR, convertTo<int>(mob_cache[nr]->s["kar"]));
      setStat(STAT_CHOSEN, STAT_SPE, convertTo<int>(mob_cache[nr]->s["spe"]));
      
      setPosition(mapFileToPos(convertTo<int>(mob_cache[nr]->s["pos"])));
      
      if (getPosition() == POSITION_DEAD) {
	// can happen.  no legs and trying to set resting, etc
	vlogf(LOG_LOW, format("Mob (%s) put in dead position during creation.") % 
	      getName());
      }
      
      default_pos = mapFileToPos(convertTo<int>(mob_cache[nr]->s["def_position"]));
      
      setSexUnsafe(convertTo<int>(mob_cache[nr]->s["sex"]));
      
      spec = convertTo<int>(mob_cache[nr]->s["spec_proc"]);
      
      if (!UtilProcs(spec) && !GuildProcs(spec) && !isTestmob()) 
	//    if !(is_abbrev(name, "trainer") || is_abbrev(name, "guildmaster"))
	{
	  tmp = (int)((getHPLevel() + getACLevel() + getDamLevel())/3);
	  fixLevels(tmp);
	  
	  //    reallvl = tmp;
	}
      
      // don't set the xp until here, since a lot of things factor in
      // gold isn't calculated until here either...
      addToExp(determineExp());
      
      setMaterial(convertTo<int>(mob_cache[nr]->s["skin"]));
      
      canBeSeen = convertTo<int>(mob_cache[nr]->s["can_be_seen"]);
      
      visionBonus = convertTo<int>(mob_cache[nr]->s["vision"]);
      
      max_exist = convertTo<int>(mob_cache[nr]->s["max_exist"]);
      
      if (!should_alloc) {
	rc = checkSpec(this, CMD_GENERIC_INIT, "", NULL);
	if (IS_SET_DELETE(rc, DELETE_THIS) ||
	    IS_SET_DELETE(rc, DELETE_VICT)) {
	  return DELETE_THIS;
	}
      }
      if (mob_cache[nr]->s["local_sound"].length() > 0)
	sounds=mud_str_dup(mob_cache[nr]->s["local_sound"]);
      if (mob_cache[nr]->s["adjacent_sound"].length() > 0)
	distantSnds=mud_str_dup(mob_cache[nr]->s["adjacent_sound"]);
    }

    for(unsigned int i=0;i<mob_cache.imm[nr].size();++i){
      setImmunity((immuneTypeT) mob_cache.imm[nr][i]->type, mob_cache.imm[nr][i]->amt);
    }
    
    extraDescription *tExDescr;
    for(unsigned int i=0;i<mob_cache.extra[nr].size();++i){
      tExDescr              = new extraDescription();
      tExDescr->keyword     = mud_str_dup(mob_cache.extra[nr][i]->keyword);
      tExDescr->description = mud_str_dup(mob_cache.extra[nr][i]->description);
      tExDescr->next        = ex_description;
      ex_description        = tExDescr;
    }


  } else {
    if (ch && should_alloc) {
      db = DB_IMMORTAL;
      db.query("select * from mob where owner = '%s' and vnum = %i", ch->name, virt);
    } else {
      db = DB_SNEEZY;
      db.query("select * from mob where vnum = %i", virt);
    }
    if (!db.fetchRow()) {
      if (!should_alloc) {
	vlogf(LOG_LOW, format("Failure to load mob vnum %d from database.") % virt);
      }
      return FALSE;
    }
    
    if (should_alloc) {
      number = -1;
      name = mud_str_dup(db["name"]);
      shortDescr = mud_str_dup(db["short_desc"]);
      player.longDescr = mud_str_dup(db["long_desc"]);
      setDescr(mud_str_dup(db["description"]));
    } else {
      name = mob_index[number].name;
      shortDescr = mob_index[number].short_desc;
      player.longDescr = mob_index[number].long_desc;
      setDescr(mob_index[number].description);
    }
    
    setMult(1.0);
    
    specials.act = convertTo<int>(db["actions"]);
    if (should_alloc)
      SET_BIT(specials.act, ACT_STRINGS_CHANGED);
    
    specials.affectedBy = convertTo<int>(db["affects"]);
    
    if (isAffected(AFF_SANCTUARY)) {
      REMOVE_BIT(this->specials.affectedBy, AFF_SANCTUARY);
      
      affectedData aff;
      
      aff.type = SPELL_SANCTUARY;
      aff.level = 50;
      aff.duration = PERMANENT_DURATION;
      aff.location = APPLY_PROTECTION;
      aff.modifier = 50;
      aff.bitvector = AFF_SANCTUARY;
      affectJoin(this, &aff, AVG_DUR_NO, AVG_EFF_YES);
      // setProtection(50);
    }
    
    tmp=convertTo<int>(db["faction"]);
    mud_assert(tmp >= MIN_FACTION && tmp < MAX_FACTIONS, "Bad faction value");
    setFaction(factionTypeT(tmp));
    
    setPerc((double) convertTo<double>(db["fact_perc"]));
    
    letter=convertTo<char>(db["letter"]);
    
    if ((letter == 'A') || (letter == 'L')) {
      setMult((double) convertTo<double>(db["attacks"]));
      
      setClass(convertTo<int>(db["class"]));
      fixLevels(convertTo<int>(db["level"]));
      // int lvl = convertTo<int>(db["level"]);
      
      setHitroll(convertTo<int>(db["tohit"]));
      
      setACLevel(convertTo<float>(db["ac"]));
      setACFromACLevel();
      
      setHPLevel(convertTo<float>(db["hpbonus"]));
      setHPFromHPLevel();
      
      setDamLevel(convertTo<float>(db["damage_level"]));
      setDamPrecision(convertTo<int>(db["damage_precision"]));
      
      calc_level = (int) (getHPLevel() + getACLevel() + getDamLevel())/3;
      
      setMana(10);
      setMaxMana(10);
      setLifeforce(9000);
      setMaxMove(50 + 10*GetMaxLevel());
      setMove(moveLimit());
      
      moneyConst = (ubyte) convertTo<int>(db["gold"]);
      
      setExp(0);
      
      setRace(race_t(convertTo<int>(db["race"])));
      setWeight(convertTo<float>(db["weight"]));
      setHeight(convertTo<int>(db["height"]));
      
      // statTypeT local_stat;
      
      setStat(STAT_CHOSEN, STAT_STR, convertTo<int>(db["str"]));
      setStat(STAT_CHOSEN, STAT_BRA, convertTo<int>(db["bra"]));
      setStat(STAT_CHOSEN, STAT_CON, convertTo<int>(db["con"]));
      setStat(STAT_CHOSEN, STAT_DEX, convertTo<int>(db["dex"]));
      setStat(STAT_CHOSEN, STAT_AGI, convertTo<int>(db["agi"]));
      setStat(STAT_CHOSEN, STAT_INT, convertTo<int>(db["intel"]));
      setStat(STAT_CHOSEN, STAT_WIS, convertTo<int>(db["wis"]));
      setStat(STAT_CHOSEN, STAT_FOC, convertTo<int>(db["foc"]));
      setStat(STAT_CHOSEN, STAT_PER, convertTo<int>(db["per"]));
      setStat(STAT_CHOSEN, STAT_CHA, convertTo<int>(db["cha"]));
      setStat(STAT_CHOSEN, STAT_KAR, convertTo<int>(db["kar"]));
      setStat(STAT_CHOSEN, STAT_SPE, convertTo<int>(db["spe"]));
      
      setPosition(mapFileToPos(convertTo<int>(db["pos"])));
      
      if (getPosition() == POSITION_DEAD) {
	// can happen.  no legs and trying to set resting, etc
	vlogf(LOG_LOW, format("Mob (%s) put in dead position during creation.") % 
	      getName());
      }
      
      default_pos = mapFileToPos(convertTo<int>(db["def_position"]));
      
      setSexUnsafe(convertTo<int>(db["sex"]));
      
      spec = convertTo<int>(db["spec_proc"]);
      
      if (!UtilProcs(spec) && !GuildProcs(spec) && !isTestmob()) 
	//    if !(is_abbrev(name, "trainer") || is_abbrev(name, "guildmaster"))
	{
	  tmp = (int)((getHPLevel() + getACLevel() + getDamLevel())/3);
	  fixLevels(tmp);
	  
	  //    reallvl = tmp;
	}
      
      // don't set the xp until here, since a lot of things factor in
      // gold isn't calculated until here either...
      addToExp(determineExp());
      
      setMaterial(convertTo<int>(db["skin"]));
      
      canBeSeen = convertTo<int>(db["can_be_seen"]);
      
      visionBonus = convertTo<int>(db["vision"]);
      
      max_exist = convertTo<int>(db["max_exist"]);
      
      if (!should_alloc) {
	rc = checkSpec(this, CMD_GENERIC_INIT, "", NULL);
	if (IS_SET_DELETE(rc, DELETE_THIS) ||
	    IS_SET_DELETE(rc, DELETE_VICT)) {
	  return DELETE_THIS;
	}
      }
      if (db["local_sound"].length() > 0)
	sounds=mud_str_dup(db["local_sound"]);
      if (db["adjacent_sound"].length() > 0)
	distantSnds=mud_str_dup(db["adjacent_sound"]);
      
    }
    db.query("select * from mob_imm where vnum=%i", virt);
    while(db.fetchRow()){
      setImmunity((immuneTypeT) convertTo<int>(db["type"]), convertTo<int>(db["amt"]));
    }
    
    db.query("select * from mob_extra where vnum=%i", virt);
    extraDescription *tExDescr;
    while(db.fetchRow()){
      tExDescr              = new extraDescription();
      tExDescr->keyword     = mud_str_dup(db["keyword"]);
      tExDescr->description = mud_str_dup(db["description"]);
      tExDescr->next        = ex_description;
      ex_description        = tExDescr;
    }
  }
    
  
  player.time->birth = time(0);
  player.time->played = 0;
  player.time->logon = time(0);
  
  condTypeT ic;
  for (ic = MIN_COND; ic < MAX_COND_TYPE; ++ic)
    setCond(ic, -1);
  
  aiMobCreation();
  
  // have read chosen, must set current stats now
  // curr stats are needed to properly assign discs (next step)
  // have to do this AFTER chosen read and AFTER race assigned
  statTypeT ik;
  for (ik = MIN_STAT; ik < MAX_STATS_USED; ik++)
    setStat(STAT_CURRENT, ik, getStat(STAT_NATURAL, ik));
  
  // assign disc before anything else
  // skills are needed by almost everything
  assignDisciplinesClass();

  return TRUE;
}


TObj *read_object(int nr, readFileTypeT type)
{
  TObj *obj = NULL;
  int i, rc, tmpcost;
  TDatabase db(DB_SNEEZY);

  i = nr;
  if (type == VIRTUAL)
    nr = real_object(nr);

  if ((nr < 0) || (nr >= (signed int) obj_index.size())) {
    vlogf(LOG_BUG, format("read_object: bad nr %d (i = %d)") % nr % i);
    return NULL;
  }

  if(/*bootTime &&*/ obj_cache[nr]!=NULL){
    obj = makeNewObj(mapFileToItemType(convertTo<int>(obj_cache[nr]->s["type"])));
    obj->number=nr;
    if (!obj->isObjStat(ITEM_STRUNG)) {
      obj->name = obj_index[nr].name;
      obj->shortDescr = obj_index[nr].short_desc;
      obj->setDescr(obj_index[nr].long_desc);
      obj->action_description = obj_index[nr].description;
      obj->ex_description=obj_index[nr].ex_description;
    }

    obj->setObjStat(convertTo<int>(obj_cache[nr]->s["action_flag"]));
    obj->obj_flags.wear_flags = convertTo<int>(obj_cache[nr]->s["wear_flag"]);
    obj->assignFourValues(convertTo<int>(obj_cache[nr]->s["val0"]), convertTo<int>(obj_cache[nr]->s["val1"]), convertTo<int>(obj_cache[nr]->s["val2"]), convertTo<int>(obj_cache[nr]->s["val3"]));
    obj->setWeight(convertTo<float>(obj_cache[nr]->s["weight"]));
    obj->obj_flags.cost = convertTo<int>(obj_cache[nr]->s["price"]);
    obj->canBeSeen = convertTo<int>(obj_cache[nr]->s["can_be_seen"]);
    obj->spec = convertTo<int>(obj_cache[nr]->s["spec_proc"]);
    obj->setMaxStructPoints(convertTo<int>(obj_cache[nr]->s["max_struct"]));
    obj->setStructPoints(convertTo<int>(obj_cache[nr]->s["cur_struct"]));
    obj->setDepreciation(0);
    obj->obj_flags.decay_time=convertTo<int>(obj_cache[nr]->s["decay"]);
    obj->setVolume(convertTo<int>(obj_cache[nr]->s["volume"]));
    obj->setMaterial(convertTo<int>(obj_cache[nr]->s["material"]));
    // beta is used to test LOW loads, so don't let max_exist be a factor
    obj->max_exist = (gamePort == Config::Port::BETA ? 9999 : convertTo<int>(obj_cache[nr]->s["max_exist"]));

  } else {
    db.query("select type, action_flag, wear_flag, val0, val1, val2, val3, weight, price, can_be_seen, spec_proc, max_struct, cur_struct, decay, volume, material, max_exist from obj where vnum=%i", obj_index[nr].virt);
  
    if(!db.fetchRow())
      return NULL;
    
    obj = makeNewObj(mapFileToItemType(convertTo<int>(db["type"])));
    obj->number=nr;
    if (!obj->isObjStat(ITEM_STRUNG)) {
      obj->name = obj_index[nr].name;
      obj->shortDescr = obj_index[nr].short_desc;
      obj->setDescr(obj_index[nr].long_desc);
      obj->action_description = obj_index[nr].description;
      obj->ex_description=obj_index[nr].ex_description;
    }
    obj->setObjStat(convertTo<int>(db["action_flag"]));
    obj->obj_flags.wear_flags = convertTo<int>(db["wear_flag"]);
    obj->assignFourValues(convertTo<int>(db["val0"]), convertTo<int>(db["val1"]), convertTo<int>(db["val2"]), convertTo<int>(db["val3"]));
    obj->setWeight(convertTo<float>(db["weight"]));
    obj->obj_flags.cost = convertTo<int>(db["price"]);
    obj->canBeSeen = convertTo<int>(db["can_be_seen"]);
    obj->spec = convertTo<int>(db["spec_proc"]);
    obj->setMaxStructPoints(convertTo<int>(db["max_struct"]));
    obj->setStructPoints(convertTo<int>(db["cur_struct"]));
    obj->setDepreciation(0);
    obj->obj_flags.decay_time=convertTo<int>(db["decay"]);
    obj->setVolume(convertTo<int>(db["volume"]));
    obj->setMaterial(convertTo<int>(db["material"]));
    // beta is used to test LOW loads, so don't let max_exist be a factor
    obj->max_exist = (gamePort == Config::Port::BETA ? 9999 : convertTo<int>(db["max_exist"]));
  }
  

  for(i=0;i<MAX_OBJ_AFFECT;++i){
    obj->affected[i].location = obj_index[nr].affected[i].location;
    obj->affected[i].modifier = obj_index[nr].affected[i].modifier;
    obj->affected[i].modifier2 = obj_index[nr].affected[i].modifier2;
    obj->affected[i].type = obj_index[nr].affected[i].type;
    obj->affected[i].level = obj_index[nr].affected[i].level;
    obj->affected[i].bitvector = obj_index[nr].affected[i].bitvector;

    if (obj->affected[i].location == APPLY_LIGHT)
      obj->addToLight(obj->affected[i].modifier);
  }

  obj_index[nr].addToNumber(1);
  
  obj->weightCorrection();
  obj->updateDesc();

  rc = obj->checkSpec(NULL, CMD_GENERIC_CREATED, "", NULL);
  if (IS_SET_DELETE(rc, DELETE_THIS)) {
    delete obj;
    obj = NULL;
    return NULL;
  }


  // use suggested price if available, otherwise use the set price
  if((tmpcost = obj->suggestedPrice())){
    obj->obj_flags.cost = tmpcost;
  }
  

  obj->checkObjStats();

  if(/*bootTime &&*/ obj_cache[nr]==NULL){
    //    vlogf(LOG_PEEL, format("caching object - %s") %  obj->shortDescr);
    cached_object *c=new cached_object;
    
    c->number=nr;
    c->s["type"]=db["type"];
    c->s["action_flag"]=db["action_flag"];
    c->s["wear_flag"]=db["wear_flag"];
    c->s["val0"]=db["val0"];
    c->s["val1"]=db["val1"];
    c->s["val2"]=db["val2"];
    c->s["val3"]=db["val3"];
    c->s["weight"]=db["weight"];
    c->s["price"]=db["price"];
    c->s["can_be_seen"]=db["can_be_seen"];
    c->s["spec_proc"]=db["spec_proc"];
    c->s["max_struct"]=db["max_struct"];
    c->s["cur_struct"]=db["cur_struct"];
    c->s["decay"]=db["decay"];
    c->s["volume"]=db["volume"];
    c->s["material"]=db["material"];
    c->s["max_exist"]=db["max_exist"];

    obj_cache.cache[c->number]=c;
  }

  return obj;
}

void zoneData::closeDoors()
{
  int bottom, i, x;
  roomDirData *ep = NULL;
  TRoom *rp;

  bottom = zone_nr ? (zone_table[zone_nr - 1].top + 1) : 0;
  for (i = bottom; i <= top; i++) {
    for (x = 0; x <= 9; x++) {
      if ((rp = real_roomp(i)) && (ep = rp->dir_option[x]) && 
          (ep->door_type != DOOR_NONE) && 
          (!IS_SET(ep->condition, EX_DESTROYED)))
        SET_BIT(ep->condition, EX_CLOSED);
    }
  }
}


// procZoneUpdate
procZoneUpdate::procZoneUpdate(const int &p)
{
  trigger_pulse=p;
  name="procZoneUpdate";
}

void procZoneUpdate::run(const TPulse &) const
{
// update zone ages, queue for reset if necessary, and dequeue when possible
  unsigned int i;
  resetQElement *update_u, *temp, *tmp2;
  const int ZO_DEAD = 999;

  for (i = 0; i < zone_table.size(); i++) {
    if (zone_table[i].age < zone_table[i].lifespan &&
        zone_table[i].reset_mode)
      (zone_table[i].age)++;
    else if (zone_table[i].age < ZO_DEAD && zone_table[i].reset_mode) {
      // enqueue zone
      update_u = new resetQElement();

      update_u->zone_to_reset = i;
      update_u->next = 0;

      if (!r_q.head)
        r_q.head = r_q.tail = update_u;
      else {
        r_q.tail->next = update_u;
        r_q.tail = update_u;
      }
      zone_table[i].age = ZO_DEAD;
    }
  }
  for (update_u = r_q.head; update_u; update_u = tmp2) {
    if (update_u->zone_to_reset >= zone_table.size()) {
      // this may or may not work
      // may result in some lost memory, 
      // but the loss is not signifigant over the short run
      update_u->zone_to_reset = 0;
      update_u->next = 0;
    }
    tmp2 = update_u->next;

    zoneData *z=&zone_table[update_u->zone_to_reset];

    if (z->reset_mode == 2 || z->isEmpty()) {
      z->closeDoors();
      z->resetZone(FALSE);
      // dequeue 

      if (update_u == r_q.head)
        r_q.head = r_q.head->next;
      else {
        for (temp = r_q.head; temp->next != update_u; temp = temp->next);

        if (!update_u->next)
          r_q.tail = temp;

        temp->next = update_u->next;
      }
      delete update_u;
    }
  }
}

// takes a number stored in the zone file for the slot to load a piece
//of equipment on, and converts it to a WEAR_SLOT
// this routine is also used to map slots to their old values in some arrays
// notably slot_chance and ac_percent_pos 
// load == TRUE, num is the slot as it is in the physical file
// load == FALSE, num is the slot as it is in the mud
wearSlotT mapFileToSlot(int num)
{
    switch (num) {
      case 1:
        return WEAR_FINGER_R;
      case 2:
        return WEAR_FINGER_L;
      case 3:
        return WEAR_NECK;
      case 4:
        return WEAR_BODY;
      case 5:
        return WEAR_HEAD;
      case 6:
        return WEAR_LEG_R;
      case 7:
        return WEAR_LEG_L;
      case 8:
        return WEAR_FOOT_R;
      case 9:
        return WEAR_FOOT_L;
      case 10:
        return WEAR_HAND_R;
      case 11:
        return WEAR_HAND_L;
      case 12:
        return WEAR_ARM_R;
      case 13:
        return WEAR_ARM_L;
      case 14:
        return WEAR_BACK;
      case 15:
        return WEAR_WAIST;
      case 16:
        return WEAR_WRIST_R;
      case 17:
        return WEAR_WRIST_L;
      case 18:
        return HOLD_RIGHT;
      case 19:
        return HOLD_LEFT;
      case 20:
        return WEAR_EX_LEG_R;
      case 21:
        return WEAR_EX_LEG_L;
      case 22:
        return WEAR_EX_FOOT_R;
      case 23:
        return WEAR_EX_FOOT_L;
      default:
        vlogf(LOG_LOW, format("Bogus slot (%d, 1) in zone file") %  num);
        vlogf(LOG_BUG, "forced crash");
        return wearSlotT(0);
    }
}

int mapSlotToFile(wearSlotT num)
{
  // saving
  switch (num) {
    case WEAR_FINGER_R:
      return 1;
    case WEAR_FINGER_L:
      return 2;
    case WEAR_NECK:
      return 3;
    case WEAR_BODY:
      return 4;
    case WEAR_HEAD:
      return 5;
    case WEAR_LEG_R:
      return 6;
    case WEAR_LEG_L:
      return 7;
    case WEAR_FOOT_R:
      return 8;
    case WEAR_FOOT_L:
      return 9;
    case WEAR_HAND_R:
      return 10;
    case WEAR_HAND_L:
      return 11;
    case WEAR_ARM_R:
      return 12;
    case WEAR_ARM_L:
      return 13;
    case WEAR_BACK:
      return 14;
    case WEAR_WAIST:
      return 15;
    case WEAR_WRIST_R:
      return 16;
    case WEAR_WRIST_L:
      return 17;
    case HOLD_RIGHT:
      return 18;
    case HOLD_LEFT:
      return 19;
    case WEAR_EX_LEG_R:
      return 20;
    case WEAR_EX_LEG_L:
      return 21;
    case WEAR_EX_FOOT_R:
      return 22;
    case WEAR_EX_FOOT_L:
      return 23;
    case MAX_WEAR:
    default:
      vlogf(LOG_LOW, format("Bogus slot (%d, 2) in zone file") %  num);
      vlogf(LOG_BUG, "forced crash");
      return 0;
  }
}

static void mobRepop(TMonster *mob, int zone, int tRPNum = 0)
{
  mob->specials.zone = zone;
  mob->oldRoom = (!tRPNum ? mob->roomp->number : tRPNum);

  if (!UtilMobProc(mob) && !GuildMobProc(mob)) {
    double grl = mob->getRealLevel();
    zone_table[zone].num_mobs++;
    zone_table[zone].mob_levels += grl;
    zone_table[zone].min_mob_level = min(grl, zone_table[zone].min_mob_level);
    zone_table[zone].max_mob_level = max(grl, zone_table[zone].max_mob_level);
  }

#if 1
  colorAct(COLOR_MOBS, ((mob->ex_description && mob->ex_description->findExtraDesc("repop")) ?
                        mob->ex_description->findExtraDesc("repop") :
                        "$n appears suddenly in the room."),
           TRUE, mob, 0, 0, TO_ROOM);
#else
  act("$n appears suddenly in the room.", TRUE, mob, 0, 0, TO_ROOM);
#endif

  if (mob->spec && zone && UtilProcs(mob->spec))
    vlogf(LOG_LOW, format("Mob (%s:%d) has a utility proc (%s:%d) and is not in zone #0") % 
         mob->getName() % mob->mobVnum() % 
         mob_specials[mob->spec].name % mob->spec);

  if (mob->roomp->isFlyingSector()) {
    TBeing *tbr = dynamic_cast<TBeing *>(mob->riding);
    if (!mob->riding && !mob->isFlying()) {
      mob->setPosition(POSITION_FLYING);
      mob->sendTo("You start to fly up in the air.\n\r");
    } else if (mob->riding && !tbr) {
      mob->dismount(POSITION_FLYING);
      mob->sendTo("You start to fly up in the air.\n\r");
    } else if (tbr && !tbr->isFlying()) {
      tbr->setPosition(POSITION_FLYING);
      mob->sendTo("Your mount starts to fly up in the air.\n\r");
    }
  }

  mob->quickieDefend();
}

/* --------------------------
armorSetLoad
used in zoneflie load/etc to set and update sets of eq
--------------------------- */
armorSetLoad::armorSetLoad()
{
  mud_assert(cElements(local_armor[0].slots) == MAX_WEAR, "slots not == MAX_WEAR in armorSetLoad ctor");
  resetArmor();
}

void armorSetLoad::resetArmor()
{
  memset(local_armor, 0, sizeof(struct armor_set_struct)*16);
}

void armorSetLoad::setArmor(int set, int slot, int value)
{
  local_armor[set].slots[slot] = value;
}

int armorSetLoad::getArmor(int set, int slot)
{
  return local_armor[set].slots[slot];
}
/* ------------------------ */


// runs the resetCom command for command = 'E'
void runResetCmdE(zoneData &zone, resetCom &rs, resetFlag flags, bool &mobload, TMonster *&mob, bool &objload, TObj *&obj, bool &last_cmd)
{
  if ((flags & resetFlagFindLoadPotential))
  {
    tallyObjLoadPotential(obj_index[rs.arg1].virt);
    return;
  }

  if (!mob)
  {
    vlogf(LOG_LOW, format("no mob for 'E' command.  Obj (%i)") %  rs.arg1);
    last_cmd = objload = false;
    return;
  }

  int obj_lp = getObjLoadPotential(obj_index[rs.arg1].virt);
  if (obj_lp == 0) {
    vlogf(LOG_MISC, format("Didn't find load potential of %s [%d].  rs.arg1=%d") % obj_index[rs.arg1].short_desc % obj_index[rs.arg1].virt % rs.arg1);
    obj_lp = 1;
  }

  // 1-e**((ln(1-0.01n**1/3)/n)) = normalized load rate
  // adj_obj_lp_ratio = 1 - pow(exp(1), ((log(1 - 0.01*cbrt((double)obj_lp))/(double)obj_lp)));
  // 1 - ((1-0.01*n**1/3)^(1/n)) = normalized load rate, less math
  double adj_obj_lp_ratio = 1 - pow((1 - cbrt((double)obj_lp)/100), 1/(double)obj_lp);

  // obj_lp_ratio = 1 - pow((1 - (double)fixed_chance/100), (double)obj_lp);
  double obj_lp_ratio = (double)fixed_chance/100;

  // getting to this point means we've already beat the fixed_chance%
  // chance of loading an object.  This has to be taken into account
  // when computing the odds of the normalized load potential.
  // vlogf(LOG_MISC, format("(10000000 * adj_obj_lp_ratio / obj_lp_ratio * stats.equip) = %d") % (int) (10000000 * adj_obj_lp_ratio / obj_lp_ratio * stats.equip));
  bool loadFail = true;
  if((obj_index[rs.arg1].getNumber() < obj_index[rs.arg1].max_exist) &&
     (::number(0, 9999999) < (int)(10000000*adj_obj_lp_ratio / obj_lp_ratio*stats.equip)))
  {
    if (!(flags & resetFlagPropLoad))
      obj = read_object_buy_build(mob, rs.arg1, REAL);
    else
      obj = read_object(rs.arg1, REAL);
    loadFail = obj == NULL;
  }

  if (loadFail) {
    repoCheck(mob, rs.arg1);
    last_cmd = objload = false;
    return;
  }

  // so now we've loaded the item, lets place it
  wearSlotT realslot = wearSlotT(rs.arg3);
  mud_assert(realslot >= MIN_WEAR && realslot < MAX_WEAR, "bad slot");

  // check for double-equip
  if (mob->equipment[realslot])
  {
    vlogf(LOG_LOW, format("'E' command operating on already equipped slot.  %s, %s slot %d\n\rpre-equipped with %s, is_same: %s") %  
          mob->getName() % obj->getName() % realslot %
          mob->equipment[realslot]->getName() %
          ((mob->equipment[realslot] == obj) ? "true" : "false"));
    delete obj;
    last_cmd = objload = false;
    return;
  }

  // these are just safety logs, equipping will be done regardless
  if (!mob->canUseEquipment(obj, SILENT_YES))
    vlogf(LOG_LOW, format("'E' command equipping unusable item (%s:%d) on (%s:%d).") % obj->getName() % obj->objVnum() % mob->getName() % mob->mobVnum());
  TBaseClothing *tbc = dynamic_cast<TBaseClothing *>(obj);
  if (tbc && tbc->canWear(ITEM_WEAR_FINGERS) && gamePort != Config::Port::PROD) {
    vlogf(LOG_LOW, format("RINGLOAD: [%s][%-6.2f] loading on [%s][%d]") % 
          obj->getName() % tbc->armorLevel(ARMOR_LEV_REAL) %
          mob->getName() % mob->GetMaxLevel());
  }
  if (tbc && !mob->validEquipSlot(realslot) && !tbc->isSaddle())
    vlogf(LOG_LOW, format("'E' command for %s equipping item (%s) on nonvalid slot %d.") % mob->getName() % tbc->getName() % realslot);
  if (!check_size_restrictions(mob, obj, realslot, mob) &&
      realslot != HOLD_RIGHT && realslot != HOLD_LEFT)
  {
    int size_per = 100;
    if (race_vol_constants[mapSlotToFile(realslot)])
    {
      size_per = (int)(100.0 * obj->getVolume() / race_vol_constants[mapSlotToFile( realslot)]);
      if (obj->isPaired())
        size_per /= 2;
    }
    vlogf(LOG_LOW, format("'E' for (%s:%d) equipping (%s:%d) with bad fit. (m:%d%%/o:%d%%) change vol to %d, or height to %d.") %  
        mob->getName() % mob->mobVnum() % obj->getName() % obj->objVnum() % (mob->getHeight() * 100) %
        size_per % (mob->getHeight() * (obj->isPaired() ? 2 : 1) * race_vol_constants[mapSlotToFile( realslot)]) %
        (size_per / 100));
  }
  // end sanity checks

  // OK, actually do the equip
  if (Config::LoadOnDeath() && !(flags & resetFlagPropLoad))
    *mob += *obj;
  else
    mob->equipChar(obj, realslot);
  mob->logItem(obj, CMD_LOAD);
  log_object(obj);

  // run some sanity checks after load
  // for items without levels, objLevel = 0 so this logic is OK
  double al = obj->objLevel();
  double grl = mob->getRealLevel();
  if (al > (grl + 1))
    vlogf(LOG_LOW, format("Mob (%s:%d) of level %.1f loading item (%s:%d) thought to be level %.1f.") %  mob->getName() % mob->mobVnum() % grl % obj->getName() % obj->objVnum() % al);
  if (!Config::LoadOnDeath() && !mob->equipment[realslot])
    vlogf(LOG_LOW, format("Zone-file %s (%d) failed to equip %s (%d)") % mob->getName() % mob->mobVnum() % obj->getName() % obj->objVnum());

  last_cmd = true;
}

void runResetCmdM(zoneData &zone, resetCom &rs, resetFlag flags, bool &mobload, TMonster *&mob, bool &objload, TObj *&obj, bool &last_cmd)
{
  mob = NULL;
  last_cmd = mobload = false;

  // check if zone is disabled or if mob exceeds absolute max
  if (rs.arg1 < 0 || rs.arg1 >= (signed int) mob_index.size()) {
    vlogf(LOG_LOW, format("Detected bogus mob number (%d) on read_mobile call for resetZone (load room %d).") % rs.arg1 % rs.arg3);
    return;
  }

  if(zone.zone_value != 0 && (flags & resetFlagBootTime)){
    mob_index[rs.arg1].doesLoad=true;
    mob_index[rs.arg1].numberLoad++;
  }

  if (zone.zone_value == 0)
    return;

  // catch cases where builder used global max over zonefile max
  if (rs.arg2 > mob_index[rs.arg1].max_exist && gamePort != Config::Port::BETA && rs.arg3 != zone.random_room)
  {
    vlogf(LOG_LOW, format("Mob %s (%i) tried has improper load max (%i) compared to global (%i) in zonefile") %
      mob_index[rs.arg1].short_desc % mob_index[rs.arg1].virt % rs.arg2 % mob_index[rs.arg1].max_exist);
  }

  if (mob_index[rs.arg1].getNumber() >= mob_index[rs.arg1].max_exist)
  {
    if((flags & resetFlagBootTime))
      vlogf(LOG_LOW, format("Mob %s (%i) tried to load but hit max_exist") % 
            mob_index[rs.arg1].short_desc % mob_index[rs.arg1].virt);
    return;
  }

  TRoom *rp = real_roomp(rs.arg3);
  if (!rp)
  {
    vlogf(LOG_LOW, format("No room (%d) in M command (%d)") % rs.arg3 % rs.arg1);
    return;
  }

  int existingCount = 0;
  for (TThing *t = rp->tBornInsideMe; t; t = t->nextBorn)
  {
    TMonster *tMonster = dynamic_cast<TMonster *>(t);
    if (tMonster && tMonster->mobVnum() == mob_index[rs.arg1].virt)
      existingCount++;
  }

  // check room load max
  if (existingCount >= rs.arg2)
    return;

  if (!(mob = read_mobile(rs.arg1, REAL))) {
    vlogf(LOG_BUG, format("Error reading mobile (%d).  You suck.") %  rs.arg1);
    return;
  }

  if (!Config::LoadOnDeath())
    mob->createWealth();

  if (mob->isShopkeeper())
    mob->calculateGoldFromConstant();

  // add mob to room
  if (rs.command == 'M' && (mob->isNocturnal() || mob->isDiurnal()))
    *real_roomp(Room::NOCTURNAL_STORAGE) += *mob;
  else
    *rp += *mob;

  // Slap the mob on the born list.
  *rp << *mob;

  if (rs.command == 'M')
  {
    mob->brtRoom = (rp ? rp->number : Room::NOWHERE);
    mobRepop(mob, zone.zone_nr, (rp ? rp->number : Room::NOWHERE));
  }
  last_cmd = mobload = true;
}


void runResetCmdC(zoneData &zone, resetCom &rs, resetFlag flags, bool &mobload, TMonster *&mob, bool &objload, TObj *&obj, bool &last_cmd)
{
  last_cmd = mobload = false;

  if (!mob)
    return;

  TMonster *charmie = NULL;
  runResetCmdM(zone, rs, flags, mobload, charmie, objload, obj, last_cmd);
  if (!charmie)
    return;

  // set master
  TBeing *leader = mob;
  while(leader->master)
    leader = leader->master;
  leader->addFollower(charmie);
  SET_BIT(charmie->specials.affectedBy, AFF_CHARM);

  // mob popped
  charmie->brtRoom = (charmie->roomp ? charmie->roomp->number : Room::NOWHERE);
  mobRepop(charmie, zone.zone_nr, (charmie->roomp ? charmie->roomp->number : Room::NOWHERE));
  mob = charmie;
  last_cmd = mobload = true;
}


void runResetCmdK(zoneData &zone, resetCom &rs, resetFlag flags, bool &mobload, TMonster *&mob, bool &objload, TObj *&obj, bool &last_cmd)
{
  last_cmd = mobload = false;

  if (!mob)
    return;

  TMonster *grouper = NULL;
  runResetCmdM(zone, rs, flags, mobload, grouper, objload, obj, last_cmd);
  if (!grouper)
    return;

  // set master
  TBeing *leader = mob;
  while(leader->master)
    leader = leader->master;
  leader->addFollower(grouper);
  SET_BIT(leader->specials.affectedBy, AFF_GROUP);
  SET_BIT(grouper->specials.affectedBy, AFF_GROUP);

  // mob popped
  grouper->brtRoom = (grouper->roomp ? grouper->roomp->number : Room::NOWHERE);
  mobRepop(grouper, zone.zone_nr, (grouper->roomp ? grouper->roomp->number : Room::NOWHERE));
  mob = grouper;
  last_cmd = mobload = true;
}


void runResetCmdR(zoneData &zone, resetCom &rs, resetFlag flags, bool &mobload, TMonster *&mob, bool &objload, TObj *&obj, bool &last_cmd)
{
  last_cmd = mobload = false;

  if (!mob)
    return;

  TMonster *rider = NULL;
  runResetCmdM(zone, rs, flags, mobload, rider, objload, obj, last_cmd);
  if (!rider)
    return;

  // sanity checks
  if (mob->getHeight() <= (6 * rider->getHeight() / 10))
    vlogf(LOG_LOW, format("Mob mounting mount that is too small.  [%s] [%s]") % rider->getName() % mob->getName());
  if (mob->getHeight() >= (5 * rider->getHeight() / 2))
    vlogf(LOG_LOW, format("Mob mounting mount that is too big.  [%s] [%s]") % rider->getName() % mob->getName());
  if (compareWeights(rider->getTotalWeight(TRUE),(mob->carryWeightLimit() - mob->getCarriedWeight())) == -1)
    vlogf(LOG_LOW, format("Mob mounting mount that is too weak.  [%s] [%s]") % rider->getName() % mob->getName());
  if (mob->GetMaxLevel() > rider->GetMaxLevel())
    vlogf(LOG_LOW, format("Mob mounting mount that is too strong.  [%s:%d] [%s:%d]") % rider->getName() % rider->GetMaxLevel() % mob->getName() % mob->GetMaxLevel());

  // mount up
  rider->mount(mob);
  rider->setPosition(POSITION_MOUNTED);
  if (mob->master && mob->master != rider && !mob->rider)
    mob->stopFollower(TRUE);
  if (!mob->master)
    rider->addFollower(mob);

  // needs to be after we are set riding
  rider->brtRoom = (rider->roomp ? rider->roomp->number : Room::NOWHERE);
  mobRepop(rider, zone.zone_nr);
  mob = rider;
  last_cmd = mobload = true;
}

void runResetCmdA(zoneData &zone, resetCom &rs, resetFlag flags, bool &mobload, TMonster *&mob, bool &objload, TObj *&obj, bool &last_cmd)
{
  int iRoom;
  for (iRoom = 0; iRoom < 10; iRoom++)
    if (real_roomp(zone.random_room = ::number(rs.arg1, rs.arg2)))
      break;

  if (iRoom >= 10)
    vlogf(LOG_LOW, format("Unable to detect room in 'A' %d %d") % rs.arg1 % rs.arg2);
}

void runResetCmdQMark(zoneData &zone, resetCom &rs, resetFlag flags, bool &mobload, TMonster *&mob, bool &objload, TObj *&obj, bool &last_cmd)
{
  if (!rs.character)
    return;

  if ((rs.character == 'M') ||
      (rs.character == 'O') ||
      (rs.character == 'B') ||
      (rs.character == 'L') ||
      (objload && (rs.character == 'P')) ||
      (mobload && (rs.character != 'P'))) {

    // If we are putting certain objects into the world or
    // giving certain objects to mobs, follow the chance defined
    // in the zonefile.  Otherwise, set the chance to fixed_chance.
    int roll = (flags & resetFlagFindLoadPotential) ? 1 : dice(1, 100);
    bool useArgs = (objload && rs.character == 'P') || (mobload && rs.character == 'G');
    int my_chance = useArgs ? rs.arg1 : fixed_chance;

    last_cmd = (rs.arg1 >= 98 || roll <= my_chance || gamePort == Config::Port::BETA);
    if (!last_cmd) {
      if (rs.character == 'M')
        mobload = 0; // cancel all operations after this 'M' which use the mob ptr by setting !mobload
      else if (rs.character == 'O' || rs.character == 'B')
        objload = 0; // cancel all operations after this 'B' or 'O' which use the obj ptr by setting !objload
    }
  }
}

// load object onto floor - usually done for furniture and props
void runResetCmdB(zoneData &zone, resetCom &rs, resetFlag flags, bool &mobload, TMonster *&mob, bool &objload, TObj *&obj, bool &last_cmd)
{
  objload = false;
  last_cmd = false;

  TRoom *rp = real_roomp(rs.arg3);
  if (!rp)
  {
    vlogf(LOG_LOW, format("No room (%d) in B/O command (%d).  cmd=%d, zone=%d") %  rs.arg3 % rs.arg1 % rs.cmd_no % zone.zone_nr);
    return;
  }

  // count all of the objects
  int count = 0;
  TObj *found = NULL;
  for(StuffIter it= rp->stuff.begin();it!= rp->stuff.end();++it)
  {
    TObj *objScan = dynamic_cast<TObj *>(*it);
    if(objScan && objScan->objVnum() == obj_index[rs.arg1].virt)
    {
      found = objScan;
      count++;
    }
  }

  // if we are over or at max, just re-use the last one
  if (count >= rs.arg2)
  {
    obj = found;
    last_cmd = objload = true;
    return;
  }

  // if we are at world max, use last one (if exists)
  if (obj_index[rs.arg1].getNumber() >= obj_index[rs.arg1].max_exist)
  {
    obj = found;
    last_cmd = objload = (obj != NULL);
    return;
  }

  // load is the number we are allowed to load, should always be > 0
  int load = min(max(rs.arg2, 0), max(0, obj_index[rs.arg1].max_exist - obj_index[rs.arg1].getNumber()));
  if (load <= 0)
  {
    vlogf(LOG_LOW, format("Strange error attempting to load %i of object %i in room %i.") % rs.arg2 % rs.arg1 % rs.arg3);
    obj = found;
    last_cmd = objload = (obj != NULL);
    return;
  }

  // load the objects
  for(; count < load; ++count)
  {
    obj = read_object(rs.arg1, REAL);
    if (obj == NULL)
    {
      vlogf(LOG_LOW, format("No obj (%d) in O command (room=%d).  cmd=%d, zone=%d") %  rs.arg1 % rs.arg3 % rs.cmd_no % zone.zone_nr);
      objload = last_cmd = false;
      continue;
    }
    *rp += *obj;
    obj->onObjLoad();
    last_cmd = objload = true;
  }
}

// like 'B' except boot time only
void runResetCmdO(zoneData &zone, resetCom &rs, resetFlag flags, bool &mobload, TMonster *&mob, bool &objload, TObj *&obj, bool &last_cmd)
{
  objload = last_cmd = false;

  if (!(flags & resetFlagBootTime))
    return;

  return runResetCmdB(zone, rs, flags, mobload, mob, objload, obj, last_cmd);
}

// used to 'place' objects on tables, into containers, etc
void runResetCmdP(zoneData &zone, resetCom &rs, resetFlag flags, bool &mobload, TMonster *&mob, bool &objload, TObj *&obj, bool &last_cmd)
{
  last_cmd = false;

  if (!obj || obj_index[rs.arg1].getNumber() >= obj_index[rs.arg1].max_exist)
  {
    vlogf(LOG_LOW, format("Error placing (%d) in P command.  cmd=%d, zone=%d") %  rs.arg1 % rs.cmd_no % zone.zone_nr);
    return;
  }

  bool isContainer = dynamic_cast<TBaseContainer *>(obj);
  bool isTable = dynamic_cast<TTable *>(obj);
  if (!isContainer && !isTable)
  {
    vlogf(LOG_LOW, format("Error placing (%d) in P command into object %d - not a container.  cmd=%d, zone=%d") %  rs.arg1 % obj->objVnum() % rs.cmd_no % zone.zone_nr);
    return;
  }

  TObj *newobj = read_object(rs.arg1, REAL);
  if (!newobj)
    return;

  if (isContainer)
    *obj += *newobj;
  else if (isTable)
    newobj->mount(obj);

  newobj->onObjLoad();
  last_cmd = true;
  log_object(newobj);
}

// Change ONE value of the four values upon reset- Russ 
void runResetCmdV(zoneData &zone, resetCom &rs, resetFlag flags, bool &mobload, TMonster *&mob, bool &objload, TObj *&obj, bool &last_cmd)
{
  last_cmd = false;

  if (rs.arg1 < 0 && rs.arg1 >= 4)
  {
    vlogf(LOG_LOW, format("Bad slot (%d) for V command (%d)") % rs.arg1 % rs.arg2);
    return;
  }

  if (!obj)
    return;

  int values[4];
  obj->getFourValues(&values[0], &values[1], &values[2], &values[3]);
  values[rs.arg1] = rs.arg2;
  obj->assignFourValues(values[0], values[1], values[2], values[3]);
  last_cmd = true;
}


// Set traps for doors and containers - Russ
void runResetCmdT(zoneData &zone, resetCom &rs, resetFlag flags, bool &mobload, TMonster *&mob, bool &objload, TObj *&obj, bool &last_cmd)
{
  // if_flag-->0 : trap door, else trap previous object 
  if (!rs.if_flag)
  {
    TRoom *rp = real_roomp(rs.arg1);
    if (rp && rp->dir_option[rs.arg2])
    {
      SET_BIT(rp->dir_option[rs.arg2]->condition, EX_TRAPPED);
      rp->dir_option[rs.arg2]->trap_info = rs.arg3;
      rp->dir_option[rs.arg2]->trap_dam = rs.arg4;
    }
  }
  else
  {
    TOpenContainer *trc = dynamic_cast<TOpenContainer *>(obj);
    if (trc)
    {
      trc->addContainerFlag(CONT_TRAPPED);
      trc->setContainerTrapType(mapFileToDoorTrap(rs.arg1));
      trc->setContainerTrapDam(rs.arg2);
    }
  }
  last_cmd = true;
}

void runResetCmdG(zoneData &zone, resetCom &rs, resetFlag flags, bool &mobload, TMonster *&mob, bool &objload, TObj *&obj, bool &last_cmd)
{
  mud_assert(rs.arg1 >= 0 && rs.arg1 < (signed int) obj_index.size(), "Range error (%d not in obj_index)  G command #%d in %s", rs.arg1, rs.cmd_no, zone.name);
  if ((flags & resetFlagFindLoadPotential))
  {
    tallyObjLoadPotential(obj_index[rs.arg1].virt);
    return;
  }

  objload = last_cmd = false;

  if (!(obj_index[rs.arg1].getNumber() < obj_index[rs.arg1].max_exist &&
      (obj = read_object_buy_build(mob, rs.arg1, REAL))))
  {
    repoCheck(mob, rs.arg1);
    return;
  }

  *mob += *obj;
  obj->onObjLoad();
  mob->logItem(obj, CMD_LOAD);
  objload = last_cmd = true;
}


void runResetCmdH(zoneData &zone, resetCom &rs, resetFlag flags, bool &mobload, TMonster *&mob, bool &objload, TObj *&obj, bool &last_cmd)
{
  last_cmd = mob ? mob->addHatred(zoneHateT(rs.arg1), rs.arg2) : 0;
}

// X <set num>3 <slot>1 <vnum>2
void runResetCmdX(zoneData &zone, resetCom &rs, resetFlag flags, bool &mobload, TMonster *&mob, bool &objload, TObj *&obj, bool &last_cmd)
{
  if (rs.arg3 >= 0 && rs.arg3 < 16 && rs.arg1 >= MIN_WEAR && rs.arg1 < MAX_WEAR) {
    zone.armorSets.setArmor(rs.arg3, rs.arg1, rs.arg2);
  }
}

// Z <if flag> <set num> <perc chance>
void runResetCmdZ(zoneData &zone, resetCom &rs, resetFlag flags, bool &mobload, TMonster *&mob, bool &objload, TObj *&obj, bool &last_cmd)
{
  if ((flags & resetFlagFindLoadPotential))
  {
    for (wearSlotT i = MIN_WEAR; i < MAX_WEAR; i++)
      if (zone.armorSets.getArmor(rs.arg1, i) != 0)
        tallyObjLoadPotential(zone.armorSets.getArmor(rs.arg1, i));
    return;
  }

  if (mob && mobload && rs.arg1 >=0)
  {
    for (wearSlotT i = MIN_WEAR; i < MAX_WEAR; i++)
      if (zone.armorSets.getArmor(rs.arg1, i) != 0)
        loadsetCheck(mob, zone.armorSets.getArmor(rs.arg1, i),(rs.arg2 >= 98) ? rs.arg2 : fixed_chance, i, "(null... for now)");
  }
}

void runResetCmdY(zoneData &zone, resetCom &rs, resetFlag flags, bool &mobload, TMonster *&mob, bool &objload, TObj *&obj, bool &last_cmd)
{
  if ((flags & resetFlagFindLoadPotential))
  {
    mob->loadSetEquipment(rs.arg1, NULL, (rs.arg2 >= 98) ? rs.arg2 : fixed_chance, true);
    return;
  }

  if (!mob || !mobload)
    return; // log error here?

  mob->loadSetEquipment(rs.arg1, NULL, (rs.arg2 >= 98) ? rs.arg2 : fixed_chance);

  if (!Config::LoadOnDeath() && mob->hasClass(CLASS_MAGE)) {
    TSpellBag *tBagA = NULL,
              *tBagB = NULL;
    TThing    *tThing=NULL;

    // Find Held Spellbag
    for(StuffIter it=mob->stuff.begin();it!=mob->stuff.end() && (tThing=*it);++it)
      if ((tBagA = dynamic_cast<TSpellBag *>(tThing)))
        break;

    // Find Worn Spellbag
    for (wearSlotT tWear = MIN_WEAR; tWear < MAX_WEAR; tWear++)
      if (mob->equipment[tWear] &&
          (tBagB = dynamic_cast<TSpellBag *>(mob->equipment[tWear])))
        break;

    if (tBagA && tBagB) {
      for(StuffIter itt=tBagA->stuff.begin();itt!=tBagA->stuff.end();){
	tThing=*(itt++);
        --(*tThing);
        *tBagB += *tThing;
      }

      --(*tBagA);
      delete tBagA;
      tBagA = NULL;
    }
  }
  last_cmd = true;
}

void runResetCmdF(zoneData &zone, resetCom &rs, resetFlag flags, bool &mobload, TMonster *&mob, bool &objload, TObj *&obj, bool &last_cmd)
{
  last_cmd = mob ? mob->addFears(zoneHateT(rs.arg1), rs.arg2) : 0;
}


void runResetCmdD(zoneData &zone, resetCom &rs, resetFlag flags, bool &mobload, TMonster *&mob, bool &objload, TObj *&obj, bool &last_cmd)
{          
  TRoom *rp = real_roomp(rs.arg1);
  if (!rp)
    return;

  roomDirData * exitp = rp->dir_option[rs.arg2];
  if (!exitp || IS_SET(exitp->condition, EX_DESTROYED) ||
      IS_SET(exitp->condition, EX_CAVED_IN) || exitp->door_type == DOOR_NONE)
  {
    vlogf(LOG_LOW, format("'D' command operating on DOOR_NONE in room %d") %  rp->number);
    return;
  }

  switch (rs.arg3)
  {
    case 0:
      if (IS_SET(exitp->condition, EX_CLOSED))
        sendrpf(rp, "The %s opens.\n\r", exitp->getName().uncap().c_str());
      REMOVE_BIT(exitp->condition, EX_LOCKED);
      REMOVE_BIT(exitp->condition, EX_CLOSED);
      break;
    case 1:
      if (!IS_SET(exitp->condition, EX_CLOSED))
        sendrpf(rp, "The %s closes.\n\r", exitp->getName().uncap().c_str());
      SET_BIT(exitp->condition, EX_CLOSED);
      REMOVE_BIT(exitp->condition, EX_LOCKED);
      break;
    case 2:
      if (exitp->key < 0) 
        vlogf(LOG_LOW, format("Door with key < 0 set to lock in room %d.") % rp->number);
      if (!IS_SET(exitp->condition, EX_CLOSED))
        sendrpf(rp, "The %s closes.\n\r", exitp->getName().uncap().c_str());
      SET_BIT(exitp->condition, EX_LOCKED);
      SET_BIT(exitp->condition, EX_CLOSED);
      break;
    default:
      vlogf(LOG_LOW, format("Error in 'D' command in room %d - bad arg3 parameter of %i.") % rp->number % rs.arg3);
      break;
  }

  last_cmd = true;
}

void runResetCmdL(zoneData &zone, resetCom &rs, resetFlag flags, bool &mobload, TMonster *&mob, bool &objload, TObj *&obj, bool &last_cmd)
{
  last_cmd = (flags & resetFlagBootTime) ? sysLootLoad(rs, mob, obj, false) : false;
}

// marks an object as a 'prop' - not for player use
void markProp(TObj *obj)
{
  if (!obj)
    return;

  obj->addObjStat(ITEM_NORENT | ITEM_NEWBIE | ITEM_NOLOCATE | ITEM_PROTOTYPE);
  obj->obj_flags.cost = 0;
}

// loads eq, just like 'E' command and then marks as 'prop' (counts as regular eq: best to load specialized eq)
// note: this eq loads without buy_build operations and equips it even if LoadOnDeath is configured on
void runResetCmdI(zoneData &zone, resetCom &rs, resetFlag flags, bool &mobload, TMonster *&mob, bool &objload, TObj *&obj, bool &last_cmd)
{
  runResetCmdE(zone, rs, flags & resetFlagPropLoad, mobload, mob, objload, obj, last_cmd);
  if (obj && last_cmd)
    markProp(obj);
}

// loads a local set of eq as prop objects same syntax as 'Z' (see 'I' cmd for more info on props) 
void runResetCmdJ(zoneData &zone, resetCom &rs, resetFlag flags, bool &mobload, TMonster *&mob, bool &objload, TObj *&obj, bool &last_cmd)
{
  if ((flags & resetFlagFindLoadPotential))
  {
    for (wearSlotT i = MIN_WEAR; i < MAX_WEAR; i++)
      if (zone.armorSets.getArmor(rs.arg1, i) != 0)
        tallyObjLoadPotential(zone.armorSets.getArmor(rs.arg1, i));
    return;
  }

  if (mob && mobload && rs.arg1 >=0)
    for (wearSlotT i = MIN_WEAR; i < MAX_WEAR; i++)
      if (zone.armorSets.getArmor(rs.arg1, i) != 0)
        if (loadsetCheck(mob, zone.armorSets.getArmor(rs.arg1, i),(rs.arg2 >= 98) ? rs.arg2 : fixed_chance, i, "(null... for now)", flags & resetFlagPropLoad))
          markProp(dynamic_cast<TObj*>(mob->equipment[i])); // assume: loadsetCheck returning true = obj on mob in that slot
}

void zoneData::resetZone(bool bootTime, bool findLoadPotential)
{
  bool last_cmd = true;
  bool mobload = false;
  bool objload = false;
  bool lastStuck = false;
  TMonster *mob = NULL;
  TObj *obj = NULL;
  resetFlag flags = resetFlagNone;

  if (this->enabled == FALSE && gamePort == Config::Port::PROD) {
    if (bootTime)
      vlogf(LOG_MISC, "*** Zone was disabled.");
    return;
  }
  
  if (bootTime)
    flags |= resetFlagBootTime;
  if (findLoadPotential)
    flags |= resetFlagFindLoadPotential;

  armorSets.resetArmor();
  random_room = -1;

  if(!bootTime) {
    vlogf(LOG_SILENT, format("Resetting zone '%s' (rooms %d-%d).") % name % bottom % top);
    update_commod_index();
  }

  for (int cmd_no = 0;cmd_no < (int)cmd.size(); cmd_no++) {
    resetCom &rs = cmd[cmd_no];
    rs.cmd_no = cmd_no;

    // skip non-load commands when checking load potentials
    if (findLoadPotential && !rs.hasLoadPotential())
      continue;

    // skip commands that are dependant on failed prev commands
    if (!last_cmd && rs.if_flag)
      continue;

    // save commands on mobs for later load
    if (mobload && mob && rs.shouldStickToMob(lastStuck)) {
      mob->loadCom.push_back(rs);
      continue;
    }

    // simplify the room assignment logic - if they use a random room, just assign them the proper roomId
    bool random = rs.usesRandomRoom();
    if (random && random_room > 0)
      rs.arg3 = random_room;

    // execute this command
    bool ret = rs.execute(*this, flags, mobload, mob, objload, obj, last_cmd);

    // restore random
    if (random)
      rs.arg3 = ZONE_ROOM_RANDOM;

    if (!ret)
      break;
  }

  if (!findLoadPotential)
    doGenericReset(); // sends CMD_GENERIC_RESET to all objects in zone

  this->age = 0;
}


bool zoneData::doGenericReset(void) 
{
  int top = 0;
  int bottom = 0;
  int rc;
  
  if (zone_nr < 0 || zone_nr >= (signed int) zone_table.size())
  {
    vlogf(LOG_BUG, format("Bad zone number in doGenericReset (%d)") %  zone_nr);
    return FALSE;
  }
  bottom = zone_nr ? (zone_table[zone_nr - 1].top + 1) : 0;
  top = zone_table[zone_nr].top;

  TObj *o;
  //  TTrashPile *pile;
  for(TObjIter iter=object_list.begin();iter!=object_list.end();++iter){
    o=*iter;
    if (o->objVnum() >= bottom && o->objVnum() <= top)
    {
      if (o->spec)
      {
        rc = o->checkSpec(NULL, CMD_GENERIC_RESET, "", NULL);
        if (IS_SET_DELETE(rc, DELETE_ITEM)) {
          delete o;
          o = NULL;
          continue;
        }
      }

      // disabled via mudadmin resolution 222, April 20th, 2005
      //      if((pile=dynamic_cast<TTrashPile *>(o)))
      //        pile->attractVermin();
    }
  }
  return TRUE;
}


// echos a sstring to every room in the zone except exclude_room
void zoneData::sendTo(sstring s, int exclude_room)
{
  TBeing *tmp_victim, *temp;
  
  for (tmp_victim = character_list; tmp_victim; tmp_victim = temp) {
    temp = tmp_victim->next;
    
    if(tmp_victim->roomp->getZoneNum() == zone_nr &&
       tmp_victim->in_room != exclude_room){
      tmp_victim->sendTo(COLOR_BASIC, s);
    }
  }
}


// for use in resetZone; return TRUE if zone 'nr' is free of PC's  
bool zoneData::isEmpty(void)
{
  Descriptor *i;

  for (i = descriptor_list; i; i = i->next)
    if (!i->connected && i->character && i->character->roomp)
      if (i->character->roomp->getZoneNum() == zone_nr)
        return (false);

  return (true);
}

// I Could have give fread_string an additional argument to
// tell it whether to allocate or not, but I'd rather just
// do this instead - Russ
void readStringNoAlloc(FILE *fp)
{
  char buf[MAX_STRING_LENGTH], *ptr, *marker = NULL;

  *buf = 0;
  ptr = buf;
  while(fgets(ptr, MAX_STRING_LENGTH, fp)) {
    //  Check if we've hit the end of sstring marker. 
    if ((marker=strchr(ptr, '~')) != 0) 
      break;
    //  Set the pointer to the end of the sstring. NOTE: This is better then
    // the strlen because we're not starting at the beggining every time. 
    if ((ptr = strchr(ptr, '\000')) == 0) {
      vlogf(LOG_FILE, "fread_string(): read error.");
      return;
    }
    //  Add the return char. 
    *ptr++ = '\r';
  }
  *marker = 0;   // Nuke the ~ 
}

// read and allocate space for a '~'-terminated sstring from a given file 
char *fread_string(FILE *fp)
{
  char buf[MAX_STRING_LENGTH], *ptr, *marker = NULL;

  *buf = 0;
  ptr = buf;
  unsigned int read_len = MAX_STRING_LENGTH;
  while(fgets(ptr, read_len, fp)) {
    //  Check if we've hit the end of sstring marker. 
    if((marker=strchr(ptr, '~')) != 0) {
      break;
    }  
    //  Set the pointer to the end of the sstring. NOTE: This is better then the
    // strlen because we're not starting at the beggining every time. 
    if((ptr = strchr(ptr, '\000')) == 0) {
      vlogf(LOG_FILE, "fread_string(): read error. ack!");
      return mud_str_dup("Empty");
    }
    //  Add the return char. 
    *ptr++ = '\r';

    if ((int) (ptr - buf) >= (int) sizeof(buf)) {
    vlogf(LOG_MISC, "SHIT! buf overflow!");
      vlogf(LOG_BUG, "buf overflow");
    }

    read_len = MAX_STRING_LENGTH - (ptr-buf);
  }
  if (marker)
    *marker = 0;   // Nuke the ~ 
    // if ((int) (ptr - buf) == 0) {
      // vlogf(LOG_MISC, "(int) (ptr - buf) == 0");
      // return NULL;
      //    }
    if (*buf == 0)
      return NULL;
    return mud_str_dup(buf);
    vlogf(LOG_MISC, "mud_str_dup called");
}

// read contents of a text file, and place in buf 
bool file_to_sstring(const char *name, sstring &buf, concatT concat)
{
  FILE *fl;
  char tmp[256];

  if (!concat)
    buf = "";

  if (!(fl = fopen(name, "r"))) {
    return false;
  }
  do {
    fgets(tmp, 256, fl);

    if (!feof(fl)) {
      *(tmp + strlen(tmp) + 1) = '\0';
      *(tmp + strlen(tmp)) = '\r';
      buf += tmp;
    }
  }
  while (!feof(fl));

  fclose(fl);

  return true;
}

TRoom *real_roomp(int virt)
{
  return ((virt < WORLD_SIZE) && (virt > -1)) ? room_db[virt] : 0;
}


// returns the real number of the monster with given virtual number 
int real_mobile(int virt)
{
  int bot, top, mid;

  bot = 0;
  top = mob_index.size()-1;

  // perform binary search on mob-table 
  for (;;) {
    mid = (bot + top) / 2;

    if (mob_index[mid].virt == virt)
      return (mid);
    if (bot >= top)
      return (-1);
    if (mob_index[mid].virt > virt)
      top = mid - 1;
    else
      bot = mid + 1;
  }
}


// returns the real number of the object with given virtual number 
int real_object(int virt)
{
  int bot, top, mid;

  bot = 0;
  top = obj_index.size()-1;

  // perform binary search on obj-table 
  for (;;) {
    mid = (bot + top) / 2;

    if (obj_index[mid].virt == virt)
      return (mid);
    if (bot >= top) {
      vlogf(LOG_SILENT, format("real_object: probable failure for %d") %  virt);
      return (-1);
    }
    if (obj_index[mid].virt > virt)
      top = mid - 1;
    else
      bot = mid + 1;
  }
}

TObj * makeNewObj(itemTypeT tmp)
{
  switch (tmp) {
    case ITEM_LIGHT:
      return new TLight();
    case ITEM_SCROLL:
      return new TScroll();
    case ITEM_WAND:
      return new TWand();
    case ITEM_STAFF:
      return new TStaff();
    case ITEM_WEAPON:
      return new TGenWeapon();
    case ITEM_FUEL:
      return new TFuel();
    case ITEM_OPAL:
      return new TOpal();
    case ITEM_TREASURE:
      return new TTreasure();
    case ITEM_ARMOR:
      return new TArmor();
    case ITEM_POTION:
      return new TPotion();
    case ITEM_WORN:
      return new TWorn();
    case ITEM_OTHER:
      return new TOtherObj();
    case ITEM_TRASH:
      return new TTrash();
    case ITEM_TRAP:
      return new TTrap();
    case ITEM_CHEST:
      return new TChest();
    case ITEM_NOTE:
      return new TNote();
    case ITEM_DRINKCON:
      return new TDrinkCon();
    case ITEM_KEY:
      return new TKey();
    case ITEM_FOOD:
      return new TFood();
    case ITEM_MONEY:
      return new TMoney();
    case ITEM_PEN:
      return new TPen();
    case ITEM_BOAT:
      return new TBoat();
    case ITEM_AUDIO:
      return new TAudio();
    case ITEM_BOARD:
      return new TBoard();
    case ITEM_BOW:
      return new TBow();
    case ITEM_ARROW:
      return new TArrow();
    case ITEM_BAG:
      return new TBag();
    case ITEM_CORPSE:
      return new TCorpse();
    case ITEM_SPELLBAG:
      return new TSpellBag();
    case ITEM_KEYRING:
      return new TKeyring();
    case ITEM_COMPONENT:
      return new TComponent();
    case ITEM_BOOK:
      return new TBook();
    case ITEM_PORTAL:
      return new TPortal();
    case ITEM_WINDOW:
      return new TWindow();
    case ITEM_TREE:
      return new TTree();
    case ITEM_TOOL:
      return new TTool();
    case ITEM_HOLY_SYM:
      return new TSymbol();
    case ITEM_QUIVER:
      return new TQuiver();
    case ITEM_BANDAGE:
      return new TBandaid();
    case ITEM_STATUE:
      return new TStatue();
    case ITEM_BED:
      return new TBed();
    case ITEM_TABLE:
      return new TTable();
    case ITEM_RAW_MATERIAL:
      return new TCommodity();
    case ITEM_GEMSTONE:
      return new TGemstone();
    case ITEM_JEWELRY:
      return new TJewelry();
    case ITEM_VIAL:
      return new TVial();
    case ITEM_PCORPSE:
      return new TPCorpse();
    case ITEM_POOL:
      return new TPool();
    case ITEM_RAW_ORGANIC:
      return new TOrganic();
    case ITEM_FLAME:
      return new TFFlame();
    case ITEM_APPLIED_SUB:
      return new TASubstance();
    case ITEM_GAS:
      return new TGas();
    case ITEM_ARMOR_WAND:
      return new TArmorWand();
    case ITEM_DRUG_CONTAINER:
      return new TDrugContainer();
    case ITEM_DRUG:
      return new TDrug();
    case ITEM_GUN:
      return new TGun();
    case ITEM_AMMO:
      return new TAmmo();
    case ITEM_PLANT:
      return new TPlant();
    case ITEM_COOKWARE:
      return new TCookware();
    case ITEM_VEHICLE:
      return new TVehicle();
    case ITEM_CARD_DECK:
      return new TCardDeck();
    case ITEM_CASINO_CHIP:
      return new TCasinoChip();
    case ITEM_POISON:
      return new TPoison();
    case ITEM_HANDGONNE:
      return new THandgonne();
    case ITEM_EGG:
      return new TEgg();
    case ITEM_CANNON:
      return new TCannon();
    case ITEM_TOOTH_NECKLACE:
      return new TToothNecklace();
    case ITEM_TRASH_PILE:
      return new TTrashPile();
    case ITEM_SUITCASE:
      return new TSuitcase();
    case ITEM_SADDLE:
      return new TSaddle();
    case ITEM_HARNESS:
      return new THarness();
    case ITEM_SADDLEBAG:
      return new TSaddlebag();
    case ITEM_WAGON:
      return new TWagon();
    case ITEM_MONEYPOUCH:
      return new TMoneypouch();
    case ITEM_FRUIT:
      return new TFruit();
    case ITEM_UNDEFINED:
    case ITEM_MARTIAL_WEAPON:
    case MAX_OBJ_TYPES:
      vlogf(LOG_BUG, format("Unknown item type (%d)") %  tmp);
  }
  return NULL;
}

resetCom::exec_fn *resetCom::executeMethods[resetCom::cmd_Max];

resetCom::resetCom() :
  command('\0'),
  if_flag(0),
  arg1(0),
  arg2(0),
  arg3(0),
  arg4(0),
  character('\0')
{
  static bool v_isInitialized = false;
  if (!v_isInitialized)
  {
    memset(executeMethods, 0, sizeof(executeMethods));
    executeMethods[cmd_Stop] = NULL;
    executeMethods[cmd_LoadMob] = runResetCmdM;
    executeMethods[cmd_LoadMobGrouped] = runResetCmdK;
    executeMethods[cmd_LoadMobCharmed] = runResetCmdC;
    executeMethods[cmd_LoadMobRidden] = runResetCmdR;
    executeMethods[cmd_SetRandomRoom] = runResetCmdA;
    executeMethods[cmd_LoadChance] = runResetCmdQMark;
    executeMethods[cmd_LoadObjGround] = runResetCmdB;
    executeMethods[cmd_LoadObjGroundBoot] = runResetCmdO;
    executeMethods[cmd_LoadObjPlaced] = runResetCmdP;
    executeMethods[cmd_LoadObjInventory] = runResetCmdG;
    executeMethods[cmd_LoadObjEquipped] = runResetCmdE;
    executeMethods[cmd_CreateLocalSet] = runResetCmdX;
    executeMethods[cmd_LoadObjSetLocal] = runResetCmdZ;
    executeMethods[cmd_LoadObjSet] = runResetCmdY;
    executeMethods[cmd_ChangeFourValues] = runResetCmdV;
    executeMethods[cmd_SetTrap] = runResetCmdT;
    executeMethods[cmd_SetHate] = runResetCmdH;
    executeMethods[cmd_SetFear] = runResetCmdF;
    executeMethods[cmd_SetDoor] = runResetCmdD;
    executeMethods[cmd_LoadLoot] = runResetCmdL;
    executeMethods[cmd_LoadObjEquippedProp] = runResetCmdI;
    executeMethods[cmd_LoadObjSetLocalProp] = runResetCmdJ;
  }
}

resetCom::resetCom(const resetCom &t) :
  command(t.command),
  if_flag(t.if_flag),
  arg1(t.arg1),
  arg2(t.arg2),
  arg3(t.arg3),
  arg4(t.arg4),
  character(t.character),
  cmd_no(0)
{
}

resetCom::~resetCom()
{
}

resetCom & resetCom::operator =(const resetCom &t)
{
  if (this == &t) return *this;

  command = t.command;
  if_flag = t.if_flag;
  character = t.character;
  arg1 = t.arg1;
  arg2 = t.arg2;
  arg3 = t.arg3;
  arg4 = t.arg4;
  arg1 = t.arg1;
  cmd_no = t.cmd_no;

  return *this;
}

// helper func for resetCom::hasLoadPotential
bool comHasLoadPotential(const char cmd)
{
  switch(cmd)
  {
  case 'E':
  case 'G':
  case 'Y':
  case 'Z':
  case 'X':
  case 'I':
  case 'J':
    return true;
  }
  return false;
}

// true if this command possibly affects load potentials
bool resetCom::hasLoadPotential()
{
  if (command == '?')
    return comHasLoadPotential(character);
  return comHasLoadPotential(command); 
}

// returns if this command uses the last set random room
bool resetCom::usesRandomRoom()
{
  return (command == 'C' || command == 'K' || command == 'M' ||
    command == 'R' || command == 'O' || command == 'B') && arg3 == ZONE_ROOM_RANDOM;
}

// returns true if we should stick this to a mob instead of execute during reset
// lastComStuck - true if the previous resetCom before this one was 'stuck' to a mob
bool resetCom::shouldStickToMob(bool &lastComStuck)
{
  if (!Config::LoadOnDeath())
    return lastComStuck = false;

  // special case: since V's modify a potentially stuck (non-existant) obj, we need to stick ?'s for V's too
  if (command == '?' && character == 'V' && lastComStuck)
    return lastComStuck = true;

  // all other '?' commands we should just execute it to let further commands stick or not
  if (command == '?')
    return lastComStuck = false;

  // Prop loads should never stick
  if (command == 'I' || command == 'J')
    return lastComStuck = false;

  return lastComStuck = (hasLoadPotential() ||
    (command == 'P' && lastComStuck) ||
    (command == 'L' && arg4 == 0) ||
    (command == 'V' && lastComStuck) ||
    (command == 'T' && if_flag && lastComStuck));
}

resetCom::resetCommandId resetCom::getCommandId()
{
  switch (command)
  {
    case 'S': return cmd_Stop;
    case 'M': return cmd_LoadMob;
    case 'K': return cmd_LoadMobGrouped;
    case 'C': return cmd_LoadMobCharmed;
    case 'R': return cmd_LoadMobRidden;
    case 'A': return cmd_SetRandomRoom;
    case '?': return cmd_LoadChance;
    case 'B': return cmd_LoadObjGround;
    case 'O': return cmd_LoadObjGroundBoot;
    case 'P': return cmd_LoadObjPlaced;
    case 'G': return cmd_LoadObjInventory;
    case 'E': return cmd_LoadObjEquipped;
    case 'X': return cmd_CreateLocalSet;
    case 'Z': return cmd_LoadObjSetLocal;
    case 'Y': return cmd_LoadObjSet;
    case 'V': return cmd_ChangeFourValues;
    case 'T': return cmd_SetTrap;
    case 'H': return cmd_SetHate;
    case 'F': return cmd_SetFear;
    case 'D': return cmd_SetDoor;
    case 'L': return cmd_LoadLoot;
  }
  return cmd_Stop;
}

bool resetCom::execute(zoneData &zone, resetFlag flags, bool &mobload, TMonster *&mob, bool &objload, TObj *&obj, bool &last_cmd)
{
  if (!executeMethods[getCommandId()])
    return false;

  executeMethods[getCommandId()](zone, *this, flags, mobload, mob, objload, obj, last_cmd);
  return true;
}


zoneData::zoneData() :
  name(NULL),
  zone_nr(0),
  lifespan(0),
  age(0),
  bottom(0),
  top(0),
  reset_mode(0),
  enabled(false),
  zone_value(0),
  num_mobs(0),
  mob_levels(0),
  min_mob_level(128),
  max_mob_level(0),
  stat_mobs_total(0),
  stat_mobs_unique(0),
  stat_objs_unique(0),
  cmd(0)
{
}

zoneData::zoneData(const zoneData &t) :
  zone_nr(t.zone_nr),
  lifespan(t.lifespan),
  age(t.age),
  bottom(t.bottom),
  top(t.top),
  reset_mode(t.reset_mode),
  enabled(t.enabled),
  zone_value(t.zone_value),
  num_mobs(t.num_mobs),
  mob_levels(t.mob_levels),
  min_mob_level(t.min_mob_level),
  max_mob_level(t.max_mob_level),
  stat_mobs(t.stat_mobs),
  stat_objs(t.stat_objs),
  stat_mobs_total(t.stat_mobs_total),
  stat_mobs_unique(t.stat_mobs_unique),
  stat_objs_unique(t.stat_objs_unique),
  cmd(t.cmd)
{
  name = mud_str_dup(t.name);
}

zoneData::~zoneData()
{
  delete [] name;
  cmd.erase(cmd.begin(), cmd.end());
  stat_mobs.clear();
  stat_objs.clear();
}

zoneData & zoneData::operator= (const zoneData &t)
{
  if (this == &t) return *this;

  delete [] name;
  name = mud_str_dup(t.name);

  zone_nr = t.zone_nr;
  lifespan = t.lifespan;
  age = t.age;
  bottom = t.bottom;
  top = t.top;
  reset_mode = t.reset_mode;
  enabled = t.enabled;
  zone_value = t.zone_value;
  num_mobs = t.num_mobs;
  mob_levels = t.mob_levels;
  min_mob_level = t.min_mob_level;
  max_mob_level = t.max_mob_level;
  stat_mobs.clear();
  stat_mobs = t.stat_mobs;
  stat_objs.clear();
  stat_objs = t.stat_objs;
  stat_mobs_total = t.stat_mobs_total;
  stat_mobs_unique = t.stat_mobs_unique;
  stat_objs_unique = t.stat_objs_unique;
  cmd.erase(cmd.begin(), cmd.end());
  cmd = t.cmd;

  return *this;
}

// we have a number of global structs holding heap memory
// this is here to clean up and release the heap memory
// it is intended to be called just before exiting
// obviously, this isn't REALLY necessary since exit frees heap anyway
// but memory tools (insure/purify) will otherwise say we didn't free memory
void generic_cleanup()
{
#ifdef __INSURE__
  vlogf(LOG_SILENT, "generic_cleanup called");
  unsigned int i;

  // purge all mobs
  while (character_list)
    delete character_list;
  // purge all objs
  for(TObjIter iter=object_list.begin();iter!=object_list.end();++iter){
    delete *iter;

  // purge all rooms
  int ii;
  for (ii = 0; ii < WORLD_SIZE; ii++)
    delete room_db[ii];

  // no TThings exist anymore at this point

  // close up open handlers
  if (mob_f)
    fclose(mob_f);

  // delete some arrays that allocated heap memory
  for (i = 0; i <= (unsigned int) MAX_SKILL; i++)
    delete discArray[i];

  for (i = 0; i < (unsigned int) MAX_SECTOR_TYPES; i++)
    delete TerrainInfo[i];

extern void cleanUpHelp();
  cleanUpHelp();

  for(race_t rindex=RACE_NORACE;rindex<MAX_RACIAL_TYPES;rindex++)
    delete Races[rindex];

  for (ii=0; ii < MAX_CMD_LIST; ii++)
    delete commandArray[ii];

  for (ii=0; ii < MAX_OBJ_TYPES; ii++)
    delete ItemInfo[ii];

extern void cleanUpMail();
  cleanUpMail();
#endif
}


