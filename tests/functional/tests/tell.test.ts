/**
 * Tell command alt-detection behavior.
 *
 * When an immortal tells to an offline character, the server queries the
 * database for other characters on the same account and reveals any that
 * are currently online. Mortals do not get this hint.
 *
 * Required immortal powers: none beyond being immortal (level > MAX_MORT).
 * The test uses the configured immortal from .env for the immortal role
 * and creates an ephemeral account with two characters for the mortal role.
 */

import { afterAll, beforeAll, describe, expect, it } from "bun:test";

import {
  ACCOUNT_TEST_TIMEOUT,
  addCharacterToAccount,
  createAccount,
  DEFAULT_PASSWORD,
  deleteAccount,
  uniqueIdentity,
} from "../harness/accounts.ts";
import { MudClient } from "../harness/client.ts";
import { loadConfig } from "../harness/config.ts";

const config = loadConfig();

describe("Tell Alt-Detection", () => {
  let imm: MudClient;

  let account: string;
  let charA: string;
  let charB: string;

  beforeAll(async () => {
    imm = await MudClient.connect(config);
    await imm.login(config);

    const identity = uniqueIdentity();
    account = identity.account;
    charA = identity.character;

    // Append "b" for a distinct second character name (within length limit).
    charB = `${charA}b`;

    await createAccount({
      account,
      character: charA,
      connection: config,
      password: DEFAULT_PASSWORD,
    });
    await addCharacterToAccount({
      account,
      character: charB,
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
    "immortal sees alt suggestion when telling to offline alt",
    async () => {
      const mortal = await MudClient.connect(config);
      try {
        await mortal.login({
          account,
          character: charA,
          password: DEFAULT_PASSWORD,
        });

        // Immortal tells to charB (offline, same account as charA)
        const output = await imm.command(`tell ${charB} test message`);

        expect(output).toContainCaseInsensitive("you fail to tell");
        expect(output).toContainCaseInsensitive(
          "logged in under the same account",
        );
      } finally {
        await mortal.close();
      }
    },
    ACCOUNT_TEST_TIMEOUT,
  );

  it(
    "mortal does NOT see alt suggestion when telling to offline alt",
    async () => {
      const mortal = await MudClient.connect(config);
      try {
        await mortal.login({
          account,
          character: charA,
          password: DEFAULT_PASSWORD,
        });

        // Mortal tells to charB (offline, same account)
        const output = await mortal.command(`tell ${charB} test message`);

        expect(output).toContainCaseInsensitive("you fail to tell");
        expect(output).not.toContainCaseInsensitive(
          "logged in under the same account",
        );
      } finally {
        await mortal.close();
      }
    },
    ACCOUNT_TEST_TIMEOUT,
  );

  it("tell to non-existent player shows simple failure", async () => {
    const output = await imm.command("tell zzznonexist test message");
    expect(output).toContainCaseInsensitive("you fail to tell");
    expect(output).not.toContainCaseInsensitive(
      "logged in under the same account",
    );
  });

  it("tell to online player succeeds", async () => {
    const mortal = await MudClient.connect(config);
    try {
      await mortal.login({
        account,
        character: charA,
        password: DEFAULT_PASSWORD,
      });

      const output = await imm.command(`tell ${charA} hello there`);
      expect(output).not.toContainCaseInsensitive("you fail to tell");
    } finally {
      await mortal.close();
    }
  });
});
