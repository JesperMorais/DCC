import { Worker } from "node:worker_threads";
import type { Lang } from "../../shared/languages.ts";
import { runC, type CProfile } from "./c.ts";
import { typeCheck, transpile } from "./check.ts";
import { runPython } from "./python.ts";
import { verdict, type RunResult } from "./types.ts";

export type { Diagnostic, RunResult, SimTrace, TestResult } from "./types.ts";
export type { CProfile } from "./c.ts";

const WORKER_URL = new URL("./worker.mjs", import.meta.url);
const HARD_KILL_MS = 6000;

function execute(userJs: string, testsJs: string) {
  return new Promise<Pick<RunResult, "setupErrors" | "tests" | "logs">>((resolve) => {
    const worker = new Worker(WORKER_URL, {
      workerData: { userJs, testsJs, syncTimeoutMs: 1500, testTimeoutMs: 2000 },
      resourceLimits: { maxOldGenerationSizeMb: 128 },
    });
    const kill = setTimeout(() => {
      worker.terminate();
      resolve({ setupErrors: [{ file: "tests", message: "Execution timed out — is there an infinite loop or a promise that never resolves?" }], tests: [], logs: [] });
    }, HARD_KILL_MS);
    worker.once("message", (msg) => {
      clearTimeout(kill);
      worker.terminate();
      resolve({ setupErrors: msg.setupErrors, tests: msg.results, logs: msg.logs });
    });
    worker.once("error", (err: Error) => {
      clearTimeout(kill);
      resolve({ setupErrors: [{ file: "your-code", message: `Crashed: ${err.message}` }], tests: [], logs: [] });
    });
  });
}

async function runTypeScript(userCode: string, testsCode: string, mode: "runtime" | "types"): Promise<RunResult> {
  const started = performance.now();
  const diagnostics = typeCheck(userCode, testsCode);
  // Type-only challenges are judged purely by the compiler.
  const exec = mode === "types" ? { setupErrors: [], tests: [], logs: [] } : await execute(transpile(userCode), transpile(testsCode));
  const r = { diagnostics, ...exec };
  return { ...r, passed: verdict(r, mode === "types"), durationMs: Math.round(performance.now() - started) };
}

export function runChallenge(
  language: Lang,
  userCode: string,
  testsCode: string,
  mode: "runtime" | "types" = "runtime",
  profile: CProfile = "c",
): Promise<RunResult> {
  switch (language) {
    case "typescript":
      return runTypeScript(userCode, testsCode, mode);
    case "python":
      return runPython(userCode, testsCode);
    case "c":
      return runC(userCode, testsCode, profile);
  }
}
