test("adult citizen can vote", () => {
  expect(canVote(20, true)).toBe(true);
});

test("exactly 18 is old enough", () => {
  expect(canVote(18, true)).toBe(true);
});

test("17 is too young", () => {
  expect(canVote(17, true)).toBe(false);
});

test("non-citizen cannot vote", () => {
  expect(canVote(40, false)).toBe(false);
});

test("too young and not a citizen", () => {
  expect(canVote(10, false)).toBe(false);
});
