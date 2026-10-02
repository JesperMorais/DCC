interface Product {
  sku: string;
  price: number;
}

const viewed: string[] = ["mug", "tee", "cap"];
const recent = pushRecent(viewed, "sock", 3);
const ids = pushRecent([3, 1, 4], 1, 5);
const pairs = zip([1, 2, 3], ["a", "b"]);

type cases = [
  Expect<Equal<typeof recent, string[]>>,
  Expect<Equal<typeof ids, number[]>>,
  Expect<Equal<typeof pairs, [number, string][]>>,
  Expect<Equal<ReturnType<typeof zip<Product, boolean>>, [Product, boolean][]>>,
];

// @ts-expect-error — a string doesn't belong in a number[]
pushRecent([1, 2, 3], "4", 3);

void (() => {
  // @ts-expect-error — the result is a string[], so it has no .toFixed
  recent[0].toFixed(2);
});

test("pushRecent puts the new item first and drops the oldest", () => {
  expect(recent).toEqual(["sock", "mug", "tee"]);
});

test("pushRecent moves an existing item to the front", () => {
  expect(pushRecent(viewed, "cap", 3)).toEqual(["cap", "mug", "tee"]);
  expect(ids).toEqual([1, 3, 4]);
});

test("pushRecent works on an empty list and respects max", () => {
  expect(pushRecent([], "mug", 3)).toEqual(["mug"]);
  expect(pushRecent(["a", "b", "c", "d"], "e", 2)).toEqual(["e", "a"]);
});

test("pushRecent compares objects by identity", () => {
  const mug: Product = { sku: "MUG", price: 9.5 };
  const tee: Product = { sku: "TEE", price: 15 };
  const list = pushRecent(pushRecent([mug, tee], tee, 5), { sku: "MUG", price: 9.5 }, 5);
  expect(list).toHaveLength(3);
  expect(list[1]).toBe(tee);
});

test("zip pairs by position and stops at the shorter list", () => {
  expect(pairs).toEqual([[1, "a"], [2, "b"]]);
  expect(zip(["x"], [true, false])).toEqual([["x", true]]);
  expect(zip([], [1, 2])).toEqual([]);
});

test("neither function modifies its input", () => {
  pushRecent(viewed, "sock", 2);
  const as = [1, 2];
  zip(as, ["a", "b"]);
  expect(viewed).toEqual(["mug", "tee", "cap"]);
  expect(as).toEqual([1, 2]);
});
