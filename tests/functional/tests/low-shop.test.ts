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

describe("Low Shop", () => {
  let mud: MudClient;

  beforeAll(async () => {
    mud = await MudClient.connect(config);
    await mud.login(config);
  }, 30_000);

  afterAll(async () => {
    await mud.command(CLEANUP);
    await mud.close();
  });

  describe("Routing & Display", () => {
    it("no subcommand shows usage", async () => {
      const output = await mud.command("low shop", { delay: 1 });
      expect(output).toContainCaseInsensitive("Syntax:");
    });

    it("list shows shops with column headers", async () => {
      // Full shop list is large (~200 shops); give the server extra time.
      const output = await mud.pagedCommand("low shop list", { delay: 3 });
      expect(output).toContainCaseInsensitive("Shop#");
      expect(output).toContainCaseInsensitive("Keeper");
    }, 30_000);

    it("list filters by zone", async () => {
      const output = await mud.pagedCommand("low shop list 1");
      expect(output).toContainCaseInsensitive("shop(s) found");
    });

    it("list rejects out-of-range zone", async () => {
      const output = await mud.command("low shop list 99999");
      expect(output).toContainCaseInsensitive("Zone must be between");
    });

    it("non-existent shop shows error", async () => {
      const output = await mud.command("low shop info 99999");
      expect(output).toContainCaseInsensitive("Shop 99999 does not exist");
    });
  });

  describe("Shop Creation", () => {
    afterAll(async () => cleanupTestShop(mud));

    it("creates shop and info confirms it exists", async () => {
      await mud.command(CLEANUP);
      const createOutput = await mud.command(CREATE);
      expect(createOutput).toContainCaseInsensitive("Shop 999 created");

      const infoOutput = await mud.pagedCommand("low shop info 999");
      expect(infoOutput).toContainCaseInsensitive("Shop #999 Information");
    });

    it("rejects duplicate shop number", async () => {
      // Re-create shop 999 (just created by the previous test) with a
      // different keeper and room to avoid earlier validation checks
      // (keeper/room uniqueness) firing before the shop-number check.
      const output = await mud.command("low shop create 999 11 1");
      expect(output).toContainCaseInsensitive("Shop 999 already exists");
    });

    it("rejects keeper already assigned to another shop", async () => {
      const output = await mud.command("low shop create 999 150 0");
      expect(output).toContainCaseInsensitive(
        "Mob 150 is already the keeper of shop",
      );
    });

    it("rejects negative shop number", async () => {
      const output = await mud.command("low shop create -1 10 0");
      expect(output).toContainCaseInsensitive(
        "Shop number must be non-negative",
      );
    });
  });

  describe("Field Modification", () => {
    beforeAll(async () => setupTestShop(mud));
    afterAll(async () => cleanupTestShop(mud));

    it("sets float field", async () => {
      const output = await mud.command("low shop modify 999 profit_buy 1.2");
      expect(output).toContainCaseInsensitive("profit_buy set to '1.2'");
    });

    it("rejects non-positive float value", async () => {
      const output = await mud.command("low shop modify 999 profit_buy -1.0");
      expect(output).toContainCaseInsensitive("must be a positive number");
    });

    it("accepts hour value 96 as always-open special case", async () => {
      const output = await mud.command("low shop modify 999 open1 96");
      expect(output).toContainCaseInsensitive("open1 set to '96'");
    });

    it("rejects hour value outside 0-23 and not 96", async () => {
      const output = await mud.command("low shop modify 999 open1 100");
      expect(output).toContainCaseInsensitive(
        "must be 0-23 (hour) or 96 (always open)",
      );
    });

    it("sets string field", async () => {
      const output = await mud.command(
        "low shop modify 999 no_such_item1 Custom message",
      );
      expect(output).toContainCaseInsensitive("no_such_item1 set to");
    });

    it("sets keeper via mob vnum lookup", async () => {
      const output = await mud.command("low shop modify 999 keeper 11");
      expect(output).toContainCaseInsensitive("keeper set to '11'");
    });

    it("rejects unknown field name", async () => {
      const output = await mud.command(
        "low shop modify 999 invalidfield value",
      );
      expect(output).toContainCaseInsensitive("Unknown field 'invalidfield'");
    });
  });

  describe("Type Management", () => {
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

    it("rejects invalid item type", async () => {
      const output = await mud.command("low shop addtype 999 9999");
      expect(output).toContainCaseInsensitive("Invalid item type 9999");
    });
  });

  describe("Ownership", () => {
    beforeAll(async () => setupTestShop(mud));
    afterAll(async () => cleanupTestShop(mud));

    it("shows unowned, sets owner, shows owned, removes owner", async () => {
      await mud.command("low shop owner 999 remove");

      let output = await mud.pagedCommand("low shop owner 999");
      expect(output).toContainCaseInsensitive("Status: Not owned");

      output = await mud.command("low shop owner 999 set 1");
      expect(output).toContainCaseInsensitive("Shop 999 is now owned by");

      output = await mud.pagedCommand("low shop owner 999");
      expect(output).toContainCaseInsensitive("Status: Owned by");

      output = await mud.command("low shop owner 999 remove");
      expect(output).toContainCaseInsensitive("Shop 999 ownership removed");
    });

    it("rejects invalid corporation", async () => {
      const output = await mud.command("low shop owner 999 set 999999");
      expect(output).toContainCaseInsensitive(
        "Corporation 999999 does not exist",
      );
    });
  });

  describe("Access Control", () => {
    beforeAll(async () => setupTestShop(mud));
    afterAll(async () => {
      await mud.command("low shop owner 999 remove");
      await cleanupTestShop(mud);
    });

    it("rejects access on unowned shop", async () => {
      await mud.command("low shop owner 999 remove");
      const output = await mud.command("low shop access 999 TestPlayer 1");
      expect(output).toContainCaseInsensitive("Shop 999 is not owned");
    });

    it("sets access, describes bitmask, and removes via zero", async () => {
      await mud.command("low shop owner 999 set 1");

      let output = await mud.command(
        `low shop access 999 ${config.character} 1`,
      );
      expect(output).toContainCaseInsensitive("access set to 1 (owner)");

      output = await mud.command(`low shop access 999 ${config.character} 7`);
      expect(output).toContainCaseInsensitive(
        "access set to 7 (owner, info, rates)",
      );

      output = await mud.command(`low shop access 999 ${config.character} 0`);
      expect(output).toContainCaseInsensitive("access removed");
    });

    it("rejects non-existent player", async () => {
      await mud.command("low shop owner 999 set 1");
      const output = await mud.command("low shop access 999 ZzzNobodyHere 1");
      expect(output).toContainCaseInsensitive("not found");
    });

    it("rejects access level above 255", async () => {
      await mud.command("low shop owner 999 set 1");
      const output = await mud.command(
        `low shop access 999 ${config.character} 256`,
      );
      expect(output).toContainCaseInsensitive("Access level must be 0-255");
    });
  });

  describe("Shop Deletion", () => {
    it("deletes shop and info confirms it is gone", async () => {
      await setupTestShop(mud);

      const deleteOutput = await mud.command(CLEANUP);
      expect(deleteOutput).toContainCaseInsensitive(
        "Shop 999 and all related data deleted",
      );

      const infoOutput = await mud.command("low shop info 999");
      expect(infoOutput).toContainCaseInsensitive("Shop 999 does not exist");
    });
  });

  describe("SQL Safety", () => {
    beforeAll(async () => setupTestShop(mud));
    afterAll(async () => cleanupTestShop(mud));

    it("handles quotes in string fields", async () => {
      const output = await mud.command(
        `low shop modify 999 no_such_item1 He says, "We don't have that!"`,
      );
      expect(output).toContainCaseInsensitive("no_such_item1 set to");
    });

    it("SQL injection attempt does not corrupt data", async () => {
      await mud.command(
        "low shop modify 999 no_such_item1 '; DROP TABLE shop; --",
      );
      const output = await mud.pagedCommand("low shop info 999");
      expect(output).toContainCaseInsensitive("Shop #999 Information");
    });
  });
});
