from typing import Self


class CountdownIterator:
    def __init__(self, start: int) -> None:
        self.remaining = start

    def __iter__(self) -> Self:
        return self

    def __next__(self) -> int:
        if self.remaining <= 0:
            raise StopIteration
        value = self.remaining
        self.remaining -= 1
        return value


class Countdown:
    def __init__(self, start: int) -> None:
        if start < 0:
            raise ValueError("start must not be negative")
        self.start = start

    def __iter__(self) -> CountdownIterator:
        return CountdownIterator(self.start)
