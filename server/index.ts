import { serve } from "@hono/node-server";
import { serveStatic } from "@hono/node-server/serve-static";
import { Hono } from "hono";
import crypto from "node:crypto";
import fs from "node:fs";
import path from "node:path";
import { fileURLToPath } from "node:url";
import { loadChallenges, LEVELS, type Challenge } from "./challenges.ts";
import { applyResult, dashboard, dailyChallenge, expectedScore, START_RATING } from "./engine.ts";
import { runChallenge } from "./runner/index.ts";
import { HARNESS_DTS } from "./runner/harness.ts";
import { load, reset, save, today, type Attempt, type Experience } from "./store.ts";

const PORT = Number(process.env.PORT ?? 4321);
const ROOT = path.resolve(path.dirname(fileURLToPath(import.meta.url)), "..");

let challenges = loadChallenges();
const byId = () => new Map(challenges.map((c) => [c.id, c]));

const app = new Hono();

// Challenge files are plain folders; pick up edits without restarting in dev.
if (process.env.NODE_ENV !== "production") {
  app.use("/api/*", async (_c, next) => {
    challenges = loadChallenges();
    await next();
  });
}

const activeAttempt = (challengeId: string) =>
  load().attempts.find((a) => a.challengeId === challengeId && a.status === "in-progress");

function challengeView(c: Challenge) {
  const data = load();
  const attempts = data.attempts.filter((a) => a.challengeId === c.id);
  const active = attempts.find((a) => a.status === "in-progress");
  const everFinished = attempts.some((a) => a.status !== "in-progress");
  const best = attempts.filter((a) => a.status === "solved").sort((a, b) => (b.score ?? 0) - (a.score ?? 0))[0];
  return {
    id: c.id,
    title: c.title,
    level: c.level,
    levelName: LEVELS[c.level].name,
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
    isDaily: data.dailies[today()] === c.id,
    expected: data.profile ? expectedScore(data.profile.rating, c.rating) : null,
  };
}

app.get("/api/harness", (c) => c.text(HARNESS_DTS));

app.get("/api/state", (c) => {
  const data = load();
  if (!data.profile) return c.json({ profile: null });
  const d = dashboard(data, challenges);
  save(data); // dashboard may have assigned today's daily
  return c.json(d);
});

app.post("/api/profile", async (c) => {
  const body = await c.req.json<{ name?: string; experience?: Experience }>();
  const experience = body.experience && body.experience in START_RATING ? body.experience : "new";
  const data = load();
  const now = new Date().toISOString();
  const rating = START_RATING[experience];
  if (data.profile) {
    data.profile.name = body.name?.trim() || data.profile.name;
  } else {
    data.profile = { name: body.name?.trim() || "Coder", experience, createdAt: now, rating };
    data.ratingHistory = [{ at: now, rating }];
  }
  save(data);
  return c.json({ ok: true });
});

app.post("/api/reset", (c) => {
  reset();
  return c.json({ ok: true });
});

app.get("/api/challenges", (c) => {
  const data = load();
  const status = (id: string) => {
    const as = data.attempts.filter((a) => a.challengeId === id);
    if (as.some((a) => a.status === "solved")) return "solved";
    if (as.some((a) => a.status === "in-progress")) return "in-progress";
    if (as.some((a) => a.status === "gave-up")) return "gave-up";
    return "new";
  };
  return c.json({
    levels: Object.entries(LEVELS).map(([k, v]) => ({ level: Number(k), name: v.name, band: v.band })),
    rating: data.profile?.rating ?? null,
    challenges: challenges.map((ch) => ({
      id: ch.id,
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

app.get("/api/challenges/:id", (c) => {
  const ch = byId().get(c.req.param("id"));
  if (!ch) return c.json({ error: "not found" }, 404);
  return c.json(challengeView(ch));
});

app.post("/api/challenges/:id/start", (c) => {
  const ch = byId().get(c.req.param("id"));
  const data = load();
  if (!ch || !data.profile) return c.json({ error: "not found" }, 404);
  if (!activeAttempt(ch.id)) {
    dailyChallenge(data, challenges);
    const date = today();
    const attempt: Attempt = {
      id: crypto.randomUUID(),
      challengeId: ch.id,
      date,
      startedAt: new Date().toISOString(),
      status: "in-progress",
      hintsUsed: 0,
      runs: 0,
      isDaily: data.dailies[date] === ch.id,
      rated: !data.attempts.some((a) => a.challengeId === ch.id && a.status !== "in-progress"),
    };
    data.attempts.push(attempt);
    save(data);
  }
  return c.json(challengeView(ch));
});

app.post("/api/challenges/:id/hint", (c) => {
  const ch = byId().get(c.req.param("id"));
  const a = ch && activeAttempt(ch.id);
  if (!ch || !a) return c.json({ error: "no active attempt" }, 400);
  a.hintsUsed = Math.min(ch.hints.length, a.hintsUsed + 1);
  save(load());
  return c.json({ hints: ch.hints.slice(0, a.hintsUsed) });
});

app.post("/api/challenges/:id/run", async (c) => {
  const ch = byId().get(c.req.param("id"));
  if (!ch) return c.json({ error: "not found" }, 404);
  const { code } = await c.req.json<{ code: string }>();
  const result = await runChallenge(String(code ?? ""), ch.tests, ch.mode);
  const a = activeAttempt(ch.id);
  if (a) {
    a.runs++;
    save(load());
  }
  return c.json(result);
});

function finish(ch: Challenge, a: Attempt, status: "solved" | "gave-up", code: string) {
  const data = load();
  a.status = status;
  a.finishedAt = new Date().toISOString();
  a.code = code;
  applyResult(data, a, ch);
  save(data);
  return {
    status,
    score: a.score,
    rated: a.rated,
    ratingBefore: a.ratingBefore ?? data.profile!.rating,
    ratingAfter: a.ratingAfter ?? data.profile!.rating,
    minutes: (Date.parse(a.finishedAt) - Date.parse(a.startedAt)) / 60_000,
    hintsUsed: a.hintsUsed,
    solution: ch.solution,
  };
}

app.post("/api/challenges/:id/submit", async (c) => {
  const ch = byId().get(c.req.param("id"));
  const a = ch && activeAttempt(ch.id);
  if (!ch || !a) return c.json({ error: "no active attempt" }, 400);
  const { code } = await c.req.json<{ code: string }>();
  const result = await runChallenge(String(code ?? ""), ch.tests, ch.mode);
  if (!result.passed) return c.json({ error: "Not all checks pass yet", result }, 400);
  return c.json({ ...finish(ch, a, "solved", code), result });
});

app.post("/api/challenges/:id/giveup", async (c) => {
  const ch = byId().get(c.req.param("id"));
  const a = ch && activeAttempt(ch.id);
  if (!ch || !a) return c.json({ error: "no active attempt" }, 400);
  const { code } = await c.req.json<{ code: string }>().catch(() => ({ code: "" }));
  return c.json(finish(ch, a, "gave-up", String(code ?? "")));
});

// Production: serve the built SPA.
const dist = path.join(ROOT, "dist");
if (fs.existsSync(dist)) {
  app.use("/*", serveStatic({ root: path.relative(process.cwd(), dist) }));
  app.get("*", (c) => c.html(fs.readFileSync(path.join(dist, "index.html"), "utf8")));
}

serve({ fetch: app.fetch, port: PORT, hostname: "127.0.0.1" }, () => {
  console.log(`\n  Daily TS  →  http://localhost:${PORT}${process.env.NODE_ENV === "production" ? "" : "  (API; UI on vite)"}\n`);
});
