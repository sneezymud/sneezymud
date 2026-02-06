---
title: Economy System
description: Complete economic infrastructure including NPC and player-owned shops, corporations, fractional reserve banking, taxes, and double-entry accounting.
category: critical
keywords: [shops, banking, corporations, pricing, transactions, currency, taxes, accounting]
primary_symbols:
  functions: [shopping_buy, shopping_sell, doBuyTransaction, doSellTransaction, create_money, giveMoney, addToMoney, doReserve, chargeTax, journalize, shoplog]
  classes: [shopData, TShopOwned, TCorporation, TShopJournal, TMoney]
  enums: [transactionTypeT, TX_BUYING, TX_BUYING_SERVICE, TX_RECYCLING, TX_SELLING, TX_PRODUCING, TX_GIVING_TALENS, TX_RECEIVING_TALENS, TX_PAYING_INTEREST, TX_WITHDRAWAL, TX_DEPOSIT, TX_FACTORY, CURRENCY_GRIMHAVEN, CURRENCY_LOGRUS, CURRENCY_BRIGHTMOON, CURRENCY_AMBER, SHOPACCESS_OWNER, SHOPACCESS_INFO, SHOPACCESS_RATES, SHOPACCESS_GIVE, SHOPACCESS_SELL, SHOPACCESS_ACCESS, SHOPACCESS_LOGS, SHOPACCESS_DIVIDEND, CORPACCESS_PARTNER, CORPACCESS_INFO, CORPACCESS_GIVE, CORPACCESS_ACCESS, CORPACCESS_LOGS, GOLD_XFER, GOLD_INCOME, GOLD_SHOP, GOLD_REPAIR, GOLD_GAMBLE, GOLD_RENT, GOLD_TITHE, SPEC_BANKER, SPEC_CENTRAL_BANKER, SPEC_TAXMAN, AUTO_SPLIT, MAX_SHOP_INVENTORY]
---

## Overview

How does money flow through a MUD with player-owned businesses, corporate structures, fractional reserve banking, and double-entry accounting? This system manages the complete economic infrastructure: from simple shop transactions to complex corporate dividend distributions and tax collection.

The economy system governs all monetary transactions in SneezyMUD. It encompasses NPC shops with fixed pricing, player-owned shops with customizable rates and access permissions, corporations that own multiple shops and share finances, a banking system with interest and reserve requirements, and a full accounting system with journal entries and year-end closing.

Money itself is represented as physical objects that can be dropped, given, and picked up. When you pick up a money pile, the object is consumed and the amount is added directly to your money counter. Money piles of the same currency type automatically merge when placed in the same location.

**Core economic flow:**

1. Players earn money from monster drops and quest rewards
2. Players spend at shops, which collect the money
3. Player-owned shops send dividends to their owning corporation
4. Shops automatically balance cash with corporate reserves
5. Tax is collected on transactions and sent to tax offices
6. Banks hold deposits and pay interest
7. All transactions are logged and journalized for accounting

The system supports four currency types associated with different factions: talens (default), dinars (Cult), kroners (Brotherhood), and guilders (Snake). Each currency is a separate monetary system with its own money objects.

---

## Patterns

### Money Transfer Safety

**Always save both parties after money transfers.** The `giveMoney()` function transfers money between beings, but if a crash occurs before saving, the money is duplicated (given but not deducted from source) or lost (deducted but not given to target). Call `saveItems()` for shopkeepers and `doQueueSave()` for players immediately after transfers.

```cpp
// CORRECT: Both parties save
keeper->giveMoney(ch, amount, GOLD_SHOP);
keeper->saveItems(shop_nr);
ch->doQueueSave();

// WRONG: Missing save - crash here duplicates money
keeper->giveMoney(ch, amount, GOLD_SHOP);
```

**Always check keeper balance before player sells.** Before calling `giveMoney()` to pay a player, verify the shopkeeper has sufficient funds. Use `getMoney() < amount` check and display `missing_cash1` message if insufficient.

```cpp
// CORRECT: Check before transfer
if (keeper->getMoney() < amount) {
  keeper->doTell(ch->getName(), shop_index[shop_nr].missing_cash1);
  return false;
}
keeper->giveMoney(ch, amount, GOLD_SHOP);
```

**Note that `giveMoney()` returns void.** It simply calls `addToMoney()` on both parties to transfer the amount. There are no DELETE flags to check after a money transfer.

### Shop Configuration

**Never set profit_buy lower than profit_sell.** This creates arbitrage opportunities where players can buy and immediately resell for profit, draining the shop. The system validates this but you should understand why: profit_buy is what the shop charges players (should be higher), profit_sell is what the shop pays players (should be lower).

**Always set reserve_min and reserve_max at least 100,000 apart.** Closer values cause constant thrashing as the shop oscillates between depositing and withdrawing from the corporation. The system enforces this minimum spread.

### Transaction Types

**Always use the correct transaction type enum.** The `transactionTypeT` enum controls how transactions are journalized. Using the wrong type (e.g., `TX_BUYING` for a service) causes incorrect COGS tracking and financial reports. Services use `TX_BUYING_SERVICE` which skips COGS entries.

### Inventory Management

**Check shop inventory count before accepting items.** The maximum is 2,500 items. Exceeding this causes inventory overflow. Call `getInventoryCount()` and reject the sale if at limit.

**Handle strung items specially for inventory counts.** Strung items (custom-named) must be matched by `short_desc` rather than vnum when checking how many the shop already has.

### Price Calculation Priority

When calculating prices for player-owned shops, the system checks in this order:
1. Player-specific pricing (highest priority)
2. Per-vnum pricing
3. Keyword matching
4. Default shop rates

Later checks override earlier ones. If you set both a vnum price and a player price for the same item/player combination, the player price wins.

### Common Mistakes

**Forgetting to deduct expenses before paying dividends.** Expenses must be deducted first, then dividends calculated on net revenue. Paying dividends on gross revenue causes the shop to lose money on every transaction.

**Accessing desc->autobits without checking desc exists.** Mobs do not have descriptors. Before checking AUTO_SPLIT or other descriptor flags, verify desc is not null.

**Using %r format specifier with user-provided shop names.** User input in shop names or item descriptions must use %s format with proper SQL escaping. The %r specifier bypasses escaping and allows SQL injection.

---

## Reference

### Symbol Quick Reference

| Symbol | Type | Purpose |
|--------|------|---------|
| `shopData` | class | NPC shop configuration and base operations |
| `TShopOwned` | class | Player-owned shop operations and customization |
| `TCorporation` | class | Corporation management and shared finances |
| `TShopJournal` | class | Double-entry accounting journal |
| `TMoney` | class | Physical currency object, inherits from TMergeable |
| `shopping_buy()` | function | Handle player buy command |
| `shopping_sell()` | function | Handle player sell command |
| `doBuyTransaction()` | function | Process purchase with accounting |
| `doSellTransaction()` | function | Process sale with accounting |
| `create_money()` | function | Create TMoney object from amount |
| `giveMoney()` | function | Transfer money between beings |
| `addToMoney()` | function | Modify money with statistics tracking |
| `doReserve()` | function | Balance shop cash with corporate reserves |
| `chargeTax()` | function | Collect sales tax on transactions |
| `journalize()` | function | Record double-entry journal entry |
| `shoplog()` | function | Log transaction to shoplog table |

### Key Constants

| Constant | Value | Purpose |
|----------|-------|---------|
| `MAX_SHOP_INVENTORY` | 2500 | Hard limit on shop item count |
| `SHOP_PURCHASE_FEE` | 1000000 | Flat fee for buying a shop |
| `SHOP_PURCHASE_MARKUP` | 1.15 | Multiplier on (cash + inventory value) |
| `GRIMHAVEN_TAX_OFFICE` | 14 | Default tax office shop_nr |
| `RESERVE_MIN_SPREAD` | 100000 | Minimum gap between reserve min/max |
| `Obj::GENERIC_TALEN` | 13 | Template vnum for money objects |

### Profit Multipliers

| Multiplier | Direction | Typical Range | Effect |
|------------|-----------|---------------|--------|
| `profit_buy` | Shop to player | 1.1 - 5.0 | Higher = more expensive to buy |
| `profit_sell` | Player to shop | 0.1 - 0.9 | Higher = shop pays more |

### Currency Types

| Enum | Name | Faction | Vnum |
|------|------|---------|------|
| `CURRENCY_GRIMHAVEN` | talen | FACT_NONE (default) | 13 |
| `CURRENCY_LOGRUS` | dinar | FACT_CULT | 13 |
| `CURRENCY_BRIGHTMOON` | kroner | FACT_BROTHERHOOD | 13 |
| `CURRENCY_AMBER` | guilder | FACT_SNAKE | 13 |

### Shop Access Flags

| Flag | Bit | Permission |
|------|-----|------------|
| `SHOPACCESS_OWNER` | 0 | Full access |
| `SHOPACCESS_INFO` | 1 | View shop info |
| `SHOPACCESS_RATES` | 2 | Change prices |
| `SHOPACCESS_GIVE` | 3 | Withdraw money |
| `SHOPACCESS_SELL` | 4 | Sell the shop |
| `SHOPACCESS_ACCESS` | 5 | Manage permissions |
| `SHOPACCESS_LOGS` | 6 | View transaction logs |
| `SHOPACCESS_DIVIDEND` | 7 | Set dividend rate |

### Corporation Access Flags

| Flag | Bit | Permission |
|------|-----|------------|
| `CORPACCESS_PARTNER` | 0 | Full partner access |
| `CORPACCESS_INFO` | 1 | View corp info |
| `CORPACCESS_GIVE` | 3 | Withdraw money |
| `CORPACCESS_ACCESS` | 5 | Manage permissions |
| `CORPACCESS_LOGS` | 6 | View logs |

### Transaction Types

| Enum | Use Case | Affects |
|------|----------|---------|
| `TX_BUYING` | Player buying item | Cash, Sales, COGS |
| `TX_BUYING_SERVICE` | Player buying service | Cash, Sales (no COGS) |
| `TX_RECYCLING` | Scrap value transaction | Cash, Recycling revenue |
| `TX_SELLING` | Player selling item | Cash, Inventory |
| `TX_PRODUCING` | Shop producing item | Inventory, Production cost |
| `TX_GIVING_TALENS` | Owner depositing money | Cash, Paid-in Capital |
| `TX_RECEIVING_TALENS` | Owner withdrawing money | Cash, Dividends |
| `TX_PAYING_INTEREST` | Bank interest payment | Cash, Interest Expense |
| `TX_WITHDRAWAL` | Bank withdrawal | Cash, Deposits liability |
| `TX_DEPOSIT` | Bank deposit | Cash, Deposits liability |
| `TX_FACTORY` | Factory production | Inventory, Production cost |

### Money Statistics Types

| Type | Tracks |
|------|--------|
| `GOLD_XFER` | Player-to-player transfers |
| `GOLD_INCOME` | Money from ground/corpses |
| `GOLD_SHOP` | Shop transactions |
| `GOLD_REPAIR` | Repair shop payments |
| `GOLD_GAMBLE` | Gambling wins/losses |
| `GOLD_RENT` | Rent payments |
| `GOLD_TITHE` | Faction tithing |

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

### Tax Exemptions

| Context | Why Exempt |
|---------|------------|
| Owner buying from own shop | Self-transaction |
| Casino transactions | Gambling separate system |
| Commodity purchases | Raw materials |

### Special Shop Ranges

| Range | Purpose |
|-------|---------|
| 127-134 | Repair shops |

### Database Tables

**Core Tables:**
| Table | Purpose |
|-------|---------|
| `shop` | Base shop configuration |
| `shoptype` | Item types shop trades |
| `shopmaterial` | Materials shop accepts |
| `shopproducing` | Items shop produces |
| `shopowned` | Player ownership data |

**Ownership Tables:**
| Table | Purpose |
|-------|---------|
| `shopownedaccess` | Player permissions |
| `shopownedratios` | Per-vnum pricing |
| `shopownedmatch` | Keyword pricing |
| `shopownedplayer` | Per-player pricing |
| `shopownedrepair` | Repair shop settings |

**Financial Tables:**
| Table | Purpose |
|-------|---------|
| `shopownedbank` | Player bank accounts |
| `shopownedcorpbank` | Corporate accounts |
| `shopownedcentralbank` | Central bank links |
| `shopownedloans` | Active loans |
| `shopownedloanrate` | Loan terms |

**Accounting Tables:**
| Table | Purpose |
|-------|---------|
| `shoplog` | Transaction history |
| `shoplogjournal` | Current year journal |
| `shoplogjournalarchive` | Past years |
| `shoplogcogs` | COGS tracking |
| `shoplogaccountchart` | Account definitions |

**Corporation Tables:**
| Table | Purpose |
|-------|---------|
| `corporation` | Corporation definitions |
| `corpaccess` | Member permissions |
| `corplog` | Corporate transactions |

---

## Implementation

### Shop Types

**NPC Shops** are standard shops with fixed pricing defined in the `shop` database table. The table stores the keeper mob vnum, room vnum, operating hours (two time windows), profit multipliers, and expense ratio paid to the SBA.

**Player-Owned Shops** extend NPC shops via the `shopowned` table. When a player purchases a shop, a row is added linking the shop_nr to custom settings: profit multipliers, inventory limits, owning corporation, dividend percentage, reserve thresholds, and tax office assignment.

The purchase price formula is: 15% markup on (current cash + inventory value) plus 1,000,000 talen flat fee. This is calculated by `TShopOwned::getPurchasePrice()`.

### Pricing Calculations

**Base price** comes from `TObj::adjPrice()`, which scales the item's base value by its current structure points divided by maximum structure points. Damaged items are worth proportionally less.

**Buy price** (shop to player) is: adjPrice * profit_buy * charisma_modifier

**Sell price** (player to shop) is: adjPrice * profit_sell / charisma_modifier

The charisma modifier combines `getChaShopPenalty()` (charisma-based penalty) minus `getSwindleBonus()` (from the Swindle skill), floored at 1.0. Higher charisma and swindle skill both improve prices.

**Custom pricing** in player-owned shops uses three cache structures checked in order by `getProfitBuy()`: vnum-specific ratios, keyword matches against item names, and player-specific rates. Each subsequent match overwrites the previous.

### Transaction Flow

**Player buying from shop:**

`shopping_buy()` validates shop hours, visibility, and weight/volume limits. It calculates price via charisma modifier, verifies player funds, then calls `TObj::buyMe()` which calls `TObj::purchaseMe()`. This invokes `TShopOwned::doBuyTransaction()` which: deducts expenses via `doExpenses()`, transfers money via `giveMoney()`, logs via `shoplog()`, pays dividend via `doDividend()`, balances reserves via `doReserve()`, charges tax via `chargeTax()`, and records journal entry via `journalize()`. Finally both keeper and player are saved.

**Player selling to shop:**

`shopping_sell()` validates item type, damage state, and inventory limits. It calculates price with a structure penalty (damaged items get 60% base deduction scaled by remaining structure). Then `generic_sell()` calls `TObj::sellMe()` which calls `TShopOwned::doSellTransaction()`. This transfers money to player, saves item to shop inventory, balances reserves, and records journal entry. Both parties are saved.

### Reserve System

Shops with corporate ownership automatically balance cash via `doReserve()`. If shop money falls below `reserve_min`, the shop withdraws from the corporation to reach the midpoint between min and max. If money exceeds `reserve_max`, the shop deposits to the corporation to reach the midpoint. This uses `giveMoney()` between keeper and the corporation's banker mob.

Reserve limits must be at least 100,000 talens apart. If both are zero, reserve balancing is disabled. The corporation must have sufficient funds for withdrawals; if the corporation balance is less than the withdrawal amount, the withdrawal is skipped and the shop remains below minimum reserves.

### Banking System

Banks are shops with the `SPEC_BANKER` spec proc. Player accounts are stored in `shopownedbank` with shop_nr, player_id, talens, and earned_interest. Corporate accounts use `shopownedcorpbank`.

Interest is calculated daily by `procBankInterest()`: earned_interest accumulates as talens times profit_sell divided by 365 (daily compound). The integer portion is added to talens and subtracted from earned_interest, preserving fractional accumulation.

Central banks (`SPEC_CENTRAL_BANKER`) set reserve requirements via `shopownedcentralbank`. Regular banks must maintain total deposits times the central bank's profit_buy as reserves. When a player attempts to withdraw, the bank checks if the withdrawal would violate reserve requirements and rejects it if so.

### Tax System

The `chargeTax()` function collects sales tax on transactions. It finds the assigned tax office (defaulting to Grimhaven tax office, shop_nr 14), calculates tax as cost times the tax office's profit_buy rate, and transfers from keeper to taxman via `giveMoney()`.

### Accounting System

Player-owned shops maintain double-entry bookkeeping in `shoplogjournal`. The `journalize()` function records entries with debit and credit amounts against account codes from the chart of accounts. COGS tracking uses `shoplogcogs` to maintain running totals and counts for average cost calculation. `COGS_add()` increments totals, `COGS_get()` returns average cost times quantity.

Year-end closing via `TShopJournal::closeTheBooks()` archives journal entries to `shoplogjournalarchive` and carries forward balances.

### Corporation System

Corporations are defined in the `corporation` table with corp_id, name, and associated bank shop_nr. Member permissions are in `corpaccess`. Shops link to corporations via `corp_id` in `shopowned`.

Corporations receive dividends from their shops: after each sale, `doDividend()` calculates the dividend amount as transaction value times dividend percentage, then adds it to `corp.getMoney()`.

### TMoney Class

Money is represented as `TMoney` objects inheriting from `TMergeable`. Each object stores an amount and currency type. The `create_money()` function creates a TMoney from template object `Obj::GENERIC_TALEN` (vnum 13), setting it as strung for custom descriptions. Descriptions vary by pile size (from "a talen" to "A tremendously HUGE pile of talens"). Physical properties scale: volume is amount * 0.0048, weight is amount / 303.0.

### Currency Merging

When TMoney objects are placed in the same container, `willMerge()` checks if they're the same currency type. If so, `doMerge()` adds the other pile's amount to this one, then removes and deletes the merged pile. This happens automatically in `operator+=` when objects are added to containers.

### Money Pickup

`TMoney::getMe()` handles pickup, calling `moneyMeMoney()` which: removes the object from its container, notifies the player of the amount, logs large pickups, alerts greedy mobs in the room (increasing their urgency and targeting the player), adds money to the player via `addToMoney()`, triggers auto-split if enabled, and returns `DELETE_THIS` to signal the object should be deleted. The money object is consumed and converted to the player's money counter.

### Greedy Mob Interaction

When a player picks up money, `moneyMeMoney()` iterates through all beings in the room checking for greedy mobs. Greedy mobs have their urgency increased by 1 plus the money amount divided by 1000. Higher urgency makes the mob more likely to act on its greed by attacking the player or demanding money. This creates emergent gameplay where picking up large amounts of money in the presence of greedy mobs is dangerous.

### Money Commands

**Drop:** `doDrop()` handles "drop X talens" by creating a TMoney object via `create_money()`, adding it to the room (where it merges with existing piles), and deducting from the player via `addToMoney()`.

**Give:** `doGive()` handles "give X talens target" by validating the recipient (not on solo quest, has hands), calling `giveMoney()` to transfer, and saving both parties immediately.

**Split:** `doSplit()` divides money among grouped players. It counts shares for all grouped members in the same room (via `splitShares()`), deducts the total from the splitter (minus their share), and distributes to each group member proportionally.

### Auto-Split

When `AUTO_SPLIT` is enabled via toggle, money is automatically split with the group when picking up from ground, selling items, selling commodities, or selling components. The check is: player is grouped, has a descriptor, has AUTO_SPLIT set, and has a master or followers.

### Money Statistics

`addToMoney()` tracks all money changes by type in `gold_statistics` arrays, indexed by player level. For positive income, it also tracks in `gold_positive` and applies faction tithe: the faction's tithe percentage is deducted and added to faction coffers.

### Special Shop Types

**Repair Shops** (shop_nr 127-134, checked by `isRepairShop()`) use `shopownedrepair` for quality and speed settings. Speed divides the repair time, quality multiplies the restoration amount, both affect price via profit_buy.

**Loan Sharks** (`SPEC_LOAN_SHARK`) manage player loans via `shopownedloans` (active loans) and `shopownedloanrate` (terms). When a player takes a loan, a row is inserted with amount, interest rate, and due date. The spec proc checks for overdue loans and applies penalties.

**Auctioneers** (`SPEC_AUCTIONEER`) manage auctions via `shopownedauction`. Players can list items for auction with a minimum bid and duration. Other players bid via commands handled by the spec proc. When the auction expires, the highest bidder receives the item and the seller receives payment minus commission.

### TMoney Special Behaviors

Money objects are always carryable regardless of weight/volume limits (`canCarryMe()` returns true). They are scavengeable by mobs (`scavengeMe()` sets them as best target). On load, `onObjLoad()` adjusts the amount by the global economic modifier `gold_modifier[GOLD_INCOME]`.

---

## Troubleshooting

### Money Duplication After Crash

**Symptom:** Player has more money than expected, or shop is missing money after a server crash.

**Likely cause:** A money transfer completed but the save operation was interrupted. The `giveMoney()` call succeeded, modifying both parties' in-memory money values, but one or both parties weren't saved to the database before the crash.

**Diagnostic approach:** Check shoplog for the last transaction. Compare shoptalens (balance after) with actual shop inventory. Look for gaps in transaction timestamps that correspond to crash time.

**Fix:** Always ensure `keeper->saveItems(shop_nr)` and `ch->doQueueSave()` are called immediately after `giveMoney()` in transaction code.

### Shop Pays More Than It Charges

**Symptom:** Players can profit by buying and immediately selling the same item.

**Likely cause:** profit_buy is less than profit_sell, creating arbitrage.

**Diagnostic approach:** Query shopowned table for the shop_nr. Compare profit_buy and profit_sell values. The validation should prevent this, but database edits could bypass it.

**Fix:** Set profit_buy higher than profit_sell. The markup (profit_buy) must exceed the buyback rate (profit_sell).

### Shop Constantly Depositing/Withdrawing

**Symptom:** Excessive shoplog entries showing reserve transfers, corp bank balance fluctuating.

**Likely cause:** reserve_min and reserve_max are too close together, causing the shop to cross thresholds repeatedly.

**Diagnostic approach:** Check shopowned for the reserve_min and reserve_max values. If the spread is less than 100,000, this is the cause.

**Fix:** Set reserve_min and reserve_max at least 100,000 apart. The midpoint targeting means transfers only happen when crossing thresholds, so wider spread means fewer transfers.

### Keeper Says "I Don't Have That Much Money"

**Symptom:** Player cannot sell items to shop despite shop appearing to have inventory.

**Likely cause:** The shopkeeper mob's on-hand cash is less than the sell price. This can happen if the shop sold expensive items without receiving deposits from corporation.

**Diagnostic approach:** Check the keeper's actual money (via admin commands or database query of rent table for keeper's saved inventory). Check shopowned.reserve_min - if the shop is supposed to maintain minimum reserves from corporation, the corp bank may be empty.

**Fix:** Either deposit money directly to the shop, or ensure the corporation has funds and reserve_min triggers a withdrawal. Check that the corporation bank (referenced by corporation.bank column) has the proper shop configured.

### Wrong COGS in Financial Reports

**Symptom:** Cost of Goods Sold doesn't match expected values, profit margins look wrong.

**Likely cause:** Wrong transaction type used. Using TX_BUYING for services records COGS when there shouldn't be any. Using TX_BUYING_SERVICE for physical goods skips COGS tracking.

**Diagnostic approach:** Query shoplogjournal for recent entries. Check if COGS entries (account 600) appear for service transactions or are missing for goods transactions.

**Fix:** Ensure transaction type matches the transaction: TX_BUYING for goods (tracks COGS), TX_BUYING_SERVICE for services (no COGS).

### Strung Items Not Counting Against Inventory Limit

**Symptom:** Shop accepts more of a strung item than max_num setting should allow.

**Likely cause:** Inventory count query is using vnum match instead of short_desc match for strung items. Since strung items share the same base vnum, vnum matching doesn't distinguish them.

**Diagnostic approach:** Check if the item has ITEM_STRUNG flag. If so, the count query should match on short_desc with the strung flag check.

**Fix:** The counting code must check `isObjStat(ITEM_STRUNG)` and use short_desc matching for strung items, vnum matching for regular items.

### Tax Not Collected

**Symptom:** No tax entries in shoplog despite transactions occurring.

**Likely cause:** Tax office shop_nr is invalid, or transaction type is exempt.

**Diagnostic approach:** Check tax_nr in shopowned table. Verify tax office shop exists and has a keeper. Check if buyer is shop owner (exempt). Check if transaction is commodity or casino type (exempt).

**Fix:** Set valid tax_nr in shopowned. Ensure tax office keeper exists and has SPEC_TAXMAN. If intentionally exempt, verify exemption logic is correct.

### Money Pile Does Not Merge

**Symptom:** Multiple separate money piles in same location when they should combine.

**Likely cause:** Currency types differ, or merge code is bypassed.

**Diagnostic approach:** Check currency type of both piles. Verify both are in same container (same room or same inventory). Enable debug logging in TMergeable to trace willMerge and doMerge calls.

**Fix:** Ensure both piles are same currency. If they are same currency and still not merging, check that operator+= in container management is correctly invoking merge logic.

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
