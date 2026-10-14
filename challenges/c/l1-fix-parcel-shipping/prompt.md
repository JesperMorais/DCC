**Bug report from the warehouse:** "We just shipped a 12 kg box of books for 79 kr. The courier bills us 129 for anything that heavy, so we lose money on every big order. Small parcels are priced correctly."

`parcel_shipping_cost` returns the shipping price in kronor for a parcel weighing `grams`:

```c
int parcel_shipping_cost(int grams);
```

| Weight | Price |
|---|---|
| up to and including 1000 g | 49 |
| more than 1000 g, up to and including 5000 g | 79 |
| more than 5000 g | 129 |

What it should do:

- `parcel_shipping_cost(400)` → `49`
- `parcel_shipping_cost(3000)` → `79`
- `parcel_shipping_cost(5001)` → `129`

**Reproduction:** `parcel_shipping_cost(12000)` returns `79`, expected `129`.

Find the bug and fix it.
