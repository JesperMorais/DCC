### Searching an array

Arrays come with methods for finding things:

```ts
const colors = ["red", "green", "blue"];

colors.includes("green"); // true   — is it there?
colors.includes("pink");  // false

colors.indexOf("blue");   // 2      — where is it?
colors.indexOf("pink");   // -1     — -1 means "not found"
```

### Making a new array with spread

`...` (the **spread** operator) unpacks an array's items into a new array literal.
That lets you create a copy with extra items, without touching the original:

```ts
const small = [1, 2];
const bigger = [...small, 3]; // [1, 2, 3]
const front = [0, ...small];  // [0, 1, 2]
// small is still [1, 2]
```

Compare that to `small.push(3)`, which **changes** `small` itself. Returning a new array
instead is often safer: whoever gave you the list won't be surprised by changes.
