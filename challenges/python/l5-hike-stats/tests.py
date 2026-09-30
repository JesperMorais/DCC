def test_the_example_track():
    assert hike_stats([100, 120, 150, 140, 140, 160, 170, 175, 90]) == (85, 95, 50)


def test_longest_means_most_metres_not_most_steps():
    # a one-step climb of 50 beats a three-step climb of 15
    assert hike_stats([0, 50, 40, 45, 50, 55]) == (65, 10, 50)


def test_a_flat_step_ends_a_climb():
    assert hike_stats([0, 10, 10, 20]) == (20, 0, 10)


def test_all_downhill():
    assert hike_stats([300, 250, 250, 100]) == (0, 200, 0)


def test_a_single_climb_to_the_end():
    assert hike_stats([5, 6, 8, 11]) == (6, 0, 6)


def test_fewer_than_two_samples():
    assert hike_stats([]) == (0, 0, 0)
    assert hike_stats([1234]) == (0, 0, 0)
