import { test } from "node:test";
import assert from "node:assert/strict";
import { clearDone, markDone, removeTodo, type Todo } from "../src/todo.ts";

// Frozen inputs: if your function tries to change them, the test throws.
const freeze = (todos: Todo[]): Todo[] => Object.freeze(todos.map((t) => Object.freeze({ ...t }))) as Todo[];

const sample = () =>
  freeze([
    { id: 1, text: "buy milk", done: false },
    { id: 2, text: "call mum", done: true },
    { id: 3, text: "fix bike", done: false },
  ]);

test("markDone marks only the matching to-do", () => {
  assert.deepEqual(markDone(sample(), 3), [
    { id: 1, text: "buy milk", done: false },
    { id: 2, text: "call mum", done: true },
    { id: 3, text: "fix bike", done: true },
  ]);
});

test("markDone on an already-done or unknown id changes nothing", () => {
  assert.deepEqual(markDone(sample(), 2), sample());
  assert.deepEqual(markDone(sample(), 99), sample());
});

test("markDone doesn't change the original list or its to-dos", () => {
  const todos = sample();
  const next = markDone(todos, 1);
  assert.equal(todos[0].done, false);
  assert.equal(next[0].done, true);
});

test("removeTodo drops the matching to-do and keeps the others' ids", () => {
  assert.deepEqual(removeTodo(sample(), 2), [
    { id: 1, text: "buy milk", done: false },
    { id: 3, text: "fix bike", done: false },
  ]);
});

test("removeTodo with an unknown id changes nothing", () => {
  assert.deepEqual(removeTodo(sample(), 7), sample());
});

test("clearDone keeps only the to-dos that aren't done", () => {
  assert.deepEqual(clearDone(sample()), [
    { id: 1, text: "buy milk", done: false },
    { id: 3, text: "fix bike", done: false },
  ]);
  assert.deepEqual(clearDone([]), []);
});
