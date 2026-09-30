def test_counts_each_option():
    assert tally_votes(["pizza", "tacos", "pizza"]) == {"pizza": 2, "tacos": 1}


def test_no_votes_gives_an_empty_dict():
    assert tally_votes([]) == {}


def test_a_single_vote():
    assert tally_votes(["curry"]) == {"curry": 1}


def test_everyone_agrees():
    assert tally_votes(["salad"] * 5) == {"salad": 5}


def test_blank_votes_are_not_counted():
    assert tally_votes(["sushi", "", "sushi"]) == {"sushi": 2}
    assert tally_votes(["", ""]) == {}
