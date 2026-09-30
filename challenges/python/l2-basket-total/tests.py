PRICES = {"milk": 15, "bread": 29, "cheese": 64}


def test_adds_up_two_items():
    assert basket_total(["milk", "bread"], PRICES) == 44


def test_counts_repeated_items_every_time():
    assert basket_total(["milk", "milk", "cheese"], PRICES) == 94


def test_unknown_items_cost_nothing():
    assert basket_total(["milk", "unicorn"], PRICES) == 15


def test_empty_basket_is_free():
    assert basket_total([], PRICES) == 0


def test_empty_price_list():
    assert basket_total(["milk", "bread"], {}) == 0
