test("sum to 3", () => {
  expect(sumTo(3)).toBe(6);
});

test("sum to 5", () => {
  expect(sumTo(5)).toBe(15);
});

test("sum to 1", () => {
  expect(sumTo(1)).toBe(1);
});

test("sum to 0 is 0", () => {
  expect(sumTo(0)).toBe(0);
});

test("sum to 100", () => {
  expect(sumTo(100)).toBe(5050);
});
