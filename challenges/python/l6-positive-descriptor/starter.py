from typing import Self, overload


class Positive:
    """A data descriptor that only accepts numbers greater than zero."""

    def __set_name__(self, owner: type, name: str) -> None:
        pass  # Python calls this once, when the owning class is created

    @overload
    def __get__(self, obj: None, owner: type | None = None) -> Self: ...
    @overload
    def __get__(self, obj: object, owner: type | None = None) -> float: ...
    def __get__(self, obj: object | None, owner: type | None = None) -> Self | float:
        raise NotImplementedError

    def __set__(self, obj: object, value: float) -> None:
        raise NotImplementedError
