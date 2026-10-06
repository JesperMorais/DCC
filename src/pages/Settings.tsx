import { useState } from "react";
import { Link } from "react-router-dom";
import { api, type Dashboard } from "../api";
import { Mascot } from "../components/Mascot";
import { setTessVoice, setConceptTips, useConceptTips, useTessVoice, type TessVoice } from "../tess/prefs";
import { setTheme, useTheme, type ThemePref } from "../theme";

export default function SettingsPage({ state, onChange }: { state: Dashboard; onChange: () => void }) {
  const [name, setName] = useState(state.profile?.name ?? "");
  const [saved, setSaved] = useState(false);
  const [confirmReset, setConfirmReset] = useState("");
  const { pref } = useTheme();
  const voice = useTessVoice();
  const tips = useConceptTips();

  return (
    <div className="mx-auto max-w-2xl px-4 py-8 sm:px-8">
      <h1 className="text-[28px] font-semibold tracking-tight">Settings</h1>

      <section className="card mt-6 p-5">
        <h2 className="text-[15px] font-semibold">Profile</h2>
        <label className="mt-4 block text-sm text-ink-2" htmlFor="name">
          Name
        </label>
        <div className="mt-1.5 flex gap-2">
          <input
            id="name"
            value={name}
            onChange={(e) => {
              setName(e.target.value);
              setSaved(false);
            }}
            className="h-9 flex-1 rounded-lg border border-line-strong bg-surface-2 px-3 text-sm outline-none focus:border-accent"
          />
          <button
            className="btn btn-primary"
            onClick={async () => {
              await api.rename(name);
              setSaved(true);
              onChange();
            }}
          >
            {saved ? "Saved" : "Save"}
          </button>
        </div>
        <p className="mt-3 text-xs text-muted">
          Member since {new Date(state.profile!.createdAt).toLocaleDateString()} · {state.languages.filter((l) => l.started).map((l) => `${l.id === "typescript" ? "TS" : l.id === "python" ? "Python" : "C"} ${l.rating}`).join(" · ")}
        </p>
      </section>

      <section className="card mt-5 p-5">
        <h2 className="text-[15px] font-semibold">Appearance</h2>
        <div className="mt-4 flex rounded-lg border border-line bg-surface-2 p-0.5">
          {(["system", "light", "dark"] as ThemePref[]).map((t) => (
            <button
              key={t}
              onClick={() => setTheme(t)}
              className={`flex-1 rounded-md py-1.5 text-sm font-medium capitalize ${pref === t ? "bg-surface text-ink shadow-sm" : "text-muted"}`}
            >
              {t}
            </button>
          ))}
        </div>
      </section>

      <section className="card mt-5 p-5">
        <h2 className="text-[15px] font-semibold">How the difficulty works</h2>
        <div className="prose-ts mt-3 !text-sm">
          <p>
            You and every challenge each have a rating, like chess players. Before a challenge you're shown your <b>match</b>: the chance
            you'd solve it cleanly. Afterwards your rating moves by how much better or worse you did than that.
          </p>
          <ul>
            <li>Solving counts most. Hints scale the score down: if you needed every hint on a hard challenge, your rating stays where it is, so you keep practising at this level until it sticks.</li>
            <li>Time doesn't count. Take as long as you need; a very slow solve only marks the topic as worth practising in the skill tree.</li>
            <li>Giving up counts as 0, and the challenge comes back in a week.</li>
            <li>Only your first attempt at a challenge is rated. Practising it again is free.</li>
            <li>Your first five results move your rating more, so it finds your level quickly.</li>
            <li>Each language (TypeScript, Python, C) has its own rating and its own daily challenge. Your streak counts any language.</li>
          </ul>
        </div>
      </section>

      <section className="card mt-5 p-5">
        <div className="flex items-center gap-4">
          <Mascot mood={voice === "off" ? "sleep" : voice === "quiet" ? "idle" : "wave"} size={64} className="shrink-0 text-ink-2" />
          <div className="flex-1">
            <h2 className="text-[15px] font-semibold">Tess</h2>
            <p className="text-sm text-ink-2">
              How much the pangolin talks. <Link to="/mascot" className="text-accent hover:underline">Meet Tess</Link>
            </p>
          </div>
        </div>
        <div className="mt-4 flex rounded-lg border border-line bg-surface-2 p-0.5">
          {(
            [
              ["chatty", "Chatty"],
              ["quiet", "Quiet"],
              ["off", "Off"],
            ] as [TessVoice, string][]
          ).map(([v, label]) => (
            <button
              key={v}
              onClick={() => setTessVoice(v)}
              className={`flex-1 rounded-md py-1.5 text-sm font-medium ${voice === v ? "bg-surface text-ink shadow-sm" : "text-muted"}`}
            >
              {label}
            </button>
          ))}
        </div>
        <p className="mt-2 text-xs text-muted">
          {voice === "chatty" && "Greetings, reactions to every run, hints in Tess's voice, and celebrations."}
          {voice === "quiet" && "Greetings, celebrations and concept tips only. No reactions while you code."}
          {voice === "off" && "No speech bubbles. Tess only shows up as art."}
        </p>
        <label className="mt-4 flex items-center justify-between gap-3 border-t border-line pt-4 text-sm">
          <span>
            <span className="font-medium">Concept tips</span>
            <span className="block text-xs text-muted">Tess suggests the free Concept lesson when you're stuck. It switches itself off after 3 dismissals.</span>
          </span>
          <input type="checkbox" className="size-4 accent-[var(--accent)]" checked={tips} onChange={(e) => setConceptTips(e.target.checked)} />
        </label>
      </section>

      <section className="card mt-5 border-bad/30 p-5">
        <h2 className="text-[15px] font-semibold text-bad">Reset progress</h2>
        <p className="mt-2 text-sm text-ink-2">Deletes your profile, rating and history from this machine. This can't be undone.</p>
        <div className="mt-3 flex gap-2">
          <input
            value={confirmReset}
            onChange={(e) => setConfirmReset(e.target.value)}
            placeholder='Type "reset" to confirm'
            className="h-9 flex-1 rounded-lg border border-line-strong bg-surface-2 px-3 text-sm outline-none focus:border-bad"
          />
          <button
            className="btn"
            disabled={confirmReset !== "reset"}
            onClick={async () => {
              await api.reset();
              window.location.href = "/";
            }}
          >
            Reset everything
          </button>
        </div>
      </section>
    </div>
  );
}
