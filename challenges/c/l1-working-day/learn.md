### Yes-or-no values: `bool`

Some answers are just "yes" or "no". C has a **type** for that, `bool`, whose only values are `true` and `false`. It lives in a small library header, so you switch it on at the top of the file:

```c
#include <stdbool.h>
```

### `switch`: pick a branch by value

When you're comparing **one** value against a list of exact numbers, a `switch` reads better than a long `if`/`else if` chain:

```c
int medal_points(int place) {
    switch (place) {
        case 1:
            return 10;
        case 2:
            return 6;
        case 3:
            return 4;
        default:
            return 0;   // everything else: no medal
    }
}
```

- `switch (place)` looks at the value of `place` once.
- It jumps to the `case` with that exact value, or to `default` if none match.
- `return` leaves the function right away. (Outside a function-ending `return`, you'd write `break;` to leave the `switch`.)

### Sharing code between cases

Labels can be stacked, so several values run the same code:

```c
switch (month) {
    case 6:
    case 7:
    case 8:
        return true;   // summer
    default:
        return false;
}
```
