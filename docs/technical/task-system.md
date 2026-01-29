---
title: SneezyMUD Task System
description: The task system provides delayed, sequenced, and periodic character actions through scheduler-driven processes that tie up characters for set durations across multiple game pulses.
keywords:
  - taskData
  - start_task
  - stopTask
  - procCharTasks
  - CMD_TASK_CONTINUE
  - CMD_TASK_FIGHTING
  - taskTypeT
  - TaskEntry
  - calcNextUpdate
  - isTaskObj
  - DELETE flag handling
  - Pulse::MOBACT
  - nobrainerTaskCommand
  - utilityTaskCommand
category: Understanding Systems

  - scheduler-system.md
  - command-implementation.md
last_updated: 2026-01-29
source_files:
  - code/code/misc/task.h
  - code/code/misc/task.cc
  - code/code/sys/socket.cc
related: [memory-safety.md]
---

# SneezyMUD Task System

The task system provides delayed/sequenced/periodic mob/player actions. Tasks tie up characters for set durations, allowing timed events (sharpening, brewing, picking locks) to occur across multiple game pulses.

## Core Files

- `code/code/misc/task.h` - Task type enum, `TaskEntry`, function declarations
- `code/code/misc/task.cc` - `taskData` class, `start_task()`, `stopTask()`, task table
- `code/code/sys/socket.cc` - `procCharTasks::run()` (lines 1106-1126)

## taskData Structure

```cpp
class taskData {
  public:
    taskTypeT task;       // Which task (TASK_SHARPEN, TASK_BREWING, etc.)
    int nextUpdate;       // Next pulse when CMD_TASK_CONTINUE fires
    int timeLeft;         // Counter, typically incremented each CMD_TASK_CONTINUE
    const char* orig_arg; // Original argument string (dynamically allocated)
    int wasInRoom;        // Room number where task started
    ubyte status;         // Task-specific state byte
    int flags;            // Task-specific flags (often stores spell/item data)
    TObj* obj;            // RAW POINTER - see hazard below
    TRoom* room;          // RAW POINTER - see hazard below
};
```

**CRITICAL: Dangling Pointer Hazard** - The `obj` and `room` fields are raw pointers. If the referenced object/room is deleted while the task is active, these become dangling. Task functions must validate before use. The `isTaskObj()` flag on objects provides partial protection.

## Task Types (51 total)

Defined in `taskTypeT` enum (task.h): TASK_BOGUS through TASK_PREEN. Major categories include repair tasks, crafting (brewing, scribing, cooking), trap-setting, meditation/rest states, and skill-based actions (tracking, fishing, logging).

## Task Lifecycle

### Starting: `start_task()`

```cpp
start_task(ch, obj, roomp, TASK_SHARPEN, arg, 0, in_room, 1, 0, 40);
// Creates taskData, stores object pointer, marks object with setIsTaskObj(true)
```

### Processing: `procCharTasks::run()`

Called every `Pulse::MOBACT` (1.2 seconds). When `pulse >= task->nextUpdate`, invokes the task function with `CMD_TASK_CONTINUE`:

```cpp
int rc = (*(tasks[tmp_ch->task->task].taskf))(
  tmp_ch, CMD_TASK_CONTINUE, "", pl.pulse, tmp_ch->task->room, tmp_ch->task->obj);
if (IS_SET_DELETE(rc, DELETE_ITEM)) delete tmper_obj;
if (IS_SET_DELETE(rc, DELETE_THIS)) return true;  // Character deleted
```

### Stopping: `TBeing::stopTask()`

Clears `isTaskObj` flag, frees `orig_arg`, deletes task struct, sets `task = NULL`.

## Task Function Signature

```cpp
int task_xxx(TBeing* ch, cmdTypeT cmd, const char* arg, int pulse, TRoom* rp, TObj* obj);
```

## Command Handling

| Command | When Sent | Typical Response |
|---------|-----------|------------------|
| `CMD_TASK_CONTINUE` | Every pulse when ready | Advance task state |
| `CMD_TASK_FIGHTING` | Combat starts | Stop task |
| `CMD_ABORT`/`CMD_STOP` | Player input | Stop gracefully |
| Other | Player input | Block or allow |

### Allowed Commands During Tasks

`nobrainerTaskCommand()`: say, tell, shout, inventory, equipment, social emotes, wiznet, reply
`utilityTaskCommand()`: look, score, who, help, save, time, weather, toggle, etc.

### Standard Task Pattern

```cpp
int task_xxx(TBeing* ch, cmdTypeT cmd, ...) {
  if (ch->utilityTaskCommand(cmd) || ch->nobrainerTaskCommand(cmd))
    return FALSE;  // Allow command
  switch (cmd) {
    case CMD_TASK_CONTINUE:
      ch->task->calcNextUpdate(pulse, 2 * Pulse::MOBACT);
      // Do task work
      return FALSE;
    case CMD_ABORT:
    case CMD_STOP:
      ch->stopTask();
      break;
    case CMD_TASK_FIGHTING:
      ch->sendTo("Cannot do this while fighting!\n\r");
      ch->stopTask();
      break;
    default:
      if (cmd < MAX_CMD_LIST) warn_busy(ch);
      break;  // Eat command
  }
  return TRUE;  // Command consumed
}
```

## Room Validation

Most tasks check movement: `if (ch->in_room != ch->task->wasInRoom) { ch->stopTask(); return FALSE; }`

## nextUpdate Timing

```cpp
void taskData::calcNextUpdate(int pulse, int interval) {
  nextUpdate = (pulse + interval) % 2400;  // Wraps at 2400
}
```

`Pulse::MOBACT` = 12 pulses (1.2 sec). Common intervals: `Pulse::MOBACT` (1.2s), `2*Pulse::MOBACT` (2.4s), `5*Pulse::MOBACT` (6s).

**Gotcha**: Initial `nextUpdate` in `start_task()` is meaningless - first `CMD_TASK_CONTINUE` fires almost immediately. Set proper timing during first continue using `status` field to detect first call.

## DELETE Flag Handling

Task functions must check and propagate DELETE flags:

```cpp
int rc = someOperation();
if (IS_SET_DELETE(rc, DELETE_THIS)) return DELETE_THIS;  // Character deletion
if (IS_SET_DELETE(rc, DELETE_ITEM)) return DELETE_ITEM;  // Object deletion
```

## TaskEntry Table

```cpp
typedef struct _tasks_entry {
  const char* const name;             // "sharpening a weapon"
  const char* const you_are_busy_msg; // "You are too busy sharpening.\n\r"
  int (*taskf)(...);                  // Task function
} TaskEntry;
extern TaskEntry tasks[NUM_TASKS];
```

## Common Gotchas

1. **Dangling pointers**: Validate `task->obj`/`task->room` before dereferencing
2. **DELETE flag propagation**: Check and return DELETE_THIS/DELETE_ITEM
3. **First continue timing**: Initial nextUpdate ignored; set it in first CMD_TASK_CONTINUE
4. **Linkdead check**: Stop tasks on `ch->isLinkdead()`
5. **Room movement**: Verify `ch->in_room == ch->task->wasInRoom`
6. **Return semantics**: TRUE = command consumed, FALSE = let it proceed
