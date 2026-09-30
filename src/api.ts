// Typed client for the local API. Types come straight from the server code,
// so a change on one side is a compile error on the other.
import type { Dashboard } from "../server/engine.ts";
import type { RunResult } from "../server/runner/index.ts";
import type { Attempt, Experience } from "../server/store.ts";

export type { Dashboard, RunResult, Attempt, Experience };

export interface ChallengeView {
  id: string;
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
}

export interface LibraryItem {
  id: string;
  title: string;
  level: number;
  rating: number;
  topics: string[];
  estMinutes: number;
  mode: "runtime" | "types";
  status: "new" | "in-progress" | "solved" | "gave-up";
}

export interface Library {
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
  state: () => call<Dashboard | { profile: null }>("/state"),
  createProfile: (name: string, experience: Experience) => call("/profile", { method: "POST", body: { name, experience } }),
  rename: (name: string) => call("/profile", { method: "POST", body: { name } }),
  reset: () => call("/reset", { method: "POST", body: {} }),
  library: () => call<Library>("/challenges"),
  challenge: (id: string) => call<ChallengeView>(`/challenges/${id}`),
  start: (id: string) => call<ChallengeView>(`/challenges/${id}/start`, { method: "POST", body: {} }),
  hint: (id: string) => call<{ hints: string[] }>(`/challenges/${id}/hint`, { method: "POST", body: {} }),
  run: (id: string, code: string) => call<RunResult>(`/challenges/${id}/run`, { method: "POST", body: { code } }),
  submit: (id: string, code: string) => call<FinishResult>(`/challenges/${id}/submit`, { method: "POST", body: { code } }),
  giveUp: (id: string, code: string) => call<FinishResult>(`/challenges/${id}/giveup`, { method: "POST", body: { code } }),
  harness: () => fetch("/api/harness").then((r) => r.text()),
};

export const isOnboarded = (s: Dashboard | { profile: null }): s is Dashboard => s.profile !== null;
