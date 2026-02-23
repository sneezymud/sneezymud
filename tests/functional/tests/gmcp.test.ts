/**
 * GMCP (Generic MUD Communication Protocol) over telnet.
 *
 * Tests that the server correctly parses GMCP Core.Hello JSON payloads
 * sent via telnet subnegotiation and stores the client name/version on
 * the connection descriptor.
 *
 * Required immortal powers: POWER_USERS (granted to auto-promoted admin).
 */

import { afterAll, beforeAll, describe, expect, it } from "bun:test";

import { MudClient } from "../harness/client.ts";
import { loadConfig } from "../harness/config.ts";

const config = loadConfig();

/** Build a GMCP telnet subnegotiation frame: IAC SB GMCP <payload> IAC SE */
function gmcpFrame(payload: string): Uint8Array {
  const encoder = new TextEncoder();
  const payloadBytes = encoder.encode(payload);
  const frame = new Uint8Array(payloadBytes.length + 5);
  frame[0] = 0xff; // IAC
  frame[1] = 0xfa; // SB
  frame[2] = 0xc9; // GMCP (201)
  frame.set(payloadBytes, 3);
  frame[frame.length - 2] = 0xff; // IAC
  frame[frame.length - 1] = 0xf0; // SE
  return frame;
}

describe("GMCP Core.Hello", () => {
  let mud: MudClient;

  beforeAll(async () => {
    mud = await MudClient.connect(config);

    // Send Core.Hello before login so the descriptor has client info
    // by the time we query it with `users`.
    await mud.sendRaw(
      gmcpFrame('Core.Hello {"client":"TestHarness","version":"42.0"}'),
    );

    await mud.login(config);
  }, 30_000);

  afterAll(async () => {
    await mud.close();
  });

  it("client name and version appear in users output", async () => {
    const output = await mud.pagedCommand("users");
    expect(output).toContainCaseInsensitive("TestHarness");
    expect(output).toContainCaseInsensitive("42.0");
  });
});
