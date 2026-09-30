A workout app shows the length of a session as `hours:minutes`. Write `format_duration(minutes)` that turns a number of minutes into that text.

- The minutes part always has **two digits** (`"2:05"`, not `"2:5"`).
- The hours part has no padding, and it keeps growing past 24 (it's a duration, not a time of day).

Examples:

- `format_duration(125)` → `"2:05"`
- `format_duration(45)` → `"0:45"`
- `format_duration(60)` → `"1:00"`

You can assume `minutes` is never negative.
