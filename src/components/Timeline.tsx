// Draws a scheduling timeline: one lane per task, coloured blocks where it ran, markers for events.
import { useMemo, useState } from "react";

export interface TimelineData {
  ticks: number;
  tasks: { name: string; prio: number; coop?: boolean }[];
  /** [start, end, taskIndex | -1] */
  runs: [number, number, number][];
  /** [tick, type, a, b] */
  events: [number, string, string, string][];
  /** Optional per-task deadline misses (playground) */
  misses?: { task: number; tick: number }[];
  /** Optional shared-resource holding spans (playground) */
  holds?: { task: number; start: number; end: number }[];
}

// Categorical palette (dataviz skill reference instance), in fixed order.
const PALETTE = ["#3987e5", "#d95926", "#199e70", "#c98500", "#d55181", "#008300", "#9085e9", "#e66767"];
export const taskColor = (i: number) => PALETTE[i % PALETTE.length];

const EVENT_STYLE: Record<string, { glyph: string; label: string; color: string }> = {
  irq: { glyph: "⚡", label: "interrupt", color: "#f2b33d" },
  timer: { glyph: "⏱", label: "timer", color: "#f2b33d" },
  lock: { glyph: "🔒", label: "locked", color: "#a6adbd" },
  unlock: { glyph: "🔓", label: "unlocked", color: "#a6adbd" },
  prio: { glyph: "▲", label: "priority change", color: "#ef6b6b" },
  timeout: { glyph: "⌛", label: "timeout", color: "#ef6b6b" },
  overrun: { glyph: "!", label: "overrun", color: "#ef6b6b" },
  mark: { glyph: "◆", label: "mark", color: "#46c26a" },
  deadlock: { glyph: "☠", label: "deadlock", color: "#ef6b6b" },
  fail: { glyph: "✖", label: "failure", color: "#ef6b6b" },
  miss: { glyph: "✖", label: "deadline missed", color: "#ef6b6b" },
};
const LANE_EVENTS = new Set(["lock", "unlock", "prio", "timeout", "overrun", "mark", "fail"]);

export function Timeline({ data, height = 30, maxTicks }: { data: TimelineData; height?: number; maxTicks?: number }) {
  const [hover, setHover] = useState<{ x: number; y: number; text: string } | null>(null);
  const ticks = Math.max(1, Math.min(data.ticks, maxTicks ?? data.ticks));
  const labelW = 110;
  const width = Math.max(560, Math.min(1400, ticks * 14));
  const plotW = width - labelW - 8;
  const x = (t: number) => labelW + (Math.min(t, ticks) / ticks) * plotW;
  const lanes = [...data.tasks.map((t, i) => ({ ...t, i })).sort((a, b) => b.prio - a.prio), { name: "idle", prio: -1, i: -1, coop: false }];
  const laneY = (i: number) => 20 + lanes.findIndex((l) => l.i === i) * (height + 8);
  const totalH = 20 + lanes.length * (height + 8) + 18;
  const tickStep = ticks <= 40 ? 5 : ticks <= 120 ? 10 : ticks <= 400 ? 50 : 100;

  const isrEvents = useMemo(() => data.events.filter((e) => (e[1] === "irq" || e[1] === "timer" || e[1] === "deadlock") && e[0] <= ticks), [data, ticks]);
  const laneEvents = useMemo(() => data.events.filter((e) => LANE_EVENTS.has(e[1]) && e[0] <= ticks), [data, ticks]);
  const nameIndex = useMemo(() => new Map(data.tasks.map((t, i) => [t.name, i])), [data]);

  const tip = (e: React.MouseEvent, text: string) => {
    const box = (e.currentTarget.closest("svg") as SVGSVGElement).getBoundingClientRect();
    setHover({ x: e.clientX - box.left, y: e.clientY - box.top, text });
  };

  return (
    <div className="relative overflow-x-auto">
      <svg width={width} height={totalH} role="img" aria-label="Scheduling timeline" onMouseLeave={() => setHover(null)}>
        {/* tick grid */}
        {Array.from({ length: Math.floor(ticks / tickStep) + 1 }, (_, k) => k * tickStep).map((t) => (
          <g key={t}>
            <line x1={x(t)} x2={x(t)} y1={14} y2={totalH - 16} stroke="var(--grid)" />
            <text x={x(t)} y={totalH - 4} fontSize={10} textAnchor="middle" fill="var(--muted)">
              {t}
            </text>
          </g>
        ))}
        <text x={labelW - 8} y={totalH - 4} fontSize={10} textAnchor="end" fill="var(--muted)">
          tick
        </text>
        {/* lanes */}
        {lanes.map((l) => {
          const y = laneY(l.i);
          return (
            <g key={l.i}>
              <rect x={labelW} y={y} width={plotW} height={height} rx={4} fill="var(--surface-2)" />
              <text x={labelW - 8} y={y + height / 2 + 4} fontSize={12} textAnchor="end" fill={l.i < 0 ? "var(--muted)" : "var(--text)"} fontWeight={l.i < 0 ? 400 : 550}>
                {l.name}
              </text>
              {l.i >= 0 && (
                <text x={labelW - 8} y={y + height / 2 + 15} fontSize={9} textAnchor="end" fill="var(--muted)">
                  prio {l.prio}
                  {l.coop ? " · coop" : ""}
                </text>
              )}
            </g>
          );
        })}
        {/* resource holds (playground) */}
        {data.holds?.map((h, k) => (
          <rect key={k} x={x(h.start)} y={laneY(h.task) - 3} width={Math.max(1, x(h.end) - x(h.start))} height={3} rx={1.5} fill="var(--warn)" />
        ))}
        {/* runs */}
        {data.runs
          .filter((r) => r[0] < ticks)
          .map((r, k) => {
            const y = laneY(r[2]);
            const w = Math.max(1, x(r[1]) - x(r[0]) - 1);
            const name = r[2] < 0 ? "idle" : data.tasks[r[2]]?.name;
            return (
              <rect
                key={k}
                x={x(r[0]) + 0.5}
                y={y + 3}
                width={w}
                height={height - 6}
                rx={3}
                fill={r[2] < 0 ? "var(--surface-3)" : taskColor(r[2])}
                opacity={r[2] < 0 ? 0.6 : 1}
                onMouseMove={(e) => tip(e, `${name}: ticks ${r[0]}–${r[1]} (${r[1] - r[0]} ms)`)}
              />
            );
          })}
        {/* interrupts & timers: a marker at the top of the plot + a hairline */}
        {isrEvents.map((e, k) => {
          const st = EVENT_STYLE[e[1]];
          return (
            <g key={k} onMouseMove={(ev) => tip(ev, `tick ${e[0]}: ${st.label}${e[2] ? ` ${e[2]}` : ""}${e[3] ? ` — ${e[3]}` : ""}`)}>
              <line x1={x(e[0])} x2={x(e[0])} y1={14} y2={totalH - 16} stroke={st.color} strokeWidth={1} strokeDasharray="2 3" opacity={0.7} />
              <text x={x(e[0])} y={11} fontSize={11} textAnchor="middle">
                {st.glyph}
              </text>
            </g>
          );
        })}
        {/* per-task events */}
        {laneEvents.map((e, k) => {
          const st = EVENT_STYLE[e[1]];
          const ti = nameIndex.get(e[2]);
          if (ti === undefined) return null;
          return (
            <text
              key={k}
              x={x(e[0])}
              y={laneY(ti) + height / 2 + 4}
              fontSize={10}
              textAnchor="middle"
              fill={st.color}
              style={{ paintOrder: "stroke", stroke: "var(--surface)", strokeWidth: 3 }}
              onMouseMove={(ev) => tip(ev, `tick ${e[0]}: ${e[2]} ${st.label}${e[3] ? ` (${e[3]})` : ""}`)}
            >
              {st.glyph}
            </text>
          );
        })}
        {/* deadline misses (playground) */}
        {data.misses?.map((m, k) => (
          <text key={k} x={x(m.tick)} y={laneY(m.task) - 4} fontSize={11} textAnchor="middle" fill="var(--bad)" fontWeight={700}>
            ✖
          </text>
        ))}
      </svg>
      {hover && (
        <div
          className="pointer-events-none absolute z-10 -translate-x-1/2 -translate-y-full rounded-md border border-line bg-surface px-2 py-1 text-xs whitespace-nowrap text-ink shadow-lg"
          style={{ left: hover.x, top: hover.y - 8 }}
        >
          {hover.text}
        </div>
      )}
      <div className="mt-1 flex flex-wrap gap-x-4 gap-y-1 text-[11px] text-muted">
        <span>⚡ interrupt</span>
        <span>⏱ timer</span>
        <span>🔒/🔓 mutex</span>
        <span>▲ priority change</span>
        <span>⌛ timeout</span>
        <span>◆ mark</span>
        {data.misses && <span className="text-bad">✖ deadline missed</span>}
        {data.holds && (
          <span>
            <span className="inline-block h-[3px] w-4 rounded align-middle" style={{ background: "var(--warn)" }} /> holds resource R
          </span>
        )}
      </div>
    </div>
  );
}
