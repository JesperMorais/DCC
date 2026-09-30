A cash register prints one line per item on the receipt. Write `price_label(item, unit_price, quantity)` that returns a line in this exact format:

```
<quantity> x <item> = <total>
```

where the total is `unit_price * quantity`, always shown **rounded to exactly two decimals** (so `0.999` shows as `1.00`).

- `price_label("apple", 2.5, 3)` → `"3 x apple = 7.50"`
- `price_label("coffee", 4.0, 1)` → `"1 x coffee = 4.00"`
- `price_label("gum", 0.1, 3)` → `"3 x gum = 0.30"`

The last one is trickier than it looks. Try `0.1 * 3` in Python and see what you get!
