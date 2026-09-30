def test_fifteen_percent_of_100():
    assert bill_with_tip(100.0, 15.0) == approx(115.0)


def test_twenty_percent_of_40():
    assert bill_with_tip(40.0, 20.0) == approx(48.0)


def test_no_tip_means_just_the_bill():
    assert bill_with_tip(25.0, 0.0) == approx(25.0)


def test_a_fractional_percentage():
    assert bill_with_tip(80.0, 12.5) == approx(90.0)


def test_an_uneven_bill():
    assert bill_with_tip(19.99, 18.0) == approx(23.5882)


def test_an_empty_bill_stays_zero():
    assert bill_with_tip(0.0, 20.0) == approx(0.0)
