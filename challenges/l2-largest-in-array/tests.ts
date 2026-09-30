test("largest in the middle", () => {
  expect(largest([3, 7, 2])).toBe(7);
});

test("single element", () => {
  expect(largest([10])).toBe(10);
});

test("all negative numbers", () => {
  expect(largest([-8, -3, -12])).toBe(-3);
});

test("largest at the end", () => {
  expect(largest([1, 2, 3, 4, 50])).toBe(50);
});

test("largest at the start with duplicates", () => {
  expect(largest([9, 9, 1, 9])).toBe(9);
});
