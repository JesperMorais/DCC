import type { BadRow, Row, Transaction } from "./types.ts";

const DATE = /^(\d{4})-(\d{2})-(\d{2})$/;
const AMOUNT = /^[+-]?\d+(\.\d{1,2})?$/;

function validDate(text: string): boolean {
  const m = DATE.exec(text);
  if (!m) return false;
  const month = Number(m[2]);
  const day = Number(m[3]);
  return month >= 1 && month <= 12 && day >= 1 && day <= 31;
}

/** One data line → a transaction or the reason it's unreadable. `null` for a blank line. */
export function parseRow(text: string, line: number): Row | null {
  if (text.trim() === "") return null;
  const fields = text.split(",").map((f) => f.trim());
  if (fields.length !== 3) return { line, reason: `expected 3 fields, got ${fields.length}` };
  const [date, description, amount] = fields as [string, string, string];
  if (!validDate(date)) return { line, reason: `bad date "${date}"` };
  if (description === "") return { line, reason: "empty description" };
  if (!AMOUNT.test(amount)) return { line, reason: `bad amount "${amount}"` };
  return { line, date, description, cents: Math.round(Number(amount) * 100) };
}

/** Parses a whole export. The first line is the header and is always skipped. */
export function parseCsv(text: string): { transactions: Transaction[]; bad: BadRow[] } {
  const rows = text
    .split(/\r?\n/)
    .map((t, i) => (i === 0 ? null : parseRow(t, i + 1)))
    .filter((r): r is Row => r !== null);
  return {
    transactions: rows.filter((r): r is Transaction => !("reason" in r)),
    bad: rows.filter((r): r is BadRow => "reason" in r),
  };
}
