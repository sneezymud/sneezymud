/**
 * Low shop command tests that require a running server.
 *
 * These tests exercise code paths that depend on global game state loaded
 * at server boot (mob_index for keeper validation, ItemInfo[] for item
 * type resolution, zone data for list filtering). Tests that only need
 * database access are in the C++ integration suite (low_shop_test.cc).
 *
 * Required immortal powers: low.
 */

import { afterAll, beforeAll, describe, expect, it } from "bun:test";

import { MudClient } from "../harness/client.ts";
import { loadConfig } from "../harness/config.ts";

const config = loadConfig();

const CLEANUP = "low shop delete 999 confirm";
const CREATE = "low shop create 999 10 0";

async function setupTestShop(mud: MudClient): Promise<void> {
  await mud.command(CLEANUP);
  await mud.command(CREATE);
}

async function cleanupTestShop(mud: MudClient): Promise<void> {
  await mud.command(CLEANUP);
}

describe("Low Shop (server-dependent)", () => {
  let mud: MudClient;

  beforeAll(async () => {
    mud = await MudClient.connect(config);
    await mud.login(config);
  }, 30_000);

  afterAll(async () => {
    await mud.command(CLEANUP);
    await mud.close();
  });

  it("list filters by zone", async () => {
    const output = await mud.pagedCommand("low shop list 1");
    expect(output).toContainCaseInsensitive("shop(s) found");
  });

  describe("Shop Creation (requires mob_index)", () => {
    afterAll(async () => cleanupTestShop(mud));

    it("creates shop and info confirms it exists", async () => {
      await mud.command(CLEANUP);
      const createOutput = await mud.command(CREATE);
      expect(createOutput).toContainCaseInsensitive("Shop 999 created");

      const infoOutput = await mud.pagedCommand("low shop info 999");
      expect(infoOutput).toContainCaseInsensitive("Shop #999 Information");
    });

    it("rejects duplicate shop number", async () => {
      const output = await mud.command("low shop create 999 11 1");
      expect(output).toContainCaseInsensitive("Shop 999 already exists");
    });

    it("rejects keeper already assigned to another shop", async () => {
      const output = await mud.command("low shop create 999 150 0");
      expect(output).toContainCaseInsensitive(
        "Mob 150 is already the keeper of shop",
      );
    });
  });

  it("modify keeper validates via mob vnum lookup", async () => {
    await setupTestShop(mud);
    const output = await mud.command("low shop modify 999 keeper 11");
    expect(output).toContainCaseInsensitive("keeper set to '11'");
    await cleanupTestShop(mud);
  });

  describe("Type Management (requires ItemInfo[])", () => {
    beforeAll(async () => setupTestShop(mud));
    afterAll(async () => cleanupTestShop(mud));

    it("add, reject duplicate, remove, reject absent removal", async () => {
      let output = await mud.command("low shop addtype 999 5");
      expect(output).toContainCaseInsensitive("now trades item type 5");

      output = await mud.command("low shop addtype 999 5");
      expect(output).toContainCaseInsensitive("already trades item type 5");

      output = await mud.command("low shop rmtype 999 5");
      expect(output).toContainCaseInsensitive("Removed item type 5");

      output = await mud.command("low shop rmtype 999 5");
      expect(output).toContainCaseInsensitive("was not trading item type 5");
    });
  });
});
