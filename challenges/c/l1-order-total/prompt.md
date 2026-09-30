A small web shop stores every price in **whole cents** (so €2.50 is `250`). That way the numbers are always whole and nothing gets lost to rounding.

Write `order_total`, which works out what a customer pays at checkout:

```c
int order_total(int quantity, int unit_price, int shipping);
```

- `quantity` is how many of the item they bought.
- `unit_price` is the price of **one** item, in cents.
- `shipping` is a flat fee in cents. It's added once per order, **even if `quantity` is 0**.

Examples:

- `order_total(3, 250, 49)` → `799` (3 × 250 = 750, plus 49 shipping)
- `order_total(1, 1999, 0)` → `1999` (free shipping)
- `order_total(0, 500, 49)` → `49`
