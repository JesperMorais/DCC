import { test } from "node:test";
import assert from "node:assert/strict";
import { Store } from "../src/store.ts";
import { counter, initial } from "./fixtures.ts";

test("dispatch runs the reducer and replaces the state", () => {
  const store = new Store(initial, counter);
  store.dispatch({ type: "increment" });
  store.dispatch({ type: "setStep", step: 5 });
  store.dispatch({ type: "increment" });
  assert.deepEqual(store.state, { count: 6, step: 5, label: "clicks" });
  assert.equal(store.select("count"), 6);
});

test("the old state object is never mutated", () => {
  const store = new Store(initial, counter);
  const before = store.state;
  store.dispatch({ type: "increment" });
  assert.equal(before.count, 0);
  assert.notEqual(store.state, before);
  assert.deepEqual(initial, { count: 0, step: 1, label: "clicks" });
});

test("a rejected action leaves the state untouched", () => {
  const store = new Store(initial, counter);
  store.dispatch({ type: "increment" });
  const before = store.state;
  assert.equal(store.dispatch({ type: "setStep", step: -1 }), false);
  assert.equal(store.state, before);
});

test("bugs in the reducer are not swallowed", () => {
  const store = new Store(initial, counter);
  assert.throws(() => store.dispatch({ type: "crash" }), TypeError);
  assert.equal(store.state, initial);
});

function typeChecks(store: Store<typeof initial, Parameters<typeof counter>[1]>) {
  const count: number = store.select("count");
  void count;
  // @ts-expect-error not a key of the state
  store.select("total");
  // @ts-expect-error select("label") is a string
  const n: number = store.select("label");
  void n;
  // @ts-expect-error not one of the actions
  store.dispatch({ type: "decrement" });
  // @ts-expect-error setStep needs a step
  store.dispatch({ type: "setStep" });
  // @ts-expect-error state is read-only from the outside
  store.state.count = 99;
  // @ts-expect-error a store's state must be an object
  new Store(42, (s: number) => s);
}
void typeChecks;
