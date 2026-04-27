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

describe("Session Basics", () => {
  it("who shows connected player list", async () => {
    const output = await mud.command("who");
    expect(output).toContainCaseInsensitive("Players:");
    expect(output).toContainCaseInsensitive("Total:");
  });

  it("score shows character vitals", async () => {
    const output = await mud.command("score");
    expect(output).toContainCaseInsensitive("hit points");
    expect(output).toContainCaseInsensitive("Your level:");
  });

  it("prompt shows prompt options", async () => {
    // prompt output spans 2 pages, activating the MUD's built-in pager.
    // command() would leave the pager active, swallowing the next command.
    const output = await mud.pagedCommand("prompt");
    expect(output).toContainCaseInsensitive("Prompt Line Options:");
  });

  it("yodel produces flavor text", async () => {
    const output = await mud.command("yodel");
    expect(output).toContainCaseInsensitive("You long for the hills");
  });
});

describe("Paged Output", () => {
  it("help index content is visible via pager", async () => {
    const output = await mud.pagedCommand("help");
    expect(output).toContainCaseInsensitive("INDEX OF HELP TOPICS");
    expect(output).toContainCaseInsensitive("help index");
  });
});
