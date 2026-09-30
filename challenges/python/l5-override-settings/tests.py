def test_applies_changes_inside_the_block():
    flags = {"dark_mode": False}
    with override(flags, dark_mode=True, new_ui=True) as f:
        assert f is flags
        assert flags == {"dark_mode": True, "new_ui": True}


def test_restores_old_values_and_removes_new_keys():
    flags = {"dark_mode": False, "beta": None}
    with override(flags, dark_mode=True, beta=True, new_ui=True):
        pass
    assert flags == {"dark_mode": False, "beta": None}


def test_restores_even_when_the_block_raises():
    flags = {"dark_mode": False}
    with raises(RuntimeError, match="boom"):
        with override(flags, dark_mode=True, new_ui=True):
            raise RuntimeError("boom")
    assert flags == {"dark_mode": False}


def test_edits_to_other_keys_are_kept():
    flags = {"dark_mode": False, "lang": "en"}
    with override(flags, dark_mode=True):
        flags["lang"] = "sv"
    assert flags == {"dark_mode": False, "lang": "sv"}


def test_nested_overrides_unwind_in_order():
    flags = {"level": 1}
    with override(flags, level=2):
        with override(flags, level=3):
            assert flags["level"] == 3
        assert flags["level"] == 2
    assert flags == {"level": 1}


def test_no_changes_is_a_no_op():
    flags = {"a": 1}
    with override(flags):
        assert flags == {"a": 1}
    assert flags == {"a": 1}
