// Milestone 4: keep the list in a JSON file between runs.
// Both functions take the file path, so the tests can point them at a temp file.
import fs from "node:fs";
import type { Todo } from "./todo.ts";

/** The to-dos saved in `file`, or an empty list if the file doesn't exist yet. */
export function loadTodos(file: string): Todo[] {
  if (!fs.existsSync(file)) return [];
  return JSON.parse(fs.readFileSync(file, "utf8"));
}

/** Write `todos` to `file`, replacing whatever was there. */
export function saveTodos(file: string, todos: readonly Todo[]): void {
  fs.writeFileSync(file, JSON.stringify(todos, null, 2) + "\n");
}
