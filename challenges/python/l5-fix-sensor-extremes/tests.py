def test_finds_the_coldest_and_warmest_reading():
    stats = reading_stats([4.5, -2.0, 7.25, 3.0])
    assert stats == ReadingStats(count=4, missing=0, low=-2.0, high=7.25)


def test_gaps_are_counted_and_skipped():
    stats = reading_stats([None, 12.0, None, 9.5])
    assert stats == ReadingStats(count=2, missing=2, low=9.5, high=12.0)


def test_a_single_reading_is_both_low_and_high():
    stats = reading_stats([18.0])
    assert (stats.low, stats.high) == (18.0, 18.0)


def test_a_sensor_that_was_offline_all_day():
    stats = reading_stats([None, None, None])
    assert stats == ReadingStats(count=0, missing=3, low=None, high=None)


def test_no_readings_at_all():
    assert reading_stats([]) == ReadingStats(count=0, missing=0, low=None, high=None)
