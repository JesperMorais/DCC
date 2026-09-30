Your monitoring page groups log entries so you can collapse all the `"info"` noise and focus on errors. Write a generic `groupBy(items, getKey)` that returns a **`Map`** from each key to the items that produced it.

- `getKey` is a function that computes the group key for one item.
- Groups appear in the Map in the order their key was **first seen**; items inside a group keep their original order.
- An empty input gives an empty Map.
- Types must flow through: grouping `LogEntry[]` by `level` must give `Map<LogLevel, LogEntry[]>` — the tests check it.

```ts
const logs: LogEntry[] = [
  { level: "info",  message: "boot" },
  { level: "error", message: "db down" },
  { level: "info",  message: "retry" },
];

const byLevel = groupBy(logs, (log) => log.level);
byLevel.get("info");   // [{ level: "info", message: "boot" }, { level: "info", message: "retry" }]
byLevel.get("error");  // [{ level: "error", message: "db down" }]
byLevel.get("warn");   // undefined
[...byLevel.keys()];   // ["info", "error"]
```
