---
title: Economy System
category: important
keywords: [shops, pricing, banking, corporations, money, TMoney, transactions, accounting, reserves, tax]
related: [faction-system.md, object-system.md]
primary_symbols:
  functions: [doBuyTransaction, doSellTransaction, create_money, giveMoney, shoplog, journalize, doReserve, chargeTax, adjPrice, getProfitBuy, getProfitSell]
  classes: [shopData, TShopOwned, TCorporation, TShopJournal, TMoney, TMergeable]
  files: [code/code/misc/shop.cc, code/code/misc/shopowned.cc, code/code/misc/shopaccounting.cc, code/code/misc/corporation.cc, code/code/obj/obj_money.cc]
---

## Overview

How does a MUD prevent money from appearing out of thin air when the server crashes mid-transaction? How does it track whether a player-owned shop is profitable? How does it ensure multiple piles of coins automatically combine into one?

SneezyMUD's economy system manages all money flow through shops, banking, corporations, and direct player transactions. Unlike most MUDs where money is just a number on the player, money exists as physical TMoney objects that can be dropped, picked up, and automatically merge when placed together. The system provides player-owned shops with automated accounting, tax collection, reserve management, and double-entry bookkeeping that tracks every transaction.

The system exists because crashes during money transfers can duplicate or destroy currency. Every money transfer requires immediate saves of both parties. Player-owned shops must track profitability, manage inventory, balance cash reserves with their owning corporation, pay taxes, and provide financial reports. The banking system implements fractional reserve banking where central banks set reserve requirements for regular banks.

### Core Concepts

**Shop types:** NPC shops are configured in the database with fixed pricing. Player-owned shops extend NPC shops with custom pricing tiers, permission systems, automated reserve management, and full accounting.

**Money as objects:** TMoney inherits from TMergeable. Physical money objects exist in rooms and on corpses. When picked up, the object is destroyed and the amount is added to the player's money counter. When dropped, a new TMoney object is created. Multiple money piles of the same currency automatically merge when placed in the same container.

**Transaction flow:** All shop transactions follow a pattern: validate, calculate price, transfer money, log transaction, save both parties. Player-owned shops add: deduct expenses, pay dividends to corporation, balance reserves, charge tax, record journal entry.

**Pricing system:** Shop prices multiply the item's structure-adjusted value by profit multipliers and charisma modifiers. Player-owned shops support three customization tiers: per-item, per-keyword, and per-player pricing.

**Reserve system:** Player-owned shops automatically balance cash between the shop and its owning corporation. When cash falls below the minimum reserve, money is withdrawn from the corporation. When cash exceeds the maximum reserve, money is deposited to the corporation.

**Corporation ownership:** Corporations own multiple shops and provide shared banking. Shops pay dividends to their corporation on sales. Corporation members have permission-based access to view finances, withdraw money, and manage settings.

**Accounting system:** Player-owned shops maintain double-entry bookkeeping with a chart of accounts. Every transaction generates journal entries that debit and credit the appropriate accounts. Cost of Goods Sold tracks average purchase price for inventory valuation.

### Common Scenarios

**Player buys from shop:**
- Shop calculates price based on structure-adjusted value times profit_buy multiplier times charisma modifier
- Player gives money to keeper via giveMoney
- For player-owned shops: deduct expenses, pay dividend to corporation, balance reserves, charge tax, record journal entries
- Both keeper and player save immediately
- If crash occurs after money transfer but before save, money is duplicated

**Player sells to shop:**
- Shop calculates offer based on structure-adjusted value times profit_sell multiplier divided by charisma modifier
- Damaged items receive additional 60% base deduction
- Shop checks it has enough cash and inventory space
- Keeper gives money to player
- Item is added to shop inventory
- Both parties save immediately

**Money on ground merges:**
- Player drops 100 talens in room containing 50 talens
- TMergeable system detects same currency type
- Piles automatically merge into single 150 talen object
- Room contains one money pile instead of two

**Shop reserve balancing:**
- Shop cash falls below minimum reserve of 100,000 talens
- Shop has maximum reserve of 500,000 talens
- Shop automatically withdraws from corporation to reach midpoint of 300,000
- Corporation banker gives money to keeper
- Both save immediately

**Tax collection:**
- Player buys item for 1,000 talens
- Tax office has profit_buy of 0.05
- Shop charges 50 talen tax
- Tax is transferred to tax office keeper
- Tax transaction is logged separately from sale

## Patterns

### Money Transfer Safety

**Always save both parties immediately after money transfer:**
```cpp
// CORRECT: Both parties save
keeper->giveMoney(ch, amount, GOLD_SHOP);
keeper->saveItems(shop_nr);
ch->doQueueSave();

// WRONG: Missing save
keeper->giveMoney(ch, amount, GOLD_SHOP);
// Crash here duplicates money
```

**Rationale:** If the server crashes between the money transfer and the save, the money transfer persists in memory but never reaches the database. On restart, the giver still has the money and the receiver gained the money. This creates inflation.

**Always validate sender has sufficient funds:**
```cpp
// CORRECT: Check before transfer
if (keeper->getMoney() < amount) {
  keeper->doTell(ch->getName(), shop_index[shop_nr].missing_cash1);
  return false;
}
keeper->giveMoney(ch, amount, GOLD_SHOP);

// WRONG: Transfer without checking
keeper->giveMoney(ch, amount, GOLD_SHOP);
// Keeper goes negative
```

**Rationale:** giveMoney does not validate balances. It will happily make the sender's money negative. This creates money out of nothing.

**Never use wrong transaction type in shoplog or journalize:**
Each transaction type affects different accounts in double-entry bookkeeping. Using TX_BUYING when you mean TX_SELLING will credit Sales instead of debiting COGS, creating incorrect financial reports.

### Pricing Pattern

**Always use adjPrice for shop transactions:**
```cpp
// CORRECT: Structure-adjusted price
int price = obj->adjPrice() * profit_buy * charisma_modifier;

// WRONG: Full value for damaged item
int price = obj->getValue() * profit_buy * charisma_modifier;
```

**Rationale:** Items with reduced structure points are worth less. A sword at 10% durability should sell for 10% of base price, not 100%.

**Always enforce profit_buy greater than profit_sell:**
```cpp
// CORRECT: Validate before setting
if (profit_buy < profit_sell) {
  sendTo("You can't set your buy profit lower than your sell profit!");
  return FALSE;
}

// WRONG: Allow arbitrage
shop.profit_buy = 0.9;
shop.profit_sell = 1.1;
// Players can buy for 900 and sell for 1100
```

**Rationale:** If profit_buy is less than profit_sell, players can buy items and immediately sell them back for profit, draining the shop.

**Apply charisma modifier correctly based on transaction direction:**
For player buying: multiply by charisma penalty. For player selling: divide by charisma penalty. Never invert this or high-charisma players pay more instead of less.

### Shop Inventory Management

**Always check inventory limits before accepting items:**
```cpp
// CORRECT: Validate before purchase
if (shop_index[shop_nr].getInventoryCount() >= MAX_SHOP_INVENTORY) {
  keeper->doTell(ch->getName(), "My inventory is full!");
  return TRUE;
}

// WRONG: Accept unlimited inventory
// Shop database fills with junk
```

**Rationale:** Shops have a hard limit of 2500 items. Exceeding this causes database performance issues and shop operations fail.

**Never sell items the shop doesn't trade:**
Each shop has a whitelist of item types in the shoptype table. Attempting to sell disallowed types should be rejected immediately. Check the shop's type list before allowing the transaction.

**For strung items, match by short description:**
Strung items have unique descriptions but no fixed vnum. When counting shop inventory, query by short_desc and the ITEM_STRUNG flag, not by vnum.

### Reserve and Corporation Patterns

**Always set reserve min/max at least 100k apart:**
```cpp
// CORRECT: Adequate spread
reserve_min = 100000;
reserve_max = 500000;

// WRONG: Too close
reserve_min = 100000;
reserve_max = 150000;
// Thrashes between deposit and withdrawal
```

**Rationale:** If reserves are too close, small transactions cause constant deposits and withdrawals, creating transaction log spam and database load.

**Never withdraw from corporation without checking balance:**
```cpp
// CORRECT: Validate corp has funds
if (corp.getMoney() < amt) {
  // Can't withdraw, shop stays low
  return 0;
}
corp.setMoney(corp.getMoney() - amt);

// WRONG: Withdraw without checking
corp.setMoney(corp.getMoney() - amt);
// Corp goes negative
```

**Rationale:** Corporations share money across multiple shops. One shop draining the corporation can break reserve balancing for all other shops.

### Banking Patterns

**Always calculate interest on daily compound basis:**
The procBankInterest scheduler runs daily. Interest uses profit_sell of the bank as the annual rate, divided by 365 for daily compounding. Fractional talens accumulate in earned_interest until they reach 1.0, then transfer to the principal.

**Never allow bank withdrawal without verifying central bank reserve requirements:**
Regular banks must maintain reserves at their central bank equal to total deposits times the central bank's profit_buy ratio. Withdrawals that would violate this requirement must be rejected.

### Tax Patterns

**Exempt shop owners from tax on their own purchases:**
When the buyer is listed in shopownedaccess for the shop, skip tax collection. This prevents circular taxation where owners are taxed for stocking their own inventory.

**Never charge tax on commodity or casino transactions:**
These transaction types are exempt from sales tax. Check the transaction type before calling chargeTax.

**Default to Grimhaven tax office if no tax_nr set:**
If a shop's tax_nr is not set or invalid, default to shop_nr 14, the Grimhaven tax office. This ensures all transactions are taxed even if shop configuration is incomplete.

### Accounting Patterns

**Always journalize in transaction pairs:**
Every transaction affects at least two accounts. When journalizing a sale, debit Cash and credit Sales. When recording COGS, debit COGS and credit Inventory. Missing entries break the double-entry system.

**Track COGS with running average:**
When shop purchases items from players, add the cost to the running total in shoplogcogs. When shop sells items, calculate average cost and debit that amount to COGS. This provides accurate profit calculation.

**Close books at year-end:**
The closeTheBooks function archives the current year's journal entries to shoplogjournalarchive and carries forward balances to Retained Earnings. This must run annually or journal tables grow unbounded.

### Common Mistakes

**Forgetting to deduct expenses before paying dividends:**
Expenses must be deducted first, then dividends calculated on net revenue. If you pay dividends on gross revenue, the shop loses money on every transaction.

**Not checking DELETE flags after money transfers:**
giveMoney can trigger DELETE_THIS or DELETE_VICT if one party dies during the transfer (e.g., from a spec proc or trigger). Always check return codes and stop execution if deletion flags are set.

**Accessing desc->autobits without checking desc exists:**
Mobs do not have descriptors. Before checking AUTO_SPLIT or other descriptor flags, verify desc is not null or you'll dereference null.

**Using %r format specifier with user-provided shop names:**
User input in shop names or item descriptions must use %s format with proper SQL escaping. The %r specifier bypasses escaping and allows SQL injection.

## Reference

### Symbol Quick Reference

| Symbol | Type | Purpose |
|--------|------|---------|
| `shopData` | class | NPC shop configuration and pricing |
| `TShopOwned` | class | Player-owned shop operations and accounting |
| `TCorporation` | class | Multi-shop corporate ownership |
| `TShopJournal` | class | Double-entry bookkeeping journal |
| `TMoney` | class | Physical money object (inherits TMergeable) |
| `doBuyTransaction` | function | Process player purchase with accounting |
| `doSellTransaction` | function | Process player sale with accounting |
| `doReserve` | function | Balance shop cash with corporation |
| `chargeTax` | function | Collect sales tax and transfer to tax office |
| `journalize` | function | Record transaction in accounting journal |
| `shoplog` | function | Log transaction to shoplog table |
| `create_money` | function | Create TMoney object from template |
| `giveMoney` | function | Transfer money between beings with statistics |
| `adjPrice` | function | Calculate structure-adjusted item value |
| `getProfitBuy` | function | Get buy multiplier with custom tier lookup |
| `getProfitSell` | function | Get sell multiplier with custom tier lookup |

### Shop Access Flags

| Flag | Bit | Permission |
|------|-----|------------|
| SHOPACCESS_OWNER | 0 | Full administrative access |
| SHOPACCESS_INFO | 1 | View shop information and stats |
| SHOPACCESS_RATES | 2 | Modify pricing multipliers |
| SHOPACCESS_GIVE | 3 | Withdraw money from shop |
| SHOPACCESS_SELL | 4 | Sell the shop to another player |
| SHOPACCESS_ACCESS | 5 | Grant/revoke permissions to others |
| SHOPACCESS_LOGS | 6 | View transaction logs |
| SHOPACCESS_DIVIDEND | 7 | Set dividend rate to corporation |

### Corporation Access Flags

| Flag | Bit | Permission |
|------|-----|------------|
| CORPACCESS_PARTNER | 0 | Full partner privileges |
| CORPACCESS_INFO | 1 | View corporate information |
| CORPACCESS_GIVE | 3 | Withdraw money from corp bank |
| CORPACCESS_ACCESS | 5 | Manage member permissions |
| CORPACCESS_LOGS | 6 | View corporate transaction logs |

### Transaction Types

| Type | Purpose | Affects |
|------|---------|---------|
| TX_BUYING | Player buys item | Cash, Sales, COGS |
| TX_BUYING_SERVICE | Player buys service | Cash, Sales (no COGS) |
| TX_RECYCLING | Scrap value transaction | Cash, Recycling revenue |
| TX_SELLING | Player sells item | Cash, Inventory |
| TX_PRODUCING | Shop produces item | Inventory, Production cost |
| TX_GIVING_TALENS | Owner deposits money | Cash, Paid-in Capital |
| TX_RECEIVING_TALENS | Owner withdraws money | Cash, Dividends |
| TX_PAYING_INTEREST | Bank pays interest | Cash, Interest Expense |
| TX_WITHDRAWAL | Bank withdrawal | Cash, Deposits liability |
| TX_DEPOSIT | Bank deposit | Cash, Deposits liability |
| TX_FACTORY | Factory production | Inventory, Production cost |

### Money Statistics Types

| Type | Purpose |
|------|---------|
| GOLD_XFER | Player-to-player transfers |
| GOLD_INCOME | Money from ground/corpses/loot |
| GOLD_SHOP | Shop buy/sell transactions |
| GOLD_REPAIR | Repair shop payments |
| GOLD_GAMBLE | Gambling wins/losses |
| GOLD_RENT | Rent payments |
| GOLD_TITHE | Faction automatic tithing |

### Currency Types

| Currency | Name | Faction | Vnum |
|----------|------|---------|------|
| CURRENCY_GRIMHAVEN | talen | FACT_NONE | 13 |
| CURRENCY_LOGRUS | dinar | FACT_CULT | 13 |
| CURRENCY_BRIGHTMOON | kroner | FACT_BROTHERHOOD | 13 |
| CURRENCY_AMBER | guilder | FACT_SNAKE | 13 |

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

### Key Constants

| Constant | Value | Purpose |
|----------|-------|---------|
| MAX_SHOP_INVENTORY | 2500 | Hard limit on shop item count |
| SHOP_PURCHASE_FEE | 1000000 | Flat fee for buying a shop |
| SHOP_PURCHASE_MARKUP | 1.15 | Multiplier on (cash + inventory value) |
| GRIMHAVEN_TAX_OFFICE | 14 | Default tax office shop_nr |
| RESERVE_MIN_SPREAD | 100000 | Minimum gap between reserve min/max |
| Obj::GENERIC_TALEN | 13 | Template vnum for money objects |

### Special Shop Number Ranges

| Range | Purpose |
|-------|---------|
| 127-134 | Repair shops |

### Key Database Tables

| Table | Purpose |
|-------|---------|
| shop | Base shop configuration |
| shoptype | Item types shop trades |
| shopmaterial | Materials shop accepts |
| shopproducing | Items shop produces |
| shopowned | Player ownership data |
| shopownedaccess | Player permissions |
| shopownedratios | Per-vnum pricing |
| shopownedmatch | Keyword pricing |
| shopownedplayer | Per-player pricing |
| shopownedrepair | Repair shop settings |
| shopownedbank | Player bank accounts |
| shopownedcorpbank | Corporate bank accounts |
| shopownedcentralbank | Central bank reserve links |
| shopownedloans | Active player loans |
| shopownedloanrate | Loan terms |
| shoplog | Transaction history |
| shoplogjournal | Current year accounting journal |
| shoplogjournalarchive | Past year journal entries |
| shoplogcogs | Cost of Goods Sold tracking |
| shoplogaccountchart | Chart of accounts definitions |
| corporation | Corporation definitions |
| corpaccess | Corporate member permissions |
| corplog | Corporate transaction log |

## Implementation

### Shop Structure and Configuration

NPC shops are defined in the shop database table with base configuration including keeper mob vnum, room location, operating hours, and profit multipliers. The shop_nr serves as the primary key across all shop-related tables.

When a player purchases a shop, a row is added to shopowned that links to the base shop. The purchase price is calculated as the shop's current cash plus inventory value, multiplied by 1.15, plus a flat 1,000,000 talen fee. This prevents players from buying profitable shops cheaply.

Player-owned shops can customize pricing at three tiers with priority ordering. The highest priority is per-player pricing stored in shopownedplayer, which allows different prices for specific character names. Next is per-vnum pricing in shopownedratios for specific item numbers. Lowest priority is keyword matching in shopownedmatch where any item whose name contains the keyword receives the custom multiplier. These tiers are checked in order during getProfitBuy and getProfitSell.

### TMoney Implementation

Money exists as physical objects descended from TMergeable. The TMoney class stores an amount and currency type. All money objects are created from the template vnum 13 by create_money, which clones the template and converts it to a strung object with custom descriptions based on amount.

The descriptions scale with quantity. A single coin is "a talen" while small piles are "some talens" and large piles get increasingly dramatic descriptions like "A HUGE pile of talens lies here." Physical properties also scale: volume is approximately 0.0048 cubic units per coin and weight is approximately 1.5 grams per coin.

Currency type determines the coin name and which faction it belongs to. Grimhaven uses talens, Logrus uses dinars, Brightmoon uses kroners, and Amber uses guilders. An overload of create_money accepts a faction and automatically selects the appropriate currency.

When TMoney objects are placed in the same container, the TMergeable base class checks willMerge to see if they are the same currency type. If so, doMerge adds the amounts together, removes one object from the container, and deletes it. This happens automatically through the operator+= in container management, ensuring players never see multiple separate piles of the same currency.

When a player picks up money, TMoney::getMe is called, which invokes moneyMeMoney. This function removes the object from its container, adds the amount to the player's money counter via addToMoney, triggers auto-split if enabled, and returns DELETE_THIS to signal the object should be deleted. Money does not persist as objects in player inventory - it is immediately converted to an integer.

When a player drops money, doDrop creates a new TMoney object via create_money, adds it to the room with operator+=, and deducts the amount from the player's money counter. If money already exists in the room, the new object automatically merges with it.

### Price Calculation Flow

Shop pricing starts with adjPrice which returns the item's base value multiplied by the ratio of current structure points to maximum structure points. An item at full durability returns full value. An item at 50% durability returns half value. Items with zero max structure points are not durability-tracked and always return full value.

For player purchases, shopPrice multiplies adjPrice by profit_buy and then by the charisma modifier. The charisma modifier is calculated as the player's charisma shop penalty minus their swindle skill bonus, with a minimum of 1.0. High charisma and high swindle both reduce this modifier, making prices lower.

For shop purchases from players, sellPrice multiplies adjPrice by profit_sell and then divides by the charisma modifier. This inverts the charisma effect so high charisma players receive more money when selling.

If the item has reduced structure points, sellPrice applies an additional penalty. It multiplies the price by 0.6 and then by the structure ratio, resulting in damaged items being worth significantly less than their durability alone would suggest.

Custom pricing tiers are checked in order during getProfitBuy and getProfitSell. The functions first check if the player's name exists in buy_player_cache, then check if the item vnum exists in buy_ratios_cache, then iterate through buy_matches_cache to see if any keyword matches the item name. The first match found is used. If no custom pricing exists, the base profit_buy or profit_sell from the shop table is returned.

### Transaction Processing

Player purchases flow through shopping_buy to TObj::buyMe to TObj::purchaseMe. At each stage, validations occur: shop hours, item visibility, weight and volume limits, player money availability. Once validated, purchaseMe calls doBuyTransaction.

doBuyTransaction handles player-owned shop logic. It first calls doExpenses which deducts the expense_ratio portion of the cost and logs it as an expense. Then it transfers money from player to keeper using giveMoney with GOLD_SHOP type. The transaction is logged to shoplog with positive talen amount and the action "buying".

For owned shops, doDividend calculates the dividend percentage of the net cost and transfers that amount from the keeper to the corporation's bank. Then doReserve checks if the keeper's cash is outside the reserve range and balances with the corporation if needed. chargeTax calculates tax as the cost multiplied by the tax office's profit_buy ratio and transfers that to the tax office keeper. Finally journalize records double-entry accounting for the transaction, debiting Cash and crediting Sales, and debiting COGS and crediting Inventory based on the average cost from shoplogcogs.

Both keeper and player save immediately via saveItems and doQueueSave. If the server crashes after the money transfer but before these saves, the transfer persists in memory but not in the database, duplicating money on restart.

Player sales flow through shopping_sell to generic_sell to TObj::sellMe to TObj::sellMeMoney. Validations check that the shop trades this item type, that the item is not excluded due to damage or burning, that the shop has inventory space, and that the keeper has sufficient money.

sellMeMoney calls doSellTransaction which transfers money from keeper to player, saves the item to shop inventory, calls doReserve to balance cash, and journalizes the transaction debiting Inventory and crediting Cash. Both parties save immediately.

### Reserve System Mechanics

The doReserve function checks the keeper's current money against reserve_min and reserve_max. If money is below the minimum, it calculates the midpoint between min and max, subtracts current money, and withdraws that amount from the corporation. The corporation's money is decremented and the banker gives that amount to the keeper via giveMoney.

If money is above the maximum, the midpoint calculation determines how much to deposit. The keeper gives that amount to the banker and the corporation's money is incremented. Both save immediately.

Reserve limits must be at least 100,000 talens apart. If they are too close, small transactions cause constant reserve balancing, creating transaction log spam. If reserve_min and reserve_max are both zero, reserve balancing is disabled.

The corporation must have sufficient funds for withdrawals. If the corporation balance is less than the withdrawal amount, the withdrawal is skipped and the shop remains below minimum reserves. This prevents one shop from draining the corporation and breaking all other shops.

### Corporation Structure

Corporations are defined in the corporation table with an auto-incrementing corp_id, name, and bank shop_nr. Multiple shops reference the same corp_id in their shopowned row, linking them to the corporation.

Corporate money is stored in shopownedcorpbank at the corporation's designated bank. This is a separate balance from player accounts, indexed by corp_id instead of player_id. Interest accrues daily based on the bank's profit_sell ratio as an annual rate.

Corporation members are defined in corpaccess with permission flags. CORPACCESS_PARTNER grants full access. Other flags control info viewing, money withdrawal, permission management, and log access. These are checked before allowing corporate operations.

Corporate dividends are paid by shops when doDividend is called during doBuyTransaction. The dividend percentage is multiplied by the net cost after expenses, and that amount is transferred from keeper to corporation banker. This is logged as a separate transaction and recorded in journal entries debiting Dividends and crediting Cash.

### Banking Implementation

Banks are shops with the SPEC_BANKER spec proc. They maintain player accounts in shopownedbank and corporate accounts in shopownedcorpbank. Each account stores talens and earned_interest, where fractional interest accumulates until reaching 1.0 and then transfers to the principal.

The procBankInterest scheduler runs daily on all banks. For each account, it calculates daily interest as balance times annual rate divided by 365. This is added to earned_interest. Then the integer portion of earned_interest is added to talens and subtracted from earned_interest, leaving only the fractional remainder.

Central banks are designated with the SPEC_CENTRAL_BANKER spec proc. They set reserve requirements for regular banks via shopownedcentralbank which links a regular bank shop_nr to a central bank shop_nr. The reserve requirement is calculated as total deposits at the regular bank multiplied by the central bank's profit_buy ratio.

When a player attempts to withdraw from a bank, the bank checks if it has a central bank link. If so, it calculates required reserves and compares to current reserves at the central bank. If the withdrawal would violate the requirement, it is rejected. This implements fractional reserve banking where banks must maintain a percentage of deposits in reserve.

### Tax Collection

The chargeTax function determines the appropriate tax office by checking the shop's tax_nr field in shopowned. If not set or invalid, it defaults to GRIMHAVEN_TAX_OFFICE (shop_nr 14). The tax amount is calculated as the transaction cost multiplied by the tax office's profit_buy ratio for that item.

The keeper transfers the tax amount to the tax office keeper via giveMoney with GOLD_SHOP type. This is logged as a separate shoplog entry with action "tax". For journal entries, the tax amount is debited to the Tax expense account.

Tax is exempted for shop owners buying from their own shop by checking if the buyer name exists in shopownedaccess. Casino transactions and commodity purchases are also exempt by checking transaction type before calling chargeTax.

### Accounting System Details

The journalize function implements double-entry bookkeeping by creating paired entries in shoplogjournal. Each transaction generates at least two entries: one debit and one credit to different accounts, with amounts balanced.

For a typical purchase transaction (TX_BUYING), journalize creates entries debiting Cash for the amount received and crediting Sales for the revenue. Then it retrieves the average cost from shoplogcogs, debits COGS for that amount, and credits Inventory. Additional entries debit Expenses for the expense_ratio portion, debit Dividends for the dividend amount, and debit Tax for the tax amount.

The shoplogcogs table maintains running totals for each item by name. When the shop purchases an item from a player via TX_SELLING, COGS_add increments the count and total_cost. When the shop sells an item via TX_BUYING, COGS_get calculates the average cost as total_cost divided by count, multiplied by the quantity being sold.

At year-end, closeTheBooks is called to archive journal entries. It moves all entries from shoplogjournal to shoplogjournalarchive with a year timestamp, calculates net income by summing all revenue and expense accounts, creates a final entry transferring net income to Retained Earnings, and clears the temporary accounts for the new year.

### Transaction Logging

Every transaction is logged to shoplog regardless of ownership. The shoplog function inserts a row with shop_nr, player name, action string, item name, talen amount, keeper's resulting balance, shop inventory value, timestamp, and item count.

Positive talen amounts indicate money flowing to the keeper (purchases). Negative amounts indicate money flowing from the keeper (sales). The shoptalens and shopvalue fields provide a running snapshot of shop financial state at each transaction.

Action strings are standardized: "buying", "selling", "producing", "recycling", "deposit", "withdrawal", "interest", "tax". These allow filtering and analysis of transaction types.

For non-player transactions like reserve balancing or dividend payments, the name field contains the entity name like "DIVIDEND" or "RESERVE". This distinguishes automated transactions from player actions.

### Special Shop Types

Repair shops are identified by shop_nr in the range 127-134. The isRepairShop function checks this range. Repair shops use shopownedrepair to store speed and quality multipliers. Speed divides profit_buy, making faster repairs more expensive. Quality multiplies profit_buy, making higher quality repairs more expensive. The repair cost calculation uses adjPrice times these modifiers.

Loan sharks have the SPEC_LOAN_SHARK spec proc and use shopownedloans to track active loans and shopownedloanrate to define interest rates and terms. When a player takes a loan, a row is inserted with amount, interest rate, and due date. The spec proc checks for overdue loans and applies penalties.

Auctioneers have the SPEC_AUCTIONEER spec proc and use shopownedauction for listings. Players can list items for auction with a minimum bid and duration. Other players bid via commands handled by the spec proc. When the auction expires, the highest bidder receives the item and the seller receives payment minus the auctioneer's commission.

### Money Transfer Internals

The giveMoney function validates that the amount is non-negative, then calls addToMoney on the giver with a negative amount and addToMoney on the receiver with a positive amount. Both calls specify the moneyTypeT for statistics tracking.

The addToMoney function increments or decrements the being's money counter, then updates global statistics arrays indexed by transaction type and level. For players below level 60, it tracks positive income separately for economic analysis. If the being is a player and the amount is positive income, it calculates the faction tithe as a percentage of the amount, deducts it from the being's money, and adds it to the faction's accumulated tithe balance.

Money statistics are stored in global arrays gold_statistics and gold_positive, indexed by GOLD_* type and level. These track money flow throughout the economy and are used by immortals to monitor inflation, identify money fountains and sinks, and balance the economy.

### Money Command Processing

The doDrop function handles dropping money by checking if the second argument abbreviates "talens". If so, it parses the amount as either a number or "all". It validates the player has that much money, creates a TMoney object via create_money, adds it to the room with operator+=, deducts from the player via addToMoney with GOLD_INCOME type, and saves the player.

The doGive function handles giving money by checking if the object name is a number or "all" and the target exists. It validates the recipient can receive items by checking for solo quest flag, hand availability, and other restrictions. It calls giveMoney to transfer the amount, then saves both characters via saveChar with AUTO_RENT flag to immediately persist the transfer.

The doSplit function handles group money splitting by finding the group leader, counting shares for all grouped members in the same room, calculating per-share amount, deducting from the splitter, and distributing to each member via addToMoney. The splitShares helper function returns the number of shares for each member, typically 1 but can be higher for special cases.

Auto-split is triggered during money pickup if the player has AFF_GROUP, has a descriptor with AUTO_SPLIT enabled, and has a master or followers. It formats the pickup amount as a string and calls doSplit with tell suppressed to avoid spam.

### Greedy Mob Interaction

When a player picks up money, moneyMeMoney iterates through all beings in the room checking for greedy mobs. Greedy mobs have their urgency increased by 1 plus the money amount divided by 1000. Higher urgency makes the mob more likely to act on its greed by attacking the player or demanding money. The mob's AI target is set to the player who picked up the money.

This creates emergent gameplay where picking up large amounts of money in the presence of greedy mobs is dangerous. The urgency scaling means small pickups are relatively safe while large pickups provoke immediate aggression.

## Troubleshooting

### Money Duplication

**Symptom:** Shop has negative balance or players report receiving money twice for the same transaction.

**Likely cause:** Server crashed between giveMoney call and save calls.

**Diagnostic approach:** Check shoplog for duplicate entries with same timestamp. Check server logs for crash time. Examine transaction code to verify both saveItems and doQueueSave are called after giveMoney.

**Fix:** Add or verify save calls immediately after all money transfers. Consider adding transaction boundaries if using database transactions. Review all giveMoney call sites to ensure saves follow.

### Shop Cannot Afford Purchases

**Symptom:** Keeper tells player "I can't afford that" despite shop showing adequate balance.

**Likely cause:** Reserve system withdrew money to corporation, or corporation drained by other shops.

**Diagnostic approach:** Check keeper's current money via debug commands. Check corporation balance in shopownedcorpbank. Check recent shoplog entries for reserve withdrawals. Verify reserve_min and reserve_max settings are appropriate.

**Fix:** If corporation is drained, deposit money to corporate bank. Adjust reserve_min/reserve_max to maintain adequate shop cash. Consider increasing reserve_max if shop frequently hits the limit.

### Pricing Arbitrage

**Symptom:** Players buying items and immediately reselling for profit.

**Likely cause:** profit_buy is less than or equal to profit_sell, creating inverted pricing.

**Diagnostic approach:** Check shop table profit_buy and profit_sell values. Calculate effective buy price and sell price for a test item. Verify buy price is higher than sell price.

**Fix:** Ensure profit_buy is significantly greater than profit_sell. Typical values are profit_buy 1.5 to 3.0 and profit_sell 0.3 to 0.7. Wider spreads prevent arbitrage.

### Reserve Thrashing

**Symptom:** Shoplog shows constant deposit and withdrawal transactions. Corporation money oscillates rapidly.

**Likely cause:** reserve_min and reserve_max are too close together.

**Diagnostic approach:** Check reserve_min and reserve_max values in shopowned. Calculate spread as max minus min. Check shoplog frequency of reserve transactions.

**Fix:** Set reserve_min and reserve_max at least 100,000 talens apart. Larger spreads reduce transaction frequency. Typical values are min 100k, max 500k for medium shops.

### Incorrect Profit Reports

**Symptom:** Shop financial reports show incorrect profit or loss. COGS does not match sales.

**Likely cause:** Wrong transaction type used when calling journalize, or COGS tracking not updated.

**Diagnostic approach:** Query shoplogjournal for recent transactions. Verify debit and credit entries balance. Check shoplogcogs for item count and total_cost accuracy. Trace transaction code to verify TX_* type matches actual operation.

**Fix:** Ensure all doBuyTransaction calls use TX_BUYING and all doSellTransaction calls use TX_SELLING. Verify COGS_add is called when purchasing inventory and COGS_get is called when selling. Manually correct shoplogcogs if necessary.

### Tax Not Collected

**Symptom:** No tax entries in shoplog despite transactions occurring.

**Likely cause:** Tax office shop_nr is invalid, or transaction type is exempt.

**Diagnostic approach:** Check tax_nr in shopowned table. Verify tax office shop exists and has a keeper. Check if buyer is shop owner (exempt). Check if transaction is commodity or casino type (exempt).

**Fix:** Set valid tax_nr in shopowned. Ensure tax office keeper exists and has SPEC_TAXMAN. If intentionally exempt, verify exemption logic is correct.

### Money Pile Does Not Merge

**Symptom:** Multiple separate money piles in same location when they should combine.

**Likely cause:** Currency types differ, or merge code is bypassed.

**Diagnostic approach:** Check currency type of both piles. Verify both are in same container (same room or same inventory). Enable debug logging in TMergeable to trace willMerge and doMerge calls.

**Fix:** Ensure both piles are same currency. If they are same currency and still not merging, check that operator+= in container management is correctly invoking merge logic. May indicate bug in TMergeable system requiring code fix.

### Bank Interest Not Accruing

**Symptom:** Player account balance does not increase despite deposits and time passing.

**Likely cause:** procBankInterest scheduler not running, or profit_sell is zero.

**Diagnostic approach:** Check if procBankInterest is in active scheduler tasks. Verify bank shop has non-zero profit_sell. Check shopownedbank earned_interest column for fractional accumulation.

**Fix:** Verify scheduler is running and procBankInterest is registered. Set appropriate profit_sell annual rate (typical 0.03 to 0.10). If earned_interest is accumulating but not transferring, check that integer conversion and update logic is correct.

### Central Bank Reserve Violation

**Symptom:** Regular bank allows withdrawal that should be blocked by reserve requirements.

**Likely cause:** Central bank link not configured, or reserve calculation is wrong.

**Diagnostic approach:** Check shopownedcentralbank for entry linking bank to central bank. Calculate required reserves as total deposits times central bank profit_buy. Compare to bank's actual reserves at central bank.

**Fix:** Add shopownedcentralbank entry linking the banks. Verify central bank has appropriate profit_buy reserve ratio (typical 0.10 to 0.20). Ensure withdrawal code checks reserve requirement before allowing transaction.
