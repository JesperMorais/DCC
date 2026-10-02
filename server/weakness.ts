// The daily challenges diagnose, the skill tree treats: a rough daily marks the tree nodes that teach its topics.
// Nothing here blocks the daily. A marked node is only recommended (and opened early if it was still locked).
import type { Challenge } from "./challenges.ts";
import { codingMinutes } from "./engine.ts";
import type { LoadedPath, PathNode } from "./paths.ts";
import type { Attempt, Data } from "./store.ts";

export interface Weakness {
  /** When the rough attempt finished. Only practice after this counts towards clearing it. */
  since: string;
  reason: string;
  challenge: { id: string; title: string };
  /** Clean daily solves on the node's topics since then. Two of them clear it. */
  cleanSolves: number;
}

export const CLEAN_SOLVES_TO_CLEAR = 2;

/** Why this attempt counts as "went badly", or null if it went fine. */
export function struggleReason(a: Attempt, c: Challenge): string | null {
  if (a.status === "gave-up") return "you gave up";
  if (a.hintsUsed >= 2) return `${a.hintsUsed} hints`;
  const minutes = codingMinutes(a);
  if (minutes > 2 * c.estMinutes) return `${Math.round(minutes)} min (aim ~${c.estMinutes})`;
  return null;
}

const isClean = (a: Attempt, c: Challenge) => a.status === "solved" && a.hintsUsed === 0 && codingMinutes(a) <= 2 * c.estMinutes;

/** The node was practised since `since`: quiz passed again and, if it has a lab, the lab solved without hints. */
function reviewedSince(data: Data, path: LoadedPath, node: PathNode, since: string) {
  const np = data.paths?.[path.id]?.nodes[node.id];
  if (!np?.quizPassedAt || np.quizPassedAt <= since) return false;
  return !node.labId || (!!np.cleanLabAt && np.cleanLabAt > since);
}

export function weaknesses(data: Data, path: LoadedPath, challenges: Challenge[]): Map<string, Weakness> {
  const byId = new Map(challenges.filter((c) => c.bank !== "path" && c.language === path.language).map((c) => [c.id, c]));
  const finished = data.attempts
    .filter((a) => a.status !== "in-progress" && a.finishedAt && byId.has(a.challengeId))
    .sort((a, b) => a.finishedAt!.localeCompare(b.finishedAt!));
  const out = new Map<string, Weakness>();
  for (const node of path.nodes) {
    const covers = new Set(node.covers ?? []);
    if (!covers.size) continue;
    const relevant = finished.filter((a) => byId.get(a.challengeId)!.topics.some((t) => covers.has(t)));
    let last: { a: Attempt; reason: string } | null = null;
    for (const a of relevant) {
      const reason = struggleReason(a, byId.get(a.challengeId)!);
      if (reason) last = { a, reason };
    }
    if (!last) continue;
    const since = last.a.finishedAt!;
    const cleanSolves = relevant.filter((a) => a.finishedAt! > since && isClean(a, byId.get(a.challengeId)!)).length;
    if (cleanSolves >= CLEAN_SOLVES_TO_CLEAR || reviewedSince(data, path, node, since)) continue;
    const c = byId.get(last.a.challengeId)!;
    out.set(node.id, { since, reason: last.reason, challenge: { id: c.id, title: c.title }, cleanSolves });
  }
  return out;
}

/** Dashboard list: the nodes to practise in this language, most recent first. */
export function focusFor(data: Data, paths: LoadedPath[], challenges: Challenge[], lang: string) {
  return paths
    .filter((p) => p.language === lang)
    .flatMap((p) =>
      [...weaknesses(data, p, challenges)].map(([nodeId, w]) => ({
        path: p.id,
        node: nodeId,
        title: p.nodes.find((n) => n.id === nodeId)!.title,
        ...w,
      })),
    )
    .sort((a, b) => b.since.localeCompare(a.since));
}

export type Focus = ReturnType<typeof focusFor>[number];
