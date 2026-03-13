/**
 * Shop cost-of-goods-sold (COGS) accounting.
 *
 * Verifies that COGS_add() correctly aggregates via ON DUPLICATE KEY UPDATE
 * when a shop buys items from players. Tests the atomicity fix that replaced
 * a SELECT+INSERT/UPDATE race with a single atomic statement.
 *
 * Required immortal powers: goto, load.
 * Uses shop 0 (room 559), which is an owned shop that trades weapon types.
 *
 * NOTE: The shopkeeper's doTell triggers triggerSpecialOnPerson on the
 * player's inventory. Items with spec procs (e.g. exchange coins) cause a
 * UBSan bad-downcast warning due to a pre-existing static_cast<TObj*> on
 * a TMonster in TObj::checkSpec. Non-fatal unless UBSan abort is enabled.
 */

import { afterAll, beforeAll, describe, expect, it } from "bun:test";

import { MudClient } from "../harness/client.ts";
import { loadConfig } from "../harness/config.ts";
import { dbQuery } from "../harness/db.ts";

const config = loadConfig();

// Shop 0 is in room 559, keeper vnum 150. It trades type 5 (weapons).
const SHOP_ROOM = "559";
// Vnum 305 is "a small dagger" (type 5 = weapon).
const WEAPON_VNUM = "305";

describe("Shop COGS", () => {
  let mud: MudClient;

  beforeAll(async () => {
    mud = await MudClient.connect(config);
    await mud.login(config);
    await mud.command(`goto ${SHOP_ROOM}`);
  }, 30_000);

  afterAll(async () => {
    // Clean up COGS test data
    await dbQuery({
      sql: `DELETE FROM shoplogcogs WHERE shop_nr=0 AND obj_name LIKE '%small dagger%'`,
    });
    await mud.close();
  });

  it("aggregates COGS via ON DUPLICATE KEY UPDATE", async () => {
    // Clean any prior COGS data for this item at this shop
    await dbQuery({
      sql: `DELETE FROM shoplogcogs WHERE shop_nr=0 AND obj_name LIKE '%small dagger%'`,
    });

    // First sell: load a dagger and sell it
    await mud.command(`load object ${WEAPON_VNUM}`);
    const sell1 = await mud.command("sell dagger", { delay: 2 });
    expect(sell1).toContainCaseInsensitive("talens");

    // Check DB: should have count=1
    const row1 = await dbQuery({
      sql: `SELECT count, total_cost FROM shoplogcogs WHERE shop_nr=0 AND obj_name LIKE '%small dagger%'`,
    });
    expect(row1).not.toBe("");
    const [count1] = row1.split("\t");
    expect(count1).toBe("1");

    // Second sell: same item type triggers ON DUPLICATE KEY UPDATE
    await mud.command(`load object ${WEAPON_VNUM}`);
    await mud.command("sell dagger", { delay: 2 });

    // Check DB: count should now be 2
    const row2 = await dbQuery({
      sql: `SELECT count, total_cost FROM shoplogcogs WHERE shop_nr=0 AND obj_name LIKE '%small dagger%'`,
    });
    const [count2] = row2.split("\t");
    expect(count2).toBe("2");
  });
});
