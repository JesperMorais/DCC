def test_simple_lowercase_names():
    assert name_badge("ada", "lovelace") == "ADA L."


def test_strips_extra_spaces():
    assert name_badge("  grace ", " hopper") == "GRACE H."


def test_fixes_messy_capitals():
    assert name_badge("gUIDO", "van rossum") == "GUIDO V."


def test_no_last_name():
    assert name_badge("Linus", "") == "LINUS"


def test_last_name_of_only_spaces():
    assert name_badge(" linus ", "   ") == "LINUS"
