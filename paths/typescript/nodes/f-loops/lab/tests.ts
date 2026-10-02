const week = [4000, 8000, 12000, 3000, 9000, 10000, 11000];

type cases = [
  Expect<Equal<ReturnType<typeof averageSteps>, number>>,
  Expect<Equal<ReturnType<typeof improvedDays>, number>>,
  Expect<Equal<Parameters<typeof longestStreak>, [days: number[], goal: number]>>,
];

// @ts-expect-error the step counts are numbers, not strings
improvedDays(["4000", "8000"]);

test("averageSteps rounds to a whole step", () => {
  expect(averageSteps(week)).toBe(8143);
  expect(averageSteps([1000, 2000])).toBe(1500);
});

test("averageSteps of an empty week is 0", () => {
  expect(averageSteps([])).toBe(0);
});

test("improvedDays counts days that beat the day before", () => {
  expect(improvedDays(week)).toBe(5);
});

test("improvedDays: equal is not an improvement, and day one never counts", () => {
  expect(improvedDays([5000, 5000, 5000])).toBe(0);
  expect(improvedDays([9000])).toBe(0);
  expect(improvedDays([])).toBe(0);
});

test("longestStreak finds the longest run in a row", () => {
  expect(longestStreak(week, 8000)).toBe(3);
  expect(longestStreak([9000, 9000, 1000, 9000], 8000)).toBe(2);
});

test("longestStreak: reaching the goal exactly counts", () => {
  expect(longestStreak([8000, 8000], 8000)).toBe(2);
});

test("longestStreak is 0 when no day reaches the goal", () => {
  expect(longestStreak(week, 20000)).toBe(0);
  expect(longestStreak([], 1)).toBe(0);
});

test("the input array is not changed", () => {
  const days = [3000, 1000, 2000];
  averageSteps(days);
  improvedDays(days);
  longestStreak(days, 1500);
  expect(days).toEqual([3000, 1000, 2000]);
});
