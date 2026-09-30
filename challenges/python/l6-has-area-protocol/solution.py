from collections.abc import Iterable
from typing import Protocol, runtime_checkable


@runtime_checkable
class HasArea(Protocol):
    def area(self) -> float: ...


def largest[T: HasArea](shapes: Iterable[T]) -> T | None:
    return max(shapes, key=lambda s: s.area(), default=None)


def total_area(shapes: Iterable[HasArea]) -> float:
    return sum((s.area() for s in shapes), 0.0)
