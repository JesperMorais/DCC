// Tess the pangolin: the daily.ts mascot. Scales = types: armour for your code.
// Drawn as inline SVG so it scales crisply, themes cleanly and works offline.
import { useEffect, useId, useState, type ReactNode } from "react";

export type Mood = "idle" | "wave" | "happy" | "think" | "sleep" | "sad" | "cheer" | "read" | "curl";

const C = {
  outline: "#15325a",
  scaleTop: "#245fa6",
  scaleBottom: "#4b8fe0",
  scaleEdge: "#173f73",
  scaleShine: "#8cc0f5",
  cream: "#f7e7d0",
  creamShade: "#ead0ad",
  snout: "#f0d7b6",
  nose: "#3a2a2c",
  eye: "#171b26",
  cheek: "#f4a3b4",
  claw: "#c9a47e",
};

/** Overlapping scales, drawn bottom row first so each row's tips overlap the row below. */
function Scales({ gid, x0, y0, x1, y1, w = 13, h = 12, step = 8 }: { gid: string; x0: number; y0: number; x1: number; y1: number; w?: number; h?: number; step?: number }) {
  const rows: ReactNode[] = [];
  const count = Math.ceil((y1 - y0) / step) + 1;
  for (let r = count; r >= 0; r--) {
    const cy = y0 + r * step - h / 2;
    const offset = r % 2 ? w / 2 : 0;
    for (let cx = x0 - w + offset; cx <= x1 + w; cx += w) {
      rows.push(
        <g key={`${r}-${cx}`}>
          <path
            d={`M${cx - w / 2} ${cy} C${cx - w / 2} ${cy + h * 0.75} ${cx - w * 0.22} ${cy + h} ${cx} ${cy + h} C${cx + w * 0.22} ${cy + h} ${cx + w / 2} ${cy + h * 0.75} ${cx + w / 2} ${cy}Z`}
            fill={`url(#${gid})`}
            stroke={C.scaleEdge}
            strokeWidth={1.1}
          />
          <path
            d={`M${cx - w * 0.24} ${cy + h * 0.7} Q${cx} ${cy + h * 0.92} ${cx + w * 0.24} ${cy + h * 0.7}`}
            fill="none"
            stroke={C.scaleShine}
            strokeWidth={1.2}
            strokeLinecap="round"
            opacity={0.55}
          />
        </g>,
      );
    }
  }
  return <>{rows}</>;
}

/** A silhouette filled with scales and outlined like a sticker. */
function Scaly({ id, d, gid, box }: { id: string; d: string; gid: string; box: [number, number, number, number] }) {
  return (
    <g>
      <clipPath id={id}>
        <path d={d} />
      </clipPath>
      <path d={d} fill={C.scaleTop} />
      <g clipPath={`url(#${id})`}>
        <Scales gid={gid} x0={box[0]} y0={box[1]} x1={box[2]} y1={box[3]} />
      </g>
      <path d={d} fill="none" stroke={C.outline} strokeWidth={2.6} strokeLinejoin="round" />
    </g>
  );
}

function Arm({ from, to }: { from: [number, number]; to: [number, number] }) {
  const [x1, y1] = from;
  const [x2, y2] = to;
  const len = Math.hypot(x2 - x1, y2 - y1) || 1;
  const [ux, uy] = [(x2 - x1) / len, (y2 - y1) / len];
  const [px, py] = [-uy, ux];
  // Pangolins have big digging claws: three little hooks at the paw.
  const claws = [-3.2, 0, 3.2].map((o) => {
    const bx = x2 + px * o + ux * 3.5;
    const by = y2 + py * o + uy * 3.5;
    return `M${bx} ${by} q${ux * 3 + px * 0.8} ${uy * 3 + py * 0.8} ${ux * 4.5 - px * 0.6} ${uy * 4.5 - py * 0.6}`;
  });
  return (
    <g strokeLinecap="round">
      <line x1={x1} y1={y1} x2={x2} y2={y2} stroke={C.outline} strokeWidth={15} />
      <line x1={x1} y1={y1} x2={x2} y2={y2} stroke={C.cream} strokeWidth={10.4} />
      <path d={claws.join(" ")} fill="none" stroke={C.outline} strokeWidth={3.4} />
      <path d={claws.join(" ")} fill="none" stroke={C.claw} strokeWidth={1.6} />
    </g>
  );
}

const BODY = "M80 64 C110 64 128 92 126 120 C124 144 106 155 80 155 C54 155 36 144 34 120 C32 92 50 64 80 64Z";
const HOOD = "M45 60 C39 30 58 13 80 13 C102 13 121 30 115 60 C109 48 97 42 80 42 C63 42 51 48 45 60Z";
const TAIL = "M108 132 C132 150 162 134 156 104 C153 88 138 80 128 88 C124 92 126 99 132 98 C140 97 145 104 142 114 C138 128 124 130 114 120Z";

export function Mascot({
  mood = "idle",
  size = 120,
  className = "",
  title,
  animated = false,
}: {
  mood?: Mood;
  size?: number;
  className?: string;
  title?: string;
  /** Gentle idle bob + blink. Only for calm, non-coding surfaces. */
  animated?: boolean;
}) {
  const uid = useId().replace(/:/g, "");
  const gid = `sg${uid}`;
  if (mood === "curl") return <CurledTess uid={uid} size={size} className={className} title={title} />;

  const arms: Record<Mood, [[number, number], [number, number]][]> = {
    idle: [[[60, 102], [66, 114]], [[100, 102], [94, 114]]],
    wave: [[[60, 102], [66, 114]], [[104, 98], [118, 80]]],
    happy: [[[58, 98], [42, 80]], [[102, 98], [118, 80]]],
    cheer: [[[60, 102], [66, 114]], [[102, 100], [106, 86]]],
    think: [[[60, 102], [66, 114]], [[100, 102], [89, 92]]],
    sleep: [[[60, 104], [70, 116]], [[100, 104], [90, 116]]],
    sad: [[[60, 104], [66, 118]], [[100, 104], [94, 118]]],
    read: [[[60, 102], [60, 106]], [[100, 102], [100, 106]]],
    curl: [],
  };

  const eyes = (() => {
    switch (mood) {
      case "happy":
      case "cheer":
        return (
          <g fill="none" stroke={C.eye} strokeWidth={3} strokeLinecap="round">
            <path d="M60 57 Q66 50 72 57" />
            <path d="M88 57 Q94 50 100 57" />
          </g>
        );
      case "sleep":
        return (
          <g fill="none" stroke={C.eye} strokeWidth={2.6} strokeLinecap="round">
            <path d="M61 55 Q66 60 71 55" />
            <path d="M89 55 Q94 60 99 55" />
          </g>
        );
      default: {
        const look = mood === "think" ? { dx: 1.5, dy: -2 } : mood === "read" ? { dx: 0, dy: 2 } : { dx: 0, dy: 0 };
        return (
          <g className={animated ? "tess-blink" : undefined}>
            {[65, 95].map((x) => (
              <g key={x}>
                <ellipse cx={x + look.dx} cy={56 + look.dy} rx={5} ry={5.9} fill={C.eye} />
                <circle cx={x + look.dx + 1.7} cy={53.8 + look.dy} r={1.9} fill="#fff" />
                <circle cx={x + look.dx - 1.5} cy={58.2 + look.dy} r={0.8} fill="#fff" opacity={0.8} />
              </g>
            ))}
            {mood === "sad" && (
              <g stroke={C.creamShade} strokeWidth={2.4} strokeLinecap="round">
                <path d="M60 46 L70 44" />
                <path d="M100 46 L90 44" />
              </g>
            )}
          </g>
        );
      }
    }
  })();

  const mouth =
    mood === "sad" ? (
      <path d="M76 86 Q80 83 84 86" fill="none" stroke={C.nose} strokeWidth={1.8} strokeLinecap="round" />
    ) : mood === "happy" || mood === "cheer" || mood === "wave" ? (
      <path d="M75 83 Q80 89 85 83" fill={C.nose} stroke={C.nose} strokeWidth={1.4} strokeLinejoin="round" />
    ) : mood === "sleep" ? (
      <ellipse cx={80} cy={85} rx={2} ry={1.6} fill={C.nose} />
    ) : (
      <path d="M76 84 Q80 87 84 84" fill="none" stroke={C.nose} strokeWidth={1.8} strokeLinecap="round" />
    );

  return (
    <svg
      viewBox="0 0 170 170"
      width={size}
      height={size}
      className={`${animated ? "tess-bob " : ""}${className}`}
      role="img"
      aria-label={title ?? `Tess the pangolin${mood !== "idle" ? ` (${mood})` : ""}`}
    >
      <defs>
        <linearGradient id={gid} x1="0" y1="0" x2="0" y2="1">
          <stop offset="0" stopColor={C.scaleTop} />
          <stop offset="1" stopColor={C.scaleBottom} />
        </linearGradient>
      </defs>

      {/* ground shadow */}
      <ellipse cx={86} cy={158} rx={52} ry={6} fill="#000" opacity={0.12} />

      <Scaly id={`t${uid}`} d={TAIL} gid={gid} box={[104, 80, 160, 150]} />
      <Scaly id={`b${uid}`} d={BODY} gid={gid} box={[30, 62, 130, 158]} />

      {/* belly */}
      <ellipse cx={80} cy={123} rx={26} ry={28} fill={C.cream} stroke={C.outline} strokeWidth={2.2} />
      <path d="M66 112 Q80 106 94 112 M64 124 Q80 118 96 124 M66 136 Q80 131 94 136" fill="none" stroke={C.creamShade} strokeWidth={1.6} strokeLinecap="round" />

      {/* feet */}
      {[64, 96].map((x) => (
        <g key={x}>
          <ellipse cx={x} cy={154} rx={12} ry={6.5} fill={C.cream} stroke={C.outline} strokeWidth={2.2} />
          <path d={`M${x - 5} 157 v3 M${x} 158 v3 M${x + 5} 157 v3`} stroke={C.claw} strokeWidth={1.8} strokeLinecap="round" />
        </g>
      ))}

      {arms[mood].map(([from, to], i) => (
        <Arm key={i} from={from} to={to} />
      ))}

      {mood === "read" && (
        <g>
          <path d="M55 95 L80 98 L105 95 L105 115 L80 118 L55 115Z" fill={C.scaleTop} stroke={C.outline} strokeWidth={2.2} strokeLinejoin="round" />
          <path d="M58 96 L79 99 L79 114.5 L58 112Z" fill="#fffaf2" />
          <path d="M81 99 L102 96 L102 112 L81 114.5Z" fill="#fffaf2" />
          <path d="M62 101 L75 102.6 M62 105 L75 106.6 M62 109 L72 110.2 M85 102.6 L98 101 M85 106.6 L98 105 M85 110.4 L95 109.2" stroke={C.creamShade} strokeWidth={1.3} strokeLinecap="round" />
          <line x1={80} y1={98.5} x2={80} y2={118} stroke={C.outline} strokeWidth={1.6} />
          {[56, 104].map((x) => (
            <g key={x}>
              <circle cx={x} cy={106} r={5.6} fill={C.cream} stroke={C.outline} strokeWidth={2} />
              <circle cx={x} cy={106} r={1.2} fill={C.claw} />
            </g>
          ))}
        </g>
      )}

      {/* head */}
      <ellipse cx={80} cy={58} rx={35} ry={30} fill={C.cream} stroke={C.outline} strokeWidth={2.6} />
      <Scaly id={`h${uid}`} d={HOOD} gid={gid} box={[40, 12, 120, 60]} />
      {/* ears */}
      <path d="M45 54 Q38 50 40 43 Q47 45 49 52Z M115 54 Q122 50 120 43 Q113 45 111 52Z" fill={C.creamShade} stroke={C.outline} strokeWidth={2} strokeLinejoin="round" />

      {/* cheeks */}
      <ellipse cx={58} cy={67} rx={6} ry={3.6} fill={C.cheek} opacity={0.75} />
      <ellipse cx={102} cy={67} rx={6} ry={3.6} fill={C.cheek} opacity={0.75} />

      {eyes}

      {/* snout: tapered, blending into the face at the top */}
      <defs>
        <linearGradient id={`sn${uid}`} x1="0" y1="0" x2="0" y2="1">
          <stop offset="0" stopColor={C.cream} />
          <stop offset="1" stopColor={C.creamShade} />
        </linearGradient>
      </defs>
      <path d="M72 50 C72.5 62 74.5 71 77.5 77 C78.8 79.4 81.2 79.4 82.5 77 C85.5 71 87.5 62 88 50 Q80 46 72 50Z" fill={`url(#sn${uid})`} />
      <path d="M72.6 57 C73.6 65 75 71.5 77.5 77 C78.8 79.4 81.2 79.4 82.5 77 C85 71.5 86.4 65 87.4 57" fill="none" stroke={C.outline} strokeWidth={2} strokeLinecap="round" />
      <ellipse cx={80} cy={77} rx={4.6} ry={3.3} fill={C.nose} />
      <circle cx={81.6} cy={75.9} r={1.1} fill="#fff" opacity={0.6} />
      {mouth}

      {/* extras */}
      {mood === "happy" && (
        <g fill="#f2b33d">
          <Sparkle x={24} y={40} s={1} />
          <Sparkle x={140} y={34} s={0.8} />
          <Sparkle x={148} y={70} s={0.55} />
        </g>
      )}
      {mood === "sleep" && (
        <g fill="currentColor" fontFamily="ui-sans-serif, system-ui" fontWeight={700} opacity={0.55}>
          <text x={116} y={30} fontSize={16}>z</text>
          <text x={130} y={18} fontSize={12}>z</text>
        </g>
      )}
      {mood === "think" && (
        <g>
          <circle cx={122} cy={38} r={3} fill="currentColor" opacity={0.35} />
          <circle cx={132} cy={26} r={4.5} fill="currentColor" opacity={0.45} />
          <text x={138} y={20} fontSize={22} fontWeight={800} fill="currentColor" opacity={0.6} fontFamily="ui-sans-serif, system-ui">?</text>
        </g>
      )}
      {mood === "sad" && <path d="M104 60 C101 66 102 70 105 70 C108 70 109 66 104 60Z" fill="#8cc0f5" stroke={C.outline} strokeWidth={1.2} />}
    </svg>
  );
}

/** Rolled into a ball: what a pangolin does when it's waiting something out. */
function CurledTess({ uid, size, className, title }: { uid: string; size: number; className: string; title?: string }) {
  const gid = `sg${uid}`;
  const ball = "M38 112 A46 46 0 1 0 130 112 A46 46 0 1 0 38 112Z";
  // Tail wraps around the outside of the ball, lower-left.
  const band = "M112 155 A50 50 0 0 1 34.6 105 Q35.5 97.5 43.5 98.5 L46.8 106 A38 38 0 0 0 105.5 145 Q114 147.5 112 155Z";
  return (
    <svg viewBox="22 50 124 124" width={size} height={size} className={className} role="img" aria-label={title ?? "Tess the pangolin, curled into a ball"}>
      <defs>
        <linearGradient id={gid} x1="0" y1="0" x2="0" y2="1">
          <stop offset="0" stopColor={C.scaleTop} />
          <stop offset="1" stopColor={C.scaleBottom} />
        </linearGradient>
      </defs>
      <ellipse cx={84} cy={163} rx={48} ry={5.5} fill="#000" opacity={0.12} />
      <Scaly id={`cb${uid}`} d={ball} gid={gid} box={[36, 64, 132, 160]} />
      <Scaly id={`ct${uid}`} d={band} gid={gid} box={[30, 94, 118, 160]} />
      {/* snout tip + nose peeking out where the tail tucks in */}
      <path d="M44 104 C39.5 99 36.5 94.5 35.2 90.6 C34.4 87.6 36.8 85.4 39.6 86.8 C45 89.4 51 91.6 56 94.5Z" fill={C.snout} stroke={C.outline} strokeWidth={2} strokeLinejoin="round" />
      <ellipse cx={36.4} cy={88.6} rx={3.9} ry={3} fill={C.nose} transform="rotate(35 36.4 88.6)" />
      <path d="M55 86 Q58 88.5 61 86" fill="none" stroke={C.eye} strokeWidth={2.2} strokeLinecap="round" />
      <g fill="currentColor" fontFamily="ui-sans-serif, system-ui" fontWeight={700} opacity={0.5}>
        <text x={124} y={62} fontSize={13}>…</text>
      </g>
    </svg>
  );
}

function Sparkle({ x, y, s }: { x: number; y: number; s: number }) {
  return <path transform={`translate(${x} ${y}) scale(${s})`} d="M0 -10 C1.5 -2 2 -1.5 10 0 C2 1.5 1.5 2 0 10 C-1.5 2 -2 1.5 -10 0 C-2 -1.5 -1.5 -2 0 -10Z" />;
}

/** Tess with a speech bubble, for empty states and friendly nudges. */
export function TessSays({ mood = "idle", children, size = 88 }: { mood?: Mood; children: ReactNode; size?: number }) {
  return (
    <div className="flex items-end gap-3">
      <Mascot mood={mood} size={size} className="shrink-0 text-ink-2" />
      <div className="relative mb-4 rounded-2xl rounded-bl-sm border border-line bg-surface-2 px-3.5 py-2.5 text-sm text-ink-2">{children}</div>
    </div>
  );
}

/** A compact one-line Tess reaction (results panel, nudges). */
export function TessRow({
  mood,
  children,
  actions,
  onDismiss,
  tone = "neutral",
}: {
  mood: Mood;
  children: ReactNode;
  actions?: ReactNode;
  onDismiss?: () => void;
  tone?: "neutral" | "good" | "accent";
}) {
  const toneCls =
    tone === "good" ? "border-good/30 bg-good-soft" : tone === "accent" ? "border-accent/30 bg-accent-soft" : "border-line bg-surface-2";
  return (
    <div className={`pop-in flex items-center gap-3 rounded-xl border px-3 py-2 ${toneCls}`}>
      <Mascot mood={mood} size={mood === "curl" ? 44 : 36} className="-my-1 shrink-0 text-ink-2" />
      <div className="min-w-0 flex-1 text-[13px] leading-snug text-ink-2">{children}</div>
      {actions}
      {onDismiss && (
        <button onClick={onDismiss} className="rounded-md p-1 text-muted hover:bg-surface-3 hover:text-ink" aria-label="Dismiss">
          <svg width="14" height="14" viewBox="0 0 24 24" fill="none" stroke="currentColor" strokeWidth="2" strokeLinecap="round">
            <path d="M18 6 6 18M6 6l12 12" />
          </svg>
        </button>
      )}
    </div>
  );
}

/** Loading state that only shows after 300ms, so fast loads don't flash. */
export function TessLoading({ label = "Loading…" }: { label?: string }) {
  const [show, setShow] = useState(false);
  useEffect(() => {
    const t = setTimeout(() => setShow(true), 300);
    return () => clearTimeout(t);
  }, []);
  if (!show) return null;
  return (
    <div className="grid h-full place-items-center">
      <div className="flex flex-col items-center gap-2 text-sm text-muted">
        <Mascot mood="think" size={96} className="text-ink-2" />
        {label}
      </div>
    </div>
  );
}

