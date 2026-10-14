**Bug report from the alarm clock app**

> I set "no alarm on weekends". My phone woke me at 6:30 on Saturday anyway.

`isWeekend(day)` gets the name of a day and returns `true` for Saturday and Sunday, `false` for every other day. Users type day names however they like, so **capital letters must not matter**.

- `isWeekend("sunday")` → `true`
- `isWeekend("MONDAY")` → `false`
- `isWeekend("SaTuRdAy")` → `true`

**Reproduction:** `isWeekend("Saturday")` returns `false`, expected `true`.

Find the bug and fix it.
