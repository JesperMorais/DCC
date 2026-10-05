import { test } from "node:test";
import assert from "node:assert/strict";
import { cli, fixture } from "./helpers.ts";

test("months prints out, in and net per month, oldest first", () => {
  const r = cli("months", fixture("clean.csv"));
  assert.equal(r.code, 0);
  assert.deepEqual(r.out, [
    "2026-08  out 412.30  in 25000.00  net 24587.70",
    "2026-09  out 11059.55  in 300.00  net -10759.55",
    "2026-10  out 8500.00  in 0.00  net -8500.00",
  ]);
});

test("months skips bad rows with the usual warnings", () => {
  const r = cli("months", fixture("messy.csv"));
  assert.equal(r.code, 0);
  assert.equal(r.err.length, 7);
  assert.deepEqual(r.out, ["2026-09  out 8778.90  in 300.00  net -8478.90"]);
});

test("flags can come before the positional arguments", () => {
  const r = cli("top", "--month", "2026-09", "--rules", fixture("rules.txt"), fixture("clean.csv"), "1");
  assert.equal(r.code, 0);
  assert.deepEqual(r.out, ["1. 2026-09-01  8500.00  Rent September (rent)"]);
});

test("an unknown flag is an error", () => {
  const r = cli("report", fixture("clean.csv"), "2026-09", "--colour", "red");
  assert.equal(r.code, 1);
  assert.deepEqual(r.err, ["error: unknown flag --colour"]);
  assert.deepEqual(r.out, []);
});

test("a flag without a value is an error", () => {
  for (const args of [["--rules"], ["--rules", "--month", "2026-09"]]) {
    const r = cli("report", fixture("clean.csv"), "2026-09", ...args);
    assert.equal(r.code, 1, args.join(" "));
    assert.deepEqual(r.err, ["error: --rules needs a value"], args.join(" "));
  }
});
