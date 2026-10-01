import Editor, { type OnMount } from "@monaco-editor/react";
import { useCallback, useEffect, useMemo, useRef, useState } from "react";
import { Link, useNavigate, useParams } from "react-router-dom";
import { api, type ChallengeView, type Clock, type FinishResult, type RunResult } from "../api";
import { editorTheme, monaco } from "../monaco"; // configures the bundled Monaco before <Editor> mounts
import { LangBadge, LANGUAGES, type Lang } from "../lang";
import { Mascot, TessLoading, TessRow } from "../components/Mascot";
import { categorize, conceptCardCopy, conceptNudge, finishLine, finishTitle, runReaction, type Line } from "../tess/lines";
import {
  bumpNudge,
  conceptMuted,
  conceptOpened as readConceptOpened,
  dismissConceptTip,
  markConceptOpened,
  nudgeCount,
  reducedMotion,
  useConceptTips,
  useTessVoice,
} from "../tess/prefs";
import { useTheme } from "../theme";
import { CodeBlock, fmtClock, fmtMinutes, Icon, LevelBadge, Markdown, Tag } from "../ui";

type LeftTab = "task" | "concept" | "tests" | "solution";
type EditorInstance = Parameters<OnMount>[0];

const draftKey = (attemptId: string) => `draft:${attemptId}`;
const readDraft = (attemptId: string) => {
  try {
    return localStorage.getItem(draftKey(attemptId));
  } catch {
    return null;
  }
};

export default function ChallengePage({ onFinished }: { onFinished: () => void }) {
  const { lang: langParam = "typescript", slug = "" } = useParams();
  const lang = langParam as Lang;
  const id = `${lang}/${slug}`;
  const info = LANGUAGES[lang];
  const navigate = useNavigate();
  const { theme } = useTheme();

  const [view, setView] = useState<ChallengeView | null>(null);
  const [code, setCode] = useState("");
  const [tab, setTab] = useState<LeftTab>("task");
  const [result, setResult] = useState<RunResult | null>(null);
  const [running, setRunning] = useState(false);
  const [finish, setFinish] = useState<FinishResult | null>(null);
  const [confirmGiveUp, setConfirmGiveUp] = useState(false);
  const [error, setError] = useState<string | null>(null);
  const editorRef = useRef<EditorInstance | null>(null);
  const codeRef = useRef("");
  const currentCode = () => editorRef.current?.getValue() ?? codeRef.current;
  const voice = useTessVoice();
  const tipsOn = useConceptTips();

  // Tess's per-attempt memory.
  const [reaction, setReaction] = useState<Line | null>(null);
  const [nudge, setNudge] = useState<string | null>(null);
  const [hintAsk, setHintAsk] = useState(false);
  const [conceptSeen, setConceptSeen] = useState(false);
  const runs = useRef({ prevPassing: null as number | null, runNo: 0, fails: 0 });
  // The clock starts on the first real edit, not on opening the page.
  const [clock, setClock] = useState<Clock>(NO_CLOCK);
  const codingStartedAt = clock.codingStartedAt;
  const setCodingStartedAt = (at: string) => setClock((c) => ({ ...c, codingStartedAt: at }));
  const beginning = useRef(false);

  const active = view?.attempt ?? null;
  const reviewing = !!view && !active;
  const paused = !!active && !!clock.pausedAt;
  // Only the clock's lifetime matters to the effects below, not every view update.
  const clockId = active && view ? view.id : null;

  // Pause/resume: update the display at once, then let the server's answer settle it. Requests are
  // chained so a quick pause→resume can't arrive out of order.
  const clockRef = useRef(clock);
  clockRef.current = clock;
  const clockQueue = useRef(Promise.resolve());
  const setPaused = useCallback(
    (pause: boolean, reason: "manual" | "away" = "manual") => {
      const c = clockRef.current;
      if (!clockId || !c.codingStartedAt) return;
      if (pause === !!c.pausedAt && !(pause && reason === "manual" && c.pauseReason === "away")) return;
      const now = Date.now();
      setClock(
        pause
          ? { ...c, pausedAt: c.pausedAt ?? new Date(now).toISOString(), pauseReason: reason }
          : { ...c, pausedMs: c.pausedMs + Math.max(0, now - Date.parse(c.pausedAt!)), pausedAt: null, pauseReason: null },
      );
      clockQueue.current = clockQueue.current
        .then(() => (pause ? api.pause(clockId, reason) : api.resume(clockId)))
        .then(setClock)
        .catch(() => {});
    },
    [clockId],
  );

  // Stop the clock when the learner isn't here: tab hidden, page closed, or navigated away in the app.
  // "Away" pauses resume by themselves on return; a manual pause waits for the Resume button.
  useEffect(() => {
    if (!clockId) return;
    const challengeId = clockId;
    const onVisibility = () => {
      if (document.hidden) setPaused(true, "away");
      else if (clockRef.current.pauseReason === "away") setPaused(false);
    };
    const onPageHide = () => {
      if (clockRef.current.codingStartedAt && !clockRef.current.pausedAt) api.pauseOnUnload(challengeId);
    };
    document.addEventListener("visibilitychange", onVisibility);
    window.addEventListener("pagehide", onPageHide);
    return () => {
      document.removeEventListener("visibilitychange", onVisibility);
      window.removeEventListener("pagehide", onPageHide);
      if (clockRef.current.codingStartedAt && !clockRef.current.pausedAt) api.pause(challengeId, "away").catch(() => {});
    };
  }, [clockId, setPaused]);

  // Coming back to a challenge that was paused because we left: carry on.
  useEffect(() => {
    if (clock.pauseReason === "away" && !document.hidden) setPaused(false);
  }, [clock.pauseReason, setPaused]);

  // Nudges only when tips are on, the concept hasn't been read, and we're under the per-attempt cap.
  const canNudge = () =>
    !!active && voice !== "off" && tipsOn && !readConceptOpened(active.id) && !conceptMuted(active.id) && nudgeCount(active.id) < 2;

  // Reading the Concept tab for 3s counts as "opened".
  useEffect(() => {
    if (tab !== "concept" || !active) return;
    const t = setTimeout(() => {
      markConceptOpened(active.id);
      setConceptSeen(true);
      setNudge(null);
    }, 3000);
    return () => clearTimeout(t);
  }, [tab, active]);

  const openConcept = () => setTab("concept"); // leaves the editor cursor alone
  const dismissNudge = () => {
    if (active) dismissConceptTip(active.id);
    setNudge(null);
  };

  const load = useCallback(
    async (forceStart = false) => {
      setResult(null);
      setFinish(null);
      try {
        let v = await api.challenge(id);
        // Start automatically unless this is a finished challenge being reviewed.
        if (!v.attempt && (forceStart || v.solution === null)) v = await api.start(id);
        setView(v);
        setCode(v.attempt ? (readDraft(v.attempt.id) ?? v.starter) : (v.bestCode ?? v.starter));
        setTab("task");
        setReaction(null);
        setNudge(null);
        setConceptSeen(!!v.attempt && readConceptOpened(v.attempt.id));
        setClock(
          v.attempt
            ? {
                codingStartedAt: v.attempt.codingStartedAt ?? null,
                pausedMs: v.attempt.pausedMs ?? 0,
                pausedAt: v.attempt.pausedAt ?? null,
                pauseReason: v.attempt.pauseReason ?? null,
              }
            : NO_CLOCK,
        );
        beginning.current = false;
        runs.current = { prevPassing: null, runNo: 0, fails: 0 };
      } catch (e) {
        setError((e as Error).message);
      }
    },
    [id],
  );

  useEffect(() => {
    load();
  }, [load]);

  // Persist the draft so a reload or a coffee break doesn't lose work.
  useEffect(() => {
    if (!active) return;
    const t = setTimeout(() => {
      try {
        localStorage.setItem(draftKey(active.id), code);
      } catch {}
    }, 300);
    return () => clearTimeout(t);
  }, [code, active]);

  const run = useCallback(async () => {
    if (!view || running) return;
    setRunning(true);
    try {
      // Read straight from the editor: a keypress right after an edit can beat React's re-render.
      const r = await api.run(view.id, currentCode());
      const h = runs.current;
      const cat = categorize(r, h.prevPassing);
      setReaction(runReaction(r, cat, h.prevPassing, h.runNo, lang));
      showMarkers(r);
      h.prevPassing = r.tests.filter((t) => t.pass).length;
      h.runNo++;
      h.fails = r.passed ? 0 : h.fails + 1;
      // Concept nudge (b): 2nd failing run in a row, or the 1st when the topic is new to you. Chatty mode only.
      const threshold = view.newTopics.length ? 1 : 2;
      const testFailure = cat === "failing" || cat === "progress";
      if (!testFailure) setNudge(null);
      if (testFailure && voice === "chatty" && h.fails >= threshold && !nudge && canNudge()) {
        bumpNudge(active!.id);
        setNudge(conceptNudge(active!.id));
      }
      if (r.passed) setNudge(null);
      setResult(r);
    } finally {
      setRunning(false);
    }
  }, [view, code, running, voice, nudge, tipsOn, active]);

  const submit = useCallback(async () => {
    if (!view || !active) return;
    setRunning(true);
    try {
      const f = await api.submit(view.id, currentCode());
      if (f.result) setResult(f.result);
      setFinish(f);
      localStorage.removeItem(draftKey(active.id));
      onFinished();
    } catch (e) {
      const r = (e as { data?: { result?: RunResult } }).data?.result;
      if (r) setResult(r);
    } finally {
      setRunning(false);
    }
  }, [view, active, code, onFinished]);

  const giveUp = async () => {
    if (!view) return;
    setConfirmGiveUp(false);
    const f = await api.giveUp(view.id, currentCode());
    setFinish(f);
    onFinished();
  };

  const hint = async (skipAsk = false) => {
    if (!view) return;
    // Concept nudge (c): offer the free lesson once, right before the first hint.
    if (!skipAsk && !nudge && view.hints.length === 0 && canNudge()) {
      bumpNudge(active!.id);
      setNudge(null);
      setHintAsk(true);
      setTab("task");
      return;
    }
    setHintAsk(false);
    setNudge(null);
    const { hints } = await api.hint(view.id);
    setView({ ...view, hints, attempt: view.attempt && { ...view.attempt, hintsUsed: hints.length } });
    setTab("task");
  };

  // Keyboard: Ctrl/⌘+Enter runs, Ctrl/⌘+Shift+Enter submits.
  const canSubmit = !!result?.passed && !!active;
  const keys = useRef({ run, submit, canSubmit });
  keys.current = { run, submit, canSubmit };
  useEffect(() => {
    const onKey = (e: KeyboardEvent) => {
      if ((e.ctrlKey || e.metaKey) && e.key === "Enter") {
        e.preventDefault();
        if (e.shiftKey && keys.current.canSubmit) keys.current.submit();
        else keys.current.run();
      }
    };
    window.addEventListener("keydown", onKey);
    return () => window.removeEventListener("keydown", onKey);
  }, []);

  const onMount: OnMount = (editor, monaco) => {
    editorRef.current = editor;
    // Inside Monaco the window listener doesn't fire, so bind the same shortcuts here.
    editor.addCommand(monaco.KeyMod.CtrlCmd | monaco.KeyCode.Enter, () => keys.current.run());
    editor.addCommand(monaco.KeyMod.CtrlCmd | monaco.KeyMod.Shift | monaco.KeyCode.Enter, () => {
      if (keys.current.canSubmit) keys.current.submit();
    });
  };

  // Monaco checks TypeScript live; for C and Python, paint the compiler's findings onto the code.
  const showMarkers = (r: RunResult | null) => {
    const model = editorRef.current?.getModel();
    if (!model) return;
    if (lang === "typescript" || !r) {
      monaco.editor.setModelMarkers(model, "daily", []);
      return;
    }
    monaco.editor.setModelMarkers(
      model,
      "daily",
      r.diagnostics
        .filter((d) => d.file === "your-code")
        .map((d) => ({
          startLineNumber: d.line,
          startColumn: d.column,
          endLineNumber: d.line,
          endColumn: Math.max(d.column + 1, model.getLineMaxColumn(Math.min(d.line, model.getLineCount()))),
          message: `${d.message} (${d.tool}${d.code && d.code !== d.tool ? ` ${d.code}` : ""})`,
          severity: d.severity === "error" ? monaco.MarkerSeverity.Error : monaco.MarkerSeverity.Warning,
        })),
    );
  };

  const jumpTo = (line: number, column: number) => {
    const ed = editorRef.current;
    if (!ed) return;
    ed.revealLineInCenter(line);
    ed.setPosition({ lineNumber: line, column });
    ed.focus();
  };

  if (error)
    return (
      <div className="p-10 text-ink-2">
        {error} — <Link to={`/${lang}`} className="text-accent">back to dashboard</Link>
      </div>
    );
  if (!view) return <TessLoading />;

  return (
    <div className="flex h-full flex-col">
      {/* Top bar */}
      <header className="flex h-14 shrink-0 items-center gap-3 border-b border-line bg-surface px-4">
        <button className="btn btn-ghost px-2" onClick={() => navigate(-1)} title="Back">
          <Icon name="back" size={16} />
        </button>
        <div className="flex min-w-0 items-center gap-2.5">
          <LangBadge lang={lang} size={22} />
          <h1 className="truncate text-[15px] font-semibold">{view.title}</h1>
          <LevelBadge level={view.level} />
          {view.isDaily && (
            <span className="hidden items-center gap-1 rounded-md bg-accent-soft px-1.5 py-0.5 text-[11px] font-semibold text-accent-strong sm:inline-flex">
              <Icon name="zap" size={11} /> Daily
            </span>
          )}
        </div>
        <div className="ml-auto flex items-center gap-2">
          {active && <Timer clock={clock} targetMin={Math.max(10, view.estMinutes)} onToggle={() => setPaused(!paused)} />}
          {active && (
            <>
              <div className="relative">
                <button
                  className="btn"
                  onClick={() => hint()}
                  disabled={view.hints.length >= view.hintCount}
                  title={voice === "off" ? "Each hint trims your score a little" : "Tess's hints. Each one trims this attempt's score a little."}
                >
                  <Icon name="bulb" size={15} />
                  <span className="hidden md:inline">Hint</span>
                  <span className="tabular text-muted">
                    {view.hints.length}/{view.hintCount}
                  </span>
                </button>
              </div>
              <button className="btn btn-ghost hidden md:inline-flex" onClick={() => setConfirmGiveUp(true)}>
                <Icon name="flag" size={15} /> Give up
              </button>
              <button className="btn" onClick={run} disabled={running}>
                <Icon name="play" size={13} /> {running ? "Running…" : "Run"} <span className="kbd hidden lg:inline">Ctrl ↵</span>
              </button>
              <button
                className={`btn ${canSubmit ? "btn-good" : ""}`}
                onClick={submit}
                disabled={!canSubmit || running}
                title={canSubmit ? "Submit (Ctrl+Shift+Enter)" : "Run your code until every check passes"}
              >
                <Icon name="check" size={15} /> Submit
              </button>
            </>
          )}
          {reviewing && (
            <button className="btn btn-primary" onClick={() => load(true)}>
              <Icon name="refresh" size={15} /> Practice again
            </button>
          )}
        </div>
      </header>

      <div className="flex min-h-0 flex-1 flex-col lg:flex-row">
        {/* Left: task / concept / tests / solution */}
        <section className="flex min-h-0 flex-col border-line max-lg:h-[40%] max-lg:border-b lg:w-[42%] lg:max-w-[640px] lg:min-w-[360px] lg:border-r">
          <div className="flex shrink-0 gap-1 border-b border-line bg-surface px-3 pt-2">
            {(
              [
                ["task", "Task", "code"],
                ["concept", "Concept", "book"],
                ["tests", "Tests", "check"],
                ...(view.solution ? [["solution", "Solution", "eye"]] : []),
              ] as [LeftTab, string, "code"][]
            ).map(([k, label, icon]) => (
              <button
                key={k}
                onClick={() => setTab(k)}
                className={`-mb-px inline-flex items-center gap-1.5 border-b-2 px-3 pb-2.5 pt-1.5 text-[13px] font-medium ${
                  tab === k ? "border-accent text-ink" : "border-transparent text-muted hover:text-ink-2"
                }`}
              >
                <Icon name={icon} size={14} /> {label}
                {k === "concept" && active && !conceptSeen && voice !== "off" && (
                  <span className="size-1.5 rounded-full bg-accent" aria-label="not read yet" />
                )}
              </button>
            ))}
          </div>
          <div className="min-h-0 flex-1 overflow-y-auto bg-surface p-6">
            {tab === "task" && (
              <>
                <div className="mb-5 flex flex-wrap items-center gap-1.5">
                  {view.mode === "types" && <Tag>type-level: the compiler is the judge</Tag>}
                  {view.topics.map((t) => (
                    <Tag key={t}>{t}</Tag>
                  ))}
                  <span className="ml-1 inline-flex items-center gap-1 text-xs text-muted">
                    <Icon name="clock" size={12} /> ~{view.estMinutes} min
                  </span>
                </div>
                {view.newTopics.includes(view.topics[0]) && !conceptSeen && <ConceptCard view={view} onOpen={openConcept} tess={voice !== "off"} top />}
                <Markdown source={view.prompt} />
                {view.hints.length > 0 && (
                  <div className="mt-6 flex flex-col gap-2">
                    {view.hints.map((h, i) => (
                      <div key={i} className="pop-in flex gap-2.5">
                        {voice !== "off" && <Mascot mood="think" size={34} className="mt-1 shrink-0" />}
                        <div className="min-w-0 flex-1 rounded-xl rounded-tl-sm border border-line border-l-accent/50 bg-surface-2 p-3.5 [border-left-width:3px]">
                          <div className="mb-1 text-xs font-semibold text-accent-strong">
                            {voice === "off" ? `Hint ${i + 1}` : `Tess · hint ${i + 1} of ${view.hintCount}`}
                          </div>
                          {voice !== "off" && i === view.hintCount - 1 && (
                            <p className="mb-1 text-[13px] text-ink-2 italic">Last one. It's the biggest nudge I've got:</p>
                          )}
                          <Markdown source={h} className="!text-[13px]" />
                        </div>
                      </div>
                    ))}
                  </div>
                )}
                {hintAsk && (
                  <HintAsk
                    onConcept={() => {
                      setHintAsk(false);
                      openConcept();
                    }}
                    onHint={() => hint(true)}
                  />
                )}
                {(!view.newTopics.includes(view.topics[0]) || conceptSeen) && <ConceptCard view={view} onOpen={openConcept} tess={voice !== "off"} />}
              </>
            )}
            {tab === "concept" && <Markdown source={view.learn} />}
            {tab === "tests" && (
              <>
                <p className="mb-3 text-sm text-ink-2">
                  {lang === "typescript" && "These run against your code. They're type-checked too, so a wrong parameter or return type shows up as a type error."}
                  {lang === "python" &&
                    "These pytest-style tests run against your code. Your type hints are checked by mypy --strict: its findings show as warnings, which are advice and won't block a solve."}
                  {lang === "c" &&
                    "Compiled together with your code using gcc -Wall -Wextra and AddressSanitizer. Out-of-bounds access, use-after-free and memory leaks fail a test."}
                </p>
                <CodeBlock code={view.tests} />
              </>
            )}
            {tab === "solution" && view.solution && (
              <>
                <p className="mb-3 text-sm text-ink-2">One good way to do it. Yours can differ and still be great — compare the approach, not the characters.</p>
                <CodeBlock code={view.solution} />
              </>
            )}
          </div>
        </section>

        {/* Right: editor + results */}
        <SplitPane
          top={
            <div className="relative h-full">
              {reviewing && (
                <div className="absolute top-2 right-4 z-10 rounded-md bg-surface-2 px-2 py-1 text-xs text-muted">Read-only · your submitted code</div>
              )}
              {paused && (
                <div className="absolute inset-0 z-20 grid place-items-center bg-bg/70 backdrop-blur-sm">
                  <div className="flex flex-col items-center gap-3 text-center">
                    <Mascot mood="idle" size={88} className="text-ink-2" />
                    <p className="font-semibold">Paused</p>
                    <p className="max-w-xs text-sm text-muted">The clock is stopped. Your code is safe.</p>
                    <button className="btn btn-primary" onClick={() => setPaused(false)} autoFocus>
                      <Icon name="play" size={14} /> Resume
                    </button>
                  </div>
                </div>
              )}
              <Editor
                path={`file:///your-code.${info.ext}`}
                language={info.monaco}
                value={code}
                onChange={(v) => {
                  setCode(v ?? "");
                  codeRef.current = v ?? "";
                  if (active && !codingStartedAt && !beginning.current) {
                    beginning.current = true;
                    setCodingStartedAt(new Date().toISOString()); // start the display instantly
                    api
                      .begin(view.id)
                      .then((r) => setCodingStartedAt(r.codingStartedAt))
                      .catch(() => (beginning.current = false));
                  }
                }}
                onMount={onMount}
                theme={editorTheme(lang, theme === "dark" ? "dark" : "light")}
                options={{
                  readOnly: reviewing || paused,
                  fontFamily: "'JetBrains Mono Variable', monospace",
                  fontSize: 14,
                  lineHeight: 22,
                  minimap: { enabled: false },
                  scrollBeyondLastLine: false,
                  padding: { top: 14 },
                  tabSize: lang === "typescript" ? 2 : 4,
                  insertSpaces: true,
                  renderLineHighlight: "line",
                  smoothScrolling: true,
                  cursorBlinking: "smooth",
                  fontLigatures: true,
                  automaticLayout: true,
                }}
              />
            </div>
          }
          bottom={
            <ResultsPanel
              result={result}
              running={running}
              mode={view.mode}
              lang={lang}
              onJump={jumpTo}
              canSubmit={canSubmit}
              onSubmit={submit}
              reviewing={reviewing}
              reaction={voice === "off" || (voice === "quiet" && !result?.passed) ? null : reaction}
              nudge={nudge ? { text: nudge, onOpen: openConcept, onDismiss: dismissNudge } : null}
            />
          }
        />
      </div>

      {confirmGiveUp && (
        <Modal onClose={() => setConfirmGiveUp(false)}>
          <h2 className="text-lg font-semibold">Show the solution?</h2>
          <p className="mt-2 text-sm text-ink-2">
            This ends the attempt and counts as not solved, so your rating dips a little. That's fine — reading a good solution is learning
            too. This challenge comes back around in a week.
          </p>
          {active && !readConceptOpened(active.id) && voice !== "off" && (
            <button
              className="mt-3 flex items-center gap-2 text-left text-sm text-accent hover:underline"
              onClick={() => {
                setConfirmGiveUp(false);
                openConcept();
              }}
            >
              <Mascot mood="read" size={32} /> Want to try the Concept lesson first? It's free.
            </button>
          )}
          <div className="mt-5 flex justify-end gap-2">
            <button className="btn" onClick={() => setConfirmGiveUp(false)}>
              Keep trying
            </button>
            <button className="btn btn-primary" onClick={giveUp}>
              Show solution
            </button>
          </div>
        </Modal>
      )}

      {finish && (
        <FinishModal
          finish={finish}
          conceptOpened={!!active && readConceptOpened(active.id)}
          voice={voice}
          onClose={() => {
            setFinish(null);
            load();
          }}
          onDashboard={() => navigate(`/${lang}`)}
        />
      )}
    </div>
  );
}

/* ---------- Concept card (the free lesson) ---------- */
function ConceptCard({ view, onOpen, tess, top = false }: { view: ChallengeView; onOpen: () => void; tess: boolean; top?: boolean }) {
  const copy = conceptCardCopy(view);
  return (
    <button
      onClick={onOpen}
      className={`flex w-full items-center gap-3 rounded-xl border p-3 text-left hover:bg-surface-2 ${top ? "mb-5 border-accent/40 bg-accent-soft" : "mt-6 border-line"}`}
    >
      {tess ? (
        <Mascot mood="read" size={44} className="shrink-0" />
      ) : (
        <span className="grid size-8 place-items-center rounded-lg bg-accent-soft text-accent">
          <Icon name="book" size={16} />
        </span>
      )}
      <span className="flex-1">
        <span className="block text-sm font-medium text-ink">{copy.title}</span>
        <span className="block text-xs text-muted">{copy.sub}</span>
      </span>
      <Icon name="chevron" size={16} className="text-muted" />
    </button>
  );
}

/** One-time "free option first?" before the first hint, inline in the task panel. Enter/Esc = just show the hint. */
function HintAsk({ onConcept, onHint }: { onConcept: () => void; onHint: () => void }) {
  const hintBtn = useRef<HTMLButtonElement>(null);
  const ref = useRef<HTMLDivElement>(null);
  useEffect(() => {
    ref.current?.scrollIntoView({ block: "nearest", behavior: "smooth" });
    hintBtn.current?.focus({ preventScroll: true });
    const k = (e: KeyboardEvent) => {
      if (e.key === "Escape") {
        e.preventDefault();
        onHint();
      }
    };
    window.addEventListener("keydown", k);
    return () => window.removeEventListener("keydown", k);
  }, [onHint]);
  return (
    <div ref={ref} className="mt-6">
      <TessRow mood="read" tone="accent">
        <p>Want the free option first? The Concept lesson covers this. Hints cost a little score.</p>
        <div className="mt-2 flex gap-1.5">
          <button className="btn h-8 text-xs" onClick={onConcept}>
            <Icon name="book" size={13} /> Read concept
          </button>
          <button ref={hintBtn} className="btn btn-primary h-8 text-xs" onClick={onHint}>
            Show hint
          </button>
        </div>
      </TessRow>
    </div>
  );
}

/* ---------- Timer ---------- */
const NO_CLOCK: Clock = { codingStartedAt: null, pausedMs: 0, pausedAt: null, pauseReason: null };

function Timer({ clock, targetMin, onToggle }: { clock: Clock; targetMin: number; onToggle: () => void }) {
  const startedAt = clock.codingStartedAt;
  const [now, setNow] = useState(Date.now());
  useEffect(() => {
    if (!startedAt || clock.pausedAt) return;
    setNow(Date.now());
    const t = setInterval(() => setNow(Date.now()), 1000);
    return () => clearInterval(t);
  }, [startedAt, clock.pausedAt]);
  if (!startedAt)
    return (
      <div
        className="mr-1 hidden items-center gap-2 text-sm text-muted sm:flex"
        title="Take your time reading the task and the Concept tab. The clock starts on your first keystroke."
      >
        <Icon name="clock" size={16} />
        <span>Starts when you type</span>
      </div>
    );
  const end = clock.pausedAt ? Date.parse(clock.pausedAt) : Math.max(now, Date.now());
  const elapsed = Math.max(0, end - Date.parse(startedAt) - clock.pausedMs);
  const frac = Math.min(1, elapsed / (targetMin * 60_000));
  const over = elapsed > targetMin * 60_000;
  const r = 8;
  const c = 2 * Math.PI * r;
  return (
    <div className="mr-1 hidden items-center gap-2 text-sm tabular sm:flex" title={`Target ≈ ${targetMin} min. No hard limit — take the time you need.`}>
      <svg width="20" height="20" viewBox="0 0 20 20" className="-rotate-90">
        <circle cx="10" cy="10" r={r} fill="none" stroke="var(--surface-3)" strokeWidth="2.5" />
        <circle
          cx="10"
          cy="10"
          r={r}
          fill="none"
          stroke={over ? "var(--warn)" : "var(--accent)"}
          strokeWidth="2.5"
          strokeDasharray={c}
          strokeDashoffset={c * (1 - frac)}
          strokeLinecap="round"
        />
      </svg>
      <span className={clock.pausedAt ? "text-muted" : over ? "text-warn" : "text-ink-2"}>{fmtClock(elapsed)}</span>
      <button
        className="btn btn-ghost h-7 w-7 justify-center p-0"
        onClick={onToggle}
        title={clock.pausedAt ? "Resume the clock" : "Pause the clock (it also pauses when you leave this tab)"}
        aria-label={clock.pausedAt ? "Resume" : "Pause"}
      >
        <Icon name={clock.pausedAt ? "play" : "pause"} size={14} />
      </button>
    </div>
  );
}

/* ---------- Resizable editor / results split ---------- */
function SplitPane({ top, bottom }: { top: React.ReactNode; bottom: React.ReactNode }) {
  const [ratio, setRatio] = useState(0.6);
  const ref = useRef<HTMLDivElement>(null);
  const drag = (e: React.PointerEvent) => {
    const el = ref.current;
    if (!el) return;
    (e.target as HTMLElement).setPointerCapture(e.pointerId);
    const rect = el.getBoundingClientRect();
    const move = (ev: PointerEvent) => setRatio(Math.min(0.85, Math.max(0.25, (ev.clientY - rect.top) / rect.height)));
    const up = () => {
      window.removeEventListener("pointermove", move);
      window.removeEventListener("pointerup", up);
    };
    window.addEventListener("pointermove", move);
    window.addEventListener("pointerup", up);
  };
  return (
    <div ref={ref} className="flex min-h-0 min-w-0 flex-1 flex-col">
      <div style={{ height: `${ratio * 100}%` }} className="min-h-0">
        {top}
      </div>
      <div onPointerDown={drag} className="group relative h-1.5 shrink-0 cursor-row-resize border-y border-line bg-surface-2">
        <div className="absolute top-1/2 left-1/2 h-0.5 w-8 -translate-x-1/2 -translate-y-1/2 rounded bg-line-strong group-hover:bg-accent" />
      </div>
      <div className="min-h-0 flex-1">{bottom}</div>
    </div>
  );
}

/* ---------- Results ---------- */
function ResultsPanel({
  result,
  running,
  mode,
  onJump,
  canSubmit,
  onSubmit,
  reviewing,
  reaction,
  nudge,
  lang,
}: {
  lang: Lang;
  reaction: Line | null;
  nudge: { text: string; onOpen: () => void; onDismiss: () => void } | null;
  result: RunResult | null;
  running: boolean;
  mode: "runtime" | "types";
  onJump: (line: number, col: number) => void;
  canSubmit: boolean;
  onSubmit: () => void;
  reviewing: boolean;
}) {
  const [tab, setTab] = useState<"results" | "console">("results");
  const passing = result?.tests.filter((t) => t.pass).length ?? 0;
  const total = result?.tests.length ?? 0;

  const summary = useMemo(() => {
    if (!result) return null;
    const warns = result.diagnostics.filter((d) => d.severity === "warning").length;
    if (result.passed)
      return { tone: "good", text: (mode === "types" ? "All type checks pass" : `All ${total} tests pass`) + (warns ? ` · ${warns} warning${warns > 1 ? "s" : ""}` : "") };
    const parts: string[] = [];
    const errs = result.diagnostics.filter((d) => d.severity === "error").length;
    const noun = lang === "typescript" ? "type error" : lang === "python" ? "syntax error" : "compile error";
    if (errs) parts.push(`${errs} ${noun}${errs > 1 ? "s" : ""}`);
    if (result.setupErrors.length) parts.push("runtime error");
    if (mode === "runtime" && total) parts.push(`${passing}/${total} tests pass`);
    return { tone: "bad", text: parts.join(" · ") };
  }, [result, mode, passing, total, lang]);

  return (
    <div className="flex h-full flex-col bg-surface">
      <div className="flex h-10 shrink-0 items-center gap-1 border-b border-line px-3">
        {(["results", "console"] as const).map((k) => (
          <button
            key={k}
            onClick={() => setTab(k)}
            className={`rounded-md px-2.5 py-1 text-xs font-medium capitalize ${tab === k ? "bg-surface-2 text-ink" : "text-muted hover:text-ink-2"}`}
          >
            {k === "console" ? (
              <span className="inline-flex items-center gap-1">
                <Icon name="terminal" size={12} /> Console{result?.logs.length ? ` (${result.logs.length})` : ""}
              </span>
            ) : (
              "Results"
            )}
          </button>
        ))}
        {summary && (
          <span className="ml-auto inline-flex items-center gap-1.5 text-xs font-semibold" style={{ color: summary.tone === "good" ? "var(--good)" : "var(--bad)" }}>
            <Icon name={summary.tone === "good" ? "check" : "x"} size={13} strokeWidth={2.5} />
            {summary.text}
            <span className="font-normal text-muted">· {result!.durationMs}ms</span>
          </span>
        )}
      </div>

      <div className={`min-h-0 flex-1 overflow-y-auto p-4 ${running ? "opacity-50" : ""}`}>
        {!result && (
          <div className="flex h-full flex-col items-center justify-center gap-2 text-center text-sm text-muted">
            <Mascot mood="idle" size={72} className="text-ink-2" />
            {reviewing ? (
              "Reviewing a finished challenge. Hit “Practice again” for a fresh, unrated attempt."
            ) : (
              <p>
                Press <span className="kbd">Ctrl ↵</span> or <b className="text-ink-2">Run</b> to {lang === "c" ? "compile" : lang === "python" ? "check" : "type-check"} and test your code.
              </p>
            )}
          </div>
        )}

        {result && tab === "console" && (
          <div className="font-mono text-[12.5px]">
            {result.logs.length === 0 && <p className="font-sans text-sm text-muted">No output. {lang === "python" ? "print()" : lang === "c" ? "printf()" : "console.log()"} in your code shows up here.</p>}
            {result.logs.map((l, i) => (
              <pre
                key={i}
                className="border-b border-line py-1 whitespace-pre-wrap"
                style={{ color: l.level === "error" ? "var(--bad)" : l.level === "warn" ? "var(--warn)" : "var(--text-2)" }}
              >
                {l.text}
              </pre>
            ))}
          </div>
        )}

        {result && tab === "results" && (
          <div className="flex flex-col gap-4">
            {result.passed && canSubmit ? (
              <TessRow
                key={reaction?.text}
                mood="happy"
                tone="good"
                actions={
                  <button className="btn btn-good" onClick={onSubmit}>
                    Submit <span className="kbd hidden lg:inline">Ctrl ⇧ ↵</span>
                  </button>
                }
              >
                <span className="text-sm">{reaction?.text ?? "Everything passes. Tidy up if you like, then submit."}</span>
              </TessRow>
            ) : (
              (reaction || nudge) && (
                // One Tess row: the reaction, plus the Concept suggestion as a second line when there is one.
                <TessRow key={reaction?.text} mood={nudge ? "read" : reaction!.mood} tone={nudge ? "accent" : "neutral"} onDismiss={nudge?.onDismiss}>
                  {reaction && <p>{reaction.text}</p>}
                  {nudge && (
                    <p className="mt-1 flex flex-wrap items-center gap-x-2 gap-y-1">
                      <span>{nudge.text}</span>
                      <button className="btn h-7 px-2 text-xs" onClick={nudge.onOpen}>
                        <Icon name="book" size={12} /> Open Concept
                      </button>
                    </p>
                  )}
                </TessRow>
              )
            )}

            {result.diagnostics.length > 0 && (
              <div>
                <h3 className="mb-2 text-xs font-semibold tracking-wide text-muted uppercase">{LANGUAGES[lang].checkLabel}</h3>
                <div className="flex flex-col gap-2">
                  {result.diagnostics.map((e, i) => (
                    <button
                      key={i}
                      onClick={() => e.file === "your-code" && onJump(e.line, e.column)}
                      className={`rounded-lg border bg-surface-2 p-3 text-left ${e.severity === "warning" ? "border-warn/30" : "border-line"} ${e.file === "your-code" ? "hover:border-line-strong" : "cursor-default"}`}
                    >
                      <div className="flex flex-wrap items-center gap-2 text-xs text-muted">
                        <span
                          className={`rounded px-1.5 py-0.5 font-mono font-semibold ${e.severity === "error" ? "bg-bad-soft text-bad" : "bg-warn-soft text-warn"}`}
                        >
                          {e.severity === "warning" ? "warning" : "error"}
                        </span>
                        <span className="font-mono">{e.tool === "tsc" ? e.code : `${e.tool}${e.code && e.code !== e.tool && e.code !== "error" && e.code !== "warning" ? ` · ${e.code}` : ""}`}</span>
                        {e.file === "your-code" ? "your code" : "tests"} · line {e.line}
                      </div>
                      <div className="mt-1.5 text-[13px] whitespace-pre-wrap text-ink">{e.message}</div>
                      {e.source && <code className="mt-1.5 block truncate font-mono text-xs text-muted">{e.source}</code>}
                    </button>
                  ))}
                </div>
                {lang === "typescript" && result.diagnostics.some((e) => e.file === "tests") && mode === "runtime" && (
                  <p className="mt-2 text-xs text-muted">Errors in the tests usually mean a function's parameter or return type doesn't match what the tests expect.</p>
                )}
                {result.diagnostics.some((e) => e.tool === "mypy") && (
                  <p className="mt-2 text-xs text-muted">mypy checks your type hints. Python runs fine without fixing these, but teams treat them like bugs waiting to happen.</p>
                )}
                {lang === "c" && result.passed && result.diagnostics.length > 0 && (
                  <p className="mt-2 text-xs text-muted">It works, but gcc has concerns. In real C projects, warnings are usually treated as errors (-Werror).</p>
                )}
              </div>
            )}

            {result.setupErrors.map((e, i) => (
              <div key={i} className="rounded-lg border border-bad/30 bg-bad-soft p-3">
                <div className="text-xs font-semibold text-bad">
                  {/Linker error|main\(\)/.test(e.message) ? "Couldn't link" : `Crashed while loading ${e.file === "your-code" ? "your code" : "the tests"}`}
                </div>
                <pre className="mt-1 font-mono text-xs whitespace-pre-wrap text-ink">{e.message}</pre>
              </div>
            ))}

            {mode === "types" && result.diagnostics.length === 0 && result.setupErrors.length === 0 && (
              <div className="flex items-center gap-2 text-sm text-good">
                <Icon name="check" size={15} /> The compiler accepts every type test.
              </div>
            )}

            {result.tests.length > 0 && (
              <div>
                <h3 className="mb-2 text-xs font-semibold tracking-wide text-muted uppercase">
                  Tests · {passing}/{total}
                </h3>
                <div className="overflow-hidden rounded-lg border border-line">
                  {result.tests.map((t, i) => (
                    <div key={i} className="border-b border-line p-3 last:border-b-0">
                      <div className="flex items-center gap-2 text-[13px]">
                        <span
                          className="grid size-4 place-items-center rounded-full text-white"
                          style={{ background: t.pass ? "var(--good)" : "var(--bad)" }}
                        >
                          <Icon name={t.pass ? "check" : "x"} size={10} strokeWidth={3.5} />
                        </span>
                        <span className={t.pass ? "text-ink-2" : "font-medium text-ink"}>{t.name}</span>
                        <span className="ml-auto text-[11px] tabular text-muted">{t.ms < 1 ? "<1" : Math.round(t.ms)}ms</span>
                      </div>
                      {t.error && (
                        <div className="mt-2 ml-6 font-mono text-xs">
                          {t.error.expected !== undefined ? (
                            <div className="grid grid-cols-[72px_1fr] gap-y-1">
                              <span className="text-muted">Expected</span>
                              <pre className="whitespace-pre-wrap text-good">{t.error.expected}</pre>
                              <span className="text-muted">Received</span>
                              <pre className="whitespace-pre-wrap text-bad">{t.error.received}</pre>
                            </div>
                          ) : (
                            <pre className="whitespace-pre-wrap text-bad">{t.error.message}</pre>
                          )}
                          {t.error.stack && <pre className="mt-1 whitespace-pre-wrap text-muted">{t.error.stack}</pre>}
                        </div>
                      )}
                    </div>
                  ))}
                </div>
              </div>
            )}
          </div>
        )}
      </div>
    </div>
  );
}

/* ---------- Modals ---------- */
function Modal({ children, onClose, wide = false }: { children: React.ReactNode; onClose: () => void; wide?: boolean }) {
  useEffect(() => {
    const k = (e: KeyboardEvent) => e.key === "Escape" && onClose();
    window.addEventListener("keydown", k);
    return () => window.removeEventListener("keydown", k);
  }, [onClose]);
  return (
    <div className="fixed inset-0 z-50 grid place-items-center bg-black/50 p-4 backdrop-blur-sm" onClick={onClose}>
      <div className={`card pop-in max-h-[90vh] w-full overflow-y-auto p-6 ${wide ? "max-w-2xl" : "max-w-md"}`} onClick={(e) => e.stopPropagation()}>
        {children}
      </div>
    </div>
  );
}

function Confetti() {
  const pieces = useMemo(
    () =>
      Array.from({ length: 70 }, (_, i) => ({
        left: Math.random() * 100,
        delay: Math.random() * 0.4,
        dur: 1.8 + Math.random() * 1.4,
        dx: `${(Math.random() - 0.5) * 200}px`,
        rot: `${Math.random() * 720 - 360}deg`,
        color: ["#3987e5", "#46c26a", "#f2b33d", "#e87ba4", "#9085e9"][i % 5],
        w: 6 + Math.random() * 5,
      })),
    [],
  );
  return (
    <div className="pointer-events-none fixed inset-0 z-[60] overflow-hidden" aria-hidden>
      {pieces.map((p, i) => (
        <span
          key={i}
          className="absolute top-0 block rounded-[2px]"
          style={
            {
              left: `${p.left}%`,
              width: p.w,
              height: p.w * 0.45,
              background: p.color,
              animation: `confetti-fall ${p.dur}s ${p.delay}s cubic-bezier(.2,.6,.4,1) forwards`,
              "--dx": p.dx,
              "--rot": p.rot,
            } as React.CSSProperties
          }
        />
      ))}
    </div>
  );
}

function FinishModal({
  finish,
  onClose,
  onDashboard,
  conceptOpened,
  voice,
}: {
  finish: FinishResult;
  onClose: () => void;
  onDashboard: () => void;
  conceptOpened: boolean;
  voice: "chatty" | "quiet" | "off";
}) {
  const solved = finish.status === "solved";
  const delta = finish.ratingAfter - finish.ratingBefore;
  const line = finishLine(finish, conceptOpened);
  const confetti = line.confetti === "full" && !reducedMotion();
  return (
    <>
      {confetti && <Confetti />}
      <Modal onClose={onClose} wide>
        <div className="flex items-start gap-4">
          <Mascot
            mood={line.mood}
            size={line.mood === "cheer" ? 120 : 96}
            className={`-my-2 -ml-2 shrink-0 text-ink-2 ${line.confetti === "none" && solved ? "tess-hop" : ""}`}
          />
          <div>
            <h2 className="text-xl font-semibold tracking-tight">{finishTitle(finish)}</h2>
            <p className="mt-1 text-sm text-ink-2">
              {voice === "off" ? (solved ? "Nice work." : "Here's a reference solution to compare against.") : line.text}
            </p>
          </div>
        </div>

        <div className="mt-5 grid grid-cols-3 gap-3">
          <Metric label="Time" value={fmtMinutes(finish.minutes)} />
          <Metric label="Hints used" value={String(finish.hintsUsed)} />
          <Metric
            label={finish.rated ? "Rating" : "Rating (practice)"}
            value={finish.rated ? `${finish.ratingAfter}` : "—"}
            delta={finish.rated ? delta : undefined}
          />
        </div>

        <h3 className="mt-6 mb-2 text-xs font-semibold tracking-wide text-muted uppercase">Reference solution</h3>
        <CodeBlock code={finish.solution} className="max-h-72" />

        <div className="mt-6 flex justify-end gap-2">
          <button className="btn" onClick={onClose}>
            Stay here
          </button>
          <button className="btn btn-primary" onClick={onDashboard}>
            Back to dashboard <Icon name="chevron" size={14} />
          </button>
        </div>
      </Modal>
    </>
  );
}

function Metric({ label, value, delta }: { label: string; value: string; delta?: number }) {
  return (
    <div className="rounded-xl border border-line bg-surface-2 p-3">
      <div className="text-xs text-muted">{label}</div>
      <div className="mt-1 flex items-baseline gap-2 text-lg font-semibold tabular">
        {value}
        {delta !== undefined && (
          <span className="text-sm" style={{ color: delta >= 0 ? "var(--good)" : "var(--bad)" }}>
            {delta >= 0 ? "+" : ""}
            {delta}
          </span>
        )}
      </div>
    </div>
  );
}
