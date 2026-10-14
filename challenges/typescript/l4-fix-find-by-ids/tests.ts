const customers = new Map([
  ["c1", { name: "Ada", email: "ada@example.com" }],
  ["c2", { name: "Linus", email: "linus@example.com" }],
]);

test("returns records in the order of ids", () => {
  expect(findByIds(customers, ["c2", "c1"]).map((c) => c.name)).toEqual(["Linus", "Ada"]);
});

test("skips ids that are not in the map", () => {
  const found = findByIds(customers, ["c1", "gone"]);
  expect(found).toHaveLength(1);
  expect(found[0].name).toBe("Ada");
});

test("no matching ids gives an empty list", () => {
  expect(findByIds(customers, ["x", "y"])).toHaveLength(0);
  expect(findByIds(customers, [])).toHaveLength(0);
});

test("isPresent rejects null and undefined", () => {
  expect(isPresent(null)).toBe(false);
  expect(isPresent(undefined)).toBe(false);
});

test("isPresent keeps falsy real values", () => {
  expect(isPresent(0)).toBe(true);
  expect(isPresent("")).toBe(true);
  expect(isPresent(false)).toBe(true);
});

const foundNames = findByIds(new Map([[1, "one"]]), [1, 2]);
type find_cases = [Expect<Equal<typeof foundNames, string[]>>];
