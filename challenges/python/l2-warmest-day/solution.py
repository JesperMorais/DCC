def warmest_day(temps: list[float]) -> int | None:
    if not temps:
        return None
    best = 0
    for i, temp in enumerate(temps):
        if temp > temps[best]:
            best = i
    return best
