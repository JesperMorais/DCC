const activity = [
  { at: 30, text: "deploy" },
  { at: 10, text: "login" },
  { at: 20, text: "build" },
];

test("returns the n most recent, oldest first", () => {
  expect(latestEntries(activity, 2)).toEqual([{ at: 20, text: "build" }, { at: 30, text: "deploy" }]);
  expect(latestEntries(activity, 1)).toEqual([{ at: 30, text: "deploy" }]);
});

test("fewer entries than n returns all of them, sorted", () => {
  expect(latestEntries(activity, 10).map((e) => e.text)).toEqual(["login", "build", "deploy"]);
});

test("does not reorder the input", () => {
  const input = [{ at: 2 }, { at: 1 }];
  latestEntries(input, 1);
  expect(input).toEqual([{ at: 2 }, { at: 1 }]);
});

test("n of 0 returns nothing", () => {
  expect(latestEntries(activity, 0)).toEqual([]);
});

test("a negative n returns nothing", () => {
  expect(latestEntries(activity, -2)).toEqual([]);
});

test("an empty list returns an empty list", () => {
  expect(latestEntries([], 3)).toEqual([]);
});

const latestLines = latestEntries([{ at: 1, level: "info" as const }], 1);
type latest_cases = [Expect<Equal<typeof latestLines, { at: number; level: "info" }[]>>];
