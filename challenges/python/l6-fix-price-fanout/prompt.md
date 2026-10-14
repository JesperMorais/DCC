**Bug report from the purchasing team:** "The price comparison page is fast now, great, but it's wrong. The hammer shows up at 45 kr and the glue at 249 kr. The basket total at the bottom is somehow correct. And yesterday it said the *hammer* was out of stock everywhere, when it was the drill."

`PriceAggregator` asks several supplier APIs for prices concurrently:

- `best_quote(sku)` asks every supplier at once and returns the cheapest `Quote(sku, price_cents, supplier)`. Suppliers that time out or don't stock the item (`LookupError`) are skipped. No answers at all gives `None`.
- `best_quotes(skus)` looks up all SKUs **concurrently** and returns `{sku: best quote or None}`.
- `basket_total(skus)` sums the best prices, or raises `LookupError("no supplier has: …")` listing the SKUs nobody stocks.
- At least one supplier is required (`ValueError`).

```python
agg = PriceAggregator({"Dahl": dahl_api, "Byggmax": byggmax_api})
await agg.best_quote("hammer")             # Quote("hammer", 19900, "Byggmax")
await agg.best_quotes(["nails"])           # {"nails": Quote("nails", 4500, "Dahl")}
await agg.basket_total(["hammer", "nails"])  # 24400
```

**Reproduction:** one supplier, "Dahl", stocks hammer (24 900), nails (4 500) and glue (8 900). Its API answers fastest for nails and slowest for the hammer.

```python
await agg.best_quotes(["hammer", "nails", "glue"])
# returns  {"hammer": Quote("nails", 4500, "Dahl"), "nails": Quote("glue", 8900, "Dahl"), "glue": Quote("hammer", 24900, "Dahl")}
# expected {"hammer": Quote("hammer", 24900, "Dahl"), "nails": Quote("nails", 4500, "Dahl"), "glue": Quote("glue", 8900, "Dahl")}
```
