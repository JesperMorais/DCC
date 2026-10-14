from dataclasses import dataclass


@dataclass(frozen=True)
class ReadingStats:
    count: int
    missing: int
    low: float | None
    high: float | None


def reading_stats(readings: list[float | None]) -> ReadingStats:
    present = (r for r in readings if r is not None)
    low = min(present, default=None)
    high = max(present, default=None)
    count = sum(r is not None for r in readings)
    return ReadingStats(count=count, missing=len(readings) - count, low=low, high=high)
