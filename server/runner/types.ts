// One result shape for every language, so the UI and Tess treat them all the same.
export interface Diagnostic {
  file: "your-code" | "tests";
  line: number; // 1-based
  column: number; // 1-based
  message: string;
  /** "TS2345", "-Wunused-variable", "assignment" (mypy)… */
  code: string;
  /** The source line the diagnostic points at, for context in the UI. */
  source: string;
  /** Errors block a solve; warnings are advice (C -Wall, mypy). */
  severity: "error" | "warning";
  tool: "tsc" | "gcc" | "python" | "mypy";
}

export interface TestResult {
  name: string;
  pass: boolean;
  ms: number;
  error?: { message: string; expected?: string; received?: string; stack?: string };
}

export interface RunResult {
  /** True when there are no error diagnostics, nothing crashed on load, and every test passes. */
  passed: boolean;
  diagnostics: Diagnostic[];
  setupErrors: { file: string; message: string; stack?: string }[];
  tests: TestResult[];
  logs: { level: "log" | "warn" | "error"; text: string }[];
  durationMs: number;
}

export const LOOP_MSG = "Took longer than 1500ms — infinite loop?";
export const SKIPPED_MSG = "Skipped: an earlier test looped forever.";

export function verdict(r: Omit<RunResult, "passed" | "durationMs">, typesOnly = false) {
  return (
    !r.diagnostics.some((d) => d.severity === "error") &&
    r.setupErrors.length === 0 &&
    (typesOnly || (r.tests.length > 0 && r.tests.every((t) => t.pass)))
  );
}

/** Source line lookup for diagnostics. */
export const lineOf = (code: string, line: number) => (code.split("\n")[line - 1] ?? "").trim();
