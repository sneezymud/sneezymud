#ifndef __TASK_COOK_H
#define __TASK_COOK_H

const int TYPE_VNUM=0;
const int TYPE_LIQUID=1;
const int TYPE_MATERIAL=2;
const int TYPE_CORPSE=3;
const int TYPE_ITEM=4;

const int BOGUS_PLACEHOLDER = 10913;

class ingredientTypeT {
public:
  int recipe, ingredient, amt, type, num;
};

class recipeTypeT {
public:
  int recipe;
  const sstring keywords, name;
  int vnum;
};

// recipe number, keywords, name
recipeTypeT recipes[] =
{
  {0, "steak marinated", "marinated steak", 405},
  {1, "catfish fried", "fried catfish", 14358},
  {2, "rat stick", "rat on a stick", 14352},
  {3, "potatoes mashed", "side of mashed potatoes", 31773},
  {4, "salad side", "small side salad", 31774},
  {5, "steak dinner", "steak dinner", 31775},
  {6, "chicken fried", "fried chicken", 3324},
  {7, "pita sandwich", "pita sandwich", 34700},
  {8, "offal pot pie", "offal pot pie", 34701},
  {9, "beggar stew", "beggar stew", 34704},
  {10,"berry friendship bread", "berry friendship bread", 34705},
  {11,"pancake", "pancake", 34708},
  {12,"berry pancake", "berry pancake", 34709},
  {-1, NULL, NULL, -1}
};


// recipe number, amount of item, type of item, item vnum
ingredientTypeT ingredients[] =
{ 
  // marinated steak
  {0, 1, 1, TYPE_VNUM, GENERIC_STEAK},  // any steak will do
  {0, 2, 3, TYPE_LIQUID, LIQ_WHISKY}, // 3 ounces of whiskey
  {0, 3, 1, TYPE_VNUM, 432}, // 1 orange
  
  // fried catfish
  {1, 1, 1, TYPE_VNUM, 13803}, // 1 catfish
  {1, 2, 1, TYPE_VNUM, 263}, // jar of whale grease

  // rat on a stick
  {2, 1, 1, TYPE_CORPSE, RACE_RODENT}, // 1 rat
  {2, 2, 1, TYPE_ITEM, ITEM_ARROW}, // 1 arrow

  // mashed potatoes
  {3, 1, 1, TYPE_VNUM, 31766}, // 1 potato
  {3, 2, 1, TYPE_VNUM, 31767}, // butter

  // side salad
  {4, 1, 1, TYPE_VNUM, 10037}, // lettuce
  {4, 1, 1, TYPE_VNUM, 14349},
  {4, 1, 1, TYPE_VNUM, 28947},
  {4, 1, 1, TYPE_VNUM, 31768},
  {4, 2, 1, TYPE_VNUM, 14348}, // tomato
  {4, 3, 1, TYPE_VNUM, 31769}, // dressing
  {4, 4, 1, TYPE_VNUM, 31770}, // onion
  {4, 5, 1, TYPE_VNUM, 31771}, // carrot
  {4, 6, 1, TYPE_VNUM, 31772}, // croutons

  // steak dinner
  {5, 1, 1, TYPE_VNUM, 405}, // marinated steak, recipe 0
  {5, 2, 1, TYPE_VNUM, 31773}, // side of mashed potatoes, recipe 3
  {5, 3, 1, TYPE_VNUM, 31774}, // side salad, recipe 4

  // fried chicken
  {6, 1, 1, TYPE_CORPSE, RACE_BIRD}, // preferably chicken, heh
  {6, 2, 1, TYPE_VNUM, 263}, // jar of whale grease

  // pita sandwich
  {7, 1, 1, TYPE_VNUM, 25550}, // pita
  {7, 2, 1, TYPE_VNUM, 10037}, // lettuce
  {7, 2, 1, TYPE_VNUM, 14349},
  {7, 2, 1, TYPE_VNUM, 28947},
  {7, 2, 1, TYPE_VNUM, 31768},
  {7, 3, 1, TYPE_VNUM, 14348}, // tomato

  // offal pot pie
  {8, 1, 1, TYPE_VNUM, 10030}, // offal
  {8, 2, 1, TYPE_VNUM, 256}, // gnome flour
  {8, 3, 3, TYPE_LIQUID, LIQ_WATER}, // water
  {8, 4, 1, TYPE_VNUM, 34703}, // parsley

  // beggar stew
  {9, 1, 1, TYPE_VNUM, 10030}, // offal
  {9, 2, 1, TYPE_VNUM, 10913}, // rock
  {9, 3, 10, TYPE_LIQUID, LIQ_WATER}, // water
  
  // friendship bread
  {10, 1, 1, TYPE_VNUM, 256}, // gnome flour
  {10, 2, 1, TYPE_VNUM, 276}, // berries
  {10, 2, 1, TYPE_VNUM, 5701},
  {10, 2, 1, TYPE_VNUM, 10900},
  {10, 2, 1, TYPE_VNUM, 10907},
  {10, 2, 1, TYPE_VNUM, 10911},
  {10, 2, 1, TYPE_VNUM, 24703},
  {10, 3, 3, TYPE_LIQUID, LIQ_WATER},
  {10, 4, 1, TYPE_VNUM, 34706}, //sugar
  {10, 5, 1, TYPE_VNUM, 34707}, //yeast

  // pancake
  {11, 1, 1, TYPE_VNUM, 34706}, //sugar
  {11, 2, 1, TYPE_VNUM, 256}, // gnome flour
  {11, 3, 2, TYPE_LIQUID, LIQ_WATER},

  // berry pancake
  {12, 1, 1, TYPE_VNUM, 34706}, //sugar
  {12, 2, 1, TYPE_VNUM, 256}, // gnome flour
  {12, 3, 2, TYPE_LIQUID, LIQ_WATER},
  {12, 4, 1, TYPE_VNUM, 276}, // berries
  {12, 4, 1, TYPE_VNUM, 5701},
  {12, 4, 1, TYPE_VNUM, 10900},
  {12, 4, 1, TYPE_VNUM, 10907},
  {12, 4, 1, TYPE_VNUM, 10911},
  {12, 4, 1, TYPE_VNUM, 24703},


  
  {-1, -1, -1, -1}
};

#endif
