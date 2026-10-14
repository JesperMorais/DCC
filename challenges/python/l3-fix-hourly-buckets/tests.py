def test_always_returns_24_hours():
    assert len(events_by_hour([])) == 24
    assert len(events_by_hour([(3, "cron")])) == 24

def test_no_events_means_every_hour_is_empty():
    assert events_by_hour([]) == [[] for _ in range(24)]

def test_an_event_lands_in_its_own_hour_only():
    result = events_by_hour([(9, "deploy")])
    assert result[9] == ["deploy"]
    assert result[0] == []
    assert result[10] == []

def test_events_keep_their_order_within_an_hour():
    result = events_by_hour([(9, "deploy"), (14, "backup"), (9, "alert")])
    assert result[9] == ["deploy", "alert"]
    assert result[14] == ["backup"]

def test_first_and_last_hour():
    result = events_by_hour([(0, "midnight job"), (23, "nightly report")])
    assert result[0] == ["midnight job"]
    assert result[23] == ["nightly report"]
    assert sum(len(hour) for hour in result) == 2
