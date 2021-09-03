//////////////////////////////////////////////////////////////////////////
//
// SneezyMUD - All rights reserved, SneezyMUD Coding Team
//
//////////////////////////////////////////////////////////////////////////


#include <stdio.h>

#include "extern.h"
#include "configuration.h"
#include "statistics.h"
#include "database.h"

extern FILE *obj_f;
extern FILE *mob_f;

std::vector<mobIndexData> mob_index;
std::vector<objIndexData> obj_index;

indexData::indexData()
    : virt(0),
      pos(0),
      number(0),
      name(nullptr),
      short_desc(nullptr),
      long_desc(nullptr),
      description(nullptr),
      max_exist(-99),
      spec(0),
      weight(0) {}

indexData::indexData(const indexData &a) :
  virt(a.virt),
  pos(a.pos),
  number(a.number),
  name(a.name),
  short_desc(a.short_desc),
  long_desc(a.long_desc),
  description(a.description),
  max_exist(a.max_exist),
  spec(a.spec),
  weight(a.weight) {}

mobIndexData::mobIndexData()
    : faction(-99),
      Class(-99),
      level(-99),
      race(-99),
      doesLoad(false),
      numberLoad(0) {}

mobIndexData::mobIndexData(int _virt, const sstring &_name, const sstring &_short_desc,
                           const sstring &_long_desc, const sstring &_description, int _max_exist,
                           int _spec, float _weight, long faction, long Class, long level,
                           long race)
    : faction(faction), Class(Class), level(level), race(race) {
  virt = _virt;
  name = _name;
  short_desc = _short_desc;
  long_desc = _long_desc;
  description = _description;
  max_exist = _max_exist;
  spec = _spec;
  weight = _weight;
}

mobIndexData::mobIndexData(const mobIndexData &a)
    : indexData(a),
      faction(a.faction),
      Class(a.Class),
      level(a.level),
      race(a.race),
      doesLoad(a.doesLoad),
      numberLoad(a.numberLoad) {}

objIndexData::objIndexData()
    : max_struct(-99),
      armor(-99),
      where_worn(0),
      itemtype(MAX_OBJ_TYPES),
      value(-99),
      ex_description(nullptr) {}

objIndexData::objIndexData(int _virt, sstring _name, sstring _short_desc, sstring _long_desc,
                           int _max_exist, int _spec, float _weight, byte max_struct,
                           unsigned int where_worn, ubyte itemtype, int value, sstring _description,
                           extraDescription *ex_description, objAffData _affected[MAX_OBJ_AFFECT])
    : max_struct(max_struct), where_worn(where_worn), itemtype(itemtype), value(value), ex_description(ex_description) {
  virt = _virt;
  name = _name;
  short_desc = _short_desc;
  long_desc = _long_desc;
  max_exist = _max_exist;
  spec = _spec;
  weight = _weight;
  description = _description;
  
  for (int i = 0; i < MAX_OBJ_AFFECT; i++)
    affected[i] = _affected[i];
}

objIndexData::objIndexData(const objIndexData &a)
    : indexData(a),
      max_struct(a.max_struct),
      armor(a.armor),
      where_worn(a.where_worn),
      itemtype(a.itemtype),
      value(a.value) {
  // use copy operator;
  if (a.ex_description)
    ex_description = new extraDescription(*a.ex_description);
  else
    ex_description = NULL;

  int i;
  for (i = 0; i < MAX_OBJ_AFFECT; ++i) {
    affected[i] = a.affected[i];
  }
}

objIndexData::~objIndexData() {
  extraDescription *tmp;
  while ((tmp = ex_description)) {
    ex_description = tmp->next;
    delete tmp;
  }
}

// generate index table for object
void generate_obj_index() {
  // to prevent constant resizing (slows boot), declare an appropriate initial
  // size.  Should be smallest power of 2 that will hold everything
  
  // 2021-08-26: This cache is currently ending at 9212 entries, so updating this 
  // value from 8192 to 16,384
  obj_index.reserve(16384);

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
  db.query(
      "select vnum, name, short_desc, long_desc, max_exist, spec_proc, weight, max_struct, wear_flag, type, price, action_desc from obj order by vnum");

  while (db.fetchRow()) {
    int virt = convertTo<int>(db["vnum"]);
    sstring name = db["name"];
    sstring short_desc = db["short_desc"];
    sstring long_desc = db["long_desc"];

    int max_exist = convertTo<int>(db["max_exist"]);
    max_exist = static_cast<int>(max_exist * stats.max_exist);
    max_exist = min(max(max_exist, 1), 9999);

    int spec = convertTo<int>(db["spec_proc"]);
    float weight = convertTo<float>(db["weight"]);
    byte max_struct = convertTo<int>(db["max_struct"]);
    unsigned int where_worn = convertTo<int>(db["wear_flag"]);
    ubyte itemtype = convertTo<int>(db["type"]);
    int value = convertTo<int>(db["price"]);
    sstring description = db["action_desc"];

    extraDescription *ex_description = nullptr;
    while (!extra_db["vnum"].empty() && convertTo<int>(extra_db["vnum"]) <= virt) {
      if (convertTo<int>(extra_db["vnum"]) == virt) {
        auto new_descr = new extraDescription();
        new_descr->keyword = extra_db["name"];
        new_descr->description = extra_db["description"];
        new_descr->next = ex_description;
        ex_description = new_descr;
      }
      extra_db.fetchRow();
    }

    objAffData affected[MAX_OBJ_AFFECT];
    int i = 0;
    while (!affect_db["vnum"].empty() && convertTo<int>(affect_db["vnum"]) <= virt) {
      if (convertTo<int>(affect_db["vnum"]) == virt && i < MAX_OBJ_AFFECT) {
        affected[i].location = mapFileToApply(convertTo<int>(affect_db["type"]));

        if (affected[i].location == APPLY_SPELL)
          affected[i].modifier = mapFileToSpellnum(convertTo<int>(affect_db["mod1"]));
        else
          affected[i].modifier = convertTo<int>(affect_db["mod1"]);

        affected[i].modifier2 = convertTo<int>(affect_db["mod2"]);
        affected[i].type = TYPE_UNDEFINED;
        affected[i].level = 0;
        affected[i].bitvector = 0;

        i++;
      }
      affect_db.fetchRow();
    }

    obj_index.emplace_back(virt, name, short_desc, long_desc, max_exist, spec, weight, max_struct,
                           where_worn, itemtype, value, description, ex_description, affected);
  }
}

// generate index table for monster file
void generate_mob_index() {
  TDatabase db(DB_SNEEZY);

  // to prevent constant resizing (slows boot), declare an appropriate initial
  // size.  Should be smallest power of 2 that will hold everything
  mob_index.reserve(8192);

  db.query("select * from mob");

  while (db.fetchRow()) {
    int virt = convertTo<int>(db["vnum"]);
    sstring name = db["name"];
    sstring short_desc = db["short_desc"];
    sstring long_desc = db["long_desc"];
    sstring description = db["description"];
    long faction = convertTo<int>(db["faction"]);
    long Class = convertTo<int>(db["class"]);
    float arm = convertTo<int>(db["ac"]);
    float hp = convertTo<int>(db["hpbonus"]);
    float daml = convertTo<int>(db["damage_level"]);
    long level = static_cast<long>((arm + hp + daml) / 3);
    long race = convertTo<int>(db["race"]);
    float weight = convertTo<float>(db["weight"]);
    int spec = convertTo<int>(db["spec_proc"]);
    int max_exist = convertTo<int>(db["max_exist"]);

    if (level <= 5)
      stats.mobs_1_5++;
    else if (level <= 10)
      stats.mobs_6_10++;
    else if (level <= 15)
      stats.mobs_11_15++;
    else if (level <= 20)
      stats.mobs_16_20++;
    else if (level <= 25)
      stats.mobs_21_25++;
    else if (level <= 30)
      stats.mobs_26_30++;
    else if (level <= 40)
      stats.mobs_31_40++;
    else if (level <= 50)
      stats.mobs_41_50++;
    else if (level <= 60)
      stats.mobs_51_60++;
    else if (level <= 70)
      stats.mobs_61_70++;
    else if (level <= 100)
      stats.mobs_71_100++;
    else
      stats.mobs_101_127++;

    mob_index.emplace_back(virt, name, short_desc, long_desc, description, max_exist, spec, weight,
                           faction, Class, level, race);
  }
}
