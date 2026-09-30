def test_tall_and_old_enough():
    assert can_ride(130, 10) is True


def test_exactly_on_both_limits():
    assert can_ride(120, 8) is True


def test_one_cm_too_short():
    assert can_ride(119, 10) is False


def test_tall_enough_but_too_young():
    assert can_ride(150, 7) is False


def test_too_short_and_too_young():
    assert can_ride(100, 5) is False
