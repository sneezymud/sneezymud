#include "handler.h"
#include "being.h"
#include "obj_cookware.h"
#include "task_cook.h"
#include "obj_pool.h"
#include "obj_corpse.h"
#include "obj_food.h"
#include "obj.h"
#include "materials.h"


bool check_ingredients(TCookware* pot, int recipe) {
  int nfound = 0;
  TThing* t = NULL;
  TPool* pool;
  TCorpse* corpse;
  TObj* obj;

  // check basic ingredients
  for (int i = 0; ingredients[i].recipe >= 0; ++i) {
    if (ingredients[i].recipe != recipe)
      continue;

    nfound = 0;

    for (int j = i; ingredients[j].recipe >= 0 &&
                    ingredients[j].ingredient == ingredients[i].ingredient;
         ++j) {
      // look for this ingredient
      for (StuffIter it = pot->stuff.begin();
           it != pot->stuff.end() && (t = *it); ++it) {
        switch (ingredients[i].type) {
          case TYPE_VNUM:
            if (obj_index[t->number].virt == ingredients[i].num)
              nfound++;
            break;
          case TYPE_LIQUID:
            if ((pool = dynamic_cast<TPool*>(t)) &&
                pool->getDrinkType() == ingredients[i].num)
              nfound += pool->getDrinkUnits();
            break;
          case TYPE_MATERIAL:
            if (t->getMaterial() == ingredients[i].num)
              nfound++;
            break;
          case TYPE_CORPSE:
            if ((corpse = dynamic_cast<TCorpse*>(t)) &&
                corpse->getCorpseRace() == ingredients[i].num)
              nfound++;
            break;
          case TYPE_ITEM:
            if ((obj = dynamic_cast<TObj*>(t)) &&
                obj->itemType() == ingredients[i].num)
              nfound++;
            break;
        }
      }

      i = j;
    }

    if (nfound < ingredients[i].amt) {
      return false;
    }
  }

  return true;
}

TCookware* find_pot(TBeing* ch, const sstring& cookware) {
  TThing* tpot = NULL;
  TCookware* pot = NULL;
  int count = 0;

  if ((tpot = searchLinkedListVis(ch, cookware, ch->stuff, &count))) {
    pot = dynamic_cast<TCookware*>(tpot);
  }

  return pot;
}

int find_recipe(sstring recipearg) {
  // find which recipe - returns array index, not recipe code
  for (int i = 0; recipes[i].recipe >= 0; ++i) {
    if (isname(recipearg, recipes[i].keywords)) {
      return i;  // return the index, not the recipe code
    }
  }

  return -1;  // not found
}

void TBeing::doCook(sstring arg) {
  int recipe_index = -1;
  TCookware* pot = NULL;
  sstring cookware, recipearg, tmparg = arg;

  tmparg = one_argument(tmparg, cookware);
  tmparg = one_argument(tmparg, recipearg);

  if (!(pot = find_pot(this, cookware))) {
    sendTo("You need to specify a piece of cookware to use.\n\r");
    return;
  }

  if ((recipe_index = find_recipe(recipearg)) == -1) {
    sendTo("You need to specify a recipe.\n\r");
    return;
  }

  // check ingredients - pass recipe code, not index
  if (!check_ingredients(pot, recipes[recipe_index].recipe)) {
    sendTo("You seem to be missing an ingredient.\n\r");
    return;
  }

  // delete the ingredients and then place the finished product in the pot
  // if the cooking fails we delete the finished product later
  for (StuffIter it = pot->stuff.begin(); it != pot->stuff.end();) {
    TThing* t = *(it++);
    delete t;
  }
  TObj* o;
  if ((o = read_object(recipes[recipe_index].vnum, VIRTUAL)))
    *pot += *o;
  else {
    sendTo("Error loading food, alert an admin.\n\r");
    return;
  }

  sendTo(COLOR_BASIC,
    format("You begin to cook %s.\n\r") % recipes[recipe_index].name);
  start_task(this, pot, NULL, TASK_COOK, "", recipes[recipe_index].difficulty, inRoom(), recipe_index, 0, 5);
}

double getCookingDifficultyScale(taskDiffT diff) {
  switch(diff) {
    case TASK_HOPELESS:    return .80;   // Most exp - hardest task
    case TASK_DANGEROUS:   return .65;
    case TASK_DIFFICULT:   return .50;
    case TASK_NORMAL:      return .35;
    case TASK_EASY:        return .25;
    case TASK_TRIVIAL:     return .20;   // Least exp - easiest task
    default:               return .35;   // Normal as default
  }
}

int task_cook(TBeing* ch, cmdTypeT cmd, const char*, int pulse, TRoom*, TObj* pot) {
  int recipe_index = ch->task->status;
  const int MAX_TIME = 50; // Prevent infinite failure loop
  
  // Basic validation
  if (recipe_index < 0 || recipes[recipe_index].recipe < 0) {
    ch->sendTo("Something went wrong with the recipe.\n\r");
    ch->stopTask();
    return FALSE;
  }

  // basic tasky safechecking
  if (ch->isLinkdead() || (ch->in_room != ch->task->wasInRoom) || !pot) {
    act("You stop cooking.", FALSE, ch, 0, 0, TO_CHAR);
    act("$n stops cooking.", TRUE, ch, 0, 0, TO_ROOM);
    ch->stopTask();

    if (!pot->stuff.empty()) {
      delete pot->stuff.front();
    }
    
    return FALSE;
  }

  if (ch->utilityTaskCommand(cmd) || ch->nobrainerTaskCommand(cmd)) {
    return FALSE;
  }

  if (ch->task->timeLeft < 0) {
    act("You finish cooking.", FALSE, ch, pot, 0, TO_CHAR);
    act("$n finishes cooking.", TRUE, ch, pot, 0, TO_ROOM);
    
    // Verify the food object matches our recipe
    TObj* food = dynamic_cast<TObj*>(pot->stuff.front());
   if (!food || obj_index[food->number].virt != recipes[recipe_index].vnum) {
      ch->sendTo("Something went wrong with the cooking.\n\r");
      ch->stopTask();
      if (food) delete food;
      return FALSE;
    }
    
    if (ch->bSuccess(ch->getSkillValue(SKILL_COOK), SKILL_COOK)) {
      ch->sendTo(format("You successfully prepare %s.\n\r") % recipes[recipe_index].name);
      int diffValue = recipes[recipe_index].difficulty * 10;
      if (diffValue <= 1)
        diffValue = 5;
      
      // Apply difficulty scaling to experience gain
      double scaleFactor = getCookingDifficultyScale(recipes[recipe_index].difficulty);
      ch->gainTaskExp(diffValue, scaleFactor);
    } else {
      ch->sendTo("Your cooking attempt fails, ruining the ingredients.\n\r");
      if (recipes[recipe_index].difficulty >= TASK_DIFFICULT) {
        ch->sendTo("You burn yourself in the process!\n\r");
        int dam = 1 + number(recipes[recipe_index].difficulty, recipes[recipe_index].difficulty * 5);
        int rc = ch->reconcileDamage(ch, dam, DAMAGE_FIRE);
        if (rc == -1)
          return DELETE_THIS;
      }
      delete food;
      TObj* burnt = read_object(Obj::GENERIC_COMMODITY, VIRTUAL);
      
      if (burnt) {
        burnt->setMaterial(MAT_POWDER);  // Ash
        *pot += *burnt;
      }
    }
    
    ch->doSave(SILENT_YES);
    ch->stopTask();
    return FALSE;
  }

  switch (cmd) {
    case CMD_TASK_CONTINUE:
      ch->task->calcNextUpdate(pulse, Pulse::MOBACT * 3);

      if (ch->bSuccess(ch->getSkillValue(SKILL_COOK), SKILL_COOK)) {
        // More skilled = more time removed
        int timeReduction = 1;
        if (ch->getSkillValue(SKILL_COOK) > 50) timeReduction++;
        if (ch->getSkillValue(SKILL_COOK) > 75) timeReduction++;
        ch->task->timeLeft -= timeReduction;
        
        switch (ch->task->timeLeft) {
          case 2:
          case 1:
            act("You prepare the ingredients in $p.", FALSE, ch, pot, 0, TO_CHAR);
            act("$n prepares $s ingredients.", TRUE, ch, pot, 0, TO_ROOM);
            break;
          case 0:
          case -1:
            act("You cook the ingredients in $p.", FALSE, ch, pot, 0, TO_CHAR);
            act("$n cooks the ingredients in $p.", TRUE, ch, pot, 0, TO_ROOM);
            break;
        }
      } else {
        // Higher difficulty = more time added on failure
        int penalty = (recipes[recipe_index].difficulty + 1) / 2;
        
        if (ch->task->timeLeft < MAX_TIME) {
          ch->task->timeLeft += penalty;
          act("You make a mistake and ruin some of your work.", FALSE, ch, pot, 0, TO_CHAR);
          act("$n makes a mistake while cooking.", TRUE, ch, pot, 0, TO_ROOM);
          
          if (recipes[recipe_index].difficulty >= TASK_DIFFICULT) {
            ch->sendTo("You burn yourself in the process!\n\r");
            int dam = 1 + number(recipes[recipe_index].difficulty, recipes[recipe_index].difficulty * 5);
            int rc = ch->reconcileDamage(ch, dam, DAMAGE_FIRE);
            if (rc == -1)
              return DELETE_THIS;
          }
        } else {
          // Task automatically fails if it takes too long
          ch->sendTo("You've completely ruined this cooking attempt.\n\r");
          ch->stopTask();
          if (!pot->stuff.empty())
            delete pot->stuff.front();
          TObj* burnt = read_object(Obj::GENERIC_COMMODITY, VIRTUAL);
          if (burnt) {
            burnt->setMaterial(MAT_POWDER);  // Ash
            *pot += *burnt;
          }
          return FALSE;
        }
      }
      break;

    case CMD_ABORT:
    case CMD_STOP:
      act("You stop cooking.", FALSE, ch, 0, 0, TO_CHAR);
      act("$n stops cooking.", TRUE, ch, 0, 0, TO_ROOM);
      ch->stopTask();
      if (!pot->stuff.empty())
        delete pot->stuff.front();
      break;

    case CMD_TASK_FIGHTING:
      ch->sendTo("You can't properly cook while under attack.\n\r");
      ch->stopTask();
      if (!pot->stuff.empty())
        delete pot->stuff.front();
      break;

    default:
      if (cmd < MAX_CMD_LIST)
        warn_busy(ch);
      if (!pot->stuff.empty())
        delete pot->stuff.front();
      break;
  }
  return TRUE;
}
