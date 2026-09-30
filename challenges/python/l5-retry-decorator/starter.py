import functools
from collections.abc import Callable


def retry[**P, R](
    times: int, on: tuple[type[Exception], ...] = (Exception,)
) -> Callable[[Callable[P, R]], Callable[P, R]]:
    def decorate(fn: Callable[P, R]) -> Callable[P, R]:
        return fn

    return decorate
