A developer wrote a helper to drop an item from a shopping cart, and the cart page started acting haunted: the item vanished from the "saved for later" list too, and the undo button stopped working.

```ts
function removeItem(cart: string[], item: string): string[] {
  cart.splice(cart.indexOf(item), 1);   // changes the caller's array!
  return cart;
}
```

The function *removed* the item, which was the job. But it removed it from the **caller's** array, the one that other parts of the page were still using. When a task says "don't modify the input", it doesn't mean "don't remove anything". It means: **build a new array and return that.**

### Arrays and their types

An array is an ordered list of values. Its type is the element type plus `[]`:

```ts
const names: string[] = ["Ada", "Linus"];
const scores = [90, 72, 85];   // inferred: number[]
names[0];                       // "Ada"   (indices start at 0, like strings)
names.length;                   // 2
```

The type only exists while you write code. At runtime, `console.log(names)` shows the values `["Ada", "Linus"]`, never `string[]`.

### Variables hold a reference, not a copy

This is the key idea behind the opening bug:

```ts
const a = ["milk", "eggs"];
const b = a;          // b is the SAME array, not a copy
b.push("jam");
console.log(a);       // ["milk", "eggs", "jam"]  ← a changed too
```

```
a ──┐
    ├──► [ "milk", "eggs", "jam" ]
b ──┘
```

Passing an array to a function works the same way: the parameter points at the caller's array. So anything the function does *to* that array, the caller sees. Note that `const` doesn't help: it stops you reassigning `a`, not changing what's inside it.

### Mutating vs. non-mutating

Some methods change the array they're called on; others leave it alone and give you something new:

| Changes the array (mutates) | Returns something new |
|---|---|
| `push(x)` add at end | `[...arr, x]` spread into a new array |
| `unshift(x)` add at start | `[x, ...arr]` |
| `splice(i, 1)` remove at `i` | `[...arr.slice(0, i), ...arr.slice(i + 1)]` |
| `sort()`, `reverse()` | `slice()` copy first, or `toSorted()`, `toReversed()` |

**Spread**, `...arr`, unpacks an array's items into a new array literal. `[...a]` is a shallow copy; `[...a, ...b]` joins two arrays; `["first", ...a]` puts something in front.

**`slice(start, end)`** works exactly like it does on strings: it returns the items from `start` up to (not including) `end`, as a **new** array. Don't confuse it with **`splice`**, which cuts items out of the original. One letter apart, opposite behaviour.

### Searching: `includes` and `indexOf`

```ts
const cart = ["milk", "eggs", "jam"];
cart.includes("eggs");   // true      is it there?
cart.indexOf("jam");     // 2         where is it?
cart.indexOf("tea");     // -1        not found
```

`indexOf` returns `-1` when the item is missing, so **always check for `-1`** before using the result. Negative indices count from the end in `slice` and `splice`, so a missing check quietly does the wrong thing. The opening bug had this problem too: for an item that isn't in the cart, `splice(-1, 1)` deletes the *last* item.

### Worked example: remove without mutating

```ts
function without(cart: readonly string[], item: string): string[] {
  const i = cart.indexOf(item);
  if (i === -1) return [...cart];                    // not there: a copy, unchanged
  return [...cart.slice(0, i), ...cart.slice(i + 1)]; // everything except index i
}

const saved = ["milk", "eggs", "jam"];
const next = without(saved, "eggs");
// next  → ["milk", "jam"]
// saved → ["milk", "eggs", "jam"]   untouched
```

```
index:      0       1       2
saved:  [ "milk", "eggs", "jam" ]
         slice(0,1)       slice(2)
next:   [ "milk",         "jam" ]
```

The **`readonly string[]`** parameter type makes the promise checkable: inside the function, `cart.push(…)` and `cart.splice(…)` are compile errors, while `slice`, `includes`, `indexOf` and spread still work. It's erased at runtime like every type, but it stops the opening bug before it ships.

### Gotchas

- **"Don't modify the input" ≠ "don't remove items".** You remove items from a **new** array that you return. The input stays exactly as it was.
- **`slice` vs `splice`.** `slice` copies, `splice` cuts. If in doubt, use `slice` and spread.
- **`push` returns the new length**, not the array. `return list.push(x)` returns a number (and mutated `list`).
- **Check `indexOf` for `-1`.** Negative indices count from the end in `slice` and `splice`.
- **Spread is shallow.** `[...users]` is a new array, but the objects inside are the same objects. (That matters later, when arrays hold objects.)

### In the wild

- **React and Redux** decide whether to re-render by checking if the array is a *new* one. `state.push(x)` keeps the same array, so the screen doesn't update; `[...state, x]` works.
- **Undo/redo** works by keeping old arrays around. If you mutated them, there's nothing to go back to.
- **ES2023 added `toSorted`, `toReversed` and `toSpliced`** precisely so you can do those operations without mutating.
- **TypeScript's `readonly` arrays** are used throughout libraries like Immer and Redux Toolkit to mark data you must not change.
