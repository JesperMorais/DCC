**Bug report from the dashboard team:** "Users can choose how many recent events the activity widget shows. Somebody set it to **0** to hide the widget's list, and instead it dumped their entire history: 4,000 rows."

`latestEntries` takes any list of timestamped entries and returns the `n` most recent ones, **oldest first** (so they read top to bottom like a chat log).

```ts
function latestEntries<T extends { at: number }>(entries: readonly T[], n: number): T[]
```

- The input may be in any order, and it must not be changed.
- If there are fewer than `n` entries, return all of them (sorted).
- `n` of `0` or less returns `[]`.

```ts
const log = [
  { at: 30, text: "deploy" },
  { at: 10, text: "login" },
  { at: 20, text: "build" },
];

latestEntries(log, 2);  // [{ at: 20, text: "build" }, { at: 30, text: "deploy" }]
latestEntries(log, 10); // all three, sorted by at
```

**Reproduction:**

```ts
latestEntries(log, 0); // returns all three entries, expected []
```
