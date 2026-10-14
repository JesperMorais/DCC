def test_a_small_order_pays_full_shipping():
    assert shipping_fee(12.0) == approx(7.99)

def test_a_medium_order_pays_reduced_shipping():
    assert shipping_fee(35.0) == approx(4.99)

def test_exactly_20_gets_reduced_shipping():
    assert shipping_fee(20.0) == approx(4.99)

def test_a_big_order_ships_free():
    assert shipping_fee(80.0) == approx(0.0)

def test_exactly_50_ships_free():
    assert shipping_fee(50.0) == approx(0.0)

def test_an_empty_order_pays_full_shipping():
    assert shipping_fee(0.0) == approx(7.99)
