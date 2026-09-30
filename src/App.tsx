import { useCallback, useEffect, useState } from "react";
import { Navigate, NavLink, Route, Routes, useLocation, useNavigate } from "react-router-dom";
import { api, isOnboarded, type Dashboard } from "./api";
import { Mascot, TessLoading } from "./components/Mascot";
import { LangBadge, LANGUAGES, useActiveLang, type Lang } from "./lang";
import ChallengePage from "./pages/Challenge";
import DashboardPage from "./pages/Dashboard";
import LanguageStart from "./pages/LanguageStart";
import LibraryPage from "./pages/Library";
import MascotGallery from "./pages/MascotGallery";
import Onboarding from "./pages/Onboarding";
import SettingsPage from "./pages/Settings";
import { sidebarStreakLine } from "./tess/lines";
import { useTessVoice } from "./tess/prefs";
import { setTheme, useTheme } from "./theme";
import { Icon, type IconName } from "./ui";

export default function App() {
  const [state, setState] = useState<Dashboard | null | "loading" | "error">("loading");
  const location = useLocation();
  const lang = useActiveLang();

  const refresh = useCallback(async () => {
    try {
      const s = await api.state(lang);
      setState(isOnboarded(s) ? s : null);
    } catch {
      setState("error");
    }
  }, [lang]);

  useEffect(() => {
    refresh();
  }, [refresh, location.pathname]);

  if (state === "loading") return <TessLoading />;
  if (state === "error")
    return (
      <div className="grid h-full place-items-center text-center text-ink-2">
        <div className="flex flex-col items-center">
          <Mascot mood="sad" size={120} className="text-ink-2" />
          <p className="mt-3 font-semibold text-ink">Tess can't reach the local server.</p>
          <p className="mt-1 text-sm">
            Start it with <code className="font-mono">npm run dev</code> and reload.
          </p>
        </div>
      </div>
    );
  if (state === null) return <Onboarding onDone={refresh} />;

  const inChallenge = location.pathname.startsWith("/solve/");
  // The state may briefly belong to the previous language while switching.
  const ready = state.language === lang;
  const gate = (el: React.ReactNode) => (!ready ? <TessLoading /> : state.started ? el : <LanguageStart lang={lang} onStarted={refresh} />);

  return (
    <div className="flex h-full">
      <Sidebar state={state} lang={lang} />
      <main className={`min-w-0 flex-1 ${inChallenge ? "overflow-hidden" : "overflow-y-auto"}`}>
        <Routes>
          <Route path="/" element={<Navigate to={`/${lang}`} replace />} />
          <Route path="/mascot" element={<MascotGallery />} />
          <Route path="/settings" element={<SettingsPage state={state} onChange={refresh} />} />
          <Route path="/solve/:lang/:slug" element={gate(<ChallengePage onFinished={refresh} />)} />
          <Route path="/:lang" element={gate(<DashboardPage state={state} />)} />
          <Route path="/:lang/library" element={gate(<LibraryPage />)} />
          <Route path="*" element={<Navigate to={`/${lang}`} replace />} />
        </Routes>
      </main>
    </div>
  );
}

function LanguageSwitcher({ state, lang }: { state: Dashboard; lang: Lang }) {
  const navigate = useNavigate();
  return (
    <div className="flex flex-col gap-1">
      <div className="mb-1 hidden px-2.5 text-[11px] font-semibold tracking-wider text-muted uppercase lg:block">Languages</div>
      {state.languages.map((l) => {
        const active = l.id === lang;
        return (
          <button
            key={l.id}
            onClick={() => navigate(`/${l.id}`)}
            title={LANGUAGES[l.id].name}
            className={`flex items-center gap-3 rounded-lg px-2 py-1.5 text-left text-sm transition-colors ${
              active ? "bg-surface-2 text-ink" : "text-ink-2 hover:bg-surface-2 hover:text-ink"
            }`}
          >
            <LangBadge lang={l.id} size={24} />
            <span className="hidden min-w-0 flex-1 lg:block">
              <span className="block truncate font-medium">{LANGUAGES[l.id].name}</span>
              <span className="block text-[11px] text-muted">
                {l.started ? `${l.rating} · L${l.level} · ${l.solved}/${l.total}` : "Not started yet"}
              </span>
            </span>
            {l.started && l.dailyDone && <Icon name="check" size={14} className="hidden text-good lg:block" />}
          </button>
        );
      })}
    </div>
  );
}

function Sidebar({ state, lang }: { state: Dashboard; lang: Lang }) {
  const { theme } = useTheme();
  const voice = useTessVoice();
  const items: { to: string; icon: IconName; label: string; end?: boolean }[] = [
    { to: `/${lang}`, icon: "dashboard", label: "Dashboard", end: true },
    { to: `/${lang}/library`, icon: "library", label: "Library" },
    { to: "/settings", icon: "settings", label: "Settings" },
  ];
  return (
    <aside className="flex w-[68px] shrink-0 flex-col items-center overflow-y-auto border-r border-line bg-surface py-4 lg:w-[232px] lg:items-stretch lg:px-3">
      <NavLink to={`/${lang}`} className="mb-6 flex items-center gap-2.5 px-2">
        <Mascot mood="idle" size={36} className="shrink-0" title="daily.ts" />
        <span className="hidden text-[15px] font-semibold tracking-tight lg:block">
          daily<span className="text-accent">.{LANGUAGES[lang].ext}</span>
        </span>
      </NavLink>
      <nav className="flex flex-col gap-1">
        {items.map((it) => (
          <NavLink
            key={it.to}
            to={it.to}
            end={it.end}
            className={({ isActive }) =>
              `flex items-center gap-3 rounded-lg px-2.5 py-2 text-sm font-medium transition-colors ${
                isActive ? "bg-accent-soft text-accent-strong" : "text-ink-2 hover:bg-surface-2 hover:text-ink"
              }`
            }
          >
            <Icon name={it.icon} size={18} />
            <span className="hidden lg:block">{it.label}</span>
          </NavLink>
        ))}
      </nav>

      <div className="mt-6 border-t border-line pt-4">
        <LanguageSwitcher state={state} lang={lang} />
      </div>

      <div className="mt-auto flex flex-col gap-3 pt-6">
        <div className="hidden rounded-xl border border-line bg-surface-2 p-3 lg:block">
          <div className="flex items-center gap-2 text-xs text-muted">
            <Icon name="flame" size={14} style={{ color: state.stats.streak ? "var(--flame)" : undefined }} />
            Streak · all languages
          </div>
          <div className="mt-1 text-xl font-semibold tabular">
            {state.stats.streak} <span className="text-sm font-medium text-muted">{state.stats.streak === 1 ? "day" : "days"}</span>
          </div>
          <p className="mt-1 text-xs text-muted">
            {voice === "off" ? (state.stats.solvedToday ? "Done for today." : "Solve one today to keep it going.") : sidebarStreakLine(state)}
          </p>
        </div>
        <button className="btn btn-ghost justify-center lg:justify-start" onClick={() => setTheme(theme === "dark" ? "light" : "dark")} title="Toggle theme">
          <Icon name={theme === "dark" ? "sun" : "moon"} size={16} />
          <span className="hidden lg:inline">{theme === "dark" ? "Light mode" : "Dark mode"}</span>
        </button>
      </div>
    </aside>
  );
}
