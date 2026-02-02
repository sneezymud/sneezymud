---
title: Task System
category: critical
keywords: [taskData, start_task, stopTask, procCharTasks, CMD_TASK_CONTINUE, CMD_TASK_FIGHTING, taskTypeT, DELETE_flags]
related: [scheduler-pulses.md, command-implementation.md, memory-safety.md]
primary_symbols:
  functions: [start_task, stopTask, procCharTasks::run, calcNextUpdate, nobrainerTaskCommand, utilityTaskCommand]
  classes: [taskData, TaskEntry]
  files: [code/code/misc/task.h, code/code/misc/task.cc, code/code/sys/socket.cc]
---

## Overview

The task system enables delayed, sequenced, and periodic character actions that span multiple game pulses. Tasks tie up characters for set durations, preventing most commands while background work occurs. Used for crafting (brewing, scribing, cooking), repair actions (sharpening, smithing), utility skills (tracking, fishing, lockpicking), trap-setting, and meditation states.

Tasks execute through the scheduler pulse system. Every Pulse::MOBACT interval, procCharTasks::run checks whether each character's task is ready for its next update. When the task's scheduled pulse arrives, the system invokes the task function with CMD_TASK_CONTINUE, allowing it to advance state, perform validation, schedule the next update, or complete.

### Critical Safety Properties

Tasks store raw pointers to objects and rooms. If these entities are deleted while the task is active, the pointers become dangling. The isTaskObj flag on objects provides partial protection by allowing destructors to clean up referencing tasks. Task functions must validate all pointer fields before use. The system integrates with DELETE flag propagation to handle character and object deletion during task execution.

### Architecture

The taskData structure stores task state: which task is active, when the next update occurs, time elapsed, original arguments, starting room, task-specific status and flags, and raw pointers to associated objects/rooms. The TaskEntry table maps each taskTypeT enum value to a task function, descriptive name, and busy message.

Task functions follow a command-dispatch pattern. They receive commands indicating continue, abort, stop, fighting, or player input. Most commands are blocked during task execution, but nobrainerTaskCommand and utilityTaskCommand allow communication and information queries. Tasks must return TRUE to consume commands or FALSE to allow them to proceed.

## Patterns

### Starting a Task

Call start_task with the character, optional object and room pointers, task type enum, argument string, status byte, initial room number, flags, and initial nextUpdate value. The function allocates a taskData structure, copies the argument string, stores pointers, and marks objects with setIsTaskObj to enable cleanup. The initial nextUpdate value is overridden immediately on first continue, so set proper timing during the first CMD_TASK_CONTINUE using the status field to detect the initial call.

### Task Function Structure

Implement the standard signature accepting TBeing pointer, cmdTypeT, argument string, pulse number, TRoom pointer, and TObj pointer. Check utility and nobrainer commands first, returning FALSE to allow them. Implement a switch statement handling CMD_TASK_CONTINUE for work advancement, CMD_ABORT and CMD_STOP for graceful termination, and CMD_TASK_FIGHTING for combat interruption. Block all other commands by warning the character and returning TRUE to consume the input.

### Timing Updates

Within CMD_TASK_CONTINUE, call calcNextUpdate on the task object passing the current pulse and desired interval. Common intervals are Pulse::MOBACT for rapid updates, two times Pulse::MOBACT for standard progression, and five times Pulse::MOBACT for slower actions. The function wraps nextUpdate at 2400 pulses to prevent overflow. Schedule the next update before performing task work to ensure consistent timing even if the work triggers early completion.

### Room Validation

Check whether the character has moved from the starting room by comparing in_room with task->wasInRoom. If they differ, stop the task and return FALSE. Most tasks require the character to remain stationary, though some like tracking permit limited movement.

### Object Validation

Before dereferencing task->obj, verify the pointer is non-null and check any object-specific conditions like equipment state or flag requirements. If the object is no longer valid, stop the task and notify the character. Never assume the object pointer remains valid throughout task execution.

### Completion and Cleanup

When the task finishes successfully, call stopTask before sending completion messages. This clears the isTaskObj flag, frees the orig_arg string, deletes the taskData structure, and sets the character's task pointer to NULL. If the task creates or modifies objects, perform those operations before calling stopTask to ensure the task context remains valid during object manipulation.

### DELETE Flag Propagation

After any operation that might delete the character or associated objects, check return codes with IS_SET_DELETE. If DELETE_THIS is set, return DELETE_THIS immediately without further processing. If DELETE_ITEM is set and the item is the task object, stop the task first, then return DELETE_ITEM. Never continue task execution after a DELETE flag is detected.

### Allowing Commands

Return FALSE from the task function to permit a command to proceed normally. Use this for utility commands, communication commands, and any task-specific commands that should work during the task. Return TRUE to consume the command and prevent execution, typically sending a busy message to the character.

### First Update Handling

Use the status field to detect the first CMD_TASK_CONTINUE. Set status to zero in start_task, then check for zero during the first continue to initialize timing, display startup messages, and establish the proper update interval. Increment status afterward to prevent re-initialization on subsequent continues.

## Reference

### taskData Fields

The task field stores the taskTypeT enum identifying which task is active. The nextUpdate field holds the pulse number when the next CMD_TASK_CONTINUE will fire, calculated modulo 2400. The timeLeft field functions as a counter, typically incremented each continue to track progress. The orig_arg field stores the original argument string passed to start_task, dynamically allocated and freed during cleanup. The wasInRoom field records the room number where the task started for movement validation. The status field provides a task-specific state byte for custom logic. The flags field stores task-specific integer data, often spell numbers or item vnums. The obj field holds a raw pointer to an associated object, requiring validation before use. The room field holds a raw pointer to an associated room, requiring validation before use.

### Task Types

The taskTypeT enum defines 51 task types from TASK_BOGUS through TASK_PREEN. Repair tasks include sharpening, smithing, and mending. Crafting tasks include brewing, scribing, cooking, leatherworking, and tailoring. Skill tasks include tracking, fishing, logging, mining, and foraging. Combat-adjacent tasks include bandaging and poison extraction. Utility tasks include meditation, resting, trap-setting, lockpicking, and preen. Each task type maps to a TaskEntry in the global tasks table.

### TaskEntry Structure

The name field provides a descriptive string like "sharpening a weapon" for display purposes. The you_are_busy_msg field stores the message sent when a player tries to execute a blocked command, like "You are too busy sharpening.\n\r". The taskf field holds a function pointer to the task implementation matching the standard task function signature.

### Command Types

CMD_TASK_CONTINUE indicates the scheduled update pulse has arrived and the task should advance. CMD_TASK_FIGHTING signals that combat has started and most tasks should abort. CMD_ABORT and CMD_STOP represent explicit player requests to terminate the task. All other command types represent player input that tasks typically block unless permitted by utility or nobrainer checks.

### Allowed Command Sets

nobrainerTaskCommand returns true for communication commands that require no action: say, tell, shout, gossip, auction, holler, emote, semote, sockets, wiznet, reply, and related social interactions. utilityTaskCommand returns true for information commands that don't affect game state: look, exits, inventory, equipment, score, who, help, save, time, weather, toggle, and similar queries. Tasks can safely allow these without breaking immersion or mechanics.

### Pulse Intervals

Pulse::MOBACT equals 12 pulses, approximately 1.2 seconds. Multiplying by two yields 2.4 seconds per update, suitable for most tasks. Multiplying by five yields 6 seconds per update, appropriate for slow tasks like brewing or meditation. The pulse system wraps at 2400, so calcNextUpdate applies modulo arithmetic to prevent overflow.

### Return Value Semantics

Returning FALSE from a task function allows the command to proceed through normal command processing. This is correct for utility commands, communication commands, and task-specific permitted commands. Returning TRUE consumes the command and prevents further processing, typically used to block actions while the character is busy. DELETE_THIS indicates the character has been deleted and processing must stop immediately. DELETE_ITEM indicates the task object has been deleted and appropriate cleanup must occur.

## Implementation

### Scheduler Integration

procCharTasks::run executes every Pulse::MOBACT interval as part of the main game loop. The function iterates all descriptors, retrieves the character, and checks whether a task exists. If pulse is greater than or equal to task->nextUpdate, the function invokes the task function pointer from the tasks table, passing the character, CMD_TASK_CONTINUE, empty string, current pulse, task room pointer, and task object pointer. The return value is checked for DELETE_ITEM to delete the object if necessary, then checked for DELETE_THIS to signal character deletion back to the scheduler.

### start_task Allocation

start_task allocates a new taskData structure using new, sets the task type, copies the orig_arg string using mud_str, stores the object and room pointers, records wasInRoom and nextUpdate, and sets status and flags. If an object pointer is provided, the function calls setIsTaskObj with true to mark the object as task-associated, enabling cleanup if the object is deleted before task completion. The function assigns the taskData pointer to the character's task field, making the task active.

### stopTask Cleanup

stopTask first checks whether the task pointer is null, returning early if no task is active. It calls setIsTaskObj with false on the associated object if present, clearing the task protection flag. It frees the orig_arg string using delete_mud_str. It deletes the taskData structure using delete. It sets the character's task field to NULL. This sequence ensures all allocated resources are released and the object is no longer protected.

### calcNextUpdate Arithmetic

calcNextUpdate adds the interval to the current pulse, then applies modulo 2400 to wrap around. The result is stored in the nextUpdate field. This wrapping prevents nextUpdate from growing unbounded and ensures consistent comparison semantics in the scheduler. Tasks must call this function during each CMD_TASK_CONTINUE to schedule the next update, otherwise the task will fire every pulse.

### Dangling Pointer Detection

When an object marked with isTaskObj is deleted, the destructor iterates all descriptors to find characters whose task->obj matches the deleted object. For each match, it calls stopTask to cleanly terminate the task before the object memory is freed. This prevents dangling pointer access but creates a dependency between object deletion and descriptor iteration. Rooms lack similar protection, so task functions must validate room pointers independently.

### Command Dispatch

Task functions receive commands through the cmdTypeT parameter. The character's parseCommand flow checks whether a task is active and invokes the task function before normal command processing. If the task function returns TRUE, the command is consumed and normal processing is skipped. If it returns FALSE, the command proceeds through standard handler lookup and execution. This interception allows tasks to control which commands are permitted while maintaining normal command semantics for allowed actions.

### Task Table Initialization

The global tasks array contains one TaskEntry per taskTypeT enum value. Each entry is initialized with a string literal name, a string literal busy message, and a function pointer to the task implementation. The table is indexed directly by the task type enum, so the enum order must match the table order. Adding a new task requires defining the enum value, implementing the task function, and adding the corresponding TaskEntry to the table.

### Linkdead Handling

procCharTasks checks isLinkdead on the character before invoking the task function. If the character has disconnected, most tasks are stopped automatically. Some tasks like meditation may continue during linkdead states. This check prevents tasks from consuming resources or creating side effects for disconnected characters who cannot observe or respond to task progression.

## Troubleshooting

### Task Continues Every Pulse

The task function failed to call calcNextUpdate during CMD_TASK_CONTINUE. Without updating nextUpdate, the pulse comparison remains true every cycle. Add calcNextUpdate call at the start of CMD_TASK_CONTINUE before any other processing.

### Task Stops Immediately After Start

The task function is checking conditions incorrectly during the first continue. The initial nextUpdate from start_task is meaningless and the first continue fires almost immediately. Use the status field to detect first call and initialize timing properly before checking completion conditions.

### Dangling Pointer Crash

The task->obj or task->room pointer was dereferenced after the object or room was deleted. Validate pointers before use by checking non-null. For objects, verify isTaskObj flag is still set. For rooms, check against NULL and Room::NOWHERE. If validation fails, stop the task and return.

### DELETE Flag Not Propagated

An operation returned DELETE_THIS or DELETE_ITEM but the task function did not check the return value and continued executing. After any call that might delete entities, use IS_SET_DELETE to check for deletion flags. If DELETE_THIS is set, return it immediately. If DELETE_ITEM is set and affects the task object, stop the task first, then return the flag.

### Commands Blocked Incorrectly

The task function is returning TRUE for commands that should be allowed. Check utility and nobrainer command sets first, returning FALSE to permit them. Only block commands that genuinely conflict with the task action.

### Commands Allowed Incorrectly

The task function is returning FALSE for commands that should be blocked. Ensure the default case in the switch statement returns TRUE to consume unexpected commands. Only return FALSE for explicitly permitted command types.

### Task Persists After Movement

The task function is not validating wasInRoom. Add a check comparing in_room with task->wasInRoom at the start of CMD_TASK_CONTINUE. If they differ, stop the task and return FALSE.

### Memory Leak on Task Abort

The task function is not calling stopTask before returning from abort conditions. Always call stopTask when terminating the task, whether from successful completion, player abort, combat interruption, or validation failure.

### Object Not Cleaned on Task Stop

The task function is calling stopTask before clearing object modifications. stopTask clears the isTaskObj flag, so object state changes must occur before calling stopTask if they depend on task context. Perform all object modifications, then call stopTask as the final cleanup step.

### Timing Drift Over Long Tasks

The task is incrementing timeLeft but not tracking actual elapsed time. For precise timing, calculate expected completion pulse during first continue, then compare current pulse against that target. This prevents drift from calcNextUpdate wrapping and scheduler jitter.
