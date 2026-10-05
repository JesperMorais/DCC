// The to-do logic. Every function here is pure: it takes the list, returns a NEW
// list or a value, and never changes what it was given.

/** One to-do. `id` is the number the user types (`npm start done 2`). */
export type Todo = {
  id: number;
  text: string;
  done: boolean;
};

// ── Milestone 1: add and show ──────────────────────────────────────────────

/** The list with a new, not-done to-do at the end. Ids are never reused: the new id is one more than the highest so far (1 for an empty list). Text is trimmed; blank text adds nothing. */
export function addTodo(todos: readonly Todo[], text: string): Todo[] {
  const clean = text.trim();
  if (clean === "") return [...todos];

  let highest = 0;
  for (const t of todos) {
    if (t.id > highest) highest = t.id;
  }
  return [...todos, { id: highest + 1, text: clean, done: false }];
}

/** One line for one to-do: "[ ] 1. buy milk" or "[x] 2. call mum". */
export function formatTodo(todo: Todo): string {
  const box = todo.done ? "[x]" : "[ ]";
  return `${box} ${todo.id}. ${todo.text}`;
}

/** Every to-do on its own line, in list order. An empty list gives "Nothing to do!". */
export function formatList(todos: readonly Todo[]): string {
  if (todos.length === 0) return "Nothing to do!";
  return todos.map(formatTodo).join("\n");
}

// ── Milestone 2: done, remove, clear ───────────────────────────────────────

/** The list with to-do `id` marked done. An unknown id changes nothing. */
export function markDone(todos: readonly Todo[], id: number): Todo[] {
  return todos.map((t) => (t.id === id ? { ...t, done: true } : t));
}

/** The list without to-do `id`. An unknown id changes nothing. */
export function removeTodo(todos: readonly Todo[], id: number): Todo[] {
  return todos.filter((t) => t.id !== id);
}

/** The list with every done to-do gone. */
export function clearDone(todos: readonly Todo[]): Todo[] {
  return todos.filter((t) => !t.done);
}

// ── Milestone 3: count and search ──────────────────────────────────────────

/** How many to-dos are not done yet. */
export function countLeft(todos: readonly Todo[]): number {
  let left = 0;
  for (const t of todos) {
    if (!t.done) left++;
  }
  return left;
}

/** "Nothing to do!", "All 3 done!" or "2 of 3 left". */
export function summary(todos: readonly Todo[]): string {
  const left = countLeft(todos);
  if (todos.length === 0) return "Nothing to do!";
  if (left === 0) return `All ${todos.length} done!`;
  return `${left} of ${todos.length} left`;
}

/** The to-dos whose text contains `query`, ignoring case and surrounding spaces. A blank query matches everything. */
export function search(todos: readonly Todo[], query: string): Todo[] {
  const needle = query.trim().toLowerCase();
  return todos.filter((t) => t.text.toLowerCase().includes(needle));
}
