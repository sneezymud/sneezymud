---
title: Object Maintenance
description: SneezyMUD's object maintenance systems, including decay timers, structure points (object health), damage mechanics, and repair systems for managing item durability and lifespan.
keywords: [decay_time, struct_points, OBJ_NOTIMER, objectDecay, decayMe, damageItem, makeScraps, repairPrice, repair_time, maxFix, willDent, willTear, willPuncture, genericDamCheck, stripSpellAffects, depreciation, CORPSE_SACRIFICE, ITEM_RUSTY, ITEM_NEWBIE]
category: Important Systems

  - material-system.md
  - weapon-system.md
  - combat-formulas.md
  - economy-system.md
  - task-system.md
  - persistence-rent.md
last_updated: 2026-01-29
source_files:
  - code/code/misc/obj.h
  - code/code/misc/periodic.cc
  - code/code/misc/combat.cc
  - code/code/misc/materials.cc
  - code/code/misc/repair.cc
  - code/code/misc/info.cc
  - code/code/task/task_blacksmithing.cc
  - code/code/obj/obj_base_corpse.cc
  - lib/help/repair shops
  - lib/help/item damage
related: [object-system.md]
---
# Object Maintenance

This document describes SneezyMUD's object maintenance systems, including decay timers, structure points (object health), damage mechanics, and repair systems.

## Overview

Objects in SneezyMUD have two primary health metrics that determine their lifespan and effectiveness:

1. **Decay Timer** (`decay_time`) - A countdown to deletion, primarily for temporary objects like corpses, summoned items, and food
2. **Structure Points** (`struct_points`) - Object durability that degrades from combat damage and can be repaired

These systems work independently: decay causes automatic deletion after a timer expires, while structure damage affects item effectiveness and can be fixed through repair.

**Key characteristics:**
- Decay timer of -1 (OBJ_NOTIMER) means the item never decays
- Structure points reaching 0 causes the item to become "scrapped"
- Repair shops and player skills can restore structure points
- Monogrammed items receive special protection from scrapping

## Object Decay System

### How Decay Works

Decay is processed in `TObj::objectTickUpdate()` (`code/code/misc/periodic.cc:1873-1979`).

**Flow:**

```
1. Check if decay_time > -1 (object has decay enabled)
2. Call decayMe() to decrement decay_time (only if object is in a room)
3. When decay_time reaches 0:
   a. Call objectDecay() (virtual, type-specific behavior)
   b. If objectDecay() returns non-zero, delete the object
   c. Otherwise, relocate contents and delete
```

**Source: `code/code/misc/periodic.cc` (lines 1866-1979)**

```cpp
void TObj::decayMe() {
  if (obj_flags.decay_time > 0)
    if (in_room != Room::NOWHERE)
      obj_flags.decay_time--;
}
```

### Decay Timer Values

| Timer Value | Meaning |
|-------------|---------|
| -1 (`OBJ_NOTIMER`) | Never decays - permanent object |
| 0 | Ready to decay this tick |
| > 0 | Ticks remaining until decay |

The constant `OBJ_NOTIMER` is defined in `code/code/misc/obj.h:48`.

### Decay Messages

When objects decay, players see location-specific messages:

| Location | Message |
|----------|---------|
| Equipped | "$p decay$Q into nothing." |
| In inventory | "$p disintegrate$Q in your hands." |
| In room | "$n fade$R into insignificance." |

**Source:** `code/code/misc/periodic.cc` (lines 1952-1975)

### Type-Specific Decay

Different object types override `objectDecay()` for special behavior:

| Type | Behavior | Source |
|------|----------|--------|
| `TBaseCorpse` | Decay message, logs player corpse info | `obj_base_corpse.cc` |
| `TPlayerCorpse` | Extended logging, content relocation | `obj_player_corpse.cc` |
| `TLight` | "Burns out" message | `obj_light.cc` |
| `TFood` | Spoilage message | `obj_food.cc` |
| `TPortal` | Simple deletion | `obj_portal.cc` |
| Default `TObj` | Relocate contents, delete | `periodic.cc:1951-1977` |

### Protected Items (No Decay)

Objects with `decay_time == -1` never decay. These include:

- Most permanent equipment (weapons, armor)
- Quest items
- Housing items
- Player-crafted permanent items

Objects with positive decay timers that cannot be customized or monogrammed are rejected by customizers (`code/code/spec/spec_mobs_customizers.cc:283-286`):

```cpp
if (valued->obj_flags.decay_time >= 0) {
    me->doTell(ch->getName(),
      "Sorry, but this item won't last long enough to bother with an customizing!");
    return TRUE;
}
```

### Corpse Decay

Corpses have special decay handling:

**Source:** `code/code/obj/obj_base_corpse.cc`

When a corpse decays:
1. Items inside are relocated to the room, parent container, or held by owner
2. ITEM_NEWBIE objects inside are destroyed (except in donation room or starting rooms)
3. Tables and furniture preserve items placed on them

Player corpses (`TPlayerCorpse`) have extended decay times and additional logging to help players recover their equipment.

## Structure Points System

### Structure Point Storage

**Source:** `code/code/misc/obj.h` (lines 507-508, 588-593)

```cpp
class objFlagData {
    short struct_points;      // Current structural health
    short max_struct_points;  // Maximum structural health
    // ...
};
```

**Accessor methods:**
- `getStructPoints()` - Current structure
- `setStructPoints(short)` - Set current structure
- `addToStructPoints(short)` - Add to current structure (can be negative)
- `getMaxStructPoints()` - Maximum structure
- `setMaxStructPoints(short)` - Set maximum structure

### Condition Levels

Object condition is displayed based on the ratio of current to maximum structure points.

**Source:** `code/code/misc/info.cc` (lines 3788-3840)

| Ratio | Condition | Color |
|-------|-----------|-------|
| > 100% | better than new | White |
| 100% | brand new | Cyan |
| > 90% | like new | cyan |
| > 80% | excellent | Blue |
| > 70% | very good | blue |
| > 60% | good | Purple |
| > 50% | fine | purple |
| > 40% | fair | Green |
| > 30% | poor | green |
| > 20% | very poor | yellow |
| > 10% | bad | orange |
| > 0.1% | very bad | Red |
| <= 0.1% | destroyed | red |

### How Items Take Damage

Items are damaged during combat based on material susceptibility checks.

**Source:** `code/code/misc/materials.cc` (lines 18-207)

**Damage types:**

| Function | Damage Type | Materials Vulnerable |
|----------|-------------|---------------------|
| `willDent()` | Blunt impact | Metals, minerals |
| `willTear()` | Slashing | Cloth, leather |
| `willPuncture()` | Piercing | Soft materials |

**Combat damage flow:**

```cpp
// In TBeing::dentItem(), tearItem(), pierceItem()
1. Check if arena fight (no damage in arena)
2. Get hardness of attacking slot/weapon
3. Call willDent/willTear/willPuncture check
4. If damage occurs:
   a. Show damage message (dent/chip/tear/puncture)
   b. Call item->damageItem(amt)
```

**The damage check formula (`genericDamCheck`):**

```cpp
// 30% base chance to check for damage
if (::number(0, 999) >= 300)
  return false;

// Compare attacker hardness + defender susceptibility
if (sharp && susc && ((min(30, sharp) + susc) > ::number(20, 120)))
  return TRUE;
```

### damageItem() Function

**Source:** `code/code/misc/combat.cc` (lines 1258-1275)

```cpp
int TObj::damageItem(short amt) {
  if (amt < 0) {
    vlogf(LOG_BUG, format("%s::damageItem(%i) got passed a negative number!") %
                     getName() % amt);
    amt = -amt;
  }

  addToStructPoints(max(-amt, -getStructPoints()));

  if (getStructPoints() <= 0) {
    if (!makeScraps()) {
      return DELETE_THIS;
    }
    return TRUE;
  }
  return FALSE;
}
```

### When Items Scrap

When structure points reach 0, `makeScraps()` is called.

**Source:** `code/code/misc/combat.cc` (lines 1348-1477)

**Scrapping flow:**
1. Remove ITEM_BURNING flag if set
2. If liquid container, drop liquid in room
3. Relocate contents to parent/room
4. If monogrammed, call `scrapMonogrammed()` (special handling)
5. Otherwise:
   a. Display scrap message ("falls to the ground, scrapped")
   b. Create TTrash object with appropriate description
   c. Delete original object

**Monogrammed items** (personalized equipment) have special handling - they are dropped but not fully destroyed, allowing recovery.

**Source:** `code/code/misc/combat.cc` (lines 1277-1345)

## Repair System

### Repair Shops

**Source:** `code/code/misc/repair.cc`

Repair shops are NPC-operated facilities that restore structure points to damaged items.

**Help file:** `lib/help/repair shops`

**Valid commands:**
- `VALUE <item>` - Get repair cost and estimated time
- `VALUE all.damaged` - Value all damaged inventory items
- `GIVE <item> <repairman>` - Submit item for repair
- `GIVE all.damaged <repairman>` - Submit all damaged items
- `GIVE <ticket> <repairman>` - Retrieve repaired item

### Repair Shop Mechanics

**Repair cost calculation** (`code/code/misc/repair.cc:136-211`):

```cpp
int TObj::repairPrice(TBeing* repair, TBeing* buyer, ...) const {
    int gsp = obj_flags.cost;

    // Subtract material cost portion
    gsp -= (int)(getWeight() * 10.0 * material_nums[getMaterial()].price);

    // Calculate percentage being repaired
    float perc_repaired = (maxFix(...) - getStructPoints()) / (float)getMaxStructPoints();

    // Apply gold modifier for repair economy
    int price = (int)(gsp * gold_modifier[GOLD_REPAIR].getVal());

    // Scale by repair percentage
    price = (int)(price * perc_repaired);

    // 5x cost for "destroyed" items (struct <= 0)
    if (getStructPoints() <= 0)
        price *= 5;

    // Add raw material costs
    int mat_price = findRepairMaterials(...);

    return mat_price + price;
}
```

**Repair time calculation** (`code/code/misc/repair.cc:214-278`):

```cpp
static int repair_time(TBeing* keeper, const TObj* o) {
    int structs = o->getMaxStructPoints() - o->getStructPoints();

    // Based on player repair formula:
    // 4 + (struct damage / success rate)
    double iTime = 4 + (structs / 0.3);
    iTime *= (Pulse::MOBACT / Pulse::ONE_SECOND);  // Convert to seconds
    iTime *= 1.25;  // Penalty for using NPC instead of self-repair

    // Modified by shop speed setting
    if (speed <= 5.0 && speed > 0)
        iTime *= speed;

    return (int)iTime;
}
```

### Maximum Repair Quality

Items cannot be repaired to full maximum; there is a cap.

**Source:** `code/code/misc/repair.cc` (lines 33-63)

```cpp
int TObj::maxFix(const TBeing* keeper, depreciationTypeT dep_done) const {
    int amount = getMaxStructPoints() - getDepreciation();

    amount *= 95;
    amount /= 100;  // 95% of max at best

    // Shop quality modifier
    if (shop_index[shop_nr].isOwned()) {
        float quality = shop_index[shop_nr].getRepairQuality();
        if (quality <= 1.0 && quality > 0)
            amount = (int)((float)amount * quality);
    }

    return amount;
}
```

### Items That Cannot Be Repaired

**Source:** `code/code/misc/repair.cc` (lines 492-591)

| Condition | Reason |
|-----------|--------|
| Not rentable | Unrepairable items |
| Already at max | No damage to repair |
| Depreciation maxed | Permanent damage threshold |
| Repair time = 0 | Already repaired |
| `objVnum() == -1` | Temporary/virtual items |
| `ITEM_NODROP` | Cursed items |
| `ITEM_BURNING` | Safety hazard |
| Over max_exist | Reclamation contract |
| Non-empty containers | Must empty first |

### Player Repair Skills

Players can repair items themselves using class-specific skills.

**Source:** `code/code/task/task_blacksmithing.cc`

| Skill | Class | Materials | Tools Required |
|-------|-------|-----------|----------------|
| `SKILL_BLACKSMITHING` | Warrior | Metal | Hammer, tongs, forge, anvil |
| `SKILL_REPAIR_SHAMAN` | Shaman | Organic/bone | Scalpel, forceps, operating table |
| `SKILL_REPAIR_MONK` | Monk | Wood | Ladle, soil, water source |
| `SKILL_REPAIR_MAGE` | Mage | Various | Skill-specific |
| `SKILL_REPAIR_CLERIC` | Cleric | Various | Skill-specific |
| `SKILL_REPAIR_THIEF` | Thief | Various | Skill-specific |
| `SKILL_REPAIR_DEIKHAN` | Deikhan | Various | Skill-specific |

**Repair skill formula** (`code/code/task/task_blacksmithing.cc:316-345`):

```cpp
// Success check
if ((percent = ::number(1, 101)) != 101)  // 101 is complete failure
    percent -= m_ch->getDexReaction() * 3;

// Repair the object
if (percent < m_ch->getSkillValue(m_skill))
    o->addToStructPoints(1);   // Success: +1 structure
else if (o->getStructPoints() > 0)
    o->addToStructPoints(-1);  // Failure: -1 structure
```

### Material Requirements

Player repair consumes raw materials based on item weight and repair percentage.

**Source:** `code/code/task/task_blacksmithing.cc` (lines 192-214)

```cpp
bool BaseRepair::ConsumeRepairMats(TObj* o) {
    int mats_needed = (int)((o->getWeight() / (float)o->getMaxStructPoints()) * 10.0);
    mats_needed = (int)(repair_mats_ratio * mats_needed);  // 10% ratio

    // Monogrammed items: 25% material cost
    if (o->isMonogrammed())
        mats_needed = mats_needed / 4;

    // Find matching commodity material in inventory
    TCommodity* mat = getRepairMaterial(m_ch->stuff, o->getMaterial());
    if (!mat || mat->numUnits() < mats_needed) {
        act("You don't have enough material to continue repairing $p.", ...);
        return false;
    }

    // Consume materials
    mat->setWeight(mat->getWeight() - (mats_needed / 10.0));
    return true;
}
```

## Depreciation System

Depreciation tracks permanent wear on items that cannot be repaired away.

**Source:** `code/code/misc/obj.h` (lines 594-599)

```cpp
short getDepreciation() const {
    return 0;  // Currently disabled
    // return obj_flags.depreciation;
}
void setDepreciation(short num) { obj_flags.depreciation = num; }
void addToDepreciation(short num) { obj_flags.depreciation += num; }
```

**Note:** Depreciation is currently disabled (always returns 0) but the infrastructure remains. When active:
- Each repair cycle adds 1 depreciation
- Maximum repair quality decreases as depreciation increases
- Items eventually become unrepairable

## Common Gotchas

### Decay vs Structure

- Decay timer reaching 0 deletes the object entirely
- Structure points reaching 0 scraps the object (creates trash, may drop contents)
- They are independent systems - an item can have high structure but low decay timer

### Arena Protection

Items do not take structure damage during arena fights (`code/code/misc/materials.cc:66-68`):

```cpp
if (victim->roomp && victim->roomp->isRoomFlag(ROOM_ARENA))
    return FALSE;
```

### Object Lock System

Items can be temporarily protected from damage using the lock system:

```cpp
bool getLocked() const { return isLocked; }
void setLocked(bool l) { isLocked = l; }
```

**Source:** `code/code/misc/obj.h` (lines 605-607)

### Repair Shop Abandonement

Items left at repair shops for more than 15 days past their due date may be deleted (`lib/help/repair shops`).

### ITEM_RUSTY Flag

Rusty items (`ITEM_RUSTY`, bit 11) get the rust removed during repair:

```cpp
fixed_obj->remObjStat(ITEM_RUSTY);
```

**Source:** `code/code/misc/repair.cc:473`

## Key Constants

| Constant | Value | Location | Purpose |
|----------|-------|----------|---------|
| `OBJ_NOTIMER` | -1 | `obj.h:48` | Decay disabled |
| `repair_mats_ratio` | 0.10 | `repair.cc:31` | Material consumption rate |
| `ITEM_RUSTY` | `1 << 11` | `obj.h:467` | Item has rust damage |
| `ITEM_NOPURGE` | `1 << 23` | `obj.h:479` | Protected from purge |

## Related Documentation

- [Object Types](object-types.md) - Object class hierarchy and lifecycle
- [Material System](material-system.md) - Material properties and hardness
- [Weapon System](weapon-system.md) - Weapon sharpness and durability
- [Combat Formulas](combat-formulas.md) - How damage affects equipment
- [Economy System](economy-system.md) - Repair shop economics
- [Task System](task-system.md) - Player repair task framework
- [Persistence/Rent](persistence-rent.md) - How object state is saved

## Key Source Files

| File | Purpose |
|------|---------|
| `code/code/misc/obj.h` | Object flag definitions, accessor methods |
| `code/code/misc/periodic.cc` | Decay processing in objectTickUpdate() |
| `code/code/misc/combat.cc` | damageItem(), makeScraps() |
| `code/code/misc/materials.cc` | Damage susceptibility checks |
| `code/code/misc/repair.cc` | Repair shop mechanics |
| `code/code/misc/info.cc` | equip_condition() display |
| `code/code/task/task_blacksmithing.cc` | Player repair skill framework |
| `code/code/obj/obj_base_corpse.cc` | Corpse decay handling |
| `lib/help/repair shops` | Player help for repair commands |
| `lib/help/item damage` | Player help for damage mechanics |
