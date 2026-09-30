import fs from "node:fs";
import os from "node:os";
import path from "node:path";
import { fileURLToPath } from "node:url";
import { runProcess } from "./python.ts";
import { LOOP_MSG, SKIPPED_MSG, lineOf, verdict, type Diagnostic, type RunResult, type TestResult } from "./types.ts";

const HARNESS_DIR = fileURLToPath(new URL("./c/", import.meta.url));
const CC = process.env.DAILY_TS_CC ?? "gcc";
const BUILD_DIR = path.join(os.tmpdir(), "daily-ts-c-build");

export const C_FLAGS = ["-std=c17", "-Wall", "-Wextra", "-pedantic", "-g", "-O0", "-fno-omit-frame-pointer", "-fdiagnostics-color=never"];
const SAN = ["-fsanitize=address,undefined", "-fno-sanitize-recover=undefined"];

let harnessObj: Promise<string> | null = null;

/** Build the harness runtime once per server start (it never changes at runtime). */
function buildHarness(): Promise<string> {
  harnessObj ??= (async () => {
    fs.mkdirSync(BUILD_DIR, { recursive: true });
    const obj = path.join(BUILD_DIR, `harness-${process.pid}.o`);
    const p = await runProcess(CC, ["-std=gnu17", "-g", "-O0", "-fno-omit-frame-pointer", ...SAN, "-c", path.join(HARNESS_DIR, "harness.c"), "-o", obj]);
    if (p.code !== 0) throw new Error(`Couldn't build the C test harness with ${CC}:\n${p.stderr}`);
    return obj;
  })();
  harnessObj.catch(() => (harnessObj = null));
  return harnessObj;
}

const DIAG_RE = /^(your-code|tests)\.c:(\d+):(\d+): (fatal error|error|warning): (.*?)(?: \[(-W[\w=-]+|-fpermissive)\])?$/;

function parseDiagnostics(stderr: string, userCode: string, testsCode: string): Diagnostic[] {
  const out: Diagnostic[] = [];
  const seen = new Set<string>();
  for (const line of stderr.split("\n")) {
    const m = DIAG_RE.exec(line);
    if (!m) continue;
    const file = m[1] as "your-code" | "tests";
    const key = `${file}:${m[2]}:${m[3]}:${m[5]}`;
    if (seen.has(key)) continue;
    seen.add(key);
    out.push({
      file,
      line: Number(m[2]),
      column: Number(m[3]),
      message: explainDiagnostic(file, m[5].replace(/‘|’/g, "'")),
      code: m[6] ?? (m[4] === "warning" ? "warning" : "error"),
      source: lineOf(file === "your-code" ? userCode : testsCode, Number(m[2])),
      severity: m[4] === "warning" ? "warning" : "error",
      tool: "gcc",
    });
  }
  return out.sort((a, b) => (a.severity === b.severity ? 0 : a.severity === "error" ? -1 : 1) || (a.file === b.file ? a.line - b.line : a.file === "your-code" ? -1 : 1));
}

/** Add the "what this means for you" part to the compiler's most confusing messages. */
function explainDiagnostic(file: "your-code" | "tests", msg: string) {
  const implicit = /^implicit declaration of function '(\w+)'/.exec(msg);
  if (implicit && file === "tests") return `${msg} — the tests call ${implicit[1]}(), but your code doesn't define it (check the name and spelling).`;
  if (implicit) return `${msg} — declare or #include it before you use it (e.g. #include <stdlib.h> for malloc/free).`;
  if (/control reaches end of non-void function/.test(msg)) return `${msg} — some path through the function doesn't return a value.`;
  if (/is used uninitialized/.test(msg)) return `${msg} — give it a starting value; C doesn't zero local variables for you.`;
  return msg;
}

function linkErrors(stderr: string): RunResult["setupErrors"] {
  const errs: RunResult["setupErrors"] = [];
  const missing = new Set([...stderr.matchAll(/undefined reference to [`'‘]([\w]+)['’]/g)].map((m) => m[1]));
  for (const name of missing) {
    errs.push({
      file: "your-code",
      message:
        name === "main"
          ? "Linker error: no main(). That's unexpected — please report this challenge."
          : `Linker error: the tests call \`${name}\`, but your code doesn't define it. Check the spelling and that it isn't commented out.`,
    });
  }
  if (/multiple definition of [`'‘]main['’]/.test(stderr))
    errs.push({ file: "your-code", message: "Don't write a main() function — the tests bring their own. Remove it and run again." });
  if (!errs.length && /ld returned|collect2/.test(stderr)) errs.push({ file: "your-code", message: `Linker error:\n${stderr.trim().split("\n").slice(-4).join("\n")}` });
  return errs;
}

/** Turn a raw AddressSanitizer / UBSan / LSan report into a message a learner can act on. */
export function explainSanitizer(report: string, kind: "crash" | "leak", signal: string): { message: string; stack?: string } {
  const where = [...report.matchAll(/#\d+ 0x[0-9a-f]+ in (\w+) .*?(your-code|tests)\.c:(\d+)/g)].map((m) => `${m[2]}.c line ${m[3]}, in ${m[1]}()`);
  const firstUser = where.find((w) => w.startsWith("your-code")) ?? where[0];
  const at = firstUser ? ` (${firstUser})` : "";
  if (kind === "leak") {
    const bytes = /SUMMARY: AddressSanitizer: (\d+) byte\(s\) leaked in (\d+) allocation/.exec(report);
    return {
      message: `Memory leak: ${bytes ? `${bytes[1]} byte(s) in ${bytes[2]} allocation(s)` : "memory"} ${bytes && bytes[2] === "1" ? "was" : "were"} never freed${at}. Every malloc() needs a matching free().`,
      stack: where.slice(0, 4).join("\n") || undefined,
    };
  }
  const ub = /(your-code|tests)\.c:(\d+):\d+: runtime error: (.*)/.exec(report);
  if (ub && /null pointer/.test(ub[3])) return { message: `NULL pointer: ${ub[3]} (${ub[1]}.c line ${ub[2]}). Check for NULL before you dereference.`, stack: where.slice(0, 4).join("\n") || undefined };
  if (ub) return { message: `Undefined behavior: ${ub[3]} (${ub[1]}.c line ${ub[2]}).`, stack: where.slice(0, 4).join("\n") || undefined };
  const kinds: [RegExp, string][] = [
    [/heap-buffer-overflow/, "Out of bounds: you read or wrote past the end of a malloc'd block"],
    [/stack-buffer-overflow/, "Out of bounds: you went past the end of an array on the stack"],
    [/global-buffer-overflow/, "Out of bounds: you went past the end of a global array or string literal"],
    [/heap-use-after-free/, "Use after free: this memory was already free()d"],
    [/attempting double-free/, "Double free: free() was called twice on the same pointer"],
    [/attempting free on address which was not malloc/, "Invalid free: that pointer didn't come from malloc()"],
    [/stack-overflow/, "Stack overflow: probably infinite recursion — is there a base case?"],
    [/SEGV on unknown address (0x0+\b|\(nil\))|SEGV on unknown address 0x0{1,6}[0-9a-f]{0,3}\b/, "NULL pointer: you dereferenced a pointer that is NULL"],
    [/SEGV/, "Segmentation fault: you touched memory you don't own (bad pointer?)"],
    [/stack-use-after-return|stack-use-after-scope/, "Dangling pointer: you used a pointer to a local variable that no longer exists"],
    [/allocation-size-too-big|requested allocation size/, "malloc() was asked for an absurd amount of memory — check your size calculation"],
  ];
  for (const [re, text] of kinds) if (re.test(report)) return { message: `${text}${at}.`, stack: where.slice(0, 4).join("\n") || undefined };
  if (/Aborted/i.test(signal)) return { message: `The program called abort()${at} — a failed assert()?`, stack: where.slice(0, 4).join("\n") || undefined };
  return { message: `Crashed (${signal})${at}.`, stack: report.trim().split("\n").slice(0, 6).join("\n") || undefined };
}

function parseResults(raw: string): TestResult[] {
  const results: TestResult[] = [];
  for (const rec of raw.split("\x1e")) {
    if (!rec.trim()) continue;
    const [rawName, status, ms, message, expected, received, detail] = rec.split("\x1f");
    const name = (rawName ?? "test").replace(/_/g, " ");
    const t = Number(ms) || 0;
    switch (status) {
      case "P":
        results.push({ name, pass: true, ms: t });
        break;
      case "F":
        results.push({ name, pass: false, ms: t, error: { message, ...(expected || received ? { expected, received } : {}) } });
        break;
      case "L":
        results.push({ name, pass: false, ms: t, error: explainSanitizer(detail ?? "", "leak", "") });
        break;
      case "T":
        results.push({ name, pass: false, ms: t, error: { message: `Error: ${LOOP_MSG}` } });
        break;
      case "S":
        results.push({ name, pass: false, ms: 0, error: { message: SKIPPED_MSG } });
        break;
      default:
        results.push({ name, pass: false, ms: t, error: explainSanitizer(detail ?? "", "crash", message ?? "") });
    }
  }
  return results;
}

export async function runC(userCode: string, testsCode: string): Promise<RunResult> {
  const started = performance.now();
  const dir = fs.mkdtempSync(path.join(os.tmpdir(), "daily-ts-c-"));
  const elapsed = () => Math.round(performance.now() - started);
  try {
    let obj: string;
    try {
      obj = await buildHarness();
    } catch (e) {
      const r = { diagnostics: [], setupErrors: [{ file: "your-code", message: (e as Error).message }], tests: [], logs: [] };
      return { ...r, passed: false, durationMs: elapsed() };
    }
    // One translation unit; #line keeps compiler messages pointing at the right file and line.
    const unit = `#include "harness.h"\n#line 1 "your-code.c"\n${userCode}\n#line 1 "tests.c"\n${testsCode}\n`;
    fs.writeFileSync(path.join(dir, "unit.c"), unit);
    const exe = path.join(dir, "program");
    const cc = await runProcess(CC, [...C_FLAGS, ...SAN, "-I", HARNESS_DIR, "unit.c", obj, "-o", exe, "-lm"], { cwd: dir, timeoutMs: 30_000 });
    const diagnostics = parseDiagnostics(cc.stderr, userCode, testsCode);
    if (cc.code !== 0) {
      const setupErrors = diagnostics.some((d) => d.severity === "error") ? [] : linkErrors(cc.stderr);
      if (!setupErrors.length && !diagnostics.some((d) => d.severity === "error"))
        setupErrors.push({ file: "your-code", message: cc.code === -1 ? `Couldn't run the C compiler (${CC}). Is gcc installed?` : cc.stderr.trim().slice(0, 2000) });
      const r = { diagnostics, setupErrors, tests: [], logs: [] };
      return { ...r, passed: false, durationMs: elapsed() };
    }

    const run = await runProcess(exe, [], {
      cwd: dir,
      fd3: true,
      timeoutMs: 15_000,
      env: {
        PATH: process.env.PATH,
        ASAN_OPTIONS: "detect_leaks=1:abort_on_error=0:allocator_may_return_null=1:detect_stack_use_after_return=1:print_summary=1",
        UBSAN_OPTIONS: "print_stacktrace=1",
        LSAN_OPTIONS: "exitcode=0",
      },
    });
    const tests = parseResults(run.fd3);
    const logs: RunResult["logs"] = run.stdout
      .split("\n")
      .filter((l, i, a) => l !== "" || i < a.length - 1)
      .slice(0, 200)
      .map((text) => ({ level: "log" as const, text }));
    const setupErrors: RunResult["setupErrors"] = [];
    if (run.timedOut) setupErrors.push({ file: "tests", message: "Execution timed out — is there an infinite loop?" });
    else if (!tests.length && run.code !== 0) setupErrors.push({ file: "tests", message: `The test program crashed on startup.\n${run.stderr.trim().slice(0, 1500)}` });
    const r = { diagnostics, setupErrors, tests, logs };
    return { ...r, passed: verdict(r), durationMs: elapsed() };
  } finally {
    fs.rmSync(dir, { recursive: true, force: true });
  }
}
