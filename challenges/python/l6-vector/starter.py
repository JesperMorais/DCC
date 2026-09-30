import math
from dataclasses import dataclass


@dataclass(frozen=True)
class Vector:
    x: float
    y: float

    def __add__(self, other: "Vector") -> "Vector":
        raise NotImplementedError
