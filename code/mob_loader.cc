//////////////////////////////////////////////////////////////////////////
//
//   SneezyMUD           (c) 1993 SneezyMUD Coding Team.   All Rights Reserved.
//
//   mob_loader.cc : loader functions for mobs
//
//////////////////////////////////////////////////////////////////////////

#include "stdsneezy.h"
#include "obj_open_container.h"
#include "obj_component.h"
#include "obj_potion.h"

static void treasureCreate(int prob, int cost, int &wealth, int vnum, const char *str, TObj *bag, TMonster *ch)
{
  if (!::number(0,prob) && (wealth >= cost)) {
    int num = min(20, ::number(0,wealth/cost));
    if (num) {
      wealth -= num * cost;
      TObj * obj = read_object(vnum, VIRTUAL);
      obj->describeTreasure(str,num,cost);
      if (bag)
        *bag += *obj;
      else
        *ch += *obj;
    }
  }
}


void potionLoader(TMonster *tmons)
{
  vector<int>potions;
  int pot=0;
  
  potions.push_back(800); // remove poison
  potions.push_back(809); // refresh
  potions.push_back(810); // second wind
  potions.push_back(801); // heal light
  potions.push_back(802); // heal crit
  potions.push_back(803); // heal
  potions.push_back(804); // sanc
  potions.push_back(805); // flight
  potions.push_back(823); // heal full

  if(::number(0,4))
    return;

  if(tmons->GetMaxLevel() < 10)
    return;

  if(tmons->GetMaxLevel() < 20)
    pot=1; // refresh
  else if(tmons->GetMaxLevel() < 35)
    pot=3; // heal light
  else if(tmons->GetMaxLevel() < 50)
    pot=5; // heal
  else
    pot=7; // flight
  
  // equal distribution among the 3 potions in our area
  pot+=::number(-1,1);
  
  // crits
  switch(::number(0,99)){
    case 0:
      pot=max(0, pot-::number(1,3));
      break;
    case 99:
      pot=min(8, pot+::number(1,3));
      break;
  }

  // safety check
  pot=min(pot, (int)potions.size());
  pot=max(pot, 0);
  

  TObj *obj;
  TPotion *tpot;

  obj=read_object(potions[pot], VIRTUAL);

  // buy it, if we can afford it
  if(tmons->getMoney() > obj->obj_flags.cost){
    if((tpot=dynamic_cast<TPotion *>(obj))){
      tpot->setMaxDrinkUnits(1);
      tpot->setDrinkUnits(1);
    }

    tmons->setMoney(tmons->getMoney()-obj->obj_flags.cost);
    *tmons += *obj;
    tmons->logItem(obj, CMD_LOAD);
  } else
    delete obj;
}


void loadRepairItems(TMonster *tmons)
{
  int tool = 0;
  int tool2 = 0;
  int tool3 = 0;
  int tool4 = 0;
  int tool5 = 0;


  if(tmons->GetMaxLevel() > 40) {
    if(tmons->hasClass(CLASS_MAGE)) {
      tool = 576;
      tool2 = 580;
      tool3 = 582;
      tool4 = 584;
      tool5 = 572; // chalk
    }
    if(tmons->hasClass(CLASS_CLERIC)) {
      tool = 2347;
      tool2 = 2349;
    }
    if(tmons->hasClass(CLASS_THIEF)) {
      tool = 587;
      tool2 = 589;
    }
    if(tmons->hasClass(CLASS_WARRIOR)) {
      tool = 150;
      tool2 = 560;
      tool3 = 157;
      tool4 = 156;
    }
    /*
    if(tmons->hasClass(CLASS_RANGER)) {
      tool = 568;
      tool2 = 570;
      tool3 = 574;
    }
    */
    if(tmons->hasClass(CLASS_MONK)) {
      tool = 591;
      tool2 = 593;
      tool3 = 568;
      tool4 = 570;
      tool5 = 574;
    }
    if(tmons->hasClass(CLASS_SHAMAN)) {
      tool = 564;
      tool2 = 566;
    }
    if(tmons->hasClass(CLASS_DEIKHAN)) {
      tool = 2347;
      tool2 = 2349;
    } 
  } else if(tmons->GetMaxLevel() > 15) {
    if(tmons->hasClass(CLASS_MAGE)) {
      tool = 575;
      tool2 = 579;
      tool3 = 581;
      tool4 = 582;
      tool5 = 571; // chalk
    }
    if(tmons->hasClass(CLASS_CLERIC)) {
      tool = 2346;
      tool2 = 2348;
    }
    if(tmons->hasClass(CLASS_THIEF)) {
      tool = 586;
      tool2 = 588;
    }
    if(tmons->hasClass(CLASS_WARRIOR)) {
      tool = 150;
      tool2 = 559;
      tool3 = 153;
      tool4 = 155;
    }
    /*
    if(tmons->hasClass(CLASS_RANGER)) {
      tool = 567;
      tool2 = 569;
      tool3 = 573;
    }
    */
    if(tmons->hasClass(CLASS_MONK)) {
      tool = 590;
      tool2 = 592;
      tool3 = 567;
      tool4 = 569;
      tool5 = 573;
    }
    if(tmons->hasClass(CLASS_SHAMAN)) {
      tool = 563;
      tool2 = 565;
    }
    if(tmons->hasClass(CLASS_DEIKHAN)) {
      tool = 2346;
      tool2 = 2348;      
    }
  }


  TObj *obj=NULL;
  if (tool && !::number(0,99) && (obj = read_object(tool,VIRTUAL))){
    *tmons += *obj;
    tmons->logItem(obj, CMD_LOAD);
  }
  if (tool2 && !::number(0,99) && (obj = read_object(tool2,VIRTUAL))){
    *tmons += *obj;
    tmons->logItem(obj, CMD_LOAD);
  }
  if (tool3 && !::number(0,99) && (obj = read_object(tool3,VIRTUAL))){
    *tmons += *obj;
    tmons->logItem(obj, CMD_LOAD);
  }
  if (tool4 && !::number(0,99) && (obj = read_object(tool4,VIRTUAL))){
    *tmons += *obj;
    tmons->logItem(obj, CMD_LOAD);
  }
  if (tool5 && !::number(0,99) && (obj = read_object(tool5,VIRTUAL))){
    *tmons += *obj;
    tmons->logItem(obj, CMD_LOAD);
  }
}

void TMonster::thiefLootLoader()
{
  int vnum=0;
  vector<int>loot;
  TObj *obj;

  if(::number(0,3))
    return;

  // thief trap stuff
  for(int i=900;i<=923;++i)
    loot.push_back(i);
  for(int i=926;i<=934;++i)
    loot.push_back(i);

  // make a list of the available poisons
  for(int i=31008;i<=31020;++i)
    loot.push_back(i);

  // pick one
  vnum=loot[::number(0, loot.size()-1)];
  
  // load it
  if (!(obj = read_object(vnum,VIRTUAL))){
    vlogf(LOG_BUG, fmt("couldn't load object %i") %  vnum);
    return;
  }
    
  // buy it, if we can afford it
  // divided cost by 2 - mobs are buying at cost here to get load rate
  //   more reasonable - Maror
  if(getMoney() > obj->obj_flags.cost/2){
    setMoney(getMoney()-obj->obj_flags.cost/2);
    *this += *obj;
    logItem(obj, CMD_LOAD);
  } else
    delete obj;
}

void TMonster::createWealth(void)
{
  TOpenContainer *bag;

  if (isPc())
    return;
  if (UtilMobProc(this) || (getMoney() <= 0))
    return;

  // rare and random chance at hitting the Jackpot!
  if (!::number(0,1000)) {
    setMoney(10 * getMoney());
    while (!::number(0,9))
      setMoney(10 * getMoney());
  }

  // class based loading
  if (hasClass(CLASS_THIEF))
    thiefLootLoader();
  if (hasClass(CLASS_SHAMAN))
    shamanComponentLoader();
  if (hasClass(CLASS_MAGE))
    mageComponentLoader();
  if (hasClass(CLASS_RANGER))
    rangerComponentLoader();
  if (hasClass(CLASS_CLERIC)) {
    // which ever one is on top will take precedence (shortages in the other)
    // so....
    if (::number(0,1)) {
      clericHolyWaterLoader();
      clericSymbolLoader();
    } else {
      clericSymbolLoader();
      clericHolyWaterLoader();
    }
  }


  // load specialty items
  loadRepairItems(this);
  buffMobLoader();
  genericMobLoader(&bag);
  potionLoader(this);

  // convert some money into commoditys
  int wealth = getMoney();

     // higher prob_ --> less chance of it loading
  int prob_athanor = 15,             
      cost_athanor = obj_index[real_object(TREASURE_ATHANOR)].value;
  int prob_mithril = 50,             
      cost_mithril = obj_index[real_object(TREASURE_MITHRIL)].value;
  int prob_admantium = 50,           
      cost_admantium = obj_index[real_object(TREASURE_ADMANTIUM)].value;
  int prob_obsidian = 50,            
      cost_obsidian = obj_index[real_object(TREASURE_OBSIDIAN)].value;
  int prob_titanium = 9,             
      cost_titanium = obj_index[real_object(TREASURE_TITANIUM)].value;
  int prob_platinum = 9,             
      cost_platinum = obj_index[real_object(TREASURE_PLATINUM)].value;
  int prob_gold = 5,                 
      cost_gold = obj_index[real_object(TREASURE_GOLD)].value;
  int prob_silver = 4,               
      cost_silver = obj_index[real_object(TREASURE_SILVER)].value;
  int prob_electrum = 5,             
      cost_electrum = obj_index[real_object(TREASURE_ELECTRUM)].value;
  int prob_steel = 4,                
      cost_steel = obj_index[real_object(TREASURE_STEEL)].value;
  int prob_iron = 4,                 
      cost_iron = obj_index[real_object(TREASURE_IRON)].value;
  int prob_bronze = 4,               
      cost_bronze = obj_index[real_object(TREASURE_BRONZE)].value;
  int prob_brass = 4,                
      cost_brass = obj_index[real_object(TREASURE_BRASS)].value;
  int prob_copper = 3,               
      cost_copper = obj_index[real_object(TREASURE_COPPER)].value;
  int prob_aluminum = 4,             
      cost_aluminum = obj_index[real_object(TREASURE_ALUMINUM)].value;
  int prob_tin = 6,                  
      cost_tin = obj_index[real_object(TREASURE_TIN)].value;

  // might want some sort of a switch here for propensity to have
  // items of a certain type
  if ((getRace() == RACE_ELVEN) ||(getRace() == RACE_DROW)) {
    prob_mithril = 7;
    cost_mithril /= 2;
  } else if (getRace() == RACE_DWARF) {
    prob_admantium = 7;
    cost_admantium /= 2;
  }

  int num = 0;

  treasureCreate(prob_athanor, cost_athanor, wealth, TREASURE_ATHANOR, 
         "athanor", bag, this);
  treasureCreate(prob_mithril, cost_mithril, wealth, TREASURE_MITHRIL, 
         "mithril", bag, this);
  treasureCreate(prob_obsidian, cost_obsidian, wealth, TREASURE_OBSIDIAN,
         "obsidian", bag, this);
  treasureCreate(prob_admantium, cost_admantium, wealth, TREASURE_ADMANTIUM,
         "admantium", bag, this);
  treasureCreate(prob_titanium, cost_titanium, wealth, TREASURE_TITANIUM,
         "titanium", bag, this);
  treasureCreate(prob_platinum, cost_platinum, wealth, TREASURE_PLATINUM,
         "platinum", bag, this);
  treasureCreate(prob_gold, cost_gold, wealth, TREASURE_GOLD,
         "gold", bag, this);
  treasureCreate(prob_silver, cost_silver, wealth, TREASURE_SILVER,
         "silver", bag, this);
  treasureCreate(prob_electrum, cost_electrum, wealth, TREASURE_ELECTRUM,
         "electrum", bag, this);
  treasureCreate(prob_steel, cost_steel, wealth, TREASURE_STEEL,
         "steel", bag, this);
  treasureCreate(prob_iron, cost_iron, wealth, TREASURE_IRON,
         "iron", bag, this);
  treasureCreate(prob_bronze, cost_bronze, wealth, TREASURE_BRONZE,
         "bronze", bag, this);
  treasureCreate(prob_brass, cost_brass, wealth, TREASURE_BRASS,
         "brass", bag, this);
  treasureCreate(prob_copper, cost_copper, wealth, TREASURE_COPPER,
         "copper", bag, this);
  treasureCreate(prob_aluminum, cost_aluminum, wealth, TREASURE_ALUMINUM,
         "aluminum", bag, this);
  treasureCreate(prob_tin, cost_tin, wealth, TREASURE_TIN,
         "tin", bag, this);

  // trap bag sometimes
  if (bag) {
    num = ::number(0, 400);
    if (hasClass(CLASS_THIEF))
      num /= 4;
    if (num < GetMaxLevel()) {

      bag->addContainerFlag(CONT_TRAPPED | CONT_CLOSED);

      // pick a random damage type
      // sleep is aggravating, avoid
      // teleport also aggravating, slightly avoid
      switch (::number(1,76)) {
        case 1:
        case 29:
        case 30:
          bag->setContainerTrapType(DOOR_TRAP_TNT);
          break;
        case 2:
        case 31:
        case 32:
          bag->setContainerTrapType(DOOR_TRAP_ENERGY);
          break;
        case 3:
        case 4:
          bag->setContainerTrapType(DOOR_TRAP_SLEEP);
          break;
        case 5:
        case 6:
        case 7:
        case 33:
        case 34:
        case 35:
        case 36:
        case 37:
        case 38:
          bag->setContainerTrapType(DOOR_TRAP_POISON);
          break;
        case 8:
        case 9:
        case 10:
        case 39:
        case 40:
        case 41:
        case 42:
        case 43:
        case 44:
          bag->setContainerTrapType(DOOR_TRAP_FIRE);
          break;
        case 11:
        case 12:
        case 45:
        case 46:
        case 47:
        case 48:
          bag->setContainerTrapType(DOOR_TRAP_ACID);
          break;
        case 13:
        case 14:
        case 49:
        case 50:
        case 51:
        case 52:
          bag->setContainerTrapType(DOOR_TRAP_DISEASE);
          break;
        case 15:
        case 16:
        case 17:
        case 53:
        case 54:
        case 55:
        case 56:
        case 57:
        case 58:
          bag->setContainerTrapType(DOOR_TRAP_SPIKE);
          break;
        case 18:
        case 19:
        case 20:
        case 59:
        case 60:
        case 61:
        case 62:
        case 63:
        case 64:
          bag->setContainerTrapType(DOOR_TRAP_BLADE);
          break;
        case 21:
        case 22:
        case 23:
        case 65:
        case 66:
        case 67:
        case 68:
        case 69:
        case 70:
          bag->setContainerTrapType(DOOR_TRAP_PEBBLE);
          break;
        case 24:
        case 25:
        case 71:
        case 72:
        case 73:
        case 74:
          bag->setContainerTrapType(DOOR_TRAP_FROST);
          break;
        case 26:
        case 27:
        case 28:
        case 75:
        case 76:
        default:
          if (GetMaxLevel() > 30)
            bag->setContainerTrapType(DOOR_TRAP_TELEPORT);
          else 
            bag->setContainerTrapType(DOOR_TRAP_SPIKE);
          break;
      } 
      int amt = ::number(1 * GetMaxLevel(), 3 * GetMaxLevel());
      amt = min(150, amt);
      bag->setContainerTrapDam(amt);
    }
  }

  setMoney(wealth);
  return;
}

bool isMobComponentSeller(int comp, int mvn)
{
  struct ComponentSeller {
    int mob_vnum;
    int comp_vnum;
  };
  ComponentSeller CompSellArray[] = {
    {  512,  COMP_DUST_STORM},
    {24421,  COMP_CONJURE_AIR},
    {22430,  COMP_ILLUMINATE},
    {  509,  COMP_DISPEL_MAGIC},
    {  328,  COMP_GRANITE_FISTS},
    {  513,  COMP_PEBBLE_SPRAY},
    {22431,  COMP_LEVITATE},
    {  514,  COMP_FLAMING_SWORD},
    { 2773,  COMP_STUNNING_ARROW},
    {  511,  COMP_SORCERERS_GLOBE},
    {  298,  COMP_FARLOOK},
    { 7873,  COMP_INVISIBILITY},
    {  510,  COMP_DETECT_INVIS},
    {22438,  COMP_DISPEL_INVIS},
    {12430,  COMP_TELEPATHY},
    {14317,  COMP_TRUE_SIGHT},
    {20421,  COMP_ICY_GRIP},
    {  515,  COMP_ARCTIC_BLAST},
    { 1389,  COMP_GILLS_OF_FLESH},
    {22407,  COMP_PROT_EARTH},
    {  309,  COMP_TRANSFORM_LIMB},
    {   -1,  -1}   // intentional final terminator
  };

  unsigned int iter;
  for (iter = 0; CompSellArray[iter].mob_vnum != -1; iter++) {
    if ((comp == CompSellArray[iter].comp_vnum) &&
        (mvn == CompSellArray[iter].mob_vnum))
      return true;
  }
  return false;
}

void TMonster::mageComponentLoader(void)
{
  int wealth = getMoney();
  TObj *obj,
       *bag = NULL;
  int num = -1, iters = 0;
  spellNumT spell;
  int comp = 0;
  int bag_num = 0;
  bool found = FALSE;
  int inksloaded = 0;

  if (GetMaxLevel() >= 50  && wealth > 1000) {
    wealth -= 800;
    bag_num = 323;
  } else if (GetMaxLevel() >= 17 && wealth > 500) {
    wealth -= 200;
    bag_num = 322;
  } else 
    bag_num = 321;

  if (!(bag = read_object(bag_num, VIRTUAL)))
    return;

  while (::number(0,3) && (wealth > 10)) { 
    iters = 0;
    num = -1;
    while ((num == -1) && (iters < 25)) {
      num = ::number(0, CompIndex.size() - 1);
      spell = CompIndex[num].spell_num;
      comp = CompIndex[num].comp_vnum;
      iters++;
    
      if (spell < TYPE_UNDEFINED || spell >= MAX_SKILL) {
        vlogf(LOG_BUG, fmt("Component (%d) defined with bad spell (%d).  num=%d") %  comp % spell % num);
        continue;
      }
      if (spell != TYPE_UNDEFINED && hideThisSpell(spell)) {
        num = -1;
        continue;
      }
      // only load mage spell component
      if (spell != TYPE_UNDEFINED && discArray[spell]->typ != SPELL_MAGE) {
      	num=-1;
      	continue;
      }

      if (isDissectComponent(comp))
        num = -1;

      if (isInkComponent(comp) && ++inksloaded>3)
	num = -1;

      // disallow certain components
      switch (comp) {
        case COMP_FEATHERY_DESCENT:
        case COMP_FALCON_WINGS:
        case COMP_ANTIGRAVITY:
        case COMP_POWERSTONE:
        case COMP_SHATTER:
        case COMP_ILLUMINATE:
        case COMP_DETECT_MAGIC:
        case COMP_DISPEL_MAGIC:
        case COMP_COPY:
        case COMP_TRAIL_SEEK:
        case COMP_FLARE:
        case COMP_ANIMATE:
        case COMP_BIND:
        case COMP_TELEPORT:
        case COMP_SENSE_LIFE:
        case COMP_SILENCE:
        case COMP_STEALTH:
        case COMP_CALM:
        case COMP_ENSORCER:
        case COMP_FEAR:
        case COMP_CLOUD_OF_CONCEAL:
        case COMP_DETECT_INVIS:
        case COMP_DETECT_SHADOW:
        case COMP_DISPEL_INVIS:
        case COMP_TELEPATHY:
        case COMP_POLYMORPH:
        case COMP_GILLS_OF_FLESH:
        case COMP_BREATH_SARAHAGE:
        case COMP_INFRAVISION:
        case COMP_FLIGHT:
          // we'll make utility comps more rare so that relatively speaking
          // the comps for offensive spells are more prevalent
          if (::number(0,2))
            num = -1;
          break;
        case COMP_FARLOOK:
        case COMP_INVISIBILITY:
        case COMP_LEVITATE:
        case COMP_TRUE_SIGHT:
          // these are also "utility" comps, but players have asked for a
          // slightly higher load rate on them
          if (::number(0,9) < 5)
            num = -1;
          break;
        case COMP_GALVANIZE:
          // keep fairly rare
          if (::number(0,19))
            num = -1;
          break;
        case COMP_ENHANCE_WEAPON:
          // keep VERY rare
          if (::number(0,29))
            num = -1;
          break;
        default:
          break;
      }
      if (num == -1)
        continue;

      // this check is to prevent a mob that "sells" comps via responses
      // from loading the comp they sell, and hence preventing the response
      // load from working
      if (isMobComponentSeller(comp, mobVnum()))
        num = -1;

      if (num == -1)
        continue;

      if (comp == -1) {
        vlogf(LOG_BUG, fmt("Bogus component on spell %d (%s)") % 
                 spell % discArray[spell]->name);
        continue;
      }
    }
    if (num == -1)
      continue;
    // gets here if component is valid for mob

    if (!(obj = read_object(comp,VIRTUAL)))
      continue;

    TComponent *tcom =dynamic_cast<TComponent *>(obj);
    spell = CompIndex[num].spell_num;

    if (tcom && tcom->isComponentType(COMP_SPELL) && spell == TYPE_UNDEFINED) {
      num = -1;
      delete tcom;
      continue;
    }
    if (tcom->isComponentType(COMP_POTION)) {
      num = -1;
      delete tcom;
      continue;
    }

    // 15 is a wee high, 12 is a wee low
    // this is the best way to up the overall availability of components
    // increase the divisor to increase the loads
    int price = obj->obj_flags.cost / 13;
    if (wealth >= price) {
      *bag += *obj;
      wealth -= price;
      found = TRUE;
    } else {
      delete obj;
      obj = NULL;
    }
  }
  if (found || ::number(0,2))
    *this += *bag;
  else {
    delete bag;
    bag = NULL;
  }

  setMoney(wealth);
  return;
}

void TMonster::rangerComponentLoader(void)
{
  int wealth = getMoney();
  TObj *obj, *bag;
  int num = -1, iters = 0;
  int spell, comp = 0;
  int bag_num = 0;
  int found = FALSE;

  if (GetMaxLevel() >= 50  && wealth > 1000) {
    wealth -= 800;
    bag_num = 332;
  } else if (GetMaxLevel() >= 17 && wealth > 500) {
    wealth -= 200;
    bag_num = 331;
  } else 
    bag_num = 330;

  if (!(bag = read_object(bag_num, VIRTUAL)))
    return;

  while (::number(0,3) && (wealth > 10)) { 
    iters = 0;
    num = -1;
    while ((num == -1) && (iters < 15)) {
      num = ::number(0, CompIndex.size() - 1);
      spell = CompIndex[num].spell_num;
      comp = CompIndex[num].comp_vnum;
      iters++;
    
      // no mage spell components
      if (spell != TYPE_UNDEFINED && discArray[spell] && 
	 discArray[spell]->typ != SPELL_RANGER){
      	num=-1;
      	continue;
      }

      if (isDissectComponent(comp))
        num = -1;

      // disallow certain components
      switch (comp) {
        default:
          break;
      }
      if (num == -1)
        continue;

      if (comp == -1) {
        vlogf(LOG_BUG, fmt("Bogus component on spell %d (%s)") % 
                 spell % discArray[spell]->name);
        continue;
      }
    }
    if (num == -1)
      continue;
    // gets here if component is valid for mob

    if (!(obj = read_object(comp,VIRTUAL)))
      continue;

    // skip comps that are "disabled"
    TComponent *tcom =dynamic_cast<TComponent *>(obj);
    spell = CompIndex[num].spell_num;
    if (tcom && tcom->isComponentType(COMP_SPELL) && spell == TYPE_UNDEFINED) {
      num = -1;
      delete tcom;
      continue;
    }
    // skip scribe comps
    if (tcom->isComponentType(COMP_SCRIBE)) {
      num = -1;
      delete tcom;
      continue;
    }
    // skip scribe comps
    if (tcom->isComponentType(COMP_POTION)) {
      num = -1;
      delete tcom;
      continue;
    }
    // 15 is a wee high

    int price = obj->obj_flags.cost / 12;
    if (wealth >= price) {
      *bag += *obj;
      wealth -= price;
      found = TRUE;
    } else {
      delete obj;
      obj = NULL;
    }
  }
  if (found || ::number(0,2))
    *this += *bag;
  else {
    delete bag;
    bag = NULL;
  }

  setMoney(wealth);
  return;
}

void TMonster::shamanComponentLoader(void)
{
  int wealth = getMoney();
  TObj *obj,
       *bag = NULL;
  int num = -1, iters = 0;
  spellNumT spell;
  int comp = 0;
  int bag_num = 0;
  bool found = FALSE;
  int Brewload = 0;

  if (GetMaxLevel() >= 50  && wealth > 1000) {
    wealth -= 800;
    bag_num = 31319;
  } else if (GetMaxLevel() >= 17 && wealth > 500) {
    wealth -= 200;
    bag_num = 31318;
  } else 
    bag_num = 31317;

  if (!(bag = read_object(bag_num, VIRTUAL)))
    return;


  while (::number(0,3) && (wealth > 10)) { 
    iters = 0;
    num = -1;
    while ((num == -1) && (iters < 25)) {
      num = ::number(0, CompIndex.size() - 1);
      spell = CompIndex[num].spell_num;
      comp = CompIndex[num].comp_vnum;
      iters++;
    
      if (spell < TYPE_UNDEFINED || spell >= MAX_SKILL) {
        vlogf(LOG_BUG, fmt("Component (%d) defined with bad spell (%d).  num=%d") %  comp % spell % num);
        continue;
      }
      if (spell != TYPE_UNDEFINED && hideThisSpell(spell)) {
        num = -1;
        continue;
      }
      // only load mage spell component
      if (spell != TYPE_UNDEFINED && discArray[spell]->typ != SPELL_SHAMAN) {
      	num=-1;
      	continue;
      }

      if (isDissectComponent(comp))
        num = -1;

      if (isBrewComponent(comp) && ++Brewload>3)
	num = -1;

      // disallow certain components
      switch (comp) {
	case COMP_BLOOD_BOIL:
	case COMP_AQUALUNG:
        case COMP_HYPNOSIS:
          // we'll make utility comps more rare so that relatively speaking
          // the comps for offensive spells are more prevalent
          if (::number(0,2))
            num = -1;
          break;
	case COMP_CELERITE:
        case COMP_CLARITY:
        case COMP_SHADOW_WALK:
          // these are also "utility" comps, but players have asked for a
          // slightly higher load rate on them
          if (::number(0,9) < 5)
            num = -1;
          break;
#ifdef RARECOMPSHAMAN
	case COMP_XXXXXXXXXXXXXXXXXXX:
          // keep fairly rare
          if (::number(0,19))
            num = -1;
          break;
        case COMP_XXXXXXXXXXXX:
          // keep VERY rare
          if (::number(0,29))
            num = -1;
          break;
#endif
        default:
          break;
      }
      if (num == -1)
        continue;

      // this check is to prevent a mob that "sells" comps via responses
      // from loading the comp they sell, and hence preventing the response
      // load from working
      if (isMobComponentSeller(comp, mobVnum()))
        num = -1;

      if (num == -1)
        continue;

      if (comp == -1) {
        vlogf(LOG_BUG, fmt("Bogus component on spell %d (%s)") % 
                 spell % discArray[spell]->name);
        continue;
      }
    }
    if (num == -1)
      continue;
    // gets here if component is valid for mob

    if (!(obj = read_object(comp,VIRTUAL)))
      continue;

    TComponent *tcom =dynamic_cast<TComponent *>(obj);
    spell = CompIndex[num].spell_num;

    if (tcom && tcom->isComponentType(COMP_SPELL) && spell == TYPE_UNDEFINED) {
      num = -1;
      delete tcom;
      continue;
    }
    // skip scribe comps
    if (tcom->isComponentType(COMP_SCRIBE)) {
      num = -1;
      delete tcom;
      continue;
    }

    // 15 is a wee high, 12 is a wee low
    // this is the best way to up the overall availability of components
    // increase the divisor to increase the loads
    int price = obj->obj_flags.cost / 13;
    if (wealth >= price) {
      *bag += *obj;
      wealth -= price;
      found = TRUE;
    } else {
      delete obj;
      obj = NULL;
    }
  }
  if (found || ::number(0,2))
    *this += *bag;
  else {
    delete bag;
    bag = NULL;
  }

  setMoney(wealth);
  return;
}

void TMonster::clericHolyWaterLoader(void)
{
  int wealth = getMoney();
  TObj *obj;
  int num = 0;

  if (isPc())
    return;

  int iter;
  for (iter = 0; iter < 3; iter++) {
    if (::number(0,3))  // randomize
      continue;

    if (iter == 0)
      num = 165;  // large
    else if (iter == 1)
      num = 164;  // medium
    else if (iter == 2)
      num = 163;  // small

    if (!(obj = read_object(num, VIRTUAL))) {
      vlogf(LOG_BUG, "Error in cleric Holy Water Loader");
      return;
    }
    int value = obj->obj_flags.cost / 10;

    // newbie cleric gift
    if (GetMaxLevel() < 5)
      value /= 2;

    if (wealth > value) {
      *this += *obj;
      wealth -= value;
      setMoney(wealth);
    } else {
      delete obj;
      obj = NULL;
    }
  }
  return;
}

void TMonster::clericSymbolLoader(void)
{
  int wealth = getMoney();
  TObj *obj;
  int value, div=5;

  // list of syms, in order of preference (highest level first)
  int syms[]={514,513,512,511,510,509,508,507,506,505,504,503,502,501,-1};

  if (::number(0,3) || isPc())
    return;

  for(int i=0;syms[i]!=-1;++i){
    value = obj_index[real_object(syms[i])].value / div;
    
    if(wealth>value){
      if (!(obj = read_object(syms[i], VIRTUAL))) {
	vlogf(LOG_BUG, "Error in cleric Component Loader");
	return;
      }

      *this += *obj;
      logItem(obj, CMD_LOAD);
      setMoney(wealth - value);
      return;
    }
  }

  return;
}

void TMonster::buffMobLoader()
{
  // no idea whats trying to be done
  // bat changed down on line 906 so
  // i commented out this declaration
  //  TObj *obj;
  int num, vnum;

#if 1
  const int level_min = 20;
  if (GetMaxLevel() < level_min)
    return;
  if (GetMaxLevel() > 75)
    return;
#endif

  // value of 99  : 5 potions, 7770 mobs : 6.43e-4
  // value of 199 : 4 potions, 6330 mobs : 6.32e-4
  if (::number(0,249))
    return;

  // level based chance
  if (::number(0,9999) > (GetMaxLevel() + 10) *
                         (GetMaxLevel() + 10))
    return;

//  char buf[1024];
//  sprintf(buf, "%s made it thru buffMob.\n\r", getName());
  int obj_lev = -1;

  num = ::number(1,19);
  switch (num) {
    case 1:
    case 14:
    case 15:
    case 16:
    case 17:
      vnum = MYSTERY_POTION;
      break;
    case 18:
      vnum = LEARNING_POTION;
      break;
    case 19:
      vnum = YOUTH_POTION;
      break;
    case 2:
      vnum = STATS_POTION;
      obj_lev = STAT_STR;
      break;
    case 3:
      vnum = STATS_POTION;
      obj_lev = STAT_DEX;
      break;
    case 4:
      vnum = STATS_POTION;
      obj_lev = STAT_BRA;
      break;
    case 5:
      vnum = STATS_POTION;
      obj_lev = STAT_AGI;
      break;
    case 6:
      vnum = STATS_POTION;
      obj_lev = STAT_CON;
      break;
    case 7:
      vnum = STATS_POTION;
      obj_lev = STAT_INT;
      break;
    case 8:
      vnum = STATS_POTION;
      obj_lev = STAT_WIS;
      break;
    case 9:
      vnum = STATS_POTION;
      obj_lev = STAT_FOC;
      break;
    case 10:
      vnum = STATS_POTION;
      obj_lev = STAT_CHA;
      break;
    case 11:
      vnum = STATS_POTION;
      obj_lev = STAT_KAR;
      break;
    case 12:
      vnum = STATS_POTION;
      obj_lev = STAT_SPE;
      break;
    case 13:
      vnum = STATS_POTION;
      obj_lev = STAT_PER;
      break;
    default:
      return;
  }

#if 1
// builder port uses stripped down database which was causing problems
// hence this setup instead.
  int robj = real_object(vnum);
  if (robj < 0 || robj >= (signed int) obj_index.size()) {
    vlogf(LOG_BUG, fmt("buffMobLoader(): No object (%d) in database!") %  vnum);
    return;
  }

  TObj * obj = read_object(robj, REAL);
#else
  TObj * obj = read_object(vnum, VIRTUAL);
#endif

  if (!obj) {
    vlogf(LOG_BUG, fmt("Error in buffMobLoader (%d)") %  vnum);
    return;
  }

  *this += *obj;


  return;
}

void TMonster::genericMobLoader(TOpenContainer **bag)
{
  int wealth = getMoney();
  int amount;

  *bag = NULL;

  //sneezy sweeps
#if 0
  if (!::number(0,19) && wealth > 0)
  {
    TObj *tile = read_object(29301, VIRTUAL);
    if (tile)
      *this += *tile;
  }
#endif
  
  if (GetMaxLevel() < 9)
    return;
  if (!isHumanoid())
    return;
  if (::number(0,9) <= 5)
    return;

  if (wealth > 5) {
    wealth -= 5;
  } else
    return;
 
  TObj *obj;
  if (!(obj = read_object(GENERIC_MONEYPOUCH, VIRTUAL)) ||
      !(*bag = dynamic_cast<TOpenContainer *>(obj)))
    return;

  amount = ::number(1, wealth);
  (*bag)->addContainerFlag(CONT_CLOSEABLE | CONT_CLOSED);
  (*bag)->setObjStat(ITEM_NEWBIE);

  *this += **bag;
  **bag += *create_money(amount);
  setMoney(wealth-amount);

  return;
}













