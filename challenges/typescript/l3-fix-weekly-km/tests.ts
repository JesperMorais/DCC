test("adds up two runs", () => {
  expect(weeklyKm([{ day: "Mon", km: 5.2 }, { day: "Thu", km: 7.4 }])).toBe(12.6);
});

test("a single run", () => {
  expect(weeklyKm([{ day: "Sat", km: 21.1 }])).toBe(21.1);
});

test("a week with no runs is 0", () => {
  expect(weeklyKm([])).toBe(0);
});

test("rounds to one decimal", () => {
  expect(weeklyKm([{ day: "Tue", km: 3.33 }, { day: "Wed", km: 4.44 }, { day: "Sun", km: 10 }])).toBe(17.8);
});
