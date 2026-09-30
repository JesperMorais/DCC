A bank's nightly job scans account balances (in cents) for the first overdrawn account, so a clerk can inspect it and fix it. Write `find_first_negative`:

```c
int *find_first_negative(int *balances, size_t count);
```

- Return a **pointer to the first element that is below zero**, pointing *into the caller's array*, so the caller can read it, work out its index, or overwrite it.
- If there's no negative balance, or `count` is 0, or `balances` is `NULL`, return `NULL`.
- Only look at the first `count` elements.

```c
int balances[] = {1500, 0, -250, 800, -40};
int *p = find_first_negative(balances, 5);
// p == &balances[2], *p == -250, p - balances == 2
*p = 0;   // the clerk writes it off: balances[2] is now 0
```
