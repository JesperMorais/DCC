// Skill-tree progress: unlocks, stars, XP and ranks.
import type { Challenge } from "./challenges.ts";
import { gradeQuestion, publicQuiz, XP, type LoadedPath, type PathNode } from "./paths.ts";
import type { Data, NodeProgress, PathProgress } from "./store.ts";
import { CLEAN_SOLVES_TO_CLEAR, weaknesses } from "./weakness.ts";

export type NodeStatus = "locked" | "available" | "in-progress" | "done";

export const RANKS = [
  { xp: 0, title: "Blinky", blurb: "You made an LED blink. Everyone starts here." },
  { xp: 300, title: "Bit Twiddler", blurb: "Masks and shifts hold no fear." },
  { xp: 800, title: "Interrupt Wrangler", blurb: "ISRs short, data shared safely." },
  { xp: 1500, title: "Scheduler Whisperer", blurb: "You can predict a timeline before you run it." },
  { xp: 2500, title: "Kernel Hacker", blurb: "RTOS internals are your playground." },
  { xp: 4000, title: "RTOS Guru", blurb: "People call you when the system misses deadlines." },
] as const;

export function rankFor(xp: number, ranks: readonly { xp: number; title: string; blurb: string }[] = RANKS) {
  let i = 0;
  while (i + 1 < ranks.length && xp >= ranks[i + 1].xp) i++;
  const next = ranks[i + 1];
  return { ...ranks[i], index: i, next: next ? { title: next.title, xp: next.xp } : null };
}

export function progressOf(data: Data, pathId: string): PathProgress {
  data.paths ??= {};
  data.paths[pathId] ??= { nodes: {} };
  return data.paths[pathId];
}

const blank = (): NodeProgress => ({ quizPassed: false, quizAttempts: 0, quizFirstTry: false, labSolved: false, labHints: null, stars: 0, xp: 0 });

/** `weak`: a daily showed this node's topic is shaky, so it opens early even if its prerequisites aren't done. */
export function nodeStatus(p: PathProgress, node: PathNode, weak = false): NodeStatus {
  const np = p.nodes[node.id];
  if (np?.completedAt) return "done";
  const unlocked = weak || node.requires.every((r) => p.nodes[r]?.completedAt);
  if (!unlocked) return "locked";
  return np && (np.quizAttempts > 0 || np.labSolved || Object.keys(np.milestones ?? {}).length) ? "in-progress" : "available";
}

/** Marks the node complete once all its parts are done; returns what changed. */
function maybeComplete(path: LoadedPath, p: PathProgress, node: PathNode) {
  const np = (p.nodes[node.id] ??= blank());
  const needsLab = !!node.labId;
  const unlockedBefore = path.nodes.filter((n) => nodeStatus(p, n) !== "locked").map((n) => n.id);
  if (node.project) {
    // A project is done when every milestone is ticked. It's a badge: no stars, no XP.
    if (np.completedAt || !node.project.milestones.every((m) => np.milestones?.[m.id])) return { completed: false, xpGained: 0, unlocked: [] as string[] };
    np.completedAt = new Date().toISOString();
    return { completed: true, xpGained: 0, unlocked: [] as string[] };
  }
  if (np.completedAt || !np.quizPassed || (needsLab && !np.labSolved)) return { completed: false, xpGained: 0, unlocked: [] as string[] };
  np.completedAt = new Date().toISOString();
  const cleanLab = needsLab ? np.labHints === 0 : np.quizFirstTry;
  np.stars = 1 + (np.quizFirstTry ? 1 : 0) + (cleanLab ? 1 : 0);
  const base = XP[node.kind];
  np.xp = Math.round(base * (1 + 0.25 * (np.stars - 1)));
  const unlocked = path.nodes.filter((n) => nodeStatus(p, n) !== "locked" && !unlockedBefore.includes(n.id)).map((n) => n.id);
  return { completed: true, xpGained: np.xp, unlocked };
}

export function totalXp(p: PathProgress) {
  return Object.values(p.nodes).reduce((s, n) => s + (n.completedAt ? n.xp : 0), 0);
}

export function pathView(data: Data, path: LoadedPath, labs: Map<string, Challenge>, challenges: Challenge[]) {
  const p = progressOf(data, path.id);
  const xp = totalXp(p);
  const weak = weaknesses(data, path, challenges);
  const nodes = path.nodes.map((n) => {
    const np = p.nodes[n.id];
    const lab = n.labId ? labs.get(n.labId) : undefined;
    const w = weak.get(n.id) ?? null;
    return {
      id: n.id,
      title: n.title,
      section: n.section,
      kind: n.kind,
      row: n.row,
      col: n.col,
      requires: n.requires,
      status: nodeStatus(p, n, !!w),
      weak: w,
      stars: np?.stars ?? 0,
      xp: XP[n.kind],
      quizPassed: !!np?.quizPassed,
      labSolved: !!np?.labSolved,
      hasLab: !!n.labId,
      estMinutes: n.project ? n.project.estHours[1] * 60 : 6 + (lab?.estMinutes ?? 2),
      estHours: n.project?.estHours ?? null,
      ready: !!(n.lesson || n.project),
    };
  });
  const gateDone = !!path.gate && !!p.nodes[path.gate]?.completedAt;
  return {
    id: path.id,
    title: path.title,
    tagline: path.tagline,
    language: path.language,
    before: path.before,
    gate: path.gate ?? null,
    gateDone,
    branch: p.branch ?? null,
    // Projects are optional side quests: they don't count towards sections or stars.
    sections: path.sections.map((s) => {
      const inSec = nodes.filter((n) => n.section === s.id && n.kind !== "project");
      return { ...s, total: inSec.length, done: inSec.filter((n) => n.status === "done").length };
    }),
    nodes,
    xp,
    rank: rankFor(xp, path.ranks),
    stars: nodes.reduce((s, n) => s + n.stars, 0),
    maxStars: nodes.filter((n) => n.kind !== "project").length * 3,
  };
}

export function nodeView(data: Data, path: LoadedPath, node: PathNode, labs: Map<string, Challenge>, challenges: Challenge[]) {
  const p = progressOf(data, path.id);
  const np = p.nodes[node.id] ?? blank();
  const weak = weaknesses(data, path, challenges).get(node.id) ?? null;
  const lab = node.labId ? labs.get(node.labId) : undefined;
  const idx = path.nodes.findIndex((n) => n.id === node.id);
  const nextNodes = path.nodes.filter((n) => n.requires.includes(node.id)).map((n) => ({ id: n.id, title: n.title, status: nodeStatus(p, n) }));
  return {
    pathId: path.id,
    pathTitle: path.title,
    id: node.id,
    title: node.title,
    kind: node.kind,
    language: path.language,
    section: path.sections.find((s) => s.id === node.section)!,
    status: nodeStatus(p, node, !!weak),
    weak: weak && { ...weak, cleanSolvesToClear: CLEAN_SOLVES_TO_CLEAR },
    lesson: node.lesson ?? "_This lesson is still being written._",
    quiz: publicQuiz(node.quiz, `${path.id}:${node.id}`),
    progress: np,
    lab: lab && {
      id: lab.id,
      title: lab.title,
      estMinutes: lab.estMinutes,
      profile: lab.profile ?? null,
      solved: np.labSolved,
    },
    project: node.project && {
      title: node.project.title,
      estHours: node.project.estHours,
      testsGiven: node.project.testsGiven,
      brief: node.project.brief,
      // C projects build with make; the README says what to run first.
      starterCommand: `mkdir -p ~/code && cp -r "${node.project.starterDir}" ~/code/${node.project.folder} && cd ~/code/${node.project.folder}${node.project.language === "typescript" ? " && npm install" : ""}`,
      milestones: node.project.milestones.map((m) => ({ ...m, doneAt: np.milestones?.[m.id] ?? null })),
      // The write-up is a reward for finishing, not something to read instead of thinking.
      review: np.completedAt ? node.project.review : null,
    },
    requires: node.requires.map((r) => ({ id: r, title: path.nodes.find((n) => n.id === r)?.title ?? r, done: !!p.nodes[r]?.completedAt })),
    nextNodes,
    position: idx + 1,
    total: path.nodes.length,
  };
}

export function submitQuiz(data: Data, path: LoadedPath, node: PathNode, answers: unknown[]) {
  const p = progressOf(data, path.id);
  const np = (p.nodes[node.id] ??= blank());
  const results = node.quiz.map((q, i) => ({ correct: gradeQuestion(q, answers[i]), explain: q.explain }));
  const allCorrect = results.every((r) => r.correct);
  np.quizAttempts++;
  if (allCorrect) np.quizPassedAt = new Date().toISOString();
  if (allCorrect && !np.quizPassed) {
    np.quizPassed = true;
    np.quizFirstTry = np.quizAttempts === 1;
  }
  return { results, allCorrect, firstTry: allCorrect && np.quizAttempts === 1, ...maybeComplete(path, p, node), progress: np };
}

export function labSolved(data: Data, path: LoadedPath, node: PathNode, hintsUsed: number) {
  const p = progressOf(data, path.id);
  const np = (p.nodes[node.id] ??= blank());
  np.labSolved = true;
  if (hintsUsed === 0) np.cleanLabAt = new Date().toISOString();
  np.labHints = np.labHints === null ? hintsUsed : Math.min(np.labHints, hintsUsed);
  return { ...maybeComplete(path, p, node), progress: np };
}

export function setMilestone(data: Data, path: LoadedPath, node: PathNode, milestone: string, done: boolean) {
  const p = progressOf(data, path.id);
  const np = (p.nodes[node.id] ??= blank());
  np.milestones ??= {};
  if (done) np.milestones[milestone] ??= new Date().toISOString();
  else delete np.milestones[milestone];
  return { ...maybeComplete(path, p, node), progress: np };
}
