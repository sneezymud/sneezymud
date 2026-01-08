#pragma once

#include "low.h"
#include "obj.h"
#include "task.h"
#include "spell2.h"
#include "materials.h"
#include "race.h"


const int TYPE_VNUM = 0;
const int TYPE_LIQUID = 1;
const int TYPE_MATERIAL = 2;
const int TYPE_CORPSE = 3;
const int TYPE_ITEM = 4;

class ingredientTypeT {
  public:
    int recipe, ingredient, amt, type, num;
};

class recipeTypeT {
  public:
    int recipe;
    const sstring keywords, name;
    int vnum;
    taskDiffT difficulty;
};

// recipe number, keywords, name, vnum, difficulty
recipeTypeT recipes[] = {
    // Basic Food Items (Simple preparation)
    {0, "steak marinated", "marinated steak", 405, TASK_EASY},
    {1, "catfish fried", "fried catfish", 14358, TASK_EASY},
    {2, "rat stick", "rat on a stick", 14352, TASK_TRIVIAL},
    {3, "potatoes mashed", "side of mashed potatoes", 31773, TASK_TRIVIAL},
    {4, "salad side", "small side salad", 31774, TASK_TRIVIAL},

    // Complete Meals (More complex preparation)
    {5, "steak dinner", "steak dinner", 31775, TASK_NORMAL},
    {6, "chicken fried", "fried chicken", 3324, TASK_EASY},
    {7, "pita sandwich", "pita sandwich", 34700, TASK_EASY},

    // Baked goods
    {8, "offal pot pie", "offal pot pie", 34701, TASK_NORMAL},
    {9, "beggar stew", "beggar stew", 34704, TASK_NORMAL},
    {10, "berry friendship bread", "berry friendship bread", 34705, TASK_NORMAL},
    {11, "pancake", "pancake", 34708, TASK_EASY},
    {12, "berry pancake", "berry pancake", 34709, TASK_NORMAL},

    // Poisons (Dangerous ingredients, precise measurements)
    {13, "poison death camas", "vial death camas", 31008, TASK_DANGEROUS},
    {14, "poison destroying angel", "vial destroying angel", 31009, TASK_DANGEROUS},
    {15, "poison jimson weed", "vial jimson weed", 31010, TASK_DANGEROUS},
    {16, "poison hemlock", "vial hemlock", 31011, TASK_DANGEROUS},
    {17, "poison monkshood", "vial monkshood", 31012, TASK_DANGEROUS},
    {18, "poison glow fish", "vial glow fish", 31013, TASK_DANGEROUS},
    {19, "poison scorpion", "vial scorpion", 31014, TASK_DANGEROUS},
    {20, "poison violet fungus", "vial violet fungus", 31015, TASK_DANGEROUS},
    {21, "poison devil ice", "vial devil ice", 31016, TASK_DANGEROUS},
    {22, "poison firedrake", "vial firedrake", 31017, TASK_DANGEROUS},
    {23, "poison infant", "vial infant", 31018, TASK_DANGEROUS},
    {24, "poison pea seed", "vial pea seed", 31019, TASK_DANGEROUS},
    {25, "poison acacia", "vial acacia", 31020, TASK_DANGEROUS},

    // Water and holy water
    {26, "water skin regular", "regular water skin", 410, TASK_TRIVIAL},
    {27, "water skin large", "large water skin", 411, TASK_EASY},
    {28, "holy water tiny vial" , "tiny vial of holy water", 163, TASK_TRIVIAL},
    {29, "holy water small vial" , "small vial of holy water", 164, TASK_EASY},
    {30, "holy water large vial" , "large vial of holy water", 165, TASK_NORMAL},

    {-1, "", "", -1, TASK_TRIVIAL}  // List terminator
};
// recipe number, amount of item, type of item, item vnum
ingredientTypeT ingredients[] = {
  // marinated steak
  {0, 1, 1, TYPE_VNUM, Obj::GENERIC_STEAK},  // any steak will do
  {0, 2, 3, TYPE_LIQUID, LIQ_WHISKY},        // 3 ounces of whiskey
  {0, 3, 1, TYPE_VNUM, 432},                 // 1 orange

  // fried catfish
  {1, 1, 1, TYPE_VNUM, 13803},  // 1 catfish
  {1, 2, 1, TYPE_VNUM, 263},    // jar of whale grease

  // rat on a stick
  {2, 1, 1, TYPE_CORPSE, RACE_RODENT},  // 1 rat
  {2, 2, 1, TYPE_ITEM, ITEM_ARROW},     // 1 arrow

  // mashed potatoes
  {3, 1, 1, TYPE_VNUM, 31766},  // 1 potato
  {3, 2, 1, TYPE_VNUM, 31767},  // butter

  // side salad
  {4, 1, 1, TYPE_VNUM, 10037},  // lettuce
  {4, 1, 1, TYPE_VNUM, 14349}, {4, 1, 1, TYPE_VNUM, 28947},
  {4, 1, 1, TYPE_VNUM, 31768}, {4, 2, 1, TYPE_VNUM, 14348},  // tomato
  {4, 3, 1, TYPE_VNUM, 31769},                               // dressing
  {4, 4, 1, TYPE_VNUM, 31770},                               // onion
  {4, 5, 1, TYPE_VNUM, 31771},                               // carrot
  {4, 6, 1, TYPE_VNUM, 31772},                               // croutons

  // steak dinner
  {5, 1, 1, TYPE_VNUM, 405},    // marinated steak, recipe 0
  {5, 2, 1, TYPE_VNUM, 31773},  // side of mashed potatoes, recipe 3
  {5, 3, 1, TYPE_VNUM, 31774},  // side salad, recipe 4

  // fried chicken
  {6, 1, 1, TYPE_CORPSE, RACE_BIRD},  // preferably chicken, heh
  {6, 2, 1, TYPE_VNUM, 263},          // jar of whale grease

  // pita sandwich
  {7, 1, 1, TYPE_VNUM, 25550},  // pita
  {7, 2, 1, TYPE_VNUM, 10037},  // lettuce
  {7, 2, 1, TYPE_VNUM, 14349}, {7, 2, 1, TYPE_VNUM, 28947},
  {7, 2, 1, TYPE_VNUM, 31768}, {7, 3, 1, TYPE_VNUM, 14348},  // tomato

  // offal pot pie
  {8, 1, 1, TYPE_VNUM, 10030},        // offal
  {8, 2, 1, TYPE_VNUM, 256},          // gnome flour
  {8, 3, 3, TYPE_LIQUID, LIQ_WATER},  // water
  {8, 4, 1, TYPE_VNUM, 34703},        // parsley

  // beggar stew
  {9, 1, 1, TYPE_VNUM, 10030},         // offal
  {9, 2, 1, TYPE_VNUM, 10913},         // rock
  {9, 3, 10, TYPE_LIQUID, LIQ_WATER},  // water

  // friendship bread
  {10, 1, 1, TYPE_VNUM, 256},  // gnome flour
  {10, 2, 1, TYPE_VNUM, 276},  // berries
  {10, 2, 1, TYPE_VNUM, 5701}, {10, 2, 1, TYPE_VNUM, 10900},
  {10, 2, 1, TYPE_VNUM, 10907}, {10, 2, 1, TYPE_VNUM, 10911},
  {10, 2, 1, TYPE_VNUM, 24703}, {10, 3, 3, TYPE_LIQUID, LIQ_WATER},
  {10, 4, 1, TYPE_VNUM, 34706},  // sugar
  {10, 5, 1, TYPE_VNUM, 34707},  // yeast

  // pancake
  {11, 1, 1, TYPE_VNUM, 34706},  // sugar
  {11, 2, 1, TYPE_VNUM, 256},    // gnome flour
  {11, 3, 2, TYPE_LIQUID, LIQ_WATER},

  // berry pancake
  {12, 1, 1, TYPE_VNUM, 34706},  // sugar
  {12, 2, 1, TYPE_VNUM, 256},    // gnome flour
  {12, 3, 2, TYPE_LIQUID, LIQ_WATER}, {12, 4, 1, TYPE_VNUM, 276},  // berries
  {12, 4, 1, TYPE_VNUM, 5701}, {12, 4, 1, TYPE_VNUM, 10900},
  {12, 4, 1, TYPE_VNUM, 10907}, {12, 4, 1, TYPE_VNUM, 10911},
  {12, 4, 1, TYPE_VNUM, 24703},

  // death camas
  {13, 1, 1, TYPE_VNUM, 31032},  // death camas flower
  {13, 2, 1, TYPE_VNUM, 4306},  // swamproot
  {13, 3, 1, TYPE_VNUM, 13849}, // can of leeches
  {13, 4, 5, TYPE_LIQUID, LIQ_FIREBRT},  // firebreath

  // destroying angel
  {14, 1, 1, TYPE_VNUM, 31033},  // destroying angel cap
  {14, 2, 2, TYPE_LIQUID, LIQ_BLOOD},  // blood
  {14, 3, 1, TYPE_VNUM, 37135}, // cocoon
  {14, 4, 5, TYPE_LIQUID, LIQ_FIREBRT},  // firebreath

  // jimson weed
  {15, 1, 1, TYPE_VNUM, 31034},  // jimson weed
  {15, 2, 1, TYPE_VNUM, 13849}, // can of leeches
  {15, 3, 1, TYPE_VNUM, 34706}, // sugar
  {15, 4, 5, TYPE_LIQUID, LIQ_VODKA},  // grain alcohol

  // hemlock
  {16, 1, 1, TYPE_VNUM, 31035},  // hemlock
  {16, 2, 1, TYPE_VNUM, 13849}, // can of leeches
  {16, 3, 1, TYPE_VNUM, 37139}, // honeysap
  {16, 4, 5, TYPE_LIQUID, LIQ_VODKA},  // grain alcohol

  // monkshood
  {17, 1, 1, TYPE_VNUM, 31036},  // monkshood
  {17, 2, 1, TYPE_VNUM, 278}, // small egg
  {17, 3, 1, TYPE_VNUM, 37140}, // jojupus
  {17, 4, 5, TYPE_LIQUID, LIQ_VODKA},  // grain alcohol

  // glow fish
  {18, 1, 1, TYPE_VNUM, 31042},  // glow fish organ
  {18, 2, 1, TYPE_VNUM, 23654}, // pixie parts
  {18, 3, 1, TYPE_VNUM, 37141}, // ginseng
  {18, 4, 5, TYPE_LIQUID, LIQ_VODKA},  // grain alcohol

  // scorpion
  {19, 1, 1, TYPE_VNUM, 31043},  // scorpion
  {19, 2, 1, TYPE_VNUM, 89}, // rat tail
  {19, 3, 1, TYPE_VNUM, 431}, // donut pastry
  {19, 3, 1, TYPE_VNUM, 26896}, // donut glazed
  {19, 4, 5, TYPE_LIQUID, LIQ_VODKA},  // grain alcohol

  // violet fungus
  {20, 1, 1, TYPE_VNUM, 31037},  // violet fungus
  {20, 2, 1, TYPE_VNUM, 27237}, // moldy book
  {20, 3, 5, TYPE_LIQUID, LIQ_MILK}, // milk
  {20, 4, 1, TYPE_VNUM, 7510},  // ancient wine
  {20, 4, 1, TYPE_VNUM, 7511},  // ancient wine

  // devil ice
  {21, 1, 1, TYPE_VNUM, 31038},  // devil ice
  {21, 2, 1, TYPE_VNUM, 10030}, // offal
  {21, 3, 1, TYPE_VNUM, 37143}, // snow locust
  {21, 4, 5, TYPE_LIQUID, LIQ_URINE},  // urine

  // firedrake
  {22, 1, 1, TYPE_VNUM, 31039},  // firedrake
  {22, 2, 1, TYPE_VNUM, 10030}, // offal
  {22, 3, 1, TYPE_VNUM, 37144}, // fireweed
  {22, 4, 5, TYPE_LIQUID, LIQ_URINE},  // urine

  // infant
  {23, 1, 1, TYPE_VNUM, 31040},  // infant
  {23, 2, 1, TYPE_VNUM, 10030}, // offal
  {23, 3, 1, TYPE_VNUM, 37145}, // ginseng
  {23, 4, 5, TYPE_LIQUID, LIQ_URINE},  // urine

  // pea seed
  {24, 1, 1, TYPE_VNUM, 31041},  // pea seed
  {24, 2, 1, TYPE_VNUM, 27237}, // moldy book
  {24, 3, 5, TYPE_LIQUID, LIQ_MILK}, // milk
  {24, 4, 1, TYPE_VNUM, 7510},  // ancient wine
  {24, 4, 1, TYPE_VNUM, 7511},  // ancient wine

  // acacia
  {25, 1, 1, TYPE_VNUM, 31044},  // acacia
  {25, 2, 1, TYPE_VNUM, 27237}, // moldy book
  {25, 3, 5, TYPE_LIQUID, LIQ_MILK}, // milk
  {25, 4, 1, TYPE_VNUM, 7510},  // ancient wine
  {25, 4, 1, TYPE_VNUM, 7511},  // ancient wine

  // regular waterskin
  {26, 1, 1, TYPE_VNUM, 410},  // waterskin
  {26, 2, 70, TYPE_LIQUID, LIQ_URINE}, // pee
  {26, 2, 70, TYPE_LIQUID, LIQ_HOLYWATER}, // holy water
  {26, 2, 70, TYPE_LIQUID, LIQ_SALTWATER}, // salt water
  {26, 2, 70, TYPE_LIQUID, LIQ_BEER}, // beer

  // large waterskin
  {27, 1, 1, TYPE_VNUM, 411},  // large waterskin
  {27, 2, 150, TYPE_LIQUID, LIQ_URINE}, // pee
  {27, 2, 150, TYPE_LIQUID, LIQ_HOLYWATER}, // holy water
  {27, 2, 150, TYPE_LIQUID, LIQ_SALTWATER}, // salt water
  {27, 2, 150, TYPE_LIQUID, LIQ_BEER}, // beer

  // tiny vial of holy water
  {28, 1, 1, TYPE_VNUM, 163},  // tiny vial
  {28, 2, 10, TYPE_LIQUID, LIQ_WATER}, // water
  {28, 2, 10, TYPE_LIQUID, LIQ_URINE}, // pee
  {28, 2, 10, TYPE_LIQUID, LIQ_BEER}, // beer
  {28, 3, 1, TYPE_VNUM, 500}, // holy symbol - wood
  {28, 3, 1, TYPE_VNUM, 501}, // holy symbol - porcelain
  {28, 3, 1, TYPE_VNUM, 502}, // holy symbol - tin
  {28, 3, 1, TYPE_VNUM, 503}, // holy symbol - aluminum
  {28, 3, 1, TYPE_VNUM, 504}, // holy symbol - copper
  {28, 3, 1, TYPE_VNUM, 505}, // holy symbol - brass

  // small vial of holy water
  {29, 1, 1, TYPE_VNUM, 164},  // small vial
  {29, 2, 25, TYPE_LIQUID, LIQ_WATER}, // water
  {29, 2, 25, TYPE_LIQUID, LIQ_URINE}, // pee
  {29, 2, 25, TYPE_LIQUID, LIQ_BEER}, // beer
  {29, 3, 1, TYPE_VNUM, 506}, // holy symbol - iron
  {29, 3, 1, TYPE_VNUM, 507}, // holy symbol - bronze
  {29, 3, 1, TYPE_VNUM, 508}, // holy symbol - metal
  {29, 3, 1, TYPE_VNUM, 509}, // holy symbol - steel
  {29, 3, 1, TYPE_VNUM, 510}, // holy symbol - silver

  // large vial of holy water
  {30, 1, 1, TYPE_VNUM, 165},  // large vial
  {30, 2, 50, TYPE_LIQUID, LIQ_WATER}, // water
  {30, 2, 50, TYPE_LIQUID, LIQ_URINE}, // pee
  {30, 2, 50, TYPE_LIQUID, LIQ_BEER}, // beer
  {30, 3, 1, TYPE_VNUM, 511}, // holy symbol - gold
  {30, 3, 1, TYPE_VNUM, 512}, // holy symbol - platinum
  {30, 3, 1, TYPE_VNUM, 513}, // holy symbol - titanium
  {30, 3, 1, TYPE_VNUM, 514}, // holy symbol - mithril


  {-1, -1, -1, -1, -1}};


