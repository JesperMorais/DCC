import { marked } from "marked";
import { useMemo, type ReactNode, type SVGProps } from "react";

/* ---------- Icons (lucide-style strokes, inline so everything works offline) ---------- */
const paths = {
  dashboard: "M3 3h7v9H3zM14 3h7v5h-7zM14 12h7v9h-7zM3 16h7v5H3z",
  library: "M4 19.5A2.5 2.5 0 0 1 6.5 17H20V3H6.5A2.5 2.5 0 0 0 4 5.5v14zM4 19.5A2.5 2.5 0 0 0 6.5 22H20v-5",
  settings:
    "M12 15a3 3 0 1 0 0-6 3 3 0 0 0 0 6zM19.4 15a1.65 1.65 0 0 0 .33 1.82l.06.06a2 2 0 1 1-2.83 2.83l-.06-.06a1.65 1.65 0 0 0-1.82-.33 1.65 1.65 0 0 0-1 1.51V21a2 2 0 1 1-4 0v-.09A1.65 1.65 0 0 0 9 19.4a1.65 1.65 0 0 0-1.82.33l-.06.06a2 2 0 1 1-2.83-2.83l.06-.06A1.65 1.65 0 0 0 4.68 15a1.65 1.65 0 0 0-1.51-1H3a2 2 0 1 1 0-4h.09A1.65 1.65 0 0 0 4.6 9a1.65 1.65 0 0 0-.33-1.82l-.06-.06a2 2 0 1 1 2.83-2.83l.06.06A1.65 1.65 0 0 0 9 4.68a1.65 1.65 0 0 0 1-1.51V3a2 2 0 1 1 4 0v.09a1.65 1.65 0 0 0 1 1.51 1.65 1.65 0 0 0 1.82-.33l.06-.06a2 2 0 1 1 2.83 2.83l-.06.06A1.65 1.65 0 0 0 19.4 9a1.65 1.65 0 0 0 1.51 1H21a2 2 0 1 1 0 4h-.09a1.65 1.65 0 0 0-1.51 1z",
  flame: "M8.5 14.5A2.5 2.5 0 0 0 11 12c0-1.38-.5-2-1-3-1.07-2.14-.22-4.05 2-6 .5 2.5 2 4.9 4 6.5 2 1.6 3 3.5 3 5.5a7 7 0 1 1-14 0c0-1.15.43-2.29 1-3a2.5 2.5 0 0 0 2.5 2.5z",
  play: "M6 4l14 8-14 8z",
  pause: "M6 4h4v16H6zM14 4h4v16h-4z",
  check: "M20 6 9 17l-5-5",
  x: "M18 6 6 18M6 6l12 12",
  bulb: "M9 18h6M10 22h4M15.09 14c.18-.98.65-1.74 1.41-2.5A4.65 4.65 0 0 0 18 8 6 6 0 0 0 6 8c0 1 .23 2.23 1.5 3.5A4.61 4.61 0 0 1 8.91 14",
  flag: "M4 15s1-1 4-1 5 2 8 2 4-1 4-1V3s-1 1-4 1-5-2-8-2-4 1-4 1zM4 22v-7",
  clock: "M12 22a10 10 0 1 0 0-20 10 10 0 0 0 0 20zM12 6v6l4 2",
  trophy:
    "M6 9H4.5a2.5 2.5 0 0 1 0-5H6M18 9h1.5a2.5 2.5 0 0 0 0-5H18M4 22h16M10 14.66V17c0 .55-.47.98-.97 1.21C7.85 18.75 7 20.24 7 22M14 14.66V17c0 .55.47.98.97 1.21C16.15 18.75 17 20.24 17 22M18 2H6v7a6 6 0 0 0 12 0V2z",
  back: "M19 12H5M12 19l-7-7 7-7",
  chevron: "m9 18 6-6-6-6",
  sun: "M12 17a5 5 0 1 0 0-10 5 5 0 0 0 0 10zM12 1v2M12 21v2M4.22 4.22l1.42 1.42M18.36 18.36l1.42 1.42M1 12h2M21 12h2M4.22 19.78l1.42-1.42M18.36 5.64l1.42-1.42",
  moon: "M21 12.79A9 9 0 1 1 11.21 3 7 7 0 0 0 21 12.79z",
  target: "M12 22a10 10 0 1 0 0-20 10 10 0 0 0 0 20zM12 18a6 6 0 1 0 0-12 6 6 0 0 0 0 12zM12 14a2 2 0 1 0 0-4 2 2 0 0 0 0 4z",
  code: "m16 18 6-6-6-6M8 6l-6 6 6 6",
  sparkles: "M12 3l1.9 5.8L20 11l-6.1 2.2L12 19l-1.9-5.8L4 11l6.1-2.2zM19 3v4M21 5h-4",
  zap: "M13 2 3 14h9l-1 8 10-12h-9l1-8z",
  book: "M2 3h6a4 4 0 0 1 4 4v14a3 3 0 0 0-3-3H2zM22 3h-6a4 4 0 0 0-4 4v14a3 3 0 0 1 3-3h7z",
  terminal: "m4 17 6-6-6-6M12 19h8",
  refresh: "M3 12a9 9 0 0 1 15-6.7L21 8M21 3v5h-5M21 12a9 9 0 0 1-15 6.7L3 16M3 21v-5h5",
  eye: "M1 12s4-8 11-8 11 8 11 8-4 8-11 8-11-8-11-8zM12 15a3 3 0 1 0 0-6 3 3 0 0 0 0 6z",
  lock: "M5 11h14v10H5zM8 11V7a4 4 0 0 1 8 0v4",
  alert: "M10.29 3.86 1.82 18a2 2 0 0 0 1.71 3h16.94a2 2 0 0 0 1.71-3L13.71 3.86a2 2 0 0 0-3.42 0zM12 9v4M12 17h.01",
} as const;

export type IconName = keyof typeof paths;

export function Icon({ name, size = 16, ...rest }: { name: IconName; size?: number } & SVGProps<SVGSVGElement>) {
  return (
    <svg
      width={size}
      height={size}
      viewBox="0 0 24 24"
      fill="none"
      stroke="currentColor"
      strokeWidth={1.9}
      strokeLinecap="round"
      strokeLinejoin="round"
      aria-hidden
      {...rest}
    >
      <path d={paths[name]} />
    </svg>
  );
}

/* ---------- Small building blocks ---------- */
export const LEVEL_NAMES = ["", "First steps", "Building blocks", "Shaping data", "Generics", "Advanced patterns", "Type wizardry"];

export function LevelBadge({ level, name }: { level: number; name?: string }) {
  return (
    <span className="inline-flex items-center gap-1.5 rounded-md border border-line bg-surface-2 px-1.5 py-0.5 text-[11px] font-semibold tracking-wide text-ink-2">
      <LevelPips level={level} />
      L{level}
      {name && <span className="font-medium text-muted">· {name}</span>}
    </span>
  );
}

export function LevelPips({ level }: { level: number }) {
  return (
    <span className="inline-flex items-end gap-[2px]" aria-hidden>
      {[1, 2, 3, 4, 5, 6].map((i) => (
        <span
          key={i}
          className="w-[3px] rounded-[1px]"
          style={{ height: 4 + i * 1.3, background: i <= level ? "var(--accent)" : "var(--surface-3)" }}
        />
      ))}
    </span>
  );
}

export function Tag({ children }: { children: ReactNode }) {
  return <span className="rounded-md bg-surface-2 px-1.5 py-0.5 text-[11px] font-medium text-ink-2">{children}</span>;
}

export function StatusDot({ status }: { status: string }) {
  const map: Record<string, { c: string; label: string; icon: IconName }> = {
    solved: { c: "var(--good)", label: "Solved", icon: "check" },
    "gave-up": { c: "var(--bad)", label: "Gave up", icon: "flag" },
    "in-progress": { c: "var(--warn)", label: "In progress", icon: "clock" },
    new: { c: "var(--muted)", label: "New", icon: "sparkles" },
    "not-started": { c: "var(--muted)", label: "Not started", icon: "sparkles" },
  };
  const s = map[status] ?? map.new;
  return (
    <span className="inline-flex items-center gap-1 text-xs font-medium text-ink-2">
      <Icon name={s.icon} size={13} style={{ color: s.c }} />
      {s.label}
    </span>
  );
}

export const fmtMinutes = (m: number | null | undefined) => {
  if (m === null || m === undefined || !Number.isFinite(m)) return "—";
  if (m < 1) return `${Math.max(1, Math.round(m * 60))}s`;
  if (m < 60) return `${Math.floor(m)}m ${String(Math.round((m % 1) * 60)).padStart(2, "0")}s`;
  return `${Math.floor(m / 60)}h ${Math.round(m % 60)}m`;
};

export const fmtClock = (ms: number) => {
  const s = Math.max(0, Math.floor(ms / 1000));
  return `${String(Math.floor(s / 60)).padStart(2, "0")}:${String(s % 60).padStart(2, "0")}`;
};

export const relTime = (iso: string) => {
  const diff = (Date.now() - Date.parse(iso)) / 1000;
  if (diff < 60) return "just now";
  if (diff < 3600) return `${Math.floor(diff / 60)}m ago`;
  if (diff < 86400) return `${Math.floor(diff / 3600)}h ago`;
  const d = Math.floor(diff / 86400);
  return d === 1 ? "yesterday" : `${d}d ago`;
};

/* ---------- Markdown with lightweight TS highlighting ---------- */
const KEYWORDS =
  "const|let|var|function|return|if|else|for|of|in|while|do|switch|case|break|continue|new|class|extends|implements|interface|type|enum|import|export|from|as|async|await|throw|try|catch|finally|typeof|keyof|infer|readonly|public|private|protected|static|this|null|undefined|true|false|void|never|unknown|any|satisfies|is|asserts|declare|default";
const TYPES = "string|number|boolean|bigint|symbol|object|Array|Record|Partial|Pick|Omit|Readonly|Promise|Map|Set|ReturnType|Parameters";
const TOKEN_RE = new RegExp(
  [
    "(\\/\\/[^\\n]*|\\/\\*[\\s\\S]*?\\*\\/)", // comments
    "(`(?:\\\\.|[^`])*`|\"(?:\\\\.|[^\"])*\"|'(?:\\\\.|[^'])*')", // strings
    `\\b(${KEYWORDS})\\b`,
    `\\b(${TYPES}|[A-Z][A-Za-z0-9_]*)\\b`,
    "\\b(\\d+(?:\\.\\d+)?)\\b",
  ].join("|"),
  "g",
);

const escapeHtml = (s: string) => s.replace(/&/g, "&amp;").replace(/</g, "&lt;").replace(/>/g, "&gt;");

export function highlightTs(code: string) {
  let out = "";
  let last = 0;
  for (const m of code.matchAll(TOKEN_RE)) {
    out += escapeHtml(code.slice(last, m.index));
    const cls = m[1] ? "tok-com" : m[2] ? "tok-str" : m[3] ? "tok-kw" : m[4] ? "tok-type" : "tok-num";
    out += `<span class="${cls}">${escapeHtml(m[0])}</span>`;
    last = m.index! + m[0].length;
  }
  return out + escapeHtml(code.slice(last));
}

const renderer = new marked.Renderer();
renderer.code = ({ text, lang }) => {
  const body = !lang || /^(ts|typescript|js|javascript)$/.test(lang) ? highlightTs(text) : escapeHtml(text);
  return `<pre><code>${body}</code></pre>`;
};

export function Markdown({ source, className = "" }: { source: string; className?: string }) {
  const html = useMemo(() => marked.parse(source, { renderer, async: false }) as string, [source]);
  return <div className={`prose-ts ${className}`} dangerouslySetInnerHTML={{ __html: html }} />;
}

export function CodeBlock({ code, className = "" }: { code: string; className?: string }) {
  return (
    <pre
      className={`overflow-auto rounded-xl border border-line bg-surface-2 p-4 font-mono text-[12.5px] leading-relaxed ${className}`}
      dangerouslySetInnerHTML={{ __html: highlightTs(code) }}
    />
  );
}
