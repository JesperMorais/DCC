// Tess's voice. Lines are picked with a seed (date + key), so they stay stable for the day
// instead of flickering every time the app refetches state.
import type { ChallengeView, Dashboard, FinishResult, RunResult } from "../api";
import type { Mood } from "../components/Mascot";

export interface Line {
  mood: Mood;
  text: string;
}

const localDate = () => new Date().toLocaleDateString("sv-SE");

function hash(s: string) {
  let h = 2166136261;
  for (let i = 0; i < s.length; i++) h = Math.imul(h ^ s.charCodeAt(i), 16777619);
  return h >>> 0;
}

/** Deterministic pick: same key on the same day → same line. */
export function pick<T>(key: string, variants: readonly T[], salt: string | number = ""): T {
  return variants[hash(`${localDate()}|${key}|${salt}`) % variants.length];
}

const MILESTONES = new Set([3, 7, 14, 30, 50, 100]);

const timeOfDay = () => {
  const h = new Date().getHours();
  return h < 12 ? "morning" : h < 18 ? "afternoon" : "evening";
};

/* ---------- Dashboard greeting (first matching rule wins) ---------- */
export function greeting(s: Dashboard): Line {
  const name = s.profile?.name ?? "there";
  const { stats, daily } = s;
  const last = s.recent[0]?.finishedAt;
  const daysAway = last ? Math.floor((Date.now() - Date.parse(last)) / 86_400_000) : 0;
  const hour = new Date().getHours();
  const d = stats.ratingDelta7d;

  if (stats.attempts === 0)
    return { mood: "wave", text: pick("g-first", [`Hi ${name}. I picked your first one to fit you. No pressure.`, "Day one. I'll keep score, you write the code."]) };
  if (stats.solvedToday && MILESTONES.has(stats.streak))
    return { mood: "cheer", text: pick("g-mile", [`${stats.streak} days in a row. That's a habit now.`, `${stats.streak}-day streak. The armour's getting thick.`]) };
  if (stats.solvedToday && stats.streak > 1 && stats.streak === stats.bestStreak)
    return { mood: "happy", text: `New personal best: ${stats.streak} days.` };
  if (daily?.status === "solved")
    return {
      mood: "happy",
      text: pick("g-done", ["Done for today. The bonus is there if you're hungry.", "Today's in the bag. Rest counts too.", "Solved. See you tomorrow, same time?"]),
    };
  if (daily?.status === "gave-up") return { mood: "think", text: "You read the solution, and that's still learning. It'll come back in a week." };
  if (daysAway >= 3)
    return { mood: "wave", text: pick("g-back", ["Welcome back. I had a nap. Today's pick is a gentle one.", "Good to see you again. Let's ease back in."]) };
  if (stats.streak >= 2 && !stats.solvedToday && hour >= 17)
    return {
      mood: "idle",
      text: pick("g-risk", [`${stats.streak}-day streak. One small challenge keeps it going.`, `About ${daily?.estMinutes ?? 10} minutes keeps the streak going.`]),
    };
  if (daily?.status === "in-progress") return { mood: "think", text: "Your draft is saved. Pick up where you left off." };
  if (d >= 20) return { mood: "happy", text: `Up ${d} this week. The challenges will stretch you a bit more.` };
  if (d <= -20) return { mood: "idle", text: "It's been a tougher week. The picks ease off automatically." };
  return {
    mood: "wave",
    text: pick("g-default", [
      daily ? `Good ${timeOfDay()}. Today's topic is ${daily.topics[0]}.` : `Good ${timeOfDay()}.`,
      "One challenge, about ten minutes. Ready when you are.",
    ]),
  };
}

export function sidebarStreakLine(s: Dashboard) {
  if (s.stats.solvedToday) return "Done for today. Nice.";
  if (s.stats.streak === 0) return "Solve one today to start a streak.";
  if (new Date().getHours() >= 17) return `One solve keeps ${s.stats.streak} going.`;
  return "Solve one today to keep it going.";
}

/* ---------- Run result reactions ---------- */
export type RunCategory = "timeout" | "crash" | "types" | "progress" | "failing" | "pass";

export function categorize(r: RunResult, prevPassing: number | null): RunCategory {
  const msgs = [...r.setupErrors.map((e) => e.message), ...r.tests.map((t) => t.error?.message ?? "")];
  if (msgs.some((m) => /infinite loop|timed out/i.test(m))) return "timeout";
  if (r.setupErrors.length) return "crash";
  if (r.typeErrors.length) return "types";
  if (r.passed) return "pass";
  const passing = r.tests.filter((t) => t.pass).length;
  if (prevPassing !== null && passing > prevPassing) return "progress";
  return "failing";
}

export function runReaction(r: RunResult, cat: RunCategory, prevPassing: number | null, runNo: number): Line {
  const total = r.tests.length;
  const passing = r.tests.filter((t) => t.pass).length;
  const firstFail = r.tests.find((t) => !t.pass);
  const rot = <T,>(v: T[]) => v[runNo % v.length];
  switch (cat) {
    case "timeout":
      return {
        mood: "curl",
        text: rot([
          "I curled up waiting. Something loops forever, so check your loop's exit condition.",
          "That took too long. Is there a while loop that never ends, or a promise that never resolves?",
        ]),
      };
    case "crash":
      return { mood: "think", text: rot(["It crashed while loading, before any test ran. The message below says where.", "Runtime error on load. Try a console.log just above it."]) };
    case "types": {
      if (r.typeErrors.every((e) => e.file === "tests")) return { mood: "think", text: "Your function's shape doesn't match what the tests expect. Check the parameter and return types." };
      const n = r.typeErrors.length;
      return { mood: "think", text: rot([`The compiler spotted ${n} thing${n > 1 ? "s" : ""}. Click one to jump there.`, "Types first. Fix these and the tests will run cleanly."]) };
    }
    case "progress":
      return { mood: "cheer", text: rot([`${passing}/${total} now, up from ${prevPassing}. Keep going.`, `Progress. ${total - passing} to go.`]) };
    case "failing":
      return {
        mood: "think",
        text: rot([
          `${passing}/${total} passing. Compare Expected and Received on the first red one.`,
          firstFail ? `Not yet. Start with “${firstFail.name}”.` : "Not yet. Look at the first failing test.",
        ]),
      };
    case "pass":
      return { mood: "happy", text: rot(["Everything passes. Tidy up if you like, then submit.", "All green. Submit when you're happy with it."]) };
  }
}

export const conceptNudge = (salt: string) =>
  pick("concept-nudge", [
    "Stuck on this one? The Concept tab explains the exact idea these tests check.",
    "The Concept lesson takes two minutes and costs nothing.",
  ], salt);

/* ---------- Finish moments ---------- */
export function finishLine(f: FinishResult, conceptOpened: boolean): Line & { confetti: "full" | "none" } {
  if (f.status === "gave-up")
    return {
      mood: "read",
      confetti: "none",
      text:
        "Read it line by line and find where yours went a different way. It'll come back in a week." +
        (conceptOpened ? "" : " The Concept tab explains the idea behind it."),
    };
  if (f.rated && f.levelAfter > f.levelBefore)
    return { mood: "cheer", confetti: "full", text: `Level ${f.levelAfter}: ${f.levelName}. New kinds of challenges open up from here.` };
  if (f.firstSolveToday && MILESTONES.has(f.streak))
    return { mood: "cheer", confetti: "full", text: pick("f-streak", [`${f.streak} days in a row.`, `That's a ${f.streak}-day streak. Proper armour.`]) };
  if (!f.rated) return { mood: "happy", confetti: "none", text: "Practice rep done. It's unrated, but it all counts." };
  if (f.hintsUsed === 0)
    return { mood: "happy", confetti: f.firstSolveToday ? "full" : "none", text: pick("f-clean", ["No hints needed. That concept is yours.", "Clean solve. Your rating noticed."]) };
  return {
    mood: "happy",
    confetti: f.firstSolveToday ? "full" : "none",
    text: pick("f-hints", ["Using a hint to get unstuck is exactly what they're for.", "Nice push. Done is done."]),
  };
}

export const finishTitle = (f: FinishResult) =>
  f.status === "gave-up" ? "Here's one way to do it" : f.rated && f.levelAfter > f.levelBefore ? "Level up!" : "Solved!";

export const conceptCardCopy = (v: ChallengeView) =>
  v.newTopics.includes(v.topics[0])
    ? { title: `New topic for you: ${v.topics[0]}`, sub: "Free 2-minute lesson. Most people solve faster after it." }
    : { title: "Refresher: the concept behind this", sub: "A 2-minute lesson on the idea this challenge practises. Free, no score cost." };
