import { test } from "node:test";
import assert from "node:assert/strict";
import fs from "node:fs";
import os from "node:os";
import path from "node:path";
import { loadTodos, saveTodos } from "../src/storage.ts";
import type { Todo } from "../src/todo.ts";

// Each test gets its own empty folder, so they never see each other's files.
const tempFile = () => path.join(fs.mkdtempSync(path.join(os.tmpdir(), "todo-")), "todos.json");

const todos: Todo[] = [
  { id: 1, text: "buy milk", done: false },
  { id: 2, text: "call mum", done: true },
];

test("loadTodos gives an empty list when there's no file yet", () => {
  assert.deepEqual(loadTodos(tempFile()), []);
});

test("saveTodos writes a file that loadTodos reads back", () => {
  const file = tempFile();
  saveTodos(file, todos);
  assert.ok(fs.existsSync(file), "the file should exist after saving");
  assert.deepEqual(loadTodos(file), todos);
});

test("the file is plain JSON, so you can open it and read it", () => {
  const file = tempFile();
  saveTodos(file, todos);
  assert.deepEqual(JSON.parse(fs.readFileSync(file, "utf8")), todos);
});

test("saving again replaces the old list", () => {
  const file = tempFile();
  saveTodos(file, todos);
  saveTodos(file, [todos[1]]);
  assert.deepEqual(loadTodos(file), [todos[1]]);
});
