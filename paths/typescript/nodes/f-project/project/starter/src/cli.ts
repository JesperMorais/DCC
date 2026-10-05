// Milestone 5: turn the command-line words into an action and a message.
// `run` gets the words after `npm start` (for example ["done", "2"]) and the
// file to keep the list in. It returns the text to print instead of printing
// it, so the tests can check it.
import type { Todo } from "./todo.ts";
import { loadTodos, saveTodos } from "./storage.ts";

export const USAGE = `Usage:
  npm start add <text>     add a to-do
  npm start list           show every to-do
  npm start done <id>      mark a to-do as done
  npm start remove <id>    delete a to-do
  npm start clear          delete every done to-do
  npm start find <text>    show the to-dos that contain <text>`;

export function run(args: string[], file: string): string {
  throw new Error("TODO (m5): run");
}
