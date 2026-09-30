from itertools import groupby, pairwise


def hike_stats(elevations: list[int]) -> tuple[int, int, int]:
    steps = [b - a for a, b in pairwise(elevations)]
    ascent = sum(d for d in steps if d > 0)
    descent = -sum(d for d in steps if d < 0)
    longest_climb = max(
        (sum(run) for rising, run in groupby(steps, key=lambda d: d > 0) if rising),
        default=0,
    )
    return ascent, descent, longest_climb
