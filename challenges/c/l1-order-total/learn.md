### What's a function?

A **function** is a small named machine: you give it some inputs, it does a calculation, and it hands back one result. Here's one that turns days into minutes:

```c
int minutes_in(int days) {
    int hours = days * 24;
    return hours * 60;
}
```

Read it like this:

- `int minutes_in(...)`: the function is called `minutes_in` and gives back an `int` (a whole number).
- `int days`: the input, called a **parameter**. Inside the function, `days` is whatever number the caller passed in.
- `int hours = days * 24;` creates a **variable**, a named box that holds a value, and puts `days * 24` in it.
- `return` hands the answer back and ends the function.

`minutes_in(2)` gives back `2880`.

### Why does C want all those `int`s?

A computer stores everything as bytes. C needs to know the **type** of each value (whole number, decimal number, letter…) so it knows how many bytes to reserve and how to do maths with them. `int` means "a whole number, possibly negative". You'll meet other types soon.

### Arithmetic

`+`, `-`, `*` and `/` work as you'd expect, and `*` and `/` go before `+` and `-`. Use parentheses when you want a different order: `(2 + 3) * 4` is `20`.
