import { useSyncExternalStore } from "react";

export type ThemePref = "system" | "light" | "dark";

const listeners = new Set<() => void>();
const media = window.matchMedia("(prefers-color-scheme: dark)");

function readPref(): ThemePref {
  try {
    const v = localStorage.getItem("theme");
    return v === "light" || v === "dark" ? v : "system";
  } catch {
    return "system";
  }
}

let pref = readPref();
const resolved = () => (pref === "system" ? (media.matches ? "dark" : "light") : pref);
media.addEventListener("change", () => listeners.forEach((l) => l()));

export function setTheme(next: ThemePref) {
  pref = next;
  try {
    if (next === "system") localStorage.removeItem("theme");
    else localStorage.setItem("theme", next);
  } catch {}
  if (next === "system") delete document.documentElement.dataset.theme;
  else document.documentElement.dataset.theme = next;
  listeners.forEach((l) => l());
}

const subscribe = (l: () => void) => {
  listeners.add(l);
  return () => listeners.delete(l);
};

/** Returns the effective theme ("light" | "dark") and re-renders on change. */
export const useTheme = () => ({ theme: useSyncExternalStore(subscribe, resolved), pref: useSyncExternalStore(subscribe, () => pref) });
