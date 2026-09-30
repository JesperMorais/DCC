import dataclasses

def test_is_a_dataclass_with_an_empty_default():
    assert dataclasses.is_dataclass(Playlist)
    assert Playlist("Focus").tracks == []

def test_each_playlist_has_its_own_list():
    a, b = Playlist("A"), Playlist("B")
    a.add(60)
    assert a.tracks == [60]
    assert b.tracks == []

def test_add_and_duration():
    p = Playlist("Focus")
    p.add(185)
    p.add(200)
    assert p.tracks == [185, 200]
    assert p.duration() == "6:25"

def test_duration_pads_seconds_and_does_not_cap_minutes():
    assert Playlist("Empty").duration() == "0:00"
    assert Playlist("Short", [5]).duration() == "0:05"
    assert Playlist("Long", [3600, 1203]).duration() == "80:03"

def test_equality_and_repr_come_from_the_dataclass():
    assert Playlist("Mix", [30, 30]) == Playlist("Mix", [30, 30])
    assert Playlist("Mix", [30]) != Playlist("Mix", [31])
    assert repr(Playlist("Mix", [30])) == "Playlist(name='Mix', tracks=[30])"
