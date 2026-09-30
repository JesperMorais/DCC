The shop's orders store their status as loose strings, and typos like `"shiped"` keep slipping through. Replace them with an `Enum`.

1. `OrderStatus` is an `Enum` with these members **in this order**:

   | Member | Value |
   |---|---|
   | `PENDING` | `"pending"` |
   | `PAID` | `"paid"` |
   | `SHIPPED` | `"shipped"` |
   | `DELIVERED` | `"delivered"` |

2. `status.advance()` returns the **next** status in that order. Calling it on `DELIVERED` raises `ValueError` with a message containing `"final"`.
3. `parse_status(raw)` turns text from the database or a form into a member. It ignores case and surrounding whitespace. Unknown text raises `ValueError`.

```python
OrderStatus.PENDING.advance()   # OrderStatus.PAID
OrderStatus.DELIVERED.advance() # ValueError: delivered is final
parse_status("  Shipped ")      # OrderStatus.SHIPPED
parse_status("shiped")          # ValueError
```
