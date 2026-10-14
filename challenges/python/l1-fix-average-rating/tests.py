def test_an_even_average():
    assert average_rating(10, 2) == approx(5.0)

def test_half_stars_are_kept():
    assert average_rating(9, 2) == approx(4.5)

def test_no_reviews_means_zero():
    assert average_rating(0, 0) == approx(0.0)

def test_a_single_review():
    assert average_rating(3, 1) == approx(3.0)

def test_an_uneven_average():
    assert average_rating(14, 3) == approx(4.6667, rel=1e-3)
