**Bug report from support:** "A customer called, furious: they opened the grocery app, added one bag of coffee, and the checkout said 4 items. The tea and bread in there weren't theirs. Another customer reports the same thing. It's like the carts are… mixing?"

`GroceryCart` is supposed to work like this:

- `GroceryCart(customer)` starts **empty**.
- `add(name, price_cents, quantity=1)` puts a product in the cart. Adding a product that's already there increases that line's quantity instead of adding a new line. A quantity below 1 raises `ValueError`.
- `item_count()` is the total number of units, `total_cents()` the total price.

```python
cart = GroceryCart("Ada")
cart.add("milk", 1490, 2)
cart.add("bread", 3250)
cart.item_count()    # 3
cart.total_cents()   # 6230
```

**Reproduction:**

```python
ada = GroceryCart("Ada")
ada.add("coffee", 5900)
linus = GroceryCart("Linus")
linus.total_cents()   # returns 5900, expected 0
```
