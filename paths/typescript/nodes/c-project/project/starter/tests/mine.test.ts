// Your own unit tests. The milestone tests run the whole CLI and only see what it prints;
// these call one function directly, so when something's wrong you see exactly which step.
// Run them with `npm run test:mine` (they're part of `npm test` too).
//
// The first test is ours, as an example. It tests the parseRow suggested in src/csv.ts.
// If you rename or reshape that function, change the test to match. The rest are yours:
// one test per rule is a good habit (a bad date, an empty description, a blank line...).
import { test } from "node:test";
import assert from "node:assert/strict";
import { parseRow } from "../src/csv.ts";

test("a good line becomes a Transaction in whole cents", () => {
  // arrange: one data line as it appears in the file, and its line number
  const text = "2026-09-03,ICA Nara,-89.90";
  // act
  const row = parseRow(text, 5);
  // assert: deepEqual compares field by field
  assert.deepEqual(row, { line: 5, date: "2026-09-03", description: "ICA Nara", cents: -8990 });
});

// Milestone 2: replace each todo with a real test.
test.todo("a line with a bad amount becomes a BadRow with its reason");
test.todo("a blank line is not a row at all");
