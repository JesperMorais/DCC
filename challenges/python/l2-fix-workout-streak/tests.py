def test_no_days_means_no_streak():
    assert longest_streak([]) == 0

def test_only_rest_days():
    assert longest_streak([False, False]) == 0

def test_a_single_workout():
    assert longest_streak([False, True, False]) == 1

def test_three_days_in_a_row():
    assert longest_streak([True, True, True]) == 3

def test_a_rest_day_breaks_the_streak():
    assert longest_streak([True, True, False, True]) == 2

def test_the_longest_run_can_come_last():
    assert longest_streak([True, False, True, True, True]) == 3
