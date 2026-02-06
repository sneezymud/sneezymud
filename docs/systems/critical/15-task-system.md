---
title: Task System
description: Delayed, sequenced, and periodic character actions via scheduler-driven processes spanning multiple game pulses.
category: critical
keywords: [delayed actions, scheduler, crafting, sharpening]
primary_symbols:
  functions: [start_task, stopTask, procCharTasks, calcNextUpdate, nobrainerTaskCommand, utilityTaskCommand]
  classes: [taskData]
  enums: [taskTypeT, CMD_TASK_CONTINUE, CMD_TASK_FIGHTING, CMD_ABORT, CMD_STOP, TASK_BOGUS, TASK_PREEN, Pulse]
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

**Always call stopTask before returning from abort conditions.** Whether from successful completion, player abort, combat interruption, or validation failure, call stopTask to prevent memory leaks.

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

- **nobrainerTaskCommand**: say, say2, glance, tell, shout, weather, inventory, equipment, smile, shake, nod, gt, wiznet, reply
- **utilityTaskCommand**: look, score, who, help, save, time, weather, toggle

### Task Type Categories

| Category | Examples |
|----------|----------|
| Repair | sharpening, smithing, mending |
| Crafting | brewing, scribing, cooking, leatherworking, tailoring |
| Skill | tracking, fishing, logging, mining, foraging |
| Combat-adjacent | bandaging, poison extraction |
| Utility | meditation, resting, trap-setting, lockpicking, preen |

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

52 task types exist (TASK_BOGUS through TASK_PREEN), covering repair, crafting, meditation, tracking, fishing, and other timed actions. Adding a new task requires defining the enum value, implementing the task function, and adding the corresponding TaskEntry to the table.

### Dangling Pointer Protection

When an object marked with `isTaskObj` is deleted, the destructor iterates `character_list` to find characters whose `task->obj` matches the deleted object. For each match, it calls `stopTask()` to cleanly terminate the task before the object memory is freed. Rooms lack similar protection, so task functions must validate room pointers independently.

### Command Dispatch

The character's `parseCommand` flow checks whether a task is active and invokes the task function before normal command processing. If the task function returns TRUE, the command is consumed and normal processing is skipped. If it returns FALSE, the command proceeds through standard handler lookup and execution.

## Troubleshooting

**Symptom**: Crash when accessing task object
**Cause**: Object deleted while task active; dangling `task->obj` pointer
**Fix**: Validate pointer before use; set `isTaskObj(true)` when starting task so object destructor can clean up

**Symptom**: Task fires immediately without delay
**Cause**: Initial nextUpdate ignored; relying on start_task() timing
**Fix**: Set proper interval in first CMD_TASK_CONTINUE using `calcNextUpdate()`

**Symptom**: Task fires every pulse instead of at intervals
**Cause**: Task function not calling `calcNextUpdate()` during CMD_TASK_CONTINUE
**Fix**: Add `calcNextUpdate()` call at the start of CMD_TASK_CONTINUE before any other processing

**Symptom**: Task continues after character moved rooms
**Cause**: Missing room validation
**Fix**: Check `ch->in_room != ch->task->wasInRoom` early in CMD_TASK_CONTINUE

**Symptom**: Character deleted but execution continues
**Cause**: DELETE_THIS not propagated from task function
**Fix**: Check IS_SET_DELETE immediately after operations and return the flag

**Symptom**: Linkdead character stuck in task
**Cause**: Missing linkdead check in task handler
**Fix**: Check `ch->isLinkdead()` in CMD_TASK_CONTINUE and call stopTask()

**Symptom**: Commands blocked that should be allowed
**Cause**: Task function returning TRUE for utility/communication commands
**Fix**: Check nobrainer and utility command sets first, returning FALSE to permit them

**Symptom**: Commands allowed that should be blocked
**Cause**: Task function returning FALSE for disallowed commands
**Fix**: Ensure default case returns TRUE to consume unexpected commands

**Symptom**: Memory leak on task abort
**Cause**: Task function not calling stopTask() before returning
**Fix**: Always call stopTask() when terminating the task for any reason

**Symptom**: Timing drift over long tasks
**Cause**: Incrementing timeLeft without tracking actual elapsed time
**Fix**: Calculate expected completion pulse during first continue, then compare against target
