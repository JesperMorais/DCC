import type { Experience } from "../api";
import { LANGUAGES, type Lang } from "../lang";
import { Icon, LevelPips } from "../ui";

const ORDER: [Experience, number][] = [
  ["new", 1],
  ["other-lang", 2],
  ["js", 3],
  ["some-ts", 4],
  ["pro", 5],
];

export function ExperiencePicker({ lang, value, onChange }: { lang: Lang; value: Experience | null; onChange: (e: Experience) => void }) {
  const opts = LANGUAGES[lang].experience;
  return (
    <div className="grid gap-2">
      {ORDER.map(([key, level]) => (
        <button
          key={key}
          onClick={() => onChange(key)}
          className={`flex items-center gap-4 rounded-xl border p-3.5 text-left transition-colors ${
            value === key ? "border-accent bg-accent-soft" : "border-line hover:border-line-strong hover:bg-surface-2"
          }`}
        >
          <LevelPips level={level} />
          <div className="min-w-0 flex-1">
            <div className="text-sm font-semibold">{opts[key].title}</div>
            <div className="text-xs text-ink-2">{opts[key].body}</div>
          </div>
          <span className={`grid size-5 place-items-center rounded-full border ${value === key ? "border-accent bg-accent text-white" : "border-line-strong"}`}>
            {value === key && <Icon name="check" size={12} strokeWidth={3} />}
          </span>
        </button>
      ))}
    </div>
  );
}
