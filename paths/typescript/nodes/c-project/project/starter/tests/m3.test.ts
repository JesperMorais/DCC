import { test } from "node:test";
import assert from "node:assert/strict";
import { cli, fixture } from "./helpers.ts";

test("report totals each category, largest first, then TOTAL", () => {
  const r = cli("report", fixture("clean.csv"), "2026-09", "--rules", fixture("rules.txt"));
  assert.equal(r.code, 0);
  assert.deepEqual(r.out, [
    "Spending in 2026-09",
    "rent             8500.00",
    "groceries        1147.55",
    "transport         970.00",
    "eating out        323.00",
    "subscriptions     119.00",
    "TOTAL           11059.55",
  ]);
  assert.deepEqual(r.err, []);
});

test("without --rules everything is 'other'", () => {
  const r = cli("report", fixture("clean.csv"), "2026-09");
  assert.deepEqual(r.out, ["Spending in 2026-09", "other           11059.55", "TOTAL           11059.55"]);
});

test("equal totals are sorted by category name", () => {
  const r = cli("report", fixture("ties.csv"), "2026-09", "--rules", fixture("ties-rules.txt"));
  assert.deepEqual(r.out, [
    "Spending in 2026-09",
    "fun                80.00",
    "books              40.00",
    "travel             40.00",
    "TOTAL             160.00",
  ]);
});

test("a month without spending says so", () => {
  const r = cli("report", fixture("clean.csv"), "2026-07");
  assert.equal(r.code, 0);
  assert.deepEqual(r.out, ["no spending in 2026-07"]);
});

test("a malformed month is an error, exit 1", () => {
  for (const month of ["2026-9", "2026-13", "09-2026"]) {
    const r = cli("report", fixture("clean.csv"), month);
    assert.equal(r.code, 1, month);
    assert.deepEqual(r.err, ["error: month must be YYYY-MM"], month);
  }
});

test("malformed rules lines are warned about and skipped", () => {
  const r = cli("report", fixture("clean.csv"), "2026-09", "--rules", fixture("rules-messy.txt"));
  assert.equal(r.code, 0);
  assert.deepEqual(r.err, [
    'warning: rules line 2: expected "category: keywords"',
    'warning: rules line 3: expected "category: keywords"',
    'warning: rules line 5: expected "category: keywords"',
  ]);
  assert.deepEqual(r.out, [
    "Spending in 2026-09",
    "other            9544.15",
    "transport         970.00",
    "groceries         545.40",
    "TOTAL           11059.55",
  ]);
});

test("an unreadable rules file is an error, exit 1", () => {
  const r = cli("report", fixture("clean.csv"), "2026-09", "--rules", fixture("nope.txt"));
  assert.equal(r.code, 1);
  assert.deepEqual(r.err, ["error: cannot read tests/fixtures/nope.txt"]);
});
