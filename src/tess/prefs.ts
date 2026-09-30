// Tess preferences + small per-attempt memory, all in localStorage (per machine, best effort).
import { useSyncExternalStore } from "react";

export type TessVoice = "chatty" | "quiet" | "off";

const listeners = new Set<() => void>();
const notify = () => listeners.forEach((l) => l());
const subscribe = (l: () => void) => {
  listeners.add(l);
  return () => listeners.delete(l);
};

export function read(key: string): string | null {
  try {
    return localStorage.getItem(key);
  } catch {
    return null;
  }
}
export function write(key: string, value: string | null) {
  try {
    if (value === null) localStorage.removeItem(key);
    else localStorage.setItem(key, value);
  } catch {}
  notify();
}

const voice = (): TessVoice => {
  const v = read("tess:voice");
  return v === "quiet" || v === "off" ? v : "chatty";
};
export const setTessVoice = (v: TessVoice) => write("tess:voice", v === "chatty" ? null : v);
export const useTessVoice = () => useSyncExternalStore(subscribe, voice);

/* Concept tips: stop after 3 total dismissals, or when turned off in Settings. */
const conceptTipsOn = () => read("tess:concept-tips") !== "off" && Number(read("tess:concept-dismissals") ?? 0) < 3;
export const useConceptTips = () => useSyncExternalStore(subscribe, conceptTipsOn);
export const setConceptTips = (on: boolean) => {
  write("tess:concept-tips", on ? null : "off");
  if (on) write("tess:concept-dismissals", null);
};
export function dismissConceptTip(attemptId: string) {
  write(`concept-muted:${attemptId}`, "1");
  write("tess:concept-dismissals", String(Number(read("tess:concept-dismissals") ?? 0) + 1));
}

/* Per attempt: did they open the Concept tab, how many nudges shown, hint intercept used. */
export const conceptOpened = (attemptId: string) => read(`concept:${attemptId}`) === "1";
export const markConceptOpened = (attemptId: string) => write(`concept:${attemptId}`, "1");
export const conceptMuted = (attemptId: string) => read(`concept-muted:${attemptId}`) === "1";
export const nudgeCount = (attemptId: string) => Number(read(`nudges:${attemptId}`) ?? 0);
export const bumpNudge = (attemptId: string) => write(`nudges:${attemptId}`, String(nudgeCount(attemptId) + 1));

export const reducedMotion = () => {
  try {
    return window.matchMedia("(prefers-reduced-motion: reduce)").matches;
  } catch {
    return false;
  }
};
