from collections.abc import Callable
from functools import lru_cache

type Coords = tuple[float, float]


def make_geocoder(fetch: Callable[[str], Coords], maxsize: int) -> Callable[[str], Coords]:
    @lru_cache(maxsize=maxsize)
    def cached(key: str) -> Coords:
        return fetch(key)

    def lookup(address: str) -> Coords:
        return cached(address.strip().casefold())

    return lookup
