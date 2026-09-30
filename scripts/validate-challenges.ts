// Checks every challenge: meta is sane, the reference solution passes,
// and the starter does NOT pass (so the tests actually test something).
import { loadChallenges, LEVELS, type Challenge } from "../server/challenges.ts";
import { runChallenge } from "../server/runner/index.ts";

const only = process.argv.slice(2);
const all = loadChallenges().filter((c) => only.length === 0 || only.some((o) => c.id.includes(o)));
let failures = 0;

function problem(c: Challenge, msg: string) {
  failures++;
  console.log(`  \x1b[31m✗ ${c.id}\x1b[0m ${msg}`);
}

for (const c of all) {
  const before = failures;
  const band = LEVELS[c.level]?.band;
  if (!band) problem(c, `invalid level ${c.level}`);
  else if (c.rating < band[0] || c.rating > band[1]) problem(c, `rating ${c.rating} outside level ${c.level} band ${band.join("–")}`);
  if (!c.title || !c.topics?.length || !c.hints?.length) problem(c, "missing title/topics/hints");
  if (!["runtime", "types"].includes(c.mode)) problem(c, `bad mode ${c.mode}`);
  if (/\b(import|export)\b/.test(c.starter + c.solution + c.tests)) problem(c, "must not use import/export (files are global scripts)");

  const sol = await runChallenge(c.solution, c.tests, c.mode);
  if (!sol.passed) {
    problem(c, "reference solution FAILS");
    for (const e of sol.typeErrors) console.log(`      type [${e.file}:${e.line}] ${e.message}`);
    for (const e of sol.setupErrors) console.log(`      setup [${e.file}] ${e.message}`);
    for (const t of sol.tests.filter((t) => !t.pass)) console.log(`      test "${t.name}": ${t.error?.message}`);
  } else if (c.mode === "runtime" && sol.tests.length < 3) problem(c, `only ${sol.tests.length} tests (want ≥3)`);

  const start = await runChallenge(c.starter, c.tests, c.mode);
  if (start.passed) problem(c, "starter code already passes — tests are too weak");
  // Starter should compile on its own, so the learner begins from a clean slate.
  const starterOwnErrors = start.typeErrors.filter((e) => e.file === "your-code");
  if (starterOwnErrors.length) problem(c, `starter has type errors: ${starterOwnErrors.map((e) => e.message).join("; ")}`);

  if (failures === before) console.log(`  \x1b[32m✓\x1b[0m ${c.id.padEnd(34)} L${c.level} ${c.rating}  ${sol.tests.length} tests  ${sol.durationMs}ms`);
}
console.log(`\n${all.length} challenges, ${failures} problem(s)`);
process.exit(failures ? 1 : 0);
