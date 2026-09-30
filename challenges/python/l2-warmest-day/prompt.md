A weather station logs one temperature per day. Write `warmest_day(temps)` that returns the **index** (position) of the warmest day.

- If two days tie for warmest, return the **earlier** one.
- If the list is empty, return `None`.
- Solve it with a loop. Don't use `max()`, since the point is to practise the "best so far" pattern.

Examples:

- `warmest_day([12.5, 18.0, 15.2])` → `1`
- `warmest_day([-5.0, -2.5, -8.0])` → `1` (a winter week: every day is below zero)
- `warmest_day([])` → `None`
