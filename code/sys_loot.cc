#include "stdsneezy.h"
#include "statistics.h"

#include "sys_loot.h"
#include "obj_money.h"
#include "obj_magic_item.h"
#include "obj_potion.h"
#include "obj_scroll.h"
#include "obj_staff.h"
#include "obj_wand.h"

TLootStructure * tLoot;


// Put objects that should *NOT* be loaded through the LootBooter here.
// List these by VNum, Not RNum
bool isLegalLoot(int tValue)
{
  switch (tValue) {
    case 31300: // mystery potion
    case 29997: // Learning Potion
    case 29993: // Generic Potion
    case 29992: // Youth-Potion
    case 29991: // Stat-Potion
    case 11130: // Sketch Pad [none/none/none]
    case 29990: // Generic Scroll
    case  7816: // Scroll-Withered [portal_quest_prop]
    case  1210: // Frying Pan
    case   831: // Sturdy Glass Flask [none/none/none]
    case 23500: // Warped Staff
      return false;

    default:
      return true;
  }

  return true;
}

// Add names to this list to prevent that way
bool isLegalLoot(const char * tString)
{
  if (isname("[quest]", tString))
    return false;

  if (isname("[prop]", tString))
    return false;

  if (isname("[quest_object]", tString))
    return false;

  return true;
}

// If you add to this list You MUST Add to the one below
bool isLegalLoot(itemTypeT tType)
{
  switch (tType) {
    case ITEM_POTION:
    case ITEM_WAND:
    case ITEM_STAFF:
    case ITEM_SCROLL:
      return true;
    default:
      return false;
  }

  return false;
}

// Add to this list those 'per-entry' things.
bool isSpecialLegalLoot(int tValue)
{
  switch (tValue) {

    default:
      return false;
  }

  return false;
}

// This goes through the object lists and makes a db of possible loot.
bool sysLootBoot()
{
  TObj           * tObj       = NULL;
  TMagicItem     * tMagicItem = NULL;
  TPotion *pot=NULL;

  int tLevel = 0;

  tLoot = NULL;

  for (unsigned int tOIndex = 0; tOIndex < obj_index.size(); tOIndex++)
    if (((isLegalLoot(itemTypeT(obj_index[tOIndex].itemtype)) &&
          isLegalLoot(obj_index[tOIndex].virt) &&
          isLegalLoot(obj_index[tOIndex].name)) ||
          isSpecialLegalLoot(tOIndex)) &&
        obj_index[tOIndex].max_exist == 9999) {
      vlogf(LOG_BUG, fmt("Loot Object Debug, reading object: %d") %  tOIndex);
      tObj = read_object(tOIndex, REAL);

      switch (obj_index[tOIndex].itemtype) { // Set tLevel
        case ITEM_WAND:
        case ITEM_STAFF:
        case ITEM_SCROLL:
          if (!(tMagicItem = dynamic_cast<TMagicItem *>(tObj))) {
            vlogf(LOG_BUG, fmt("Screwup in Loot Loader: %s") %  tObj->getName());
            delete tObj;
            tObj = NULL;
          } else
            tLevel = tMagicItem->getMagicLevel();

          break;
        case ITEM_POTION:
	  if(!(pot = dynamic_cast<TPotion *>(tObj))){
	    vlogf(LOG_BUG, fmt("Screwup in Loot Loader (potion): %s") %  tObj->getName());
	    delete tObj;
	    tObj = NULL;
	  } else
	    tLevel = pot->getDrinkUnits();
	  break;
        default:
          vlogf(LOG_BUG, fmt("sysLootBoot Error: Unrecognized Type: %d") % 
                obj_index[tOIndex].virt);
          delete tObj;
          tObj = NULL;
          break;
      }

      if (tObj) {
        vlogf(LOG_BUG, fmt("Adding New Loot Object: %s") %  tObj->name);
        TLootStructure *tNLoot;

        if (!(tNLoot = new TLootStructure))
          vlogf(LOG_LOW, "Error allocing for TLootStructure");
        else {
          tNLoot->tLevel = tLevel;
          tNLoot->tRNum  = tOIndex;

          tNLoot->tNext = tLoot;
          tLoot = tNLoot;
        }

        delete tObj;
        tObj = NULL;
      }
    }

  return true;
}

bool sysLootLoad(resetCom & rs, TBeing *tBeing, TObj *tObj, bool isImmortal)
{
  // L (if-flag) (level-min) (level-max) (help/harm) (mob/obj/room)
  // (mob/obj/room)
  // 0 = mob
  // 1 = obj
  // > = room

  // Just resume and assume.
  if (!rs.if_flag)
    return false;

  TRoom  * tRoom   = NULL;
  TThing * tThing  = NULL;
  int      tFound  = 0;
  bool     tLoaded = false,
           cashLoaded = false;

  if (rs.arg4 == 0) {
    if (!(tThing = tBeing)) {
      vlogf(LOG_LOW, fmt("L used on Mobile failed: %d") %  rs.arg4);
      return false;
    }
  } else if (rs.arg4 == 1) {
    if (!(tThing = tObj)) {
      vlogf(LOG_LOW, fmt("L used on Object failed: %d") %  rs.arg4);
      return false;
    }
  } else if (!(tThing = (tRoom = real_roomp(rs.arg4)))) {
    vlogf(LOG_LOW, fmt("L used on Room failed: %d") %  rs.arg4);
    return false;
  }

  for (TLootStructure * tTLoot = tLoot; tTLoot; tTLoot = tTLoot->tNext)
    if (in_range(tTLoot->tLevel, rs.arg1, rs.arg2))
      tFound++;

  if (!isImmortal)
    tFound = (rand() % (tFound + 2));

  for (TLootStructure * tTLoot = tLoot; tTLoot; tTLoot = tTLoot->tNext)
    if (in_range(tTLoot->tLevel, rs.arg1, rs.arg2) &&
        (isImmortal || --tFound == 0)) {
      TObj *tObj;

      if (obj_index[tTLoot->tRNum].getNumber() >=
          obj_index[tTLoot->tRNum].max_exist)
        continue;

      tLoaded = true;

      unsigned int tObjn = tTLoot->tRNum;

      if (tObjn < 0 || tObjn >= obj_index.size() ||
          !(tObj = read_object(tObjn, REAL))) {
        vlogf(LOG_LOW, fmt("Error in Loot Loader: %d") %  tTLoot->tRNum);
        return false;
      }

      *tThing += *tObj;

      if (!isImmortal)
        return true;
    } else if (!isImmortal && !cashLoaded && tFound <= 0) {
      // We end up here if the object was maxed or we hit one of the bonus 2
      // in this case we decide a talen amount and do that instead.

      float tCashValue = (rs.arg1 + (rand() % (rs.arg2 - rs.arg1)));

      /*
       Level  1:    100
       Level  2:    400
       Level 10:  2,000
       Level 50: 10,000
       */

      tCashValue *= 200.0;
      tCashValue *= gold_modifier[GOLD_INCOME].getVal();
      cashLoaded = true;

      if (rs.arg4 == 0)
        tBeing->setMoney((tBeing->getMoney() + (int) tCashValue));
      else {
        TMoney * tMoney;

        if (!(tMoney = create_money((int) tCashValue))) {
          vlogf(LOG_BUG, "Problem creating money");
          return false;
        }

        *tThing += *tMoney;
      }

    }

  return tLoaded;
}

bool hasSpellOnIt(TMagicItem * tObj, spellNumT tSpell)
{
  TScroll * tScroll = dynamic_cast<TScroll *>(tObj);
  TWand   * tWand   = dynamic_cast<TWand   *>(tObj);
  TStaff  * tStaff  = dynamic_cast<TStaff  *>(tObj);

  if (tScroll) {
    if (tScroll->getSpell(0) == tSpell ||
        tScroll->getSpell(1) == tSpell ||
        tScroll->getSpell(2) == tSpell)
      return true;
  } else if (tWand) {
    if (tWand->getSpell() == tSpell)
      return true;
  } else if (tStaff) {
    if (tStaff->getSpell() == tSpell)
      return true;
  }

  return false;
}

void TBeing::doLoot(const sstring & tStString)
{
  if (!desc || !isImmortal())
    return;

  if (!hasWizPower(POWER_WIZARD)) {
    sendTo("You don't have the power to do this.  Sorry.\n\r");
    return;
  }

  sstring tStOutput(""),
         tStBuffer(""),
         tStArg(tStString),
         tStCommand(""),
         tStType(""),
         tStSpell(""),
         tStLevelMin(""),
         tStLevelMax("");
  char   tString[256];
  int    tLevelMin =  -1,
         tLevelMax = 101;
  TObj  *tObj = NULL;
  bool   bType  = false,
         bSpell = false;

  itemTypeT tType  = ITEM_UNDEFINED;
  spellNumT tSpell = TYPE_UNDEFINED;

  tStArg = one_argument(tStArg, tStCommand);

  if (is_abbrev(tStCommand, "list")) {
    tStArg = one_argument(tStArg, tStBuffer);

    while (!tStBuffer.empty()) {
      strcpy(tString, tStBuffer.c_str());

      if (is_number(tString)) {
        if (tLevelMin == -1) {
          tLevelMin = convertTo<int>((tStLevelMin = tStBuffer));

          if (tLevelMin < 0 || tLevelMin > 99) {
            sendTo("Incorrect Min Level.  (0-99)\n\r");
            return;
          }
        } else {
          tLevelMax = convertTo<int>((tStLevelMax = tStBuffer));

          if (tLevelMax < 1 || tLevelMax > 100) {
            sendTo("Incorrect Max Level.  (1-100)\n\r");
            return;
          }
        }

        if (tLevelMin > tLevelMax) {
          sendTo("Min-Level must be greater than Max-Level.\n\r");
          return;
        }
      } else if (is_abbrev(tStBuffer, "type")) {
        tStArg = one_argument(tStArg, tStType);
        bType = true;

        if (tStType.empty()) {
          sendTo("Must provide a type with this switch(item type)\n\r");
          return;
        }
      } else if (is_abbrev(tStBuffer, "spell")) {
        tStArg = one_argument(tStArg, tStSpell);
        bSpell = true;

        if (tStSpell.empty()) {
          sendTo("Must provide a spell with this switch(spell name)\n\r");
          return;
        }
      } else {
        sendTo("Syntax Error.  See Help File.\n\r");
        return;
      }

      tStArg = one_argument(tStArg, tStBuffer);
    }

    if (bType) {
      for (tType = MIN_OBJ_TYPE; tType < MAX_OBJ_TYPES; tType++)
        if (ItemInfo[tType] && isname(ItemInfo[tType]->name, tStType))
          break;

      if (tType == MAX_OBJ_TYPES) {
        sendTo("Incorrect Item Type.\n\r");
        return;
      }
    }

    if (bSpell) {
      for (tSpell = MIN_SPELL; tSpell < MAX_SKILL; tSpell++)
        if (discArray[tSpell] &&
            isname(discArray[tSpell]->name, tStSpell) &&
            (discArray[tSpell]->typ == SPELL_MAGE   ||
             discArray[tSpell]->typ == SPELL_CLERIC))
          break;

      if (tSpell == MAX_SKILL) {
        sendTo("Incorrect Spell Type.\n\r");
        return;
      }
    }

    if (tLevelMin == -1)
      tLevelMin = 0;

    if (tLevelMax == 101)
      tLevelMax = 100;

    tStOutput  = "Loot: Min:";
    tStOutput += tStLevelMin;
    tStOutput += " Max:";
    tStOutput += tStLevelMax;
    tStOutput += "\n\r__________\n\r";

    int tTotalCount = 0;

    for (TLootStructure * tNLoot = tLoot; tNLoot; tNLoot = tNLoot->tNext)
      if (in_range(tNLoot->tLevel, tLevelMin, tLevelMax))
        if ((tObj = read_object(tNLoot->tRNum, REAL))) {
          if (tType != ITEM_UNDEFINED &&
              obj_index[tNLoot->tRNum].itemtype != tType) {
            delete tObj;
            tObj = NULL;
            continue;
          }

          TMagicItem *tMagicItem = dynamic_cast<TMagicItem *>(tObj);

          if (tSpell != TYPE_UNDEFINED &&
              !hasSpellOnIt(tMagicItem, tSpell)) {
            delete tObj;
            tObj = NULL;
            continue;
          }

          sprintf(tString, "%6d: %3d: %s\n\r",
                  obj_index[tNLoot->tRNum].virt,
                  tNLoot->tLevel,
                  tObj->getNameForShow(false, true, this).c_str());

          tTotalCount++;
          tStOutput += tString;
          delete tObj;
          tObj = NULL;
        }

    tStOutput += "__________\n\r";
    tStOutput += "Total Count: ";
    sprintf(tString, "%d\n\r", tTotalCount);
    tStOutput += tString;

    desc->page_string(tStOutput);
    return;
  } else if (is_abbrev(tStCommand, "load")) {
    tStArg = one_argument(tStArg, tStLevelMin);
    tStArg = one_argument(tStArg, tStLevelMax);

    resetCom tRs;
    tRs.if_flag = 1;
    tRs.arg1 = convertTo<int>(tStLevelMin);
    tRs.arg2 = convertTo<int>(tStLevelMax);
    tRs.arg3 = 0;
    tRs.arg4 = 0;

    if (sysLootLoad(tRs, this, NULL, true)) {
      act("You make some funky gestures and load some loot.",
          FALSE, this, NULL, NULL, TO_CHAR);
      act("$n makes some funky gestures and loads some loot.",
          TRUE, this, NULL, NULL, TO_ROOM);
    }

    return;
  }

  sendTo("Syntax Error.  See Help File.\n\r");
}
