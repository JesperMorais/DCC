Config files and API responses are deeply nested. Write **`getPath`**, which reads a value by a dotted path.

```ts
function getPath(obj: unknown, path: string): unknown
```

- `"a.b.c"` means `obj.a.b.c`. Array elements use their index as a segment: `"users.0.name"`.
- The empty path `""` returns `obj` itself.
- If any step along the way is missing, or isn't an object/array, return `undefined` — never throw.
- Real values are returned as-is, even falsy ones (`0`, `""`, `null`, `false`).

```ts
const cfg = { db: { host: "localhost", port: 5432 }, users: [{ name: "Ada" }] };

getPath(cfg, "db.port");       // 5432
getPath(cfg, "users.0.name");  // "Ada"
getPath(cfg, "db.user.name");  // undefined (db.user doesn't exist)
getPath(cfg, "db.host.length"); // undefined (strings are not descended into)
```
