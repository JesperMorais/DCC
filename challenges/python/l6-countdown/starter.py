from typing import Self


class CountdownIterator:
    def __init__(self, start: int) -> None:
        self.remaining = start

    def __iter__(self) -> Self:
        return self

    def __next__(self) -> int:
        raise NotImplementedError


class Countdown:
    def __init__(self, start: int) -> None:
        self.start = start

    def __iter__(self) -> CountdownIterator:
        raise NotImplementedError
