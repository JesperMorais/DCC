def test_removes_dashes_and_spaces():
    assert digits_only("070-123 45 67") == "0701234567"


def test_removes_plus_and_brackets():
    assert digits_only("+46 (0)70 123") == "46070123"


def test_no_digits_at_all():
    assert digits_only("call me") == ""


def test_empty_string():
    assert digits_only("") == ""


def test_already_clean_number_is_unchanged():
    assert digits_only("112") == "112"
