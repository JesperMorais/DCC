def test_three_apples():
    assert price_label("apple", 2.5, 3) == "3 x apple = 7.50"


def test_whole_price_still_gets_two_decimals():
    assert price_label("coffee", 4.0, 1) == "1 x coffee = 4.00"


def test_hides_float_noise():
    assert price_label("gum", 0.1, 3) == "3 x gum = 0.30"


def test_rounds_to_two_decimals():
    assert price_label("pen", 0.333, 3) == "3 x pen = 1.00"


def test_zero_quantity():
    assert price_label("bread", 29.9, 0) == "0 x bread = 0.00"
