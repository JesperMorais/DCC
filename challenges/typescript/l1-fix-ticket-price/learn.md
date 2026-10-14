### Comparison operators

A **comparison** asks a yes/no question about two numbers. The answer is a boolean, `true` or `false`:

```ts
const age = 12;
age < 12;   // false  — "less than"
age <= 12;  // true   — "less than or equal to"
age > 12;   // false  — "greater than"
age >= 12;  // true   — "greater than or equal to"
```

The only difference between `>` and `>=` is what happens when both sides are **equal**. That one case is easy to forget, because most numbers aren't on the edge.

### Debugging: test the boundaries

When rules say "under 12" or "65 or older", the numbers where the answer changes are called **boundaries**. Bugs love to hide there. A good habit is to try the value **on** the boundary and one on **each side** of it:

```ts
sizeLabel(159); // just below the line
sizeLabel(160); // exactly on the line
sizeLabel(161); // just above
```

If the middle one gives a surprising answer, look closely at the comparison operator for that rule.
