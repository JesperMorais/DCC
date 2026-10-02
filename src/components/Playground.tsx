import { useMemo, useState } from "react";
import { simulate, type PgConfig, type PgTask } from "../playground/scheduler";
import { Icon } from "../ui";
import { Mascot } from "./Mascot";
import { Timeline, taskColor } from "./Timeline";

const pct = (x: number) => `${Math.round(x * 1000) / 10}%`;

export function Playground({ initial }: { initial: PgConfig }) {
  const [tasks, setTasks] = useState<PgTask[]>(initial.tasks);
  const [inheritance, setInheritance] = useState(!!initial.inheritance);
  const [ticks, setTicks] = useState(initial.ticks ?? 40);
  const editable = initial.editable !== false;
  const hasLock = tasks.some((t) => t.lock);
  const res = useMemo(() => simulate({ ...initial, tasks, inheritance, ticks }), [initial, tasks, inheritance, ticks]);

  const set = (i: number, patch: Partial<PgTask>) => setTasks((ts) => ts.map((t, k) => (k === i ? { ...t, ...patch } : t)));
  const num = (v: string, min: number) => Math.max(min, Math.min(999, Math.round(Number(v) || 0)));

  return (
    <div className="not-prose my-6 rounded-2xl border border-accent/30 bg-surface p-4 shadow-sm">
      <div className="mb-3 flex flex-wrap items-center gap-2">
        <span className="inline-flex items-center gap-1.5 rounded-md bg-accent-soft px-2 py-0.5 text-[11px] font-semibold tracking-wide text-accent-strong uppercase">
          <Icon name="zap" size={12} /> Playground
        </span>
        <span className="text-sm font-semibold text-ink">{initial.title ?? "Scheduler"}</span>
        <span className="ml-auto flex items-center gap-2">
          {hasLock && (
            <label className="flex cursor-pointer items-center gap-1.5 rounded-lg border border-line px-2 py-1 text-xs text-ink-2">
              <input type="checkbox" checked={inheritance} onChange={(e) => setInheritance(e.target.checked)} className="accent-[var(--accent)]" />
              Priority inheritance
            </label>
          )}
          <button
            className="btn h-7 px-2 text-xs"
            onClick={() => {
              setTasks(initial.tasks);
              setInheritance(!!initial.inheritance);
              setTicks(initial.ticks ?? 40);
            }}
          >
            <Icon name="refresh" size={12} /> Reset
          </button>
        </span>
      </div>

      <div className="overflow-x-auto">
        <table className="w-full min-w-[520px] text-xs">
          <thead>
            <tr className="text-left text-muted">
              <th className="pb-1 font-medium">Task</th>
              <th className="pb-1 font-medium" title="Higher number = more urgent">Priority</th>
              <th className="pb-1 font-medium">Period</th>
              <th className="pb-1 font-medium" title="Worst-case execution time per job">WCET</th>
              <th className="pb-1 font-medium">Offset</th>
              {hasLock && <th className="pb-1 font-medium" title="Holds shared resource R from execution tick 'at' for 'len' ticks">Lock R (at+len)</th>}
              <th className="pb-1 text-right font-medium">Worst response</th>
              <th className="pb-1 text-right font-medium">Misses</th>
            </tr>
          </thead>
          <tbody>
            {tasks.map((t, i) => {
              const r = res.perTask[i];
              const field = (key: "priority" | "period" | "wcet" | "offset", min: number) =>
                editable ? (
                  <input
                    type="number"
                    value={t[key] ?? 0}
                    min={min}
                    onChange={(e) => set(i, { [key]: num(e.target.value, min) } as Partial<PgTask>)}
                    className="h-7 w-16 rounded-md border border-line bg-surface-2 px-1.5 text-ink tabular outline-none focus:border-accent"
                  />
                ) : (
                  <span className="tabular">{t[key] ?? 0}</span>
                );
              return (
                <tr key={i} className="border-t border-line">
                  <td className="py-1.5 pr-2">
                    <span className="inline-flex items-center gap-1.5 font-medium text-ink">
                      <span className="size-2.5 rounded-sm" style={{ background: taskColor(i) }} />
                      {t.name}
                    </span>
                  </td>
                  <td className="pr-2">{field("priority", 0)}</td>
                  <td className="pr-2">{field("period", 1)}</td>
                  <td className="pr-2">{field("wcet", 1)}</td>
                  <td className="pr-2">{field("offset", 0)}</td>
                  {hasLock && (
                    <td className="pr-2 text-ink-2">
                      {t.lock ? (
                        editable ? (
                          <span className="inline-flex items-center gap-1">
                            <input
                              type="number"
                              value={t.lock.at}
                              min={0}
                              onChange={(e) => set(i, { lock: { ...t.lock!, at: Math.min(num(e.target.value, 0), Math.max(0, t.wcet - 1)) } })}
                              className="h-7 w-12 rounded-md border border-line bg-surface-2 px-1.5 tabular outline-none focus:border-accent"
                            />
                            +
                            <input
                              type="number"
                              value={t.lock.len}
                              min={1}
                              onChange={(e) => set(i, { lock: { ...t.lock!, len: Math.max(1, Math.min(num(e.target.value, 1), t.wcet - t.lock!.at)) } })}
                              className="h-7 w-12 rounded-md border border-line bg-surface-2 px-1.5 tabular outline-none focus:border-accent"
                            />
                          </span>
                        ) : (
                          `${t.lock.at}+${t.lock.len}`
                        )
                      ) : (
                        "—"
                      )}
                    </td>
                  )}
                  <td className="text-right tabular text-ink-2">
                    {r.worstResponse ?? "—"}
                    {r.worstResponse !== null && <span className="text-muted"> / {t.deadline ?? t.period}</span>}
                  </td>
                  <td className="text-right font-semibold tabular" style={{ color: r.misses ? "var(--bad)" : "var(--good)" }}>
                    {r.misses}
                  </td>
                </tr>
              );
            })}
          </tbody>
        </table>
      </div>

      <div className="mt-4">
        <Timeline data={res.timeline} height={24} />
      </div>

      <div className="mt-3 flex flex-wrap items-center gap-x-5 gap-y-2 text-xs">
        <span className="text-ink-2">
          CPU utilisation <b className="text-ink tabular">{pct(res.utilization)}</b>
        </span>
        <span className="text-ink-2" title="Liu & Layland: n tasks with rate-monotonic priorities are guaranteed schedulable if U ≤ n(2^(1/n) − 1)">
          RMS bound for {tasks.length} tasks <b className="text-ink tabular">{pct(res.rmsBound)}</b>
        </span>
        <label className="flex items-center gap-1.5 text-ink-2">
          Simulate
          <input
            type="number"
            value={ticks}
            min={10}
            max={400}
            onChange={(e) => setTicks(Math.max(10, Math.min(400, Math.round(Number(e.target.value) || 40))))}
            className="h-6 w-14 rounded-md border border-line bg-surface-2 px-1 text-ink tabular outline-none focus:border-accent"
          />
          ticks
        </label>
        <span className="ml-auto flex items-center gap-2">
          <Mascot mood={res.totalMisses ? "think" : res.utilization > 1 ? "sad" : "happy"} size={30} />
          <span className="font-medium" style={{ color: res.totalMisses ? "var(--bad)" : "var(--good)" }}>
            {res.utilization > 1
              ? "Over 100% CPU: no priority order can save this."
              : res.totalMisses
                ? `${res.totalMisses} deadline miss${res.totalMisses > 1 ? "es" : ""}`
                : "Every deadline met"}
          </span>
        </span>
      </div>
    </div>
  );
}
