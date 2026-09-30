import math


class Rect:  # note: no base class at all
    def __init__(self, w: float, h: float) -> None:
        self.w, self.h = w, h

    def area(self) -> float:
        return self.w * self.h


class Disc:
    def __init__(self, r: float) -> None:
        self.r = r

    def area(self) -> float:
        return math.pi * self.r**2


def test_isinstance_matches_by_shape_not_inheritance():
    assert isinstance(Rect(1, 1), HasArea)
    assert isinstance(Disc(1), HasArea)
    assert not isinstance("kitchen", HasArea)
    assert not isinstance(42, HasArea)


def test_largest_returns_the_object_itself():
    small, circle, square = Rect(5, 1), Disc(2), Rect(3, 4)
    assert largest([small, circle, square]) is circle


def test_largest_keeps_the_first_on_a_tie():
    a, b = Rect(2, 3), Rect(3, 2)
    assert largest([a, b]) is a


def test_largest_of_nothing_is_none():
    assert largest([]) is None


def test_total_area_sums_mixed_shapes():
    assert total_area([Rect(3, 4), Disc(2), Rect(5, 1)]) == approx(12 + 4 * math.pi + 5)
    assert total_area([]) == 0.0


def test_accepts_one_shot_generators():
    assert total_area(Rect(n, 1) for n in range(1, 4)) == approx(6.0)
    biggest = largest(Rect(n, n) for n in [2, 7, 3])
    assert biggest is not None and biggest.w == 7
