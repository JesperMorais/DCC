### Characters have positions

A string is a row of characters, and each one has a position number called an **index**.
Counting starts at **0**, not 1:

```
 "cat"
  c  a  t
  0  1  2
```

You can grab one character with `charAt`:

```ts
const word = "cat";
word.charAt(0); // "c"
word.charAt(2); // "t"
```

### Chaining methods

A method gives you back a new value — and you can immediately call another method on it:

```ts
"hello".charAt(1).toUpperCase(); // "E"
```

Read it left to right: take `"hello"`, get the character at index 1 (`"e"`), make it uppercase.

### Building a string

Glue pieces with `+`, or use a **template literal** (backticks) with `${...}` slots:

```ts
const a = "x";
const b = "y";
a + "-" + b;   // "x-y"
`${a}-${b}`;   // "x-y"
```
