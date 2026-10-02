type cases = [
  Expect<Equal<ReturnType<typeof freeShipping>, boolean>>,
  Expect<Equal<ReturnType<typeof shippingCost>, number>>,
  Expect<Equal<Parameters<typeof shippingCost>, [subtotal: number, weightKg: number, isMember: boolean]>>,
];

// @ts-expect-error isMember is a boolean, not the text "yes"
freeShipping(20, 3, "yes");

test("free for big orders and for members", () => {
  expect(freeShipping(60, 3, false)).toBe(true);
  expect(freeShipping(20, 3, true)).toBe(true);
  expect(freeShipping(20, 3, false)).toBe(false);
});

test("$50 exactly counts as a big order", () => {
  expect(freeShipping(50, 3, false)).toBe(true);
  expect(freeShipping(49.99, 3, false)).toBe(false);
});

test("freight is never free", () => {
  expect(freeShipping(60, 25, true)).toBe(false);
  expect(freeShipping(60, 20, true)).toBe(true);
});

test("free orders cost 0", () => {
  expect(shippingCost(75, 4, false)).toBe(0);
  expect(shippingCost(10, 12, true)).toBe(0);
});

test("weight bands include their upper limit", () => {
  expect(shippingCost(20, 0.5, false)).toBe(4);
  expect(shippingCost(20, 1, false)).toBe(4);
  expect(shippingCost(20, 5, false)).toBe(8);
  expect(shippingCost(20, 5.5, false)).toBe(12);
  expect(shippingCost(20, 20, false)).toBe(12);
});

test("over 20 kg is freight, even for members", () => {
  expect(shippingCost(80, 30, false)).toBe(25);
  expect(shippingCost(10, 20.5, true)).toBe(25);
});
