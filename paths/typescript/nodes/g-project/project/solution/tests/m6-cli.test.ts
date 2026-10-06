import { test } from "node:test";
import assert from "node:assert/strict";
import { execFileSync } from "node:child_process";

/** Runs the CLI with the given lines on stdin and returns everything it printed. */
function runCli(lines: string[]): string {
  return execFileSync(process.execPath, ["--import", "tsx", "src/main.ts"], { input: lines.join("\n") + "\n", encoding: "utf8" });
}

test("the CLI reacts to store events", () => {
  const out = runCli(["add apple 3", "add pear", "coupon HALF", "set apple 0", "undo", "quit"]);
  assert.match(out, /3 x apple\s+\$1\.50/);
  assert.match(out, /error: Unknown product "pear"/);
  assert.match(out, /coupon HALF applied/);
  assert.match(out, /total\s+\$0\.75/);
  assert.match(out, /error: setQty: Quantity must be/);
  assert.equal(out.trim().split("\n").at(-1)?.trim().startsWith("total"), true); // undo reprints the cart
});

test("unknown commands print an error and the program keeps going", () => {
  const out = runCli(["dance", "add bread", "quit", "add apple"]);
  assert.match(out, /error: Unknown command "dance"/);
  assert.match(out, /1 x bread/);
  assert.doesNotMatch(out, /apple/, "nothing after quit is read");
});

test("undo with nothing to undo says so", () => {
  assert.match(runCli(["undo"]), /nothing to undo/);
});
