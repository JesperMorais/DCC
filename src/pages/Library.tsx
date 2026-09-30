import { useEffect, useMemo, useState } from "react";
import { Link } from "react-router-dom";
import { api, type Library } from "../api";
import { LangBadge, LANGUAGES, useActiveLang } from "../lang";
import { TessLoading, TessSays } from "../components/Mascot";
import { Icon, LevelPips, StatusDot, Tag } from "../ui";

type Filter = "all" | "new" | "solved" | "open";

export default function LibraryPage() {
  const [lib, setLib] = useState<Library | null>(null);
  const [filter, setFilter] = useState<Filter>("all");
  const [q, setQ] = useState("");

  const lang = useActiveLang();
  useEffect(() => {
    setLib(null);
    api.library(lang).then(setLib);
  }, [lang]);

  const visible = useMemo(() => {
    if (!lib) return [];
    const query = q.trim().toLowerCase();
    return lib.challenges.filter((c) => {
      if (filter === "new" && c.status !== "new") return false;
      if (filter === "solved" && c.status !== "solved") return false;
      if (filter === "open" && !(c.status === "in-progress" || c.status === "gave-up")) return false;
      return !query || c.title.toLowerCase().includes(query) || c.topics.some((t) => t.toLowerCase().includes(query));
    });
  }, [lib, filter, q]);

  if (!lib) return <TessLoading label="Fetching the library…" />;

  return (
    <div className="mx-auto max-w-[1240px] px-4 py-8 sm:px-8">
      <header className="mb-6">
        <h1 className="flex items-center gap-3 text-[28px] font-semibold tracking-tight">
          <LangBadge lang={lang} size={30} /> {LANGUAGES[lang].name} library
        </h1>
        <p className="mt-1 text-sm text-ink-2">
          Every challenge, grouped by level. The daily pick comes from here automatically — but you can practise anything, any time.
        </p>
      </header>

      <div className="mb-6 flex flex-wrap items-center gap-3">
        <div className="flex rounded-lg border border-line bg-surface p-0.5">
          {(["all", "new", "open", "solved"] as const).map((f) => (
            <button
              key={f}
              onClick={() => setFilter(f)}
              className={`rounded-md px-3 py-1.5 text-xs font-medium capitalize ${filter === f ? "bg-surface-2 text-ink" : "text-muted hover:text-ink-2"}`}
            >
              {f === "open" ? "In progress / retry" : f}
            </button>
          ))}
        </div>
        <input
          value={q}
          onChange={(e) => setQ(e.target.value)}
          placeholder="Search title or topic…"
          className="h-9 w-64 rounded-lg border border-line bg-surface px-3 text-sm outline-none focus:border-accent"
        />
      </div>

      <div className="flex flex-col gap-8">
        {lib.levels.map((lvl) => {
          const items = visible.filter((c) => c.level === lvl.level);
          if (items.length === 0) return null;
          const here = lib.rating !== null && lib.rating >= lvl.band[0] && lib.rating < lvl.band[1];
          return (
            <section key={lvl.level}>
              <div className="mb-3 flex items-center gap-3">
                <LevelPips level={lvl.level} />
                <h2 className="text-[15px] font-semibold">
                  Level {lvl.level} · {lvl.name}
                </h2>
                <span className="text-xs tabular text-muted">
                  {lvl.band[0]}–{lvl.band[1]}
                </span>
                {here && <span className="rounded-md bg-accent-soft px-1.5 py-0.5 text-[11px] font-semibold text-accent-strong">You are here</span>}
              </div>
              <div className="grid gap-3 sm:grid-cols-2 xl:grid-cols-3">
                {items.map((c) => (
                  <Link key={c.id} to={`/solve/${c.id}`} className="card group flex flex-col p-4 transition-colors hover:border-line-strong">
                    <div className="flex items-start justify-between gap-2">
                      <h3 className="font-medium group-hover:text-accent">{c.title}</h3>
                      <span className="shrink-0 text-xs tabular text-muted">{c.rating}</span>
                    </div>
                    <div className="mt-2 flex flex-wrap gap-1">
                      {c.mode === "types" && <Tag>type-level</Tag>}
                      {c.topics.slice(0, 3).map((t) => (
                        <Tag key={t}>{t}</Tag>
                      ))}
                    </div>
                    <div className="mt-auto flex items-center justify-between pt-4">
                      <StatusDot status={c.status} />
                      <span className="inline-flex items-center gap-1 text-xs text-muted">
                        <Icon name="clock" size={12} /> ~{c.estMinutes} min
                      </span>
                    </div>
                  </Link>
                ))}
              </div>
            </section>
          );
        })}
        {visible.length === 0 && (
          <div className="flex justify-center py-10">
            <TessSays mood="think" size={72}>
              Nothing by that name. Try a topic instead, like <i>generics</i>.
            </TessSays>
          </div>
        )}
      </div>
    </div>
  );
}
