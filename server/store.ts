import fs from "node:fs";
import path from "node:path";
import { fileURLToPath } from "node:url";

export type Experience = "new" | "other-lang" | "js" | "some-ts" | "pro";

export interface Profile {
  name: string;
  experience: Experience;
  createdAt: string;
  rating: number;
}

export interface Attempt {
  id: string;
  challengeId: string;
  /** Local calendar day (YYYY-MM-DD) the attempt started. */
  date: string;
  startedAt: string;
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
  version: 1;
  profile: Profile | null;
  attempts: Attempt[];
  /** date → challenge id, fixed once assigned so the daily doesn't shift mid-day. */
  dailies: Record<string, string>;
  ratingHistory: { at: string; rating: number; challengeId?: string }[];
}

const DATA_FILE =
  process.env.DAILY_TS_DATA ?? path.resolve(path.dirname(fileURLToPath(import.meta.url)), "../data/progress.json");

const empty = (): Data => ({ version: 1, profile: null, attempts: [], dailies: {}, ratingHistory: [] });

let cache: Data | null = null;

export function load(): Data {
  if (cache) return cache;
  try {
    cache = { ...empty(), ...JSON.parse(fs.readFileSync(DATA_FILE, "utf8")) } as Data;
  } catch {
    cache = empty();
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
