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
  int recipe, amt, type, num;
};

class recipeTypeT {
public:
  int recipe;
  const char *keywords, *name;
  int vnum; // base, quality adds more
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
  {-1, NULL, NULL, -1}
};


// recipe number, amount of item, type of item, item vnum
ingredientTypeT ingredients[] =
{ 
  // marinated steak
  {0, 1, TYPE_VNUM, GENERIC_STEAK},  // any steak will do
  {0, 3, TYPE_LIQUID, LIQ_WHISKY}, // 3 ounces of whiskey
  {0, 1, TYPE_VNUM, 432}, // 1 orange
  
  // fried catfish
  {1, 1, TYPE_VNUM, 13803}, // 1 catfish
  {1, 1, TYPE_VNUM, 263}, // jar of whale grease

  // rat on a stick
  {2, 1, TYPE_CORPSE, RACE_RODENT}, // 1 rat
  {2, 1, TYPE_ITEM, ITEM_ARROW}, // 1 arrow

  // mashed potatoes
  {3, 1, TYPE_VNUM, 31766}, // 1 potato
  {3, 1, TYPE_VNUM, 31767}, // butter

  // side salad
  {4, 1, TYPE_VNUM, 31768}, // lettuce
  {4, 1, TYPE_VNUM, 14348}, // tomato
  {4, 1, TYPE_VNUM, 31769}, // dressing
  {4, 1, TYPE_VNUM, 31770}, // onion
  {4, 1, TYPE_VNUM, 31771}, // carrot
  {4, 1, TYPE_VNUM, 31772}, // croutons

  // steak dinner
  {5, 1, TYPE_VNUM, 405}, // marinated steak, recipe 0
  {5, 1, TYPE_VNUM, 31773}, // side of mashed potatoes, recipe 3
  {5, 1, TYPE_VNUM, 31774}, // side salad, recipe 4

  {-1, -1, -1, -1}
};

#endif
