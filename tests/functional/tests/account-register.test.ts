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

describe("Account Registration", () => {
  it(
    "registered character can login and rent",
    async () => {
      await withEphemeralAccount({
        connection: config,
        fn: async (account, character) => {
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

  it(
    "duplicate account name is rejected",
    async () => {
      await withEphemeralAccount({
        connection: config,
        fn: async (account) => {
          await using client = await MudClient.connect(config);
          await client.rawSend({ text: "NEW" });
          const output = await client.rawSend({ text: account });
          expect(output).toContainCaseInsensitive("already exists");
        },
      });
    },
    ACCOUNT_TEST_TIMEOUT,
  );
});
