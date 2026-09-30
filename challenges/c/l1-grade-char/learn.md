### A type for single letters

So far you've seen `int` (whole numbers) and maybe `double` (decimals). C has a third basic **type**, `char`, for **one** character. A `char` literal is written in **single** quotes:

```c
char initial = 'J';
```

Double quotes (`"J"`) mean something different in C: a *string*, which is a whole row of characters. You'll meet those later. For now, one letter means one `char` and single quotes.

### Many choices: `else if`

When there are more than two possibilities, chain the checks. C tries them **from top to bottom** and runs only the **first** one that's true:

```c
char shirt_size(int chest_cm) {
    if (chest_cm >= 110) {
        return 'L';
    } else if (chest_cm >= 95) {
        return 'M';
    } else {
        return 'S';
    }
}
```

`shirt_size(100)` returns `'M'`. The second check doesn't need to say "and less than 110", because if we got that far, the first check already failed. **Order matters!**

### `return` ends the function

As soon as a function reaches `return`, it's done. Nothing after it runs.
