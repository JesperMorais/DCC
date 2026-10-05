// Quality gate for skill-tree projects (kind "project"), which learners build in their own editor:
//  - the starter type-checks/compiles, and (when tests are given) has tests per milestone that fail on the stubs
//  - the reference solution, copied over the starter, type-checks/compiles and passes every test
// TypeScript projects: tsc + node's test runner. C projects: `make test`, whose output ends with
// TAP-style "# pass N" / "# fail N" lines (see paths/README.md).
// usage: npm run validate:projects [-- filter ...]
import { execFileSync } from "node:child_process";
import fs from "node:fs";
import os from "node:os";
import path from "node:path";
import { fileURLToPath } from "node:url";
import { loadPaths } from "../server/paths.ts";

const ROOT = path.resolve(path.dirname(fileURLToPath(import.meta.url)), "..");
const only = process.argv.slice(2);

function run(cwd: string, cmd: string, args: string[]) {
  try {
    return { ok: true, out: execFileSync(cmd, args, { cwd, encoding: "utf8", stdio: "pipe", timeout: 300_000 }) };
  } catch (e) {
    const err = e as { stdout?: string; stderr?: string };
    return { ok: false, out: `${err.stdout ?? ""}${err.stderr ?? ""}` };
  }
}

const tsTypecheck = (dir: string) => run(dir, path.join(ROOT, "node_modules/.bin/tsc"), ["--noEmit", "-p", "."]);
const cBuild = (dir: string) => run(dir, "make", ["-s", "build"]);
function test(dir: string, lang: string) {
  const r =
    lang === "typescript"
      ? run(dir, process.execPath, ["--import", "tsx", "--test", "--test-reporter=tap", "tests/**/*.test.ts"])
      : run(dir, "make", ["-s", "test"]);
  const count = (k: string) => Number(r.out.match(new RegExp(`^# ${k} (\\d+)`, "m"))?.[1] ?? 0);
  return { ...r, pass: count("pass"), fail: count("fail") };
}

// A scratch copy with the repo's node_modules linked in, so validation needs no npm install.
function workspace(starter: string) {
  const dir = fs.mkdtempSync(path.join(os.tmpdir(), "daily-project-"));
  fs.cpSync(starter, dir, { recursive: true });
  fs.symlinkSync(path.join(ROOT, "node_modules"), path.join(dir, "node_modules"));
  return dir;
}

let failures = 0;
for (const p of loadPaths().paths) {
  for (const n of p.nodes.filter((x) => x.kind === "project")) {
    const where = `${p.id}/${n.id}`;
    if (only.length && !only.some((o) => where.includes(o))) continue;
    const problems: string[] = [];
    const pr = n.project;
    const solution = pr && path.join(path.dirname(pr.starterDir), "solution");
    if (!pr || !fs.existsSync(pr.starterDir) || !fs.existsSync(solution!)) problems.push("needs project/project.json, starter/ and solution/");
    else {
      const ts = pr.language === "typescript";
      const typecheck = ts ? tsTypecheck : cBuild;
      const required = ts ? ["package.json", "tsconfig.json", "README.md"] : ["Makefile", "README.md"];
      for (const f of required) if (!fs.existsSync(path.join(pr.starterDir, f))) problems.push(`starter/${f} missing`);
      if (pr.verify === "manual") {
        // The toolchain isn't available here. Structure only, and say so loudly.
        if (!problems.length) console.log(`  \x1b[33m!\x1b[0m ${where.padEnd(28)} ${pr.milestones.length} milestones, structure only (verify: manual — try it by hand)`);
        else {
          failures++;
          console.log(`  \x1b[31m✗ ${where}\x1b[0m`);
          for (const x of problems) console.log(`      ${x}`);
        }
        continue;
      }
      const dir = workspace(pr.starterDir);
      try {
        const tc = typecheck(dir);
        if (!tc.ok) problems.push(`starter doesn't ${ts ? "type-check" : "build"}:\n${tc.out.trim().slice(-2000)}`);
        if (pr.testsGiven) {
          if (ts) for (const m of pr.milestones) if (!fs.existsSync(path.join(dir, "tests", `${m.id}.test.ts`))) problems.push(`tests/${m.id}.test.ts missing`);
          const t = test(dir, pr.language);
          if (t.ok || t.fail === 0) problems.push("starter already passes its tests: the tests don't test anything");
        }
        fs.cpSync(solution!, dir, { recursive: true });
        const stc = typecheck(dir);
        if (!stc.ok) problems.push(`solution doesn't ${ts ? "type-check" : "build"}:\n${stc.out.trim().slice(-2000)}`);
        const st = test(dir, pr.language);
        if (!st.ok || st.fail > 0) problems.push(`solution fails its tests:\n${st.out.split("\n").filter((l) => /^not ok|error|Error/.test(l.trim())).slice(0, 12).join("\n")}`);
        else if (st.pass < pr.milestones.length * 2) problems.push(`only ${st.pass} tests (want ≥2 per milestone)`);
        if (!problems.length) console.log(`  \x1b[32m✓\x1b[0m ${where.padEnd(28)} ${pr.milestones.length} milestones, ${st.pass} tests${pr.testsGiven ? "" : " (learner-written in the real project)"}`);
      } finally {
        fs.rmSync(dir, { recursive: true, force: true });
      }
    }
    if (problems.length) {
      failures++;
      console.log(`  \x1b[31m✗ ${where}\x1b[0m`);
      for (const x of problems) console.log(`      ${x.replace(/\n/g, "\n      ")}`);
    }
  }
}
console.log(`\n${failures} project(s) with problems`);
process.exit(failures ? 1 : 0);
