test("4 is even", () => {
  expect(isEven(4)).toBe(true);
});

test("7 is odd", () => {
  expect(isEven(7)).toBe(false);
});

test("0 is even", () => {
  expect(isEven(0)).toBe(true);
});

test("-2 is even", () => {
  expect(isEven(-2)).toBe(true);
});

test("-3 is odd", () => {
  expect(isEven(-3)).toBe(false);
});

test("a big even number", () => {
  expect(isEven(1000)).toBe(true);
});
