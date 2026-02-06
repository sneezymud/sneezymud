---
title: Network and Protocol Architecture
description: Single-threaded event loop, descriptor lifecycle, telnet/GMCP protocols, and I/O buffering
keywords: [network, socket, descriptor, telnet, GMCP, select, non-blocking, I/O]
category: critical
primary_symbols:
  functions: [gameLoop, handleTimeAndSockets, inputProcessing, processAllInput, outputProcessing, parseCommand, nanny, sendGmcp, handleTelnetOpts]
  classes: [Descriptor, TMainSocket, TSocket, Comm, GmcpComm]
  enums: [CON_PLYNG, CON_NME, CON_NMECNF, CON_PWDNRM, CON_PWDCNF, CON_RMOTD, CON_REDITING, CON_OEDITING, CON_MEDITING, CON_CREATION_START, DELETE_THIS, IS_SET_DELETE, MAX_INPUT_LENGTH, iac, dont, do_, wont, will, sb, GMCP]
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
- Always truncate lines exceeding `MAX_INPUT_LENGTH` (1024 characters)
- Always handle telnet escape sequences before parsing commands
- Always handle backspace characters by deleting previous character
- Support command history expansion: `!` for last command, `!!` for repeat, `!number` for specific recall

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
| `CON_CREATION_START` | 27 | Character creation begins |
| `CON_CREATION_NAME` | 27 | Character name entry (same as START) |
| `CON_CREATION_SEX` | 30 | Sex selection |
| `CON_CREATION_RACE` | 31 | Race selection |
| `CON_CREATION_CLASS` | 32 | Class selection |
| `CON_CREATION_DONE` | 50 | Creation finalization |
| `CON_REDITING` | above MAX_CON_STATUS | Room editor |
| `CON_OEDITING` | above MAX_CON_STATUS | Object editor |
| `CON_MEDITING` | above MAX_CON_STATUS | Mobile editor |
| `CON_HELP` | above MAX_CON_STATUS | Help file editor |
| `CON_WRITING` | above MAX_CON_STATUS | String editor |

Editor states are above MAX_CON_STATUS. Creation states start at 27 and increment from there with distinct substates for disclaimers, trait selection, and stat customization.

### Telnet Constants

| Name | Value | Meaning |
|------|-------|---------|
| `iac` | 255 (0xFF) | Interpret As Command |
| `dont` | 254 (0xFE) | Don't option |
| `do_` | 253 (0xFD) | Do option |
| `wont` | 252 (0xFC) | Won't option |
| `will` | 251 (0xFB) | Will option |
| `sb` | 250 (0xFA) | Subnegotiation Begin |
| `GMCP` | 201 (0xC9) | GMCP option code |

Note: `se` (Subnegotiation End, 240/0xF0) is commented out in the code and not defined as a constant. The raw value 0xF0 is used directly where needed.

Telnet sequences use IAC as escape prefix. Negotiations are three bytes: IAC WILL/WONT/DO/DONT option. Subnegotiations are: IAC SB option data IAC SE.

### GMCP Commands

| Command | Handler | Behavior |
|---------|---------|----------|
| `Core.Hello` | `handleCoreHello` | Stores client name/version in descriptor |
| `Core.Supports.Set` | Squelched | Ignored (clients send extensive capability lists) |
| `External.Discord.Hello` | `handleDiscord` | Returns Discord invite URL |
| `request sectors` | `handleRequestSectors` | Returns sector type list |
| `request area` | `handleRequestArea` | Returns current zone info |
| `remember` | `handleRemember` | Player memory system storage |
| `retrieve` | `handleRetrieve` | Player memory retrieval |

Commands are dispatched through a std::map of std::function handlers. Unrecognized commands are silently ignored.

### Comm Classes

| Class | Purpose | Metadata |
|-------|---------|----------|
| `UncategorizedComm` | Generic text output | None |
| `GmcpComm` | GMCP protocol messages | JSON payload |
| `LoginComm` | Login prompts | None |
| `PromptComm` | Player prompts | HP, mana, move, position, flags |
| `SnoopComm` | Snoop output | Source character name |
| `TellFromComm` / `TellToComm` | Tell messages | Target/sender name |
| `RoomExitComm` | Exit data for clients | Exit directions and vnums |
| `SoundComm` | Sound effects | Sound file identifier |
| `WhoListComm` | Who list entries | Character data |

The Comm hierarchy uses shared_ptr ownership through CommPtr typedef. Each subclass implements getText to provide formatted output.

### Global Variables

| Variable | Type | Purpose |
|----------|------|---------|
| `descriptor_list` | `Descriptor*` | Head of linked list |
| `next_to_process` | `Descriptor*` | Safe iteration pointer |
| `maxdesc` | `int` | Highest fd in use |
| `avail_descs` | `int` | Max allowed descriptors (150) |
| `gSocket` | `TMainSocket*` | Global main socket |

The next_to_process global is updated by the Descriptor destructor to skip deleted entries.

### Socket Configuration

| Setting | Value | Purpose |
|---------|-------|---------|
| Socket type | PF_INET6, SOCK_STREAM | IPv6 TCP (also accepts IPv4) |
| SO_REUSEADDR | enabled | Allow port reuse |
| SO_LINGER | disabled | Close immediately, don't wait for send buffer |
| SO_KEEPALIVE | enabled | Detect dead connections |
| TCP_KEEPIDLE | 180 seconds | Idle time before probes |
| TCP_KEEPINTVL | 180 seconds | Probe interval |
| FNDELAY | enabled | Non-blocking I/O |
| Listen backlog | 10 | Pending connection queue |

### Limits and Constants

| Constant | Value | Purpose |
|----------|-------|---------|
| MAX_INPUT_LENGTH | 1024 | Maximum command length before truncation |
| m_raw buffer size | 4096 | Raw input accumulation buffer |
| avail_descs | 150 | Maximum concurrent connections |
| Tick duration | 0.1 seconds | Event loop cycle time |
| Command pacing | 1 tick minimum | Default wait counter value |

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

`inputProcessing()` reads into `m_raw` accumulating partial data until newline. Telnet sequences are stripped via `handleTelnetOpts()`. Lines are parsed and pushed to the `input` queue. Dollar signs are doubled to prevent printf format string issues. Backspace characters delete the previous character. Command history expansion supports `!`, `!!`, and `!number` syntax.

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

`setPrompts()` generates state-appropriate prompts (task prompts, editor prompts, game prompts with HP/mana/move). Exit data is sent via RoomExitComm for clients that display mini-maps. `afterPromptProcessing()` calls `outputProcessing()` for descriptors with pending output.

### Telnet/GMCP Handling

`handleTelnetOpts()` recursively processes IAC sequences. Any telnet command auto-enables GMCP (pragmatic hack for clients that don't negotiate properly). WILL/DO/WONT/DONT are answered appropriately. Subnegotiations (SB...SE) extract the payload and dispatch to `handleGmcpCommand()` for GMCP option.

GMCP messages are sent via `sendGmcp()` which wraps content in IAC SB GMCP ... IAC SE framing. The `gmcp` flag is checked before sending. The `strip` parameter controls color code removal from the payload.

### GMCP Negotiation Sequence

On new connection, `startGmcp` pushes IAC WILL GMCP to the output queue. The client responds with IAC DO GMCP or IAC WILL GMCP. Both responses are accepted and set `gmcp` to true.

After negotiation, the client sends `Core.Hello` via IAC SB GMCP Core.Hello JSON IAC SE. The handler parses JSON to extract client name and version, storing them in `mudclient`. `Core.Supports.Set` follows with capability lists, which are squelched.

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
| Lost GMCP messages | Sending without checking gmcp flag | Guard all `sendGmcp()` calls with `if (descriptor->gmcp)` |
| Input not processing | Wait counter stuck positive | Verify wait counter decrements each tick in processAllInput |
| Telnet negotiation loops | Client doesn't respect DONT/WONT | Verify handleTelnetOpts sends appropriate rejections |
| Command history corruption | History buffer indexing error | Check bounds and circular buffer wraparound logic |
| Snoop chain crashes | Stale snoop pointers after deletion | Destructor must clear all snoop relationships first |
