import time
from collections.abc import Callable


def make_rate_limiter(
    limit: int,
    window: float,
    clock: Callable[[], float] = time.monotonic,
) -> Callable[[], bool]:
    def allow() -> bool:
        return True

    return allow
