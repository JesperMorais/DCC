// Runs the CLI as a separate process, exactly like `npm start -- ...` would.
// The tests only look at what it prints and its exit code, so any internal structure passes.
import { spawnSync } from "node:child_process";
import { fileURLToPath } from "node:url";

const root = fileURLToPath(new URL("..", import.meta.url));

export interface Result {
  /** stdout split into lines, without the final newline. */
  out: string[];
  /** stderr split into lines, without the final newline. */
  err: string[];
  code: number | null;
}

export function cli(...args: string[]): Result {
  const r = spawnSync(process.execPath, ["--import", "tsx", "src/main.ts", ...args], { cwd: root, encoding: "utf8" });
  const lines = (s: string) => (s === "" ? [] : s.replace(/\n$/, "").split("\n"));
  return { out: lines(r.stdout), err: lines(r.stderr), code: r.status };
}

/** Path of a fixture, relative to the project root (where the CLI runs). */
export const fixture = (name: string) => `tests/fixtures/${name}`;
