/**
 * Tell command - online delivery.
 *
 * Verifies that telling to an online player succeeds. Alt-detection
 * privilege gating (immortal vs mortal, offline alts) is tested in
 * the C++ unit suite (tell_test.cc).
 *
 * Required immortal powers: none beyond being immortal (level > MAX_MORT).
 * The test uses the configured immortal from .env for the teller role
 * and creates an ephemeral account for the target.
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

describe("Tell", () => {
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
    "tell to online player succeeds",
    async () => {
      const mortal = await MudClient.connect(config);
      try {
        await mortal.login({
          account,
          character,
          password: DEFAULT_PASSWORD,
        });

        const output = await imm.command(`tell ${character} hello there`);
        expect(output).toContainCaseInsensitive("you tell");

        // Verify the receiver got the tell
        const received = await mortal.readUntilIdle(0.5);
        expect(received).toContainCaseInsensitive("hello there");
      } finally {
        await mortal.close();
      }
    },
    ACCOUNT_TEST_TIMEOUT,
  );
});
