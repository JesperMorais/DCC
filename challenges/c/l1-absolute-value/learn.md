### Functions in C

C makes you state the type of **everything**: what goes in, and what comes out.

```c
int twice(int n) {
    return n * 2;
}
```

- the first `int` is the **return type**
- `int n` is the **parameter**: a whole number, called `n` inside the function
- `{ ... }` is the body, and every statement ends with `;`

### Making decisions

```c
if (temperature < 0) {
    return 1;   // freezing
} else {
    return 0;
}
```

The condition goes in parentheses. `<`, `>`, `<=`, `>=`, `==` (equal) and `!=` (not equal) compare numbers.

There's no `main()` here: daily.ts compiles your function together with the tests, and the tests call it.
