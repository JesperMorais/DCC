from collections.abc import Iterable
from typing import Protocol, runtime_checkable


class HasArea(Protocol):
    """Anything with an `area() -> float` method. Declare it here."""


def largest[T: HasArea](shapes: Iterable[T]) -> T | None:
    raise NotImplementedError


def total_area(shapes: Iterable[HasArea]) -> float:
    raise NotImplementedError
