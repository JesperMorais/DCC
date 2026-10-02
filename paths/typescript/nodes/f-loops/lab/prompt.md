Your fitness app stores a week of step counts as an array, one number per day, oldest first. Build the numbers for the weekly summary card. Use loops; don't change the array.

**`averageSteps(days)`** is the average per day, rounded to the nearest whole step with `Math.round`. An empty array gives `0`.

**`improvedDays(days)`** counts the days that had **more** steps than the day before. The first day has no day before it, so it never counts.

**`longestStreak(days, goal)`** is the longest run of days **in a row** that reached the goal (`>= goal`).

```ts
const week = [4000, 8000, 12000, 3000, 9000, 10000, 11000];

averageSteps(week);        // 8143    (57000 / 7 = 8142.86…)
improvedDays(week);        // 5       (8000, 12000, 9000, 10000, 11000)
longestStreak(week, 8000); // 3       (9000, 10000, 11000)
longestStreak(week, 20000); // 0
```

All three take a `number[]` (and `goal` is a `number`) and return a `number`.
