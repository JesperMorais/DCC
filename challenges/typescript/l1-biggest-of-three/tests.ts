test("biggest is last", () => {
  expect(biggestOfThree(1, 2, 3)).toBe(3);
});

test("biggest is first", () => {
  expect(biggestOfThree(9, 4, 7)).toBe(9);
});

test("biggest is in the middle", () => {
  expect(biggestOfThree(5, 8, 2)).toBe(8);
});

test("works with negative numbers", () => {
  expect(biggestOfThree(-1, -5, -3)).toBe(-1);
});

test("handles ties", () => {
  expect(biggestOfThree(4, 4, 2)).toBe(4);
});

test("all the same", () => {
  expect(biggestOfThree(6, 6, 6)).toBe(6);
});
