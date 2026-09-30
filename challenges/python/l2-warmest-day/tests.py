def test_finds_the_warmest_day():
    assert warmest_day([12.5, 18.0, 15.2]) == 1


def test_warmest_can_be_the_first_or_last_day():
    assert warmest_day([30.0, 21.0, 25.5]) == 0
    assert warmest_day([10.0, 11.0, 12.0]) == 2


def test_works_when_every_day_is_below_zero():
    assert warmest_day([-5.0, -2.5, -8.0]) == 1


def test_a_tie_returns_the_earlier_day():
    assert warmest_day([20.0, 22.0, 19.0, 22.0]) == 1


def test_a_single_day():
    assert warmest_day([3.0]) == 0


def test_no_days_gives_none():
    assert warmest_day([]) is None
