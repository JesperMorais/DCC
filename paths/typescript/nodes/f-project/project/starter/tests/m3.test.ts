import { test } from "node:test";
import assert from "node:assert/strict";
import { countLeft, search, summary, type Todo } from "../src/todo.ts";

const todos: Todo[] = [
  { id: 1, text: "Buy milk", done: false },
  { id: 2, text: "call mum", done: true },
  { id: 3, text: "buy a bike lock", done: false },
];
const allDone: Todo[] = todos.map((t) => ({ ...t, done: true }));

test("countLeft counts the to-dos that aren't done", () => {
  assert.equal(countLeft(todos), 2);
  assert.equal(countLeft(allDone), 0);
  assert.equal(countLeft([]), 0);
});

test("summary says how many are left out of how many", () => {
  assert.equal(summary(todos), "2 of 3 left");
});

test("summary celebrates when everything is done, and handles an empty list", () => {
  assert.equal(summary(allDone), "All 3 done!");
  assert.equal(summary([]), "Nothing to do!");
});

test("search finds text anywhere, ignoring case", () => {
  assert.deepEqual(search(todos, "BUY").map((t) => t.id), [1, 3]);
  assert.deepEqual(search(todos, "mum").map((t) => t.id), [2]);
});

test("search ignores spaces around the query and returns [] for no match", () => {
  assert.deepEqual(search(todos, "  bike ").map((t) => t.id), [3]);
  assert.deepEqual(search(todos, "dentist"), []);
});

test("a blank query matches everything", () => {
  assert.deepEqual(search(todos, "  "), todos);
});
