/**
 * Shop cost-of-goods-sold (COGS) accounting.
 *
 * Verifies that selling an item to a shopkeeper correctly records COGS
 * via the sell -> journalize -> COGS_add pipeline, including aggregation
 * via ON DUPLICATE KEY UPDATE on repeated sales.
 *
 * Required immortal powers: goto, load.
 * Uses shop 0 (room 559), which is an owned shop that trades weapon types.
 */

import { afterAll, beforeAll, describe, expect, it } from "bun:test";

import { MudClient } from "../harness/client.ts";
import { loadConfig } from "../harness/config.ts";
import { dbQuery } from "../harness/db.ts";

const config = loadConfig();

const SHOP_ROOM = "559";
const WEAPON_VNUM = "305";

describe("Shop COGS", () => {
  let mud: MudClient;

  beforeAll(async () => {
    mud = await MudClient.connect(config);
    await mud.login(config);
    await mud.command(`goto ${SHOP_ROOM}`);
  }, 30_000);

  afterAll(async () => {
    await dbQuery({
      sql: `DELETE FROM shoplogcogs WHERE shop_nr=0 AND obj_name LIKE '%small dagger%'`,
    });
    await mud.close();
  });

  it("aggregates COGS via sell pipeline", async () => {
    // Clean prior test data
    await dbQuery({
      sql: `DELETE FROM shoplogcogs WHERE shop_nr=0 AND obj_name LIKE '%small dagger%'`,
    });

    // First sell
    await mud.command(`load object ${WEAPON_VNUM}`);
    const sell1 = await mud.command("sell dagger", { delay: 2 });
    expect(sell1).toContainCaseInsensitive("talens");

    // Verify COGS row created with count=1
    const row1 = await dbQuery({
      sql: `SELECT count FROM shoplogcogs WHERE shop_nr=0 AND obj_name LIKE '%small dagger%'`,
    });
    expect(row1).toBe("1");

    // Second sell - triggers ON DUPLICATE KEY UPDATE aggregation
    await mud.command(`load object ${WEAPON_VNUM}`);
    await mud.command("sell dagger", { delay: 2 });

    // Verify count aggregated to 2
    const row2 = await dbQuery({
      sql: `SELECT count FROM shoplogcogs WHERE shop_nr=0 AND obj_name LIKE '%small dagger%'`,
    });
    expect(row2).toBe("2");
  });
});
