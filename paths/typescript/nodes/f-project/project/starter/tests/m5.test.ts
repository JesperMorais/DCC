import { test } from "node:test";
import assert from "node:assert/strict";
import fs from "node:fs";
import os from "node:os";
import path from "node:path";
import { run, USAGE } from "../src/cli.ts";
import { loadTodos } from "../src/storage.ts";

const tempFile = () => path.join(fs.mkdtempSync(path.join(os.tmpdir(), "todo-")), "todos.json");

test("add saves the to-do and confirms it", () => {
  const file = tempFile();
  assert.equal(run(["add", "buy", "milk"], file), "Added 1. buy milk");
  assert.equal(run(["add", "call mum"], file), "Added 2. call mum");
  assert.deepEqual(loadTodos(file), [
    { id: 1, text: "buy milk", done: false },
    { id: 2, text: "call mum", done: false },
  ]);
});

test("add with no text explains what's missing and saves nothing", () => {
  const file = tempFile();
  assert.equal(run(["add"], file), "What should I add? Try: npm start add buy milk");
  assert.equal(fs.existsSync(file), false);
});

test("list shows every to-do and a summary line", () => {
  const file = tempFile();
  assert.equal(run(["list"], file), "Nothing to do!");
  run(["add", "buy milk"], file);
  run(["add", "call mum"], file);
  run(["done", "1"], file);
  assert.equal(run(["list"], file), "[x] 1. buy milk\n[ ] 2. call mum\n\n1 of 2 left");
});

test("done and remove name the to-do they changed", () => {
  const file = tempFile();
  run(["add", "buy milk"], file);
  run(["add", "call mum"], file);
  assert.equal(run(["done", "2"], file), "Done: call mum");
  assert.equal(run(["remove", "1"], file), "Removed: buy milk");
  assert.deepEqual(loadTodos(file), [{ id: 2, text: "call mum", done: true }]);
});

test("done and remove reject ids that don't exist or aren't numbers", () => {
  const file = tempFile();
  run(["add", "buy milk"], file);
  assert.equal(run(["done", "7"], file), "No to-do with id 7");
  assert.equal(run(["remove", "abc"], file), "No to-do with id abc");
  assert.equal(run(["done"], file), "Which to-do? Try: npm start done 2");
  assert.equal(run(["remove"], file), "Which to-do? Try: npm start remove 2");
  assert.deepEqual(loadTodos(file), [{ id: 1, text: "buy milk", done: false }]);
});

test("clear removes the done to-dos and says how many", () => {
  const file = tempFile();
  run(["add", "a"], file);
  run(["add", "b"], file);
  run(["add", "c"], file);
  run(["done", "1"], file);
  run(["done", "3"], file);
  assert.equal(run(["clear"], file), "Cleared 2 done");
  assert.deepEqual(loadTodos(file).map((t) => t.text), ["b"]);
});

test("find shows the matching to-dos, or says there are none", () => {
  const file = tempFile();
  run(["add", "Buy milk"], file);
  run(["add", "call mum"], file);
  assert.equal(run(["find", "buy"], file), "[ ] 1. Buy milk");
  assert.equal(run(["find", "dentist"], file), 'No to-dos match "dentist"');
});

test("no command, help or an unknown command prints the usage", () => {
  const file = tempFile();
  assert.equal(run([], file), USAGE);
  assert.equal(run(["help"], file), USAGE);
  assert.equal(run(["dance"], file), `Unknown command "dance"\n\n${USAGE}`);
});
