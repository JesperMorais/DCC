// An example of the test syntax. Delete it once you have tests of your own.
// Name your files tests/<anything>.test.ts and `npm test` picks them up.
import { test } from "node:test";
import assert from "node:assert/strict";

function add(a: number, b: number): number {
  return a + b;
}

test("add sums two numbers", () => {
  assert.equal(add(2, 3), 5); // ===
  assert.deepEqual({ total: add(1, 1) }, { total: 2 }); // compares structure, not identity
  assert.throws(() => JSON.parse("{"), SyntaxError); // the call must throw this kind of error
});

// Type-level checks: these never run, `npm run check` (tsc) verifies them.
// `@ts-expect-error` says "the next line MUST be a compile error". If it compiles fine,
// tsc reports the directive as unused, so the check fails. That is how you test types.
function typeChecks() {
  // @ts-expect-error a string is not a number
  add("2", 3);
}
void typeChecks; // never called; it only exists for the type checker
