/**
 * Immortal `wipe` command: player character deletion.
 *
 * Verifies the two-step lookup+delete (SELECT id, DELETE WHERE id) and
 * CASCADE FK cleanup when an immortal wipes a player character.
 *
 * Required immortal powers: wipe.
 * Uses an ephemeral account so the wiped character is isolated.
 */

import { afterAll, beforeAll, describe, expect, it } from "bun:test";

import {
  ACCOUNT_TEST_TIMEOUT,
  createAccount,
  DEFAULT_PASSWORD,
  deleteAccount,
  uniqueIdentity,
} from "../harness/accounts.ts";
import { MudClient } from "../harness/client.ts";
import { loadConfig } from "../harness/config.ts";
import { dbQuery } from "../harness/db.ts";

const config = loadConfig();

describe("Immortal Wipe", () => {
  let imm: MudClient;

  let account: string;
  let character: string;

  beforeAll(async () => {
    imm = await MudClient.connect(config);
    await imm.login(config);

    const identity = uniqueIdentity();
    account = identity.account;
    character = identity.character;

    await createAccount({
      account,
      character,
      connection: config,
      password: DEFAULT_PASSWORD,
    });
  }, ACCOUNT_TEST_TIMEOUT);

  afterAll(async () => {
    await imm.close();
    try {
      await deleteAccount({
        account,
        connection: config,
        password: DEFAULT_PASSWORD,
      });
    } catch {
      // Account may already be gone if wipe cleaned it up
    }
  });

  it(
    "wipes an offline player and removes their database record",
    async () => {
      // Verify the player exists before wipe
      const beforeId = await dbQuery({
        sql: `SELECT id FROM player WHERE name='${character}'`,
      });
      expect(beforeId).not.toBe("");

      // Wipe the character (they're offline, rented at the inn)
      const output = await imm.command(`wipe ${character} ole'chicken`);
      expect(output).toContainCaseInsensitive("Player not online");
      expect(output).toContainCaseInsensitive("Removing:");

      // Verify the player record is gone from the database
      const afterId = await dbQuery({
        sql: `SELECT id FROM player WHERE name='${character}'`,
      });
      expect(afterId).toBe("");
    },
    ACCOUNT_TEST_TIMEOUT,
  );
});
