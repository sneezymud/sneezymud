/**
 * Direct database query helper for functional tests.
 *
 * Runs mariadb CLI commands to inspect or manipulate database state
 * that isn't accessible through game commands.
 */

export async function dbQuery({
  db = "sneezy",
  sql,
}: {
  db?: string;
  sql: string;
}): Promise<string> {
  const proc = Bun.spawn(["mariadb", db, "-N", "-e", sql], {
    stderr: "pipe",
    stdout: "pipe",
  });
  const [output, stderr, exitCode] = await Promise.all([
    new Response(proc.stdout).text(),
    new Response(proc.stderr).text(),
    proc.exited,
  ]);
  if (exitCode !== 0) {
    throw new Error(`mariadb query failed (exit ${exitCode}): ${stderr}`);
  }
  return output.trim();
}
