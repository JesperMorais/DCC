def test_two_hours_five_minutes():
    assert format_duration(125) == "2:05"


def test_less_than_an_hour():
    assert format_duration(45) == "0:45"
    assert format_duration(59) == "0:59"


def test_exactly_one_hour():
    assert format_duration(60) == "1:00"


def test_zero_minutes():
    assert format_duration(0) == "0:00"


def test_hours_keep_growing_past_24():
    assert format_duration(1500) == "25:00"
