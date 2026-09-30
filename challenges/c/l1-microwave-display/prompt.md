A microwave's display shows up to four digits, like `2:05` or `12:30`. The chip inside doesn't store the colon, so it keeps the number `205`: the minutes, then **exactly two digits** of seconds.

Write `microwave_display`, which turns a cooking time in seconds into that display number:

```c
int microwave_display(int seconds);
```

Examples:

- `microwave_display(125)` → `205` (2 minutes and 5 seconds)
- `microwave_display(90)` → `130` (1 minute and 30 seconds)
- `microwave_display(45)` → `45` (0 minutes, so it shows `0:45`)
- `microwave_display(600)` → `1000` (10 minutes and 0 seconds)

`seconds` is never negative.
