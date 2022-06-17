//////////////////////////////////////////////////////////////////////////
//
// SneezyMUD - All rights reserved, SneezyMUD Coding Team
//
//////////////////////////////////////////////////////////////////////////


#include <stdio.h>

#include "db.h"
#include "extern.h"
#include "configuration.h"
#include "statistics.h"
#include "database.h"

extern FILE *obj_f;
extern FILE *mob_f;

std::vector<mobIndexData> mob_index;
std::vector<objIndexData> obj_index;

objIndexData::objIndexData(const objIndexData& a) :
  indexData(a),
  max_struct(a.max_struct),
  armor(a.armor),
  where_worn(a.where_worn),
  itemtype(a.itemtype),
  value(a.value) {
  if (a.ex_description)
    ex_description = new extraDescription(*a.ex_description);
  else
    ex_description = nullptr;

  for (int i = 0; i < MAX_OBJ_AFFECT; ++i) {
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
    objIndexData o;
    o.virt = convertTo<int>(db["vnum"]);
    o.name = db["name"];
    o.short_desc = db["short_desc"];
    o.long_desc = db["long_desc"];

    o.max_exist = convertTo<int>(db["max_exist"]);
    o.max_exist = static_cast<int>(o.max_exist * stats.max_exist);
    o.max_exist = min(max(o.max_exist, 1), 9999);

    o.spec = convertTo<int>(db["spec_proc"]);
    o.weight = convertTo<float>(db["weight"]);
    o.max_struct = convertTo<int>(db["max_struct"]);
    o.where_worn = convertTo<int>(db["wear_flag"]);
    o.itemtype = convertTo<int>(db["type"]);
    o.value = convertTo<int>(db["price"]);
    o.description = db["action_desc"];

    while (!extra_db["vnum"].empty() && convertTo<int>(extra_db["vnum"]) <= o.virt) {
      if (convertTo<int>(extra_db["vnum"]) == o.virt) {
        auto *new_descr = new extraDescription();
        new_descr->keyword = extra_db["name"];
        new_descr->description = extra_db["description"];
        new_descr->next = o.ex_description;
        o.ex_description = new_descr;
      }
      extra_db.fetchRow();
    }

    int i = 0;
    while (!affect_db["vnum"].empty() && convertTo<int>(affect_db["vnum"]) <= o.virt) {
      if (convertTo<int>(affect_db["vnum"]) == o.virt && i < MAX_OBJ_AFFECT) {
        objAffData& aff = o.affected[i];
        aff.location = mapFileToApply(convertTo<int>(affect_db["type"]));

        if (aff.location == APPLY_SPELL)
          aff.modifier = mapFileToSpellnum(convertTo<int>(affect_db["mod1"]));
        else
          aff.modifier = convertTo<int>(affect_db["mod1"]);

        aff.modifier2 = convertTo<int>(affect_db["mod2"]);
        aff.type = TYPE_UNDEFINED;
        aff.level = 0;
        aff.bitvector = 0;

        i++;
      }
      affect_db.fetchRow();
    }

    obj_index.emplace_back(o);
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
    mobIndexData m;
    m.virt = convertTo<int>(db["vnum"]);
    m.name = db["name"];
    m.short_desc = db["short_desc"];
    m.long_desc = db["long_desc"];
    m.description = db["description"];
    m.faction = convertTo<int>(db["faction"]);
    m.Class = convertTo<int>(db["class"]);
    float arm = convertTo<int>(db["ac"]);
    float hp = convertTo<int>(db["hpbonus"]);
    float daml = convertTo<int>(db["damage_level"]);
    m.level = static_cast<long>((arm + hp + daml) / 3);
    m.race = convertTo<int>(db["race"]);
    m.weight = convertTo<float>(db["weight"]);
    m.spec = convertTo<int>(db["spec_proc"]);
    m.max_exist = convertTo<int>(db["max_exist"]);

    if (m.level <= 5)
      stats.mobs_1_5++;
    else if (m.level <= 10)
      stats.mobs_6_10++;
    else if (m.level <= 15)
      stats.mobs_11_15++;
    else if (m.level <= 20)
      stats.mobs_16_20++;
    else if (m.level <= 25)
      stats.mobs_21_25++;
    else if (m.level <= 30)
      stats.mobs_26_30++;
    else if (m.level <= 40)
      stats.mobs_31_40++;
    else if (m.level <= 50)
      stats.mobs_41_50++;
    else if (m.level <= 60)
      stats.mobs_51_60++;
    else if (m.level <= 70)
      stats.mobs_61_70++;
    else if (m.level <= 100)
      stats.mobs_71_100++;
    else
      stats.mobs_101_127++;

    mob_index.emplace_back(m);
  }
}
