RATES = {"SE": 0.25, "DE": 0.19, "CH": 0.081}


def test_a_single_region_adds_its_tax():
    calc = make_tax_calculators({"SE": 0.25})
    assert calc["SE"](10_000) == 12_500


def test_every_region_uses_its_own_rate():
    calc = make_tax_calculators(RATES)
    assert calc["SE"](10_000) == 12_500
    assert calc["DE"](10_000) == 11_900
    assert calc["CH"](10_000) == 10_810


def test_gross_total_mixes_regions():
    calc = make_tax_calculators(RATES)
    lines = [("SE", 2_000), ("DE", 5_000), ("SE", 400)]
    assert gross_total(lines, calc) == 2_500 + 5_950 + 500


def test_zero_rate_leaves_prices_unchanged():
    calc = make_tax_calculators({"HK": 0.0, "SE": 0.25})
    assert calc["HK"](999) == 999


def test_rejects_impossible_rates():
    with raises(ValueError, match="DE"):
        make_tax_calculators({"SE": 0.25, "DE": 19.0})
    with raises(ValueError, match="XX"):
        make_tax_calculators({"XX": -0.1})
