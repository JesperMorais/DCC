def test_picks_the_highest_of_similar_bids():
    assert highest_bid(["15", " 40", "25 "]) == 40

def test_no_bids_gives_zero():
    assert highest_bid([]) == 0
    assert highest_bid(["", "  "]) == 0

def test_a_single_bid():
    assert highest_bid([" 75 "]) == 75

def test_a_bigger_number_wins_even_with_a_smaller_first_digit():
    assert highest_bid(["9", "120"]) == 120

def test_more_digits_wins():
    assert highest_bid(["300", "1000", "95"]) == 1000
