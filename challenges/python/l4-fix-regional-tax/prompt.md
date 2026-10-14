**Bug report from finance:** "Since we started selling in Germany and Switzerland, every invoice is charged Swiss VAT, even the Swedish ones. Our unit tests passed, but they only ever set up one country at a time."

`make_tax_calculators(rates)` takes a VAT rate per region and returns a **function per region** that turns a net price in cents into the gross price (rounded to whole cents). A rate must be in `0 <= rate < 1`, otherwise `ValueError` naming the region. `gross_total(lines, calculators)` adds up `(region, net_cents)` lines.

```python
calc = make_tax_calculators({"SE": 0.25, "DE": 0.19})
calc["SE"](10_000)   # 12500
calc["DE"](10_000)   # 11900
gross_total([("SE", 2_000), ("DE", 5_000)], calc)   # 2500 + 5950 = 8450
```

**Reproduction:**

```python
calc = make_tax_calculators({"SE": 0.25, "DE": 0.19, "CH": 0.081})
calc["SE"](10_000)   # returns 10810, expected 12500
```
