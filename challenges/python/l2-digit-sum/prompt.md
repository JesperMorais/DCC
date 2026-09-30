Many ID numbers (credit cards, national IDs, ISBNs) use check digits built from the **sum of the digits**. Write `digit_sum(n)` that adds up all the digits of the whole number `n`.

- `digit_sum(1234)` → `10` (1 + 2 + 3 + 4)
- `digit_sum(7)` → `7`
- `digit_sum(0)` → `0`

For a negative number, ignore the minus sign: `digit_sum(-47)` → `11`.

For an extra challenge, do it with a `while` loop and arithmetic, without turning the number into a string.
