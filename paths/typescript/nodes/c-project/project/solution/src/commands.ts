import { money, reportLine } from "./format.ts";
import { categorise } from "./rules.ts";
import type { Rule, Transaction } from "./types.ts";

const plural = (n: number, word: string) => `${n} ${word}${n === 1 ? "" : "s"}`;
const isExpense = (t: Transaction) => t.cents < 0;
const inMonth = (month: string) => (t: Transaction) => t.date.startsWith(`${month}-`);

export function list(txs: readonly Transaction[], skipped: number): string[] {
  const lines = txs.map((t) => `${t.date}  ${money(t.cents)}  ${t.description}`);
  return [...lines, plural(txs.length, "transaction") + (skipped > 0 ? ` (${skipped} skipped)` : "")];
}

export function report(txs: readonly Transaction[], month: string, rules: readonly Rule[]): string[] {
  const spent = txs.filter(inMonth(month)).filter(isExpense);
  if (spent.length === 0) return [`no spending in ${month}`];
  const totals = new Map<string, number>();
  for (const t of spent) {
    const category = categorise(t.description, rules);
    totals.set(category, (totals.get(category) ?? 0) - t.cents);
  }
  const sorted = [...totals].sort(([a, x], [b, y]) => y - x || a.localeCompare(b));
  const total = sorted.reduce((sum, [, cents]) => sum + cents, 0);
  return [`Spending in ${month}`, ...sorted.map(([name, cents]) => reportLine(name, cents)), reportLine("TOTAL", total)];
}

export function top(txs: readonly Transaction[], n: number, rules: readonly Rule[], month?: string): string[] {
  const pool = month === undefined ? txs : txs.filter(inMonth(month));
  return pool
    .filter(isExpense)
    .sort((a, b) => a.cents - b.cents || a.date.localeCompare(b.date) || a.line - b.line)
    .slice(0, n)
    .map((t, i) => `${i + 1}. ${t.date}  ${money(-t.cents)}  ${t.description} (${categorise(t.description, rules)})`);
}

export function months(txs: readonly Transaction[]): string[] {
  const byMonth = new Map<string, { out: number; in: number }>();
  for (const t of txs) {
    const key = t.date.slice(0, 7);
    const m = byMonth.get(key) ?? { out: 0, in: 0 };
    if (isExpense(t)) m.out -= t.cents;
    else m.in += t.cents;
    byMonth.set(key, m);
  }
  return [...byMonth]
    .sort(([a], [b]) => a.localeCompare(b))
    .map(([key, m]) => `${key}  out ${money(m.out)}  in ${money(m.in)}  net ${money(m.in - m.out)}`);
}
