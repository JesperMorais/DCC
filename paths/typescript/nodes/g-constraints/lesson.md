A team ships a helper that finds a record by any field:

```ts
function findBy(items: any[], key: string, value: any): any {
  return items.find((item) => item[key] === value);
}

const user = findBy(users, "emial", "ada@example.com"); // typo
user.name.toUpperCase();                                 // 💥 at runtime
```

The compiler said nothing. `"emial"` is a string, `any` accepts everything, and `any` comes back out. The typo only showed up in production, as `Cannot read properties of undefined`. This lesson shows how to write the same helper so the compiler catches the typo and the caller still gets back a real `User`.

### First: types don't exist when the code runs

When you `console.log(items)`, you see **values**: `[{ id: 1, name: "Ada" }, …]`. You never see `Customer[]`. TypeScript's types are checked while you write the code and then **erased**. The JavaScript that runs has no types left in it.

So there is no runtime check that tells you "what type is in this `any[]`", and you don't need one. The job of the types is to describe, **at the call site**, what the caller is allowed to pass and what they get back. The function body only ever deals with values.

### A bare `T` knows nothing

A type parameter carries the caller's type through the function:

```ts
function first<T>(items: readonly T[]): T | undefined {
  return items[0];
}
first(customers); // T = Customer → returns Customer | undefined
```

Inside `first`, though, `T` could be *anything*: `number`, `string`, `null`. So TypeScript won't let you touch any property:

```ts
function ids<T>(items: readonly T[]) {
  return items.map((item) => item.id);
  //                              ~~ Property 'id' does not exist on type 'T'
}
```

It's right to refuse. Nothing stops a caller from passing `[1, 2, 3]`.

### `extends`: "anything, as long as it has…"

A **constraint** sets a minimum shape for `T`:

```ts
function ids<T extends { id: number }>(items: readonly T[]): number[] {
  return items.map((item) => item.id); // ✓ every T has an id
}

ids(customers);              // ✓ T = Customer, extra fields are fine
ids([{ id: 5, sku: "X" }]);  // ✓ T = { id: number; sku: string }
ids([{ name: "no id" }]);    // ✗ error: 'id' is missing
```

Read `T extends { id: number }` as **"T is any type that has *at least* an `id: number`."** It's a deal with the caller. They promise to pass something with an `id`, and in return the function body may use `.id`, and only `.id`.

### Why not just write `{ id: number }[]`?

```ts
function dedupe(items: { id: number }[]): { id: number }[] { … }

const out = dedupe(customers);
out[0].name; // ✗ Property 'name' does not exist on type '{ id: number }'
```

That signature accepts customers, but it returns the *minimum shape*. Everything else the caller knew about their data is lost. With `<T extends { id: number }>` and a return type of `T[]`, the caller's exact type goes in **and comes back out**:

```
Customer[] ──► dedupe<T = Customer> ──► Customer[]
```

**Rule of thumb:** use the constraint for what you *need*, and return `T` for what the caller *had*.

### `keyof`: the keys of a type, as a type

`keyof T` is the union of `T`'s property names:

```ts
interface User { name: string; age: number; admin: boolean }
type UserKey = keyof User; // "name" | "age" | "admin"
```

Combine it with a constraint and the key becomes a *second* type parameter, one that must be a real key of the first:

```ts
function get<T, K extends keyof T>(obj: T, key: K) {
  return obj[key];
}
get(user, "age");   // ✓
get(user, "emial"); // ✗ '"emial"' is not assignable to '"name" | "age" | "admin"'
```

That's the typo from the opening, caught as you type.

### `T[K]`: the type *at* a key

What does `get` return? It's the type of the property you asked for. You write that `T[K]`, an **indexed access type**:

```ts
function get<T, K extends keyof T>(obj: T, key: K): T[K] {
  return obj[key];
}
const age = get(user, "age");   // number
const name = get(user, "name"); // string
```

`T[K]` works on parameters too. `value: T[K]` means "a value of the same type as that field", so passing `"36"` for `age` is an error.

### Gotchas

- **Don't over-constrain.** `<T extends { id: number; name: string }>` rejects every item without a `name`, even though you only use `id`. Require exactly what the body uses.
- **`String` ≠ `string`.** Lowercase `string` is the type you want. Capital `String` is the wrapper object type.
- **`for...in` gives you keys, not items.** On an array, `for (const i in items)` loops over the indices as strings: `"0"`, `"1"`…. Use `for (const item of items)` to get the items.
- **Constraints don't change the values.** A constraint only affects what compiles. At runtime your function receives exactly what the caller passed, with all the extra fields.

### In the wild

- **`Array.prototype.find`** in TypeScript's own `lib.d.ts` is generic over the element type, so `customers.find(...)` gives you `Customer | undefined` and not `any`.
- **React's `useState<T>`** carries your state type through the setter and the value with a bare `T`, without any constraint.
- **Lodash's types** for `_.get`, `_.pick` and `_.sortBy` use `K extends keyof T` so that `_.pick(user, "emial")` is a compile error.
- **ORMs like Prisma** use `keyof` and indexed access to type `select: { name: true }`, so the result only has the fields you asked for.
