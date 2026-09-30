import { useState } from "react";
import { api, type Experience } from "../api";
import { ExperiencePicker } from "../components/ExperiencePicker";
import { Mascot } from "../components/Mascot";
import { LangBadge, LANGUAGES, type Lang } from "../lang";
import { Icon } from "../ui";

/** First visit to a language: Tess introduces the track and asks where you're starting from. */
export default function LanguageStart({ lang, onStarted }: { lang: Lang; onStarted: () => void }) {
  const [exp, setExp] = useState<Experience | null>(null);
  const [busy, setBusy] = useState(false);
  const info = LANGUAGES[lang];

  return (
    <div className="mx-auto max-w-2xl px-4 py-10 sm:px-8">
      <div className="pop-in">
        <div className="flex items-center gap-5">
          <Mascot mood="wave" size={120} className="-ml-2 shrink-0 text-ink-2" />
          <div>
            <div className="flex items-center gap-2.5">
              <LangBadge lang={lang} size={28} />
              <h1 className="text-[26px] font-semibold tracking-tight">{info.name} track</h1>
            </div>
            <div className="relative mt-2 rounded-2xl rounded-bl-sm border border-line bg-surface px-4 py-2.5 text-sm text-ink-2 shadow-sm">{info.tessIntro}</div>
          </div>
        </div>
        <p className="mt-6 text-ink-2">{info.tagline}</p>
        <div className="mt-3 flex flex-wrap gap-2 text-xs text-muted">
          {info.levels.slice(1).map((name, i) => (
            <span key={name} className="rounded-md border border-line bg-surface px-2 py-1">
              L{i + 1} · {name}
            </span>
          ))}
        </div>

        <div className="card mt-6 p-6">
          <p className="text-sm font-medium">How much {info.name} do you know?</p>
          <p className="mt-1 text-xs text-muted">This gives {info.name} its own starting rating. Your other languages aren't affected, but your streak carries over.</p>
          <div className="mt-3">
            <ExperiencePicker lang={lang} value={exp} onChange={setExp} />
          </div>
          <button
            className="btn btn-primary mt-6 h-11 w-full justify-center text-[15px]"
            disabled={!exp || busy}
            onClick={async () => {
              setBusy(true);
              await api.startLanguage(lang, exp!);
              onStarted();
            }}
          >
            Start {info.name} <Icon name="chevron" size={16} />
          </button>
        </div>
      </div>
    </div>
  );
}
