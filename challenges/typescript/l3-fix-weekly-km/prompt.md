**Bug report from the running app**

> I had the flu all week and didn't run once. When I opened my weekly summary the app crashed instead of saying 0 km.

`weeklyKm(runs)` returns the total distance of a week's runs, **rounded to one decimal**. A week with no runs is `0`.

```ts
weeklyKm([{ day: "Mon", km: 5.2 }, { day: "Thu", km: 7.4 }]); // 12.6
weeklyKm([{ day: "Sat", km: 21.1 }]);                         // 21.1
weeklyKm([]);                                                  // 0
```

**Reproduction:** `weeklyKm([])` throws `TypeError: Reduce of empty array with no initial value`, expected `0`.

Find the bug and fix it.
