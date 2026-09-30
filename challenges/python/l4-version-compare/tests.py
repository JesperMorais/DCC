def test_from_string_parses_the_parts():
    v = Version.from_string("1.10.3")
    assert isinstance(v, Version)
    assert (v.major, v.minor, v.patch) == (1, 10, 3)
    assert Version.from_string("v2.0.1") == Version(2, 0, 1)

def test_compares_numerically_not_as_text():
    assert Version.from_string("1.10.0") > Version.from_string("1.9.2")
    assert Version(1, 9, 2) < Version(1, 10, 0)
    assert Version(2, 0, 0) > Version(1, 99, 99)

def test_equality_and_all_comparison_operators():
    assert Version(1, 2, 3) == Version(1, 2, 3)
    assert Version(1, 2, 3) != Version(1, 2, 4)
    assert Version(1, 2, 3) <= Version(1, 2, 3)
    assert Version(1, 2, 3) >= Version(1, 2, 3)
    assert Version(0, 1, 0) <= Version(0, 2, 0)

def test_is_never_equal_to_other_types():
    assert Version(1, 0, 0) != "1.0.0"
    assert not (Version(1, 0, 0) == (1, 0, 0))

def test_sorts_and_has_a_readable_repr():
    versions = [Version.from_string(s) for s in ["1.10.0", "1.2.0", "0.9.9", "1.9.2"]]
    assert sorted(versions) == [Version(0, 9, 9), Version(1, 2, 0), Version(1, 9, 2), Version(1, 10, 0)]
    assert max(versions) == Version(1, 10, 0)
    assert repr(Version(1, 10, 0)) == "Version('1.10.0')"

def test_rejects_invalid_strings():
    for bad in ["1.2", "1.2.3.4", "1.x.3", "", "1..3", "1.-2.3", "V1.2.3"]:
        with raises(ValueError, match="invalid version"):
            Version.from_string(bad)
