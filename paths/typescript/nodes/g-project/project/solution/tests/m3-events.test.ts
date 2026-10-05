import { test } from "node:test";
import assert from "node:assert/strict";
import { Store } from "../src/store.ts";
import { counter, initial } from "./fixtures.ts";

test("change carries prev, next and the action that caused it", () => {
  const store = new Store(initial, counter);
  const seen: string[] = [];
  store.on("change", ({ prev, next, cause }) => {
    if (cause.kind === "action") seen.push(`${cause.action.type}: ${prev.count} -> ${next.count}`);
  });
  store.dispatch({ type: "increment" });
  store.dispatch({ type: "increment" });
  assert.deepEqual(seen, ["increment: 0 -> 1", "increment: 1 -> 2"]);
});

test("no change event when the reducer returns the same state", () => {
  const store = new Store(initial, counter);
  let changes = 0;
  store.on("change", () => changes++);
  store.dispatch({ type: "noop" });
  assert.equal(changes, 0);
});

test("rejected actions emit rejected, not change", () => {
  const store = new Store(initial, counter);
  const events: string[] = [];
  store.on("change", () => events.push("change"));
  store.on("rejected", ({ action, error }) => events.push(`${action.type}: ${error.message}`));
  store.dispatch({ type: "setStep", step: 0 });
  assert.deepEqual(events, ["setStep: step must be positive"]);
});

test("watch fires only when its key changes", () => {
  const store = new Store(initial, counter);
  const steps: [number, number][] = [];
  const stop = store.watch("step", (next, prev) => steps.push([prev, next]));
  store.dispatch({ type: "increment" }); // count changes, step doesn't
  store.dispatch({ type: "setStep", step: 3 });
  stop();
  store.dispatch({ type: "setStep", step: 4 });
  assert.deepEqual(steps, [[1, 3]]);
});

function typeChecks(store: Store<typeof initial, Parameters<typeof counter>[1]>) {
  store.watch("label", (next) => next.toUpperCase());
  // @ts-expect-error step is a number, no toUpperCase
  store.watch("step", (next) => next.toUpperCase());
  // @ts-expect-error not a key of the state
  store.watch("total", () => {});
  // @ts-expect-error not a store event
  store.on("updated", () => {});
  // @ts-expect-error the change payload has no `state`
  store.on("change", (e) => e.state);
}
void typeChecks;
