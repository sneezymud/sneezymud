---
title: Object Maintenance
description: Decay timers, structure points, damage mechanics, and repair systems for managing item durability and lifespan.
category: critical
keywords: [decay timers, structure points, item durability, repair mechanics, damage system]
primary_symbols:
  functions: [objectTickUpdate, decayMe, objectDecay, damageItem, makeScraps, repairPrice, repair_time, maxFix, willDent, willTear, willPuncture, genericDamCheck]
  classes: [TObj, objFlagData, TBaseCorpse, TPCorpse]
  enums: [OBJ_NOTIMER, ITEM_BURNING, ITEM_RUSTY, ITEM_NOPURGE, ITEM_NODROP, ITEM_STRUNG, ITEM_PROTOTYPE, DELETE_THIS, ROOM_ARENA, SKILL_BLACKSMITHING, SKILL_REPAIR_SHAMAN, SKILL_REPAIR_MONK, SKILL_REPAIR_MAGE, SKILL_REPAIR_CLERIC, SKILL_REPAIR_THIEF, SKILL_REPAIR_DEIKHAN, GOLD_REPAIR]
---

# Object Maintenance

## Overview

Objects have two independent health metrics controlling their lifespan. **Decay timers** count down to automatic deletion for temporary items like corpses and summoned objects. **Structure points** represent durability that degrades from combat and can be restored through repair. An item can have full structure but be about to decay, or be nearly destroyed but permanent.

Decay deletes objects completely. Structure damage creates salvageable scraps that preserve contents. Monogrammed items receive special protection from scrapping.

## Patterns

### Decay Handling

- Always check `decay_time == -1` before assuming an object will persist; OBJ_NOTIMER means permanent
- Never rely on decay timer for equipped or carried items; decay only ticks when objects are in rooms
- Always relocate container contents before deletion; items inside decaying containers need new homes
- Never monogram or customize items with positive decay timers; they will not last long enough

### Structure Point Management

- Always check `getStructPoints()` before combat operations that might scrap equipment
- Never assume `damageItem()` returns cleanly; it returns DELETE_THIS when item is scrapped
- Always call `makeScraps()` through `damageItem()`; direct scrapping bypasses safety checks
- Never damage items in arena rooms; arena protection prevents structure point loss

### Repair Operations

- Always verify item is rentable before attempting repair; non-rentable items cannot be repaired
- Never repair burning items; ITEM_BURNING must be removed first
- Always empty containers before submitting to repair shops
- Never exceed maxFix() quality; shops cannot restore items above 95% of max structure

### Material Consumption

- Always ensure sufficient commodity materials before player repair; repair fails and wastes the attempt otherwise
- Never forget that monogrammed items consume only 25% normal materials
- Always match material type to item material; incompatible materials prevent repair

## Reference

### Decay Timer Values

| Value | Meaning |
|-------|---------|
| -1 | Never decays (OBJ_NOTIMER) |
| 0 | Decays this tick |
| >0 | Ticks remaining |

### Decay Messages by Location

| Location | Message |
|----------|---------|
| Equipped | "$p decay$Q into nothing." |
| Inventory | "$p disintegrate$Q in your hands." |
| Room | "$n fade$R into insignificance." |

### Type-Specific Decay Behavior

| Type | Behavior |
|------|----------|
| TBaseCorpse | Message, logs player corpse info |
| TPCorpse | Extended logging, content relocation |
| TLight | "flickers then fades into insignificance" message |
| TFood | Sets FOOD_SPOILED flag and extends decay_time (no message) |
| TPortal | Simple deletion |
| TObj (default) | Relocate contents, delete |

### Condition Display

| Ratio | Condition | Color |
|-------|-----------|-------|
| >100% | better than new | White |
| 100% | brand new | Cyan |
| >90% | like new | cyan |
| >80% | excellent | Blue |
| >70% | very good | blue |
| >60% | good | Purple |
| >50% | fine | purple |
| >40% | fair | Green |
| >30% | poor | green |
| >20% | very poor | yellow |
| >10% | bad | orange |
| >0.1% | very bad | Red |
| <=0.1% | destroyed | red |

### Damage Type Susceptibility

| Function | Damage Type | Vulnerable Materials |
|----------|-------------|---------------------|
| willDent() | Blunt impact | Metals, minerals |
| willTear() | Slashing | Cloth, leather |
| willPuncture() | Piercing | Soft materials |

### Structure Point Accessors

| Method | Purpose |
|--------|---------|
| getStructPoints() | Returns current structural health |
| setStructPoints() | Assigns current structure |
| addToStructPoints() | Modifies current structure by positive or negative amount |
| getMaxStructPoints() | Returns maximum structural health |
| setMaxStructPoints() | Assigns maximum structure |

### Unrepairable Conditions

| Condition | Reason |
|-----------|--------|
| Not rentable | Unrepairable item class |
| At max structure | No damage present |
| Depreciation maxed | Permanent damage threshold |
| Zero repair time | Already repaired |
| objVnum() == -1 | Temporary/virtual item |
| ITEM_NODROP | Cursed item |
| ITEM_BURNING | Safety hazard |
| Over max_exist | Reclamation contract |
| Non-empty container | Must empty first |

### Player Repair Skills

| Skill | Class | Materials | Tools |
|-------|-------|-----------|-------|
| SKILL_BLACKSMITHING | Warrior | Metal | Hammer, tongs, forge, anvil |
| SKILL_REPAIR_SHAMAN | Shaman | Organic/bone | Scalpel, forceps, operating table |
| SKILL_REPAIR_MONK | Monk | Wood | Ladle, soil, water source |
| SKILL_REPAIR_MAGE | Mage | Various | Skill-specific |
| SKILL_REPAIR_CLERIC | Cleric | Various | Skill-specific |
| SKILL_REPAIR_THIEF | Thief | Various | Skill-specific |
| SKILL_REPAIR_DEIKHAN | Deikhan | Various | Skill-specific |

### Repair Shop Commands

| Command | Action |
|---------|--------|
| VALUE <item> | Get repair cost and estimated time |
| VALUE all.damaged | Value all damaged inventory items |
| GIVE <item> <repairman> | Submit item for repair |
| GIVE all.damaged <repairman> | Submit all damaged items |
| GIVE <ticket> <repairman> | Retrieve repaired item |

### Key Constants

| Constant | Value | Purpose |
|----------|-------|---------|
| OBJ_NOTIMER | -1 | Decay disabled |
| repair_mats_ratio | 0.10 | Material consumption rate |
| ITEM_RUSTY | bit 11 | Item has rust damage |
| ITEM_NOPURGE | bit 23 | Protected from purge |
| Shop speed range | 0-5.0 | Repair time modifier (1.0 = no change) |

## Implementation

### Decay Processing

Decay processing occurs in `TObj::objectTickUpdate()` in periodic.cc. The system first checks if `decay_time > -1` to determine if the object has decay enabled. If so, `decayMe()` decrements the timer only when the object is physically in a room (`in_room != Room::NOWHERE`), not carried or equipped.

When `decay_time` reaches zero, `objectDecay()` is called. This is a virtual method allowing type-specific behavior. The base implementation relocates container contents to the parent location before deletion. Tables and furniture preserve items placed on them through special handling. Corpses log additional information and handle ITEM_NEWBIE items specially, destroying them unless in donation or starting rooms.

### Damage Processing

Combat damage flows through `TBeing::dentItem()`, `tearItem()`, and `pierceItem()` in materials.cc. Each checks arena protection first (ROOM_ARENA flag causes immediate return), then retrieves the attacking weapon's hardness. The `genericDamCheck()` function applies a 30% base chance to check damage at all, then compares attacker sharpness (capped at 30) plus defender susceptibility against a random 20-120 range.

When damage occurs, `damageItem()` subtracts structure points and calls `makeScraps()` if points reach zero. Scrapping removes ITEM_BURNING, drops liquid from containers, relocates contents, and creates a TTrash replacement. Monogrammed items trigger `scrapMonogrammed()` for special recovery handling.

### Repair Price Calculation

Shop repair pricing in repair.cc subtracts material cost from base item cost, calculates the percentage being repaired, applies the GOLD_REPAIR economy modifier, and scales by repair percentage. Destroyed items (struct <= 0) pay 5x cost. Raw material costs are added on top.

### Repair Time Formula

Base repair time is `4 + (damage / 0.3)` converted to real seconds via pulse constants, assuming a 30% success rate matching player repair skill difficulty. NPC shops apply a 1.25x penalty compared to self-repair. Shop speed settings (0-5.0 range) can further modify this.

### Maximum Repair Quality

Items cannot be restored to full maximum. The `maxFix()` function caps repair at 95% of max structure points minus depreciation. Owned shops can further reduce this via their quality setting (0-1.0 range).

### Player Repair Mechanics

Repair tasks implement multi-stage state machines: Start phase validates item condition, checks tool presence, and verifies material availability; Repair phase executes skill checks each pulse; Finish phase completes repair and displays success messages.

Player repair in task_blacksmithing.cc rolls against skill value modified by DEX reaction bonus. Success adds 1 structure point. Failure on items with remaining structure causes 1 point of additional damage. Rolling exactly 101 is complete failure regardless of skill.

Material consumption via `ConsumeRepairMats()` is based on weight divided by max structure, scaled by the 10% ratio constant. Monogrammed items consume only 25% of normal materials.

### Depreciation

The depreciation system tracks permanent wear that cannot be repaired. Infrastructure exists via `getDepreciation()`, `setDepreciation()`, and `addToDepreciation()` methods, but the getter currently returns 0 (system disabled). When active, each repair cycle adds depreciation, progressively lowering maximum repair quality until items become unrepairable.

### Object Locking

Objects can be temporarily protected from damage via `setLocked()`. The `getLocked()` method checks this protection status before damage is applied.

### Repair Ticket System

Item submission creates repair tickets as TNote objects containing item references and repair timers. Shop inventory stores actual items with repair_time counters that decrement during ticks. When timers reach zero, items move to ready state. Ticket retrieval deletes tickets, removes items from shop inventory, restores structure points via setStructPoints to maxFix values, removes ITEM_RUSTY flags, and gives items back to players.

## Troubleshooting

### Item Decayed While Equipped

**Symptom:** Player reports item disappeared while wearing it.
**Cause:** Object had positive decay timer, not OBJ_NOTIMER. Decay ticks when in room, but the timer may have been set before equipping.
**Fix:** Ensure permanent equipment uses `decay_time = -1`. Check zone file or creation code.

### Items Not Decaying

**Symptom:** Temporary objects persist indefinitely.
**Cause:** decay_time is -1, object is in inventory/equipment, or objectTickUpdate not executing.
**Fix:** Verify decay_time is not -1. Check whether object resides in a room versus inventory. Examine type-specific objectDecay for decay prevention logic.

### Repair Shop Rejects Item

**Symptom:** Repair NPC refuses item with no clear message.
**Cause:** Item fails one of many unrepairable conditions.
**Fix:** Check: rentable, not burning, not cursed, not over max_exist, container is empty, objVnum != -1.

### Structure Damage in Arena

**Symptom:** Equipment taking damage during arena fights.
**Cause:** Room missing ROOM_ARENA flag.
**Fix:** Add ROOM_ARENA flag to zone file room definition.

### Unexpected Scrapping

**Symptom:** Item scrapped unexpectedly during combat.
**Cause:** Structure points reached zero from accumulated damage.
**Fix:** Check current structure points with getStructPoints(). Review combat logs for damage events. Verify material susceptibility matches expected values.

### Player Repair Consuming Excess Materials

**Symptom:** Materials depleting faster than expected.
**Cause:** Material type mismatch forcing fallback, or item not monogrammed when expected.
**Fix:** Verify commodity material matches item material exactly. Check isMonogrammed() status.

### Monogram Protection Not Working

**Symptom:** Monogrammed item scrapped instead of dropped.
**Cause:** isMonogrammed() returning false, or scrapMonogrammed() call path bypassed.
**Fix:** Verify item has monogram set through isMonogrammed check. Review scrapMonogrammed call path in makeScraps.

### Items Left at Shop Disappearing

**Symptom:** Player cannot retrieve repaired item.
**Cause:** Items abandoned more than 15 days past due date are deleted.
**Fix:** Players must retrieve promptly. No recovery possible after deletion.

### Rust Not Removed After Repair

**Symptom:** ITEM_RUSTY flag persists after repair.
**Cause:** Repair completed via player skill, not shop.
**Fix:** Shop repair explicitly removes ITEM_RUSTY. Player repair may need similar logic added.

### Structure Points Above Maximum

**Symptom:** Item shows "better than new" condition unexpectedly.
**Cause:** addToStructPoints() call without capping, or maxFix() bypass.
**Fix:** Review addToStructPoints calls for positive values without capping. Verify maxFix enforces 95% cap during repairs.

### Depreciation Not Accumulating

**Symptom:** Items never reach unrepairable state despite repeated repairs.
**Cause:** Depreciation system is disabled; getDepreciation() returns hardcoded 0.
**Fix:** This is by design. System infrastructure exists but requires enabling to function.

### makeScraps() DELETE_THIS Not Handled

**Symptom:** Use-after-free crash after item destruction.
**Cause:** Caller continued execution after damageItem() returned DELETE_THIS.
**Fix:** Always check return value: `if (rc == DELETE_THIS) return DELETE_ITEM;`
