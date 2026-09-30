### `bool`: true or false

A **type** tells C what kind of value a variable or function holds. `bool` is the type for answers that are just yes or no: its only values are `true` and `false`. It comes from a header you include at the top:

```c
#include <stdbool.h>
```

A comparison such as `age >= 18` already *is* a `bool`, so you can return it directly:

```c
bool is_adult(int age) {
    return age >= 18;
}
```

### Combining conditions

- `&&` means **and**: true only if both sides are true.
- `||` means **or**: true if at least one side is true.
- `!` means **not**: it flips true and false.

```c
bool can_ride(int age, int height_cm, bool with_parent) {
    return height_cm >= 120 && (age >= 8 || with_parent);
}
```

Parentheses group things just like in maths. Here the height rule always applies, and then *either* age *or* a parent will do.

### "Divisible by"

`n % d` is the remainder after dividing `n` by `d`, so `n % d == 0` means "`d` divides `n` evenly": `15 % 5 == 0` is `true`, and `15 % 4 == 0` is `false`.

When rules have exceptions, it often helps to check the **most specific** rule first.
