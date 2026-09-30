### Tokenize, then parse

Nearly every language tool — compilers, JSON parsers, template engines — works in two stages:

1. **Tokenizer (lexer):** characters → a flat list of meaningful pieces. `"12 + 3"` becomes `[num 12, op +, num 3]`. Whitespace disappears here.
2. **Parser/evaluator:** tokens → meaning, respecting the grammar's rules.

Keeping them separate means neither stage has to think about the other's problems.

### Model tokens as a discriminated union

```ts
type Tok =
  | { kind: "num"; value: number }
  | { kind: "op"; op: "+" | "-" };

function show(t: Tok) {
  return t.kind === "num" ? String(t.value) : t.op;  // narrowed per branch
}
```

The `kind` field makes every later `if`/`switch` type-safe: you can't read `.value` off an operator by accident.

### Reading multi-digit numbers

A common tokenizer loop keeps an index `i` and, when it sees a digit, keeps advancing while the next char is also a digit:

```ts
let j = i;
while (j < s.length && s[j] >= "0" && s[j] <= "9") j++;
const digits = s.slice(i, j);
```

### Precedence

"Tighter binding" means `*` and `/` must be resolved before `+`/`-` sees their results — think of `3 + 4 * 2` as `3 + (4 * 2)`.
