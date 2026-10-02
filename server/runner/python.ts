import { spawn, spawnSync } from "node:child_process";
import fs from "node:fs";
import os from "node:os";
import path from "node:path";
import { fileURLToPath } from "node:url";
import { lineOf, verdict, type Diagnostic, type RunResult } from "./types.ts";

const HARNESS = fileURLToPath(new URL("./python/harness.py", import.meta.url));
const PYTHON = process.env.DAILY_TS_PYTHON ?? "python3";
const MYPY_CACHE = path.join(os.tmpdir(), "daily-ts-mypy-cache");
const HARD_KILL_MS = 10_000;

const hasMypy = (() => {
  const r = spawnSync("mypy", ["--version"], { encoding: "utf8" });
  return r.status === 0;
})();

interface Proc {
  code: number | null;
  signal: NodeJS.Signals | null;
  stdout: string;
  stderr: string;
  fd3: string;
  fd4: string;
  timedOut: boolean;
}

export function runProcess(
  cmd: string,
  args: string[],
  opts: { cwd?: string; env?: NodeJS.ProcessEnv; timeoutMs?: number; fd3?: boolean; fd4?: boolean } = {},
) {
  return new Promise<Proc>((resolve) => {
    const child = spawn(cmd, args, {
      cwd: opts.cwd,
      env: opts.env ?? process.env,
      stdio: opts.fd4 ? ["ignore", "pipe", "pipe", "pipe", "pipe"] : opts.fd3 ? ["ignore", "pipe", "pipe", "pipe"] : ["ignore", "pipe", "pipe"],
    });
    const chunks = { stdout: "", stderr: "", fd3: "", fd4: "" };
    const cap = 256 * 1024;
    child.stdout!.on("data", (d) => chunks.stdout.length < cap && (chunks.stdout += d));
    child.stderr!.on("data", (d) => chunks.stderr.length < cap && (chunks.stderr += d));
    if (opts.fd3 || opts.fd4) (child.stdio[3] as NodeJS.ReadableStream).on("data", (d) => (chunks.fd3 += d));
    if (opts.fd4) (child.stdio[4] as NodeJS.ReadableStream).on("data", (d) => chunks.fd4.length < 4 * 1024 * 1024 && (chunks.fd4 += d));
    let timedOut = false;
    const t = setTimeout(() => {
      timedOut = true;
      child.kill("SIGKILL");
    }, opts.timeoutMs ?? HARD_KILL_MS);
    child.on("close", (code, signal) => {
      clearTimeout(t);
      resolve({ code, signal, ...chunks, timedOut });
    });
    child.on("error", (err) => {
      clearTimeout(t);
      resolve({ code: -1, signal: null, stdout: "", stderr: String(err.message), fd3: "", fd4: "", timedOut });
    });
  });
}

async function mypy(dir: string, userCode: string): Promise<Diagnostic[]> {
  if (!hasMypy) return [];
  const p = await runProcess(
    "mypy",
    ["--strict", "--no-error-summary", "--show-column-numbers", "--no-color-output", "--no-pretty", "--cache-dir", MYPY_CACHE, "your_code.py"],
    { cwd: dir, timeoutMs: 15_000 },
  );
  const out: Diagnostic[] = [];
  for (const line of p.stdout.split("\n")) {
    const m = /^your_code\.py:(\d+):(\d+): (error|note): (.*?)(?:\s+\[([\w-]+)\])?$/.exec(line.trim());
    if (!m || m[3] !== "error") continue;
    out.push({
      file: "your-code",
      line: Number(m[1]),
      column: Number(m[2]),
      message: m[4],
      code: m[5] ?? "mypy",
      source: lineOf(userCode, Number(m[1])),
      severity: "warning",
      tool: "mypy",
    });
  }
  return out;
}

export async function runPython(userCode: string, testsCode: string): Promise<RunResult> {
  const started = performance.now();
  const dir = fs.mkdtempSync(path.join(os.tmpdir(), "daily-ts-py-"));
  try {
    fs.writeFileSync(path.join(dir, "your-code.py"), userCode);
    fs.writeFileSync(path.join(dir, "your_code.py"), userCode); // importable name for mypy
    fs.writeFileSync(path.join(dir, "tests.py"), testsCode);

    const [proc, typeHints] = await Promise.all([
      runProcess(PYTHON, ["-I", "-X", "utf8", HARNESS, "your-code.py", "tests.py"], { cwd: dir, fd3: true }),
      mypy(dir, userCode),
    ]);

    let report: { diagnostics: Diagnostic[]; setupErrors: RunResult["setupErrors"]; results: RunResult["tests"]; logs: RunResult["logs"] };
    try {
      report = JSON.parse(proc.fd3);
    } catch {
      const why = proc.timedOut
        ? "Execution timed out — is there an infinite loop?"
        : proc.code === -1
          ? `Couldn't start Python (${PYTHON}). Is Python 3 installed?`
          : `Python stopped unexpectedly${proc.signal ? ` (${proc.signal})` : ""}. ${proc.stderr.trim().split("\n").slice(-1)[0] ?? ""}`;
      report = { diagnostics: [], setupErrors: [{ file: "your-code", message: why }], results: [], logs: [] };
    }
    // mypy advice only matters once the code actually parses.
    const diagnostics = report.diagnostics.length ? report.diagnostics : typeHints;
    const r = { diagnostics, setupErrors: report.setupErrors, tests: report.results, logs: report.logs };
    return { ...r, passed: verdict(r), durationMs: Math.round(performance.now() - started) };
  } finally {
    fs.rmSync(dir, { recursive: true, force: true });
  }
}
