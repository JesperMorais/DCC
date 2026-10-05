import { test } from "node:test";
import assert from "node:assert/strict";
import { cli, fixture } from "./helpers.ts";

test("list prints every transaction and a count", () => {
  const r = cli("list", fixture("clean.csv"));
  assert.equal(r.code, 0);
  assert.deepEqual(r.out, [
    "2026-08-28  -412.30  ICA Kvantum",
    "2026-08-30  25000.00  Salary August",
    "2026-09-01  -8500.00  Rent September",
    "2026-09-02  -970.00  SL access card",
    "2026-09-03  -89.90  ICA Nara",
    "2026-09-05  -119.00  Spotify",
    "2026-09-07  -245.50  Coop Konsum",
    "2026-09-12  -189.00  Pizzeria Napoli",
    "2026-09-15  -602.15  Willys",
    "2026-09-18  -210.00  Taxi to ICA Maxi",
    "2026-09-20  300.00  Swish from Anna",
    "2026-09-25  -134.00  Max Burgers",
    "2026-10-01  -8500.00  Rent October",
    "13 transactions",
  ]);
  assert.deepEqual(r.err, []);
});

test("one transaction is singular", () => {
  const r = cli("list", fixture("single.csv"));
  assert.equal(r.code, 0);
  assert.deepEqual(r.out, ["2026-09-03  -42.00  Bakery", "1 transaction"]);
});

test("no arguments prints usage and exits 1", () => {
  const r = cli();
  assert.equal(r.code, 1);
  assert.match(r.err[0] ?? "", /^usage: /);
  assert.deepEqual(r.out, []);
});

test("an unknown command prints usage and exits 1", () => {
  const r = cli("sum", fixture("clean.csv"));
  assert.equal(r.code, 1);
  assert.match(r.err[0] ?? "", /^usage: /);
});

test("list without a file prints usage and exits 1", () => {
  const r = cli("list");
  assert.equal(r.code, 1);
  assert.match(r.err[0] ?? "", /^usage: /);
});
