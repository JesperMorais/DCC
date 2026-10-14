**Bug report from the garage owner:** "A customer parked for 3 hours and the machine asked for 45 kr. They should have paid 15. Short visits look fine."

`garage_parking_fee` works out what a driver pays when they leave:

```c
int garage_parking_fee(int hours);
```

- The **first 2 hours are free**.
- Every hour **after** those 2 costs **15 kr**.
- `hours` is a whole number, 0 or more.

What it should do:

- `garage_parking_fee(1)` → `0`
- `garage_parking_fee(2)` → `0`
- `garage_parking_fee(4)` → `30` (2 free hours, then 2 paid hours × 15)

**Reproduction:** `garage_parking_fee(3)` returns `45`, expected `15`.

Find the bug and fix it.
