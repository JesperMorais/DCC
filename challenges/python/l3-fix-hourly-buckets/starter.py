def events_by_hour(events: list[tuple[int, str]]) -> list[list[str]]:
    buckets: list[list[str]] = [[]] * 24
    for hour, name in events:
        buckets[hour].append(name)
    return buckets
