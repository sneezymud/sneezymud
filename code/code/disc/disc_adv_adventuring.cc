#include "handler.h"
#include "extern.h"
#include "room.h"
#include "being.h"
#include "client.h"
#include "low.h"
#include "disc_adv_adventuring.h"
#include "disease.h"
#include "combat.h"
#include "obj_component.h"
#include "obj_base_corpse.h"
#include "obj_tool.h"
#include "obj_portal.h"
#include "obj_drinkcon.h"
#include "pathfinder.h"
#include "obj_trash.h"
#include "obj_food.h"

void forage(TBeing * caster)
{
  if (caster->getMyRace()->hasTalent(TALENT_INSECT_EATER))
    forage_insect(caster);
  else
    forage(caster, caster->getSkillValue(SKILL_FORAGE));
}

static const int FORAGE_BEGIN = 276, FORAGE_END = 281;
static const int FORAGE_ARCTIC_BEGIN = 37130, FORAGE_ARCTIC_END = 37133;
static const int FORAGE_CAVE_BEGIN = 37134, FORAGE_CAVE_END = 37136;
static const int FORAGE_DESERT_BEGIN = 37137, FORAGE_DESERT_END = 37140;
static const int FORAGE_INSECT_FOOD = 4;

int forage(TBeing *caster, short bKnown)
{
  TObj *obj = NULL;
  int forageItem;
  affectedData aff;

  spellNumT skill = caster->getSkillNum(SKILL_FORAGE);

  if (!caster->roomp) {
    vlogf(LOG_BUG, format("Forage called without room pointer: %s") % caster->name);
    return SPELL_FAIL;
  }
  
  if (!caster->doesKnowSkill(skill)) {
    caster->sendTo("You know nothing about foraging for food!\n\r");
    return SPELL_FAIL;
  }
  

  if (caster->roomp->isCitySector()
      || !(caster->roomp->isForestSector()
      || caster->roomp->isBeachSector()
      || caster->roomp->isHillSector()
      || caster->roomp->isMountainSector()
      || caster->roomp->isNatureSector()
      || caster->roomp->isRoadSector()
      || caster->roomp->isSwampSector()
      || caster->roomp->isArcticSector()
      || caster->roomp->getSectorType() == SECT_TEMPERATE_CAVE
      || caster->roomp->getSectorType() == SECT_TROPICAL_CAVE
      || caster->roomp->getSectorType() == SECT_ARCTIC_CAVE)) {
    caster->sendTo("You need to be in nature to forage.\n\r");
    return SPELL_FAIL;
  } else if (caster->roomp->isFlyingSector()
      || caster->roomp->isVertSector()
      || caster->roomp->isUnderwaterSector()
      || caster->roomp->isAirSector()
      || caster->roomp->isOceanSector()
      || caster->roomp->isRiverSector()) {
    // being safe here - allowing arctic allows a bunch of undesired sector types
    caster->sendTo("You cannot forage here.\n\r");
    return SPELL_FAIL;
  } else if (caster->roomp->isRoomFlag(ROOM_INDOORS) && !(caster->roomp->getSectorType() == SECT_TEMPERATE_CAVE || caster->roomp->getSectorType() == SECT_TROPICAL_CAVE || caster->roomp->getSectorType() == SECT_ARCTIC_CAVE)) {
    caster->sendTo("You need to be outside to forage.\n\r");
    return SPELL_FAIL;
  } else if (caster->roomp->isRoomFlag(ROOM_FLOODED)) {
    caster->sendTo("The flood in here needs to subside before you can forage.\n\r");
    return SPELL_FAIL;
  } else if (caster->roomp->isRoomFlag(ROOM_ON_FIRE)) {
    caster->sendTo("The raging fire here needs to be controlled before you can forage.\n\r");
    return SPELL_FAIL;
  }
  
  if (caster->checkForSkillAttempt(SKILL_FORAGE)) {
    act("You tried foraging recently and are not prepared to try again so soon.", FALSE, caster, NULL, NULL, TO_CHAR);
    return SPELL_FAIL;
   }

  if (caster->affectedBySpell(SKILL_FORAGE)) {
    act("You must wait before foraging again.", FALSE, caster, NULL, NULL, TO_CHAR);
    return SPELL_FAIL;
  }
  int forage_move = ::number(5,15);
  if (caster->getMove() < forage_move) {
    caster->sendTo("You lack the vitality to forage.\n\r");
    return SPELL_FAIL;
  }
  caster->addToMove(-forage_move);

  if (caster->bSuccess(bKnown, SKILL_FORAGE)) {
    int foodpile = 1000;
    while (::number(0, 999) < foodpile) {
      if (caster->roomp->isIndoorSector())
        forageItem = ::number(FORAGE_CAVE_BEGIN, FORAGE_CAVE_END);
      else if (caster->roomp->isArcticSector())
        forageItem = ::number(FORAGE_ARCTIC_BEGIN, FORAGE_ARCTIC_END);
      else if (caster->roomp->getSectorType() == SECT_DESERT)
        forageItem = ::number(FORAGE_DESERT_BEGIN, FORAGE_DESERT_END);
      else 
        forageItem = ::number(FORAGE_BEGIN, FORAGE_END);
      
      obj = read_object(forageItem, VIRTUAL);
  
      if (!obj) {
        caster->sendTo("Something went wrong, bug Cosmo.\n\r");
        vlogf(LOG_BUG, format("Forage tried to load a NULL object (%d)") % forageItem);
        return SPELL_FAIL;
      }
  
      act("You rustle up $p.", FALSE, caster, obj, NULL, TO_CHAR);
      act("$n rustles up $p.", TRUE, caster, obj, NULL, TO_ROOM);
      *caster->roomp += *obj;
      foodpile /= 3;
    }
    aff.type = SKILL_FORAGE;
    aff.location = APPLY_NONE;
    aff.duration = 4 * UPDATES_PER_MUDHOUR;
    aff.bitvector = 0;
    aff.modifier = 0;
    caster->affectTo(&aff, -1);
    return SPELL_SUCCESS;
  } else {
    aff.type = AFFECT_SKILL_ATTEMPT;
    aff.location = APPLY_NONE;
    aff.duration = 2 * UPDATES_PER_MUDHOUR;
    aff.bitvector = 0;
    aff.modifier = SKILL_FORAGE;

    caster->affectTo(&aff, -1);

    act("You don't seem to find anything.", FALSE, caster, obj, NULL, TO_CHAR);
    return SPELL_FAIL;
  }
}

typedef struct _insectForage
{
  double baseFood;
  int cBugs;
  const sstring bugs[3];
} insectForage;

static const insectForage terrain_insects[MAX_SECTOR_TYPES] = {
  { .25, 2, { "snow beetle", "ice spider" }},        //SECT_SUBARCTIC,
  { .25, 2, { "snow beetle", "tundra cricket" }},    //SECT_ARCTIC_WASTE,
  { .25, 2, { "gadfly", "march fly" }},              //SECT_ARCTIC_CITY,
  { .25, 2, { "white wasp", "frost scorpion" }},     //SECT_ARCTIC_ROAD,
  { .30, 2, { "frost scorpion", "tundra cricket" }}, //SECT_TUNDRA,
  { .25, 2, { "ice beetle", "avalanche moth" }},     //SECT_ARCTIC_MOUNTAINS,
  { .25, 2, { "white wasp", "ice spider"  }},        //SECT_ARCTIC_FOREST,
  {  .5, 2, { "peat beetle", "water skimmer" }},     //SECT_ARCTIC_MARSH,
  { .30, 2, { "ice beetle", "water skimmer" }},      //SECT_ARCTIC_RIVER_SURFACE,
  {   0, 0, { "" }},                                 //SECT_ICEFLOW,
  { .25, 1, { "ice beetle" }},                       //SECT_COLD_BEACH,
  {   0, 0, { "" }},                                 //SECT_SOLID_ICE,
  { .25, 2, { "gadfly", "march fly" }},              //SECT_ARCTIC_BUILDING,
  { .50, 2, { "armadillo bug", "white moth" }},      //SECT_ARCTIC_CAVE,
  { .25, 2, { "gadfly", "march fly" }},              //SECT_ARCTIC_ATMOSPHERE,
  { .25, 2, { "gadfly", "march fly" }},              //SECT_ARCTIC_CLIMBING,
  { .25, 2, { "white wasp", "ice spider" }},         //SECT_ARCTIC_FOREST_ROAD,
  {   1, 2, { "butterfly", "locust" }},              //SECT_PLAINS,
  { .75, 3, { "black fly", "moth", "ant" }},         //SECT_TEMPERATE_CITY,
  {   1, 3, { "wasp", "ladybug", "ant" }},           //SECT_TEMPERATE_ROAD,
  {   1, 2, { "grasshopper", "ladybug" }},           //SECT_GRASSLANDS,
  {   1, 2, { "junebug", "grasshopper" }},           //SECT_TEMPERATE_HILLS,
  {   1, 2, { "butterfly", "junebug" }},             //SECT_TEMPERATE_MOUNTAINS,
  {   1, 3, { "bumblebee", "spider", "earwig" }},    //SECT_TEMPERATE_FOREST,
  {   2, 2, { "dragonfly", "mosquito" }},            //SECT_TEMPERATE_SWAMP,
  {   0, 0, { "" }},                                 //SECT_TEMPERATE_OCEAN,
  {1.25, 2, { "water beetle", "cranefly" }},         //SECT_TEMPERATE_RIVER_SURFACE,
  {   0, 0, { "" }},                                 //SECT_TEMPERATE_UNDERWATER,
  { .50, 2, { "noseum", "sandflea" }},               //SECT_TEMPERATE_BEACH,
  { .75, 3, { "black fly", "horsefly", "ant" }},     //SECT_TEMPERATE_BUILDING,
  { .75, 3, { "armadillo bug", "spider", "moth" }},  //SECT_TEMPERATE_CAVE,
  {   1, 2, { "midge", "gnat" }},                    //SECT_TEMPERATE_ATMOSPHERE,
  {   1, 2, { "midge", "gnat" }},                    //SECT_TEMPERATE_CLIMBING,
  {   1, 2, { "bumblebee", "ladybug" }},             //SECT_TEMPERATE_FOREST_ROAD,
  { .50, 2, { "black scarab", "scorpion" }},         //SECT_DESERT,
  {   1, 3, { "cicada", "locust", "termite" }},      //SECT_SAVANNAH,
  {   1, 2, { "cicada", "veld hopper" }},            //SECT_VELDT,
  {   1, 2, { "cockroach", "jungle fly" }},          //SECT_TROPICAL_CITY,
  { 1.5, 2, { "jungle fly", "red wasp" }},           //SECT_TROPICAL_ROAD,
  {   2, 2, { "jungle nymph", "botfly" }},           //SECT_JUNGLE,
  {   2, 2, { "mantis", "peacock butterfly" }},      //SECT_RAINFOREST,
  { 1.5, 2, { "blood spider", "stickbug" }},         //SECT_TROPICAL_HILLS,
  { 1.5, 2, { "blood spider", "fruit beetle" }},     //SECT_TROPICAL_MOUNTAINS,
  {   0, 0, { "" }},                                 //SECT_VOLCANO_LAVA,
  { 2.5, 2, { "water cricket", "hoary mosquito" }},  //SECT_TROPICAL_SWAMP,
  {   0, 0, { "" }},                                 //SECT_TROPICAL_OCEAN,
  {1.75, 2, { "daddy long legs", "hoary mosquito" }},//SECT_TROPICAL_RIVER_SURFACE,
  {   0, 0, { "" }},                                 //SECT_TROPICAL_UNDERWATER,
  {   1, 2, { "sandflea", "tiger beetle" }},         //SECT_TROPICAL_BEACH,
  {   1, 3, { "cockroach", "weevil", "jungle fly" }},//SECT_TROPICAL_BUILDING,
  {   1, 2, { "cockroach", "spotted moth" }},        //SECT_TROPICAL_CAVE,
  {   1, 2, { "jungle fly", "hoary mosquito" }},     //SECT_TROPICAL_ATMOSPHERE,
  {   1, 2, { "jungle fly", "hoary mosquito" }},     //SECT_TROPICAL_CLIMBING,
  {   1, 2, { "red wasp", "vibrant butterfly" }},    //SECT_RAINFOREST_ROAD,
  {   0, 0, { "" }},                                 //SECT_ASTRAL_ETHREAL,
  {   0, 0, { "" }},                                 //SECT_SOLID_ROCK,
  {   0, 0, { "" }},                                 //SECT_FIRE,
  {   0, 0, { "" }},                                 //SECT_INSIDE_MOB,
  {   0, 0, { "" }},                                 //SECT_FIRE_ATMOSPHERE,
  {   0, 0, { "" }},                                 //SECT_MAKE_FLY,
  {   1, 3, { "white moth", "grey moth", "black moth" }}, //SECT_DEAD_WOODS,
};

int forage_insect(TBeing *caster)
{
  affectedData aff;
  double food = 1;
  sstring bug;

  if (caster->getCond(FULL) <= -1 || caster->getCond(FULL) > 20)
  {
    act("You are too full to consider eating right now.", FALSE, caster, NULL, NULL, TO_CHAR);
    return SPELL_FAIL;
  }
  if (!caster->roomp)
  {
    vlogf(LOG_BUG, format("Forage called without room pointer: %s") % caster->name);
    return SPELL_FAIL;
  }
  if (caster->checkForSkillAttempt(SKILL_FORAGE))
  {
    act("You tried foraging insects recently and are not prepared to try again so soon.", FALSE, caster, NULL, NULL, TO_CHAR);
    return SPELL_FAIL;
  }

  // start out with random number
  food *= (::number(5,20)/10);

  // if you know foraging, this will work much better (up to +50%)
  if (caster->doesKnowSkill(caster->getSkillNum(SKILL_FORAGE)) && caster->bSuccess(SKILL_FORAGE))
    food *= (int(caster->getSkillValue(caster->getSkillNum(SKILL_FORAGE)) + 50) / 50);

  // choose base food and bug on terrain
  const insectForage *forage = &terrain_insects[caster->roomp->getSectorType()];
  food *= forage->baseFood;
  if (forage->cBugs)
    bug = forage->bugs[::number(0,forage->cBugs-1)];

  // increase chance if there is spoiled food or trash
  for(StuffIter it=caster->roomp->stuff.begin();it!=caster->roomp->stuff.end();++it)
  {
    if (dynamic_cast<TTrash*>(*it) || (dynamic_cast<TFood*>(*it) && dynamic_cast<TFood*>(*it)->isFoodFlag(FOOD_SPOILED)))
    {
      food += 0.5;
      break;
    }
  }

  // the amount of food we find base is FORAGE_INSECT_FOOD
  food *= FORAGE_INSECT_FOOD;

  aff.type = AFFECT_SKILL_ATTEMPT;
  aff.location = APPLY_NONE;
  aff.duration = UPDATES_PER_MUDHOUR;
  aff.bitvector = 0;
  aff.modifier = SKILL_FORAGE;
  caster->affectTo(&aff, -1);

  if (int(food) <= 0)
  {
    act("You can't seem to find any insects to eat here.", FALSE, caster, NULL, NULL, TO_CHAR);
    return SPELL_FAIL;
  }

  static const sstring bugsizes[] = { "miniscule", "tiny", "juicy", "fat", "giant", "gargantuan", "HUMONGOUS" };
  static const sstring actions[] = { "quickly gobbles up a %s", "crunches down a %s", "swallows a %s in a single gulp", "tears up a %s and eats it" };

  sstring bugname = bugsizes[min(max(0, int(food * int(cElements(bugsizes)-1) / (3*FORAGE_INSECT_FOOD))), int(cElements(bugsizes)-1))];
  bugname += " ";
  bugname += bug;
  sstring action = actions[::number(0, cElements(actions)-1)];
  action = format(action) % bugname;

  act(format("You flick your tongue out and snag a %s!") % bugname, false, caster, NULL, NULL, TO_CHAR);
  act(format("$n flicks $s tongue out and snags a %s!") % bugname, false, caster, NULL, NULL, TO_ROOM);
  act(format("You eat a %s.") % bugname, false, caster, NULL, NULL, TO_CHAR);
  act(format("$n %s!") % action, false, caster, NULL, NULL, TO_ROOM);

  caster->gainCondition(FULL, int(food));
  if (caster->getCond(FULL) > 20)
    act("You are full.", FALSE, caster, 0, 0, TO_CHAR);

  return SPELL_SUCCESS;
}


void TBeing::doForage()
{
  if (getPosition() < POSITION_STANDING) {
    sendTo("You need to be standing in order to forage for food.\n\r");
    return;
  }
  if (fight()) {
    sendTo("The ensuing battle makes it difficult to search for food.\n\r");
    return;
  }
 
  addToWait(combatRound(1));
  forage(this);
}

int determineSkinningItem(TBaseCorpse * corpse, int * amount, char * msg, char * gl_msg)
{
  int num = -1;

  // default quantities and messages
  // override these where appropriate.
  *amount = 50;
  sprintf(   msg, "You skin $p from $N.");
  sprintf(gl_msg, "$n skins $p from $N.");

  if (num == -1) {
    // base on specific vnum here if needed
    switch(corpse->getCorpseVnum()) {
      case 7032:  // othora
      case 20611:
      case 20612:
      case 20613:
      case 20621:
      case 20622:
      case 20623:
        num = 20643;
        break;
      case 20625:  // kelk
      case 20626:
      case 20627:
        num = 20644;
        break;
      case 20628:  // ganthi
        num = 20645;
        break;
      case 20629:  // prarie digger
        num = 20646;
        break;
      case 122:  // fuzzy mouse
        num = 2400;
        break;
      case 3400:  // wolves
      case 3701:
      case 3711:
      case 10516:
      case 23629:
        num = 2403;
        break;
      case 3404:  // lynx
        num = 2404;
        break;
      case 8502:  // gazelle
        num = 2410;
        break;
      case 8503:  // zebra
        num = 2411;
        break;
      case 8507:  // hyena
      case 932:  // hyena
        num = 2414;
        break;
      case 14107:  // fox
        num = 2415;
        break;
      case 8511:  // cheetah
        num = 2416;
        break;
      case 8515:  // lioness
        num = 2418;
        break;
      case 10920:  // skunk
        num = 2419;
        break;
      case 10908:  // wolverine
        num = 2423;
        break;
      case 10907:  // elk
        num = 2425;
        break;
      case 10200:  // moose
      case 10210:  // moose
        num = 2426;
        break;
      case 10201:  // caribou
      case 10211:  // caribou
        num = 2427;
        break;
      case 10202:  // reindeer
      case 10212:  // reindeer
        num = 2428;
        break;
      case 10906:  // raccoon
        num = 2430;
        break;
      case 139:  // cyclops
        num = 2436;
        break;
      case 1600:  // stone giant
      case 922:  // stone giant
        num = 2437;
        break;
      case 2100:  // ice worm
      case 2140:  // ice worm
        num = 2438;
        break;
      case 2121:  // ice spider
        num = 2440;
        break;
      case 2402:  // crocodile
      case 10019: // crocodile
        num = 2441;
        break;
      case 3416:  // dragon green
        num = 2442;
        break;
      case 2107:  // dragon ice
        num = 2443;
        break;
      case 4300:  // bullywug
      case 4301:  // bullywug
      case 4302:  // bullywug
      case 4303:  // bullywug
      case 4304:  // bullywug
      case 4305:  // bullywug
      case 4306:  // bullywug
        num = 2444;
        break;
      case 4796:  // dragon-gold
        num = 2445;
        break;
      case 4858:  // dragon-bronze
        num = 2446;
        break;
      case 5120:  // alligator
        num = 2449;
        break;
      case 5114:  // spider
      case 5115:  // spider
      case 5116:  // spider
      case 5117:  // spider
      case 5118:  // spider
      case 7819:  // spider
      case 948:  // spider
      case 9909:  // spider
      case 12434:  // spider
        num = 2450;
        break;
      case 6605:  // black spider
      case 7717:  // black spider
        num = 2456;
        break;
      case 6844:  // dragon-work
        num = 2452;
        break;
      case 6843:  // dragon-gold
        num = 2453;
        break;
      case 7501:  // kamodo
        num = 2455;
        break;
      case 8501:  // wildebeast
        num = 2457;
        break;
      case 12401:  // dragon-silver Kassdedra
        num = 2471;
        break;
      case 12405:  // dragon-silver generic
        num = 2472;
        break;
      case 14360:  // dragon-silver Silubra
        num = 2458;
        break;
      case 7832:  // warthog
      case 8510:  // warthog
        num = 2459;
        break;
      case 8512:  // hippopotamus
        num = 2460;
        break;
      case 8513:  // buffalo
      case 9315:  // buffalo
        num = 2461;
        break;
      case 5726:  // giant-titanium
        num = 2464;
        break;
      case 5383:  // gargoyle
      case 918:  // gargoyle
        num = 2465;
        break;
      case 10220:  // giant-frost
      case 10356:  // giant-frost
        num = 2466;
        break;
      case 10395:  // giant-granite
        num = 2467;
        break;
      case 10500:  // donkey
        num = 2468;
        break;
      case 11450:  // crab-giant
      case 14442:  // crab-blue
        num = 2470;
        break;
      case 12403:  // dragon-red
        num = 2473;
        break;
      case 12404:  // dragon-aquatic Dukashakin
        num = 2474;
        break;
      case Mob::AQUATIC_DRAGON:  // dragon-aquatic
        num = 2479;
        break;
      case 12402:  // leviathin
        num = 2475;
        break;
      case 12413:  // shark
      case 12414:  // shark
      case 12420:  // shark
      case 12431:  // shark
      case 14127:  // shark
        num = 2476;
        break;
      case 23633:  // dragon-cloud
        num = 2477;
        break;
      case 14361:  // firedrake
        num = 2478;
        break;
      case Mob::CHAMELEON:
        num = COMP_POLYMORPH;
        break;
      case Mob::DOPPLEGANGER:
        num = COMP_COPY;
        break;
    }
  }
  if (num == -1) {
    // generic switch based on race
    switch(corpse->getCorpseRace()) {
      case RACE_SIMAL:
        num = 20642;
        break;
      case RACE_SQUIRREL:
        num = 2401;
        break;
      case RACE_DEER:
        num = 2402;
        break;
      case RACE_BEAR:
        num = 2405;
        break;
      case RACE_GRIFFON:
        num = 2406;
        break;
      case RACE_LAMMASU:
        num = 2407;
        break;
      case RACE_SPHINX:
        num = 2408;
        break;
      case RACE_OWLBEAR:
        num = 2409;
        break;
      case RACE_GIRAFFE:
        num = 2412;
        break;
      case RACE_GOPHER:
        num = 2413;
        break;
      case RACE_BADGER:
        num = 2417;
        break;
      case RACE_LION:
        num = 2420;
        break;
      case RACE_OTTER:
        num = 2421;
        break;
      case RACE_CHIMERA:
        num = 2422;
        break;
      case RACE_RABBIT:
        num = 2424;
        break;
      case RACE_COUGAR:
        num = 2429;
        break;
      case RACE_BEAVER:
        num = 2431;
        break;
      case RACE_TIGER:
        num = 2432;
        break;
      case RACE_HORSE:
        num = 2433;
        break;
      case RACE_BOVINE:
        num = 2434;
        break;
      case RACE_OX:
        num = 2435;
        break;
      case RACE_TROLL:
        num = 2439;
        break;
      case RACE_DRAGONNE:
        num = 2447;
        break;
      case RACE_HIPPOGRIFF:
        num = 2448;
        break;
      case RACE_PANTATH:
        num = 2451;
        break;
      case RACE_NAGA:
        num = 2454;
        break;
      case RACE_RHINO:
        num = 2462;
        break;
      case RACE_ELEPHANT:
        num = 2463;
        break;
      case RACE_BOAR:
        num = 2469;
        break;
#if 0
      case RACE_DRAGON:
      case RACE_DINOSAUR:
      case RACE_FISH:
      case RACE_BIRD:
      case RACE_SNAKE: 
      case RACE_PRIMATE:
      case RACE_RODENT:
      case RACE_FISHMAN:
      case RACE_FELINE:
      case RACE_CANINE:
      case RACE_REPTILE:
      case RACE_GOAT:
      case RACE_SHEEP:
      case RACE_WEASEL:
      case RACE_PIG:
      case RACE_TURTLE:
      case RACE_LEOPARD:
      case RACE_FROG:
// this is a 24K talen item, don't enable this without changing the item it gives
        num = 3106;
        break;
#endif
      default:
        return -1;
    }
  }
  return num;
}

void TThing::skinMe(TBeing *ch, const char *arg)
{
  TObj *obj;
  TBaseCorpse *corpse;
  TBeing *dummy;
  int amount, num, rnum, skin_pulses;
  char msg[256], gl_msg[256];

  // Check to see if argument passed exists in room
  if (!generic_find(arg, FIND_OBJ_ROOM, ch, &dummy, &obj)) {
    ch->sendTo(format("You do not see a %s here.\n\r") % arg);
    return;
  }
  // Check to see if corpse is a corpse
  
  if (!(corpse = dynamic_cast<TBaseCorpse *>(obj))) {
    ch->sendTo(COLOR_OBJECTS, format("You cannot skin %s.\n\r") % obj->getName());
    return;
  }
  if (corpse->isCorpseFlag(CORPSE_NO_REGEN)) {
    // a body part or something
    act("$p: You aren't able to skin that.",
          FALSE, ch, corpse, 0, TO_CHAR);    return;
  }
  if (corpse->isCorpseFlag(CORPSE_NO_SKIN)) {
    act("$p: It can't be skinned further.",
          FALSE, ch, corpse, 0, TO_CHAR);
    return;
  }
  if (corpse->isCorpseFlag(CORPSE_PC_SKINNING)) {
    act("$p: Someone else is already skinning this.",
        FALSE, ch, corpse, 0, TO_CHAR);
    return;
  }
  num = determineSkinningItem(corpse, &amount, msg, gl_msg);
  if (num == -1 || !(rnum = real_object(num))) {
    // no item
    act("$p can not be skinned.", FALSE, ch, corpse, 0, TO_CHAR);
    return;
  }
  corpse->addCorpseFlag(CORPSE_PC_SKINNING);
  if (corpse->isCorpseFlag(CORPSE_HALF_SKIN)) {
    act("$p: This has been partly skinned, only half the hide remains.",
        FALSE, ch, corpse, 0, TO_CHAR);
  }

  ch->sendTo("You start skinning the corpse.\n\r");
  act("$n begins skinning the corpse.", FALSE, ch, NULL, 0, TO_ROOM);

  // What we do here is find out how many 'pulses' it will take to skin the chosen
  // corpse.  We then add to that the PC's (Skill-70)/10 to either grant bonus turns to
  // complete or penantly turns.  Then we make sure that is all higher than 1, then add
  // 2 onto that.  This way even a rock bottom example will have at least 3 turns to
  // complete a skin.
  // ***This has been changed so don't rely on the comments, will redo them when I finalize
  // ***this formula.  -Lapsos
  num         = max(1, (int) (ch->getSkillValue(SKILL_SKIN)/25));
  skin_pulses = (corpse->isCorpseFlag(CORPSE_HALF_SKIN)?2:1);
  int lev = ch->getSkillLevel(SKILL_SKIN);
  skin_pulses = 5+
    min(max((int) (lev*2)+((ch->getSkillValue(SKILL_SKIN)-70)/10), 4),
    (int) (((((corpse->getWeight()*.10)/2)/skin_pulses)+1)/num));
  start_task(ch, corpse, NULL, TASK_SKINNING, "", skin_pulses, ch->in_room, 1, 0, 40);
}

void TTool::skinMe(TBeing *ch, const char *arg)
{
  TObj *obj;
  TBaseCorpse *corpse;
  TBeing *dummy;
  int amount, num, rnum, skin_pulses;
  char msg[256], gl_msg[256];

  if (getToolType() != TOOL_SKIN_KNIFE) {
    ch->sendTo("You must be holding a skinning knife to perform this task.\n\r");
    return;
  }
  // Check to see if argument passed exists in room
  if (!generic_find(arg, FIND_OBJ_ROOM, ch, &dummy, &obj)) {
    ch->sendTo(format("You do not see a %s here.\n\r") % arg);
    return;
  }
  // Check to see if corpse is a corpse
  
  if (!(corpse = dynamic_cast<TBaseCorpse *>(obj))) {
    ch->sendTo(COLOR_OBJECTS, format("You cannot skin %s.\n\r") % obj->getName());
    return;
  }
  if (corpse->isCorpseFlag(CORPSE_NO_REGEN)) {
    // a body part or something
    act("$p: You aren't able to skin that.",
          FALSE, ch, corpse, 0, TO_CHAR);    return;
  }
  if (corpse->isCorpseFlag(CORPSE_NO_SKIN)) {
    act("$p: It can't be skinned further.",
          FALSE, ch, corpse, 0, TO_CHAR);
    return;
  }
  num = determineSkinningItem(corpse, &amount, msg, gl_msg);
  if (num == -1 || !(rnum = real_object(num))) {
    // no item
    act("$p can not be skinned.", FALSE, ch, corpse, 0, TO_CHAR);
    return;
  }

  ch->sendTo("You start skinning the corpse.\n\r");
  act("$n begins skinning the corpse.", FALSE, ch, NULL, 0, TO_ROOM);

  // Start skinning task, all skill checks etc are done inside the task
  skin_pulses = 20 + ((100 - max(0, (int) ch->getSkillValue(SKILL_SKIN))) * 4/3);

  start_task(ch, corpse, NULL, TASK_SKINNING, "", skin_pulses, ch->in_room, 1, 0, 40);
}

void TBeing::doSkin(const char *arg)
{
  TThing *tobj;

  for (; isspace(*arg); arg++);

  if (!doesKnowSkill(SKILL_SKIN)) {
    sendTo("You are not wise in the ways of the woods.\n\r");
    return;
  }

  tobj = heldInPrimHand();

  if (!tobj || (!tobj->isPierceWeapon() && !tobj->isSlashWeapon())) {
    sendTo("You must be holding a slash or pierce weapon to perform this task.\n\r");
    return;
  } else if (tobj->getVolume() > 6000) {
    sendTo("I'm afraid that weapon is a bit too big and clumsy to do the job.\n\r");
    return;
  }
  tobj->skinMe(this, arg);
}

void TBeing::doSeekwater()
{
  int dist = 0, code, skill, worked;
  int targrm;
  TThing *t = NULL;
  affectedData aff;
  char buf[256], buf2[256];

  if (affectedBySpell(SKILL_TRACK) || specials.hunting) {
    sendTo("You can't search for water while tracking something else as well.\n\r");
    return;
  }
  skill = getSkillValue(SKILL_SEEKWATER);
  int lev = getSkillLevel(SKILL_SEEKWATER);

  if (skill <= 0) {
    sendTo("You do not have the ability to search for water.\n\r");
    return;
  }
  if (getPosition() < POSITION_STANDING) {
    sendTo("You need to be standing in order to search for a trail.\n\r");
    return;
  }
  if (fight()) {
    sendTo("The ensuing battle makes it difficult to search for a trail.\n\r");
    return;
  }

  // Let's determine the distance:
  // Ranges: 21, ..., 120
  dist = (max(5, lev)/5) * (max(10, skill)/10) + 20;
  dist = lev * max(10, skill);
 
  switch (getRace()) {
    case RACE_ELVEN:
      dist *= 2;                // even better 
      break;
    case RACE_DEVIL:
    case RACE_DEMON:
      dist = MAX_ROOMS;         //  4 as good as can be 
      break;
    default:
      break;
  }
 
  if (isImmortal())
    dist = MAX_ROOMS;
 
  hunt_dist = dist;
  specials.hunting = NULL;
  targrm = inRoom();
  TPathFinder path(dist);
 
  // note: -dist will look THRU doors.
  // all subsequent calls use track() which does not go thru doors
  // this is intentional so they lose track after 1 step
  if (lev < MIN_GLOB_TRACK_LEV){
    path.setStayZone(true);
    code=path.findPath(in_room, findWater());
    targrm=path.getDest();
  } else {
    code=path.findPath(in_room, findWater());
    targrm=path.getDest();
  }
 
  if (code == -1) {
    addToWait(combatRound(1));
    if (targrm == inRoom())
      sendTo("Look!  Water!  It was here all the time.\n\r");
    else
      sendTo("You are unable to find traces of any.\n\r");
    return;
  } else {
    addPlayerAction(PLR_HUNTING);
    skill = getSkillValue(SKILL_SEEKWATER);
    worked = bSuccess(skill, SKILL_SEEKWATER);
    if (worked) {
      if (code <= 9)
        sendTo(format("%sYou see traces of water %s.%s\n\r") % purple() %
               dirs_to_blank[code] % norm());
      else {
        int count = code - 9, seen = 0;
        for(StuffIter it=roomp->stuff.begin();it!=roomp->stuff.end() && (t=*it);++it) {
          TPortal *tp = dynamic_cast<TPortal *>(t);
          if (tp) {
            seen++;
            if (count == seen) {
              sendTo(COLOR_OBJECTS, format("%sYou see traces of water through %s.%s\n\r") % 
                     purple() % tp->getName() % norm());
              break;
            }
          }
        }
        if (!t) {
          sendTo("Error finding path target!  Tell a god.\n\r");
          vlogf(LOG_BUG, "Error finding path (doSeekwater)");
          return;
        }
      }
    } else {
      code = -2;
      sendTo("You begin searching for water.\n\r");
    }
  }

  aff.type = SKILL_SEEKWATER;
  aff.level = getSkillLevel(SKILL_SEEKWATER);
  aff.duration = PERMANENT_DURATION;
  affectTo(&aff);

  if (desc && desc->m_bIsClient)
    desc->clientf(format("%d|%d") % CLIENT_TRACKING % (1 << code));

  if (code <= 9) {
    if (code >= 0 && desc && (desc->autobits & AUTO_HUNT)) {
      strcpy(buf, dirs[code]);
      addCommandToQue(buf);
    }
  } else if (desc && (desc->autobits & AUTO_HUNT) && t) {
      strcpy(buf, t->name);
      strcpy(buf, add_bars(buf).c_str());
      addToWait(combatRound(1));
      sprintf(buf2, "enter %s", buf);
      addCommandToQue(buf2);
  }

  addToWait(combatRound(1));
  addToMove((int) min(10, (-2-((110-getSkillValue(SKILL_SEEKWATER))/6))));

  start_task(this, NULL, NULL, TASK_TRACKING, "", 1, in_room, 1, code+1, 40);
}

// we are going to return 0 if they are not in a camp
// otherwise, return the level of the campers skill
// for groupmates of camper, return fraction of the level.
int TBeing::inCamp() const
{
  TThing *t=NULL;
  TBeing *ch;
  affectedData *aff;

  for (aff = affected; aff; aff = aff->next) {
    if (aff->type == SKILL_ENCAMP)
      return aff->level;
  }

  // this just simplifies the loop below
  if (!isAffected(AFF_GROUP))
    return FALSE;

  for(StuffIter it=roomp->stuff.begin();it!=roomp->stuff.end() && (t=*it);++it) {
    ch = dynamic_cast<TBeing *>(t);
    if (!ch)
      continue;
    if (inGroup(*ch)) {
      for (aff = ch->affected; aff; aff = aff->next) {
        if (aff->type == SKILL_ENCAMP)
          // make sure to return non-zero here
          return max(1, (aff->level/ 2));
      }
    }
  }
  return FALSE;
}

int TBeing::doEncamp()
{
  int rc;

  if (!doesKnowSkill(SKILL_ENCAMP)) {
    sendTo("You know nothing about camping.\n\r");
    return FALSE;
  }
  
  rc = encamp(this);
  if (rc)
    addSkillLag(SKILL_ENCAMP, rc);
  return rc;
}

int encamp(TBeing * caster)
{
  if (!caster || !caster->isPc()) {
    vlogf(LOG_BUG, format("Non-PC in encamp() call.  %s") %  caster->getName());
    return FALSE;
  }

  affectedData aff;
  int level = caster->getSkillLevel(SKILL_ENCAMP);
  int bKnown = caster->getSkillValue(SKILL_ENCAMP);

  if (caster->affectedBySpell(SKILL_ENCAMP)) {
    caster->sendTo("You already have a camp set up here.\n\r");
    return FALSE;
  }
  
  if (caster->roomp->isRoomFlag(ROOM_ON_FIRE)) {
    caster->sendTo("The raging fire here needs to be controlled before setting up camp.\n\r");
    return SPELL_FAIL;
  }
  if (caster->roomp->isRoomFlag(ROOM_FLOODED)) {
    caster->sendTo("The flood in here needs to subside before setting up camp.\n\r");
    return SPELL_FAIL;
  }
  if (caster->roomp->getSectorType() == SECT_ARCTIC_BUILDING
      || caster->roomp->getSectorType() == SECT_ARCTIC_BUILDING
      || caster->roomp->getSectorType() == SECT_ARCTIC_BUILDING
      || (caster->roomp->isRoomFlag(ROOM_INDOORS) && !(caster->roomp->getSectorType() == SECT_TEMPERATE_CAVE || caster->roomp->getSectorType() == SECT_TROPICAL_CAVE || caster->roomp->getSectorType() == SECT_ARCTIC_CAVE))) {
    caster->sendTo("You cannot camp indoors.\n\r");
    return SPELL_FAIL;
  }
  if (caster->roomp->isCitySector()
      || !(caster->roomp->isForestSector()
      || caster->roomp->isBeachSector()
      || caster->roomp->isHillSector()
      || caster->roomp->isMountainSector()
      || caster->roomp->isNatureSector()
      || caster->roomp->isRoadSector()
      || caster->roomp->isSwampSector()
      || caster->roomp->isArcticSector()
      || caster->roomp->getSectorType() == SECT_TEMPERATE_CAVE
      || caster->roomp->getSectorType() == SECT_TROPICAL_CAVE
      || caster->roomp->getSectorType() == SECT_ARCTIC_CAVE)) {
    caster->sendTo("You need to be in nature to camp.\n\r");
    return SPELL_FAIL;
  }
  if (caster->roomp->isFlyingSector()
      || caster->roomp->isVertSector()
      || caster->roomp->isUnderwaterSector()
      || caster->roomp->isAirSector()
      || caster->roomp->isOceanSector()
      || caster->roomp->isRiverSector()) {
    caster->sendTo("This is not a good spot for a camp.\n\r");
    return SPELL_FAIL;
  }
  if (caster->roomp->isRoomFlag(ROOM_NO_FLEE)
      || caster->roomp->isRoomFlag(ROOM_NO_ESCAPE)
      || caster->roomp->isRoomFlag(ROOM_NO_HEAL)
      || caster->roomp->isRoomFlag(ROOM_HAVE_TO_WALK)) {
    caster->sendTo("This room is unfit for a camp.\n\r");
    return SPELL_FAIL;
  }
  
  aff.duration = PERMANENT_DURATION;
  aff.level = level;
  aff.type = SKILL_ENCAMP;
  aff.location = APPLY_NONE;
  aff.modifier = 0;
  aff.bitvector = 0;
  aff.be = caster->roomp;

  if (caster->bSuccess(bKnown, SKILL_ENCAMP)) {
    caster->sendTo("You stop and set up camp.\n\r");
    act("$n stops and begins to camp.", FALSE, caster, 0, 0, TO_ROOM);

    caster->affectTo(&aff);
  } else {
    caster->sendTo("You stop and set up a poorly made camp.\n\r");
    act("$n stops and begins to camp.",
              FALSE, caster, 0, 0, TO_ROOM);
    aff.level /= 2;

    caster->affectTo(&aff);
  }
  return TRUE;
}

void divine(TBeing * caster, TThing *obj)
{
  divine(caster, caster->getSkillLevel(SKILL_DIVINATION), caster->getSkillValue(SKILL_DIVINATION), obj);
}

int divine(TBeing *caster, int x, short y, TThing *obj)
{
  return obj->divineMe(caster, x, y);
}

int TThing::divineMe(TBeing *caster, int, short)
{
  act("$p is not a drink container.", FALSE, caster, this, 0, TO_CHAR);
  return SPELL_FALSE;
}

int TDrinkCon::divineMe(TBeing *caster, int, short bKnown)
{
  affectedData aff;
  int units;
  
  if (!caster || !caster->roomp)
    return SPELL_FAIL;
  
  if (!(caster->roomp->isForestSector()
      || caster->roomp->isBeachSector()
      || caster->roomp->isHillSector()
      || caster->roomp->isMountainSector()
      || caster->roomp->isNatureSector()
      || caster->roomp->isRoadSector()
      || caster->roomp->isSwampSector())) {
    caster->sendTo("You need to be in nature to divine for water.\n\r");
    return SPELL_FAIL;
  } else if (caster->roomp->isIndoorSector() || caster->roomp->isRoomFlag(ROOM_INDOORS)) {
    caster->sendTo("You need to be outside to divine.\n\r");
    return SPELL_FAIL;
  } else if (caster->roomp->isArcticSector() ) {
    caster->sendTo("It is too cold to divine here.\n\r");
    return SPELL_FAIL;
  } else if (caster->roomp->isRoomFlag(ROOM_FLOODED)) {
    caster->sendTo("The flood in here needs to subside before you can divine.\n\r");
    return SPELL_FAIL;
  } else if (caster->roomp->isRoomFlag(ROOM_ON_FIRE)) {
    caster->sendTo("The raging fire here needs to be controlled before you can divine.\n\r");
    return SPELL_FAIL;
  }

  if (caster->affectedBySpell(SKILL_DIVINATION)) {
    act("You are not ready to divine again so soon.", FALSE, caster, NULL, NULL, TO_CHAR);
    return SPELL_FAIL;
  }

  aff.type = SKILL_DIVINATION;
  aff.location = APPLY_NONE;
  aff.duration = 12 * UPDATES_PER_MUDHOUR;
  aff.bitvector = 0;
  aff.modifier = 0;

  caster->affectTo(&aff, -1);

  act("You divine for water.", FALSE, caster, this, 0, TO_CHAR);

  if (caster->bSuccess(bKnown, SKILL_DIVINATION)) {
    units = 10 + caster->getSkillLevel(SKILL_DIVINATION)/10;
    units = min(units, (getMaxDrinkUnits() - getDrinkUnits()));
 
    act("$n divines for water, which $e adds to $p.", FALSE, caster, this, 0, TO_ROOM);

    if ((getDrinkType() != LIQ_WATER) &&
          (getDrinkUnits() != 0)) {
      setDrinkType(LIQ_SLIME);
      act("$p is partially filled (but you won't like what it's filled with!)", FALSE,
          caster, this, 0, TO_CHAR);
    } else if (units > 0) {
      setDrinkType(LIQ_WATER);
      addToDrinkUnits(units);
      weightChangeObject(units * SIP_WEIGHT);
      caster->sendTo(format("You divine %d ounces of water.\n\r") % units);
      if (getDrinkUnits() == getMaxDrinkUnits())
        act("$p is filled.", FALSE, caster, this, 0, TO_CHAR);
      else
        act("$p is partially filled.", FALSE, caster, this, 0, TO_CHAR);
    } else {
      caster->sendTo("That is already completely full!\n\r");
    }
    return SPELL_SUCCESS;
  } else {
    act("You don't seem to find anything.", FALSE, caster, NULL, NULL, TO_CHAR);
    return SPELL_FAIL;
  }
}

void TBeing::doDivine(const char *arg)
{
  TThing *obj;
  char arg2[256];

  if (getPosition() < POSITION_STANDING) {
    sendTo("You need to be standing in order to divine for water.\n\r");
    return;
  }
  if (fight()) {
    sendTo("The ensuing battle makes it difficult to search for water.\n\r");
    return;
  }
 
  one_argument(arg, arg2, cElements(arg2));
  if (!(obj = get_thing_char_using(this, arg2, 0, FALSE, FALSE))) {
    sendTo("You don't have that drink container in your inventory!\n\r");
    sendTo("Syntax: divine <drink container>\n\r");
    return;
  }

  addToWait(combatRound(1));
 
  divine(this, obj);
}

