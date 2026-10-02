A team has a `User` interface and, next to it, a hand-written `UserUpdate` with the same fields made optional. Months later someone adds `phone: string` to `User`. Nobody touches `UserUpdate`, so the "edit profile" form can never change a phone number, and the compiler is perfectly happy: the two types were never connected.

The fix is to **derive** one type from the other, so they can't drift apart. TypeScript ships helpers for the common cases, and you can write your own with the same trick they use.

### Utility types: types built from types

A **utility type** is a generic type that takes a type and gives back a new one. You call it with `<…>`, just like a generic function:

```ts
interface User {
  readonly id: number;
  name: string;
  email: string;
  phone?: string;
}

type UserUpdate = Partial<User>;      // every field optional
type NewUser = Omit<User, "id">;      // everything except id
type Contact = Pick<User, "email" | "phone">; // only these two
type Frozen = Readonly<User>;         // every field readonly
type Full = Required<User>;           // every field required, phone included
```

Add `phone` to `User`, and every one of these picks it up automatically. You already met `Record` in *Records, Maps & Sets*: `Record<"draft" | "live", number>` is "an object with exactly those keys, each a number". It's a utility type too.

They combine. A PATCH request that may change anything except the id:

```ts
type UserPatch = Partial<Omit<User, "id">>;
// { name?: string; email?: string; phone?: string }
```

Read it inside-out: first drop `id`, then make the rest optional.

### Mapped types: how they're built

None of these are magic. Here is `Partial`, written out:

```ts
type MyPartial<T> = {
  [K in keyof T]?: T[K];
};
```

That's a **mapped type**. Read it as a loop over keys:

- `keyof T` is the union of `T`'s keys (from *Generic constraints & keyof*): `"id" | "name" | "email" | "phone"`.
- `[K in keyof T]` means "for each key `K`, make a property called `K`".
- `T[K]` is the type at that key, so each property keeps its original type.
- The `?` after the brackets is the change: every property becomes optional.

`Readonly` is the same loop with `readonly` in front:

```ts
type MyReadonly<T> = {
  readonly [K in keyof T]: T[K];
};
```

`Record` loops over a union you give it instead of over `keyof T`:

```ts
type MyRecord<K extends string, V> = { [P in K]: V };
```

### Modifiers: adding and removing `?` and `readonly`

A mapped type can **add** a modifier (`?`, `readonly`, or with an explicit `+`) or **remove** one with `-`:

```ts
type Complete<T> = { [K in keyof T]-?: T[K] };          // remove ?
type Mutable<T>  = { -readonly [K in keyof T]: T[K] };  // remove readonly
```

`-?` also removes the `undefined` that came with the `?`. For `User`, `Complete<User>["phone"]` is `string`, not `string | undefined`. That's exactly how the built-in `Required` works.

You can combine them in one type. "A settings object after defaults are filled in: every field present, and nobody may change it" is:

```ts
type Resolved<T> = { readonly [K in keyof T]-?: T[K] };
```

### Worked example: an edit form

```ts
function applyEdit(user: User, patch: UserPatch): User {
  return { ...user, ...patch }; // a new object; user is untouched
}

applyEdit(ada, { name: "Ada L." }); // ✓
applyEdit(ada, { id: 2 });          // ✗ 'id' does not exist in type UserPatch
```

The type does the guarding. The function body just spreads.

### Gotchas

- **Types don't change values.** `Omit<User, "email">` says nothing about the object at runtime. If you pass a full `User` where an `Omit<User, "email">` is expected, the email is still in there and still gets sent. To really drop a field, build a new object: `const { email, ...rest } = user;`.
- **`Omit` doesn't check its keys.** `Omit<User, "emial">` compiles and removes nothing. `Pick` does check (`K extends keyof T`), so a typo there is an error.
- **`undefined` in a patch overwrites.** `{ ...user, ...{ name: undefined } }` sets `name` to `undefined`. If a patch can contain `undefined`, skip those keys yourself.
- **They're shallow.** `Partial<User>` makes `address` optional, but not the fields *inside* `address`. Same for `Readonly`.
- **`readonly` is compile-time only.** It stops your code from assigning. It doesn't freeze the object. For that, use `Object.freeze`.

### In the wild

- **REST and GraphQL clients** type PATCH bodies as `Partial<Omit<Entity, "id" | "createdAt">>`, exactly like `UserPatch`.
- **React's** `setState` in class components takes a `Pick<State, K>`, and component libraries use `Omit<ButtonProps, "onClick">` to wrap a component and replace one prop.
- **Prisma** generates create and update input types from your schema with mapped types, so a new column shows up in every input type at once.
- **Vue's** `readonly(state)` is typed with a deep version of `Readonly`, so components can read shared state but not assign to it.
