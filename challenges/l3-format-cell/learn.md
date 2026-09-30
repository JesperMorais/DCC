### Union types

A union type says "one of these":

```ts
type Id = string | number;
```

With a union you may only use what **all** members support. `id.toUpperCase()` is an error, because numbers don't have that method.

### Narrowing with `typeof`

Inside an `if` that checks the type, TypeScript *narrows* the variable to the matching member:

```ts
function describe(id: string | number): string {
  if (typeof id === "number") {
    return `#${id.toFixed(0)}`; // here id: number
  }
  return id.toUpperCase();      // here id: string — the only option left
}
```

`typeof` returns strings like `"string"`, `"number"`, `"boolean"`, `"undefined"`, `"function"` and `"object"`. Careful: `typeof null` is `"object"`, so check for `null` with `=== null`.
