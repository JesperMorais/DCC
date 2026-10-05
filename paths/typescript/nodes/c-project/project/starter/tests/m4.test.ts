import { test } from "node:test";
import assert from "node:assert/strict";
import { cli, fixture } from "./helpers.ts";

test("top n lists the biggest expenses with their category", () => {
  const r = cli("top", fixture("clean.csv"), "3", "--rules", fixture("rules.txt"));
  assert.equal(r.code, 0);
  assert.deepEqual(r.out, [
    "1. 2026-09-01  8500.00  Rent September (rent)",
    "2. 2026-10-01  8500.00  Rent October (rent)",
    "3. 2026-09-02  970.00  SL access card (transport)",
  ]);
});

test("--month limits top to one month", () => {
  const r = cli("top", fixture("clean.csv"), "2", "--rules", fixture("rules.txt"), "--month", "2026-09");
  assert.deepEqual(r.out, [
    "1. 2026-09-01  8500.00  Rent September (rent)",
    "2. 2026-09-02  970.00  SL access card (transport)",
  ]);
});

test("asking for more than there are lists them all", () => {
  const r = cli("top", fixture("clean.csv"), "50", "--month", "2026-08");
  assert.equal(r.code, 0);
  assert.deepEqual(r.out, ["1. 2026-08-28  412.30  ICA Kvantum (other)"]);
});

test("equal amounts keep date order, then file order", () => {
  const r = cli("top", fixture("ties.csv"), "4", "--rules", fixture("ties-rules.txt"));
  assert.deepEqual(r.out, [
    "1. 2026-09-01  40.00  Bus ticket (travel)",
    "2. 2026-09-02  40.00  Book shop (books)",
    "3. 2026-09-03  40.00  Cinema (fun)",
    "4. 2026-09-03  40.00  Arcade (fun)",
  ]);
});

test("n must be a positive whole number", () => {
  for (const n of ["0", "-2", "2.5", "lots"]) {
    const r = cli("top", fixture("clean.csv"), n);
    assert.equal(r.code, 1, n);
    assert.deepEqual(r.err, ["error: n must be a positive whole number"], n);
    assert.deepEqual(r.out, [], n);
  }
});

test("a malformed --month is an error", () => {
  const r = cli("top", fixture("clean.csv"), "3", "--month", "Sept");
  assert.equal(r.code, 1);
  assert.deepEqual(r.err, ["error: month must be YYYY-MM"]);
});
