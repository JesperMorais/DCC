import fs from "node:fs";
import path from "node:path";
import { fileURLToPath } from "node:url";

export type Mode = "runtime" | "types";

export interface ChallengeMeta {
  id: string;
  title: string;
  /** 1 (first steps) … 6 (type wizardry) */
  level: 1 | 2 | 3 | 4 | 5 | 6;
  /** Elo-style difficulty, see LEVELS for the band each level lives in. */
  rating: number;
  topics: string[];
  estMinutes: number;
  mode: Mode;
  hints: string[];
}

export interface Challenge extends ChallengeMeta {
  prompt: string;
  learn: string;
  starter: string;
  solution: string;
  tests: string;
}

export const LEVELS = {
  1: { name: "First steps", band: [700, 850] },
  2: { name: "Building blocks", band: [850, 1000] },
  3: { name: "Shaping data", band: [1000, 1150] },
  4: { name: "Generics", band: [1150, 1300] },
  5: { name: "Advanced patterns", band: [1300, 1450] },
  6: { name: "Type wizardry", band: [1450, 1650] },
} as const;

export const CHALLENGES_DIR = path.resolve(path.dirname(fileURLToPath(import.meta.url)), "../challenges");

const read = (dir: string, file: string) => fs.readFileSync(path.join(dir, file), "utf8");

export function loadChallenges(dir = CHALLENGES_DIR): Challenge[] {
  const out: Challenge[] = [];
  for (const entry of fs.readdirSync(dir, { withFileTypes: true })) {
    if (!entry.isDirectory()) continue;
    const d = path.join(dir, entry.name);
    const meta = JSON.parse(read(d, "meta.json")) as Omit<ChallengeMeta, "id">;
    out.push({
      ...meta,
      id: entry.name,
      prompt: read(d, "prompt.md"),
      learn: read(d, "learn.md"),
      starter: read(d, "starter.ts"),
      solution: read(d, "solution.ts"),
      tests: read(d, "tests.ts"),
    });
  }
  return out.sort((a, b) => a.rating - b.rating || a.id.localeCompare(b.id));
}
