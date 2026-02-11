# Functional Integration Tests

These tests connect to a running SneezyMUD server over TCP and verify behavior through the same interface a player uses - logging in, running commands, and checking what comes back.

## Getting Started

If you've never run the functional tests before, follow these steps end to end.

**1. Get a running server and database.** These tests drive a live MUD server over TCP - they don't start or manage the server process. Follow the [Development Setup](../../README.md#development-setup) instructions in the project README to build from source and seed the database, then start the server (`code/sneezy`). Alternatively, use the [sneezymud-docker](https://github.com/sneezymud/sneezymud-docker) setup.

**2. Install Bun.** The test runner requires the [Bun](https://bun.sh/) JavaScript runtime. Install it if you haven't already.

**3. Install dependencies.**

```bash
cd tests/functional
bun install
```

**4. Configure credentials.**

```bash
cp .env.example .env
```

Edit `.env` and fill in the values. `MUD_HOST` and `MUD_PORT` default to `localhost:7900`.

`MUD_ACCOUNT`, `MUD_PASSWORD`, and `MUD_CHARACTER` identify the account used by tests that log in. What kind of account you need depends on which tests you're running - each test file documents its requirements in a comment at the top. Account lifecycle tests (create/delete) only need host/port.

On a fresh database, the first character created is auto-promoted to a level 60 immortal with all wizpowers, which satisfies every test. The bootstrap script handles this using your `.env` credentials:

```bash
bun run harness/bootstrap-admin.ts
```

On an existing database, create an account manually and ensure the character has whatever wizpowers are required by the tests you're running.

**5. Verify your setup.** Run the smoke tests to confirm the harness can connect and interact with your server:

```bash
bun test tests/smoke.test.ts
```

From here, run whichever tests are relevant to your work. These tests are for verifying specific features, not as a regression suite that runs on every change. The full suite is slow - tests run serially against a live server over TCP - so you typically run only the tests relevant to what you're working on. See [Running](#running) for more options, or [Troubleshooting](#troubleshooting) if something isn't working.

## Running

```bash
bun test                          # all tests
bun test tests/smoke.test.ts      # specific file
bun test -t "who shows"           # by name pattern
bun run check                     # typecheck + lint
```

## Writing Tests

All tests are plain TypeScript using `bun:test` (`describe`/`it`/`expect`). Test logic and test data live together in each `.test.ts` file, so the type system covers every test definition and IDE autocomplete works everywhere.

### Adding a new in-game command test

1. Create `tests/<feature>.test.ts`.
2. Use `MudClient.connect()` + `client.login()` in a `beforeAll` hook.
3. Call `client.close()` in `afterAll`.
4. Use `client.command()` for standard commands, `client.pagedCommand()` for output that triggers the MUD pager.
5. Assert with custom matchers (see `harness/matchers.ts`). All are case-insensitive and support `.not` for negation.

```typescript
import { afterAll, beforeAll, describe, expect, it } from "bun:test";
import { MudClient } from "../harness/client.ts";
import { loadConfig } from "../harness/config.ts";

const config = loadConfig();

describe("Feature Name", () => {
  let mud: MudClient;

  beforeAll(async () => {
    mud = await MudClient.connect(config);
    await mud.login(config);
  }, 30_000);

  afterAll(async () => {
    await mud.close();
  });

  it("does the thing", async () => {
    const output = await mud.command("the command");
    expect(output).toContainCaseInsensitive("expected output");
  });
});
```

### Account lifecycle tests

Use `withEphemeralAccount()` from `harness/accounts.ts` for tests that need a dedicated account. It creates an account with a random name, runs your callback, then deletes the account on exit (best-effort). Use `loadConnectionConfig()` instead of `loadConfig()` since these tests don't need shared login credentials.

```typescript
import { describe, expect, it } from "bun:test";

import {
  ACCOUNT_TEST_TIMEOUT,
  DEFAULT_PASSWORD,
  loginAndRent,
  withEphemeralAccount,
} from "../harness/accounts.ts";
import { MudClient } from "../harness/client.ts";
import { loadConnectionConfig } from "../harness/config.ts";

const config = loadConnectionConfig();

describe("My Account Feature", () => {
  it(
    "does something with a fresh account",
    async () => {
      await withEphemeralAccount({
        connection: config,
        fn: async (account, character) => {
          // account and character are random unique names.
          // The account already has one character created and rented.
          await loginAndRent({
            account,
            character,
            connection: config,
            password: DEFAULT_PASSWORD,
          });
        },
      });
    },
    ACCOUNT_TEST_TIMEOUT,
  );
});
```

Account tests need the `ACCOUNT_TEST_TIMEOUT` (120s) since each menu step requires a separate TCP connection with ~1s delay.

### Configuring test characters

Use `setupCharacter()` from `harness/setup-character.ts` to configure a test character via immortal `@set` commands. The immortal connection (your `.env` account) must already be logged in. Currently supports setting level and discipline values:

```typescript
import { setupCharacter } from "../harness/setup-character.ts";

// In beforeAll, after logging in:
await setupCharacter({
  imm: mud, // logged-in immortal MudClient
  setup: {
    name: "Targetchar",
    level: 50,
    disciplines: [{ index: 0, value: 100 }],
  },
});
```

### Conventions

- Keep tests self-contained per file. Do not rely on cross-file execution order.
- Test files run serially (`maxConcurrency = 1` in bunfig.toml) since all tests share a single MUD server.
- Within a file, `describe` blocks and `it` cases run in declaration order. Module-level hooks (`beforeAll`/`afterAll` outside any `describe`) apply to all describes in the file; describe-level hooks scope to that block.
- Include explicit timeout parameters on `beforeAll` hooks that perform login (30s) or account operations (120s).
- Required immortal powers should be noted in a comment at the top of the file.

## Troubleshooting

**Connection refused / timeout in `beforeAll`** - The MUD server isn't running, or `MUD_HOST`/`MUD_PORT` in `.env` don't match. Verify with `telnet localhost 7900`.

**"Missing required env var: MUD_ACCOUNT"** - You're running a test that requires login credentials but `.env` only has host/port. Add `MUD_ACCOUNT`, `MUD_PASSWORD`, and `MUD_CHARACTER` pointing to a valid account.

**"That character is already connected"** - Another test run or telnet session is logged in as the same character. Disconnect the other session, or use different test accounts.

**Test hangs then times out** - Usually means the server sent unexpected output and `readUntilPrompt` never saw a prompt. Check that the test account's character is in a valid state (alive, not linkdead, not in a broken room). You can also telnet in manually and run the same command to see what the server actually sends.

**Assertion failure with "Actual output: ..."** - The custom matchers print a truncated snippet of what the server actually returned. Read this carefully - it usually makes the problem obvious (different formatting, missing data, command error message instead of expected output).

**`bun run check` reports type errors** - Run this before committing test code. It catches forgotten `await` calls (which silently break test sequencing), wrong argument types, and other issues that the test runner won't catch at runtime.
