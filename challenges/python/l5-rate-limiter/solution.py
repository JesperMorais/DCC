import time
from collections.abc import Callable


def make_rate_limiter(
    limit: int,
    window: float,
    clock: Callable[[], float] = time.monotonic,
) -> Callable[[], bool]:
    window_start: float | None = None
    used = 0

    def allow() -> bool:
        nonlocal window_start, used
        now = clock()
        if window_start is None or now - window_start >= window:
            window_start, used = now, 0
        if used < limit:
            used += 1
            return True
        return False

    return allow
