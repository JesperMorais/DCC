import type { ReactNode } from "react";
import { Link, useNavigate } from "react-router-dom";
import type { Dashboard } from "../api";
import { Heatmap, LevelProgress, RatingChart, TopicMastery } from "../components/charts";
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
          <p className="text-sm text-muted">{new Date().toLocaleDateString(undefined, { weekday: "long", month: "long", day: "numeric" })}</p>
          <h1 className="mt-1 text-[28px] font-semibold tracking-tight">
            {greeting()}, {name}
          </h1>
        </div>
        <Link to="/library" className="btn">
          <Icon name="library" /> Browse all challenges
        </Link>
      </header>

      <div className="grid gap-5 lg:grid-cols-3">
        <DailyCard daily={daily} next={next} />
        <RatingCard stats={stats} level={level} />
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

function DailyCard({ daily, next }: { daily: Dashboard["daily"]; next: Dashboard["next"] }) {
  const navigate = useNavigate();
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
      <div className="relative flex h-full flex-col">
        <div className="flex items-center gap-2 text-xs font-semibold tracking-wider text-accent uppercase">
          <Icon name="zap" size={14} /> Today's challenge
        </div>
        <h2 className="mt-3 text-2xl font-semibold tracking-tight">{daily.title}</h2>
        <div className="mt-3 flex flex-wrap items-center gap-2">
          <LevelBadge level={daily.level} withName />
          {daily.mode === "types" && <Tag>type-level</Tag>}
          {daily.topics.map((t) => (
            <Tag key={t}>{t}</Tag>
          ))}
        </div>

        <div className="mt-6 flex flex-wrap items-center gap-x-6 gap-y-2 text-sm text-ink-2">
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
            <button className="btn btn-primary h-10 px-5" onClick={() => navigate(`/c/${daily.id}`)}>
              <Icon name="play" size={14} /> {daily.status === "in-progress" ? "Continue" : "Start challenge"}
            </button>
          ) : (
            <>
              <div className="inline-flex items-center gap-2 rounded-lg bg-good-soft px-3 py-2 text-sm font-medium text-good">
                <Icon name="check" size={15} /> {daily.status === "solved" ? "Done for today" : "Attempted today"}
              </div>
              {next && (
                <button className="btn btn-primary h-10" onClick={() => navigate(`/c/${next.id}`)}>
                  Bonus: {next.title} <Icon name="chevron" size={14} />
                </button>
              )}
              <button className="btn btn-ghost h-10" onClick={() => navigate(`/c/${daily.id}`)}>
                Review
              </button>
            </>
          )}
        </div>
      </div>
    </section>
  );
}

function RatingCard({ stats, level }: { stats: Dashboard["stats"]; level: Dashboard["level"] }) {
  const d = stats.ratingDelta7d;
  return (
    <section className="card flex flex-col p-6">
      <div className="text-xs font-medium text-muted">Skill rating</div>
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
        <p className="mt-3 text-xs leading-relaxed text-muted">
          Challenges are picked just above this number. Clean, fast solves push it up; hints and giving up pull it back.
        </p>
      </div>
    </section>
  );
}

function RecentTable({ recent }: { recent: Dashboard["recent"] }) {
  if (recent.length === 0) return <p className="py-4 text-center text-sm text-muted">Nothing yet — today's challenge is waiting.</p>;
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
                <Link to={`/c/${r.challengeId}`} className="flex items-center gap-2 hover:text-accent">
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
