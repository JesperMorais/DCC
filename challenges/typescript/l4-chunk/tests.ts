interface Order {
  id: number;
  total: number;
}

const orders: readonly Order[] = [
  { id: 1, total: 20 },
  { id: 2, total: 35 },
  { id: 3, total: 12 },
];

const letterPages = chunk(["a", "b", "c"], 2);
const orderPages = chunk(orders, 2);

// chunk must keep the element type
type cases = [
  Expect<Equal<typeof letterPages, string[][]>>,
  Expect<Equal<typeof orderPages, Order[][]>>,
];

test("splits into pages of 2", () => {
  expect(chunk([1, 2, 3, 4, 5], 2)).toEqual([[1, 2], [3, 4], [5]]);
});

test("exact multiple has no short page", () => {
  expect(chunk([1, 2, 3, 4], 2)).toEqual([[1, 2], [3, 4]]);
});

test("size bigger than the array", () => {
  expect(chunk(["a", "b", "c"], 5)).toEqual([["a", "b", "c"]]);
});

test("size 1 wraps every element", () => {
  expect(letterPages.length).toBe(2);
  expect(chunk(["x", "y"], 1)).toEqual([["x"], ["y"]]);
});

test("works on objects", () => {
  expect(orderPages).toEqual([[{ id: 1, total: 20 }, { id: 2, total: 35 }], [{ id: 3, total: 12 }]]);
});

test("empty input", () => {
  expect(chunk([], 3)).toEqual([]);
});
