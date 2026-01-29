---
title: Economy System
description: Comprehensive economy management including shops, pricing, banking, corporations, and all money flow in SneezyMUD with player-owned shops, automated accounting, tax collection, and fractional reserve banking.
keywords:
  - shopData
  - TShopOwned
  - TCorporation
  - TShopJournal
  - profit-buy
  - profit-sell
  - doBuyTransaction
  - doSellTransaction
  - shoplog
  - journalize
  - TMoney
  - create-money
  - banking-system
  - double-entry-accounting
  - corporation-wealth
category: Important Systems
related:
  - faction-system.md
  - database-queries.md
  - object-types.md
last_updated: 2026-01-29
source_files:
  - code/code/misc/shop.h
  - code/code/misc/shop.cc
  - code/code/misc/shopowned.h
  - code/code/misc/shopowned.cc
  - code/code/misc/shopaccounting.h
  - code/code/misc/shopaccounting.cc
  - code/code/misc/corporation.h
  - code/code/misc/corporation.cc
  - code/code/spec/spec_mobs_banker.cc
  - code/code/obj/obj_money.h
  - code/code/obj/obj_money.cc
  - code/code/misc/utility.cc
  - code/code/misc/inventory.cc
  - code/code/misc/other.cc
---

# Economy System

The economy system manages shops, pricing, banking, corporations, and all money flow in SneezyMUD. This is a complex system with player-owned shops, automated accounting, tax collection, and a fractional reserve banking system.

**Misusing this system causes money duplication or loss bugs.** Common errors: forgetting to save after money transfers, not checking keeper cash before purchases, incorrect transaction type logging.

## Core Components

| Component | Purpose | Key Files |
|-----------|---------|-----------|
| `shopData` | NPC shop configuration | `shop.h`, `shop.cc` |
| `TShopOwned` | Player-owned shop operations | `shopowned.h`, `shopowned.cc` |
| `TCorporation` | Corporation management | `corporation.h`, `corporation.cc` |
| `TShopJournal` | Double-entry accounting | `shopaccounting.h`, `shopaccounting.cc` |

## Shop Types

### NPC Shops (Unowned)

Standard shops with fixed pricing configured in the database. The `shop` table defines base shop data:

```sql
CREATE TABLE shop (
  shop_nr INT PRIMARY KEY,
  profit_buy DOUBLE,       -- Multiplier for selling TO players (>1.0)
  profit_sell DOUBLE,      -- Multiplier for buying FROM players (<1.0)
  keeper INT,              -- Mob vnum of shopkeeper
  in_room INT,             -- Room vnum where shop operates
  open1, close1,           -- First operating hours
  open2, close2,           -- Second operating hours (optional)
  expense_ratio DOUBLE     -- Portion paid to SBA for services
);
```

### Player-Owned Shops

When a player purchases a shop, a row is added to `shopowned`:

```sql
CREATE TABLE shopowned (
  shop_nr INT PRIMARY KEY,
  profit_buy DOUBLE,       -- Custom buy multiplier
  profit_sell DOUBLE,      -- Custom sell multiplier
  max_num INT,             -- Default max inventory per item
  corp_id INT,             -- Owning corporation
  dividend DOUBLE,         -- Percentage sent to corp bank
  reserve_min INT,         -- Min cash before corp withdrawal
  reserve_max INT,         -- Max cash before corp deposit
  tax_nr INT               -- Tax office shop_nr
);
```

**Shop purchase price:**
```cpp
int TShopOwned::getPurchasePrice(int talens, int value) {
  return (int)(((talens + value) * 1.15) + 1000000);
}
```

Purchase requires: 15% markup on (cash + inventory value) plus 1,000,000 talen flat fee.

## Pricing Calculations

### Price Formulas

**Shop sells to player (buy command):**
```cpp
price = adjPrice() * profit_buy * charisma_modifier
```

**Shop buys from player (sell command):**
```cpp
price = adjPrice() * profit_sell / charisma_modifier
```

### adjPrice() - Structure-Based Value

```cpp
int TObj::adjPrice() const {
  // Damaged items worth less
  if (getMaxStructPoints() <= 0)
    return getValue();
  return (int)(getValue() * getStructPoints() / getMaxStructPoints());
}
```

### Profit Multipliers

| Multiplier | Direction | Typical Value | Effect |
|------------|-----------|---------------|--------|
| `profit_buy` | Shop to player | 1.1 - 5.0 | Higher = more expensive to buy |
| `profit_sell` | Player to shop | 0.1 - 0.9 | Higher = shop pays more |

**Critical:** `profit_buy` MUST be greater than `profit_sell` or the shop will lose money on arbitrage.

```cpp
if (profit_buy < profit_sell) {
  keeper->doTell(ch->getName(),
    "You can't set your buy profit lower than your sell profit!");
  return FALSE;
}
```

### Charisma and Swindle Effects

Player charisma and the Swindle skill affect prices:

```cpp
chr = ch->getChaShopPenalty() - ch->getSwindleBonus();
chr = max(1.0f, chr);  // Minimum modifier of 1.0
```

- High charisma (low penalty) = better prices
- Higher Swindle skill = additional discount
- Combined modifier multiplies buy prices, divides sell prices

### Custom Pricing Tiers

Player-owned shops support three pricing customization levels:

1. **By vnum** (`shopownedratios`): Specific item prices
2. **By keyword** (`shopownedmatch`): Items matching name keywords
3. **By player** (`shopownedplayer`): Per-player pricing

Priority: player > vnum > keyword > default

```cpp
// Check order in getProfitBuy()
if (buy_ratios_cache.count(vnum))      // 1. Specific vnum
  profit = buy_ratios_cache[vnum];
for (iter : buy_matches_cache)         // 2. Keyword match
  if (isname(iter->first, obj->name))
    profit = iter->second;
for (iter : buy_player_cache)          // 3. Player-specific
  if (iter->first == ch->name)
    profit = iter->second;
```

## Transaction Flow

### Player Buying from Shop

```
shopping_buy() -> TObj::buyMe() -> TObj::purchaseMe()
                                 -> TShopOwned::doBuyTransaction()
```

1. Validate shop hours, visibility, weight/volume limits
2. Calculate price with charisma modifier
3. Check player has enough money
4. Transfer item to player
5. `doBuyTransaction()`:
   - Deduct expenses (`doExpenses()`)
   - Transfer money (`ch->giveMoney(keeper, cost, GOLD_SHOP)`)
   - Log transaction (`shoplog()`)
   - Pay dividend to corp (`doDividend()`)
   - Balance reserves (`doReserve()`)
   - Charge tax (`chargeTax()`)
   - Record in journal (`journalize()`)
6. Save both keeper and player

### Player Selling to Shop

```
shopping_sell() -> generic_sell() -> TObj::sellMe()
                                   -> TShopOwned::doSellTransaction()
```

1. Validate shop will buy item type
2. Check item is not damaged/burning/rusty (configurable)
3. Check shop inventory limits
4. Check keeper has enough money
5. Calculate price with structure penalty:
   ```cpp
   if (getStructPoints() != getMaxStructPoints()) {
     cost *= 6;  // 60% base deduction
     cost /= 10;
     cost *= getStructPoints() / getMaxStructPoints();
   }
   ```
6. `doSellTransaction()`:
   - Transfer money to player (`keeper->giveMoney(ch, cost, GOLD_SHOP)`)
   - Save item to shop inventory
   - Balance reserves
   - Record in journal

## Shop Access Permissions

```cpp
const unsigned int SHOPACCESS_OWNER    = (1 << 0);  // Full access
const unsigned int SHOPACCESS_INFO     = (1 << 1);  // View shop info
const unsigned int SHOPACCESS_RATES    = (1 << 2);  // Change prices
const unsigned int SHOPACCESS_GIVE     = (1 << 3);  // Withdraw money
const unsigned int SHOPACCESS_SELL     = (1 << 4);  // Sell the shop
const unsigned int SHOPACCESS_ACCESS   = (1 << 5);  // Manage permissions
const unsigned int SHOPACCESS_LOGS     = (1 << 6);  // View transaction logs
const unsigned int SHOPACCESS_DIVIDEND = (1 << 7);  // Set dividend rate
```

Stored in `shopownedaccess` table by character name.

## Corporation System

Corporations own shops and manage shared finances.

```sql
CREATE TABLE corporation (
  corp_id BIGINT PRIMARY KEY AUTO_INCREMENT,
  name VARCHAR(80),
  bank INT           -- shop_nr of corporation's bank
);

CREATE TABLE corpaccess (
  corp_id INT,
  access INT,        -- Permission flags
  player_id INT,
  name VARCHAR(80)
);
```

### Corporation Access Flags

```cpp
const unsigned int CORPACCESS_PARTNER = (1 << 0);  // Full partner
const unsigned int CORPACCESS_INFO    = (1 << 1);  // View corp info
const unsigned int CORPACCESS_GIVE    = (1 << 3);  // Withdraw money
const unsigned int CORPACCESS_ACCESS  = (1 << 5);  // Manage permissions
const unsigned int CORPACCESS_LOGS    = (1 << 6);  // View logs
```

### Reserve System

Shops automatically balance cash with their corporation:

```cpp
int TShopOwned::doReserve() {
  if (money < min_reserve) {
    // Withdraw from corp to reach midpoint
    amt = ((max + min) / 2) - money;
    corp.setMoney(corp.getMoney() - amt);
    banker->giveMoney(keeper, amt, GOLD_SHOP);
  } else if (money > max_reserve) {
    // Deposit to corp to reach midpoint
    amt = money - ((max + min) / 2);
    corp.setMoney(corp.getMoney() + amt);
    keeper->giveMoney(banker, amt, GOLD_SHOP);
  }
}
```

## Banking System

Banks are special shops (`SPEC_BANKER`) that hold player and corporation deposits.

### Player Accounts (`shopownedbank`)

```sql
CREATE TABLE shopownedbank (
  shop_nr INT,
  player_id INT,
  talens INT,
  earned_interest DOUBLE
);
```

### Corporate Accounts (`shopownedcorpbank`)

```sql
CREATE TABLE shopownedcorpbank (
  shop_nr INT,
  corp_id INT,
  talens INT,
  earned_interest DOUBLE
);
```

### Interest Calculation

`procBankInterest` runs daily and applies interest:

```cpp
// Daily compound interest
earned_interest += talens * (profit_sell / 365.0);
talens += truncate(earned_interest, 0);
earned_interest -= truncate(earned_interest, 0);
```

### Central Banking

Central banks (`SPEC_CENTRAL_BANKER`) set reserve requirements for regular banks:

```sql
CREATE TABLE shopownedcentralbank (
  bank INT,         -- Regular bank shop_nr
  centralbank INT   -- Central bank shop_nr
);
```

Reserve requirement = total deposits * `profit_buy` of central bank.

## Tax System

Shops can be assigned to tax offices (`SPEC_TAXMAN`):

```cpp
int TShopOwned::chargeTax(int cost, const sstring& name, TObj* o) {
  tax_office = getTaxShopNr();
  if (tax_office <= 0)
    tax_office = GRIMHAVEN_TAX_OFFICE;  // Default: shop_nr 14

  int tax = (int)(cost * shop_index[tax_office].getProfitBuy(o, ch));
  keeper->giveMoney(taxman, tax, GOLD_SHOP);
}
```

Tax is exempt for:
- Shop owners buying from their own shop
- Casino transactions
- Commodity purchases

## Accounting System

Player-owned shops maintain double-entry bookkeeping via `shoplogjournal`.

### Chart of Accounts

| Post Ref | Account | Type |
|----------|---------|------|
| 100 | Cash | Asset |
| 101 | Dividends | Contra-Equity |
| 130 | Inventory | Asset |
| 300 | Paid-in Capital | Equity |
| 310 | Deposits | Liability |
| 500 | Sales | Revenue |
| 510 | Recycling | Revenue |
| 600 | COGS | Expense |
| 610 | Interest | Expense |
| 630 | Expenses | Expense |
| 700 | Tax | Expense |
| 800 | Retained Earnings | Equity |

### COGS Tracking

Cost of Goods Sold tracks average purchase price:

```cpp
void TShopOwned::COGS_add(const sstring& name, int amt, int num) {
  // Add to running total
  db.query("update shoplogcogs set count=count+%i, total_cost=total_cost+%i "
           "where obj_name='%s' and shop_nr=%i", num, amt, name, shop_nr);
}

int TShopOwned::COGS_get(const sstring& name, int num) {
  // Calculate average cost * quantity
  return (total_cost / count) * num;
}
```

### Year-End Close

`TShopJournal::closeTheBooks()` archives journal entries and carries forward balances.

## Transaction Logging

All transactions are logged to `shoplog`:

```sql
CREATE TABLE shoplog (
  shop_nr INT,
  name VARCHAR(80),      -- Player/entity name
  action VARCHAR(32),    -- Transaction type
  item VARCHAR(80),      -- Item name or "talens"
  talens INT,            -- Amount (+/- based on direction)
  shoptalens INT,        -- Shop balance after
  shopvalue INT,         -- Shop inventory value
  logtime TIMESTAMP,
  itemcount INT
);
```

```cpp
void shoplog(int shop_nr, TBeing* ch, TMonster* keeper,
             const sstring& item, int talens, const sstring& action);
```

## Special Shop Types

### Repair Shops (shop_nr 127-134)

```cpp
bool shopData::isRepairShop() {
  return (shop_nr >= 127 && shop_nr <= 134);
}
```

Repair shops use `shopownedrepair` for quality/speed settings:
- `speed`: Faster repairs cost more (divides profit_buy)
- `quality`: Better repairs cost more (multiplies profit_buy)

### Loan Sharks (`SPEC_LOAN_SHARK`)

Use `shopownedloans` and `shopownedloanrate` for player loans.

### Auctioneers (`SPEC_AUCTIONEER`)

Use `shopownedauction` for auction listings.

## Database Schema Summary

### Core Tables

| Table | Purpose |
|-------|---------|
| `shop` | Base shop configuration |
| `shoptype` | Item types shop trades |
| `shopmaterial` | Materials shop accepts |
| `shopproducing` | Items shop produces |
| `shopowned` | Player ownership data |

### Ownership Tables

| Table | Purpose |
|-------|---------|
| `shopownedaccess` | Player permissions |
| `shopownedratios` | Per-vnum pricing |
| `shopownedmatch` | Keyword pricing |
| `shopownedplayer` | Per-player pricing |
| `shopownedrepair` | Repair shop settings |

### Financial Tables

| Table | Purpose |
|-------|---------|
| `shopownedbank` | Player bank accounts |
| `shopownedcorpbank` | Corporate accounts |
| `shopownedcentralbank` | Central bank links |
| `shopownedloans` | Active loans |
| `shopownedloanrate` | Loan terms |

### Accounting Tables

| Table | Purpose |
|-------|---------|
| `shoplog` | Transaction history |
| `shoplogjournal` | Current year journal |
| `shoplogjournalarchive` | Past years |
| `shoplogcogs` | COGS tracking |
| `shoplogaccountchart` | Account definitions |

### Corporation Tables

| Table | Purpose |
|-------|---------|
| `corporation` | Corporation definitions |
| `corpaccess` | Member permissions |
| `corplog` | Corporate transactions |

## Common Bugs and Edge Cases

### Money Duplication

```cpp
// WRONG: Forgot to save after transfer
keeper->giveMoney(ch, amount, GOLD_SHOP);
// Crash here = money given but not deducted from keeper

// CORRECT: Always save both parties
keeper->giveMoney(ch, amount, GOLD_SHOP);
keeper->saveItems(shop_nr);
ch->doQueueSave();
```

### Keeper Insufficient Funds

```cpp
// WRONG: Not checking keeper balance
keeper->giveMoney(ch, amount, GOLD_SHOP);

// CORRECT: Check before transfer
if (keeper->getMoney() < amount) {
  keeper->doTell(ch->getName(), shop_index[shop_nr].missing_cash1);
  return false;
}
```

### Shop Inventory Overflow

```cpp
const unsigned int MAX_SHOP_INVENTORY = 2500;

if (shop_index[shop_nr].getInventoryCount() >= MAX_SHOP_INVENTORY) {
  keeper->doTell(ch->getName(), "My inventory is full!");
  return TRUE;
}
```

### Reserve Deadlock

If `reserve_min` and `reserve_max` are too close:

```cpp
if (!(min == 0 && max == 0) && (min > max || (max - min) < 100000)) {
  keeper->doTell(ch->getName(),
    "The two reserve values must be at least 100k apart.");
  return;
}
```

### Transaction Type Mismatch

Using wrong transaction type causes incorrect journal entries:

```cpp
enum transactionTypeT {
  TX_BUYING,            // Player buying item
  TX_BUYING_SERVICE,    // Player buying service (no COGS)
  TX_RECYCLING,         // Scrap value transaction
  TX_SELLING,           // Player selling item
  TX_PRODUCING,         // Shop producing item
  TX_GIVING_TALENS,     // Owner depositing money
  TX_RECEIVING_TALENS,  // Owner withdrawing money
  TX_PAYING_INTEREST,   // Bank interest payment
  TX_WITHDRAWAL,        // Bank withdrawal
  TX_DEPOSIT,           // Bank deposit
  TX_FACTORY            // Factory production
};
```

### Strung Item Matching

Strung items need special handling for inventory counts:

```cpp
if (obj->isObjStat(ITEM_STRUNG))
  db.query("select count(*) from rent ... where rs.short_desc='%s' "
           "and (r.extra_flags & 4) = 4", obj->shortDescr);
else
  db.query("select count(*) from rent where vnum=%i", obj->objVnum());
```

## Currency Mechanics

This section covers the low-level mechanics of how money objects work, including the TMoney class, currency stacking, money-related commands, and money handling in transactions.

### TMoney Class

Money in SneezyMUD is represented by `TMoney` objects that inherit from `TMergeable`. Unlike most games where money is just an integer on the player, physical money objects exist that can be dropped, given, and picked up.

**Class hierarchy:**
```
TObj -> TMergeable -> TMoney
```

**Key data members:**
```cpp
class TMoney : public TMergeable {
  private:
    int money;           // Amount of currency in this pile
    currencyTypeT type;  // Which currency (talen, dinar, etc.)
};
```

**Currency types:**
```cpp
enum currencyTypeT {
  CURRENCY_GRIMHAVEN = 0,  // talens
  CURRENCY_LOGRUS,         // dinars
  CURRENCY_BRIGHTMOON,     // kroners
  CURRENCY_AMBER,          // guilders
  MAX_CURRENCY
};
```

Each currency is associated with a faction:

| Currency | Name | Faction |
|----------|------|---------|
| CURRENCY_GRIMHAVEN | talen | FACT_NONE (default) |
| CURRENCY_LOGRUS | dinar | FACT_CULT |
| CURRENCY_BRIGHTMOON | kroner | FACT_BROTHERHOOD |
| CURRENCY_AMBER | guilder | FACT_SNAKE |

**Creating money objects:**

The `create_money()` function creates TMoney objects from the template object `Obj::GENERIC_TALEN` (vnum 13):

```cpp
TMoney* create_money(int amount, currencyTypeT currency) {
  TObj* obj = read_object(Obj::GENERIC_TALEN, VIRTUAL);
  TMoney* money = dynamic_cast<TMoney*>(obj);

  money->swapToStrung();  // Make it a strung object for custom descriptions
  money->setCurrency(currency);
  money->setMoney(amount);

  // Set descriptions based on amount
  if (amount == 1) {
    money->shortDescr = "a " + money->getCurrencyName();
  } else {
    money->shortDescr = format("some %ss") % money->getCurrencyName();
  }

  // Pile descriptions vary by size
  if (amount > 100000)
    buf = "A tremendously HUGE pile of talens lies here.";
  else if (amount > 50000)
    buf = "A HUGE pile of talens lies here.";
  // ... etc

  // Physical properties scale with amount
  money->setVolume(max(1, (int)(amount * 0.0048)));  // ~0.078 cm3 per coin
  money->setWeight(amount / 303.0);                   // ~1.5 grams per coin

  return money;
}
```

**Faction-based currency creation:**

An overload of `create_money()` selects currency based on faction:

```cpp
TMoney* create_money(int amount, factionTypeT fact) {
  switch (fact) {
    case FACT_BROTHERHOOD: return create_money(amount, CURRENCY_BRIGHTMOON);
    case FACT_CULT:        return create_money(amount, CURRENCY_LOGRUS);
    case FACT_SNAKE:       return create_money(amount, CURRENCY_AMBER);
    default:               return create_money(amount, CURRENCY_GRIMHAVEN);
  }
}
```

### Currency Stacking (Merging)

When money objects are placed in the same container (inventory, room, etc.), they automatically merge if they're the same currency type. This uses the `TMergeable` base class.

**Merge check:**
```cpp
bool TMoney::willMerge(TMergeable* tm) {
  TMoney* tMoney = dynamic_cast<TMoney*>(tm);

  // Won't merge if: not money, same object, or different currency
  if (!tMoney || this == tMoney || tMoney->getCurrency() != getCurrency())
    return false;

  return true;
}
```

**Merge execution:**
```cpp
void TMoney::doMerge(TMergeable* tm) {
  TMoney* tMoney = dynamic_cast<TMoney*>(tm);
  if (!tMoney) return;

  // Add the other pile's amount to this one
  setMoney(getMoney() + tMoney->getMoney());

  // Remove and delete the merged pile
  --(*tMoney);
  delete tMoney;
}
```

**When merging occurs:**

The `operator+=` in structs.cc handles merging when objects are added to containers:

```cpp
TMergeable* tm = dynamic_cast<TMergeable*>(&t);
if (tm) {
  for (StuffIter it = stuff.begin(); it != stuff.end(); ++it) {
    TMergeable* tMerge = dynamic_cast<TMergeable*>(*it);
    if (tMerge && tm != tMerge && tm->willMerge(tMerge)) {
      tm->doMerge(tMerge);
      break;
    }
  }
}
```

This means:
- Picking up money merges it with existing money in inventory
- Dropping money merges it with money already on the ground
- Money in containers merges when another pile is added

### Money Commands

#### Dropping Money

**Syntax:** `drop <amount> talens` or `drop all talens`

The `doDrop()` function handles money drops:

```cpp
if (!tng && is_abbrev(arg2, "talens")) {
  amount = (arg.lower() == "all") ? getMoney() : convertTo<int>(arg);

  if (amount <= 0) {
    sendTo("Sorry, you can't do that!\n\r");
    return FALSE;
  }
  if (!isImmortal() && (getMoney() < amount)) {
    sendTo("You haven't got that many talens!\n\r");
    return FALSE;
  }

  // Create money object and place in room
  TMoney* money = create_money(amount);
  *roomp += *money;           // Add to room (will merge with existing)
  addToMoney(-amount, GOLD_INCOME);  // Deduct from player

  doQueueSave();
}
```

#### Giving Money

**Syntax:** `give <amount> talens <target>` or `give all talens <target>`

The `doGive()` function handles money transfers:

```cpp
if (is_number(obj_name) || obj_name.lower() == "all") {
  amount = (obj_name.lower() == "all") ? getMoney() : convertTo<int>(obj_name);

  // Validate recipient can receive
  if (vict->isPlayerAction(PLR_SOLOQUEST)) {
    act("$N is on a solo quest; you can't give anything to $M.", ...);
    return FALSE;
  }
  if (!vict->hasHands()) {
    act("$N has no hands, you can't give $M things.", ...);
    return FALSE;
  }

  // Transfer the money
  giveMoney(vict, amount, GOLD_XFER);

  // Both parties save immediately
  saveChar(Room::AUTO_RENT);
  vict->saveChar(Room::AUTO_RENT);
}
```

The actual transfer uses `giveMoney()`:

```cpp
void TBeing::giveMoney(TBeing* ch, int money, moneyTypeT type) {
  if (money < 0) {
    vlogf(LOG_BUG, format("%s tried to give negative money (%i) to %s") % ...);
    return;
  }
  addToMoney(-money, type);
  ch->addToMoney(money, type);
}
```

#### Splitting Money

**Syntax:** `split <amount>`

The `doSplit()` function divides money among grouped players:

```cpp
void TBeing::doSplit(const char* argument, bool tell) {
  // Find group leader
  if (!(k = master))
    k = this;

  // Count shares for all grouped members in same room
  no_members = splitShares(this, k);
  for (f = k->followers; f; f = f->next) {
    no_members += splitShares(this, f->follower);
  }

  // Validate
  if (no_members <= 1 || !isAffected(AFF_GROUP)) {
    if (tell) sendTo("Split your talens with whom?\n\r");
    return;
  }

  // Deduct from splitter (minus their share)
  int myshares = splitShares(this, this);
  tmp_amount = amount * (no_members - myshares) / no_members;
  addToMoney(-tmp_amount, GOLD_XFER);

  // Distribute to group members
  for (f = k->followers; f; f = f->next) {
    if (f->follower->isAffected(AFF_GROUP) && sameRoom(*f->follower)) {
      int myshares = splitShares(this, f->follower);
      tmp_amount = amount * myshares / no_members;
      f->follower->addToMoney(tmp_amount, GOLD_XFER);
      f->follower->sendTo(format("%s splits %d talens, you receive %d.\n\r")
                          % getName() % amount % tmp_amount);
    }
  }
}
```

**Auto-split feature:**

When `AUTO_SPLIT` is enabled via `toggle split`, money is automatically split when:
- Picking up money from the ground
- Selling items to shops
- Selling commodities
- Selling components

```cpp
// From obj_money.cc
if (ch->isAffected(AFF_GROUP) && ch->desc &&
    IS_SET(ch->desc->autobits, AUTO_SPLIT) &&
    (ch->master || ch->followers)) {
  sprintf(buf, "%d", amount);
  ch->doSplit(buf, false);
}
```

### Picking Up Money

The `TMoney::getMe()` function handles picking up money:

```cpp
int TMoney::getMe(TBeing* ch, TThing* sub) {
  int rc = TObj::getMe(ch, sub);  // Base class handling
  if (rc) return rc;

  rc = moneyMeMoney(ch, sub);
  if (IS_SET_DELETE(rc, DELETE_THIS)) {
    delete this;  // Money object consumed
  }
  return TRUE;
}
```

The `moneyMeMoney()` function converts the object to player money:

```cpp
int TMoney::moneyMeMoney(TBeing* ch, TThing* sub) {
  (*this)--;  // Remove from container
  int amount = getMoney();

  // Notify player
  if (amount == 1)
    ch->sendTo(format("There was one %s.\n\r") % getCurrencyName());
  else
    ch->sendTo(format("There were %d %ss.\n\r") % amount % getCurrencyName());

  // Log large pickups
  if (ch->getMoney() > 500000 && amount > 100000)
    vlogf(LOG_MISC, format("%s just got %d talens") % ch->getName() % amount);

  // Alert greedy mobs in room
  for (t : ch->roomp->stuff) {
    TMonster* tmons = dynamic_cast<TMonster*>(t);
    if (tmons && tmons->isGreedy()) {
      tmons->UG(1 + amount / 1000);  // Increase greed urgency
      tmons->aiTarget(ch);
    }
  }

  // Add money to player (tracks statistics)
  ch->addToMoney(amount, GOLD_INCOME, !isMyCorpse && !ch->isImmortal());

  // Auto-split if enabled
  if (ch->isAffected(AFF_GROUP) && IS_SET(ch->desc->autobits, AUTO_SPLIT))
    ch->doSplit(buf, false);

  return DELETE_THIS;  // Signal that this object should be deleted
}
```

**Key point:** When picking up money, the TMoney object is deleted and the amount is added directly to the player's money counter. Money doesn't exist as objects in inventory - it's converted immediately.

### Money in Transactions

#### Player Buying from Shop

When a player buys from a shop, `TObj::purchaseMe()` is called:

```cpp
void TObj::purchaseMe(TBeing* ch, TMonster* keeper, int cost, int shop_nr) {
  TShopOwned tso(shop_nr, keeper, ch);
  tso.doBuyTransaction(cost, getName(), TX_BUYING, this);
}
```

This calls `doBuyTransaction()`:

```cpp
void TShopOwned::doBuyTransaction(int cashCost, const sstring& name,
                                   transactionTypeT action, TObj* obj) {
  int expenses = doExpenses(cashCost, obj);

  // Player gives money to keeper
  ch->giveMoney(keeper, cashCost, GOLD_SHOP);

  // Log transaction
  shoplog(shop_nr, ch, keeper, name, cashCost, "buying");

  if (owned) {
    int corp_cash = doDividend(cashCost, name);
    corp_cash += doReserve();
    int tax = chargeTax(cashCost, name, obj);
    journalize(ch->getName(), name, TX_BUYING, cashCost, expenses, corp_cash, tax);
  }

  keeper->saveItems(shop_nr);
  ch->doQueueSave();
}
```

#### Player Selling to Shop

When a player sells to a shop, `TObj::sellMeMoney()` is called:

```cpp
void TObj::sellMeMoney(TBeing* ch, TMonster* keeper, int cost, int shop_nr) {
  TShopOwned tso(shop_nr, keeper, ch);
  tso.doSellTransaction(cost, getName(), TX_SELLING);
}
```

This calls `doSellTransaction()`:

```cpp
void TShopOwned::doSellTransaction(int cashCost, const sstring& name,
                                    transactionTypeT action, int num) {
  // Keeper gives money to player
  keeper->giveMoney(ch, cashCost, GOLD_SHOP);

  // Log transaction
  shoplog(shop_nr, ch, keeper, name, -cashCost, "selling");

  if (owned) {
    int corp_cash = doReserve();
    journalize(ch->getName(), name, action, cashCost, 0, corp_cash, 0, num);
  }

  keeper->saveItems(shop_nr);
  ch->doQueueSave();
}
```

### Money Statistics Tracking

All money transactions are tracked for economic analysis via `moneyTypeT`:

| Type | Purpose |
|------|---------|
| `GOLD_XFER` | Player-to-player transfers (give, split) |
| `GOLD_INCOME` | Money from ground/corpses |
| `GOLD_SHOP` | Shop transactions |
| `GOLD_REPAIR` | Repair shop payments |
| `GOLD_GAMBLE` | Gambling wins/losses |
| `GOLD_RENT` | Rent payments |
| `GOLD_TITHE` | Faction tithing |

The `addToMoney()` function tracks these statistics:

```cpp
void TBeing::addToMoney(int money, moneyTypeT type, bool allowTithe) {
  setMoney(getMoney() + money);

  if (isPc() && GetMaxLevel() <= 60) {
    gold_statistics[type][GetMaxLevel() - 1] += money;
    gold_positive[GOLD_INCOME][lev - 1] += max(money, 0);

    // Apply faction tithe on income
    if (money > 0 && allowTithe) {
      int amount = money * FactionInfo[getFaction()].faction_tithe / 100.0;
      setMoney(getMoney() - amount);
      FactionInfo[getFaction()].addToMoney(amount);
    }
  }
}
```

### TMoney Special Properties

Money objects have special behaviors:

**Always carryable:**
```cpp
bool TMoney::canCarryMe(const TBeing*, silentTypeT) const {
  return true;  // Can always pick up money regardless of weight/volume limits
}
```

**Scavengeable by mobs:**
```cpp
int TMoney::scavengeMe(TBeing*, TObj** best_o) {
  *best_o = this;  // Mobs will always try to pick up money
  return FALSE;
}
```

**Gold modifier on load:**
```cpp
void TMoney::onObjLoad() {
  // Adjust money by global economic modifier
  int x = getMoney();
  x = (int)(x * gold_modifier[GOLD_INCOME].getVal());
  setMoney(x);
}
```

## Key Functions Reference

| Function | File | Purpose |
|----------|------|---------|
| `shopping_buy()` | shop.cc | Handle buy command |
| `shopping_sell()` | shop.cc | Handle sell command |
| `TObj::shopPrice()` | shop.cc | Calculate buy price |
| `TObj::sellPrice()` | shop.cc | Calculate sell price |
| `TObj::adjPrice()` | shop.cc | Structure-adjusted value |
| `shopData::getProfitBuy()` | shop.cc | Get buy multiplier |
| `shopData::getProfitSell()` | shop.cc | Get sell multiplier |
| `TShopOwned::doBuyTransaction()` | shopowned.cc | Process purchase |
| `TShopOwned::doSellTransaction()` | shopowned.cc | Process sale |
| `TShopOwned::doReserve()` | shopowned.cc | Balance cash reserves |
| `TShopOwned::chargeTax()` | shopowned.cc | Collect sales tax |
| `TShopOwned::journalize()` | shopaccounting.cc | Record journal entry |
| `shoplog()` | shop.cc | Log transaction |
| `create_money()` | obj_money.cc | Create TMoney object |
| `TMoney::willMerge()` | obj_money.cc | Check if piles can merge |
| `TMoney::doMerge()` | obj_money.cc | Merge two money piles |
| `TMoney::getMe()` | obj_money.cc | Handle pickup |
| `TMoney::moneyMeMoney()` | obj_money.cc | Convert to player money |
| `TBeing::giveMoney()` | utility.cc | Transfer between beings |
| `TBeing::addToMoney()` | utility.cc | Add/remove with statistics |
| `TBeing::doSplit()` | other.cc | Split with group |
| `TBeing::doDrop()` (money) | inventory.cc | Drop money |
| `TBeing::doGive()` (money) | inventory.cc | Give money |
