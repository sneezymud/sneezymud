//////////////////////////////////////////////////////////////////////////
//
// SneezyMUD - All rights reserved, SneezyMUD Coding Team
//
//    "db.cc" - All functions and routines  related to tinyworld databases
//
//////////////////////////////////////////////////////////////////////////

#include "stdsneezy.h"

#include "socket.h"
#include "statistics.h"
#include "help.h"
#include "mail.h"
#include "obj_component.h"
#include "stdsneezy.h"
#include "loadset.h"
#include "sys_loot.h"
#include "shop.h"
#include "database.h"
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

int top_of_world = 0;         // ref to the top element of world 

TRoom *room_db[WORLD_SIZE];
long  room_file_pos[WORLD_SIZE];

TObj *object_list = 0;    // the global linked list of obj's 
TBeing *character_list = 0; // global l-list of chars          
TMonster *pawnman = NULL;
TPCorpse *pc_corpse_list = NULL;
// table of reset data 
vector<zoneData>zone_table(0);

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

FILE *mob_f = NULL;        // file containing mob prototypes  
FILE *obj_f = NULL;        // obj prototypes                  

vector<TRoom *>roomspec_db(0);

struct time_info_data time_info;        // the infomation about the time   
struct weather_data weather_info;        // the infomation about the weather 
class lag_data lag_info;

// local procedures
static void bootZones(void);
static void bootWorld(void);
static void bootHomes(void);
static void renum_zone_table(void);
static void reset_time(void);
static void bootGovMoney(void);

struct reset_q_type
{
  resetQElement *head;
  resetQElement *tail;
} r_q;

void bootPulse(const char *str, bool end_str)
{
  /*  This function gets called periodically during bootup
    It basically will do socket stuff to bind new connections
    And sends some boot info to all descriptors.
    This is good for giving feedback during boots that might be long
    But be careful not to send immortal-only info  :)
  */

  Descriptor *d;
  string sc;
  static string tLastRealMessage("");
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
    d->output.putInQ(colorString(NULL, d, sc.c_str(), NULL, COLOR_BASIC, TRUE).c_str());
    d->outputProcessing();
  }

  if (str && strcmp(str, "."))
    vlogf(LOG_MISC, "%s", str);
}

void bootDb(void)
{
  bootPulse("Boot db -- BEGIN.");

  bootPulse("Resetting the game time.");
  reset_time();
  bootPulse("Initializing game statistics.");
  if (!init_game_stats()) {
    vlogf(LOG_MISC, "bad result from init_game_stats");
//    exit(0);
  }
  bootPulse("Loading Races.");
  for(race_t rindex=RACE_NORACE;rindex<MAX_RACIAL_TYPES;rindex++)
    Races[rindex] = new Race(rindex);

  bootPulse("Initializing faction data.");
  if (!load_factions()) {
    vlogf(LOG_MISC, "Bad loading of factions.");
    exit(0);
  }
#if 1
  if (!load_newfactions()) {
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

  bootPulse("Opening mobile file.");
  if (!(mob_f = fopen(MOB_FILE, "r"))) {
    perror("boot");
    exit(0);
  }
  // mob_f and obj_f must be open prior to here.
  bootPulse("Generating index tables for mobile file.");
  generate_mob_index();

  bootPulse("Opening object file.");
  if (!(obj_f = fopen(OBJ_FILE, "r"))) {
    perror("boot");
    exit(0);
  }
  bootPulse("Generating index tables for object file.");
  generate_obj_index();

  bootPulse("Building help tables.");
  buildHelpIndex();

  bootPulse("Loading zone table.");
  bootZones();

  // must be done before loading objects
  bootPulse("Loading drink-type information.");
  assign_drink_types();
  bootPulse("Loading drug-type information.");
  assign_drug_info();

  unsigned int i;
  bootPulse("Loading rooms:", false);
  bootWorld();
  bootPulse(NULL, true);

  bootPulse("Loading homes:", false);
  bootHomes();
  bootPulse(NULL, true);

  bootPulse("Building suitset information.");
  suitSets.SetupLoadSetSuits();

  // indices must be assigned before this gets called.
  bootPulse("Renumbering zone table.");
  renum_zone_table();

  vlogf(LOG_MISC, "Assigning function pointers:");
  vlogf(LOG_MISC, "   Shopkeepers.");
  bootTheShops();
  vlogf(LOG_MISC, "Reading shop prices.");
  loadShopPrices();

  bootPulse("Initializing boards.");
  InitBoards();
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


  bootPulse("Booting mail system:", false);
  if (!scan_file(SILENT_NO)) {
    vlogf(LOG_MISC, "   Mail system error -- mail system disabled!");
    no_mail = 1;
  }
  bootPulse(NULL, true);

  bootPulse("Calculating number of items in rent.");
  vlogf(LOG_MISC, "Totals on limited items:");
  printLimitedInRent();

  bootPulse("Creating Loot List.");
  sysLootBoot();

  bootGovMoney();

  bootPulse("Resetting zones:", false);
  for (i = 0; i < zone_table.size(); i++) {
    char *s;
    int d, e;
    s = zone_table[i].name;
    d = (i ? (zone_table[i - 1].top + 1) : 0);
    e = zone_table[i].top;

    vlogf(LOG_MISC, "Performing boot-time reset of %s (rooms %d-%d).", s, d, e);
    zone_table[i].resetZone(TRUE);
    if (i%10 == 0)
      bootPulse(".", false);
  }
  bootPulse(NULL, true);

  bootPulse("Performing playerfile maintenance and data extraction:",false);
  fixup_players();
  
  bootPulse("Initializing light levels.");
  sunriseAndSunset();

  r_q.head = r_q.tail = 0;

  bootPulse("Boot -- DONE.");
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

  vlogf(LOG_MISC, "   Current Gametime: %dm, %dH %dD %dM %dY.", 
        time_info.minutes, time_info.hours, time_info.day, time_info.month, time_info.year);

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



// update the time file 
void update_time(void)
{
  return;
#if 0
  FILE *f1;
  long current_time;

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
  int virtual_nr;
  TRoom *rp;
  int num = 0;
  FILE *room_f;       // room file

  memset((char *) room_db, 0, sizeof(TRoom *) * WORLD_SIZE);
  character_list = NULL;
  object_list = NULL;

  if (!(room_f = fopen(WORLD_FILE, "r"))) {
    vlogf(LOG_FILE, "World file not found");
    exit(0);
  }
  while (fscanf(room_f, " #%d\n", &virtual_nr) == 1) {
    if (virtual_nr/1000 > num) {
      num = virtual_nr/1000;
      vlogf(LOG_MISC, "Room %ld allocated", num*1000);
      bootPulse(".", false);
    } 
    allocate_room(virtual_nr);
    rp = real_roomp(virtual_nr);
    rp->loadOne(room_f, true);


#if 0
// modified weather stuff, not used yet, BAT
    rp->initWeather();
#endif

    if (rp->isRoomFlag(ROOM_SAVE_ROOM))
      rp->loadItems();

    if ((rp->number == ROOM_NOCTURNAL_STORAGE))
      continue;

    if (rp->isRoomFlag(ROOM_PEACEFUL) && !rp->isRoomFlag(ROOM_NO_HEAL))
      vlogf(LOG_LOW, "%s room %d set peaceful && !no_heal (bit: %d)",
                rp->name,rp->number, ROOM_NO_HEAL);
    if (rp->isRoomFlag(ROOM_PEACEFUL) && !rp->isRoomFlag(ROOM_NO_STEAL))
      vlogf(LOG_LOW, "%s room %d set peaceful && !no_steal (bit: %d)",
                rp->name,rp->number, ROOM_NO_STEAL);
    if (rp->isRoomFlag(ROOM_PEACEFUL) && !rp->isRoomFlag(ROOM_NO_MAGIC))
      vlogf(LOG_LOW, "%s room %d set PEACEFUL && !no_magic (bit: %d)",
                rp->name,rp->number, ROOM_NO_MAGIC);
    if (rp->isRoomFlag(ROOM_NO_HEAL) && rp->isRoomFlag(ROOM_HOSPITAL))
      vlogf(LOG_LOW, "%s room %d set NO_HEAL(%d) and HOSPITAL(%d)",
                rp->name,rp->number, ROOM_NO_HEAL, ROOM_HOSPITAL);

    if (rp->isIndoorSector() && !rp->isRoomFlag(ROOM_INDOORS)) {
      // in general, this is an error
      // of course you could have a bldg whose roof has collapsed...
      if (rp->number != 27349)
        vlogf(LOG_LOW,"%s room %d set building & !indoor",
                rp->name,rp->number);
    }
    if (rp->isRoomFlag(ROOM_INDOORS) && rp->getRoomHeight() <= 0)
      vlogf(LOG_LOW,"%s indoor room %d set with unlimited height",
                rp->name,rp->number);
    if (!rp->isRoomFlag(ROOM_INDOORS) && rp->getRoomHeight() >= 0)
      vlogf(LOG_LOW,"%s outdoor room %d set with limited height",
                rp->name,rp->number);

#if 0
    if ((rp->getRoomHeight() >= 0) && rp->isFallSector())
      vlogf(LOG_LOW,"%s fall room %d set with limited height",
                rp->name,rp->number);
#endif
  }
  fclose(room_f);
}


// none of this flip stuff handles diagonals
#define FLIP_EASTWEST   (1<<0)
#define FLIP_NORTHSOUTH (1<<1)
#define FLIP_UPDOWN     (1<<2)
#define FLIP_ALL        FLIP_EASTWEST+FLIP_NORTHSOUTH+FLIP_UPDOWN

dirTypeT flip_dir(dirTypeT dir, int flip){
  if(flip & FLIP_UPDOWN){
    flip-=FLIP_UPDOWN;
    
    switch(dir){
      case DIR_UP: dir=DIR_DOWN; break;
      case DIR_DOWN: dir=DIR_UP; break;
      default: break;
    }
  }

  if(flip & FLIP_NORTHSOUTH){
    flip-=FLIP_NORTHSOUTH;
    
    switch(dir){
      case DIR_NORTH: dir=DIR_SOUTH; break;
      case DIR_SOUTH: dir=DIR_NORTH; break;
      default: break;
    }
  }
  
  if(flip & FLIP_EASTWEST){
    flip-=FLIP_EASTWEST;

    switch(dir){
      case DIR_EAST: dir=DIR_WEST; break;
      case DIR_WEST: dir=DIR_EAST; break;
      default: break;
    }
  }

  return dir;
}

dirTypeT rotate_dir(dirTypeT dir, int rotate){
  for(int i=0;i<rotate;++i){
    switch(dir){
      case DIR_NORTH:     dir=DIR_EAST;  break;
      case DIR_EAST:      dir=DIR_SOUTH; break;
      case DIR_SOUTH:     dir=DIR_WEST;  break;
      case DIR_WEST:      dir=DIR_NORTH; break;
      default: break;
    }
  }

  return dir;
}


bool bootHome(int plan_i, int plot_start, int plot_end, 
	      int keynum, int flip, int rotate, bool copy_objs=FALSE)
{
  int template_start=0, template_end=0, template_i=0;
  int plot_i;
  TRoom *src, *dest;
  TDatabase db("sneezy");

  db.query("select template_start, template_end from homeplans where plan=%i", plan_i);
  if(!db.fetchRow())
    return FALSE;
    
  template_start=atoi(db.getColumn(0));
  template_end=atoi(db.getColumn(1));
  
  plot_i=plot_start;
  for(template_i=template_start;template_i<=template_end;++template_i){
    bootPulse(".", false);
    
    src=real_roomp(template_i);
    dest=real_roomp(plot_i);
    
    if (dest->getDescr())
      delete [] dest->descr;
    dest->descr = mud_str_dup(src->getDescr());
    
    if (dest->name)
      delete [] dest->name;
    dest->name = mud_str_dup(src->name);
    
    dest->setRoomFlags(src->getRoomFlags());
    dest->setSectorType(src->getSectorType());
    dest->setRoomHeight(src->getRoomHeight());
    dest->setMoblim(src->getMoblim());
    
    dirTypeT realdir;
    
    // copy exits now
    for(dirTypeT dir=DIR_NORTH;dir<MAX_DIR;dir++){	
      realdir=flip_dir(dir, flip);
      realdir=rotate_dir(realdir, rotate);
      
      if(src->dir_option[dir]){ // if the template has this exit, copy it
	dest->dir_option[realdir]->door_type =
	  src->dir_option[dir]->door_type;
	dest->dir_option[realdir]->condition =
	  src->dir_option[dir]->condition;
	dest->dir_option[realdir]->lock_difficulty =
	  src->dir_option[dir]->lock_difficulty;
	dest->dir_option[realdir]->weight =
	  src->dir_option[dir]->weight;
	dest->dir_option[realdir]->key =
	  src->dir_option[dir]->key;
      } else if(dest->dir_option[realdir]){
	TRoom *outside=real_roomp(dest->dir_option[realdir]->to_room);
	
	// if the template doesn't have the exit, then remove it from dest
	if((dest->dir_option[realdir]->to_room >= plot_start &&
	    dest->dir_option[realdir]->to_room <= plot_end) ||
	   !outside->isRoomFlag(ROOM_INDOORS)){
	  
	  dest->dir_option[realdir]=NULL;
	  // delete?
	} else { // unless it goes outside, in which case make a door
	  // external exit, make a door and lock it
	  
	  dirTypeT dir_outside=flip_dir(realdir, FLIP_ALL);
	  
	  // do outside room
	  outside->dir_option[dir_outside]->door_type=DOOR_DOOR;
	  outside->dir_option[dir_outside]->condition=EX_CLOSED + EX_LOCKED;
	  outside->dir_option[dir_outside]->lock_difficulty=100;
	  outside->dir_option[dir_outside]->weight=5;
	  outside->dir_option[dir_outside]->key=keynum;
	  
	  if(outside->dir_option[dir_outside]->keyword)
	    delete [] outside->dir_option[dir_outside]->keyword;
	  outside->dir_option[dir_outside]->keyword=mud_str_dup("door");
	  
	  // do inside room
	  dest->dir_option[realdir]->door_type=DOOR_DOOR;
	  dest->dir_option[realdir]->condition=EX_CLOSED + EX_LOCKED;
	  dest->dir_option[realdir]->lock_difficulty=100;
	  dest->dir_option[realdir]->weight=5;
	  dest->dir_option[realdir]->key=keynum;
	  
	  if(dest->dir_option[realdir]->keyword)
	    delete [] dest->dir_option[realdir]->keyword;
	  dest->dir_option[realdir]->keyword=mud_str_dup("door");
	  
	}
      }
    }
    
    // copy objects now
    if(copy_objs){
      for(TThing *t=src->getStuff();t;t=t->nextThing){
	TObj *obj=read_object(t->number, REAL);
	*dest+=*obj;
      }
    }
    
    ++plot_i;
  }
  
  return TRUE;
}


void bootHomes(void)
{
  int plot_start=0, plot_end=0, plan_i=0, keynum=0, flip, rotate;
  TDatabase db("sneezy");

  db.query("select plan, plot_start, plot_end, keynum, flip, rotate from homeplots where homeowner is not null");
  
  while(db.fetchRow()){
    plan_i=atoi(db.getColumn(0));
    plot_start=atoi(db.getColumn(1));
    plot_end=atoi(db.getColumn(2));
    keynum=atoi(db.getColumn(3));
    flip=atoi(db.getColumn(4));    
    rotate=atoi(db.getColumn(5));

    if(!bootHome(plan_i, plot_start, plot_end, keynum, flip, rotate)){
      vlogf(LOG_BUG, "bootHome failed");
    }
  }
}

void TRoom::colorRoom(int title, int full)
{
  int len, place = 0, letter;
  char buf[MAX_STRING_LENGTH], argument[MAX_STRING_LENGTH];
  char buf2[10] = "";
  char buf3[10] = "";


  if (title == 1) {
    strcpy(argument, name);
  } else if (title == 2) {
    strcpy(argument, getDescr());
  } else {
    return;
  }
// Found had to initialize with this logic and too tired to figure out why
  strcpy(buf3, "<z>");

  switch (getSectorType()) {
    case SECT_SUBARCTIC:
      strcpy(buf2, "<P>");
      strcpy(buf3, "<p>");
      break;
    case SECT_ARCTIC_WASTE:
      strcpy(buf2, "<w>");
      strcpy(buf3, "<W>");
      break;
    case SECT_ARCTIC_CITY:
      strcpy(buf2, "<C>");
      break;
    case SECT_ARCTIC_ROAD:
      strcpy(buf2, "<W>");
      break;
    case SECT_TUNDRA:
      strcpy(buf2, "<o>");
      strcpy(buf3, "<g>");
      break;
    case SECT_ARCTIC_MOUNTAINS:
      strcpy(buf2, "<o>");
      strcpy(buf3, "<W>");
      break;
    case SECT_ARCTIC_FOREST:
      strcpy(buf2, "<G>");
      strcpy(buf3, "<W>");
      break;
    case SECT_ARCTIC_MARSH:
      strcpy(buf2, "<B>");
      strcpy(buf3, "<p>");
      break;
    case SECT_ARCTIC_RIVER_SURFACE:
      strcpy(buf2, "<C>");
      strcpy(buf3, "<c>");
      break;
    case SECT_ICEFLOW:
      strcpy(buf2, "<C>");
      strcpy(buf3, "<W>");
      break;
    case SECT_COLD_BEACH:
      strcpy(buf2, "<p>");
      strcpy(buf3, "<P>");
      break;
    case SECT_SOLID_ICE:
      strcpy(buf2, "<c>");
      strcpy(buf3, "<C>");
      break;
    case SECT_ARCTIC_BUILDING:
      strcpy(buf2, "<p>");
      break;
    case SECT_ARCTIC_CAVE:
      strcpy(buf2, "<c>");
      strcpy(buf3, "<k>");
      break;
    case SECT_ARCTIC_ATMOSPHERE:
      strcpy(buf2, "<C>");
      strcpy(buf3, "<C>");
      break;
    case SECT_ARCTIC_CLIMBING:
    case SECT_ARCTIC_FOREST_ROAD:
      strcpy(buf2, "<p>");
    case SECT_PLAINS:
      strcpy(buf2, "<G>");
      strcpy(buf3, "<g>");
      break;
    case SECT_TEMPERATE_CITY:
    case SECT_TEMPERATE_ROAD:
      strcpy(buf2, "<p>");
      break;
    case SECT_GRASSLANDS:
      strcpy(buf2, "<G>");
      strcpy(buf3, "<g>");
      break;
    case SECT_TEMPERATE_HILLS:
      strcpy(buf2, "<o>");
      strcpy(buf3, "<g>");
      break;
    case SECT_TEMPERATE_MOUNTAINS:
      strcpy(buf2, "<G>");
      strcpy(buf3, "<o>");
      break;
    case SECT_TEMPERATE_FOREST:
      strcpy(buf2, "<G>");
      strcpy(buf3, "<g>");
      break;
    case SECT_TEMPERATE_SWAMP:
      strcpy(buf2, "<P>");
      strcpy(buf3, "<p>");
      break;
    case SECT_TEMPERATE_OCEAN:
      strcpy(buf2, "<C>");
      strcpy(buf3, "<c>");
      break;
    case SECT_TEMPERATE_RIVER_SURFACE:
      strcpy(buf2, "<B>");
      strcpy(buf3, "<b>");
      break;
    case SECT_TEMPERATE_UNDERWATER:
      strcpy(buf2, "<C>");
      strcpy(buf3, "<b>");
      break;
    case SECT_TEMPERATE_CAVE:
      strcpy(buf2, "<o>");
      strcpy(buf3, "<k>");
      break;
    case SECT_TEMPERATE_ATMOSPHERE:
      strcpy(buf2, "<G>");
      break;
    case SECT_TEMPERATE_CLIMBING:
      strcpy(buf2, "<G>");
      break;
    case SECT_TEMPERATE_FOREST_ROAD:
      strcpy(buf2, "<g>");
      break;
    case SECT_DESERT:
    case SECT_SAVANNAH:
      strcpy(buf2, "<y>");
      strcpy(buf3, "<o>");
      break;
    case SECT_VELDT:
      strcpy(buf2, "<g>");
      strcpy(buf3, "<o>");
      break;
    case SECT_TROPICAL_CITY:
      strcpy(buf2, "<G>");
      break;
    case SECT_TROPICAL_ROAD:
      strcpy(buf2, "<g>");
      break;
    case SECT_JUNGLE:
      strcpy(buf2, "<P>");
      strcpy(buf3, "<g>");
      break;
    case SECT_RAINFOREST:
      strcpy(buf2, "<G>");
      strcpy(buf3, "<g>");
      break;
    case SECT_TROPICAL_HILLS:
      strcpy(buf2, "<R>");
      strcpy(buf3, "<g>");
      break;
    case SECT_TROPICAL_MOUNTAINS:
      strcpy(buf2, "<P>");
      strcpy(buf3, "<p>");
      break;
    case SECT_VOLCANO_LAVA:
      strcpy(buf2, "<y>");
      strcpy(buf3, "<R>");
      break;
    case SECT_TROPICAL_SWAMP:
      strcpy(buf2, "<G>");
      strcpy(buf3, "<g>");
      break;
    case SECT_TROPICAL_OCEAN:
      strcpy(buf2, "<b>");
      strcpy(buf3, "<c>");
      break;
    case SECT_TROPICAL_RIVER_SURFACE:
      strcpy(buf2, "<C>");
      strcpy(buf3, "<B>");
      break;
    case SECT_TROPICAL_UNDERWATER:
      strcpy(buf2, "<B>");
      strcpy(buf3, "<b>");
      break;
    case SECT_TROPICAL_BEACH:
      strcpy(buf2, "<P>");
      strcpy(buf3, "<y>");
      break;
    case SECT_TROPICAL_BUILDING:
      strcpy(buf2, "<p>");
      break;
    case SECT_TROPICAL_CAVE:
      strcpy(buf2, "<P>");
      strcpy(buf3, "<k>");
      break;
    case SECT_TROPICAL_ATMOSPHERE:
      strcpy(buf2, "<P>");
      break;
    case SECT_TROPICAL_CLIMBING:
      strcpy(buf2, "<P>");
      break;
    case SECT_RAINFOREST_ROAD:
      strcpy(buf2, "<P>");
      break;
    case SECT_ASTRAL_ETHREAL:
      strcpy(buf2, "<C>");
      strcpy(buf3, "<c>");
      break;
    case SECT_SOLID_ROCK:
      strcpy(buf2, "<k>");
      strcpy(buf3, "<w>");
      break;
    case SECT_FIRE:
      strcpy(buf2, "<y>");
      strcpy(buf3, "<R>");
      break;
    case SECT_INSIDE_MOB:
      strcpy(buf2, "<R>");
      strcpy(buf3, "<r>");
      break;
    case SECT_FIRE_ATMOSPHERE:
      strcpy(buf2, "<y>");
      strcpy(buf3, "<R>");
      break;
    case MAX_SECTOR_TYPES:
    case SECT_TEMPERATE_BEACH:
    case SECT_TEMPERATE_BUILDING:
    case SECT_MAKE_FLY:
      strcpy(buf2, "<p>");
      break;
  }
  memset(buf, '\0', sizeof(buf));
  if (title == 1) {
    if (buf2) {
      strcpy(buf, buf2);
      place = 3;
    }
  } else if (title == 2) {
    if (buf3) {
      strcpy(buf, buf3);
      place = 3;
    }
  }

  len = strlen(argument);
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
            if (buf2) {
              strcat(buf, buf2);
              place +=3;
            }
          } else if (title == 2) {
            if (buf3) {
              strcat(buf, buf3);
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


  strcat(buf, "<1>");
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


void setup_dir(FILE * fl, int room, dirTypeT dir, TRoom *tRoom = NULL)
{
  int tmp;
  TRoom *rp;

  if (!(rp = real_roomp(room)) && !(rp = tRoom)) {
    vlogf(LOG_MISC, "Setup_dir called with bad room number %d", room);
    return;
  }
  rp->dir_option[dir] = new roomDirData();

  rp->dir_option[dir]->description = fread_string(fl);
  rp->dir_option[dir]->keyword = fread_string(fl);

  fscanf(fl, " %d ", &tmp);
  if (tmp < 0 || tmp >= MAX_DOOR_TYPES) {
    vlogf(LOG_LOW,"bogus door type (%d) in room (%d) dir %d.",
        tmp, room, dir);
    return;
  }
  rp->dir_option[dir]->door_type = doorTypeT(tmp);
  if ((tmp == DOOR_NONE) && (rp->dir_option[dir]->keyword)){
    if (strcmp(rp->dir_option[dir]->keyword, "_unique_door_"))
      vlogf(LOG_LOW,"non-door with name in room %d",room);
  }
  if ((tmp != DOOR_NONE) && !(rp->dir_option[dir]->keyword)){
    vlogf(LOG_LOW,"door with no name in room %d",room);
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
    if (IS_SET(rp->dir_option[dir]->condition, EX_CLOSED))
      vlogf(LOG_LOW, "See thru door set secret. (%d, %d)", room, dir);
    else
      vlogf(LOG_LOW, "Secret door saved as open. (%d, %d)", room, dir);
  }
}


void zoneData::logError(char ch, const char *type, int cmd, int value)
{
  vlogf(LOG_LOW, "zone %s cmd %d (%c) resolving %s number (%d)",
      name, cmd, ch, type, value);
}

void renum_zone_table(void)
{
  int comm;
  int value;
  unsigned int zone;

  for (zone = 0; zone < zone_table.size(); zone++) {
    // init the zone_value array
    if (nuke_inactive_mobs)
      zone_table[zone].zone_value = ((zone <= 1) ? -1 : 0);
    else
      zone_table[zone].zone_value = -1;

    for (comm = 0; zone_table[zone].cmd[comm].command != 'S'; comm++) {
      resetCom &cmd = zone_table[zone].cmd[comm];
      switch (cmd.command) {
        case 'A':
	  if (cmd.arg1 < 0 || cmd.arg1 >= WORLD_SIZE)
            zone_table[zone].logError('A', "room 1",comm, cmd.arg1);
          if (cmd.arg2 < 0 || cmd.arg2 >= WORLD_SIZE)
            zone_table[zone].logError('A', "room 2",comm, cmd.arg2);
          if (cmd.arg2 <= cmd.arg1)
            zone_table[zone].logError('A', "no overlap",comm, cmd.arg2);
          break;
        case 'C':
          cmd.arg1 = real_mobile(value = cmd.arg1);
          if (cmd.arg1 < 0)
            zone_table[zone].logError('C', "mobile",comm, value);
          if (cmd.arg3 < 0 && cmd.arg3 != ZONE_ROOM_RANDOM)
            zone_table[zone].logError('C', "room",comm, cmd.arg3);
          break;
        case 'K':
          cmd.arg1 = real_mobile(value = cmd.arg1);
          if (cmd.arg1 < 0)
            zone_table[zone].logError('K', "mobile",comm, value);
          if (cmd.arg3 < 0 && cmd.arg3 != ZONE_ROOM_RANDOM)
            zone_table[zone].logError('K', "room",comm, cmd.arg3);
          break;
        case 'M':
          cmd.arg1 = real_mobile(value = cmd.arg1);
          if (cmd.arg1 < 0)
            zone_table[zone].logError('M', "mobile",comm, value);
          if (cmd.arg3 < 0 && cmd.arg3 != ZONE_ROOM_RANDOM)
            zone_table[zone].logError('M', "room",comm, cmd.arg3);
          break;
        case 'R':
          cmd.arg1 = real_mobile(value = cmd.arg1);
          if (cmd.arg1 < 0)
            zone_table[zone].logError('R', "mobile",comm, value);
          if (cmd.arg3 < 0 && cmd.arg3 != ZONE_ROOM_RANDOM)
            zone_table[zone].logError('R', "room",comm, cmd.arg3);
          break;
        case 'O':
          cmd.arg1 = real_object(value = cmd.arg1);
          if (cmd.arg1 < 0)
            zone_table[zone].logError('O', "object",comm, value);
          if (cmd.arg3 < 0 && cmd.arg3 != ZONE_ROOM_RANDOM)
            zone_table[zone].logError('O', "room",comm, cmd.arg3);
          break;
        case 'G':
          cmd.arg1 = real_object(value = cmd.arg1);
          if (cmd.arg1 < 0)
            zone_table[zone].logError('G', "object",comm, value);
          break;
	case 'X': // X <set num> <slot> <vnum>
	  if (cmd.arg3 < 0 || cmd.arg3 > 15)
	    zone_table[zone].logError('X', "macro",comm, cmd.arg2);
          cmd.arg1 = mapFileToSlot(value = cmd.arg1); 
          if (cmd.arg1 < MIN_WEAR || cmd.arg1 >= MAX_WEAR)
            zone_table[zone].logError('X', "bogus slot",comm, value);
	  break;
	case 'Z': // Z <if flag> <set num> <perc chance>
	  if (cmd.arg1 < 0 || cmd.arg1 > 15)
	    zone_table[zone].logError('Z', "macro",comm, cmd.arg3);
	  if (cmd.arg2 <= 0 || cmd.arg2 > 100)
	    zone_table[zone].logError('Z', "percent",comm, cmd.arg2);
	  break;
// Add one for each suit load ..loadset
        case 'Y':
          if (cmd.arg1 <= 0 || cmd.arg1 > (signed) suitSets.suits.size())
            zone_table[zone].logError('Y', "macro",comm, cmd.arg1);
          if (cmd.arg2 <= 0 || cmd.arg2 > 100)
            zone_table[zone].logError('Y', "percent",comm, cmd.arg2);
          break;
        case 'E':
          cmd.arg1 = real_object(value = cmd.arg1);
          if (cmd.arg1 < 0)
            zone_table[zone].logError('E', "object",comm, value);
          cmd.arg3 = mapFileToSlot(value = cmd.arg3); 
          if (cmd.arg3 < MIN_WEAR || cmd.arg3 >= MAX_WEAR)
            zone_table[zone].logError('E', "bogus slot",comm, value);
          break;
        case 'P':
          cmd.arg1 = real_object(value = cmd.arg1);
          if (cmd.arg1 < 0)
            zone_table[zone].logError('P', "object",comm, value);
          cmd.arg3 = real_object(cmd.arg3);
          if (cmd.arg3 < 0)
            zone_table[zone].logError('P', "container",comm, cmd.arg3);
          break;
        case 'D':
          if (cmd.arg1 < 0)
            zone_table[zone].logError('D', "room",comm, cmd.arg1);
          break;
        case 'B':
          cmd.arg1 = real_object(value = cmd.arg1);
          if (cmd.arg1 < 0)
            zone_table[zone].logError('B', "object",comm, value);
          if (cmd.arg3 < 0 && cmd.arg3 != ZONE_ROOM_RANDOM)
            zone_table[zone].logError('B', "room",comm, cmd.arg3);
          break;
        case 'H':
          if (cmd.arg1 < MIN_HATE || cmd.arg1 >= MAX_HATE) {
            zone_table[zone].logError('H', "hate",comm, cmd.arg1);
            cmd.arg1 = MIN_HATE;
          }
          break;
        case 'F':
          if (cmd.arg1 < MIN_HATE || cmd.arg1 >= MAX_HATE) {
            zone_table[zone].logError('F', "fear",comm, cmd.arg1);
            cmd.arg1 = MIN_HATE;
          }
          break;
      }
    }
  }
}

void bootZones(void)
{
  FILE *fl;
  int zon = 0, tmp;
  char *check, buf[256];
  int rc, zone_nr = 0;
  int i1 = 0, i2, i3, i4;

  if (!(fl = fopen(ZONE_FILE, "r"))) {
    perror("bootZones");
    exit(0);
  }
  for (;;) {
    fscanf(fl, " #%*d\n");
    check = fread_string(fl);

    if (*check == '$')
      break;                        

    zoneData zd;
    zd.name = check;
    zd.zone_nr=zone_nr++;
    rc = fscanf(fl, " %d %d %d %d", &i1, &i2, &i3, &i4);
    if (rc == 4) {
      zd.top = i1;
      zd.lifespan = i2;
      zd.reset_mode = i3;
      zd.enabled = i4;
      zd.age = 0;
    } else { 
      vlogf(LOG_LOW, "Bad zone format for zone %d (%s)", zon, check);
      exit(0);
    }

    for (;;) {
      resetCom rs;

      fscanf(fl, " ");                
      fscanf(fl, "%c", &rs.command);

      if (rs.command == 'S') {
        zd.cmd.push_back(rs);
        break;
      }

      if (rs.command == '*') {
        fgets(buf, 255, fl);        
        continue;
      }

#if 0
// ZZ
if (rs.command == 'X' || rs.command == 'Z') {
fgets(buf, 255, fl);        
continue;
}
#endif

      int numc = fscanf(fl, " %d %d %d", &tmp, &rs.arg1, &rs.arg2);
      if (numc != 3)
        vlogf(LOG_LOW,"command %u ('%c') in %s missing some of first three args [%d : %d %d %d]",
               zd.cmd.size(),
               rs.command,
               zd.name,
               numc,
               numc >= 1 ? tmp : -99,
               numc >= 2 ? rs.arg1 : -99,
               numc >= 3 ? rs.arg2 : -99);

      if(rs.command=='X')
	rs.arg3 = tmp;
      else
	rs.if_flag = tmp;
      
      switch (rs.command) {
        case 'G':
        case 'P':
        case 'E':
          if (!rs.if_flag) {
            vlogf(LOG_LOW,"command %u in %s has bogus if_flag",
               zd.cmd.size(),zd.name);
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
          vlogf(LOG_LOW,"command %u ('%c') in %s missing arg3 (rc=%d)",
               zd.cmd.size(),
               rs.command,
               zd.name, rc);

      if (rs.command == '?')
        if (fscanf(fl, " %c", &rs.character) != 1)
          vlogf(LOG_LOW,"command %u ('?') in %s missing character",
               zd.cmd.size(),zd.name);

      if (rs.command == 'T' &&
          !rs.if_flag) 
        if (fscanf(fl, " %d", &rs.arg4) != 1)
          vlogf(LOG_LOW,"command %u ('T') in %s missing arg4",
               zd.cmd.size(), zd.name);

      if (rs.command == 'L')
        if (fscanf(fl, " %d", &rs.arg4) != 1)
          vlogf(LOG_LOW, "command %u ('L') in %s missing arg4",
                zd.cmd.size(), zd.name);

      zd.cmd.push_back(rs);

      fgets(buf, 255, fl);        
    }

    zone_table.push_back(zd);
  }

  // the only way out of the above loop is when check = "$"
  // delete this final line as not needed.
  delete [] check;

  fclose(fl);
}

TMonster *read_mobile(int nr, readFileTypeT type)
{
  int i, rc;
  TMonster *mob = NULL;

  i = nr;

  if (type == VIRTUAL) {
    nr = real_mobile(nr);
  }
  if (nr < 0) {
    vlogf(LOG_FILE, "Mobile (V) %d does not exist in database.", i);
    return NULL;
  }
  fseek(mob_f, mob_index[nr].pos, 0);

  try {
    mob = new TMonster();
  } catch (...) {
    forceCrash("caught an exception");
    return NULL;
  }
  mob->number = nr;

  rc = mob->readMobFromFile(mob_f, FALSE);
  if (IS_SET_DELETE(rc, DELETE_THIS)) {
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

  mob->next = character_list;
  character_list = mob;

  mob->checkMobStats(TINYFILE_YES);

  mob->setRacialStuff();

  wearSlotT j;
  for (j = MIN_WEAR; j < MAX_WEAR; j++) {
    mob->setLimbFlags(j, 0);
    mob->setCurLimbHealth(j, mob->getMaxLimbHealth(j));
    mob->setStuckIn(j, NULL);
  }
  mob_index[nr].number++;

  mob->checkSpec(mob, CMD_GENERIC_CREATED, "", NULL);

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

  return (mob);
}

#if !USE_SQL
TObj *read_object(int nr, readFileTypeT type)
{
  TObj *obj = NULL;
  int tmp = 0, i, rc;
  int tmp2, tmp3, tmp4;
  float tmpf;
  char chk[50];

  i = nr;
  vlogf(LOG_FILE, "Checking object %d", nr);
  if (type == VIRTUAL)
    nr = real_object(nr);

  // I changed this assert to just a log and NULL return - Russ 020997
  // Because if someone has a bad object somehow in rent, it'll just "crash"
  // in this assert over and over as they try to log in.
  // Assert are usually only good for unrecoverable happenings. This
  // is recoverable.
  //mud_assert(nr >= 0 && nr < obj_index.size(), "read_object: bad nr %d", nr);

  if ((nr < 0) || (nr >= (signed int) obj_index.size())) {
    forceCrash( "read_object: bad nr %d (i = %d)", nr, i);
    return NULL;
  }

  fseek(obj_f, obj_index[nr].pos, 0);

  // We allocated the names and strings in the indeces, don't do it again!
  readStringNoAlloc(obj_f);
  readStringNoAlloc(obj_f);
  readStringNoAlloc(obj_f);
  readStringNoAlloc(obj_f);

  if (fscanf(obj_f, " %d ", &tmp) != 1) {
    vlogf(LOG_MISC, "Unable to read item type in read_object");
    forceCrash("bad obj read, (%s : %d)", obj_index[nr].name, nr);
    return NULL;
  }

  obj = makeNewObj(mapFileToItemType(tmp));

  obj->name = obj_index[nr].name;
  obj->shortDescr = obj_index[nr].short_desc;
  obj->setDescr(obj_index[nr].long_desc);
  obj->action_description = obj_index[nr].description;

  if (obj_index[nr].itemtype == MAX_OBJ_TYPES)
    obj_index[nr].itemtype = tmp;
  
  fscanf(obj_f, " %d ", &tmp);
  obj->setObjStat(tmp);
  fscanf(obj_f, " %d ", &tmp);
  obj->obj_flags.wear_flags = tmp;
  if (obj_index[nr].where_worn == 0)
    (obj_index[nr].where_worn = tmp);

  fscanf(obj_f, " %d %d %d %d ", &tmp, &tmp2, &tmp3, &tmp4);
  obj->assignFourValues(tmp, tmp2, tmp3, tmp4);

  fscanf(obj_f, " %f ", &tmpf);
  obj->setWeight(tmpf);
  if (obj_index[nr].weight == (float) 0.0)
    obj_index[nr].weight = (float) tmpf;
  fscanf(obj_f, " %d ", &tmp);
  obj->obj_flags.cost = tmp;
  if (obj_index[nr].value == -99)
    obj_index[nr].value = tmp;
  fscanf(obj_f, " %d ", &tmp);
  obj->canBeSeen = tmp;
  fscanf(obj_f, " %d ", &tmp);
  obj->spec = tmp;
  if (obj_index[nr].spec == -99)
    obj_index[nr].spec = tmp;
  fscanf(obj_f, " %d ", &tmp);

  // beta is used to test LOW loads, so don't let max_exist be a factor
  obj->max_exist = (gamePort == BETA_GAMEPORT ? 9999 : tmp);

  if (obj_index[nr].max_exist == -99) {
    obj_index[nr].max_exist = tmp;
    // use 327 so we don't go over 32765 in calculation
    if (obj_index[nr].max_exist < 327) {
      obj_index[nr].max_exist *= (sh_int) (stats.max_exist * 100);
      obj_index[nr].max_exist /= 100;
    }
    if (tmp)
      obj_index[nr].max_exist = max(obj_index[nr].max_exist, (short int) (gamePort == BETA_GAMEPORT ? 9999 : 1));
  }
  fscanf(obj_f, " %d ", &tmp);
  obj->setStructPoints(tmp);
  fscanf(obj_f, " %d ", &tmp);
  obj->setMaxStructPoints(tmp);
  if (obj_index[nr].max_struct == -99)
    (obj_index[nr].max_struct = tmp);
  obj->setDepreciation(0);

  fscanf(obj_f, " %d ", &tmp);
  obj->obj_flags.decay_time = tmp;
  fscanf(obj_f, " %d ", &tmp);
  obj->setVolume(tmp);
  fscanf(obj_f, " %d ", &tmp);
  obj->setMaterial((ubyte) tmp);

  obj->ex_description = obj_index[nr].ex_description;

  while (fscanf(obj_f, " %s \n", chk), *chk == 'E') {
    // We allocated the names and strings in the indeces, don't do it again!
    readStringNoAlloc(obj_f);
    readStringNoAlloc(obj_f);
  }

  for (i = 0; (i < MAX_OBJ_AFFECT) && (*chk == 'A'); i++) {
    fscanf(obj_f, " %d ", &tmp);
    obj->affected[i].location = mapFileToApply(tmp);
    fscanf(obj_f, " %d ", &tmp);

    if (obj->affected[i].location == APPLY_SPELL)
      obj->affected[i].modifier = mapFileToSpellnum(tmp);
    else
      obj->affected[i].modifier = tmp;

    fscanf(obj_f, " %d \n", &tmp);
    obj->affected[i].modifier2 = tmp;
    if (obj->affected[i].location == APPLY_LIGHT)
      obj->addToLight(obj->affected[i].modifier);
    obj->affected[i].type = TYPE_UNDEFINED;
    obj->affected[i].level = 0;
    obj->affected[i].bitvector = 0;
    fscanf(obj_f, " %s \n", chk);
  }

  for (; (i < MAX_OBJ_AFFECT); i++) {
    obj->affected[i].location = APPLY_NONE;
    obj->affected[i].modifier = 0;
    obj->affected[i].type = TYPE_UNDEFINED;
    obj->affected[i].level = 0;
    obj->affected[i].bitvector = 0;
  }
  obj->number = nr;

  if (nr >= 0)
    obj_index[nr].number++;

  obj->weightCorrection();

  rc = obj->checkSpec(NULL, CMD_GENERIC_CREATED, "", NULL);
  if (IS_SET_DELETE(rc, DELETE_THIS)) {
    delete obj;
    obj = NULL;
    return NULL;
  }

  obj->checkObjStats();
  return (obj);
}
#else
TObj *read_object(int nr, readFileTypeT type)
{
  TObj *obj = NULL;
  int i, rc, tmpcost;
  TDatabase db("sneezy");

  i = nr;
  if (type == VIRTUAL)
    nr = real_object(nr);

  if ((nr < 0) || (nr >= (signed int) obj_index.size())) {
    forceCrash( "read_object: bad nr %d (i = %d)", nr, i);
    return NULL;
  }

  db.query("select type, action_flag, wear_flag, val0, val1, val2, val3, weight, price, can_be_seen, spec_proc, max_struct, cur_struct, decay, volume, material, max_exist from obj where vnum=%i", obj_index[nr].virt);
  
  if(!db.fetchRow())
    return NULL;
  

  obj = makeNewObj(mapFileToItemType(atoi(db.getColumn(0))));
  obj->number=nr;
  obj->name = obj_index[nr].name;
  obj->shortDescr = obj_index[nr].short_desc;
  obj->setDescr(obj_index[nr].long_desc);
  obj->action_description = obj_index[nr].description;
  obj->setObjStat(atoi(db.getColumn(1)));
  obj->obj_flags.wear_flags = atoi(db.getColumn(2));
  obj->assignFourValues(atoi(db.getColumn(3)), atoi(db.getColumn(4)), atoi(db.getColumn(5)), atoi(db.getColumn(6)));
  obj->setWeight(atof(db.getColumn(7)));
  obj->obj_flags.cost = atoi(db.getColumn(8));
  obj->canBeSeen = atoi(db.getColumn(9));
  obj->spec = atoi(db.getColumn(10));
  obj->setMaxStructPoints(atoi(db.getColumn(11)));
  obj->setStructPoints(atoi(db.getColumn(12)));
  obj->setDepreciation(0);
  obj->obj_flags.decay_time=atoi(db.getColumn(13));
  obj->setVolume(atoi(db.getColumn(14)));
  obj->setMaterial(atoi(db.getColumn(15)));
  // beta is used to test LOW loads, so don't let max_exist be a factor
  obj->max_exist = (gamePort == BETA_GAMEPORT ? 9999 : atoi(db.getColumn(16)));
  obj->ex_description=obj_index[nr].ex_description;

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

  obj_index[nr].number++;
  
  obj->weightCorrection();

  rc = obj->checkSpec(NULL, CMD_GENERIC_CREATED, "", NULL);
  if (IS_SET_DELETE(rc, DELETE_THIS)) {
    delete obj;
    obj = NULL;
    return NULL;
  }


#if 1
  // use suggested price if available, otherwise use the set price
  if((tmpcost = obj->suggestedPrice())){
    obj->obj_flags.cost = tmpcost;
  }
#endif

  obj->checkObjStats();

  return (obj);
}
#endif

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


// update zone ages, queue for reset if necessary, and dequeue when possible
void zone_update(void)
{
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
        return WEAR_LEGS_R;
      case 7:
        return WEAR_LEGS_L;
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
        return WEAR_WAISTE;
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
        vlogf(LOG_LOW, "Bogus slot (%d, 1) in zone file", num);
        forceCrash("forced crash");
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
    case WEAR_LEGS_R:
      return 6;
    case WEAR_LEGS_L:
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
    case WEAR_WAISTE:
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
      vlogf(LOG_LOW, "Bogus slot (%d, 2) in zone file", num);
      forceCrash("forced crash");
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
    vlogf(LOG_LOW, "Mob (%s:%d) has a utility proc (%s:%d) and is not in zone #0",
         mob->getName(), mob->mobVnum(), 
         mob_specials[mob->spec].name, mob->spec);

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

void zoneData::resetZone(bool bootTime)
{
  int cmd_no;
  bool last_cmd = 1;
  bool mobload = FALSE;
  bool objload = FALSE;
  TMonster *mob = NULL;
  TMonster *old_mob = NULL;
  TObj *obj = NULL, *obj_to = NULL;
  TRoom *rp = NULL, *storageRoom = NULL;
  TRoom *random_room = NULL;
  TThing *t;
  int count;
  wearSlotT realslot;
  char doorbuf[80];

  struct armor_set_struct {
    int slots[MAX_WEAR];
  } local_armor[16];
  memset(local_armor, 0, sizeof(struct armor_set_struct)*16);

  if (this->enabled == FALSE && gamePort == PROD_GAMEPORT) {
    if (bootTime)
      vlogf(LOG_MISC, "*** Zone was disabled.");
    return;
  }
  storageRoom = real_roomp(ROOM_NOCTURNAL_STORAGE);
  for (cmd_no = 0;; cmd_no++) {
    resetCom &rs = this->cmd[cmd_no];
    if (rs.command == 'S')
      break;

    if (last_cmd || !rs.if_flag)
      switch (rs.command) {
        case 'A':
          for (count = 0; count < 10; count++) {
            if ((random_room = real_roomp(::number(rs.arg1, rs.arg2))))
              break;
          }
          if (!random_room) {
            vlogf(LOG_LOW, "Unable to detect room in 'A' %d %d", 
                 rs.arg1, rs.arg2);
          }
          break;
        case 'M':
          // check if zone is disabled or if mob exceeds absolute max
          if (rs.arg1 < 0 || rs.arg1 >= (signed int) mob_index.size()) {
            vlogf(LOG_LOW, "Detected bogus mob number (%d) on read_mobile call for resetZone (load room %d).", rs.arg1, rs.arg3);
            last_cmd = 0;
            mobload = 0;
            continue;
          }
          if ((this->zone_value != 0) &&
              mob_index[rs.arg1].number < mob_index[rs.arg1].max_exist) {
            if (rs.arg3 != ZONE_ROOM_RANDOM) {
              rp = real_roomp(rs.arg3);
            } else {
              rp = random_room;
              random_room = NULL;
            }
            if (!rp) {
              vlogf(LOG_LOW, "No room (%d) in M command (%d)", 
                  rs.arg3, rs.arg1);
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
              vlogf(LOG_BUG, "Error reading mobile (%d).  You suck.", rs.arg1);
              last_cmd = 0;
              mobload = 0;
              continue;
            }

            mob->createWealth();

	    if(mob->spec != SPEC_SHOPKEEPER)
	      saveGovMoney("mob load wealth", mob->getMoney());

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
            last_cmd = 0;
            mobload = 0;
          }
          break;
        case 'C':
          // a charmed follower of the previous mob

          // check if zone is disabled or if mob exceeds absolute max
          if ((this->zone_value != 0) && mobload &&
              mob_index[rs.arg1].number < mob_index[rs.arg1].max_exist) {
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
              vlogf(LOG_LOW, "No room (%d) in C command (%d)",
                  rs.arg3, rs.arg1);
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

	    if(mob->spec != SPEC_SHOPKEEPER)
	      saveGovMoney("mob load wealth", mob->getMoney());


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
          // a grouped follower of the previous mob

          // check if zone is disabled or if mob exceeds absolute max
          if ((this->zone_value != 0) && mobload &&
              mob_index[rs.arg1].number < mob_index[rs.arg1].max_exist) {
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
              vlogf(LOG_LOW, "No room (%d) in K command (%d)",
                  rs.arg3, rs.arg1);
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

	    if(mob->spec != SPEC_SHOPKEEPER)
	      saveGovMoney("mob load wealth", mob->getMoney());

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

              int tmp = dice(1, 100);
              if (tmp <= rs.arg1 || gamePort == BETA_GAMEPORT) {
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
          // check if zone is disabled or if mob exceeds absolute max
          if ((this->zone_value != 0) && mobload &&
              mob_index[rs.arg1].number < mob_index[rs.arg1].max_exist) {
            if (rs.arg3 != ZONE_ROOM_RANDOM) {
              rp = real_roomp(rs.arg3);
            } else {
              rp = random_room;
              random_room = NULL;
            }
            if (!rp) {
              vlogf(LOG_LOW, "No room (%d) in R command (%d)",
                  rs.arg3, rs.arg1);
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

	    if(mob->spec != SPEC_SHOPKEEPER)
	      saveGovMoney("mob load wealth", mob->getMoney());
	    

#if 1
            // Slap the mob on the born list.
            *rp << *mob;
#endif

            if (old_mob->getHeight() <= (6 * mob->getHeight() / 10))
              vlogf(LOG_LOW, "Mob mounting mount that is too small.  [%s] [%s]",
                    mob->getName(), old_mob->getName());

            if (old_mob->getHeight() >= (5 * mob->getHeight() / 2))
              vlogf(LOG_LOW, "Mob mounting mount that is too big.  [%s] [%s]",
                    mob->getName(), old_mob->getName());

            if (compareWeights(mob->getTotalWeight(TRUE),
                               (old_mob->carryWeightLimit() -
                                old_mob->getCarriedWeight())) == -1)
              vlogf(LOG_LOW, "Mob mounting mount that is too weak.  [%s] [%s]",
                    mob->getName(), old_mob->getName());

            if (old_mob->GetMaxLevel() > mob->GetMaxLevel())
              vlogf(LOG_LOW, "Mob mounting mount that is too strong.  [%s:%d] [%s:%d]",
                    mob->getName(), mob->GetMaxLevel(),
                    old_mob->getName(), old_mob->GetMaxLevel());

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
          if ((bootTime) && (obj_index[rs.arg1].number < 
                 obj_index[rs.arg1].max_exist)) {
            if (rs.arg3 != ZONE_ROOM_RANDOM) 
              rp = real_roomp(rs.arg3);
            else {
              rp = random_room;
              random_room = NULL;
            }
            if (!rp) {
              vlogf(LOG_LOW, "No room (%d) in O command (%d).  cmd=%d, zone=%d", rs.arg3, rs.arg1, cmd_no, zone_nr);
              last_cmd = 0;
              objload = 0;
              continue;
            }

            obj = read_object(rs.arg1, REAL);
            if (obj != NULL) {
              *rp += *obj;
              obj->onObjLoad();
              last_cmd = 1;
              objload = TRUE;
            } else {
              vlogf(LOG_LOW, "No obj (%d) in O command (room=%d).  cmd=%d, zone=%d", rs.arg1, rs.arg3, cmd_no, zone_nr);
              objload = FALSE;
              last_cmd = 0;
            }
          } else {
            objload = FALSE;
            last_cmd = 0;
          }
          break;
        case 'B':               
          if (obj_index[rs.arg1].number <
                 obj_index[rs.arg1].max_exist) {
            if (rs.arg3 != ZONE_ROOM_RANDOM) {
              rp = real_roomp(rs.arg3);
            } else {
              rp = random_room;
              random_room = NULL;
            }
            if (!rp) {
              vlogf(LOG_LOW, "No room (%d) in B command (%d)",
                  rs.arg3, rs.arg1);
              last_cmd = 0;
              objload = 0;
              continue;
            }

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
          } else {
            objload = FALSE;
            last_cmd = 0;
          }
          break;
        case 'P':                
          if (obj_index[rs.arg1].number < obj_index[rs.arg1].max_exist) {
            obj = read_object(rs.arg1, REAL);
            obj_to = get_obj_num(rs.arg3);
            if (obj_to && obj && dynamic_cast<TBaseContainer *>(obj_to)) {
              *obj_to += *obj;
              obj->onObjLoad();
              last_cmd = 1;
            } else if (obj_to && obj && dynamic_cast<TTable *>(obj_to)) {
              obj->mount(obj_to);
              obj->onObjLoad();
              last_cmd = 1;
            } else 
              last_cmd = 0;
          } else
            last_cmd = 0;
          break;
        case 'V':    // Change ONE value of the four values upon reset- Russ 
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
                vlogf(LOG_LOW, "Bad slot (%d) for V command (%d)",
                                    rs.arg1, rs.arg2);
            }
            last_cmd = 1;
          } else
            last_cmd = 0;
          break;
        case 'T':        // Set traps for doors and containers - Russ 
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
          mud_assert(rs.arg1 >= 0 && rs.arg1 < (signed int) obj_index.size(), "Range error (%d not in obj_index)  G command #%d in %s", rs.arg1, cmd_no, this->name);
          if (obj_index[rs.arg1].number < obj_index[rs.arg1].max_exist &&
              (obj = read_object(rs.arg1, REAL))) {
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
	  if (mob && mobload && rs.arg1 >=0) {
	    wearSlotT i;
	    for (i = MIN_WEAR; i < MAX_WEAR; i++) {
	      if (local_armor[rs.arg1].slots[i]) {
		loadsetCheck(mob, local_armor[rs.arg1].slots[i], rs.arg2, 
			     i, "(null... for now)");
	      }
	    }
	  }
	  break;
        case 'Y':
          if (mob && mobload) {
            mob->loadSetEquipment(rs.arg1, NULL, rs.arg2);

            if (mob->hasClass(CLASS_MAGE | CLASS_MAGE_THIEF)) {
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
          if (mob->addFears(zoneHateT(rs.arg1), rs.arg2))
            last_cmd = 1;
          else
            last_cmd = 0;
          break;
        case 'E':                
          if ((obj_index[rs.arg1].number < obj_index[rs.arg1].max_exist) &&
              (::number(0,99) < (int) (100 * stats.equip)) &&  
              (obj = read_object(rs.arg1, REAL))) {
            if (!mob) {
              vlogf(LOG_LOW, "no mob for 'E' command.  Obj (%s)", obj->getName());
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
                vlogf(LOG_LOW, "'E' command equipping unusable item (%s:%d) on (%s:%d).", obj->getName(), obj->objVnum(), mob->getName(), mob->mobVnum());
              }
              TBaseClothing *tbc = dynamic_cast<TBaseClothing *>(obj);
              if (tbc && tbc->canWear(ITEM_WEAR_FINGER) && gamePort != PROD_GAMEPORT) {
                vlogf(LOG_LOW, "RINGLOAD: [%s][%-6.2f] loading on [%s][%d]",
                      obj->getName(), tbc->armorLevel(ARMOR_LEV_REAL),
                      mob->getName(), mob->GetMaxLevel());

              }
              if (tbc && !mob->validEquipSlot(realslot) && !tbc->isSaddle()) {
                vlogf(LOG_LOW, "'E' command for %s equipping item (%s) on nonvalid slot %d.", mob->getName(), tbc->getName(), realslot);
              }
              if (!check_size_restrictions(mob, obj, realslot, mob) &&
                  realslot != HOLD_RIGHT && realslot != HOLD_LEFT) {
                int size_per = 100;
                if (race_vol_constants[mapSlotToFile(realslot)]) {
                  size_per = 100 * obj->getVolume() / race_vol_constants[mapSlotToFile( realslot)];
                  if (obj->isPaired())
                    size_per /= 2;
                }
                vlogf(LOG_LOW, "'E' for (%s:%d) equipping (%s:%d) with bad fit. (m:%d%%/o:%d%%) change vol to %d, or height to %d.", 
                    mob->getName(), mob->mobVnum(), 
                    obj->getName(), obj->objVnum(),
                    mob->getHeight() * 100 / 70,
                    size_per, 
                    (mob->getHeight() * (obj->isPaired() ? 2 : 1) *
                       race_vol_constants[mapSlotToFile( realslot)] / 70),
                    size_per * 70 / 100
                    );
              }
              // OK, actually do the equip
              mob->equipChar(obj, realslot);
              mob->logItem(obj, CMD_LOAD);

              // for items without levels, objLevel = 0 so this logic is OK
              double al = obj->objLevel();
              double grl = mob->getRealLevel();
              if (al > (grl + 1)) {
                vlogf(LOG_LOW, "Mob (%s:%d) of level %.1f loading item (%s:%d) thought to be level %.1f.", mob->getName(), mob->mobVnum(), grl, obj->getName(), obj->objVnum(), al);
              }

            } else {
              vlogf(LOG_LOW, "'E' command operating on already equipped slot.  %s, %s slot %d\n\rpre-equipped with %s, is_same: %s", 
                    mob->getName(), obj->getName(), realslot,
                    mob->equipment[realslot]->getName(),
                    ((mob->equipment[realslot] == obj) ? "true" : "false"));
              delete obj;
              obj = NULL;
              last_cmd = 0;
              objload = 0;
              continue;
            }
            if (!mob->equipment[realslot])
              vlogf(LOG_LOW, "Zone-file %s (%d) failed to equip %s (%d)",
                   mob->getName(), mob->mobVnum(), obj->getName(), obj->objVnum());
            last_cmd = 1;
          } else {
            repoCheck(mob, rs.arg1);
            last_cmd = 0;
            objload = 0;
          }
          
          break;
        case 'D':                
          rp = real_roomp(rs.arg1);
          if (rp) {
            roomDirData * exitp = rp->dir_option[rs.arg2];
            if (exitp &&
               !IS_SET(exitp->condition, EX_DESTROYED) &&
               !IS_SET(exitp->condition, EX_CAVED_IN)) {
              if (exitp->door_type != DOOR_NONE) {
                strcpy(doorbuf, exitp->getName().c_str());
                switch (rs.arg3) {
                  case 0:
                    if (IS_SET(exitp->condition, EX_CLOSED))
                      sendrpf(rp, "The %s opens.\n\r", uncap(doorbuf));
                    REMOVE_BIT(exitp->condition, EX_LOCKED);
                    REMOVE_BIT(exitp->condition, EX_CLOSED);
                    break;
                  case 1:
                    if (!IS_SET(exitp->condition, EX_CLOSED))
                      sendrpf(rp, "The %s closes.\n\r", uncap(doorbuf));
                    SET_BIT(exitp->condition, EX_CLOSED);
                    REMOVE_BIT(exitp->condition, EX_LOCKED);
                    break;
                  case 2:
                    if (!IS_SET(exitp->condition, EX_CLOSED))
                      sendrpf(rp, "The %s closes.\n\r", uncap(doorbuf));
                    SET_BIT(exitp->condition, EX_LOCKED);
                    SET_BIT(exitp->condition, EX_CLOSED);
                    break;
                }
                last_cmd = 1;
              } else {
                vlogf(LOG_LOW, "'D' command operating on DOOR_NONE in rm %d", rp->number); 
              }
            } // check for valid,legal exit
          }  // check for dest room
          break;
        case 'L':
          if (bootTime)
            last_cmd = sysLootLoad(rs, mob, obj, false);
          else
            last_cmd = 0;
          break;
        default:
          vlogf(LOG_BUG, "Undefd cmd in reset table; zone %d cmd %d.\n\r", zone_nr, cmd_no);
          break;
    } else
      last_cmd = 0;
  }
  this->age = 0;
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
    //  Check if we've hit the end of string marker. 
    if ((marker=strchr(ptr, '~')) != 0) 
      break;
    //  Set the pointer to the end of the string. NOTE: This is better then
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

// read and allocate space for a '~'-terminated string from a given file 
char *fread_string(FILE *fp)
{
  char buf[MAX_STRING_LENGTH], *ptr, *marker = NULL;

  *buf = 0;
  ptr = buf;
  unsigned int read_len = MAX_STRING_LENGTH;
  while(fgets(ptr, read_len, fp)) {
    //  Check if we've hit the end of string marker. 
    if((marker=strchr(ptr, '~')) != 0) {
      break;
    }  
    //  Set the pointer to the end of the string. NOTE: This is better then the
    // strlen because we're not starting at the beggining every time. 
    if((ptr = strchr(ptr, '\000')) == 0) {
      vlogf(LOG_FILE, "fread_string(): read error. ack!");
      return mud_str_dup("Empty");
    }
    //  Add the return char. 
    *ptr++ = '\r';

    if ((int) (ptr - buf) >= (int) sizeof(buf)) {
    vlogf(LOG_MISC, "SHIT! buf overflow!");
      forceCrash("buf overflow");
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
bool file_to_string(const char *name, string &buf, concatT concat)
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
      vlogf(LOG_SILENT, "real_object: probable failure for %d", virt);
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
    case ITEM_UNDEFINED:
    case ITEM_MARTIAL_WEAPON:
    case MAX_OBJ_TYPES:
      forceCrash("Unknown item type (%d)", tmp);
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
  top(0),
  reset_mode(0),
  enabled(false),
  zone_value(0),
  num_mobs(0),
  mob_levels(0),
  min_mob_level(128),
  max_mob_level(0),
  cmd(0)
{
}

zoneData::zoneData(const zoneData &t) :
  zone_nr(t.zone_nr),
  lifespan(t.lifespan),
  age(t.age),
  top(t.top),
  reset_mode(t.reset_mode),
  enabled(t.enabled),
  zone_value(t.zone_value),
  num_mobs(t.num_mobs),
  mob_levels(t.mob_levels),
  min_mob_level(t.min_mob_level),
  max_mob_level(t.max_mob_level),
  cmd(t.cmd)
{
  name = mud_str_dup(t.name);
}

zoneData::~zoneData()
{
  delete [] name;
  cmd.erase(cmd.begin(), cmd.end());
}

zoneData & zoneData::operator= (const zoneData &t)
{
  if (this == &t) return *this;

  delete [] name;
  name = mud_str_dup(t.name);

  zone_nr = t.zone_nr;
  lifespan = t.lifespan;
  age = t.age;
  top = t.top;
  reset_mode = t.reset_mode;
  enabled = t.enabled;
  zone_value = t.zone_value;
  num_mobs = t.num_mobs;
  mob_levels = t.mob_levels;
  min_mob_level = t.min_mob_level;
  max_mob_level = t.max_mob_level;

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
  while(object_list)
    delete object_list;
  // purge all rooms
  int ii;
  for (ii = 0; ii < WORLD_SIZE; ii++)
    delete room_db[ii];

  // no TThings exist anymore at this point

  // close up open handlers
  if (obj_f)
    fclose(obj_f);
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


void saveGovMoney(const char *what, int talens){
  TDatabase db("sneezy");

  db.query("update govmoney set talens=talens+%i where type='%s'", talens, what);
}

int getGovMoney(int talens){
  int amount=talens, transaction=0;
  TDatabase db("sneezy");

  db.query("select type, talens from govmoney where talens>0");

  while((db.fetchRow()) && amount>0){
    if(atoi(db.getColumn(1)) < amount){
      transaction=atoi(db.getColumn(1));
    } else {
      transaction=amount;
    }

    db.query("update govmoney set talens=talens-%i where type='%s'", transaction, db.getColumn(0));

    amount-=transaction;
  }

  return(talens-amount);
}

int find_shopnr(int number){
  unsigned int shop_nr;

  for (shop_nr = 0; (shop_nr < shop_index.size()) && (shop_index[shop_nr].keeper != number); shop_nr++);
  
  if (shop_nr >= shop_index.size()) {
    vlogf(LOG_BUG, "Warning... shop # for mobile %d (real nr) not found.", number);
    return FALSE;
  }

  return shop_nr;
}


void countMobWealth(){
  int wealth=0, shopwealth=0;
      
  for (TBeing *tmp_ch = character_list; tmp_ch; tmp_ch = tmp_ch->next) {
    if(dynamic_cast<TMonster *>(tmp_ch)){
      if(tmp_ch->spec != SPEC_SHOPKEEPER){
	wealth+=tmp_ch->getMoney();
      } else if(!shopOwned(find_shopnr(tmp_ch->number))){
	shopwealth+=tmp_ch->getMoney();
      }
    }
  }

  TDatabase db("sneezy");

  db.query("replace govmoney values ('mob wealth', %i)", wealth);
  db.query("replace govmoney values ('shop wealth', %i)", shopwealth);
}  


void bootGovMoney(){
  int mobLoadWealth=0, mobWealth=0;
  int shopLoadWealth=0, shopWealth=0;
  TDatabase db("sneezy");

  db.query("select talens from govmoney where type='mob load wealth' and 1=1");
  if(db.fetchRow())
    mobLoadWealth=atoi(db.getColumn(0));

  db.query("select talens from govmoney where type='mob wealth' and 1=1");
  if(db.fetchRow())
    mobWealth=atoi(db.getColumn(0));

  saveGovMoney("mob debt wealth", mobLoadWealth-mobWealth);

  db.query("update govmoney set talens=0 where type='mob load wealth' and 1=1");
  db.query("select talens from govmoney where type='shop load wealth' and 1=1");
  if(db.fetchRow())
    shopLoadWealth=atoi(db.getColumn(0));

  db.query("select talens from govmoney where type='shop wealth' and 1=1");
  if(db.fetchRow())
    shopWealth=atoi(db.getColumn(0));

  saveGovMoney("shop debt wealth", shopLoadWealth-shopWealth);
  
  db.query("update govmoney set talens=0 where type='shop load wealth' and 1=1");
}

