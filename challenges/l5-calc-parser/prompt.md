Write **`evaluate(expr: string): number`** — a tiny calculator for integer arithmetic.

- Supports non-negative integers (`0`, `7`, `42`, `1000`) and the operators `+ - * /`.
- `*` and `/` bind tighter than `+` and `-`. Operators of equal precedence are applied **left to right**.
- Whitespace anywhere is ignored.
- `/` is normal JavaScript division (it may produce a fraction).
- Throw an `Error` if the input contains an unknown character, or if it isn't a valid alternation of number, operator, number, … (e.g. `""`, `"3 +"`, `"* 2"`, `"3 4"`).

No parentheses and no unary minus — keep it small. And no `eval` / `Function`, of course.

```ts
evaluate("3 + 4 * 2");   // 11
evaluate("10 - 2 - 3");  // 5   (left to right, not 10 - (2 - 3))
evaluate("2*3+4*5");     // 26
evaluate("3 + x");       // throws
```
