import { useMemo, useState } from "react";
import { Area, AreaChart, CartesianGrid, ReferenceLine, ResponsiveContainer, Tooltip, XAxis, YAxis } from "recharts";
import type { Dashboard } from "../api";
import { LEVEL_NAMES } from "../ui";
import { TessSays } from "./Mascot";

const LEVEL_EDGES = [850, 1000, 1150, 1300, 1450];

/* ---------- Skill rating over time ---------- */
export function RatingChart({ history }: { history: Dashboard["ratingHistory"] }) {
  const data = history.map((p, i) => ({ i, rating: p.rating, at: p.at }));
  if (data.length < 2)
    return (
      <EmptyChart>
        <TessSays mood="wave" size={72}>
          Your rating curve starts after your first challenge. The bigger the stretch, the bigger the jump.
        </TessSays>
      </EmptyChart>
    );
  const min = Math.min(...data.map((d) => d.rating));
  const max = Math.max(...data.map((d) => d.rating));
  const lo = Math.floor((min - 40) / 50) * 50;
  const hi = Math.ceil((max + 40) / 50) * 50;
  const step = hi - lo > 400 ? 100 : 50;
  const ticks = Array.from({ length: Math.floor((hi - lo) / step) + 1 }, (_, i) => lo + i * step);

  return (
    <ResponsiveContainer width="100%" height={220}>
      <AreaChart data={data} margin={{ top: 8, right: 12, left: -8, bottom: 0 }}>
        <defs>
          <linearGradient id="ratingFill" x1="0" y1="0" x2="0" y2="1">
            <stop offset="0%" stopColor="var(--accent)" stopOpacity={0.18} />
            <stop offset="100%" stopColor="var(--accent)" stopOpacity={0} />
          </linearGradient>
        </defs>
        <CartesianGrid vertical={false} stroke="var(--grid)" />
        {LEVEL_EDGES.filter((e) => e > lo && e < hi).map((e) => (
          <ReferenceLine
            key={e}
            y={e}
            stroke="var(--border-strong)"
            strokeDasharray="0"
            label={{ value: `L${LEVEL_EDGES.indexOf(e) + 2}`, position: "insideTopRight", fill: "var(--muted)", fontSize: 10 }}
          />
        ))}
        <XAxis
          dataKey="i"
          type="number"
          domain={[0, data.length - 1]}
          allowDecimals={false}
          tickLine={false}
          axisLine={false}
          tick={{ fill: "var(--muted)", fontSize: 11 }}
          tickFormatter={(i: number) => new Date(data[i]?.at ?? 0).toLocaleDateString(undefined, { month: "short", day: "numeric" })}
          minTickGap={40}
        />
        <YAxis domain={[lo, hi]} ticks={ticks} tickLine={false} axisLine={false} tick={{ fill: "var(--muted)", fontSize: 11 }} width={44} />
        <Tooltip
          cursor={{ stroke: "var(--border-strong)" }}
          content={({ active, payload }) => {
            if (!active || !payload?.length) return null;
            const p = payload[0].payload as (typeof data)[number];
            const prev = data[p.i - 1]?.rating;
            const delta = prev === undefined ? null : p.rating - prev;
            return (
              <div className="rounded-lg border border-line bg-surface px-3 py-2 text-xs shadow-lg">
                <div className="text-muted">{new Date(p.at).toLocaleString(undefined, { dateStyle: "medium", timeStyle: "short" })}</div>
                <div className="mt-0.5 text-sm font-semibold tabular text-ink">
                  {p.rating}
                  {delta !== null && (
                    <span className="ml-2 font-medium" style={{ color: delta >= 0 ? "var(--good)" : "var(--bad)" }}>
                      {delta >= 0 ? "+" : ""}
                      {delta}
                    </span>
                  )}
                </div>
              </div>
            );
          }}
        />
        <Area
          type="monotone"
          dataKey="rating"
          stroke="var(--accent)"
          strokeWidth={2}
          fill="url(#ratingFill)"
          dot={false}
          activeDot={{ r: 5, fill: "var(--accent)", stroke: "var(--surface)", strokeWidth: 2 }}
        />
      </AreaChart>
    </ResponsiveContainer>
  );
}

function EmptyChart({ children }: { children: React.ReactNode }) {
  return (
    <div className="grid h-[220px] place-items-center rounded-xl border border-dashed border-line px-8 text-center text-sm text-muted">
      <div className="max-w-md text-left">{children}</div>
    </div>
  );
}

/* ---------- Activity heatmap (GitHub-style) ---------- */
export function Heatmap({ days }: { days: Dashboard["heatmap"] }) {
  const [hover, setHover] = useState<{ date: string; solved: number; x: number; y: number } | null>(null);

  const weeks = useMemo(() => {
    // Pad so each column starts on a Monday.
    const first = new Date(days[0].date + "T12:00:00");
    const pad = (first.getDay() + 6) % 7;
    const cells: ((typeof days)[number] | null)[] = [...Array(pad).fill(null), ...days];
    const cols: (typeof cells)[] = [];
    for (let i = 0; i < cells.length; i += 7) cols.push(cells.slice(i, i + 7));
    return cols;
  }, [days]);

  const level = (n: number) => (n === 0 ? 0 : n === 1 ? 2 : n === 2 ? 3 : 4);
  const cell = 13;
  const gap = 3;
  const months = weeks.map((w, i) => {
    const d = w.find(Boolean);
    if (!d) return null;
    const m = new Date(d.date + "T12:00:00").getMonth();
    const prev = weeks[i - 1]?.find(Boolean);
    return !prev || new Date(prev.date + "T12:00:00").getMonth() !== m
      ? new Date(d.date + "T12:00:00").toLocaleDateString(undefined, { month: "short" })
      : null;
  });

  return (
    <div className="relative">
      <div className="overflow-x-auto pb-1">
        <div className="inline-flex flex-col gap-1">
          <div className="flex pl-7" style={{ gap }}>
            {months.map((m, i) => (
              <div key={i} className="text-[10px] text-muted" style={{ width: cell }}>
                {m && <span className="whitespace-nowrap">{m}</span>}
              </div>
            ))}
          </div>
          <div className="flex">
            <div className="mr-1.5 flex w-6 flex-col text-[10px] text-muted" style={{ gap }}>
              {["Mon", "", "Wed", "", "Fri", "", ""].map((d, i) => (
                <div key={i} style={{ height: cell, lineHeight: `${cell}px` }}>
                  {d}
                </div>
              ))}
            </div>
            <div className="flex" style={{ gap }}>
              {weeks.map((w, wi) => (
                <div key={wi} className="flex flex-col" style={{ gap }}>
                  {Array.from({ length: 7 }, (_, di) => {
                    const d = w[di];
                    if (!d) return <div key={di} style={{ width: cell, height: cell }} />;
                    return (
                      <div
                        key={di}
                        role="img"
                        aria-label={`${d.date}: ${d.solved} solved`}
                        onMouseEnter={(e) => {
                          const r = (e.target as HTMLElement).getBoundingClientRect();
                          const p = (e.target as HTMLElement).closest(".relative")!.getBoundingClientRect();
                          setHover({ ...d, x: r.left - p.left + cell / 2, y: r.top - p.top });
                        }}
                        onMouseLeave={() => setHover(null)}
                        className="rounded-[3px] transition-transform hover:scale-125"
                        style={{ width: cell, height: cell, background: `var(--heat-${level(d.solved)})` }}
                      />
                    );
                  })}
                </div>
              ))}
            </div>
          </div>
        </div>
      </div>
      <div className="mt-3 flex items-center justify-end gap-1.5 text-[11px] text-muted">
        Less
        {[0, 2, 3, 4].map((l) => (
          <span key={l} className="size-[11px] rounded-[3px]" style={{ background: `var(--heat-${l})` }} />
        ))}
        More
      </div>
      {hover && (
        <div
          className="pointer-events-none absolute z-10 -translate-x-1/2 -translate-y-full rounded-md border border-line bg-surface px-2 py-1 text-xs whitespace-nowrap shadow-lg"
          style={{ left: hover.x, top: hover.y - 6 }}
        >
          <span className="font-semibold text-ink">
            {hover.solved} solved
          </span>{" "}
          <span className="text-muted">
            · {new Date(hover.date + "T12:00:00").toLocaleDateString(undefined, { weekday: "short", month: "short", day: "numeric" })}
          </span>
        </div>
      )}
    </div>
  );
}

/* ---------- Horizontal bar rows ---------- */
export function BarRow({ label, value, max, right, title }: { label: string; value: number; max: number; right: string; title?: string }) {
  const pct = max > 0 ? Math.max(0, Math.min(1, value / max)) : 0;
  return (
    <div className="group" title={title}>
      <div className="mb-1.5 flex items-baseline justify-between gap-3 text-[13px]">
        <span className="truncate text-ink">{label}</span>
        <span className="shrink-0 text-xs tabular text-muted">{right}</span>
      </div>
      <div className="h-2 rounded-full bg-surface-2">
        <div
          className="h-2 rounded-full bg-accent transition-[width] duration-500"
          style={{ width: `${pct * 100}%`, minWidth: value > 0 ? 6 : 0 }}
        />
      </div>
    </div>
  );
}

export function LevelProgress({ levels, current }: { levels: Dashboard["levels"]; current: number }) {
  return (
    <div className="flex flex-col gap-3.5">
      {levels.map((l) => (
        <div key={l.level} className={l.level === current ? "" : "opacity-80"}>
          <BarRow
            label={`L${l.level} · ${LEVEL_NAMES[l.level]}${l.level === current ? "  ← you" : ""}`}
            value={l.solved}
            max={l.total}
            right={`${l.solved} / ${l.total}`}
          />
        </div>
      ))}
    </div>
  );
}

export function TopicMastery({ topics }: { topics: Dashboard["topics"] }) {
  if (topics.length === 0)
    return (
      <div className="py-2">
        <TessSays mood="think" size={72}>
          Topics show up here as you practise, so you can see what's clicking and what needs another rep.
        </TessSays>
      </div>
    );
  return (
    <div className="flex flex-col gap-3.5">
      {topics.slice(0, 7).map((t) => (
        <BarRow
          key={t.topic}
          label={t.topic}
          value={t.avgScore}
          max={1}
          right={`${Math.round(t.avgScore * 100)}% · ${t.solved}/${t.attempts}`}
          title={`Average score ${Math.round(t.avgScore * 100)}% over ${t.attempts} attempt(s), ${t.solved} solved`}
        />
      ))}
    </div>
  );
}
