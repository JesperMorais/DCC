test("2 in the morning and 3 in the afternoon is 5", () => {
  expect(cupsToday("2", "3")).toBe(5);
});

test("1 and 1 is 2", () => {
  expect(cupsToday("1", "1")).toBe(2);
});

test("none in the morning", () => {
  expect(cupsToday("0", "4")).toBe(4);
});

test("none in the afternoon", () => {
  expect(cupsToday("6", "0")).toBe(6);
});

test("a day without coffee", () => {
  expect(cupsToday("0", "0")).toBe(0);
});
