def test_groups_messages_by_level():
    lines = ["INFO: server started", "ERROR: db timeout", "INFO: ready"]
    assert group_by_level(lines) == {
        "INFO": ["server started", "ready"],
        "ERROR": ["db timeout"],
    }

def test_levels_are_normalised():
    assert group_by_level(["warn : disk full", " Warn:cpu hot", "WARN: fan"]) == {
        "WARN": ["disk full", "cpu hot", "fan"],
    }

def test_splits_at_the_first_colon_only():
    assert group_by_level(["ERROR: retry failed: code=5"]) == {"ERROR": ["retry failed: code=5"]}

def test_skips_lines_without_a_colon():
    assert group_by_level(["garbage line", "", "DEBUG: x=1"]) == {"DEBUG": ["x=1"]}

def test_levels_appear_in_first_seen_order():
    result = group_by_level(["ERROR: a", "INFO: b", "ERROR: c", "DEBUG: d"])
    assert list(result) == ["ERROR", "INFO", "DEBUG"]

def test_returns_a_plain_dict():
    result = group_by_level(["INFO: hi"])
    assert type(result) is dict
    assert group_by_level([]) == {}
