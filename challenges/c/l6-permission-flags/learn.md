### Many booleans in one integer

When a thing has several independent yes/no properties, you can give each one its own bit and pack them all into a single integer. It's compact, cheap to copy and compare, and it's how Unix file modes, `open()` flags and hardware registers work.

```c
typedef enum {
    LED_RED   = 1u << 0,   // 0b001
    LED_GREEN = 1u << 1,   // 0b010
    LED_BLUE  = 1u << 2,   // 0b100
} Led;

unsigned lit = LED_RED | LED_BLUE;   // 0b101: two flags at once
```

Writing each value as `1u << k` makes it obvious that every flag owns exactly one bit and that no two overlap.

### Asking questions with masks

`&` keeps only the bits that are set in both operands, so `lit & LED_BLUE` is non-zero exactly when blue is on. With a mask of *several* flags there are two different questions:

- "Is at least one of them on?" means the masked result is non-zero.
- "Are all of them on?" means the masked result equals the whole mask.

Removing flags needs the complement: `~mask` has 1s everywhere *except* those flags, so `&`-ing with it clears them and leaves the rest alone.

A named "all flags" mask is also handy for **sanitizing** input: `&` with it throws away any bits you don't recognise.
