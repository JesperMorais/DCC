The city museum prices its tickets by age (prices in kronor):

| Age | Price |
|---|---|
| 0 – 2 | `0` (free) |
| 3 – 12 | `60` |
| 13 – 64 | `120` |
| 65 and older | `80` |

Write `ticket_price(age)` that returns the price as a whole number.

- `ticket_price(2)` → `0`
- `ticket_price(10)` → `60`
- `ticket_price(30)` → `120`
- `ticket_price(70)` → `80`

Watch the edges: a 12-year-old still pays the child price, and a 13-year-old pays the full price.
