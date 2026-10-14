def test_removes_unsubscribed_readers():
    assert active_subscribers(["bo@mail.se", "cy@mail.se"], ["cy@mail.se"]) == ["bo@mail.se"]

def test_each_reader_appears_once_as_first_typed():
    assert active_subscribers(["Bo@mail.se", " bo@MAIL.se", "cy@mail.se"], []) == ["Bo@mail.se", "cy@mail.se"]

def test_no_signups_gives_an_empty_list():
    assert active_subscribers([], ["x@y.z"]) == []

def test_unsubscribing_ignores_case():
    signups = ["Ana@Example.com", "bo@mail.se"]
    assert active_subscribers(signups, ["Ana@Example.com"]) == ["bo@mail.se"]

def test_unsubscribing_ignores_case_and_spaces_on_both_sides():
    signups = [" dee@mail.se", "eve@mail.se"]
    assert active_subscribers(signups, ["  DEE@mail.se "]) == ["eve@mail.se"]
