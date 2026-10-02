// Checks every skill-tree node: lesson present and well-formed, quiz schema, playground blocks, lab presence.
import { loadPaths, type QuizQuestion } from "../server/paths.ts";

const only = process.argv.slice(2);
const { paths } = loadPaths();
let problems = 0;
const fail = (where: string, msg: string) => {
  problems++;
  console.log(`  \x1b[31m✗\x1b[0m ${where}: ${msg}`);
};

function checkQuiz(where: string, quiz: QuizQuestion[]) {
  if (!Array.isArray(quiz) || quiz.length < 2 || quiz.length > 5) fail(where, `quiz needs 2–5 questions (has ${Array.isArray(quiz) ? quiz.length : "none"})`);
  quiz.forEach((q, i) => {
    const w = `${where} quiz[${i}]`;
    if (!q.q?.trim()) fail(w, "empty question");
    if (!q.explain?.trim()) fail(w, "missing explain");
    switch (q.type) {
      case "single":
        if (!Array.isArray(q.options) || q.options.length < 2) fail(w, "needs ≥2 options");
        else if (!Number.isInteger(q.answer) || q.answer < 0 || q.answer >= q.options.length) fail(w, `answer ${q.answer} out of range`);
        break;
      case "multi":
        if (!Array.isArray(q.options) || q.options.length < 3) fail(w, "needs ≥3 options");
        else if (!Array.isArray(q.answer) || !q.answer.length || q.answer.some((a) => !Number.isInteger(a) || a < 0 || a >= q.options.length))
          fail(w, "answer must be a non-empty list of valid indices");
        break;
      case "order":
        if (!Array.isArray(q.items) || q.items.length < 3) fail(w, "needs ≥3 items");
        else if (new Set(q.items).size !== q.items.length) fail(w, "order items must be unique");
        break;
      case "number":
        if (typeof q.answer !== "number") fail(w, "answer must be a number");
        break;
      default:
        fail(w, `unknown type ${(q as { type: string }).type}`);
    }
  });
}

function checkPlaygrounds(where: string, md: string) {
  const blocks = [...md.matchAll(/```playground\n([\s\S]*?)```/g)];
  blocks.forEach((m, i) => {
    const w = `${where} playground[${i}]`;
    let cfg: { tasks?: { name?: string; priority?: number; period?: number; wcet?: number; lock?: { at: number; len: number } }[]; ticks?: number };
    try {
      cfg = JSON.parse(m[1]);
    } catch (e) {
      fail(w, `invalid JSON: ${(e as Error).message}`);
      return;
    }
    if (!Array.isArray(cfg.tasks) || !cfg.tasks.length) fail(w, "needs tasks[]");
    for (const t of cfg.tasks ?? []) {
      if (!t.name || typeof t.priority !== "number" || !(Number(t.period) > 0) || !(Number(t.wcet) > 0)) fail(w, `task ${t.name ?? "?"} needs name, priority, period>0, wcet>0`);
      if (t.lock && (t.lock.at < 0 || t.lock.len <= 0 || t.lock.at + t.lock.len > (t.wcet ?? 0))) fail(w, `task ${t.name}: lock must fit inside wcet`);
    }
    if (cfg.ticks !== undefined && !(cfg.ticks > 0 && cfg.ticks <= 400)) fail(w, "ticks must be 1–400");
  });
  return blocks.length;
}

for (const p of paths) {
  console.log(`\n\x1b[1m${p.title}\x1b[0m`);
  for (const n of p.nodes) {
    if (only.length && !only.some((o) => n.id.includes(o))) continue;
    const before = problems;
    const where = `${p.id}/${n.id}`;
    if (!n.lesson) {
      fail(where, "missing lesson.md");
      continue;
    }
    const words = n.lesson.split(/\s+/).length;
    if (words < 250) fail(where, `lesson is only ${words} words (want ~500–1200)`);
    if (!/In the wild/i.test(n.lesson)) fail(where, 'lesson needs an "In the wild" section');
    checkQuiz(where, n.quiz);
    const pg = checkPlaygrounds(where, n.lesson);
    if ((n.kind === "lab" || n.kind === "boss") && !n.labId) fail(where, `kind "${n.kind}" needs a complete lab/ folder`);
    if (n.kind === "lesson" && n.labId) fail(where, `kind "lesson" must not have a lab`);
    if (problems === before) console.log(`  \x1b[32m✓\x1b[0m ${n.id.padEnd(16)} ${String(words).padStart(5)} words  ${n.quiz.length} q${pg ? `  ${pg} playground` : ""}${n.labId ? "  + lab" : ""}`);
  }
}
console.log(`\n${problems} problem(s)`);
process.exit(problems ? 1 : 0);
