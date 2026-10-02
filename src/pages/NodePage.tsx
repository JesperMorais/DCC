import { useEffect, useState } from "react";
import { Link, useNavigate, useParams } from "react-router-dom";
import { api, type NodeView, type QuizResult } from "../api";
import { LessonContent } from "../components/LessonContent";
import { Mascot, TessLoading } from "../components/Mascot";
import { Quiz } from "../components/Quiz";
import { LANGUAGES } from "../lang";
import { Icon } from "../ui";

export default function NodePage() {
  const { pathId = "embedded", nodeId = "" } = useParams();
  const navigate = useNavigate();
  const [node, setNode] = useState<NodeView | null>(null);
  const [celebrate, setCelebrate] = useState<QuizResult | null>(null);

  const load = () => api.node(pathId, nodeId).then(setNode);
  useEffect(() => {
    setNode(null);
    setCelebrate(null);
    load();
    document.querySelector("main")?.scrollTo(0, 0);
  }, [pathId, nodeId]);

  if (!node) return <TessLoading label="Opening the lesson…" />;
  const c = node.section.color;
  const done = node.status === "done";
  const hat = node.language === "c" ? "hardhat" : undefined;
  const langName = LANGUAGES[node.language].name;

  if (node.status === "locked")
    return (
      <div className="mx-auto max-w-2xl px-4 py-10 sm:px-8">
        <Link to={`/paths/${pathId}`} className="inline-flex items-center gap-1.5 text-sm text-muted hover:text-ink">
          <Icon name="back" size={14} /> Back to the tree
        </Link>
        <div className="card mt-6 flex items-center gap-5 p-6">
          <Mascot mood="think" size={96} className="shrink-0 text-ink-2" />
          <div>
            <h1 className="text-xl font-semibold">{node.title} is locked</h1>
            <p className="mt-1 text-sm text-ink-2">Finish these first. Each one builds a piece you'll need here:</p>
            <ul className="mt-3 flex flex-col gap-1.5">
              {node.requires.map((r) => (
                <li key={r.id}>
                  <Link to={`/paths/${pathId}/${r.id}`} className="inline-flex items-center gap-2 text-sm hover:text-accent">
                    <Icon name={r.done ? "check" : "lock"} size={14} className={r.done ? "text-good" : "text-muted"} />
                    {r.title}
                  </Link>
                </li>
              ))}
            </ul>
          </div>
        </div>
      </div>
    );

  return (
    <div className="mx-auto max-w-3xl px-4 py-8 sm:px-8">
      <div className="flex flex-wrap items-center gap-2 text-sm text-muted">
        <Link to={`/paths/${pathId}`} className="inline-flex items-center gap-1.5 hover:text-ink">
          <Icon name="back" size={14} /> {node.pathTitle}
        </Link>
        <span>›</span>
        <span className="inline-flex items-center gap-1.5">
          <span className="size-2 rounded-full" style={{ background: c }} />
          {node.section.title}
        </span>
        <span className="ml-auto tabular">
          Node {node.position} of {node.total}
        </span>
      </div>

      <header className="mt-4 flex items-start gap-4">
        <div className="min-w-0 flex-1">
          <div className="flex items-center gap-2">
            <span className="rounded-md px-1.5 py-0.5 text-[11px] font-semibold tracking-wide text-white uppercase" style={{ background: c }}>
              {node.kind === "boss" ? "Boss" : node.kind === "lesson" ? "Lesson" : "Lesson + lab"}
            </span>
            {done && (
              <span className="text-sm text-[#f2b33d]">
                {"★".repeat(node.progress.stars)}
                <span className="text-line-strong">{"★".repeat(3 - node.progress.stars)}</span>
              </span>
            )}
          </div>
          <h1 className="mt-2 text-[30px] leading-tight font-semibold tracking-tight">{node.title}</h1>
        </div>
        <Mascot mood={done ? "happy" : node.kind === "boss" ? "cheer" : "read"} size={86} animated hat={hat} className="shrink-0 text-ink-2" />
      </header>

      {node.weak && (
        <div className="mt-5 rounded-xl border border-warn/40 bg-warn-soft p-4 text-sm">
          <div className="flex items-center gap-2 font-semibold text-warn">
            <Icon name="target" size={15} /> Needs practice
          </div>
          <p className="mt-1 text-ink-2">
            “{node.weak.challenge.title}” went rough ({node.weak.reason}), and it uses what this node teaches.{" "}
            {done ? "Pass the quiz again" : "Pass the quiz"}
            {node.lab ? " and solve the lab without hints" : ""} to clear it, or solve {node.weak.cleanSolvesToClear} dailies on the topic without hints (
            {node.weak.cleanSolves}/{node.weak.cleanSolvesToClear} so far).
          </p>
        </div>
      )}

      {/* progress strip */}
      <div className="mt-5 grid grid-cols-3 gap-2 text-xs">
        <Step label="Read the lesson" done={node.progress.quizAttempts > 0 || done} icon="book" />
        <Step label="Pass the quiz" done={node.progress.quizPassed} icon="target" />
        <Step label={node.lab ? "Solve the lab" : "No lab here"} done={!node.lab || node.progress.labSolved} icon="code" muted={!node.lab} />
      </div>

      <article className="mt-8">
        <LessonContent source={node.lesson} />
      </article>

      <section className="mt-10">
        <h2 className="mb-1 flex items-center gap-2 text-lg font-semibold">
          <Icon name="target" size={18} /> Check your understanding
        </h2>
        <p className="mb-4 text-sm text-ink-2">Get them all right to pass. Wrong answers come with an explanation, and you can try again.</p>
        <Quiz
          path={pathId}
          node={nodeId}
          questions={node.quiz}
          passed={node.progress.quizPassed}
          onResult={(r) => {
            if (r.completed) setCelebrate(r);
            load();
          }}
        />
      </section>

      {node.lab && (
        <section className="mt-10">
          <h2 className="mb-3 flex items-center gap-2 text-lg font-semibold">
            <Icon name="code" size={18} /> {node.kind === "boss" ? "Boss lab" : "Lab"}
          </h2>
          <div className="card flex flex-wrap items-center gap-4 p-5" style={{ borderColor: `${c}66` }}>
            <span className="grid size-12 place-items-center rounded-xl text-white" style={{ background: c }}>
              <Icon name={node.kind === "boss" ? "trophy" : node.lab.profile === "linux" ? "terminal" : node.language === "c" ? "cpu" : "code"} size={22} />
            </span>
            <div className="min-w-0 flex-1">
              <div className="font-semibold">{node.lab.title}</div>
              <div className="text-xs text-muted">
                {node.language === "c" ? `C · ${node.lab.profile === "linux" ? "real Linux APIs" : "simulated MCU + RTOS, with a live timeline"}` : langName} · ~
                {node.lab.estMinutes} min
              </div>
            </div>
            {node.lab.solved ? (
              <span className="inline-flex items-center gap-1.5 rounded-lg bg-good-soft px-3 py-2 text-sm font-medium text-good">
                <Icon name="check" size={15} /> Solved
              </span>
            ) : null}
            <button className="btn btn-primary" onClick={() => navigate(`/solve/${node.lab!.id}`)}>
              <Icon name="play" size={13} /> {node.lab.solved ? "Open again" : "Start the lab"}
            </button>
          </div>
        </section>
      )}

      {done && (
        <section className="mt-10 rounded-2xl border border-good/30 bg-good-soft p-5">
          <div className="flex items-center gap-4">
            <Mascot mood="cheer" size={72} className="shrink-0" />
            <div className="flex-1">
              <div className="font-semibold text-ink">Node complete · +{node.progress.xp} XP</div>
              <div className="text-sm text-ink-2">
                {node.nextNodes.length ? "Unlocked next:" : "That's the end of this branch. Legendary."}
              </div>
              <div className="mt-2 flex flex-wrap gap-2">
                {node.nextNodes.map((n) => (
                  <button key={n.id} className="btn btn-primary h-8 text-xs" onClick={() => navigate(`/paths/${pathId}/${n.id}`)} disabled={n.status === "locked"}>
                    {n.title} <Icon name="chevron" size={12} />
                  </button>
                ))}
                <button className="btn h-8 text-xs" onClick={() => navigate(`/paths/${pathId}`)}>
                  Back to the tree
                </button>
              </div>
            </div>
          </div>
        </section>
      )}

      {celebrate && (
        <div className="fixed inset-0 z-50 grid place-items-center bg-black/50 p-4 backdrop-blur-sm" onClick={() => setCelebrate(null)}>
          <div className="card pop-in w-full max-w-md p-6 text-center" onClick={(e) => e.stopPropagation()}>
            <Mascot mood="cheer" size={120} hat={hat} className="mx-auto text-ink-2" />
            <h2 className="mt-2 text-xl font-semibold">Node complete!</h2>
            <p className="mt-1 text-sm text-ink-2">
              +{celebrate.xpGained} XP{celebrate.unlocked.length ? ` · ${celebrate.unlocked.length} new node${celebrate.unlocked.length > 1 ? "s" : ""} unlocked` : ""}
            </p>
            <div className="mt-5 flex justify-center gap-2">
              <button className="btn" onClick={() => setCelebrate(null)}>
                Stay here
              </button>
              <button className="btn btn-primary" onClick={() => navigate(`/paths/${pathId}`)}>
                See the tree <Icon name="chevron" size={14} />
              </button>
            </div>
          </div>
        </div>
      )}
    </div>
  );
}

function Step({ label, done, icon, muted = false }: { label: string; done: boolean; icon: "book" | "target" | "code"; muted?: boolean }) {
  return (
    <div className={`flex items-center gap-2 rounded-lg border px-3 py-2 ${done && !muted ? "border-good/30 bg-good-soft text-good" : "border-line bg-surface text-ink-2"} ${muted ? "opacity-50" : ""}`}>
      <Icon name={done && !muted ? "check" : icon} size={14} />
      {label}
    </div>
  );
}
