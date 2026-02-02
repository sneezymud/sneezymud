---
title: Task System
description: Delayed, sequenced, and periodic character actions via scheduler-driven processes spanning multiple game pulses.
created_by_model: opus
---

## Overview

Tasks are timed character actions that span multiple game pulses. When a character sharpens a weapon, brews a potion, or picks a lock, the task system manages the duration, handles interrupts, and coordinates with the command parser to block conflicting actions.

The scheduler calls task functions every 1.2 seconds, passing control signals that drive state machine transitions. Tasks persist until completed, aborted, or interrupted by combat or movement.

## Patterns

**Always validate task object pointers before use.** The `obj` and `room` fields are raw pointers that become dangling if the referenced entity is deleted mid-task. Check for null and use `isTaskObj()` for partial protection.

**Always propagate DELETE flags from task functions.** When operations return DELETE_THIS or DELETE_ITEM, immediately return that flag rather than continuing execution.

**Always check room movement early.** Compare `ch->in_room` against `task->wasInRoom` and stop the task if the character moved.

**Always set timing in the first CMD_TASK_CONTINUE.** The initial `nextUpdate` from `start_task()` is effectively ignored; the first continue fires immediately. Use the `status` field to detect the first call.

**Always stop tasks on linkdead.** Check `ch->isLinkdead()` in CMD_TASK_CONTINUE handlers.

**Never block utility commands.** Call `nobrainerTaskCommand()` and `utilityTaskCommand()` at the start of every task function to allow passive actions like say, look, score, and inventory.

**Never continue after DELETE detection.** Check IS_SET_DELETE immediately after any operation that can return deletion flags.

## Reference

### Command Signals

| Signal | Trigger | Expected Response |
|--------|---------|-------------------|
| CMD_TASK_CONTINUE | Scheduler pulse ready | Advance state, recalculate nextUpdate |
| CMD_TASK_FIGHTING | Combat initiated | Stop task, notify player |
| CMD_ABORT | Player types "abort" | Stop task gracefully |
| CMD_STOP | Player types "stop" | Stop task gracefully |
| Other commands | Player input | Block and warn, or allow via utility checks |

### Return Semantics

| Return Value | Meaning |
|--------------|---------|
| FALSE (0) | Command proceeds normally |
| TRUE (1) | Command consumed/blocked |
| DELETE_THIS | Character was deleted |
| DELETE_ITEM | Object was deleted |

### Timing Constants

| Interval | Pulses | Real Time |
|----------|--------|-----------|
| Pulse::MOBACT | 12 | 1.2 seconds |
| 2 * Pulse::MOBACT | 24 | 2.4 seconds |
| 5 * Pulse::MOBACT | 60 | 6.0 seconds |

### Allowed Commands During Tasks

- **nobrainerTaskCommand**: say, tell, shout, inventory, equipment, social emotes, wiznet, reply
- **utilityTaskCommand**: look, score, who, help, save, time, weather, toggle

### Source Files

| File | Contents |
|------|----------|
| code/code/misc/task.h | taskTypeT enum, TaskEntry struct, function declarations |
| code/code/misc/task.cc | taskData class, start_task(), stopTask(), task table |
| code/code/sys/socket.cc | procCharTasks::run() scheduler entry point |

## Implementation

### taskData Structure

The `taskData` class stores all per-task state. Key fields:

- `task` - taskTypeT enum identifying the task type
- `nextUpdate` - pulse count when next CMD_TASK_CONTINUE fires
- `timeLeft` - counter typically incremented each continue
- `orig_arg` - dynamically allocated original argument string
- `wasInRoom` - room vnum where task started (for movement detection)
- `status` - task-specific state byte
- `flags` - task-specific flags (often stores spell/item data)
- `obj` - raw TObj pointer (dangling hazard)
- `room` - raw TRoom pointer (dangling hazard)

### Lifecycle Phases

**Starting**: `start_task()` allocates taskData, stores object pointer, marks object with `setIsTaskObj(true)`, and attaches to character.

**Processing**: `procCharTasks::run()` executes every Pulse::MOBACT. When `pulse >= task->nextUpdate`, it invokes the task function with CMD_TASK_CONTINUE and handles DELETE flag returns.

**Stopping**: `TBeing::stopTask()` clears `isTaskObj` flag, frees `orig_arg`, deletes the taskData struct, and sets `task = NULL`.

### nextUpdate Calculation

`calcNextUpdate(pulse, interval)` sets nextUpdate to `(pulse + interval) % 2400`. The wrap at 2400 matches the pulse cycle. Initial nextUpdate from start_task() fires almost immediately, so the first CMD_TASK_CONTINUE must set proper timing for subsequent calls.

### TaskEntry Table

The `tasks[]` array maps taskTypeT to TaskEntry structs containing:
- `name` - descriptive string like "sharpening a weapon"
- `you_are_busy_msg` - message shown when blocking commands
- `taskf` - function pointer to task handler

51 task types exist (TASK_BOGUS through TASK_PREEN), covering repair, crafting, meditation, tracking, fishing, and other timed actions.

## Troubleshooting

**Symptom**: Crash when accessing task object
**Cause**: Object deleted while task active; dangling `task->obj` pointer
**Fix**: Validate pointer before use; set `isTaskObj(true)` when starting task so object destructor can clean up

**Symptom**: Task fires immediately without delay
**Cause**: Initial nextUpdate ignored; relying on start_task() timing
**Fix**: Set proper interval in first CMD_TASK_CONTINUE using `calcNextUpdate()`

**Symptom**: Task continues after character moved rooms
**Cause**: Missing room validation
**Fix**: Check `ch->in_room != ch->task->wasInRoom` early in CMD_TASK_CONTINUE

**Symptom**: Character deleted but execution continues
**Cause**: DELETE_THIS not propagated from task function
**Fix**: Check IS_SET_DELETE immediately after operations and return the flag

**Symptom**: Linkdead character stuck in task
**Cause**: Missing linkdead check in task handler
**Fix**: Check `ch->isLinkdead()` in CMD_TASK_CONTINUE and call stopTask()
