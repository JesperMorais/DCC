import fs from "node:fs";
import path from "node:path";
import { fileURLToPath } from "node:url";
import { LANG_IDS, LANGUAGES, LEVEL_BANDS, type Lang } from "../shared/languages.ts";

export type Mode = "runtime" | "types";

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
  hints: string[];
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
      let meta: Omit<ChallengeMeta, "id" | "slug" | "language" | "mode"> & { mode?: Mode };
      try {
        meta = JSON.parse(read(d, "meta.json"));
      } catch (e) {
        console.warn(`[challenges] skipping ${language}/${entry.name}: bad meta.json (${(e as Error).message})`);
        continue;
      }
      out.push({
        ...meta,
        mode: meta.mode ?? "runtime",
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
