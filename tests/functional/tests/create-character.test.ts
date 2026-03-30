import { afterAll, describe, expect, it } from "bun:test";

import {
  createAccount,
  DEFAULT_PASSWORD,
  deleteAccount,
  uniqueIdentity,
} from "../harness/accounts";
import { loadConnectionConfig } from "../harness/config";
import { dbQuery } from "../harness/db";

describe("character creation", () => {
  const { account, character } = uniqueIdentity();
  const connection = loadConnectionConfig();
  const password = DEFAULT_PASSWORD;

  afterAll(async () => {
    try {
      await deleteAccount({ account, connection, password });
    } catch {
      console.warn(
        `Best-effort cleanup of ephemeral account "${account}" failed`,
      );
    }
  });

  it("new character has correct account_id linkage in database", async () => {
    await createAccount({ account, character, connection, password });

    // Query the database to verify the player row has account_id set.
    // The JOIN proves account_id is non-null and points to the right account.
    const result = await dbQuery({
      sql: `SELECT p.name, a.name
              FROM player p
              JOIN account a ON p.account_id = a.account_id
              WHERE LOWER(p.name) = LOWER('${character}')`,
    });

    expect(result).toContainCaseInsensitive(character);
    expect(result).toContainCaseInsensitive(account);
  }, 120_000);
});
