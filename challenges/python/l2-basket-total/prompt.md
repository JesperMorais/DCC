A self-service till scans a basket of items and looks up each price in a price list (a `dict` from item name to price in kronor).

Write `basket_total(basket, prices)` that returns the total price of the basket.

- An item can be in the basket more than once. Count it every time.
- Items that **aren't in the price list** cost nothing (the till skips them).

```python
prices = {"milk": 15, "bread": 29, "cheese": 64}

basket_total(["milk", "bread"], prices)           # 44
basket_total(["milk", "milk", "cheese"], prices)  # 94
basket_total(["milk", "unicorn"], prices)         # 15
```
