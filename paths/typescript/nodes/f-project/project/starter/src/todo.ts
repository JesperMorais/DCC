// The to-do logic. Every function here is pure: it takes the list, returns a NEW
// list or a value, and never changes what it was given. That's what makes them
// easy to test. Replace each `throw` with your implementation.

/** One to-do. `id` is the number the user types (`npm start done 2`). */
export type Todo = {
  id: number;
  text: string;
  done: boolean;
};

// ── Milestone 1: add and show ──────────────────────────────────────────────

/** The list with a new, not-done to-do at the end. Ids are never reused: the new id is one more than the highest so far (1 for an empty list). Text is trimmed; blank text adds nothing. */
export function addTodo(todos: readonly Todo[], text: string): Todo[] {
  throw new Error("TODO (m1): addTodo");
}

/** One line for one to-do: "[ ] 1. buy milk" or "[x] 2. call mum". */
export function formatTodo(todo: Todo): string {
  throw new Error("TODO (m1): formatTodo");
}

/** Every to-do on its own line, in list order. An empty list gives "Nothing to do!". */
export function formatList(todos: readonly Todo[]): string {
  throw new Error("TODO (m1): formatList");
}

// ── Milestone 2: done, remove, clear ───────────────────────────────────────

/** The list with to-do `id` marked done. An unknown id changes nothing. */
export function markDone(todos: readonly Todo[], id: number): Todo[] {
  throw new Error("TODO (m2): markDone");
}

/** The list without to-do `id`. An unknown id changes nothing. */
export function removeTodo(todos: readonly Todo[], id: number): Todo[] {
  throw new Error("TODO (m2): removeTodo");
}

/** The list with every done to-do gone. */
export function clearDone(todos: readonly Todo[]): Todo[] {
  throw new Error("TODO (m2): clearDone");
}

// ── Milestone 3: count and search ──────────────────────────────────────────

/** How many to-dos are not done yet. */
export function countLeft(todos: readonly Todo[]): number {
  throw new Error("TODO (m3): countLeft");
}

/** "Nothing to do!", "All 3 done!" or "2 of 3 left". */
export function summary(todos: readonly Todo[]): string {
  throw new Error("TODO (m3): summary");
}

/** The to-dos whose text contains `query`, ignoring case and surrounding spaces. A blank query matches everything. */
export function search(todos: readonly Todo[], query: string): Todo[] {
  throw new Error("TODO (m3): search");
}
