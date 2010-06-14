//////////////////////////////////////////////////////////////////////////
//
//   SneezyMUD           (c) 1993 SneezyMUD Coding Team.   All Rights Reserved.
//
//   mob_loader.cc : loader functions for mobs
//
//////////////////////////////////////////////////////////////////////////

#include "room.h"
#include "low.h"
#include "handler.h"
#include "extern.h"
#include "monster.h"
#include "obj_open_container.h"
#include "materials.h"
#include "obj_component.h"
#include "obj_potion.h"
#include "obj_commodity.h"
#include "loadset.h"
#include "spec_mobs.h"
#include "configuration.h"

static void treasureCreate(int num, int mat, int &wealth, TObj *bag, TMonster *ch)
{
  float cost=TCommodity::demandCurvePrice(1,0,mat);

  // make sure they can afford it
  if(wealth < cost)
    return;

  if(num<=0)
    return;

  if((num * cost) < 1.0){
    return;
  }

  wealth -= (int)(num * cost);
  TObj * obj = read_object(Obj::GENERIC_COMMODITY, VIRTUAL);
    
  obj->setWeight(num / 10.0);
  obj->setMaterial(mat);

  if (bag)
    *bag += *obj;
  else
    *ch += *obj;
}

struct mat_sort
{
  bool operator()(const int &a, const int &b)
  {
    float pa=TCommodity::demandCurvePrice(1,0,a);// - material_nums[a].price;
    float pb=TCommodity::demandCurvePrice(1,0,b);// - material_nums[b].price;


    // we're sorting on the difference between the average price based on
    // global supply versus the base price
    // the idea being that the first in the list is the most in-demand
    // commodity and thus loaded first; but skewed towards the base value
    // in theory, this should keep our commodities having relative values
    // sort of, like diamond should usually be higher priced than cat fur,
    // because mobs will ignore diamond and load cat fur until they reach
    // their base prices
    // PAPPY: this ends up with lots of mobs using cat fur as their commodity of value,
    // which turns out to be unintuitave for players - so now we sort on value alone

    return pa > pb;
    
    //    return material_nums[a].price > material_nums[b].price;
  }
};

void commodLoader(TMonster *tmons, TObj *bag)
{
  // convert up to 50% of cash into commodities (normal dist. around 25%)
  int wealth = (int)(((::number(0,25)+::number(0,25))/100.0) * tmons->getMoney());
  tmons->setMoney(tmons->getMoney()-wealth);

  std::vector <int> base_mats;
  for (int i=0; i<200; ++i){
    if (material_nums[i].mat_name[0]){
      base_mats.push_back(i);
    }
  }

  std::sort(base_mats.begin(), base_mats.end(), mat_sort());

  int max_units=100;
  float commodValue = 0;

  for(unsigned int i=0;i<base_mats.size();++i){
    commodValue = TCommodity::demandCurvePrice(1,0,base_mats[i]);
    // skip buying 'cheap' commods.
    if (commodValue < 1.0)
      continue;

    // amount of current commod we can afford
    int n_afford=min(max_units, (int)(wealth / commodValue));

    // if we can afford LESS, then there is a higher chance we'll buy it
    // the idea being we prefer to buy expensive commods
    // very wealthy mobs prefer to have cash
    if(n_afford > 0 && n_afford <= ::number(0,max_units)){
      treasureCreate(max(1, ::number(n_afford/2, n_afford)), base_mats[i],
		     wealth, bag, tmons);
      break;
    }
  }
  
  // add the leftover cash back
  tmons->setMoney(tmons->getMoney()+wealth);

}

void potionLoader(TMonster *tmons)
{
  std::vector<int>potions;
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

// Note: this doesn't load multi-tools for multi-class mobs.
/*void loadRepairItems(TMonster *tmons)
{
  if (tmons->GetMaxLevel() <= 15 || tmons->bestClass() >= MAX_CLASSES)
    return;

  static int crappyTools[MAX_CLASSES][5] = {
    // chalk, runes, spike, chisel, silica
    { 571, 575, 579, 581, 583 }, // Mage
    // brush, resin
    { 2346, 2348, 0, 0, 0 }, // Cleric
    // hammer, whetstone, file, tongs
    { 150, 153, 155, 559, 0 }, // Warrior
    // loupe, pliers
    { 586, 588, 0, 0, 0 }, // Thief
    // scalpel, forceps
    { 563, 565, 0, 0, 0 }, // Shaman
    // brush, resin
    { 2346, 2348, 0, 0, 0 }, // Deikhan
    // ladle, manure, oil, punch, cording
    { 567, 569, 573, 590, 592 }, // Monk
    { 0, 0, 0, 0, 0 }, // Ranger
    { 0, 0, 0, 0, 0 }  // Commoner
  };

  static int powerTools[MAX_CLASSES][5] = {
    // chalk, runes, orb, chisel, silica
    { 572, 576, 580, 582, 584 }, // Mage
    // brush, resin
    { 2347, 2349, 0, 0, 0 }, // Cleric
    // whetstone, file, tongs
    { 156, 157, 560, 0, 0 }, // Warrior
    // loupe, pliers
    { 587, 589, 0, 0, 0 }, // Thief
    // scalpel, forceps
    { 564, 566, 0, 0, 0 }, // Shaman
    // brush, resin
    { 2347, 2349, 0, 0, 0 }, // Deikhan
    // ladle, ash, oil, punch, cording
    { 568, 570, 574, 591, 593 }, // Monk
    { 0, 0, 0, 0, 0 }, // Ranger
    { 0, 0, 0, 0, 0 }  // Commoner
  };

  int * myTools = crappyTools[tmons->bestClass()];
  
  if (tmons->GetMaxLevel() > 40)
    myTools = powerTools[tmons->bestClass()];

  for (int iTool = 0; iTool < 5 && myTools[iTool]; iTool++)
  {
    TObj * obj;
    if (!::number(0,49) && (obj = read_object_buy_build(tmons, myTools[iTool], VIRTUAL)))
    {
      *tmons += *obj;
      tmons->logItem(obj, CMD_LOAD);
    }
  }
}*/

void TMonster::thiefLootLoader()
{
  int vnum=0;
  std::vector<int>loot;
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
  if (!(obj = read_object_buy_build(this,vnum,VIRTUAL))){
    vlogf(LOG_BUG, format("couldn't load object %i") %  vnum);
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
  if (UtilMobProc(this))
    return;

  int cashOnHand = 0;
  if (Config::LoadOnDeath()) {
    cashOnHand = getMoney();
    calculateGoldFromConstant(); // can reset money
  }

  // execute our post-load commands
  if (Config::LoadOnDeath() && loadCom.size() > 0) {
    bool last_cmd = true;
    bool objload = false;
    bool mobload = true;
    TObj *obj = NULL;
    TMonster *myself = this;
    TRoom *birthRoom = real_roomp(brtRoom);

    zoneData *zone = birthRoom ? birthRoom->getZone() : NULL;

    for(unsigned int iLoad = 0; zone && iLoad < loadCom.size(); iLoad++)
      if (!loadCom[iLoad].execute(*zone, resetFlagNone, mobload, myself, objload, obj, last_cmd))
        break;
  }

  // skip special loads here - if we skip too early then zonefile loads are affected
  if (getMoney() <= 0) {
    setMoney(cashOnHand);
    return;
  }

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
  // if (hasClass(CLASS_RANGER))
  //   rangerComponentLoader();
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
  //loadRepairItems(this);
  buffMobLoader();
  genericMobLoader(&bag);
  potionLoader(this);
  commodLoader(this, bag);

  // trap bag sometimes
  if (bag) {
    int num = ::number(0, 400);
    if (hasClass(CLASS_THIEF))
      num /= 4;
    if (num < GetMaxLevel()) {

      bag->addContainerFlag(CONT_TRAPPED | CONT_CLOSED);
      weightedRandomizer wr;

      // pick a random damage type
      // sleep is aggravating, avoid
      // teleport also aggravating, slightly avoid
      wr.add(DOOR_TRAP_TNT, -1, 25);
      wr.add(DOOR_TRAP_ENERGY, -1, 25);
      wr.add(DOOR_TRAP_SLEEP, -1, 15);
      wr.add(DOOR_TRAP_POISON, -1, 100);
      wr.add(DOOR_TRAP_FIRE, -1, 100);
      wr.add(DOOR_TRAP_ACID, -1, 75);
      wr.add(DOOR_TRAP_DISEASE, -1, 100);
      wr.add(DOOR_TRAP_SPIKE, -1, 100);
      wr.add(DOOR_TRAP_BLADE, -1, 100);
      wr.add(DOOR_TRAP_PEBBLE, -1, 100);
      wr.add(DOOR_TRAP_FROST, -1, 75);
      wr.add(DOOR_TRAP_TELEPORT, 30, 15);

      bag->setContainerTrapType(doorTrapT(wr.getRandomItem(GetMaxLevel())));
      bag->setContainerTrapDam(min(150, ::number(1 * GetMaxLevel(), 3 * GetMaxLevel())));
    }
  }

  // restore held cash so it doesnt count as load cash for purposes of commod/tool loads
  setMoney(getMoney() + cashOnHand);

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
  const int priceAdjust = 13;

  if (GetMaxLevel() >= 50  && wealth > 1000) {
    wealth -= 800;
    bag_num = 323;
  } else if (GetMaxLevel() >= 17 && wealth > 500) {
    wealth -= 200;
    bag_num = 322;
  } else 
    bag_num = 321;

  // can't buy_build this obj if its doing to be deleted
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
        vlogf(LOG_BUG, format("Component (%d) defined with bad spell (%d).  num=%d") %  comp % spell % num);  
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
      if (!Config::LoadOnDeath() && isMobComponentSeller(comp, mobVnum()))  
        num = -1;  
     
      if (num == -1)  
        continue;  
     
      if (comp == -1) {  
        vlogf(LOG_BUG, format("Bogus component on spell %d (%s)") %   
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
    int price = obj->obj_flags.cost / priceAdjust;
    if (wealth >= price) {
      *bag += *obj;
      wealth -= price;
      found = TRUE;
    } else {
      delete obj;
      obj = NULL;
    }
  }
  if (found)
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

  // cant buy_build this obj if its going to be deleted
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
        vlogf(LOG_BUG, format("Bogus component on spell %d (%s)") % 
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
    // skip brew comps
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
  if (found || !::number(0,3))
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

  // can't but this obj if its going to be deleted
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
        vlogf(LOG_BUG, format("Component (%d) defined with bad spell (%d).  num=%d") %  comp % spell % num);
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
        vlogf(LOG_BUG, format("Bogus component on spell %d (%s)") % 
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
  if (found || !::number(0,3))
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
      if (!(obj = read_object_buy_build(this,syms[i], VIRTUAL))) {
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
  const int level_min = 40;
  if (GetMaxLevel() < level_min)
    return;

  // value of 99  : 5 potions, 7770 mobs : 6.43e-4
  // value of 199 : 4 potions, 6330 mobs : 6.32e-4
  if (::number(0,249))
    return;

  // level based chance
  if (::number(0, max(0, 10000 - GetMaxLevel() * GetMaxLevel())))
    return;

  int vnums[19], num = 0;
  while(num < 5)
    vnums[num++] = Obj::MYSTERY_POTION; // 5 are mystery pot
  vnums[num++] = Obj::LEARNING_POTION;
  vnums[num++] = Obj::YOUTH_POTION;
  while(num < (int)cElements(vnums))
    vnums[num++] = Obj::STATS_POTION; // default is stats pot

  num = vnums[::number(0, cElements(vnums))];

  TObj * obj = read_object(num, VIRTUAL);
  if (!obj) {
    vlogf(LOG_BUG, format("Error in buffMobLoader (%d)") %  num);
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

#if BRICKQUEST
  // brick quest
  if (!::number(0,99) && wealth > 0) {
    TObj *brick = read_object(23091, VIRTUAL);
    if (brick)
      *this += *brick;
  }  
#endif

//October Critter Quest food commodity loads - berries
#if 0
  if (!::number(0,4) && wealth > 0)
  {
    TObj *berries = read_object(276, VIRTUAL);
    if (berries)
      *this += *berries;
  }
#endif

//October Critter Quest food commodity loads - mushrooms
#if 0
  if (!::number(0,4) && wealth > 0)
  {
    TObj *mushrooms = read_object(281, VIRTUAL);
    if (mushrooms)
      *this += *mushrooms;
  }
#endif

//October Critter Quest food commodity loads - roots
#if 0
  if (!::number(0,4) && wealth > 0)
  {
    TObj *roots = read_object(277, VIRTUAL);
    if (roots)
      *this += *roots;
  }
#endif

//October Critter Quest food commodity loads - egg  
#if 0
  if (!::number(0,4) && wealth > 0)
  {
    TObj *egg = read_object(37130, VIRTUAL);
    if (egg)
      *this += *egg;
  }
#endif

//October Critter Quest food commodity loads - steak    
#if 0
  if (!::number(0,9) && wealth > 0)
  {
    TObj *steak = read_object(405, VIRTUAL);
    if (steak)
      *this += *steak;
  }
#endif



  if (GetMaxLevel() < 9)
    return;
  if (!isHumanoid())
    return;
  if (::number(0,9) <= 8)
    return;

  if (wealth > 5) {
    wealth -= 5;
  } else
    return;
 
  TObj *obj;
  // moneypouches are newbie flagged and not sellable, so probably shouldn't buy_build
  if (!(obj = read_object(Obj::GENERIC_MONEYPOUCH, VIRTUAL)) ||
      !(*bag = dynamic_cast<TOpenContainer *>(obj)))
    return;

  amount = ::number(1, wealth);
  (*bag)->addContainerFlag(CONT_CLOSEABLE | CONT_CLOSED);
  (*bag)->setObjStat(ITEM_NEWBIE);

  *this += **bag;
  **bag += *create_money(amount, getFaction());
  setMoney(wealth-amount);

  return;
}

