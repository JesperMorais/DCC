### Undefined behavior isn't just a wrong answer

In C, overflowing a **signed** integer is *undefined behavior* (UB). The standard doesn't say "it wraps around". It says nothing at all, and optimizers rely on that. Because a correct program can never overflow, the compiler is allowed to assume yours doesn't:

```c
bool will_wrap(int x) {
    return x + 1 < x;   // "can't be true", so gcc may compile this to `return false;`
}
```

An overflow check written after the fact can be deleted by the compiler. That's why these checks have to happen *before* the operation, using arithmetic that provably stays in range. (Unsigned arithmetic is different: it's defined to wrap modulo 2ⁿ.)

### Check with the limits, not the result

`<limits.h>` gives you `INT_MAX` and `INT_MIN`. The trick is to rearrange the condition you care about so the risky operation disappears. For example, "would `x * 2` be too big?" is the same question as `x > INT_MAX / 2` when `x ≥ 0`, and the division can't overflow.

Two's-complement ranges are **asymmetric**: `INT_MIN` is `-INT_MAX - 1`, so `-INT_MIN` doesn't fit. Watch out for "just negate it" shortcuts.
