test("hasItem finds an item", () => {
  expect(hasItem(["milk", "eggs"], "eggs")).toBe(true);
});

test("hasItem on a missing item", () => {
  expect(hasItem(["milk", "eggs"], "bread")).toBe(false);
  expect(hasItem(["milk"], "Milk")).toBe(false);
});

test("positionOf returns the index", () => {
  expect(positionOf(["milk", "eggs", "jam"], "jam")).toBe(2);
  expect(positionOf(["milk", "eggs", "jam"], "milk")).toBe(0);
});

test("positionOf returns -1 when missing", () => {
  expect(positionOf(["milk"], "tea")).toBe(-1);
  expect(positionOf([], "tea")).toBe(-1);
});

test("addIfMissing adds a new item at the end", () => {
  expect(addIfMissing(["milk"], "eggs")).toEqual(["milk", "eggs"]);
  expect(addIfMissing([], "jam")).toEqual(["jam"]);
});

test("addIfMissing skips duplicates", () => {
  expect(addIfMissing(["milk", "eggs"], "milk")).toEqual(["milk", "eggs"]);
});

test("addIfMissing does not change the original", () => {
  const original = ["milk"];
  addIfMissing(original, "eggs");
  expect(original).toEqual(["milk"]);
});
