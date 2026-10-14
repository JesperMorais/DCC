**Bug report from accounting:** "The revenue-per-day report doesn't add up. The days look right, but the sum of the daily totals is lower than the sum of all orders. It started after orders began arriving from the mobile app, which syncs late."

`daily_revenue(orders)` adds up `amount_cents` per calendar day of `placed_at`. Every order must be counted exactly once, whatever order the list is in. No orders gives `{}`.

```python
daily_revenue([
    Order("A1", datetime(2026, 10, 12, 9, 15), 12_000),
    Order("A2", datetime(2026, 10, 12, 17, 40), 3_500),
    Order("A3", datetime(2026, 10, 13, 11, 5), 8_000),
])
# {date(2026, 10, 12): 15500, date(2026, 10, 13): 8000}
```

**Reproduction:**

```python
daily_revenue([
    Order("C1", datetime(2026, 10, 12, 10, 0), 1_000),
    Order("C2", datetime(2026, 10, 13, 8, 30), 2_000),
    Order("C3", datetime(2026, 10, 12, 21, 10), 4_000),   # synced late
    Order("C4", datetime(2026, 10, 13, 12, 0), 500),
])
# returns  {date(2026, 10, 12): 4000, date(2026, 10, 13): 500}
# expected {date(2026, 10, 12): 5000, date(2026, 10, 13): 2500}
```
