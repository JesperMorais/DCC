import { useMemo, useState } from "react";
import { api, type PublicQuestion, type QuizResult } from "../api";
import { CodeBlock, Icon, Markdown } from "../ui";
import { Mascot } from "./Mascot";

type Answer = number | number[] | string[] | string | null;

export function Quiz({
  path,
  node,
  questions,
  passed,
  onResult,
}: {
  path: string;
  node: string;
  questions: PublicQuestion[];
  passed: boolean;
  onResult: (r: QuizResult) => void;
}) {
  const initial = useMemo<Answer[]>(
    () => questions.map((q) => (q.type === "order" ? [...(q as { items: string[] }).items] : q.type === "multi" ? [] : null)),
    [questions],
  );
  const [answers, setAnswers] = useState<Answer[]>(initial);
  const [result, setResult] = useState<QuizResult | null>(null);
  const [busy, setBusy] = useState(false);

  const setA = (i: number, v: Answer) => {
    setAnswers((a) => a.map((x, k) => (k === i ? v : x)));
    if (result) setResult(null);
  };
  const complete = answers.every((a, i) => {
    const q = questions[i];
    if (q.type === "multi") return Array.isArray(a) && a.length > 0;
    if (q.type === "number") return a !== null && a !== "" && Number.isFinite(Number(a));
    return a !== null;
  });

  const check = async () => {
    setBusy(true);
    try {
      const payload = answers.map((a, i) => (questions[i].type === "number" ? Number(a) : a));
      const r = await api.submitQuiz(path, node, payload);
      setResult(r);
      onResult(r);
    } finally {
      setBusy(false);
    }
  };

  return (
    <div className="flex flex-col gap-4">
      {questions.map((q, i) => {
        const res = result?.results[i];
        return (
          <div
            key={i}
            className={`rounded-xl border p-4 ${res ? (res.correct ? "border-good/40 bg-good-soft" : "border-bad/40 bg-bad-soft") : "border-line bg-surface"}`}
          >
            <div className="mb-2 flex items-start gap-2">
              <span className="grid size-6 shrink-0 place-items-center rounded-full bg-surface-2 text-xs font-semibold text-ink-2">{i + 1}</span>
              <Markdown source={q.q} className="!text-[14px] !text-ink" />
              {res && <Icon name={res.correct ? "check" : "x"} size={18} className={`ml-auto shrink-0 ${res.correct ? "text-good" : "text-bad"}`} />}
            </div>
            {q.code && <CodeBlock code={q.code} className="mb-3 !text-[12px]" />}

            {(q.type === "single" || q.type === "multi") && (
              <div className="flex flex-col gap-1.5">
                {(q as { options: string[] }).options.map((opt, k) => {
                  const multi = q.type === "multi";
                  const on = multi ? (answers[i] as number[]).includes(k) : answers[i] === k;
                  return (
                    <button
                      key={k}
                      onClick={() =>
                        multi
                          ? setA(i, on ? (answers[i] as number[]).filter((x) => x !== k) : [...(answers[i] as number[]), k])
                          : setA(i, k)
                      }
                      className={`flex items-start gap-2.5 rounded-lg border px-3 py-2 text-left text-sm transition-colors ${
                        on ? "border-accent bg-accent-soft text-ink" : "border-line bg-surface-2 text-ink-2 hover:border-line-strong"
                      }`}
                    >
                      <span
                        className={`mt-0.5 grid size-4 shrink-0 place-items-center border ${multi ? "rounded" : "rounded-full"} ${on ? "border-accent bg-accent text-white" : "border-line-strong"}`}
                      >
                        {on && <Icon name="check" size={10} strokeWidth={3.5} />}
                      </span>
                      <Markdown source={opt} className="!text-[13px]" />
                    </button>
                  );
                })}
                {q.type === "multi" && <p className="text-xs text-muted">Select all that apply.</p>}
              </div>
            )}

            {q.type === "order" && (
              <div className="flex flex-col gap-1.5">
                {(answers[i] as string[]).map((item, k, arr) => (
                  <div key={item} className="flex items-center gap-2 rounded-lg border border-line bg-surface-2 px-2 py-1.5 text-sm text-ink">
                    <span className="w-5 text-center text-xs font-semibold text-muted tabular">{k + 1}</span>
                    <Markdown source={item} className="!text-[13px] flex-1" />
                    <button
                      className="rounded p-1 text-muted hover:bg-surface-3 hover:text-ink disabled:opacity-30"
                      disabled={k === 0}
                      onClick={() => {
                        const n = [...arr];
                        [n[k - 1], n[k]] = [n[k], n[k - 1]];
                        setA(i, n);
                      }}
                      aria-label="Move up"
                    >
                      <Icon name="up" size={14} />
                    </button>
                    <button
                      className="rounded p-1 text-muted hover:bg-surface-3 hover:text-ink disabled:opacity-30"
                      disabled={k === arr.length - 1}
                      onClick={() => {
                        const n = [...arr];
                        [n[k + 1], n[k]] = [n[k], n[k + 1]];
                        setA(i, n);
                      }}
                      aria-label="Move down"
                    >
                      <Icon name="down" size={14} />
                    </button>
                  </div>
                ))}
                <p className="text-xs text-muted">Use the arrows to put them in order.</p>
              </div>
            )}

            {q.type === "number" && (
              <div className="flex items-center gap-2">
                <input
                  type="number"
                  step="any"
                  value={(answers[i] as string | null) ?? ""}
                  onChange={(e) => setA(i, e.target.value)}
                  className="h-9 w-36 rounded-lg border border-line-strong bg-surface-2 px-3 text-sm text-ink tabular outline-none focus:border-accent"
                  placeholder="Your answer"
                />
                {"unit" in q && q.unit && <span className="text-sm text-muted">{q.unit}</span>}
              </div>
            )}

            {res && (
              <div className="pop-in mt-3 border-t border-line pt-3">
                <div className={`mb-1 text-xs font-semibold ${res.correct ? "text-good" : "text-bad"}`}>{res.correct ? "Correct" : "Not quite"}</div>
                <Markdown source={res.explain} className="!text-[13px]" />
              </div>
            )}
          </div>
        );
      })}

      {result?.allCorrect ? (
        <div className="pop-in flex items-center gap-3 rounded-xl border border-good/30 bg-good-soft p-3">
          <Mascot mood="happy" size={44} />
          <div className="text-sm text-ink">
            <b className="text-good">All correct{result.firstTry ? " on the first try" : ""}.</b>{" "}
            {result.completed ? `Node complete: +${result.xpGained} XP.` : "Now finish the lab to complete this node."}
          </div>
        </div>
      ) : (
        <div className="flex items-center gap-3">
          <button className="btn btn-primary" disabled={!complete || busy} onClick={check}>
            <Icon name="check" size={15} /> {result ? "Check again" : passed ? "Check (already passed)" : "Check answers"}
          </button>
          {result && !result.allCorrect && (
            <span className="text-sm text-ink-2">
              {result.results.filter((r) => r.correct).length}/{result.results.length} right. Read the explanations, adjust, and check again.
            </span>
          )}
        </div>
      )}
    </div>
  );
}
