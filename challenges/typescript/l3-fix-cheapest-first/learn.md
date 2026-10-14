### Arrays are shared, not copied

Assigning an array or passing it to a function does **not** copy it. Both names point at the very same array, so a change made through one is visible through the other:

```ts
const original = [3, 1, 2];
const alias = original;
alias.push(4);
original; // [3, 1, 2, 4]
```

### Mutating vs non-mutating methods

Some array methods build a **new** array and leave the old one alone: `map`, `filter`, `slice`, `concat`. Others **mutate** the array in place: `push`, `pop`, `splice`, `reverse`, `sort`. A mutating method used on a parameter quietly changes the caller's data.

To work on a copy, spread it into a new array first:

```ts
const copy = [...original];
copy.reverse(); // original is untouched
```

### Debugging: check the inputs, not just the output

A function can return the right answer and still be wrong, if it damages something on the way. When a bug shows up "somewhere else", log the arguments **before and after** the call and compare them.
