def test_four_digits():
    assert digit_sum(1234) == 10


def test_a_single_digit():
    assert digit_sum(7) == 7


def test_zero():
    assert digit_sum(0) == 0


def test_zeros_in_the_middle_add_nothing():
    assert digit_sum(1000) == 1
    assert digit_sum(90807) == 24


def test_ignores_the_minus_sign():
    assert digit_sum(-47) == 11


def test_a_big_number():
    assert digit_sum(99999) == 45
