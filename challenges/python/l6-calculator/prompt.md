Build the engine of a pocket calculator. No `eval`, obviously.

```python
def calculate(expr: str) -> float
```

The starter already gives you a **tokenizer**: `tokenize("12*(3.5 + 4)")` returns `["12", "*", "(", "3.5", "+", "4", ")"]` and raises `ValueError` on any other character. It also gives you a shared cursor over the tokens, with `peek()` (look at the current token, `None` at the end) and `take()` (consume it, `ValueError` at the end). Your job is the **evaluator**.

It evaluates the tokens and returns a `float`:

- `*` and `/` bind tighter than `+` and `-`. Equal precedence is applied **left to right**.
- Parentheses group, and can nest.
- `/` is true division, and dividing by zero raises `ZeroDivisionError`.
- A malformed expression raises `ValueError`: empty input, a dangling or doubled operator (`"3 +"`, `"* 2"`, `"2 ** 3"`), two numbers in a row (`"3 4"`), unbalanced or empty parentheses (`"(1"`, `"1)"`, `"()"`). No unary minus.

```python
calculate("2 + 3 * 4")       # 14.0
calculate("(2 + 3) * 4")     # 20.0
calculate("10 - 2 - 3")      # 5.0   (left to right)
calculate("2 ** 3")          # ValueError
```
