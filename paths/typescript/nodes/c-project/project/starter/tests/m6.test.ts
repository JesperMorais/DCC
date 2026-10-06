import { test } from "node:test";
import assert from "node:assert/strict";
import { cli, fixture } from "./helpers.ts";

test("flags can come before the positional arguments", () => {
  const r = cli("top", "--month", "2026-09", "--rules", fixture("rules.txt"), fixture("clean.csv"), "1");
  assert.equal(r.code, 0);
  assert.deepEqual(r.out, ["1. 2026-09-01  8500.00  Rent September (rent)"]);
});

test("flags can come between the positional arguments", () => {
  const r = cli("report", fixture("clean.csv"), "--rules", fixture("rules.txt"), "2026-09");
  assert.equal(r.code, 0);
  assert.equal(r.out[1], "rent             8500.00");
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

test("a missing positional argument is still a usage error once the flags are taken out", () => {
  const r = cli("top", fixture("clean.csv"), "--month", "2026-09");
  assert.equal(r.code, 1);
  assert.match(r.err[0] ?? "", /^usage: /);
  assert.deepEqual(r.out, []);
});
