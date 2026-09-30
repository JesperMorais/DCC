People type phone numbers in all sorts of ways: `070-123 45 67`, `+46 (0)70 123`, … Before saving one, a contacts app keeps **only the digits**.

Write `digits_only(phone)` that returns a new string with every character that isn't a digit (`0`–`9`) removed. Keep the digits in their original order.

- `digits_only("070-123 45 67")` → `"0701234567"`
- `digits_only("+46 (0)70 123")` → `"46070123"`
- `digits_only("call me")` → `""`
