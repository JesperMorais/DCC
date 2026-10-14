type Layer = { name: string; visible: boolean };
type Drawing = { title: string; zoom: number; layers: Layer[] };

const blank = (): Drawing => ({ title: "Untitled", zoom: 1, layers: [{ name: "Background", visible: true }] });

test("commit changes the present", () => {
  const h = new UndoHistory(blank());
  h.commit((d) => {
    d.title = "Logo";
  });
  expect(h.present.title).toBe("Logo");
  expect(h.canUndo).toBe(true);
});

test("undo brings back a top-level field", () => {
  const h = new UndoHistory(blank());
  h.commit((d) => {
    d.zoom = 2;
  });
  expect(h.undo()).toBe(true);
  expect(h.present.zoom).toBe(1);
});

test("undo brings back a change inside a nested object", () => {
  const h = new UndoHistory(blank());
  h.commit((d) => {
    d.layers[0].visible = false;
  });
  h.undo();
  expect(h.present.layers[0].visible).toBe(true);
});

test("undo removes an added layer", () => {
  const h = new UndoHistory(blank());
  h.commit((d) => {
    d.layers.push({ name: "Sketch", visible: true });
  });
  h.commit((d) => {
    d.layers.push({ name: "Ink", visible: true });
  });
  h.undo();
  expect(h.present.layers.map((l) => l.name)).toEqual(["Background", "Sketch"]);
  h.undo();
  expect(h.present.layers.map((l) => l.name)).toEqual(["Background"]);
});

test("redo re-applies an undone change, and a new commit clears redo", () => {
  const h = new UndoHistory(blank());
  h.commit((d) => {
    d.title = "Logo";
  });
  h.undo();
  expect(h.redo()).toBe(true);
  expect(h.present.title).toBe("Logo");
  h.undo();
  h.commit((d) => {
    d.zoom = 3;
  });
  expect(h.canRedo).toBe(false);
  expect(h.redo()).toBe(false);
});

test("nothing to undo returns false and keeps the present", () => {
  const h = new UndoHistory(blank());
  expect(h.canUndo).toBe(false);
  expect(h.undo()).toBe(false);
  expect(h.present.title).toBe("Untitled");
});

test("only the last `limit` steps can be undone", () => {
  const h = new UndoHistory({ count: 0 }, 2);
  for (let i = 1; i <= 4; i++) {
    h.commit((d) => {
      d.count = i;
    });
  }
  expect(h.undo()).toBe(true);
  expect(h.undo()).toBe(true);
  expect(h.undo()).toBe(false);
  expect(h.present.count).toBe(2);
});
