import { test } from "node:test";
import assert from "node:assert/strict";
import { addTodo, formatList, formatTodo, type Todo } from "../src/todo.ts";

// Frozen inputs: if your function tries to change them, the test throws.
const freeze = (todos: Todo[]): Todo[] => Object.freeze(todos.map((t) => Object.freeze({ ...t }))) as Todo[];

test("addTodo adds a not-done to-do with id 1 to an empty list", () => {
  assert.deepEqual(addTodo([], "buy milk"), [{ id: 1, text: "buy milk", done: false }]);
});

test("addTodo gives the next id after the highest one, even with gaps", () => {
  const todos = freeze([
    { id: 1, text: "a", done: false },
    { id: 4, text: "b", done: true },
  ]);
  const next = addTodo(todos, "c");
  assert.equal(next.length, 3);
  assert.deepEqual(next[2], { id: 5, text: "c", done: false });
});

test("addTodo trims the text and ignores blank text", () => {
  assert.deepEqual(addTodo([], "  call mum  "), [{ id: 1, text: "call mum", done: false }]);
  assert.deepEqual(addTodo([], "   "), []);
});

test("addTodo returns a new list and leaves the old one alone", () => {
  const todos = freeze([{ id: 1, text: "a", done: false }]);
  const next = addTodo(todos, "b");
  assert.notEqual(next, todos);
  assert.equal(todos.length, 1);
});

test("formatTodo shows a checkbox, the id and the text", () => {
  assert.equal(formatTodo({ id: 1, text: "buy milk", done: false }), "[ ] 1. buy milk");
  assert.equal(formatTodo({ id: 12, text: "call mum", done: true }), "[x] 12. call mum");
});

test("formatList puts one to-do per line, in order", () => {
  const todos = freeze([
    { id: 1, text: "buy milk", done: false },
    { id: 2, text: "call mum", done: true },
  ]);
  assert.equal(formatList(todos), "[ ] 1. buy milk\n[x] 2. call mum");
});

test("formatList of an empty list says so", () => {
  assert.equal(formatList([]), "Nothing to do!");
});
