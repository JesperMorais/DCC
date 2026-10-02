const race: readonly Run[] = [
  { runner: "Ada", seconds: 12.3, finished: true },
  { runner: "Linus", seconds: 0, finished: false },
  { runner: "Grace", seconds: 11.955, finished: true },
  { runner: "Margaret", seconds: 13, finished: true },
  { runner: "Alan", seconds: 12.3, finished: true },
  { runner: "Barbara", seconds: 0, finished: false },
];

test("podium shows the three fastest finishers, two decimals", () => {
  expect(podium(race)).toEqual(["1. Grace 11.96s", "2. Ada 12.30s", "3. Alan 12.30s"]);
});

test("podium ignores DNFs even with 0 seconds", () => {
  expect(podium(race).join(" ")).not.toContain("Linus");
});

test("podium with fewer than three finishers", () => {
  expect(podium([{ runner: "Ada", seconds: 9, finished: true }, { runner: "Linus", seconds: 0, finished: false }]))
    .toEqual(["1. Ada 9.00s"]);
  expect(podium([])).toEqual([]);
});

test("raceStats counts finishers and DNFs and averages the finishers", () => {
  const stats = raceStats(race);
  expect(stats.finishers).toBe(4);
  expect(stats.dnf).toBe(2);
  expect(stats.average).toBeCloseTo((12.3 + 11.955 + 13 + 12.3) / 4);
});

test("raceStats average is null when nobody finished", () => {
  expect(raceStats([{ runner: "Linus", seconds: 0, finished: false }])).toEqual({ finishers: 0, dnf: 1, average: null });
  expect(raceStats([])).toEqual({ finishers: 0, dnf: 0, average: null });
});

test("neither function modifies the input", () => {
  const runs = Object.freeze([
    { runner: "Slow", seconds: 20, finished: true },
    { runner: "Fast", seconds: 10, finished: true },
  ]);
  podium(runs);
  raceStats(runs);
  expect(runs.map((r) => r.runner)).toEqual(["Slow", "Fast"]);
});

// A normal (mutable) array must be accepted too.
const mutableRuns: Run[] = [{ runner: "Ada", seconds: 1, finished: true }];
podium(mutableRuns);
