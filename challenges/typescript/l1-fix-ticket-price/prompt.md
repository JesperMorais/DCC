**Bug report from the cinema box office**

> A man came to the counter on his 65th birthday, proud to finally get the senior price. The till charged him the full 10. He was not amused.

`ticketPrice(age)` returns the price of one cinema ticket:

| Age          | Price |
|--------------|-------|
| under 12     | `5`   |
| 12 – 64      | `10`  |
| 65 or older  | `7`   |

- `ticketPrice(8)` → `5`
- `ticketPrice(30)` → `10`
- `ticketPrice(70)` → `7`

**Reproduction:** `ticketPrice(65)` returns `10`, expected `7`.

Find the bug and fix it.
