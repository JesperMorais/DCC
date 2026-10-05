import type { Rule } from "./types.ts";

export const UNCATEGORISED = "other";

/** Parses "category: kw, kw" lines. Returns the rules and the 1-based numbers of malformed lines. */
export function parseRules(text: string): [Rule[], number[]] {
  const rules: Rule[] = [];
  const bad: number[] = [];
  text.split(/\r?\n/).forEach((raw, i) => {
    const line = raw.trim();
    if (line === "" || line.startsWith("#")) return;
    const colon = line.indexOf(":");
    const category = colon === -1 ? "" : line.slice(0, colon).trim();
    const keywords = line
      .slice(colon + 1)
      .split(",")
      .map((k) => k.trim().toLowerCase())
      .filter((k) => k !== "");
    if (category === "" || keywords.length === 0) bad.push(i + 1);
    else rules.push({ category, keywords });
  });
  return [rules, bad];
}

/** The first rule (in file order) with a keyword inside the description wins. */
export function categorise(description: string, rules: readonly Rule[]): string {
  const text = description.toLowerCase();
  return rules.find((r) => r.keywords.some((k) => text.includes(k)))?.category ?? UNCATEGORISED;
}
