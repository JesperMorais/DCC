test("single-digit scores", () => {
  expect(topThree([5, 3, 9, 1])).toEqual([9, 5, 3]);
});

test("scores with different numbers of digits", () => {
  expect(topThree([100, 25, 9, 80])).toEqual([100, 80, 25]);
});

test("fewer than three scores", () => {
  expect(topThree([4, 7])).toEqual([7, 4]);
});

test("no scores", () => {
  expect(topThree([])).toEqual([]);
});

test("a big score among small ones", () => {
  expect(topThree([3, 1200, 45, 7, 300])).toEqual([1200, 300, 45]);
});

test("doesn't change the original array", () => {
  const scores = [2, 8, 5];
  topThree(scores);
  expect(scores).toEqual([2, 8, 5]);
});
