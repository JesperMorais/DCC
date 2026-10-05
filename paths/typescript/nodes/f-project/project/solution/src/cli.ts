// Milestone 5: turn the command-line words into an action and a message.
// `run` gets the words after `npm start` (for example ["done", "2"]) and the
// file to keep the list in. It returns the text to print instead of printing
// it, so the tests can check it.
import { addTodo, clearDone, formatList, markDone, removeTodo, search, summary, type Todo } from "./todo.ts";
import { loadTodos, saveTodos } from "./storage.ts";

export const USAGE = `Usage:
  npm start add <text>     add a to-do
  npm start list           show every to-do
  npm start done <id>      mark a to-do as done
  npm start remove <id>    delete a to-do
  npm start clear          delete every done to-do
  npm start find <text>    show the to-dos that contain <text>`;

export function run(args: string[], file: string): string {
  const command = args[0];
  const rest = args.slice(1).join(" ");
  const todos = loadTodos(file);

  if (command === "add") {
    const next = addTodo(todos, rest);
    if (next.length === todos.length) return "What should I add? Try: npm start add buy milk";
    saveTodos(file, next);
    const added = next[next.length - 1];
    return `Added ${added.id}. ${added.text}`;
  }

  if (command === "list") {
    if (todos.length === 0) return formatList(todos);
    return `${formatList(todos)}\n\n${summary(todos)}`;
  }

  if (command === "done" || command === "remove") {
    if (rest === "") return `Which to-do? Try: npm start ${command} 2`;
    const target = findById(todos, Number(rest));
    if (target === undefined) return `No to-do with id ${rest}`;
    if (command === "done") {
      saveTodos(file, markDone(todos, target.id));
      return `Done: ${target.text}`;
    }
    saveTodos(file, removeTodo(todos, target.id));
    return `Removed: ${target.text}`;
  }

  if (command === "clear") {
    const next = clearDone(todos);
    saveTodos(file, next);
    return `Cleared ${todos.length - next.length} done`;
  }

  if (command === "find") {
    const matches = search(todos, rest);
    if (matches.length === 0) return `No to-dos match "${rest}"`;
    return formatList(matches);
  }

  if (command === undefined || command === "help") return USAGE;
  return `Unknown command "${command}"\n\n${USAGE}`;
}

/** The to-do with this id, or undefined. `Number("abc")` is NaN, which matches nothing. */
function findById(todos: readonly Todo[], id: number): Todo | undefined {
  return todos.filter((t) => t.id === id)[0];
}
