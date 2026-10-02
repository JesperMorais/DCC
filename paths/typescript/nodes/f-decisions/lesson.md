An online shop launched a two-tier discount: 10% off orders over $10, 20% off orders over $50. On launch day, nobody got 20%. The code:

```ts
if (total > 10) discount = 0.1;
else if (total > 50) discount = 0.2;   // never reached
```

A $200 order is also over $10, so the first branch won and the chain stopped. Each line was correct on its own; the **order** was the bug. This lesson is about writing decisions that do what you meant.

### Booleans and comparisons

A `boolean` is either `true` or `false`. You mostly get one by **comparing** two values:

```ts
age >= 18        // greater than or equal
qty < 1          // less than
status === "paid"   // equal
status !== "paid"   // not equal
```

Always use `===` and `!==` (three characters). The two-character `==` converts types before comparing, so `0 == ""` and `"1" == 1` are both `true`. TypeScript catches many of these, but `===` never surprises you.

A comparison is just a value, so you can store it in a well-named variable:

```ts
const isAdult = age >= 18;   // boolean
```

### `if` / `else`

```ts
if (stock === 0) {
  label = "Sold out";
} else {
  label = "In stock";
}
```

The condition in the parentheses is checked; exactly one of the two blocks runs.

### `if` / `else if` chains: first match wins

```ts
function discountFor(total: number): number {
  if (total > 50) {
    return 0.2;
  } else if (total > 10) {
    return 0.1;
  } else {
    return 0;
  }
}
```

The chain is checked **top to bottom**, and the first condition that's `true` wins. Everything below is skipped. That's why the most specific (or largest) case goes first. Here, by the time we reach `total > 10`, we already know `total` is *not* over 50, so that branch really means "between 10 and 50".

You'll often see the same thing with **early returns** and no `else` at all, because a `return` already leaves the function:

```ts
if (total > 50) return 0.2;
if (total > 10) return 0.1;
return 0;
```

### Logical operators: `&&`, `||`, `!`

Combine booleans with:

| Operator | Means | `true` when |
|---|---|---|
| `a && b` | and | both are true |
| `a \|\| b` | or | at least one is true |
| `!a` | not | `a` is false |

```ts
const canCheckout = cartCount > 0 && !isBlocked;
const freeShipping = isMember || total >= 50;
```

They **short-circuit**: in `a && b`, if `a` is false, `b` is never even evaluated. That's useful for guards like `user !== undefined && user.isAdmin`.

When you mix `&&` and `||`, **use parentheses**. `&&` binds tighter, so `a || b && c` means `a || (b && c)`. Write the parentheses anyway; the next reader will thank you.

### Worked example: can this order ship today?

The rule: it ships today if it's paid **and** in stock, **and** either it was placed before 14:00 **or** the customer pays for express.

```ts
function shipsToday(paid: boolean, inStock: boolean, hour: number, express: boolean): boolean {
  return paid && inStock && (hour < 14 || express);
}
shipsToday(true, true, 9, false);   // true
shipsToday(true, true, 16, false);  // false: too late, no express
shipsToday(true, false, 9, true);   // false: out of stock
```

No `if` needed: the expression *is* the boolean. Writing `if (cond) return true; else return false;` is the long way of writing `return cond;`.

### Gotchas

- **Order your chain.** Put the narrowest or highest threshold first, or every value falls into the first broad branch.
- **Boundaries.** "Over 50" is `> 50`; "50 or more" is `>= 50`. Read the requirement twice and test the exact boundary value.
- **`=` is not `===`.** `if (status = "paid")` *assigns* `"paid"` to `status`, and the condition is always true. TypeScript lets it through; a linter will catch it.
- **Truthiness.** `if (count)` is false for `0`, and `if (name)` is false for `""`. When `0` or `""` are valid values, compare explicitly: `count !== undefined`.
- **Parentheses around mixed `&&` / `||`.** Don't rely on precedence you have to look up.

### In the wild

- **Shipping and tax rules** on every checkout are if/else chains with thresholds, and boundary bugs ("exactly $50") are some of the most common e-commerce support tickets.
- **Feature flags** are booleans combined with `&&`: `flags.newCheckout && user.isBetaTester`.
- **Permission checks** like `isOwner || (isAdmin && !isSuspended)` guard every admin page.
- **Short-circuit guards** like `user && user.name` are so common that JavaScript added `user?.name` as a shortcut.
