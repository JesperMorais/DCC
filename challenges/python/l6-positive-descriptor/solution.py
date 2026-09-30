from typing import Self, overload


class Positive:
    """A data descriptor that only accepts numbers greater than zero."""

    def __set_name__(self, owner: type, name: str) -> None:
        self.name = name

    @overload
    def __get__(self, obj: None, owner: type | None = None) -> Self: ...
    @overload
    def __get__(self, obj: object, owner: type | None = None) -> float: ...
    def __get__(self, obj: object | None, owner: type | None = None) -> Self | float:
        if obj is None:
            return self
        try:
            value: float = obj.__dict__[self.name]
        except KeyError:
            raise AttributeError(f"{self.name} has not been set") from None
        return value

    def __set__(self, obj: object, value: float) -> None:
        if isinstance(value, bool) or not isinstance(value, (int, float)):
            raise TypeError(f"{self.name} must be a number, got {value!r}")
        if value <= 0:
            raise ValueError(f"{self.name} must be positive, got {value!r}")
        obj.__dict__[self.name] = value
