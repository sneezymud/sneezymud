/**
 * Tell history persistence and retrieval.
 *
 * Verifies that tells are stored in the database and can be retrieved via
 * the `history` command. Tests the tellhistory table's player_id columns
 * and the 25-row-per-recipient cap.
 *
 * Tell history is loaded from the database at login time, not updated
 * in real-time during a session. So the mortal must re-login after
 * receiving tells to see them in `history` output.
 *
 * Required immortal powers: none beyond being immortal (for the tell command).
 * Uses an ephemeral account so tell history starts empty.
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

const config = loadConfig();

describe("Tell History", () => {
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
      console.warn(
        `Best-effort cleanup of ephemeral account "${account}" failed`,
      );
    }
  });

  it(
    "persists tells and retrieves them via history",
    async () => {
      // Phase 1: mortal logs in and receives a tell
      let mortal = await MudClient.connect(config);
      await mortal.login({
        account,
        character,
        password: DEFAULT_PASSWORD,
      });

      const marker = `histtest${Date.now()}`;
      await imm.command(`tell ${character} ${marker}`);

      // Drain the real-time tell notification
      await mortal.readUntilIdle(1);

      // Log out: mortal starts at the inn from character creation.
      // Wait for rent output before closing to avoid broken pipe on server.
      await mortal.send("rent");
      await mortal.readUntilIdle(1);
      await mortal.close();

      // Phase 2: re-login to load tell history from database
      mortal = await MudClient.connect(config);
      try {
        await mortal.login({
          account,
          character,
          password: DEFAULT_PASSWORD,
        });

        const output = await mortal.pagedCommand("history");
        expect(output).toContainCaseInsensitive(marker);
        expect(output).toContainCaseInsensitive("told you");
      } finally {
        await mortal.close();
      }
    },
    ACCOUNT_TEST_TIMEOUT,
  );

  it(
    "caps history at 25 entries per recipient",
    async () => {
      // Phase 1: mortal logs in and receives 27 tells (2 early + 25 cap)
      let mortal = await MudClient.connect(config);
      await mortal.login({
        account,
        character,
        password: DEFAULT_PASSWORD,
      });

      const earlyMarker = `early${Date.now()}`;
      // These 2 early tells should be evicted after the 25 that follow.
      await imm.command(`tell ${character} ${earlyMarker}`);
      await imm.command(`tell ${character} ${earlyMarker}`);

      for (let i = 0; i < 25; i++) {
        await imm.command(`tell ${character} cap${i}`);
      }

      // Drain notifications and log out.
      // Wait for rent output before closing to avoid broken pipe on server.
      await mortal.readUntilIdle(2);
      await mortal.send("rent");
      await mortal.readUntilIdle(1);
      await mortal.close();

      // Phase 2: re-login to load tell history from database
      mortal = await MudClient.connect(config);
      try {
        await mortal.login({
          account,
          character,
          password: DEFAULT_PASSWORD,
        });

        const output = await mortal.pagedCommand("history");
        expect(output).not.toContainCaseInsensitive(earlyMarker);
        // The recent tells should still be present
        expect(output).toContainCaseInsensitive("cap24");
      } finally {
        await mortal.close();
      }
    },
    ACCOUNT_TEST_TIMEOUT,
  );
});
