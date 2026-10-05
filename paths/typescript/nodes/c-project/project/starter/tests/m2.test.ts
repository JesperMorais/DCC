import { test } from "node:test";
import assert from "node:assert/strict";
import { cli, fixture } from "./helpers.ts";

test("bad rows are warned about on stderr, in file order", () => {
  const r = cli("list", fixture("messy.csv"));
  assert.equal(r.code, 0);
  assert.deepEqual(r.err, [
    "warning: line 5: expected 3 fields, got 4",
    'warning: line 6: bad date "2026-9-05"',
    'warning: line 7: bad amount "abc"',
    "warning: line 9: expected 3 fields, got 2",
    'warning: line 10: bad date "2026-13-01"',
    "warning: line 11: empty description",
    'warning: line 12: bad amount "-12.345"',
  ]);
});

test("good rows are trimmed and still listed, with the skipped count", () => {
  const r = cli("list", fixture("messy.csv"));
  assert.deepEqual(r.out, [
    "2026-09-01  -8500.00  Rent September",
    "2026-09-03  -89.90  ICA Nara",
    "2026-09-12  -189.00  Pizzeria Napoli",
    "2026-09-20  300.00  Swish from Anna",
    "4 transactions (7 skipped)",
  ]);
});

test("a clean file has no warnings and no skipped count", () => {
  const r = cli("list", fixture("clean.csv"));
  assert.deepEqual(r.err, []);
  assert.equal(r.out.at(-1), "13 transactions");
});

test("a missing file is an error, exit 1", () => {
  const r = cli("list", fixture("nope.csv"));
  assert.equal(r.code, 1);
  assert.deepEqual(r.err, ["error: cannot read tests/fixtures/nope.csv"]);
  assert.deepEqual(r.out, []);
});
