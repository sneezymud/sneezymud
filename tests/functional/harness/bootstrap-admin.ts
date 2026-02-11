/**
 * Creates the initial admin account on a fresh database.
 *
 * On a freshly-seeded SneezyMUD database the first character created is
 * auto-promoted to level 60 immortal, which several functional tests require.
 * Run this once before the test suite.
 *
 * The immortal starts in Imperia (room 1) instead of the newbie room, so this
 * uses IMMORTAL_CREATION_WIZARD (which uses "goto" to reach the inn) rather
 * than the standard createAccount flow.
 *
 * Reads credentials from MUD_ACCOUNT / MUD_CHARACTER / MUD_PASSWORD env vars
 * (same as the test suite) so CI and local .env stay in sync automatically.
 */

import { IMMORTAL_CREATION_WIZARD } from "../harness/accounts.ts";
import { MudClient } from "../harness/client.ts";
import { loadConfig } from "../harness/config.ts";

const { account, character, password, ...connection } = loadConfig();

await using client = await MudClient.connect(connection);

for (const { cmd: text, delay, expect } of [
  { cmd: "NEW" },
  { cmd: account },
  { cmd: password },
  { cmd: password },
  { cmd: "a" }, // account menu: add new character
  { cmd: character },
  ...IMMORTAL_CREATION_WIZARD,
] as Array<{ cmd: string; delay?: number; expect?: string }>) {
  const output = await client.rawSend({ delay, text });
  if (expect && !output.toLowerCase().includes(expect.toLowerCase())) {
    throw new Error(
      `Expected "${expect}" in output of "${text}", got: ${output.slice(0, 200)}`,
    );
  }
}
