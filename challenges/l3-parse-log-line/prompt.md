Your server writes log lines like this:

```
12:04:55 [WARN] Disk almost full
```

That is: a **time**, one space, a **level** in square brackets, one space, then the **message** (which may contain spaces).

1. Write `parseLogLine(line)` that returns a `LogEntry` object, or `null` if the line doesn't match the format **or** the level isn't one of `INFO`, `WARN`, `ERROR`.
2. Write `parseLog(text)` that parses a multi-line string (lines separated by `\n`) and returns only the valid entries, in order.

```ts
parseLogLine("12:04:55 [WARN] Disk almost full");
// { time: "12:04:55", level: "WARN", message: "Disk almost full" }

parseLogLine("12:05:01 [DEBUG] cache hit");   // null (unknown level)
parseLogLine("garbage");                       // null

parseLog("09:00:00 [INFO] Boot\n\n09:00:02 [ERROR] DB down");
// [ { time: "09:00:00", level: "INFO",  message: "Boot" },
//   { time: "09:00:02", level: "ERROR", message: "DB down" } ]
```
