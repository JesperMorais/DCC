import { useState } from "react";
import { api, type NodeView } from "../api";
import { read, write } from "../tess/prefs";
import { Icon, Markdown } from "../ui";
import { Mascot } from "./Mascot";

type Project = NonNullable<NodeView["project"]>;

/** A skill-tree project: built in the learner's own editor, ticked off milestone by milestone. */
export function ProjectView({ pathId, nodeId, project, color, onChange }: { pathId: string; nodeId: string; project: Project; color: string; onChange: () => void }) {
  const [copied, setCopied] = useState(false);
  const [busy, setBusy] = useState<string | null>(null);
  const done = project.milestones.filter((m) => m.doneAt).length;

  const toggle = async (id: string, value: boolean) => {
    setBusy(id);
    try {
      await api.setMilestone(pathId, nodeId, id, value);
      onChange();
    } finally {
      setBusy(null);
    }
  };

  return (
    <>
      <div className="mt-5 grid grid-cols-3 gap-2 text-xs">
        <Chip icon="clock" label={`~${project.estHours} h in your own editor`} />
        <Chip icon="flag" label={`${done} / ${project.milestones.length} milestones`} good={done === project.milestones.length} />
        <Chip icon="code" label={project.testsGiven ? "Tests given" : "You write the tests"} />
      </div>

      <article className="mt-8">
        <Markdown source={project.brief} />
      </article>

      <section className="mt-8">
        <h2 className="mb-3 flex items-center gap-2 text-lg font-semibold">
          <Icon name="terminal" size={18} /> Get the starter
        </h2>
        <div className="card p-4">
          <p className="text-sm text-ink-2">Copy the starter folder somewhere of your own, install, and open it in your editor. The README inside says how to run and test it.</p>
          <div className="mt-3 flex items-stretch gap-2">
            <code className="min-w-0 flex-1 overflow-x-auto rounded-lg border border-line bg-surface-2 px-3 py-2 font-mono text-[12px] whitespace-nowrap">{project.starterCommand}</code>
            <button
              className="btn shrink-0"
              onClick={() => {
                navigator.clipboard?.writeText(project.starterCommand);
                setCopied(true);
                setTimeout(() => setCopied(false), 1500);
              }}
            >
              <Icon name={copied ? "check" : "terminal"} size={14} /> {copied ? "Copied" : "Copy"}
            </button>
          </div>
        </div>
      </section>

      <section className="mt-10">
        <h2 className="mb-1 flex items-center gap-2 text-lg font-semibold">
          <Icon name="flag" size={18} /> Milestones
        </h2>
        <p className="mb-4 text-sm text-ink-2">
          Each one says what has to work, not how to build it. That part is yours. Stuck? The hints nudge you, one at a time, and never contain code.
        </p>
        <ol className="flex flex-col gap-3">
          {project.milestones.map((m, i) => (
            <Milestone key={m.id} pathId={pathId} nodeId={nodeId} index={i} m={m} color={color} busy={busy === m.id} onToggle={(v) => toggle(m.id, v)} />
          ))}
        </ol>
      </section>

      {project.review && (
        <section className="mt-10 rounded-2xl border border-good/30 bg-good-soft p-5">
          <div className="mb-3 flex items-center gap-4">
            <Mascot mood="cheer" size={64} className="shrink-0" />
            <div>
              <div className="font-semibold text-ink">Project done. You built that yourself.</div>
              <div className="text-sm text-ink-2">Here's how we'd structure it. Compare it with yours: different isn't wrong.</div>
            </div>
          </div>
          <Markdown source={project.review} />
        </section>
      )}
    </>
  );
}

function Milestone({
  pathId,
  nodeId,
  index,
  m,
  color,
  busy,
  onToggle,
}: {
  pathId: string;
  nodeId: string;
  index: number;
  m: Project["milestones"][number];
  color: string;
  busy: boolean;
  onToggle: (done: boolean) => void;
}) {
  // Revealed hints are a per-viewer convenience; they don't affect anything.
  const key = `project-hints:${pathId}/${nodeId}/${m.id}`;
  const [shown, setShown] = useState(() => Number(read(key) ?? 0));
  const reveal = () => {
    const next = Math.min(m.hints.length, shown + 1);
    write(key, String(next));
    setShown(next);
  };
  const done = !!m.doneAt;
  return (
    <li className={`rounded-xl border p-4 ${done ? "border-good/30 bg-good-soft" : "border-line bg-surface"}`}>
      <div className="flex items-start gap-3">
        <button
          className="mt-0.5 grid size-6 shrink-0 place-items-center rounded-md border-2 transition-colors disabled:opacity-50"
          style={{ borderColor: done ? "var(--good)" : color, background: done ? "var(--good)" : "transparent" }}
          onClick={() => onToggle(!done)}
          disabled={busy}
          aria-label={done ? `Mark milestone ${index + 1} as not done` : `Mark milestone ${index + 1} as done`}
        >
          {done && <Icon name="check" size={14} className="text-white" strokeWidth={3} />}
        </button>
        <div className="min-w-0 flex-1">
          <div className="text-xs font-semibold tracking-wide text-muted uppercase">Milestone {index + 1}</div>
          <div className="font-semibold text-ink">{m.title}</div>
          <Markdown source={m.body} className="mt-2 text-sm" />
          {shown > 0 && (
            <ol className="mt-3 flex flex-col gap-2">
              {m.hints.slice(0, shown).map((h, k) => (
                <li key={k} className="flex gap-2 rounded-lg bg-surface-2 px-3 py-2 text-sm text-ink-2">
                  <Icon name="bulb" size={14} className="mt-0.5 shrink-0 text-warn" />
                  <Markdown source={h} />
                </li>
              ))}
            </ol>
          )}
          {!done && shown < m.hints.length && (
            <button className="btn btn-ghost mt-2 h-8 px-2 text-xs" onClick={reveal}>
              <Icon name="bulb" size={13} /> {shown === 0 ? "Stuck? Get a nudge" : `Another hint (${shown + 1} of ${m.hints.length})`}
            </button>
          )}
        </div>
      </div>
    </li>
  );
}

function Chip({ icon, label, good = false }: { icon: "clock" | "flag" | "code"; label: string; good?: boolean }) {
  return (
    <div className={`flex items-center gap-2 rounded-lg border px-3 py-2 ${good ? "border-good/30 bg-good-soft text-good" : "border-line bg-surface text-ink-2"}`}>
      <Icon name={good ? "check" : icon} size={14} />
      {label}
    </div>
  );
}
