from collections.abc import Callable
from functools import lru_cache

type Coords = tuple[float, float]


def make_geocoder(fetch: Callable[[str], Coords], maxsize: int) -> Callable[[str], Coords]:
    return fetch
