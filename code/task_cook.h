#ifndef __TASK_COOK_H
#define __TASL_COOK_H

const int TYPE_VNUM=0;
const int TYPE_LIQUID=1;
const int TYPE_MATERIAL=2;
const int TYPE_CORPSE=3;

class ingredientTypeT {
public:
  int recipe, amt, type, num;
};

class recipeTypeT {
public:
  int recipe;
  const char *keywords, *name;
  int vnum;
};

// recipe number, keywords, name, resulting item
recipeTypeT recipes[] =
{
  {0, "steak marinated", "marinated steak", 405},
  {1, "catfish", "fried catfish", 14358},
  {2, "rat stick", "rat on a stick", 14352},
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
  {2, 1, TYPE_VNUM, 3413}, // 1 long arrow

  {-1, -1, -1, -1}
};

ingredientTypeT opt_ingredients[] =
{ 
    {-1, -1, -1, -1}
};


#endif
