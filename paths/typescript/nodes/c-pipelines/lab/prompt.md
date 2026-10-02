Your running club's app shows results after every race. Each entry is a `Run`:

```ts
interface Run { runner: string; seconds: number; finished: boolean }
```

A runner who didn't finish (`finished: false`) is a **DNF**. Their `seconds` is meaningless.

**1. `podium(runs)`** returns up to three display lines for the fastest finishers:

- Only finished runs count. Fastest (fewest `seconds`) first. Ties keep their original order.
- Each line is `"<place>. <runner> <seconds>s"`, with seconds to **two decimals**: `"1. Ada 12.30s"`.
- Fewer than three finishers gives fewer lines, and none gives `[]`.

**2. `raceStats(runs)`** returns a `RaceStats`:

```ts
interface RaceStats { finishers: number; dnf: number; average: number | null }
```

- `average` is the mean `seconds` of the finishers (unrounded), or `null` if nobody finished.
- Use `reduce` to collect the counts and total in one pass.

Both functions take a `readonly Run[]` and must not modify it.

```ts
podium([
  { runner: "Ada", seconds: 12.3, finished: true },
  { runner: "Linus", seconds: 0, finished: false },
  { runner: "Grace", seconds: 11.955, finished: true },
]); // ["1. Grace 11.96s", "2. Ada 12.30s"]

raceStats([...same runs]); // { finishers: 2, dnf: 1, average: 12.1275 }
```
