**Bug report from the to-do app team:** "Undo is broken. I tick a task as done, hit *Undo*, and the old version of the list *also* shows it as done. The history snapshots seem to change by themselves."

The history keeps every version of the list, and each edit goes through this helper:

```ts
function patchItemAt<T>(list: readonly T[], index: number, patch: Partial<T>): T[]
```

It should return a **new** list where the item at `index` has `patch` merged into it. The list you passed in is **never changed**. An `index` outside the list returns an unchanged copy.

```ts
const todos = [{ title: "Milk", done: false }, { title: "Eggs", done: false }];

patchItemAt(todos, 1, { done: true });
// → [{ title: "Milk", done: false }, { title: "Eggs", done: true }]

patchItemAt(todos, 0, { title: "Oat milk" });
// → [{ title: "Oat milk", done: false }, { title: "Eggs", done: false }]
```

**Reproduction:**

```ts
const before = [{ title: "Milk", done: false }];
const after = patchItemAt(before, 0, { done: true });
before[0].done; // true, expected false
```
