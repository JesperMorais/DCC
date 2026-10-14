import fs from "node:fs";
import path from "node:path";
import { fileURLToPath } from "node:url";
import { LANG_IDS, LANGUAGES, LEVEL_BANDS, type Lang } from "../shared/languages.ts";

export type Mode = "runtime" | "types";
/** "write" = build it from a starter; "fix" = find and fix the one bug in working-looking code (Fix-it Friday). */
export type Kind = "write" | "fix";

export interface ChallengeMeta {
  /** `${language}/${slug}` */
  id: string;
  slug: string;
  language: Lang;
  title: string;
  /** 1 (first steps) … 6 (expert) */
  level: 1 | 2 | 3 | 4 | 5 | 6;
  /** Elo-style difficulty, see LEVEL_BANDS for the band each level lives in. */
  rating: number;
  topics: string[];
  estMinutes: number;
  /** "types" = judged by the TypeScript compiler alone (TypeScript only). */
  mode: Mode;
  kind: Kind;
  hints: string[];
  /** "daily" = the adaptive daily bank (default); "path" = a skill-tree lab (unrated, not picked as a daily). */
  bank?: "daily" | "path";
  /** C build profile: plain C17, the embedded simulator, or Linux userspace. */
  profile?: "c" | "embedded" | "linux";
  pathNode?: { path: string; node: string };
}

export interface Challenge extends ChallengeMeta {
  prompt: string;
  learn: string;
  starter: string;
  solution: string;
  tests: string;
}

export const levelName = (lang: Lang, level: number) => LANGUAGES[lang].levels[level] ?? `Level ${level}`;
export { LEVEL_BANDS };

export const CHALLENGES_DIR = path.resolve(path.dirname(fileURLToPath(import.meta.url)), "../challenges");

const read = (dir: string, file: string) => fs.readFileSync(path.join(dir, file), "utf8");

export function loadChallenges(dir = CHALLENGES_DIR): Challenge[] {
  const out: Challenge[] = [];
  for (const language of LANG_IDS) {
    const langDir = path.join(dir, language);
    if (!fs.existsSync(langDir)) continue;
    const ext = LANGUAGES[language].ext;
    for (const entry of fs.readdirSync(langDir, { withFileTypes: true })) {
      if (!entry.isDirectory()) continue;
      const d = path.join(langDir, entry.name);
      const files = ["meta.json", "prompt.md", "learn.md", `starter.${ext}`, `solution.${ext}`, `tests.${ext}`];
      const missing = files.filter((f) => !fs.existsSync(path.join(d, f)));
      if (missing.length) {
        if (missing.length < files.length) console.warn(`[challenges] skipping ${language}/${entry.name}: missing ${missing.join(", ")}`);
        continue;
      }
      let meta: Omit<ChallengeMeta, "id" | "slug" | "language" | "mode" | "kind"> & { mode?: Mode; kind?: Kind };
      try {
        meta = JSON.parse(read(d, "meta.json"));
      } catch (e) {
        console.warn(`[challenges] skipping ${language}/${entry.name}: bad meta.json (${(e as Error).message})`);
        continue;
      }
      out.push({
        ...meta,
        mode: meta.mode ?? "runtime",
        kind: meta.kind ?? "write",
        id: `${language}/${entry.name}`,
        slug: entry.name,
        language,
        prompt: read(d, "prompt.md"),
        learn: read(d, "learn.md"),
        starter: read(d, `starter.${ext}`),
        solution: read(d, `solution.${ext}`),
        tests: read(d, `tests.${ext}`),
      });
    }
  }
  return out.sort((a, b) => a.language.localeCompare(b.language) || a.rating - b.rating || a.id.localeCompare(b.id));
}

/**
 * How many lines an edit touches: the larger of lines removed and lines added, after matching
 * unchanged lines (longest common subsequence). Trailing whitespace and blank lines don't count.
 */
export function changedLines(before: string, after: string) {
  const norm = (s: string) => s.split("\n").map((l) => l.trimEnd()).filter((l) => l !== "");
  const a = norm(before);
  const b = norm(after);
  let prev = new Array<number>(b.length + 1).fill(0);
  for (let i = 1; i <= a.length; i++) {
    const row = new Array<number>(b.length + 1).fill(0);
    for (let j = 1; j <= b.length; j++) row[j] = a[i - 1] === b[j - 1] ? prev[j - 1] + 1 : Math.max(prev[j], row[j - 1]);
    prev = row;
  }
  const same = prev[b.length];
  return Math.max(a.length - same, b.length - same);
}
