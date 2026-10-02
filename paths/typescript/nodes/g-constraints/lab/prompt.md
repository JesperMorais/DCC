Your support dashboard has lists of tickets, users and orders, and you keep writing the same two loops. Write two helpers that work for **any** of them and keep their types.

**1. `newest(items)`** returns the item with the largest `createdAt` (a timestamp number), or `undefined` for an empty list.

- It works for any object type that has a numeric `createdAt`, and it returns that same type. A `Ticket[]` gives back a `Ticket | undefined`.
- Passing items **without** `createdAt` must be a type error.
- If two items tie, return the first one.

**2. `findBy(items, key, value)`** returns the first item whose `key` field equals `value`, or `undefined`.

- `key` must be a real key of the item type. `findBy(users, "emial", …)` is a type error.
- `value` must have the type of that field. `findBy(users, "age", "36")` is a type error, because `age` is a number.

Neither function may modify the input.

```ts
newest([{ id: 1, createdAt: 100 }, { id: 2, createdAt: 300 }]); // { id: 2, createdAt: 300 }
findBy(users, "name", "Grace");                                  // the Grace user
findBy(users, "age", 99);                                        // undefined
```
