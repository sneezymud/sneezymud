#include "stdsneezy.h"
#include "shop.h"
#include "shopowned.h"
#include "corporation.h"

bool TObj::isPluralItem() const
{
  return FALSE;
}

bool TObj::engraveMe(TBeing *, TMonster *, bool)
{
  return FALSE;
}

bool TObj::isUnique() const
{
  return (!obj_index[getItemIndex()].getNumber());
}

int TObj::objVnum() const
{
  return ((number < 0) ? -1 : obj_index[number].virt);
}

int TObj::getValue() const
{
  return obj_flags.cost;
}

bool TObj::isObjStat(unsigned int num) const
{
  return (obj_flags.extra_flags & num) != 0;
}

unsigned int TObj::getObjStat() const
{
  return obj_flags.extra_flags;
}

void TObj::setObjStat(unsigned int num)
{
  obj_flags.extra_flags = num;
}

void TObj::remObjStat(unsigned int num)
{
  obj_flags.extra_flags &= ~num;
}

void TObj::addObjStat(unsigned int num)
{
  obj_flags.extra_flags |= num;
}

bool TObj::isPaired() const
{
  return (isObjStat(ITEM_PAIRED));
}

bool TObj::usedAsPaired() const
{
  return (isPaired() &&
          (eq_pos == WEAR_LEGS_L || eq_pos == WEAR_LEGS_R ||
           eq_pos == WEAR_EX_LEG_R || eq_pos == WEAR_EX_LEG_L ||
           eq_pos == HOLD_RIGHT || eq_pos == HOLD_LEFT));
}

bool TObj::isLevitating() const
{
  return isObjStat(ITEM_HOVER);
}

bool TObj::fitInShop(const char *buf3, const TBeing *) const
{
  return (!strcmp(buf3, "yes") || 
          !strcmp(buf3, "N/A") || 
          !strcmp(buf3, "paired") ||
          !strcmp(buf3, "either hand") || 
          !strcmp(buf3, "secondary only") || 
          !strcmp(buf3, "primary only"));
}

void TObj::swapToStrung()
{
  if (isObjStat(ITEM_STRUNG))
    return;

  addObjStat(ITEM_STRUNG);
  name = mud_str_dup(obj_index[getItemIndex()].name);
  shortDescr = mud_str_dup(obj_index[getItemIndex()].short_desc);
  if (obj_index[getItemIndex()].long_desc)
    setDescr(mud_str_dup(obj_index[getItemIndex()].long_desc));
  else
    setDescr(NULL);
  if (obj_index[getItemIndex()].description)
    action_description = mud_str_dup(obj_index[getItemIndex()].description);
  else
    action_description = NULL;

  if (obj_index[getItemIndex()].ex_description)
    ex_description =
      new extraDescription(*obj_index[getItemIndex()].ex_description);
  else
    ex_description = NULL;
}

sstring TObj::getNameForShow(bool useColor, bool useName, const TBeing *ch) const
{
  return useName ? name : (useColor ? getName() : getNameNOC(ch));
}

itemTypeT mapFileToItemType(int num)
{
  switch (num) {
    case 0:
      return ITEM_UNDEFINED;
    case 1:
      return ITEM_LIGHT;
    case 2:
      return ITEM_SCROLL;
    case 3:
      return ITEM_WAND;
    case 4:
      return ITEM_STAFF;
    case 5:
      return ITEM_WEAPON;
    case 6:
      return ITEM_FUEL;
    case 7:
      return ITEM_OPAL;
    case 8:
      return ITEM_TREASURE;
    case 9:
      return ITEM_ARMOR;
    case 10:
      return ITEM_POTION;
    case 11:
      return ITEM_WORN;
    case 12:
      return ITEM_OTHER;
    case 13:
      return ITEM_TRASH;
    case 14:
      return ITEM_TRAP;
    case 15:
      return ITEM_CHEST;
    case 16:
      return ITEM_NOTE;
    case 17:
      return ITEM_DRINKCON;
    case 18:
      return ITEM_KEY;
    case 19:
      return ITEM_FOOD;
    case 20:
      return ITEM_MONEY;
    case 21:
      return ITEM_PEN;
    case 22:
      return ITEM_BOAT;
    case 23:
      return ITEM_AUDIO;
    case 24:
      return ITEM_BOARD;
    case 25:
      return ITEM_BOW;
    case 26:
      return ITEM_ARROW;
    case 27:
      return ITEM_BAG;
    case 28:
      return ITEM_CORPSE;
    case 29:
      return ITEM_SPELLBAG;
    case 30:
      return ITEM_COMPONENT;
    case 31:
      return ITEM_BOOK;
    case 32:
      return ITEM_PORTAL;
    case 33:
      return ITEM_WINDOW;
    case 34:
      return ITEM_TREE;
    case 35:
      return ITEM_TOOL;
    case 36:
      return ITEM_HOLY_SYM;
    case 37:
      return ITEM_QUIVER;
    case 38:
      return ITEM_BANDAGE;
    case 39:
      return ITEM_STATUE;
    case 40:
      return ITEM_BED;
    case 41:
      return ITEM_TABLE;
    case 42:
      return ITEM_RAW_MATERIAL;
    case 43:
      return ITEM_GEMSTONE;
    case 44:
      return ITEM_MARTIAL_WEAPON;
    case 45:
      return ITEM_JEWELRY;
    case 46:
      return ITEM_VIAL;
    case 47:
      return ITEM_PCORPSE;
    case 48:
      return ITEM_POOL;
    case 49:
      return ITEM_KEYRING;
    case 50:
      return ITEM_RAW_ORGANIC;
    case 51:
      return ITEM_FLAME;
    case 52:
      return ITEM_APPLIED_SUB;
    case 53:
      return ITEM_SMOKE;
    case 54:
      return ITEM_ARMOR_WAND;
    case 55:
      return ITEM_DRUG_CONTAINER;
    case 56:
      return ITEM_DRUG;
    case 57:
      return ITEM_GUN;
    case 58:
      return ITEM_AMMO;
    case 59:
      return ITEM_PLANT;
    case 60:
      return ITEM_COOKWARE;
    case 61:
      return ITEM_VEHICLE;
    case 62:
      return ITEM_CASINO_CHIP;
    case 63:
      return ITEM_POISON;
    case 64:
      return ITEM_HANDGONNE;
    case 65:
      return ITEM_EGG;
    case 66:
      return ITEM_CANNON;
    case 67:
      return ITEM_TOOTH_NECKLACE;
    case 68:
      return ITEM_TRASH_PILE;
  }
  vlogf(LOG_BUG, fmt("Unknown type %d in map file") %  num);
  return ITEM_UNDEFINED;
}

int mapItemTypeToFile(itemTypeT itt)
{
  switch (itt) {
    case ITEM_UNDEFINED:
      return 0;
    case ITEM_LIGHT:
      return 1;
    case ITEM_SCROLL:
      return 2;
    case ITEM_WAND:
      return 3;
    case ITEM_STAFF:
      return 4;
    case ITEM_WEAPON:
      return 5;
    case ITEM_FUEL:
      return 6;
    case ITEM_OPAL:
      return 7;
    case ITEM_TREASURE:
      return 8;
    case ITEM_ARMOR:
      return 9;
    case ITEM_POTION:
      return 10;
    case ITEM_WORN:
      return 11;
    case ITEM_OTHER:
      return 12;
    case ITEM_TRASH:
      return 13;
    case ITEM_TRAP:
      return 14;
    case ITEM_CHEST:
      return 15;
    case ITEM_NOTE:
      return 16;
    case ITEM_DRINKCON:
      return 17;
    case ITEM_KEY:
      return 18;
    case ITEM_FOOD:
      return 19;
    case ITEM_MONEY:
      return 20;
    case ITEM_PEN:
      return 21;
    case ITEM_BOAT:
      return 22;
    case ITEM_AUDIO:
      return 23;
    case ITEM_BOARD:
      return 24;
    case ITEM_BOW:
      return 25;
    case ITEM_ARROW:
      return 26;
    case ITEM_BAG:
      return 27;
    case ITEM_CORPSE:
      return 28;
    case ITEM_SPELLBAG:
      return 29;
    case ITEM_COMPONENT:
      return 30;
    case ITEM_BOOK:
      return 31;
    case ITEM_PORTAL:
      return 32;
    case ITEM_WINDOW:
      return 33;
    case ITEM_TREE:
      return 34;
    case ITEM_TOOL:
      return 35;
    case ITEM_HOLY_SYM:
      return 36;
    case ITEM_QUIVER:
      return 37;
    case ITEM_BANDAGE:
      return 38;
    case ITEM_STATUE:
      return 39;
    case ITEM_BED:
      return 40;
    case ITEM_TABLE:
      return 41;
    case ITEM_RAW_MATERIAL:
      return 42;
    case ITEM_GEMSTONE:
      return 43;
    case ITEM_MARTIAL_WEAPON:
      return 44;
    case ITEM_JEWELRY:
      return 45;
    case ITEM_VIAL:
      return 46;
    case ITEM_PCORPSE:
      return 47;
    case ITEM_POOL:
      return 48;
    case ITEM_KEYRING:
      return 49;
    case ITEM_RAW_ORGANIC:
      return 50;
    case ITEM_FLAME:
      return 51;
    case ITEM_APPLIED_SUB:
      return 52;
    case ITEM_SMOKE:
      return 53;
    case ITEM_ARMOR_WAND:
      return 54;
    case ITEM_DRUG_CONTAINER:
      return 55;
    case ITEM_DRUG:
      return 56;
    case ITEM_GUN:
      return 57;
    case ITEM_AMMO:
      return 58;
    case ITEM_PLANT:
      return 59;
    case ITEM_COOKWARE:
      return 60;
    case ITEM_VEHICLE:
      return 61;
    case ITEM_CASINO_CHIP:
      return 62;
    case ITEM_POISON:
      return 63;
    case ITEM_HANDGONNE:
      return 64;
    case ITEM_EGG:
      return 65;
    case ITEM_CANNON:
      return 66;
    case ITEM_TOOTH_NECKLACE:
      return 67;
    case ITEM_TRASH_PILE:
      return 68;
    case MAX_OBJ_TYPES:
      break;
  }
  vlogf(LOG_BUG, fmt("Unknown type %d in map item") %  itt);
  return 0;
}

itemTypeT & operator++(itemTypeT &c, int)
{
  return c = (c == MAX_OBJ_TYPES) ? MIN_OBJ_TYPE : itemTypeT(c+1);

}
void TObj::lowCheck()
{
  int i;

  if (!getVolume() && canWear(ITEM_TAKE))
    vlogf(LOG_LOW,fmt("item (%s:%d) had 0 volume.") % getName() % objVnum());

  // not sure logically, but would a canWear(ITEM_TAKE) check be appropriate here?
  // allow APPLY_LIGHT on untakeable things?
  // ^ yes... Maror 08/04
  for (i=0; i<MAX_OBJ_AFFECT;i++) {
    if (affected[i].location == APPLY_LIGHT && canWear(ITEM_TAKE)) {
      vlogf(LOG_LOW,fmt("item %s was defined apply-light.") % getName());
    }
  }
}

bool TObj::lowCheckSlots(silentTypeT silent)
{
  unsigned int ui;

  // check for multiple wear slots
  unsigned int value = obj_flags.wear_flags;
  REMOVE_BIT(value, ITEM_TAKE);
  REMOVE_BIT(value, ITEM_THROW);

  for (ui = 0; ui < MAX_ITEM_WEARS; ui++) {
    if (IS_SET(value, (unsigned) (1<<ui)))
      if (value != (unsigned) (1<<ui)) {
        if (!silent)
          vlogf(LOG_LOW, fmt("item (%s) with multiple wear slots: %s") % 
               getName() % wear_bits[ui]);
        return true;
      }
  }
  return false;
}

void TObj::waterCreate(const TBeing *caster, int)
{
  caster->nothingHappens();
}

void TObj::fillMe(const TBeing *ch, liqTypeT)
{
  ch->sendTo("That's not a drink container!\n\r");
}

void TObj::addGlowEffects()
{
  canBeSeen -= (1 + getVolume()/1500);

  int i;
  for (i=0; i<MAX_OBJ_AFFECT;i++) {
    if (affected[i].location == APPLY_NONE) {
      affected[i].location = APPLY_LIGHT;
      affected[i].modifier = (1 + getVolume()/6000);
      addToLight(affected[i].modifier);
      if (affected[i].modifier > 5 && canWear(ITEM_TAKE))
        vlogf(LOG_LOW,fmt("Mega light on %s") % getName());
      break;
    } else if (i==(MAX_OBJ_AFFECT-1))
      vlogf(LOG_LOW,fmt("obj %s has too many affects to set glow on it.") % 
             getName());
  }
}

double TObj::objLevel() const
{
  return 0.0;
}

void TObj::purchaseMe(TBeing *ch, TMonster *keeper, int cost, int shop_nr)
{
  ch->giveMoney(keeper, cost, GOLD_SHOP);


  shoplog(shop_nr, ch, keeper, getName(), cost, "buying");

  if(shop_index[shop_nr].isOwned()){
    TShopOwned tso(shop_nr, keeper, ch);

    tso.doDividend(this, cost);
    tso.doReserve();
    tso.chargeTax(this, cost);
  }
}

void TObj::sellMeMoney(TBeing *ch, TMonster *keeper, int cost, int shop_nr)
{
  keeper->giveMoney(ch, cost, GOLD_SHOP);

  shoplog(shop_nr, ch, keeper, getName(), -cost, "selling");

  if(shop_index[shop_nr].isOwned()){
    TShopOwned tso(shop_nr, keeper, ch);

    tso.doReserve();
  }
}

void TObj::peeOnMe(const TBeing *ch)
{
  act("With no apparent shame, $n happily relieves $mself on $p.",
             TRUE, ch, this, NULL, TO_ROOM);
  ch->sendTo("Ok, but have you no pride?\n\r");
}

void TObj::extinguishWater(TBeing *ch)
{
  if (isObjStat(ITEM_BURNING)){
    remBurning(ch);

    act("$p is put out by the room's water.", TRUE, ch, this, 0, TO_CHAR);
    act("$p is put out by the room's water.", TRUE, ch, this, 0, TO_ROOM);
  }
}

void TObj::extinguishWater()
{
  if (isObjStat(ITEM_BURNING)) {
    remBurning(NULL);
    act("$p is put out by the room's water.", TRUE, 0, this, 0, TO_ROOM);
  }
}

bool TObj::canGetMeDeny(const TBeing *, silentTypeT) const
{
  return false;
}

int TObj::galvanizeMe(TBeing *caster, byte bKnown)
{
  caster->sendTo("You can't galvanize that!\n\r");
  caster->nothingHappens(SILENT_YES);
  return SPELL_FAIL;
}

void TObj::onObjLoad()
{
}
