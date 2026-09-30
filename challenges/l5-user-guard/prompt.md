Data from `JSON.parse` or an API is `unknown` — the compiler has no idea what's in it. Write two functions that check it at runtime **and** tell the compiler what you proved.

```ts
interface User {
  id: number;
  name: string;
  email?: string;
  roles: string[];
}
```

1. **`isUser(value: unknown): value is User`** — returns `true` only if `value` is a non-null object where:
   - `id` is a number, `name` is a string,
   - `email` is either missing/`undefined` or a string,
   - `roles` is an array of strings (may be empty).
2. **`assertUser(value: unknown): asserts value is User`** — returns normally for a valid user, otherwise throws `new Error("Invalid user")`.

Extra properties are allowed.

```ts
isUser({ id: 1, name: "Ada", roles: ["admin"] });   // true
isUser({ id: "1", name: "Ada", roles: [] });        // false (id is a string)
isUser({ id: 1, name: "Ada", roles: ["a", 2] });    // false (a role isn't a string)
```
