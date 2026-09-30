### Variables: boxes with names

A **variable** is a named box that holds a value.

```ts
const pi = 3.14;   // const: this box can never be refilled
let score = 0;     // let: this box can be refilled later
score = 10;        // OK, because it's a let
```

Use `const` when the value never changes and `let` when it will.

### Making decisions with `if`

`if` runs some code **only when** a condition is true:

```ts
let message = "cold";
if (temperature > 25) {
  message = "hot";
}
```

You can add `else` for the "otherwise" case:

```ts
if (hour < 12) {
  greeting = "Good morning";
} else {
  greeting = "Good afternoon";
}
```

### A handy pattern

"Keep a running best": start with a first guess in a `let` variable, and every time you
find something better, put that into the box instead. At the end, the box holds the winner.
