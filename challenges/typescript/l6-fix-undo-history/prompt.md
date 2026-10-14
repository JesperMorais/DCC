**Bug report from the drawing app team:** "Undo is unreliable. Renaming the drawing or zooming undoes fine, but if I hide a layer or add one and press *Undo*, nothing happens. The layer stays hidden, even though the undo button says there was something to undo."

`UndoHistory<T>` stores snapshots of an editor's state. Edits go through `commit(recipe)`, where `recipe` may freely **mutate** the draft it's given, at any depth.

- `present` is the current state. `canUndo` / `canRedo` say whether there's a step to move to.
- `undo()` goes back one commit and returns `true`, or returns `false` if there is nothing to undo. `redo()` is the mirror image.
- A new `commit` after an `undo` throws away the redo steps.
- Only the last `limit` commits (default 100) can be undone.
- Every snapshot is independent: a later commit never changes an earlier snapshot.

```ts
const h = new UndoHistory({ title: "Untitled", zoom: 1, layers: [{ name: "Background", visible: true }] });

h.commit((d) => { d.zoom = 2; });
h.undo();  // true, present.zoom === 1

h.commit((d) => { d.layers.push({ name: "Sketch", visible: true }); });
h.undo();  // true, present.layers has just "Background"
```

**Reproduction:**

```ts
h.commit((d) => { d.layers[0].visible = false; });
h.undo();
h.present.layers[0].visible; // false, expected true
```
