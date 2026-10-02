A checkout page reads the quantity from a form field and computes the total:

```js
function total(price, qty) {
  return price * qty + 4.99; // shipping
}
total(10, document.querySelector("#qty").value); // qty is "3", a string
```

`10 * "3"` happens to work (it's `30`), so this passed testing. Then someone changed it to `price + qty * …`, and `+` with a string *glues text together*. A customer was quoted `"103…"` dollars. Plain JavaScript said nothing. TypeScript exists to say something.

### Values and types

A **value** is a piece of data: `42`, `"Ada"`, `true`. A **type** is the kind of value it is: `number`, `string`, `boolean`. TypeScript is JavaScript plus a checker that knows the type of every value and complains *before the code runs* when you mix them up.

Then the types are **erased**. The JavaScript that actually runs has no types in it, and `console.log` only ever shows you values. Types are a tool for you and your editor, not something the program checks at runtime.

### Variables: `const` and `let`

```ts
const taxRate = 0.25;  // can't be reassigned
let count = 0;         // can be reassigned
count = count + 1;     // ✓
taxRate = 0.3;         // ✗ Cannot assign to 'taxRate' because it is a constant
```

Use `const` by default and `let` only when the variable really changes. (Skip `var`; it's the old keyword with confusing scoping rules.)

You rarely need to write the type of a variable. TypeScript **infers** it from the value: `count` is a `number`, so `count = "five"` is an error. You *can* annotate it, `let count: number = 0;`, but that adds nothing here.

### Numbers and arithmetic

There is one number type, `number`. It covers whole numbers and decimals alike (they're all 64-bit floating point, like `double` in C or Java):

```ts
7 + 2   // 9
7 - 2   // 5
7 * 2   // 14
7 / 2   // 3.5  (not 3! there's no integer division)
2 ** 3  // 8    (power)
```

Because there's no integer division, you ask for rounding explicitly: `Math.floor(7 / 2)` is `3` (round down), `Math.ceil(7 / 2)` is `4` (round up), `Math.round(3.5)` is `4`.

### The remainder operator `%`

`a % b` is what's **left over** after dividing `a` by `b` as many whole times as possible:

```
17 % 5  →  17 = 5 + 5 + 5 + 2   →  2
10 % 5  →  10 = 5 + 5           →  0
 3 % 5  →   3 (5 doesn't fit)    →  3
```

It's the workhorse of everyday code. `n % 2 === 0` means "n is even". `seconds % 60` is the seconds part of a duration. `Math.floor(total / size)` and `total % size` together split a total into "full groups" and "what's left".

### Functions

A function takes **parameters** and **returns** a value. In TypeScript you annotate the parameters, and usually the return type too:

```ts
function lineTotal(price: number, qty: number): number {
  return price * qty;
}

lineTotal(10, 3);     // 30
lineTotal(10, "3");   // ✗ Argument of type 'string' is not assignable to parameter of type 'number'
```

That error is the checkout bug from the opening, caught while typing. The parameter annotations are the important part. TypeScript can't guess what callers will pass, so without them the parameters become `any` (under `strict`, that's an error you'll see).

The return type `: number` is optional because TypeScript can infer it from the `return`. Writing it is still good practice: it documents the function, and if you accidentally return the wrong thing the error points at the function, not at some distant caller.

### Worked example: minutes and seconds

```ts
function minutesPart(totalSeconds: number): number {
  return Math.floor(totalSeconds / 60);
}
function secondsPart(totalSeconds: number): number {
  return totalSeconds % 60;
}
minutesPart(135); // 2
secondsPart(135); // 15  → "2:15"
```

### Gotchas

- **`/` never rounds.** `7 / 2` is `3.5`. If you want whole groups, use `Math.floor`.
- **Decimals aren't exact.** `0.1 + 0.2` is `0.30000000000000004`. For money, count in cents (whole numbers) where you can.
- **`%` keeps the sign of the left side.** `-7 % 3` is `-1`, not `2`. Fine for counts, surprising for negative numbers.
- **Lowercase types.** Write `number`, `string`, `boolean`. The capitalised `Number`, `String`, `Boolean` are wrapper objects you never want in an annotation.

### In the wild

- **Video players** turn `135` seconds into `2:15` with exactly `Math.floor(s / 60)` and `s % 60`.
- **Pagination** ("page 3 of 8") is `Math.ceil(items / perPage)`.
- **Zebra-striped tables** colour every other row with `index % 2`.
- **Stripe's API** stores amounts as whole numbers of cents (`1999` for $19.99) to avoid decimal rounding errors.
