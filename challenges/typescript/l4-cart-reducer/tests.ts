const tea = { id: "tea", name: "Tea", priceCents: 300 };
const cake = { id: "cake", name: "Cake", priceCents: 450 };

const twoLines: CartState = {
  lines: [
    { ...tea, qty: 2 },
    { ...cake, qty: 1 },
  ],
};

// Compile-time checks only (never called)
function invalidActions() {
  // @ts-expect-error — "checkout" is not a CartAction
  cartReducer(twoLines, { type: "checkout" });

  // @ts-expect-error — "remove" needs an id
  cartReducer(twoLines, { type: "remove" });
}

test("add appends a new line with qty 1", () => {
  expect(cartReducer({ lines: [] }, { type: "add", line: tea })).toEqual({ lines: [{ ...tea, qty: 1 }] });
});

test("add on an existing line bumps qty", () => {
  const next = cartReducer(twoLines, { type: "add", line: tea });
  expect(next.lines).toEqual([
    { ...tea, qty: 3 },
    { ...cake, qty: 1 },
  ]);
});

test("remove drops the line", () => {
  expect(cartReducer(twoLines, { type: "remove", id: "tea" }).lines).toEqual([{ ...cake, qty: 1 }]);
  expect(cartReducer(twoLines, { type: "remove", id: "nope" }).lines).toHaveLength(2);
});

test("setQty changes the quantity", () => {
  expect(cartReducer(twoLines, { type: "setQty", id: "cake", qty: 5 }).lines[1]).toEqual({ ...cake, qty: 5 });
});

test("setQty to 0 or less removes the line", () => {
  expect(cartReducer(twoLines, { type: "setQty", id: "tea", qty: 0 }).lines).toEqual([{ ...cake, qty: 1 }]);
  expect(cartReducer(twoLines, { type: "setQty", id: "cake", qty: -2 }).lines).toEqual([{ ...tea, qty: 2 }]);
});

test("clear empties the cart", () => {
  expect(cartReducer(twoLines, { type: "clear" })).toEqual({ lines: [] });
});

test("never mutates the previous state", () => {
  const snapshot = structuredClone(twoLines);
  const next = cartReducer(twoLines, { type: "add", line: tea });
  cartReducer(twoLines, { type: "setQty", id: "cake", qty: 9 });
  cartReducer(twoLines, { type: "remove", id: "tea" });
  expect(twoLines).toEqual(snapshot);
  expect(next === twoLines).toBe(false);
  expect(next.lines[0] === twoLines.lines[0]).toBe(false);
});
