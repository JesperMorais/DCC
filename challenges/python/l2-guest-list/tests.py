def test_adds_a_new_guest_to_the_list():
    guests = ["Ada", "Linus"]
    assert add_guest(guests, "Grace", 3) == "added"
    assert guests == ["Ada", "Linus", "Grace"]


def test_adds_the_first_guest_to_an_empty_list():
    guests: list[str] = []
    assert add_guest(guests, "Ada", 5) == "added"
    assert guests == ["Ada"]


def test_does_not_add_someone_twice():
    guests = ["Ada", "Linus"]
    assert add_guest(guests, "Ada", 5) == "already invited"
    assert guests == ["Ada", "Linus"]


def test_refuses_when_the_list_is_full():
    guests = ["Ada", "Linus", "Grace"]
    assert add_guest(guests, "Guido", 3) == "full"
    assert guests == ["Ada", "Linus", "Grace"]


def test_already_invited_wins_over_full():
    guests = ["Ada", "Linus"]
    assert add_guest(guests, "Linus", 2) == "already invited"


def test_capacity_zero_is_always_full():
    guests: list[str] = []
    assert add_guest(guests, "Ada", 0) == "full"
    assert guests == []
