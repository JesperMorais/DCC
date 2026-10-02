import { Fragment } from "react";
import type { PgConfig } from "../playground/scheduler";
import { Markdown } from "../ui";
import { Playground } from "./Playground";

/** Markdown with interactive ```playground blocks. */
export function LessonContent({ source }: { source: string }) {
  const parts: ({ md: string } | { pg: PgConfig } | { err: string })[] = [];
  let last = 0;
  for (const m of source.matchAll(/```playground\n([\s\S]*?)```/g)) {
    parts.push({ md: source.slice(last, m.index) });
    try {
      parts.push({ pg: JSON.parse(m[1]) as PgConfig });
    } catch (e) {
      parts.push({ err: (e as Error).message });
    }
    last = m.index! + m[0].length;
  }
  parts.push({ md: source.slice(last) });
  return (
    <>
      {parts.map((p, i) => (
        <Fragment key={i}>
          {"md" in p && p.md.trim() && <Markdown source={p.md} />}
          {"pg" in p && <Playground initial={p.pg} />}
          {"err" in p && <div className="my-4 rounded-lg border border-bad/30 bg-bad-soft p-3 text-xs text-bad">Playground config error: {p.err}</div>}
        </Fragment>
      ))}
    </>
  );
}
