import { Worker } from "node:worker_threads";
import { typeCheck, transpile, type Diagnostic } from "./check.ts";

export interface TestResult {
  name: string;
  pass: boolean;
  ms: number;
  error?: { message: string; expected?: string; received?: string; stack?: string };
}

export interface RunResult {
  /** True when the code type-checks cleanly AND every test passes. */
  passed: boolean;
  typeErrors: Diagnostic[];
  setupErrors: { file: string; message: string; stack?: string }[];
  tests: TestResult[];
  logs: { level: "log" | "warn" | "error"; text: string }[];
  durationMs: number;
}

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

export async function runChallenge(userCode: string, testsCode: string, mode: "runtime" | "types"): Promise<RunResult> {
  const started = performance.now();
  const typeErrors = typeCheck(userCode, testsCode);

  // Type-only challenges are judged purely by the compiler.
  const exec =
    mode === "types"
      ? { setupErrors: [], tests: [], logs: [] }
      : await execute(transpile(userCode), transpile(testsCode));

  const passed =
    typeErrors.length === 0 &&
    exec.setupErrors.length === 0 &&
    (mode === "types" || (exec.tests.length > 0 && exec.tests.every((t) => t.pass)));

  return { passed, typeErrors, ...exec, durationMs: Math.round(performance.now() - started) };
}
