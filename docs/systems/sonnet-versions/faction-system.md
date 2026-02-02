---
title: Faction System
category: important
keywords: [factionTypeT, TFactionInfo, reconcileHelp, reconcileHurt, percModifier, FACTIONS_IN_USE, caravan-system, potency-system, tithe-system, faction-percentage]
related: [affects-system.md, economy-system.md, deity-system.md]
primary_symbols:
  functions: [reconcileHelp, reconcileHurt, percModifier, factionNumber, doNewMember, boot_factions]
  classes: [TFactionInfo, TPerson]
  files: [code/code/misc/faction.h, code/code/misc/faction.cc, code/code/misc/being.h, code/code/misc/person.h]
---

## Overview

Why does helping a Brotherhood member as a Cult follower feel like a betrayal to your deity? The faction system tracks political and religious allegiances, measuring how your actions align with factional interests. When you heal an ally or strike an enemy, the system calculates whether you're advancing your faction's goals or undermining them.

Factions represent political and religious organizations in the game world. Three active factions compete for influence: the Brotherhood of Galek, the Cult of Logrus, and the Order of the Serpents. Players can join one faction or remain unaffiliated. Each faction has its own leadership hierarchy, corporate wealth, trade caravans, and relationships with other factions.

The system serves multiple purposes. It provides organizational structure for player communities, creates economic competition through caravans and corporate wealth, and was originally designed to track loyalty through faction percentages that would affect divine spellcasting power. Currently, percentage tracking is disabled, but membership management, leadership commands, and economic features remain active.

When percentage tracking is enabled, the system monitors every helpful and harmful action you take toward other characters. Healing a faction ally increases your loyalty percentage, while harming them decreases it. Your composite faction percentage determines prayer power for clerics and deikhans, ranging from 0% effectiveness at -100 faction percentage to 100% effectiveness at +100. With tracking disabled, all divine casters receive a fixed 75% prayer power regardless of actions.

A typical faction lifecycle begins when a player learns the faction password and requests membership from a leader. The leader uses the newmember command to add them, setting their faction affiliation and zeroing percentage values. The player participates in faction activities: protecting caravans, contributing tithes, and advancing faction interests. Their actions would affect faction percentage if tracking were enabled. Leadership promotes valuable members to subleader positions, granting authority over recruitment and communication. Factions compete across five metrics: average member level, fishing records, trophy achievements, shop ownership, and corporate wealth.

The system balances individual achievement with collective success. High-level members contribute more to faction scoring than multiple low-level members, preventing padding with alts. Shop ownership rewards business development. Corporate wealth accumulates from tithes and successful caravans but can be spent on caravan defense and faction operations. The Brotherhood, Cult, and Order each control a base city where their caravans originate, creating geographical anchors for factional identity.

## Patterns

### Checking Faction Status

Always use the accessor methods to check faction affiliation. Never access the internal faction member directly.

```cpp
// Correct
factionTypeT playerFaction = ch->getFaction();
if (playerFaction == FACT_BROTHERHOOD) {
  // Handle Brotherhood member
}

// Wrong - direct member access
if (ch->faction == FACT_BROTHERHOOD) { }
```

Use `FACT_NONE` to check for unaffiliated characters, not comparisons to zero or negative values.

### Parsing Faction Names

Always use the `factionNumber()` helper when parsing player input. It handles abbreviations and aliases correctly.

```cpp
factionTypeT fnum = factionNumber(argument);
if (fnum == FACT_UNDEFINED) {
  sendTo("Invalid faction name.\n\r");
  return;
}
```

Valid input includes abbreviations: "bro" for Brotherhood, "cu" for Cult, "or" for Order. Aliases like "galek", "chaos", "logrus", "serpents", "snakes" are also accepted.

### Handling Percentage Updates

When FACTIONS_IN_USE is enabled, never manually adjust faction percentages. Always use `reconcileHelp()` for beneficial actions and `reconcileHurt()` for harmful actions. These functions handle composite percentage calculation, inter-faction relationships, and potency pool updates.

```cpp
// Healing a target
if (successfully_healed) {
  reconcileHelp(victim, healing_amount);
}

// Damaging a target
if (dealt_damage) {
  reconcileHurt(victim, damage_amount);
}
```

The amplitude parameter scales the percentage change. Larger values (healing more damage, dealing more damage) produce larger percentage shifts.

### Checking FACTIONS_IN_USE Status

When implementing features that depend on percentage tracking, always guard code with the preprocessor check.

```cpp
#if FACTIONS_IN_USE
  // Percentage-dependent logic
  double modifier = percModifier();
  prayer_power *= modifier;
#else
  // Fixed behavior when disabled
  prayer_power *= 0.75;
#endif
```

Never assume FACTIONS_IN_USE is enabled. The current codebase has it disabled, and features should degrade gracefully.

### Leadership Authority Validation

Always verify leadership authority before allowing faction management commands. Primary leaders (slot 0) have full authority; subleaders (slots 1-3) have varying authority based on faction policy.

```cpp
bool hasAuthority = false;
for (int i = 0; i < FACT_LEADER_SLOTS; i++) {
  if (faction_data[fnum].leader[i] &&
      !strcmp(faction_data[fnum].leader[i], ch->getName().c_str())) {
    hasAuthority = true;
    if (i == 0) isPrimaryLeader = true;
    break;
  }
}

if (!hasAuthority) {
  sendTo("You don't have leadership authority.\n\r");
  return;
}
```

The makeleader command requires primary leader status. Membership management (newmember, rmember) can be performed by any leader or subleader.

### Faction Array Access Safety

The faction relationship array uses two indices: target faction and relationship type (help/hurt). Always validate both indices.

```cpp
if (victFact < MIN_FACTION || victFact >= MAX_FACTIONS) {
  // Invalid faction
  return;
}

double helpValue = faction_data[victFact].faction_array[myFact][OFF_HELP];
double hurtValue = faction_data[victFact].faction_array[myFact][OFF_HURT];
```

Never access `faction_array` with `FACT_UNDEFINED` or values beyond `MAX_FACTIONS`. The array is sized `[MAX_FACTIONS][2]`.

### Database Synchronization

Always update the database immediately when faction membership changes. Don't rely on periodic saves.

```cpp
// After changing faction
ch->setFaction(newFaction);
ch->saveChar(Room::AUTO_RENT);

// Update factionmembers table
TDatabase db(DB_SNEEZY);
db.query("delete from factionmembers where name='%s'",
         ch->getName().c_str());
if (newFaction != FACT_NONE) {
  db.query("insert into factionmembers values ('%s', '%s', %d)",
           ch->getName().c_str(),
           FactionInfo[newFaction]->faction_name,
           ch->GetMaxLevel());
}
```

Failure to synchronize causes rollcall displays to show stale membership and score calculations to be incorrect.

### Caravan Defense Spending

Never allow negative caravan defense values. Spending is capped by faction corporate wealth.

```cpp
int requestedDefense = convertTo<int>(argument);
if (requestedDefense < 0) {
  sendTo("Defense spending must be non-negative.\n\r");
  return;
}

int availableWealth = getCorpWealth(faction_data[fnum].corp_id);
if (requestedDefense > availableWealth) {
  sendTo("Insufficient faction wealth for that defense level.\n\r");
  return;
}
```

Defense spending affects caravan success rates. Higher defense reduces chance of caravan raids but costs talens.

### Avoiding ABS_MAX_FACTION Modification

Never change `ABS_MAX_FACTION` without a full player wipe. This constant determines the size of `f_percx[]` in the binary charFile format.

```cpp
// FROZEN - DO NOT CHANGE
const factionTypeT ABS_MAX_FACTION = factionTypeT(6);
```

Changing this value causes read/write offsets to misalign, corrupting all player files. If more factions are needed, the implementation must pad unused slots or migrate the entire playerbase through a conversion tool.

### Tithe Processing

Always deposit tithe funds to the faction corporation before calculating percentage increases.

```cpp
// Correct order
giveMoney(ch, -titheAmount);
depositToFactionCorp(titheAmount);
ch->doQueueSave(SAVE_INVENTORY);

#if FACTIONS_IN_USE
  double percentIncrease = titheAmount * TITHE_FACTOR;
  reconcileHelp(ch, percentIncrease);
#endif
```

Wrong order risks duplication if the server crashes between percentage increase and money deduction.

## Reference

### Primary Symbol Reference

| Symbol | Type | Purpose |
|--------|------|---------|
| `reconcileHelp()` | function | Updates faction percentage for helpful actions |
| `reconcileHurt()` | function | Updates faction percentage for harmful actions |
| `percModifier()` | function | Returns prayer power modifier based on faction percentage |
| `factionNumber()` | function | Parses faction name/alias to enum value |
| `doNewMember()` | function | Adds player to faction with zeroed percentages |
| `boot_factions()` | function | Loads faction data from file at server startup |
| `TFactionInfo` | class | Stores faction configuration, leadership, wealth, caravans |
| `TPerson` | class | Defines reconcileHelp/reconcileHurt virtual functions |

### Faction Enumeration Values

| Constant | Value | Description |
|----------|-------|-------------|
| `FACT_UNDEFINED` | -1 | Invalid/error state returned by parsing |
| `FACT_NONE` | 0 | Unaffiliated character |
| `FACT_BROTHERHOOD` | 1 | Brotherhood of Galek |
| `FACT_CULT` | 2 | Cult of Logrus |
| `FACT_SNAKE` | 3 | Order of the Serpents |
| `MAX_FACTIONS` | 4 | Loop limit for active factions |
| `ABS_MAX_FACTION` | 6 | charFile array size (frozen) |
| `MIN_FACTION` | 0 | Loop start for faction iteration |

### Faction Name Aliases

| Faction | Primary Name | Accepted Aliases |
|---------|--------------|------------------|
| `FACT_BROTHERHOOD` | brotherhood | galek |
| `FACT_CULT` | cult | chaos, logrus |
| `FACT_SNAKE` | order | serpents, snakes |

### Leadership Slot Authority

| Slot | Role | Authority |
|------|------|-----------|
| 0 | Primary Leader | Full authority: newmember, rmember, makeleader, send, adjust |
| 1 | Subleader | Varies by faction: typically newmember, rmember, send |
| 2 | Subleader | Varies by faction: typically newmember, rmember, send |
| 3 | Subleader | Varies by faction: typically newmember, rmember, send |

### Faction Array Indices

| Constant | Value | Purpose |
|----------|-------|---------|
| `OFF_HELP` | 0 | Index for help relationship value |
| `OFF_HURT` | 1 | Index for hurt relationship value |

### Prayer Power Modifier Ranges

| Faction Percentage | percModifier() | Prayer Power |
|--------------------|----------------|--------------|
| -100 | 0.0 | 0% effectiveness |
| -50 | 0.25 | 25% effectiveness |
| 0 | 0.50 | 50% effectiveness |
| +50 | 0.75 | 75% effectiveness |
| +100 | 1.0 | 100% effectiveness |
| N/A (disabled) | 0.75 | Fixed 75% effectiveness |

### Faction Scoring Metrics

| Metric | Calculation | Purpose |
|--------|-------------|---------|
| Average Level | Biased sum: higher levels weighted more | Rewards high-level membership |
| Pounds of Fish | Record count + total pounds | Tracks fishing achievements |
| Average Trophy | Biased sum: combines trophy % and levels | Rewards both quality and achievement |
| Shops Owned | Count of shops with faction owner | Encourages business development |
| Faction Wealth | Talens in faction corporation | Direct economic power |

### Caravan Fields

| Field | Type | Purpose |
|-------|------|---------|
| `caravan_interval` | int | Ticks between caravan spawns |
| `caravan_counter` | int | Countdown to next spawn |
| `caravan_flags` | unsigned int | Configuration (route, protection) |
| `caravan_value` | int | Talens per successful delivery |
| `caravan_defense` | int | Money spent on guards |
| `caravan_attempts` | int | Total caravans launched |
| `caravan_successes` | int | Caravans that reached destination |

### Key Base Cities and Vnums

| Faction | Base City | Vnum | Purpose |
|---------|-----------|------|---------|
| `FACT_BROTHERHOOD` | Brightmoon | 1395 | Caravan origin, faction headquarters |
| `FACT_CULT` | Logrus | 3768 | Caravan origin, faction headquarters |
| `FACT_SNAKE` | Amber | 8713 | Caravan origin, faction headquarters |
| `FACT_NONE` | Grimhaven | 432 | Unaffiliated base |

### TBeing Accessor Methods

| Method | Purpose |
|--------|---------|
| `getFaction()` | Returns current faction enum |
| `setFaction(factionTypeT)` | Changes faction affiliation |
| `getPerc()` | Returns composite faction percentage |
| `setPerc(double)` | Sets composite faction percentage |
| `getPercX(factionTypeT)` | Returns individual faction percentage |
| `setPercX(double, factionTypeT)` | Sets individual faction percentage |

### Feature Status by FACTIONS_IN_USE

| Feature | Enabled (1) | Disabled (0) |
|---------|-------------|--------------|
| Faction membership | Active | Active |
| Leadership commands | Active | Active |
| Caravan system | Active | Active |
| Corporate wealth | Active | Active |
| Tithe deposits | Affects wealth + percentage | Affects wealth only |
| Prayer power | Dynamic 0.0-1.0x | Fixed 0.75x |
| reconcileHelp/reconcileHurt | Tracks percentages | Empty functions |
| Help/hurt adjustment | Allowed | Blocked |
| Potency effects | Active | No gameplay impact |

## Implementation

### Faction Percentage Calculation

The faction system maintains two types of percentages for each character: individual faction percentages (one per faction) and a composite percentage representing overall factional purity. When FACTIONS_IN_USE is enabled, `reconcileHelp()` and `reconcileHurt()` update these values based on inter-faction relationships.

Each faction defines help and hurt values toward every other faction in the `faction_array` field. This is a two-dimensional array indexed by target faction and relationship type. When a character performs an action affecting another character, the system looks up the relationship between their factions. For example, if a Brotherhood member heals a Cult member, it retrieves `faction_data[FACT_CULT].faction_array[FACT_BROTHERHOOD][OFF_HELP]`. This value determines how much the action affects the actor's faction percentage.

The `reconcileHelp()` function receives a victim and an amplitude parameter. It retrieves both characters' factions, looks up the help value from the faction array, and adds `help_value * amplitude` to the actor's individual percentage for the victim's faction. After updating the individual percentage, it recalculates the composite percentage by summing all four individual faction percentages (Brotherhood, Cult, Snake, and None). This composite value is what `percModifier()` uses to determine prayer power.

The function also updates the faction's potency pool by adding `help_value * amplitude / 5.0` to `faction_data[actor_faction].faction_power`. Potency represents the faction's accumulated divine power, though its effects are minimal when percentage tracking is disabled.

The `reconcileHurt()` function works identically but uses the hurt values and subtracts from percentages rather than adding. Actions that trigger reconcileHurt include dealing damage, killing, stealing, and other harmful behaviors.

When FACTIONS_IN_USE is disabled, both functions are empty shells wrapped in preprocessor conditionals. All calls compile but execute no code, making the performance impact zero.

### Prayer Power Modification

The `percModifier()` function translates faction percentage into a prayer power multiplier. With percentage tracking enabled, it implements the formula `(getPerc() + 100.0) / 200.0`. This maps the -100 to +100 percentage range onto 0.0 to 1.0. A character with -100 faction percentage has 0% prayer effectiveness, making divine spells completely ineffective. A character with 0 faction percentage has 50% effectiveness. A character with +100 faction percentage has full 100% effectiveness.

This modifier is applied when clerics and deikhans cast divine spells. The spell power, healing, or other effects are multiplied by the modifier before taking effect. A cleric with low faction percentage finds their prayers weakened, while one with high percentage receives full divine support.

When FACTIONS_IN_USE is disabled, `percModifier()` returns a constant 0.75, giving all divine casters 75% effectiveness regardless of their actions. This provides functional divine magic without percentage tracking overhead.

### Inter-Faction Relationship Storage

The `TFactionInfo::faction_array` field stores relationship values as a `double[MAX_FACTIONS][2]` array. The first dimension indexes the target faction (whose member is being helped or hurt). The second dimension uses `OFF_HELP` (0) and `OFF_HURT` (1) to store separate values for helpful and harmful actions.

Relationship values typically range from -4.0 to +4.0. Positive values mean the action aligns with faction interests; negative values mean it conflicts. For example, the Brotherhood might have +4.0 help value toward other Brotherhood members (strongly encouraged), +1.0 help value toward unaffiliated characters (mildly encouraged), -2.0 help value toward Cult members (discouraged), and -4.0 help value toward Snake members (strongly discouraged).

These values are configured by faction leaders using the adjust command when FACTIONS_IN_USE is enabled. The command is blocked when disabled because modifying relationships has no effect without percentage tracking. The values persist in the faction data file and reload at boot.

### Faction Name Parsing

The `factionNumber()` function accepts a string and returns the corresponding factionTypeT enum. It uses `is_abbrev()` to handle abbreviations, checking if the input matches "brotherhood" or "galek" for FACT_BROTHERHOOD, "cult", "chaos", or "logrus" for FACT_CULT, and "order", "serpents", or "snakes" for FACT_SNAKE. If no match is found, it returns FACT_UNDEFINED.

This function appears in every faction-related command that accepts user input. Always check for FACT_UNDEFINED before using the result, as invalid input should produce clear error messages rather than treating FACT_UNDEFINED as a valid faction.

### Membership Management Flow

When a faction leader executes the newmember command, `doNewMember()` parses the target name and validates that the target exists, is not already in a faction, and that the leader has authority. It then calls `setFaction()` to assign the faction enum and, if FACTIONS_IN_USE is enabled, zeros all faction percentages using `setPerc(0.0)` and a loop setting each `setPercX(0.0, i)` for all faction indices.

After modifying the character's faction, the function saves the character with `saveChar(Room::AUTO_RENT)` and updates the factionmembers database table. The database insertion includes name, faction name string, and current level. This database table supports faction rollcall displays and scoring calculations, which query the table rather than iterating all online characters.

The rmember command reverses this process. It sets faction to FACT_NONE, clears percentages if enabled, saves the character, and deletes the row from factionmembers. The disband command allows characters to leave voluntarily, performing the same operations with different permission checks.

### Leadership Hierarchy

The `TFactionInfo::leader` array stores up to four leader names as null-terminated C strings. Slot 0 is the primary leader with full authority. Slots 1-3 are subleaders with varying authority levels based on faction policy (not enforced by code, but by social convention).

Leadership validation iterates the array comparing stored names against the character's name using `strcmp()`. If a match is found, the character has leadership authority. The slot index determines whether they can execute restricted commands like makeleader (primary leader only).

The makeleader command accepts a slot number and a player name. It validates that the caller is the primary leader, that the slot number is valid (0-3), and that the target exists. It then updates `faction_data[fnum].leader[slot]` with the new name and saves faction data. Passing an empty name clears the slot, demoting the previous leader.

### Corporate Wealth Integration

Each TFactionInfo has a `corp_id` field linking to a corporation in the economy system. This corporation has a bank account holding faction talens. Tithes deposit money to this account. Caravan rewards deposit to this account. Faction spending (caravan defense, leader withdrawals) deducts from this account.

When a player donates talens through the pray command, the code calls `giveMoney()` to deduct talens from the player, then deposits to the faction corporation, then calls `doQueueSave()` to persist inventory changes. If FACTIONS_IN_USE is enabled, it calculates `talens * TITHE_FACTOR` (0.0003) as a percentage increase and calls `reconcileHelp()` with the player as both actor and victim, simulating helping oneself to increase faction purity.

The corp_id is initialized at faction creation and remains constant. Faction scoring queries the corporation bank balance directly for the wealth metric.

### Caravan Lifecycle

Each faction has caravan-related fields in TFactionInfo. The `caravan_interval` determines how many game ticks pass between caravan spawns. The `caravan_counter` decrements each tick. When it reaches zero, the system spawns a caravan mob with goods, resets the counter to the interval, and increments `caravan_attempts`.

Caravans travel from the faction's base city to a destination city. The `caravan_flags` field encodes the route configuration. The `caravan_defense` value determines how many guards accompany the caravan, affecting the probability of successful completion against player raids.

If the caravan reaches its destination without being destroyed, `caravan_successes` increments and `caravan_value` talens deposit to the faction corporation. If the caravan is destroyed by players, the attackers can loot the goods, and the faction loses the investment.

The success rate calculation compares defense spending against raid difficulty. Higher defense spending increases success rate but reduces net profit per caravan. Faction leaders must balance defense investment against caravan frequency and value.

### Faction Data Persistence

At server boot, `boot_factions()` opens `lib/faction/faction_info` and reads serialized TFactionInfo structures into the `faction_data[]` array. This array is indexed by factionTypeT enum, so `faction_data[FACT_BROTHERHOOD]` contains the Brotherhood's configuration.

Whenever faction data changes (leadership, relationships, caravan settings, potency), the system writes the entire array back to the file. A backup copy at `lib/faction/faction_info.bak` protects against corruption during writes.

The serialization format is binary, writing the TFactionInfo structures directly. This is fragile to struct layout changes. Adding or removing fields requires a migration tool to read the old format and write the new format.

Character faction data persists in the charFile binary format. The `f_type` field stores the faction enum as a short. The `f_percent` field stores the composite percentage as a double. The `f_percx` array stores individual faction percentages as a `double[ABS_MAX_FACTION]` array. This array size is frozen at 6 to maintain binary compatibility.

### Conditional Compilation Architecture

Over 40 `#if FACTIONS_IN_USE` blocks appear throughout the codebase. The most critical are in faction.cc, where `reconcileHelp()` and `reconcileHurt()` have their entire bodies wrapped. When disabled, these functions compile to empty stubs that immediately return.

The percModifier() function has a simpler conditional: the entire function body is an `#if FACTIONS_IN_USE` block returning the percentage-based formula, with an `#else` block returning 0.75.

Integration points in combat, healing, and other systems have conditional blocks around their reconcileHelp/reconcileHurt calls. When disabled, these calls are omitted entirely, reducing runtime overhead to zero.

The adjust command has a guard at the beginning that checks `#if !FACTIONS_IN_USE` and returns an error message if the compile-time flag is disabled. This prevents leaders from modifying relationship values that have no effect.

Tithe processing in the pray command has percentage increase code wrapped in a conditional. The money transfer happens unconditionally, but the percentage reward only applies when enabled.

### Faction Scoring Calculations

Five metrics contribute to faction score, queried from different data sources.

Average level queries the factionmembers table, sums the level column for each faction, and applies a biased weighting formula. Higher-level members contribute disproportionately more than their numerical level suggests. This prevents factions from gaining score by recruiting many low-level alts.

Pounds of fish queries the fishing records table, counting how many records are held by faction members and summing total pounds caught. Each record held contributes one point; total pounds contribute another value. This rewards both record diversity and total fishing effort.

Average trophy queries character trophy percentages (percent of zones completed) and applies a similar biased weighting. A level 50 character with 50% trophy contributes more than two level 25 characters with 50% trophy each, encouraging both individual achievement and high-level membership.

Shops owned queries the shop ownership table, counting distinct shops where at least one faction member has owner privileges. Multiple owners from the same faction don't increase the count; the faction gets one point per shop regardless of how many members own it. This encourages shop ownership diversity rather than concentrating ownership.

Faction wealth queries the corporation bank balance directly. Each talen in the faction corporation contributes to the score. This is the most transparent metric and the easiest to affect through tithes and caravan success.

### Deity Integration

Each faction associates with different deities for flavor. The `yourDeity()` function returns a deity name string based on the character's faction. Brotherhood members might call upon Galek, Cult members upon chaos deities, and Snake members upon serpent-aligned deities. Unaffiliated characters can choose from neutral deities.

This integration is entirely flavor; the deity name appears in prayer messages and divine spell echoes but doesn't affect mechanics. The percModifier() function provides the mechanical effect, not the specific deity name.

Holy symbols can be attuned to faction alignment. A symbol attuned to the Brotherhood works better for Brotherhood clerics than for Cult clerics. This creates another layer of factional identity, encouraging characters to use equipment aligned with their faction.

### Database Schema

The factionmembers table uses three columns: name (varchar 80, primary key), faction (varchar 8, stores faction name string), and level (int, stores character level). The table uses InnoDB engine with latin1 charset.

This table is updated immediately when membership changes. The code executes a DELETE query to remove any existing row for the character name, then an INSERT query to add the new membership record if joining a faction. If leaving to FACT_NONE, only the DELETE executes.

Faction rollcall displays query this table with `SELECT name, level FROM factionmembers WHERE faction='Brotherhood' ORDER BY level DESC`. This returns all members sorted by level, allowing leaders to see faction composition without waiting for members to log in.

Scoring calculations join this table with other tables (fishing records, trophy data, shop ownership) to compute faction metrics without iterating online characters.

### File Organization

The primary implementation lives in faction.cc and faction.h. These files define TFactionInfo, declare the faction_data array, implement all faction commands, and provide reconcileHelp/reconcileHurt.

The being.h header declares virtual reconcileHelp/reconcileHurt stubs in TBeing, allowing the base class to have empty implementations. The person.h header declares TPerson overrides for these virtuals, and faction.cc provides the actual implementation.

Character accessors (getFaction, setFaction, getPerc, setPerc, getPercX, setPercX) are declared in being.h and implemented inline or in being.cc.

Integration points exist in utility.cc (tithes), damage.cc (hurt on damage), offense.cc (help on healing), combat.cc (hurt in combat), and other.cc (group sharing adjustments). Each integration point has a conditional block calling reconcileHelp or reconcileHurt when FACTIONS_IN_USE is enabled.

Help files live in lib/help/ with topics: factions (basic overview), faction overview (comprehensive guide), faction percent (percentage mechanics), faction leaders (leadership system), faction rules (OOC policies), and faction score (metrics explanation).

## Troubleshooting

### Symptom: New Members Not Appearing in Rollcall

**Likely cause:** Database insert failed or saveChar not called after faction assignment.

**Diagnostic approach:** Check the factionmembers table directly with `SELECT * FROM factionmembers WHERE name='PlayerName'`. If the row is missing, the database operation failed. Check sneezy.log for SQL errors. Verify that doNewMember calls saveChar and executes the INSERT query without exceptions.

**Fix:** Re-run the newmember command. If the problem persists, check database permissions and table structure. The INSERT query requires latin1 charset compatibility; character name encoding issues can cause silent failures.

### Symptom: Prayer Power Not Changing Despite Actions

**Likely cause:** FACTIONS_IN_USE is disabled, fixing prayer power at 0.75.

**Diagnostic approach:** Check faction.h for `#define FACTIONS_IN_USE 0`. If disabled, percModifier always returns 0.75 regardless of faction percentage. Verify by checking the player's faction percentage with a wstat command; if it never changes, tracking is disabled.

**Fix:** If percentage tracking is desired, change FACTIONS_IN_USE to 1 and recompile. This enables reconcileHelp/reconcileHurt tracking. Existing characters will start with their current percentage values (likely zero for newer characters) and begin accumulating changes from actions.

### Symptom: Faction Percentage Stuck at Zero

**Likely cause:** No actions triggering reconcileHelp/reconcileHurt, or actions involve FACT_NONE characters with zero relationship values.

**Diagnostic approach:** Verify FACTIONS_IN_USE is enabled. Check that the character is performing actions affecting factioned characters. Healing or harming unaffiliated (FACT_NONE) characters may have minimal relationship values. Check faction_data arrays to see if help/hurt values toward FACT_NONE are configured.

**Fix:** Perform actions affecting factioned characters. Healing a Brotherhood member while in the Brotherhood should increase percentage. If relationship values are misconfigured, use the adjust command to set appropriate help/hurt values for each faction pair.

### Symptom: Adjust Command Blocked

**Likely cause:** FACTIONS_IN_USE is disabled, blocking relationship modifications.

**Diagnostic approach:** Attempt the adjust command. If the response is "not permitted to alter help/harm values because faction percent is not in use", FACTIONS_IN_USE is disabled. This is intentional; modifying relationships has no effect when percentage tracking is off.

**Fix:** If relationship modification is needed, enable FACTIONS_IN_USE and recompile. Otherwise, accept that relationship values are static when percentage tracking is disabled.

### Symptom: Caravan Not Spawning

**Likely cause:** caravan_counter not decrementing, caravan_interval set to zero, or caravan spawn code not executing.

**Diagnostic approach:** Check faction data with a stat command showing caravan_counter and caravan_interval values. If interval is zero, caravans never spawn. If counter is not decrementing, the periodic update isn't running.

**Fix:** Set caravan_interval to a positive value (number of ticks between spawns). Verify that the game's periodic update function includes caravan counter decrementing logic. If the counter decrements but no caravan spawns when it reaches zero, check the caravan spawn function for errors or missing mob vnums.

### Symptom: Faction Wealth Not Increasing from Tithes

**Likely cause:** Corporation ID invalid or deposit function failing.

**Diagnostic approach:** Check faction_data[faction].corp_id value. Query the corporation table to verify the corporation exists. When a player donates, check if the corporation bank balance increases. If not, the deposit function is failing.

**Fix:** Verify corp_id is a valid corporation ID. If the corporation doesn't exist, create it or update corp_id to an existing corporation. Ensure the deposit function receives the correct amount and doesn't fail due to transaction errors.

### Symptom: Leadership Commands Failing for Subleaders

**Likely cause:** Name mismatch between leader array entry and character name.

**Diagnostic approach:** Check faction_data[faction].leader[] array values. Compare stored names against the character's getName() result. Trailing spaces, capitalization differences, or truncated names cause strcmp to fail.

**Fix:** Use makeleader to re-assign the subleader, ensuring exact name match. Leadership validation uses strcmp, which is case-sensitive and requires exact matches including whitespace.

### Symptom: Crash When Accessing Faction Data

**Likely cause:** Faction enum out of bounds, accessing faction_data with FACT_UNDEFINED or value >= MAX_FACTIONS.

**Diagnostic approach:** Add bounds checking around faction_data accesses. Log the faction value before indexing. If it's -1 (FACT_UNDEFINED) or beyond MAX_FACTIONS, the bounds check failed.

**Fix:** Always validate faction values before indexing faction_data. Check that factionNumber() didn't return FACT_UNDEFINED. Ensure loops use `i < MAX_FACTIONS` not `i <= MAX_FACTIONS`.

### Symptom: Faction Percentage Extreme Values

**Likely cause:** Relationship values too high or repeated actions amplifying percentage beyond intended range.

**Diagnostic approach:** Check faction_array values for the involved factions. If help/hurt values exceed 4.0 or are negative when they should be positive, misconfiguration causes extreme percentage changes. Check if repeated actions (e.g., healing spam) accumulate percentage without decay.

**Fix:** Use adjust command to set relationship values within -4.0 to +4.0 range. If percentage has already reached extreme values, manually reset with setPerc command (immortal privilege). Consider implementing percentage caps in reconcileHelp/reconcileHurt if unbounded growth is problematic.

### Symptom: Faction Data File Corruption

**Likely cause:** Server crash during faction data write, struct layout change, or manual file editing.

**Diagnostic approach:** Attempt to boot the server and check for faction load errors. If boot_factions fails, the file is corrupted. Compare file size against expected size (sizeof(TFactionInfo) * MAX_FACTIONS). If sizes don't match, struct layout changed or file was truncated.

**Fix:** Restore from faction_info.bak backup. If backup is also corrupted, recreate faction data from scratch, reinitializing leadership, relationship values, and caravan settings. This loses accumulated potency and caravan statistics but preserves faction structure.
