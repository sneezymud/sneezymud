/**
 * Spell name parsing via the `cast` command.
 *
 * Tests that the recursive spell parser (parseSpellNum) correctly resolves
 * abbreviated and partial spell names. The parser drops words from the end
 * and retries, enabling multi-word abbreviations like "de i" for "detect
 * invisibility". The parser requires a unique match — ambiguous abbreviations
 * like "d i" (matching both "detect" and "dispel") are rejected.
 *
 * Spell names are passed without quotes — the server does not strip them.
 *
 * These tests don't require the character to actually know the spells —
 * the error message distinguishes "spell not found" (parser failure) from
 * "you don't know that spell" (parser succeeded, character lacks the skill).
 *
 * Required immortal powers: none (uses default test character).
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

// "No such spell exists." = parser didn't find a matching spell name.
// "You don't know that spell!" = parser found a spell, but character hasn't learned it.
// Anything else = the spell was found AND the character knows it (casting attempt).

/** Returns true if the output indicates the spell parser found a matching spell. */
function spellWasFound(output: string): boolean {
  return !output.toLowerCase().includes("no such spell exists");
}

/** Returns true if the output indicates the character doesn't know the spell. */
function characterDoesNotKnow(output: string): boolean {
  return output.toLowerCase().includes("you don't know that spell");
}

describe("Spell Name Parsing", () => {
  it("rejects non-existent spell name", async () => {
    const output = await mud.command("cast blarg");
    expect(output).toContainCaseInsensitive("no such spell exists");
  });

  it("rejects complete gibberish", async () => {
    const output = await mud.command("cast xyzzy plugh");
    expect(output).toContainCaseInsensitive("no such spell exists");
  });

  it("rejects ambiguous abbreviation: 'd i' matches both detect and dispel", async () => {
    const output = await mud.command("cast d i");
    expect(output).toContainCaseInsensitive("no such spell exists");
  });

  it("resolves abbreviated multi-word spell: 'de i' -> detect invisibility", async () => {
    const output = await mud.command("cast de i");
    expect(spellWasFound(output)).toBe(true);
  });

  it("resolves partial spell name: 'detect invis'", async () => {
    const output = await mud.command("cast detect invis");
    expect(spellWasFound(output)).toBe(true);
  });

  it("resolves full spell name: 'detect invisibility'", async () => {
    const output = await mud.command("cast detect invisibility");
    expect(spellWasFound(output)).toBe(true);
  });

  it("resolves single-word spell: 'teleport'", async () => {
    const output = await mud.command("cast teleport");
    expect(spellWasFound(output)).toBe(true);
  });

  it("resolves abbreviated single-word spell: 'telepo'", async () => {
    // "telep" is ambiguous (matches both "teleport" and "telepathy")
    const output = await mud.command("cast telepo");
    expect(spellWasFound(output)).toBe(true);
  });
});

describe("Spell Parsing with Known Spells", () => {
  it("immortal can cast a spell they know", async () => {
    // The test immortal should know at least some spells. If the cast
    // attempt doesn't say "no such spell" or "don't know", it means
    // the parser found the spell AND the character knows it.
    const output = await mud.command("cast teleport");
    // Should either succeed or fail for some reason other than not
    // knowing the spell (e.g. missing target, insufficient mana).
    // An immortal at level 60 should know teleport.
    expect(spellWasFound(output)).toBe(true);
    expect(characterDoesNotKnow(output)).toBe(false);
  });
});
