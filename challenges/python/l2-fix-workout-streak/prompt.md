**Bug report from a user of the fitness app:** "I worked out Monday, Tuesday and Wednesday, but my profile says my longest streak is **1 day**."

`longest_streak(days)` takes one `bool` per day (`True` = worked out, `False` = rest day) and returns the length of the **longest run of `True` days in a row**. No days, or no workouts at all, gives `0`.

- `longest_streak([True, True, False, True])` → `2`
- `longest_streak([False, True, True, True])` → `3`
- `longest_streak([False, False])` → `0`

**Reproduction:** `longest_streak([True, True, True])` returns `1`, expected `3`.
