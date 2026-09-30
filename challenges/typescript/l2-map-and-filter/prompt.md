Write two short functions — each one should be a single `return` using an array method.

**`shoutAll(words)`** returns a new array with every word in UPPERCASE:

- `shoutAll(["hi", "there"])` → `["HI", "THERE"]`
- `shoutAll([])` → `[]`

**`longWords(words, minLength)`** returns only the words that have **at least** `minLength` characters, in their original order:

- `longWords(["a", "tree", "is", "green"], 4)` → `["tree", "green"]`
- `longWords(["cat", "dog"], 5)` → `[]`

Neither function should change the array it was given.
