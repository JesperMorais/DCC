// An example of the test syntax. Delete it once you have tests of your own.
// Name your files tests/m<number>-<anything>.test.ts (e.g. tests/m1-store.test.ts):
// `npm test` picks up every one, and `npm run test:m1` runs only the m1 files.
import { test } from "node:test";
import assert from "node:assert/strict";
import type { Equal, Expect } from "./type-helpers.ts";

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
// It accepts ANY error, though: write the line without the directive first, read tsc's
// message, and only add the directive once it's the error you meant.
function typeChecks() {
  const sum = add(1, 2); // a positive line: proves add isn't simply rejecting everything
  type _cases = [Expect<Equal<typeof sum, number>>];

  // @ts-expect-error a string is not a number
  add("2", 3);
}
// The function is never called, so it never runs. Mentioning it here only stops the editor
// from flagging it as unused; `void` evaluates an expression and throws the result away.
void typeChecks;
