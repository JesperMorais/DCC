**Bug report from the warehouse floor**

> The first delivery of a new SKU shows up in the system, but every delivery after that vanishes. We received 40 more `BOLT-M6` this morning and the count didn't move.

The warehouse keeps up to `MAX_ITEMS` stock items, each with a SKU and a quantity.

- `stock_receive(w, sku, qty)` adds `qty` to that SKU's quantity. If the SKU is new it's appended with quantity `qty`. It returns `false` (changing nothing) when the SKU is new and the warehouse is full, or the SKU is too long to store; otherwise `true`.
- `stock_ship(w, sku, qty)` removes `qty` units and returns `true`, or returns `false` (changing nothing) if the SKU is unknown or there aren't enough units.
- `stock_quantity(w, sku)` returns the quantity, or `0` for an unknown SKU.

```c
Warehouse w = {0};
stock_receive(&w, "BOLT-M6", 100);   // true, BOLT-M6: 100
stock_ship(&w, "BOLT-M6", 30);       // true, BOLT-M6: 70
stock_ship(&w, "BOLT-M6", 500);      // false, still 70
```

**Reproduction**

```c
Warehouse w = {0};
stock_receive(&w, "BOLT-M6", 100);
stock_receive(&w, "BOLT-M6", 40);
stock_quantity(&w, "BOLT-M6");   // returns 100, expected 140
```
