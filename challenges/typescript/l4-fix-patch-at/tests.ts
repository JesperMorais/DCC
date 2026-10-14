type Todo = { title: string; done: boolean };

test("merges the patch into the item at index", () => {
  const todos: Todo[] = [{ title: "Milk", done: false }, { title: "Eggs", done: false }];
  expect(patchItemAt(todos, 1, { done: true })).toEqual([{ title: "Milk", done: false }, { title: "Eggs", done: true }]);
});

test("keeps fields the patch doesn't mention", () => {
  const todos: Todo[] = [{ title: "Milk", done: true }];
  expect(patchItemAt(todos, 0, { title: "Oat milk" })).toEqual([{ title: "Oat milk", done: true }]);
});

test("does not change the list you passed in", () => {
  const before: Todo[] = [{ title: "Milk", done: false }];
  patchItemAt(before, 0, { done: true });
  expect(before).toEqual([{ title: "Milk", done: false }]);
});

test("returns a new array", () => {
  const before: Todo[] = [{ title: "Milk", done: false }];
  const after = patchItemAt(before, 0, { done: true });
  expect(after === before).toBe(false);
});

test("an index outside the list returns an unchanged copy", () => {
  const todos: Todo[] = [{ title: "Milk", done: false }];
  const out = patchItemAt(todos, 5, { done: true });
  expect(out).toEqual(todos);
  expect(out === todos).toBe(false);
  expect(patchItemAt(todos, -1, { done: true })).toEqual(todos);
});

const patched = patchItemAt([{ id: 1, name: "a" }], 0, { name: "b" });
type patch_cases = [Expect<Equal<typeof patched, { id: number; name: string }[]>>];
