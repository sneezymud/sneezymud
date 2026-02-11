import { describe, expect, it } from "bun:test";

import {
  ACCOUNT_TEST_TIMEOUT,
  attemptLogin,
  DEFAULT_PASSWORD,
  deleteAccount,
  withEphemeralAccount,
} from "../harness/accounts.ts";
import { loadConnectionConfig } from "../harness/config.ts";

const config = loadConnectionConfig();

describe("Account Deletion", () => {
  it(
    "deleted account cannot login",
    async () => {
      await withEphemeralAccount({
        connection: config,
        fn: async (account) => {
          // Delete early so we can verify login fails; the wrapper's cleanup will no-op.
          await deleteAccount({
            account,
            connection: config,
            password: DEFAULT_PASSWORD,
          });

          const output = await attemptLogin({
            account,
            connection: config,
            password: DEFAULT_PASSWORD,
          });
          expect(output).toContainCaseInsensitive("Incorrect login");
        },
      });
    },
    ACCOUNT_TEST_TIMEOUT,
  );
});
