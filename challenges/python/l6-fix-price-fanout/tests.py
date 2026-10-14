import asyncio


def supplier(prices, delays=None, default_delay=0.001):
    """A fake supplier API: answers after a per-SKU delay, KeyError if it doesn't stock the SKU."""
    async def fetch(sku):
        await asyncio.sleep((delays or {}).get(sku, default_delay))
        return prices[sku]
    return fetch


async def test_best_quote_picks_the_cheapest_supplier():
    agg = PriceAggregator({
        "Dahl": supplier({"hammer": 24_900}),
        "Byggmax": supplier({"hammer": 19_900}),
    })
    assert await agg.best_quote("hammer") == Quote("hammer", 19_900, "Byggmax")


async def test_slow_and_out_of_stock_suppliers_are_skipped():
    agg = PriceAggregator({
        "Slow": supplier({"saw": 1_000}, default_delay=1.0),
        "Empty": supplier({}),
        "Ok": supplier({"saw": 34_900}),
    }, timeout=0.05)
    assert await agg.best_quote("saw") == Quote("saw", 34_900, "Ok")
    assert await agg.best_quote("drill") is None


async def test_best_quotes_for_one_sku():
    agg = PriceAggregator({"Dahl": supplier({"nails": 4_500})})
    assert await agg.best_quotes(["nails"]) == {"nails": Quote("nails", 4_500, "Dahl")}


async def test_each_sku_gets_its_own_quote():
    agg = PriceAggregator({
        "Dahl": supplier(
            {"hammer": 24_900, "nails": 4_500, "glue": 8_900},
            delays={"hammer": 0.06, "nails": 0.002, "glue": 0.03},
        ),
    })
    quotes = await agg.best_quotes(["hammer", "nails", "glue"])
    assert quotes == {
        "hammer": Quote("hammer", 24_900, "Dahl"),
        "nails": Quote("nails", 4_500, "Dahl"),
        "glue": Quote("glue", 8_900, "Dahl"),
    }


async def test_basket_total_adds_the_best_prices():
    agg = PriceAggregator({
        "Dahl": supplier({"hammer": 24_900, "nails": 4_500}, delays={"hammer": 0.03}),
        "Byggmax": supplier({"hammer": 19_900, "nails": 5_000}),
    })
    assert await agg.basket_total(["hammer", "nails"]) == 24_400


async def test_basket_total_names_the_unavailable_sku():
    agg = PriceAggregator({
        "Dahl": supplier({"hammer": 24_900, "nails": 4_500}, delays={"hammer": 0.04, "nails": 0.02}),
    })
    with raises(LookupError, match="no supplier has: drill$"):
        await agg.basket_total(["hammer", "drill", "nails"])


def test_needs_at_least_one_supplier():
    with raises(ValueError):
        PriceAggregator({})
