// The return type must be a tuple, not number[]
type cases = [Expect<Equal<ReturnType<typeof minMax>, [number, number] | undefined>>];

const temps: readonly number[] = [12, -3, 7, 22, 5];

test("finds the lowest and highest", () => {
  expect(minMax(temps)).toEqual([-3, 22]);
});

test("does not change the input", () => {
  minMax(temps);
  expect(temps).toEqual([12, -3, 7, 22, 5]);
});

test("a single reading is both min and max", () => {
  expect(minMax([4])).toEqual([4, 4]);
});

test("works with all-negative readings", () => {
  expect(minMax([-8, -2, -15])).toEqual([-15, -2]);
});

test("an empty list gives undefined", () => {
  expect(minMax([])).toBeUndefined();
});

test("can be destructured", () => {
  const range = minMax([10, 30, 20]);
  const spread = range ? range[1] - range[0] : 0;
  expect(spread).toBe(20);
});
