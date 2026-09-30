const cart: CartItem[] = [
  { name: "Coffee", priceCents: 450, quantity: 2 },
  { name: "Bagel", priceCents: 325, quantity: 1 },
];

test("sums price × quantity", () => {
  expect(cartTotal(cart)).toBe(1225);
});

test("an empty cart costs nothing", () => {
  expect(cartTotal([])).toBe(0);
});

test("a line with quantity 0 adds nothing", () => {
  expect(cartTotal([...cart, { name: "Muffin", priceCents: 299, quantity: 0 }])).toBe(1225);
});

test("formats dollars and cents", () => {
  expect(formatCents(1225)).toBe("$12.25");
});

test("pads small amounts", () => {
  expect(formatCents(5)).toBe("$0.05");
  expect(formatCents(0)).toBe("$0.00");
});

test("whole dollars keep two decimals", () => {
  expect(formatCents(cartTotal([{ name: "Mug", priceCents: 800, quantity: 3 }]))).toBe("$24.00");
});
