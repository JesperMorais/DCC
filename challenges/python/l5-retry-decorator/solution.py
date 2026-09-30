import functools
from collections.abc import Callable


def retry[**P, R](
    times: int, on: tuple[type[Exception], ...] = (Exception,)
) -> Callable[[Callable[P, R]], Callable[P, R]]:
    if times < 1:
        raise ValueError("times must be at least 1")

    def decorate(fn: Callable[P, R]) -> Callable[P, R]:
        @functools.wraps(fn)
        def wrapper(*args: P.args, **kwargs: P.kwargs) -> R:
            for attempt in range(1, times + 1):
                try:
                    return fn(*args, **kwargs)
                except on:
                    if attempt == times:
                        raise
            raise AssertionError("unreachable")

        return wrapper

    return decorate
