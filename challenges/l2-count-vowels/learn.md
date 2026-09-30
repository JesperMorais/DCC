### Looping over things with `for...of`

When you want to visit **every item** in something, `for...of` is simpler than a counting loop.
It works on arrays **and on strings** (a string is a sequence of characters):

```ts
for (const letter of "hey") {
  console.log(letter); // "h", then "e", then "y"
}
```

Each round, `letter` holds the next character. You don't need an index at all.

### Does a string contain something?

`includes` answers "is this piece somewhere inside?":

```ts
"banana".includes("nan"); // true
"xyz".includes("a");      // false
```

Since a single character is just a short string, you can flip it around and ask a
"set of allowed characters" whether it includes the character you're looking at.

### Counting

```ts
let spaces = 0;
for (const ch of "a b c") {
  if (ch === " ") spaces++;  // ++ adds 1
}
// spaces is 2
```
