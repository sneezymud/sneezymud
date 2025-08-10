//////////////////////////////////////////////////////////////////////////
//
// SneezyMUD - All rights reserved, SneezyMUD Coding Team
//
//////////////////////////////////////////////////////////////////////////

#include <stdio.h>

#include "handler.h"
#include "extern.h"
#include "room.h"
#include "being.h"
#include "monster.h"
#include "obj_trap_component.h"
#include "shop.h"
#include "shopowned.h"
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
  TRoom* rp = NULL;
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
  char buf[256];
  sprintf(buf, "Charges: %d, Type: %d", getTrapComponentCharges(), getTrapComponentType());
  sstring a(buf);
  return a;
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
  return TRUE;
}

void TTrapComponent::lowCheck() {
  if (!isname("component", name)) {
    vlogf(LOG_LOW, format("Trap component without COMPONENT in name (%s : %d)") %
                     getName() % objVnum());
  }
  
  int sp = suggestedPrice();
  if ((obj_flags.cost != sp) && sp > 0) {
    vlogf(LOG_LOW, format("trap component (%s:%d) with bad price %d should be %d.") %
                     getName() % objVnum() % obj_flags.cost % sp);
    obj_flags.cost = sp;
  }

  TMergeable::lowCheck();
}

int TTrapComponent::suggestedPrice() const {
  // Base price calculation: charges * base_cost_per_charge
  int base_price = getTrapComponentCharges() * 10; // 10 talens per charge base

  // TODO: Add material pricing adjustment once material_nums issue is resolved
  // base_price = (int)(base_price * material_nums[getMaterial()].price);

  return max(1, base_price);
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
    return TRUE;
  }
  return FALSE;
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
  tComponent->obj_flags.cost = (obj_flags.cost * tCount) / getTrapComponentCharges();

  // Reduce this component's charges and cost
  addToTrapComponentCharges(-tCount);
  obj_flags.cost -= tComponent->obj_flags.cost;

  // Give the split component to the player
  *ch += *tComponent;

  ch->sendTo(format("You split off %d charges into a new component.\n\r") % tCount);
  return true;
}

int TTrapComponent::trapComponentSell(TBeing* ch, TMonster* keeper, int shop_nr, TThing* sub) {
  return sellMe(ch, keeper, shop_nr);
}

int TTrapComponent::trapComponentNumSell(TBeing* ch, TMonster* keeper, int shop_nr, TThing* sub, int num) {
  return sellMe(ch, keeper, shop_nr, num);
}

int TTrapComponent::trapComponentValue(TBeing* ch, TMonster* keeper, int shop_nr, TThing* sub) {
  valueMe(ch, keeper, shop_nr);
  return FALSE;
}

int TTrapComponent::trapComponentNumValue(TBeing* ch, TMonster* keeper, int shop_nr, TThing* sub, int num) {
  valueMe(ch, keeper, shop_nr, num);
  return FALSE;
}

int TTrapComponent::sellMe(TBeing* ch, TMonster* tKeeper, int tShop, int num) {
  if (sellMeCheck(ch, tKeeper, tShop, num))
    return -1;

  int cost = sellPrice(num, tShop, 1.0, ch);
  sellMeMoney(ch, tKeeper, cost, tShop);

  if (num >= getTrapComponentCharges()) {
    // Selling all charges, delete the component
    --(*this);
    delete this;
  } else {
    // Selling partial charges
    addToTrapComponentCharges(-num);
    obj_flags.cost = (obj_flags.cost * getTrapComponentCharges()) / (getTrapComponentCharges() + num);
  }

  return cost;
}

int TTrapComponent::buyMe(TBeing* ch, TMonster* tKeeper, int tNum, int tShop) {
  int tCost = shopPrice(tNum, tShop, 1.0, ch);

  if ((ch->getCarriedVolume() + getTotalVolume()) > ch->carryVolumeLimit()) {
    ch->sendTo(format("%s: You can not carry that much volume.\n\r") % fname(name));
    return -1;
  }

  if (compareWeights(getTotalWeight(TRUE),
        (ch->carryWeightLimit() - ch->getCarriedWeight())) == -1) {
    ch->sendTo(format("%s: You can not carry that much weight.\n\r") % fname(name));
    return -1;
  }

  purchaseMe(ch, tKeeper, tCost, tShop);
  *ch += *this;

  return tCost;
}

void TTrapComponent::valueMe(TBeing* ch, TMonster* keeper, int shop_nr, int num) {
  int cost = sellPrice(num, shop_nr, 1.0, ch);
  ch->sendTo(format("%s will pay you %d talens for %d charges of %s.\n\r") %
             keeper->getName() % cost % num % getName());
}

int TTrapComponent::shopPrice(int num, int shop_nr, float, const TBeing*) const {
  return (obj_flags.cost * num) / getTrapComponentCharges();
}

int TTrapComponent::sellPrice(int num, int shop_nr, float, const TBeing*) {
  int base_price = (obj_flags.cost * num) / getTrapComponentCharges();
  return (int)(base_price * 0.5); // Sell for 50% of shop price
}
