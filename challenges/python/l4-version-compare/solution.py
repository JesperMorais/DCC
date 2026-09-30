from functools import total_ordering
from typing import Self


@total_ordering
class Version:
    def __init__(self, major: int, minor: int, patch: int) -> None:
        self.major = major
        self.minor = minor
        self.patch = patch

    @classmethod
    def from_string(cls, text: str) -> Self:
        parts = text.removeprefix("v").split(".")
        if len(parts) != 3 or not all(part.isdecimal() for part in parts):
            raise ValueError(f"invalid version: {text!r}")
        major, minor, patch = (int(part) for part in parts)
        return cls(major, minor, patch)

    def _key(self) -> tuple[int, int, int]:
        return (self.major, self.minor, self.patch)

    def __eq__(self, other: object) -> bool:
        if not isinstance(other, Version):
            return NotImplemented
        return self._key() == other._key()

    def __lt__(self, other: object) -> bool:
        if not isinstance(other, Version):
            return NotImplemented
        return self._key() < other._key()

    def __hash__(self) -> int:
        return hash(self._key())

    def __repr__(self) -> str:
        return f"Version('{self.major}.{self.minor}.{self.patch}')"
