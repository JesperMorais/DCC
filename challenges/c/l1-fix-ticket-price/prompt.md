**Bug report from the cinema box office:** "A lady celebrating her 65th birthday was charged the full adult price. When we tried her husband, who is 70, he got the senior price."

`cinema_ticket_price` returns the price of one ticket, in kronor:

```c
int cinema_ticket_price(int age);
```

- Children **under 12** pay **60**.
- Seniors **65 or older** pay **80**.
- Everyone else pays **120**.

What it should do:

- `cinema_ticket_price(8)` → `60`
- `cinema_ticket_price(30)` → `120`
- `cinema_ticket_price(70)` → `80`

**Reproduction:** `cinema_ticket_price(65)` returns `120`, expected `80`.

Find the bug and fix it.
