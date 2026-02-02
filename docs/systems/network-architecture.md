---
title: Network and Protocol Architecture
description: Single-threaded event loop, descriptor lifecycle, telnet/GMCP protocols, and I/O buffering
category: Critical Systems
related: [memory-safety.md, spatial-relationships.md, player-interface.md]
source_files: [sys/socket.cc, sys/socket.h, sys/connect.cc, sys/connect.h, sys/gmcphandlers.cc, sys/comm.h, sys/DescriptorList.h]
created_by_model: opus
---

# Network and Protocol Architecture

## Overview

SneezyMUD uses a classic single-threaded, non-blocking I/O model with `select()`. All network operations occur in the main game loop without threading. Each connected client gets a Descriptor that manages connection state, input/output buffering, and character association.

The tick rate is 0.1 seconds. Each tick: accept new connections, close exceptional sockets, read input, process commands, generate prompts, flush output.

**Why this matters:** Misusing these patterns causes disconnects, data loss, or crashes. The single-threaded model requires careful iterator management since any operation can delete any descriptor.

## Patterns

### Safe Iteration

- Always cache `next_to_process` BEFORE any operation on the current descriptor
- Always check `DELETE_THIS` return values immediately after command dispatch
- Never continue processing a descriptor after detecting deletion
- Never assume a descriptor still exists after calling functions that might delete it

### Input Processing

- Always handle partial reads by accumulating in the raw buffer until newline
- Always double `$` characters on input to prevent printf issues (undone on output)
- Always truncate lines exceeding `MAX_INPUT_LENGTH`
- Always handle telnet escape sequences before parsing commands

### Output Processing

- Always check the `gmcp` flag before sending GMCP messages
- Always forward output to snoopers before writing to socket
- Always apply color codes before socket write
- Never write to a socket after detecting a write error

### Descriptor Lifecycle

- Always update `next_to_process` in the destructor to prevent use-after-free
- Always handle snoop relationships during destruction
- Always return polymorphed characters to original form before deletion
- Always save in-game characters and mark linkdead rather than deleting them

### DELETE_THIS Handling

- Always check `IS_SET_DELETE(rc, DELETE_THIS)` after `nanny()`, `parseCommand()`, `sendLogin()`, and similar dispatch functions
- Always delete the descriptor and set pointer to NULL when DELETE_THIS is detected
- Always use `continue` to skip remaining loop body after deletion

## Reference

### Connection States

| State | Value | Context |
|-------|-------|---------|
| `CON_PLYNG` | 0 | Actively playing |
| `CON_NME` | 1 | Getting name |
| `CON_NMECNF` | 2 | Confirming name |
| `CON_PWDNRM` | 3 | Getting password |
| `CON_PWDCNF` | 4 | Confirming password |
| `CON_RMOTD` | 5 | Reading MOTD |
| `CON_NEWACT` | 10 | Creating new account |
| `CON_ACTPWD` | 11 | Account password |
| `CON_NEWLOG` | 12 | New account login |
| `CON_EMAIL` | 14 | Getting email |
| `CON_TERM` | 15 | Terminal type |
| `CON_CONN` | 16 | Connecting character |
| `CON_CREATION_START` | 84 | Character creation begins |
| `CON_CREATION_MAX` | 108 | Character creation ends |
| `CON_REDITING` | >MAX_CON_STATUS | Room editor |
| `CON_OEDITING` | >MAX_CON_STATUS | Object editor |
| `CON_MEDITING` | >MAX_CON_STATUS | Mobile editor |

### Telnet Constants

| Name | Value | Meaning |
|------|-------|---------|
| `iac` | 255 (0xFF) | Interpret As Command |
| `dont` | 254 (0xFE) | Don't option |
| `do_` | 253 (0xFD) | Do option |
| `wont` | 252 (0xFC) | Won't option |
| `will` | 251 (0xFB) | Will option |
| `sb` | 250 (0xFA) | Subnegotiation Begin |
| `se` | 240 (0xF0) | Subnegotiation End |
| `GMCP` | 201 (0xC9) | GMCP option code |

### GMCP Commands

| Command | Behavior |
|---------|----------|
| `Core.Hello` | Stores client name/version in descriptor |
| `Core.Supports.Set` | Ignored (squelched) |
| `External.Discord.Hello` | Returns Discord invite URL |
| `request sectors` | Returns sector type list |
| `request area` | Returns current zone info |
| `remember` | Player memory system storage |
| `retrieve` | Player memory retrieval |

### Comm Classes

| Class | Purpose |
|-------|---------|
| `UncategorizedComm` | Generic text output |
| `GmcpComm` | GMCP protocol messages |
| `LoginComm` | Login prompts |
| `PromptComm` | Player prompts with metadata |
| `SnoopComm` | Snoop output |
| `TellFromComm` / `TellToComm` | Tell messages |
| `RoomExitComm` | Exit data for clients |
| `SoundComm` | Sound effects |
| `WhoListComm` | Who list entries |

### Global Variables

| Variable | Type | Purpose |
|----------|------|---------|
| `descriptor_list` | `Descriptor*` | Head of linked list |
| `next_to_process` | `Descriptor*` | Safe iteration pointer |
| `maxdesc` | `int` | Highest fd in use |
| `avail_descs` | `int` | Max allowed descriptors (150) |
| `gSocket` | `TMainSocket*` | Global main socket |

### Socket Configuration

| Setting | Value | Purpose |
|---------|-------|---------|
| TCP_KEEPIDLE | 180 seconds | Idle time before probes |
| TCP_KEEPINTVL | 180 seconds | Probe interval |
| Listen backlog | 10 | Pending connection queue |
| SO_REUSEADDR | enabled | Allow port reuse |
| FNDELAY | enabled | Non-blocking I/O |

## Implementation

### Main Loop Structure

`TMainSocket::gameLoop()` runs a scheduler that fires `procHandleTimeAndSockets` every tick. That proc calls `handleTimeAndSockets()` which orchestrates:

1. `select()` on all sockets with timeout
2. `newDescriptor()` for connections on main socket
3. Close exceptional sockets (FD in exc_set)
4. `inputProcessing()` for each readable descriptor
5. `processAllInput()` to dispatch queued commands
6. `setPrompts()` to generate prompts for descriptors with pending output
7. `afterPromptProcessing()` to flush output buffers

### Descriptor Structure

Key fields: `socket` (TSocket wrapper), `connected` (state enum), `character` (TBeing pointer), `account` (TAccount pointer), `original` (for polymorph), `m_raw[4096]` (input accumulator), `input` (parsed command queue), `output` (pending Comm queue), `wait` (command delay), `gmcp` (GMCP enabled flag), `next` (linked list pointer).

The constructor prepends to `descriptor_list`. The destructor updates `next_to_process`, removes from list, handles snoop relationships, and either marks character linkdead or deletes it depending on connection state.

### Input Flow

`inputProcessing()` reads into `m_raw` accumulating partial data until newline. Telnet sequences are stripped via `handleTelnetOpts()`. Lines are parsed and pushed to the `input` queue. Dollar signs are doubled to prevent printf format string issues.

`processAllInput()` iterates descriptors, always caching `next_to_process` first. For each descriptor with positive `wait` counter and non-empty input queue, it pops a command and dispatches based on connection state:

- String editor active: `sstring_add()`
- Pager active: `page_file()`
- No account: `sendLogin()` (login flow)
- Account not confirmed: `doAccountStuff()` (account menu)
- `CON_PLYNG`: `parseCommand()` (game commands)
- Editor states: respective editor functions
- Other: `nanny()` (connection state machine)

### Output Flow

Output uses polymorphic Comm objects via shared pointers in a queue. `outputProcessing()` pops each Comm, undoes dollar doubling, forwards to snooper if any, applies color codes, and writes to socket. Errors return -1 triggering descriptor deletion.

`setPrompts()` generates state-appropriate prompts (task prompts, editor prompts, game prompts with HP/mana/move). `afterPromptProcessing()` calls `outputProcessing()` for descriptors with pending output.

### Telnet/GMCP Handling

`handleTelnetOpts()` recursively processes IAC sequences. Any telnet command auto-enables GMCP (pragmatic hack for clients that don't negotiate properly). WILL/DO/WONT/DONT are answered appropriately. Subnegotiations (SB...SE) extract the payload and dispatch to `handleGmcpCommand()` for GMCP option.

GMCP messages are sent via `sendGmcp()` which wraps content in IAC SB GMCP ... IAC SE framing. The `gmcp` flag is checked before sending.

### Linkdead Handling

When a descriptor is destroyed while character is in-game (connected >= CON_REDITING or connected == 0): character remains in-world marked linkdead, followers are removed (prevents item duplication), character is saved, invisibility set to GOD_LEVEL1, mount is dismounted, master/follower relationships broken (unless charmed).

## Troubleshooting

| Symptom | Cause | Fix |
|---------|-------|-----|
| Crash in descriptor iteration | Not caching `next_to_process` before operations | Cache at loop start: `next_to_process = d->next;` |
| Use-after-free on descriptor | Ignoring DELETE_THIS return | Check immediately: `if (IS_SET_DELETE(rc, DELETE_THIS)) { delete d; continue; }` |
| Partial command execution | Not handling partial reads | Accumulate in `m_raw` until newline detected |
| Format string vulnerability | Dollar signs in player input | Input processing doubles `$`, output undoes it |
| GMCP not working | Client doesn't negotiate properly | Auto-enable on any telnet command (already implemented) |
| Write to closed socket | Continuing after write error | Check `writeToSocket()` return and bail on non-zero |
| Snoop data leaking | Not checking snoop relationship | Forward to `snoop.snoop_by->desc` in output processing |
| Linkdead character lost | Not saving on disconnect | Destructor calls `doQueueSave()` for in-game characters |
| Duplicate items after disconnect | Followers not removed | Destructor removes followers before marking linkdead |
| Connection refused | Capacity exceeded | Check `maxdesc + 1 >= avail_descs` in `newDescriptor()` |
