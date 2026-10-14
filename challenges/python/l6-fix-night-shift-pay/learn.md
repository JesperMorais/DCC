### Wall-clock time vs elapsed time

A `datetime` without `tzinfo` is **naive**: just a reading on some clock. Attach a zone and it becomes **aware**: it now names one exact instant.

```python
from datetime import UTC, datetime
from zoneinfo import ZoneInfo

oslo = ZoneInfo("Europe/Oslo")
t = datetime(2026, 7, 1, 12, 0, tzinfo=oslo)
t.utcoffset()          # timedelta(seconds=7200): summer time, UTC+2
t.astimezone(UTC)      # 2026-07-01 10:00:00+00:00, the same instant
```

With a `ZoneInfo`, the offset is looked up per date: the same zone is +2 in summer and +1 in winter. On the night the clocks change, an hour of wall-clock time is either repeated or skipped, so "08:00 minus 22:00" on the clock is not always the time that actually passed.

Python's datetime arithmetic has a documented subtlety here: how it treats two aware values depends on whether they share the **same `tzinfo` object** or not. It's worth reading the "aware arithmetic" notes in the `datetime` docs before trusting any duration that crosses a DST change. UTC has no DST, so instants converted to UTC always differ by exactly the elapsed time.

### Debugging technique: test the boundaries of the calendar

Time code breaks on the dates nobody picks for examples: DST transitions, month ends, leap days, New Year's Eve. When a duration is wrong, print **each step** for a boundary case: the values, their `utcoffset()`, and the intermediate results. Compare against what you get after converting each value to UTC.
