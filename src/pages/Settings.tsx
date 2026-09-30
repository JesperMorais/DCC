import { useState } from "react";
import { Link } from "react-router-dom";
import { api, type Dashboard } from "../api";
import { Mascot } from "../components/Mascot";
import { setTheme, useTheme, type ThemePref } from "../theme";

export default function SettingsPage({ state, onChange }: { state: Dashboard; onChange: () => void }) {
  const [name, setName] = useState(state.profile?.name ?? "");
  const [saved, setSaved] = useState(false);
  const [confirmReset, setConfirmReset] = useState("");
  const { pref } = useTheme();

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
          Member since {new Date(state.profile!.createdAt).toLocaleDateString()} · rating {state.stats.rating}
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
            <li>Solving counts most. Each hint takes about 12% off that attempt's score, and going well past ~10 minutes takes off a little more.</li>
            <li>Giving up counts as 0, and the challenge comes back in a week.</li>
            <li>Only your first attempt at a challenge is rated. Practising it again is free.</li>
            <li>Your first five results move your rating more, so it finds your level quickly.</li>
          </ul>
        </div>
      </section>

      <Link to="/mascot" className="card mt-5 flex items-center gap-4 p-5 hover:border-line-strong">
        <Mascot mood="wave" size={64} className="shrink-0 text-ink-2" />
        <div>
          <h2 className="text-[15px] font-semibold">Meet Tess</h2>
          <p className="text-sm text-ink-2">The daily.ts pangolin. The only mammal with armour-plated scales, just like your code with types.</p>
        </div>
      </Link>

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
