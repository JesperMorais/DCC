import ts from "typescript";
import { HARNESS_DTS } from "./harness.ts";
import type { Diagnostic } from "./types.ts";

export const COMPILER_OPTIONS: ts.CompilerOptions = {
  target: ts.ScriptTarget.ES2022,
  lib: ["lib.es2023.d.ts"],
  strict: true,
  noEmit: true,
  skipLibCheck: true,
  types: [],
  noFallthroughCasesInSwitch: true,
};

const FILES = {
  harness: "/challenge/harness.d.ts",
  user: "/challenge/your-code.ts",
  tests: "/challenge/tests.ts",
} as const;

// Lib .d.ts files are large and never change, so parse them once and share them.
const baseHost = ts.createCompilerHost(COMPILER_OPTIONS, true);
const libCache = new Map<string, ts.SourceFile | undefined>();
let oldProgram: ts.Program | undefined;

export function typeCheck(userCode: string, testsCode: string): Diagnostic[] {
  const virtual = new Map<string, string>([
    [FILES.harness, HARNESS_DTS],
    [FILES.user, userCode],
    [FILES.tests, testsCode],
  ]);

  const host: ts.CompilerHost = {
    ...baseHost,
    fileExists: (f) => virtual.has(f) || baseHost.fileExists(f),
    readFile: (f) => virtual.get(f) ?? baseHost.readFile(f),
    getSourceFile(fileName, languageVersion) {
      const text = virtual.get(fileName);
      if (text !== undefined) return ts.createSourceFile(fileName, text, languageVersion, true);
      if (!libCache.has(fileName)) libCache.set(fileName, baseHost.getSourceFile(fileName, languageVersion));
      return libCache.get(fileName);
    },
    writeFile: () => {},
  };

  const program = ts.createProgram({
    rootNames: [FILES.harness, FILES.user, FILES.tests],
    options: COMPILER_OPTIONS,
    host,
    oldProgram,
  });
  oldProgram = program;

  const raw = [...program.getSyntacticDiagnostics(), ...program.getSemanticDiagnostics()];
  const out: Diagnostic[] = [];
  for (const d of raw) {
    if (!d.file || d.start === undefined) continue;
    const file = d.file.fileName === FILES.user ? "your-code" : d.file.fileName === FILES.tests ? "tests" : null;
    if (!file) continue;
    const { line, character } = d.file.getLineAndCharacterOfPosition(d.start);
    const lines = d.file.text.split("\n");
    out.push({
      file,
      line: line + 1,
      column: character + 1,
      message: ts.flattenDiagnosticMessageText(d.messageText, "\n"),
      code: `TS${d.code}`,
      source: (lines[line] ?? "").trim(),
      severity: "error",
      tool: "tsc",
    });
  }
  // User's own errors first — that's what they can fix.
  return out.sort((a, b) => (a.file === b.file ? a.line - b.line : a.file === "your-code" ? -1 : 1));
}

/** Strip types. Both files are scripts (no import/export) sharing one global scope. */
export function transpile(code: string): string {
  return ts.transpileModule(code, {
    compilerOptions: { target: ts.ScriptTarget.ES2022, module: ts.ModuleKind.None, removeComments: false },
    reportDiagnostics: false,
  }).outputText;
}
