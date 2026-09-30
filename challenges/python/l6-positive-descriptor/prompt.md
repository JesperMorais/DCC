Every model in your shop has fields like `price` and `weight` that must be positive numbers. Instead of a pile of copy-pasted properties, write one reusable **descriptor**, `Positive`:

```python
class Product:
    price = Positive()
    weight = Positive()

    def __init__(self, name: str, price: float, weight: float) -> None:
        self.name = name
        self.price = price      # validated by Positive
        self.weight = weight
```

- Assigning an `int` or `float` greater than zero stores it **per instance**, and reading gives it back.
- Assigning a number `<= 0` raises `ValueError` with a message containing `"<attr> must be positive"`, e.g. `"price must be positive, got -1"`. The previous value stays.
- Assigning a non-number (including `bool`, which is technically an `int`) raises `TypeError` containing `"<attr> must be a number"`.
- Messages use the **attribute's own name**, so the same class works for `price`, `weight`, …, without passing the name in by hand.
- `Product.price` (accessed on the class) returns the `Positive` descriptor itself.
- Reading an attribute that was never set raises `AttributeError`.

```python
p = Product("kettle", 249, 1.2)
p.price = 0      # ValueError: price must be positive, got 0
p.weight = "1"   # TypeError: weight must be a number, got '1'
```
