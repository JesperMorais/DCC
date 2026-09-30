def test_toddlers_are_free():
    assert ticket_price(0) == 0
    assert ticket_price(2) == 0


def test_children_pay_60():
    assert ticket_price(3) == 60
    assert ticket_price(10) == 60


def test_twelve_is_still_a_child_but_thirteen_is_not():
    assert ticket_price(12) == 60
    assert ticket_price(13) == 120


def test_adults_pay_120():
    assert ticket_price(30) == 120
    assert ticket_price(64) == 120


def test_seniors_pay_80():
    assert ticket_price(65) == 80
    assert ticket_price(101) == 80
