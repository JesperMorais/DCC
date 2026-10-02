type cases = [
  Expect<Equal<ReturnType<typeof fullBoxes>, number>>,
  Expect<Equal<ReturnType<typeof leftover>, number>>,
  Expect<Equal<ReturnType<typeof boxesNeeded>, number>>,
  Expect<Equal<Parameters<typeof boxesNeeded>, [items: number, perBox: number]>>,
];

// @ts-expect-error items must be a number, not a string from a form field
fullBoxes("14", 6);

test("fullBoxes rounds down to whole boxes", () => {
  expect(fullBoxes(14, 6)).toBe(2);
  expect(fullBoxes(5, 6)).toBe(0);
});

test("leftover is what doesn't fill a box", () => {
  expect(leftover(14, 6)).toBe(2);
  expect(leftover(5, 6)).toBe(5);
});

test("an exact fit leaves nothing over", () => {
  expect(fullBoxes(12, 6)).toBe(2);
  expect(leftover(12, 6)).toBe(0);
  expect(boxesNeeded(12, 6)).toBe(2);
});

test("boxesNeeded counts the partly filled box", () => {
  expect(boxesNeeded(14, 6)).toBe(3);
  expect(boxesNeeded(1, 6)).toBe(1);
});

test("full boxes and leftovers add back up to the order", () => {
  expect(fullBoxes(100, 8) * 8 + leftover(100, 8)).toBe(100);
});

test("an empty order needs no boxes", () => {
  expect(fullBoxes(0, 6)).toBe(0);
  expect(leftover(0, 6)).toBe(0);
  expect(boxesNeeded(0, 6)).toBe(0);
});
