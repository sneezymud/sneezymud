import { expect } from "bun:test";

expect.extend({
  toContainAnyCaseInsensitive(actual: unknown, candidates: readonly string[]) {
    const str = String(actual);
    const lower = str.toLowerCase();
    const pass = candidates.some((c) => lower.includes(c.toLowerCase()));
    return {
      message: () =>
        pass
          ? `Expected output NOT to contain any of: ${JSON.stringify(candidates)}`
          : `Expected output to contain at least one of: ${JSON.stringify(candidates)}\n\nActual output:\n${formatSnippet(str)}`,
      pass,
    };
  },

  toContainCaseInsensitive(actual: unknown, expected: string) {
    const str = String(actual);
    const pass = str.toLowerCase().includes(expected.toLowerCase());
    return {
      message: () =>
        pass
          ? `Expected output NOT to contain (case-insensitive): "${expected}"`
          : `Expected output to contain (case-insensitive): "${expected}"\n\nActual output:\n${formatSnippet(str)}`,
      pass,
    };
  },

  toMatchPatternCaseInsensitive(actual: unknown, pattern: RegExp) {
    const str = String(actual);
    const flags = pattern.flags.includes("i")
      ? pattern.flags
      : `${pattern.flags}i`;
    const re = new RegExp(pattern.source, flags);
    const pass = re.test(str);
    return {
      message: () =>
        pass
          ? `Expected output NOT to match pattern: ${String(pattern)}`
          : `Expected output to match pattern: ${String(pattern)}\n\nActual output:\n${formatSnippet(str)}`,
      pass,
    };
  },
});

function formatSnippet(output: string, limit = 1200): string {
  const normalized = output.replaceAll("\r", "");
  if (normalized.length <= limit) {
    return normalized;
  }
  return `${normalized.slice(0, limit)}\n... (truncated)`;
}

declare module "bun:test" {
  // eslint-disable-next-line @typescript-eslint/no-unused-vars -- T must match original Matchers<T> declaration
  interface Matchers<T> {
    toContainAnyCaseInsensitive(candidates: readonly string[]): void;
    toContainCaseInsensitive(expected: string): void;
    toMatchPatternCaseInsensitive(pattern: RegExp): void;
  }
}
