// Typed client for the local API. Types come straight from the server code,
// so a change on one side is a compile error on the other.
import type { Dashboard as EngineDashboard } from "../server/engine.ts";
import type { Focus, projectIdeas } from "../server/weakness.ts";
import type { RunResult } from "../server/runner/index.ts";
import type { Attempt } from "../server/store.ts";
import type { Experience, Lang } from "../shared/languages.ts";

/** The dashboard plus the skill-tree nodes a rough daily pointed at. */
export type Dashboard = EngineDashboard & { focus: Focus[]; projectIdeas: ReturnType<typeof projectIdeas> };
export type { Focus, RunResult, Attempt, Experience, Lang };
export type { Diagnostic, SimTrace } from "../server/runner/index.ts";
import type { nodeView, pathView, setMilestone, submitQuiz } from "../server/pathProgress.ts";
export type PathView = ReturnType<typeof pathView>;
export type NodeView = ReturnType<typeof nodeView>;
export type QuizResult = ReturnType<typeof submitQuiz>;
export type PublicQuestion = NodeView["quiz"][number];

/** The challenge clock as the server sees it. */
export interface Clock {
  codingStartedAt: string | null;
  pausedMs: number;
  pausedAt: string | null;
  pauseReason: "manual" | "away" | null;
}

export interface ChallengeView {
  id: string;
  slug: string;
  language: Lang;
  languageStarted: boolean;
  title: string;
  level: number;
  levelName: string;
  rating: number;
  topics: string[];
  estMinutes: number;
  mode: "runtime" | "types";
  prompt: string;
  learn: string;
  starter: string;
  tests: string;
  hintCount: number;
  hints: string[];
  solution: string | null;
  attempt: Attempt | null;
  solved: boolean;
  bestCode: string | null;
  isDaily: boolean;
  expected: number | null;
  /** Topics in this challenge the learner hasn't met before. */
  newTopics: string[];
  profile: "c" | "embedded" | "linux";
  pathNode: { path: string; node: string } | null;
}

export interface LibraryItem {
  id: string;
  slug: string;
  language: Lang;
  title: string;
  level: number;
  rating: number;
  topics: string[];
  estMinutes: number;
  mode: "runtime" | "types";
  status: "new" | "in-progress" | "solved" | "gave-up";
}

export interface Library {
  language: Lang;
  levels: { level: number; name: string; band: readonly [number, number] }[];
  rating: number | null;
  challenges: LibraryItem[];
}

export interface FinishResult {
  status: "solved" | "gave-up";
  score: number;
  rated: boolean;
  ratingBefore: number;
  ratingAfter: number;
  minutes: number;
  hintsUsed: number;
  solution: string;
  levelBefore: number;
  levelAfter: number;
  levelName: string;
  streak: number;
  firstSolveToday: boolean;
  tree: { completed: boolean; xpGained: number; unlocked: string[]; path: string; node: string } | null;
  result?: RunResult;
}

async function call<T>(path: string, init?: { method?: string; body?: unknown }): Promise<T> {
  const res = await fetch(`/api${path}`, {
    method: init?.method ?? "GET",
    headers: init?.body !== undefined ? { "content-type": "application/json" } : undefined,
    body: init?.body !== undefined ? JSON.stringify(init.body) : undefined,
  });
  const data = await res.json();
  if (!res.ok) throw Object.assign(new Error(data.error ?? res.statusText), { data });
  return data as T;
}

export const api = {
  state: (lang: Lang) => call<Dashboard | { profile: null }>(`/state?lang=${lang}`),
  createProfile: (name: string, experience: Experience, language: Lang) => call("/profile", { method: "POST", body: { name, experience, language } }),
  startLanguage: (lang: Lang, experience: Experience) => call(`/languages/${lang}/start`, { method: "POST", body: { experience } }),
  rename: (name: string) => call("/profile", { method: "POST", body: { name } }),
  reset: () => call("/reset", { method: "POST", body: {} }),
  library: (lang: Lang) => call<Library>(`/challenges?lang=${lang}`),
  challenge: (id: string) => call<ChallengeView>(`/challenges/${id}`),
  start: (id: string) => call<ChallengeView>(`/challenges/${id}/start`, { method: "POST", body: {} }),
  begin: (id: string) => call<{ codingStartedAt: string }>(`/challenges/${id}/begin`, { method: "POST", body: {} }),
  pause: (id: string, reason: "manual" | "away") => call<Clock>(`/challenges/${id}/pause?reason=${reason}`, { method: "POST", body: {} }),
  resume: (id: string) => call<Clock>(`/challenges/${id}/resume`, { method: "POST", body: {} }),
  /** Fire-and-forget pause that survives the page closing. */
  pauseOnUnload: (id: string) => navigator.sendBeacon(`/api/challenges/${id}/pause?reason=away`),
  hint: (id: string) => call<{ hints: string[] }>(`/challenges/${id}/hint`, { method: "POST", body: {} }),
  run: (id: string, code: string) => call<RunResult>(`/challenges/${id}/run`, { method: "POST", body: { code } }),
  submit: (id: string, code: string) => call<FinishResult>(`/challenges/${id}/submit`, { method: "POST", body: { code } }),
  giveUp: (id: string, code: string) => call<FinishResult>(`/challenges/${id}/giveup`, { method: "POST", body: { code } }),
  paths: () => call<{ id: string; title: string; tagline: string; language: Lang; xp: number; rank: PathView["rank"]; stars: number; maxStars: number }[]>("/paths"),
  path: (id: string) => call<PathView>(`/paths/${id}`),
  node: (path: string, node: string) => call<NodeView>(`/paths/${path}/nodes/${node}`),
  submitQuiz: (path: string, node: string, answers: unknown[]) => call<QuizResult>(`/paths/${path}/nodes/${node}/quiz`, { method: "POST", body: { answers } }),
  setMilestone: (path: string, node: string, id: string, done: boolean) =>
    call<ReturnType<typeof setMilestone>>(`/paths/${path}/nodes/${node}/milestone`, { method: "POST", body: { id, done } }),
  chooseBranch: (path: string, branch: string) => call(`/paths/${path}/branch`, { method: "POST", body: { branch } }),
  harness: () => fetch("/api/harness").then((r) => r.text()),
};

export const isOnboarded = (s: Dashboard | { profile: null }): s is Dashboard => s.profile !== null;
