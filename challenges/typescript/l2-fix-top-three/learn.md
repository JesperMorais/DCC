### Sorting arrays

`sort()` puts an array's items in order. It changes the array **in place**, so a common trick is to copy it first with the spread syntax `[...arr]`:

```ts
const names = ["Mia", "Al", "Zoe"];
const sorted = [...names].sort(); // ["Al", "Mia", "Zoe"], names is untouched
```

### Telling `sort` how to compare

You can hand `sort` a **compare function**. It gets two items, `a` and `b`, and returns a number:

- negative: `a` goes first
- positive: `b` goes first
- zero: keep them as they are

```ts
const ages = [30, 4, 18];
[...ages].sort((a, b) => a - b); // [4, 18, 30]
```

`slice(0, n)` then takes the first `n` items (or fewer, if the array is shorter).

### Debugging: shrink the failing case

When a function gives a wrong answer, break it into steps and check each one separately. Run just the first step on the failing input and look at its result. The first step whose output surprises you is where the bug lives.
