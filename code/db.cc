//////////////////////////////////////////////////////////////////////////
//
// SneezyMUD - All rights reserved, SneezyMUD Coding Team
//
//    "db.cc" - All functions and routines  related to tinyworld databases
//
//////////////////////////////////////////////////////////////////////////

#include "stdsneezy.h"

#include <sys/types.h>
#include <dirent.h>

#include "socket.h"
#include "statistics.h"
#include "help.h"
#include "mail.h"
#include "obj_component.h"
#include "stdsneezy.h"
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
#include "obj_smoke.h"
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

int top_of_world = 0;         // ref to the top element of world 

TRoom *room_db[WORLD_SIZE];

TObjList object_list; // the global linked list of obj's 

int commod_index[200];

TBeing *character_list = 0; // global l-list of chars          
TMonster *pawnman = NULL;
TPCorpse *pc_corpse_list = NULL;
// table of reset data 
vector<zoneData>zone_table(0);

liqInfoT liquidInfo;

int top_of_script = 0;

// Lots o' global variables!!!!!! - Russ 
long total_bc = 0;
long roomCount = 0;
long mobCount = 0;
long objCount = 0;
bool no_mail = 0;
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


vector<TRoom *>roomspec_db(0);
vector<TRoom *>roomsave_db(0);


struct cached_object { int number;map <sstring, sstring> s; };

class TObjectCache {
public:
  map<int, cached_object *>cache;

  void preload(void);
  cached_object *operator[](int);

} obj_cache;


bool bootTime=false;

struct time_info_data time_info;        // the infomation about the time   
struct weather_data weather_info;        // the infomation about the weather 
class lag_data lag_info;

// count of the number of mobs that can load an object
map<int, int> obj_load_potential;

// local procedures
static void bootZones(void);
static void bootWorld(void);
static void reset_time(void);

// fish init
void initialize_fish_records(void);

struct reset_q_type
{
  resetQElement *head;
  resetQElement *tail;
} r_q;

void update_commod_index()
{
  TCommodity *comm;
  TDatabase db(DB_SNEEZY);
  TMonster *keeper, *tm;
  TBeing *tb;
  int count=0;

  for(int i=0;i<200;++i)
    commod_index[i]=0;

  // we want to use this at boot time, and the shop tables aren't setup yet
  db.query("select s.keeper as keeper from shoptype st, shop s where st.type=42 and s.shop_nr=st.shop_nr");

  while(db.fetchRow()){
    count++;
    keeper=NULL;
    for(tb=character_list;tb;tb=tb->next){

      if((tm=dynamic_cast<TMonster *>(tb))){
	if(tm->mobVnum()==convertTo<int>(db["keeper"])){
	  keeper=tm;
	  break;
	}
      }
    }

    if(!keeper)
      continue;

    for(TThing *t=keeper->getStuff();t;t=t->nextThing){
      if((comm=dynamic_cast<TCommodity *>(t))){
	commod_index[comm->getMaterial()]+=comm->numUnits();
      }
    }
  }

  for(int i=0;i<200;++i)
    commod_index[i]/=count;

}

int getObjLoadPotential(const int obj_num)
{
  map<int, int>::iterator tIter;
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
  map<int, int>::iterator tIter;

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
    d->output.putInQ(colorString(NULL, d, sc, NULL, COLOR_BASIC, TRUE));
    d->outputProcessing();
  }

  if (str && strcmp(str, "."))
    vlogf(LOG_MISC, fmt("%s") %  str);
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
    
    vlogf(LOG_MISC, fmt("[%6i] %-17s") % count[li] % ItemInfo[li]->name);
    total += count[li];
    count[li]=-1;
  }
  
  vlogf(LOG_MISC, fmt("[%6i] %-17s") % total % "Total");
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
  reset_time();
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

  vlogf(LOG_MISC, fmt("Boot timing: misc 1: %.2f seconds") % 
      (t.getElapsedReset()));

  bootPulse("Generating index tables for mobile file.");
  generate_mob_index();

  bootPulse("Generating index tables for object file.");
  generate_obj_index();

  vlogf(LOG_MISC, fmt("Boot timing: mob/obj indexes: %.2f seconds") % (t.getElapsedReset()));

  bootPulse("Pre-loading object cache.");
  obj_cache.preload();

  vlogf(LOG_MISC, fmt("Boot timing: obj cache: %.2f seconds") % (t.getElapsedReset()));

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
  
  vlogf(LOG_MISC, fmt("Boot timing: misc 2: %.2f seconds") % (t.getElapsedReset()));

  unsigned int i;
  bootPulse("Loading rooms:", false);
  bootWorld();
  bootPulse(NULL, true);

  vlogf(LOG_MISC, fmt("Boot timing: rooms: %.2f seconds") % (t.getElapsedReset()));

  vlogf(LOG_MISC, "Assigning function pointers:");
  vlogf(LOG_MISC, "   Shopkeepers.");
  bootTheShops();

  vlogf(LOG_MISC, fmt("Boot timing: shops: %.2f seconds") % (t.getElapsedReset()));

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

  vlogf(LOG_MISC, fmt("Boot timing: misc 3: %.2f seconds") % (t.getElapsedReset()));

  bootPulse("Updating characters with saved items:", false);
  updateRentFiles();
  bootPulse("Processing shop-save files.");
  processShopFiles();
  bootPulse("Processing repair-save files.");
  processRepairFiles();
  bootPulse("Processing saved-room files.");
  updateSavedRoomItems();
  bootPulse("Processing corpse-save files.");
  processCorpseFiles();

  vlogf(LOG_MISC, fmt("Boot timing: save files: %.2f seconds") % (t.getElapsedReset()));

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

    vlogf(LOG_MISC, fmt("Calculating object load potentials of %s (rooms %d-%d).") % zone_table[i].name % d % e);
    zone_table[i].resetZone(true, true);

    if (i%10 == 0)
      bootPulse(".", false);
  }
  bootPulse(NULL, true);

  vlogf(LOG_MISC, fmt("Object load potentials:"));
  map<int, int>::iterator tIter = obj_load_potential.begin();
  while (tIter != obj_load_potential.end()) {
    vlogf(LOG_MISC, fmt("VNum[%d] = %d") % tIter->first % tIter->second);
    ++tIter;
  }

  vlogf(LOG_MISC, fmt("Boot timing: load potentials: %.2f seconds") % (t.getElapsedReset()));

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


    vlogf(LOG_MISC, fmt("Performing boot-time reset of %s (rooms %d-%d).") % zone_table[i].name % d % e);
    zone_table[i].resetZone(TRUE);

    // stagger reset times
    zone_table[i].age = ::number(0, zone_table[i].lifespan);

    if (i%10 == 0)
      bootPulse(".", false);
  }
  bootPulse(NULL, true);

  vlogf(LOG_MISC, fmt("Boot timing: zones and shop rent: %.2f seconds") % (t.getElapsedReset()));

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
  sunriseAndSunset();

  r_q.head = r_q.tail = 0;

  vlogf(LOG_MISC, fmt("Boot timing: pfiles: %.2f seconds") % (t.getElapsedReset()));

  bootPulse("Boot -- DONE.");
  bootTime=false;
}


void reset_time(void)
{
  mudTimePassed(time(0), BEGINNING_OF_TIME, &time_info);
  time_info.year += YEAR_ADJUST;
  moontype = time_info.day;

  calcNewSunRise();
  calcNewSunSet();

  extern void fixSunlight();
  fixSunlight();

  vlogf(LOG_MISC, fmt("   Current Gametime: %dm, %dH %dD %dM %dY.") %  
        time_info.minutes % time_info.hours % time_info.day % time_info.month % time_info.year);

  weather_info.pressure = 960;
  if ((time_info.month >= 7) && (time_info.month <= 12))
    weather_info.pressure += dice(1, 50);
  else
    weather_info.pressure += dice(1, 80);

  weather_info.change = 0;

  if (weather_info.pressure <= 980) {
    if ((time_info.month >= 3) && (time_info.month <= 9))
      weather_info.sky = SKY_LIGHTNING;
    else
      weather_info.sky = SKY_LIGHTNING;
  } else if (weather_info.pressure <= 1000) {
    if ((time_info.month >= 3) && (time_info.month <= 9))
      weather_info.sky = SKY_RAINING;
    else
      weather_info.sky = SKY_RAINING;
  } else if (weather_info.pressure <= 1020) {
    weather_info.sky = SKY_CLOUDY;
  } else {
    weather_info.sky = SKY_CLOUDLESS;
  }
}



// procUpdateTime
procUpdateTime::procUpdateTime(const int &p)
{
  trigger_pulse=p;
  name="procUpdateTime";
}

void procUpdateTime::run(int pulse) const
{
  return;
#if 0
  FILE *f1;
  long current_time;

  if (time_info.hours != 1)
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
  fprintf(f1, "%d\n", time_info.minutes);
  fprintf(f1, "%d\n", time_info.hours);
  fprintf(f1, "%d\n", time_info.day);
  fprintf(f1, "%d\n", time_info.month);
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
  db_exits.query("select * from roomexit order by vnum asc");
  db_exits.fetchRow();
  db_extras.query("select * from roomextra order by vnum asc");  
  db_extras.fetchRow();

  while(db.fetchRow()){
    virtual_nr=convertTo<int>(db["vnum"]);

    if (virtual_nr/1000 > num) {
      num = virtual_nr/1000;
      vlogf(LOG_MISC, fmt("Room %ld allocated") %  (num*1000));
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
        vlogf(LOG_EDIT, fmt("Room %d is outside of any zone.\n") % rp->number);
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
        vlogf(LOG_EDIT, fmt("No keyword in room %d\n") %  rp->number);
      
      new_descr->description = mud_str_dup(db_extras["description"]);
      if (!new_descr->description || !*new_descr->description)
        vlogf(LOG_LOW, fmt("No desc in room %d\n") %  rp->number);
      
      new_descr->next = rp->ex_description;
      rp->ex_description = new_descr;

      if(!db_extras.fetchRow())
        break;
    }

    dirTypeT dir;
    for (dir = MIN_DIR; dir < MAX_DIR; dir++)
      rp->dir_option[dir] = 0;

    // in case there are exits with no associated room, we need this
    while(convertTo<int>(db_exits["vnum"]) < rp->number)
      if(!db_exits.fetchRow())
        break;


    while(convertTo<int>(db_exits["vnum"]) == rp->number){
      dir=mapFileToDir(convertTo<int>(db_exits["direction"]));

      rp->dir_option[dir] = new roomDirData();

      if(!db_exits["name"].empty())
        rp->dir_option[dir]->keyword = mud_str_dup(db_exits["name"]);
      else
        rp->dir_option[dir]->keyword = NULL;

      if(!db_exits["description"].empty())
        rp->dir_option[dir]->description = mud_str_dup(db_exits["description"]);
      else
        rp->dir_option[dir]->description = NULL;

      tmp=convertTo<int>(db_exits["type"]);
      if (tmp < 0 || tmp >= MAX_DOOR_TYPES) {
        vlogf(LOG_LOW,fmt("bogus door type (%d) in room (%d) dir %d.") % 
            tmp % rp->number % dir);
        return;
      }
      rp->dir_option[dir]->door_type = doorTypeT(tmp);
      if ((tmp == DOOR_NONE) && (rp->dir_option[dir]->keyword)){
        if (strcmp(rp->dir_option[dir]->keyword, "_unique_door_"))
          vlogf(LOG_LOW,fmt("non-door with name in room %d") % rp->number);
      }
      if ((tmp != DOOR_NONE) && !(rp->dir_option[dir]->keyword)){
        vlogf(LOG_LOW,fmt("door with no name in room %d") % rp->number);
      }

      rp->dir_option[dir]->condition = convertTo<int>(db_exits["condition_flag"]);
      rp->dir_option[dir]->lock_difficulty= convertTo<int>(db_exits["lock_difficulty"]);;
      rp->dir_option[dir]->weight= convertTo<int>(db_exits["weight"]);
      rp->dir_option[dir]->key = convertTo<int>(db_exits["key_num"]);

      rp->dir_option[dir]->to_room = convertTo<int>(db_exits["destination"]);

      if (IS_SET(rp->dir_option[dir]->condition, EX_SECRET) && 
          canSeeThruDoor(rp->dir_option[dir])) {
        if (IS_SET(rp->dir_option[dir]->condition, EX_CLOSED)){
          //vlogf(LOG_LOW, fmt("See thru door set secret. (%d, %d)") %  room % dir);
        } else
          vlogf(LOG_LOW, fmt("Secret door saved as open. (%d, %d)") % 
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

    if ((rp->number == ROOM_NOCTURNAL_STORAGE))
      continue;

    if (rp->isRoomFlag(ROOM_PEACEFUL) && !rp->isRoomFlag(ROOM_NO_HEAL))
      vlogf(LOG_LOW, fmt("%s room %d set peaceful && !no_heal (bit: %d)") % 
                rp->name %rp->number % ROOM_NO_HEAL);
    if (rp->isRoomFlag(ROOM_PEACEFUL) && !rp->isRoomFlag(ROOM_NO_STEAL))
      vlogf(LOG_LOW, fmt("%s room %d set peaceful && !no_steal (bit: %d)") % 
                rp->name %rp->number % ROOM_NO_STEAL);
    if (rp->isRoomFlag(ROOM_PEACEFUL) && !rp->isRoomFlag(ROOM_NO_MAGIC))
      vlogf(LOG_LOW, fmt("%s room %d set PEACEFUL && !no_magic (bit: %d)") % 
                rp->name %rp->number % ROOM_NO_MAGIC);
    if (rp->isRoomFlag(ROOM_NO_HEAL) && rp->isRoomFlag(ROOM_HOSPITAL))
      vlogf(LOG_LOW, fmt("%s room %d set NO_HEAL(%d) and HOSPITAL(%d)") % 
                rp->name %rp->number % ROOM_NO_HEAL % ROOM_HOSPITAL);

    if (rp->isIndoorSector() && !rp->isRoomFlag(ROOM_INDOORS)) {
      // in general, this is an error
      // of course you could have a bldg whose roof has collapsed...
      if (rp->number != 27349)
        vlogf(LOG_LOW,fmt("%s room %d set building & !indoor") % 
                rp->name %rp->number);
    }
    if (rp->isRoomFlag(ROOM_INDOORS) && rp->getRoomHeight() <= 0)
      vlogf(LOG_LOW,fmt("%s indoor room %d set with unlimited height") % 
                rp->name %rp->number);
    if (!rp->isRoomFlag(ROOM_INDOORS) && rp->getRoomHeight() >= 0)
      vlogf(LOG_LOW,fmt("%s outdoor room %d set with limited height") % 
                rp->name %rp->number);

#if 0
    if ((rp->getRoomHeight() >= 0) && rp->isFallSector())
      vlogf(LOG_LOW,fmt("%s fall room %d set with limited height") % 
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
    vlogf(LOG_MISC, fmt("Setup_dir called with bad room number %d") %  room);
    return;
  }
  rp->dir_option[dir] = new roomDirData();

  rp->dir_option[dir]->description = fread_string(fl);
  rp->dir_option[dir]->keyword = fread_string(fl);

  fscanf(fl, " %d ", &tmp);
  if (tmp < 0 || tmp >= MAX_DOOR_TYPES) {
    vlogf(LOG_LOW,fmt("bogus door type (%d) in room (%d) dir %d.") % 
        tmp % room % dir);
    return;
  }
  rp->dir_option[dir]->door_type = doorTypeT(tmp);
  if ((tmp == DOOR_NONE) && (rp->dir_option[dir]->keyword)){
    if (strcmp(rp->dir_option[dir]->keyword, "_unique_door_"))
      vlogf(LOG_LOW,fmt("non-door with name in room %d") % room);
  }
  if ((tmp != DOOR_NONE) && !(rp->dir_option[dir]->keyword)){
    vlogf(LOG_LOW,fmt("door with no name in room %d") % room);
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
      //      vlogf(LOG_LOW, fmt("See thru door set secret. (%d, %d)") %  room % dir);
    } else
      vlogf(LOG_LOW, fmt("Secret door saved as open. (%d, %d)") %  room % dir);
  }


}


void zoneData::logError(char ch, const char *type, int cmd, int value)
{
  vlogf(LOG_LOW, fmt("zone %s cmd %d (%c) resolving %s number (%d)") % 
      name % cmd % ch % type % value);
}

void zoneData::renumCmd(void)
{
  int comm;
  int value;

  // init the zone_value array
  if (nuke_inactive_mobs)
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
  map<int,int>::iterator iter;
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
  TThing *t, *t2;
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


  sendTo(fmt("Rebooting zone %s (%i) (rooms %i-%i)\n\r")
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
        for(t=real_roomp(r)->getStuff();t;t = t2){
          t2 = t->nextThing;
          
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
  int i1 = 0, i2, i3, i4;;
  int rc;
  FILE *fl=fopen((fmt("zonefiles/%i") % zone_nr).c_str(), "r");

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
    vlogf(LOG_LOW, fmt("Bad zone format for zone %d (%s)") % zone_nr % check);
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

    if (rs.command == '*' || gamePort == GAMMA_GAMEPORT) {
      fgets(buf, 255, fl);        
      continue;
    }


    int numc = fscanf(fl, " %d %d %d", &tmp, &rs.arg1, &rs.arg2);
    if (numc != 3)
      vlogf(LOG_LOW,fmt("command %u ('%c') in %s missing some of first three args [%d : %d %d %d]") % 
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
          vlogf(LOG_LOW,fmt("command %u in %s has bogus if_flag") % 
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
        vlogf(LOG_LOW,fmt("command %u ('%c') in %s missing arg3 (rc=%d)") % 
            cmd.size() %
            rs.command %
            name % rc);

    if (rs.command == '?')
      if (fscanf(fl, " %c", &rs.character) != 1)
        vlogf(LOG_LOW,fmt("command %u ('?') in %s missing character") % cmd.size() %name);

    if (rs.command == 'T' && !rs.if_flag) 
      if (fscanf(fl, " %d", &rs.arg4) != 1)
        vlogf(LOG_LOW,fmt("command %u ('T') in %s missing arg4") % 
            cmd.size() % name);

    if (rs.command == 'L')
      if (fscanf(fl, " %d", &rs.arg4) != 1)
        vlogf(LOG_LOW, fmt("command %u ('L') in %s missing arg4") % 
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
  multimap <int, sstring, std::less<int> > files;
  multimap <int, sstring, std::less<int> >::iterator it;
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

    files.insert(pair<int,sstring>(tmp, dp->d_name));
  }
  
  db.query("update zone set util_flag = 0");
  for(it=files.begin();it!=files.end();++it){
    zoneData zd;
    if(zd.bootZone((*it).first)){
      zd.renumCmd();
      vlogf(LOG_MISC, fmt("booting zone %d") % zon);
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
    vlogf(LOG_FILE, fmt("Mobile (V) %d does not exist in database.") %  i);
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
    vlogf(LOG_BUG, fmt("Mobile %d returned DELETE_THIS on init.") %  virt);
    delete mob;
    return NULL;
  } else if (!rc) {
    vlogf(LOG_BUG, fmt("Mobile %d failed to load from database.") %  virt);
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
  map<int, cached_object *>::iterator tIter;
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

  db.query("select vnum, type, action_flag, wear_flag, val0, val1, val2, val3, weight, price, can_be_seen, spec_proc, max_struct, cur_struct, decay, volume, material, max_exist from obj");

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


// the idea here is to search all shops for the object we want to load
// and if we find it at a decent price, buy it.
// if we can't find it, then try to buy some commods to "make it".
// failing that, just load it.
TObj *read_object_buy_build(TBeing *buyer, int nr, readFileTypeT type)
{

  if(bootTime){
    //    vlogf(LOG_BUG, "read_object_buy_build() called during bootTime");
    return read_object(nr, type);
  }

  if (type == VIRTUAL)
    nr = real_object(nr);

  TDatabase db(DB_SNEEZY);

  db.query("select material, weight, short_desc, type, price from obj where vnum=%i",
	   obj_index[nr].virt);
  if(!db.fetchRow()){
    vlogf(LOG_BUG, fmt("didn't find object %i in query") % obj_index[nr].virt);
    return read_object(nr, type);
  }

  int material=convertTo<int>(db["material"]);
  unsigned int item_type=convertTo<int>(db["type"]);
  float weight=convertTo<float>(db["weight"]);
  sstring name=db["short_desc"];
  int indexed_cost=convertTo<int>(db["price"]);

  // check shops for item available < basePrice
  vector <shopData>::iterator iter;
  TThing *t;
  TObj *o, *cheapest=NULL;
  TCommodity *commod, *cheapest_commod=NULL;
  int price, cheapest_price=0, shop_nr=0;
  int basePrice=0;
  int commod_price=0, commod_shop_nr=0;
  bool is_commod_shop=false, is_ok_shop=false;

  for(iter=shop_index.begin();iter!=shop_index.end();++iter){
    TShopOwned tso((*iter).shop_nr, buyer);

    // check shop type
    is_commod_shop=is_ok_shop=false;
    for(unsigned int i=0;i<shop_index[(*iter).shop_nr].type.size();++i){
      if(shop_index[(*iter).shop_nr].type[i] == ITEM_RAW_MATERIAL)
	is_commod_shop=true;
      if(shop_index[(*iter).shop_nr].type[i] == item_type)
	is_ok_shop=true;
    }

    // doesn't deal in commods or the item type we want, so skip it
    if(!is_commod_shop && !is_ok_shop)
      continue;

    // go through the shop inventory
    for(t=tso.getStuff();t;t=t->nextThing){
      if(!(o=dynamic_cast<TObj *>(t)))
	continue;
      
      // check if this object is one we can buy
      if(o->objVnum() == obj_index[nr].virt && is_ok_shop){
	price = o->shopPrice(1, (*iter).shop_nr, -1, buyer);
	basePrice=o->suggestedPrice();

	if(/*price <= basePrice &&*/
	   (price <= cheapest_price || cheapest_price==0)){
	  cheapest_price=price;
	  cheapest=o;
	  shop_nr=(*iter).shop_nr;
	}
      }

      // check if this object is a commod we can use to make our object
      if((commod=dynamic_cast<TCommodity *>(t)) && is_commod_shop){
	price=commod->shopPrice((int)(weight*10), 
				       (*iter).shop_nr, -1, buyer);
	if(commod->getMaterial() == material &&
	   commod->getWeight() >= weight &&
	   (price <= commod_price || commod_price==0)){
	  commod_shop_nr=(*iter).shop_nr;
	  commod_price=price;
	  cheapest_commod=commod;
	}
      }
    }
  }

  // if we have a cheap item, and no commodity to consider, OR
  // we do have a commod and it's more expensive...
  if(cheapest && (!cheapest_commod || 
		  (cheapest_price <= (commod_price+indexed_cost)))){
    TShopOwned tso(shop_nr, buyer);
    --(*cheapest);
    buyer->addToMoney(cheapest_price, GOLD_XFER); // this is to offset cost
    tso.doBuyTransaction(cheapest_price, cheapest->getName(), TX_BUYING, cheapest);
    vlogf(LOG_PEEL, fmt("%s purchased %s from shop %i for %i talens.") %
	  buyer->getName() % cheapest->getName() % shop_nr % cheapest_price);

    return cheapest;
  // otherwise buy the commod if it is available
  } else if(cheapest_commod){
    TShopOwned tso(commod_shop_nr, buyer);
    buyer->addToMoney(commod_price, GOLD_XFER); // this is to offset cost
    tso.doBuyTransaction(commod_price, cheapest_commod->getName(), 
			 TX_BUYING, cheapest_commod);

    cheapest_commod->setWeight(cheapest_commod->getWeight() - weight);
    vlogf(LOG_PEEL, fmt("%s purchased %s (%i) from shop %i for %i talens.") %
	  buyer->getName() % cheapest_commod->getName() % (int)(weight*10) %
	  commod_shop_nr % commod_price);
    return read_object(nr, REAL);    
  }


  return read_object(nr, REAL);
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
    vlogf(LOG_BUG, fmt("read_object: bad nr %d (i = %d)") % nr % i);
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
    obj->max_exist = (gamePort == BETA_GAMEPORT ? 9999 : convertTo<int>(obj_cache[nr]->s["max_exist"]));

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
    obj->max_exist = (gamePort == BETA_GAMEPORT ? 9999 : convertTo<int>(db["max_exist"]));
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
    //    vlogf(LOG_PEEL, fmt("caching object - %s") %  obj->shortDescr);
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

void procZoneUpdate::run(int pulse) const
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
        vlogf(LOG_LOW, fmt("Bogus slot (%d, 1) in zone file") %  num);
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
      vlogf(LOG_LOW, fmt("Bogus slot (%d, 2) in zone file") %  num);
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
    vlogf(LOG_LOW, fmt("Mob (%s:%d) has a utility proc (%s:%d) and is not in zone #0") % 
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

void zoneData::resetZone(bool bootTime, bool findLoadPotential)
{
  int cmd_no;
  bool last_cmd = 1;
  bool mobload = FALSE;
  bool objload = FALSE;
  TMonster *mob = NULL;
  TMonster *old_mob = NULL;
  TObj *obj = NULL, *newobj = NULL;
  TRoom *rp = NULL, *storageRoom = NULL;
  TRoom *random_room = NULL;
  TThing *t;
  int count;
  wearSlotT realslot;
  int obj_lp = 0;
  double obj_lp_ratio, adj_obj_lp_ratio;

  struct armor_set_struct {
    int slots[MAX_WEAR];
  } local_armor[16];
  memset(local_armor, 0, sizeof(struct armor_set_struct)*16);

  if (this->enabled == FALSE && gamePort == PROD_GAMEPORT) {
    if (bootTime)
      vlogf(LOG_MISC, "*** Zone was disabled.");
    return;
  }

  if(!bootTime)
     update_commod_index();

  storageRoom = real_roomp(ROOM_NOCTURNAL_STORAGE);
  for (cmd_no = 0;; cmd_no++) {
    resetCom &rs = this->cmd[cmd_no];
    if (rs.command == 'S')
      break;

    if (last_cmd || !rs.if_flag) {
      switch (rs.command) {
        case 'A':
          if (findLoadPotential) {
            break;
          }
          for (count = 0; count < 10; count++) {
            if ((random_room = real_roomp(::number(rs.arg1, rs.arg2))))
              break;
          }
          if (!random_room) {
            vlogf(LOG_LOW, fmt("Unable to detect room in 'A' %d %d") %  
                 rs.arg1 % rs.arg2);
          }
          break;
        case 'M':
          if (findLoadPotential) {
            break;
          }
          // check if zone is disabled or if mob exceeds absolute max
          if (rs.arg1 < 0 || rs.arg1 >= (signed int) mob_index.size()) {
            vlogf(LOG_LOW, fmt("Detected bogus mob number (%d) on read_mobile call for resetZone (load room %d).") %  rs.arg1 % rs.arg3);
            last_cmd = 0;
            mobload = 0;
            continue;
          }

          if(this->zone_value != 0 && bootTime){
            mob_index[rs.arg1].doesLoad=true;
            mob_index[rs.arg1].numberLoad++;
          }

          if ((this->zone_value != 0) &&
              mob_index[rs.arg1].getNumber() < mob_index[rs.arg1].max_exist) {
            if (rs.arg3 != ZONE_ROOM_RANDOM) {
              rp = real_roomp(rs.arg3);
            } else {
              rp = random_room;
              random_room = NULL;
            }
            if (!rp) {
              vlogf(LOG_LOW, fmt("No room (%d) in M command (%d)") %  
                  rs.arg3 % rs.arg1);
              last_cmd = 0;
              mobload = 0;
              continue;
            }
            count = 0;

#if 1
            for (t = rp->tBornInsideMe; t; t = t->nextBorn) {
              TMonster *tMonster = dynamic_cast<TMonster *>(t);

              if (tMonster && tMonster->mobVnum() == mob_index[rs.arg1].virt)
                count++;
            }
#else
            for (t = rp->getStuff(); t; t = t->nextThing) {
              TMonster * tmon = dynamic_cast<TMonster *>(t);

              if (tmon && tmon->mobVnum() == mob_index[rs.arg1].virt)
                count++;
            }
#endif
            if (count >= rs.arg2) {
              // mob exceeds max for room
              last_cmd = 0;
              mobload = 0;
              continue;
            }
            if (!(mob = read_mobile(rs.arg1, REAL))) {
              vlogf(LOG_BUG, fmt("Error reading mobile (%d).  You suck.") %  rs.arg1);
              last_cmd = 0;
              mobload = 0;
              continue;
            }

            mob->createWealth();

            if ((mob->isNocturnal() || mob->isDiurnal()) && storageRoom)
              *storageRoom += *mob;
            else
              *rp += *mob;

#if 1
            // Slap the mob on the born list.
            *rp << *mob;
#endif

            last_cmd = 1;
            mobload = 1;
            mob->brtRoom = (rp ? rp->number : ROOM_NOWHERE);
            mobRepop(mob, zone_nr, (rp ? rp->number : ROOM_NOWHERE));
          } else {
            if(bootTime && mob_index[rs.arg1].getNumber() >= mob_index[rs.arg1].max_exist)
              vlogf(LOG_LOW, fmt("Mob %s (%i) tried to load but hit max_exist") % 
                  mob_index[rs.arg1].short_desc % mob_index[rs.arg1].virt);

            last_cmd = 0;
            mobload = 0;
          }
          break;
        case 'C':
          if (findLoadPotential) {
            break;
          }
          // a charmed follower of the previous mob

          // check if zone is disabled or if mob exceeds absolute max
          if ((this->zone_value != 0) && mobload &&
              mob_index[rs.arg1].getNumber() < mob_index[rs.arg1].max_exist) {
            if (!(old_mob = mob)) {
              vlogf(LOG_BUG, "Lack of master mob in 'C' command.");
              continue;
            }
            if (rs.arg3 != ZONE_ROOM_RANDOM) {
              rp = real_roomp(rs.arg3);
            } else {
              rp = random_room;
              random_room = NULL;
            }
            if (!rp) {
              vlogf(LOG_LOW, fmt("No room (%d) in C command (%d)") % 
                  rs.arg3 % rs.arg1);
              last_cmd = 0;
              mobload = 0;
              continue;
            }

            count = 0;
#if 1
            for (t = rp->tBornInsideMe; t; t = t->nextBorn) {
              TMonster *tMonster = dynamic_cast<TMonster *>(t);

              if (tMonster && tMonster->mobVnum() == mob_index[rs.arg1].virt)
                count++;
            }
#else
            for (t = rp->getStuff(); t; t = t->nextThing) {
              TMonster * tmon = dynamic_cast<TMonster *>(t);
              if (tmon && tmon->mobVnum() == mob_index[rs.arg1].virt)
                count++;
            }
#endif
            if (count >= rs.arg2) {
              // mob exceeds max for room
              last_cmd = 0;
              mobload = 0;
              continue;
            }
            if (!(mob = read_mobile(rs.arg1, REAL))) {
              vlogf(LOG_BUG, "Error reading mobile.  You suck.");
              continue;
            }
            *rp += *mob;
            mob->brtRoom = (rp ? rp->number : ROOM_NOWHERE);
            mobRepop(mob, zone_nr);


#if 1
            // Slap the mob on the born list.
            *rp << *mob;
#endif

            old_mob->addFollower(mob);
            SET_BIT(mob->specials.affectedBy, AFF_CHARM);
            last_cmd = 1;
          } else {
            last_cmd = 0;
            mobload = 0;
          }
          break;
        case 'K':
          if (findLoadPotential) {
            break;
          }
          // a grouped follower of the previous mob

          // check if zone is disabled or if mob exceeds absolute max
          if ((this->zone_value != 0) && mobload &&
              mob_index[rs.arg1].getNumber() < mob_index[rs.arg1].max_exist) {
            if (!(old_mob = mob)) {
              vlogf(LOG_BUG, "Lack of master mob in 'K' command.");
              continue;
            }
            if (rs.arg3 != ZONE_ROOM_RANDOM) {
              rp = real_roomp(rs.arg3);
            } else {
              rp = random_room;
              random_room = NULL;
            }
            if (!rp) {
              vlogf(LOG_LOW, fmt("No room (%d) in K command (%d)") % 
                  rs.arg3 % rs.arg1);
              last_cmd = 0;
              mobload = 0;
              continue;
            }

            count = 0;
#if 1
            for (t = rp->tBornInsideMe; t; t = t->nextBorn) {
              TMonster *tMonster = dynamic_cast<TMonster *>(t);

              if (tMonster && tMonster->mobVnum() == mob_index[rs.arg1].virt)
                count++;
            }
#else
            for (t = rp->getStuff(); t; t = t->nextThing) {
              TMonster * tmon = dynamic_cast<TMonster *>(t);
              if (tmon && tmon->mobVnum() == mob_index[rs.arg1].virt)
                count++;
            }
#endif
            if (count >= rs.arg2) {
              // mob exceeds max for room
              last_cmd = 0;
              mobload = 0;
              continue;
            }
            if (!(mob = read_mobile(rs.arg1, REAL))) {
              vlogf(LOG_BUG, "Error reading mobile.  You suck.");
              continue;
            }
            *rp += *mob;
            mob->brtRoom = (rp ? rp->number : ROOM_NOWHERE);
            mobRepop(mob, zone_nr);

#if 1
            // Slap the mob on the born list.
            *rp << *mob;
#endif

            old_mob->addFollower(mob);
            SET_BIT(old_mob->specials.affectedBy, AFF_GROUP);
            SET_BIT(mob->specials.affectedBy, AFF_GROUP);
            last_cmd = 1;
          } else {
            last_cmd = 0;
            mobload = 0;
          }
          break;
        case '?':
          if (rs.character) {
            if ((rs.character == 'M') ||
                (rs.character == 'O') ||
                (rs.character == 'B') ||
                (rs.character == 'L') ||
                (objload && (rs.character == 'P')) ||
                (mobload && (rs.character != 'P'))) {

              int tmp, my_chance;
              if (findLoadPotential) {
                tmp = 1;
              } else {
                tmp = dice(1, 100);
              }

              // If we are putting certain objects into the world or
              // giving certain objects to mobs, follow the chance defined
              // in the zonefile.  Otherwise, set the chance to fixed_chance.
              if ((objload && (rs.character == 'P')) ||
                 (mobload && (rs.character == 'G'))) {
                my_chance = rs.arg1;
              } else {
                my_chance = fixed_chance;
              }
              if (rs.arg1 >= 98 || tmp <= my_chance ||
                  gamePort == BETA_GAMEPORT) {
                last_cmd = 1;
              } else {
                last_cmd = 0;
                if (rs.character == 'M')
                  mobload = 0;
                else if (rs.character == 'O' || rs.character == 'B')
                  objload = 0;
              }
            }
          } else
            vlogf(LOG_BUG, "No rs. character in ? command");
          break;
        case 'R':
          if (findLoadPotential) {
            break;
          }
          // check if zone is disabled or if mob exceeds absolute max
          if ((this->zone_value != 0) && mobload &&
              mob_index[rs.arg1].getNumber() < mob_index[rs.arg1].max_exist) {
            if (rs.arg3 != ZONE_ROOM_RANDOM) {
              rp = real_roomp(rs.arg3);
            } else {
              rp = random_room;
              random_room = NULL;
            }
            if (!rp) {
              vlogf(LOG_LOW, fmt("No room (%d) in R command (%d)") % 
                  rs.arg3 % rs.arg1);
              last_cmd = 0;
              mobload = 0;
              continue;
            }
      
            if(this->zone_value != 0 && bootTime){
              mob_index[rs.arg1].doesLoad=true;
              mob_index[rs.arg1].numberLoad++;
            }

            count = 0;
#if 1
            for (t = rp->tBornInsideMe; t; t = t->nextBorn) {
              TMonster *tMonster = dynamic_cast<TMonster *>(t);

              if (tMonster && tMonster->mobVnum() == mob_index[rs.arg1].virt)
                count++;
            }
#else
            for (t = rp->getStuff(); t; t = t->nextThing) {
              TMonster * tmon = dynamic_cast<TMonster *>(t);
              if (tmon && tmon->mobVnum() == mob_index[rs.arg1].virt)
                count++;
            }
#endif
            if (count >= rs.arg2) {
              // mob exceeds max for room
              last_cmd = 0;
              mobload = 0;
              continue;
            }
            if (!(old_mob = mob)) {
              vlogf(LOG_BUG, "Lack of master mob in 'R' command.");
              continue;
            }
            if (!(mob = read_mobile(rs.arg1, REAL))) {
              vlogf(LOG_BUG, "Error reading mobile.  You suck.");
              continue;
            }
            rp = real_roomp(rs.arg3);
            *rp += *mob;
            

#if 1
            // Slap the mob on the born list.
            *rp << *mob;
#endif

            if (old_mob->getHeight() <= (6 * mob->getHeight() / 10))
              vlogf(LOG_LOW, fmt("Mob mounting mount that is too small.  [%s] [%s]") % 
                    mob->getName() % old_mob->getName());

            if (old_mob->getHeight() >= (5 * mob->getHeight() / 2))
              vlogf(LOG_LOW, fmt("Mob mounting mount that is too big.  [%s] [%s]") % 
                    mob->getName() % old_mob->getName());

            if (compareWeights(mob->getTotalWeight(TRUE),
                               (old_mob->carryWeightLimit() -
                                old_mob->getCarriedWeight())) == -1)
              vlogf(LOG_LOW, fmt("Mob mounting mount that is too weak.  [%s] [%s]") % 
                    mob->getName() % old_mob->getName());

            if (old_mob->GetMaxLevel() > mob->GetMaxLevel())
              vlogf(LOG_LOW, fmt("Mob mounting mount that is too strong.  [%s:%d] [%s:%d]") % 
                    mob->getName() % mob->GetMaxLevel() %
                    old_mob->getName() % old_mob->GetMaxLevel());

            mob->mount(old_mob);
            mob->setPosition(POSITION_MOUNTED);
            if (old_mob->master && old_mob->master != mob && !old_mob->rider)
              old_mob->stopFollower(TRUE);
            if (!old_mob->master)
              mob->addFollower(old_mob);

            // needs to be after we are set riding
            mob->brtRoom = (rp ? rp->number : ROOM_NOWHERE);
            mobRepop(mob, zone_nr);

            last_cmd = 1;
          } else {
            last_cmd = 0;
            mobload = 0;
          }
          break;

        case 'O':                
          if (findLoadPotential) {
            break;
          }
          if(bootTime){
            // check conditions
            if (rs.arg3 != ZONE_ROOM_RANDOM) 
              rp = real_roomp(rs.arg3);
            else {
              rp = random_room;
              random_room = NULL;
            }
    
            if (!rp) {
              vlogf(LOG_LOW, fmt("No room (%d) in O command (%d).  cmd=%d, zone=%d") %  rs.arg3 % rs.arg1 % cmd_no % zone_nr);
              last_cmd = 0;
              objload = 0;
              continue;
            }
            
            count=0;
            for(t=rp->getStuff();t;t=t->nextThing){
              obj = dynamic_cast<TObj *>(t);
              if(obj && obj->objVnum() == obj_index[rs.arg1].virt)
                count++;
            }

            if (count >= rs.arg2) {
              last_cmd = 1;
              objload = TRUE;
              continue;
            }

            // load the objects
            for(;count<rs.arg2;++count){
              if(obj_index[rs.arg1].getNumber() < obj_index[rs.arg1].max_exist){
                obj = read_object(rs.arg1, REAL);

                if (obj != NULL) {
                  *rp += *obj;
                  obj->onObjLoad();
                  last_cmd = 1;
                  objload = TRUE;
                } else {
                  vlogf(LOG_LOW, fmt("No obj (%d) in O command (room=%d).  cmd=%d, zone=%d") %  rs.arg1 % rs.arg3 % cmd_no % zone_nr);
                  objload = FALSE;
                  last_cmd = 0;
                }
              }
            }
          } else {
            objload = FALSE;
            last_cmd = 0;
          }
          break;
        case 'B':               
          if (findLoadPotential) {
            break;
          }
          if (obj_index[rs.arg1].getNumber() < obj_index[rs.arg1].max_exist) {
            if (rs.arg3 != ZONE_ROOM_RANDOM) {
              rp = real_roomp(rs.arg3);
            } else {
              rp = random_room;
              random_room = NULL;
            }
            if (!rp) {
              vlogf(LOG_LOW, fmt("No room (%d) in B command (%d)") % 
                  rs.arg3 % rs.arg1);
              last_cmd = 0;
              objload = 0;
              continue;
            }

            count=0;
            for(t=rp->getStuff();t;t=t->nextThing){
              TObj *o = dynamic_cast<TObj *>(t);
              if(o && o->objVnum() == obj_index[rs.arg1].virt)
                count++;
            }
  
            if (count >= rs.arg2) {
              last_cmd = 0;
              objload = 0;
              continue;
            }

            for(;count<rs.arg2;++count){
              if(obj_index[rs.arg1].getNumber() < obj_index[rs.arg1].max_exist){
                obj = read_object(rs.arg1, REAL);
                if (obj != NULL) {
                  *rp += *obj;
                  obj->onObjLoad();
                  last_cmd = 1;
                  objload = TRUE;
                } else {
                  objload = FALSE;
                  last_cmd = 0;
                }
              }
            }
          } else {
            objload = FALSE;
            last_cmd = 0;
          }
          break;
        case 'P':                
          if (findLoadPotential) {
            break;
          }
          if (obj_index[rs.arg1].getNumber() < obj_index[rs.arg1].max_exist) {
            newobj = read_object(rs.arg1, REAL);
      // we're relying on obj being preserved from the previous 'O'
      //            obj = get_obj_num(rs.arg3);
            if (obj && newobj && dynamic_cast<TBaseContainer *>(obj)) {
              *obj += *newobj;
              newobj->onObjLoad();
              last_cmd = 1;
              log_object(newobj);
            } else if (obj && newobj && dynamic_cast<TTable *>(obj)) {
              newobj->mount(obj);
              newobj->onObjLoad();
              last_cmd = 1;
              log_object(newobj);
            } else {
	      delete newobj;
              last_cmd = 0;
            }
          } else
            last_cmd = 0;
          break;
        case 'V':    // Change ONE value of the four values upon reset- Russ 
          if (findLoadPotential) {
            break;
          }
          if (obj) {
            int tmp, tmp2, tmp3, tmp4;
            obj->getFourValues(&tmp, &tmp2, &tmp3, &tmp4);
            switch (rs.arg1) {
              case 0: 
                obj->assignFourValues(rs.arg2, tmp2, tmp3, tmp4);
                break;
              case 1: 
                obj->assignFourValues(tmp, rs.arg2, tmp3, tmp4);
                break;
              case 2: 
                obj->assignFourValues(tmp, tmp2, rs.arg2, tmp4);
                break;
              case 3: 
                obj->assignFourValues(tmp, tmp2, tmp3, rs.arg2);
                break;
              default:
                vlogf(LOG_LOW, fmt("Bad slot (%d) for V command (%d)") % 
                                    rs.arg1 % rs.arg2);
            }
            last_cmd = 1;
          } else
            last_cmd = 0;
          break;
        case 'T':        // Set traps for doors and containers - Russ 
          if (findLoadPotential) {
            break;
          }
          // if_flag-->0 : trap door, else trap previous object 
          if (!rs.if_flag) {
            rp = real_roomp(rs.arg1);
            if (rp && rp->dir_option[rs.arg2]) {
              SET_BIT(rp->dir_option[rs.arg2]->condition, EX_TRAPPED);
              rp->dir_option[rs.arg2]->trap_info = rs.arg3;
              rp->dir_option[rs.arg2]->trap_dam = rs.arg4;
            }
          } else {
            TOpenContainer *trc = dynamic_cast<TOpenContainer *>(obj);
            if (trc) {
              trc->addContainerFlag(CONT_TRAPPED);
              trc->setContainerTrapType(mapFileToDoorTrap(rs.arg1));
              trc->setContainerTrapDam(rs.arg2);
            }
          }
          last_cmd = 1;
          break;
        case 'G':        
          if (findLoadPotential) {
            tallyObjLoadPotential(obj_index[rs.arg1].virt);
            break;
          }
          mud_assert(rs.arg1 >= 0 && rs.arg1 < (signed int) obj_index.size(), "Range error (%d not in obj_index)  G command #%d in %s", rs.arg1, cmd_no, this->name);
          if (obj_index[rs.arg1].getNumber() < obj_index[rs.arg1].max_exist &&
              (obj = read_object(rs.arg1, REAL))) {

	    //              (obj = read_object_buy_build(mob, rs.arg1, REAL))) {

            *mob += *obj;
            obj->onObjLoad();
            mob->logItem(obj, CMD_LOAD);
            objload = 1;
            last_cmd = 1;
          } else {
            repoCheck(mob, rs.arg1);
            last_cmd = 0;
            objload = 0;
          }
          break;
        case 'H':                
          if (findLoadPotential) {
            break;
          }
          if (mob->addHatred(zoneHateT(rs.arg1), rs.arg2))
            last_cmd = 1;
          else
            last_cmd = 0;
          break;
        case 'X': // X <set num>3 <slot>1 <vnum>2
          if (rs.arg3 >= 0 && rs.arg3 < 16 && 
              rs.arg1 >= MIN_WEAR && rs.arg1 < MAX_WEAR) {
            local_armor[rs.arg3].slots[rs.arg1] = rs.arg2;
          }
          break;
        case 'Z': // Z <if flag> <set num> <perc chance>
          if (findLoadPotential) {
            wearSlotT i;
            for (i = MIN_WEAR; i < MAX_WEAR; i++) {
              if (local_armor[rs.arg1].slots[i]) {
                      tallyObjLoadPotential(local_armor[rs.arg1].slots[i]);
              }
            }
            break;
          }
          if (mob && mobload && rs.arg1 >=0) {
            wearSlotT i;
            for (i = MIN_WEAR; i < MAX_WEAR; i++) {
              if (local_armor[rs.arg1].slots[i]) {
                      // loadsetCheck(mob, local_armor[rs.arg1].slots[i], rs.arg2, 
                      //   i, "(null... for now)");
          if(rs.arg2 >= 98){
            loadsetCheck(mob, local_armor[rs.arg1].slots[i], 
                   rs.arg2, i, "(null... for now)");
          } else {
            loadsetCheck(mob, local_armor[rs.arg1].slots[i], 
                   fixed_chance, i, "(null... for now)");
          }
        }
      }
    }
    break;
        case 'Y':
          if (findLoadPotential) {
            if(rs.arg2 >= 98){
              mob->loadSetEquipment(rs.arg1, NULL, rs.arg2, true);
            } else {
              mob->loadSetEquipment(rs.arg1, NULL, fixed_chance, true);
            }
            break;
          }
          if (mob && mobload) {
            // mob->loadSetEquipment(rs.arg1, NULL, rs.arg2);
            if(rs.arg2 >= 98){
              mob->loadSetEquipment(rs.arg1, NULL, rs.arg2);
            } else {
              mob->loadSetEquipment(rs.arg1, NULL, fixed_chance);
            }

            if (mob->hasClass(CLASS_MAGE)) {
              TSpellBag *tBagA = NULL,
                        *tBagB = NULL;
              TThing    *tThing;

              // Find Held Spellbag
              for (tThing = mob->getStuff(); tThing; tThing = tThing->nextThing)
                if ((tBagA = dynamic_cast<TSpellBag *>(tThing)))
                  break;

              // Find Worn Spellbag
              for (wearSlotT tWear = MIN_WEAR; tWear < MAX_WEAR; tWear++)
                if (mob->equipment[tWear] &&
                    (tBagB = dynamic_cast<TSpellBag *>(mob->equipment[tWear])))
                  break;

              if (tBagA && tBagB) {
                while ((tThing = tBagA->getStuff())) {
                  --(*tThing);
                  *tBagB += *tThing;
                }

                --(*tBagA);
                delete tBagA;
                tBagA = NULL;
              }
            }
          }
          break;
        case 'F':                
          if (findLoadPotential) {
            break;
          }
          if (mob->addFears(zoneHateT(rs.arg1), rs.arg2))
            last_cmd = 1;
          else
            last_cmd = 0;
          break;
        case 'E':                
          if (findLoadPotential) {
            tallyObjLoadPotential(obj_index[rs.arg1].virt);
            break;
          }
          obj_lp = getObjLoadPotential(obj_index[rs.arg1].virt);
          if (obj_lp == 0) {
            vlogf(LOG_MISC, fmt("Didn't find load potential of %s [%d].  rs.arg1=%d") % obj_index[rs.arg1].short_desc % obj_index[rs.arg1].virt % rs.arg1);
            obj_lp = 1;
          }
          // 1-e**((ln(1-0.01n**1/3)/n)) = normalized load rate
          // adj_obj_lp_ratio = 1 - pow(exp(1), ((log(1 - 0.01*cbrt((double)obj_lp))/(double)obj_lp)));
          // 1 - ((1-0.01*n**1/3)^(1/n)) = normalized load rate, less math
          adj_obj_lp_ratio = 1 - pow((1 - cbrt((double)obj_lp)/100), 1/(double)obj_lp);
          // obj_lp_ratio = 1 - pow((1 - (double)fixed_chance/100), (double)obj_lp);
          obj_lp_ratio = (double)fixed_chance/100;
          // getting to this point means we've already beat the fixed_chance%
          // chance of loading an object.  This has to be taken into account
          // when computing the odds of the normalized load potential.
          // vlogf(LOG_MISC, fmt("(10000000 * adj_obj_lp_ratio / obj_lp_ratio * stats.equip) = %d") % (int) (10000000 * adj_obj_lp_ratio / obj_lp_ratio * stats.equip));
          if((obj_index[rs.arg1].getNumber() < obj_index[rs.arg1].max_exist) &&
              (::number(0, 9999999) < 
	       (int)(10000000*adj_obj_lp_ratio / obj_lp_ratio*stats.equip)) &&
	     (obj = read_object_buy_build(mob, rs.arg1, REAL))) {
            // vlogf(LOG_MISC, fmt("Adjusted probability for load of %s [%d]: %lf -> %lf") % obj_index[rs.arg1].short_desc % obj_index[rs.arg1].virt % obj_lp_ratio % adj_obj_lp_ratio);
            if (!mob) {
              vlogf(LOG_LOW, fmt("no mob for 'E' command.  Obj (%s)") %  obj->getName());
              delete obj;
              obj = NULL;
              last_cmd = 0;
              objload = 0;
              continue;
            }
            realslot = wearSlotT(rs.arg3);
            mud_assert(realslot >= MIN_WEAR && realslot < MAX_WEAR, "bad slot");

            if (!mob->equipment[realslot]) {
              // these are just safety logs, equipping will be done regardless
              if (!mob->canUseEquipment(obj, SILENT_YES)) {
                vlogf(LOG_LOW, fmt("'E' command equipping unusable item (%s:%d) on (%s:%d).") % obj->getName() % obj->objVnum() % mob->getName() % mob->mobVnum());
              }
              TBaseClothing *tbc = dynamic_cast<TBaseClothing *>(obj);
              if (tbc && tbc->canWear(ITEM_WEAR_FINGERS) && gamePort != PROD_GAMEPORT) {
                vlogf(LOG_LOW, fmt("RINGLOAD: [%s][%-6.2f] loading on [%s][%d]") % 
                      obj->getName() % tbc->armorLevel(ARMOR_LEV_REAL) %
                      mob->getName() % mob->GetMaxLevel());

              }
              if (tbc && !mob->validEquipSlot(realslot) && !tbc->isSaddle()) {
                vlogf(LOG_LOW, fmt("'E' command for %s equipping item (%s) on nonvalid slot %d.") % mob->getName() % tbc->getName() % realslot);
              }
              if (!check_size_restrictions(mob, obj, realslot, mob) &&
                  realslot != HOLD_RIGHT && realslot != HOLD_LEFT) {
                int size_per = 100;
                if (race_vol_constants[mapSlotToFile(realslot)]) {
                  size_per = (int)(100.0 * obj->getVolume() / race_vol_constants[mapSlotToFile( realslot)]);
                  if (obj->isPaired())
                    size_per /= 2;
                }
                vlogf(LOG_LOW, fmt("'E' for (%s:%d) equipping (%s:%d) with bad fit. (m:%d%%/o:%d%%) change vol to %d, or height to %d.") %  
                    mob->getName() % mob->mobVnum() % 
                    obj->getName() % obj->objVnum() %
                    (mob->getHeight() * 100) %
                    size_per % 
                    (mob->getHeight() * (obj->isPaired() ? 2 : 1) *
                                 race_vol_constants[mapSlotToFile( realslot)]) %
                    (size_per / 100));
              }
              // OK, actually do the equip
              mob->equipChar(obj, realslot);
              mob->logItem(obj, CMD_LOAD);
              log_object(obj);

              // for items without levels, objLevel = 0 so this logic is OK
              double al = obj->objLevel();
              double grl = mob->getRealLevel();
              if (al > (grl + 1)) {
                vlogf(LOG_LOW, fmt("Mob (%s:%d) of level %.1f loading item (%s:%d) thought to be level %.1f.") %  mob->getName() % mob->mobVnum() % grl % obj->getName() % obj->objVnum() % al);
              }

            } else {
              vlogf(LOG_LOW, fmt("'E' command operating on already equipped slot.  %s, %s slot %d\n\rpre-equipped with %s, is_same: %s") %  
                    mob->getName() % obj->getName() % realslot %
                    mob->equipment[realslot]->getName() %
                    ((mob->equipment[realslot] == obj) ? "true" : "false"));
              delete obj;
              obj = NULL;
              last_cmd = 0;
              objload = 0;
              continue;
            }
            if (!mob->equipment[realslot])
              vlogf(LOG_LOW, fmt("Zone-file %s (%d) failed to equip %s (%d)") % 
                   mob->getName() % mob->mobVnum() % obj->getName() % obj->objVnum());
            last_cmd = 1;
          } else {
            repoCheck(mob, rs.arg1);
            last_cmd = 0;
            objload = 0;
          }
          
          break;
        case 'D':                
          if (findLoadPotential) {
            break;
          }
          rp = real_roomp(rs.arg1);
          if (rp) {
            roomDirData * exitp = rp->dir_option[rs.arg2];
            if (exitp &&
               !IS_SET(exitp->condition, EX_DESTROYED) &&
               !IS_SET(exitp->condition, EX_CAVED_IN)) {
              if (exitp->door_type != DOOR_NONE) {
                switch (rs.arg3) {
                  case 0:
                    if (IS_SET(exitp->condition, EX_CLOSED))
                      sendrpf(rp, "The %s opens.\n\r", 
            exitp->getName().uncap().c_str());
                    REMOVE_BIT(exitp->condition, EX_LOCKED);
                    REMOVE_BIT(exitp->condition, EX_CLOSED);
                    break;
                  case 1:
                    if (!IS_SET(exitp->condition, EX_CLOSED))
                      sendrpf(rp, "The %s closes.\n\r",
            exitp->getName().uncap().c_str());
                    SET_BIT(exitp->condition, EX_CLOSED);
                    REMOVE_BIT(exitp->condition, EX_LOCKED);
                    break;
                  case 2:
                    if (exitp->key < 0) 
                      vlogf(LOG_LOW, 
                        fmt("Door with key < 0 set to lock in room %d.")
                        % rp->number);
                    if (!IS_SET(exitp->condition, EX_CLOSED))
                      sendrpf(rp, "The %s closes.\n\r",
                          exitp->getName().uncap().c_str());
                    SET_BIT(exitp->condition, EX_LOCKED);
                    SET_BIT(exitp->condition, EX_CLOSED);
                    break;
                }
                last_cmd = 1;
              } else {
                vlogf(LOG_LOW, fmt("'D' command operating on DOOR_NONE in rm %d") %  rp->number); 
              }
            } // check for valid,legal exit
          }  // check for dest room
          break;
        case 'L':
          if (findLoadPotential) {
            break;
          }
          if (bootTime)
            last_cmd = sysLootLoad(rs, mob, obj, false);
          else
            last_cmd = 0;
          break;
        default:
          vlogf(LOG_BUG, fmt("Undefd cmd in reset table; zone %d cmd %d.\n\r") %  zone_nr % cmd_no);
          break;
      }
    } else {
      last_cmd = 0;
    }
  }
  if (!findLoadPotential) {
    doGenericReset(); // sends CMD_GENERIC_RESET to all objects in zone
  }
  this->age = 0;
}

bool zoneData::doGenericReset(void) 
{
  int top = 0;
  int bottom = 0;
  int rc;
  
  if (zone_nr < 0 || zone_nr >= (signed int) zone_table.size())
  {
    vlogf(LOG_BUG, fmt("Bad zone number in doGenericReset (%d)") %  zone_nr);
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
      vlogf(LOG_SILENT, fmt("real_object: probable failure for %d") %  virt);
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
    case ITEM_SMOKE:
      return new TSmoke();
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
    case ITEM_UNDEFINED:
    case ITEM_MARTIAL_WEAPON:
    case MAX_OBJ_TYPES:
      vlogf(LOG_BUG, fmt("Unknown item type (%d)") %  tmp);
  }
  return NULL;
}

resetCom::resetCom() :
  command('\0'),
  if_flag(0),
  arg1(0),
  arg2(0),
  arg3(0),
  arg4(0),
  character('\0')
{
}

resetCom::resetCom(const resetCom &t) :
  command(t.command),
  if_flag(t.if_flag),
  arg1(t.arg1),
  arg2(t.arg2),
  arg3(t.arg3),
  arg4(t.arg4),
  character(t.character)
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

  return *this;
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


