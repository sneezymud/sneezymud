/**
 * Mail system: send, check, receive, and error handling.
 *
 * Tests the full mail round-trip through the postmaster NPC, exercising the
 * name-to-player_id migration (mail table uses from_id/to_id columns) and
 * the inverted faction-condition bug fix in postmasterSendMail.
 *
 * Required immortal powers: goto (to reach the post office).
 * The test character must be immortal to bypass stamp costs.
 */

import { afterAll, beforeAll, describe, expect, it } from "bun:test";

import { MudClient } from "../harness/client.ts";
import { loadConfig } from "../harness/config.ts";

const config = loadConfig();

// Room 406 is the Grimhaven Post Office, where postmaster mob 406 lives.
const POST_OFFICE = "406";

describe("Mail System", () => {
  let mud: MudClient;

  beforeAll(async () => {
    mud = await MudClient.connect(config);
    await mud.login(config);

    // Move to the post office
    await mud.command(`goto ${POST_OFFICE}`);
  }, 30_000);

  afterAll(async () => {
    // Drain any pending mail so it doesn't pollute other tests
    await mud.command("receive");
    await mud.close();
  });

  it("rejects mail to non-existent player", async () => {
    const output = await mud.command("mail ZzzNobodyHere");
    expect(output).toContainCaseInsensitive("no such player");
  });

  it("sends and receives mail round-trip", async () => {
    const marker = `mailtest${Date.now()}`;

    // Start the mail compose flow - this puts us in CON_WRITING state
    // where normal prompt detection doesn't apply (writing prompt ends
    // with "-> " which isPromptLine() correctly rejects).
    await mud.send(`mail ${config.character}`);
    await mud.readUntilIdle(0.5);

    // Write the message body and terminate with ~. After processing ~,
    // the server exits CON_WRITING back to CON_PLYNG and sends a normal
    // status prompt, so command() with prompt detection works here.
    await mud.send(marker);
    const composeOutput = await mud.command("~");
    expect(composeOutput).toContainCaseInsensitive("Message sent");

    // Check for mail
    const checkOutput = await mud.command("check");
    expect(checkOutput).toContainCaseInsensitive("you have mail");

    // Receive the mail - gives a closed envelope
    const receiveOutput = await mud.command("receive", { delay: 2 });
    expect(receiveOutput).toContainCaseInsensitive(config.character);

    // Open the envelope and extract the letter inside
    await mud.command("open envelope");
    await mud.command("get all envelope");

    // Read the letter
    const readOutput = await mud.command("read letter");
    expect(readOutput).toContainCaseInsensitive(marker);

    // Clean up - drop the envelope/letter
    await mud.command("drop all");
  });
});
