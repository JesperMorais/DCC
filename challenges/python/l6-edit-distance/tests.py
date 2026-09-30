def test_classic_examples():
    assert edit_distance("kitten", "sitting") == 3
    assert edit_distance("flaw", "lawn") == 2
    assert edit_distance("pyhton", "python") == 2


def test_identical_strings_are_zero_apart():
    assert edit_distance("same", "same") == 0
    assert edit_distance("", "") == 0


def test_empty_side_costs_the_other_length():
    assert edit_distance("", "abc") == 3
    assert edit_distance("abcd", "") == 4


def test_it_is_symmetric_and_case_sensitive():
    assert edit_distance("sunday", "saturday") == edit_distance("saturday", "sunday") == 3
    assert edit_distance("Python", "python") == 1


def test_substitution_beats_delete_plus_insert():
    assert edit_distance("cat", "cut") == 1
    assert edit_distance("abc", "xyz") == 3


def test_long_strings_finish_quickly():
    a = "the quick brown fox " * 20
    b = "the quack brown fix " * 20
    assert edit_distance(a, b) == 40
    assert edit_distance("saturday" * 50, "sunday" * 50) == 150
