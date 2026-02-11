export interface ConnectionConfig {
  readonly host: string;
  readonly port: number;
}

export interface MudConfig extends ConnectionConfig {
  readonly account: string;
  readonly character: string;
  readonly password: string;
}

export function loadConnectionConfig(): ConnectionConfig {
  return {
    host: process.env["MUD_HOST"] ?? "localhost",
    port: Number(process.env["MUD_PORT"] ?? "7900"),
  };
}

export function loadConfig(): MudConfig {
  return {
    ...loadConnectionConfig(),
    account: requireEnv("MUD_ACCOUNT"),
    character: requireEnv("MUD_CHARACTER"),
    password: requireEnv("MUD_PASSWORD"),
  };
}

function requireEnv(name: string): string {
  const value = process.env[name];
  if (!value) {
    throw new Error(
      `Missing required env var: ${name}. Set in .env or environment.`,
    );
  }
  return value;
}
