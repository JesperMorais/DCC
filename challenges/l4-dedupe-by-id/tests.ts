interface Customer {
  id: number;
  name: string;
}

const customers: Customer[] = [
  { id: 1, name: "Ada" },
  { id: 2, name: "Linus" },
  { id: 1, name: "Ada (copy)" },
  { id: 3, name: "Grace" },
  { id: 2, name: "Linus (copy)" },
];

const unique = dedupeById(customers);

type cases = [
  // the full element type is preserved
  Expect<Equal<typeof unique, Customer[]>>,
];

// @ts-expect-error — objects without an id are not allowed
dedupeById([{ name: "no id" }]);

test("keeps the first occurrence", () => {
  expect(unique).toEqual([
    { id: 1, name: "Ada" },
    { id: 2, name: "Linus" },
    { id: 3, name: "Grace" },
  ]);
});

test("does not modify the input", () => {
  expect(customers).toHaveLength(5);
});

test("already unique → same items", () => {
  expect(dedupeById([{ id: 5, sku: "X" }, { id: 6, sku: "Y" }])).toEqual([
    { id: 5, sku: "X" },
    { id: 6, sku: "Y" },
  ]);
});

test("all duplicates collapse to one", () => {
  expect(dedupeById([{ id: 9 }, { id: 9 }, { id: 9 }])).toEqual([{ id: 9 }]);
});

test("empty input", () => {
  expect(dedupeById([])).toEqual([]);
});
