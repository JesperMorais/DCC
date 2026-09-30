import { useState } from "react";
import { api, type Experience } from "../api";
import { Mascot } from "../components/Mascot";
import { Icon, LevelPips } from "../ui";

const OPTIONS: { key: Experience; title: string; body: string; level: number }[] = [
  { key: "new", title: "Brand new to coding", body: "I've never really written code. Start from the very beginning.", level: 1 },
  { key: "other-lang", title: "I know another language", body: "Python, C#, Java… I get loops and functions, TypeScript is new.", level: 2 },
  { key: "js", title: "I write JavaScript", body: "Comfortable with JS. Types and interfaces are the new part.", level: 3 },
  { key: "some-ts", title: "I use some TypeScript", body: "I've shipped TS, but generics and advanced types are fuzzy.", level: 4 },
  { key: "pro", title: "TypeScript regular", body: "Bring on async patterns, conditional types and infer.", level: 5 },
];

export default function Onboarding({ onDone }: { onDone: () => void }) {
  const [name, setName] = useState("");
  const [exp, setExp] = useState<Experience | null>(null);
  const [busy, setBusy] = useState(false);

  const start = async () => {
    if (!exp) return;
    setBusy(true);
    await api.createProfile(name, exp);
    onDone();
  };

  return (
    <div className="min-h-full bg-bg px-4 py-12">
      <div className="pop-in mx-auto max-w-2xl">
        <div className="mb-6 flex items-center gap-5">
          <Mascot mood="wave" size={132} className="-ml-3 shrink-0 text-ink-2" />
          <div>
            <span className="text-xl font-semibold tracking-tight">
              daily<span className="text-accent">.ts</span>
            </span>
            <div className="relative mt-2 rounded-2xl rounded-bl-sm border border-line bg-surface px-4 py-2.5 text-sm text-ink-2 shadow-sm">
              Hi, I'm <b className="text-ink">Tess</b> 👋 Pangolins are covered in scales, and your code gets covered in types. Both are armour.
            </div>
          </div>
        </div>
        <h1 className="text-3xl font-semibold tracking-tight">One small TypeScript challenge a day.</h1>
        <p className="mt-3 max-w-xl text-ink-2">
          About ten minutes each. Every challenge teaches one concept, and the difficulty adjusts to how you do — solve fast
          and it gets harder, struggle and it eases off. Everything runs and stays on this machine.
        </p>

        <div className="card mt-8 p-6">
          <label className="text-sm font-medium" htmlFor="name">
            What should we call you?
          </label>
          <input
            id="name"
            value={name}
            onChange={(e) => setName(e.target.value)}
            placeholder="Your name"
            className="mt-2 h-10 w-full rounded-lg border border-line-strong bg-surface-2 px-3 text-sm outline-none focus:border-accent"
            autoFocus
          />

          <p className="mt-6 text-sm font-medium">Where are you starting from?</p>
          <p className="mt-1 text-xs text-muted">This only sets your starting point. Your rating takes over after a few challenges.</p>
          <div className="mt-3 grid gap-2">
            {OPTIONS.map((o) => (
              <button
                key={o.key}
                onClick={() => setExp(o.key)}
                className={`flex items-center gap-4 rounded-xl border p-3.5 text-left transition-colors ${
                  exp === o.key ? "border-accent bg-accent-soft" : "border-line hover:border-line-strong hover:bg-surface-2"
                }`}
              >
                <LevelPips level={o.level} />
                <div className="min-w-0 flex-1">
                  <div className="text-sm font-semibold">{o.title}</div>
                  <div className="text-xs text-ink-2">{o.body}</div>
                </div>
                <span
                  className={`grid size-5 place-items-center rounded-full border ${exp === o.key ? "border-accent bg-accent text-white" : "border-line-strong"}`}
                >
                  {exp === o.key && <Icon name="check" size={12} strokeWidth={3} />}
                </span>
              </button>
            ))}
          </div>

          <button className="btn btn-primary mt-6 h-11 w-full justify-center text-[15px]" disabled={!exp || busy} onClick={start}>
            Start my first challenge <Icon name="chevron" size={16} />
          </button>
        </div>
      </div>
    </div>
  );
}
