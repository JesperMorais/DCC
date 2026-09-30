import { useLocation } from "react-router-dom";
import { isLang, LANGUAGES, type Lang } from "../shared/languages";

export { LANGUAGES, isLang, type Lang };

/** The language the user is looking at: from the URL, else the last one they used. */
export function useActiveLang(): Lang {
  const { pathname } = useLocation();
  const parts = pathname.split("/").filter(Boolean);
  const fromUrl = parts[0] === "solve" ? parts[1] : parts[0];
  if (isLang(fromUrl)) {
    try {
      localStorage.setItem("lang", fromUrl);
    } catch {}
    return fromUrl;
  }
  let last: string | undefined;
  try {
    last = localStorage.getItem("lang") ?? undefined;
  } catch {}
  return isLang(last) ? last : "typescript";
}

export function LangBadge({ lang, size = 22 }: { lang: Lang; size?: number }) {
  const l = LANGUAGES[lang];
  return (
    <span
      className="inline-grid shrink-0 place-items-center rounded-md font-mono font-bold"
      style={{ width: size, height: size, background: l.badge.bg, color: l.badge.fg, fontSize: size * (l.short.length > 1 ? 0.42 : 0.5) }}
      aria-hidden
    >
      {l.short}
    </span>
  );
}
