# Systems Documentation

Technical documentation for SneezyMUD's core systems, organized by criticality for developers working on the codebase.

## Categories

### Critical (`critical/`)

Systems where misunderstanding leads to **crashes, memory corruption, data loss, or security vulnerabilities**. These documents cover:

- Memory safety patterns
- Iterator safety during deletion
- Bidirectional pointer consistency
- Data persistence and binary format constraints
- Security-sensitive operations

**Read these first.** Bugs in critical systems cause use-after-free, double-delete, dangling pointers, or data corruption. The numbered prefixes (01-23) indicate recommended reading order, from most foundational to more specialized.

### Critical File Ordering

Critical files are numbered 01-24 by foundational importance:

| #   | File                        | Why It's Ordered Here                          |
| --- | --------------------------- | ---------------------------------------------- |
| 01  | memory-safety               | DELETE flag system underpins everything        |
| 02  | scheduler-pulses            | Central timing for all game updates            |
| 03  | combat-rounds               | Complex DELETE handling, global iterator cache |
| 04  | network-architecture        | Descriptor lifecycle, I/O handling             |
| 05  | spatial-relationships       | Bidirectional pointers for containment         |
| 06  | object-system               | Physical items, traps, containers              |
| 07  | command-implementation      | All player actions flow through here           |
| 08  | persistence-storage         | Binary format - changes destroy data           |
| 09  | group-party                 | reformGroup() prevents dangling pointers       |
| 10  | spec-procs                  | Extensibility with memory safety concerns      |
| 11  | movement-terrain-navigation | Multiple death paths with DELETE flags         |
| 12  | trap-mechanics              | Environmental hazards, iterator safety         |
| 13  | mount-riding-system         | Rider chains, fall damage propagation          |
| 14  | monster-ai-behavior         | NPC autonomy, charList cleanup                 |
| 15  | task-system                 | Multi-pulse actions, object validation         |
| 16  | spell-combat                | Flag translation, area effects                 |
| 17  | economy-system              | Dual saves prevent money duplication           |
| 18  | magic-items                 | Scrolls/wands DELETE flag handling             |
| 19  | object-manipulation         | Get/drop/put with trap triggers                |
| 20  | object-maintenance          | Decay, structure points                        |
| 21  | sstring-guide               | String wrapper with silent failures            |
| 22  | admin-systems               | Privilege escalation risks                     |
| 23  | snoop-switch                | Descriptor manipulation                        |
| 24  | transformation-system       | Polymorph/disguise/shapeshift mechanics        |

### Important (`important/`)

Systems where misunderstanding causes **gameplay bugs, balance issues, or incorrect behavior** but not crashes or corruption. These documents cover:

- Combat mechanics and formulas
- Character progression and stats
- Spell and skill systems
- Environmental interactions
- Builder tools and content systems

Understanding these systems is necessary for feature work but errors are recoverable.

### Informational (`informational/`)

**Reference material, convenience features, and supplementary documentation**. These documents cover:

- UI and display systems
- Player convenience features (aliases, toggles)
- Disabled or legacy systems
- Material and crafting reference data

Useful for specific tasks but not required reading.

## Document Structure

Each document follows a consistent format:

```yaml
---
title: System Name
description: One-line summary
keywords: [searchable, terms]
category: critical|important|informational
related: [paths/to/related/docs.md]
---
```

- **Overview**: What the system does and why it exists
- **Patterns**: Correct usage with code examples
- **Anti-patterns**: Common mistakes to avoid
- **Implementation Notes**: Key functions and files

## Using the Frontmatter

The YAML frontmatter enables efficient navigation and discovery across the documentation.

### `keywords`

Searchable terms for finding relevant documentation. Include:

- Function names (`reconcileDamage`, `reformGroup`, `IS_SET_DELETE`)
- Class names (`TBeing`, `TObj`, `Descriptor`)
- Concept names (`DELETE_THIS`, `ownership`, `iterator safety`)
- Related terms users might search for

**Example usage**: Search for `DELETE_VICT` across all docs to find every system that deals with victim deletion.

### `related`

Cross-references to connected documentation. Use relative paths from `docs/systems/`:

```yaml
related: [critical/01-memory-safety.md, important/combat-formulas.md]
```

Follow `related` links to:

- Understand prerequisites before reading a document
- Find deeper detail on mentioned concepts
- Discover systems that interact with the current one

### `category`

Must match the directory the file is in (`critical`, `important`, or `informational`). Used for:

- Validation that files are correctly placed
- Filtering documentation by risk level
- Automated tooling and indexes

### `primary_symbols` (optional)

Some documents include detailed symbol references:

```yaml
primary_symbols:
  functions: [reconcileDamage, applyDamage, rawKill]
  classes: [TBeing, TMonster]
  files: [code/code/misc/combat.cc]
```

Use this to quickly locate the source files and symbols a document covers.

### `source_files` (optional)

Direct references to implementation files:

```yaml
source_files: [code/code/misc/parse.cc, code/code/cmd/cmd_kick.cc]
```

Jump directly to the code when you need implementation details beyond what the documentation covers.

## Contributing

When adding or updating documentation:

1. Place files in the appropriate category based on crash/corruption risk
2. Update `category` in frontmatter to match the directory
3. Add `related` links to connected systems
4. For critical systems, propose ordering changes if the new content is more foundational than existing entries
