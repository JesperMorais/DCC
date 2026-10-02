import { serve } from "@hono/node-server";
import { serveStatic } from "@hono/node-server/serve-static";
import { Hono, type Context } from "hono";
import crypto from "node:crypto";
import fs from "node:fs";
import path from "node:path";
import { fileURLToPath } from "node:url";
import { isLang, LANGUAGES, LEVEL_BANDS, type Lang } from "../shared/languages.ts";
import { loadChallenges, type Challenge } from "./challenges.ts";
import {
  applyResult,
  codingMinutes,
  currentStreak,
  dailyChallenge,
  dashboard,
  expectedScore,
  levelForRating,
  newTopicsFor,
  ratingFor,
  START_RATING,
} from "./engine.ts";
import { labSolved, nodeView, pathView, progressOf, submitQuiz } from "./pathProgress.ts";
import { loadPaths } from "./paths.ts";
import { focusFor } from "./weakness.ts";
import { HARNESS_DTS } from "./runner/harness.ts";
import { runChallenge } from "./runner/index.ts";
import { load, reset, save, today, type Attempt, type Experience } from "./store.ts";

const PORT = Number(process.env.PORT ?? 4321);
const ROOT = path.resolve(path.dirname(fileURLToPath(import.meta.url)), "..");

// The daily bank and the skill-tree labs live side by side; labs are tagged bank: "path".
const loadAll = () => {
  const { paths, labs } = loadPaths();
  return { challenges: [...loadChallenges(), ...labs], paths };
};
let { challenges, paths } = loadAll();
const byId = () => new Map(challenges.map((c) => [c.id, c]));

const app = new Hono();

// Challenge files are plain folders; pick up edits without restarting in dev.
if (process.env.NODE_ENV !== "production") {
  app.use("/api/*", async (_c, next) => {
    ({ challenges, paths } = loadAll());
    await next();
  });
}

const activeAttempt = (challengeId: string) => load().attempts.find((a) => a.challengeId === challengeId && a.status === "in-progress");

const langParam = (c: Context, fallback: Lang = "typescript"): Lang => {
  const l = c.req.query("lang") ?? c.req.param("lang");
  return isLang(l) ? l : fallback;
};

/** Resolve `/:lang/:slug` to a challenge. */
const findChallenge = (c: Context) => byId().get(`${c.req.param("lang")}/${c.req.param("slug")}`);

function challengeView(c: Challenge) {
  const data = load();
  const attempts = data.attempts.filter((a) => a.challengeId === c.id);
  const active = attempts.find((a) => a.status === "in-progress");
  const everFinished = attempts.some((a) => a.status !== "in-progress");
  const best = attempts.filter((a) => a.status === "solved").sort((a, b) => (b.score ?? 0) - (a.score ?? 0))[0];
  const rating = ratingFor(data, c.language);
  return {
    id: c.id,
    slug: c.slug,
    language: c.language,
    title: c.title,
    level: c.level,
    levelName: LANGUAGES[c.language].levels[c.level],
    rating: c.rating,
    topics: c.topics,
    estMinutes: c.estMinutes,
    mode: c.mode,
    prompt: c.prompt,
    learn: c.learn,
    starter: c.starter,
    tests: c.tests,
    hintCount: c.hints.length,
    hints: c.hints.slice(0, active?.hintsUsed ?? 0),
    solution: everFinished ? c.solution : null,
    attempt: active ?? null,
    solved: attempts.some((a) => a.status === "solved"),
    bestCode: best?.code ?? null,
    isDaily: data.dailies[today()]?.[c.language] === c.id,
    expected: rating !== null ? expectedScore(rating, c.rating) : null,
    newTopics: c.bank === "path" ? [] : newTopicsFor(data, challenges, c),
    languageStarted: rating !== null || c.bank === "path",
    profile: c.profile ?? "c",
    pathNode: c.pathNode ?? null,
  };
}

app.get("/api/harness", (c) => c.text(HARNESS_DTS));

app.get("/api/state", (c) => {
  const data = load();
  if (!data.profile) return c.json({ profile: null });
  const lang = langParam(c);
  const d = dashboard(data, challenges, lang);
  save(data); // dashboard may have assigned today's daily
  return c.json({ ...d, focus: focusFor(data, paths, challenges, lang) });
});

function startLanguage(lang: Lang, experience: Experience) {
  const data = load();
  const now = new Date().toISOString();
  const p = data.profile!;
  if (!p.languages[lang]) {
    const rating = START_RATING[experience];
    p.languages[lang] = { experience, rating, startedAt: now };
    data.ratingHistory.push({ at: now, rating, language: lang });
  }
  save(data);
}

const experienceOf = (e: unknown): Experience => (typeof e === "string" && e in START_RATING ? (e as Experience) : "new");

app.post("/api/profile", async (c) => {
  const body = await c.req.json<{ name?: string; experience?: Experience; language?: string }>();
  const data = load();
  if (data.profile) {
    data.profile.name = body.name?.trim() || data.profile.name;
    save(data);
  } else {
    data.profile = { name: body.name?.trim() || "Coder", createdAt: new Date().toISOString(), languages: {} };
    save(data);
    startLanguage(isLang(body.language) ? body.language : "typescript", experienceOf(body.experience));
  }
  return c.json({ ok: true });
});

app.post("/api/languages/:lang/start", async (c) => {
  const lang = c.req.param("lang");
  if (!isLang(lang) || !load().profile) return c.json({ error: "unknown language" }, 400);
  const body = await c.req.json<{ experience?: Experience }>().catch(() => ({ experience: undefined }));
  startLanguage(lang, experienceOf(body.experience));
  return c.json({ ok: true });
});

app.post("/api/reset", (c) => {
  reset();
  return c.json({ ok: true });
});

app.get("/api/challenges", (c) => {
  const data = load();
  const lang = langParam(c);
  const status = (id: string) => {
    const as = data.attempts.filter((a) => a.challengeId === id);
    if (as.some((a) => a.status === "solved")) return "solved";
    if (as.some((a) => a.status === "in-progress")) return "in-progress";
    if (as.some((a) => a.status === "gave-up")) return "gave-up";
    return "new";
  };
  return c.json({
    language: lang,
    levels: Object.entries(LEVEL_BANDS).map(([k, band]) => ({ level: Number(k), name: LANGUAGES[lang].levels[Number(k)], band })),
    rating: ratingFor(data, lang),
    challenges: challenges
      .filter((ch) => ch.language === lang && ch.bank !== "path")
      .map((ch) => ({
        id: ch.id,
        slug: ch.slug,
        language: ch.language,
        title: ch.title,
        level: ch.level,
        rating: ch.rating,
        topics: ch.topics,
        estMinutes: ch.estMinutes,
        mode: ch.mode,
        status: status(ch.id),
      })),
  });
});

app.get("/api/challenges/:lang/:slug", (c) => {
  const ch = findChallenge(c);
  if (!ch) return c.json({ error: "not found" }, 404);
  return c.json(challengeView(ch));
});

app.post("/api/challenges/:lang/:slug/start", (c) => {
  const ch = findChallenge(c);
  const data = load();
  if (!ch || !data.profile) return c.json({ error: "not found" }, 404);
  if (!data.profile.languages[ch.language] && ch.bank !== "path") return c.json({ error: "language not started" }, 409);
  if (!activeAttempt(ch.id)) {
    dailyChallenge(data, challenges, ch.language);
    const date = today();
    const attempt: Attempt = {
      id: crypto.randomUUID(),
      challengeId: ch.id,
      language: ch.language,
      date,
      startedAt: new Date().toISOString(),
      status: "in-progress",
      hintsUsed: 0,
      runs: 0,
      isDaily: data.dailies[date]?.[ch.language] === ch.id,
      // Tree labs are progression, not skill rating.
      rated: ch.bank !== "path" && !data.attempts.some((a) => a.challengeId === ch.id && a.status !== "in-progress"),
    };
    data.attempts.push(attempt);
    save(data);
  }
  return c.json(challengeView(ch));
});

// First keystroke: start the clock (idempotent, so the first call wins).
app.post("/api/challenges/:lang/:slug/begin", (c) => {
  const ch = findChallenge(c);
  const a = ch && activeAttempt(ch.id);
  if (!ch || !a) return c.json({ error: "no active attempt" }, 400);
  if (!a.codingStartedAt) {
    a.codingStartedAt = new Date().toISOString();
    save(load());
  }
  return c.json({ codingStartedAt: a.codingStartedAt });
});

// Pause / resume the clock. Both are idempotent; the client fires "away" pauses on tab hide and close.
app.post("/api/challenges/:lang/:slug/pause", (c) => {
  const ch = findChallenge(c);
  const a = ch && activeAttempt(ch.id);
  if (!ch || !a) return c.json({ error: "no active attempt" }, 400);
  const reason = c.req.query("reason") === "away" ? "away" : "manual";
  if (a.codingStartedAt && !a.pausedAt) {
    a.pausedAt = new Date().toISOString();
    a.pauseReason = reason;
    save(load());
  } else if (a.pausedAt && reason === "manual" && a.pauseReason !== "manual") {
    a.pauseReason = "manual"; // a deliberate pause shouldn't auto-resume on return
    save(load());
  }
  return c.json(clock(a));
});

app.post("/api/challenges/:lang/:slug/resume", (c) => {
  const ch = findChallenge(c);
  const a = ch && activeAttempt(ch.id);
  if (!ch || !a) return c.json({ error: "no active attempt" }, 400);
  if (resumeClock(a)) save(load());
  return c.json(clock(a));
});

app.post("/api/challenges/:lang/:slug/hint", (c) => {
  const ch = findChallenge(c);
  const a = ch && activeAttempt(ch.id);
  if (!ch || !a) return c.json({ error: "no active attempt" }, 400);
  a.hintsUsed = Math.min(ch.hints.length, a.hintsUsed + 1);
  save(load());
  return c.json({ hints: ch.hints.slice(0, a.hintsUsed) });
});

app.post("/api/challenges/:lang/:slug/run", async (c) => {
  const ch = findChallenge(c);
  if (!ch) return c.json({ error: "not found" }, 404);
  const { code } = await c.req.json<{ code: string }>();
  const result = await runChallenge(ch.language, String(code ?? ""), ch.tests, ch.mode, ch.profile);
  const a = activeAttempt(ch.id);
  if (a) {
    a.runs++;
    save(load());
  }
  return c.json(result);
});

function resumeClock(a: Attempt) {
  if (!a.pausedAt) return false;
  a.pausedMs = (a.pausedMs ?? 0) + Math.max(0, Date.now() - Date.parse(a.pausedAt));
  delete a.pausedAt;
  delete a.pauseReason;
  return true;
}

const clock = (a: Attempt) => ({
  codingStartedAt: a.codingStartedAt ?? null,
  pausedMs: a.pausedMs ?? 0,
  pausedAt: a.pausedAt ?? null,
  pauseReason: a.pauseReason ?? null,
});

function finish(ch: Challenge, a: Attempt, status: "solved" | "gave-up", code: string) {
  const data = load();
  resumeClock(a);
  const solvedTodayBefore = data.attempts.some((x) => x.status === "solved" && x.date === today());
  a.status = status;
  a.finishedAt = new Date().toISOString();
  a.code = code;
  if (ch.bank !== "path") applyResult(data, a, ch);
  let tree: ReturnType<typeof labSolved> | null = null;
  if (ch.pathNode && status === "solved") {
    const p = paths.find((x) => x.id === ch.pathNode!.path);
    const node = p?.nodes.find((n) => n.id === ch.pathNode!.node);
    if (p && node) tree = labSolved(data, p, node, a.hintsUsed);
  }
  save(data);
  const rating = ratingFor(data, ch.language) ?? 0;
  return {
    status,
    score: a.score,
    rated: a.rated,
    ratingBefore: a.ratingBefore ?? rating,
    ratingAfter: a.ratingAfter ?? rating,
    minutes: codingMinutes(a),
    hintsUsed: a.hintsUsed,
    solution: ch.solution,
    levelBefore: levelForRating(ch.language, a.ratingBefore ?? rating).level,
    levelAfter: levelForRating(ch.language, rating).level,
    levelName: levelForRating(ch.language, rating).name,
    streak: currentStreak(data),
    firstSolveToday: status === "solved" && !solvedTodayBefore,
    tree: tree && { ...tree, path: ch.pathNode!.path, node: ch.pathNode!.node },
  };
}

app.post("/api/challenges/:lang/:slug/submit", async (c) => {
  const ch = findChallenge(c);
  const a = ch && activeAttempt(ch.id);
  if (!ch || !a) return c.json({ error: "no active attempt" }, 400);
  const { code } = await c.req.json<{ code: string }>();
  const result = await runChallenge(ch.language, String(code ?? ""), ch.tests, ch.mode, ch.profile);
  if (!result.passed) return c.json({ error: "Not all checks pass yet", result }, 400);
  return c.json({ ...finish(ch, a, "solved", code), result });
});

app.post("/api/challenges/:lang/:slug/giveup", async (c) => {
  const ch = findChallenge(c);
  const a = ch && activeAttempt(ch.id);
  if (!ch || !a) return c.json({ error: "no active attempt" }, 400);
  const { code } = await c.req.json<{ code: string }>().catch(() => ({ code: "" }));
  return c.json(finish(ch, a, "gave-up", String(code ?? "")));
});

/* ---------------- skill trees ---------------- */
const labsById = () => new Map(challenges.filter((c) => c.bank === "path").map((c) => [c.id, c]));

app.get("/api/paths", (c) => {
  const data = load();
  return c.json(paths.map((p) => {
    const v = pathView(data, p, labsById(), challenges);
    return { id: v.id, title: v.title, tagline: v.tagline, language: v.language, xp: v.xp, rank: v.rank, stars: v.stars, maxStars: v.maxStars, sections: v.sections };
  }));
});

app.get("/api/paths/:path", (c) => {
  const p = paths.find((x) => x.id === c.req.param("path"));
  if (!p) return c.json({ error: "not found" }, 404);
  return c.json(pathView(load(), p, labsById(), challenges));
});

app.post("/api/paths/:path/branch", async (c) => {
  const p = paths.find((x) => x.id === c.req.param("path"));
  const { branch } = await c.req.json<{ branch: string }>();
  if (!p || !p.sections.some((s) => s.id === branch)) return c.json({ error: "unknown branch" }, 400);
  const data = load();
  progressOf(data, p.id).branch = branch;
  save(data);
  return c.json({ ok: true });
});

app.get("/api/paths/:path/nodes/:node", (c) => {
  const p = paths.find((x) => x.id === c.req.param("path"));
  const node = p?.nodes.find((n) => n.id === c.req.param("node"));
  if (!p || !node) return c.json({ error: "not found" }, 404);
  return c.json(nodeView(load(), p, node, labsById(), challenges));
});

app.post("/api/paths/:path/nodes/:node/quiz", async (c) => {
  const p = paths.find((x) => x.id === c.req.param("path"));
  const node = p?.nodes.find((n) => n.id === c.req.param("node"));
  if (!p || !node) return c.json({ error: "not found" }, 404);
  const { answers } = await c.req.json<{ answers: unknown[] }>();
  const data = load();
  if (pathView(data, p, labsById(), challenges).nodes.find((n) => n.id === node.id)?.status === "locked") return c.json({ error: "locked" }, 403);
  const r = submitQuiz(data, p, node, Array.isArray(answers) ? answers : []);
  save(data);
  return c.json(r);
});

// Production: serve the built SPA.
const dist = path.join(ROOT, "dist");
if (fs.existsSync(dist)) {
  app.use("/*", serveStatic({ root: path.relative(process.cwd(), dist) }));
  app.get("*", (c) => c.html(fs.readFileSync(path.join(dist, "index.html"), "utf8")));
}

serve({ fetch: app.fetch, port: PORT, hostname: "127.0.0.1" }, () => {
  console.log(`\n  daily.ts  →  http://localhost:${PORT}${process.env.NODE_ENV === "production" ? "" : "  (API; UI on vite)"}\n`);
});
