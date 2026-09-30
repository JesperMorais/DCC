### Stage 1: tokens (done for you)

Parsing characters directly is painful. First turn the text into **tokens**, meaningful chunks like `"12"`, `"*"`, `"("`. A regex with alternatives does most of the work, and `\S` catches "any other non-space character" so you can reject it. The starter's `tokenize` works exactly like this:

```python
re.findall(r"[a-z]+|\S", "ab + cd!")   # ["ab", "+", "cd", "!"]
```

### Stage 2: a grammar, one function per rule

Precedence is naturally expressed as a **grammar** where each level is built from the next-tighter level. Here is one for boolean expressions, where `and` binds tighter than `or`:

```text
disjunction := conjunction ("or" conjunction)*
conjunction := atom ("and" atom)*
atom        := "true" | "false" | "(" disjunction ")"
```

In a **recursive-descent parser**, each rule becomes a function that consumes tokens from a shared position and returns a value:

```python
def disjunction() -> bool:
    value = conjunction()
    while peek() == "or":
        take()
        rhs = conjunction()          # always evaluate: this consumes tokens!
        value = value or rhs
    return value
```

- **Precedence** comes from *who calls whom*: `disjunction` only ever combines finished `conjunction`s.
- **Left-to-right** comes from the `while` loop that folds into `value` as it goes.
- **Parentheses** are where the recursion closes the loop: `atom` calls back up to the top rule.

Keep the position in one place (a closure variable with `nonlocal`, or an attribute on a small class) behind two helpers: `peek()` looks at the current token without consuming it, and `take()` consumes it and fails cleanly at the end of the input. The starter has both.

At the end, check that **every** token was consumed. Otherwise `"3 4"` would quietly return `3`.
