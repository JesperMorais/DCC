import { useEffect, useMemo, useState } from "react";
import { useNavigate, useParams } from "react-router-dom";
import { api, type PathView } from "../api";
import { Mascot, TessLoading } from "../components/Mascot";
import { Icon, type IconName } from "../ui";

type Node = PathView["nodes"][number];

const COL_W = 150;
const ROW_H = 140;
const LABEL_H = 46; // stars + up to 3 label lines under a node
const TOP = 70;
const R = 28;

export default function PathPage() {
  const { pathId = "embedded" } = useParams();
  const navigate = useNavigate();
  const [view, setView] = useState<PathView | null>(null);
  const [hover, setHover] = useState<Node | null>(null);
  const [choosing, setChoosing] = useState(false);

  const load = () => api.path(pathId).then(setView);
  useEffect(() => {
    load();
  }, [pathId]);

  const layout = useMemo(() => {
    if (!view) return null;
    const cols = view.nodes.map((n) => n.col);
    const minCol = Math.min(...cols);
    const maxCol = Math.max(...cols);
    const width = (maxCol - minCol) * COL_W + COL_W * 1.6;
    const height = TOP + Math.max(...view.nodes.map((n) => n.row)) * ROW_H + 120;
    const pos = new Map(view.nodes.map((n) => [n.id, { x: (n.col - minCol) * COL_W + COL_W * 0.8, y: TOP + n.row * ROW_H }]));
    return { width, height, pos };
  }, [view]);

  useEffect(() => {
    if (view?.gateDone && !view.branch) setChoosing(true);
  }, [view?.gateDone, view?.branch]);

  if (!view || !layout) return <TessLoading label="Growing the tree…" />;

  const color = (section: string) => view.sections.find((s) => s.id === section)?.color ?? "var(--accent)";
  const byId = new Map(view.nodes.map((n) => [n.id, n]));
  const frontier = view.nodes.find((n) => n.status === "in-progress") ?? view.nodes.find((n) => n.status === "available");
  const gateNode = view.gate ? byId.get(view.gate) : undefined;
  const gatePos = view.gate ? layout.pos.get(view.gate) : undefined;
  const hat = view.language === "c" ? "hardhat" : undefined;
  const weak = view.nodes.filter((n) => n.weak);
  const branches = view.sections.filter((s) => s.id !== "fundamentals");
  const pct = view.rank.next ? (view.xp - (view.rank.xp ?? 0)) / (view.rank.next.xp - view.rank.xp) : 1;

  return (
    <div className="mx-auto max-w-[1240px] px-4 py-8 sm:px-8">
      {/* header */}
      <header className="mb-6 grid gap-5 lg:grid-cols-[1fr_340px]">
        <div>
          <p className="flex items-center gap-2 text-sm text-muted">
            <Icon name={view.language === "c" ? "cpu" : "tree"} size={15} /> Skill tree
          </p>
          <h1 className="mt-1 text-[28px] font-semibold tracking-tight">{view.title}</h1>
          <p className="mt-2 max-w-2xl text-ink-2">{view.tagline}</p>
          <div className="mt-4 flex flex-wrap gap-2">
            {view.sections.map((s) => (
              <span key={s.id} className="inline-flex items-center gap-2 rounded-lg border border-line bg-surface px-2.5 py-1.5 text-xs">
                <span className="size-2.5 rounded-full" style={{ background: s.color }} />
                <span className="font-medium text-ink">{s.title}</span>
                <span className="tabular text-muted">
                  {s.done}/{s.total}
                </span>
                {view.branch === s.id && <span className="rounded bg-accent-soft px-1 text-[10px] font-semibold text-accent-strong">YOUR BRANCH</span>}
              </span>
            ))}
          </div>
        </div>
        <div className="card flex items-center gap-4 p-4">
          <Mascot mood={view.xp ? "cheer" : "wave"} size={72} hat={hat} className="shrink-0 text-ink-2" />
          <div className="min-w-0 flex-1">
            <div className="text-xs text-muted">Rank</div>
            <div className="text-lg font-semibold">{view.rank.title}</div>
            <div className="text-xs text-ink-2">{view.rank.blurb}</div>
            <div className="mt-2 h-2 rounded-full bg-accent-soft">
              <div className="h-2 rounded-full bg-accent transition-[width] duration-700" style={{ width: `${Math.max(3, Math.min(1, pct) * 100)}%` }} />
            </div>
            <div className="mt-1 flex justify-between text-[11px] text-muted tabular">
              <span>{view.xp} XP</span>
              <span>
                ★ {view.stars}/{view.maxStars}
              </span>
              <span>{view.rank.next ? `${view.rank.next.title} at ${view.rank.next.xp}` : "Max rank"}</span>
            </div>
          </div>
        </div>
      </header>

      {weak.length > 0 && (
        <div className="card mb-4 flex flex-wrap items-center gap-x-4 gap-y-2 border-warn/40 p-4 text-sm">
          <span className="inline-flex items-center gap-2 font-semibold text-warn">
            <Icon name="target" size={15} /> Your dailies say: practise these
          </span>
          {weak.map((n) => (
            <button key={n.id} className="btn h-8 text-xs" onClick={() => navigate(`/paths/${view.id}/${n.id}`)}>
              {n.title} <span className="text-muted">· {n.weak!.challenge.title}</span>
            </button>
          ))}
        </div>
      )}

      {/* map */}
      <div className="card relative overflow-x-auto p-2">
        <div className="relative mx-auto" style={{ width: layout.width, height: layout.height }}>
          <svg width={layout.width} height={layout.height} className="absolute inset-0">
            <defs>
              <filter id="glow" x="-50%" y="-50%" width="200%" height="200%">
                <feGaussianBlur stdDeviation="4" result="b" />
                <feMerge>
                  <feMergeNode in="b" />
                  <feMergeNode in="SourceGraphic" />
                </feMerge>
              </filter>
            </defs>
            {/* section headers for branches */}
            {branches.map((s) => {
              const first = view.nodes.filter((n) => n.section === s.id).sort((a, b) => a.row - b.row)[0];
              const p = first && layout.pos.get(first.id);
              if (!p) return null;
              return (
                <g key={s.id}>
                  <text x={p.x} y={p.y - R - 34} textAnchor="middle" fontSize={13} fontWeight={700} fill={s.color} letterSpacing={0.5}>
                    {s.title.toUpperCase()}
                  </text>
                </g>
              );
            })}
            {/* gate banner */}
            {gatePos && (
              <g>
                <line x1={30} x2={layout.width - 30} y1={gatePos.y + ROW_H * 0.52} y2={gatePos.y + ROW_H * 0.52} stroke="var(--border-strong)" strokeDasharray="4 6" />
                <rect x={layout.width / 2 - 92} y={gatePos.y + ROW_H * 0.52 - 12} width={184} height={24} rx={12} fill="var(--surface)" stroke="var(--border-strong)" />
                <text x={layout.width / 2} y={gatePos.y + ROW_H * 0.52 + 4} textAnchor="middle" fontSize={11} fontWeight={600} fill="var(--text-2)">
                  {view.gateDone ? "Branches unlocked" : "Beat the boss to pick a branch"}
                </text>
              </g>
            )}
            {/* edges */}
            {view.nodes.flatMap((n) =>
              n.requires.map((r) => {
                const a = layout.pos.get(r)!;
                const b = layout.pos.get(n.id)!;
                const src = byId.get(r)!;
                const lit = src.status === "done";
                const my = (a.y + b.y) / 2;
                void my;
                return (
                  <path
                    key={`${r}-${n.id}`}
                    d={`M${a.x} ${a.y + R + LABEL_H} C${a.x} ${my + LABEL_H / 2}, ${b.x} ${my + LABEL_H / 2}, ${b.x} ${b.y - R - 4}`}
                    fill="none"
                    stroke={lit ? color(n.section) : "var(--border-strong)"}
                    strokeWidth={lit ? 3 : 2}
                    strokeDasharray={n.status === "locked" ? "5 6" : undefined}
                    opacity={lit ? 0.9 : 0.8}
                  />
                );
              }),
            )}
            {/* nodes */}
            {view.nodes.map((n) => {
              const p = layout.pos.get(n.id)!;
              const c = color(n.section);
              const boss = n.kind === "boss";
              const r = boss ? R + 6 : R;
              const icon: IconName = n.status === "locked" ? "lock" : n.status === "done" ? "check" : boss ? "trophy" : n.kind === "lesson" ? "book" : "code";
              const fill = n.status === "done" ? c : "var(--surface)";
              const ring = n.weak ? "var(--warn)" : n.status === "locked" ? "var(--border-strong)" : c;
              return (
                <g
                  key={n.id}
                  className="cursor-pointer"
                  onClick={() => navigate(`/paths/${view.id}/${n.id}`)}
                  onMouseEnter={() => setHover(n)}
                  onMouseLeave={() => setHover(null)}
                  opacity={n.status === "locked" ? 0.55 : 1}
                >
                  {(n.status === "available" || n.status === "in-progress") && (
                    <circle cx={p.x} cy={p.y} r={r + 7} fill="none" stroke={c} strokeWidth={2} opacity={0.35} className="tree-pulse" />
                  )}
                  {boss ? (
                    <polygon
                      points={Array.from({ length: 6 }, (_, k) => {
                        const ang = (Math.PI / 3) * k - Math.PI / 2;
                        return `${p.x + r * Math.cos(ang)},${p.y + r * Math.sin(ang)}`;
                      }).join(" ")}
                      fill={fill}
                      stroke={ring}
                      strokeWidth={3}
                      filter={n.status === "done" ? "url(#glow)" : undefined}
                    />
                  ) : (
                    <circle cx={p.x} cy={p.y} r={r} fill={fill} stroke={ring} strokeWidth={3} />
                  )}
                  {n.status === "in-progress" && (
                    <path d={`M${p.x} ${p.y - r} A${r} ${r} 0 0 1 ${p.x} ${p.y + r}`} fill="none" stroke={c} strokeWidth={6} opacity={0.6} />
                  )}
                  <foreignObject x={p.x - 10} y={p.y - 10} width={20} height={20} style={{ pointerEvents: "none" }}>
                    <div style={{ color: n.status === "done" ? "#fff" : n.status === "locked" ? "var(--muted)" : c }}>
                      <Icon name={icon} size={20} strokeWidth={2.2} />
                    </div>
                  </foreignObject>
                  {n.weak && (
                    <g>
                      <circle cx={p.x + r * 0.75} cy={p.y - r * 0.75} r={9} fill="var(--warn)" />
                      <text x={p.x + r * 0.75} y={p.y - r * 0.75 + 4} textAnchor="middle" fontSize={12} fontWeight={800} fill="var(--surface)">
                        !
                      </text>
                    </g>
                  )}
                  {/* stars */}
                  {n.status === "done" && (
                    <text x={p.x} y={p.y + r + 13} textAnchor="middle" fontSize={11} fill="#f2b33d" letterSpacing={1}>
                      {"★".repeat(n.stars)}
                      <tspan fill="var(--border-strong)">{"★".repeat(3 - n.stars)}</tspan>
                    </text>
                  )}
                  <NodeLabel x={p.x} y={p.y + r + (n.status === "done" ? 27 : 16)} text={n.title} dim={n.status === "locked"} />
                </g>
              );
            })}
          </svg>
          {/* Tess stands by the frontier */}
          {frontier && layout.pos.get(frontier.id) && (
            <div
              className="tess-bob pointer-events-none absolute"
              style={{ left: layout.pos.get(frontier.id)!.x + R + 12, top: layout.pos.get(frontier.id)!.y - 46 }}
            >
              <Mascot mood="wave" size={64} hat={hat} className="text-ink-2" />
            </div>
          )}
          {/* hover card */}
          {hover && layout.pos.get(hover.id) && (
            <div
              className="pointer-events-none absolute z-10 w-60 -translate-x-1/2 rounded-xl border border-line bg-surface p-3 text-xs shadow-xl"
              style={{ left: layout.pos.get(hover.id)!.x, top: layout.pos.get(hover.id)!.y + R + 46 }}
            >
              <div className="text-sm font-semibold text-ink">{hover.title}</div>
              <div className="mt-1 text-muted">
                {hover.kind === "boss" ? "Boss lab" : hover.kind === "lesson" ? "Lesson + quiz" : "Lesson + quiz + lab"} · ~{hover.estMinutes + (hover.kind === "lesson" ? 0 : 0)} min ·{" "}
                {hover.xp} XP
              </div>
              {hover.status === "locked" && (
                <div className="mt-2 text-ink-2">Needs: {hover.requires.map((r) => byId.get(r)?.title).join(", ")}</div>
              )}
              {hover.weak && (
                <div className="mt-2 text-warn">
                  Needs practice: “{hover.weak.challenge.title}” went rough ({hover.weak.reason}).
                </div>
              )}
              {!hover.ready && <div className="mt-2 text-warn">Being written by the curriculum team.</div>}
            </div>
          )}
        </div>
      </div>
      {gateNode && gateNode.status !== "done" && (
        <p className="mt-3 text-center text-xs text-muted">Branches unlock after “{gateNode.title}”. Fundamentals first; that's how the pros learned it too.</p>
      )}

      {choosing && (
        <div className="fixed inset-0 z-50 grid place-items-center bg-black/50 p-4 backdrop-blur-sm" onClick={() => setChoosing(false)}>
          <div className="card pop-in w-full max-w-3xl p-6" onClick={(e) => e.stopPropagation()}>
            <div className="flex items-center gap-4">
              <Mascot mood="cheer" size={90} hat={hat} className="shrink-0 text-ink-2" />
              <div>
                <h2 className="text-xl font-semibold">Fundamentals: done. Pick your branch.</h2>
                <p className="mt-1 text-sm text-ink-2">
                  You can explore them all eventually. Choose where to go deep first. This only highlights it on the map.
                </p>
              </div>
            </div>
            <div className="mt-5 grid gap-3 sm:grid-cols-3">
              {branches.map((s) => (
                <button
                  key={s.id}
                  className="rounded-xl border border-line p-4 text-left transition-colors hover:border-line-strong hover:bg-surface-2"
                  onClick={async () => {
                    await api.chooseBranch(view.id, s.id);
                    setChoosing(false);
                    load();
                  }}
                >
                  <span className="mb-2 block h-1.5 w-10 rounded-full" style={{ background: s.color }} />
                  <span className="block font-semibold text-ink">{s.title}</span>
                  <span className="mt-1 block text-xs text-ink-2">{s.blurb}</span>
                </button>
              ))}
            </div>
          </div>
        </div>
      )}
    </div>
  );
}

function NodeLabel({ x, y, text, dim }: { x: number; y: number; text: string; dim: boolean }) {
  // Wrap to ~20 chars per line, at most 3 lines.
  const words = text.split(" ");
  const lines: string[] = [];
  for (const w of words) {
    const cur = lines[lines.length - 1];
    if (cur !== undefined && (cur + " " + w).length <= 20) lines[lines.length - 1] = cur + " " + w;
    else lines.push(w);
  }
  return (
    <text x={x} y={y} textAnchor="middle" fontSize={11.5} fontWeight={600} fill={dim ? "var(--muted)" : "var(--text)"}>
      {lines.slice(0, 3).map((l, i) => (
        <tspan key={i} x={x} dy={i === 0 ? 0 : 13}>
          {l}
        </tspan>
      ))}
    </text>
  );
}
