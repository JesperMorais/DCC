Your API has a `PATCH /users/:id` endpoint: the client sends only the fields it wants to change.

1. Define the type **`UserPatch`**: every field of `User` becomes optional, **except** that `id` and `updatedAt` must not be in it at all — sending them is a type error. Build it from `User` with utility types rather than writing the fields out by hand.
2. Write `updateUser(user, patch, now)` that returns a **new** `User`:
   - fields in `patch` replace the old values; everything else stays
   - `updatedAt` is set to `now`
   - if the patch contains `email`, store it **trimmed and lowercased**
   - the original `user` object must not be changed

```ts
const ada: User = { id: 1, name: "Ada", email: "ada@example.com", role: "member", updatedAt: 0 };

updateUser(ada, { role: "admin" }, 100);
// { id: 1, name: "Ada", email: "ada@example.com", role: "admin", updatedAt: 100 }

updateUser(ada, { email: "  Ada@Lovelace.dev " }, 200).email;
// "ada@lovelace.dev"

updateUser(ada, { id: 2 }, 300); // ✗ compile error
```
