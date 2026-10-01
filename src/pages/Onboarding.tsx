import { useState } from "react";
import { useNavigate } from "react-router-dom";
import { api, type Experience } from "../api";
import { ExperiencePicker } from "../components/ExperiencePicker";
import { Mascot } from "../components/Mascot";
import { LangBadge, LANGUAGES, useLangColors, type Lang } from "../lang";
import { Icon } from "../ui";

const PICK_LINE: Record<Experience, string> = {
  new: "We'll start from the very beginning. I'll explain things as we go.",
  "other-lang": "You know the ideas already. Now we learn how this language says them.",
  js: "Good, you've got the basics. The fun part is next.",
  "some-ts": "Let's firm up the tricky parts.",
  pro: "Hard mode it is. Let's see what you've got.",
};

export default function Onboarding({ onDone }: { onDone: () => void }) {
  const [name, setName] = useState("");
  const [lang, setLang] = useState<Lang>("typescript");
  useLangColors(lang);
  const [exp, setExp] = useState<Experience | null>(null);
  const [busy, setBusy] = useState(false);
  const navigate = useNavigate();

  const start = async () => {
    if (!exp) return;
    setBusy(true);
    await api.createProfile(name, exp, lang);
    navigate(`/${lang}`);
    onDone();
  };

  return (
    <div className="min-h-full bg-bg px-4 py-12">
      <div className="pop-in mx-auto max-w-2xl">
        <div className="mb-6 flex items-center gap-5">
          <Mascot mood="wave" size={132} className="-ml-3 shrink-0 text-ink-2" />
          <div>
            <span className="text-xl font-semibold tracking-tight">
              daily<span className="text-accent">.{LANGUAGES[lang].ext}</span>
            </span>
            <div className="relative mt-2 rounded-2xl rounded-bl-sm border border-line bg-surface px-4 py-2.5 text-sm text-ink-2 shadow-sm">
              {exp ? (
                PICK_LINE[exp]
              ) : (
                <>
                  Hi, I'm <b className="text-ink">Tess</b>. {LANGUAGES[lang].tessIntro}
                </>
              )}
            </div>
          </div>
        </div>
        <h1 className="text-3xl font-semibold tracking-tight">One small coding challenge a day.</h1>
        <p className="mt-3 max-w-xl text-ink-2">
          About ten minutes each, in TypeScript, Python or C. Every challenge teaches one concept, and the difficulty adjusts to how you do.
          Solve fast and it gets harder; struggle and it eases off. Everything runs and stays on this machine.
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

          <p className="mt-6 text-sm font-medium">Which language first?</p>
          <p className="mt-1 text-xs text-muted">You can add the others any time. Each language has its own rating.</p>
          <div className="mt-3 grid gap-2 sm:grid-cols-3">
            {(Object.keys(LANGUAGES) as Lang[]).map((l) => (
              <button
                key={l}
                onClick={() => setLang(l)}
                className={`flex items-center gap-3 rounded-xl border p-3 text-left transition-colors ${
                  lang === l ? "border-accent bg-accent-soft" : "border-line hover:border-line-strong hover:bg-surface-2"
                }`}
              >
                <LangBadge lang={l} size={30} />
                <span className="text-sm font-semibold">{LANGUAGES[l].name}</span>
              </button>
            ))}
          </div>

          <p className="mt-6 text-sm font-medium">Where are you starting from in {LANGUAGES[lang].name}?</p>
          <p className="mt-1 text-xs text-muted">This only sets your starting point. Your rating takes over after a few challenges.</p>
          <div className="mt-3">
            <ExperiencePicker lang={lang} value={exp} onChange={setExp} />
          </div>

          <button className="btn btn-primary mt-6 h-11 w-full justify-center text-[15px]" disabled={!exp || busy} onClick={start}>
            Start my first challenge <Icon name="chevron" size={16} />
          </button>
        </div>
      </div>
    </div>
  );
}
