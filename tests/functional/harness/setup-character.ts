/**
 * Immortal-driven character configuration helpers.
 *
 * Uses an immortal's MudClient connection to configure test characters via
 * `@set` commands. The immortal character is the existing test account
 * configured in `.env` (MUD_ACCOUNT/MUD_CHARACTER), which is already level 60.
 */

import type { MudClient } from "./client.ts";

export interface CharacterSetup {
  disciplines?: Array<{ index: number; value: number }>;
  level?: number;
  name: string;
}

/**
 * Configure a test character using immortal @set commands.
 *
 * Each @set is a separate command() call since the MUD processes them
 * one at a time. The immortal connection must already be logged in.
 */
export async function setupCharacter({
  imm,
  setup,
}: {
  imm: MudClient;
  setup: CharacterSetup;
}): Promise<void> {
  if (setup.level !== undefined) {
    await imm.command(`@set character ${setup.name} level ${setup.level}`);
  }

  if (setup.disciplines) {
    for (const disc of setup.disciplines) {
      await imm.command(
        `@set discipline ${setup.name} ${disc.index} ${disc.value}`,
      );
    }
  }
}
