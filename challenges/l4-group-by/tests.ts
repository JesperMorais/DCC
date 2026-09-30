type LogLevel = "info" | "warn" | "error";

interface LogEntry {
  level: LogLevel;
  message: string;
}

const logs: LogEntry[] = [
  { level: "info", message: "boot" },
  { level: "error", message: "db down" },
  { level: "info", message: "retry" },
  { level: "warn", message: "slow query" },
  { level: "error", message: "db still down" },
];

const byLevel = groupBy(logs, (log) => log.level);
const byLength = groupBy(["hi", "hey", "yo", "hello"], (word) => word.length);

type cases = [
  Expect<Equal<typeof byLevel, Map<LogLevel, LogEntry[]>>>,
  Expect<Equal<typeof byLength, Map<number, string[]>>>,
];

test("groups items by key, keeping order", () => {
  expect(byLevel.get("info")).toEqual([
    { level: "info", message: "boot" },
    { level: "info", message: "retry" },
  ]);
  expect(byLevel.get("error")).toEqual([
    { level: "error", message: "db down" },
    { level: "error", message: "db still down" },
  ]);
});

test("keys appear in first-seen order", () => {
  expect([...byLevel.keys()]).toEqual(["info", "error", "warn"]);
});

test("missing groups are undefined", () => {
  const onlyInfo = groupBy([logs[0]], (log) => log.level);
  expect(onlyInfo.get("error")).toBeUndefined();
  expect(onlyInfo.size).toBe(1);
});

test("keys don't have to be strings", () => {
  expect(byLength.get(2)).toEqual(["hi", "yo"]);
  expect([...byLength.keys()]).toEqual([2, 3, 5]);
});

test("empty input gives an empty Map", () => {
  expect(groupBy([], (x: number) => x).size).toBe(0);
});
