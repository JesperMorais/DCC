from functools import total_ordering
from typing import Self


class Version:
    def __init__(self, major: int, minor: int, patch: int) -> None:
        self.major = major
        self.minor = minor
        self.patch = patch

    @classmethod
    def from_string(cls, text: str) -> Self:
        raise NotImplementedError
