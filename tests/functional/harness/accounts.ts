/**
 * Ephemeral account lifecycle helpers for integration tests.
 *
 * All flows drive SneezyMUD's pre-login account menu via raw TCP commands,
 * which means each operation needs its own connection (the menu doesn't
 * support multiplexing).
 */

import type { ConnectionConfig } from "./config.ts";

import { LOGIN_DELAY, MudClient } from "./client.ts";

export const DEFAULT_PASSWORD = "7estPw";

/**
 * Generous timeout for account lifecycle tests. These involve 25-30+
 * sequential network round-trips at ~1s each (create + action + delete).
 */
export const ACCOUNT_TEST_TIMEOUT = 120_000;

// Character creation wizard steps, split into phases so callers can inject
// different post-creation commands. Mortals start in the newbie room (584)
// and go "d" to the inn to rent; immortals (e.g. the bootstrap-admin first
// character) start in Imperia (room 1) and just quit.
//
// CREATION_STEPS: disclaimers through entering the game world.
// The default inputs happen to advance through each server state correctly:
//   3x space  → dismiss disclaimers 1-3
//   "y"       → accept multiplay warning
//   space     → trigger reset (applies defaults)
//   "d"       → "Done" on the launchpad (finalize character with defaults)
//   space     → dismiss congratulations screen
//   space     → CON_RMOTD - loads character into the game world
//
const CREATION_STEPS = [
  { cmd: " " }, // dismiss disclaimer 1
  { cmd: " " }, // dismiss disclaimer 2
  { cmd: " " }, // dismiss disclaimer 3
  { cmd: "y" }, // accept multiplay warning
  { cmd: " " }, // reset (apply defaults)
  { cmd: "d" }, // launchpad "Done" (finalize character with defaults)
  { cmd: " " }, // dismiss congratulations
  { cmd: " " }, // enter game (CON_RMOTD)
] as const;

// RENT_AND_EXIT: rent at the inn and return to the account menu.
const RENT_AND_EXIT = [
  { cmd: "rent", expect: "Have a nice stay!" },
  { cmd: "e" }, // exit to menu
  // Dismiss the menu prompt; extra delay for the server to finish session teardown.
  { cmd: " ", delay: LOGIN_DELAY * 2 },
] as const;

// Mortal characters start in room 584 (newbie alcove); "d" goes to the inn.
const MORTAL_CREATION_WIZARD = [
  ...CREATION_STEPS,
  { cmd: "d" }, // go down to The Roaring Lion Inn (room 557)
  ...RENT_AND_EXIT,
] as const;

// The first character on a fresh database is auto-promoted to immortal.
// Immortals don't rent - they just quit.
export const IMMORTAL_CREATION_WIZARD = [
  ...CREATION_STEPS,
  { cmd: "quit!" }, // force-quit (no confirmation prompt)
  { cmd: "e" }, // exit account menu
  { cmd: " ", delay: LOGIN_DELAY * 2 },
] as const;

/**
 * Attempt an account login (without selecting a character).
 * Returns the output after entering the password.
 */
export async function attemptLogin({
  account,
  connection,
  password,
}: {
  account: string;
  connection: ConnectionConfig;
  password: string;
}): Promise<string> {
  await using client = await MudClient.connect(connection);
  await client.rawSend({ text: account });
  return await client.rawSend({ text: password });
}

/**
 * Create a new account + character via the pre-login menu flow.
 *
 * NEW → account name → password (x2) → account menu "a" → character name →
 * disclaimers (x3) → multiplay warning → reset → launchpad "Done" →
 * congratulations → enter game → go down to inn → rent → exit menu.
 */
export async function createAccount({
  account,
  character,
  connection,
  password,
}: {
  account: string;
  character: string;
  connection: ConnectionConfig;
  password: string;
}): Promise<void> {
  await using client = await MudClient.connect(connection);
  await rawSequence({
    client,
    commands: [
      { cmd: "NEW" },
      { cmd: account },
      { cmd: password },
      { cmd: password },
      { cmd: "a" }, // account menu: add new character
      { cmd: character },
      ...MORTAL_CREATION_WIZARD,
    ],
  });
}

/**
 * Add a second character to an existing account via the account menu.
 *
 * The flow: login → account menu → "a" (add character) → character name →
 * disclaimers (x3) → multiplay warning → reset → launchpad "Done" →
 * congratulations → enter game → go down to inn → rent → exit menu.
 */
export async function addCharacterToAccount({
  account,
  character,
  connection,
  password,
}: {
  account: string;
  character: string;
  connection: ConnectionConfig;
  password: string;
}): Promise<void> {
  await using client = await MudClient.connect(connection);
  await rawSequence({
    client,
    commands: [
      { cmd: account },
      { cmd: password },
      { cmd: "a" }, // add new character
      { cmd: character },
      ...MORTAL_CREATION_WIZARD,
    ],
  });
}

export async function deleteAccount({
  account,
  connection,
  password,
}: {
  account: string;
  connection: ConnectionConfig;
  password: string;
}): Promise<void> {
  await using client = await MudClient.connect(connection);
  await rawSequence({
    client,
    commands: [
      { cmd: account },
      { cmd: password },
      { cmd: "d" }, // choose delete
      { cmd: "1" }, // delete entire account (not single character)
      { cmd: password, expect: "Your account has been deleted" },
    ],
  });
}

/** Verifies a character can enter the game. Exits cleanly via rent + menu. */
export async function loginAndRent({
  account,
  character,
  connection,
  password,
}: {
  account: string;
  character: string;
  connection: ConnectionConfig;
  password: string;
}): Promise<void> {
  await using client = await MudClient.connect(connection);
  await rawSequence({
    client,
    commands: [
      { cmd: account },
      { cmd: password },
      { cmd: "c" }, // select character
      { cmd: character },
      { cmd: " " }, // enter game (CON_RMOTD)
      { cmd: "rent", expect: "stores your stuff in the safe" },
      { cmd: "e" }, // exit to menu
    ],
  });
}

// Account names are lowercase; character names must be capitalized. The "t"/"T"
// prefix avoids collisions with real accounts. 5 random alpha chars gives enough
// entropy for parallel test runs.
export function uniqueIdentity(): { account: string; character: string } {
  const chars = "abcdefghijklmnopqrstuvwxyz";
  let suffix = "";
  for (let i = 0; i < 5; i++) {
    suffix += chars.charAt(Math.floor(Math.random() * chars.length));
  }
  return { account: `t${suffix}`, character: `T${suffix}` };
}

/** Create an ephemeral account, run the callback, then best-effort delete it. */
export async function withEphemeralAccount({
  connection,
  fn,
}: {
  connection: ConnectionConfig;
  fn: (account: string, character: string) => Promise<void>;
}): Promise<void> {
  const { account, character } = uniqueIdentity();
  const password = DEFAULT_PASSWORD;
  try {
    await createAccount({ account, character, connection, password });
    await fn(account, character);
  } finally {
    try {
      await deleteAccount({ account, connection, password });
    } catch {
      console.warn(
        `Best-effort cleanup of ephemeral account "${account}" failed`,
      );
    }
  }
}

async function rawSequence({
  client,
  commands,
}: {
  client: MudClient;
  commands: Array<{ cmd: string; delay?: number; expect?: string }>;
}): Promise<void> {
  for (const { cmd: text, delay, expect } of commands) {
    const output = await client.rawSend({ delay, text });
    if (expect && !output.toLowerCase().includes(expect.toLowerCase())) {
      throw new Error(
        `Expected "${expect}" in output of "${text}", got: ${output.slice(0, 200)}`,
      );
    }
  }
}
