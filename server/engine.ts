// The adaptive part: an Elo-style skill rating per language, challenge selection and dashboard stats.
import { LANG_IDS, LANGUAGES, LEVEL_BANDS, type Lang } from "../shared/languages.ts";
import type { Challenge } from "./challenges.ts";
import { today, type Attempt, type Data, type Experience } from "./store.ts";

export const START_RATING: Record<Experience, number> = {
  new: 720,
  "other-lang": 880,
  js: 1020,
  "some-ts": 1160,
  pro: 1340,
};

const DAY = 86_400_000;
const TARGET_MINUTES = 10;

/** Time spent paused (ms), including a pause that is still running. */
export const pausedMs = (a: Attempt, now = Date.now()) =>
  (a.pausedMs ?? 0) + (a.pausedAt ? Math.max(0, Date.parse(a.finishedAt ?? new Date(now).toISOString()) - Date.parse(a.pausedAt)) : 0);

/** Minutes spent coding: from the first edit (older attempts: from opening) to finishing, minus pauses. */
export const codingMinutes = (a: Attempt) =>
  Math.max(0, (Date.parse(a.finishedAt ?? new Date().toISOString()) - Date.parse(a.codingStartedAt ?? a.startedAt) - pausedMs(a)) / 60_000);

export const ratingFor = (data: Data, lang: Lang) => data.profile?.languages[lang]?.rating ?? null;

/** Probability the learner "beats" a challenge, classic Elo curve. */
export const expectedScore = (rating: number, difficulty: number) => 1 / (1 + 10 ** ((difficulty - rating) / 400));

/**
 * How well did it go, 0..1. Solving counts for a lot; hints and running long shave a bit off.
 * A solve never scores below 0.5, so finishing a well-matched challenge never costs rating.
 */
export function performanceScore(a: Attempt, c: Challenge): number {
  if (a.status !== "solved") return 0;
  const minutes = codingMinutes(a);
  const overtime = Math.max(0, minutes - Math.max(TARGET_MINUTES, c.estMinutes));
  const timePenalty = Math.min(0.2, (overtime / 20) * 0.2);
  const hintPenalty = Math.min(c.hints.length, a.hintsUsed) * 0.12;
  return Math.max(0.5, 1 - hintPenalty - timePenalty);
}

function kFactor(ratedCount: number) {
  if (ratedCount < 5) return 64; // calibrate fast in the first few days of each language
  if (ratedCount < 15) return 40;
  return 28;
}

export function applyResult(data: Data, a: Attempt, c: Challenge) {
  const progress = data.profile!.languages[c.language]!;
  a.score = performanceScore(a, c);
  if (!a.rated) return;
  const ratedCount = data.attempts.filter((x) => x.language === c.language && x.rated && x.status !== "in-progress" && x.id !== a.id).length;
  const delta = kFactor(ratedCount) * (a.score - expectedScore(progress.rating, c.rating));
  a.ratingBefore = progress.rating;
  progress.rating = Math.round(Math.max(600, Math.min(1800, progress.rating + delta)));
  a.ratingAfter = progress.rating;
  data.ratingHistory.push({ at: a.finishedAt!, rating: progress.rating, language: c.language, challengeId: c.id });
}

function hash(s: string) {
  let h = 2166136261;
  for (let i = 0; i < s.length; i++) h = Math.imul(h ^ s.charCodeAt(i), 16777619);
  return (h >>> 0) / 2 ** 32;
}

const finished = (a: Attempt) => a.status !== "in-progress";

/**
 * Pick the best next challenge in a language: close to (slightly above) the learner's rating,
 * not recently seen, leaning towards topics they're weakest at.
 */
export function pickChallenge(data: Data, all: Challenge[], lang: Lang, exclude: Set<string> = new Set()): Challenge | null {
  const challenges = all.filter((c) => c.language === lang);
  const rating = ratingFor(data, lang) ?? 900;
  const target = rating + 30; // a gentle stretch
  const now = Date.now();
  const done = data.attempts.filter((a) => a.language === lang && finished(a));

  const lastFinish = new Map<string, Attempt>();
  for (const a of done) lastFinish.set(a.challengeId, a);

  const recentTopics = new Set(done.slice(-3).flatMap((a) => challenges.find((c) => c.id === a.challengeId)?.topics ?? []));
  const topicScores = topicStats(data, all, lang);

  const eligible = (c: Challenge) => {
    if (exclude.has(c.id)) return false;
    const last = lastFinish.get(c.id);
    if (!last) return true;
    // Given-up challenges come back around after a week.
    return last.status === "gave-up" && now - Date.parse(last.finishedAt!) > 7 * DAY;
  };

  let pool = challenges.filter(eligible);
  if (pool.length === 0) {
    // Everything done: recycle the ones touched longest ago.
    pool = challenges
      .filter((c) => !exclude.has(c.id))
      .sort((a, b) => Date.parse(lastFinish.get(a.id)?.finishedAt ?? "0") - Date.parse(lastFinish.get(b.id)?.finishedAt ?? "0"))
      .slice(0, 10);
  }
  if (pool.length === 0) return null;

  const seed = today();
  const cost = (c: Challenge) => {
    let s = Math.abs(c.rating - target);
    if (c.rating > rating + 200) s += 150; // don't jump too far ahead
    s += 35 * c.topics.filter((t) => recentTopics.has(t)).length;
    const weak = c.topics.some((t) => {
      const st = topicScores.find((x) => x.topic === t);
      return st && st.attempts >= 1 && st.avgScore < 0.6;
    });
    if (weak) s -= 30;
    return s + hash(seed + c.id) * 30;
  };
  return pool.reduce((best, c) => (cost(c) < cost(best) ? c : best));
}

export function dailyChallenge(data: Data, challenges: Challenge[], lang: Lang): Challenge | null {
  const d = today();
  const existing = data.dailies[d]?.[lang];
  const found = existing && challenges.find((c) => c.id === existing);
  if (found) return found;
  if (!data.profile?.languages[lang]) return null; // pick only once the language is started
  const inProgress = new Set(data.attempts.filter((a) => a.status === "in-progress").map((a) => a.challengeId));
  const c = pickChallenge(data, challenges, lang, inProgress) ?? pickChallenge(data, challenges, lang);
  if (c) data.dailies[d] = { ...data.dailies[d], [lang]: c.id };
  return c;
}

export function levelForRating(lang: Lang, rating: number) {
  const entries = Object.entries(LEVEL_BANDS).map(([k, band]) => ({ level: Number(k), band }));
  const current = entries.find((l) => rating < l.band[1]) ?? entries[entries.length - 1];
  const [lo, hi] = current.band;
  return {
    level: current.level,
    name: LANGUAGES[lang].levels[current.level],
    progress: Math.max(0, Math.min(1, (rating - lo) / (hi - lo))),
    nextAt: hi,
  };
}

export function topicStats(data: Data, challenges: Challenge[], lang: Lang) {
  const byId = new Map(challenges.map((c) => [c.id, c]));
  const m = new Map<string, { attempts: number; solved: number; total: number }>();
  for (const a of data.attempts.filter((x) => x.language === lang && finished(x))) {
    for (const t of byId.get(a.challengeId)?.topics ?? []) {
      const s = m.get(t) ?? { attempts: 0, solved: 0, total: 0 };
      s.attempts++;
      if (a.status === "solved") s.solved++;
      s.total += a.score ?? 0;
      m.set(t, s);
    }
  }
  return [...m.entries()]
    .map(([topic, s]) => ({ topic, attempts: s.attempts, solved: s.solved, avgScore: s.total / s.attempts }))
    .sort((a, b) => b.attempts - a.attempts || b.avgScore - a.avgScore);
}

export function streaks(solvedDays: Set<string>) {
  const dayStr = (offset: number) => today(new Date(Date.now() - offset * DAY));
  // Current streak counts back from today, or from yesterday if today isn't done yet.
  const start = solvedDays.has(dayStr(0)) ? 0 : 1;
  let current = 0;
  while (solvedDays.has(dayStr(start + current))) current++;

  const sorted = [...solvedDays].sort();
  let best = 0;
  let run = 0;
  let prev: number | null = null;
  for (const d of sorted) {
    const t = Date.parse(d);
    run = prev !== null && Math.round((t - prev) / DAY) === 1 ? run + 1 : 1;
    best = Math.max(best, run);
    prev = t;
  }
  return { current, best: Math.max(best, current) };
}

/** Streaks are global: a day you practised any language counts. */
export function currentStreak(data: Data) {
  return streaks(new Set(data.attempts.filter((a) => a.status === "solved").map((a) => a.date))).current;
}

/** Topics of a challenge the learner has never finished a challenge in (within that language). */
export function newTopicsFor(data: Data, challenges: Challenge[], c: Challenge) {
  const seen = new Set(topicStats(data, challenges, c.language).map((t) => t.topic));
  return c.topics.filter((t) => !seen.has(t));
}

export function languageSummaries(data: Data, challenges: Challenge[]) {
  return LANG_IDS.map((lang) => {
    const p = data.profile?.languages[lang];
    const solved = new Set(data.attempts.filter((a) => a.language === lang && a.status === "solved").map((a) => a.challengeId));
    const d = data.dailies[today()]?.[lang];
    const dailyDone = !!d && data.attempts.some((a) => a.challengeId === d && a.date === today() && a.status !== "in-progress");
    return {
      id: lang,
      started: !!p,
      rating: p?.rating ?? null,
      level: p ? levelForRating(lang, p.rating).level : null,
      solved: solved.size,
      total: challenges.filter((c) => c.language === lang).length,
      dailyDone,
    };
  });
}

export function dashboard(data: Data, challenges: Challenge[], lang: Lang) {
  const inLang = challenges.filter((c) => c.language === lang);
  const byId = new Map(challenges.map((c) => [c.id, c]));
  const all = data.attempts.filter(finished);
  const done = all.filter((a) => a.language === lang);
  const solved = done.filter((a) => a.status === "solved");
  const solvedIds = new Set(solved.map((a) => a.challengeId));
  const allSolvedDays = new Set(all.filter((a) => a.status === "solved").map((a) => a.date));
  const rating = ratingFor(data, lang) ?? 0;
  const history = data.ratingHistory.filter((p) => p.language === lang);

  const daily = dailyChallenge(data, challenges, lang);
  const todayStr = today();
  const dailyAttempt = daily ? [...data.attempts].reverse().find((a) => a.challengeId === daily.id && a.date === todayStr) : undefined;

  const solveMinutes = solved.map(codingMinutes);
  const weekAgo = Date.now() - 7 * DAY;
  const ratingWeekAgo = [...history].reverse().find((p) => Date.parse(p.at) <= weekAgo)?.rating ?? history[0]?.rating ?? rating;

  // The heatmap shows every language, so the streak it illustrates is the real one.
  const heatDays = 7 * 20;
  const heatmap = Array.from({ length: heatDays }, (_, i) => {
    const date = today(new Date(Date.now() - (heatDays - 1 - i) * DAY));
    return { date, solved: all.filter((a) => a.status === "solved" && a.date === date).length };
  });

  const inProgress = data.attempts.filter((a) => a.status === "in-progress").map((a) => a.challengeId);
  const next = dailyAttempt && dailyAttempt.status !== "in-progress" ? pickChallenge(data, challenges, lang, new Set([daily!.id, ...inProgress])) : null;

  const summary = (c: Challenge | null | undefined) =>
    c && {
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
    };
  const { current, best } = streaks(allSolvedDays);

  return {
    profile: data.profile && { name: data.profile.name, createdAt: data.profile.createdAt },
    language: lang,
    started: !!data.profile?.languages[lang],
    languages: languageSummaries(data, challenges),
    level: levelForRating(lang, rating),
    stats: {
      rating,
      ratingDelta7d: rating - ratingWeekAgo,
      streak: current,
      bestStreak: best,
      solved: solvedIds.size,
      totalChallenges: inLang.length,
      attempts: done.length,
      successRate: done.length ? solved.length / done.length : null,
      medianMinutes: solveMinutes.length ? solveMinutes.sort((a, b) => a - b)[Math.floor(solveMinutes.length / 2)] : null,
      hintsPerSolve: solved.length ? solved.reduce((s, a) => s + a.hintsUsed, 0) / solved.length : null,
      solvedToday: allSolvedDays.has(todayStr),
      solvedTodayHere: solved.some((a) => a.date === todayStr),
    },
    daily: daily && {
      ...summary(daily)!,
      status: dailyAttempt?.status ?? "not-started",
      expected: expectedScore(rating, daily.rating),
    },
    next: summary(next),
    ratingHistory: history,
    heatmap,
    topics: topicStats(data, challenges, lang),
    levels: Object.keys(LEVEL_BANDS).map((k) => {
      const level = Number(k);
      const inLevel = inLang.filter((c) => c.level === level);
      return { level, name: LANGUAGES[lang].levels[level], total: inLevel.length, solved: inLevel.filter((c) => solvedIds.has(c.id)).length };
    }),
    recent: [...done]
      .reverse()
      .slice(0, 8)
      .map((a) => ({
        id: a.id,
        challengeId: a.challengeId,
        title: byId.get(a.challengeId)?.title ?? a.challengeId,
        level: byId.get(a.challengeId)?.level ?? 0,
        status: a.status,
        finishedAt: a.finishedAt!,
        minutes: codingMinutes(a),
        hintsUsed: a.hintsUsed,
        ratingChange: a.ratingAfter !== undefined && a.ratingBefore !== undefined ? a.ratingAfter - a.ratingBefore : null,
        isDaily: a.isDaily,
      })),
  };
}

export type Dashboard = ReturnType<typeof dashboard>;
