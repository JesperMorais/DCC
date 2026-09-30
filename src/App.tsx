import { useCallback, useEffect, useState } from "react";
import { NavLink, Route, Routes, useLocation } from "react-router-dom";
import { api, isOnboarded, type Dashboard } from "./api";
import ChallengePage from "./pages/Challenge";
import DashboardPage from "./pages/Dashboard";
import LibraryPage from "./pages/Library";
import MascotGallery from "./pages/MascotGallery";
import Onboarding from "./pages/Onboarding";
import SettingsPage from "./pages/Settings";
import { setTheme, useTheme } from "./theme";
import { Mascot } from "./components/Mascot";
import { Icon, type IconName } from "./ui";

export default function App() {
  const [state, setState] = useState<Dashboard | null | "loading" | "error">("loading");
  const location = useLocation();

  const refresh = useCallback(async () => {
    try {
      const s = await api.state();
      setState(isOnboarded(s) ? s : null);
    } catch {
      setState("error");
    }
  }, []);

  useEffect(() => {
    refresh();
  }, [refresh, location.pathname]);

  if (state === "loading")
    return (
      <div className="grid h-full place-items-center text-muted">
        <Mascot mood="think" size={96} className="text-ink-2" />
      </div>
    );
  if (state === "error")
    return (
      <div className="grid h-full place-items-center text-center text-ink-2">
        <div className="flex flex-col items-center">
          <Mascot mood="sleep" size={120} className="text-ink-2" />
          <p className="mt-3 font-semibold text-ink">Tess can't reach the local server.</p>
          <p className="mt-1 text-sm">
            Start it with <code className="font-mono">npm run dev</code> and reload.
          </p>
        </div>
      </div>
    );
  if (state === null) return <Onboarding onDone={refresh} />;

  const inChallenge = location.pathname.startsWith("/c/");

  return (
    <div className="flex h-full">
      <Sidebar state={state} />
      <main className={`min-w-0 flex-1 ${inChallenge ? "overflow-hidden" : "overflow-y-auto"}`}>
        <Routes>
          <Route path="/" element={<DashboardPage state={state} />} />
          <Route path="/library" element={<LibraryPage />} />
          <Route path="/mascot" element={<MascotGallery />} />
          <Route path="/settings" element={<SettingsPage state={state} onChange={refresh} />} />
          <Route path="/c/:id" element={<ChallengePage onFinished={refresh} />} />
        </Routes>
      </main>
    </div>
  );
}

function Sidebar({ state }: { state: Dashboard }) {
  const { theme } = useTheme();
  const items: { to: string; icon: IconName; label: string }[] = [
    { to: "/", icon: "dashboard", label: "Dashboard" },
    { to: "/library", icon: "library", label: "Library" },
    { to: "/settings", icon: "settings", label: "Settings" },
  ];
  return (
    <aside className="flex w-[68px] shrink-0 flex-col items-center border-r border-line bg-surface py-4 lg:w-[220px] lg:items-stretch lg:px-3">
      <NavLink to="/" className="mb-6 flex items-center gap-2.5 px-2">
        <Mascot mood="idle" size={36} className="shrink-0" title="daily.ts" />
        <span className="hidden text-[15px] font-semibold tracking-tight lg:block">
          daily<span className="text-accent">.ts</span>
        </span>
      </NavLink>
      <nav className="flex flex-col gap-1">
        {items.map((it) => (
          <NavLink
            key={it.to}
            to={it.to}
            end={it.to === "/"}
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

      <div className="mt-auto flex flex-col gap-3">
        <div className="hidden rounded-xl border border-line bg-surface-2 p-3 lg:block">
          <div className="flex items-center gap-2 text-xs text-muted">
            <Icon name="flame" size={14} style={{ color: state.stats.streak ? "var(--flame)" : undefined }} />
            Streak
          </div>
          <div className="mt-1 text-xl font-semibold tabular">
            {state.stats.streak} <span className="text-sm font-medium text-muted">{state.stats.streak === 1 ? "day" : "days"}</span>
          </div>
          <p className="mt-1 text-xs text-muted">{state.stats.solvedToday ? "Done for today — nice." : "Solve one today to keep it going."}</p>
        </div>
        <button
          className="btn btn-ghost justify-center lg:justify-start"
          onClick={() => setTheme(theme === "dark" ? "light" : "dark")}
          title="Toggle theme"
        >
          <Icon name={theme === "dark" ? "sun" : "moon"} size={16} />
          <span className="hidden lg:inline">{theme === "dark" ? "Light mode" : "Dark mode"}</span>
        </button>
      </div>
    </aside>
  );
}
