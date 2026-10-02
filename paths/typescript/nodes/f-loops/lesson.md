A fitness app showed a weekly total of `"0123456"` steps. The developer had come from Python, where `for x in list` gives you the items, and wrote this:

```ts
let total = "";
for (const day in steps) {
  total += day;   // day is "0", "1", "2"… the indices, as strings!
}
```

It compiles, it runs, and it's wrong. TypeScript has three `for` loops that look alike and do different things. This lesson sorts them out and shows the one pattern you'll use in almost every loop: the accumulator.

### The classic `for`: when you need the index

```ts
for (let i = 0; i < steps.length; i++) {
  console.log(i, steps[i]);
}
```

It has three parts, separated by `;`:

```
for ( let i = 0 ;  i < steps.length ;  i++ )
      start here   keep going while   after each round
```

`i` runs `0, 1, 2, …, length - 1`. The condition is `<`, not `<=`: index `length` is one past the end, and `steps[steps.length]` is `undefined`. Use the classic `for` when you need the **position**, or when you're counting rather than walking an array (`for (let i = 1; i <= n; i++)`).

### `for...of`: when you need the items

```ts
for (const count of steps) {
  console.log(count);   // 8000, 12000, 4500 …
}
```

`for...of` hands you each **value** in order. No index, no off-by-one. It also works on strings, one character at a time: `for (const ch of "hey")` gives `"h"`, `"e"`, `"y"`. This is your default loop.

### `for...in`: keys, not items

```ts
for (const key in steps) {
  console.log(key);   // "0", "1", "2" — strings!
}
```

`for...in` walks the **keys** (property names) of an object. For an array, the keys are the indices, and they arrive as **strings**. That's how the opening story ended up gluing `"0"`, `"1"`, `"2"`… together. It's meant for plain objects; on arrays, avoid it.

| Loop | You get | Use it for |
|---|---|---|
| `for (let i = 0; i < a.length; i++)` | the index `i` (a number) | positions, counting |
| `for (const x of a)` | each item | almost everything |
| `for (const k in obj)` | each key (a string) | plain objects, not arrays |

### The accumulator pattern

Most loops answer a question about *all* the items: the total, how many, the biggest. The pattern is always the same:

1. **Before** the loop, create a variable with a starting value.
2. **Inside** the loop, update it from each item.
3. **After** the loop, return it.

```ts
function totalSteps(days: number[]): number {
  let total = 0;            // 1. start
  for (const n of days) {
    total += n;             // 2. update (same as total = total + n)
  }
  return total;             // 3. result
}
```

The starting value matters. For a sum it's `0`; for a count it's `0`; for a product it's `1`; for "the text so far" it's `""`. And it must be declared **outside** the loop. Inside, it would be reset on every round.

### Worked example: count and find

Count the days that hit the goal, and find the **first** such day:

```ts
function daysOverGoal(days: number[], goal: number): number {
  let count = 0;
  for (const n of days) {
    if (n >= goal) count++;
  }
  return count;
}

function firstGoalDay(days: number[], goal: number): number {
  for (let i = 0; i < days.length; i++) {
    if (days[i] >= goal) return i;   // found it: stop right here
  }
  return -1;                          // the loop finished without finding one
}
```

`firstGoalDay` needs the position, so it uses the classic `for`. `return` inside a loop exits the whole function immediately; `break` exits just the loop. The `-1` after the loop is the "not found" answer, a convention you'll see again with `indexOf`.

Accumulators can also remember things between rounds. To track the longest run of good days in a row, keep two: the **current** run (reset to `0` on a bad day) and the **best** run so far.

### Gotchas

- **`<=` vs `<`.** `i <= arr.length` reads one past the end and gives you `undefined`.
- **`for...in` on arrays** gives string indices. If you see `"01"` where you expected `1`, look for an `in` that should be an `of`.
- **Accumulator declared inside the loop** gets reset every time. Declare it before.
- **`const` in `for...of`, `let` in classic `for`.** `for (const x of a)` gets a fresh `x` each round. The classic loop changes `i`, so it must be `let`.
- **`while`** exists too: `while (cond) { … }`. Use it when you don't know in advance how many rounds you need.

### In the wild

- **Shopping carts** sum line totals with an accumulator (later you'll write the same thing as `reduce`).
- **Activity trackers** compute streaks, "days in a row", with a current and a best counter.
- **Search boxes** loop until the first match and return early, exactly like `firstGoalDay`.
- **Linters** like ESLint's `guard-for-in` rule and TypeScript-ESLint's `prefer-for-of` exist because of the `for...in` mix-up.
