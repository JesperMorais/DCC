import type { Lang } from "../shared/languages.ts";
import type { Challenge } from "./challenges.ts";
import { today, type Attempt, type Data } from "./store.ts";

const DAY = 86_400_000;

/**
 * Spaced review, Leitner style. Each challenge sits in a box. The box sets how long until you
 * should write it again from a blank editor: recalling something just as it starts to fade
 * is what makes it stick, while redoing it the same day mostly rehearses short-term memory.
 */
export const INTERVAL_DAYS = [3, 7, 21, 60, 150];

/**
 * Replays a challenge's finished attempts to find its box.
 * A clean solve (no hints) moves it up a box. A solve that needed hints moves it down one,
 * and giving up sends it back to the start.
 */
export function boxFor(attempts: Attempt[]) {
  let box = 0;
  for (const a of attempts) {
    if (a.status === "solved" && a.hintsUsed === 0) box = Math.min(box + 1, INTERVAL_DAYS.length - 1);
    else if (a.status === "solved") box = Math.max(0, box - 1);
    else box = 0;
  }
  return box;
}

export function reviewQueue(data: Data, all: Challenge[], lang: Lang, now = Date.now()) {
  const byId = new Map(all.filter((c) => c.language === lang && c.bank !== "path").map((c) => [c.id, c]));
  const history = new Map<string, Attempt[]>();
  const inProgress = new Set<string>();
  for (const a of data.attempts) {
    if (!byId.has(a.challengeId)) continue;
    if (a.status === "in-progress") inProgress.add(a.challengeId);
    else history.set(a.challengeId, [...(history.get(a.challengeId) ?? []), a]);
  }

  const daily = data.dailies[today()]?.[lang];
  const scheduled = [...history].map(([id, attempts]) => {
    const last = attempts[attempts.length - 1];
    const box = boxFor(attempts);
    const lastAt = Date.parse(last.finishedAt!);
    const dueAt = lastAt + INTERVAL_DAYS[box] * DAY;
    return {
      challenge: byId.get(id)!,
      box,
      reviews: attempts.length - 1,
      lastAt,
      dueAt,
      /** How far past due, relative to the interval: 1 week late on a 3-day interval matters more than on a 60-day one. */
      overdue: (now - dueAt) / (INTERVAL_DAYS[box] * DAY),
      skip: inProgress.has(id) || id === daily || last.date === today(),
    };
  });

  const due = scheduled.filter((s) => !s.skip && s.dueAt <= now).sort((a, b) => b.overdue - a.overdue);
  const upcoming = scheduled.filter((s) => s.dueAt > now).sort((a, b) => a.dueAt - b.dueAt)[0];
  const view = (s: (typeof scheduled)[number]) => ({
    id: s.challenge.id,
    title: s.challenge.title,
    level: s.challenge.level,
    topics: s.challenge.topics,
    box: s.box,
    reviews: s.reviews,
    lastAt: new Date(s.lastAt).toISOString(),
    dueAt: new Date(s.dueAt).toISOString(),
  });
  return {
    due: due.map(view),
    next: upcoming ? view(upcoming) : null,
    scheduled: scheduled.length,
    intervals: INTERVAL_DAYS,
  };
}

export type ReviewQueue = ReturnType<typeof reviewQueue>;
