### Comparing numbers

An `if` runs its block only when its **condition** is true. Conditions compare numbers:

| You write | It means | True for 18? |
|---|---|---|
| `age < 18` | less than 18 | no |
| `age <= 18` | 18 or less | yes |
| `age > 18` | more than 18 | no |
| `age >= 18` | 18 or more | yes |

```c
int bus_fare(int age) {
    if (age < 7) {
        return 0;          // small kids ride free
    } else if (age <= 19) {
        return 25;         // youth fare, up to and including 19
    } else {
        return 40;
    }
}
```

An `if / else if / else` chain checks the conditions from the top and runs **only the first** block whose condition is true.

### Debugging: test the boundaries

Rules like "under 12" or "65 or older" have an exact age where the answer changes. That edge is where bugs like to hide. Most ages work no matter whether you wrote `<` or `<=`, so a quick test with 30 or 80 tells you nothing.

Instead, try the numbers **right at the edge and one on each side**: for "under 7" try 6, 7 and 8. Read the rule in the bug report word for word, decide what each of those should give, and compare with what the code does.
