/**
 * Room special procedure listing.
 *
 * Verifies that `show rproc` displays the room specials registry correctly
 * after the migration from a positional C-style array to an ID-based
 * constexpr std::array.
 *
 * Required immortal powers: show.
 */

import { afterAll, beforeAll, describe, expect, it } from "bun:test";

import { MudClient } from "../harness/client.ts";
import { loadConfig } from "../harness/config.ts";

const config = loadConfig();
let mud: MudClient;

beforeAll(async () => {
  mud = await MudClient.connect(config);
  await mud.login(config);
}, 30_000);

afterAll(async () => {
  await mud.close();
});

describe("show rproc", () => {
  it("lists room specials with IDs and names", async () => {
    const output = await mud.pagedCommand("show rproc");

    // Header
    expect(output).toContainCaseInsensitive("Room Specials");

    // Spot-check entries from different parts of the array to verify
    // the ID-based iteration works end-to-end. These are stable names
    // that have existed for decades.
    expect(output).toMatchPatternCaseInsensitive(/2\).*Bank Main Entrance/);
    expect(output).toMatchPatternCaseInsensitive(/7\).*Bank Room/);
    expect(output).toMatchPatternCaseInsensitive(/33\).*blazingroom/);
  });

  it("assignable filter excludes non-assignable specs", async () => {
    const output = await mud.pagedCommand("show rproc assignable");

    // blazingroom (33) is the only assignable spec
    expect(output).toMatchPatternCaseInsensitive(/33\).*blazingroom/);

    // Bank Room (7) is not assignable and should be filtered out
    expect(output).not.toMatchPatternCaseInsensitive(/7\).*Bank Room/);
  });
});
