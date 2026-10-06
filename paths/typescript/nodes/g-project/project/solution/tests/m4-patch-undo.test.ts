import { test } from "node:test";
import assert from "node:assert/strict";
import { Store } from "../src/store.ts";
import { counter, initial } from "./fixtures.ts";

test("update merges a partial patch and reports it as the cause", () => {
  const store = new Store(initial, counter);
  const causes: string[] = [];
  store.on("change", ({ cause }) => causes.push(cause.kind));
  store.update({ label: "taps" });
  assert.deepEqual(store.state, { count: 0, step: 1, label: "taps" });
  assert.deepEqual(causes, ["patch"]);
});

test("a patch that changes nothing emits nothing", () => {
  const store = new Store(initial, counter);
  let changes = 0;
  store.on("change", () => changes++);
  store.update({ step: 1, label: "clicks" });
  store.update({});
  assert.equal(changes, 0);
  assert.equal(store.canUndo, false);
});

test("undo steps back through actions and patches, newest first", () => {
  const store = new Store(initial, counter);
  store.dispatch({ type: "increment" });
  store.update({ label: "taps" });
  store.dispatch({ type: "increment" });
  assert.equal(store.undo(), true);
  assert.deepEqual(store.state, { count: 1, step: 1, label: "taps" });
  store.undo();
  store.undo();
  assert.equal(store.state, initial);
  assert.equal(store.undo(), false);
});

test("undo emits change, and watchers see the value go back", () => {
  const store = new Store(initial, counter);
  const counts: number[] = [];
  store.watch("count", (next) => counts.push(next));
  store.dispatch({ type: "increment" });
  store.update({ label: "taps" }); // another field: the count watcher stays quiet
  store.undo();
  store.undo();
  assert.deepEqual(counts, [1, 0]);
});

test("rejected actions and no-ops don't add undo steps", () => {
  const store = new Store(initial, counter);
  store.dispatch({ type: "setStep", step: -2 });
  store.dispatch({ type: "noop" });
  assert.equal(store.canUndo, false);
});

function typeChecks(store: Store<typeof initial, Parameters<typeof counter>[1]>) {
  store.update({ count: 5 });
  // @ts-expect-error typo: no such field
  store.update({ cuont: 5 });
  // @ts-expect-error wrong type for the field
  store.update({ count: "5" });
}
void typeChecks;
