import { useState, type ReactNode } from "react";
import { Link, useNavigate } from "react-router-dom";
import type { Dashboard } from "../api";
import { Heatmap, LevelProgress, RatingChart, TopicMastery } from "../components/charts";
import { Mascot, TessSays, type Mood } from "../components/Mascot";
import { LangBadge, LANGUAGES } from "../lang";
import { greeting as tessGreeting } from "../tess/lines";
import { read, useTessVoice, write } from "../tess/prefs";
import { fmtMinutes, Icon, LevelBadge, relTime, StatusDot, Tag, type IconName } from "../ui";

const greeting = () => {
  const h = new Date().getHours();
  return h < 5 ? "Up late" : h < 12 ? "Good morning" : h < 18 ? "Good afternoon" : "Good evening";
};

export default function DashboardPage({ state }: { state: Dashboard }) {
  const { stats, level, daily, next } = state;
  const name = state.profile?.name ?? "";

  return (
    <div className="mx-auto max-w-[1240px] px-4 py-8 sm:px-8">
      <header className="mb-7 flex flex-wrap items-end justify-between gap-4">
        <div>
          <p className="flex items-center gap-2 text-sm text-muted">
            <LangBadge lang={state.language} size={18} />
            <span className="font-medium text-ink-2">{LANGUAGES[state.language].name}</span> ·{" "}
            {new Date().toLocaleDateString(undefined, { weekday: "long", month: "long", day: "numeric" })}
          </p>
          <h1 className="mt-1 text-[28px] font-semibold tracking-tight">
            {greeting()}, {name}
          </h1>
        </div>
        <Link to={`/${state.language}/library`} className="btn">
          <Icon name="library" /> Browse all challenges
        </Link>
      </header>

      <div className="grid gap-5 lg:grid-cols-3">
        <DailyCard state={state} />
        <RatingCard stats={stats} level={level} lang={state.language} />
      </div>

      <div className="mt-5 grid grid-cols-2 gap-5 lg:grid-cols-4">
        <StatTile
          icon="flame"
          iconColor={stats.streak ? "var(--flame)" : undefined}
          label="Current streak"
          value={`${stats.streak} ${stats.streak === 1 ? "day" : "days"}`}
          sub={`Best ${stats.bestStreak} ${stats.bestStreak === 1 ? "day" : "days"}`}
        />
        <StatTile icon="trophy" label="Challenges solved" value={String(stats.solved)} sub={`of ${stats.totalChallenges} in the library`} />
        <StatTile
          icon="target"
          label="Success rate"
          value={stats.successRate === null ? "—" : `${Math.round(stats.successRate * 100)}%`}
          sub={`${stats.attempts} finished ${stats.attempts === 1 ? "attempt" : "attempts"}`}
        />
        <StatTile
          icon="clock"
          label="Median solve time"
          value={fmtMinutes(stats.medianMinutes)}
          sub={stats.hintsPerSolve === null ? "Target ≈ 10 min" : `${stats.hintsPerSolve.toFixed(1)} hints per solve`}
        />
      </div>

      <div className="mt-5 grid gap-5 lg:grid-cols-3">
        <Panel title="Skill rating" subtitle="Moves after each first-time attempt" className="lg:col-span-2">
          <RatingChart history={state.ratingHistory} />
        </Panel>
        <Panel title="Level progress" subtitle="Challenges solved per level">
          <LevelProgress levels={state.levels} current={level.level} />
        </Panel>
      </div>

      <div className="mt-5 grid gap-5 lg:grid-cols-3">
        <Panel title="Activity" subtitle="Challenges solved per day, last 20 weeks" className="lg:col-span-2">
          <Heatmap days={state.heatmap} />
        </Panel>
        <Panel title="Topic mastery" subtitle="Average score per concept">
          <TopicMastery topics={state.topics} />
        </Panel>
      </div>

      <Panel title="Recent attempts" className="mt-5">
        <RecentTable recent={state.recent} />
      </Panel>
    </div>
  );
}

function Panel({ title, subtitle, children, className = "" }: { title: string; subtitle?: string; children: ReactNode; className?: string }) {
  return (
    <section className={`card p-5 ${className}`}>
      <div className="mb-4">
        <h2 className="text-[15px] font-semibold">{title}</h2>
        {subtitle && <p className="text-xs text-muted">{subtitle}</p>}
      </div>
      {children}
    </section>
  );
}

function StatTile({ icon, iconColor, label, value, sub }: { icon: IconName; iconColor?: string; label: string; value: string; sub: string }) {
  return (
    <div className="card p-4">
      <div className="flex items-center gap-2 text-xs font-medium text-muted">
        <Icon name={icon} size={14} style={{ color: iconColor }} />
        {label}
      </div>
      <div className="mt-2 text-2xl font-semibold tracking-tight tabular">{value}</div>
      <div className="mt-0.5 text-xs text-muted">{sub}</div>
    </div>
  );
}

function DailyCard({ state }: { state: Dashboard }) {
  const { daily, next } = state;
  const navigate = useNavigate();
  const voice = useTessVoice();
  const today = new Date().toLocaleDateString("sv-SE");
  const [dismissed, setDismissed] = useState(() => read(`tess:greet-dismissed:${today}`) === "1");
  const line = tessGreeting(state);
  const showBubble = voice !== "off" && !dismissed;
  if (!daily)
    return (
      <div className="card p-6 lg:col-span-2">
        <p className="text-ink-2">No challenges found. Add some to the challenges/ folder.</p>
      </div>
    );
  const done = daily.status === "solved" || daily.status === "gave-up";
  const odds = Math.round(daily.expected * 100);

  return (
    <section className="card relative overflow-hidden p-6 lg:col-span-2">
      <div
        className="pointer-events-none absolute -top-24 -right-24 size-72 rounded-full opacity-60 blur-3xl"
        style={{ background: "radial-gradient(circle, color-mix(in oklab, var(--accent) 30%, transparent), transparent 70%)" }}
      />
      <div className="absolute right-5 bottom-2 hidden flex-col items-end sm:flex">
        {showBubble && (
          <div className="pop-in relative mr-6 mb-1 max-w-[240px] rounded-2xl rounded-br-sm border border-line bg-surface-2 py-2 pr-7 pl-3 text-[13px] leading-snug text-ink-2 shadow-sm">
            {line.text}
            <button
              onClick={() => {
                write(`tess:greet-dismissed:${today}`, "1");
                setDismissed(true);
              }}
              className="absolute top-1.5 right-1.5 rounded p-0.5 text-muted hover:text-ink"
              aria-label="Hide Tess's message until tomorrow"
              title="Hide until tomorrow"
            >
              <Icon name="x" size={12} />
            </button>
          </div>
        )}
        <Mascot mood={voice === "off" ? (TESS_MOOD[daily.status] ?? "wave") : line.mood} size={140} animated={voice === "chatty"} className="pointer-events-none text-ink-2" />
      </div>
      <div className="relative flex h-full flex-col sm:pr-36">
        <div className="flex items-center gap-2 text-xs font-semibold tracking-wider text-accent uppercase">
          <Icon name="zap" size={14} /> Today's challenge
        </div>
        <h2 className={`mt-3 text-2xl font-semibold tracking-tight ${showBubble ? "sm:pr-36" : ""}`}>{daily.title}</h2>
        {voice !== "off" && <p className="mt-1 text-sm text-ink-2 sm:hidden">{line.text}</p>}
        <div className={`mt-3 flex flex-wrap items-center gap-2 ${showBubble ? "sm:pr-36" : ""}`}>
          <LevelBadge level={daily.level} name={daily.levelName} />
          {daily.mode === "types" && <Tag>type-level</Tag>}
          {daily.topics.map((t) => (
            <Tag key={t}>{t}</Tag>
          ))}
        </div>

        <div className={`mt-6 flex flex-wrap items-center gap-x-6 gap-y-2 text-sm text-ink-2 ${showBubble ? "sm:pr-36" : ""}`}>
          <span className="inline-flex items-center gap-1.5">
            <Icon name="clock" size={15} /> ~{daily.estMinutes} min
          </span>
          <span className="inline-flex items-center gap-1.5" title="Chance of solving cleanly, based on your rating vs. the challenge's">
            <Icon name="target" size={15} /> {odds}% match for you
          </span>
          <StatusDot status={daily.status} />
        </div>

        <div className="mt-auto flex flex-wrap items-center gap-3 pt-6">
          {!done ? (
            <button className="btn btn-primary h-10 px-5" onClick={() => navigate(`/solve/${daily.id}`)}>
              <Icon name="play" size={14} /> {daily.status === "in-progress" ? "Continue" : "Start challenge"}
            </button>
          ) : (
            <>
              <div className="inline-flex items-center gap-2 rounded-lg bg-good-soft px-3 py-2 text-sm font-medium text-good">
                <Icon name="check" size={15} /> {daily.status === "solved" ? "Done for today" : "Attempted today"}
              </div>
              {next && (
                <button className="btn btn-primary h-10" onClick={() => navigate(`/solve/${next.id}`)}>
                  Bonus: {next.title} <Icon name="chevron" size={14} />
                </button>
              )}
              <button className="btn btn-ghost h-10" onClick={() => navigate(`/solve/${daily.id}`)}>
                Review
              </button>
            </>
          )}
        </div>
      </div>
    </section>
  );
}

const TESS_MOOD: Record<string, Mood> = { "not-started": "wave", "in-progress": "think", solved: "happy", "gave-up": "cheer" };

function RatingCard({ stats, level, lang }: { stats: Dashboard["stats"]; level: Dashboard["level"]; lang: Dashboard["language"] }) {
  const d = stats.ratingDelta7d;
  const voice = useTessVoice();
  // Once per level: Tess congratulates you on the new level. The starting level doesn't count.
  const [newLevel, setNewLevel] = useState(() => {
    const key = `tess:level-seen:${lang}`;
    const seen = Number(read(key) ?? (lang === "typescript" ? read("tess:level-seen") : null) ?? 0);
    if (!seen) {
      write(key, String(level.level));
      return false;
    }
    return level.level > seen;
  });
  return (
    <section className="card flex flex-col p-6">
      <div className="text-xs font-medium text-muted">{LANGUAGES[lang].name} skill rating</div>
      <div className="mt-2 flex items-baseline gap-3">
        <span className="text-5xl font-semibold tracking-tight tabular">{stats.rating.toLocaleString()}</span>
        {d !== 0 && (
          <span className="text-sm font-semibold" style={{ color: d > 0 ? "var(--good)" : "var(--bad)" }}>
            {d > 0 ? "▲" : "▼"} {Math.abs(d)} <span className="font-normal text-muted">7d</span>
          </span>
        )}
      </div>
      <div className="mt-auto pt-6">
        <div className="flex items-baseline justify-between text-sm">
          <span className="font-semibold">
            Level {level.level} <span className="font-normal text-ink-2">· {level.name}</span>
          </span>
          <span className="text-xs tabular text-muted">{level.level < 6 ? `L${level.level + 1} at ${level.nextAt}` : "Max level"}</span>
        </div>
        <div className="mt-2 h-2 rounded-full bg-accent-soft">
          <div className="h-2 rounded-full bg-accent transition-[width] duration-700" style={{ width: `${Math.max(3, level.progress * 100)}%` }} />
        </div>
        {newLevel && voice !== "off" && (
          <div className="pop-in mt-3 flex items-center gap-2 rounded-xl bg-accent-soft p-2 pr-3 text-[13px] text-ink">
            <Mascot mood="cheer" size={40} className="shrink-0" />
            <span className="flex-1">New level: {level.name}. Nice work.</span>
            <button
              className="text-muted hover:text-ink"
              aria-label="Dismiss"
              onClick={() => {
                write(`tess:level-seen:${lang}`, String(level.level));
                setNewLevel(false);
              }}
            >
              <Icon name="x" size={12} />
            </button>
          </div>
        )}
        <p className="mt-3 text-xs leading-relaxed text-muted">
          Challenges are picked just above this number. Clean, fast solves push it up; hints and giving up pull it back.
        </p>
      </div>
    </section>
  );
}

function RecentTable({ recent }: { recent: Dashboard["recent"] }) {
  if (recent.length === 0)
    return (
      <div className="flex justify-center py-2">
        <TessSays mood="wave" size={72}>
          Nothing here yet. Today's challenge is waiting, and I'll keep score.
        </TessSays>
      </div>
    );
  return (
    <div className="-mx-2 overflow-x-auto">
      <table className="w-full min-w-[640px] text-sm">
        <thead>
          <tr className="text-left text-xs text-muted">
            <th className="px-2 pb-2 font-medium">Challenge</th>
            <th className="px-2 pb-2 font-medium">Result</th>
            <th className="px-2 pb-2 text-right font-medium">Time</th>
            <th className="px-2 pb-2 text-right font-medium">Hints</th>
            <th className="px-2 pb-2 text-right font-medium">Rating</th>
            <th className="px-2 pb-2 text-right font-medium">When</th>
          </tr>
        </thead>
        <tbody>
          {recent.map((r) => (
            <tr key={r.id} className="border-t border-line">
              <td className="px-2 py-2.5">
                <Link to={`/solve/${r.challengeId}`} className="flex items-center gap-2 hover:text-accent">
                  <LevelBadge level={r.level} />
                  <span className="font-medium">{r.title}</span>
                  {r.isDaily && <Tag>daily</Tag>}
                </Link>
              </td>
              <td className="px-2 py-2.5">
                <StatusDot status={r.status} />
              </td>
              <td className="px-2 py-2.5 text-right tabular text-ink-2">{fmtMinutes(r.minutes)}</td>
              <td className="px-2 py-2.5 text-right tabular text-ink-2">{r.hintsUsed}</td>
              <td className="px-2 py-2.5 text-right font-medium tabular">
                {r.ratingChange === null ? (
                  <span className="text-muted">practice</span>
                ) : (
                  <span style={{ color: r.ratingChange >= 0 ? "var(--good)" : "var(--bad)" }}>
                    {r.ratingChange >= 0 ? "+" : ""}
                    {r.ratingChange}
                  </span>
                )}
              </td>
              <td className="px-2 py-2.5 text-right text-ink-2">{relTime(r.finishedAt)}</td>
            </tr>
          ))}
        </tbody>
      </table>
    </div>
  );
}
