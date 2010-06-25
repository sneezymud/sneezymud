//////////////////////////////////////////////////////////////////////////
//
// SneezyMUD - All rights reserved, SneezyMUD Coding Team
//
//////////////////////////////////////////////////////////////////////////


#include "extern.h"
#include "configuration.h"
#include "statistics.h"
#include "database.h"

extern FILE *obj_f;
extern FILE *mob_f;

std::vector<mobIndexData>mob_index(0);
std::vector<objIndexData>obj_index(0);

indexData::indexData() :
  virt(0),
  pos(0),
  number(0),
  name(NULL),
  short_desc(NULL),
  long_desc(NULL),
  description(NULL),
  max_exist(-99),
  spec(0),
  weight(0)
{
}

indexData & indexData::operator= (const indexData &a)
{
  if (this == &a)
    return *this;

  virt = a.virt;
  pos = a.pos;
  number = a.number;
  delete [] name;
  delete [] short_desc;
  delete [] long_desc;
  delete [] description;
  name = mud_str_dup(a.name);
  short_desc = mud_str_dup(a.short_desc);
  long_desc = mud_str_dup(a.long_desc);
  description = mud_str_dup(a.description);

  max_exist = a.max_exist;
  spec = a.spec;
  weight = a.weight;

  return *this;
}

indexData::indexData(const indexData &a) :
  virt(a.virt),
  pos(a.pos),
  number(a.number),
  max_exist(a.max_exist),
  spec(a.spec),
  weight(a.weight)
{
  name = mud_str_dup(a.name);
  short_desc = mud_str_dup(a.short_desc);
  long_desc = mud_str_dup(a.long_desc);
  description = mud_str_dup(a.description);
}

indexData::~indexData()
{
  delete [] name;
  delete [] short_desc;
  delete [] long_desc;
  delete [] description;
}

mobIndexData::mobIndexData() :
  faction(-99),
  Class(-99),
  level(-99),
  race(-99),
  doesLoad(false),
  numberLoad(0)
{
}

mobIndexData & mobIndexData::operator= (const mobIndexData &a)
{
  if (this == &a)
    return *this;

  indexData::operator=(a);

  faction = a.faction;
  Class = a.Class;
  level = a.level;
  race = a.race;
  doesLoad = a.doesLoad;
  numberLoad = a.numberLoad;

  return *this;
}

mobIndexData::mobIndexData(const mobIndexData &a) :
  indexData(a),
  faction(a.faction),
  Class(a.Class),
  level(a.level),
  race(a.race),
  doesLoad(a.doesLoad),
  numberLoad(a.numberLoad)
{
}

mobIndexData::~mobIndexData()
{
}

objIndexData::objIndexData() :
  ex_description(NULL),
  max_struct(-99),
  armor(-99),
  where_worn(0),
  itemtype(MAX_OBJ_TYPES),
  value(-99)
{
}

objIndexData & objIndexData::operator= (const objIndexData &a)
{
  if (this == &a)
    return *this;

  indexData::operator=(a);

  // use copy operator;
  if (a.ex_description)
    ex_description = new extraDescription(*a.ex_description);
  else
    ex_description = NULL;

  max_struct = a.max_struct;
  armor = a.armor;
  where_worn = a.where_worn;
  itemtype = a.itemtype;
  value = a.value;

  int i;
  for(i=0;i<MAX_OBJ_AFFECT;++i){
    affected[i]=a.affected[i];
  }

  return *this;
}

objIndexData::objIndexData(const objIndexData &a) :
  indexData(a),
  max_struct(a.max_struct),
  armor(a.armor),
  where_worn(a.where_worn),
  itemtype(a.itemtype),
  value(a.value)
{
  // use copy operator;
  if (a.ex_description)
    ex_description = new extraDescription(*a.ex_description);
  else
    ex_description = NULL;

  int i;
  for(i=0;i<MAX_OBJ_AFFECT;++i){
    affected[i]=a.affected[i];
  }
}

objIndexData::~objIndexData()
{
  extraDescription *tmp;
  while ((tmp = ex_description)) {
    ex_description = tmp->next;
    delete tmp;
  }
}

// generate index table for object
void generate_obj_index()
{
  objIndexData *tmpi = NULL;
  extraDescription *new_descr;
  int i=0;

  // to prevent constant resizing (slows boot), declare an appropriate initial
  // size.  Should be smallest power of 2 that will hold everything
  obj_index.reserve(8192);

  /****** extra ******/
  TDatabase extra_db(DB_SNEEZY);
  extra_db.query("select vnum, name, description from objextra order by vnum");
  extra_db.fetchRow();

  /****** affect ******/
  TDatabase affect_db(DB_SNEEZY);
  affect_db.query("select vnum, type, mod1, mod2 from objaffect order by vnum");
  affect_db.fetchRow();

  /********************/

  TDatabase db(DB_SNEEZY);
  db.query("select vnum, name, short_desc, long_desc, max_exist, spec_proc, weight, max_struct, wear_flag, type, price, action_desc from obj order by vnum");

  while(db.fetchRow()){
    tmpi = new objIndexData();
    if (!tmpi) {
      perror("indexData");
      exit(0);
    }
    
    tmpi->virt=convertTo<int>(db["vnum"]);
    tmpi->name=mud_str_dup(db["name"]);
    tmpi->short_desc=mud_str_dup(db["short_desc"]);
    tmpi->long_desc=mud_str_dup(db["long_desc"]);
    tmpi->max_exist=convertTo<int>(db["max_exist"]);

    // use 327 so we don't go over 32765 in calculation
    if (tmpi->max_exist < 327) {
      tmpi->max_exist *= (short) (stats.max_exist * 100);
      tmpi->max_exist /= 100;
    }
    if (tmpi->max_exist)
      tmpi->max_exist = max(tmpi->max_exist, (short int) (gamePort == Config::Port::BETA ? 9999 : 1));
    

    tmpi->spec=convertTo<int>(db["spec_proc"]);
    tmpi->weight=convertTo<float>(db["weight"]);
    tmpi->max_struct=convertTo<int>(db["max_struct"]);
    tmpi->where_worn=convertTo<int>(db["wear_flag"]);
    tmpi->itemtype=convertTo<int>(db["type"]);
    tmpi->value=convertTo<int>(db["price"]);
    if(!db["action_desc"].empty())
      tmpi->description=mud_str_dup(db["action_desc"]);
    else tmpi->description=NULL;

    while(!extra_db["vnum"].empty() && convertTo<int>(extra_db["vnum"]) < tmpi->virt){
      extra_db.fetchRow();
    }

    while(!extra_db["vnum"].empty() &&
	  convertTo<int>(extra_db["vnum"])==tmpi->virt){
      new_descr = new extraDescription();
      new_descr->keyword = mud_str_dup(extra_db["name"]);
      new_descr->description = mud_str_dup(extra_db["description"]);
      new_descr->next = tmpi->ex_description;
      tmpi->ex_description = new_descr;

      extra_db.fetchRow();
    }

    while(!affect_db["vnum"].empty() &&
	  convertTo<int>(affect_db["vnum"]) < tmpi->virt){
      affect_db.fetchRow();
    }

    i=0;
    while(!affect_db["vnum"].empty() &&
	  convertTo<int>(affect_db["vnum"])==tmpi->virt){
      tmpi->affected[i].location = mapFileToApply(convertTo<int>(affect_db["type"]));

      if (tmpi->affected[i].location == APPLY_SPELL)
	tmpi->affected[i].modifier = mapFileToSpellnum(convertTo<int>(affect_db["mod1"]));
      else
	tmpi->affected[i].modifier = convertTo<int>(affect_db["mod1"]);
      
      tmpi->affected[i].modifier2 = convertTo<int>(affect_db["mod2"]);
      tmpi->affected[i].type = TYPE_UNDEFINED;
      tmpi->affected[i].level = 0;
      tmpi->affected[i].bitvector = 0;      

      affect_db.fetchRow();
      i++;
    }

    obj_index.push_back(*tmpi);
    delete tmpi;
  }

  return;
}



// generate index table for monster file 
void generate_mob_index()
{
  mobIndexData *tmpi = NULL;
  TDatabase db(DB_SNEEZY);
  
  // to prevent constant resizing (slows boot), declare an appropriate initial
  // size.  Should be smallest power of 2 that will hold everything
  mob_index.reserve(8192);

  // start by reading
  db.query("select * from mob");

  while(db.fetchRow()){
    if (tmpi) {
      // push the previous one into the stack
      mob_index.push_back(*tmpi);
      delete tmpi;
    }

    // start a new data member
    tmpi = new mobIndexData();
    if (!tmpi) {
      perror("mobIndexData");
      exit(0);
    }
    
    tmpi->virt = convertTo<int>(db["vnum"]);
    
    // read the sstrings
    tmpi->name = mud_str_dup(db["name"]);
    tmpi->short_desc = mud_str_dup(db["short_desc"]);
    tmpi->long_desc = mud_str_dup(db["long_desc"]);
    tmpi->description = mud_str_dup(db["description"]);
    
    long fac=convertTo<int>(db["faction"]);
    
    tmpi->faction = fac;
    
    long Class=convertTo<int>(db["class"]);
    long lev=convertTo<int>(db["level"]);
    float arm=convertTo<int>(db["ac"]);
    float hp=convertTo<int>(db["hpbonus"]);
    float daml=convertTo<int>(db["damage_level"]);
    
    lev = (long)((arm + hp + daml) / 3);
    
    tmpi->Class = Class;
    tmpi->level = lev;
    
    long race=convertTo<int>(db["race"]);
    long wgt=convertTo<int>(db["weight"]);
    
    tmpi->race = race;
    tmpi->weight = wgt;
    
    long spec=convertTo<int>(db["spec_proc"]);
    
    tmpi->spec = spec;
        
    long maxe=convertTo<int>(db["max_exist"]);
    
    tmpi->max_exist = (gamePort == Config::Port::BETA ? 9999 : maxe);
    
    // handle some stat counters
    if (lev <= 5) {
      stats.mobs_1_5++;
    } else if (lev <= 10) {
      stats.mobs_6_10++;
    } else if (lev <= 15) {
      stats.mobs_11_15++;
    } else if (lev <= 20) {
      stats.mobs_16_20++;
    } else if (lev <= 25) {
      stats.mobs_21_25++;
    } else if (lev <= 30) {
      stats.mobs_26_30++;
    } else if (lev <= 40) {
      stats.mobs_31_40++;
    } else if (lev <= 50) {
      stats.mobs_41_50++;
    } else if (lev <= 60) {
      stats.mobs_51_60++;
    } else if (lev <= 70) {
      stats.mobs_61_70++;
    } else if (lev <= 100) {
      stats.mobs_71_100++;
    } else {
      stats.mobs_101_127++;
    }
    // end stat counters
  }
  // and push the last one into the stack
  mob_index.push_back(*tmpi);
  delete tmpi;
  

  return;
}

