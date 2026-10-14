test("everyone at or above the pass mark", () => {
  expect(everyonePassed([70, 85, 50], 50)).toBe(true);
});

test("the first score failed", () => {
  expect(everyonePassed([30, 90], 50)).toBe(false);
});

test("a failing score in the middle", () => {
  expect(everyonePassed([80, 40, 90], 50)).toBe(false);
});

test("a failing score at the end", () => {
  expect(everyonePassed([60, 75, 95, 49], 50)).toBe(false);
});

test("no scores yet counts as everyone passed", () => {
  expect(everyonePassed([], 50)).toBe(true);
});
