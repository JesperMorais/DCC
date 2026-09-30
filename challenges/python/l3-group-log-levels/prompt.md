Your service writes log lines like `"ERROR: db timeout"`. For a quick triage view, write `group_by_level(lines)` that groups the **messages** by their **level**.

- A line is `LEVEL: message`. It's split at the **first** colon, so the message itself may contain colons.
- The level is stripped and **uppercased** (`" warn"` becomes `"WARN"`), and the message is stripped.
- Lines with **no colon** at all are skipped.
- Messages keep their original order within a level, and levels appear in the order they were **first seen**.
- Return a plain `dict[str, list[str]]`. No input gives `{}`.

```python
group_by_level([
    "INFO: server started",
    "ERROR: db timeout",
    "warn : disk 91% full",
    "garbage line",
    "ERROR: retry failed: code=5",
])
# {"INFO": ["server started"],
#  "ERROR": ["db timeout", "retry failed: code=5"],
#  "WARN": ["disk 91% full"]}
```
