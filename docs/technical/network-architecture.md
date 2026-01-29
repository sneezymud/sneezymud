---
title: Network and Protocol Architecture
description: SneezyMUD's network layer, including the single-threaded event loop, descriptor lifecycle, connection states, telnet/GMCP protocol handling, and I/O buffering.
keywords: [TMainSocket, TSocket, Descriptor, descriptor_list, next_to_process, gameLoop, inputProcessing, outputProcessing, processAllInput, setPrompts, afterPromptProcessing, handleTelnetOpts, GMCP, telnet, IAC, connectStateT, CON_PLYNG, CON_CREATION_START, safe iteration, DELETE_THIS]
category: Critical Systems

  - spatial-relationships.md
  - player-interface.md
last_updated: 2026-01-29
source_files:
  - code/code/sys/socket.cc
  - code/code/sys/socket.h
  - code/code/sys/connect.cc
  - code/code/sys/connect.h
  - code/code/sys/gmcphandlers.cc
  - code/code/sys/comm.h
  - code/code/sys/DescriptorList.h
related: [memory-safety.md]
---
# Network and Protocol Architecture

This document describes SneezyMUD's network layer, including the single-threaded event loop, descriptor lifecycle, connection states, telnet/GMCP protocol handling, and I/O buffering.

**Misusing these patterns causes disconnects, data loss, or crashes.** Common errors: deleting descriptors during iteration, ignoring the `next_to_process` global, failing to check `DELETE_THIS` returns, processing input on closed sockets.

## Single-Threaded Event Loop

SneezyMUD uses a classic single-threaded, non-blocking I/O model with `select()`. All network operations occur in the main game loop without threading.

### Core Loop Structure

```cpp
// socket.cc - TMainSocket::gameLoop()
int TMainSocket::gameLoop() {
    TScheduler scheduler;
    int pulse = 0;

    // Register all processes (combat, ticks, etc.)
    scheduler.add(new procHandleTimeAndSockets(Pulse::EVERY));
    // ... more processes

    while (!handleShutdown()) {
        scheduler.run(++pulse);
        tics++;
    }
    return Reboot;
}
```

### Per-Tick Processing Order

Each tick (0.1 seconds), `procHandleTimeAndSockets::run()` calls `handleTimeAndSockets()`:

```
handleTimeAndSockets()
    |
    +-> select() on all sockets
    +-> Accept new connections (newDescriptor)
    +-> Close exceptional sockets
    +-> Read input (inputProcessing)
    +-> processAllInput()
    +-> setPrompts()
    +-> afterPromptProcessing()
```

**Source:** `/code/code/sys/socket.cc` lines 363-485

## Core Classes

### TMainSocket

The main listening socket, created once at startup:

```cpp
class TMainSocket {
    int m_mainSockFD;           // IPv6 listening socket
    struct timeval sleeptime;   // Time regulation

    TSocket* newConnection(int);  // Accept new connection
    int newDescriptor(int);       // Create descriptor for connection
    void initSocket(int port);    // Bind and listen
    int gameLoop();               // Main loop
};

extern TMainSocket* gSocket;  // Global instance
```

### TSocket

Per-connection socket wrapper:

```cpp
class TSocket {
    int m_sock;               // Socket file descriptor
    int port;                 // Port number

    int writeToSocket(const char*);  // Non-blocking write
    int writeNull();                 // Write single null byte
    void nonBlock();                 // Set FNDELAY
    void setKeepalive(bool);         // TCP keepalive
};
```

### Descriptor

The central class for each connected client:

```cpp
class Descriptor {
    TSocket* socket;          // Underlying socket
    connectStateT connected;  // Current state (see Connection States)
    TBeing* character;        // Associated character (NULL until login)
    TAccount* account;        // Associated account
    TPerson* original;        // Original character (for polymorph)

    // I/O Buffers
    char m_raw[4096];              // Raw input accumulator
    std::queue<sstring> input;    // Parsed input commands
    std::queue<CommPtr> output;   // Pending output messages

    // State
    int wait;                // Command delay counter
    int prompt_mode;         // Prompt display state
    bool gmcp;               // GMCP enabled
    std::string mudclient;   // Client name (from GMCP)

    Descriptor* next;        // Linked list (descriptor_list)
};
```

**Source:** `/code/code/sys/connect.h` lines 419-596

## Descriptor Lifecycle

### Creation

New connections flow through `newDescriptor()`:

```cpp
int TMainSocket::newDescriptor(int v6_sock) {
    TSocket* s = newConnection(v6_sock);  // accept()
    if (!s) return 0;

    // Capacity check
    if ((maxdesc + 1) >= avail_descs) {
        s->writeToSocket("Sorry.. The game is full...\n\r");
        close(s->m_sock);
        delete s;
        return 0;
    }

    Descriptor* newd = new Descriptor(s);
    newd->startGmcp();  // Send IAC WILL GMCP

    // Get peer address for host field
    newd->host = IP_String(v6_saiSock.sin6_addr);

    // Initial input processing
    if (newd->inputProcessing() < 0) {
        delete newd;
        return 0;
    }
    return 1;
}
```

The constructor adds the descriptor to the global linked list:

```cpp
Descriptor::Descriptor(TSocket* s) : socket(s), connected(CON_CREATION_START),
    next(descriptor_list), /* ... */ {
    descriptor_list = this;  // Prepend to list
}
```

### Destruction

The destructor handles cleanup and safe list removal:

```cpp
Descriptor::~Descriptor() {
    close(socket->m_sock);
    flush();  // Clear I/O queues

    // Handle snoop relationships
    if (snoop.snooping) snoop.snooping->desc->snoop.snoop_by = 0;
    if (snoop.snoop_by) { /* notify and clear */ }

    // Handle character
    if (character) {
        if (original) character->doReturn(...);  // Unpolymorph
        if (connected >= CON_REDITING || !connected) {
            // In-game: mark linkdead
            character->desc = NULL;
            character->doQueueSave();
        } else {
            // Not yet in game: delete character
            delete character;
        }
    }

    // Update next_to_process to avoid crash
    while (next_to_process && next_to_process == this)
        next_to_process = next_to_process->next;

    // Remove from linked list
    if (this == descriptor_list)
        descriptor_list = descriptor_list->next;
    else {
        for (tmp = descriptor_list; tmp->next != this; tmp = tmp->next);
        tmp->next = next;
    }

    delete socket;
    delete account;
}
```

**CRITICAL:** The `next_to_process` update prevents use-after-free when a descriptor is deleted during iteration.

**Source:** `/code/code/sys/connect.cc` lines 477-662

## Connection States

The `connectStateT` enum defines 45+ states:

### Account/Login States

| State | Value | Description |
|-------|-------|-------------|
| `CON_NME` | 1 | Getting name |
| `CON_NMECNF` | 2 | Confirming name |
| `CON_PWDNRM` | 3 | Getting password |
| `CON_PWDCNF` | 4 | Confirming password |
| `CON_NEWACT` | 10 | Creating new account |
| `CON_ACTPWD` | 11 | Account password |
| `CON_NEWLOG` | 12 | New account login |
| `CON_EMAIL` | 14 | Getting email |
| `CON_TERM` | 15 | Terminal type |

### Character Creation States (26 states)

States `CON_CREATION_START` (84) through `CON_CREATION_MAX` (108):

| State | Description |
|-------|-------------|
| `CON_CREATION_NAME` | Entering character name |
| `CON_CREATION_DISCLAIM1-3` | Disclaimer screens |
| `CON_CREATION_SEX` | Choosing sex |
| `CON_CREATION_RACE` | Choosing race |
| `CON_CREATION_CLASS` | Choosing class |
| `CON_CREATION_TRAITS1-3` | Choosing traits |
| `CON_CREATION_CUSTOMIZE_*` | Stat customization |
| `CON_CREATION_DONE` | Finalization |

### Playing States

| State | Value | Description |
|-------|-------|-------------|
| `CON_PLYNG` | 0 | Actively playing |
| `CON_RMOTD` | 5 | Reading MOTD |
| `CON_CONN` | 16 | Connecting character |

### Editor States (above MAX_CON_STATUS)

| State | Description |
|-------|-------------|
| `CON_REDITING` | Room editor |
| `CON_OEDITING` | Object editor |
| `CON_MEDITING` | Mobile editor |
| `CON_HELP` | Help editor |
| `CON_WRITING` | String editor |

### State Machine Flow

```
New Connection
    |
    v
CON_CREATION_START --> sendLogin() --> Account menu
    |
    +-> CON_ACTPWD (existing account)
    +-> CON_NEWLOG (new account)
    |
    v
Account Menu --> doAccountMenu()
    |
    +-> CON_CONN (select character)
    +-> CON_CREATION_NAME (create character)
    |
    v
CON_CREATION_* --> creation_nanny() --> CON_CREATION_DONE
    |
    v
CON_PLYNG (playing)
    |
    +-> CON_REDITING, etc. (editors)
```

**Source:** `/code/code/sys/connect.h` lines 54-118

## Input Processing

### Raw Input to Command Queue

`inputProcessing()` reads from socket and parses into commands:

```cpp
int Descriptor::inputProcessing() {
    // Read into m_raw buffer (accumulates partial data)
    do {
        thisround = read(socket->m_sock, m_raw + bgin + sofar, 4096 - bgin - sofar - 1);
        if (thisround > 0) sofar += thisround;
        else if (thisround < 0) {
            if (errno != EWOULDBLOCK) return -1;  // Error
            else break;  // No more data
        } else {
            // EOF - connection closed
            return -1;
        }
    } while (!ISNEWL(*(m_raw + bgin + sofar - 1)));

    // Handle telnet options (strips IAC sequences)
    sstring reply = handleTelnetOpts(in, this);
    if (!reply.empty())
        output.push(CommPtr(new UncategorizedComm(reply)));

    // Parse into commands (newline-delimited)
    for (i = 0, k = 0; *(m_raw + i);) {
        if (!ISNEWL(*(m_raw + i)) && k < MAX_INPUT_LENGTH - 2) {
            // Handle backspace, double '$' for printf safety
            if (*(m_raw + i) == '$')
                *(tmp + ++k) = '$';  // Double it
            // ...
        } else {
            // End of line - push to input queue
            input.push(sstring(tmp));
            // Handle command history (! expansion)
            // ...
        }
    }
    return TRUE;
}
```

**Key behaviors:**
- Partial reads accumulate in `m_raw` until newline
- `$` characters are doubled to prevent printf issues (undone in `outputProcessing`)
- Command history supports `!`, `!!`, `!n` syntax
- Lines over `MAX_INPUT_LENGTH` are truncated

### Command Dispatch

`processAllInput()` processes queued commands:

```cpp
void processAllInput() {
    for (d = descriptor_list; d; d = next_to_process) {
        next_to_process = d->next;  // Cache before potential deletion

        if ((--(d->wait) <= 0) && !d->input.empty()) {
            strncpy(comm, d->input.front().c_str(), sizeof(comm) - 1);
            d->input.pop();

            // Handle return from void room
            if (d->character && d->character->specials.was_in_room != Room::NOWHERE) {
                // Move back to previous room
            }

            d->wait = 1;  // Reset wait counter

            // Dispatch based on state
            if (is_client_sstring(comm))
                rc = d->read_client(comm);
            else if (d->str)
                d->sstring_add(comm);        // String editor
            else if (d->pagedfile)
                d->page_file(comm);          // Pager
            else if (!d->account)
                rc = d->sendLogin(comm);     // Login flow
            else if (!d->account->status)
                rc = d->doAccountStuff(comm); // Account menu
            else if (!d->connected)
                rc = d->character->parseCommand(comm, TRUE);  // Game commands
            else if (d->connected == CON_REDITING)
                room_edit(d->character, comm);
            // ... other editors
            else
                rc = d->nanny(comm);         // Connection state machine

            // Handle DELETE_THIS
            if (IS_SET_DELETE(rc, DELETE_THIS)) {
                delete d;
                d = NULL;
                continue;
            }
        }
    }
}
```

**CRITICAL:** Always cache `next_to_process` before processing. Deletion or state changes may invalidate the current descriptor.

**Source:** `/code/code/sys/connect.cc` lines 2723-2867, 3839-3966

## Output Processing

### Output Buffer

Output uses a polymorphic `Comm` class hierarchy with shared pointers:

```cpp
class Comm {
    sstring text;
    virtual sstring getText() = 0;
public:
    sstring getComm();
};

typedef boost::shared_ptr<Comm> CommPtr;
std::queue<CommPtr> output;
```

### Comm Subclasses

| Class | Purpose |
|-------|---------|
| `UncategorizedComm` | Generic text output |
| `GmcpComm` | GMCP protocol messages |
| `LoginComm` | Login prompts |
| `PromptComm` | Player prompts with metadata |
| `SnoopComm` | Snoop output |
| `TellFromComm`/`TellToComm` | Tell messages |
| `RoomExitComm` | Exit data for clients |
| `SoundComm` | Sound effects |
| `WhoListComm` | Who list entries |

### Output Flow

```cpp
int Descriptor::outputProcessing() {
    TBeing* ch = original ? original : character;

    while (!output.empty()) {
        CommPtr c(output.front());
        output.pop();

        strncpy(i, c->getComm().c_str(), sizeof(i) - 1);

        // Undo the '$' doubling from input processing
        char* tb = i;
        while ((tc = strstr(tb, "$$"))) {
            // Replace "$$" with "$"
            memmove(tc, tc + 1, strlen(tc));
            tb = tc + 1;
        }

        // Forward to snooper
        if (snoop.snoop_by && snoop.snoop_by->desc) {
            snoop.snoop_by->desc->output.push(
                CommPtr(new SnoopComm(ch->getName(), i)));
        }

        // Apply color codes
        sstring colorBuf = colorString(ch, this, i, NULL, COLOR_BASIC, FALSE);

        // Write to socket
        if (socket->writeToSocket(colorBuf.c_str()))
            return -1;
    }
    return 1;
}
```

**Source:** `/code/code/sys/connect.cc` lines 400-475

## Prompt Handling

### setPrompts()

Called after input processing to generate prompts:

```cpp
void setPrompts(fd_set out) {
    for (d = descriptor_list; d; d = nextd) {
        nextd = d->next;

        if ((FD_ISSET(d->socket->m_sock, &out) && hasStuffToSend(*d)) ||
            d->prompt_mode) {

            // Task-specific prompts (meditation, sharpening, etc.)
            if (ch && ch->task) {
                // Generate task-specific prompt
            }

            // String editor prompt
            if (d->str && d->prompt_mode != DONT_SEND) {
                d->output.push(CommPtr(new UncategorizedComm("-> ")));
            }

            // Pager prompt
            else if (d->pagedfile && d->prompt_mode != DONT_SEND) {
                // Page navigation prompt
            }

            // Normal game prompt
            else if (!d->connected) {
                // Build prompt with HP/mana/move/etc.
                // Send via PromptComm and RoomExitComm
            }
        }
    }
}
```

### afterPromptProcessing()

Final output flush:

```cpp
void afterPromptProcessing(fd_set out) {
    for (d = descriptor_list; d; d = next_d) {
        next_d = d->next;
        if (FD_ISSET(d->socket->m_sock, &out) && !(d->output.empty())) {
            if (d->outputProcessing() < 0) {
                delete d;
                d = NULL;
            } else
                d->prompt_mode = 0;  // Reset for next cycle
        }
    }
}
```

**Source:** `/code/code/sys/connect.cc` lines 2326-2688

## Telnet Protocol

### IAC Command Handling

`handleTelnetOpts()` processes telnet escape sequences:

```cpp
sstring handleTelnetOpts(sstring& s, Descriptor* d) {
    size_t iac_pos = s.find(iac);  // 0xFF
    if (iac_pos == sstring::npos) return "";

    // Any telnet command enables GMCP (pragmatic hack)
    d->gmcp = true;

    unsigned char cmd = s[iac_pos + 1];

    if (cmd == will || cmd == do_ || cmd == wont || cmd == dont) {
        unsigned char arg = s[iac_pos + 2];

        if (cmd == will && arg == GMCP) {
            d->gmcp = true;
            result = "\xff\xfc\xc9";  // IAC DO GMCP
        } else if (cmd == do_ && arg == GMCP) {
            d->gmcp = true;
            result = "\xff\xfb\xc9";  // IAC WILL GMCP
        } else if (cmd == will) {
            // Unsupported - send DONT
            result = format("\xff\xfe%c") % arg;
        } else if (cmd == do_) {
            // Unsupported - send WONT
            result = format("\xff\xfd%c") % arg;
        }

        s.erase(iac_pos, 3);  // Remove from input
        return result + handleTelnetOpts(s, d);  // Recurse
    }
    else if (cmd == sb) {
        // Subnegotiation: IAC SB <opt> <data> IAC SE
        size_t end = s.find("\xff\xf0");  // IAC SE
        if (end == sstring::npos) return "";  // Truncated

        unsigned char arg = s[iac_pos + 2];
        sstring client_cmd = s.substr(iac_pos + 3, end - iac_pos - 3);
        s.erase(iac_pos, end + 2 - iac_pos);

        if (arg == GMCP) {
            handleGmcpCommand(client_cmd, d);
        }
        return handleTelnetOpts(s, d);
    }
    return "";
}
```

### Telnet Constants

```cpp
unsigned char GMCP = 201;   // 0xC9
unsigned char iac = 255;    // 0xFF - Interpret As Command
unsigned char dont = 254;   // 0xFE
unsigned char do_ = 253;    // 0xFD
unsigned char wont = 252;   // 0xFC
unsigned char will = 251;   // 0xFB
unsigned char sb = 250;     // 0xFA - Subnegotiation Begin
unsigned char se = 240;     // 0xF0 - Subnegotiation End
```

**Source:** `/code/code/sys/gmcphandlers.cc`

## GMCP Protocol

### GMCP Initialization

On new connection, server sends `IAC WILL GMCP`:

```cpp
void Descriptor::startGmcp() {
    sstring text = sstring("\xff\xfb\xc9");  // IAC WILL GMCP
    output.push(CommPtr(new GmcpComm(text)));
}
```

### Sending GMCP Messages

```cpp
void Descriptor::sendGmcp(const sstring& msg, bool strip) {
    if (msg.empty() || !gmcp) return;

    // IAC SB GMCP <message> IAC SE
    sstring text = sstring("\xff\xfa\xc9") +
                   (strip ? stripColorCodes(msg) : msg) +
                   sstring("\xff\xf0");
    output.push(CommPtr(new GmcpComm(text)));
}
```

### GMCP Command Handlers

| Command | Handler |
|---------|---------|
| `Core.Hello` | Stores client name/version |
| `Core.Supports.Set` | Ignored (squelched) |
| `External.Discord.Hello` | Returns Discord invite |
| `request sectors` | Returns sector type list |
| `request area` | Returns current zone info |
| `remember` | Player memory system |
| `retrieve` | Player memory retrieval |

```cpp
std::map<std::string, std::function<void(std::string, Descriptor&)>>
    commandHandlers = {
        {"Core.Hello", handleCoreHello},
        {"Core.Supports.Set", [](auto, auto&) {}},
        {"External.Discord.Hello", handleDiscord},
        {"remember", handleRemember},
        {"retrieve", handleRetrieve},
    };
```

### GMCP Quirks

**Auto-enable:** Any telnet IAC command enables GMCP. This is a pragmatic hack because many clients don't negotiate properly:

```cpp
// In handleTelnetOpts()
d->gmcp = true;  // Enabled on ANY telnet command
```

**Source:** `/code/code/sys/gmcphandlers.cc`, `/code/code/sys/connect.cc` lines 2709-2721

## Socket Configuration

### Non-blocking Mode

All client sockets use non-blocking I/O:

```cpp
void TSocket::nonBlock() {
    if (fcntl(m_sock, F_SETFL, FNDELAY) == -1) {
        perror("Noblock");
        exit(1);
    }
}
```

### TCP Keepalive

Connections use aggressive keepalive settings:

```cpp
void TSocket::setKeepalive(bool enabled) {
    set(SOL_SOCKET, SO_KEEPALIVE, 1);
    set(SOL_TCP, TCP_KEEPIDLE, 180);   // 3 minutes idle
    set(SOL_TCP, TCP_KEEPINTVL, 180);  // 3 minute probes
}
```

### Socket Options

Main socket configuration:

```cpp
void TMainSocket::initSocket(int t_port) {
    // IPv6 socket (also accepts IPv4)
    m_mainSockFD = socket(PF_INET6, SOCK_STREAM, IPPROTO_TCP);

    // Allow port reuse
    setsockopt(m_mainSockFD, SOL_SOCKET, SO_REUSEADDR, &reuseaddr, ...);

    // Linger on close
    ld.l_linger = 1000;
    ld.l_onoff = 0;
    setsockopt(m_mainSockFD, SOL_SOCKET, SO_LINGER, &ld, ...);

    bind(m_mainSockFD, ...);
    listen(m_mainSockFD, 10);  // Backlog of 10
}
```

## Disconnect Handling

### Exceptional Conditions

Sockets with exceptional conditions are closed immediately:

```cpp
for (point = descriptor_list; point; point = next_to_process) {
    next_to_process = point->next;
    if (FD_ISSET(point->socket->m_sock, &exc_set)) {
        FD_CLR(point->socket->m_sock, &input_set);
        FD_CLR(point->socket->m_sock, &output_set);
        delete point;
    }
}
```

### Read Errors

Input processing returns -1 on errors, triggering deletion:

```cpp
if (point->inputProcessing() < 0) {
    delete point;
    point = NULL;
}
```

### EOF Detection

```cpp
if (thisround == 0) {
    vlogf(LOG_PIO, format("EOF on socket read from %s") % host);
    return -1;
}
```

### Write Errors

```cpp
if (socket->writeToSocket(colorBuf.c_str()))
    return -1;  // Triggers descriptor deletion
```

### Linkdead Handling

When a descriptor is destroyed while a character is in-game:

1. Character remains in-world but marked linkdead
2. Followers are removed (prevents item duplication)
3. Character is saved to disk
4. Invisibility level set to GOD_LEVEL1
5. Mount is dismounted
6. Master/follower relationships broken (unless charmed)

## Global Variables

| Variable | Type | Purpose |
|----------|------|---------|
| `descriptor_list` | `Descriptor*` | Head of linked list |
| `next_to_process` | `Descriptor*` | Safe iteration pointer |
| `maxdesc` | `int` | Highest fd in use |
| `avail_descs` | `int` | Max allowed descriptors (150) |
| `gSocket` | `TMainSocket*` | Global main socket |

## Safe Iteration Pattern

**Always** use this pattern when iterating descriptors:

```cpp
Descriptor* d;
for (d = descriptor_list; d; d = next_to_process) {
    next_to_process = d->next;  // Cache BEFORE any operations

    // Operations that might delete d or other descriptors
    int rc = d->someOperation();
    if (IS_SET_DELETE(rc, DELETE_THIS)) {
        delete d;
        d = NULL;
        continue;
    }
}
```

**CRASH:** Failing to cache `next_to_process` before deletion causes use-after-free.

## Dangerous Anti-Patterns

```cpp
// CRASH: Not caching next pointer
for (d = descriptor_list; d; d = d->next) {
    delete d;  // d->next is now invalid!
}

// CRASH: Ignoring DELETE_THIS
int rc = d->nanny(arg);
d->writeToQ("Hello");  // d may be deleted!

// BUG: Writing to closed socket
if (d->outputProcessing() < 0) {
    d->writeToQ("Goodbye");  // Socket already broken
    delete d;
}

// BUG: Not checking gmcp flag
d->sendGmcp(msg, true);  // Should check d->gmcp first
```

## Key Files

| File | Purpose |
|------|---------|
| `/code/code/sys/socket.cc` | Main loop, TSocket, TMainSocket |
| `/code/code/sys/socket.h` | Socket class declarations |
| `/code/code/sys/connect.cc` | Descriptor, I/O processing, nanny |
| `/code/code/sys/connect.h` | Descriptor class, connection states |
| `/code/code/sys/gmcphandlers.cc` | Telnet/GMCP handling |
| `/code/code/sys/comm.h` | Comm class hierarchy |
| `/code/code/sys/DescriptorList.h` | Iterator wrapper |
