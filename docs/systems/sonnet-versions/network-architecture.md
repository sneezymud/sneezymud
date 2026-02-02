---
title: Network and Protocol Architecture
category: critical
keywords: [event loop, descriptors, telnet, GMCP, socket I/O, connection states, iterator safety, DELETE_THIS, non-blocking I/O, single-threaded]
related: [memory-safety.md, player-interface.md, spatial-relationships.md]
primary_symbols:
  functions: [gameLoop, inputProcessing, outputProcessing, processAllInput, setPrompts, handleTelnetOpts, handleGmcpCommand, newDescriptor]
  classes: [TMainSocket, TSocket, Descriptor, Comm]
  files: [code/code/sys/socket.cc, code/code/sys/connect.cc, code/code/sys/gmcphandlers.cc]
---

## Overview

SneezyMUD employs a single-threaded, non-blocking I/O architecture built on select-based event polling. Every network operation occurs within the main game loop without threading or asynchronous I/O. The TMainSocket listens for IPv6 connections, creating a Descriptor for each accepted client. Descriptors manage per-connection state including authentication flow, character association, I/O buffers, and protocol negotiation.

The event loop executes every 0.1 seconds, processing socket readiness, parsing input into command queues, dispatching commands through state machines, generating prompts, and flushing output. Input flows from raw socket reads through telnet protocol stripping into newline-delimited commands. Output uses a polymorphic Comm hierarchy to support categorized messages, GMCP protocol data, and client-specific formatting.

Connection lifecycle progresses through connectStateT states: login and account management, character creation with 26+ distinct states, active play in CON_PLYNG, and editor modes for building. The Descriptor destructor handles cleanup including snoop relationships, linkdead character persistence, and safe removal from the global descriptor_list linked list.

Telnet IAC sequences are stripped during input processing. GMCP negotiation occurs via telnet subnegotiation, with pragmatic auto-enablement on any detected telnet command to work around client negotiation bugs. Protocol handling is stateless within each tick.

Critical safety: Descriptor iteration requires caching next_to_process before any operation that might delete descriptors. The global next_to_process pointer prevents use-after-free during iteration. Command handlers return DELETE_THIS flags to signal descriptor deletion without direct cleanup responsibility.

## Patterns

### Safe Descriptor Iteration

Always cache the next pointer before operations that might delete the current or any other descriptor:

```cpp
Descriptor* d;
for (d = descriptor_list; d; d = next_to_process) {
    next_to_process = d->next;  // Cache BEFORE operations

    int rc = d->inputProcessing();
    if (rc < 0) {
        delete d;
        d = NULL;
        continue;
    }

    rc = d->processCommand();
    if (IS_SET_DELETE(rc, DELETE_THIS)) {
        delete d;
        d = NULL;
    }
}
```

The descriptor destructor updates next_to_process to skip deleted descriptors. Failing to cache the next pointer causes use-after-free when the current descriptor is deleted.

### DELETE_THIS Propagation

Command handlers and input processors return DELETE_THIS to signal that the descriptor should be destroyed. Callers must check this flag and halt further descriptor access:

```cpp
int rc = descriptor->nanny(input);
if (IS_SET_DELETE(rc, DELETE_THIS)) {
    delete descriptor;
    return;  // NEVER touch descriptor again
}

// Only safe if no DELETE_THIS
descriptor->writeToQ("Prompt");
```

Ignoring DELETE_THIS and continuing to use the descriptor causes crashes. The pattern applies throughout input processing, command dispatch, and state transitions.

### Connection State Dispatch

The processAllInput function routes input through different handlers based on connectStateT state and presence of account/character pointers:

```cpp
if (is_client_sstring(comm))
    rc = d->read_client(comm);           // GMCP client messages
else if (d->str)
    d->sstring_add(comm);                // String editor active
else if (d->pagedfile)
    d->page_file(comm);                  // Pager active
else if (!d->account)
    rc = d->sendLogin(comm);             // Pre-account login
else if (!d->account->status)
    rc = d->doAccountStuff(comm);        // Account menu
else if (!d->connected)
    rc = d->character->parseCommand(comm, TRUE);  // Playing
else if (d->connected == CON_REDITING)
    room_edit(d->character, comm);       // Room editor
else
    rc = d->nanny(comm);                 // Creation states
```

The dispatcher checks from most specific to least specific state. String editor and pager states override normal command processing. The connected field distinguishes playing from creation/editor modes.

### Output Comm Hierarchy

Output messages use polymorphic Comm subclasses pushed onto a per-descriptor queue. Each subclass implements getText to provide formatted output:

```cpp
descriptor->output.push(CommPtr(new UncategorizedComm("Generic text")));
descriptor->output.push(CommPtr(new PromptComm(hp, mana, move, ...)));
descriptor->output.push(CommPtr(new GmcpComm(gmcpMessage)));
```

The outputProcessing function pops from the queue, applies color codes, forwards to snoopers, and writes to the socket. Different Comm types enable client-specific behavior and metadata attachment without string manipulation.

### GMCP Conditional Sending

Always check the gmcp flag before sending GMCP messages. Clients that don't support GMCP will not process the telnet subnegotiation:

```cpp
if (descriptor->gmcp) {
    sstring payload = format(R"({"type": "room", "vnum": %d})") % vnum;
    descriptor->sendGmcp("Room.Info " + payload, true);
}
```

The sendGmcp function wraps the payload in IAC SB GMCP ... IAC SE. The strip parameter controls color code removal. GMCP is auto-enabled on any telnet command receipt due to client negotiation bugs.

### Telnet Protocol Stripping

handleTelnetOpts recursively processes IAC sequences in input, removing them from the input string and returning response sequences:

```cpp
sstring response = handleTelnetOpts(input, descriptor);
if (!response.empty())
    descriptor->output.push(CommPtr(new UncategorizedComm(response)));
```

The function handles WILL/WONT/DO/DONT negotiations and subnegotiation blocks. GMCP subnegotiations are dispatched to handleGmcpCommand. All other unsupported options receive DONT/WONT responses. Partial subnegotiations are left in the buffer for the next tick.

### Input Dollar Doubling

Input processing doubles dollar signs to prevent format string vulnerabilities in code that uses printf-style functions:

```cpp
// In inputProcessing
if (*(m_raw + i) == '$')
    *(tmp + ++k) = '$';  // Input "test$var" becomes "test$$var"

// In outputProcessing
while ((tc = strstr(tb, "$$"))) {
    memmove(tc, tc + 1, strlen(tc));  // "test$$var" becomes "test$var"
    tb = tc + 1;
}
```

This prevents user input from being interpreted as format specifiers. Output processing undoes the doubling before sending to the socket.

### Command Wait Counter

The wait field delays command execution for pacing. It decrements each tick and blocks input queue processing when positive:

```cpp
if ((--(d->wait) <= 0) && !d->input.empty()) {
    // Process next command
    d->wait = 1;  // Reset for next command
}
```

Most commands reset wait to 1, causing single-tick delay. Slow commands can set higher values to prevent command flooding. The counter prevents clients from executing more than 10 commands per second.

### Linkdead State Transition

When a descriptor is destroyed while the character is in CON_PLYNG or editor states, the character remains in-world but is marked linkdead:

```cpp
// In Descriptor destructor
if (connected >= CON_REDITING || !connected) {
    character->desc = NULL;  // Mark linkdead
    character->doQueueSave();
    character->reformGroup(FALSE);  // Remove followers
    character->setInvis(GOD_LEVEL1);
    // Break mount/master relationships
} else {
    delete character;  // Pre-login character deleted
}
```

The character persists for reconnection. Breaking follower relationships prevents item duplication exploits. Followers of linkdead characters are removed, but charmed followers remain.

### Socket Non-blocking Configuration

All client sockets use FNDELAY non-blocking mode. Read operations return EWOULDBLOCK when no data is available:

```cpp
thisround = read(socket->m_sock, buffer, size);
if (thisround < 0) {
    if (errno != EWOULDBLOCK)
        return -1;  // Real error
    else
        break;  // No more data this tick
}
```

Write operations similarly return immediately if the send buffer is full. The event loop resumes I/O on the next tick when select indicates readiness.

## Reference

### Connection States

| State | Value | Phase | Description |
|-------|-------|-------|-------------|
| CON_CREATION_START | 84 | Creation | Initial connection, pre-account |
| CON_NME | 1 | Login | Entering account name |
| CON_PWDNRM | 3 | Login | Account password entry |
| CON_NEWACT | 10 | Login | New account creation |
| CON_ACTPWD | 11 | Login | Account password verification |
| CON_NEWLOG | 12 | Login | New account initial login |
| CON_EMAIL | 14 | Login | Email address entry |
| CON_TERM | 15 | Login | Terminal type negotiation |
| CON_CREATION_NAME | 85 | Creation | Character name entry |
| CON_CREATION_SEX | 88 | Creation | Sex selection |
| CON_CREATION_RACE | 89 | Creation | Race selection |
| CON_CREATION_CLASS | 90 | Creation | Class selection |
| CON_CREATION_DONE | 108 | Creation | Finalization |
| CON_CONN | 16 | Play | Connecting character to world |
| CON_PLYNG | 0 | Play | Active play state |
| CON_RMOTD | 5 | Play | Reading message of the day |
| CON_REDITING | 109 | Editor | Room editor active |
| CON_OEDITING | 110 | Editor | Object editor active |
| CON_MEDITING | 111 | Editor | Mobile editor active |
| CON_HELP | 114 | Editor | Help file editor active |
| CON_WRITING | 115 | Editor | String editor active |

Editor states are above MAX_CON_STATUS. Creation states span 84-108 with 26 distinct substates for disclaimers, trait selection, and stat customization.

### Comm Message Types

| Class | Purpose | Metadata |
|-------|---------|----------|
| UncategorizedComm | Generic text output | None |
| LoginComm | Login prompts and messages | None |
| PromptComm | Game prompt with stats | HP, mana, move, position, flags |
| GmcpComm | GMCP protocol messages | JSON payload |
| SnoopComm | Snooped output with attribution | Source character name |
| TellFromComm | Outgoing tell messages | Target name |
| TellToComm | Incoming tell messages | Sender name |
| RoomExitComm | Room exit data for clients | Exit directions and vnums |
| SoundComm | Sound effect triggers | Sound file identifier |
| WhoListComm | Who list entries | Character data |

The Comm hierarchy uses shared_ptr ownership through CommPtr typedef. Each subclass implements getText to provide formatted output suitable for client consumption.

### GMCP Command Handlers

| Command | Handler | Description |
|---------|---------|-------------|
| Core.Hello | handleCoreHello | Client identification, stores name and version |
| Core.Supports.Set | Squelched | Client capability announcement, ignored |
| External.Discord.Hello | handleDiscord | Returns Discord server invite link |
| request sectors | handleRequestSectors | Returns sector type enumeration |
| request area | handleRequestArea | Returns current zone information |
| remember | handleRemember | Player memory storage |
| retrieve | handleRetrieve | Player memory retrieval |

Commands are dispatched through a std::map of std::function handlers. Unrecognized commands are silently ignored. The Core.Supports.Set command is explicitly squelched because clients send extensive capability lists that aren't used.

### Telnet Protocol Constants

| Constant | Value | Description |
|----------|-------|-------------|
| IAC | 0xFF (255) | Interpret As Command |
| DONT | 0xFE (254) | Refuse to perform option |
| DO | 0xFD (253) | Request option enable |
| WONT | 0xFC (252) | Refuse to enable option |
| WILL | 0xFB (251) | Offer to enable option |
| SB | 0xFA (250) | Subnegotiation begin |
| SE | 0xF0 (240) | Subnegotiation end |
| GMCP | 0xC9 (201) | GMCP option code |

Telnet sequences use IAC as escape prefix. Negotiations are three bytes: IAC WILL/WONT/DO/DONT option. Subnegotiations are: IAC SB option data IAC SE. GMCP is an unofficial option code registered in the telnet option registry.

### Global Variables

| Variable | Type | Scope | Purpose |
|----------|------|-------|---------|
| descriptor_list | Descriptor* | Global | Head of descriptor linked list |
| next_to_process | Descriptor* | Global | Safe iteration cache pointer |
| maxdesc | int | Global | Highest file descriptor in use |
| avail_descs | int | Global | Maximum allowed descriptors (150) |
| gSocket | TMainSocket* | Global | Main listening socket instance |

The descriptor_list is a singly-linked list using the next field of each Descriptor. The next_to_process global enables safe iteration when descriptors might be deleted. It is updated by the Descriptor destructor to skip deleted entries.

### Socket Configuration

| Setting | Value | Purpose |
|---------|-------|---------|
| Socket type | PF_INET6, SOCK_STREAM | IPv6 TCP socket (also accepts IPv4) |
| SO_REUSEADDR | Enabled | Allow immediate port reuse after shutdown |
| SO_LINGER | Off | Close immediately, don't wait for send buffer |
| SO_KEEPALIVE | Enabled | Detect dead connections |
| TCP_KEEPIDLE | 180 seconds | Idle time before keepalive probes |
| TCP_KEEPINTVL | 180 seconds | Interval between keepalive probes |
| FNDELAY | Set | Non-blocking I/O mode |
| Listen backlog | 10 | Queue size for pending connections |

The main socket binds to IPv6 but accepts IPv4 connections through v4-mapped addresses. Client sockets inherit non-blocking mode from fcntl F_SETFL FNDELAY after accept.

### Limits and Constants

| Constant | Value | Purpose |
|----------|-------|---------|
| MAX_INPUT_LENGTH | 1024 | Maximum command length before truncation |
| m_raw buffer size | 4096 | Raw input accumulation buffer |
| avail_descs | 150 | Maximum concurrent connections |
| Tick duration | 0.1 seconds | Event loop cycle time |
| Command pacing | 1 tick minimum | Default wait counter value |

Commands exceeding MAX_INPUT_LENGTH are truncated. The m_raw buffer accumulates partial reads until a newline. Connection attempts when descriptor count exceeds avail_descs are rejected with a "game is full" message.

## Implementation

### Event Loop Execution

The gameLoop function in TMainSocket initializes a TScheduler and registers processes for combat, ticks, commands, and other periodic tasks. The procHandleTimeAndSockets process runs every tick (Pulse::EVERY) and invokes handleTimeAndSockets. This function builds file descriptor sets for reading, writing, and exceptions, then calls select with a 0.1 second timeout. Select returns when sockets are ready or timeout expires.

When the listening socket is readable, newDescriptor calls accept to get the client socket, wraps it in TSocket, creates a Descriptor, and performs initial GMCP negotiation. Client sockets are checked against avail_descs capacity before creation. Exceeding capacity sends a rejection message and closes the socket.

After accepting new connections, handleTimeAndSockets iterates descriptors checking for exceptional conditions using FD_ISSET on exc_set. Sockets with exceptions are immediately deleted. Readable sockets invoke inputProcessing. Writable sockets are tracked for prompt generation.

The processAllInput function then iterates descriptors again, popping commands from input queues and dispatching through state-appropriate handlers. The wait counter gates command processing to enforce pacing. After command dispatch, setPrompts generates prompts for descriptors ready to receive them. Finally, afterPromptProcessing calls outputProcessing to flush output queues to sockets.

### Input Processing Flow

The inputProcessing function performs non-blocking reads into the m_raw buffer, accumulating data across multiple ticks until a newline appears. Read returns indicate completion (positive), blocking (EWOULDBLOCK), or errors (negative). EOF from read zero return value signals disconnect.

After reading, handleTelnetOpts recursively scans for IAC sequences. WILL/WONT/DO/DONT negotiations extract three bytes and generate appropriate responses. Unsupported options receive negative acknowledgments. GMCP negotiations are accepted. Subnegotiation blocks extract data between IAC SB and IAC SE, dispatching GMCP payloads to handleGmcpCommand.

The remaining input after telnet stripping is parsed into newline-delimited commands. Dollar sign doubling prevents format string exploits. Backspace characters delete the previous character. Command history expansion supports !, !!, and !number syntax to recall previous commands. Each completed line is pushed onto the input queue.

Input queue processing in processAllInput pops the front command if the wait counter has reached zero. The command string is copied into a static buffer and dispatched through the state machine hierarchy. String editor mode sends input to sstring_add. Pager mode sends to page_file. Account creation sends to nanny. Playing state sends to parseCommand.

### Output Processing Flow

The setPrompts function generates prompts based on descriptor state. Task-specific prompts display progress for meditation, sharpening, and other long-running activities. String editor prompts show "-> ". Pager prompts show navigation instructions. Game prompts construct HP/mana/move displays and push PromptComm messages. Exit data is sent via RoomExitComm for clients that display mini-maps.

The afterPromptProcessing function calls outputProcessing for descriptors with non-empty output queues and writable sockets. Output processing pops CommPtr shared pointers from the queue and calls getComm to retrieve formatted text. Dollar sign undoubling reverses input protection. Color code expansion uses colorString to interpret ANSI codes based on descriptor preferences.

Snoop forwarding checks snoop_by and pushes SnoopComm messages to the snooper's output queue. This creates recursive output processing for nested snoop chains. The final formatted string is written via writeToSocket, which performs non-blocking writes and returns error codes on failure.

Write failures return -1 from outputProcessing, triggering descriptor deletion in afterPromptProcessing. Successful writes clear the prompt_mode flag, resetting prompt generation for the next tick. Partial writes are not buffered; the entire output must fit in the socket send buffer or the descriptor is closed.

### Descriptor Lifecycle Phases

New descriptor creation begins in CON_CREATION_START state with no account or character pointers. The sendLogin function displays the initial connection screen and prompts for account name. Input transitions through CON_NME, CON_PWDNRM, and account creation states. Account authentication loads TAccount from the database.

The doAccountStuff handler processes account menu commands for character selection, creation, and deletion. Selecting a character loads TBeing from the database and transitions to CON_CONN. The connection handler moves the character into the game world and transitions to CON_PLYNG.

Character creation flows through 26 creation substates managed by creation_nanny. Each state presents choices for race, class, sex, traits, and stat customization. State transitions occur on valid input. Invalid input repeats the prompt. Creation completion allocates starting equipment and transitions to CON_CONN.

Playing state processes game commands through parseCommand. Editor commands like "edit room" transition to CON_REDITING and invoke specialized handlers. The string editor sets the str pointer and processes input through sstring_add. Exiting editors clears the mode and returns to CON_PLYNG.

Descriptor destruction checks connected state to determine character fate. Editor states and CON_PLYNG preserve the character as linkdead. Creation states delete the character. Linkdead characters have their desc pointer nulled, get saved, and remain in-world. The next pointer cache is updated in next_to_process to prevent iterator invalidation.

### GMCP Negotiation Sequence

On new connection, startGmcp pushes IAC WILL GMCP to the output queue. The client responds with IAC DO GMCP or IAC WILL GMCP. Both responses are accepted and set gmcp to true. The handleTelnetOpts function pragmatically enables gmcp on any IAC command because many clients fail to negotiate properly.

After negotiation, the client sends Core.Hello via IAC SB GMCP Core.Hello JSON IAC SE. The handleCoreHello handler parses the JSON to extract client name and version, storing them in mudclient. Core.Supports.Set follows with capability lists, which are squelched.

Server-initiated GMCP messages use sendGmcp to wrap payloads in IAC SB GMCP ... IAC SE. Prompt data, room information, character stats, and other metadata are sent as JSON objects. The strip parameter controls color code removal from the payload.

Client-initiated commands arrive as IAC SB GMCP command data IAC SE. The subnegotiation extraction in handleTelnetOpts passes the command string to handleGmcpCommand, which dispatches through the commandHandlers map. Unrecognized commands are silently ignored to tolerate client-specific extensions.

### Safe Iteration Implementation

The global next_to_process pointer enables safe descriptor iteration during deletion. Each iteration loop caches the next pointer before any operations:

```cpp
for (d = descriptor_list; d; d = next_to_process) {
    next_to_process = d->next;
```

When a descriptor is deleted, its destructor updates next_to_process to skip itself:

```cpp
while (next_to_process && next_to_process == this)
    next_to_process = next_to_process->next;
```

This ensures the loop continues with a valid pointer even when the current descriptor is deleted. Nested deletions are safe because each deletion advances next_to_process beyond the deleted entry.

Multiple iteration points in handleTimeAndSockets use the same pattern for exception handling, input processing, and output processing. The pattern prevents use-after-free without requiring iterator invalidation detection or deferred deletion queues.

## Troubleshooting

### Descriptor Use-After-Free

Symptoms: Crashes in processAllInput or afterPromptProcessing with stack traces showing deleted descriptor access.

Cause: Failing to cache next_to_process before operations that might delete descriptors. When d is deleted, d->next is invalid but already loaded into the loop variable.

Fix: Ensure every descriptor iteration loop uses the standard pattern. Cache next_to_process immediately after the loop header, before any function calls or operations. Verify that DELETE_THIS checks occur before any further descriptor access.

Diagnostic: Enable ASan and look for heap-use-after-free reports. The allocation backtrace shows Descriptor constructor, the free backtrace shows destructor, and the use backtrace shows the forgotten iteration site.

### Lost GMCP Messages

Symptoms: Clients report missing room updates, prompt data, or other GMCP-delivered information.

Cause: Sending GMCP messages without checking the gmcp flag. Clients that don't support GMCP ignore the messages, but the server may assume delivery. Alternatively, sending GMCP before negotiation completes.

Fix: Guard all sendGmcp calls with if (descriptor->gmcp). Ensure GMCP negotiation completes during connection setup before sending data messages. Use GmcpComm for protocol messages, not UncategorizedComm.

Diagnostic: Enable network logging and verify IAC WILL GMCP / IAC DO GMCP negotiation exchange. Check for GMCP data sent before negotiation completes. Verify client supports GMCP in Core.Hello message.

### Input Not Processing

Symptoms: Client sends commands but nothing happens. No errors, no prompts, character appears frozen.

Cause: Command stuck in input queue with positive wait counter that never decrements. Alternatively, processAllInput iteration bug skipping the descriptor.

Fix: Verify wait counter decrements each tick in processAllInput. Check for early loop continues that skip wait decrement. Ensure descriptor is in descriptor_list and not orphaned.

Diagnostic: Add logging to processAllInput to track wait counter for the affected descriptor. Verify input queue is non-empty. Check if character is in a state that blocks command processing like being incapacitated.

### Socket Write Failure Loop

Symptoms: Descriptor repeatedly deleted and recreated for the same client. Logs show write errors on outputProcessing.

Cause: Client connection is slow or broken, causing socket send buffer to fill. Non-blocking write returns EAGAIN, but code treats this as fatal error and closes connection.

Fix: Current implementation closes on write failure. A more robust approach would buffer output or implement write readiness checking via FD_SET. However, the aggressive disconnect policy prevents slow clients from blocking game ticks.

Diagnostic: Check if write errors correlate with high latency or packet loss. Verify socket is in non-blocking mode. Check if output queue grows unbounded before write failure.

### Telnet Option Negotiation Loops

Symptoms: Endless back-and-forth telnet negotiation messages. Network logs show repeated IAC WILL/WONT exchanges.

Cause: Client and server disagree on option support and keep re-negotiating. Alternatively, client doesn't respect DONT/WONT rejections.

Fix: Verify handleTelnetOpts sends DONT for WILL requests of unsupported options and WONT for DO requests. Ensure negotiation state doesn't re-send offers after rejection. Current implementation is stateless, so it should always respond the same way.

Diagnostic: Packet capture the telnet negotiation. Verify server responds with DONT to unsupported WILL offers. Check if client ignores DONT and re-sends WILL. Some broken clients require blacklisting specific options.

### Command History Corruption

Symptoms: ! command recall returns wrong commands or crashes. History appears to skip entries or duplicate.

Cause: History buffer overrun or incorrect indexing in recall logic. History is stored per-descriptor and cleared on deletion, so stale pointers are unlikely.

Fix: Verify history array bounds checking in inputProcessing. Ensure !! and !number syntax correctly indexes into the circular buffer. Check for off-by-one errors in history wraparound.

Diagnostic: Log history buffer contents and recall requests. Verify circular buffer index calculations. Check if history entries are being overwritten prematurely.

### Linkdead Character Persistence

Symptoms: Linkdead characters disappear from game world instead of persisting. Or linkdead characters remain logged in indefinitely.

Cause: Descriptor destructor incorrectly determines whether character is in-game. The connected >= CON_REDITING || !connected check is order-dependent and fragile.

Fix: Verify connected state before descriptor deletion. Ensure CON_PLYNG is 0 and editor states are above CON_REDITING. Check that character->doQueueSave executes and completes successfully.

Diagnostic: Add logging to descriptor destructor showing connected state and character save result. Verify character record in database has updated link_dead_since timestamp. Check if character is still in character_list after descriptor deletion.

### Snoop Chain Crashes

Symptoms: Crashes when snooping characters disconnect or when snoop chains are deep.

Cause: Stale snoop_by or snooping pointers after descriptor deletion. The destructor clears relationships, but recursive output forwarding may have already dereferenced them.

Fix: Ensure descriptor destructor clears all snoop relationships before deletion. Verify outputProcessing checks snoop_by descriptor validity before forwarding. Limit snoop chain depth to prevent stack overflow.

Diagnostic: Enable ASan and check for use-after-free in SnoopComm output forwarding. Verify destructor null checks for snoop_by->desc before access. Check if snoop relationships are circular.

### Capacity Rejection Not Working

Symptoms: Connections accepted beyond avail_descs limit. Server becomes unresponsive due to too many descriptors.

Cause: maxdesc counter not incremented correctly, or avail_descs check bypassed. File descriptor limit may be higher than avail_descs, allowing kernel to accept more connections.

Fix: Verify newDescriptor checks maxdesc + 1 >= avail_descs before creating descriptor. Ensure maxdesc is updated atomically with descriptor creation. Check ulimit -n file descriptor limit matches avail_descs.

Diagnostic: Log maxdesc value at each connection attempt. Verify rejection message is sent when capacity exceeded. Check if descriptor count matches maxdesc. Use lsof to count actual open file descriptors.
