import type { Row } from "./types.ts";

/**
 * A suggested first step. Keep it, rename it or reshape it: it's yours.
 *
 * One data line of the export and its 1-based line number in the file → a Row.
 * Milestone 1 only needs the good case (a Transaction). Milestone 2 adds the BadRow
 * cases, and `null` for a blank line that isn't a row at all.
 *
 * tests/mine.test.ts has a unit test for it: `npm run test:mine`.
 */
export function parseRow(text: string, line: number): Row | null {
  throw new Error("not implemented");
}
