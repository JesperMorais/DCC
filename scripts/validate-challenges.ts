// Quality gate for every challenge in every language:
//  - meta is sane and the rating sits inside its level's band
//  - the reference solution passes AND is warning-free (gcc -Wall -Wextra, mypy --strict)
//  - the starter compiles/parses on its own but does NOT pass (so the tests test something)
// usage: npm run validate [-- filter ...]   e.g. `-- python/ c/l3`
import { loadChallenges, LEVEL_BANDS, type Challenge } from "../server/challenges.ts";
import { runChallenge } from "../server/runner/index.ts";

const only = process.argv.slice(2);
const all = loadChallenges().filter((c) => only.length === 0 || only.some((o) => c.id.includes(o)));

async function check(c: Challenge): Promise<string[]> {
  const problems: string[] = [];
  const band = LEVEL_BANDS[c.level];
  if (!band) problems.push(`invalid level ${c.level}`);
  else if (c.rating < band[0] || c.rating > band[1]) problems.push(`rating ${c.rating} outside level ${c.level} band ${band.join("–")}`);
  if (!c.title || !c.topics?.length || !c.hints?.length) problems.push("missing title/topics/hints");
  if (c.hints && c.hints.length < 2) problems.push("want at least 2 hints");
  if (!c.learn.trim() || !c.prompt.trim()) problems.push("empty prompt.md or learn.md");
  if (c.mode === "types" && c.language !== "typescript") problems.push(`mode "types" is TypeScript-only`);
  if (c.language === "typescript" && /\b(import|export)\b/.test(c.starter + c.solution + c.tests)) problems.push("must not use import/export (files are global scripts)");
  if (c.language === "c" && /\bint\s+main\s*\(/.test(c.starter + c.solution)) problems.push("starter/solution must not define main()");
  if (c.language === "python" && /^\s*(from\s+\S+\s+)?import\s+(pytest|unittest)\b/m.test(c.tests)) problems.push("tests use the built-in harness (raises/approx), not pytest imports");

  const sol = await runChallenge(c.language, c.solution, c.tests, c.mode);
  if (!sol.passed) {
    problems.push("reference solution FAILS");
    for (const e of sol.diagnostics.filter((d) => d.severity === "error")) problems.push(`  ${e.tool} [${e.file}:${e.line}] ${e.message}`);
    for (const e of sol.setupErrors) problems.push(`  setup [${e.file}] ${e.message}`);
    for (const t of sol.tests.filter((t) => !t.pass)) problems.push(`  test "${t.name}": ${t.error?.message}`);
  }
  const warnings = sol.diagnostics.filter((d) => d.severity === "warning");
  for (const w of warnings) problems.push(`solution warning ${w.tool} [${w.file}:${w.line}] ${w.message}`);
  if (c.mode === "runtime" && sol.passed && sol.tests.length < 3) problems.push(`only ${sol.tests.length} tests (want ≥3)`);

  const start = await runChallenge(c.language, c.starter, c.tests, c.mode);
  if (start.passed) problems.push("starter code already passes — tests are too weak");
  const starterErrors = start.diagnostics.filter((e) => e.file === "your-code" && e.severity === "error");
  if (starterErrors.length) problems.push(`starter has errors: ${starterErrors.map((e) => e.message).join("; ")}`);
  if (start.setupErrors.some((e) => /Linker error/.test(e.message))) problems.push(`starter doesn't link: ${start.setupErrors[0].message}`);

  return problems.length ? problems : [`ok ${sol.tests.length} tests ${sol.durationMs}ms`];
}

// C compiles are the slow part; run a few at a time.
const CONCURRENCY = 4;
const results = new Map<string, string[]>();
let next = 0;
await Promise.all(
  Array.from({ length: CONCURRENCY }, async () => {
    while (next < all.length) {
      const c = all[next++];
      results.set(c.id, await check(c));
    }
  }),
);

let failures = 0;
let lang = "";
for (const c of all) {
  if (c.language !== lang) {
    lang = c.language;
    console.log(`\n\x1b[1m${lang}\x1b[0m`);
  }
  const r = results.get(c.id)!;
  if (r.length === 1 && r[0].startsWith("ok ")) console.log(`  \x1b[32m✓\x1b[0m ${c.slug.padEnd(32)} L${c.level} ${c.rating}  ${r[0].slice(3)}`);
  else {
    failures++;
    console.log(`  \x1b[31m✗ ${c.slug}\x1b[0m`);
    for (const p of r) console.log(`      ${p}`);
  }
}
const byLang = all.reduce<Record<string, number>>((m, c) => ((m[c.language] = (m[c.language] ?? 0) + 1), m), {});
console.log(`\n${all.length} challenges (${Object.entries(byLang).map(([k, v]) => `${k} ${v}`).join(", ")}), ${failures} with problems`);
process.exit(failures ? 1 : 0);
