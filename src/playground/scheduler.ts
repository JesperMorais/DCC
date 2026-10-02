// A tiny fixed-priority preemptive scheduler, same model as the C simulator, for in-lesson playgrounds.
import type { TimelineData } from "../components/Timeline";

export interface PgTask {
  name: string;
  priority: number; // higher = more urgent
  period: number;
  wcet: number;
  offset?: number;
  deadline?: number;
  /** Hold the shared resource R from execution tick `at` for `len` ticks. */
  lock?: { at: number; len: number };
}

export interface PgConfig {
  title?: string;
  ticks?: number;
  editable?: boolean;
  inheritance?: boolean;
  tasks: PgTask[];
}

interface Job {
  task: number;
  release: number;
  deadline: number;
  done: number; // executed ticks
  finished: number | null;
  missed: boolean;
}

export interface PgResult {
  timeline: TimelineData;
  utilization: number;
  rmsBound: number;
  perTask: { name: string; worstResponse: number | null; misses: number; jobs: number }[];
  totalMisses: number;
}

export function simulate(cfg: PgConfig): PgResult {
  const ticks = Math.max(1, Math.min(400, cfg.ticks ?? 40));
  const tasks = cfg.tasks;
  const jobs: Job[] = [];
  const runs: [number, number, number][] = [];
  const events: [number, string, string, string][] = [];
  const misses: { task: number; tick: number }[] = [];
  const holds: { task: number; start: number; end: number }[] = [];
  let holder: Job | null = null;
  let holdStart = 0;
  const effPrio = new Map<number, number>();

  const needsLock = (j: Job) => {
    const l = tasks[j.task].lock;
    return !!l && j.done === l.at && holder !== j;
  };

  for (let t = 0; t < ticks; t++) {
    // releases
    tasks.forEach((tk, i) => {
      const off = tk.offset ?? 0;
      if (t >= off && (t - off) % Math.max(1, tk.period) === 0) {
        jobs.push({ task: i, release: t, deadline: t + (tk.deadline ?? tk.period), done: 0, finished: null, missed: false });
      }
    });
    // deadline checks
    for (const j of jobs) {
      if (j.finished === null && !j.missed && t >= j.deadline) {
        j.missed = true;
        misses.push({ task: j.task, tick: j.deadline });
      }
    }
    const active = jobs.filter((j) => j.finished === null);
    // priority inheritance: the holder runs at the highest priority of anyone blocked on R
    const prioOf = (j: Job) => {
      let p = tasks[j.task].priority;
      if (cfg.inheritance && holder === j) for (const w of active) if (w !== j && needsLock(w) && tasks[w.task].priority > p) p = tasks[w.task].priority;
      return p;
    };
    if (holder) {
      const p = prioOf(holder);
      if (effPrio.get(holder.task) !== p) {
        if (p !== tasks[holder.task].priority || effPrio.has(holder.task)) events.push([t, "prio", tasks[holder.task].name, `${effPrio.get(holder.task) ?? tasks[holder.task].priority} → ${p}`]);
        effPrio.set(holder.task, p);
      }
    }
    const runnable = active.filter((j) => !(needsLock(j) && holder && holder !== j));
    runnable.sort((a, b) => prioOf(b) - prioOf(a) || a.release - b.release);
    const job = runnable[0];
    const who = job ? job.task : -1;
    const last = runs[runs.length - 1];
    if (last && last[2] === who && last[1] === t) last[1] = t + 1;
    else runs.push([t, t + 1, who]);
    if (!job) continue;
    const lock = tasks[job.task].lock;
    if (lock && job.done === lock.at && !holder) {
      holder = job;
      holdStart = t;
      events.push([t, "lock", tasks[job.task].name, "R"]);
    }
    job.done++;
    if (holder === job && lock && job.done === lock.at + lock.len) {
      events.push([t + 1, "unlock", tasks[job.task].name, "R"]);
      holds.push({ task: job.task, start: holdStart, end: t + 1 });
      if (effPrio.get(job.task) !== undefined && effPrio.get(job.task) !== tasks[job.task].priority)
        events.push([t + 1, "prio", tasks[job.task].name, `${effPrio.get(job.task)} → ${tasks[job.task].priority}`]);
      effPrio.delete(job.task);
      holder = null;
    }
    if (job.done >= tasks[job.task].wcet) {
      job.finished = t + 1;
      if (job.finished > job.deadline && !job.missed) {
        job.missed = true;
        misses.push({ task: job.task, tick: job.deadline });
      }
    }
  }
  if (holder) holds.push({ task: holder.task, start: holdStart, end: ticks });

  const utilization = tasks.reduce((s, t) => s + t.wcet / Math.max(1, t.period), 0);
  const n = tasks.length;
  const perTask = tasks.map((tk, i) => {
    const mine = jobs.filter((j) => j.task === i);
    const responses = mine.filter((j) => j.finished !== null).map((j) => j.finished! - j.release);
    return { name: tk.name, worstResponse: responses.length ? Math.max(...responses) : null, misses: mine.filter((j) => j.missed).length, jobs: mine.length };
  });
  return {
    timeline: { ticks, tasks: tasks.map((t) => ({ name: t.name, prio: t.priority })), runs, events, misses, holds: holds.length ? holds : undefined },
    utilization,
    rmsBound: n * (Math.pow(2, 1 / n) - 1),
    perTask,
    totalMisses: misses.length,
  };
}
