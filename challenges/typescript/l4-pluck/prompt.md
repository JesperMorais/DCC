Charts and CSV exports often need one "column" from a list of records. Write `pluck(items, key)` that returns the value of `key` from every item, in order.

The typing is the real challenge:

- `key` may only be a key that actually exists on the items — `pluck(users, "emial")` must be a **type error**.
- The return type must follow the key: plucking `"name"` (a `string`) gives `string[]`, plucking `"age"` (a `number`) gives `number[]`.

```ts
const users = [
  { name: "Ada", age: 36, admin: true },
  { name: "Linus", age: 28, admin: false },
];

pluck(users, "name");   // ["Ada", "Linus"]    — string[]
pluck(users, "age");    // [36, 28]            — number[]
pluck(users, "emial");  // ✗ compile error
```
