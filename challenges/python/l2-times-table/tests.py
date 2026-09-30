def test_first_four_multiples_of_three():
    assert times_table(3, 4) == [3, 6, 9, 12]


def test_a_single_multiple():
    assert times_table(7, 1) == [7]


def test_count_zero_gives_an_empty_list():
    assert times_table(5, 0) == []


def test_a_full_row_of_ten():
    assert times_table(9, 10) == [9, 18, 27, 36, 45, 54, 63, 72, 81, 90]


def test_zero_and_negative_n():
    assert times_table(0, 3) == [0, 0, 0]
    assert times_table(-2, 3) == [-2, -4, -6]
