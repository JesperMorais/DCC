### Comparing strings

`===` checks whether two strings are **exactly** the same, letter for letter. To the computer, a capital letter is a completely different character from a small one:

```ts
"cat" === "cat";  // true
"Cat" === "cat";  // false  — "C" and "c" are different
```

### Making case not matter

String methods return a **new** string. `toLowerCase()` gives you the same text in small letters, so two spellings that differ only in capitals become equal:

```ts
const answer = "YES";
answer.toLowerCase();             // "yes"
answer.toLowerCase() === "yes";   // true
```

### `||` means "or"

`a || b` is `true` if **either** side is true:

```ts
const isPet = animal === "cat" || animal === "dog";
```

### Debugging: change one thing at a time

When one input works and a similar one doesn't, write both down side by side and find the **one difference** between them. Here, try the same day spelled a few different ways and see which spellings break.
