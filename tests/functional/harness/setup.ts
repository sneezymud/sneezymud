#!/usr/bin/env bun

import { setDefaultTimeout } from "bun:test";

import { MudClient } from "./client.ts";
import { loadConnectionConfig } from "./config.ts";
import "./matchers.ts";

// Verify the game server is reachable before running any tests. A failed
// connection here produces a single clear message instead of every test
// file independently timing out with unhelpful socket errors.
//
// `await using` automatically closes the connection when _client goes out
// of scope (end of the try block), even if an exception is thrown.
const { host, port } = loadConnectionConfig();
try {
  await using _client = await MudClient.connect({ host, port });
} catch (error: unknown) {
  const reason = error instanceof Error ? error.message : String(error);
  console.error(
    `\nGame server is not reachable at ${host}:${port} (${reason}).\n` +
      "Start the server before running functional tests.\n",
  );
  process.exit(1);
}

// Login + character loading can take 5-10 seconds over a network connection.
// Set a generous default timeout for all tests and hooks.
setDefaultTimeout(30_000);
