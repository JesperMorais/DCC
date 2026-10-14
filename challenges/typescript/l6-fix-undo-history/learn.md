### Shallow and deep copies

Spread copies **one level**. The new object gets the same property values, and when a value is an object or array, that means the same *reference*:

```ts
const doc = { title: "A", tags: ["x"] };
const copy = { ...doc };

copy.title = "B";      // only copy changes
copy.tags.push("y");   // doc.tags is ["x", "y"] too: one shared array
```

That's fine when the code that edits the copy *replaces* nested values (`{ ...doc, tags: [...doc.tags, "y"] }`). It breaks when the edit **mutates in place**, because the "old" version gets edited as well.

For plain data (objects, arrays, numbers, strings, `Date`, `Map`, `Set`), the built-in `structuredClone(value)` makes a **deep** copy: nothing is shared with the original. It can't copy functions or class instances' methods.

### Snapshot-based undo

An undo history keeps a stack of past states. It only works if each snapshot is **frozen in time**: if any later edit can reach into an old snapshot, undoing just "restores" the already-changed data.

### Debugging technique: check what's shared

When old data changes by itself, test identity with `===` at each level: `old === now`, `old.layers === now.layers`, `old.layers[0] === now.layers[0]`. The first `true` where you expected `false` shows how deep the copy really went.
