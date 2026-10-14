test("sunday is the weekend", () => {
  expect(isWeekend("sunday")).toBe(true);
});

test("monday is not", () => {
  expect(isWeekend("monday")).toBe(false);
});

test("a capital letter doesn't matter", () => {
  expect(isWeekend("Saturday")).toBe(true);
});

test("all capitals, weekday", () => {
  expect(isWeekend("MONDAY")).toBe(false);
});

test("mixed capitals, weekend", () => {
  expect(isWeekend("SaTuRdAy")).toBe(true);
  expect(isWeekend("SUNDAY")).toBe(true);
});
