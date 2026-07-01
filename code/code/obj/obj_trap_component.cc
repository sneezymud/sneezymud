//////////////////////////////////////////////////////////////////////////
//
// SneezyMUD - All rights reserved, SneezyMUD Coding Team
//
//////////////////////////////////////////////////////////////////////////

#include "handler.h"
#include "extern.h"
#include "room.h"
#include "being.h"
#include "monster.h"
#include "obj_trap_component.h"
#include "shop.h"
#include "shopowned.h"
#include "database.h"
#include "materials.h"
#include "comm.h"

TThing::TThingKind TTrapComponent::getKind() const {
  return TThing::TThingKind::TTrapComponent;
}

TTrapComponent::TTrapComponent() :
  TMergeable(),
  charges(1),
  trap_comp_type(0) {}

TTrapComponent::TTrapComponent(const TTrapComponent& a) :
  TMergeable(a),
  charges(a.charges),
  trap_comp_type(a.trap_comp_type) {}

TTrapComponent& TTrapComponent::operator=(const TTrapComponent& a) {
  if (this == &a)
    return *this;
  TMergeable::operator=(a);
  charges = a.charges;
  trap_comp_type = a.trap_comp_type;
  return *this;
}

TTrapComponent::~TTrapComponent() {}

bool TTrapComponent::willMerge(TMergeable* tm) {
  TTrapComponent* tComp;

  // Merge components of the same type that are:
  // Same VNum, same material, both have cost > 0, total charges <= 100
  if (!(tComp = dynamic_cast<TTrapComponent*>(tm)) || tComp == this ||
      (tComp->objVnum() != objVnum()) ||
      (tComp->getMaterial() != getMaterial()) ||
      tComp->obj_flags.cost <= 0 ||
      obj_flags.cost <= 0 ||
      tComp->getTrapComponentCharges() + getTrapComponentCharges() > 100) {
    return false;
  }
  return true;
}

void TTrapComponent::doMerge(TMergeable* tm) {
  TRoom* rp = nullptr;
  TTrapComponent* tComp;

  if (!(tComp = dynamic_cast<TTrapComponent*>(tm)) || !willMerge(tm))
    return;

  // Find the room for messaging
  if (!(rp = roomp)) {
    if (parent) {
      rp = parent->roomp;
    } else {
      if (!(rp = tComp->roomp)) {
        if (tComp->parent) {
          rp = tComp->parent->roomp;
        }
      }
    }
  }

  // Send merge message to room
  if (rp) {
    sstring str = sstring(shortDescr);
    sendrpf(COLOR_BASIC, rp, "%s combines with %s.\n\r",
      str.cap().c_str(), str.c_str());
  }

  // Merge charges and cost
  addToTrapComponentCharges(tComp->getTrapComponentCharges());
  obj_flags.cost += tComp->obj_flags.cost;

  // Remove and delete the merged component
  --(*tComp);
  delete tComp;
}

void TTrapComponent::assignFourValues(int x1, int x2, int x3, int x4) {
  setTrapComponentCharges(x1);
  setTrapComponentType(x4);
}

void TTrapComponent::getFourValues(int* x1, int* x2, int* x3, int* x4) const {
  *x1 = getTrapComponentCharges();
  *x2 = 0;
  *x3 = 0;
  *x4 = getTrapComponentType();
}

sstring TTrapComponent::statObjInfo() const {
  return format("Charges: %d, Type: %d") % getTrapComponentCharges() % getTrapComponentType();
}

sstring TTrapComponent::getNameForShow(bool useColor, bool useName, const TBeing* ch) const {
  sstring buf = TMergeable::getNameForShow(useColor, useName, ch);
  
  if (getTrapComponentCharges() != 1) {
    buf += format(" (%d uses)") % getTrapComponentCharges();
  }
  
  return buf;
}

void TTrapComponent::describeObjectSpecifics(const TBeing* ch) const {
  ch->sendTo(COLOR_OBJECTS, format("%s has about %d uses left.\n\r") %
                              sstring(getName()).cap() % getTrapComponentCharges());
}

int TTrapComponent::getTrapComponentCharges() const {
  return charges;
}

void TTrapComponent::setTrapComponentCharges(int n) {
  charges = max(0, n);
}

void TTrapComponent::addToTrapComponentCharges(int n) {
  charges = max(0, charges + n);
}

int TTrapComponent::getTrapComponentType() const {
  return trap_comp_type;
}

void TTrapComponent::setTrapComponentType(int n) {
  trap_comp_type = n;
}

void TTrapComponent::addTrapComponentType(int n) {
  trap_comp_type |= n;
}

void TTrapComponent::remTrapComponentType(int n) {
  trap_comp_type &= ~n;
}

bool TTrapComponent::isTrapComponentType(int n) const {
  return IS_SET(trap_comp_type, n);
}

bool TTrapComponent::objectRepair(TBeing* ch, TMonster* repair, silentTypeT silent) {
  if (!silent) {
    repair->doTell(fname(ch->getName()),
      "I don't repair trap components. Try a tinker or alchemist!");
  }
  return true;
}

void TTrapComponent::lowCheck() {
  if (!isname("component", name)) {
    vlogf(LOG_LOW, format("Trap component without COMPONENT in name (%s : %d)") %
                     getName() % objVnum());
  }

  // Trap components keep their builder-set price (the `price` column). Unlike
  // TComponent, we deliberately do NOT clobber obj_flags.cost to a derived
  // suggestedPrice() here — the seeded reagent prices are authoritative and the
  // whole charge/merge economy (pricePerUnit, additive merge cost) reads them.
  TMergeable::lowCheck();
}

void TTrapComponent::purchaseMe(TBeing* ch, TMonster* keeper, int cost, int shop_nr) {
  TShopOwned tso(shop_nr, keeper, ch);
  tso.doBuyTransaction(cost, getName(), TX_BUYING, this);
}

void TTrapComponent::sellMeMoney(TBeing* ch, TMonster* keeper, int cost, int shop_nr) {
  TShopOwned tso(shop_nr, keeper, ch);
  tso.doSellTransaction(cost, getName(), TX_SELLING);
}

double TTrapComponent::priceMultiplier() const {
  // Trap components are moderately valuable
  return 2.0;
}

int TTrapComponent::pricePerUnit() const {
  if (getTrapComponentCharges() <= 0)
    return 0;
  return obj_flags.cost / getTrapComponentCharges();
}

void TTrapComponent::boottimeInit() {
  // Nothing special needed at boottime
}

void TTrapComponent::update(int use) {
  // Trap components don't decay over time like spell components
  for (StuffIter it = stuff.begin(); it != stuff.end(); ++it)
    (*it)->update(use);
}

void TTrapComponent::decayMe() {
  // Trap components are stable and don't decay
}

TThing& TTrapComponent::operator--() {
  TMergeable::operator--();
  return *this;
}

void TTrapComponent::evaluateMe(TBeing* ch) const {
  ch->sendTo(format("You estimate that %s has about %d uses remaining.\n\r") %
             getName() % getTrapComponentCharges());
}

void TTrapComponent::changeObjValue4(TBeing* ch) {
  ch->sendTo("Trap component type flags:\n\r");
  ch->sendTo("This is for advanced builders only.\n\r");
}

void TTrapComponent::objMenu(const TBeing* ch) const {
  ch->sendTo("Trap component menu options:\n\r");
  ch->sendTo("  split <component> <charges> - Split charges into new component\n\r");
}

bool TTrapComponent::sellMeCheck(TBeing* ch, TMonster* keeper, int, int) const {
  if (getTrapComponentCharges() <= 0) {
    keeper->doTell(ch->getName(), "I don't buy empty trap components.");
    return true;
  }
  return false;
}

bool TTrapComponent::splitMe(TBeing* ch, const sstring& tString) {
  int tCount = 0;
  TTrapComponent* tComponent;
  sstring tStString(""), tStBuffer("");

  tStString = tString.word(0);
  tStBuffer = tString.word(1);

  if (tString.empty() || ((tCount = convertTo<int>(tStBuffer)) <= 0)) {
    ch->sendTo("Syntax: split <component> <charges>\n\r");
    return true;
  }

  if (tCount >= getTrapComponentCharges()) {
    ch->sendTo(format("Charges must be between 1 and %d.\n\r") %
               (getTrapComponentCharges() - 1));
    return true;
  }

  if (!obj_flags.cost || objVnum() < 0) {
    ch->sendTo("This component is special, it can not be split up.\n\r");
    return true;
  }

  // Create new component with split charges
  tComponent = dynamic_cast<TTrapComponent*>(read_object(objVnum(), VIRTUAL));
  if (!tComponent) {
    ch->sendTo("Unable to create split component.\n\r");
    return true;
  }

  // Set up the split component
  tComponent->setTrapComponentCharges(tCount);
  // Temporarily zero cost so willMerge rejects the re-merge when added to inventory
  tComponent->obj_flags.cost = 0;

  // Reduce this component's charges and cost
  int splitCost = (obj_flags.cost * tCount) / getTrapComponentCharges();
  addToTrapComponentCharges(-tCount);
  obj_flags.cost -= splitCost;

  // Give the split component to the player (cost=0 prevents auto-merge)
  *ch += *tComponent;

  // Now set the real cost
  tComponent->obj_flags.cost = splitCost;

  ch->sendTo(format("You split off %d charges into a new component.\n\r") % tCount);
  return true;
}

int TTrapComponent::sellMe(TBeing* ch, TMonster* tKeeper, int tShop, int num) {
  if (!shop_index[tShop].profit_sell) {
    tKeeper->doTell(ch->getName(), shop_index[tShop].do_not_buy);
    return false;
  }

  if (sellMeCheck(ch, tKeeper, tShop, num))
    return false;

  num = max(0, min(num, getTrapComponentCharges()));
  if (num <= 0)
    return false;

  // Trap components stack into a single shop rent row (val0 = charge count),
  // so a sale merges the sold charges into whatever the shop already stocks.
  TDatabase db(DB_SNEEZY);
  int rentId = 0, stocked = 0;
  db.query(
    "select rent_id, val0 from rent where owner_type='shop' and owner=%i and "
    "vnum=%i",
    tShop, objVnum());
  if (db.fetchRow()) {
    rentId = convertTo<int>(db["rent_id"]);
    stocked = convertTo<int>(db["val0"]);
  }

  // Respect the shop's per-item inventory cap.
  TShopOwned tso(tShop, tKeeper, ch);
  int maxNum = tso.getMaxNum(ch, this, 50);
  if (stocked >= maxNum) {
    tKeeper->doTell(ch->getName(),
      format("I already have plenty of %s.") % getName());
    return false;
  }
  if (stocked + num > maxNum)
    num = maxNum - stocked;

  float chr = max(1.0f, ch->getChaShopPenalty() - ch->getSwindleBonus());
  int cost = max(1, sellPrice(num, tShop, chr, ch));

  if (tKeeper->getMoney() < cost) {
    tKeeper->doTell(ch->getName(), shop_index[tShop].missing_cash1);
    return false;
  }

  // Player-facing feedback, mirroring TObj::sellMe / TComponent::sellMe.
  act("$n sells $p.", false, ch, this, nullptr, TO_ROOM);
  tKeeper->doTell(ch->getName(), format(shop_index[tShop].message_sell) % cost);
  ch->sendTo(COLOR_OBJECTS,
    format("The shopkeeper now has %s.\n\r") % sstring(getName()).uncap());
  ch->logItem(this, CMD_SELL);

  // Persist the sold charges into the shop's inventory so they can be bought
  // back. A fresh vnum gets a new rent row; an existing stack is topped up.
  int ppu = pricePerUnit();
  int shopCharges = stocked + num;
  if (!rentId) {
    if (TObj* proto = read_object(objVnum(), VIRTUAL)) {
      int v[4];
      proto->getFourValues(&v[0], &v[1], &v[2], &v[3]);
      proto->assignFourValues(shopCharges, v[1], v[2], v[3]);
      proto->obj_flags.cost = shopCharges * ppu;
      tKeeper->saveItem(tShop, proto);
      delete proto;
    }
  } else {
    db.query(
      "update rent set val0=%i, price=%i where rent_id=%i and "
      "owner_type='shop' and owner=%i and vnum=%i",
      shopCharges, shopCharges * ppu, rentId, tShop, objVnum());
  }

  bool soldOut = (num >= getTrapComponentCharges());
  if (soldOut) {
    // Detach now; the caller (generic_sell) deletes on DELETE_THIS.
    --(*this);
  } else {
    // Partial sale: drop the sold charges and rescale cost per remaining unit.
    addToTrapComponentCharges(-num);
    obj_flags.cost = getTrapComponentCharges() * ppu;
  }

  sellMeMoney(ch, tKeeper, cost, tShop);

  if (ch->isAffected(AFF_GROUP) && ch->desc &&
      IS_SET(ch->desc->autobits, AUTO_SPLIT) && (ch->master || ch->followers)) {
    sstring buf = format("%d") % cost;
    ch->doSplit(buf.c_str(), false);
  }

  ch->doQueueSave();
  return soldOut ? DELETE_THIS : false;
}

int TTrapComponent::buyMe(TBeing* ch, TMonster* tKeeper, int tNum, int tShop) {
  if ((ch->getCarriedVolume() + getTotalVolume()) > ch->carryVolumeLimit()) {
    ch->sendTo(format("%s: You can not carry that much volume.\n\r") % fname(name));
    return -1;
  }

  if (compareWeights(getTotalWeight(TRUE),
        (ch->carryWeightLimit() - ch->getCarriedWeight())) == -1) {
    ch->sendTo(format("%s: You can not carry that much weight.\n\r") % fname(name));
    return -1;
  }

  int charges = getTrapComponentCharges();
  if (tNum > charges) {
    tKeeper->doTell(ch->getName(),
      format("I don't have %d charges of %s.  Here %s the %d I do have.") %
        tNum % getName() % ((charges > 2) ? "are" : "is") % charges);
    tNum = charges;
  }

  int tCost = shopPrice(tNum, tShop, 1.0, ch);

  if ((ch->getMoney() < tCost) && !ch->hasWizPower(POWER_GOD)) {
    tKeeper->doTell(ch->name, shop_index[tShop].missing_cash2);
    switch (shop_index[tShop].temper1) {
      case 0:
        tKeeper->doAction(ch->getName(), CMD_SMILE);
        break;
      case 1:
        act("$n grins happily.", false, tKeeper, nullptr, nullptr, TO_ROOM);
        break;
      default:
        break;
    }
    return -1;
  }

  TObj* tObj;
  bool partial = false;
  if (charges == tNum) {
    // Buying all charges — hand over the whole loaded object.
    tObj = this;
    --(*tObj);
  } else {
    // Partial purchase — split off a new object with requested charges.
    tObj = read_object(objVnum(), VIRTUAL);
    if (!tObj)
      return -1;

    if (auto* splitComp = dynamic_cast<TTrapComponent*>(tObj)) {
      int cost_per = pricePerUnit();
      splitComp->setTrapComponentCharges(tNum);
      addToTrapComponentCharges(-tNum);

      splitComp->obj_flags.cost = tNum * cost_per;
      obj_flags.cost = getTrapComponentCharges() * cost_per;
    }
    partial = true;
  }

  tObj->purchaseMe(ch, tKeeper, tCost, tShop);
  *ch += *tObj;

  // On a partial buy the caller deletes the original rent row, so persist the
  // shop's decremented stack as a fresh row and drop our loaded copy.
  if (partial) {
    tKeeper->saveItem(tShop, this);
    delete this;
  }

  return tCost;
}

void TTrapComponent::valueMe(TBeing* ch, TMonster* keeper, int shop_nr, int num) {
  if (!shop_index[shop_nr].willBuy(this)) {
    keeper->doTell(ch->getName(), shop_index[shop_nr].do_not_buy);
    return;
  }

  // sellMeCheck tells the player why if it refuses (e.g. empty component);
  // no point quoting a price in that case.
  if (sellMeCheck(ch, keeper, shop_nr, num))
    return;

  // Worst-case price on valuing: charisma penalty applies, swindle does not.
  float chr = max(1.0f, ch->getChaShopPenalty());
  num = max(1, min(num, getTrapComponentCharges()));
  int cost = max(1, sellPrice(num, shop_nr, chr, ch));

  keeper->doTell(ch->getName(),
    format("I'll give you %d talens for %d charge%s of %s!") %
      cost % num % (num == 1 ? "" : "s") % getName());

  if (keeper->getMoney() < cost) {
    keeper->doTell(ch->getName(),
      "Unfortunately, at the moment, I can not afford to buy that item from "
      "you.");
  }
}

int TTrapComponent::shopPrice(int num, int shop_nr, float, const TBeing*) const {
  if (getTrapComponentCharges() <= 0) {
    vlogf(LOG_BUG, format("TTrapComponent::shopPrice called with 0 charges (%s:%d)") %
                     getName() % objVnum());
    return 0;
  }
  return (obj_flags.cost * num) / getTrapComponentCharges();
}

int TTrapComponent::sellPrice(int num, int shop_nr, float, const TBeing*) {
  if (getTrapComponentCharges() <= 0) {
    vlogf(LOG_BUG, format("TTrapComponent::sellPrice called with 0 charges (%s:%d)") %
                     getName() % objVnum());
    return 0;
  }
  int base_price = (obj_flags.cost * num) / getTrapComponentCharges();
  return static_cast<int>(base_price * 0.5);
}
