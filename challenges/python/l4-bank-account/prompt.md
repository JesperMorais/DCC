Build a small `Account` class for a banking app. Money is stored in **cents** (an `int`) to avoid floating-point errors.

- `Account(owner: str)` starts with a balance of `0`.
- `deposit(amount)` adds money. Returns the new balance.
- `withdraw(amount)` removes money. Returns the new balance.
- `balance` is a **read-only property**: `acct.balance` works, `acct.balance = 100` must fail.

Reject bad input by raising `ValueError`, and leave the balance unchanged when you do:

| Situation | Message must contain |
|---|---|
| amount is 0 or negative | `"positive"` |
| withdrawing more than the balance | `"insufficient funds"` |

```python
acct = Account("Ada")
acct.deposit(500)   # 500
acct.withdraw(200)  # 300
acct.withdraw(999)  # ValueError: insufficient funds
```
