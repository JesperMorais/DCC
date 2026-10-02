// Skill trees ("paths"): nodes with lessons, server-graded quizzes and labs in the path's language.
import fs from "node:fs";
import path from "node:path";
import { fileURLToPath } from "node:url";
import { LANGUAGES, type Lang } from "../shared/languages.ts";
import type { Challenge, Mode } from "./challenges.ts";
import type { CProfile } from "./runner/index.ts";

export const PATHS_DIR = path.resolve(path.dirname(fileURLToPath(import.meta.url)), "../paths");

export type QuizQuestion =
  | { type: "single"; q: string; code?: string; options: string[]; answer: number; explain: string }
  | { type: "multi"; q: string; code?: string; options: string[]; answer: number[]; explain: string }
  | { type: "order"; q: string; code?: string; items: string[]; explain: string }
  | { type: "number"; q: string; code?: string; answer: number; tolerance?: number; unit?: string; explain: string };

export interface PathNodeMeta {
  id: string;
  section: string;
  row: number;
  col: number;
  kind: "lesson" | "lab" | "boss";
  title: string;
  requires: string[];
  /** Daily-challenge topics this node teaches. A rough daily on one of them marks the node "needs practice". */
  covers?: string[];
}

export interface PathDef {
  id: string;
  title: string;
  tagline: string;
  /** Language of the labs (default "c"). */
  language?: Lang;
  /** The node that unlocks the branch choice. Paths without one have no branches to pick. */
  gate?: string;
  ranks?: { xp: number; title: string; blurb: string }[];
  sections: { id: string; title: string; color: string; blurb: string }[];
  nodes: PathNodeMeta[];
}

export interface PathNode extends PathNodeMeta {
  lesson: string | null;
  quiz: QuizQuestion[];
  /** Challenge id of the lab, e.g. "c/embedded.f-races" */
  labId: string | null;
}

export interface LoadedPath extends Omit<PathDef, "nodes" | "language"> {
  language: Lang;
  nodes: PathNode[];
}

const readIf = (f: string) => (fs.existsSync(f) ? fs.readFileSync(f, "utf8") : null);

export const XP = { lesson: 60, lab: 100, boss: 250 } as const;

export function loadPaths(dir = PATHS_DIR): { paths: LoadedPath[]; labs: Challenge[] } {
  const paths: LoadedPath[] = [];
  const labs: Challenge[] = [];
  if (!fs.existsSync(dir)) return { paths, labs };
  for (const entry of fs.readdirSync(dir, { withFileTypes: true })) {
    if (!entry.isDirectory()) continue;
    const pdir = path.join(dir, entry.name);
    const defText = readIf(path.join(pdir, "path.json"));
    if (!defText) continue;
    const def = JSON.parse(defText) as PathDef;
    const language = def.language ?? "c";
    const ext = LANGUAGES[language].ext;
    const nodes: PathNode[] = def.nodes.map((n) => {
      const ndir = path.join(pdir, "nodes", n.id);
      let quiz: QuizQuestion[] = [];
      try {
        quiz = JSON.parse(readIf(path.join(ndir, "quiz.json")) ?? "[]");
      } catch (e) {
        console.warn(`[paths] ${def.id}/${n.id}: bad quiz.json (${(e as Error).message})`);
      }
      let labId: string | null = null;
      const ldir = path.join(ndir, "lab");
      const files = ["meta.json", "prompt.md", `starter.${ext}`, `solution.${ext}`, `tests.${ext}`];
      if (files.every((f) => fs.existsSync(path.join(ldir, f)))) {
        try {
          const meta = JSON.parse(fs.readFileSync(path.join(ldir, "meta.json"), "utf8")) as {
            title?: string;
            /** Shown on the lab like a daily's level. Default: 5, or 6 for a boss. */
            level?: Challenge["level"];
            profile?: CProfile;
            mode?: Mode;
            estMinutes?: number;
            topics?: string[];
            hints?: string[];
          };
          const slug = `${def.id}.${n.id}`;
          labId = `${language}/${slug}`;
          labs.push({
            id: labId,
            slug,
            language,
            title: meta.title ?? n.title,
            level: meta.level ?? (n.kind === "boss" ? 6 : 5),
            rating: 0,
            topics: meta.topics ?? [],
            estMinutes: meta.estMinutes ?? 12,
            mode: meta.mode ?? "runtime",
            hints: meta.hints ?? [],
            prompt: fs.readFileSync(path.join(ldir, "prompt.md"), "utf8"),
            learn: readIf(path.join(ndir, "lesson.md")) ?? "",
            starter: fs.readFileSync(path.join(ldir, `starter.${ext}`), "utf8"),
            solution: fs.readFileSync(path.join(ldir, `solution.${ext}`), "utf8"),
            tests: fs.readFileSync(path.join(ldir, `tests.${ext}`), "utf8"),
            bank: "path",
            profile: language === "c" ? (meta.profile ?? "embedded") : undefined,
            pathNode: { path: def.id, node: n.id },
          });
        } catch (e) {
          console.warn(`[paths] ${def.id}/${n.id}: bad lab/meta.json (${(e as Error).message})`);
        }
      }
      return { ...n, lesson: readIf(path.join(ndir, "lesson.md")), quiz, labId };
    });
    paths.push({ ...def, language, nodes });
  }
  return { paths, labs };
}

/** Grade one answer. `answer` comes from the client: number | number[] (indices) | number (value). */
export function gradeQuestion(q: QuizQuestion, answer: unknown): boolean {
  switch (q.type) {
    case "single":
      return answer === q.answer;
    case "multi": {
      if (!Array.isArray(answer)) return false;
      const a = [...new Set(answer as number[])].sort((x, y) => x - y);
      const b = [...q.answer].sort((x, y) => x - y);
      return a.length === b.length && a.every((v, i) => v === b[i]);
    }
    case "order":
      // The client sends the items in the order the learner arranged them.
      return Array.isArray(answer) && answer.length === q.items.length && answer.every((v, i) => v === q.items[i]);
    case "number": {
      const n = typeof answer === "number" ? answer : Number(answer);
      return Number.isFinite(n) && Math.abs(n - q.answer) <= (q.tolerance ?? 0);
    }
  }
}

/** Questions as sent to the browser: no answers. Order items are shuffled deterministically. */
export function publicQuiz(quiz: QuizQuestion[], seed: string) {
  return quiz.map((q, i) => {
    const { explain: _e, ...rest } = q as QuizQuestion & { explain: string };
    void _e;
    if (q.type === "order") return { ...rest, items: shuffle(q.items, `${seed}:${i}`), answer: undefined };
    if (q.type === "multi") return { ...rest, answer: undefined, multi: true };
    return { ...rest, answer: undefined };
  });
}

function shuffle<T>(items: T[], seed: string): T[] {
  let h = 2166136261;
  for (let i = 0; i < seed.length; i++) h = Math.imul(h ^ seed.charCodeAt(i), 16777619);
  const a = [...items];
  for (let i = a.length - 1; i > 0; i--) {
    h = Math.imul(h ^ (h >>> 15), 2246822507) >>> 0;
    const j = h % (i + 1);
    [a[i], a[j]] = [a[j], a[i]];
  }
  // Never hand out the already-correct order.
  if (a.every((v, i) => v === items[i]) && a.length > 1) [a[0], a[1]] = [a[1], a[0]];
  return a;
}
