**Bug report from the ops team:** "The new *events per hour* chart shows the exact same events in every single hour, all 24 of them. We only had one deploy today, at 09:00."

`events_by_hour(events)` takes `(hour, name)` pairs, where `hour` is `0`–`23`, and returns a list of **24 lists**. Index `h` holds the names of the events in hour `h`, in the order they were given. Hours with no events have an empty list.

```python
result = events_by_hour([(9, "deploy"), (14, "backup"), (9, "alert")])
result[9]    # ["deploy", "alert"]
result[14]   # ["backup"]
result[0]    # []
len(result)  # 24
```

**Reproduction:** `events_by_hour([(9, "deploy")])[0]` returns `["deploy"]`, expected `[]`.
