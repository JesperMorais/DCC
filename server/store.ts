import fs from "node:fs";
import path from "node:path";
import { fileURLToPath } from "node:url";
import type { Experience, Lang } from "../shared/languages.ts";

export type { Experience, Lang };

export interface LanguageProgress {
  experience: Experience;
  rating: number;
  startedAt: string;
}

export interface Profile {
  name: string;
  createdAt: string;
  /** One skill rating per language. A language is "started" once it has an entry. */
  languages: Partial<Record<Lang, LanguageProgress>>;
}

export interface Attempt {
  id: string;
  /** `${language}/${slug}` */
  challengeId: string;
  language: Lang;
  /** Local calendar day (YYYY-MM-DD) the attempt started. */
  date: string;
  /** When the challenge was opened. */
  startedAt: string;
  /** First edit in the editor. The clock runs from here, so reading the task is free. */
  codingStartedAt?: string;
  /** Total time spent paused (ms), not counting a pause still running. */
  pausedMs?: number;
  /** Set while the clock is paused. "away" = the tab was hidden or closed; it resumes on return. */
  pausedAt?: string;
  pauseReason?: "manual" | "away";
  finishedAt?: string;
  status: "in-progress" | "solved" | "gave-up";
  hintsUsed: number;
  runs: number;
  isDaily: boolean;
  /** Only the first finished attempt of a challenge moves the rating. */
  rated: boolean;
  score?: number;
  ratingBefore?: number;
  ratingAfter?: number;
  code?: string;
}

export interface Data {
  version: 2;
  profile: Profile | null;
  attempts: Attempt[];
  /** date → language → challenge id, fixed once assigned so the daily doesn't shift mid-day. */
  dailies: Record<string, Partial<Record<Lang, string>>>;
  ratingHistory: { at: string; rating: number; language: Lang; challengeId?: string }[];
}

const DATA_FILE =
  process.env.DAILY_TS_DATA ?? path.resolve(path.dirname(fileURLToPath(import.meta.url)), "../data/progress.json");

const empty = (): Data => ({ version: 2, profile: null, attempts: [], dailies: {}, ratingHistory: [] });

/** v1 (TypeScript-only) → v2 (per-language). Lossless: every attempt, daily and rating point is kept. */
// eslint-disable-next-line @typescript-eslint/no-explicit-any
export function migrate(raw: any): Data {
  if (!raw || typeof raw !== "object") return empty();
  if (raw.version === 2) return { ...empty(), ...raw };
  const ts = (id: string) => (id.includes("/") ? id : `typescript/${id}`);
  const d = empty();
  if (raw.profile) {
    d.profile = {
      name: raw.profile.name,
      createdAt: raw.profile.createdAt,
      languages: { typescript: { experience: raw.profile.experience ?? "new", rating: raw.profile.rating ?? 900, startedAt: raw.profile.createdAt } },
    };
  }
  d.attempts = (raw.attempts ?? []).map((a: Attempt) => ({ ...a, challengeId: ts(a.challengeId), language: "typescript" }));
  for (const [date, id] of Object.entries(raw.dailies ?? {})) d.dailies[date] = { typescript: ts(String(id)) };
  d.ratingHistory = (raw.ratingHistory ?? []).map((p: { at: string; rating: number; challengeId?: string }) => ({
    ...p,
    language: "typescript",
    ...(p.challengeId ? { challengeId: ts(p.challengeId) } : {}),
  }));
  return d;
}

let cache: Data | null = null;

export function load(): Data {
  if (cache) return cache;
  let raw: unknown = null;
  try {
    raw = JSON.parse(fs.readFileSync(DATA_FILE, "utf8"));
  } catch {
    raw = null;
  }
  cache = migrate(raw);
  // Keep a one-time backup of the pre-migration file, just in case.
  if (raw && (raw as { version?: number }).version === 1 && !fs.existsSync(`${DATA_FILE}.v1.bak`)) {
    fs.copyFileSync(DATA_FILE, `${DATA_FILE}.v1.bak`);
    save(cache);
  }
  return cache;
}

export function save(data: Data) {
  cache = data;
  fs.mkdirSync(path.dirname(DATA_FILE), { recursive: true });
  const tmp = `${DATA_FILE}.tmp`;
  fs.writeFileSync(tmp, JSON.stringify(data, null, 2));
  fs.renameSync(tmp, DATA_FILE);
}

export function reset() {
  save(empty());
}

export const today = (d = new Date()) => d.toLocaleDateString("sv-SE"); // YYYY-MM-DD in local time
