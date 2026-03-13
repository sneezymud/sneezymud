/**
 * Brick quest scorecard display.
 *
 * Verifies that the brickScorecard spec proc correctly resolves player
 * names via a JOIN on the player table (the name-to-player_id migration).
 *
 * Required immortal powers: goto, load (to spawn the scorecard object).
 */

import { afterAll, beforeAll, describe, expect, it } from "bun:test";

import { MudClient } from "../harness/client.ts";
import { loadConfig } from "../harness/config.ts";
import { dbQuery } from "../harness/db.ts";

const config = loadConfig();

// Object vnum 23090 is the brick quest scorecard (spec_proc 148).
const SCORECARD_VNUM = "23090";

describe("Brick Scorecard", () => {
  let mud: MudClient;
  let playerId: string;

  beforeAll(async () => {
    mud = await MudClient.connect(config);
    await mud.login(config);

    playerId = await dbQuery({
      sql: `SELECT id FROM player WHERE name='${config.character}'`,
    });

    // Go to a neutral room so a shopkeeper doesn't scavenge the dropped board
    await mud.command("goto 1");
  }, 30_000);

  afterAll(async () => {
    await dbQuery({
      sql: `DELETE FROM brickquest WHERE player_id=${playerId}`,
    });
    await mud.command("junk board");
    await mud.close();
  });

  it("displays player names resolved via player table JOIN", async () => {
    // Insert a test entry in the brickquest table
    await dbQuery({
      sql: `INSERT INTO brickquest (player_id, numbricks) VALUES (${playerId}, 42) ON DUPLICATE KEY UPDATE numbricks=42`,
    });

    // Load the scorecard object and drop it so we can look at it
    await mud.command(`load object ${SCORECARD_VNUM}`);
    await mud.command("drop board");

    const output = await mud.command("look board");

    // The spec proc should display the test character's name (resolved
    // via JOIN with the player table) alongside their brick count.
    expect(output).toContainCaseInsensitive(config.character);
    expect(output).toContainCaseInsensitive("42");
  });
});
