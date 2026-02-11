/**
 * Async telnet client for functional MUD integration tests.
 *
 * Provides prompt-aware reading, paged output collection, and the full
 * SneezyMUD multi-step login flow over a raw TCP socket.
 */

import { Socket } from "node:net";

// Empirically tuned for reliable reads against a live MUD server. Too short
// causes empty reads (server hasn't flushed yet); too long wastes time.
export const COMMAND_DELAY = 0.5;
export const PAGED_DELAY = 1;
export const LOGIN_DELAY = 1;

// Matches the standard status prompt "H:<hp> M:<mana> ...>". Anchored to
// reject mid-line false positives. Doesn't match class-specific variants
// (P: piety, LF: lifeforce) — those are caught by the endsWith(">") fallback.
const STATUS_PROMPT_RE = /^H:\d+ M:\d[^\n\r]*>\s*$/;
const RECONNECT_PROMPT_RE = /Reconnect\?\s*:/i;
const LOGIN_FAILURE_PATTERNS = [
  "incorrect login",
  "invalid password",
  "no such character",
  "unknown character",
  "authentication failed",
];

export interface CommandOptions {
  delay?: number;
  timeout?: number;
}

export interface ReadOptions {
  idleTimeout?: number;
  timeout?: number;
}

export class MudClient {
  private buffer: Uint8Array[] = [];
  private readonly commandDelay: number;
  private eof = false;
  private lastError: Error | null = null;
  private readonly loginDelay: number;
  private readonly pagedDelay: number;
  private pendingResolve: ((value: null | Uint8Array) => void) | null = null;
  private readonly socket: Socket;

  private constructor({
    commandDelay,
    loginDelay,
    pagedDelay,
    socket,
  }: {
    commandDelay?: number;
    loginDelay?: number;
    pagedDelay?: number;
    socket: Socket;
  }) {
    this.socket = socket;
    this.commandDelay = commandDelay ?? COMMAND_DELAY;
    this.pagedDelay = pagedDelay ?? PAGED_DELAY;
    this.loginDelay = loginDelay ?? LOGIN_DELAY;

    socket.on("data", (data: Buffer) => {
      if (this.pendingResolve) {
        const resolve = this.pendingResolve;
        this.pendingResolve = null;
        resolve(new Uint8Array(data));
      } else {
        this.buffer.push(new Uint8Array(data));
      }
    });

    socket.on("end", () => {
      this.handleClose();
    });
    socket.on("error", (err) => {
      this.lastError = err;
      this.handleClose();
    });
  }

  static async connect({
    host,
    port,
    ...delays
  }: {
    commandDelay?: number;
    host: string;
    loginDelay?: number;
    pagedDelay?: number;
    port: number;
  }): Promise<MudClient> {
    return new Promise((resolve, reject) => {
      const socket = new Socket();
      socket.once("error", reject);
      socket.connect(port, host, () => {
        socket.removeListener("error", reject);
        resolve(new MudClient({ socket, ...delays }));
      });
    });
  }

  async close(): Promise<void> {
    return new Promise((resolve) => {
      if (this.socket.destroyed) {
        resolve();
        return;
      }
      this.socket.once("close", () => {
        resolve();
      });
      this.socket.end();
    });
  }

  async command(cmd: string, options?: CommandOptions): Promise<string> {
    const delay = options?.delay ?? this.commandDelay;

    // Drain stale output from a previous command to prevent bleed-through.
    await this.readUntilIdle(0.1);
    await this.send(cmd);
    // idleTimeout clamped to [0.1, 0.5]: below 0.1 risks premature return;
    // above 0.5 defeats the purpose of prompt-based termination. Total timeout
    // at 4x delay gives slow commands room to finish.
    let output = await this.readUntilPrompt({
      idleTimeout: Math.max(0.1, Math.min(delay, 0.5)),
      timeout: options?.timeout ?? Math.max(2, delay * 4),
    });

    if (output.trim().length === 0) {
      // Some commands flush slowly; readUntilPrompt may return before
      // output arrives. Fall back to a simple idle-based read.
      output = await this.readUntilIdle(delay);
    }

    return output;
  }

  /**
   * Log in via SneezyMUD's multi-step flow.
   *
   * Login sequence: account -> password -> [reconnect?] -> menu choice "c"
   * for character select -> character name -> in-game prompt.
   */
  async login({
    account,
    character,
    password,
  }: {
    account: string;
    character: string;
    password: string;
  }): Promise<void> {
    await this.readUntilIdle(this.loginDelay);

    await this.send(account);
    await this.readUntilIdle(this.loginDelay);

    await this.send(password);
    let output = await this.readUntilIdle(this.loginDelay);
    if (looksLoginFailed(output)) {
      throw new Error("Login rejected after password entry.");
    }

    // If a previous session is still connected, server offers reconnect.
    if (RECONNECT_PROMPT_RE.test(output)) {
      await this.send("y");
      const reconnectOutput = await this.readUntilPrompt({
        idleTimeout: this.loginDelay,
        timeout: this.loginDelay * 4,
      });
      if (containsPrompt(reconnectOutput)) {
        await this.send("");
        await this.readUntilIdle(this.loginDelay);
        return;
      }
      throw new Error(
        `Reconnect flow did not reach in-game prompt.\nOutput: ${reconnectOutput.slice(0, 500)}`,
      );
    }

    // "c" = character select from the account menu.
    await this.send("c");
    output = await this.readUntilIdle(this.loginDelay);
    // Single-character accounts skip the name prompt and go straight in-game.
    if (containsPrompt(output)) {
      await this.send("");
      await this.readUntilIdle(this.loginDelay);
      return;
    }

    await this.send(character);
    // Longest timeout — character loading involves DB lookups and world placement.
    output = await this.readUntilPrompt({
      idleTimeout: this.loginDelay,
      timeout: this.loginDelay * 6,
    });
    if (looksLoginFailed(output)) {
      throw new Error("Login rejected after character selection.");
    }

    if (!containsPrompt(output)) {
      // Some logins stream long MOTD/welcome content before any prompt.
      await this.send("");
      output += await this.readUntilPrompt({
        idleTimeout: this.loginDelay,
        timeout: this.loginDelay * 4,
      });
    }

    if (!containsPrompt(output)) {
      // Last resort: fall back to idle-based read with a longer timeout in case
      // the server is extremely slow to produce a prompt after character load.
      await this.send("");
      output += await this.readUntilIdle(this.loginDelay * 2);
    }

    if (looksLoginFailed(output)) {
      throw new Error("Login rejected after character selection.");
    }
    if (!containsPrompt(output)) {
      throw new Error(
        `Login did not reach in-game prompt after character selection.\nOutput: ${output.slice(0, 500)}`,
      );
    }

    // Flush trailing MOTD/welcome output so the first command() call starts clean.
    await this.send("");
    await this.readUntilIdle(this.loginDelay);
  }

  async pagedCommand(cmd: string, options?: CommandOptions): Promise<string> {
    const delay = options?.delay ?? this.pagedDelay;
    // Safety limit to prevent infinite loops if page detection breaks.
    const maxPages = 50;

    await this.readUntilIdle(0.1);
    await this.send(cmd);
    const allOutput: string[] = [];

    const firstPage = await this.readUntilIdle(delay);
    if (firstPage) {
      allOutput.push(firstPage);
    }

    // MUD pager intercepts all input; an empty line means "next page".
    const pageTimeout = Math.max(0.3, delay * 0.5);
    for (let i = 0; i < maxPages; i++) {
      await this.send("");
      const chunk = await this.readUntilIdle(pageTimeout);
      if (!chunk) {
        break;
      }
      if (isPromptOnlyChunk(chunk)) {
        break;
      }
      if (RECONNECT_PROMPT_RE.test(chunk)) {
        break;
      }
      allOutput.push(chunk);
    }

    return allOutput.join("");
  }

  /**
   * Send a raw command (for pre-login menu flows) and read idle output.
   *
   * Unlike `command()`, this tolerates a closed connection (returns "")
   * since menu flows (e.g. "e" to exit) may close the connection as a
   * normal part of the protocol.
   */
  async rawSend({
    delay,
    text,
  }: {
    delay?: number | undefined;
    text: string;
  }): Promise<string> {
    const d = delay ?? this.loginDelay;
    await this.readUntilIdle(0.1);
    if (this.eof) {
      return "";
    }
    await this.send(text);
    return await this.readUntilIdle(d);
  }

  /**
   * Read until the server stops sending for `timeout` seconds.
   *
   * Simple fallback for fire-and-forget reads (login banners, flushing
   * stale output).
   */
  async readUntilIdle(timeout?: number): Promise<string> {
    const t = timeout ?? this.commandDelay;
    const chunks: string[] = [];

    while (true) {
      const data = await this.readChunk(t);
      if (data === null) {
        break;
      }
      chunks.push(stripTelnetAndAnsi(data));
    }

    return chunks.join("");
  }

  /**
   * Read until a prompt is seen and output goes idle, or total timeout.
   *
   * After seeing a prompt line AND idleTimeout elapses with no new data,
   * returns the accumulated output. The idle window prevents returning
   * prematurely when the prompt appears mid-stream.
   */
  async readUntilPrompt(options?: ReadOptions): Promise<string> {
    const timeout = options?.timeout ?? 5;
    const idleTimeout = options?.idleTimeout ?? 0.25;
    const chunks: string[] = [];
    let sawPrompt = false;
    const started = performance.now() / 1000;

    while (true) {
      const elapsed = performance.now() / 1000 - started;
      if (elapsed >= timeout) {
        break;
      }

      const waitTimeout = Math.min(idleTimeout, timeout - elapsed);
      const data = await this.readChunk(waitTimeout);

      if (data === null) {
        if (sawPrompt) {
          // Once a prompt has appeared, an idle window means done.
          break;
        }
        if (performance.now() / 1000 - started >= timeout) {
          break;
        }
        continue;
      }

      const chunk = stripTelnetAndAnsi(data);
      chunks.push(chunk);

      if (!sawPrompt) {
        // Concatenate the last two chunks so we catch prompts split across
        // TCP segment boundaries (e.g. "H:100 M:" in one chunk, "50>" in
        // the next).
        const tail = (chunks.length >= 2 ? (chunks.at(-2) ?? "") : "") + chunk;
        if (containsPrompt(tail)) {
          sawPrompt = true;
        }
      }
      // If the entire chunk is just prompt text, the server is done — break
      // immediately rather than waiting for the idle timeout.
      if (sawPrompt && isPromptOnlyChunk(chunk)) {
        break;
      }
    }

    return chunks.join("");
  }

  async send(text: string): Promise<void> {
    if (this.eof) {
      const detail = this.lastError ? `: ${this.lastError.message}` : "";
      throw new Error(`MUD server closed the connection${detail}`);
    }
    return new Promise((resolve, reject) => {
      this.socket.write(`${text}\n`, "utf8", (error) => {
        if (error) {
          reject(error);
        } else {
          resolve();
        }
      });
    });
  }

  /** Write raw bytes to the socket without newline or encoding. */
  async sendRaw(data: Uint8Array): Promise<void> {
    if (this.eof) {
      const detail = this.lastError ? `: ${this.lastError.message}` : "";
      throw new Error(`MUD server closed the connection${detail}`);
    }
    return new Promise((resolve, reject) => {
      this.socket.write(data, (error) => {
        if (error) {
          reject(error);
        } else {
          resolve();
        }
      });
    });
  }

  /**
   * Enables `await using client = await MudClient.connect(...)` so the
   * connection is automatically closed when the variable goes out of scope.
   * JS has no deterministic destructors (GC is non-deterministic), so callers
   * must opt in to automatic cleanup via `await using` at the declaration site.
   */
  async [Symbol.asyncDispose](): Promise<void> {
    await this.close();
  }

  private handleClose(): void {
    this.eof = true;
    if (this.pendingResolve) {
      const resolve = this.pendingResolve;
      this.pendingResolve = null;
      resolve(null);
    }
  }

  /**
   * Read the next chunk of data from the socket with a timeout.
   * Returns null on EOF or timeout.
   */
  private readChunk(timeoutSeconds: number): Promise<null | Uint8Array> {
    if (this.buffer.length > 0) {
      const data = this.buffer.shift();
      if (data === undefined) {
        throw new Error("Unexpected empty buffer");
      }
      return Promise.resolve(data);
    }

    if (this.eof) {
      return Promise.resolve(null);
    }

    return new Promise((resolve) => {
      const timer = setTimeout(() => {
        this.pendingResolve = null;
        resolve(null);
      }, timeoutSeconds * 1000);

      this.pendingResolve = (data) => {
        clearTimeout(timer);
        resolve(data);
      };
    });
  }
}

function isPromptLine(line: string): boolean {
  if (STATUS_PROMPT_RE.test(line)) {
    return true;
  }
  // All SneezyMUD prompts end with a hardcoded ">" (connect.cc:2638), including
  // class-specific variants (P:/LF:) that STATUS_PROMPT_RE doesn't match.
  // Exclude "->" which appears in pre-login menu prompts like "Enter name of
  // character -> " and would false-positive during the login flow.
  return line.endsWith(">") && !line.endsWith("->");
}

function nonBlankLines(text: string): string[] {
  return text
    .replaceAll("\r", "")
    .split("\n")
    .map((line) => line.trim())
    .filter((line) => line.length > 0);
}

function containsPrompt(text: string): boolean {
  return nonBlankLines(text).some((line) => isPromptLine(line));
}

function isPromptOnlyChunk(chunk: string): boolean {
  const lines = nonBlankLines(chunk);
  return lines.length > 0 && lines.every((line) => isPromptLine(line));
}

function looksLoginFailed(output: string): boolean {
  const lower = output.toLowerCase();
  return LOGIN_FAILURE_PATTERNS.some((pattern) => lower.includes(pattern));
}

/**
 * Remove telnet IAC sequences and ANSI escape codes from raw socket data.
 *
 * We roll our own instead of using a telnet library because we only need
 * to ignore protocol noise, not negotiate options.
 */
function stripTelnetAndAnsi(data: Uint8Array): string {
  const cleaned: number[] = [];
  let i = 0;

  while (i < data.length) {
    const byte = data[i];

    if (byte === undefined) {
      throw new Error(
        "Unexpected end of data while stripping telnet sequences",
      );
    }

    // Telnet IAC (Interpret As Command) byte = 0xFF
    if (byte === 0xff && i + 1 < data.length) {
      const cmd = data[i + 1];

      if (cmd === undefined) {
        throw new Error(
          "Unexpected end of data while stripping telnet sequences",
        );
      }

      if (cmd === 0xff) {
        // Escaped 0xFF literal
        cleaned.push(0xff);
        i += 2;
      } else if (cmd === 0xfb || cmd === 0xfc || cmd === 0xfd || cmd === 0xfe) {
        // WILL/WONT/DO/DONT + option byte (3-byte sequences)
        i += 3;
      } else if (cmd >= 0xf1 && cmd <= 0xf9) {
        // Single-byte telnet commands (NOP, DM, BRK, etc.)
        i += 2;
      } else if (cmd === 0xfa) {
        // Subnegotiation — skip until IAC SE (0xFF 0xF0)
        i += 2;
        while (i < data.length - 1) {
          if (data[i] === 0xff && data[i + 1] === 0xf0) {
            i += 2;
            break;
          }
          i += 1;
        }
      } else {
        i += 2;
      }
    } else {
      cleaned.push(byte);
      i += 1;
    }
  }

  let text = new TextDecoder("utf-8", { fatal: false }).decode(
    new Uint8Array(cleaned),
  );

  // eslint-disable-next-line no-control-regex -- intentional: stripping ANSI escape sequences requires matching control characters
  text = text.replaceAll(/\u001B(?:\[[0-9;]*[a-z]|\][^\u0007]*\u0007)/gi, "");

  return text;
}
